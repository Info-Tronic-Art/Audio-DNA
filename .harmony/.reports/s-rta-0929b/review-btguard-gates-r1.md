# Reviewer Verdict — btguard gates r1
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES
REVIEWED: lane/btguard 647ad69 (base d88d2ea), read via git show / diff; mutation proofs on a $TMPDIR copy (never the tree)
FILES: src/audio/{DevicePolicy,CoreAudioDeviceInfo,DeviceGuard}.{h,cpp}, AudioEngine.{h,cpp}, CombinedCallback.h, MainComponent.{h,cpp}, api/ApiServer.{h,cpp}, tests/test_device_policy.cpp, tests/CMakeLists.txt, CMakeLists.txt, .harmony/probe-btguard.sh, CLAUDE.md, docs/claude/*.md, APP-INVENTORY.md

## Answer
One MUST: the guarded DEFAULT-INDEX plumbing (DeviceGuard.cpp getDefaultDeviceIndex -> policy default; and the policy's built-in fallback rung) is not covered by any ctest that can fail. Reverting getDefaultDeviceIndex to `inner_->getDefaultDeviceIndex(forInput)` leaves all 30 cases green (VERIFIED, executed in $TMPDIR copy). D1 and M1 put the denied default at inner index 0, so inner index == filtered index by coincidence (the D1 test comment even says so). JUCE's insertDefaultDeviceNames does `deviceNames.move(type->getDefaultDeviceIndex(isInput), 0)` (juce_AudioDeviceManager.cpp:537) so on any rig where the headset is not first this is the exact defect the guard exists to prevent.
Everything else in the focus list holds; SHOULDs below.

## VERIFIED (executed)
- build-lane/tests/test_device_policy: All tests passed (162 assertions in 30 test cases); ctest -N = 997.
- Mutation table (copy of 647ad69 sources + test, same flags, JUCE objs reused). KILLED (test fails): Bluetooth allowed (17 cases fail), BLE allowed (P7), AirPlay (P6), transport-unreadable fail-open (P11), unmapped shown (P10), createDevice guard removed (D2), getDeviceNames passthrough (D1/M1/...), aggregate member-unresolved / bluetooth-uid / members-unreadable / any-member-denied (P3/P3b), reconciler 10 s interval removed (R4), CoreAudioDeviceInfo transportReadOk forced true (E1).
  SURVIVED (all green): getDefaultDeviceIndex -> inner default; policy built-in fallback rung removed; reconciler seq gate removed; reconciler settle timer replaced by an immediate call; rebuild() unmapped re-scan removed; CoreAudioDeviceInfo isDefaultOutput forced false; aggregate readMembers result forced ok; getIndexOfDevice -> `return i`. (Equivalent mutant, ignore: dropping the `!hadDevice_` re-check in timerCallback.)
- Proposed fixture X1 (below) passes on the real code (31/31) and kills BOTH default mutants (getDefaultDeviceIndex passthrough; built-in rung removed).
- M2 control: plain juce::AudioDeviceManager over the same mock opens the denied device and the spy sees it (BG5 control is real). Spy records created / opened / started names incl. JUCE's temporary probe devices.
- E1 asserts uid.startsWith("BuiltIn") devices read transport == 'bltn' exactly and transportReadOk / != Unknown for every device; A2/A3 assert transport == 'bltn' exactly (0 fails).
- BG9: recompiled AudioEngine, DeviceGuard, DevicePolicy, CoreAudioDeviceInfo, ApiServer TUs from compile_commands with -DAUDIODNA_TEST_SERVER removed: they build; `strings` for ADNA_AUDIO_DENY_DEVICES|api/debug/audio_devices|test-denied|audio_notice|test_denied = 0 in each; control (the TEST_SERVER=ON Audio-DNA binary) = 3. Only getenv of the var: DeviceGuard.cpp:207 inside #if AUDIODNA_TEST_SERVER (206-215); Config::testDeniedNames (DevicePolicy.h:72) and "test-denied" (DevicePolicy.cpp) under the same #if; ApiServer handler inside the 1740-2075 TEST_SERVER region.
- BG8: SKIP rows go through `row SKIP`; the tally greps '^PASS' only, so a SKIP never counts as a PASS. BG7 rows C3/C4/C5 present; no-device rows do not need main to pass them.
- The five audio probes: no diff touches any of them (.harmony/ changes = probe-btguard.sh + APP-INVENTORY + report only), so no re-thresholding. (Their green re-run itself is the lane's claim: INFERRED, I did not launch the app.)
- Real-time: AudioCallback.* untouched; CombinedCallback.h adds only the TEST_SERVER `opens_` counter in audioDeviceAboutToStart (message thread, lock-free atomic). No new mutex in production (deviceStatusMutex_ is TEST_SERVER-only, message<->HTTP, planned). Render thread untouched. GuardedDeviceType / DeviceReconciler are message-thread only; member order (deviceManager_ before deviceReconciler_) destroys the reconciler first.
- Stray check: `git status` shows only untracked build-lane/ (not committed); no .venv, no leftover instrumentation; the env hook is TEST_SERVER-gated.
- CLAUDE.md = 23,818 bytes (<= 25,000). Pitfall/index number is the literal "NN." as the plan says; docs match the code (checked the JUCE line cites: insertDefaultDeviceNames 537, appendNumbersToDuplicates 2180-2181).

## NOT verified (INFERRED / not run)
- I did not launch the live probes (live lock / port 7070 rig; read-only role). The lane's RED-on-main "11 PASS / 13 FAIL / 2 SKIP" and green 24/0/2 5x are the lane's numbers; by reading, the RED on main is the missing /api/debug/audio_devices endpoint + notice (A1-A5, A7, B1-B3b, B5, C1, C2), NOT the Bluetooth regression itself (lane says so too). The Bluetooth regression is provable only in ctest, which is why finding 1 matters.
- The two SKIP rows (D1/D2: deny only the default output while a second allowed output exists) never execute on this rig; probe still prints GREEN with them SKIPped.

## Proposed fixture for MUST 1 (verified to kill both mutants; passes on real code)
TEST_CASE("X1 the denied macOS default is NOT first: the guarded default still lands on the built-in", "[device_policy]") {
  juce::ScopedJuceInitialiser_GUI gui;
  GuardedRig rig([](MockDeviceType& m) {
    m.devs = { { kDenied, tp::Bluetooth, 1, 2 }, { "USB Speakers", tp::USB, 0, 2 },
               { "Built-in Mic", tp::BuiltIn, 1, 0 }, { "Built-in Speakers", tp::BuiltIn, 0, 2 } };
    m.defaultIn = kDenied; m.defaultOut = kDenied; });
  CHECK(rig.manager->initialiseWithDefaultDevices(2, 2).isEmpty());
  CHECK(rig.manager->getAudioDeviceSetup().outputDeviceName == "Built-in Speakers");
  CHECK_FALSE(rig.spy.saw(kDenied)); }
Also make D1 use a fixture where the filtered default index != the inner default index (and assert getDefaultDeviceIndex(false) is the built-in's filtered index).

## SHOULD / NIT (details in the structured verdict)
S1 R1 storm test cannot detect missing coalescing (settle timer replaced by immediate call: green). S2 seq gate untested (R4 sits inside the 10 s bound; construct the reconciler with a small minIntervalMs to isolate). S3 rebuild()'s unmapped re-scan (S7 race) untested. S4 real-HAL default flags / aggregate members / getIndexOfDevice untested (E1 could assert the isDefaultOutput device == JUCE's default output name). S5 C5 clamp (perfRecord audio=false at rate 0) never shown load-bearing; run arm C with the clamp removed (proposed mutation, builder) and expect FAIL; BG7 video-recording start/stop not gated live (lane concern, needs Harmony ack). N1 E1 `builtIn >= 1` hard-fails on a host with no built-in audio (CI is continue-on-error). N2 ApiServer.cpp ~2051: handleDebugAudioDevices was inserted between the asyncload comment "cancel the staged load" and handleDebugCancelLoad, orphaning the comment. N3 probe prints GREEN with SKIPped D rows: fine per BG8 but Harmony should not read GREEN as D proven.

METADATA: reviewer=reviewer-btguard-gates, builder_packet=btguard, date=2026-09-30
