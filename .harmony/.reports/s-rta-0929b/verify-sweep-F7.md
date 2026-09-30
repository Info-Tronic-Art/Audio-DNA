# Verify F7 (sweep s-rta-0929b) - Clip::playheadPosition, Renderer.cpp:1823 write vs LayerStrip.cpp:763 atomic_ref read
VERDICT: REAL_BUG by the letter (C++ data race / UB), severity LOW, owner app. Not a tool artefact. Live-show effect: nothing visible.

## Report read (VERIFIED: runs/tsan-2/tsan.45047 lines 685-800; also tsan-3/.49175:121, tsan-5/.50099:874, tsan-6/.50561:557)
- Write 8 B, T47 "OpenGL Renderer", holding only M0/M1 (JUCE render mutex + CGL lock): Renderer::syncMedia Renderer.cpp:1823 <- CompositorEngine.cpp:1103 <- renderOpenGL :728.
- Prior "atomic read" 8 B, main thread: LayerStrip::updateTransportView LayerStrip.cpp:763 <- timerTick :783 <- juce Timer. Block = vector<optional<Clip>> from Layer::fromVar (a live loaded composition).
- Source (VERIFIED): Renderer.cpp:1823 `clip->playheadPosition = player->getPlayheadPosition();` plain store. Clip.h:229 `mutable double playheadPosition` (not atomic).
  LayerStrip.cpp:750 `std::atomic_ref<double>(clip->playheadPosition).load(relaxed)`. UI side was made atomic_ref on purpose (comment :747-749: "GL-side writers still write it plainly -- L5's open item").

## Refutation attempts (all failed)
1. Hidden lock: renderer holds M0/M1, message thread holds neither; no MessageManager::Lock with component painting off (as in F5 verify, juce_OpenGLContext.cpp:349-389). VERIFIED (TSan mutex sets).
2. Happens-before via callAsync/timer: nothing orders GL thread -> message thread. INFERRED from stacks.
3. Tool models atomic_ref: yes, it reports "atomic read", but atomic vs NON-atomic on one address is still a race; the plain writer is the defect. VERIFIED (report: "Write" vs "Previous atomic read").
4. Third-party/benign counter: no, app-owned live control field. VERIFIED.
=> Survives. Half-fix: reader is atomic_ref, writers are not. Other writers (VERIFIED by grep): Renderer.cpp:1823/1839/1910/1924 (GL), Layer.h:268/282, MainComponent.cpp:4639/6361, LayerStrip.cpp:988, ClipInspector.cpp:305/1247/1271 (message thread). Unsynchronised message-thread reads: ClipInspector.cpp:322/502/1006/1193, ApiServer.cpp:436, ClipPositionSignal.h:35, PerfStateCapture.cpp:92 (same class, other stacks: F6/F11/F14/F17/F28/F29).

## Live-show impact
- Torn value: essentially NONE on this hardware. INFERRED: aligned 8-byte store/load on arm64 is single-copy atomic and the compiler emits one str/ldr for a scalar double.
- Staleness: UI timer (30 Hz) may see a value one frame old; cursor lags at most a frame. ASSUMED negligible.
- Lost update (separate logic race, same field): a UI scrub write (LayerStrip.cpp:988, ClipInspector) can be overwritten by the next render-frame write from the player (Renderer.cpp:1823; see MainComponent.cpp:4659 comment). Not a torn access, not a crash. INFERRED.
- Crash: none from F7; the block is live, not freed. Lifetime races on the clip vector are the F2-F6 class, not this one.
- Likelihood: TSan hit 4/6 runs because the timer reads every tick; occurs continuously, harmless in practice.

## Smallest fix
Keep the field; add Clip::setPlayhead(double)/getPlayhead() using std::atomic_ref<double>(x).store/load(relaxed) and route the ~12 write sites above plus the message-thread reads through it (making the field std::atomic<double> is invasive: Clip is copied/moved). static_assert(std::atomic_ref<double>::is_always_lock_free). Zero cost on arm64/x86-64. Low priority; safe to defer past the show.
