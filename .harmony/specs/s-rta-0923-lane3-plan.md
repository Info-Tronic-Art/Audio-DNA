# s-rta-0923 — LANE 3: the connection binding ("21 controls that do nothing") — PLAN

Architect (Fable), secondary lane, RealTimeAudio. Read-only pass at HEAD `9257537` (2026-09-23;
HEAD moved from `d434e0e` to `9257537` while I read — the H1FIX AudioTap commit; none of the files
this plan touches changed in it). Every `MainComponent.cpp` / inspector / renderer citation below is
an ANCHOR (function or lambda name, verbatim text) plus the line I saw it at today; re-grep the
anchor, never trust the line (binding-decisions.md "re-grep by anchor text").

Confidence tags: VERIFIED = read in source this session. INFERRED = follows from verified facts.
ASSUMED = stated so a builder can check it.

Scope mapping (so nobody builds the wrong lane): the session's "Lane 3" (HANDOFF s168 "NEXT
SESSION" item 4) is **s166 spec L4 (scalar targets: the 21 inspector controls) + L3(b) (the engine
tick in `tickFeaturePipeline`) + L3(e) (the ≥9 grip sites) + s167 D6a `manualWrite` (R8: this lane
owns it)**. It is NOT s166 L3(a)/(c)/(d) (effect-param `effParam` repoint, `tickModulation`
retirement, `SourceMode` deletion, TestServer route endpoints, `RoutingEngine` deletion), NOT L5
(ClipClock / timed sources), NOT the macro-knob binding (`MacroPanel` → `Macro::conn`), NOT L7.
Those are named in §12 as follow-ups with the constraint this lane leaves for each.

---

## QUESTION

Plan the lane that makes the 21 `UniversalParamControl` scalar sliders (Composition 8, Layer 7,
Clip 6) actually connect to audio signals / tempo-locked oscillators end to end — model, engine
tick, renderer read, widget — define `manualWrite` (the manual-write funnel the recorder will hook,
R8), split the work into builder lanes with disjoint file ownership and fail-first tests, give a
production-mode live-verification recipe with no output window, and list the rulings that bind it.

## APPROACH (stated first)

**Wire the four missing pieces in four disjoint lanes off one scaffold, then prove it live on
port 7070 with a square-wave LFO strobing Layer 1's opacity — no test server, no output window.**

1. The model, engine, serialization and `eff()` twins already exist and are unit-tested (`src/connect/`,
   `Clip::scalarConns/scalarLive/eff`, `Layer::…`, `Composition::…`) — VERIFIED, nothing to design there.
2. **C1 — funnel core** (`src/connect/ManualWrite.*`, headless): `resolveControl(ControlPath)` +
   `manualWriteCore/manualReleaseCore` implementing the s167 D8 ownership chain as a rank on
   `ParamConnection::Grip`. This is `manualWrite`'s definition; `MainComponent` gets a 15-line wrapper
   that maps `Origin` → rank and exposes the recorder's hook seam (`onManualWrite`/`onManualRelease`).
3. **C2 — renderer repoint** (GL thread): every raw read of the 22 backing fields in
   `CompositorEngine.cpp` / `Renderer.cpp` becomes `x.eff(Scalar)`. Gate = a grep that returns only
   comment lines.
4. **C3 — MainComponent + ApiServer + LayerStrip**: `ConnectionEngine::tick` in `tickFeaturePipeline`
   after `globalMacroBank_.updateValues`; the 9 external writer sites (OSC ×3, MIDI ×5, REST ×2) route
   through `manualWrite`; `/api/composition` gains a `live` block (the production oracle); the deck
   opacity strip grips on drag.
5. **C4 — widget + inspectors**: `UniversalParamControl::bindConnection(ParamConnection*, LiveValue*)`;
   picker → `ConnSource` translation moved into a headless `src/connect/ConnPicker.*` (tested);
   drag start/end → `gripHeld()/release()`; the 21 controls bound in `setClip/setLayer/setComposition`;
   the thumb follows `eff()` when connected. Composition's dead second "Opacity" knob binds to Master
   (ruling 11) and `compOpacity` is deleted.
6. **C5 — live gate** (Harmony runs it, builders never do): `.harmony/probe-lane3.sh`, production
   launch, `set_bpm 120` (manual tempo — beat phase runs without audio), `load_composition` of a fixture
   carrying `conns`, `trigger_clip`, then two oracles: (A) `/api/composition` `live.opacity` toggles
   0↔1 twenty times a second; (B) `render_frame` md5 differs between a live=1 and a live=0 moment.
   Same script run against the PRE-change binary must FAIL at oracle A (field absent).

The strongest counterargument to this approach and why it loses is in §11.

---

## 1. Scout recon re-verified against HEAD `9257537` (deltas only)

The recon (`.harmony/scout-lane3-connect-surface.md`, written 2026-09-06 at `6418c2f`) is
**still correct in every load-bearing claim**. Re-run today, same three negative greps:

- `grep -rn "\.eff(\|->eff(" src/` outside `src/connect/` → **0 hits** (VERIFIED). No renderer reads a twin.
- `ConnectionEngine` referenced outside `src/connect/` only in comments in `src/model/Clip.h`,
  `src/model/Layer.h`, `src/routing/MacroBank.h`, and `tests/` → **`tick()` has no production caller** (VERIFIED).
- `grep -rn "onSourceChanged\s*=" src/` → **0 hits**; `manualWrite|writeManual|touchScalar` in `src/`,
  `tests/` → **0 hits** (VERIFIED). The funnel does not exist.
- The 21 widgets: `src/ui/CompositionInspector.h:98-109` (8), `src/ui/LayerInspector.h:93-126` (7),
  `src/ui/ClipInspector.h:141-157` (6) — VERIFIED, count unchanged.

What moved since the recon (none of it changes the plan, all VERIFIED):
- `ConnShape::resetPhaseOnStructural` (s168, `src/connect/ParamConnection.h`) and `ConnectionShaper::
  beatsNow(..., totalBarCount, resetPhaseOnStructural)` — the engine already folds on the monotonic
  counter. The picker does not expose the switch; default false (ruling 25 satisfied at the model).
- `ConnSource::Envelope::points` is now `AutomationCurve curve` (`src/connect/AutomationCurve.h`, D7).
- `src/model/ControlPath.h` exists (s168 step 1) — the lane key `manualWrite` takes. Its `control`
  vocabulary: `param | dryWet | scalar | macro | speed` continuous (D2, `isContinuous()`).
- `src/recording/Player.h` `Sink { fire / touch(path, grip) / set(path, v) / release(path) }` with `bool`
  = "accepted (D8 chain)" — this is the shape step 3's `Sink` impl will map onto `manualWrite`.
- `src/recording/Lane.h:50` `enum class Origin { Human, Replay, Routine, Engine, Preamble }` —
  `manualWrite`'s `Origin` parameter is THIS type (the wrapper lives in `MainComponent`, which will
  include `recording/` for step 3 anyway; `src/connect/` must not depend on `src/recording/`, §3).
- S166-L0 landed: `applyClipEffects` takes the vector (`CompositorEngine.cpp:604` `applyClipEffects(
  clip.effects, …)`) and there is no per-frame whole-`Clip`/`Layer` copy on the GL thread
  (`grep "Clip [a-zA-Z_]* = \|= \*clip;\|Clip layerFxClip" src/render/` → 0). Safe to keep
  `std::string`-bearing `ParamConnection`s inside model structs the GL thread iterates by reference.
- The MIDI thread question the scout left INFERRED is now VERIFIED: `src/midi/MidiHandler.cpp:80,88,97`
  marshal note-on/off/CC via `juce::MessageManager::callAsync` before `BindingManager::processMidi*`
  → `actionCallback_` → `MainComponent::handleBindingAction` — every binding write is message-thread.
  OSC: `MessageLoopCallback` (recon). REST: `callAsync` at `ApiServer.cpp:411,452,495` (VERIFIED).

One thing the recon said that is now WRONG: "`Composition::masterOpacity` … `compOpacity` … no
renderer path reads" — `masterOpacity` (`Renderer.cpp:706`, S167-L4b pass) and `masterSpeed`
(`Renderer.cpp:415,1241`) and `clipOpacity` (`CompositorEngine.cpp:492,604,1239`) ARE render-consumed
today (VERIFIED; `.harmony/probe-deck-path.sh` measured master 0.5 → 0.5× luminance). Only
`compOpacity` remains render-dead, and ruling 11 merged it into master. So of the 22 scalars, all 22
backing fields render except `compOpacity`, which is not a scalar at all. The "nothing would move
on screen" warning in recon §4 is therefore about the `eff()` repoint only, not about dead fields.

