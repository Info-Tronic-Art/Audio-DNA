# Recon: Timing window + Composition Inspector dead blocks — verified against source

HEAD at time of recon: current `main` working tree (no build/run performed — read-only).
Source doc: `.harmony/surface-audit-2026-08-04d.md` (dated 2026-08-04, ~1 month old).
Every claim below re-derived from source; doc claims used only as a starting pointer, never
taken on faith. Two dead-item corrections found along the way are flagged explicitly.

---

## SURFACE 1: The Timing window

### 1. Where it is
- UI: a permanent panel in the bottom row of the main window, between the preview monitor and
  the Composition/Clip/Layer/Signal inspector — always visible, not a modal or hideable window
  despite the class name. Confirmed layout math: `src/MainComponent.cpp:2273-2294` carves the
  bottom row into 4 fixed columns (preview | **timing** | inspector | browser) and unconditionally
  calls `timingWindow_->setVisible(true)` every resize.
- Source: `src/ui/TimingWindow.h` / `src/ui/TimingWindow.cpp`. Header's own doc comment
  (`TimingWindow.h:5-7`): *"center-bottom panel with 3 tabs — BPM, Routing, Oscillators... Content
  is placeholder for now; will be fleshed out as features develop."*
- What the user sees today: 3 clickable tab buttons ("BPM", "Routing", "Oscillators") that switch
  an active-tab highlight, and below them **only the tab's own name re-drawn as grey ghost text**
  (`TimingWindow.cpp:51-65`, `contentArea` gets `g.drawText(tabName, ...)` at 40% alpha — that's it).

### 2. What's behind it
Nothing. Verified two ways:
- `TimingWindow.cpp` is 99 lines total: constructor wires 3 buttons, `paint()` draws the tab bar
  and the ghost placeholder text, `resized()` lays out 3 buttons, `setActiveTab()` just repaints.
  No other method exists.
- `TimingWindow.h` declares exactly one enum (`Tab`), one state variable (`activeTab_`), 3 buttons.
  No pointer to BPM data, RoutingEngine, or SignalRegistry anywhere in the class.
Clicking a tab changes which word is ghosted. Nothing downstream reads `getActiveTab()` — grep
for `getActiveTab` outside the class returns zero hits.

### 3. What it was evidently meant to do (INFERRED)
The three tab names name real, separately-existing machinery elsewhere in the codebase that
the Timing window never touches:
- **BPM tab** — real BPM state already exists: tap/manual BPM in the top bar (confirmed WIRED
  elsewhere), a `Composition::quantizeMode` (top bar "Quantize" dropdown) that **is now live**
  — `src/ui/TopBar.cpp:167-173` writes `composition_.quantizeMode` directly, and
  `src/MainComponent.cpp:3759,3920` reads it via `quantizeModeToForcedSnap()` to force beat-snap
  on clip triggers. **This corrects the audit doc**, which said Quantize had "no consumer outside
  JSON" — that was true a month ago, false today (a `2026-09-05`-adjacent "L5 Quantize" pass wired
  it up since). By contrast the top-bar **BPM multiplier** (÷4 ÷2 ×1 ×2 ×4) is still fully dead:
  `composition_.bpmMultiplier` is written at `TopBar.cpp:300` and read *only* by JSON
  save/load (`Composition.h:179,270`) — no consumer anywhere applies the multiplier. Ableton Link
  (`LinkSync::setEnabled`) likewise has zero callers. All of these are scattered, none of them
  live inside TimingWindow's "BPM" tab today.
- **Routing tab** — a real `RoutingEngine` exists (`src/routing/RoutingEngine.h/.cpp`) and is
  **actually running every frame**: `Renderer.cpp:236` calls `routingEngine_.processFrame(...)`
  each frame and writes results into effect params (`Renderer.cpp:238-241`, Global scope only;
  Clip/Layer scope is a `// TODO`). But the only code that ever calls `addRoute()` to create a
  route is the test server (`src/test/TestServer.h`) — there is **no UI anywhere in the shipped
  app** to create, view, or delete a route. This is exactly the gap a "Routing" tab would fill.
