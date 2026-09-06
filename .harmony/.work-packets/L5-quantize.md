# L5 — Quantize work packet

## VERDICT: BUILDABLE — one choke point does exist for every external trigger form; MIDI/OSC bypass risk named in the brief does NOT materialize (verified end to end). The "missing consumer" mostly already exists (per-clip beat-snap queue) — this lane extends it to read the global control instead of building a queue from scratch.

## SIZE: small-medium. 4 production files touched, ~60-90 changed/added lines total, zero test-signature breaks (all new params are defaulted). No new files.
- `src/model/Layer.h` (~20 lines: 1 new field, 1 signature+body change, 1 processPendingTrigger tweak)
- `src/model/Deck.h` (~4 lines: 1 signature+body change)
- `src/core/DeckCommands.h` (~5 lines: 1 new struct field + 3 functions touched)
- `src/MainComponent.cpp` (~20 lines: 2 call sites + 1 small static helper, plus a BPM-lock guard)

## CURRENT BEHAVIOUR (verified) — what happens today

**The dead control.** `Composition::QuantizeMode` (`Off, NextBeat, NextDownbeat`) lives at
`src/model/Composition.h:51-52` (`enum class QuantizeMode : uint8_t { Off, NextBeat, NextDownbeat };` /
`QuantizeMode quantizeMode = QuantizeMode::Off;`). It round-trips through save/load
(`Composition.h:170` write, `Composition.h:261` read) and is exercised by one test
(`tests/test_composition.cpp:141,178`, a pure serialization round-trip — it never checks that
anything CONSUMES the value).

The UI is `TopBar`'s `quantizeSelector_` (`src/ui/TopBar.cpp:152-163`):
```
quantizeSelector_.addItem("Off", 1);
quantizeSelector_.addItem("Next Beat", 2);
quantizeSelector_.addItem("Next Downbeat", 3);
quantizeSelector_.setSelectedId(1, juce::dontSendNotification);
quantizeSelector_.onChange = [this] {
    auto mode = static_cast<Composition::QuantizeMode>(quantizeSelector_.getSelectedId() - 1);
    composition_.quantizeMode = mode;
    if (onQuantizeChanged) onQuantizeChanged(mode);
};
```
**Zero-consumer, proven two ways:**
- Pattern A — broad: `grep -rn "quantizeMode" src/ tests/` → every hit is either the enum
  declaration, the field declaration, the two serialization lines, the one write at
  `TopBar.cpp:160`, or the round-trip test. No third hit anywhere.
- Pattern B — narrow (a real consumer would branch on it): `grep -rn "quantizeMode\s*==" src/ tests/`
  → the ONLY hit is `tests/test_composition.cpp:178`'s `REQUIRE(loaded.quantizeMode == ...)`,
  which is the round-trip assertion, not production logic.
- The callback the combobox fires, `onQuantizeChanged` (`TopBar.h:28`), is never assigned by
  `MainComponent` — `grep -n "onQuantizeChanged" src/ tests/` returns only its declaration and
  its two use-sites inside `TopBar.cpp` itself. **Not wired even at the plumbing level.**

**Adjacent, pre-existing bug found in passing (not part of this lane's fence, flagged for
awareness):** `quantizeSelector_` is set to "Off" exactly once, at construction
(`TopBar.cpp:156`), and never re-synced. After File > Open loads a composition whose saved
`quantizeMode` was NextBeat/NextDownbeat, the dropdown will show "Off" while
`composition_.quantizeMode` is actually correct underneath (the field itself loads fine via
`Composition.h:261`). This is a UI-only desync, not a playback-correctness bug (playback reads
the field, not the widget), and it is NOT unique to Quantize — no TopBar selector I found
resyncs after load. Out of scope for this lane; see OUT OF SCOPE.

## ROOT CAUSE (verified)

Nothing reads `composition_.quantizeMode`. The control writes a field that never gets consulted
by any trigger path.

## THE CENTRAL QUESTION — every way a clip/cell is triggered, traced end to end

**Method: enumerate every write-site, then trace each to the function that actually flips
`Layer::activeClipColumn`/`Clip::playing`.**

### 1. Mouse (deck-cell click / column-header click)
`DeckView` fires `onClipTriggered`/`onColumnTriggered`, wired at
`src/MainComponent.cpp:635-639`:
```
deckView_->onClipTriggered = [this](int layerIdx, int col) { handleClipTrigger(layerIdx, col); };
deckView_->onColumnTriggered = [this](int col) { handleColumnTrigger(col); };
```

