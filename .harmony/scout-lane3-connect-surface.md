# Lane 3 recon: connect-surface today (2026-09-06)

Read-only recon. Repo confirmed as the real tree (`git rev-parse --show-toplevel`
= `/Users/boriskarpman/projects/RealTimeAudio`, HEAD `6418c2f0` @ 2026-09-05
20:41, NOT the `RealTimeAudio copy` stale duplicate). No files touched except
this one.

## 1. Re-verify the 21

MEASURED — grep for `UniversalParamControl` fields in each inspector header:

- `src/ui/CompositionInspector.h`: `masterControl_`, `speedControl_`,
  `opacityControl_`, `posXControl_`, `posYControl_`, `scaleControl_`,
  `rotationControl_`, `anchorControl_` = **8**
- `src/ui/LayerInspector.h`: `masterControl_`, `opacityControl_`,
  `posXControl_`, `posYControl_`, `scaleControl_`, `rotationControl_`,
  `anchorControl_` = **7**
- `src/ui/ClipInspector.h`: `clipOpacityControl_`, `posXControl_`,
  `posYControl_`, `scaleControl_`, `rotationControl_`, `anchorControl_` = **6**
  (plus `sourceParamControls_`, a dynamic per-source vector — separate class
  of control, not counted here, see §3b)

**Total: 21. Unchanged from the inherited claim.** A connection engine has
landed since (`src/connect/`), and the *model* now actually provisions one
more slot than the UI exposes: `ClipScalar::Count` in
`src/connect/ScalarParams.h` is 7 (adds `AnchorY`), and `Clip.h`'s own
comment on `Clip::scalarConns` says anchorY is a scalar "no UI has ever
written." So the model's real connectable-scalar surface is **22**
(7 Clip + 7 Layer + 8 Comp via `ClipScalar`/`LayerScalar`/`CompScalar` in
`src/connect/ScalarParams.h`), but only 21 of those 22 have a
`UniversalParamControl` widget at all today.

For every one of the 21: does a value connected to it reach anything that
renders? **No, confirmed by three independent negative greps**, not one:

- `grep -rn "\.eff(\|->eff(" src/` → **zero hits**. `Composition::eff()`,
  `Layer::eff()`, `Clip::eff()` (declared `src/model/Composition.h:73`,
  `src/model/Layer.h` scalar block, `src/model/Clip.h` scalar block) are
  never called anywhere — only declared/defined.
- `grep -rln "ConnectionEngine" src/` → only files under `src/connect/`, plus
  two comment-only mentions in `src/model/Clip.h` / `src/model/Layer.h`.
  `ConnectionEngine::tick()` is never invoked by production code.
- `grep -rn "onSourceChanged\s*=" src/` → **zero hits** (also tried
  `onSourceChange\b`, `sourceChangedCallback`, `SourceChangeCallback` as
  alternate spellings — all zero). Confirmed by reading
  `UniversalParamControl::handleSourcePickerResult` in
  `src/ui/UniversalParamControl.cpp`: it sets `sourceMode_`/`sourceName_`
  (widget-local paint state used only by `UniversalParamControl::isConnected()`,
  itself just `sourceMode_ != SourceMode::Manual`) and fires
  `if (onSourceChanged) onSourceChanged(...)` — guarded, so with zero
  listeners it is a pure no-op today.

So the "cyan triangle + name paints, nothing downstream reads it" diagnosis
is exactly as true today as when it was inherited, for all 21.

## 2. What `src/connect/` provides now

Files: `ConnectionEngine.{h,cpp}`, `ParamConnection.h`, `LiveValue.h`,
`ScalarParams.h`, `ConnectionShaper.{h,cpp}`, `AutomationCurve.h`,
`ConnSerialization.{h,cpp}`.

