# Recon: the universal connect mechanism (s166)

Read-only recon. Repo: `/Users/boriskarpman/projects/RealTimeAudio`. Scope: the
connect-and-apply mechanism (a separate agent owns tempo/timing). Every claim
below is re-derived from source with file:line citations; `.harmony/` docs
were used only as leads.

## 1. The mechanism as it exists — VERIFIED, two shapes

Two independent grep shapes confirm the compute-and-apply machinery lives in
exactly two places:

- Shape 1 (definition sites): `grep -rn "void .*::tickModulation"` finds
  `EffectStackView::tickModulation()` (`src/ui/EffectStackView.cpp:137`) and
  `ClipInspector::tickModulation()` (`src/ui/ClipInspector.cpp:822`).
  `CompositionInspector::tickModulation()` (`CompositionInspector.cpp:436`)
  and `LayerInspector::tickModulation()` (`LayerInspector.cpp:773`) also
  exist but are pure pass-throughs — each calls only
  `effectStackView_.tickModulation()` (`CompositionInspector.cpp:441`,
  `LayerInspector.cpp:775`) and adds no compute logic of its own.
- Shape 2 (call-site loop bodies, different AST shape than a function def):
  `grep -n "SourceMode::Signal\|SourceMode::Macro"` inside `.cpp` bodies
  finds exactly two independent `for` loops that read `pc.isConnected()` /
  `pc.getSourceMode()` and write a computed value into a model field:
  `EffectStackView.cpp:141-234` and `ClipInspector.cpp:834-916`.

So the "two places" claim from the prior session holds structurally — but
see §6: a third, GL-thread mechanism (`RoutingEngine`) exists in parallel and
was not part of that count.

**End-to-end, for the two live loops:**
1. UI: `UniversalParamControl` (`src/ui/UniversalParamControl.h/.cpp`) is a
   `juce::Component` — message thread. Clicking its triangle
   (`mouseDown`, `.cpp:257-263`) opens a `PopupMenu`
   (`showSourcePickerAtTriangle`, `.cpp:311`) built by
   `buildSourcePickerMenu` (`.cpp:341-458`). Picking an item sets
   `sourceMode_`/`sourceName_` (`handleSourcePickerResult`, `.cpp:460-548`)
   and fires `onSourceChanged`. **The control itself never computes or
   applies anything** — it is a pure picker + manual-slider widget. All of
   `paramValues_`/range/invert live in it but there's no evaluation logic.
2. Compute+apply: `EffectStackView::tickModulation()` iterates
   `row.paramControls`, and for each connected one resolves the source by
   linear-scanning `SignalRegistry` by name (`.cpp:166-176`) or `MacroBank`
   by parsed index (`.cpp:177-190`), then does an **unconditional
   render-critical write** `fx.paramValues[p] = signalValue`
   (`.cpp:197`) — comment explicitly says this must never be gated.
   Display update (`pc.setSourceValue`) and the `onParamChanged` UI callback
   are separately epsilon-gated against `lastPushedValue` (`.cpp:210-231`)
   for repaint-cost reasons only. `ClipInspector::tickModulation()`
   (`.cpp:822-916`) is the same shape for `clip_->sourceParams[i].value`,
   additionally firing `onSourceParamsChanged` which feeds
   `Renderer::updateActiveSourceParams` (`ClipInspector.cpp:895-896`,
   comment names it explicitly).
3. Driver: `MainComponent::tickFeaturePipeline()` (`MainComponent.cpp:3016`)
   runs on a 120Hz **message-thread** `juce::Timer`
   (`MainComponent.h:265`, `timerCallback() { owner_.tickFeaturePipeline(); }`).
   It calls `globalMacroBank_.updateValues(signalRegistry_)` FIRST
   (`MainComponent.cpp:3029`), then `inspectorPanel_->tickModulation()`
   (`.cpp:3030`), which fans out to all three Inspectors
   (`InspectorPanel.cpp:175-177`) unconditionally — not gated on which
   Inspector tab is visible (that was the bug this L9 session fixed).
