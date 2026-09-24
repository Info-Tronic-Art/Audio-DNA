# Plan: onset LOSS (and duplication) in the render path -- act on FeatureSnapshot::onsetCount deltas

Author: Architect (Fable), s-rta-0924b, 2026-09-24. Repo HEAD read: 7deb68b (main).
Read-only on source; every codebase claim is `path:line`-cited and binned VERIFIED / INFERRED / ASSUMED.

QUESTION: Render-side consumers (and GET /api/features) read the one-hop bool
`FeatureSnapshot::onsetDetected` from the always-latest FeatureBus at their own cadence, so
onset pulses are skipped (or doubled). `onsetCount` (monotonic, db72d56) exists. How should
each consumer act on its delta -- who owns the last-seen state, what pulse semantics do
shaders get, what does the multi-client REST API expose -- with fail-first tests and a live
check?

APPROACH (stated first): The pulse state belongs to the BUS READER, never to an uploader.
Add a 20-line header-only `OnsetPulse` helper (`src/features/OnsetPulse.h`, same pattern as
`Smoother.h`) that turns consecutive `onsetCount` values into a per-read delta (unsigned,
wrap-safe, re-baselines on a backwards jump, first call = baseline only). Each thread that
calls `FeatureBus::read()` and cares about onsets owns exactly one instance: the main
`Renderer` (GL thread) derives the frame pulse ONCE per frame right after its bus read and
writes it into the frame's snapshot copy (`frameSnap_.onsetDetected = delta > 0`), which is
the single snapshot handed to CompositorEngine, EffectChain and every ProceduralSource
that frame -- so those three uploaders need ZERO changes and cannot disagree within a
frame. `OutputRenderer` (second GL thread) and `AudioReadoutPanel` (message thread, 30 Hz)
each get their own instance. `/api/features` gains a raw `onsetCount` field (clients diff
it themselves; `onsetDetected` stays as-is for back-compat); a renderer-side diagnostic
counter `renderOnsetPulses` (frames that pulsed) is exposed on `/api/status` (production)
and `onset_pulse_frames` on `/api/state` (test server) -- that counter IS the live oracle:
after a click train, `renderOnsetPulses` delta must EQUAL `onsetCount` delta. Test-mode
injection mirrors AnalysisThread's bookkeeping (an injected `onsetDetected:true` bumps the
count) so the existing Eyes tests keep working and the render path can be driven from
outside.

## 0. Facts established (the plan is built on these)

Cadences (VERIFIED constants, INFERRED probabilities):
- Analysis publishes once per 512-sample hop at the fixed 48 kHz = 10.667 ms (~93.75 Hz)
  (`src/analysis/AnalysisThread.cpp:172-180`; CLAUDE.md analysis-thread section).
- At 60 fps (16.667 ms/frame) a hop is observed by a frame with probability
  10.667/16.667 = 0.64 -> ~36% of onset hops are never seen ("LOSS").
- At ~118 fps -- what Boris's rig actually renders (`.harmony/gotchas.md:235` "fps reads a
  healthy ~110", `:376` "gravity_well, fps ~118") -- every hop is seen at least once and
  ~26% are seen TWICE -> ~26% of onsets DUPLICATED (a second impulse into stateful sims).
  So on the development rig the defect direction is duplication, not loss; below ~93.75
  fps it is loss. The same fix removes both; the live probe must record fps and interpret
  the pre-fix sign accordingly (section 5).
- aubio minioi = 50 ms (`src/analysis/OnsetDetector.cpp:22`) -> onsets are >= 4.7 hops
  apart -> a delta >= 2 in one frame is only possible if a frame stalls > 50 ms. In steady
  state the per-frame delta is 0 or 1.

