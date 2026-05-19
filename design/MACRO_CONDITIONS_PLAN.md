# Macro Conditions — Implementation Plan

Feature: Add optional Gate and ScaleRange conditions to the macro knob system,
turning macros into intelligent conditional controllers.

---

## 1. Current State Analysis

### 1.1 MacroBank Data Flow

`MacroBank` holds 8 `Macro` structs per scope (Clip, Layer, Global). Each Macro has:
- `sourceSignalId` — 0 = Manual, >0 = Signal ID from SignalRegistry
- `manualValue` — direct knob value when in Manual mode
- `currentValue` — computed each frame by `updateValues()`
- `links` — vector of `MacroLink` targets (RouteTarget + outputMin/Max + invert)
- `id` — unique ID for use as a route source

**Per-frame flow:**
1. `MacroBank::updateValues(SignalRegistry&)` reads `getCachedValue(sourceSignalId)` for signal-driven macros, or uses `manualValue` for manual macros. Result stored in `currentValue`.
2. `RoutingEngine::processFrame()` notes: "Macro sources are handled by the MacroBank before routing" — macros are evaluated before routes, and routes can reference macro IDs as sources.
3. `MacroPanel::refresh()` calls `macroBank_->updateValues(*signalRegistry_)`, then syncs each knob's display value.

### 1.2 MacroPanel Rendering

`MacroPanel` renders 8 `MacroSlot`s in a single row. Each slot:
- A `Knob` widget (rotary slider + name label + value label + mapping indicator ring)
- A `TextButton` (source picker: "Manual" or signal name)
- `showSourcePicker()` opens a `PopupMenu` with Manual + all signals from `signalRegistry_`

Height: `kPreferredHeight = 110` (header + knob row + source button row).

### 1.3 Global Macro Bank Wiring

In `MainComponent.h`:
```cpp
MacroBank globalMacroBank_{MacroBank::Scope::Global};
```

In `MainComponent.cpp`:
```cpp
inspectorPanel_->setMacroBank(&globalMacroBank_);  // line 901
```

`InspectorPanel::setMacroBank()` propagates the SAME bank pointer to all three inspectors:
```cpp
clipInspector_.setMacroBank(bank);
layerInspector_.setMacroBank(bank);
compInspector_.setMacroBank(bank);
```

This means Clip, Layer, and Composition inspectors all display the single global bank.

### 1.4 Why Clip/Layer Scopes Aren't Wired

Several gaps prevent per-scope macro banks:

1. **No storage**: `Clip` and `Layer` structs have no `MacroBank` field. There is only `globalMacroBank_` in MainComponent.
2. **Single pointer propagation**: `InspectorPanel::setMacroBank()` passes one pointer to all three inspectors. There's no mechanism to swap which bank is displayed when the selected clip/layer changes.
3. **No serialization**: `MacroBank` has no `toVar()`/`fromVar()` methods. Even the global bank isn't saved with the composition.
4. **No ID assignment**: `MacroBank::assignIds()` exists but is never called for per-clip/per-layer banks.
5. **No scope switching**: `MacroPanel` has no concept of "this is showing Clip scope vs Layer scope" — it just shows whatever `MacroBank*` is passed in.

### 1.5 ChainedSignal — Existing Gate/ScaleRange Math

`ChainedSignal` already implements the exact logic we need:

```cpp
case ChainMode::Gate:
    return (modulator > gateThreshold_) ? carrier * gain_ : 0.0f;

case ChainMode::ScaleRange:
{
    float lo = modulationDepth_ * (1.0f - modulator);
    float hi = lo + modulator;
    return (lo + carrier * (hi - lo)) * gain_;
}
```

This is carrier/modulator architecture: carrier = main signal, modulator = conditioning signal. We can reuse or extract this math directly.

---

## 2. Phase 1: Wire Per-Clip and Per-Layer MacroBanks

### 2.1 Storage

**Approach: Embed MacroBank directly in Clip and Layer structs.**

This mirrors how `effects` (vector of EffectSlot) is stored directly on each struct.

```cpp
// In Clip (model/Clip.h):
MacroBank clipMacroBank{MacroBank::Scope::Clip};

// In Layer (model/Layer.h):
MacroBank layerMacroBank{MacroBank::Scope::Layer};
```

Forward-declare or include `routing/MacroBank.h` in both model headers. MacroBank is lightweight (8 Macros, each ~64 bytes = ~512 bytes total per bank), so embedding is fine.

