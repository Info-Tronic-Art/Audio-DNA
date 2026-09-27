# s-rta-0926b lane bpm: Builder report

STATUS: DONE
RESULT: The race is proven and fixed. Before the fix, ThreadSanitizer reported 6 data races between `BPMTracker::setManualBPM` (running on the message thread) and the analysis hop. After the fix it reports 0. Every tempo writer now posts a lock-free request that the analysis thread applies at the start of its next hop, the same pattern Resync already used. A Link tick that re-sends an unchanged tempo no longer re-zeroes the phase about 30 times a second. Tap, set_bpm (including a same-value set_bpm) and Resync realign exactly as before, and this was checked live against the base build.
FACTS:
- RED under TSan on base (code 6e8f120; TSan build of only `test_bpm_stabilization`, `-DADNA_SANITIZE=thread`): the new `[tsan]` case printed `ThreadSanitizer: reported 6 warnings`, exit code 134. The conflicting accesses were `setManualBPM` at BPMTracker.cpp:554/557/559 against `updatePhase` :199/:207, `applyResync` :538 and `trackerState()` BPMTracker.h:100. (verified: scratchpad `tsan-red-base2.txt`)
- GREEN under TSan on the fix: `All tests passed (4 assertions in 1 test case)`, 0 reports, exit 0. Five repeats all had exit 0 and 0 reports. The full suite under TSan gave `All tests passed (202 assertions in 34 test cases)` with 0 reports. (verified: `tsan-rep1..5.txt`, `tsan-full.txt`)
- Deterministic RED on base (plain Release build-lane): `REQUIRE( tracker.bpm() == bpm0 )` failed `with expansion: 140.0f == 120.0f`. This means the message-thread call wrote analysis-owned state directly. (verified: `red-ctest-base.txt`)
- Link RED on base (the old Link call, `setManualMode(true); setManualBPM(120)` every 3.125 hops): `REQUIRE( jumps == 0 )` failed with `607 (0x25f) == 0`, message `Link ticks 608, phase jumps 607 (first at hop 4), bars 0`. (verified)
- GREEN plain: `[s-rta-0926b]` gave `All tests passed (34 assertions in 3 test cases)`. The whole test_bpm_stabilization binary gave `All tests passed (202 assertions in 34 test cases)`. (verified)
- Full serial ctest on build-lane: `100% tests passed, 0 tests failed out of 583` (580 before plus the 3 new cases). (verified: `ctest-full-fix1.txt`)
- Live runs on the build-lane app, each holding `/tmp/audiodna-live.lock` as `bpm`, released after each run: probe-manual-bpm `18 PASS / 0 FAIL`, probe-resync `16 PASS / 0 FAIL`, probe-downbeat-level `14 PASS / 0 FAIL`. (verified)
- `-DAUDIODNA_BUILD_LINK=ON` compile check in a scratch dir, target AudioDNA only: it fetched Link-3.1.2 and compiled MainComponent.cpp, BPMTracker.cpp and LinkSync.cpp with `AUDIODNA_HAS_LINK=1`. Result `BUILD EXIT 0` in 2m17s (configure plus build). The scratch dir has been deleted. (verified: `build-link.log`)
METHOD: I read BPMTracker in full, AnalysisThread stage 5, every `applyTempoCommand` caller and how its thread is reached (REST callAsync, OSC MessageLoopCallback, MIDI callAsync, key events, both 30 Hz timer paths), LinkSync, the TopBar tempo readers and every probe that posts set_bpm. I wrote the tests first and ran them RED on base (plain build and TSan build). Then I implemented the fix and ran GREEN (plain, TSan x5, full suite under TSan), the full serial ctest, the three live probes, an informational same-value set_bpm live check on both builds, and the Link-ON compile.
CONFIDENCE: high that the race is gone. The only state the message thread now touches is `manualMode_`, `resyncRequests_` and `tempoRequest_`, all of them atomics, and TSan is clean over the full suite. VERIFY: re-run the TSan case (commands below), the serial ctest and the three probes on the merged build.
UNKNOWNS / NOT DONE:
- The Link timer path was not exercised live. Link can only be switched on from the TopBar toggle (no REST or OSC route), and synthetic clicks are forbidden. It is covered at the tracker seam by ctest, and the Link-ON build was compile-checked only.
- A CHANGED Link tempo still realigns on every tick where it changed, for example during a peer's tempo ramp. That is today's behaviour, kept on purpose; see the open fork.
NUANCE:
- The request is applied at the START of `runPipeline`, before that hop's phase advance. The published snapshot is therefore identical to what the old direct write produced on its next hop: bpm equals the request, and beatPhase equals one hop's increment. Live, the same-value set_bpm read 60 ms later was 0.064-0.085 on the fix against 0.064-0.107 on base, so latency is unchanged within one hop.
- Requests that land between two hops coalesce. The last BPM wins, and a realign asked for by any of them is kept, so a Tap is never downgraded by a Link tick. Applied in sequence, the old code would have reached the same end state.
- `setManualMode` was already a relaxed `std::atomic<bool>` and is unchanged. It still takes effect on the hop that reads it, so a "manual" command can run one hop in manual mode at the old tempo before the new BPM lands. The old code had the same window.
HANDOFF-NEEDS: none. There are two open forks (Link ramp, and the default-build Link toggle) plus one doc inaccuracy, all listed below.

