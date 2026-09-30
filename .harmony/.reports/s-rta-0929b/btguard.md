# btguard lane report (s-rta-0929b)

STATUS: DONE_WITH_CONCERNS
RESULT: The app no longer opens a Bluetooth (or other wireless) audio device on any JUCE open path. The guard is a decorator on JUCE's CoreAudio device type (GuardedDeviceType inside GuardedAudioDeviceManager). Built per the plan as amended by HARMONY ADOPTION BG1-BG9. C3 was dropped (BG3). BG4's condition was met (ctest M5b), so a bounded device-gone reconciler is included. Gates: ctest 997/997 serial; probe-btguard RED on main 11 PASS / 13 FAIL / 2 SKIP, GREEN 24 / 0 / 2 SKIP on 5 of 5 runs; the five existing probes all GREEN.
FACTS:
- Commits on lane/btguard: 392e03f (C1 feat), 9e89204 (C2 test), 17567be-successor docs commit (this report + inventory). The branch starts from main d88d2ea. (`git -C <WT> log --oneline d88d2ea..HEAD`)
- `ctest --test-dir build-lane -j1`: "100% tests passed, 0 tests failed out of 997" (967 before this lane + 30 in test_device_policy; 108 targets).
- test_device_policy: RED against the pass-through policy stub (uncommitted, scratch) = FAIL 18 / PASS 7. RED against the no-op reconciler stub = FAIL 3 (M5b, R1, R4) / PASS 27. Real code = 30/30. After the stub run, `DeviceGuard.cpp` was restored and its sha256 matched before and after (50b85c59...). `DevicePolicy.cpp` was restored the same way (1282075f...).
- probe-btguard RED on the pre-change app (`build/.../Audio-DNA.app`, d88d2ea): "PROBE-BTGUARD: 11 PASS / 13 FAIL / 2 SKIP". GREEN on build-lane: "PROBE-BTGUARD: 24 PASS / 0 FAIL / 2 SKIP (SKIP never counts)" on 5 of 5 runs.
- Existing probes on build-lane: probe-step3 "94 PASS / 0 FAIL", probe-resync "16 PASS / 0 FAIL", probe-onset-render "13 PASS / 0 FAIL", probe-downbeat-level "14 PASS / 0 FAIL", probe-manual-bpm "22 PASS / 0 FAIL".
- BG9: I built a TEST_SERVER=OFF app in scratch. `strings` counts: ADNA_AUDIO_DENY_DEVICES 0, /api/debug/audio_devices 0, test-denied 0, audio_notice 0. The guard itself is present ("Bluetooth is never used" x4). The only getenv sits at DeviceGuard.cpp:206-207, inside `#if AUDIODNA_TEST_SERVER`.
- CLAUDE.md went from 24,522 B to 23,818 B. The Latency Budget table moved to docs/claude/architecture.md and the index gained a line "NN." (Harmony assigns the number).
METHOD: I read the plan, all 3 attacks and every adoption ruling, plus the JUCE 8.0.4 ADM and CoreAudio source. Each piece was written test-first against mocks with a spy on the inner type, then checked live with the TEST-ONLY env hook on this rig's built-in devices. No Bluetooth device was connected and no system setting was touched.
CONFIDENCE: HIGH for the hiding, default, no-input and no-device paths (unit and live). MEDIUM for the transport classification of real wireless devices: that part is unit-only, because the rig has only built-in devices.
VERIFY: `bash .harmony/probe-btguard.sh` under the live lock; `build-lane/tests/test_device_policy`; strings on a TEST_SERVER=OFF build.
UNKNOWNS / NOT-DONE:
- Video-recording start/stop in arm C (BG7) is NOT gated live. There is no REST route for it, only the menu or a binding, and synthetic input is forbidden. Source shows VideoRecorder captures video only (VideoRecorder.h:34) and reads no audio rate.
- Mutation table (gates S1): only the two whole-stub REDs were run.
NUANCE: The live env hook drives only the HIDING branch. The "Bluetooth default -> built-in opened" behaviour is proven by ctest M1 (a real juce::AudioDeviceManager with a mock inner type and a spy) and the M2 control. It is not proven live.
HANDOFF-NEEDS: Harmony assigns the Pitfall number (the "NN." in CLAUDE.md and pitfalls.md), writes ledger item 14, appends the notebook notes below, and runs the critic seat on the shots (BG6).
INBOX-RECHECK: none

## Items