**ID assignment**: When a Clip or Layer is created, call `assignIds()` with a scope-specific ID range:
- Global macros: IDs 1000-1007
- Layer macros: IDs 2000 + layerIndex*8 .. +7
- Clip macros: IDs 3000 + (layerIndex*maxCols + colIndex)*8 .. +7

### 2.2 Inspector Wiring

Change `InspectorPanel::setMacroBank()` to accept scope-aware banks:

```cpp
void InspectorPanel::setGlobalMacroBank(MacroBank* bank);
// Called once at startup with &globalMacroBank_
```

When the selected clip/layer changes (already handled in `InspectorPanel`), swap the MacroPanel pointer:

```cpp
// In InspectorPanel, when selected clip changes:
void InspectorPanel::onClipSelected(Clip* clip) {
    if (clip)
        clipInspector_.setMacroBank(&clip->clipMacroBank);
    else
        clipInspector_.setMacroBank(nullptr);
}

// Similarly for layer selection:
void InspectorPanel::onLayerSelected(Layer* layer) {
    if (layer)
        layerInspector_.setMacroBank(&layer->layerMacroBank);
    else
        layerInspector_.setMacroBank(nullptr);
}

// Composition inspector always uses global:
compInspector_.setMacroBank(&globalMacroBank_);
```

### 2.3 Serialization

Add `toVar()`/`fromVar()` to `MacroBank`:

```cpp
// In MacroBank (routing/MacroBank.h):
juce::var toVar() const;
void fromVar(const juce::var& v);
```

Each Macro serializes: `name`, `manualValue`, `sourceSignalId`, `links[]`. The `currentValue` is runtime-only, not serialized.

**Integration points:**
- `Clip::toVar()` / `Clip::fromVar()` — serialize `clipMacroBank`
- `Layer::toVar()` / `Layer::fromVar()` — serialize `layerMacroBank`
- `Composition::toVar()` / `Composition::fromVar()` — serialize `globalMacroBank` (add a field to Composition or serialize separately in MainComponent)

**Backward compatibility**: If the `"macroBank"` key is missing from JSON, `fromVar()` is a no-op — all macros stay at defaults.

### 2.4 Update Loop

Per-clip and per-layer macro banks need to be updated each frame alongside the global bank.

In MainComponent's timer callback (or wherever `globalMacroBank_.updateValues(signalRegistry_)` is called — currently it's called in `MacroPanel::refresh()`), also update the active clip and layer banks:

```cpp
// In the UI refresh timer:
globalMacroBank_.updateValues(signalRegistry_);

if (auto* deck = composition_.getActiveDeck()) {
    for (auto& layer : deck->layers) {
        layer.layerMacroBank.updateValues(signalRegistry_);
        if (auto* clip = layer.getActiveClip())
            clip->clipMacroBank.updateValues(signalRegistry_);
    }
}
```

Only update the ACTIVE clip per layer (not all clips), to avoid unnecessary signal reads.

### 2.5 Estimated Work

| File | Change | LOC |
|------|--------|-----|
| model/Clip.h | Add `MacroBank clipMacroBank` field, include header | +5 |
| model/Layer.h | Add `MacroBank layerMacroBank` field, include header | +5 |
| routing/MacroBank.h | Add `toVar()`/`fromVar()` declarations | +5 |
| routing/MacroBank.cpp (new) | Implement `toVar()`/`fromVar()` | +60 |
| model/Clip.cpp | Serialize clipMacroBank in toVar/fromVar | +10 |
| model/Layer.cpp | Serialize layerMacroBank in toVar/fromVar | +10 |
| ui/InspectorPanel.h/.cpp | Scope-aware macro bank propagation | +20 |
| MainComponent.cpp | Update loop for per-scope banks, ID assignment | +25 |

---

## 3. Phase 2: Add Macro Conditions

### 3.1 Data Model Changes

Extend the `Macro` struct inside `MacroBank`:

```cpp
struct Macro
{
    // ... existing fields ...

    // === Condition (optional gate/scale on the macro output) ===
    bool conditionEnabled = false;

    enum class ConditionType : uint8_t { Gate, ScaleRange };
    ConditionType conditionType = ConditionType::Gate;

    uint32_t conditionSignalId = 0;  // Signal that controls the condition
    float conditionThreshold = 0.1f; // Gate: signal must exceed this to pass

    // ScaleRange: condition signal maps output into [rangeMin, rangeMax]
    float conditionRangeMin = 0.0f;
    float conditionRangeMax = 1.0f;
};
```

