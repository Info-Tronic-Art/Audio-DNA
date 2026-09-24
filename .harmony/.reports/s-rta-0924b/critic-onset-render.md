# Critic report: plan-onset-render.md

Reviewer: hostile critic seat, s-rta-0924b, 2026-09-24. Repo HEAD read: 7deb68b (main).
Method: re-derived every cited file:line by reading the actual source, not by trusting the plan's prose.

## Verdict: APPROVE-WITH-AMENDMENTS

The plan's factual base is exceptionally accurate — I re-checked ~40 file:line citations across
AnalysisThread.cpp, OnsetDetector.cpp, FeatureSnapshot.h, Renderer.cpp/.h, CompositorEngine.cpp,
EffectChain.cpp, ProceduralSource.cpp, OutputWindow.cpp/.h, AudioReadoutPanel.cpp/.h, ApiServer.cpp,
TestServer.cpp/.h, RecorderHost.cpp, MappingEngine.cpp, SignalRegistry.cpp, TopBar.cpp, FeatureBus.h/.cpp,
tests/CMakeLists.txt, tests/test_integration_pipeline.cpp, gen-click-wav.py — and every load-bearing
claim held up. One MAJOR correctness gap (below) needs an amendment before Builder starts; the rest are
MINOR citation drift that costs the Builder a few seconds re-locating a line, not a design flaw.

## BLOCKER: none.

## MAJOR

**1. The TestServer/ApiServer onsetCount mirror has a real TOCTOU race the plan's own thread-ownership
section (4) claims doesn't exist.**
- Evidence: `TestServer::injectSnapshot` (`src/test/TestServer.cpp:44-56`) is the ONLY place that takes
  `injectMutex_`. Its own comment says why: "Several HTTP threads (this server's pool + the ApiServer
  relay) can land here concurrently" — concurrent access is a documented, anticipated scenario, not
  speculative.
- `ApiServer::handleInjectFeatures` (`:679-751`) computes its onsetCount delta from
  `FeatureSnapshot snap = featureBus_.read();` (`:684`) with NO lock, then eventually calls
  `onInjectFeatures(snap)` which MainComponent wires straight to `testServer_->injectSnapshot(snap)`
  (`src/MainComponent.cpp:2159-2162`) — bypassing `TestServer::handleInjectFeatures` entirely (contrary
  to what a reader might assume from the plan's framing of these as two independent "mirror" sites that
  each need their own bump logic — they are two independent BASELINE SOURCES feeding the SAME sink).
- `TestServer::handleInjectFeatures` itself (its own :8080 route) computes its delta from the plan's
  proposed `lastInjectedOnsetCount_.load()` — also read with no lock, before `injectSnapshot()`'s lock.
- Race: two concurrent inject calls (e.g. one via `/api/inject_features` on 7070 relayed to
  `injectSnapshot`, one via `/api/inject_features` on 8080 calling `TestServer::handleInjectFeatures`
  directly, or simply two callers on the same route) each read the SAME pre-lock baseline, each compute
  "baseline+1", and the second one to reach the lock overwrites the first's bump with an IDENTICAL value
  instead of baseline+2 — a lost onset, exactly the class of bug this whole plan exists to fix, now
  reintroduced in the test-mode injection path. The plan's section 4 states
  "`TestServer::lastInjectedOnsetCount_`: atomic, written under `injectMutex_`... read by HTTP threads"
  — true for the WRITE (inside `injectSnapshot`, correctly placed per 2.6), false for the READ used to
  COMPUTE the next value (outside the lock, in the caller).
- Impact: test-mode only (no production/analysis-thread path affected), and the Eyes test C as written
  is single-client sequential so it will not flake — but the live probe's stated precondition ("multi-
  client-correct" is the entire justification for choosing raw `onsetCount` over a sticky flag in section
  1) is violated by the plan's own test-mode implementation of that counter.
- Amendment: either (a) do the whole read-modify-publish under `injectMutex_` in both call sites (move
  the delta computation for both ApiServer's and TestServer's inject paths inside
  `TestServer::injectSnapshot`, which already owns the lock — i.e. pass the caller's intent, not a
  precomputed `onsetCount`, into a single locked function that mirrors AnalysisThread's
  `++totalOnsetCount_` under the lock), or (b) make `lastInjectedOnsetCount_` the single source of truth
  for BOTH paths (ApiServer's mirror logic must also consult `lastInjectedOnsetCount_`, not
  `featureBus_.read()`, and the increment must happen via `fetch_add` under the same mutex). Say which
  in the spec before the Builder writes code — as given, 2.6's two bullets independently pick different
  and incompatible baselines.

## MINOR (citation drift — verified, does not change the design)

2. `OutputWindow.h`: `effectChainGLState_` is declared at line **55**, not `:46` as the plan states in
   2.3 ("next to `effectChainGLState_` `:46`"). Line 46 is `EffectChain& effectChain_;`. Harmless — the
   Builder will find the real member by name, not by line number, but a line-precise plan should be
   line-precise.
3. `OutputWindow.cpp`: the `if (!texMgr_.hasImage()) ... return;` early-return block is at **94-99**, not
   `:99-105` as 2.3 states. The bus read itself (`:107`) is cited correctly.
