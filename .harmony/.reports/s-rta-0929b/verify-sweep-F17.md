# Verify F17 (sweep s-rta-0929b) - Clip::playheadPosition, Renderer.cpp:1823 write vs LayerStrip.cpp:763 atomic_ref read (deck-switch stacks)
STATUS: DONE
VERDICT: REAL_BUG by the letter (C++ data race / UB), severity LOW, owner app. Same defect as F7, different reader stack. Not a tool artefact. Live-show effect: nothing visible.

## Report read (VERIFIED: sweep/runs/tsan-3/tsan.49175 ~l.617-700; tsan-6/tsan.50561 same frames)
- Write 8 B, T47 "OpenGL Renderer" (Renderer::syncMedia Renderer.cpp:1823 <- CompositorEngine.cpp:1103 <- Renderer.cpp:728), holds only M0 (JUCE render mutex) + M1 (CGL ctx lock).
- Prior "atomic read" 8 B, message thread: LayerStrip::updateTransportView :763 <- refresh :737 <- DeckView::refresh :276 <- setActiveColumn :323 <- handleColumnTrigger MainComponent.cpp:4871 (tsan-3), or handleClipTrigger :4727 (tsan-6); both entered from a REST trigger via ApiServer -> callAsync.
- Block = vector<optional<Clip>> built by Layer::fromVar (Layer.cpp:283) in loadComposition: a LIVE clip, not freed. VERIFIED.
- Source (VERIFIED): Renderer.cpp:1823 plain `clip->playheadPosition = player->getPlayheadPosition();`; LayerStrip.cpp:750 `std::atomic_ref<double>(...).load(relaxed)` (comment :747-749 says GL writers still write plainly).

## Refutation attempts (all failed)
1. Hidden lock: message thread holds neither M0 nor M1; TSan lists only the GL thread's mutexes. VERIFIED (report).
2. Happens-before via callAsync: callAsync orders REST thread -> message thread, nothing orders GL -> message. INFERRED from stacks.
3. atomic_ref modelled by tool: yes, but atomic vs plain store on one address is still a race; the plain writer is the defect. VERIFIED ("Write" vs "Previous atomic read").
4. Benign stats counter / third party: no, app-owned control field driving the transport UI. VERIFIED.
5. Deck-switch path different from F7 (reading a clip being replaced)? No: clip is live (heap block from fromVar, not a free). Same field; F17 is F7 hit from the trigger path instead of the 30 Hz timer. VERIFIED (frames). Lifetime races are the F2-F6 class.
=> Survives.

## Live-show impact
- Torn value: none expected. INFERRED: aligned 8-byte double store/load is single-copy atomic on arm64/x86-64.
- Effect: playhead read one frame stale; UI cursor lags <=1 frame. ASSUMED negligible. Crash: none. Wrong visual: none.
- Likelihood: 2/6 runs only because trigger REST calls are rare in the mix; the race exists on every trigger/deck switch. Harmless in practice.

## Smallest fix
Same as F7 (do once, closes F6/F7/F11/F14/F17/F28/F29): add Clip::setPlayhead/getPlayhead using std::atomic_ref<double>(playheadPosition).store/load(relaxed); route the writers (Renderer.cpp:1823/1839/1910/1924, Layer.h, MainComponent, LayerStrip, ClipInspector) and message-thread reads through it; static_assert(std::atomic_ref<double>::is_always_lock_free). Defer past the show; zero cost.