4. Consumption: the render/GL thread (`Renderer::renderOpenGL`,
   `Renderer.h:72`) reads `Clip::EffectSlot::paramValues` and
   `Clip::sourceParams` directly out of the model — e.g.
   `CompositorEngine.cpp:278-279, 372-376, 722, 828, 892, 1072, 1338-1341`.
   **This is a plain `std::vector<float>` written on the message thread and
   read every GL frame with no synchronization** — the open race named in
   `.harmony/HANDOFF.md:1845` and confirmed here independently by citation.

## 2. The signal/source side

`Signal` (`src/signal/Signal.h:9-54`) is the base class. Its `Type` enum has
only **three** members: `Audio, Oscillator, Envelope` (`Signal.h:12-17`) —
comment says "Called once per render frame," but in practice both the GL
thread (`Renderer.cpp:231`, inside `renderOpenGL`) and the **message
thread** (`SignalBar::timerCallback`, `SignalBar.cpp:117`, a `juce::Timer`)
independently call `SignalRegistry::evaluateAll()` on the *same shared*
`SignalRegistry&` (`SignalBar.h:46` is a reference, not an owned instance).
Both writers land in the same `std::atomic<float>` slots
(`SignalRegistry.h:50`) — safe from torn reads/UB, but a genuine **dual
authoritative-writer** situation: the UI meter and the render path can each
be evaluating the same signal against a different `FeatureSnapshot` in the
same tick, so which write "wins" for a given atomic slot is
non-deterministic between the two callers. Concrete subclasses:
`AudioSignal` (wraps a `FeatureSnapshot` field via `MappingSource`),
`OscillatorSignal` (BPM-locked LFO, `Type::Oscillator`), `EnvelopeSignal`
(BPM-locked curve, `Type::Envelope`), and `ClipPositionSignal`
(`Type::Audio`, despite the name — `ClipPositionSignal.h:12`).

`SignalRegistry::initDefaults()` (`SignalRegistry.cpp:6-85`) registers 12
visible + ~19 hidden `AudioSignal`s, 2 modulation signals (`Mod 1`
oscillator, `Mod 2` envelope), and one hidden `ClipPositionSignal`. A source
gets its current value via `getCachedValue(id)` — a linear scan + atomic
load (`SignalRegistry.cpp:173-181`); callers themselves also linear-scan
`getSignalAt(i)` by name to find the id first (`EffectStackView.cpp:166-176`,
`ClipInspector.cpp:848-857`) — O(n) per connected param per tick, small n
today but not indexed.