Working-tree hazard (VERIFIED at boot): `git status` shows `UU tests/CMakeLists.txt` (resolved,
unstaged) and staged `A tests/test_bt_device_shapes.cpp`, `M src/recording/AudioTap.cpp`, plus
unstaged `src/ui/RecordPanel.{h,cpp}` (L5 in flight). **C0 must not touch `tests/CMakeLists.txt`
until that index state is committed.** Nothing in flight touches any file this plan names.

---

## 2. THE 21 INERT CONTROLS — exact list

Every row: the widget, where its `onValueChanged` writes today (anchor), the backing field, the
`ScalarEnum` + `ScalarDef::key` the model already provisions, and whether the GL thread reads the
field today. "Inert" means: the cyan triangle paints, nothing downstream reads the choice.

| # | Widget | Writes today (anchor) | Backing field | Enum / key | GL reads field today |
|---|---|---|---|---|---|
| 1 | Comp `masterControl_` "Master" | `CompositionInspector.cpp:137` `composition_->masterOpacity = val` | `Composition::masterOpacity` | `CompScalar::Opacity` / `opacity` | yes — `Renderer.cpp:706` |
| 2 | Comp `speedControl_` "Speed" | `:145` `masterSpeed = val * 4.0f` | `masterSpeed` | `CompScalar::Speed` / `speed` | yes — `Renderer.cpp:415,1241` |
| 3 | Comp `opacityControl_` "Opacity" | `:154` `composition_->compOpacity = val` | `compOpacity` (**render-dead; merged into master by ruling 11; NOT a CompScalar**) | → rebind to `CompScalar::Opacity` (§4.6) | no |
| 4 | Comp `posXControl_` | `:173` `compPositionX = (v-0.5)*3840` | `compPositionX` | `CompScalar::PosX` / `positionX` | yes — `applyCompTransform` `Renderer.cpp:2021` |
| 5 | Comp `posYControl_` | `:174` `compPositionY = (v-0.5)*2160` | `compPositionY` | `CompScalar::PosY` / `positionY` | yes `:2022` |
| 6 | Comp `scaleControl_` | `:175` `compScale = v * 2.0f` (linear, pre-existing) | `compScale` | `CompScalar::Scale` / `scale` | yes `:2023` |
| 7 | Comp `rotationControl_` | `:176` `compRotation = (v-0.5)*720` | `compRotation` | `CompScalar::Rotation` / `rotation` | yes `:2024` |
| 8 | Comp `anchorControl_` | `:177-178` `compAnchorX = (v-0.5)*3840; compAnchorY = 0.0f` (forces Y) | `compAnchorX` (+ forced `compAnchorY`) | `CompScalar::AnchorX` / `anchorX` (AnchorY connectable via JSON only, D13) | yes `:2025-2026` |
| 9 | Layer `masterControl_` "Master" | `LayerInspector.cpp:148` `layer_->opacity = val` | `Layer::opacity` | `LayerScalar::Opacity` / `opacity` | yes — `CompositorEngine.cpp:604,868,887,1064` |
| 10 | Layer `opacityControl_` "Opacity" | `:185` `layer_->opacity = val` (same field as #9) | `Layer::opacity` | `LayerScalar::Opacity` (same conn as #9) | yes |
| 11 | Layer `posXControl_` | `:399` `positionX = (v-0.5)*3840` | `Layer::positionX` | `LayerScalar::PosX` | yes — `applyLayerTransform` `:550,574` |
| 12 | Layer `posYControl_` | `:400` | `Layer::positionY` | `LayerScalar::PosY` | yes `:551,574` |
| 13 | Layer `scaleControl_` | `:401` `layerScale = 2^((v-0.5)*2)` | `Layer::layerScale` | `LayerScalar::Scale` | yes `:552,578` |
| 14 | Layer `rotationControl_` | `:402` `layerRotation = (v-0.5)*720` | `Layer::layerRotation` | `LayerScalar::Rotation` | yes `:553,580` |
| 15 | Layer `anchorControl_` | `:403` `layerAnchorX = (v-0.5)*3840` | `Layer::layerAnchorX` | `LayerScalar::AnchorX` | yes `:576` |
| 16 | Clip `clipOpacityControl_` "Opacity" | `ClipInspector.cpp:336` `clip_->clipOpacity = val` | `Clip::clipOpacity` | `ClipScalar::Opacity` | yes — `CompositorEngine.cpp:492,604,1239` |
| 17 | Clip `posXControl_` | `:404` | `Clip::positionX` | `ClipScalar::PosX` | yes — `applyClipTransform` `:442,468` |
| 18 | Clip `posYControl_` | `:405` | `Clip::positionY` | `ClipScalar::PosY` | yes `:443,469` |
| 19 | Clip `scaleControl_` | `:406` `scale = 2^((v-0.5)*2)` | `Clip::scale` | `ClipScalar::Scale` | yes `:444,473` |
| 20 | Clip `rotationControl_` | `:407` | `Clip::rotation` | `ClipScalar::Rotation` | yes `:445,475` |
| 21 | Clip `anchorControl_` | `:408` `anchorX = (v-0.5)*3840` | `Clip::anchorX` | `ClipScalar::AnchorX` | yes `:471` |

Model surface is 22 scalars (`ClipScalar::Count`=7, `LayerScalar::Count`=7, `CompScalar::Count`=8,
`src/connect/ScalarParams.h`); `Clip::anchorY`, `Layer::layerAnchorY`, `Composition::compAnchorY` have
no widget (D13: rewrite) but ARE rendered and ARE serializable as connections, so C2 repoints them too.
Formulas in `ScalarParams.h` were re-verified by the L2 builder against the inspector math above
(its header comment) — VERIFIED consistent with the anchors in this table.

Widget-side facts that shape the binding (VERIFIED in `src/ui/UniversalParamControl.{h,cpp}`):
- `valueSlider_.onValueChange` → `onValueChanged(currentValue_)` (`:15-19`); `+`/`-` buttons (`:28-42`)
  and right-click (`:246-255`) also fire `onValueChanged` (right-click fires it TWICE: once via
  `ResettableSlider::mouseDown` → `sendNotificationSync`, once directly — pre-existing, harmless).
- `setParamValue()` uses `dontSendNotification` (`:283-288`) — the inspectors' 10 Hz `syncFromX()`
  pushes never write the model. Safe to push `eff()` there.
- `handleSourcePickerResult` (`:460-548`) sets `sourceMode_/sourceName_` and fires the unassigned
  `onSourceChanged`. Menu ids: 1 Manual, 2 Clip Position, 3 Timeline, 100+i registry Audio, 200+i
  Macro (**handler accepts `< 206` while the menu offers 8 → Macro 7/8 unreachable, pre-existing
  bug**, fixed in C4), 300+s*6+d BPM Sync (shapes Sine/Saw/Triangle/Square × 1/4,1/2,1,2,4,8 beats),
  400+i Oscillator, 500+i Envelope.
- `juce::Slider::onDragStart/onDragEnd` exist (`build/_deps/juce-src/modules/juce_gui_basics/widgets/
  juce_Slider.h:620,623`, VERIFIED) — the Held grip's begin/end.

---

## 3. `manualWrite` — the definition (R8: this lane owns it; the recorder hooks it, never defines it)

### 3.1 Why it is two layers
s167 D6a: `manualWrite(const ControlPath&, float value, GripKind, Origin) → bool` "PROPOSED,
`MainComponent`". `MainComponent` is not headless-testable and `Origin` lives in `src/recording/Lane.h`,
which `src/connect/` must not include (recording already depends on connect via `AutomationCurve`).
So: a headless CORE in `src/connect/` that knows ranks, not origins; a thin WRAPPER in `MainComponent`
that maps `Origin`+`GripKind` → rank and owns the recorder's hook seam.

### 3.2 Core — `src/connect/ManualWrite.h` (NEW, Lane C0 declares + stubs, Lane C1 implements)

```cpp
#pragma once
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ScalarParams.h"
#include "model/ControlPath.h"
#include <optional>
struct Composition; class MacroBank;

// The D8 ownership chain as a rank (s167 spec D8; binding-decisions ruling 18):
//   human Held > human Decaying (MIDI/OSC/HTTP) > playing lane gesture (replay/routine) > connection > rest
enum class Hand : uint8_t { None = 0, Lane = 1, HumanDecaying = 2, HumanHeld = 3 };

// Everything a manual writer needs about ONE continuous control, resolved from a ControlPath.
struct ControlRef {
    ParamConnection* conn;          // the grip lives here even when kind == None (arrays are dense)
    float*           manual;        // the raw model field (MODEL units for scalars, [0,1] otherwise)
    LiveValue*       live;          // the twin (only needed by disconnect; may be null for macros)
    float (*toModel)(float norm);   // ScalarDef::toModel for scalars; identity for params/dryWet/macro
    float (*toNorm)(float model);
};

// ControlPath -> live model. POSITIONAL fields only (names are compile-time resolution, D2 —
// Program::compile's job, not this function's). nullopt on any out-of-range/unknown control.
//   scope Comp,  control "scalar", scalar=<key>              -> Composition::scalarConns[CompScalar(key)]
//   scope Layer, deck,layer,       control "scalar"          -> Layer::scalarConns[...]
//   scope Clip,  deck,layer,col,   control "scalar"          -> Clip::scalarConns[...]
//   scope Clip|Layer|Comp, fx>=0,  control "param", param.i  -> EffectSlot::paramConns[i] / paramValues[i]
//   scope Clip|Layer|Comp, fx>=0,  control "dryWet"          -> EffectSlot::dryWetConn / dryWet
//   scope Clip,  fx<0,             control "param", param.i  -> Clip::sourceParams[i].conn / .value   (vocabulary
//                                                               EXTENSION, see 3.5)
//   scope Macro, macroScope 0,     control "macro", macro=i  -> globalMacros.getMacro(i).conn / .manualValue
//   control "speed" (Clip::speed) -> nullopt in this lane (Clip::speed has no ParamConnection; step-3 note)
std::optional<ControlRef> resolveControl(Composition& comp, MacroBank& globalMacros, const ControlPath& path);

// Is a grip currently in force? Held: always. Decaying: only until gripHoldMs after lastTouch.
// (The engine only expires Decaying grips for CONNECTED params inside evaluate(); an unconnected
// param's grip must expire by timestamp here or a lane would be refused forever after one MIDI turn.)
bool gripActive(const ParamConnection& c, double now, float gripHoldMs);

// Open/refresh a grip at rank `hand`. Returns false (nothing changed) if a HIGHER rank holds the control.
// kind: Held = the writer has a release event (slider, lane gesture); Decaying = it does not.
bool manualTouchCore(const ControlRef& r, Hand hand, ParamConnection::Grip::Kind kind, double now, float gripHoldMs);

// Touch (as above) + write. valueNorm is the writer's [0,1] value; scalars are stored through toModel.
// Returns false and writes NOTHING if refused. Equal rank: latest writer wins (refreshes the grip).
bool manualWriteCore(const ControlRef& r, float valueNorm, Hand hand, ParamConnection::Grip::Kind kind,
                     double now, float gripHoldMs);

// Close the grip if the holder's rank <= hand (a lane cannot release a human; a human releases anything).
void manualReleaseCore(const ControlRef& r, Hand hand);

// Disconnect helper the widget/picker uses (s166 §4.3 "the model mutator stores NAN into the twin and
// releases the grip"): source.kind = None, state reset, live->v = NAN, grip cleared.
void disconnect(ParamConnection& c, LiveValue* live);
```

`ParamConnection::Grip` gains ONE field (Lane C1, `src/connect/ParamConnection.h`):
`uint8_t rank = 0;` (a `Hand` value; 0 = None). `gripHeld()`/`gripTouch()` keep their signatures and set
`rank` to 3 / 2 respectively (they are the human path — VERIFIED only `ConnectionEngine.cpp`, tests and
this plan's new code call them). `release()` zeroes `rank`. The copy ctor already resets `grip` (VERIFIED
`ParamConnection.h` copy-assign sets `grip = Grip{}`) so undo/preset-load clearing still holds.

Clock: `src/connect/ConnClock.h` (NEW, C0): `inline double connNow() { return juce::Time::
getMillisecondCounterHiRes() * 0.001; }` — THE one engine clock. `ConnectionEngine::Context::now`
(C3) and every grip timestamp (C3, C4) use it; tests pass explicit `now` values as they do today.

### 3.3 Semantics (the D8 table, made executable)

| writer | Hand | Grip::Kind | while a HumanHeld grip is in force | while a Lane grip is in force |
|---|---|---|---|---|
| inspector / LayerStrip slider drag | HumanHeld | Held | accepted (latest hand wins, same rank) | accepted; lane grip replaced (rank 3) |
| `+`/`-`, right-click reset | HumanDecaying | Decaying | **refused** (`false`) | accepted; lane grip replaced (rank 2) |
| MIDI CC / note velocity, OSC, REST `set_*` | HumanDecaying | Decaying | **refused** | accepted |
| Player `touch/set` (`Origin::Replay/Routine/Preamble`) | Lane | Held (has `release`) | **refused** → Player marks the lane `displaced` for this gesture (its own D8 logic, `Player.h`) | accepted (later `begin` wins, D9) |
| `Origin::Engine` (autopilot random picks — LATER, s167 D6) | Lane | Decaying | refused | accepted |

Engine interplay (unchanged in `ConnectionEngine::evaluate`, VERIFIED): any `grip.kind != None`
suspends publishing (twin holds NaN → renderer reads the manual field the writer is filling); a
Decaying grip expires `gripHoldMs` after `lastTouch`; release starts the 120 ms hand-back glide
from the hand's last value (D14). A Lane grip therefore ALSO suspends the connection while a
replayed gesture runs — exactly D8's "playing lane gesture > permanent owner (connection)".

### 3.4 Wrapper — `MainComponent` (Lane C3)

```cpp
// MainComponent.h — the funnel every non-widget manual writer goes through (s167 D6a; R8: defined
// HERE by the connection lane; the recorder HOOKS onManualWrite/onManualRelease in step 3, never
// redefines them). Widgets grip the bound ParamConnection directly (§4.2) and do not pass here —
// s167 D14 "inspector-widget capture: deliberately NOT built now".
using GripKind = ParamConnection::Grip::Kind;
bool manualWrite(const ControlPath& path, float valueNorm, GripKind kind, Origin origin);
void manualRelease(const ControlPath& path, Origin origin);
// STEP-3 HOOK SEAM (recorder): called AFTER the core decided. `accepted == false` means refused.
std::function<void(const ControlPath&, float valueNorm, GripKind, Origin, bool accepted)> onManualWrite;
std::function<void(const ControlPath&, Origin)> onManualRelease;

// MainComponent.cpp
static Hand handFor(Origin o, GripKind k) {
    if (o == Origin::Human) return k == GripKind::Held ? Hand::HumanHeld : Hand::HumanDecaying;
    return Hand::Lane;   // Replay, Routine, Preamble, Engine
}
bool MainComponent::manualWrite(const ControlPath& p, float v, GripKind k, Origin o) {
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    auto ref = resolveControl(composition_, globalMacroBank_, p);
    bool ok = ref && manualWriteCore(*ref, v, handFor(o, k), k, connNow(), composition_.gripHoldMs);
    if (onManualWrite) onManualWrite(p, v, k, o, ok);
    return ok;
}
void MainComponent::manualRelease(const ControlPath& p, Origin o) {
    if (auto ref = resolveControl(composition_, globalMacroBank_, p)) manualReleaseCore(*ref, handFor(o, GripKind::Held));
    if (onManualRelease) onManualRelease(p, o);
}
```
Plus three ControlPath builders in `MainComponent.cpp` (anonymous namespace):
`compScalarPath(key)`, `layerScalarPath(deckIdx, layerIdx, key)`, `clipScalarPath(deckIdx, layerIdx,
col, key)`, `macroPath(i)`, `clipParamPath(deckIdx, layerIdx, col, fxIdx, paramIdx)` — filling the
positional fields AND the names that are there (D2: deckName/layerName/clipName/fxName/paramKey)
so a take recorded through this seam is re-bindable. The names come from the live model at call time.

### 3.5 Vocabulary extension (flag to the step-3 author, not to Boris)
D2 addresses effect params as `control:"param"` with an `fx` block. Source params (`Clip::sourceParams`,
a `UniversalParamControl` each — the second existing working path) have no D2 spelling. This plan
defines **`control:"param"` with `fx.i == -1` on a Clip scope = `sourceParams[param.i]`** (additive,
D12 "ADD never REDEFINE"; the key round-trips already since `fx` is omitted when `fx < 0`,
`ControlPath.h:99`). Step 3's `Program::compile` may need to learn it when a source-param lane is
captured (LATER, L-P). `control:"speed"` (`Clip::speed`) has no `ParamConnection` → `resolveControl`
returns nullopt and `manualWrite` returns false with no write; step 3 keeps writing `clip->speed`
directly for now (it is not connectable in this lane). Named so it cannot surprise anyone.

### 3.6 The 9 external writer sites → `manualWrite` (Lane C3; all VERIFIED at these anchors today)

| # | Site (anchor) | Today | Becomes |
|---|---|---|---|
| 1 | `oscHandler_.onSetMaster` (`MainComponent.cpp:1835`) | `composition_.masterOpacity = level;` | `manualWrite(compScalarPath("opacity"), level, Decaying, Origin::Human)` |
| 2 | `oscHandler_.onSetLayerOpacity` (`:1838`) | `layer->opacity = opacity;` | `manualWrite(layerScalarPath(activeDeckIndex, layerIdx, "opacity"), opacity, Decaying, Human)` |
| 3 | `oscHandler_.onSetMacro` (`:1866`) | `getMacro(i).manualValue = value;` | `manualWrite(macroPath(i), value, Decaying, Human)` |
| 4 | MIDI `case Binding::Action::MasterOpacity` (`:5875`) | `composition_.masterOpacity = value;` | as #1 |
| 5 | MIDI `case AdjustLayerOpacity` (`:5879`) | `layer->opacity = value;` | as #2 with `resolvedLayer` |
| 6 | MIDI `case AdjustMacro` (`:5933`) | `manualValue = value;` | as #3 with `binding.targetMacroIndex` |
| 7 | MIDI `TriggerClip` velocity→opacity (`:5715`) | `clip->clipOpacity = value;` | `manualWrite(clipScalarPath(active, resolvedLayer, resolvedColumn, "opacity"), value, Decaying, Human)` |
| 8 | MIDI `TriggerColumn` velocity→opacity (`:5751`, loop over layers) | same | same per layer index |
| 9 | REST `set_layer_opacity` (`ApiServer.cpp:495-503`) | inline `lay->opacity = opacity;` inside `callAsync` | `ApiServer` gains `std::function<void(int layer, float)> onSetLayerOpacity;` the lambda body becomes `if (onSetLayerOpacity) onSetLayerOpacity(layer, opacity);` (inline write REMOVED); `MainComponent` assigns it → as #2. Mirrors `onSetBpm` (`ApiServer.h:66`, `MainComponent.cpp:1808`). |
| 10 | REST `set_param` clip branch (`ApiServer.cpp:411-441`) | inline `fx.paramValues[pi] = value;` | keep the name→index resolution in the lambda (it needs `renderer_.getEffectLibrary()`), then `onSetClipEffectParam(layer, column, fxIndex, pi, value)`; inline write REMOVED; `MainComponent` → `manualWrite(clipParamPath(...), value, Decaying, Human)`. |
| — | REST `set_param` global branch (`:452`) and OSC `onSetEffectParam` (`:1871`) | write the v1 `effectChain_` (`Effect::getParam(pi).value`) | UNCHANGED — that chain has no `ParamConnection` (s166 §3.4 master-chain family, L7). |
| 11 | `LayerStrip.cpp:421-423` opacity slider (`onValueChange` → `layer_->opacity = …`) | widget, has release | keep the write; add `opacitySlider_.onDragStart = [this]{ if (layer_) layer_->scalarConns[size_t(LayerScalar::Opacity)].gripHeld(); }` and `onDragEnd → .release(0.0)`. Direct model grip like the inspectors (§4.2), no ControlPath. |

"At least 9" (s166 §2.3) → 11 sites named. Builder re-greps `clipOpacity = |masterOpacity = |->opacity =
|\.opacity = |manualValue = ` in `src/` before claiming completeness; the ONLY remaining raw writers must
be: inspector `onValueChanged` lambdas (§4.2 grips them), `LayerStrip` (#11), `fromVar`/`initDefault`/
`replaceContent` (structural), `TestServer.cpp` oracle setters (test-only, OFF in production builds).

---

## 4. THE BINDING EACH CONTROL NEEDS (four pieces, one pattern, applied 21 times)

### 4.1 Piece 1 — engine tick (Lane C3, `MainComponent::tickFeaturePipeline`, anchor `void MainComponent::tickFeaturePipeline`, `:3055` today)
Insert after `globalMacroBank_.updateValues(signalRegistry_);` and BEFORE `inspectorPanel_->tickModulation();`:
```cpp
    // S-RTA-0923 LANE 3: the ONE evaluator for every ParamConnection (s166 §4.2/4.3). Macros were
    // updated just above (they are sources); the recorder's Player::advanceTo (step 3) MUST be
    // inserted BETWEEN updateValues and this call (ConnectionEngine.cpp's own ORDERING FACT).
    const double now = connNow();
    const float dt = (lastConnTick_ > 0.0) ? std::clamp(static_cast<float>(now - lastConnTick_), 0.0f, 0.05f)
                                           : 1.0f / static_cast<float>(kMappingTickHz);
    lastConnTick_ = now;
    ConnectionEngine::Context ctx{ signalRegistry_, globalMacroBank_, snap, dt, now,
                                   composition_.gripHoldMs, composition_.handBackGlideMs };
    connectionEngine_.tick(composition_, ctx);
```
Members (`MainComponent.h`, next to `globalMacroBank_`): `ConnectionEngine connectionEngine_; double
lastConnTick_ = 0.0;` + `#include "connect/ConnectionEngine.h"`, `"connect/ConnClock.h"`,
`"connect/ManualWrite.h"`, `"recording/Lane.h"` (for `Origin`). The old `tickModulation()` path stays
(effect/source params; retired by the effect-param lane, §12).
`onSourceParamsPublished` is NOT wired here (no UI can connect a source param yet; the hook belongs to
the effect/source lane). The walk is O(all params) with `isConnected()` checks — the s166 budget.

### 4.2 Piece 2 — widget ↔ model (Lane C4)
`UniversalParamControl` (additive; `SourceMode`/`sourceName_` stay for the unbound effect rows):
```cpp
// Bind this widget to the model's connection for the parameter it edits. Bound widgets: the picker
// writes *conn_ (via ConnPicker), paint reads conn_->isConnected()/describeSource(), the slider grips.
void bindConnection(ParamConnection* conn, LiveValue* live);   // nullptr,nullptr = unbind
bool isConnected() const { return conn_ ? conn_->isConnected() : sourceMode_ != SourceMode::Manual; }
```
- `valueSlider_.onDragStart = [this]{ if (conn_) conn_->gripHeld(); }`; `onDragEnd = [this]{ if (conn_) conn_->release(connNow()); }`
- `+`/`-`/right-click handlers: after the existing `onValueChanged(...)`, `if (conn_) conn_->gripTouch(connNow());`
- `handleSourcePickerResult`: if `conn_`, build a `PickerChoice` (§4.3) from the menu id, `*conn_ =`
  `ConnPicker::apply(choice)` (a fresh `ParamConnection` with the chosen source; RANGE/INVERT from the
  widget's `outputMin_/outputMax_/inverted_` if already set); on "Manual" call `disconnect(*conn_, live_)`.
  `sourceName_ = describeSource(conn_->source)` and `sourceBtn_` text follow, so the existing paint code
  stays as is. Existing `onSourceChanged` still fires (nobody listens; left for the effect-row lane).
- `onRangeChanged`/`onInvertChanged` (rangeMinSlider_/rangeMaxSlider_/invertToggle_, VERIFIED they exist
  and fire today): when bound, write `conn_->shape.outMin/outMax/inverted` directly. Ruling B.3 controls
  become real for the 21 with no inspector code.
- Macro menu fix: `result < 200 + MacroBank::kNumMacros`.
Inspectors (one helper per inspector, called from `setComposition` / `setLayer` / `setClip` right after
the existing `syncFromX()`, and with `nullptr` in the null branch):
```cpp
// CompositionInspector.cpp
void CompositionInspector::bindScalarControls() {
    auto bind = [this](UniversalParamControl& c, CompScalar s) {
        if (composition_) c.bindConnection(&composition_->scalarConns[size_t(s)], &composition_->scalarLive[size_t(s)]);
        else c.bindConnection(nullptr, nullptr); };
    bind(masterControl_, CompScalar::Opacity); bind(opacityControl_, CompScalar::Opacity);   // ruling 11: one master
    bind(speedControl_, CompScalar::Speed);   bind(posXControl_, CompScalar::PosX); bind(posYControl_, CompScalar::PosY);
    bind(scaleControl_, CompScalar::Scale);   bind(rotationControl_, CompScalar::Rotation); bind(anchorControl_, CompScalar::AnchorX);
}
```
Layer: `masterControl_` AND `opacityControl_` → `LayerScalar::Opacity` (they already share the field,
`LayerInspector.cpp:149,186`); the other five → their enums. Clip: six → their enums.
`opacityControl_.onValueChanged` in `CompositionInspector.cpp:154` changes from `compOpacity = val` to
`masterOpacity = val`; `syncFromComposition()` `:511` shows `masterOpacity` in both knobs.
**Thumb follows the signal** (my call; §13 Q2): in each `syncFromX()`, for a bound control,
`shown = conn.isConnected() ? def.toNorm(owner.eff(s)) : <today's formula>`; `setParamValue(shown)` is
`dontSendNotification` so nothing writes back. During a Held grip `eff()` == manual (engine publishes
NaN) so the thumb never fights the hand; after release it visibly glides back (D14). Also push
`setSourceValue(def.toNorm(owner.eff(s)))` so the expanded mini-meter shows the live value.

### 4.3 Piece 3 — picker → model translation, headless (Lane C4, `src/connect/ConnPicker.{h,cpp}`)
```cpp
struct PickerChoice { enum class Kind : uint8_t { Manual, Signal, BpmSync, ClipPosition, Timeline, Macro } kind;
                      std::string signalName; int shapeIdx = 0; int divIdx = 2; int macroIdx = -1; };
ConnSource sourceFromPicker(const PickerChoice&);   // Signal(name) | Lfo{Sine/SawUp/Triangle/Square, {0.25,0.5,1,2,4,8}[div]}
                                                    // | ClipPosition | Envelope{curve {0,0},{1,1} Linear, Beats, 4} | Macro(i)
std::string describeSource(const ConnSource&);      // "Bass" | "Square 1 Beat" | "Clip Position" | "Timeline" | "Macro 3" | ""
```
Audio/Oscillator/Envelope submenus all become `Kind::Signal(name)` (registry-backed; `Mod 1`/`Mod 2`
keep their own direction, s166 §2.4). "Timeline" becomes a REAL 4-beat linear ramp Envelope (D6 "build",
D7 shared curve) — honest, visible, editable later. "Clip Position" becomes a real `Kind::ClipPosition`
that evaluates to position 0 until L5 wires a `ClipClock` (`ConnectionEngine.cpp` passes `nullptr`,
VERIFIED) — kept per ruling 2, dependency named in §12.

### 4.4 Piece 4 — renderer read (Lane C2, GL thread; VERIFIED anchors)
| Site | Today | Becomes |
|---|---|---|
| `CompositorEngine::applyClipTransform` (`:437-493`) | `clip.positionX/Y`, `clip.scale`, `clip.rotation` (needsTransform ×4 + uniforms ×4), `clip.anchorX/Y`, `clip.clipOpacity` | read the 7 `clip.eff(ClipScalar::…)` into locals at function entry; use the locals |
| `CompositorEngine::applyLayerTransform` (`:544-585`) | 6 layer fields | 6 `layer.eff(LayerScalar::…)` locals |
| `applyFXOnlyLayer` `:604` | `combinedOpacity(layer.opacity, clip.clipOpacity)` | `combinedOpacity(layer.eff(Opacity), clip.eff(Opacity))` |
| `compositeDeck` Opaque branch `:868,:887` and every other `layer.opacity` read (builder greps; `:1064` keying `u_opacity`, persistent-layer path) | `layer.opacity` | `layer.eff(LayerScalar::Opacity)` (read once per layer per frame into a local) |
| `:1239` `prevClip->clipOpacity` (transition) | raw | `prevClip->eff(ClipScalar::Opacity)` |
| `Renderer.cpp:415` and `:1241` `composition_->masterSpeed` | raw | `composition_->eff(CompScalar::Speed)` |
| `Renderer.cpp:706` `composition_->masterOpacity` | raw | `composition_->eff(CompScalar::Opacity)` |
| `Renderer::applyCompTransform` `:2021-2026` | 6 raw | 6 `composition_->eff(CompScalar::…)` |
Thread model: `LiveValue::effective` = relaxed atomic load + fallback to the plain float the GL thread
already reads today (A2 crossing, unchanged). `Composition::eff` const_casts internally (VERIFIED) — callable
from `const Layer&`/`const Clip&` contexts. Gate: `grep -n "clip\.positionX\|clip\.positionY\|clip\.scale\b\|
clip\.rotation\|clip\.anchor\|clip\.clipOpacity\|clipOpacity\|layer\.opacity\|layer\.positionX\|layer\.positionY\|
layer\.layerScale\|layer\.layerRotation\|layer\.layerAnchor\|composition_->masterOpacity\|composition_->
masterSpeed\|composition_->comp" src/render/CompositorEngine.cpp src/render/Renderer.cpp` returns comment
lines only.

### 4.5 Serialization — already done
`Clip.cpp:134`, `Layer.cpp:126`, `Composition.h` `"conns"` sparse maps via `ConnSerialization::scalarsToVar/
FromVar` (VERIFIED). "Save / quit / load — still connected" holds by construction; pinned by a headless
round-trip in C1 (a regression PIN — it passes on HEAD, stated honestly).

### 4.6 The Composition "Opacity" twin and `compOpacity` (Lane C4, own commit)
Ruling 6/11: one master opacity; `compOpacity` merges into master. Ruling 2: do not hide dead UI, wire
it. Resolution: the second knob binds to the SAME `CompScalar::Opacity` (exactly the Layer Master/Opacity
precedent, `LayerInspector.cpp:149,186`). `compOpacity` then has no writer but `fromVar` → delete the
field: `Composition.h` (member, `toVar` line, `fromVar` guard), `TestServer.cpp:1341-1368` (oracle branch;
the `:1026` "render-dead scalars" comment is also stale — fix it), `tests/test_composition.cpp:376,437,551`,
`tests/test_composition_tier_oracle.cpp:12,137,147,150` (rename the case "the render-dead scalars" → the
three remaining, drop the `compOpacity` lines). Old files carrying `"compOpacity"` load fine (unknown keys
ignored, `Composition::fromVar` is `hasProperty`-guarded). Separate commit so a reviewer can drop it alone.

---

## 5. BUILDER LANES — disjoint files, own `-B` dir each (HANDOFF rig fact `:2493`; gotcha 2026-09-05)

Conventions for every lane: `cmake -B build-<lane> -DCMAKE_BUILD_TYPE=Release && cmake --build build-<lane>
-j$(sysctl -n hw.ncpu)`; run ONLY your own test binary from `build-<lane>/tests/`; `git add -f` for anything
under `.harmony/`; report deviations in `.harmony/.reports/s-rta-0923/C<n>.report.md`; the builder NEVER
launches the app (C5 is Harmony's). A lane that needs a file outside its list STOPS and reports.
Order: **C0 alone first** (it owns `tests/CMakeLists.txt` — WAIT until the current `UU`/staged index state
is committed). Then **C1 ‖ C2 ‖ C3 ‖ C4** (C3/C4 compile against C0's stub declarations). Merge C1→C2→C3→C4
(each compiles alone; behaviour is complete only with all four). Then clean forced rebuild in `build/`,
`ctest`, then C5.

### C0 — scaffold (~30 min) — files: `tests/CMakeLists.txt` (APPEND only, after `test_bt_device_shapes`), `tests/test_manual_write.cpp`, `tests/test_conn_picker.cpp`, `src/connect/ManualWrite.h`, `src/connect/ManualWrite.cpp` (STUB: every function returns `std::nullopt`/`false`/no-op), `src/connect/ConnPicker.h`, `src/connect/ConnPicker.cpp` (STUB: returns `ConnSource{}` / `""`), `src/connect/ConnClock.h`
Two targets shaped exactly like `test_connection` (`tests/CMakeLists.txt:400-439`): same source list +
`ManualWrite.cpp` / `ConnPicker.cpp`, same libs/defs/warnings, `apply_sanitizers`, `catch_discover_tests`.
Test TUs contain the FULL case list of C1 and C4 below (assertions written now, against the stubs).
DONE = both targets build; `./tests/test_manual_write` and `./tests/test_conn_picker` run and **FAIL**
(record the failing-case counts in the report — that is the fail-first evidence for C1/C4); every other
target still passes; `git show --stat HEAD` lists all 7 files.

### C1 — funnel core (~2 h) — files: `src/connect/ManualWrite.cpp`, `src/connect/ParamConnection.h` (Grip::rank; `gripHeld/gripTouch/release` set it), `tests/test_manual_write.cpp` (may extend, never delete a C0 case)
Cases (each FAILS on C0's stub — proven by C0's run):
(a) `resolveControl` — comp scalar `opacity` → `&comp.masterOpacity` + `&comp.scalarConns[0]`; layer
    `(deck 0, layer 1) positionX` → `&layer.positionX`; clip `(0,0,col 2) scale` → `&clip.scale` with
    `toModel(0.5)==1.0`; clip effect param `(fx 0, param 1)` → `&fx.paramValues[1]`/`&fx.paramConns[1]`
    (after `resizeParams`); clip `dryWet`; source param (`fx -1, param 0`); macro 3 → `&bank.getMacro(3).
    manualValue`; unknown scalar key, out-of-range col, `control:"speed"` → nullopt.
(b) unconnected scalar: `manualWriteCore(HumanDecaying, 0.25)` → true, `clip.positionX == (0.25-0.5)*3840`.
(c) HumanHeld while a Lane grip holds → true; `grip.rank == 3`.
(d) Lane write while HumanHeld → false; field unchanged; grip unchanged.
(e) Lane write while HumanDecaying EXPIRED (`now > lastTouch + hold`) → true.
(f) HumanDecaying while HumanHeld → false (MIDI vs held slider).
(g) `manualReleaseCore(Lane)` while HumanHeld → grip survives; `manualReleaseCore(HumanHeld)` → cleared.
(h) engine interplay: connected Lfo scalar; `manualWriteCore(HumanHeld)` → `ConnectionEngine::evaluate` returns
    NaN and the twin is untouched; `manualReleaseCore` → next evaluate publishes, first value == the hand's
    `toNorm(manual)` (glide start, `handBackGlideMs` 100).
(i) [pin, passes on HEAD] Composition with `scalarConns[Opacity]` Lfo → `toVar` → `fromVar` → connected,
    shape equal; `grip` cleared.
DONE = `test_manual_write` all pass; `test_connection` still 27 cases pass; report the sha256 of
`ParamConnection.h` before/after (only the `rank` hunk).

### C2 — renderer read repoint (~1 h) — files: `src/render/CompositorEngine.cpp`, `src/render/Renderer.cpp`
Steps in §4.4. No new test (GL). Proof = (1) the §4.4 grep gate returns comment lines only (paste the
output in the report); (2) `test_compositor` (mirrors `combinedOpacity`/`effectiveClipSpeed`, GL-free) still
passes; (3) the app is NOT launched by the builder — C5 proves the pixels. Independent of C1/C3/C4.

### C3 — MainComponent + ApiServer + LayerStrip (~2 h) — files: `src/MainComponent.h`, `src/MainComponent.cpp`, `src/api/ApiServer.h`, `src/api/ApiServer.cpp`, `src/ui/LayerStrip.cpp`
(1) §4.1 tick. (2) §3.4 wrapper + path builders + hook seam. (3) §3.6 sites 1-11; the inline REST writes
are REMOVED, replaced by `onSetLayerOpacity` / `onSetClipEffectParam` callbacks (directive corollary:
replaced code fully removed). (4) `ApiServer::handleComposition` gains, per owner, `"live"`: comp
`{"masterOpacity": eff(Opacity), "masterSpeed": …, "positionX"…}` + `"connected": [keys]`; per layer
`"live": {"opacity": …, …}` + `"connected"`; per clip the 7 keys + `"connected"`. Reads are `eff()` (atomic
twin + the same plain-float fallback the endpoint already reads at `ApiServer.cpp:269`). (5) `LayerStrip`
drag grip (#11). DONE = builds; `ctest` unchanged; report lists every `= value;` writer it removed and the
grep from §3.6 with the surviving raw writers classified. Fail-first proof for (1)/(4) is C5 (see §6: the
`live` field is absent on the pre-change binary).
**COLLISION NOTICE:** spec step 3 (recorder wiring) also lands in `tickFeaturePipeline` and at these D6a
sites. C3 MUST merge before step 3 is dispatched; step 3's packet then says "insert `RecorderClock::tick`
+ `Player::advanceTo` between `updateValues` and `connectionEngine_.tick`; assign `onManualWrite`/
`onManualRelease`; never touch the site bodies".

### C4 — widget + inspectors (~3 h) — files: `src/ui/UniversalParamControl.h`, `src/ui/UniversalParamControl.cpp`, `src/ui/CompositionInspector.h/.cpp`, `src/ui/LayerInspector.h/.cpp`, `src/ui/ClipInspector.h/.cpp`, `src/connect/ConnPicker.cpp`, `tests/test_conn_picker.cpp`; SECOND COMMIT (§4.6): `src/model/Composition.h`, `src/test/TestServer.cpp`, `tests/test_composition.cpp`, `tests/test_composition_tier_oracle.cpp`
Cases in `test_conn_picker` (FAIL on C0's stub): `BpmSync{shape 3, div 2}` → `Lfo{Square, 1.0}`; `{1, 5}` →
`Lfo{SawUp, 8.0}`; `Signal "Bass"` → `Kind::Signal`, name "Bass"; `Macro 7` → `Macro(7)` (pins the `<206`
fix); `Timeline` → `Envelope`, `curve.pts.size()==2`, `eval(0.5)==0.5`, `cycleBeats 4`, `Clock::Beats`;
`ClipPosition` → `Kind::ClipPosition`; `Manual` → `Kind::None`; `describeSource` of each == the menu
string ("Square 1 Beat", "Bass", "Macro 8", "Timeline", "Clip Position", ""). DONE = tests pass; build
green; the 21 `bindConnection` calls present (grep count 21 + the `nullptr` unbinds); manual-gate items
listed in §6.4 handed to Harmony. Second commit: `grep -rn compOpacity src tests` → 0.

### C5 — live gate (Harmony, ~40 min incl. two launches) — files: `.harmony/probe-lane3.sh`, `.harmony/probe-lane3-{comp,layer,clip}.json` (`git add -f`)
§6. Run once against the PRE-change binary (expect FAIL at oracle A), once against the merged tree
(expect all PASS). Paste both outputs in `.harmony/.reports/s-rta-0923/C5.report.md`.

Sizes: C0 S, C1 M, C2 S, C3 M, C4 M, C5 S. Critical path ≈ C0 → C4 → merge → C5 ≈ 5 h of builder time
plus reviews. Reviewer per lane (builds-never-verifies): Reviewer-on-source for C1-C4; Harmony-on-behavior
= C5.

---

## 6. LIVE-VERIFICATION RECIPE — production mode, no test server, NO output window

Rig facts used (all VERIFIED in `.harmony/VALIDATION.md`, `.harmony/gotchas.md`, `ApiServer.cpp:117-224`):
`ApiServer` is always on at `http://127.0.0.1:7070` (IPv4) with `/api/health`, `/api/set_bpm`,
`/api/load_composition`, `/api/trigger_clip`, `/api/composition`, `/api/set_layer_opacity`, `/api/render_frame`
(writes a PNG; deterministic on the deck path — proven by `probe-deck-path.sh` 2026-09-05). `--test-mode`
is NOT used (production mode runs the analysis thread; VALIDATION row 4 PASSED this session on a production
launch reading `/api/bpm` on 7070). `inject_features` is test-mode only (`ApiServer.cpp:175`) — not needed:
`set_bpm` puts the tracker in manual mode (`MainComponent.cpp` `apiServer_->onSetBpm` → `setManualMode(true)`,
VERIFIED) and beat phase then runs without audio (CLAUDE.md "Manual BPM Mode"). Launch via `open`
(gotcha 2026-07-17); first launch after a rebuild needs Boris to click Allow on the TCC mic prompt (gotcha
2026-07-25) — if `/api/health` never answers, `screencapture -x` and LOOK before concluding.

### 6.1 Preconditions
```
pgrep -fl 'MacOS/Audio-DNA'            # MUST be empty (gotcha 2026-09-05: a second instance self-quits silently)
ls build/AudioDNA_artefacts/Release/Audio-DNA.app   # the CLEAN forced rebuild of the merged tree
```
### 6.2 Fixtures (written by C5; shape = `.harmony/probe-composition.json` + `conns`)
`probe-lane3-layer.json`: one deck "A", `numColumns 2`, one layer `L1` (`opacity 1.0, visible true,
blendMode 1`) with clip `{"name":"gw","id":1,"mediaType":4,"sourceType":"gravity_well"}`, and on the LAYER:
```json
"conns": {"opacity": {"src":{"kind":"lfo","shape":"square","cycleBeats":1.0,"phase":0.0,"pulseWidth":0.5},
          "shape":{"min":0.0,"max":1.0,"invert":false,"playback":"forward","loop":true,"curve":"Linear",
                   "inMin":0.0,"inMax":1.0,"smoothMs":0.0},"enabled":true}}
```
`probe-lane3-clip.json`: same, `conns` on the CLIP: `"scale"` ← `sine`, `cycleBeats 2.0`, `min 0.3 max 0.7`.
`probe-lane3-comp.json`: same, `conns` at COMPOSITION level: `"positionX"` ← `sine`, `cycleBeats 4.0`,
`min 0.4 max 0.6` (±384 px). (Keys per `ConnSerialization.cpp:103-153`, VERIFIED.)
### 6.3 Script (`probe-lane3.sh`, structure copied from `probe-deck-path.sh` incl. its graceful-quit + CGWindowList block)
```
A='http://127.0.0.1:7070'
open --stdout /tmp/l3-out.log --stderr /tmp/l3-err.log build/AudioDNA_artefacts/Release/Audio-DNA.app
poll $A/api/health up to 60 s (patiently — first probes during startup can return empty)
POST $A/api/set_bpm {"bpm":120}                                   # 0.5 s per beat; square 1 beat = 0.25 s high / 0.25 s low
POST $A/api/load_composition {"path": <layer fixture>} → expect "ok":true
POST $A/api/trigger_clip {"layer":0,"column":0}; sleep 1
GET  $A/api/composition → assert decks[0].layers[0].activeClipColumn == 0        (deck path live — else everything below is void)
                       → assert decks[0].layers[0].connected contains "opacity"  (PRE-CHANGE: key absent → FAIL here)
ORACLE A: sample GET /api/composition 20× at 50 ms; collect layers[0].live.opacity
          PASS = both a value <= 0.05 and a value >= 0.95 observed; and it changes at least twice
ORACLE B: loop until live.opacity >= 0.95 then immediately POST render_frame → hi.png;
          loop until live.opacity <= 0.05 then immediately POST render_frame → lo.png;
          PASS = md5(hi) != md5(lo) AND mean luminance(lo) < 0.05 * mean luminance(hi)   (PIL via .venv, as probe-deck-path.sh)
GRIP:     POST $A/api/set_layer_opacity {"layer":0,"opacity":0.5}; sample live.opacity every 50 ms for 1 s
          PASS = the first 4 samples are all ≈0.5 (Decaying grip, 250 ms), AND by 600 ms a sample <=0.05 or >=0.95
          appears again (grip expired + 120 ms glide, signal resumed) — Ruling A "let go and the signal takes over"
CLIP:     load clip fixture, trigger, two render_frame 0.5 s apart → md5 differ (scale sine 2 beats)
COMP:     load comp fixture, trigger, two render_frame 1.0 s apart → md5 differ (positionX sine 4 beats)
quit gracefully (osascript, never pkill); pgrep empty; CGWindowList count 0
```
No output window is ever opened (SCREEN-SAFETY LAW satisfied by construction; the handoff says so and
still runs `screencapture -x` at close because the law says LOOK).
Save half of "save/quit/load": no REST save exists; covered by C1 case (i) + the `load_composition` path
above, which IS the load half on a real file.
### 6.4 Owner-attended checks (Boris, whenever he is around; not gates)
Connect Layer 1 ▸ Opacity → BPM Sync ▸ Square ▸ 1 Beat from the triangle: preview strobes on the beat.
Drag the slider while it strobes: it follows the hand; let go: it eases back and strobes again. Click
another clip and back: still connected (cyan). Turn a MIDI knob mapped to Master Opacity while a signal
drives it: knob wins, then hands back after a quarter second. Save the composition, quit, reopen: still
connected.

---

## 7. RULINGS IN `binding-decisions.md` THAT CONSTRAIN THIS LANE (cited by heading/number)

- **s-rta-0906 #3 CORE PRODUCT LAW** ("every single parameter … same exact method … all will be timed") —
  the reason the lane exists; one mechanism (`ParamConnection` + one engine), no per-panel special cases.
- **#2 do not hide/grey/delete dead UI — wire it** — every one of the 21 gets bound; the Composition
  "Opacity" twin is bound to Master rather than deleted (§4.6; Q1 in §13).
- **#5 the UI is disposable, mechanism first** — binding by pointer, translation in headless `ConnPicker`,
  funnel in `src/connect/`; the inspector edits are minimal and will be scrapped.
- **#1 push routinely.** **s167 #6 / #11** one master opacity, three levels multiply (`compOpacity` merges).
- **s167 #7** hand-back glides (`handBackGlideMs` 120, D14); **s166 D15** Decaying grips hold 250 ms.
- **s166 §R Ruling A** (latest input wins, automatic hand-back) and **Ruling B** (range / invert / playback
  are the per-connection controls) — §3.3, §4.2's range/invert wiring.
- **s167 #13, #16, #18, #22** — a lane is a WRITER, not a connection kind (`ParamConnection.h` Kind comment);
  the D8 chain is the `Hand` rank; `manualWrite`'s `bool` is what the Player's `Sink` builds on.
- **s167 #10** macro banks at all three levels — `Context::macros` stays caller-supplied; `resolveControl`
  takes the global bank explicitly (`macroScope 0` only).
- **s168 #25** drop-vs-oscillator-phase is a switch — `ConnShape::resetPhaseOnStructural` exists, default
  false; the picker does not expose it (UI rewrite must — noted, not built).
- **s166 §7.2 D5** Clip Position on layer/comp controls "disabled for now" — this lane keeps the menu item
  (ruling 2) and it evaluates to 0 until L5 (Q4). **D13** anchorY has no slider — rewrite.
- **s166 §7.2 D10** Cmd-Z removes a connection — free for effect rows only; scalars need a `ScalarConnCmd`
  (§12, Q5).
- **R8** (s167 §6 R8; ruling28 spec §0): the CONNECTION lane owns `manualWrite`; the recorder hooks it
  (`onManualWrite`/`onManualRelease` seam, §3.4).
- **SCREEN-SAFETY LAW** — §6 never opens the output window; graceful quit; screen looked at.
- **"The party that builds never verifies"** — C5 is Harmony's; reviewers per lane.
- **Rig facts:** own `-B` per builder; `.harmony/` needs `git add -f` + `git show --stat`; re-grep anchors.
- Not a constraint but carried: the s168 addendum item 1 (manual Resync vs `totalBarCount`) is still
  unasked — outside this lane.

---

## 8. TRADEOFFS CONSIDERED

- **Widgets call `manualWrite` with a `ControlPath` (so the recorder captures mouse moves) vs bind by
  pointer** — REJECTED for now: `ClipInspector` gets an `EffectScope` (deck/layer/col — VERIFIED
  `EffectScope.h`) so it COULD build paths, but s167 D14 explicitly defers inspector capture to the rewrite,
  the widgets are disposable, and pointer binding needs zero coordinate plumbing. The funnel is designed so
  the rewrite can route widgets through it (`makeScalarPath(EffectScope, key)` is a 10-line helper later).
- **Put `manualWrite` entirely in `MainComponent`** (the spec's literal proposal) — REJECTED: untestable
  headlessly and would make `src/connect/` depend on nothing while `MainComponent` grows another 150 lines.
  The wrapper is 15 lines; the core is tested in isolation.
- **Deliberateness rank on the grip vs "latest wins" everywhere** — rank chosen: s166 §2.3(iii) already
  says a Held grip is not displaced by a Decaying one, and D8 needs `false` to mean "a more deliberate hand
  holds it"; without a rank there is no way to answer the Player's `touch()` (Q3 asks Boris only about the
  MIDI-vs-held-slider feel).
- **Retire `tickModulation` now (s166 L3(c)) since the engine ticks anyway** — REJECTED: that is the
  effect-param vertical slice (repoint `effParam`, bind effect rows, delete `SourceMode`); doing it here
  triples the fence and reproduces the L9 freeze class if (a)/(b) are not done together (s166 L3 UNSAFE
  note). Both mechanisms coexist safely: the engine publishes only into twins, the old loop writes fields.
- **Delete the Composition "Opacity" knob** — REJECTED (ruling 2), bound to Master instead; asked (Q1).
- **Test the renderer repoint with a GL-free mirror test** (as `test_compositor` does for `combinedOpacity`)
  — REJECTED: a mirror cannot fail when the real site is wrong (the probe-deck-path comment documents
  exactly that failure: "a correct multiplier applied to the wrong quantity passes every arithmetic test").
  The grep gate + C5's pixels are the proof.
- **Thumb follows the signal vs meter-only** — follow chosen (Resolume convention; makes the hand-back glide
  visible); asked (Q2) because the rewrite inherits the convention.

---

## 9. RISKS — and what to verify while building

- **R-A `EffectSlot::resizeParams` lazy resize on the message thread** (`ConnectionEngine.cpp` `tickEffectVector`,
  VERIFIED) reallocates `paramLive` while the GL thread… does NOT read `paramLive` today (it reads
  `paramValues[p]`, `CompositorEngine.cpp:376`). Safe for THIS lane. It becomes a use-after-free the day the
  effect-param lane repoints `effParam` unless slots are sized at every creation site first (s166 §3.3, the
  3 `paramValues.push_back` sites in `MainComponent.cpp`). Named as the first line of that lane's packet.
- **R-B A blocked message thread freezes every connection** (file dialog, synchronous `executeOnGLThread`) —
  same property `tickModulation` has today; not a regression (s166 §4.3). Accepted.
- **R-C Held grip never released** (a widget destroyed mid-drag) freezes that parameter's signal forever.
  Mitigation: `UniversalParamControl::~UniversalParamControl` releases `conn_` if held (C4); `bindConnection(
  nullptr)` releases too; preset load/undo copy-reset grips (VERIFIED copy ctor).
- **R-D Unconnected-param Decaying grips never expire via the engine** — handled by `gripActive()` timestamps
  in the core (§3.2); test (e) pins it.
- **R-E `applyCompTransform`'s `isDefault` early return** reads `eff()` after C2, so a driven positionX
  correctly runs the pass; but a connection whose RANGE spans exactly the default (e.g. min=max=0.5) is a
  no-op — expected, not a bug.
- **R-F The picker's Clip Position yields outMin until L5** — a scalar connected to it jumps to its range
  minimum. Deterministic; documented; Q4.
- **R-G `masterSpeed` is read into `scaledTime_` every GL frame** (`Renderer.cpp:419`): a Square LFO on
  Speed makes time advance in steps — correct per the ruling that a connection replaces the value; verify it
  does not go negative (`speed4` maps [0,1] → [0,4], never negative).
- **R-H Two inspectors bound to the same clip** cannot happen (one inspector instance each), but the Layer
  Master/Opacity pair and Comp Master/Opacity pair are two widgets on ONE conn: both paint cyan, either
  picker edits it — by design (VERIFIED they already share the field).
- **R-I `tests/CMakeLists.txt` is currently unmerged in the index** (`UU`) — C0 must wait for that commit or
  its append will be swept into someone else's resolution.
- **R-J Fail-first honesty:** C2's repoint, C3's tick and C4's widget binding cannot fail headlessly (GL /
  app). Their fail-first proof is C5 run on the pre-change binary — which MUST fail at "connected contains
  opacity" (field absent) and would, on a tree with C3 but not C2, pass oracle A and FAIL oracle B. That
  ordering discriminates the lanes; run C5 after EACH merge if cheap enough (2 launches each).

**Strongest counterargument to this plan:** *"Do the effect-param vertical slice (s166 L3) first — it is
the demo Boris can see on any clip today, and the 21 scalars ride the same widget change."* It loses on
three facts: (1) the effect rows already move under a signal (the old `tickModulation`), so L3 changes
nothing visible until it is fully done, whereas the 21 are dead today and this lane makes them live; (2) L3
carries the R-A hazard and the `SourceMode` deletion, i.e. the widest fence; (3) this lane leaves L3
strictly smaller (engine ticking, funnel defined, `bindConnection` present) — L3 becomes "repoint 3 GL
reads, bind the effect rows, delete the old loop". Sequencing scalar-first is the cheaper path to both.

---

## 10. WHAT "DONE" LOOKS LIKE (the gate Harmony runs, in order)
1. Clean forced rebuild of the merged tree in `build/` exits 0; `ctest` = 315 (session baseline after
   R28/E/…; RE-RUN, never inherit) + the new `test_manual_write` and `test_conn_picker` cases, 0 failed.
2. §4.4 grep gate returns comment lines only; §3.6 writer grep classified.
3. `probe-lane3.sh` on the pre-change binary: FAILS at "connected contains opacity"; on the merged tree:
   all PASS, artifacts in `/tmp/audiodna-lane3/`.
4. `pgrep` empty, CGWindowList 0, `screencapture -x` looked at; handoff states the screen state.
5. `.harmony/.reports/s-rta-0923/C0..C5.report.md` + one review per lane on disk; `git show --stat` verified
   for every commit under `.harmony/`.

---

## 11. FOLLOW-UPS NAMED, NOT BUILT (each with the constraint this lane leaves it)
- **Effect/source-param vertical slice (s166 L3 a/c/d):** repoint `CompositorEngine.cpp:278-279,376,384,409`
  to `effParam/effDryWet`, the source-param upload to `sp.live.effective(sp.value)`; wire
  `onSourceParamsPublished`; size slots at creation (R-A) BEFORE the repoint; bind effect/source rows via
  `bindConnection`; delete `SourceMode`/`tickModulation`; TestServer `add_connection` endpoints;
  `RoutingEngine` deletion.
- **Macro knobs → `Macro::conn`** (`MacroPanel.cpp:136,144` still write `sourceSignalId`; `MacroBank::
  updateValues` still reads it — the engine's `Kind::Macro` reads `currentValue`, so both agree today).
- **`ScalarConnCmd`** (undo of connect/disconnect on the 21; ~40 lines on the `Command` base, no fence needed).
- **L5 ClipClock** — makes Clip Position / Envelope(ClipPosition) live for clip scalars.
- **Expose `resetPhaseOnStructural`, smoothing, playback/loop per connection in the rewrite UI.**
- **`compScale` linear vs clip/layer exponential** — pre-existing, preserved (s166 §6).

---

## 12. QUESTIONS FOR BORIS (product/feel only — technical choices above are decided)
1. The Composition inspector has two opacity knobs, "Master" and "Opacity". You ruled there is one master
   opacity. This lane makes BOTH knobs the master (so neither is dead). Do you want the second knob removed
   now, or left as a twin until the new UI? [Recommend: leave it; delete at the rewrite — no mechanism impact.]
2. When a signal drives a slider, should the on-screen slider MOVE with the signal (so grabbing it starts
   from where the signal is), or stay where your hand left it with a small meter showing the signal?
   [Built: it moves. Resolume does this; the new UI will inherit whichever you pick.]
3. You are holding a slider with the mouse and someone turns the MIDI knob mapped to the same control.
   Built: the mouse keeps it (the MIDI turn is ignored until you let go). Alternative: whichever moved last
   wins. [Recommend: mouse keeps it — a held slider is the more deliberate hand.]
4. "Clip Position" as a source on a LAYER or COMPOSITION control: keep it in the menu (it does nothing
   useful until the clip clock lands) or remove it from those menus? [Recommend: keep, label it in the
   rewrite; you ruled "wire, don't hide".]
5. Should Cmd-Z undo connecting/disconnecting a signal on these 21 controls now (small extra piece), or
   is that new-UI work? [Recommend: new UI.]

---

## 13. SUMMARY FOR HARMONY (15 lines)
1. Recon verified at HEAD 9257537: `eff()` 0 callers, `ConnectionEngine::tick` 0 callers, `onSourceChanged` 0 listeners, `manualWrite` absent — all 21 still inert.
2. 21 widgets map onto 22 provisioned scalars; all backing fields render today EXCEPT `compOpacity` (merged into master by ruling 11 → its knob binds to Master, field deleted).
3. Four missing pieces, one pattern: engine tick (C3), widget↔model binding (C4), picker→`ConnSource` translation (C4, headless-tested), renderer `eff()` repoint (C2).
4. `manualWrite` DEFINED (R8): headless core `src/connect/ManualWrite.*` (`resolveControl(ControlPath)`, `manualWriteCore/Touch/Release` with a D8 deliberateness rank on `Grip`) + 15-line `MainComponent` wrapper mapping `Origin` → rank, exposing `onManualWrite/onManualRelease` for the recorder to hook.
5. Ranks: HumanHeld 3 > HumanDecaying 2 > Lane 1; lower rank is refused (`false`), equal = latest wins; Decaying expires by timestamp even on unconnected params.
6. 11 external writer sites (OSC 3, MIDI 5, REST 2, LayerStrip 1) routed through the funnel; REST inline writes replaced by callbacks; v1 `effectChain_` writes untouched (L7).
7. Lanes: C0 scaffold (owns tests/CMakeLists.txt — WAIT for the current `UU` index state to commit) → C1 ‖ C2 ‖ C3 ‖ C4 → C5 live gate. Disjoint files listed per lane; own `-B` dirs.
8. Fail-first: C0 ships stubs so `test_manual_write` (9 cases) and `test_conn_picker` (8 cases) FAIL before C1/C4; GL/app pieces fail-first via C5 on the pre-change binary (the `live`/`connected` fields are absent).
9. Live recipe: production launch on 7070 only, `set_bpm 120`, `load_composition` fixture with `conns`, `trigger_clip`; oracle A = `/api/composition` `live.opacity` toggles 0↔1; oracle B = `render_frame` md5 differ hi/lo; grip proof via `set_layer_opacity` (0.5 held ~250 ms then signal resumes). No output window ever.
10. Collision: C3 lands in `tickFeaturePipeline` + the D6a sites that spec step 3 also needs — C3 MUST merge before step 3 is dispatched; step 3 inserts between `updateValues` and `connectionEngine_.tick`.
11. Hazard left for the effect-param lane: `resizeParams` lazy resize becomes a GL UAF once `effParam` is repointed — size at creation first.
12. Vocabulary extension for the step-3 author: `control:"param"` with `fx.i == -1` on a clip = source param; `control:"speed"` unresolvable (no ParamConnection) — decided, flagged.
13. Rulings binding: core law #3, wire-don't-hide #2, UI disposable #5, opacity #6/#11, glide #7, Ruling A/B, D14/D15, lane≠connection #13/#18/#22, switch #25, R8, SCREEN-SAFETY, builds-never-verifies, own -B dirs.
14. Five Boris questions (twin opacity knob; thumb follows signal; mouse vs MIDI while held; Clip Position on layer/comp menus; undo now vs later) — defaults chosen so nothing blocks.
15. Strongest counterargument (do the effect-param slice first) loses: those rows already move today; this lane makes 21 dead controls live and leaves that slice strictly smaller.

REPORT_FILE: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/7e364303-6335-4445-955e-49321e60ded4/scratchpad/s-rta-0923-lane3-plan.md
STATUS: COMPLETE