Exhaustive consumer inventory (`grep -rn onsetDetected src/` -- VERIFIED, 2026-09-24):
| # | Consumer | Thread / cadence | Reads bus at | Uses onsetDetected at | Fix |
|---|---|---|---|---|---|
| 1 | main Renderer frame | GL thread, VSync | `src/render/Renderer.cpp:231` | via `compositor_.setLatestSnapshot(snap)` `:481` -> `CompositorEngine::uploadAudioUniforms` `src/render/CompositorEngine.cpp:1576`; via `effectChain_.render(..., snap, ...)` `:550` -> `EffectChain::uploadEffectUniforms` `src/effects/EffectChain.cpp:342-343` | derive pulse once here (section 2.2) |
| 2 | `Renderer::renderSource` | GL thread, once PER SOURCE CLIP per frame | **second read** `src/render/Renderer.cpp:1110` | `ProceduralSource::uploadUniforms` `src/sources/ProceduralSource.cpp:179-180` (called from `ProceduralSource::render` `:120`/`:134`); callers: compositor callback `Renderer.cpp:129-133` invoked at `CompositorEngine.cpp:828, :936, :1001, :1199`, and single-source mode `Renderer.cpp:525` | replace the fresh read with the frame snapshot (section 2.2) |
| 3 | `OutputRenderer::renderOpenGL` | SECOND GL thread (output window) | `src/ui/OutputWindow.cpp:107` | `effectChain_.render(..., snap, ...)` `:161-166` (shared EffectChain, own GL state) | own `OnsetPulse` (section 2.3) |
| 4 | `AudioReadoutPanel::timerCallback` | message thread, 30 Hz (`src/ui/AudioReadoutPanel.cpp:12`) | `:17` | `:52` (display copy), `:64-67` onset flash | own `OnsetPulse` (section 2.4) -- at 30 Hz today it sees only ~32% of onsets |
| 5 | `MainComponent::tickFeaturePipeline` | message thread, 120 Hz (`src/MainComponent.h:306`, `.cpp:3430`) | `:3430` | `RecorderHost::tick` `:3463` -> ALREADY delta-based (`src/recording/RecorderHost.cpp:423-446`, `onsetCountBaseline_`); `signalRegistry_.evaluateAll` / `MappingEngine` only consume `onsetStrength` (`src/mapping/MappingEngine.cpp:82`, `src/signal/SignalRegistry.cpp:29,45`) -- continuous ODF, no pulse consumer | none |
| 6 | `ApiServer::handleGetFeatures` | HTTP threads, any client rate | `src/api/ApiServer.cpp:635` | `:649` raw bool | add `onsetCount` (section 2.5) |
| 7 | `ApiServer::handleInjectFeatures` (test-mode route, `:206`) | HTTP | `:685` read-modify-write | `:713` | accept `onsetCount`, mirror bump (section 2.6) |
| 8 | `TestServer::handleInjectFeatures` / `handleReset` (--test-mode only) | HTTP | builds a CLEARED snapshot `src/test/TestServer.cpp:436-438` -> `onsetCount` is ALWAYS 0 today; reset publishes cleared `:641-643` | `:471-472` | mirror bump + remember last count (section 2.6) |
Not consumers (VERIFIED by grep): `Autopilot`, `Layer` (beat/bar based, no onset hits);
`PerfStateCapture` (arm/stop state capture, no `onsetCount` use -- the handoff's
"RecorderHost/PerfStateCapture use the delta" is true only of RecorderHost); `TopBar.cpp:232`
reads `downbeatDetected` -- same defect class, different field, no counter today: OUT OF
SCOPE (follow-up in section 6). `src/connect/` has no onset hits.

Shaders (VERIFIED, `grep -n u_onsetDetected src/render/EmbeddedShaders.h` = 8 hits):
declared in transientFlash `:10327`, beatRipple `:10368`, sourceLightning `:11435`,
sourceGravityWell `:12196`, sourceFluidDynamics `:12331`; REFERENCED only in Gravity Well
(`:12226` scatter force, `:12302` explosion) and Fluid Dynamics (`:12422` burst). The
other three react via `u_onsetStrength` (`:10335`, `:10394`, `:11480`); an unreferenced
uniform is eliminated at link so `getUniformIDFromName` returns -1 and nothing is uploaded
(standard GL, INFERRED -- no test proves it here). Both real consumers are STATEFUL
ping-pong sims that want exactly one impulse per onset.

Frame-loop structure (VERIFIED): `Renderer::renderOpenGL` early-returns at `:219-229`
BEFORE the bus read at `:231` when nothing is loaded; the read result `snap` also feeds
`autopilot_.processFrame` `:249` and the genre/structural edge detectors `:262-286`, which
are GL-thread-owned last-value members (`lastDetectedGenre_`, `lastStructuralState_`) --
the precedent for the new `onsetPulse_` member. `newOpenGLContextCreated` is `:103`.
Eyes capture happens INSIDE the frame (`captureFrame` `:1848` waits on a promise fulfilled
by `processPendingCapture` `:1890`, called from within `renderOpenGL`), and `render_frame`
captures of a live stateful source converge to a steady state (`.harmony/gotchas.md:373-376`)
-- so a PSNR diff is NOT a usable oracle for a one-frame pulse; a counter is.

FeatureSnapshot layout (VERIFIED `src/analysis/FeatureSnapshot.h:94-137`): `onsetCount` at
offset 316, sizeof 320, static_asserts pin both; bytes 313-315 are the only spare padding
(3 bytes). Nothing in this plan changes the struct.

## 1. TRADEOFFS CONSIDERED

- Per-reader-thread `OnsetPulse`, Renderer overrides `onsetDetected` in its frame copy
  (CHOSEN). One state per bus reader; the three uploaders stay pure functions of the
  frame snapshot; intra-frame consistency across compositor/global chain/every source
  clip is guaranteed; ~15 lines of production change outside the helper.
- Each uploader (CompositorEngine/EffectChain/ProceduralSource) holds its own OnsetPulse
  -- REJECTED: they would race for the same delta inside one frame (whoever uploads first
  consumes it, the others see 0), and `ProceduralSource::uploadUniforms` runs once per
  source clip, so only the first source clip would flash. This is the trap the task's
  "who owns last-seen state" question points at.