- **`ParamConnection`** (`src/connect/ParamConnection.h`): the whole story
  for one slider — `ConnSource` (kind: None/Signal/Macro/Lfo/Envelope/
  ClipPosition), `ConnShape` (range/invert/playback/curve/smoothing),
  `Grip` (Held/Decaying/None — Ruling A "latest input wins, auto
  hand-back"), and non-serialized runtime `State`. Lives INSIDE the thing it
  controls: `EffectSlot::paramConns[p]`, `SourceParam::conn`,
  `Clip::scalarConns[i]` / `Layer::scalarConns[i]` /
  `Composition::scalarConns[i]`, `MacroBank::Macro::conn`.
- **`LiveValue`** (`src/connect/LiveValue.h`): `std::atomic<float> v{NAN}`
  twin sitting next to each manual field; `.effective(manual)` returns the
  live value or falls back to `manual` if NaN. Copy/move-safe wrapper around
  the atomic so value-type structs (`Clip`, `Layer`) keep compiling.
- **`ScalarParams.h`**: shared `ScalarDef` tables (`clipScalarDefs()`,
  `layerScalarDefs()`, `compScalarDefs()`) with `toModel`/`toNorm` conversion
  formulas, lifted and re-verified against today's inspector math (cites
  `ClipInspector.cpp`, `LayerInspector.cpp`, `CompositionInspector.cpp` by
  name in its own header comment).
- **`ConnectionEngine`** (`src/connect/ConnectionEngine.{h,cpp}`): the one
  evaluator. `static float evaluate(ParamConnection&, manualNorm, Context,
  ClipClock*)` is the pure-ish per-connection pipeline (source → shape →
  grip takeover / hand-back glide), fully unit-testable. `void
  tick(Composition&, Context)` is the per-frame walk: macros →
  `comp.scalarConns` → `comp.globalEffects` → per deck/layer
  (`layer.scalarConns`, `layer.layerEffects`) → per clip
  (`clip.scalarConns`, `clip.effects`, `clip.sourceParams`). Asserts message
  thread (`jassert(juce::MessageManager::getInstance()->isThisTheMessageThread())`
  in `ConnectionEngine::tick`).
- **`ConnSerialization`**: sparse per-scalar JSON map, already wired into
  `Composition::toVar/fromVar`, `Clip.cpp`, `Layer.cpp` (all confirmed via
  `scalarConns` grep below).

**Is there a `manualWrite` funnel? No** (checked `manualWrite`, `writeManual`
— zero hits). What exists instead is `manualRef(Owner&, ScalarEnum)`
(`src/model/Composition.h:516`, `src/model/Clip.cpp:4`,
`src/model/Layer.cpp:4`) — an internal accessor returning a mutable
reference to the raw struct field, used only by `ConnectionEngine`'s own
`tickScalars` (to read the current manual value for grip fallback) and by
`eff()`'s `.effective(manual)` call. It is **not** a funnel manual writers
are expected to go through — every existing manual writer (UI
`onValueChanged`, OSC handlers, MIDI/control-surface binding dispatch) still
writes the raw field directly today, and nothing in this design changes
that. See §6 for why that matters.