| id | verdict | evidence |
|---|---|---|
| C1 policy + guard + notice + tests + docs | DONE (392e03f) | ctest 30/30; RED stub 18 FAIL / 7 PASS; app builds warning-free in the new files |
| C2 TEST-ONLY witnesses + probe | DONE (9e89204) | probe RED main 11/13/2S; GREEN 24/0/2S x5 |
| C3 remove setSourceMode re-open | DROPPED per BG3 | today's open sequence unchanged: `opens` = 2 after launch, 4 after a file -> input switch (INFO) |
| BG1 deny set, read error fails closed, static_asserts | DONE | P6, P7, P11; CoreAudioDeviceInfo.cpp static_asserts incl. 'ccap' / 'fgrp'; E1 + A2/A3 require transport == 'bltn' and read OK |
| BG2 aggregates | DONE | P3, P3b (unresolvable member, Bluetooth UID shape, unreadable member list) |
| BG3 | DONE (no change to the open sequence) | A7 opened{} byte-identical after file -> input |
| BG4 M5 + reconciler | DONE | M5 (mono mic, XML branch): JUCE re-inits onto an allowed device with no help. M5b (stereo input, no explicit settings): JUCE alone ends with NO device (`CHECK(getCurrentAudioDevice() == nullptr)` holds), so DeviceReconciler was built: device gone only, 250 ms settle, <= 1 per 10 s, a failed re-apply retried only after a new scan, acts only when no device object exists. R1 storm of 20 -> 1 re-apply; R2 keep; R3 nothing allowed -> 0; R4 unopenable -> 1; R5 list change from inside start() |
| BG5 inner spy | DONE | every M / R case asserts the denied name was never created, opened or started (temporary devices included); M2 asserts the spy saw it |
| BG6 persistent notice | DONE | its own label, right of the file label (yellow, kMeterYellow); B3 / B3b / C2 rows; shots below |
| BG7 no-device | DONE, except video recording (above) | C3 render decodes plasma at mean 108.4; C4 source both ways; C5 perf take arm + stop 200. Clamp: MainComponent::perfRecord arms without audio when deviceRate <= 0 |
| BG8 hiding branch named; deny-default-output row | DONE, row SKIPs | rig lists 1 allowed output -> D1 / D2 SKIP (never counted) |
| BG9 production absence | DONE | strings = 0 on the OFF build; getenv inside #if; Config::testDeniedNames and "test-denied" under #if |
| Q4 input-name status | DONE | main B3b shows the old mislabel "Mic: MacBook Pro Speakers @ 48000Hz"; lane shows "Mic: MacBook Pro Microphone @ 48000Hz" (shot A) |

## Gates (raw lines)

- ctest RED (pass-through policy stub): `FAIL P1 PASS P2 FAIL P3 FAIL P3b FAIL P4 PASS P5 FAIL P6 FAIL P7 FAIL P8 FAIL P9 FAIL P10 FAIL P11 FAIL P12 PASS P13 FAIL D1 FAIL D2 PASS D3 FAIL M1 PASS M2 FAIL M3 FAIL M4 FAIL M5 FAIL M5b PASS E1 PASS E2` -> `FAIL 18 / PASS 7`. The PASS cases are expected: P2 (a USB default is kept either way), P5 (empty), P13 (toString), D3 (forwarding), M2 (the control), E1 / E2 (nothing is denied on this rig).
- ctest RED (no-op reconciler stub): `FAIL 3 / PASS 27` (M5b, R1, R4).
- ctest GREEN: `FAIL 0 / PASS 30`; full suite: `100% tests passed, 0 tests failed out of 997`.
- probe-btguard on main: A0 PASS; A1-A5 FAIL (404); A7 FAIL; A8 / A9 PASS; B0 PASS; B1 / B2 / B3 / B3b / B5 FAIL; B4 / B6 PASS; C0 PASS; C1 / C2 FAIL; C3 / C4 / C5 / C6 PASS -> `11 PASS / 13 FAIL / 2 SKIP`. Note: the live RED is the missing endpoint / notice, not the Bluetooth regression itself (vj S6).
- probe-btguard on build-lane, 5 runs: `24 PASS / 0 FAIL / 2 SKIP` every run.
- Every batch had BT_DEFAULT=no (system_profiler), `UserNotificationCenter windows (kCGWindowListOptionAll): 0` at 16 s after the last quit, and outwins "Output-named 0".

## SHOULDs (adopted / declined)