- Analysis-thread decaying `onsetEnvelope` field, no consumer state -- REJECTED: a float
  does not fit in the 3 spare bytes (layout change + FeatureBus word count + 4
  static_asserts); an envelope is still sampled, so stateful sims would inject on several
  consecutive frames (the 118-fps duplication made permanent); it fixes a render-side
  cadence problem in the analysis thread.
- Carry excess (`delta >= 2`) to the next frame like `RecorderHost` (`kMaxOnsetMarkersPerTick`
  `RecorderHost.cpp:21`) -- REJECTED for visuals: two onsets in one frame need a > 50 ms
  stall; a flash rendered a frame late is worse than a merged one; sims want one impulse
  per frame. Pulse = `delta > 0`, whole delta consumed.
- Add a render-side decaying envelope uniform (`u_onsetEnvelope`) -- REJECTED (scope):
  `u_onsetStrength` already serves shaders wanting a continuous value (transientFlash,
  beatRipple, lightning use it today); a new uniform means 3 uploaders + docs + shader
  authoring guidance. Follow-up if a shader author asks.
- `/api/features`: server-side per-client state -- IMPOSSIBLE (no client identity in a
  stateless HTTP poll); read-and-clear sticky flag -- WRONG for multi-client (the first
  poller steals the pulse); hold-for-N-ms flag -- still rate-dependent (fast pollers see
  duplicates, slow ones still miss). Raw `onsetCount` -- CHOSEN: the only stateless,
  multi-client-correct semantic, identical to the in-process contract; same precedent as
  `totalBarCount` exposed "so a live sweep can prove the fix through the app"
  (`src/api/ApiServer.cpp:601-605`).
- Keep the bus read AFTER the nothing-to-render early return -- REJECTED: idle frames would
  not consume, so the first rendered frame after an idle stretch would flash once for all
  onsets that happened meanwhile. Hoisting the read costs one 80-word seqlock copy per idle
  frame (`src/features/FeatureBus.cpp:66-97`) -- negligible.

## 2. DECISION / SPEC

### 2.1 New helper `src/features/OnsetPulse.h` (header-only, zero allocation, no atomics)

```cpp
#pragma once
#include <cstdint>

// Consumer-side view of FeatureSnapshot::onsetCount (a monotonic per-hop counter that
// AnalysisThread publishes in EVERY snapshot, AnalysisThread.cpp:172-180). Turns "how many
// onsets since MY previous look" into a per-read delta, so a consumer polling the
// always-latest FeatureBus at ANY cadence neither misses an onset (slower than analysis)
// nor double-counts one (faster than analysis). Exactly ONE instance per consumer THREAD,
// owner-thread-confined like Renderer's lastDetectedGenre_ edge detector -- never shared
// between threads, never shared between uploaders (the pulse is derived once per bus read
// and carried in the snapshot copy). RecorderHost::onsetCountBaseline_ is the same idea
// specialised for marker emission; this is the generic form.
class OnsetPulse
{
public:
    // Onsets since the previous consume(). The FIRST call only establishes the baseline and
    // returns 0 (never report onsets from before this consumer started looking). Unsigned
    // subtraction: exact across a 2^32 wrap. A backwards jump (delta >= 2^31) can only come
    // from a writer reset -- test-mode inject from a cleared snapshot / TestServer reset;
    // the production writer is monotonic for the process lifetime -- so it re-baselines
    // and returns 0 instead of a spurious ~4e9.
    uint32_t consume(uint32_t onsetCount) noexcept
    {
        if (!primed_) { primed_ = true; last_ = onsetCount; return 0u; }
        const uint32_t delta = onsetCount - last_;
        last_ = onsetCount;
        return (delta & 0x80000000u) != 0u ? 0u : delta;
    }
    // Forget the baseline; the next consume() re-primes with zero delta. Call where the
    // consumer starts looking afresh (GL context creation).
    void reset() noexcept { primed_ = false; last_ = 0u; }
    bool primed() const noexcept { return primed_; }
private:
    bool     primed_ = false;
    uint32_t last_   = 0u;
};
```

### 2.2 Main `Renderer` -- the single owner of the render-frame pulse

`src/render/Renderer.h`:
- `#include "features/OnsetPulse.h"` (next to `features/FeatureBus.h`, `:11`).
- Private members next to `scaledTime_` (`:284`, GL-thread-owned precedent), with a comment
  mirroring the style there:
  ```cpp
  // Onset pulse-loss fix: this GL thread's consumer of FeatureSnapshot::onsetCount and the
  // frame's snapshot copy carrying the derived pulse (frameSnap_.onsetDetected). Read once
  // per renderOpenGL() BEFORE any early return; every uploader this frame (CompositorEngine,
  // EffectChain, each ProceduralSource via renderSource) reads frameSnap_, never the bus.
  // GL-thread-owned, no atomic needed.
  OnsetPulse      onsetPulse_;
  FeatureSnapshot frameSnap_{};
  // Diagnostic: frames on which the pulse fired (live oracle: after a click train this
  // delta == /api/features onsetCount delta). Written on the GL thread, read by HTTP threads.
  std::atomic<uint32_t> onsetPulseFrames_{0};
  ```