- **Oscillators tab** — real oscillator machinery already exists and is *more* built than the
  audit doc implied: `OscillatorSignal` (`src/signal/OscillatorSignal.h`, a "BPM-locked waveform
  generator") is a live member of `SignalRegistry`, which is evaluated every frame
  (`Renderer.cpp:230-231`, `signalRegistry_->evaluateAll(snap)`). A full editing UI for it already
  ships: `SignalInspector` (`src/ui/SignalInspector.h/.cpp`, 422 lines) has a dedicated
  "Oscillator controls" section (wave shape, beat duration, amplitude, phase offset) and is wired
  live into the 4th Inspector tab (`InspectorPanel.h` "Signal" tab, `InspectorPanel.cpp:161`).
  Oscillators also get live meters in the always-visible `SignalBar`/`SignalStrip`
  (`src/ui/SignalBar.cpp`, `src/ui/SignalStrip.h/.cpp`) that a user can click to jump straight into
  that editor (`MainComponent.cpp:624`, `inspectorPanel_->inspectSignal(&signal)`).

So: BPM data exists (partly live, partly dead and scattered), a route-processing engine runs
every frame with no way to feed it, and oscillators are a fully-built signal type with a real
editor and meters. TimingWindow's job was evidently to be the "wiring/patch" surface that ties
these together (author a route: signal X → param Y), not to re-display things that already have
homes. Today it does none of that.

### 4. What it would take to make it real
- **BPM tab**: decide a single home for BPM-adjacent globals (multiplier, Link, quantize display)
  instead of leaving them duplicated across the top bar; wire `bpmMultiplier` to something that
  actually scales beat timing, wire `LinkSync::setEnabled` to a checkbox.
- **Routing tab**: the actual missing piece is small and specific — a UI to call
  `RoutingEngine::addRoute()`/`removeRoute()` (source signal picker + target param picker + curve
  params), a list view over `getRouteAt()`/`getNumRoutes()`, and — separately — implementing the
  `// TODO: Clip/Layer scope routing` in `Renderer.cpp:243` if per-clip/per-layer routing (not just
  Global) is wanted. The per-frame engine and the audio/signal side need no new plumbing.
- **Oscillators tab**: arguably needs the least *new* work — `SignalInspector` already renders
  full oscillator controls. What's missing is exposing that same editing surface (or a route
  target) from inside Timing, or else deciding Timing shouldn't duplicate it at all.

### 5. Duplication risk — real and specific
- The per-param "Oscillator" source option already exists as a menu choice on every
  `UniversalParamControl` (`UniversalParamControl.cpp:395-413,525`, a context-menu submenu listing
  live oscillators from the registry) — but selecting it is **cosmetic only**: it flips a local
  `sourceMode_` label and fires `onSourceChanged`, which **has zero external listeners anywhere in
  the codebase** (`onSourceChanged` declared `UniversalParamControl.h:92`, assigned nowhere). So
  there are already two different half-built "connect a param to something" UIs in the app
  (this per-param picker, and the not-yet-built Timing > Routing tab) aimed at the identical job.
  Building Routing as a wholly separate global panel — instead of making it the thing that
  actually fulfills what the per-param picker already promises — would leave the app with two
  competing, both-broken ways to say "modulate this slider from an oscillator."
- Timing > BPM risks duplicating the top-bar Quantize control, which (per the correction above)
  now genuinely works.

---

## SURFACE 2: Composition Inspector — the two dead blocks

Both live in `src/ui/CompositionInspector.h/.cpp`, the "Composition" tab of the 4-tab Inspector
panel (`InspectorPanel.h` `Tab::Composition`), directly below the Clip/Layer/Composition/Signal
tab bar. Section order per the file's own doc comment (`CompositionInspector.h:12-22`):
Name → Dashboard → **Autopilot** → Composition → Video → Transform → **Global Effects** → Output.

### Block A — the "Autopilot" section (Direction / Duration mode / Clip Loops / Loop / Master Layer)

**1. Where / what the user sees:** Header "Autopilot" painted at `CompositionInspector.cpp:233`.
Below it: 4 direction buttons (◀◀ rewind / OFF / ▶▶ forward / 🔀 random), a "Duration mode"
dropdown (Longest Clip / Clip Transport / Custom), a Clip Loops number stepper, a Loop toggle,
and a Master Layer dropdown (`CompositionInspector.h:79-87`).

**2. What's behind it:** All 5 controls write to real fields on the `Composition` model —
`autopilotDirection`, `autopilotDurationMode`, `autopilotClipLoops`, `autopilotLoop`,
`autopilotMasterLayer` (`CompositionInspector.cpp:24-81`, confirmed writes; `:520-529` confirmed
read-back for display sync) — and those fields round-trip through JSON save/load
(`Composition.h:204-208,307-316`). But **nothing outside this UI file and the model's own
(de)serializer ever reads any of the five fields.** Verified with two independently-shaped
searches: (a) grepping the widget member names in `CompositionInspector.cpp` finds only the UI
file; (b) grepping the five raw model field names across the *entire* repo
(`autopilotDirection|autopilotDurationMode|autopilotClipLoops|autopilotLoop|autopilotMasterLayer`)
returns only `Composition.h` (declaration + JSON) and `CompositionInspector.cpp` (widget wiring) —
zero hits in `src/model/Autopilot.cpp/.h`, `src/render/`, anywhere. The actual autopilot engine
(`Autopilot::processFrame`, `src/model/Autopilot.cpp`) never looks at any of them.

**3. What it was meant to do (INFERRED):** A Resolume-style *global override* sitting on top of
a per-clip/per-layer autopilot system that **does** work today: `Clip::autopilotAction`
(enum incl. `PlayNext`/`PlayPrevious`, `Clip.h:89-94`) and `Layer::autopilotEnabled` /
`Layer::defaultAutopilotAction` (`Layer.h:50,143`) are real, per-cell settings that
`Autopilot::getActionForClip`/`getBeatsForClip` actually resolve at runtime (`LayerDetermined`
falls back to the layer default). The composition-level block was evidently meant to let one
Direction/Duration/Loop choice drive (or override) every layer at once, plus name a "Master
Layer" whose clip length paces the whole composition's advance cycle — a real, sensible feature,
just never connected.

