# Reviewer Verdict -- bt2 gates r1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) -- 0 MUST, 4 SHOULD, 4 NIT
PINNED: worktree .claude/worktrees/bt2, lane/bt2, base 2d38b39, head f89b1fa (src/tests identical to c4a5938, the commit GATE-4 ran on)
LENS: gates / test teeth / gate evidence. Confidence: VERIFIED unless marked INFERRED.

## Verified (executed or read at the pinned head)
- TDP cases verbatim (AM22): scripted per-case diff of all 19 new cases against evidence-0930 (x_bt2_red, x_r13, x_seats_main, x_seats_new, x_bt2_new XL->R10). 12 identical; the rest differ ONLY by rmDev/rmDev2 -> removeDevice and the three declared kDenied-ordering edits (R15 last two CHECK_FALSE swapped, R16 kDenied moved to end, R17 final kDenied check added). Every new manager case with a kDenied fixture ends with CHECK_FALSE(saw(kDenied)).
- AM1 main-API-only: R6, R7, R8, R9, R13, M5c use initialiseWithDefaultDevices + DeviceReconciler(mgr,2,2) + reapplies() only; M5r/R14-R17 likewise. New API only in M8, P14, R10, R11, R12, R18.
- GATE-3 (AM5 one tree): gate3-red.sh builds the lane's cases verbatim (diff -r lane vs TU empty, prelude included) into a clean worktree of 2d38b39 with src/ untouched, junit verdicts. gate3.out: FAIL {AE1 AE2 R13 R14 R15 R16 R6 R7 R8}, PASS {M5c M5r R17 R9}; failed-assertion counts equal the ruling's (R14 2, R15 5, R16 3, AE1 3, AE2 1, R13 2). Method deviates from AM5's compile_commands wording but keeps its one-tree property.
- GATE-4: re-read every mutant edit in gate4-mut.sh (single-anchor asserted, diff-empty after each revert, baseline 58/0, post 58/0). 13/13 killed; every kill set intersects the pre-registered set. #6 (no settle) is killed by R1/R13 assertions; R17 itself crashes (UAF on its raw device pointer) -> 54 of 58 cases reported.
- GATE-5: K1 live = 27/12/2, FAIL set exactly {A6(2) A6b(4) A10 A11a A11 A13 B7 B7b(3) B8 C7 C8a C8}, A12 and A13a PASS; K2 = 29/10/2 (K1 minus A6/A6b, B7b reads 1); GREEN = 39/0/2, A6=1, A6b=1, A12 "0.000 s apart -> still 2", A13 opens 3->4. Each run.txt carries the AM8 tripwire (AUTHREQ_PROMPTING 0, UNC windows 0, clean quits). Row counts re-counted from results.txt. Binary mtimes (08:42:27 / 08:47:13 / 08:52:48) post-date every src mtime of the tree they represent (INFERRED same-source; the GREEN binary pre-dates the K3 commit 08:55:33 but no src file was touched after 08:50:35).
- Probe rows: A12 bar = delta vs r0/o0 + gap<=0.15 + one retry after a fresh >=10.5 s wait (matches AM4); A13a/A13 SKIP when no input or IN==OUTN (matches AM14); A10-A12 SKIP when no input.
- GATE-9 (static): every new debug symbol (setTestDeniedNames x2, debugSetDeniedDevices, debugStopDevice, last_reapply/lost_input status, audio_deny/audio_stop routes + handlers, onDebugAudio* members, the "*" compare, ADNA_AUDIO_DENY_DEVICES) sits inside #if AUDIODNA_TEST_SERVER (line ranges checked in AudioEngine.cpp/.h, DeviceGuard.cpp/.h, DevicePolicy.cpp, ApiServer.cpp/.h, MainComponent.cpp). The builder's no-TEST_SERVER compile + strings/nm = 0 hits is the lane's own unofficial run; the official GATE-9 stays Harmony's.
- GATE-11: pitfalls.md AM20 paragraph and Guards append are byte-identical (whitespace-normalised) to the ruling; the AM6 exception sentence 1x; "The startup `setSourceMode` re-open stays" 0x; STOPPED / once-per-distinct-list / "lost - now listening on" / "M5r (JUCE's own XML-branch re-init" 1x each; testing-eyes.md has audio_stop, device-stopped, lost_input; CLAUDE.md 23,996 B == base, no diff.
- Fences vs lane tsan (H4): only src/audio/*, ApiServer.{h,cpp} at 321-326 and 2076-2145 (tsan owns 93-96, 360, 384, 418-421, 1370-1544), MainComponent at ~490 / ~2153 / ~3088 / ~5707, tests/CMakeLists block directly after test_device_policy; std::cerr added only in src/audio; nothing in model/render/core/connect/media/recording.
- Nothing stray: no symlink (mode 120000) in the tree, no .venv (live.sh creates and removes it), no .new, no MUTANT/#if 0/TODO; the only setenv is inside AE1 and is unset after engine construction; worktree status = untracked build-lane/ only. Real-time: CombinedCallback.h / AudioCallback.cpp not in the diff; no mutex/lock added; reconciler and hooks are message-thread; the two routes marshal via callAsync and answer at once (Pitfall 31).

## Extra mutation attack (mine, executed in a $TMPDIR copy of f89b1fa; tree under review untouched)
baseline 58/0. Beyond the 13 registered mutants:
- X3 no 10 s bound -> KILLED (R4). X4 noInputOn_ never cleared -> KILLED (R11, R15). X7 lastAction never set -> KILLED (R11, R18).
- X1 lostInput_ "if (nowInput != previous...)" guard removed -> SURVIVES the unit suite.
- X2 openDefaultDevices never closes the open device -> SURVIVES.
- X5 restore re-opens with treatAsChosenDevice=true (writes explicit settings) -> SURVIVES.
- X6 restore gate "haveDevice &&" removed -> SURVIVES.
- X8 onDevicesReapplied never invoked -> SURVIVES units; live A10/A11/B7 label rows would catch it (INFERRED, not run).
X1 and X2 are also guarded live (A13 lost_input == "" and opens 3->4) INFERRED; X5/X6 have no row anywhere.

## Findings
SHOULD-1 tests/test_device_policy.cpp R16 (~1330-1360) and R18: nothing asserts lostInput() after a DeviceStopped re-apply onto the same mic; mutant X1 survives. Effect if shipped: a MicReplaced notice would be wiped by the next stop re-apply. Add CHECK(lostInput().isEmpty()) to R16 and a stop-after-replace step to R18.
SHOULD-2 R11 / R14: add CHECK(rig.manager->createStateXml() == nullptr) after the restore (kills X5) and a NoDevice-with-failed-open case or assertion for the restore gate (X6).
SHOULD-3 R16: assert the open device was re-created (e.g. count(spy.opened, "Built-in Speakers") == 2) so the explicit close in DeviceReconciler::openDefaultDevices (src/audio/DeviceGuard.cpp:~183) has unit teeth (X2); today only live A13 covers it.
SHOULD-4 onDevicesReapplied -> file-label write (src/MainComponent.cpp:491-495, src/audio/AudioEngine.cpp:20-21) has no unit witness; live-only. Acceptable given GATE-6, but record that GATE-6 is its sole guard.
NIT-1 R17 under the no-settle mutant segfaults (raw `d` after the re-apply destroys it) instead of failing an assertion, hiding 4 later cases; verbatim ruling text, so leave, but note it in the Pitfall/notebook.
NIT-2 .harmony/APP-INVENTORY.md row 31 headline says "112 Catch2 targets"; the same row's measured figures are 110 catch_discover_tests lines / 111 executables (base 109 / 110). Say which convention the 112 uses or headline the measured number.
NIT-3 docs/claude/pitfalls.md Guards append says "RED on 655d232" (verbatim AM20); the lane's base is 2d38b39 (src identical to 02b2913). Harmless, stale sha.
NIT-4 .harmony/.reports/s-rta-1002/bt2-gates/live.sh hard-codes the builder's scratchpad paths and a lock.sh that is not committed, so the report's "Harmony can re-run every gate" holds for gate3/gate4 (argument-driven) but not for live.sh; GATE-6 uses probe-btguard.sh directly, so no gate is blocked.
Also: AE2 is a name lint (deviceManager_/deviceReconciler_); a helper-method indirection would pass AE1 (deny-all, no device) and AE2. Ruled design (AM16); live A6/A6b is the real witness.

## Not run by me (Harmony's after merge)
GATE-6 (5 of 5), GATE-8, official GATE-9, GATE-10; no live run was made by this reviewer (live lock / Boris's screen).