### 3.2 Signal Evaluation Changes

Modify `MacroBank::updateValues()` to apply conditions after computing the base value:

```cpp
void updateValues(const SignalRegistry& signals)
{
    for (auto& macro : macros_)
    {
        // Step 1: Compute base value (unchanged)
        float baseValue;
        if (macro.isManual())
            baseValue = macro.manualValue;
        else
            baseValue = signals.getCachedValue(macro.sourceSignalId);

        // Step 2: Apply condition (new)
        if (macro.conditionEnabled && macro.conditionSignalId != 0)
        {
            float condSignal = signals.getCachedValue(macro.conditionSignalId);

            switch (macro.conditionType)
            {
                case Macro::ConditionType::Gate:
                    // Pass baseValue when condition signal exceeds threshold; else 0
                    baseValue = (condSignal > macro.conditionThreshold)
                                ? baseValue : 0.0f;
                    break;

                case Macro::ConditionType::ScaleRange:
                    // Condition signal maps output range from rangeMin to rangeMax
                    // When condSignal=0, output is at rangeMin end
                    // When condSignal=1, output is at rangeMax end
                    {
                        float lo = macro.conditionRangeMin;
                        float hi = macro.conditionRangeMax;
                        baseValue = lo + baseValue * (hi - lo) * condSignal
                                  + baseValue * (1.0f - condSignal) * lo;
                        // Simpler: lerp the output range based on condSignal
                        float scaledMin = lo + (1.0f - lo) * (1.0f - condSignal);
                        float scaledMax = hi * condSignal + (1.0f - condSignal);
                        baseValue = scaledMin + baseValue * (scaledMax - scaledMin);
                    }
                    break;
            }
        }

        macro.currentValue = baseValue;
    }
}
```

**Note on ChainedSignal reuse**: ChainedSignal's Gate/ScaleRange math operates on carrier/modulator signals looked up by ID from the registry. For macros, we already have the values in hand (baseValue and condSignal), so we can inline the math directly rather than creating ChainedSignal instances. The logic is 2-3 lines per mode — extracting a shared function would add abstraction without meaningful code savings.

However, if we later add more chain modes (Multiply, Add), we should extract a shared `applyChainMath(ChainMode, float carrier, float modulator, float threshold, float rangeMin, float rangeMax)` static function into a utility header and have both ChainedSignal and MacroBank call it.

### 3.3 UI Changes

#### 3.3.1 MacroPanel Condition Row

When a macro has a source signal assigned (not Manual), show an additional condition row below the source button. This follows the UniversalParamControl pattern of expanding to show more controls.

**New layout per slot (when source is assigned):**

```
[   Knob   ]
[ Source Btn ]
[ Cond. Row  ]   <-- NEW: only visible when sourceSignalId != 0
```

The Condition Row contains:
- **Enable toggle** (small checkbox, 16px)
- **Type dropdown** (Gate / ScaleRange, 70px)
- **Condition signal picker button** (shows signal name or "Pick...", remaining width)

When conditionEnabled is true AND conditionType is selected:
- **Gate**: Show a threshold slider below the condition row
- **ScaleRange**: Show Range Min and Range Max sliders below the condition row

This adds at most 2 extra rows (condition config + threshold/range) per macro slot that has conditions enabled.

**Height adjustment**: `MacroPanel::kPreferredHeight` becomes dynamic:

```cpp
int getPreferredHeight() const
{
    int base = 110; // header + knobs + source buttons
    if (macroBank_) {
        for (int i = 0; i < MacroBank::kNumMacros; ++i) {
            auto& m = macroBank_->getMacro(i);
            if (!m.isManual()) base += 18;       // condition row always shown for signal-driven
            if (m.conditionEnabled) base += 20;  // threshold or range sliders
        }
    }
    return base;
}
```

**Alternative (simpler, recommended for v1)**: Only show condition controls when the user clicks a "gear" icon on a macro slot, opening a floating popup editor. This avoids MacroPanel height changes entirely and keeps the dashboard row compact.

**Recommended approach for v1**: Add a small condition indicator dot on each macro knob (orange when condition is active, hidden otherwise). Right-click or long-press on the source button opens a popup with condition settings. This matches Resolume's pattern of keeping the dashboard compact with detail editing in popups.