**4. What it would take:** `Autopilot::processFrame` (or a thin layer above it) needs to actually
consult `composition_->autopilotDirection` etc. — e.g., apply Direction/Duration/Loop as a
composition-wide default that per-layer settings can still override, and make "Master Layer"
mean something (gate every other layer's advance to the named layer's clip boundaries). This is
a logic-wiring job, not new UI or new model fields — everything needed already exists on both
ends; only the middle connection is missing.

**5. Duplication — this is the important one:** The section **directly below** this dead block,
in the same panel, is "Per-Type Autopilot" (`CompositionInspector.cpp:237` header, P20) — and it
is **fully live**: its enable toggle and three per-type (Opaque/Transparent/Effect layer) cycle
sliders and randomize toggles write to `composition_->perTypeAutopilot.*`
(`CompositionInspector.cpp:86-132`), which `Autopilot.cpp:83,121,125,127,137,146,149` genuinely
reads every frame via `Autopilot::setPerTypeConfig()` (called from `Renderer.h:109`). In other
words: the app **already has** a working, composition-wide, no-per-layer-clicking-required
autopilot control — it just lives one section down and is scoped by layer *type* rather than
being a single global Direction/Duration/Loop dial. Before rebuilding the dead block, worth
deciding whether it should be merged into Per-Type Autopilot rather than duplicate it.

### Block B — "Global Effects" (the FX stack)

**1. Where / what the user sees:** Header "Global Effects" at `CompositionInspector.cpp:262`, an
`EffectStackView` (same widget class used for Clip and Layer effect stacks — drag-drop FX in,
reorder, bypass, undo/redo all present and interactive) bound to `composition_->globalEffects`
(`CompositionInspector.cpp:390,405`).

**2. What's behind it:** A real, fully-interactive effect-stack editor (add/remove/bypass/reorder,
with its own undo command `EffectStackCmd`, `src/core/EffectCommands.h`) that edits a real vector,
`Composition::globalEffects` (`Composition.h:22`). But **no renderer ever reads it.** Verified two
ways: (a) grep for the literal field name `globalEffects` across `src/render/`, `src/audio/`,
`src/analysis/` returns zero hits; (b) the compositor's own effect-application code path is
directly comparable and *does* fire for the other two scopes — `CompositorEngine.cpp:237`
(`applyClipEffects`, reads `clip.effects`) and `CompositorEngine.cpp:756-757` ("Apply per-layer
effects... same mechanism as per-clip effects", reads `layer.layerEffects`) — there is no third
call for `globalEffects` anywhere in that file or elsewhere. The code's own inline comment
(`EffectCommands.h:82-84`) independently states the same thing, but that comment was *not* taken
as evidence — it was cross-checked against the actual render path above and holds up.

**3. What it was meant to do (INFERRED):** A whole-composition FX layer that applies on top of
everything after per-clip and per-layer effects composite — the standard three-tier FX model
(clip → layer → master/global) that VJ tools like Resolume use, where the global tier is the
"master bus" effect.

**4. What it would take:** Compositor-side work only — the UI and model are complete and already
undo-safe. `CompositorEngine`'s final composite step needs one more `applyClipEffects`-shaped call
over `composition_->globalEffects` on the fully-composited frame (before or after Syphon publish,
depending on intent), the same pattern already used for Clip and Layer. `EffectCommands.h:78-90`
already flags this exact spot and even pre-emptively adds GL fencing "for free" against the day
this gets wired up — so a real engineer already anticipated this exact fix and left the seam open.

**5. Duplication:** Not a duplicate of a *different* control — it is functionally identical UI
(same `EffectStackView` widget, same interaction model) to the Clip and Layer effect stacks that
already work. The risk isn't confusion between two different controls; it's that a user who
builds a "Global Effects" stack today, expecting master-bus behavior (because it looks and
behaves exactly like the working Clip/Layer stacks sitting right next to it in the same
inspector), gets silently nothing.

---

## Corrections to the 2026-08-04d audit doc surfaced during this recon
1. **Quantize is no longer dead.** `TopBar.cpp:167-173` + `MainComponent.cpp:3759,3920` show it
   now genuinely forces beat-snap. (BPM multiplier, listed in the same doc row, is still dead —
   confirmed, unchanged.)
2. The doc's "Effect param modulation freezes when you look away" PARTIAL item appears to have
   been addressed very recently: `InspectorPanel.h` now documents an "L9 (modulation-freeze fix,
   2026-09-05)" `tickModulation()` path that runs unconditionally for all three effect hosts
   (Clip/Layer/Composition), not gated by the active tab. Not the assigned surface for this
   recon, so not independently re-derived beyond reading the header comment and confirming
   `tickModulation()` exists and is called from `InspectorPanel::tickModulation()` /
   `CompositionInspector::tickModulation()` (`CompositionInspector.h:41-47`) — flagging for
   awareness, not asserting fully verified.