INBOX-RECHECK: none

## Every writer of tracker state off the analysis thread (all reach it on the MESSAGE thread)
All of them go through `MainComponent::applyTempoCommand` (src/MainComponent.cpp:5169). That is the only code that touches the tracker off the analysis thread. `getBpmTracker()` has no other caller.
| # | Writer | Call site | How it reaches the message thread | Tracker calls (after fix) |
|---|---|---|---|---|
| 1 | TopBar Tap button | MainComponent.cpp:657-659 ("tap") | JUCE button click | setManualBPM (request, realign) |
| 2 | TopBar Manual toggle / BPM field Return | :666-668 ("manual"/"auto") | JUCE UI | setManualMode(atomic) + setManualBPM (request) / setManualMode(false) |
| 3 | TopBar Resync | :670-672 ("resync") | JUCE UI | requestResync (already a request) |
| 4 | REST POST /api/set_bpm | :1937-1940 ("link") | httplib worker -> `MessageManager::callAsync` (ApiServer.cpp:711-716) | setManualMode + setManualBPM (realign, same value too) |
| 5 | REST POST /api/resync | :1941-1944 | callAsync (ApiServer.cpp:727-731) | requestResync |
| 6 | Take/routine replay, tempo control | :1995-1998 (any action, Origin::Replay) | RecorderHost::tick / RoutineEngine::tick from timerCallback (:3472, :3481) | as its action |
| 7 | OSC /audiodna/bpm | :2163-2166 ("link") | OSCReceiver MessageLoopCallback (OscHandler.h:30) | setManualMode + setManualBPM |
| 8 | OSC /audiodna/resync | :2167-2170 | MessageLoopCallback | requestResync |
| 9 | Ableton Link tick | :3542-3548 ("link", linkTick=true) | 30 Hz timerCallback | setManualMode + **followExternalTempo** |
| 10 | Tap binding (key/MIDI) | :7203 ("tap") | key events; MIDI via callAsync (MidiHandler.cpp:80-99) | setManualBPM |
| 11 | Resync binding | :7211 | same | requestResync |
Preset/deck/composition load: none of these write the tracker. `Composition::bpmMultiplier` is model state, not tracker state (grep of src/model).
Readers on the message thread: none read tracker internals. TopBar reads `displaySnap_` (the FeatureSnapshot). MainComponent reads `snap.*`, and ApiServer reads `featureBus_.read()`. `isManualMode()` is called only from tests.

