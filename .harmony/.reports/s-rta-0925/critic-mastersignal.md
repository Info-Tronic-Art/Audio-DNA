# Critic — Master Signal design (s-rta-0925)
STATUS: DONE
Critic: Reviewer agent, 2026-09-25. Adversarial pass over diag-mastersignal.md against
RealTimeAudio HEAD. ~35 of the doc's `path:line` citations spot-checked by direct read/grep;
every one checked out exactly or within 1-2 lines of drift (files have moved slightly since the
diagnosis was written but no cited claim was FALSE). This is an unusually well-verified document.

VERDICT: APPROVE THE DESIGN, with one BLOCKING spec gap (ordering) and one MEDIUM unaddressed
signal-path bug (macro-chain double-depth) that must be resolved before or during Step 2, plus
minor citation drift noted for the builder.

---

## Direct answer to the dispatch's three hunts

**1. Beat-detection degradation from the gain recommendation: NO — because the design correctly
REJECTS reusing the Gain slider.** Verified: `CombinedCallback.h` gain path only fires under
`useInputForAnalysis` (mic mode, line 54), multiplies the RT buffer at line 79, and that gained
buffer is what feeds `analysisCallback_` (line 109) AND `AudioTap` (line 129, exact match to the
doc's citation). File mode (`else` branch, lines 91-101) never reads `inputGain` at all —
confirmed, no read anywhere in that branch. The design leaves this callback untouched and adds a
new POST-analysis depth instead (`ConnectionEngine::evaluate`, GL-thread `scaleSignalLevels` on a
snapshot COPY). Since the new mechanism never touches the audio callback, the analysis thread, or
the FeatureBus write side, it cannot affect BPM/onset detection or the recorded take. Verdict on
this specific risk: the design is safe by construction, not just by claim.

**2. RT-safety holes: none found.** `applyDepth`/`scaleSignalLevels` are flops on PODs/atomics,
no allocation, no mutex — matches the doc's §6 claim. Traced the actual mutation site: `Renderer.
cpp:229-233` binds `snap` as a REFERENCE to `frameSnap_` (not a copy), so scaling `frameSnap_` in
place at line 233 is seen by every later reader of `snap` in the same function by construction —
including the doc's proposed insertion point (before line 266's `autopilot_.processFrame`) and
the SECOND, independent `const FeatureSnapshot& snap = frameSnap_;` inside `renderSource()`
(`Renderer.cpp:1126`, confirmed) that every `ProceduralSource::uploadUniforms` call goes through.
One mutation point genuinely does cover every uploader this frame, as claimed. `CompositorEngine::
setLatestSnapshot` (`Renderer.cpp:495`) is a COPY (`latestSnapshot_ = snap`, `CompositorEngine.h:
166`) taken AFTER line 233 in execution order, so it also sees scaled data — order-dependent but
correctly ordered in the current spec. `OutputWindow` genuinely holds no `Composition*`
(`grep OutputWindow.h` confirms only `const FeatureBus&`), so the design's proposed new
`std::atomic<float> signalDepth_` there is the only viable path, not a workaround.

**3. Signal paths it missed: none of the ones named in the dispatch (shader uniforms, autopilot,
MappingEngine vs RoutingEngine, macros) are missing from the doc's §2 table — I grepped every
FeatureSnapshot/frameSnap_/featureBus_ reader in `src/` looking for an uncovered consumer
(FeedbackProcessor, ApiServer, AudioReadoutPanel, SpectrumDisplay, TopBar) and found nothing new:
FeedbackProcessor never touches FeatureSnapshot at all; the API/UI readers are all raw-meter
consumers, consistent with the doc's explicit "meters stay raw" decision (§3.2, SignalRegistry
row). Autopilot.cpp reads only `beatPhase`, `beatInBar`, `barCount`, `structuralState`,
`energyState` (confirmed by grep) — all five are on the doc's own "untouched" list. RoutingEngine
dead-code claim is a verbatim match to a comment IN Renderer.cpp itself (`Renderer.cpp:247-254`).
BUT there is one real signal-path gap the doc does not analyze — see Finding 2 below.**

---

## BLOCKING — Finding 1: Step 2's `depth` is read AFTER two of its three consumers in the same tick

The doc's Step 2 spec (§5) says `ctx.signalDepth = composition_.eff(CompScalar::Signal)` is
computed "near `MainComponent.cpp:3374-3378`" (where `ConnectionEngine::Context ctx{...}` is
built) and that this SAME `depth` value should also feed `globalMacroBank_.updateValues(
signalRegistry_, depth)` (cited at `:3344`) and `mappingEngine.processFrame(snap, chain, depth)`
(cited at `:3334`).

I read the live function (`MainComponent::tickFeaturePipeline`, `src/MainComponent.cpp:3323+`)
end to end. The actual execution order is:
1. `signalRegistry_.evaluateAll(snap)`
2. `previewPanel_.getMappingEngine().processFrame(snap, ...)` — **v1 MappingEngine, needs `depth`**
3. `globalMacroBank_.updateValues(signalRegistry_)` — **needs `depth`**
4. `recorderHost_.tick(...)`
5. `ConnectionEngine::Context ctx{...}` construction — **this is where the doc says to READ `depth`**
6. `connectionEngine_.tick(composition_, ctx)`
7. `inspectorPanel_->tickModulation()`

Steps 2 and 3 run BEFORE step 5 in the current, unmodified control flow. The doc's own line
citations (`:3334`, `:3344` vs `:3374-3378`) already encode this ordering — it is not a citation
error, it is a genuine sequencing gap in the spec: as written, a builder following the citations
literally cannot pass `depth` into steps 2-3 because the variable does not exist yet at that
point in the function. This is exactly the class of gap the diagnosis itself calls out for
`onSourceParamsPublished` firing order in Step 0 (§5, item 4) — the same rigor needs to apply
here. Fix is one line (`const float depth = composition_.eff(CompScalar::Signal);` hoisted above
step 2, reused at 2/3/5), but it is not written down, and getting it wrong silently produces a
Master Signal fader that visibly dampens `ConnectionEngine`-driven controls (opacity, transform,
speed) while leaving v1-mapping-driven effects and macro-sourced knobs unaffected — a
"half-works" bug that would read to Boris as "some sliders ignore the fader," not as a crash, so
it survives a casual smoke test. Recommend the spec be amended with the explicit hoist before
Step 2 is built.

## MEDIUM — Finding 2: macro-mediated chains get depth applied TWICE (compounding, non-uniform response)

Not discussed anywhere in §3.2 (per-path table), §4 (tradeoffs), or the RISKS section, and it
sits squarely in "macros" from the dispatch's hunt list.

Traced `ConnectionEngine::tick` (`src/connect/ConnectionEngine.cpp`, current lines ~275-286): the
macro loop runs `evaluate(macro.conn, macro.manualValue, ctx, nullptr)` — the SAME `evaluate()`
that Step 2 amends to apply `applyDepth(manualNorm, y, ctx.signalDepth)` before its `return y`
(line 177). So a macro whose OWN connection source is `Kind::Signal` gets depth-scaled once here,
producing `macro.currentValue = applyDepth(macroManual, signalDriven, d)`.

Separately, any scalar/effect/source param connected with `Kind::Macro` (`ConnectionEngine.cpp`
switch case `ConnSource::Kind::Macro`, confirmed: `raw = ctx.macros.getMacro(idx).currentValue`)
feeds that ALREADY-scaled `macro.currentValue` through shaping/smoothing as `raw`, and then that
SAME `evaluate()` call applies `applyDepth(paramManual, shapedMacroValue, d)` a SECOND time before
returning.

Boundary cases are fine: at d=1 both `applyDepth` calls are identity (`>= 1.0f return driven`), so
the chain is bit-identical to today (the "1.0 = today" invariant holds). At d=0 both collapse to
`return manual` regardless of the upstream value, so the final control lands exactly on ITS OWN
manual value — also fine, matching the "0 = static at hand-set value" invariant.

The break is at intermediate depth (the fader's entire useful range, e.g. d=0.5): the final
control's response is `paramManual + d*(shape(macroManual + d*(signal-macroManual)) -
paramManual)` — quadratic-ish in `d` for the signal contribution — versus a control connected
DIRECTLY to the same signal, which gets `paramManual + d*(shape(signal) - paramManual)` — linear
in `d`. A rig where several effect params fan out from one Macro (the doc's own listed use case,
"a Macro → Signal lets an APC fader drive it," §3.2) will visibly react LESS than a directly-wired
param at the same fader position, and the discrepancy grows as more chain depth is added (a Macro
driven by an LFO that is itself... no further chaining exists today, so this caps at one hop, but
one hop is already the common VJ pattern the design explicitly supports).

This does not break the "0%/100%" promise, so it is not launch-blocking, but it does contradict
the tooltip's plain-language claim ("how strongly audio... move the controls they are connected
to" implies uniform strength regardless of routing topology) and will be visible to Boris as soon
as he wires one macro to several params. Two ways to fix, either acceptable: (a) explicitly force
`d=1` inside the macro-tick loop (mirroring the doc's own self-exemption pattern for
`CompScalar::Signal`'s own connection) so depth is applied exactly once, at the FINAL consumer —
this matches user intuition best; or (b) document the compounding as accepted behavior with a
one-line addition to §3.2's macro row. Recommend (a); it is a one-line change (force
`ctx.signalDepth = 1.0f` for the macro-tick's local context, same trick already used for the
`CompScalar::Signal` exemption) and should be added to Step 2 before it ships.

## Minor — citation drift (non-blocking, for the builder's awareness)

`CombinedCallback.h`'s gain-multiply and "analysis fed" lines have drifted ~2 lines from the
doc's cited `:76-79` / `:107-110` (actual: multiply at :79, `analysisCallback_` call at :109) —
the recorder-refactor history in this file (visible in its own header comment, "lifted out of
AudioEngine.h") likely shifted lines after the diagnosis was written. Every substantive claim
still holds; only the exact line numbers need a `grep -n` refresh before Step 0 patches land,
same as any diagnosis that predates a same-day commit.

---

## What I verified and did NOT find fault with

- Gain/CombinedCallback semantics (§1): exact, including the hidden v1 `inputGainSlider_`
  (`MainComponent.h:354`, `setVisible(false)` at 2425-2426, persisted via `deck.inputGain` at
  3605/3700-3701, `PresetManager.h:86`) — a real, independently-verifiable footgun the doc caught.
- Render-dead effect/source-param twins (§2, Step 0): `Clip.h:90 effParam()` has zero callers
  outside its own declaration (grep confirms); `CompositorEngine.cpp` uploads raw `slot.
  paramValues[p]`/`slot.dryWet` exactly as cited. This is the correct, load-bearing prerequisite —
  without Step 0, Master Signal would silently not cover most of what a user drags onto a clip.
- Source-param NaN-on-not-driven inconsistency (§5 Step 0 item 3): confirmed live — the
  `sourceParams` loop in `ConnectionEngine::tick` currently SKIPS the store on NaN (`if (!std::
  isnan(y))`) while the scalar/effect-vector tickers already always-store; the doc's fix aligns
  the third path with the other two, independently a good catch regardless of Master Signal.
- Shader uniform occurrence counts (`u_rms` 191, `u_beatPhase` 68, `u_bass` 44, `u_onsetStrength`
  40, `u_spectralCentroid` 17, `u_bandEnergies` 13, `u_chromagram` 12, `u_mfccs` 11,
  `u_onsetDetected` 8) — all reproduced exactly via `grep -c`.
- `RoutingEngine` dead claim, `OutputWindow` holding no `Composition*`, the double independent
  Master dims (`masterLevel_` vs `masterOpacity`, `Renderer.cpp:676-716`) — all confirmed
  verbatim against source comments and code.
- Right-click-reset root cause (§10.B): `ResettableSlider::mouseDown` swallowing the right-click
  when `hasDefault_` is false, and `UniversalParamControl::setDefaultValue` only propagating when
  the OWNER calls it — confirmed exactly in `UniversalParamControl.h`.
- `applyDepth`'s `>= 1` / `<= 0` float-identity guards (§3.4): correct and necessary; `m + 1*(y-m)`
  is not guaranteed bit-identical to `y` in IEEE 754, so the guard is load-bearing as claimed.

## Confidence labels
VERIFIED (disk-read/grep, this session): §1 Gain semantics, §2 render-dead twins + NaN
inconsistency, §3 depth-injection site + ordering (Finding 1), macro double-depth mechanism
(Finding 2), shader uniform counts, RoutingEngine/OutputWindow/masterLevel claims, right-click
root cause, RT-safety of the proposed mutation site.
INFERRED: exact runtime magnitude of the Finding-2 divergence at various `d` (derived
algebraically from the read code, not measured against a running build — no build was run this
pass).
NOT RE-VERIFIED (accepted on the diagnosis's own citation, out of this pass's budget): §5 Steps
4-6 UI wiring line numbers (TopBar layout, BindingManager enum-append, OSC/REST endpoint additions,
CMakeLists test target lines) — spot pattern-matched to existing precedent (`masterOpacity`
wiring) and structurally plausible, not independently re-derived line-by-line.
