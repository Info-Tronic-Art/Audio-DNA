## BUILDER REPORT -- lane bt2, stage K (s-rta-1002)

STATUS: DONE_WITH_CONCERNS
RESULT: Items I1-I6 landed on lane/bt2 as four commits, K1 c1d88f8, K2 759a5ea, K3 d0a8822 and K4 c4a5938, followed by this report's commit. Every gate this stage owns came out exactly as pre-registered. The concerns are three small test-text deviations, a ctest flake that comes from another lane, and a target-count method mismatch (see NUANCE).
FACTS: The four reconcile rows are in `src/audio/DevicePolicy.cpp:162`. The reconciler's single device choice and its gates are in `src/audio/DeviceGuard.cpp:178` and `src/audio/DeviceGuard.cpp:188`. C3 is in `src/audio/AudioEngine.cpp:153`, and the launch now goes through `src/audio/AudioEngine.cpp:28`. The TEST-ONLY routes are at `src/api/ApiServer.cpp:2093` and `src/api/ApiServer.cpp:2121`. The Mic label and the notice are at `src/MainComponent.cpp:493` and `src/MainComponent.cpp:3091`. The tests are in `tests/test_device_policy.cpp:1039` and `tests/test_audio_engine_devices.cpp:74`. The gate runners and outputs are in `.harmony/.reports/s-rta-1002/bt2-gates/gate3-red.sh`, `.harmony/.reports/s-rta-1002/bt2-gates/gate4-mut.sh`, `.harmony/.reports/s-rta-1002/bt2-gates/gate3.out` and `.harmony/.reports/s-rta-1002/bt2-gates/gate4.out`. The live results are in `.harmony/.reports/s-rta-1002/bt2-gates/live-k1/results.txt`, `.harmony/.reports/s-rta-1002/bt2-gates/live-k2/results.txt` and `.harmony/.reports/s-rta-1002/bt2-gates/live-green/results.txt`.
METHOD: Each item was built in commit order K1 to K4. Every commit got a normal cmake build in build-lane and a full serial ctest. The live probe-btguard runs used the live lock, and AM8's tripwire ran for every run window. GATE-3 ran in a clean worktree of the base 2d38b39 under scratch: the lane's cases were extracted verbatim and the base's src/ was left untouched. GATE-4 ran on a clean worktree copy of c4a5938: each mutant was a scripted edit, rebuilt by cmake, run with `-r junit`, then reverted and diff-checked. Both worktrees were removed afterwards.
CONFIDENCE+VERIFY: High. Harmony can re-run every gate as follows. GATE-3: `bash .harmony/.reports/s-rta-1002/bt2-gates/gate3-red.sh <wt> 2d38b39 <scratch> <main>/build/_deps` should print "GATE-3: AS PREDICTED". GATE-4: `bash .harmony/.reports/s-rta-1002/bt2-gates/gate4-mut.sh <wt> c4a5938 <scratch> <deps>` should print "13 / 13 KILLED". ctest: `ctest --test-dir build-lane -j1` should give 1077 of 1077. GATE-6 is 5 runs of probe-btguard on merged main, expecting 39 PASS / 0 FAIL / 2 SKIP each time.
UNKNOWNS/NOT-DONE: The following belong to Harmony after the merge: GATE-6 (5 of 5), GATE-8 (audio battery), GATE-9 (production absence; I ran an unofficial check, below) and GATE-10. Nothing in the hardware path (a real unplug, a dead combiner's close of up to 2 s) is proven on this rig. It is covered only by reading JUCE source and by the mock cases, and it remains Boris check B2.
NUANCE: (1) Three test-text deviations, all forced by the Harmony constraint "a manager test whose fixture contains kDenied ENDS with CHECK_FALSE(rig.spy.saw(kDenied))": in R15 the last two CHECK_FALSE lines swapped order; in R16 its kDenied check moved to the end; R17 gained a final kDenied check. GATE-3 is unchanged (R15 and R16 still FAIL, R17 still PASSes on the base). (2) The K1 ctest had 2 failures, test_preset_manager T3 and T5. Both passed 5 of 5 reruns, and K2, K3 and the final ctest were 100%. The INFERRED cause is the fixed `$TMPDIR/preset_manager_test_*.json` filename those tests use, which collides with lane tsan's ctest running at the same time. (3) Target count: measured 109 to 110 `catch_discover_tests(` lines and 110 to 111 distinct ctest executables. The plan's 111 to 112 uses another convention. Either way it is +1.
HANDOFF-NEEDS: none

### SUMMARY
- C3 is done: the app opens the audio device once per launch, and switching between file and mic never touches the device. Live, the opens counter reads 1 after launch and still 1 after file -> input. The pre-change build read 2 and 4.
- When a device appears after launch, the app now adopts it in all three situations: it had no device at all, it was running output-only, or its open mic was lost or stopped.
- A working mic is never switched away from.
- If a mic is replaced, the notice names both mics, and the Mic label follows each re-apply.
- These are TEST-ONLY: `audio_deny` and `audio_stop` drive the live rows.
- Results: GATE-3, GATE-4 and GATE-5 (K1, plus the SHOULD run on K2) all came out exactly as pre-registered, and the lane GREEN run was 39 / 0 / 2.

### ITEMS (commit order; every commit built clean, with no new warning in a touched file, and passed a full serial ctest)
- **K1 c1d88f8 (I1 + AM14). TEST-ONLY runtime deny / stop and the probe rows. No behaviour change.**
  - `GuardedDeviceType::setTestDeniedNames` and `GuardedAudioDeviceManager::setTestDeniedNames`.
  - `AudioEngine::debugSetDeniedDevices` and `debugStopDevice`.
  - `POST /api/debug/audio_deny {"names": [...]}`: returns 400 unless `names` is an array of strings (`[]` is allowed) and 503 when not wired; the call goes through callAsync and is answered at once.
  - Bodyless `POST /api/debug/audio_stop` (Pitfall 31).
  - MainComponent wiring sits inside the existing `#if AUDIODNA_TEST_SERVER` block.
  - The orphaned asyncload comment moved down to handleDebugCancelLoad (r2 NIT-3).
  - Probe changes:
    - A6 and A6b are now gated rows.
    - New rows: A10, A11a, A11, A12 (AM4 delta bar, 0.15 s gap guard, one retry), INFO A6c, A13a, A13, B7, B7b, B8, C7, C8a and C8.
    - The header carries AM18's policy-only label and AM14's audio_stop line.
  - ctest 1053 / 1055. The two failures are the test_preset_manager flake (ISSUES); those tests passed 5 of 5 reruns.
- **K2 759a5ea (I2 + AM1 + AM16). C3.**
  - Removed both `setAudioDeviceSetup(setup, true)` re-opens from `setSourceMode`.
  - Added the C3 comment. Kept both stderr lines.
  - The `perfRecord` comment now says a mode change never restarts the device.
  - DevicePolicy classify accepts the `"*"` deny-all token, inside the existing TEST-ONLY branch.
  - NEW target `test_audio_engine_devices` (AE1 and AE2) was added directly after the test_device_policy block; AE2 reads `AUDIODNA_SRC_DIR`.
  - test_device_policy changes:
    - `appOpenSequence` became `appLaunch` (initialise only), used by M5b and R1-R4b.
    - M5 keeps its explicit-settings step inline and was retitled.
    - M5b was retitled.
    - New cases: M5c, P15 and E3.
    - `removeDevice` moved into the fixture prelude.
  - ctest 1060 / 1060.
- **K3 d0a8822 (I3 / I4 / I5 + AM12 / AM13 / AM15 / AM17). The reconciler.**
  - `devpolicy::reconcile` and `toString`, as specified in AM13. The rows are, in order: NoDevice, InputLost, DeviceStopped, AdoptInput, None.
  - `DeviceReconciler`:
    - `openDefaultDevices()` records the scan only after the open.
    - The settle restarts on every change message.
    - It never acts twice on the same scan, and re-applies at most once per 10 s.
    - AM12 progress guard (`noInputOn_`).
    - A failed re-apply restores the still-listed half of the previous device.
    - New accessors `lostInput()` and `lastAction()`.
    - stderr line: `[AudioEngine] re-applying the device policy (<code>)`.
  - `AudioEngine`:
    - The launch now opens through `openDefaultDevices()`.
    - New `onDevicesReapplied` callback.
    - `DeviceState::MicReplaced` and the static `deviceStateFor` table.
    - New accessors `openInput()` and `lostInput()`.
    - The TEST-ONLY status gains `last_reapply`, `lost_input` and the state `mic-replaced`.
  - MainComponent:
    - New `onDevicesReapplied` lambda: on success, while the source is the mic, it writes `"Mic: " + getDeviceStatus()` to the label.
    - New MicReplaced notice: `Mic "<lost>" lost - now listening on "<new>".`
  - test_device_policy: added M5r, M8, P14 and R6-R18, plus the mock `stopKeepingCallback` and `startWithKeptCallback`.
  - New AE3 case.
  - ctest 1077 / 1077.
- **K4 c4a5938 (I6 + AM20 + AM6 first bullet). Docs.**
  - Pitfall 61:
    - The replacement paragraph and the Guards append are copied verbatim from AM20.
    - The AM6 by-name exception sentence was added.
  - testing-eyes.md:19: the audio_deny / audio_stop text appended.
  - architecture.md item 4: one line appended.
  - APP-INVENTORY: the row 31 counts and the btguard paragraph line.
  - GATE-11:
    - "The startup `setSourceMode` re-open stays" occurs 0 times.
    - Each of these occurs exactly once: the AM6 exception sentence; "the open device is STOPPED and nothing restarted it"; "tried once per distinct list of allowed inputs"; "lost - now listening on"; "M5r (JUCE's own XML-branch re-init".
    - testing-eyes.md contains audio_stop, device-stopped and lost_input.
    - CLAUDE.md is 23,996 B, unchanged.

### GATES (raw lines verbatim)
**GATE-3: unit RED on the base 2d38b39 (AM5 method).** The src/tests of 2d38b39 are identical to 02b2913 (`git diff --stat 02b2913 2d38b39 -- src tests` is empty). Output from `bt2-gates/gate3.out`:
```
diff -r lane vs TU (test_device_policy): empty
diff -r lane vs TU (test_audio_engine_devices): empty
base src/ untouched: git diff --stat -- src = ''
AE1 FAIL 3 | AE2 FAIL 1 | M5c PASS 0 | M5r PASS 0 | R13 FAIL 2 | R14 FAIL 2 | R15 FAIL 5 | R16 FAIL 3 | R17 PASS 0 | R6 FAIL 2 | R7 FAIL 3 | R8 FAIL 2 | R9 PASS 0
test cases: 11 |  4 passed |  7 failed
assertions: 80 | 61 passed | 19 failed
test cases:  2 | 2 failed
assertions: 11 | 7 passed | 4 failed
FAIL set: {AE1 AE2 R13 R14 R15 R16 R6 R7 R8}   predicted {AE1 AE2 R13 R14 R15 R16 R6 R7 R8}
PASS set: {M5c M5r R17 R9}   predicted {M5c M5r R17 R9}
GATE-3: AS PREDICTED
cleanup: worktree + build dir removed
```
The failed-assertion counts match the earlier rulings' measurements: R14 2, R15 5, R16 3, AE1 3, AE2 1, R13 2, and R6 + R7 + R8 = 7.

**GATE-4: mutation table on the lane's real code (c4a5938).** Baseline of the unmutated copy: 58 cases, none failing. After all mutants were reverted: 58 cases, none failing.
| # | mutant | file | lines | kill set | failing cases | verdict |
|---|---|---|---|---|---|---|
| 1 | no AdoptInput | DevicePolicy.cpp | +1 -1 | {R6 R11 R12 R13 P14} | P14 R11 R12 R13 R15 R6 | KILLED |
| 2 | no InputLost | DevicePolicy.cpp | +1 -1 | {R8 R12 P14} | P14 R12 R14 R18 R8 | KILLED |
| 3 | switch a working mic | DevicePolicy.cpp | +1 -1 | {R9 R2} | R14 R18 R2 R8 R9 | KILLED |
| 4 | no restore | DeviceGuard.cpp | +1 -1 | {R11 R14} | R11 R14 R15 | KILLED |
| 5 | launch seq not recorded | DeviceGuard.cpp | +1 -1 | {R10 R11 R4b} | R10 R16 R4b | KILLED |
| 6 | no settle | DeviceGuard.cpp | +1 -1 | {R1 R13 R17} | R1 R13 R16 R17 (54 of 58 reported: segfault in R17) | KILLED |
| 7 | no progress guard | DeviceGuard.cpp | +1 -1 | {R15 R11} | R11 R15 | KILLED |
| 8 | restore without the still-listed filter | DeviceGuard.cpp | +2 -2 | {R14} | R14 | KILLED |
| 9 | no stopped-device row | DeviceGuard.cpp | +1 -1 | {R16} | R16 | KILLED |
| 10 | DeviceStopped exempt from the scan gate | DeviceGuard.cpp | +1 -1 | {R16} | R16 | KILLED |
| 11 | lostInput never set | DeviceGuard.cpp | +1 -1 | {R18} | R18 | KILLED |
| 12 | setSourceMode's re-open restored | AudioEngine.cpp | +12 -0 | {AE1 AE2} | AE1 AE2 | KILLED |
| 13 | a re-open guarded by getCurrentAudioDevice() | AudioEngine.cpp | +1 -0 | {AE2} | AE2 | KILLED |
`GATE-4: 13 / 13 KILLED`. The scratch worktree's `git diff` was empty after every revert. On mutant 6, test_device_policy segfaulted in R17: with no settle, the re-apply destroys the device R17 holds a raw pointer to. Only the mutant can reach that state. R1 and R13, both in the kill set, had already failed before the crash.

**GATE-5: live RED of record on the K1 build, c1d88f8** (`bt2-gates/live-k1/`). 08:44:46-08:46:26, MacBook Pro Microphone + Speakers (built-in).
`PROBE-BTGUARD: 27 PASS / 12 FAIL / 2 SKIP (SKIP never counts)`. The FAIL set is exactly {A6 (reads 2), A6b (4), A10, A11a, A11, A13, B7, B7b (3), B8, C7, C8a, C8}. A12 PASSes ("0.001 s apart -> still 0 re-applies") and A13a PASSes.
Tripwire: `AUTHREQ_PROMPTING for audiodna: 0`; kTCCServiceMicrophone (authValue 2) x14; `UNC windows (kCGWindowListOptionAll, >= 15 s after the last quit): 0`. Every quit was clean (A9 / B6 / C6: no .ips, 0 dialogs).

**SHOULD: K2 build, 759a5ea** (`bt2-gates/live-k2/`). 08:49:28-08:51:08.
`PROBE-BTGUARD: 29 PASS / 10 FAIL / 2 SKIP`. The FAIL set is K1's minus {A6, A6b}; B7b reads 1. AUTHREQ_PROMPTING 0, UNC 0. Three clang processes from the other lane were running at the start (not a perf gate).

**Lane GREEN on the final build** (c4a5938; K3 binaries, K4 changed docs only) (`bt2-gates/live-green/`). 08:56:38-08:58:02.
`PROBE-BTGUARD: 39 PASS / 0 FAIL / 2 SKIP (SKIP never counts)`. A6 reads 1, A6b reads 1, INFO A6c = 3, A12 "0.000 s apart -> still 2 re-applies", A13 opens 3 -> 4. AUTHREQ_PROMPTING 0, UNC 0, and 0 Output-named windows.
The app's stderr shows the expected sequence:
```
TEST-ONLY audio_deny: MacBook Pro Microphone
re-applying the device policy (input-lost)
audio devices: input none, output "MacBook Pro Speakers" (bltn)
TEST-ONLY audio_deny: none
re-applying the device policy (adopt-input)
TEST-ONLY audio_stop: stopped "MacBook Pro Speakers" (the manager keeps it)
re-applying the device policy (device-stopped)
```
I looked at the C3 sample frame (1920x1080 plasma): it shows a colour field, not black. The 5-of-5 GATE-6 runs are Harmony's.

**ctest**
- `ctest -N`: Total Tests: 1077. That is 1055 base + test_device_policy 19 (M5c, M5r, M8, P14, P15, R6-R18, E3) + test_audio_engine_devices 3 (AE1-AE3).
- Final serial run on c4a5938:
  - `100% tests passed, 0 tests failed out of 1077`
  - `Total Test time (real) =  99.80 sec`
- Target counts:
  - Textual `catch_discover_tests(` lines in tests/CMakeLists.txt: 109 on the base, 110 on the lane.
  - Distinct ctest executables: 110 on the base, 111 on the lane.
  - The plan's "111 to 112" uses another convention. Either way it is +1 target.

**Unofficial self-checks.** GATE-9 and GATE-10 are Harmony's; these were not run as those gates.
- `git diff 2d38b39..HEAD -- src/audio/CombinedCallback.h src/audio/AudioCallback.cpp`: empty.
- No added std::mutex, lock_guard or scoped_lock lines in src.
- No touched file is outside the bt2 fence.
- AE, DG, DP, ApiServer and MainComponent compiled from compile_commands with `-DAUDIODNA_TEST_SERVER=1` removed:
  - All five build.
  - `strings` shows 0 hits for each of audio_deny, audio_stop, ADNA_AUDIO_DENY_DEVICES, setTestDeniedNames, debugSetDeniedDevices and debugStopDevice.
  - `nm` shows 0 matching symbols.
  - Positive control: the TEST_SERVER build's ApiServer object has 2 audio_deny strings.

### FILES CHANGED (2d38b39..c4a5938)
- src/audio/DevicePolicy.h / .cpp: the `"*"` TEST-ONLY token (K2); `Reapply`, `reconcile` and `toString` (K3).
- src/audio/DeviceGuard.h / .cpp: `setTestDeniedNames` on the type and the manager (K1, TEST-ONLY); DeviceReconciler rewritten (K3); its class comment now states the four cases and the gates.
- src/audio/AudioEngine.h / .cpp: the debug hooks (K1, TEST-ONLY); C3 (K2); `openDefaultDevices` at launch, `onDevicesReapplied`, MicReplaced, `deviceStateFor`, `openInput`/`lostInput`, and the status fields (K3).
- src/api/ApiServer.h / .cpp: the two TEST-ONLY routes and handlers, plus the comment move (K1).
- src/MainComponent.cpp: debug wiring inside the existing #if block (K1); the perfRecord comment (K2); the label lambda and the MicReplaced notice case (K3).
- tests/test_device_policy.cpp: +19 cases (K2 / K3).
- tests/test_audio_engine_devices.cpp: NEW (K2 / K3).
- tests/CMakeLists.txt: the new target, directly after test_device_policy (K2).
- .harmony/probe-btguard.sh: K1.
- Docs: pitfalls.md, testing-eyes.md, architecture.md, APP-INVENTORY.md (K4).
- Gate runners and evidence under .harmony/.reports/s-rta-1002/bt2-gates/ (this report's commit).

### SLIM CHECK
Nothing to cut.
- `hasInputDevice()` is no longer called by `getDeviceState()`. It is public API from btguard; I left it in place on purpose (removing it is out of scope).
- Every new member maps to a ruling item.

### ISSUES
- K1 ctest: test_preset_manager T3 and T5 failed once, then passed 5 of 5 targeted reruns; K2, K3 and the final ctest were 100%. Likely cause (INFERRED): the fixed temp filename (`tests/test_preset_manager.cpp:39`) is shared with lane tsan's ctest, which was running at the same time. It is not in bt2's code.
- First run of the GATE-3 script: Catch2 v3 exits with code 42 on failures, which together with `set -o pipefail` aborted the script before printing verdicts. Fixed with `{ bin || true; } | parser` and re-run. The trap removed the worktree in both runs.
- Old scratchpad 44b528dd still exists. I used the evidence mirror `evidence-0930/` (same files).

### DEVIATIONS (all minimal; none changes a GATE-3 verdict)
- **Kept ruling text, mechanical changes only.** Evidence case IDs renamed per AM22 (XA1 -> R6, XA0 -> R7, XA2 -> R8, XK1 -> R9, XM -> M5c, XL -> R10, R11s -> R11, R12s -> R12, P14s -> P14, M8s -> M8, E3s -> E3). The local `rmDev` / `rmDev2` helpers became the existing `removeDevice`, moved into the prelude so GATE-3's extraction finds it. The tag is `[device_policy]`.
- **Changed because of the kDenied constraint.**
  - R15: the final two CHECK_FALSE lines were swapped so the case ends with `saw(kDenied)`.
  - R16: its `CHECK_FALSE(saw(kDenied))` moved to the end.
  - R17: a final `CHECK_FALSE(rig.spy.saw(kDenied))` was added (its fixture contains kDenied).
- **Ordering and placement.** The `"*"` token landed in K2 together with AE1, P15 and E3, which need it. AE3 landed in K3 (it needs `deviceStateFor`).
- **GATE-3 method.** The base worktree gets the lane's test TUs and the lane's target block. A normal cmake build compiles both the TUs and the code under test from that one tree. This replaces "compile_commands flags with -I first"; it has the same one-tree property, and src/ is proved untouched.

### RISKS
- These are unchanged from the plan and rulings. Only the policy path was exercised live; the hardware dead-device path (R2 stall, R4) remains Boris check B2. Before B2, AM7's 8.0.4 E1 check is needed.
- Low: the new label write overwrites a file name shown in the label, but only on device events and only in mic mode (plan R7).

### SKILL_PROPOSALS
none

### METRICS
- Self-check: 4 builds (rc 0, 0 new warnings in touched files); 4 full serial ctest runs (1053/1055 flake, 1060, 1077, 1077); GATE-3, GATE-4, 3 live runs.
- Tool calls: about 95.
- Files read: about 30.

### PACKET QUALITY
- Clarity: CLEAR. The plan, the 2 rulings and the adoption are very detailed.
- Missing context:
  - The kDenied "ends with" constraint conflicts with verbatim copying in R15, R16 and R17; resolved as above.
  - The target-count conventions differ from what ctest reports.
- Unused context: the plan's section 5 / G-list (superseded).
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: the plan, ruling-bt2.md, ruling-bt2-seats.md, the evidence mirror and the lock helper. All existed and all were useful.

### KNOWLEDGE CONTEXT
- Tools used: grep, and JUCE source read
- Impact authority: grep, which is not authoritative, so I took a conservative posture: no deletions on the basis of "no callers"; `hasInputDevice` was kept.
- God nodes in scope: none queried
- Risk level: NORMAL
- Queries made: 0 (graph)

### Notes for .harmony/notebook.md (Harmony appends)
- Catch2 v3 exits with code 42 when tests fail. In a `set -euo pipefail` script, `bin -r junit | parser` therefore aborts the script, so wrap it as `{ bin -r junit || true; } | parser`. (`.harmony/.reports/s-rta-1002/bt2-gates/gate3-red.sh`)
- test_preset_manager writes fixed filenames in $TMPDIR (`tests/test_preset_manager.cpp:39`). Two lanes running ctest at the same time can collide on T3 and T5. Rerun the target before blaming a lane.
- A cheap one-tree RED / mutant rig: `git worktree add --detach <scratch>/wt <sha>`, a cmake configure with the FETCHCONTENT_SOURCE_DIR_* deps, then `cmake --build --target <2 targets>`. Configure plus build takes about 2-3 minutes, and each mutant is an incremental rebuild. (`bt2-gates/gate4-mut.sh`)
- probe-btguard now takes about 100 s per run (3 launches) and has 39 counted rows. `audio_deny` replaces the denied set at runtime ("*" = every device). `audio_stop` stops the combiner, which keeps it.

INBOX-RECHECK: none

### STATUS
DONE_WITH_CONCERNS. Every item and gate this stage owns met its pre-registered bar. The concerns are reported, not open defects: the kDenied-ordering test-text deviations, the flake from another lane, and the count-convention mismatch.

### NEXT ACTION
Harmony: review, then merge. After the merge: GATE-6 (5 of 5 at 39 / 0 / 2), GATE-8, GATE-9 and GATE-10.

---

## Fix round (teeth) -- s-rta-1002 bt2-teeth (base f89b1fa)

STATUS: DONE
RESULT: The four surviving mutants were re-checked against the code. X1, X2 and X5 are now killed by new unit assertions; X5 is also coreaudio NIT-1. X6 is an EQUIVALENT mutant on every path JUCE 8.0.4 can reach, so no assertion can kill it without an artificial state; two premise assertions now pin the JUCE fact that makes it equivalent. NIT-3 doc wording is fixed. Only tests and docs changed; src/ is untouched.
FACTS:
- X1 / X2 / X5: in the OLD arm all 55 cases pass under each mutant. In the NEW arm the new assertions fail: R16 + R18 (X1), R16 (X2), R11 + R14 (X5). Evidence: `.harmony/.reports/s-rta-1002/bt2-gates/teeth.out`.
- X6: 55 of 55 cases pass in both arms (`teeth.out`).
- Full serial ctest in build-lane, under the mutex: `100% tests passed, 0 tests failed out of 1077` (`bt2-gates/teeth-ctest.out`).
- `git diff f89b1fa -- src` is empty.
METHOD: `bt2-gates/teeth-mut.sh` follows gate4-mut.sh. It makes a detached worktree of HEAD in scratch with a fresh cmake configure. The OLD arm keeps HEAD's test file; the NEW arm copies the fix-round test file over it. Each mutant is one src edit whose anchor is asserted to match exactly once. After the edit the script rebuilds test_device_policy and runs it with `-r junit`. It then prints the failing case IDs and the failing assertion expressions, runs `git checkout -- src`, and proves `git diff --quiet -- src`. The worktree and its build dir are removed at the end.
CONFIDENCE: HIGH for X1 / X2 / X5. The kills were run and the red lines are pasted below. HIGH for the X6 equivalence argument: it comes from reading JUCE 8.0.4 source, and the two premise assertions run green.
VERIFY: `bash .harmony/.reports/s-rta-1002/bt2-gates/teeth-mut.sh <lane> <new test file> <scratch> <deps>`, then `ctest --test-dir build-lane -j1`.
UNKNOWNS-NOT-DONE: X6 can still be killed with a contrived case: an external `closeAudioDevice()` (which keeps the setup names), then failed defaults. I did not add it because no app path can reach that state (see below). This is Harmony's call.
NUANCE: SHOULD-3's suggested assertion, `count(spy.opened, "Built-in Speakers") == 2`, would NOT kill X2. Under X2, JUCE's `setAudioDeviceSetup` calls `open()` again on the SAME device object (needsNewDevice false: `juce_AudioDeviceManager.cpp:755-759`, then `open` :808 and `start` :817), so `opened` grows in both arms. The kill uses a device CREATION count instead. The new test-only `Spy::createdPairs` records duplex createDevice calls only; JUCE's sample-rate probe temporaries are one-way (`juce_AudioDeviceManager.cpp:556-557`).
HANDOFF-NEEDS: none

### Per-finding verification (each finding re-checked against the code)
- SHOULD-1 (X1): CORRECT.
  - Removing `if (nowInput != previous.inputDeviceName)` (`src/audio/DeviceGuard.cpp:238`) means the formula runs on every re-apply.
  - On a DeviceStopped re-apply onto the same, non-empty mic it writes `previous` into lostInput_. That wrongly names a mic that was never lost, and it overwrites a MicReplaced name.
  - Teeth:
    - R16 `CHECK(reconciler.lostInput().isEmpty())` (`tests/test_device_policy.cpp:1325`).
    - R18 gains a step: the replaced USB mic is pulled again while the device stops. The DeviceStopped re-apply lands on the SAME built-in mic, and the step checks reapplies 2, lastAction DeviceStopped, playing, input "Built-in Mic", and `lostInput() == "USB Mic"` (`:1382-1392`).
    - The R18 title gains "and a stop re-apply onto the same mic". Its kDenied check stays last.
- SHOULD-2a (X5): CORRECT. With `setAudioDeviceSetup(keep, true)` (`DeviceGuard.cpp:230`), JUCE calls `updateXml()` (`juce_AudioDeviceManager.cpp:830-831`), so `lastExplicitSettings` is set. Teeth: `CHECK(rig.manager->createStateXml() == nullptr)` after the put-back in R11 (`:1167`) and R14 (`:1253`).
- coreaudio NIT-1: this is the same mutant as X5, and the same two assertions kill it. Cheap.
- SHOULD-2b (X6): the finding is WRONG as a kill request; X6 is an equivalent mutant on reachable states. Evidence (JUCE 8.0.4, `build/_deps/juce-src/.../juce_AudioDeviceManager.cpp`):
  - Every no-device outcome clears the setup's names through `deleteCurrentDevice()` (:663-668). This covers the empty-names branch (:745), "No such device" (:761 deletes before :771 returns), a null create or a create error (:785), and a failed open or start (:835). JUCE's own list-change re-init ends in the same `setAudioDeviceSetup` (:217-226).
  - `closeAudioDevice()` (:889-894) is the only path that drops the device and keeps the names. Its only src caller is `DeviceReconciler::openDefaultDevices` (`DeviceGuard.cpp:181`), which re-opens at once (grep of src). The app does not use AudioDeviceSelectorComponent; MainComponent only reads `getCurrentAudioDevice()` and starts MIDI.
  - So with no device, `previous` names are empty, `keep` is empty, and the `keep.isNotEmpty()` guard (`DeviceGuard.cpp:228`) already blocks the restore. Removing `haveDevice &&` changes nothing.
  - Teeth for the premise: M5c (after JUCE's failed re-init, `:877-879`) and R10 (after the failed launch open, `:1139-1140`) now CHECK that both setup names are empty. If a JUCE upgrade ever kept the names, these go red and the gate's equivalence has to be re-proved.
- SHOULD-3 (X2): CORRECT that it survived. The suggested assertion was wrong (see NUANCE). Teeth: R16 `CHECK(count(rig.spy.createdPairs, "Built-in Speakers + USB Mic") == 2)` (`:1324`): the launch plus the re-apply, so the stopped device was closed and re-created, not restarted in place.
- gates NIT-3: pitfalls.md Guards append now reads "RED on the pre-bt2 base, whose src/audio and tests/test_device_policy.cpp are identical to 655d232". I checked `git diff 655d232 2d38b39 --stat -- src/audio tests/test_device_policy.cpp`; it is empty. GATE-11 strings re-counted after the edit:
  - "The startup `setSourceMode` re-open stays": 0
  - the full AM6 exception sentence: 1
  - "the open device is STOPPED and nothing restarted it": 1
  - "tried once per distinct list of allowed inputs": 1
  - "lost - now listening on": 1
  - "M5r (JUCE's own XML-branch re-init": 1
  - Note: the Guards append is no longer byte-identical to the AM20 ruling text in this one place. That is by request.

### Per-mutant lines (verbatim, `bt2-gates/teeth.out`)
```
--- arm OLD tests (tests/test_device_policy.cpp sha 0b424e6cae59)
baseline (src unmutated): cases 55 NONE
OLD | X1 lostInput guard removed | src/audio/DeviceGuard.cpp +1 -1 | kill set {R16 R18} | failing: NONE (of 55 cases)
OLD | X2 openDefaultDevices never closes | src/audio/DeviceGuard.cpp +1 -1 | kill set {R16} | failing: NONE (of 55 cases)
OLD | X5 restore with treatAsChosenDevice=true (= coreaudio NIT-1) | src/audio/DeviceGuard.cpp +1 -1 | kill set {R11 R14} | failing: NONE (of 55 cases)
OLD | X6 restore gate without haveDevice | src/audio/DeviceGuard.cpp +1 -1 | kill set {(equivalent: none expected)} | failing: NONE (of 55 cases)
--- arm NEW tests (tests/test_device_policy.cpp sha 8a580cbda4f8)
baseline (src unmutated): cases 55 NONE
NEW | X1 lostInput guard removed | src/audio/DeviceGuard.cpp +1 -1 | kill set {R16 R18} | failing: R16 R18 (of 55 cases)
    R16: reconciler.lostInput().isEmpty()  [test_device_policy.cpp:1325]
    R18: reconciler.lostInput() == "USB Mic"  [test_device_policy.cpp:1392]
NEW | X2 openDefaultDevices never closes | src/audio/DeviceGuard.cpp +1 -1 | kill set {R16} | failing: R16 (of 55 cases)
    R16: count(rig.spy.createdPairs, "Built-in Speakers + USB Mic") == 2  [test_device_policy.cpp:1324]
NEW | X5 restore with treatAsChosenDevice=true (= coreaudio NIT-1) | src/audio/DeviceGuard.cpp +1 -1 | kill set {R11 R14} | failing: R11 R14 (of 55 cases)
    R11: rig.manager->createStateXml() == nullptr  [test_device_policy.cpp:1167]
    R14: rig.manager->createStateXml() == nullptr  [test_device_policy.cpp:1253]
NEW | X6 restore gate without haveDevice | src/audio/DeviceGuard.cpp +1 -1 | kill set {(equivalent: none expected)} | failing: NONE (of 55 cases)
post (NEW tests, all mutants reverted): cases 55 NONE
```
Every mutant was followed by `reverted: git diff -- src = '' (empty)`, and the script ended with `cleanup: worktree + build dir removed` and exit 0.

### ctest (serial, under /tmp/audiodna-ctest.lock, build-lane fully rebuilt first)
`100% tests passed, 0 tests failed out of 1077`. The count is unchanged because no case was added. test_device_policy now has 55 cases and 360 assertions (346 before).

### FILES CHANGED (f89b1fa..fix round)
- `tests/test_device_policy.cpp`:
  - `Spy::createdPairs`, recorded in `MockDeviceType::createDevice`.
  - New assertions in M5c, R10, R11, R14 and R16.
  - An R18 step and title.
- `docs/claude/pitfalls.md`: Pitfall 61 Guards, "RED on ..." wording (NIT-3).
- `.harmony/.reports/s-rta-1002/bt2-gates/teeth-mut.sh`, `teeth.out`, `teeth-ctest.out`: the rig and its evidence.
- This section.

### PACKET QUALITY (fix round)
- Clarity: CLEAR.
- Missing context: none.
- Unused context: none.
- Self-brief files: both reviews, gate4-mut.sh and the lane report were used. I also read JUCE 8.0.4 ADM source to verify X2 and X6.
- Two suggestions in the reviews were inaccurate:
  - SHOULD-3's `spy.opened` count does not kill X2.
  - SHOULD-2b's X6 is equivalent.

INBOX-RECHECK: none
### STATUS
DONE