- Public getter next to `getFps()` (`:317`):
  `uint32_t getOnsetPulseFrames() const { return onsetPulseFrames_.load(std::memory_order_relaxed); }`

`src/render/Renderer.cpp`:
- `newOpenGLContextCreated()` (`:103`): add `onsetPulse_.reset();` (a re-attached context
  starts fresh -- no stale-delta flash on preview show/hide).
- `renderOpenGL()`: move the bus read ABOVE the nothing-to-render early return. Concretely,
  after `bool deckActive = (deck != nullptr);` (`:216`) and before `// Check if we have
  anything to render` (`:218`), insert:
  ```cpp
  // Onset pulse-loss fix: read the bus FIRST (before the early return below) so idle frames
  // keep the pulse baseline current, then derive this frame's pulse from the monotonic
  // onsetCount delta. frameSnap_.onsetDetected now means "at least one onset since this
  // context's previous frame" -- loss-free below the analysis rate, duplicate-free above it
  // -- and is the ONE snapshot every uploader sees this frame (see Renderer.h).
  frameSnap_ = featureBus_.read();
  frameSnap_.onsetDetected = onsetPulse_.consume(frameSnap_.onsetCount) > 0u;
  if (frameSnap_.onsetDetected)
      onsetPulseFrames_.fetch_add(1u, std::memory_order_relaxed);
  const FeatureSnapshot& snap = frameSnap_;
  ```
  and DELETE the original `const FeatureSnapshot snap = featureBus_.read();` at `:231`
  (keep its "R5" comment content in the new block if desired). Every later `snap.` use
  (`:249`, `:262-286`, `:481`, `:550`) compiles unchanged.
- `renderSource()` (`:1109-1110`): replace
  `const FeatureSnapshot snap = featureBus_.read();` with
  `const FeatureSnapshot& snap = frameSnap_;  // frame's pulse-bearing copy, not a fresh read`
  (comment: a fresh read here could also see a DIFFERENT hop than the frame's other
  uploaders, and each source clip would consume/miss the pulse independently).
- Leave the BPM-only reads at `:1281` and `:1350` alone (surgical; they use `snap.bpm`
  only). Optional tidy-up: point them at `frameSnap_` too -- not required.

### 2.3 `OutputRenderer` (second GL context) -- own instance
`src/ui/OutputWindow.h`: `#include "features/OnsetPulse.h"`; private `OnsetPulse onsetPulse_;`
(GL-thread-owned, next to `effectChainGLState_` `:46`).
`src/ui/OutputWindow.cpp`: in `newOpenGLContextCreated()` (`:49`) add `onsetPulse_.reset();`.
In `renderOpenGL()` move the read (`:107`) ABOVE the `if (!texMgr_.hasImage()) ... return;`
block (`:99-105`) and make it:
```cpp
FeatureSnapshot snap = featureBus_.read();                       // this context's own read (W2)
snap.onsetDetected = onsetPulse_.consume(snap.onsetCount) > 0u;  // per-context frame pulse
```
`effectChain_.render(..., snap, ...)` (`:161-166`) is unchanged. Keep the W2 comment.

### 2.4 `AudioReadoutPanel` (message thread, 30 Hz)
`src/ui/AudioReadoutPanel.h`: `#include "features/OnsetPulse.h"`; private `OnsetPulse onsetPulse_;`
next to `onsetFlash_` (`:25`).
`src/ui/AudioReadoutPanel.cpp` `timerCallback()`: after the read (`:17`) compute
`const uint32_t onsets = onsetPulse_.consume(snap.onsetCount);` then
`:52` -> `displaySnap_.onsetDetected = (onsets > 0u);` and `:64` -> `if (onsets > 0u)`.
(`displaySnap_.onsetDetected` is write-only today -- `drawOnsetIndicator` uses
`onsetFlash_`, `:410-416` -- set it anyway for consistency.)

### 2.5 `ApiServer` -- multi-client REST semantics (decision + why)
- `handleGetFeatures` (`src/api/ApiServer.cpp:635-677`): after `:649` add
  `obj->setProperty("onsetCount", static_cast<juce::int64>(snap.onsetCount));`
  Keep `onsetDetected` exactly as-is (raw latest-hop flag, back-compat). Semantic, to be
  written in TESTING.md/CLAUDE.md: "`onsetDetected` is the latest analysis hop's one-hop
  flag and is rate-dependent; a poller that must count onsets diffs `onsetCount` between
  its own polls (unsigned 32-bit, monotonic for the process lifetime)". Rationale: HTTP
  polls carry no client identity, so per-client server state is impossible and any
  read-and-clear/hold scheme is wrong with two clients; the counter is stateless and is the
  same contract in-process consumers use. int64 so the value never reads negative.
