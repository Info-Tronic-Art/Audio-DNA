# btguard-fix lane report (s-rta-0929b, fix round 1)

STATUS: PARTIAL
RESULT: I checked all 7 critic findings against the code, and each one holds. Every "survives" claim was reproduced by running a mutant. The unit-test findings are fixed: 6 new ctest cases plus 3 changed ones, and each fails on the mutant it guards (8 of 8 mutants killed). The notice wording is fixed, and probe rows B3 / B3b / C2 are RED on the pre-fix app. I added probe row C5b (the no-device take is saved without audio). The live part then STOPPED at a macOS dialog, as the rig rules require. The clamp-removed mutant app was a re-signed scratch copy, and it raised the system microphone-permission prompt ("Audio-DNA would like to access the microphone"). That prompt is STILL ON BORIS'S SCREEN. Because of that stop, probe-btguard was never run GREEN on the fixed build-lane app, and C5b has no RED yet.
FACTS:
- Commits on lane/btguard after 647ad69: 3340697 (test: device-policy cases), 0c849f3 (fix: notice wording), 90f4854 (test: probe C5b), plus the docs commit that carries this report. (`git -C <WT> log --oneline 647ad69..HEAD`)
- `build-lane/tests/test_device_policy`: "All tests passed (201 assertions in 36 test cases)". It passed 5 of 5 runs, with load average 8.11 during the timing runs.
- `ctest --test-dir build-lane -j1`: "100% tests passed, 0 tests failed out of 1003" (997 + 6 new cases).
- Mutation table (scratch `mutate.py`). The harness compiles COPIES of the sources and relinks the test binary from build-lane's objects and link.txt, so the worktree is never mutated. The "old suite" is 647ad69's test file; the "new suite" is this round's. See the Gates section.
- Probe on the pre-fix app (a byte copy of build-lane's 647ad69 app, sha256 f3d808ba...): "PROBE-BTGUARD: 22 PASS / 3 FAIL / 2 SKIP (SKIP never counts)". The 3 FAILs are exactly B3, B3b and C2, because each expects the new sentence. C5b PASSes there, since the clamp is present in that app.
- Probe on the clamp-removed mutant copy: "PROBE-BTGUARD: 0 PASS / 6 FAIL / 2 SKIP". No arm ever answered /api/health. The cause was the TCC microphone prompt, captured window-only as `unc-dialog-29879.png` in scratch (UserNotificationCenter window 29879, 260x234 at 734,216). There was no new Audio-DNA .ips, so this was not a crash.
- The new binary carries both sentences: `strings` on build-lane's Audio-DNA shows "No audio device found - plug one in. Bluetooth is never used." and "No wired mic found - plug one in. Bluetooth is never used."
METHOD: For each finding I read the code it names. I then reproduced the survival claim with a mutant built from a source copy, and ran the lane's old test file (30 cases) against each mutant. Next I wrote the case the critic proposed, and checked it RED on that mutant and GREEN on the real code. The live rows followed the RIG RULES.
CONFIDENCE: HIGH for the unit-test fixes: every new case was shown RED on its mutant and GREEN on the real code. MEDIUM for the wording: the pre-fix RED is live, but the GREEN is only static (strings plus the probe constants) and was not run live. LOW for C5b's teeth: it has no RED yet.
VERIFY: `python3 <scratch>/mutate.py`; `build-lane/tests/test_device_policy`; under the lock, `bash .harmony/probe-btguard.sh` against build-lane (GREEN expected 25 PASS / 0 FAIL / 2 SKIP).
UNKNOWNS / NOT-DONE:
- The microphone-permission prompt window is still on screen as of 02:19:54. I did not touch it, per the no-synthetic-input rule. Harmony / Boris must answer it. It is not known whether the answer is stored against com.audiodna.app in general, and so could change the real app's microphone permission, or only against the mutant's signature (inferred risk, not verified).
- probe-btguard GREEN on the fixed build-lane app: NOT RUN (0 of 5 runs). I stopped live work at the dialog.
- C5b RED: NOT RUN. The mutant must be rebuilt without a new signature, or run on a rig where the prompt cannot appear, before C5b can be shown load-bearing.
- The real-HAL aggregate branch (`membersReadOk` forced true) is NOT covered. Covering it needs a real aggregate device, and creating one is a HAL/system change. The policy side is covered by P3b.
- Video recording start / stop with no device (BG7) is NOT gated live: there is no REST route. Harmony must accept this explicitly (DONE_WITH_CONCERNS item), as the critic asked.
NUANCE: C5 could never be RED: /api/perf/record answers 200 before the message thread arms (ApiServer.cpp handlePerfRecord, callAsync). That is why C5b reads take.json and /api/perf/status. On the mutant, AudioTap::start would have been called with rate 0 / 0 channels. What JUCE's WAV writer does with that is unverified, because the run never got there.
HANDOFF-NEEDS: (1) Answer or clear the microphone prompt on Boris's screen (Boris; no synthetic input). (2) Run probe-btguard GREEN on build-lane in Harmony's behavioural gate. (3) Decide how C5b gets its RED. (4) Accept the video-recording gap. (5) Append the notebook notes below.
INBOX-RECHECK: none

## Items

| id | severity | verdict | fixed | evidence |
|---|---|---|---|---|
| F1 default-index plumbing untested | MUST | VERIFIED (mutants MA / MB: old suite 30/30 green) | yes (3340697) | D1 changed (the inner default 1/0 != the filtered default 0/1), new M6 (critic's X1). New suite: MA FAILs D1 D4 M6 M7, MB FAILs D1 M6 |
| F2 decorator default index not pinned | SHOULD | VERIFIED (MA) | yes (3340697) | D4 (USB macOS default after the hidden headset: filtered 1/0 vs inner 2/1) + M7 (the real manager opens USB both ways) |
| F3 R1 cannot see a missing settle timer | SHOULD | VERIFIED (MC: old suite 30/30) | yes (3340697) | R1: `pump(100)` then `CHECK(reapplies()==0)`. MC FAILs R1 |
| F4 new-scan gate and HAL-race re-scan untested | SHOULD | VERIFIED (MD, ME: old suite 30/30) | yes (3340697) | R4b (50 ms bound; 1 after its own messages, 2 after a new scan). D5 (the first enumeration misses the USB -> the re-scan maps it). MD FAILs R4b, ME FAILs D5 |
| F5 real-HAL enumeration weakly covered | SHOULD | VERIFIED for the default flags (MF / MF2) and getIndexOfDevice (MG); membersReadOk NOT testable without an aggregate | partly (3340697) | E1: exactly one flagged default per direction == JUCE's `getDeviceNames(d)[getDefaultDeviceIndex(d)]`. D6: filtered index 0 vs inner 1. MF / MF2 FAIL E1, MG FAILs D6 |
| F6 BG7 clamp not shown load-bearing; video gap | SHOULD | VERIFIED (C5 checks only async 200s: ApiServer.cpp handlePerfRecord) | partly (90f4854) | C5b added, PASS on the pre-fix app. Clamp-mutant run BLOCKED by the TCC prompt (0/6/2S). Video gap for Harmony to accept |
| F7 notice gives no action | SHOULD | VERIFIED (MainComponent.cpp:3080-3081) | yes (0c849f3) | "... found - plug one in. Bluetooth is never used." Pre-fix app: B3 / B3b / C2 FAIL (RED). GREEN live NOT RUN |

## Gates (raw lines)

Mutation table (`<scratch>/btguard-fix/mutation-table.txt`):
```
MA-default-passthrough     old-suite cases 30  FAIL 0  PASS 30  failing: -
MA-default-passthrough     new-suite cases 36  FAIL 4  PASS 32  failing: D1 D4 M6 M7
MB-no-builtin-rung         old-suite cases 30  FAIL 0  PASS 30  failing: -
MB-no-builtin-rung         new-suite cases 36  FAIL 2  PASS 34  failing: D1 M6
MC-no-coalescing           old-suite cases 30  FAIL 0  PASS 30  failing: -
MC-no-coalescing           new-suite cases 36  FAIL 1  PASS 35  failing: R1
MD-no-seq-gate             old-suite cases 30  FAIL 0  PASS 30  failing: -
MD-no-seq-gate             new-suite cases 36  FAIL 1  PASS 35  failing: R4b
ME-no-rescan               old-suite cases 30  FAIL 0  PASS 30  failing: -
ME-no-rescan               new-suite cases 36  FAIL 1  PASS 35  failing: D5
MF-default-out-flag-false  old-suite cases 30  FAIL 0  PASS 30  failing: -
MF-default-out-flag-false  new-suite cases 36  FAIL 1  PASS 35  failing: E1
MF2-default-in-flag-false  old-suite cases 30  FAIL 0  PASS 30  failing: -
MF2-default-in-flag-false  new-suite cases 36  FAIL 1  PASS 35  failing: E1
MG-index-unfiltered        old-suite cases 30  FAIL 0  PASS 30  failing: -
MG-index-unfiltered        new-suite cases 36  FAIL 1  PASS 35  failing: D6
REAL                       old-suite cases 30  FAIL 0  PASS 30  failing: -
REAL                       new-suite cases 36  FAIL 0  PASS 36  failing: -
```
Mutants: MA `getDefaultDeviceIndex` returns `inner_->getDefaultDeviceIndex(forInput)`; MB the built-in rung removed from `filter`; MC `startTimer(settleMs_)` replaced by `timerCallback()`; MD the `lastReappliedSeq_` gate made `if (false)`; ME rebuild's unmapped re-scan made `if (false)`; MF / MF2 `isDefaultOutput` / `isDefaultInput` forced false; MG `getIndexOfDevice` returns the inner index.

- test_device_policy (real): `All tests passed (201 assertions in 36 test cases)` x5
- ctest: `100% tests passed, 0 tests failed out of 1003`
- probe-btguard, pre-fix app (RED), 02:12:23: `PROBE-BTGUARD: 22 PASS / 3 FAIL / 2 SKIP (SKIP never counts)` with
  `FAIL  B3 the notice reads "No wired mic found - plug one in. Bluetooth is never used."  [{"ok": true, "file_label": "No file loaded", "audio_notice": "No wired mic found. Bluetooth is never used."}]`,
  B3b FAIL (same old notice after the file -> input label write), `FAIL  C2 the notice reads "No audio device found - plug one in. Bluetooth is never used."`,
  `PASS  C5b the no-device take was saved WITHOUT audio and the recorder reports no error`, C3 `decoded non-black (1920x1080 mean 108.4)`, A9 / B6 / C6 quit clean 0 dialogs.
- probe-btguard, clamp mutant, 02:13:12: `PROBE-BTGUARD: 0 PASS / 6 FAIL / 2 SKIP (SKIP never counts)`: `FAIL  A0 app never answered /api/health`, `FAIL  A9 quit:still running 31 s after the quit (killed) UserNotificationCenter windows on screen: 1` (the same for B / C).
- probe-btguard GREEN on build-lane: NOT RUN (stopped at the dialog).
- Batch: 0 Bluetooth-connected lines (system_profiler); store assets created: none; outwins "audio-dna windows 0, Output-named 0"; lock released 02:18:17.

## Deviations / errors

- The clamp mutant was a relink of build-lane's objects with a mutant MainComponent.cpp object, copied into a scratch app bundle and re-signed with `codesign --force --sign -`. That new ad-hoc signature (a new cdhash) made macOS ask for microphone permission. The byte-identical copy of the pre-fix app at a scratch path did NOT prompt. So the prompt comes from the signature, not the path (inferred from this pair of runs). I stopped all live work at that point, as the rules require. The GREEN batch script (`batch2.sh`) is written but was never run.
- The first commit attempt was blocked by the stash-guard hook, which read `git add .harmony/...` as a whole-tree add. I re-ran it with absolute paths. Nothing was staged by the blocked attempt.

## Notebook notes (for Harmony to append)

- 2026-09-30 A re-signed app copy raises the macOS microphone prompt | Relinking a mutant binary into a copied bundle and running `codesign --sign -` gives a new cdhash, and launching it pops "Audio-DNA would like to access the microphone" (UserNotificationCenter), which hangs the app before /api/health. A byte-identical bundle copy does not prompt. A live mutant of app code must not be re-signed; ask Harmony first | discovered: btguard-fix clamp-mutant run 02:13.
- 2026-09-30 Unit mutation harness without touching the tree | Take compile_commands.json's test-target command for the source, compile a COPY (add `-I<src dir>` for its quoted includes) to a scratch .o, and relink with `tests/CMakeFiles/<t>.dir/link.txt`, giving relative objects absolute paths and swapping in the mutant .o. That is about 20 s per mutant, and the old test file can be compiled the same way for a before/after table | discovered: btguard-fix mutate.py.
- 2026-09-30 /api/perf/record is async | It answers 200 before the message thread arms, so a refused or wrong take is visible only in /api/perf/status lastError and the saved take.json | discovered: src/api/ApiServer.cpp handlePerfRecord.

## PACKET QUALITY

- Clarity: CLEAR.
- Missing context: the rules did not say that a re-signed app copy triggers a TCC prompt. The rules' "Unexpected system dialog: STOP" applied.
- Unused context: none.
- Self-brief files: btguard.md (lane report) and critic-btguard-r1.md were useful. JUCE AudioDeviceManager source was read to confirm that a re-apply does not re-scan (so the seq gate is meaningful).

STATUS: PARTIAL