4. `ApiServer::handleGetFeatures` starts at line **634**, not `:635` (2.5 says "`:635-677`"); the
   `onsetDetected` line inside it is correctly cited at `:649`.
5. `RecorderHost.cpp`: `kMaxOnsetMarkersPerTick` is declared at line **22**, not `:21` (section 1,
   "tradeoffs" bullet).
None of these affect the spec's correctness; they're exactly the kind of off-by-a-few-lines drift that
compounds if uncaught, so flagging per the brief.

## Things I tried to break and could NOT

- **Frame-loop reorder safety** (2.2): re-read `Renderer::renderOpenGL` lines 149-231 end to end. Nothing
  between the proposed insertion point (after `deckActive` at `:216`) and the nothing-to-render early
  return (`:218-226`) uses `snap`/`frameSnap_` today — confirmed by eye, not just grep. The reorder is safe.
- **`renderSource` confinement**: grepped every caller of `Renderer::renderSource` — only the compositor's
  `sourceRenderFn_` lambda (itself only invoked from within `compositeDeck`/`compositePersistentLayers`,
  both called from `renderOpenGL`) and the direct call at `:525` (also inside `renderOpenGL`).
  `getOrCreateSource`'s documented cross-thread marshaling (`test_renderer_source_confinement.cpp`'s
  target) is for callers OTHER than `renderSource` (e.g. the MilkDrop preset-click path at `:352/:366/
  :377`) — `renderSource` itself never runs off the GL thread or outside a `renderOpenGL` frame, so
  `frameSnap_` is always valid when it's read. This was my strongest suspected hole and it doesn't exist.
- **`FeatureBus::read()`'s per-thread "last-good" fallback** (`FeatureBus.cpp:66-97`): on retry
  exhaustion it serves a stale-but-never-backwards copy per calling thread. This actually HELPS the
  plan's backwards-jump heuristic stay rare in production (only test-mode resets/relays trigger it), and
  confirms `onsetCount` truly is monotonic-per-reader outside test mode, which is the invariant `OnsetPulse`
  needs.
- **Autopilot/downbeat interference**: grepped `Autopilot.cpp/.h` for `onsetDetected`/`onsetStrength` —
  zero hits, so overwriting `frameSnap_.onsetDetected` in place before `autopilot_.processFrame(*deck, snap)`
  cannot change autopilot behavior, contrary to what I initially suspected from the reorder.
- **kSnapshotWords / seqlock cost claim**: `FeatureBus.h:127`, `kSnapshotWords = 80` — matches the plan's
  "80-word seqlock copy" claim exactly.
- **The 20ms-burst/-6dBFS/-50dBFS/120BPM test-B parameters**: these are not invented — they are the exact
  literals of an EXISTING `TEST_CASE` at `tests/test_integration_pipeline.cpp:704-712` (db72d56's own
  onset-count test), just re-run at 20s/40-clicks instead of 10s/20-clicks. Good reuse of precedent, not
  a fabricated fail-first scenario.

## Scope / simpler-alternative check
No scope creep found: `FeatureSnapshot` layout is untouched (verified `onsetCount`'s
`static_assert(offsetof(...) == 316)` and `sizeof == 320` are pre-existing, not proposed changes), no new
GLSL uniform, downbeat's identical defect class is explicitly and correctly deferred (`TopBar.cpp:232`
citation verified exact). The three REJECTED alternatives in section 1 are argued against real constraints
(3 spare struct bytes, per-uploader race) rather than straw men — I could not find a simpler design that
also survives the "which uploader owns the pulse" trap section 1 identifies (per-uploader `OnsetPulse`
does starve every source clip after the first, confirmed by the `renderSource`-per-clip call pattern in
`CompositorEngine.cpp:820-840` etc.).

## Fail-first test discrimination
Tests A and B are genuinely discriminating (A: compile-fails on missing header today; B reuses a live
precedent test and states concrete pre-fix numbers, e.g. "26 == 40" at 60fps). Eyes test C's oracle
(`onset_pulse_frames` KeyError on main) is a real RED, not a tautology. Live-probe D's oracle (equality of
two independent monotonic counters, `renderOnsetPulses` vs `onsetCount`) is the correct choice given HTTP
polling has no client identity — I could not construct a case where that equality holds accidentally
pre-fix (the plan's own ~36% loss / ~26% duplication math at the two fps regimes is internally consistent
with `AnalysisThread`'s hop cadence, `minioi=50ms` cited exactly from `OnsetDetector.cpp:22`).

## Recommendation
APPROVE-WITH-AMENDMENTS: fix the MAJOR (test-mode inject race) before the Builder writes 2.6's code —
pick one locking design and state it explicitly, since the plan's two bullets currently disagree on where
the baseline comes from. The five MINOR citation corrections should be applied to the spec doc itself so
the Builder isn't hunting for a member that moved five lines. Nothing else needs to change; the core
design (bus-reader-owned `OnsetPulse`, `Renderer` as sole intra-frame source of truth via `frameSnap_`,
raw `onsetCount` on `/api/features`) is sound and traces cleanly to real code at every cited site.