**Model wiring is real, not aspirational** — `scalarConns`/`scalarLive`
exist as actual array members today:
`src/model/Composition.h:71-72`, `src/model/Layer.h` (comment block "===
Connections (s167-l2) ==="), `src/model/Clip.h` (same comment block), each
serialized via `ConnSerialization::scalarsToVar`/`scalarsFromVar` in
`Composition.h`, `Clip.cpp`, `Layer.cpp`. This is Lane 2 / s167-l2's output,
landed and unit-tested (`tests/test_connection.cpp`, 25 `TEST_CASE`s
covering shaper math, LFO, grip state machine, serialization round-trips,
`AutomationCurve`, and — directly — `ConnectionEngine::tick publishes into
twins with macros ticked first`). None of it is wired to a live UI or a
live per-frame caller in the app itself.

## 3. Every existing WORKING connect path

Two, both **independent of `src/connect/` entirely** — a completely
separate, older, hand-rolled mechanism, still alive and freshly touched
(comments dated 2026-09-05, one day before this recon, "L9
modulation-freeze fix"):

**(a) `EffectStackView::tickModulation()`** (`src/ui/EffectStackView.cpp`) —
drives per-effect-slot params (`Clip::EffectSlot::paramValues[p]`) and
dry/wet. Shape: for each row's `UniversalParamControl`, if
`pc.isConnected()` (widget-local), linear-scan `SignalRegistry` by name (or
`MacroBank` by parsed "Macro N" string) every tick, then **write directly
into `fx.paramValues[p]`** — the same storage the manual slider writes, no
separate live twin. Comment in the source names this explicitly:
"Render-critical write: unconditional, every tick... CompositorEngine reads
this every GL frame and must never see it suppressed." Display update
(`pc.setSourceValue(...)`) is separately gated by a change-epsilon.
`onParamChanged` (meant to "notify renderer") has **zero assignments**
anywhere — dead callback; the actual render effect happens because
`CompositorEngine` reads `fx.paramValues[p]` directly, not through a
callback.

**(b) `ClipInspector::tickModulation()`'s second loop** over
`clip_->sourceParams` (Source-type clips only), same shape, writes
`clip_->sourceParams[i].value` directly, then fires
`onSourceParamsChanged(clip_)`. Unlike (a)'s dead callback, **this one is
wired**: `src/MainComponent.cpp:1451` —
`inspectorPanel_->getClipInspector().onSourceParamsChanged = [this](Clip*
clip) { previewPanel_.getRenderer().updateActiveSourceParams(clip->sourceParams);
};` → `Renderer::updateActiveSourceParams` (`src/render/Renderer.cpp:97`) →
consumed by `CompositorEngine`'s `sourceRenderFn_`
(`src/render/CompositorEngine.h:58`).

**Call chain / thread for both:** `MainComponent::tickFeaturePipeline()`
(`src/MainComponent.cpp:3057`), driven by `mappingTickTimer_.startTimerHz(120)`
(`kMappingTickHz = 120`, `src/MainComponent.h:272`, timer started
`src/MainComponent.cpp:313`) → `inspectorPanel_->tickModulation()`
(`src/ui/InspectorPanel.cpp:170`) → `clipInspector_`/`layerInspector_`/
`compInspector_`.`tickModulation()` → `effectStackView_.tickModulation()`.
**All message thread** (JUCE `Timer::timerCallback` runs on the message
thread; `ConnectionEngine::tick`'s own jassert would be satisfied here too).

**Is `ClipInspector.cpp`'s sourceParams loop still working?** Yes, confirmed
alive today (`src/ui/ClipInspector.cpp:822` `tickModulation()`, called every
120Hz tick). **Others now?** No third working path found — the fixed 21
scalar controls in all three inspectors have no equivalent loop (§1).

## 4. The gap

Not "one of three" — **the write target already exists**, so what's missing
is everything else, and there's a 4th piece the original framing didn't
name:

1. **Registration** (MISSING): none of the 21 `UniversalParamControl`
   instances assign `onSourceChanged`. Picking a source from the popup menu
   (`UniversalParamControl::handleSourcePickerResult`,
   `src/ui/UniversalParamControl.cpp`) only sets `sourceMode_`/`sourceName_`
   for painting; nothing translates that into a `ConnSource` written into
   `comp.scalarConns[i]` / `layer.scalarConns[i]` / `clip.scalarConns[i]`.
   **Insertion point:** each of the three inspector `.cpp` files, next to
   where each control is constructed (e.g.
   `CompositionInspector.cpp` near `posXControl_.onValueChanged = ...`) —
   need a new `onSourceChanged` handler per control that writes into the
   owning struct's `scalarConns[CompScalar::PosX]` etc.
2. **Per-frame evaluation** (MISSING): `ConnectionEngine::tick()` is never
   called. **Insertion point:** `MainComponent::tickFeaturePipeline()`
   (`src/MainComponent.cpp:3057`), the same 120Hz message-thread tick that
   already drives `EffectStackView::tickModulation()` — natural spot to add
   `ConnectionEngine::tick(composition_, ctx)` once a `Context` (SignalRegistry,
   MacroBank, FeatureSnapshot, dt, now) can be assembled there. This file
   already has all four inputs in scope (`signalRegistry_`, `globalMacroBank_`,
   `snap`, and a timer-derived dt).
3. **Write target** (EXISTS, unused): `scalarConns`/`scalarLive`/`eff()` on
   `Composition`/`Layer`/`Clip` — built, serialized, unit-tested, never
   populated or read in the running app.
4. **Render read** (MISSING, not called out in the original framing but
   equally blocking): `src/render/CompositorEngine.cpp` and
   `src/render/Renderer.cpp` read the **raw manual fields directly**, never
   `eff()`:
   - `CompositorEngine.cpp`: `clip.positionX/positionY/scale/rotation/
     anchorX/anchorY/clipOpacity`, `layer.positionX/positionY/
     layerAnchorX/layerAnchorY/opacity` (all confirmed by grep, e.g.
     lines building the clip transform matrix and `combinedOpacity(layer.opacity,
     clip.clipOpacity)`).
   - `Renderer.cpp`: `composition_->compPositionX/compPositionY/compScale/
     compRotation/compAnchorX/compAnchorY` (all six, one block), plus
     `composition_->masterOpacity` (`Renderer.cpp:706`, landed under a
     comment tag "S167-L4b" — see note below) and `composition_->masterSpeed`
     (`Renderer.cpp:415,1241`).

   So even if (1) and (2) were both wired today, **nothing would move on
   screen** until these ~15 read sites are changed to call
   `.eff(ClipScalar::PosX)` / `.eff(LayerScalar::Opacity)` /
   `.eff(CompScalar::Speed)` etc. instead of the raw field. This is the
   single most important correction to the inherited framing: the
   bottleneck is not just "wire it up," it's "wire it up AND repoint every
   renderer read."

   **Side note, not part of the 21 but adjacent and worth flagging:** a
   separate recent lane ("S167-L4b") already made `masterOpacity` and
   `masterSpeed` render-consuming (`Renderer.cpp:706` comment: "masterOpacity
   was silently render-dead all session... UNCONDITIONAL"). A stale comment
   in `src/test/TestServer.cpp` (near its "S166-L8: Composition-Tier Oracle"
   block, `TestServer.cpp:1025`) still calls `masterOpacity`/`masterSpeed`
   "render-dead scalars" — that claim is now superseded by S167-L4b and
   should not be trusted without re-checking; I did not chase down whether
   anyone has updated that comment. Flagging as **INFERRED** that it's
   stale, not fixed — I only confirmed the code (`Renderer.cpp:706`) reads
   it, not that the TestServer comment was ever corrected.

## 5. Threading

- **Evaluates connections today:** nobody — `ConnectionEngine::tick()` is
  never called (§1, §4). If/when it is, its own `jassert` pins it to the
  message thread, matching `SignalRegistry`/`MacroBank`/every inspector's
  `tickModulation()` — i.e. wiring it into `MainComponent::tickFeaturePipeline()`
  introduces **no new thread**, it slots into the existing 120Hz
  message-thread cadence.
- **Owns the parameters that would need writing:** the manual fields
  (`compPositionX`, `layer.opacity`, `clip.scale`, etc.) are currently
  written exclusively from the message thread (UI `onValueChanged`, OSC
  handlers explicitly commented "fire on the message thread" in
  `MainComponent.cpp`, MIDI/control-surface dispatch via
  `bindingManager_.setActionCallback(...)` → `handleBindingAction` —
  **INFERRED** message-thread for the binding path; I did not trace
  `BindingManager`'s MIDI callback origin thread to confirm it marshals
  onto the message thread before invoking the action callback, so treat
  this one link as unverified, not measured).
- **Reader side (GL thread):** `CompositorEngine`/`Renderer` render methods
  run on the GL thread (`CompositorEngine.h`: "Initialize GL resources. Call
  from `newOpenGLContextCreated()`"). Today they read the raw manual floats
  **cross-thread with no atomics and no fence** — this is a **pre-existing,
  already-shipped hazard**, not something wiring the 21 would introduce:
  `fx.paramValues[p]` and `clip.sourceParams[i].value` (written by the
  120Hz message-thread `tickModulation()`, read every GL frame) are plain
  `float`, and so are `compPositionX`/`masterOpacity`/etc.
- **Net effect of wiring the 21 via `LiveValue`:** actually a **safety
  improvement**, not a new hazard, *provided the rule is followed
  correctly*: `LiveValue::v` is `std::atomic<float>`, so `eff()` reads on
  the GL thread would be race-free for the driven case. The **hazard to
  flag explicitly** for whoever builds this: `eff()`'s fallback path still
  reads `manualRef()` — a plain, non-atomic float — for the *undriven*
  case, so the pre-existing cross-thread hazard survives for any of the 21
  that stay disconnected; and nobody must ever call `ConnectionEngine::tick()`
  itself from the GL thread (its own jassert catches this in debug builds
  only — a release build would silently race `SignalRegistry`/`MacroBank`
  reads and `manualRef()` writes-for-grip-tracking inside `tickScalars`).

## 6. What would break — double-writer hazards

Confirmed **real, pre-existing double-writers** on at least two of the 21's
backing fields, independent of anything this lane would add:

- **`Composition::masterOpacity`** — written by (a)
  `CompositionInspector.cpp`'s `masterControl_.onValueChanged` (`if
  (composition_) composition_->masterOpacity = val;`), (b) OSC
  `oscHandler_.onSetMaster` (`MainComponent.cpp:1838`,
  `composition_.masterOpacity = level;`), (c) MIDI/control-surface dispatch
  `Binding::Action::MasterOpacity` (`MainComponent.cpp:5881`,
  `composition_.masterOpacity = value;`). **None of (b)/(c) call
  `gripHeld()`/`gripTouch()`.** Once a Signal is connected to Composition ▸
  Opacity, `ConnectionEngine::evaluate()` — ungripped — would silently
  out-race whatever OSC or MIDI just wrote on the very next 120Hz tick,
  defeating Ruling A's "latest input wins" promise for those two input
  paths specifically. This is a concrete co-requisite, not just a caveat:
  landing the connection for Master Opacity without also teaching
  `onSetMaster`/`Binding::Action::MasterOpacity` to call `gripTouch(now)`
  reintroduces exactly the silent-write-loses bug class the grip system
  exists to prevent.
- **`Layer::opacity`** — already double-written today by **two manual UI
  widgets in the same inspector**: `LayerInspector.cpp`'s `masterControl_
  .onValueChanged` AND `opacityControl_.onValueChanged` both do `layer_->
  opacity = val;` (confirmed at both assignment sites in
  `src/ui/LayerInspector.cpp`), plus a third writer, OSC's
  `onSetLayerOpacity` (`MainComponent.cpp`, `layer->opacity = opacity;`).
  Same ungripped-OSC hazard as above once a connection is added.
- **Undo commands:** checked `src/core/ClipCommands.h`,
  `src/core/EffectCommands.h`, `src/core/CompositeCommand.h` for writes to
  `positionX`/`clipOpacity`/`scale`/`rotation` — **zero hits (MEASURED
  negative)**. No undo-command double-writer risk found for the 21 today.
- **Autopilot:** checked `src/model/Autopilot.cpp` for the same field
  names — **zero hits (MEASURED negative)**. Autopilot in this repo drives
  clip triggering/advancing, not these transform/opacity scalars.
- **Preset load:** `Composition::fromVar`/`Clip::fromVar`/`Layer::fromVar`
  do overwrite these fields, but this is already accounted for by design —
  `ConnSerialization::fromVar`'s doc comment states preset load is one of
  the two places "undo/redo and preset load clear all grips" is honored
  (`src/connect/ConnSerialization.h`). Not a new hazard.

## 7. Sequencing

Cheapest-and-safest first, given the above:

1. **Wire `ConnectionEngine::tick()` into `MainComponent::tickFeaturePipeline()`**
   with a real `Context` (nullptr `MacroBank`/whatever bank the owner's
   multi-bank amendment settles on can wait — start by passing
   `globalMacroBank_`). This alone changes nothing visible (no reader yet)
   but proves the engine runs against the live `Composition` at 120Hz
   without crashing/asserting — lowest-risk, purely additive, easiest to
   verify (log/print `comp.scalarLive[i]` values, or a debugger watch).
2. **Repoint one render read to `eff()`** — pick the cheapest single field.
   `Composition::masterOpacity` in `Renderer.cpp:706` is the best
   candidate: it's a single scalar read in one already-isolated block
   (`if (composition_ != nullptr) { float masterOpacityVal =
   composition_->masterOpacity; ... }` → change to
   `composition_->eff(CompScalar::Opacity)`), it's GL-thread-safe once fed
   by `LiveValue` (atomic), and it has zero of the double-writer
   complications that Layer::opacity has (only masterOpacity's own three
   writers, not layer's four).
3. **Wire registration for exactly that one control**
   (`CompositionInspector.h`'s `opacityControl_`): add its `onSourceChanged`
   handler to write into `composition_->scalarConns[CompScalar::Opacity]`.
4. **Add `gripTouch()` calls to `onSetMaster` and `Binding::Action::
   MasterOpacity`** so the double-writer hazard in §6 doesn't reappear the
   moment this one control goes live.
5. Only after that one is proven end-to-end, repeat the same
   registration+read-repoint pattern for the remaining 20, grouped by
   struct (Clip scalars, then Layer scalars, then the rest of Composition),
   since each group shares one `manualRef()`/`eff()` pair and one
   `tickScalars<>` instantiation already in `ConnectionEngine.cpp`.

**Best first demo:** Composition ▸ Master Opacity connected to an Oscillator
or a Signal. It is a single float, already has exactly one GL read site to
repoint (`Renderer.cpp:706`), already dims the entire output (so success is
visually unambiguous — the screen visibly pulses/dims), and is the one
field with byline-level design attention already spent on it (S167-L4b's
"UNCONDITIONAL... not an alpha-channel bake" comment shows this exact
read site was recently hardened, so it's a well-understood insertion point,
not a cold read).

## What I could not establish and why

- **MIDI dispatch thread for `Binding::Action::*`** — I traced
  `bindingManager_.setActionCallback` → `handleBindingAction`
  (`MainComponent.cpp:1741-1744`) but did not open `BindingManager`'s MIDI
  input callback itself to confirm it marshals onto the message thread
  before invoking the action callback (the way `OscHandler` is explicitly
  commented to do via `MessageLoopCallback`). Flagged as INFERRED, not
  measured, in §5 and §6 — worth a direct check before relying on
  "everything that writes these fields is message-thread-confined."
  Consequences: if it turned out NOT to be marshaled, that field would
  already have a raw cross-thread write hazard today (independent of this
  lane's work) — not just a grip-bypass problem.
- **Whether `TestServer.cpp`'s "render-dead scalars" comment
  (`TestServer.cpp:1025`) has been corrected anywhere else** — I found the
  contradicting, newer evidence in `Renderer.cpp:706` (S167-L4b) but did
  not do a full search for every place that comment's claim might be
  echoed (e.g. spec docs under `.harmony/specs/`). Scoped out as outside
  "the connect surface" per the dispatch's ask; flagged in §4 instead of
  silently trusting either source.
- **`compOpacity`'s current field status** — `ScalarParams.h`'s comment
  says an owner amendment merged it into `masterOpacity` so it is "not a
  separately-connectable scalar," and `TestServer.cpp` still treats
  `compOpacity` as a live settable field in its oracle endpoint. I did not
  check whether `compOpacity` still exists as a `Composition` struct member
  at all today (i.e., whether the amendment removed the field or just
  removed its `CompScalar` entry) — irrelevant to the 21/22 connect count
  either way, so not chased further.