- `handleStatus` (`:280-300`): add
  `obj->setProperty("renderOnsetPulses", static_cast<juce::int64>(renderer_.getOnsetPulseFrames()));`
  (renderer-owned diagnostics already live here: `fps`, `frameTimeMs` `:284-285`).

### 2.6 Test-mode injection mirrors AnalysisThread (both inject paths)
Rule (mirrors `AnalysisThread.cpp:178-180`: one increment per PUBLISHED snapshot whose
`onsetDetected` the writer set): an injected snapshot's `onsetCount` = explicit
`onsetCount` field if present; otherwise previous published count + (1 if THIS request
explicitly set `onsetDetected:true` else 0).
- `ApiServer::handleInjectFeatures` (`:679-751`, starts from `featureBus_.read()` so the
  current count is already carried): immediately after the `onsetDetected` line (`:713`):
  ```cpp
  if (json.hasProperty("onsetCount"))
      snap.onsetCount = static_cast<uint32_t>(std::clamp(static_cast<int>(json["onsetCount"]), 0,
                                                         std::numeric_limits<int>::max()));
  else if (json.hasProperty("onsetDetected") && snap.onsetDetected)
      ++snap.onsetCount;  // mirror AnalysisThread: one count per injected onset hop
  ```
  (guarding on `json.hasProperty("onsetDetected")` prevents phantom bumps from an
  `onsetDetected=true` carried over from an earlier inject.)