**Dead sub-mechanism found:** `ClipPositionSignal::setCurrentPosition()` and
`::updateFromClip()` (`ClipPositionSignal.h:20-41`) and
`SignalRegistry::getClipPositionSignal()` (`SignalRegistry.h:45`,
`.cpp:183-190`) have **zero callers anywhere in `src/`** (verified by grep
excluding the signal's own file). The registered "Clip Position" audio
signal is permanently frozen at `0.0f` — nothing ever updates it, despite
existing UI (`Audio` submenu) that would let a user pick it.

## 3. UniversalParamControl — verified 21-field claim

`UniversalParamControl::SourceMode` has **8** values (`.h:61-70`): `Manual,
Signal, BPMSync, Oscillator, Envelope, ClipPosition, Timeline, Macro`. Only
4 are ever consulted by either tick loop (`Signal`, `Oscillator`, `Envelope`,
`Macro` — `EffectStackView.cpp:161-190`, `ClipInspector.cpp:844-868`).
**`BPMSync`, `ClipPosition`, and `Timeline` are set/read only inside
`UniversalParamControl.cpp` itself** (verified: `grep -rn
"SourceMode::BPMSync\|SourceMode::Timeline\|SourceMode::ClipPosition"
src/` returns only `UniversalParamControl.cpp` hits). A user can select "BPM
Sync → Sine → 1 Beat" or "Clip Position" or "Timeline" on **any** param,
including ones inside the two working loops, and it will silently do
nothing (`found` stays `false`, value freezes) forever. This is a second,
independent gap from the 21-field one below, and it affects the *working*
mechanism too.

**21-field claim — verified independently by enumeration**, not inherited:
`UniversalParamControl` instances that get `setSignalRegistry()` (so they
render the full connect UI) but are **never read** by any `tickModulation()`:

- `ClipInspector.h:141,153-157` — 6: `clipOpacityControl_`, `posXControl_`,
  `posYControl_`, `scaleControl_`, `rotationControl_`, `anchorControl_`.
- `CompositionInspector.h:98-109` — 8: `masterControl_`, `speedControl_`,
  `opacityControl_`, `posXControl_`, `posYControl_`, `scaleControl_`,
  `rotationControl_`, `anchorControl_`.
- `LayerInspector.h:93,99,122-126` — 7: `masterControl_`, `opacityControl_`,
  `posXControl_`, `posYControl_`, `scaleControl_`, `rotationControl_`,
  `anchorControl_`.

6 + 8 + 7 = **21**, confirmed. `ClipInspector::tickModulation()`
(`.cpp:822-916`) touches only `effectStackView_` and `clip_->sourceParams` —
none of the 6 above. `CompositionInspector::tickModulation()`
(`.cpp:436-442`) and `LayerInspector::tickModulation()` (`.cpp:773-776`)
touch only `effectStackView_` — none of the 15 above. Each of the 21 has a
working **manual** path (`onValueChanged` writes the model field directly,
e.g. `ClipInspector.cpp:404-408`) — only the signal/macro-driven compute path
is missing.

Connecting one of these 21 today does something visible only in the sense
that the triangle turns cyan and (if expanded) the mini-meter animates
(`UniversalParamControl.cpp:144-165`, driven by `setSourceValue` — but
nothing ever calls `setSourceValue` on these 21, so even the meter is dead;
`setSourceValue` is only called from inside the two tick loops on the
controls *they* own).

## 4. The gap, per family — target fields the renderer already reads

For 18 of the 21 (Position/Scale/Rotation/Anchor across all three scopes,
plus Layer's two opacity-shaped controls), the renderer-side target already
exists and is live — only the compute-and-write step is missing:

| Family | Scope | Manual write target (already wired) | Renderer read site |
|---|---|---|---|
| Position X/Y | Clip | `clip_->positionX/positionY` (`ClipInspector.cpp:404-405`) | `CompositorEngine::applyClipTransform`, `clip.positionX/positionY` (`CompositorEngine.cpp:442-443,467-468`), called from the live composite path `CompositorEngine.cpp:741` |
| Scale | Clip | `clip_->scale` (`ClipInspector.cpp:406`) | `CompositorEngine.cpp:444,472` |
| Rotation | Clip | `clip_->rotation` (`ClipInspector.cpp:407`) | `CompositorEngine.cpp:445,474` |
| Anchor | Clip | `clip_->anchorX` (`ClipInspector.cpp:408`; anchorY never written by UI) | `CompositorEngine.cpp:470` (`anchorX` **and** `anchorY`) |
| Opacity | Clip | `clip_->clipOpacity` (`ClipInspector.cpp:336`) | **NO CONSUMER FOUND.** `grep -rn clipOpacity src/` outside `ClipInspector`/`Clip.h`/`Clip.cpp` hits only `MainComponent.cpp:5655,5691` (MIDI-learn writers). Nothing in `render/` reads it. This field is dead at render time regardless of signal wiring — the manual slider itself is a no-op today. |
| Position X/Y | Layer | `layer_->positionX/positionY` (`LayerInspector.cpp:399-400`) | `CompositorEngine::applyLayerTransform`, `.cpp:489-490,513` |
| Scale | Layer | `layer_->layerScale` (`.cpp:401`) | `.cpp:491,517` |
| Rotation | Layer | `layer_->layerRotation` (`.cpp:402`) | `.cpp:491,519` |
| Anchor | Layer | `layer_->layerAnchorX` (`.cpp:403`; anchorY never written) | `.cpp:515` (`layerAnchorX`, `layerAnchorY`) |
| Opacity | Layer | `layer_->opacity` — **both** `masterControl_` (`.cpp:148`) and `opacityControl_` (`.cpp:185`) write the *same* field | `CompositorEngine.cpp:542,573,577,786,805,964` — heavily used, real render input |
| Position/Scale/Rotation/Anchor | Composition | `composition_->compPositionX/compPositionY/compScale/compRotation/compAnchorX(+Y)` (`CompositionInspector.cpp:173-179`) | `Renderer::applyCompTransform`, `Renderer.cpp:1913-1922`, called live from `Renderer.cpp:512` |
| Opacity | Composition | `composition_->compOpacity` (`.cpp:132`) | **NO CONSUMER.** `grep -rn compOpacity src/` outside `CompositionInspector.h/.cpp`/`Composition.h` = nothing. Dead at render time. |
| Master | Composition | `composition_->masterOpacity` (`.cpp:135-136`) | **NO CONSUMER.** Only writers: `CompositionInspector.cpp`, `MainComponent.cpp:1797,5816` (another manual UI path), and JSON (de)serialization in `Composition.h`. Nothing in `render/` reads it. |
| Speed | Composition | `composition_->masterSpeed` (`.cpp:143-145`) | **NO CONSUMER.** Same pattern — serialized, written from two UI paths, never read by anything that drives playback rate. |

So the gap is not uniform: 18 fields have a live renderer consumer and only
need the compute-and-write step a universal mechanism would add; **4 fields
(Clip Opacity, Composition Opacity, Composition Master, Composition Speed)
have no renderer consumer at all today** — connecting them to anything,
even manually, would currently be inert. A universal mechanism spec should
flag these 4 back to the owner rather than silently building modulation
plumbing for fields nothing reads.

## 5. Threading reality

- **Message thread**: all `UniversalParamControl` UI, all `tickModulation()`
  bodies (driven by `MainComponent`'s 120Hz `juce::Timer`,
  `MainComponent.h:265`), `MacroBank::updateValues`/`getMacroValue`
  (writer at `MainComponent.cpp:3029`, only readers are the two tick loops —
  no GL-thread reader of `MacroBank` was found, so it is not currently
  cross-thread-raced).
- **GL/render thread** (`Renderer::renderOpenGL`, `Renderer.h:72`):
  `SignalRegistry::evaluateAll()` (`Renderer.cpp:231`),
  `RoutingEngine::processFrame()` (`Renderer.cpp:236`), all of
  `CompositorEngine`'s transform/effect application, and `applyCompTransform`
  (`Renderer.cpp:1913`).
- **Both threads**: `SignalRegistry::evaluateAll()` is also called from the
  message-thread `SignalBar::timerCallback` (`SignalBar.cpp:117`) on the
  *same shared registry* (§2) — a dual-writer pattern that predates any
  universal-mechanism work and should be resolved (single authoritative
  evaluator) before adding more consumers.
- **Known-open, reconfirmed here**: `Clip::EffectSlot::paramValues` and
  `Clip::sourceParams` are `std::vector<float>` written unconditionally
  every tick on the message thread (`EffectStackView.cpp:197`,
  `ClipInspector.cpp:877`) and read every GL frame with no synchronization
  (`CompositorEngine.cpp:278-279,372-376,722,828,892,1072,1338-1341`).
  `SignalRegistry::cachedValues_` already solved this exact shape with
  per-element `std::atomic<float>` (`SignalRegistry.h:50`,
  `static_assert` at `SignalRegistry.cpp:3-4`) — **that is the template a
  universal mechanism should reuse** for whatever new per-parameter value
  store it introduces (the 18 live transform/opacity fields above are
  scalar `float` members on `Clip`/`Layer`/`Composition`, not vectors, so
  the fix shape is "make the specific field atomic or route writes through
  a single GL-thread-owned apply step," not a direct copy of the
  vector-of-atomics pattern).
- A universal mechanism's contract should be: **compute on the message
  thread at a fixed cadence (proven pattern: the 120Hz timer), write
  through atomics (or an atomic-swap snapshot) into whatever the render
  thread reads, never let the render thread block on message-thread work.**
  This repo already has the correct proof-of-concept (`SignalRegistry`) and
  the counter-example of what happens without it (`paramValues`/
  `sourceParams`).

## 6. What generalises and what does not — and a third system found

The `UniversalParamControl` + `tickModulation()` shape is **more of a
new-subsystem case than a lift-and-shift refactor**, for three reasons:

1. **It doesn't reach non-effect fields at all yet.** Its target write is
   always `fx.paramValues[p]` or `clip_->sourceParams[i].value` — index-into-
   a-vector. The 18 live transform/opacity fields are named scalar struct
   members (`clip_->positionX`, `layer_->layerScale`, ...), not vector
   slots. Lifting the mechanism means generalizing the *target* side from
   "vector index" to "arbitrary field setter" — a real abstraction change,
   not a copy-paste.
2. **Each tick loop hand-resolves its source by linear-scanning
   `SignalRegistry`/`MacroBank` by string name every tick** — there is no
   shared "resolve source → value" helper; `EffectStackView.cpp:161-190`
   and `ClipInspector.cpp:844-868` are near-duplicate code, already crying
   out for extraction even before generalizing further.
3. **A third mechanism already exists and is more general on the source
   side, but production-dead**: `RoutingEngine`/`Route`
   (`src/routing/RoutingEngine.h/.cpp`, `src/routing/Route.h`). It runs
   live every GL frame — `routingEngine_.processFrame(*signalRegistry_,
   writer)` at `Renderer.cpp:236`, inside `renderOpenGL()` (correct thread!)
   — and its `Route` struct already has a **richer** shaping pipeline than
   `tickModulation()`'s raw passthrough: dial-range, threshold, gain,
   falloff, per-route `Smoother`, invert, output range
   (`Route.h:26-40`, `RoutingEngine.cpp:77-126`). Its `RouteTarget`
   (`Route.h:45-60`) already generalizes addressing to
   scope+layerId+clipId+effectIndex+paramIndex. **But `addRoute()` has
   exactly one caller in the entire codebase: `TestServer.cpp:828`** — no
   production UI, no `ApiServer` endpoint (`ApiServer` only holds a
   reference, never calls `addRoute`), nothing creates a `Route` in the
   shipped app. It processes an always-empty vector every frame in
   production. This is the "three systems turned out to be six" trap this
   repo has burned agents on before — I did not find a fourth/fifth/sixth,
   but budget was limited; a follow-up should specifically grep for
   `EffectChain`/`MappingEngine` (`previewPanel_.getMappingEngine()`,
   `MainComponent.cpp:3019`, `Renderer.h`) since that is a **fourth**,
   still-live, legacy v1 system operating on `previewPanel_`'s own
   `EffectChain` — separate from the Clip/Layer/Composition `EffectSlot`
   vectors — that I did not have time to fully characterize.

**Verdict**: this is architecturally closer to **replace-and-consolidate
than extend**. A universal mechanism should not extend
`UniversalParamControl`'s tick-loop pattern to 21 more fields as-is (that
would triple the near-duplicate linear-scan code and still leave 3 dead
`SourceMode`s and 4 dead render targets in place); it should decide whether
to (a) resurrect and generalize `RoutingEngine` — thread-correct today,
richer shaping, already has scope+field addressing — as the single backend
for *all* connectable parameters including the 21, with `tickModulation()`
retired in its favor, or (b) build a new generalized target-setter
abstraction and keep `RoutingEngine` dead/removed. Either way, the four
render-dead fields (Clip Opacity, Composition Opacity/Master/Speed) and the
three dead `SourceMode`s (BPMSync, ClipPosition, Timeline) are decisions for
the owner, not implementation details — they determine how much of "every
parameter, same exact method" is UI-only theater today.