## Synchronous-visibility check (item 3)
- The REST set_bpm response has always been returned before the message thread runs `onSetBpm` (callAsync), so its behaviour is unchanged.
- The old direct write also reached readers only at the next hop's FeatureSnapshot publish, so the observable latency is unchanged.
- Probes that post set_bpm (downbeat-level, resync, routines, tempo-silence, step3, manual-bpm, mastersignal, lane3) all sleep 0.3 to 2 s before reading. Nothing needed to change.
- The only code that assumed synchronous visibility was two existing unit assertions in test_bpm_stabilization.cpp (`bpm()` right after the call in the P24 cold-tap case, and `beatPhase()==0` right after the call in the s-rta-0926 Tap case). Both now read after the next hop. Both still pass on base, since the rewrites preserve their meaning.

## Link (item 4)
Verified from source: `timerCallback` (MainComponent.cpp:3542-3548) called `applyTempoCommand("link", linkBPM)` on every 30 Hz tick while `linkSync_.isEnabled()`. Each call ran `setManualBPM`, which set `phase_ = 0`, so the phase never passed about 0.07 and no bar landed (the RED ctest had 607 jumps and 0 bars).
- Fix: `BPMTracker::followExternalTempo`, used only by the Link tick through `applyTempoCommand(..., linkTick=true)`. An unchanged tempo (the folded request equals `lockedBPM_` at apply time) keeps the phase running. A changed tempo realigns as before.
- REST/OSC set_bpm and replay keep `setManualBPM`.
- The recorded tempo point is unchanged: still "link", with the same 0.01 BPM throttle.
**Not dormant in a default build (inferred from source, not driven live):** `LinkSync::setEnabled` stores `enabled_` unconditionally, and `getBPM()` returns the `bpm_{120.0}` default when `AUDIODNA_HAS_LINK` is undefined. The TopBar "Link" toggle is always visible (TopBar.cpp:117). So one click in a default build forced manual mode at 120 BPM and re-zeroed the phase 30 times a second. After this fix only the fake 120 BPM remains. docs/claude/performance-controls.md says "every LinkSync method compiles to a no-op", which is inaccurate.

## Files changed
- `src/analysis/BPMTracker.h`: setManualBPM now documented as a request; new `followExternalTempo`; private `tempoRequest_` (a `std::atomic<uint64_t>` with a static_assert that it is always lock-free), `kTempoPending`/`kTempoRealign`, `postTempoRequest`, `applyTempoRequest`.
- `src/analysis/BPMTracker.cpp`: `runPipeline` takes a pending request first (one relaxed load per hop, an exchange only when pending). `setManualBPM`/`followExternalTempo` post the request with a CAS loop. `applyTempoRequest` holds the old setManualBPM body, and zeroes the phase only if realign was asked or the tempo changed.
- `src/MainComponent.h`/`.cpp`: `applyTempoCommand(..., bool linkTick = false)`. The "link" branch calls `followExternalTempo` when linkTick is set, else `setManualBPM`, and the Link timer site passes `true`. Other actions are unchanged.
- `tests/test_bpm_stabilization.cpp`: 3 new `[s-rta-0926b]` cases, and 2 existing assertions moved after the next hop (see above).
- Nothing in src/render or src/recording was touched, and tests/CMakeLists.txt did not need to change (same target).

## Tests (RED -> GREEN, verbatim)
- The RED runs used the base tracker. The Link case's RED body called `setManualBPM(120)` per tick, which is exactly the base Link call. After the fix that one line became `followExternalTempo(120)`, and a changed-tempo tail was added. The concurrent case gained one `followExternalTempo` writer after RED, so that TSan also covers the new entry point.
- Plain base: `test cases:  3 | 1 passed | 2 failed` / `assertions: 10 | 8 passed | 2 failed`. The TSan case passes in a plain build; it can only go RED under TSan.
- TSan base: `ThreadSanitizer: reported 6 warnings`, EXIT CODE 134. TSan fix: `All tests passed (4 assertions in 1 test case)`, EXIT CODE 0.
- Serial ctest: `100% tests passed, 0 tests failed out of 583`.
- Re-run TSan: `cmake -S . -B build-tsan -DADNA_SANITIZE=thread -DCMAKE_BUILD_TYPE=Debug` (plus the FETCHCONTENT_SOURCE_DIR_* overrides), then `cmake --build build-tsan -j3 --target test_bpm_stabilization`, then `./build-tsan/tests/test_bpm_stabilization "[tsan]"`. This takes about 1 min. My build-tsan was deleted as scratch.