### 2. REST API
`src/MainComponent.cpp:1678-1679`:
```
apiServer_->onTriggerClip = [this](int layer, int column) { handleClipTrigger(layer, column); };
apiServer_->onTriggerColumn = [this](int column) { handleColumnTrigger(column); };
```
`ApiServer` itself marshals to the message thread BEFORE invoking these — verified at
`src/api/ApiServer.cpp:332-336` (`onTriggerClip`) and `:354-358` (`onTriggerColumn`), each wrapped
in `juce::MessageManager::callAsync(...)`. So the raw (non-callAsync) assignment at
MainComponent.cpp:1678-1679 is safe — the hop already happened inside ApiServer.

### 3. OSC
`src/MainComponent.cpp:1706-1708`, explicitly re-wrapped in `callAsync` here too (belt-and-braces
with #2, harmless double-hop onto the same thread):
```
oscHandler_.onTriggerClip = [this](int layer, int column) {
    juce::MessageManager::callAsync([this, layer, column]() { handleClipTrigger(layer, column); });
};
```
(`OscHandler` has no `onTriggerColumn`; column-trigger is not exposed over OSC today — pre-existing,
not this lane's problem.)

### 4. Keyboard AND 5. MIDI note — same fan-in, one dispatcher
Both terminate in `BindingManager`'s generic `actionCallback_`, set once at
`src/MainComponent.cpp:1618-1621`:
```
bindingManager_.setActionCallback([this](const Binding& b, float val) { handleBindingAction(b, val); });
```
- Keyboard: `MainComponent::keyPressed` (`MainComponent.cpp:2348`) falls through to
  `bindingManager_.processKeyDown(...)` (`MainComponent.cpp:2451-2456`) when no hard-coded
  shortcut consumed the key.
- MIDI note-on: `MidiHandler::handleIncomingMidiMessage` (`src/midi/MidiHandler.cpp:76-84`) posts
  `bindingManager_.processMidiNoteOn(...)` via `callAsync` from the MIDI thread → message thread.
- `BindingManager::processKeyDown`/`processMidiNoteOn` (`src/binding/BindingManager.cpp:52-76`,
  `:104-127`) both just look up a matching `Binding` and call `actionCallback_(b, value)` — no
  clip logic lives in BindingManager itself.
- `MainComponent::handleBindingAction` (`MainComponent.cpp:5081`) is the real dispatcher. Its
  `Binding::Action::TriggerClip` case calls `handleClipTrigger(resolvedLayer, resolvedColumn)`
  (`MainComponent.cpp:5148`); its `Action::TriggerColumn` case calls
  `handleColumnTrigger(resolvedColumn)` (`MainComponent.cpp:5184`).
- `Binding::Action` (`src/binding/Binding.h:24-44`) has 19 values total; only `TriggerClip` and
  `TriggerColumn` ever start a NEW column's playback. Everything else (toggles, `LayerTransport`,
  `SwitchDeck`, `TapTempo`, `MasterOpacity`, `Snapshot`, `ToggleRecording`, …) does not choose a
  column, so it is out of scope for "trigger.")
- MIDI CC (`processMidiCC`, `BindingManager.cpp:152-201`) also funnels through the same
  `actionCallback_` — if a user configures a CC binding with `Action::TriggerClip` (the model does
  not forbid it) it lands in the exact same `handleClipTrigger` call. Same choke, no special case
  needed.

**Conclusion for 1-5: every external/user-initiated trigger, from any input device, reaches
exactly one of two message-thread functions: `MainComponent::handleClipTrigger` (single cell) or
`MainComponent::handleColumnTrigger` (whole column).** The brief's named risk — "MIDI/OSC bypass" —
is DISPROVEN, verified by tracing each path's literal call chain above, not inferred.

### 6. Autopilot (the one real bypass — and it is CORRECT to bypass)
`Layer::triggerClip` is called directly, three times, from `src/model/Autopilot.cpp:288`, `:310`,
`:372` (`advanceClip`/`smartAdvanceClip`), on the **GL/render thread**, never touching
`handleClipTrigger`/`handleColumnTrigger`. `src/core/TriggerCommands.h:28-32` documents this
deliberately: "command creation lives ONLY in the handlers, never inside `Layer::triggerClip` —
autopilot calls `Layer::triggerClip` directly from the GL render thread... so autopilot-driven
triggers create NO commands." Autopilot's own triggers are already beat-gated (only fires inside
`Autopilot::processFrame`'s `if (!beatCrossed) return;`, `Autopilot.cpp:44-48`) — quantizing them
again would be double-gating a trigger that is already on-beat by construction. **Recommendation:
autopilot must NOT be routed through the new global-quantize gate.** (See THE CHANGE — the gate is
added at the choke points, not inside `Layer::triggerClip`'s unconditional path, so autopilot's 3
call sites are untouched by construction.)