- `TestServer` (`src/test/TestServer.h:111-113`, `.cpp:44-57`, `:416-517`, `:614-650`):
  add private `std::atomic<uint32_t> lastInjectedOnsetCount_{0};`; in `injectSnapshot()`
  after `*staging = snap;` add
  `lastInjectedOnsetCount_.store(staging->onsetCount, std::memory_order_relaxed);`
  (this also makes `handleReset`'s cleared publish reset it to 0). In
  `handleInjectFeatures` after `:471-472`:
  ```cpp
  snap->onsetCount = obj->hasProperty("onsetCount")
      ? static_cast<uint32_t>(std::max(0, static_cast<int>(obj->getProperty("onsetCount"))))
      : lastInjectedOnsetCount_.load(std::memory_order_relaxed) + (snap->onsetDetected ? 1u : 0u);
  ```
  `handleState` (`:572-575`): add
  `obj->setProperty("onset_pulse_frames", static_cast<juce::int64>(renderer_.getOnsetPulseFrames()));`
  (snake_case like `frame_time_ms`).
- Behavioural note (test mode): an injected `onsetDetected:true` used to be STICKY for the
  render path (true on every frame until the next inject); it is now a ONE-frame pulse on
  the first frame after the inject, plus the raw bool stays true in the bus. The existing
  Eyes onset cases (`tests/visual/test_audio_reactivity.py:36`, `tests/visual/test_signals.py:74-76`)
  drive mandelbrot/julia_set/burning_ship/audio_waveform/mandelbulb/kifs -- none of the 5
  shaders that declare `u_onsetDetected` -- so they react via `onsetStrength` and are
  unaffected (VERIFIED by the shader grep in section 0). Reset (`handleReset`) publishes
  count 0 -> every consumer's `OnsetPulse` sees a backwards jump -> re-baselines, no pulse.

### 2.7 Shader-facing semantics (document in CLAUDE.md "Audio Uniform System" table)
`u_onsetDetected` = 1.0 on exactly the first render frame (per GL context) that observes
>= 1 new onset since that context's previous frame, else 0.0. Never lost at any frame
rate; never duplicated above the analysis rate; >= 2 onsets in one frame (only under a
> 50 ms stall) collapse into one pulse. Residual, unchanged: `u_onsetStrength` on that
frame is the LATEST hop's ODF (`aubio_onset_get_descriptor`, `OnsetDetector.cpp:39`),
which is already one hop past the peak on the detected hop (aubio confirms a peak one hop
late -- `.harmony/gen-click-wav.py` docstring, INFERRED) and two hops past on a recovered
frame; shaders scaling the burst by strength may flash slightly weaker on recovered
frames. No new uniform; no decaying envelope (section 1).

### 2.8 Files touched (complete list)
Production: `src/features/OnsetPulse.h` (new); `src/render/Renderer.h`, `.cpp`;
`src/ui/OutputWindow.h`, `.cpp`; `src/ui/AudioReadoutPanel.h`, `.cpp`; `src/api/ApiServer.cpp`;
`src/test/TestServer.h`, `.cpp`. Deliberately UNTOUCHED: `CompositorEngine.cpp:1576`,
`EffectChain.cpp:342-343`, `ProceduralSource.cpp:179-180` (they read the frame copy),
`RecorderHost`, `MappingEngine`, `SignalRegistry`, `Autopilot`, `Layer`, `AnalysisThread`,
`FeatureSnapshot.h` (no layout change; optionally extend the `:109` comment to name
`OnsetPulse` as the generic consumer), all shaders.
Tests: `tests/test_onset_pulse.cpp` (new) + target in `tests/CMakeLists.txt`;
`tests/test_integration_pipeline.cpp` (+1 TEST_CASE); `tests/visual/test_onset_pulse.py` (new).
Live: `.harmony/probe-onset-render.sh` (new; Harmony/live-app agent, `git add -f`).
Docs: `CLAUDE.md` (Audio Uniform System row for `u_onsetDetected`; Common Pitfalls #30:
"render-side onset consumers must use the onsetCount delta via OnsetPulse, one instance per
reader thread, never per uploader"); `tests/visual/TESTING.md:121-126` (+`onsetCount` in
inject fields, the mirror rule, `/api/features.onsetCount`, `/api/state.onset_pulse_frames`);
`.harmony/APP-INVENTORY.md` if it enumerates `/api/status`/`/api/features`/`/api/state` fields.

### 2.9 Build sequence for the Builder (fail-first, scratch build dir per rig rules)
1. Write tests A and B (section 3) FIRST. Run in the scratch dir: A fails to compile
   (missing `features/OnsetPulse.h` -- the db72d56 precedent) and B, written per its RED
   form, fails at runtime with the loss/dup numbers. Record both in the commit message.
2. Add the helper + Renderer/OutputRenderer/AudioReadoutPanel/ApiServer/TestServer edits
   (2.1-2.6). ctest: 408 + new cases green. Grep gate: no `featureBus_.read()` left in
   `Renderer::renderSource`; `u_onsetDetected` upload lines unchanged.
3. Eyes test C (section 3) against a `--test-mode` build (RED on main = KeyError
   `onset_pulse_frames`), then GREEN.
4. Live probe D (section 5) in production mode.
5. Docs (2.8), CLAUDE.md pitfall, TESTING.md.

## 3. FAIL-FIRST TESTS (Catch2 unless noted)

A. `tests/test_onset_pulse.cpp`, new target `test_onset_pulse` -- copy the header-only
   block of `test_smoother` (`tests/CMakeLists.txt:121-127`: add_executable, include
   `${SRC_DIR}`, Catch2WithMain, `apply_sanitizers`, `catch_discover_tests`). Cases
   (`[onsetpulse]`):
   1. first `consume(7)` returns 0 (baseline only); `primed()` true.
   2. same count twice -> 0 (fast poller, no duplicate).
   3. +1 -> 1; +3 in one read -> 3 (missed hops recovered).
   4. wrap: prime at 0xFFFFFFFE, then `consume(1)` -> 3.
   5. backwards jump: prime at 57, `consume(0)` -> 0, then `consume(1)` -> 1 (writer reset).
   6. `reset()`: after `reset()`, `consume(100)` -> 0, `consume(101)` -> 1.
   7. arithmetic sampling model (no bus): hop counter advances at 93.75 Hz with an onset every
      47 hops for 20 s; a reader polls the LATEST count at 60 Hz and another at 120 Hz;
      for each reader: sum of deltas == total onsets AND number of polls with delta > 0 ==
      total onsets (no merge possible: 47 hops >> 1.6 hops/frame).
   RED on main: compile error (header missing). GREEN after 2.1.

B. `tests/test_integration_pipeline.cpp` (existing target, links aubio; reuse
   `generateClickTrainWithNoiseFloor` and `PipelineRunner`, which already mirrors
   `onsetCount`, `:67-140` per db72d56). New TEST_CASE
   "Onset pulse: a render-cadence reader of the always-latest snapshot sees every onset
   exactly once via OnsetPulse" `[integration][onsetcount][render]`:
   - 120 BPM click train, 20 ms bursts, -6 dBFS, -50 dBFS floor, 20 s (40 clicks).
   - Model two readers (fps = 60 and 120). Hop j is published at
     `tPub = (j+1) * kHopSize / kSampleRate`; each reader keeps `nextFrame` starting at 0;
     after every processed hop: `while (nextFrame <= tPub) { readLatest(); nextFrame += 1.0/fps; }`
     where `readLatest()` counts (a) `framesRawTrue += latest.onsetDetected ? 1 : 0` (the
     expression production uses today) and (b) `framesPulse += pulse.consume(latest.onsetCount) > 0`.
   - GREEN assertions: `REQUIRE(framesPulse[60] == detectedOnsets)`,
     `REQUIRE(framesPulse[120] == detectedOnsets)`, `REQUIRE(detectedOnsets > 5)`.
   - Defect-documenting checks (pass before and after): `CHECK(framesRawTrue[60] < detectedOnsets)`
     (loss, expect ~64%) and `CHECK(framesRawTrue[120] > detectedOnsets)` (duplication,
     expect ~1.26x).
   - RED procedure with a runtime number: the Builder first writes the two REQUIREs against
     `framesRawTrue` (compiles on main, no helper) -> run -> FAIL with e.g. "26 == 40" at 60
     fps and "50 == 40" at 120 fps -> record -> switch the REQUIREs to `framesPulse` (helper)
     -> GREEN. This is the unit-level replica of the live probe and pins BOTH defect
     directions.

C. Eyes (pytest, `tests/visual/test_onset_pulse.py`, app launched
   `open --stdout F --stderr G <bundle> --args --test-mode` then
   `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_onset_pulse.py -v`; TestServer is on
   `http://[::1]:8080` -- `.harmony/gotchas.md:238-240, :348`). Helper
   `wait_pulses(app, target, timeout=1.0)` polls `app.state()['onset_pulse_frames']` every
   20 ms. Steps: `app.reset()`; `s0 = state()['onset_pulse_frames']` (RED on main:
   KeyError); `inject({"onsetDetected": True, "onsetStrength": 0.9})` -> == s0+1;
   `inject({"rms": 0.2})` (no onset key), sleep 0.3 s -> still s0+1 (no phantom from
   carried state); `inject({"onsetCount": 10})` -> s0+2 (delta 9 collapses to one pulse);
   `inject({"onsetCount": 10})` again, sleep 0.3 s -> still s0+2; `app.reset()`, sleep 0.3
   s -> still s0+2 (backwards jump = no pulse); `inject({"onsetDetected": True})` -> s0+3
   (count restarted at 1 after reset). The GL thread consumes on its next frame (test mode
   renders continuously at ~118 fps, gotchas `:376`), so 1 s is generous. Do NOT assert on
   PSNR for this feature (section 0, capture converges to steady state).

## 4. THREAD OWNERSHIP / NO LOCKS / NO ALLOCATION (summary the Reviewer can check)
- `Renderer::onsetPulse_`, `frameSnap_`: GL thread only (`renderOpenGL`, `renderSource`,
  `newOpenGLContextCreated` -- all GL-thread entry points). No atomics needed (same as
  `scaledTime_`, `lastDetectedGenre_`).
- `Renderer::onsetPulseFrames_`: `std::atomic<uint32_t>`, relaxed RMW on GL thread, relaxed
  load on HTTP threads (same discipline as `currentFps_` `:308/:317`).
- `OutputRenderer::onsetPulse_`: its own GL thread. `AudioReadoutPanel::onsetPulse_`: message
  thread. `RecorderHost::onsetCountBaseline_`: message thread (unchanged).
- `TestServer::lastInjectedOnsetCount_`: atomic, written under `injectMutex_` (HTTP threads
  only -- the existing test-mode mutex, `TestServer.cpp:46-51`), read by HTTP threads.
- ApiServer: no state (stateless multi-client). Analysis/audio threads: untouched.
- No heap: `OnsetPulse` is 8 bytes POD; `frameSnap_` is a member (one 320-byte copy per
  frame, which `featureBus_.read()` already produced as a local before).

## 5. LIVE CHECK (production REST, for Harmony's live-app agent -- NOT run by Architect)
Oracle: after a deterministic click train, `renderOnsetPulses` delta (frames that pulsed,
`/api/status`) must EQUAL `onsetCount` delta (`/api/features`). Pre-fix the two differ,
with the sign set by fps (below ~93.75 fps: fewer; above: more).
Script `.harmony/probe-onset-render.sh` (mirror `probe-step3.sh` conventions:
`ok/no/jget/perf_field` helpers `:112-160`, production launch `:173-177` incl. the TCC
mic-prompt caveat, PID via `pgrep -f 'MacOS/Audio-DN[A]'` -- never the unbracketed form,
rig rule). Steps:
0. `python3 .harmony/gen-click-wav.py /tmp/click_onset_30s.wav --duration-s 30`
   (stdlib only, no .venv; 60 bursts, one every 0.5 s; the -50 dBFS floor is what lets aubio
   fire -- generator docstring). Run from the MAIN checkout.
1. Launch the scratch/lane build in PRODUCTION mode (no --test-mode); wait `/api/health`
   on `http://127.0.0.1:7070`; wait 10 s (`totalBarCount` readable = analysis live, as
   `probe-step3.sh:181-184`). Preview must be VISIBLE (a detached GL context renders no
   frames -- gotchas `:235`; `render_frame` returning 200 in < 0.1 s is the attach oracle).
2. Load `.harmony/probe-step3.json` via `/api/load_composition` (a deck with clips, so the
   frame loop has content); additionally, for a human-visible witness, put Gravity Well in a
   cell (`sourceType: gravity_well`, Scatter param high) -- optional, the counter oracle
   does not need it.
3. `F0 = /api/features.onsetCount`, `S0 = /api/status.renderOnsetPulses`.
4. `POST /api/perf/record {"name":"onsetrender","audio":true,"audioFile":"/tmp/click_onset_30s.wav","onsetMarkers":true}`
   -- this loads the file, switches to File mode (mic no longer feeds analysis:
   `useInputForAnalysis` is set only in MicInput mode, `src/audio/AudioEngine.cpp:99-107`,
   INFERRED from the flag) and plays it (`src/MainComponent.cpp:2024-2031`, `:2058-2061`);
   the transport is non-looping (no `setLooping` anywhere in `AudioEngine.cpp`, VERIFIED),
   so onsets stop after 30 s.
5. Sample `/api/status.fps` at t=5 s, 15 s, 25 s (record; interprets the pre-fix sign).
   Sleep to t=34 s. `POST /api/perf/stop`. Sleep 2 s.
6. `F1`, `S1`. Read both again 1 s later; require both stable (no drift = playback ended).
7. PASS iff `(S1-S0) == (F1-F0)` AND `(F1-F0) >= 54` (>= 90% of 60 clicks, the probe-step3
   T2 tolerance). Informational: take.json onset markers (`~/Documents/Audio-DNA/Takes/onsetrender.adna-take/take.json`,
   `markers[].action == "onset"`) vs `F1-F0` within +-1 (RecorderHost's already-fixed
   consumer as a third witness; its baseline is set at arm, F0 is read just before).
Fail-first at the live level (recommended, one extra build of the same tree): commit A
adds ONLY `onsetPulseFrames_` counting frames where the RAW `snap.onsetDetected` is true
plus the two REST fields -> probe -> expect INEQUALITY (at ~118 fps: `S` delta > `F` delta
by ~26%; at 60 fps: < by ~36%). Commit B adds the pulse -> EQUALITY. If Harmony wants a
single build, the RecorderHost 115/133 evidence (db72d56 message) plus test B stand in for
the render-path RED; say so in the close-out rather than claiming a live RED.
Rig rules restated: scratch build dir or lane worktree, never `./build`; no debugger, no
GUI input, stop on any unexpected dialog; `git add -f` under `.harmony/`; check
`git show --stat HEAD`.

## 6. RISKS
- Strongest counterargument to my recommendation: "analysis-thread envelope, no consumer
  state" -- loses on layout (3 spare bytes), on stateful-sim duplication, and on fixing a
  render cadence problem in the wrong thread (section 1). Second: "let each uploader own
  its state" -- loses because uploaders race for one delta inside a frame and the
  per-source-clip uploader would starve all but the first clip.
- Perceptible behaviour change on Boris's ~118 fps rig: Gravity Well / Fluid Dynamics now
  get exactly ONE impulse per onset instead of 1-2 -> a slightly softer "kick" than the
  accidental double. Correct and display-rate-independent, but say it in the handoff so a
  "feels weaker" report is expected, not a regression. On 60 Hz displays the change is the
  opposite: ~36% MORE kicks (the lost ones).
- `u_onsetStrength` on recovered frames is 1-2 hops post-peak (2.7). If a shader author
  wants the peak, the follow-up is an analysis-side `onsetPeakStrength` held until the next
  onset: either a uint8 quantised value in the 3 spare bytes (offsets 313-315) or growing
  the struct to 384 (FeatureBus `kSnapshotWords` 80->96 + 4 static_asserts). Not now.
- The unit tests cannot link `Renderer`/`TestServer`/`ApiServer` (GL, httplib; precedent:
  `test_renderer_source_confinement` mirrors rather than links). The Renderer WIRING is
  therefore proven only by Eyes test C and live probe D; tests A/B prove the helper and
  the cadence model. Reviewer must read the three Renderer edits (2.2) by eye.
- Hoisting the read above the early return is a small reorder in a 1,900-line GL callback:
  the early-return branch must not start using `snap` (it does not today, `:219-229`).
- Test-mode stickiness change (2.6) could break a FUTURE Eyes test that PSNR-diffs an
  injected onset on Gravity Well / Fluid; today none does (VERIFIED grep of tests/visual for
  gravity/fluid: none). Documented in TESTING.md.
- `downbeatDetected` (`TopBar.cpp:232`, `AudioReadoutPanel.cpp:70`) has the identical
  one-hop-pulse class and is NOT covered. `totalBarCount` (monotonic, `FeatureSnapshot.h:51`)
  advances on the same events as bars and could drive a `BarPulse` the same way -- separate
  lane, not this one.
- Backwards-jump guard is the only heuristic in the helper; it misfires only if >= 2^31
  onsets occur between two reads (impossible: 1.45 years of continuous max-rate onsets to
  wrap once).
- Learning worth keeping (not logged: project-repo boot, disk fence): for any always-latest
  pulse consumer, the defect direction flips with the reader's rate relative to the writer's
  (loss below, duplication above); the oracle that survives both is an equality of two
  monotonic counters, and the probe must record the reader rate to interpret the RED sign.

STATUS: COMPLETE -- plan is Builder-executable with no open questions; live check and fail-first live RED are Harmony's to run (no app launch by Architect).