## Live gates (build-lane app = HEAD source; lock held per run, released after)
- `.harmony/probe-manual-bpm.sh` (MANUALBPM_BUILD_DIR=build-lane): `18 PASS / 0 FAIL`. M: 0 jumps, 1.999 beats/s, bars 1.98-2.016 s. R: first bar 2.040 s after the POST.
- `.harmony/probe-resync.sh` (RESYNC_BUILD_DIR=build-lane): `16 PASS / 0 FAIL`.
- `.harmony/probe-downbeat-level.sh` (DOWNBEAT_BUILD_DIR=build-lane): `14 PASS / 0 FAIL`.
- Informational, not a probe row: a same-value set_bpm realigns. The script sets manual 120, waits until beatPhase is in [0.40, 0.60], POSTs set_bpm 120 again, and reads the first poll at 60 ms or later.
  - Fix: `SUMMARY same-value set_bpm realigned 3/3 (control without a post: 0/3 read < 0.40)`.
  - Base app (/Users/boriskarpman/projects/RealTimeAudio/build/..., 6e8f120): `SUMMARY same-value set_bpm realigned 3/3 (control without a post: 0/3 read < 0.40)`.
- No render claims are made, so no frames were decoded. No Output window was opened (each probe's Quartz check passed). No full-screen capture, no debugger, no synthetic input.

## Found, not fixed (outside the fence)
1. Default build: the Link toggle works without Link compiled and fakes 120 BPM (src/sync/LinkSync.cpp, src/ui/TopBar.cpp). Suggested fix: hide the toggle, or make `isEnabled()` false, when `!AUDIODNA_HAS_LINK`.
2. docs/claude/performance-controls.md "every LinkSync method compiles to a no-op" is wrong (see 1). docs/claude/analysis.md and effects.md have no statement made stale by this change.
3. tests/test_downbeat_detector.cpp:192 has the comment "lockedBPM_ = 120, phase_ = 0 ..." right after `setManualBPM`. It is now applied at the first hop and the test is unaffected. The comment is slightly stale; I left it alone as a surgical choice.

## Open forks
- LINK-RAMP: should a CHANGED Link tempo realign the phase? Options: (a) yes, which is today's behaviour and kept: every tick of a peer's tempo ramp re-zeroes the phase. (b) Never: Link ticks only set the tempo and the phase stays continuous. I recommend (b). The reset is not aligned to Link's beat anyway (LinkSync's beat phase is unused), so it is always an arbitrary jump. The switch is `applyTempoRequest`'s `realign || tempoChanged`.
- The same-value set_bpm product question stays with Boris. The switch point is the realign bit that `setManualBPM` posts.

## Notebook note (for Harmony to fold; shared file not edited by this lane)
`## 2026-09-26 s-rta-0926b bpm-thread | Files: src/analysis/BPMTracker.{h,cpp}, src/MainComponent.cpp applyTempoCommand | setManualBPM/followExternalTempo are REQUESTS (one lock-free 64-bit word, applied at the start of the next runPipeline), like requestResync. A test that reads bpm()/beatPhase() right after the call sees the OLD values; do one hop first. TSan on a single test target is ~1 min: -DADNA_SANITIZE=thread, --target test_bpm_stabilization. Valid while BPMTracker keeps the request word.`

## Packet quality
- Clarity: CLEAR.
- Missing context: (1) the packet called Link dormant, but it is reachable by one click in a default build; (2) `cmake/Sanitizers.cmake` already provides the TSan variant (the packet did not say so); (3) FETCHCONTENT_SOURCE_DIR_SYPHON is also needed to stay offline.
- Unused context: none.
- Self-brief files: analysis.md, performance-controls.md, manual-bpm.md, review-manual-bpm-r1.md and the notebook's s-rta-0926 entries all existed and were useful. The performance-controls.md Link note is stale (see above).
- Deviations: none from the fence. The build took about 2 minutes, well under the 15 minute limit for the Link compile.