### 7. Undo/redo replay (also a correct bypass)
`TriggerClipCmd::execute()`/`undo()` (`src/core/TriggerCommands.h:68-69`) call `apply()`
(`:104-113`), which writes the captured `LayerRuntimeSnapshot` and `Clip::playing` fields directly
— it never calls `Layer::triggerClip()` at all. This is intentional (mutate-then-push replay of a
snapshot, not a fresh trigger decision) and correctly excluded from quantize: replaying history
must be instantaneous, not re-deferred to the next beat.

### 8. Genre auto-switch — NOT a clip trigger (corrects an implicit assumption in the essentials
plan, which listed it alongside autopilot/playlist-advance as something that "consumes the beat"
in a trigger-relevant way)
`previewPanel_.getRenderer().setOnGenreChanged(...)` (`MainComponent.cpp:601-624`) only calls
`handleDeckSwitch(deckIdx)` — it changes which DECK is active. It never calls `handleClipTrigger`,
`handleColumnTrigger`, or `Layer::triggerClip`. Whatever clip was already active on the
newly-active deck stays active, untouched. There is no "start playback" event here to quantize.

### 9. "Playlist advance" — also NOT a clip trigger; it's MilkDrop-preset cycling WITHIN a clip
Confirmed at `src/render/Renderer.cpp:294-330` (P20.5): this advances `clip->presetPlaylistIndex`
— which MilkDrop preset an already-playing MilkDrop clip is showing — on its own independent
beat-crossing detector (`lastPlaylistBeatPhase_`, `Renderer.h:406`, same
`beatPhase < last - 0.5f` wrap pattern as Autopilot's, but a SEPARATE variable — beat-crossing
detection is duplicated ad hoc in this codebase, not centralized). It never calls
`Layer::triggerClip`. Not a trigger path.

### 10. Cuepoint jump — also NOT a clip trigger
`onCuepointJump` (`MainComponent.cpp:1387-1400`) seeks the ALREADY-active clip's player to a
timeline position. No column selection happens. Not in scope.

