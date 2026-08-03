# OutputWindow Arc — Council-Ratified Design (chair synthesis)
Date: 2026-08-03 · Chair: Harmony · Seats: minimal-change / correctness-architect / operational-pragmatist (blind, independently grounded at HEAD cb4d5fa)
Status: RATIFIED (chair adjudication on the merits). Implements featurebus-thread-safety-design.md R10 + scout-outputwindow-glcrash.md R1/R3/R5. Full seat papers: session transcript 2026-08-03.

## Unanimous spine
- U1 REJECT attach-state handoff ((b)): re-arms the ca0b425 write/write class at every hide/show edge; leaves the container race standing; couples EMA feel to the venue projector's refresh. Three independent groundings.
- U2 A message-thread juce::Timer is the SOLE MappingEngine::processFrame caller in all four attach states. The GL call at Renderer.cpp:~232 is deleted. Timer cadence is message-queue-delivered (juce_Timer.cpp:264) — independent of visibility/attach/occlusion/minimise.
- U3 DELETE shared EffectChain::latestSnapshot_/setLatestSnapshot. EffectChain::render()/uploadEffectUniforms gain (EffectChainGLState&, const FeatureSnapshot&) in ONE signature change; each renderer passes its OWN featureBus_.read() (post-S2 multi-reader-safe). Audio uniforms never freeze in any state; the documented cross-GL snapshot tear (EffectChain.h:~92-99) dies structurally. CompositorEngine::latestSnapshot_ unchanged (main-only consumer).
- U4 mappings_/smoothers_ msg-vs-GL container race RETIRED by message-thread confinement — no mutex, no queue. GL threads never mutate MappingEngine (see A4).
- U5 Arc gates are CPU-observable (API param probes) — NEVER Eyes pixels. The A/B-proven pre-existing Eyes effects-deadness must not gate (or be claimed fixed by) this arc.

## Chair adjudications (divergences, with reasons)
- A1 tick scope = processFrame ONLY. Routing/signal extraction into the tick is a RECORDED FOLLOW-UP gated on a SignalRegistry thread audit (prag dissent noted; residual: ROUTED params still freeze on preview detach until then). Build the tick body as a named seam (tickFeaturePipeline()) so routing/signal can join later without re-plumb. Autopilot (Renderer.cpp:~237) stays GL-attached — deck/composition surface, excluded.
- A2 EffectParam::value stays plain float — spec Step-3/P3 staging holds (2:1). The enlarged msg→GL scalar-crossing set is a KNOWN-DEFERRED residual; zero suppressions; the TSan gate bar is "no NEW finding classes beyond the documented deferred list."
- A3 SINGLE-STORE processFrame ADOPTED as mandatory co-fix (prag NEW FINDING: reset(0)→accumulate→clamp at MappingEngine.cpp:~162/200/214 publishes intermediate zeros that the output GL thread uploads live TODAY — pre-existing zero-flash). Accumulate locally; exactly ONE store per param per tick.
- A4 Renderer.cpp:~1520 GL clearAll: DELETE after verifying minimal's no-op proof at HEAD (first-attach-only guard :~1489-1490 → nothing to clear; deletion also closes a latent pre-attach preset-mapping wipe). If the proof does NOT hold: callAsync marshal instead. Either way U4 stands.
- A5 kMappingTickHz = 120 — ONE named constant, matching the MEASURED attached render rate (119.8fps, session 2026-08-02b). Smoother has no dt term (rate IS the time constant): 120 preserves incumbent feel; 60 would double the smoothing constant. Alpha untouched. JUCE ms rounding (→8ms ≈125Hz) accepted — EMA parity gate asserts empirically. Boris rig-feel taste-check → only-Boris list.
- A6 jassert(juce::MessageManager::getInstance()->isThisTheMessageThread()) in processFrame/addMapping/removeMapping/clearAll (Debug-only; would have caught :1520).

## Requirements
- W1 EffectChainGLState per-renderer: uniformLocationCache_ + prevFrameTexture_/prevFrameFBO_/prevFrameWidth_/prevFrameHeight_ move OUT of EffectChain into a state object owned by Renderer and by OutputRenderer, passed by reference. Kills scout R1 (unordered_map UB + cross-context program-ID location poison) and R3 (cross-context prevFrame churn/deletes). Cache key may drop the programID prefix ONLY if builder verifies per-context uniqueness; else keep key — per-context state already closes the poison.
- W2 Signature change per U3; call sites Renderer.cpp:~483 and OutputWindow.cpp:~143 pass own state + own read(). latestSnapshot_/setLatestSnapshot deleted; grep-zero after.
- W3 R5 reorder: `if (outputWindow_) outputWindow_->getRenderer().detach();` lands immediately beside the existing shutdown-law detach block (MainComponent.cpp:~1778-1780); outputWindow_.reset() stays put (detach is idempotent).
- W4 Single-store processFrame per A3.
- W5 MappingTick per U2/A5/A6: timer member (MainComponent or small owned helper), startTimerHz(kMappingTickHz), body = seam tickFeaturePipeline() = { snap = bus.read(); mappingEngine_.processFrame(snap, effectChain_); }. Delete the Renderer.cpp:~232 call; apply A4; timer runs UNCONDITIONALLY (no attach gating).
- W6 Test enabler: test-mode-only mapping route (add/remove RMS→param) following the S2 inject_features precedent EXACTLY — TestServer 8080 (+7070 relay optional), registration-gated, NOT registered in production, production probe must 404.
- W7 Tests (R9 fail-first doctrine):
  (i) TSan unit case modeling the confinement change (processFrame loop vs param-load loop vs add/remove loop) — demonstrated producing findings against HEAD topology first, clean after; zero suppressions.
  (ii) NEW pytest 4-state param-tracking probe: mapped param tracks injected rms flips ≤500ms in {preview visible · SignalBar-collapsed · collapsed+output-open}; MUST FAIL in states 2-3 at the pre-W5 stage (capture run → .harmony/ow-freeze-before.log) and PASS after W5.
  (iii) EMA parity: attached-state step-response time constant within tolerance of pre-change measurement.
  (iv) During the TSan app gate: log getProgramID() from BOTH renderers once — confirms/refutes the INFERRED ID-collision half of scout R1; outcome recorded either way.

## Ship order — three commits, each independently green, committed IN ORDER by Harmony after per-stage gates
- C1 = W1+W2+W3 (crash classes + snapshot + reorder). C2 = W4 (zero-flash close; no cadence change). C3 = W5+A4+A6+W6 (cadence move; revert of C3 alone restores old cadence minus the crashes).

## Residuals (recorded, not silently fixed)
- Routed params freeze on preview detach until the routing/signal follow-up (A1). Autopilot pauses on preview detach (pre-existing, unchanged). P3 scalar crossings (enlarged set) → spec Step 3. Msg-thread stalls bound mapping updates (modal pickers) — accepted; dominates the permanent freeze. Program-ID collision INFERRED half → settled by W7(iv).

## Boris items
- Rig-feel taste-check of mapping response after C3 (constant chosen to preserve measured feel; confirm only). INFO: routed-param + autopilot detach freezes remain until follow-ups.