#### 3.3.2 Condition Popup Editor

Triggered by right-click on a macro's source button (when source is assigned):

```
+----------------------------------+
| Condition                    [X] |
+----------------------------------+
| [x] Enable                      |
| Type: [Gate       v]            |
| Signal: [Bass     v]            |
| Threshold: [====|=========]     |
+----------------------------------+
```

For ScaleRange, replace Threshold with:
```
| Range Min: [===|===========]    |
| Range Max: [==========|===]    |
```

**Signal picker**: Reuse the same popup menu pattern from `MacroPanel::showSourcePicker()` — iterate `signalRegistry_->getNumSignals()` and list all signals.

#### 3.3.3 Condition Indicator on Knob

Add a visual indicator to `Knob` for active conditions:

```cpp
// In Knob:
void setConditionIndicator(bool active, const juce::String& condTypeName);
```

When active, draw a small colored dot (orange) at the bottom-right of the knob area, with a tooltip showing "Gate: Bass > 0.10" or "ScaleRange: Energy [0.2 - 0.8]".

### 3.4 Serialization

Extend the new `MacroBank::toVar()`/`fromVar()` (from Phase 1) to include condition fields:

```cpp
// In Macro serialization:
obj->setProperty("conditionEnabled", macro.conditionEnabled);
obj->setProperty("conditionType", static_cast<int>(macro.conditionType));
obj->setProperty("conditionSignalId", static_cast<int>(macro.conditionSignalId));
obj->setProperty("conditionThreshold", static_cast<double>(macro.conditionThreshold));
obj->setProperty("conditionRangeMin", static_cast<double>(macro.conditionRangeMin));
obj->setProperty("conditionRangeMax", static_cast<double>(macro.conditionRangeMax));
```

**Backward compatibility**: `fromVar()` checks for each key's existence. Missing keys default to `conditionEnabled = false`, so compositions saved without conditions load cleanly.

### 3.5 Estimated Work

| File | Change | LOC | Risk |
|------|--------|-----|------|
| routing/MacroBank.h | Extend Macro struct with condition fields | +15 | LOW |
| routing/MacroBank.h | Modify updateValues() with condition logic | +25 | MED |
| routing/MacroBank.cpp | Extend toVar/fromVar with condition fields | +30 | LOW |
| ui/MacroPanel.h | Add condition popup state, indicator support | +15 | LOW |
| ui/MacroPanel.cpp | Condition popup editor, right-click handler | +80 | MED |
| ui/Knob.h | Add setConditionIndicator() | +5 | LOW |
| ui/Knob.cpp | Draw condition indicator dot | +15 | LOW |

---

## 4. Phase 3: MappingSuggester Integration (Stretch)

### 4.1 Current State

`MappingSuggester` generates `Suggestion` structs containing:
- `sourceName` — which audio feature to use (e.g., "Bass", "Beat Phase")
- `targetCategory` / `targetEffect` / `targetParam` — which effect parameter to drive
- `curveType` — recommended transform curve
- `reason` — human-readable explanation
- `relevance` — 0-1 score

It has two entry points:
- `suggestMappings(FeatureSnapshot)` — based on current audio
- `suggestGenreMappings(uint8_t genre)` — based on detected genre

### 4.2 Condition Suggestions

Extend `MappingSuggester::Suggestion` with optional condition recommendations:

```cpp
struct Suggestion
{
    // ... existing fields ...

    // Condition recommendation (new)
    bool suggestCondition = false;
    std::string conditionType;        // "Gate" or "ScaleRange"
    std::string conditionSignalName;  // e.g., "Energy State", "Hit"
    float conditionThreshold = 0.1f;
    std::string conditionReason;      // Why this condition helps
};
```

Example genre-specific condition suggestions:
- **Techno**: "Gate bass->ripple by structural state" (only ripple during breakdowns)
- **Ambient**: "ScaleRange volume->opacity with Energy [0.3-0.8]" (compress dynamics)
- **DnB**: "Gate hit->glitch by beat phase > 0.75" (glitch only on off-beats)

### 4.3 UI Integration

Surface condition suggestions in the existing "Suggest Mappings" UI (wherever that is currently shown). When a suggestion includes `suggestCondition = true`, the suggestion card shows an additional line:

```
Bass -> Ripple intensity (Exponential)
  Condition: Gate by Energy State > 0.10
  "Reduces visual chaos during low-energy sections"
```