**ANSWER TO THE CENTRAL QUESTION: there is one true choke point for every trigger form that
represents a live/external decision to start a DIFFERENT column** — `Layer::triggerClip()`
(`src/model/Layer.h:190`), reached either directly (`handleClipTrigger`,
`MainComponent.cpp:3277`) or once per non-ignoring layer via `Deck::triggerColumn`
(`src/model/Deck.h:108-116`, called from `handleColumnTrigger`, `MainComponent.cpp:3434`).
Autopilot and undo/redo reach playback state through different, deliberately separate paths that
should NOT be gated by quantize (see #6, #7). Genre auto-switch and playlist-advance are not
trigger paths at all (see #8, #9).

## THE EXISTING INFRASTRUCTURE (verified) — a pending-trigger queue, consumed on beat crossing,
## already ships today — just not wired to the global control

This is the single most load-bearing finding of the survey: **the thing L5 says to build already
exists**, for a DIFFERENT, PER-CLIP control.

- Every clip has its own `Clip::BeatSnapMode beatSnapMode` (`Off/Beat/Bar/TwoBar/FourBar`,
  `src/model/Clip.h:72-80`), set via `ClipInspector`'s `beatSnapSelector_`
  (`src/ui/ClipInspector.cpp:161-172`), tooltip literally reads "Quantize clip trigger to next
  beat/bar boundary" (`ClipInspector.cpp:167`) — this is a SECOND, WORKING quantize control, at
  the per-clip level, that the essentials-plan text does not appear to have found.
- `Layer::triggerClip` (`Layer.h:190-214`) already checks it: if the target clip's
  `beatSnapMode != Off` (or legacy `beatSnap`) and it's not a same-column retrigger, it sets
  `pendingTriggerColumn = column;` and returns WITHOUT playing — a real pending-trigger queue,
  today, in production.
- `Layer::processPendingTrigger(beatInBar, barCount)` (`Layer.h:254-286`) is the drain: it reads
  the pending clip's `beatSnapMode` to pick a granularity (`Beat/Bar/TwoBar/FourBar`) and fires
  `triggerClipImmediate(pendingTriggerColumn)` when the granularity condition is met.
- The drain is called from exactly one place: `Autopilot::processFrame`
  (`src/model/Autopilot.cpp:52-60`):
  ```
  for (auto& layer : deck.layers)
      if (layer.pendingTriggerColumn >= 0)
          layer.processPendingTrigger(snapshot.beatInBar, snapshot.barCount);
  ```
  **This loop is UNCONDITIONAL across all layers** — it runs regardless of each layer's
  `autopilotEnabled` flag (verified: the `autopilotEnabled` gate only wraps the LATER
  beat-based-advancement loop, `Autopilot.cpp:62-65`, not this one). So the pending-trigger drain
  already works whether or not autopilot is turned on for any layer.
- `Autopilot::processFrame` itself is called unconditionally every GL frame whenever a deck is
  active — `src/render/Renderer.cpp:257` (`autopilot_.processFrame(*deck, snap)`), gated only on
  `deckActive`, not on any global "autopilot enabled" setting. **So the drain runs continuously
  during normal use, on the GL thread.**
- Beat-crossing edge detection: `bool beatCrossed = (snapshot.beatPhase < lastBeatPhase_ - 0.5f);`
  (`Autopilot.cpp:44-45`) — a wrap-around detector on the FeatureBus's `beatPhase` sawtooth.

**Consequence:** L5's design does not need a new queue, a new drain loop, or a new beat-crossing
detector. It needs the EXISTING queue to also be reachable from `composition_.quantizeMode`, not
only from a clip's own `beatSnapMode`.

## THE CHANGE — step by step

**Design principle:** gate at the two message-thread choke points
(`handleClipTrigger`/`handleColumnTrigger`), not inside `Layer::triggerClip` itself unconditionally
— this keeps Autopilot's 3 direct calls (#6 above) and all 19 test call sites completely
unaffected by construction (new parameter is defaulted).

1. **`src/model/Layer.h`**
   - Add a new runtime field right after the existing one, `Layer.h:157`
     (`int pendingTriggerColumn = -1;  // Beat snap: queued trigger awaiting next beat`):
     ```
     Clip::BeatSnapMode pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
     // Set alongside pendingTriggerColumn whenever a trigger is queued. Off means
     // "derive granularity from the target clip's own beatSnapMode" (today's
     // behavior, unchanged). Non-Off means a caller (global Quantize) is FORCING
     // a granularity for this one queued trigger, overriding the clip's own field
     // for this trigger only — the clip's own beatSnapMode is never mutated.
     ```
   - Change `triggerClip`'s signature (`Layer.h:190`):
     `void triggerClip(int column, Clip::BeatSnapMode forcedSnap = Clip::BeatSnapMode::Off)`
   - In the empty-cell branch (`Layer.h:196-201`), also reset the new field:
     `pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;` alongside the existing
     `pendingTriggerColumn = -1;`.
   - Change the queue decision (`Layer.h:203-211`) from
     `bool snapEnabled = clipOpt->beatSnapMode != Off || clipOpt->beatSnap;`
     to
     `bool snapEnabled = forcedSnap != Off || clipOpt->beatSnapMode != Off || clipOpt->beatSnap;`
     and when queuing, also set `pendingTriggerSnapOverride = forcedSnap;` (this correctly stores
     `Off` when the trigger was queued purely because of the CLIP's own setting — the existing
     per-clip-only path is unchanged).
   - In `triggerClipImmediate` (`Layer.h:218-223`), also reset
     `pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;` alongside `pendingTriggerColumn = -1;`.
   - In `processPendingTrigger` (`Layer.h:254-286`), change the granularity derivation
     (`Layer.h:259-264`) to prefer the override when set:
     ```
     auto snapMode = Clip::BeatSnapMode::Beat; // default
     if (pendingTriggerSnapOverride != Clip::BeatSnapMode::Off) {
         snapMode = pendingTriggerSnapOverride;               // global quantize forced this
     } else if (clipOpt.has_value()) {
         snapMode = (clipOpt->beatSnapMode != Clip::BeatSnapMode::Off)
                    ? clipOpt->beatSnapMode : Clip::BeatSnapMode::Beat;   // unchanged fallback
     }
     ```

2. **`src/model/Deck.h`** — thread the forced mode through the column-wide path
   (`Deck.h:108-116`):
   ```
   void triggerColumn(int col, Clip::BeatSnapMode forcedSnap = Clip::BeatSnapMode::Off)
   {
       for (auto& layer : layers)
       {
           if (layer.ignoreColumnTrigger) continue;
           layer.triggerClip(col, forcedSnap);
       }
   }
   ```

3. **`src/core/DeckCommands.h`** — undo/redo correctness for the new field (see TRAPS for why this
   is not optional):
   - Add to `LayerRuntimeSnapshot` (`DeckCommands.h:182-188`):
     `Clip::BeatSnapMode pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;`
   - Extend `operator==` (`:190-196`) to compare it.
   - Extend `captureLayerRuntime` (`:198-202`) to capture `layer.pendingTriggerSnapOverride`.
   - Extend `applyLayerRuntime` (`:204-210`) to restore `layer.pendingTriggerSnapOverride`.

4. **`src/MainComponent.cpp`** — the two call sites, plus the BPM-lock guard (see the "real risk"
   section below for why the guard is not optional either).
   - Add a small file-local helper (near the top of the file, or immediately above
     `handleClipTrigger`):
     ```
     namespace {
     Clip::BeatSnapMode quantizeModeToForcedSnap(Composition::QuantizeMode mode,
                                                  const FeatureSnapshot& snap)
     {
         if (mode == Composition::QuantizeMode::Off) return Clip::BeatSnapMode::Off;
         // No reliable beat yet: an honest immediate trigger beats a trigger that
         // may never drain (see TRAPS — BPMTracker::STATE_SEARCHING / phase_ pinned at 0).
         if (snap.trackerState != 2 /* BPMTracker::STATE_LOCKED */) return Clip::BeatSnapMode::Off;
         return (mode == Composition::QuantizeMode::NextBeat) ? Clip::BeatSnapMode::Beat
                                                                : Clip::BeatSnapMode::Bar;
     }
     }
     ```
   - In `handleClipTrigger` (`MainComponent.cpp:3255`), before the existing
     `layer->triggerClip(column);` at line 3277:
     ```
     const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();
     const auto forcedSnap = quantizeModeToForcedSnap(composition_.quantizeMode, snap);
     layer->triggerClip(column, forcedSnap);
     ```
     (Reading the FeatureBus here is an established pattern already used later in this SAME
     function, `MainComponent.cpp:3294`, for the per-clip beat-snap seek — same thread, same API,
     no new cross-thread exposure.)
   - In `handleColumnTrigger` (`MainComponent.cpp:3411`), before the existing
     `deck->triggerColumn(column);` at line 3434, compute `forcedSnap` the same way and call
     `deck->triggerColumn(column, forcedSnap);`.

No other file needs to change. `TopBar.cpp`'s existing write to `composition_.quantizeMode`
(`TopBar.cpp:160`) is already sufficient — nothing needs to observe the combobox changing in real
time, because both choke points read `composition_.quantizeMode` fresh, synchronously, at the
moment of each trigger.

## FENCE — every file this lane will WRITE
- `src/model/Layer.h`
- `src/model/Deck.h`
- `src/core/DeckCommands.h`
- `src/MainComponent.cpp`

No other file. In particular: `src/ui/TopBar.cpp/.h` (control already writes the field correctly —
leave it alone), `src/model/Autopilot.cpp`, `src/render/Renderer.cpp`, `src/core/TriggerCommands.h`,
`src/binding/*`, `src/midi/*`, `src/osc/*`, `src/api/*` — none of these need changes; they already
route correctly into the two choke points this lane modifies.

## CALL-SITE ENUMERATION — grep commands run + raw output

**`triggerClip` — two agreeing patterns, both re-run above with full output pasted in section 6:**
`grep -rn "\btriggerClip(" src/ tests/` and the looser `grep -rn "triggerClip" src/ tests/`. Both
agree: exactly 5 non-test call sites (`MainComponent.cpp:3277` [changing], `Deck.h:114` [inside the
function being changed], `Autopilot.cpp:288,310,372` [autopilot, NOT changing — see #6]), plus the
declaration (`Layer.h:190`) and 2 internal recursive calls to `triggerClipImmediate`
(`Layer.h:213,285`, unaffected). 21 test call sites total: `test_undo_commands.cpp` (8: lines
1883,1927,1964,1975,2009,2015,2045,2056), `test_autopilot.cpp` (4: lines 49,95,188,203),
`test_composition.cpp` (4: lines 78,88,114,117), `test_compositor.cpp` (5: lines 37,38,47,48,80)
— see the raw output above for the authoritative per-line list. Every single one is a 1-argument
call; none require any change because the new parameter is defaulted.

**`triggerColumn` (Deck method) — `grep -rn "\.triggerColumn(\|->triggerColumn(" src/ tests/`:**
```
src/MainComponent.cpp:3434:    deck->triggerColumn(column);
tests/test_undo_commands.cpp:2095:    deck.triggerColumn(4);
tests/test_composition.cpp:128:        deck.triggerColumn(0);
```
1 production call site (changing), 2 test call sites (1-argument, unaffected by the new default
param).

**`LayerRuntimeSnapshot` / `captureLayerRuntime` / `applyLayerRuntime` — `grep -rn
"LayerRuntimeSnapshot\|captureLayerRuntime\|applyLayerRuntime" src/ tests/`:** full output captured
during survey — 4 production call sites construct/compare `LayerRuntimeSnapshot` via
`captureLayerRuntime(...)` (`MainComponent.cpp:686,688,3269,3279,3421,3429,3446,4500,4502`, plus the
3 definition sites in `DeckCommands.h` and the 2 usage sites in `TriggerCommands.h`); ALL test
usages (`tests/test_undo_commands.cpp`, 20+ lines) construct via `LayerRuntimeSnapshot rt;`
(default-init) or `captureLayerRuntime(...)`, then set NAMED fields (e.g.
`rt2.activeClipColumn = 5;`) — **zero positional/aggregate-brace constructions
(`LayerRuntimeSnapshot{1,2,3,4}`) exist anywhere in tests or src**, confirmed by inspecting every
matched line. This means adding a 5th named field with a default member initializer breaks nothing
at any of these call sites.

**Binding::Action enumeration — `sed -n '24-44p' src/binding/Binding.h`:** 19 values; only
`TriggerClip` and `TriggerColumn` select a new column (full list quoted in section 4-5 above).

## TRAPS

1. **The real live-performance risk, VERIFIED, not hypothetical: a queued trigger can be
   permanently stuck if the BPM tracker has never locked.** `BPMTracker::updatePhase`
   (`src/analysis/BPMTracker.cpp:175-181`):
   ```
   if (lockedBPM_ <= 0.0f) { phase_ = 0.0f; return; }
   ```
   Before ANY tempo estimate exists (start of a set, or a passage aubio can't find a beat in),
   `lockedBPM_` is 0 and `phase_` is pinned at exactly `0.0f` forever. Autopilot's beat-crossing
   detector (`beatPhase < lastBeatPhase_ - 0.5f`) then compares `0.0f < 0.0f - 0.5f`, which is
   false forever — **the crossing edge never fires, `pendingTriggerColumn` never drains, the
   queued clip never plays.** This is exactly the failure mode the brief warned about ("late or
   dropped triggers... worse than an honest Off"), and it already exists today for the per-clip
   `beatSnapMode` feature — this lane's `quantizeMode` wiring would inherit it wholesale AND make
   it far more likely to bite (a global always-on control is used far more casually than a
   per-clip authoring choice). **THE CHANGE above closes this**: `quantizeModeToForcedSnap` checks
   `FeatureSnapshot::trackerState == 2` (`STATE_LOCKED`, `src/analysis/BPMTracker.h:33`, exposed on
   the snapshot at `src/analysis/FeatureSnapshot.h:39`) and falls back to an immediate trigger
   (forcedSnap = Off) when the tracker isn't locked — "honest Off" instead of a silent hang. This
   guard does NOT touch the pre-existing per-clip `beatSnapMode` risk (a clip with its own
   beatSnapMode set can still hang under the same condition) — that's pre-existing behavior, not
   introduced or fixed by this lane; flag to Boris if he wants it closed too (small follow-on,
   same fix shape, in `Layer::triggerClip`'s clip-only branch).
2. **Cross-thread field, no lock — but this is an INHERITED risk class, not a new one.**
   `pendingTriggerColumn`/`pendingTriggerSnapOverride` are written on the message thread (inside
   `handleClipTrigger`/`handleColumnTrigger`) and read+cleared on the GL thread (inside
   `Autopilot::processFrame` → `Layer::processPendingTrigger`). `src/core/TriggerCommands.h:34-36`
   already documents this exact class of race as accepted status quo for every runtime field this
   trigger path touches ("NO GL FENCE: a trigger only writes per-layer runtime fields... in place —
   status-quo field-level race, exactly like today's direct writes"). The new field joins that same
   accepted class; it does not create a new one.
3. **Undo/redo of a QUEUED (not-yet-fired) quantized trigger, without the `DeckCommands.h` field
   addition, has a real (narrow) staleness bug.** Trace: user quantize-triggers column A (queued,
   override=Bar) → command pushed. Before it fires, user quantize-triggers column A AGAIN with the
   global mode now set to NextBeat (override=Beat) — `pendingTriggerColumn` is unchanged (still A),
   only `pendingTriggerSnapOverride` changed. If `LayerRuntimeSnapshot`'s `operator==` does not
   include the new field, `rtBefore == rtAfter` reads as UNCHANGED (all 4 old fields identical) and
   `handleClipTrigger`'s "skip pushing a no-op command" branch
   (`MainComponent.cpp:3401`, `if (!(rtBefore == rtAfter) || playBefore != playAfter)`) silently
   drops the command — the re-quantize becomes un-undoable in isolation, even though the live model
   state DID change correctly. THE CHANGE's step 3 (add the field to the struct, `operator==`,
   capture, and apply) closes this. Do not skip step 3 to save a file touch — it is cheap and this
   is the kind of gap this codebase's own history (see `TriggerCommands.h`'s documented
   `hasBeenTriggered` imperfection) treats as worth closing when the fix is this small.
4. **Global-quantize vs. per-clip beat-snap precedence is a genuine, unresolved design choice —
   see OPEN QUESTIONS.** THE CHANGE picks "global wins outright when non-Off" (simplest to explain:
   the TopBar control is a master switch for the whole set). An alternative reading ("respect
   whichever is stricter/coarser") is equally defensible and not addressed by the essentials-plan
   text. Flag to Boris before building if he has an opinion; otherwise the recommended default
   ships.
5. **Clearing the active/pending clip does not cancel a pending quantized trigger — pre-existing,
   now more visible.** `Layer::clearActiveClip()` (`Layer.h:288-298`) never touches
   `pendingTriggerColumn`. If a trigger is queued and the user hits the layer's X-button clear (or
   Cmd+X) before it fires, the queued trigger will still fire on the next beat/bar crossing,
   silently reactivating a layer the user just cleared. This is pre-existing per-clip-beat-snap
   behavior (not introduced here), but a global always-on Quantize makes it dramatically more
   likely to be hit live. Not fixed by THE CHANGE above (kept minimal/fenced); flag to Boris as a
   likely-wanted follow-on (the fix shape: `clearActiveClip` should also reset
   `pendingTriggerColumn = -1; pendingTriggerSnapOverride = Off;` when it belongs to the same
   layer — small, but touches `clearActiveClip`'s call sites' undo-snapshot assumptions, so sizing
   it needs its own look, not assumed free here).
6. **`Clip::BeatSnapMode` needs to stay visible at every touched file** — already is, transitively
   (`Composition.h` → `Deck.h` → `Layer.h` → `Clip.h`; `DeckCommands.h` already includes `Deck.h`).
   No new `#include` lines needed anywhere in THE CHANGE — verified, not assumed.
7. **Do not thread `forcedSnap` into `Autopilot.cpp`'s 3 call sites.** They call the 1-argument
   overload by omission (default `Off`), which is correct per #6 in the trigger enumeration —
   double-check this stays true after editing `Layer::triggerClip`'s signature (it will, since the
   new parameter is defaulted and those 3 call sites are untouched).

## HOW TO PROVE IT WORKS
- **Automated (ctest, once built — not run by this recon):** existing `tests/test_composition.cpp`
  round-trip stays green unchanged. New unit coverage (builder's job, not surveyed here) should
  add: (a) `Layer::triggerClip(col, Beat)` on a non-active column queues
  (`pendingTriggerColumn == col`) rather than firing immediately; (b)
  `processPendingTrigger(0,0)` with a `Beat` override fires on any beat, a `Bar` override only
  fires when `beatInBar==0`, using a clip whose OWN `beatSnapMode` is `Off` (proves the override
  path is independent of the per-clip field); (c) `Deck::triggerColumn(col, Bar)` queues on every
  non-ignoring layer; (d) undo of a queued-but-not-fired trigger restores `pendingTriggerColumn`
  AND `pendingTriggerSnapOverride` (needs the `DeckCommands.h` field addition to pass).
- **Human-only, live behavior (cannot be asserted by ctest):**
  1. Play a track with a clean, quickly-locking beat. Set Quantize = Next Beat. Click a cell
     mid-bar — clip should visibly wait and start exactly on the next beat, not instantly.
  2. Same, Quantize = Next Downbeat — clip should wait for the next bar-1, audibly/visually a
     longer wait than #1.
  3. Set Quantize = Off — clicking should feel exactly as instant as it does today (regression
     check against the existing zero-consumer behavior).
  4. **The BPM-lock guard, hardest to fake in a unit test:** start the app fresh (or feed silence /
     no-beat material) with Quantize = Next Beat, click a cell immediately. Confirm it plays
     INSTANTLY (guard engaged, honest fallback) rather than hanging silently until a beat happens
     to lock. This is the single most important behavioral check for this lane — it is the exact
     failure mode named in the brief.
  5. Column-trigger (Shift-click or column header, whichever DeckView binds today) under Quantize
     = Next Downbeat: confirm ALL non-ignoring layers start together on the same downbeat, not
     staggered.
  6. Retrigger the ALREADY-active cell under Quantize != Off: should still be instant
     (retrigger-restart bypasses the queue today via `column != activeClipColumn`, confirm this is
     still true after the change).
  7. MIDI note bound to `Action::TriggerClip` under Quantize = Next Beat: confirm it quantizes
     identically to a mouse click (proves the choke point is really shared, not just true on paper).

## OUT OF SCOPE (deliberately)
- Fixing `quantizeSelector_`'s stale-after-load display (finding #-adjacent in CURRENT BEHAVIOUR) —
  pre-existing, systemic across TopBar selectors, not unique to Quantize, and orthogonal to making
  the control functionally correct (the underlying field already loads correctly).
- Cancelling a pending quantized trigger when its layer/cell is cleared (TRAP #5) — real, but a
  distinct fix with its own undo-snapshot implications; not folded into this lane's fence.
- Closing the BPM-not-locked hang for the PRE-EXISTING per-clip `beatSnapMode` feature when
  triggered with global Quantize left Off (TRAP #1's note) — same bug class, different control,
  not this lane's stated subject.
- `onQuantizeChanged` (`TopBar.h:28`) staying permanently unassigned — harmless (nothing needs to
  react to the combobox changing in real time; both choke points read the field fresh on each
  trigger) but could be deleted as dead plumbing in a cleanup pass; not required for correctness.
- OSC's missing `onTriggerColumn` (no column-trigger exposed over OSC at all, #3 above) —
  pre-existing gap, unrelated to quantize.
- Extending the guard/logic to Autopilot's or the MilkDrop-playlist's own independent
  beat-crossing detectors (#6, #9) — both are deliberately excluded per the trigger enumeration.

## OPEN QUESTIONS
1. **Precedence when BOTH a clip's own `beatSnapMode` and the global `quantizeMode` are active and
   disagree in granularity.** THE CHANGE's default: global wins outright (forces its granularity,
   ignoring the clip's own setting, whenever global != Off). Alternative: take the coarser/stricter
   of the two. The essentials-plan text does not address this interaction at all — it was written
   as if only the global control existed; the survey found the per-clip control was already there
   first. Needs a call from Boris before or during build (cheap to change either way — it's one
   `if` inside the field-population step in THE CHANGE).
2. **Should the BPM-lock guard (TRAP #1) also suppress a trigger already-queued-and-waiting if the
   tracker LOSES lock partway through the wait** (e.g., track cuts to silence right after a
   quantized click)? Today's `updatePhase` behavior when `lockedBPM_` later drops back to 0 mid-wait
   was not traced in this survey (I traced the SEARCHING-from-zero case, not a
   was-locked-then-unlocked case) — labeled INFERRED, not verified, that the same `phase_ == 0.0f`
   freeze would recur. If Boris cares about this edge specifically, it needs its own trace pass
   before sizing.
3. **Is a visible "N pending" indicator wanted in the deck UI** so a performer can see a quantized
   trigger is waiting (vs. today's per-clip beat-snap, which has no such indicator either, and
   apparently ships without complaint)? Not asked for in the essentials-plan text; flagging because
   a global always-on quantize control makes "did my click register?" a much more likely live
   question than the niche per-clip feature ever posed. Purely a UI addition if wanted — does not
   change THE CHANGE's model-layer design.