- coreaudio S1 (take across a re-apply): adopted as a statement. The reconciler acts only when the device is already gone, and AudioTap's unequal-args prepare stops the tap cleanly (existing behaviour).
- coreaudio S2 (UID re-point on dedupe flip): declined. The reconciler is name-free (it acts on "no device"), and the name compare is JUCE's own. Filed.
- coreaudio S3 / vj S7 / gates S6 (HAL cost): adopted as logging (stderr when a scan takes > 50 ms; `enumerate_ms` on the endpoint, measured 0.25 ms). Per-id caching declined (no measured need).
- coreaudio S4 (E2 known set): adopted (E1 known set + transport != 0 + read OK; P13; static_asserts).
- coreaudio S5 / vj S5 / gates S1 (RED counts): adopted as measured counts. P2 uses a non-first default. Mutation table: partial (two stubs).
- coreaudio S6 (default rule): adopted (built-in, then USB; P12).
- vj S1 (silent swap): partly adopted: a stderr line plus the devices line on every re-apply. A UI line is declined (new surface; for Boris to decide).
- vj S2 (announce a newly plugged interface): declined; filed with the BG4 adopt-mic case.
- vj S3 (TCC mic denied): declined; outside the fence. Filed.
- vj S4 (names from arm A): adopted.
- vj S6: adopted (stated above).
- vj S8: adopted (M5b is the evidence for the reconciler).
- vj S9 (dedupe drift ctest): declined. The reconciler does not compare names. Filed.
- gates S2: adopted (B2 input_channels == 0; B5 = output-only clocks analysis).
- gates S3: adopted (no reapplying_ flag; rate limit + seq; R1 / R4).
- gates S4: adopted (opens is INFO).
- gates S5 (TEST-ONLY list-change POST): declined. The mock ctests cover it, and faking a HAL change would need a new private hook.
- gates S7: adopted (R5).
- gates S8: moot (C3 dropped).
- NITs: gates N2 fixture name "Denied Wireless" adopted; gates N3 A4 rewrite adopted; vj N1 "wireless" wording declined (sentences as ruled in BG6).

## Loose ends (for Harmony's ledger)

1. A wired INPUT unplugged while its output stays: JUCE keeps the device open with a dead input and no re-init. The reconciler (device-gone only, per BG4) does not act. Consider this for the adopt-mic follow-up.
2. The HAL-thread restartAsync race (JUCE 8.x; only JUCE 9 removes it): unchanged.
3. Temporary devices in insertDefaultDeviceNames still register HAL listeners (listener-lifetime class).
4. The aggregate branch is unexercised on real hardware (unit-only).
5. Video-recording no-device row: no REST path exists.
6. JUCE >= 8.0.9 (not 8.0.8) before any wired interface: run the bt-crash-mechanism E1 probe on the first wired interface.
7. The C3 re-open removal is filed.

## Notebook notes (for Harmony to append)

- 2026-09-30 JUCE's list-changed re-init leaves NO device when lastExplicitSettings is null | AudioDeviceManager::audioDeviceListChanged -> initialiseDefault(&currentSetup) re-opens the VANISHED names -> "No such device". setSourceMode's setAudioDeviceSetup(...,true) sets the XML only when the setup differs (mono mic yes, stereo input no) | discovered: tests/test_device_policy.cpp M5 / M5b.
- 2026-09-30 GuardedAudioDeviceManager test hook | typeFactoryForTests plus a mock named "CoreAudio" drives a REAL juce::AudioDeviceManager headlessly. It needs JUCE_MODAL_LOOPS_PERMITTED=1 on the target to pump change messages (runDispatchLoopUntil) | discovered: tests/CMakeLists.txt test_device_policy.
- 2026-09-30 window-only captures of a background (-g) app by Quartz window id work while the window is occluded (CGWindowListCreateImage + kCGWindowListOptionIncludingWindow) | discovered: .harmony/probe-btguard.sh shot().

## PACKET QUALITY

- Clarity: CLEAR. The adoption rulings resolved the plan-vs-attack conflicts.
- Missing context: BG7's "start + stop a video recording" has no REST path, so it was not gated live (see above).
- Unused context: plan section 9 (JUCE bump, a recommendation only).
- Self-brief files: plan-btguard.md and the 3 attack files were useful; JUCE source was read directly.

STATUS: DONE_WITH_CONCERNS

## Fix round (btguard-fix, 2026-09-30) -- STATUS: PARTIAL

The full report is `.harmony/.reports/s-rta-0929b/btguard-fix.md`. I checked all 7 critic-r1 findings against the code, and all 7 hold. Each "survives" claim was reproduced with a mutant built from source COPIES; the old suite stayed 30/30 green on all 8 mutants.
- 3340697 test: D1 changed, plus new D4 / D5 / D6 / M6 / M7 / R4b, R1 settle check, and E1 macOS-default flags. The new suite FAILs every mutant: MA -> D1 D4 M6 M7, MB -> D1 M6, MC -> R1, MD -> R4b, ME -> D5, MF / MF2 -> E1, MG -> D6. Real code: 36/36; ctest `100% tests passed, 0 tests failed out of 1003`.
- 0c849f3 fix: the notice now reads "No wired mic found - plug one in. Bluetooth is never used." / "No audio device found - plug one in. Bluetooth is never used." The pre-fix app gives `PROBE-BTGUARD: 22 PASS / 3 FAIL / 2 SKIP` (B3, B3b, C2).
- 90f4854 test: probe row C5b (the no-device take.json has no audio; lastError ""). It PASSes on the pre-fix app.
- STOPPED: the clamp-removed mutant app, a re-signed scratch copy, raised the macOS microphone-permission prompt. That prompt is still on screen and was left untouched. So no GREEN probe run was done on build-lane, and C5b has no RED. Still open: the real-HAL aggregate branch (needs a real aggregate device) and video recording with no device (no REST route; Harmony to accept).