Accepting the suggestion would:
1. Assign the source signal to an available macro
2. Link the macro to the target parameter
3. Enable the condition with the suggested settings

### 4.4 Estimated Work

| File | Change | LOC | Risk |
|------|--------|-----|------|
| mapping/MappingSuggester.h | Extend Suggestion struct | +10 | LOW |
| mapping/MappingSuggester.cpp | Add condition logic to genre suggestions | +50 | LOW |
| UI (TBD) | Show condition info in suggestion cards | +30 | LOW |

---

## 5. File Change List

### Phase 1 — Per-Scope MacroBanks

| File | What Changes | Est. LOC | Risk |
|------|-------------|----------|------|
| `src/routing/MacroBank.h` | Add `toVar()`/`fromVar()` declarations | +5 | LOW |
| `src/routing/MacroBank.cpp` (NEW) | Implement serialization | +60 | LOW |
| `src/model/Clip.h` | Add `MacroBank clipMacroBank` field, include header | +5 | LOW |
| `src/model/Layer.h` | Add `MacroBank layerMacroBank` field, include header | +5 | LOW |
| `src/model/Clip.cpp` | Serialize clipMacroBank in toVar/fromVar | +10 | LOW |
| `src/model/Layer.cpp` | Serialize layerMacroBank in toVar/fromVar | +10 | LOW |
| `src/model/Composition.h` | Add global MacroBank serialization hookpoint | +5 | LOW |
| `src/ui/InspectorPanel.h` | Change `setMacroBank()` to scope-aware API | +10 | LOW |
| `src/ui/InspectorPanel.cpp` | Swap macro bank pointer on clip/layer selection | +20 | MED |
| `src/ui/ClipInspector.h/.cpp` | No change needed (already accepts MacroBank*) | 0 | -- |
| `src/ui/LayerInspector.h/.cpp` | No change needed (already accepts MacroBank*) | 0 | -- |
| `src/ui/CompositionInspector.h/.cpp` | No change needed | 0 | -- |
| `src/MainComponent.h` | No change (globalMacroBank_ stays) | 0 | -- |
| `src/MainComponent.cpp` | Update loop for per-scope banks, ID assignment | +25 | MED |
| `CMakeLists.txt` | Add MacroBank.cpp to build | +1 | LOW |

**Phase 1 total**: ~156 LOC added/modified

### Phase 2 — Macro Conditions

| File | What Changes | Est. LOC | Risk |
|------|-------------|----------|------|
| `src/routing/MacroBank.h` | Extend Macro struct with condition fields | +15 | LOW |
| `src/routing/MacroBank.h` | Modify `updateValues()` with condition logic | +25 | MED |
| `src/routing/MacroBank.cpp` | Extend toVar/fromVar with condition fields | +30 | LOW |
| `src/ui/MacroPanel.h` | Add condition popup state, getPreferredHeight change | +15 | LOW |
| `src/ui/MacroPanel.cpp` | Condition popup editor (right-click), signal picker, refresh | +80 | MED |
| `src/ui/Knob.h` | Add `setConditionIndicator()` method | +5 | LOW |
| `src/ui/Knob.cpp` | Draw condition indicator dot + tooltip | +15 | LOW |
| `src/ui/ClipInspector.cpp` | Height calculation update (getPreferredHeight) | +5 | LOW |
| `src/ui/LayerInspector.cpp` | Height calculation update | +5 | LOW |
| `src/ui/CompositionInspector.cpp` | Height calculation update | +5 | LOW |

**Phase 2 total**: ~200 LOC added/modified

### Phase 3 — MappingSuggester (Stretch)

| File | What Changes | Est. LOC | Risk |
|------|-------------|----------|------|
| `src/mapping/MappingSuggester.h` | Extend Suggestion struct | +10 | LOW |
| `src/mapping/MappingSuggester.cpp` | Genre-aware condition suggestions | +50 | LOW |
| UI file (TBD) | Display condition info in suggestion cards | +30 | LOW |

**Phase 3 total**: ~90 LOC added/modified

---

## 6. Testing Strategy

### 6.1 Unit Tests

**MacroBank with conditions** (`tests/MacroBankTest.cpp`):
- Manual macro with no condition: currentValue = manualValue
- Signal-driven macro with no condition: currentValue = signal value
- Gate condition, signal above threshold: currentValue = base value
- Gate condition, signal below threshold: currentValue = 0.0
- Gate condition, threshold edge case (exactly at threshold): must be consistent
- ScaleRange condition: output correctly remapped based on condition signal
- ScaleRange with rangeMin=0, rangeMax=1, condSignal=0.5: half-range compression
- Condition enabled but conditionSignalId=0: condition is effectively skipped
- Condition disabled: no effect regardless of other condition fields

**MacroBank serialization** (`tests/MacroBankSerializationTest.cpp`):
- Round-trip: toVar() -> fromVar() produces identical state
- Missing condition fields in JSON: defaults to conditionEnabled=false
- Missing entire macroBank key: no crash, defaults used
- Backward compat: load a v1 composition (no macroBank key) into v2 code

### 6.2 Visual Tests (Eyes API)

- Macro with Gate active: verify knob stops moving when condition signal drops below threshold
- Macro with ScaleRange: verify output range compression is visible on the knob's arc
- Condition indicator dot: verify orange dot appears/disappears when condition is toggled
- Condition popup: verify popup opens on right-click, fields are populated correctly

### 6.3 Integration Tests

- Create a composition with per-clip macros, save, reload, verify macros restored
- Assign a condition, save, reload, verify condition restored
- Load a pre-conditions composition: verify no macros are lost, conditions default to off
- Performance: 8 macros * 3 scopes * condition evaluation adds negligible overhead (< 0.01ms per frame)

---

## 7. Implementation Order

Each task is independently testable and builds on the previous.

### Phase 1: Per-Scope MacroBanks

| # | Task | Depends On | Verification |
|---|------|-----------|-------------|
| 1.1 | Add `toVar()`/`fromVar()` to MacroBank (new .cpp file) | None | Unit test: round-trip serialization |
| 1.2 | Add `MacroBank clipMacroBank` to Clip struct | 1.1 | Compiles; Clip.toVar/fromVar includes macroBank |
| 1.3 | Add `MacroBank layerMacroBank` to Layer struct | 1.1 | Compiles; Layer.toVar/fromVar includes macroBank |
| 1.4 | Wire InspectorPanel to swap macro banks on selection | 1.2, 1.3 | Select clip in DeckView -> ClipInspector shows per-clip macros |
| 1.5 | Add per-scope updateValues() calls in MainComponent timer | 1.4 | Signal-driven clip macro updates in real time |
| 1.6 | Add globalMacroBank serialization in Composition save/load | 1.1 | Save composition, reload, global macros preserved |
| 1.7 | Backward compat test: load pre-macro composition | 1.6 | No crash, defaults used |

### Phase 2: Macro Conditions

| # | Task | Depends On | Verification |
|---|------|-----------|-------------|
| 2.1 | Extend Macro struct with condition fields | 1.1 | Compiles |
| 2.2 | Implement condition logic in `updateValues()` | 2.1 | Unit tests: Gate pass/block, ScaleRange remap |
| 2.3 | Extend MacroBank serialization for condition fields | 2.1 | Round-trip test with conditions |
| 2.4 | Add condition indicator to Knob widget | 2.1 | Visual: orange dot when condition active |
| 2.5 | Add condition popup editor to MacroPanel | 2.1, 2.2 | Right-click source button -> popup with condition controls |
| 2.6 | Wire condition popup to MacroBank state | 2.5 | Change condition in popup -> macro behavior changes in real time |
| 2.7 | Update inspector height calculations | 2.5 | Inspector scrolls correctly with condition UI |
| 2.8 | Backward compat test: load pre-condition composition | 2.3 | No crash, conditionEnabled=false by default |

### Phase 3: MappingSuggester (Stretch)

| # | Task | Depends On | Verification |
|---|------|-----------|-------------|
| 3.1 | Extend Suggestion struct with condition fields | 2.2 | Compiles |
| 3.2 | Add genre-aware condition suggestions | 3.1 | Unit test: Techno genre returns Gate suggestions |
| 3.3 | Surface condition suggestions in UI | 3.2 | Suggestion card shows condition line |

---

## Constraints Checklist

- [x] No render pipeline or effect system changes — control layer only
- [x] No new UI panels — extend existing MacroPanel and source picker
- [x] Only Gate and ScaleRange for v1 (not Multiply/Add)
- [x] Backward compatibility — missing condition/macroBank fields = safe defaults
- [x] Follows UniversalParamControl's source picker pattern (popup menu with signal list)
- [x] ChainedSignal math reusable — same Gate/ScaleRange formulas, inlined in updateValues()
