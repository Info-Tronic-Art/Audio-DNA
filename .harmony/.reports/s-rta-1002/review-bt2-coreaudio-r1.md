# Reviewer Verdict - bt2 coreaudio r1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS)
PINNED: lane/bt2 head f89b1fa, base 2d38b39. Read via git diff / git show only.
BLOCKING: none (no MUST).

VERIFIED (ran or read today):
- Lane's test binaries at head: test_device_policy 55/55 cases (346 assertions), test_audio_engine_devices 3/3 (AE1 real HAL read-only, deny-all).
- Production-config syntax check (no AUDIODNA_TEST_SERVER) of AudioEngine.cpp, DeviceGuard.cpp, DevicePolicy.cpp in a $TMPDIR copy: no errors.
- Re-ran 3 extra mutants in a $TMPDIR copy: remove 10-s bound -> killed (1 case); never clear noInputOn_ -> killed (2 cases); restore with treatAsChosenDevice=true -> SURVIVED (55/55 pass) = NIT-1.
- JUCE 8.0.4 reads: ADM setAudioDeviceSetup/updateXml/deleteCurrentDevice/audioDeviceListChanged; CA combiner isPlaying = callback != nullptr (:1511), stop = shutdown (:1695, :1735-1752), restartAsync (:1637). C3 claim (old re-open changed nothing) holds: useDefaultInputChannels -> updateSetupChannels re-enables 2 input bits; MidiHandler::start -> setMidiInputDeviceEnabled -> updateXml, so the pitfall's lastExplicitSettings claim holds.
- Reconcile table (DevicePolicy.cpp:162) matches AM13 row order; timerCallback order matches AM12 (clear noInputOn_ before any return; AdoptInput guard after the None return and before the scan gate); lastAttemptSeq_ recorded AFTER the open incl. the launch (AudioEngine.cpp:28); restore filters each half by the pre-open lists and goes through the guarded manager (createDevice refuses non-listed); lostInput_ formula verbatim AM17.
- No path opens a hidden device: the only opens are initialiseWithDefaultDevices and the restore, both through GuardedDeviceType (createDevice refuses unlisted names); grep of src shows no other setAudioDeviceSetup / closeAudioDevice / stopDevice caller.
- TEST-ONLY confinement: audio_deny/audio_stop routes (ApiServer.cpp #if 298-328, 1768-2145; ApiServer.h 204-213, 260-281), AudioEngine debug fns, setTestDeniedNames, "*" token (DevicePolicy.cpp:61 inside classify's #if) all inside AUDIODNA_TEST_SERVER (default OFF, CMakeLists.txt:62).
- Real-time: CombinedCallback.h / AudioCallback.* diff empty; no new mutex; reconciler = ChangeListener + Timer (message thread); route handlers marshal through callAsync and answer at once (bodyless POST ok).
- Fences: MainComponent hunks only at ~490, ~2153-2158, ~3091, ~5710; ApiServer only TEST-ONLY hunks; tests/CMakeLists block directly after test_device_policy; std::cerr only in src/audio. CLAUDE.md 23,996 B unchanged. No .venv, no env hook, no MUTANT/TODO strings, build-lane/ is untracked (not in the commit).
- Gate artifacts: gate3.out (FAIL {AE1 AE2 R13 R14 R15 R16 R6 R7 R8} / PASS {M5c M5r R17 R9}), gate4.out 13/13, live-k1 27/12/2 with the predicted 12-row FAIL set, live-green 39/0/2 with stderr showing input-lost / adopt-input / no-device / device-stopped re-applies and no skipped (hidden) device ever opened.
- Docs: Pitfall 61 / testing-eyes / architecture text matches AM20 verbatim and matches the code (settle 250 ms, 10 s bound, four cases, scan gate, AM6 exception sentence present, stale "startup setSourceMode re-open stays" gone).

NITS (non-blocking):
1. DeviceGuard.cpp:230 restore uses setAudioDeviceSetup(keep, false); the mutant "true" (would write lastExplicitSettings / take JUCE's XML branch later) survives all 55 cases. Add CHECK(manager.createStateXml() == nullptr) after the restore in R11 or R14.
2. .harmony/APP-INVENTORY.md:31 headline says 112 Catch2 targets while the same paragraph reports 110 catch_discover_tests lines / 111 executables measured; the headline follows the plan's convention. Harmless; say so or use the measured figure.
3. tests/test_audio_engine_devices.cpp:124 AE2 lint scans only setSourceMode's own body; a device call moved into a helper it calls evades it. Acceptable per AM16; note it in the test comment.
4. tests/test_audio_engine_devices.cpp:84/98 AE1 leaves ADNA_AUDIO_DENY_DEVICES set if a REQUIRE fails before unsetenv (ctest runs each case in its own process, so no cross-test effect).

NOT PROVEN HERE (honest limits): no real unplug / dead-combiner close (<= 2 s message-thread stall) on hardware (Boris B2); GATE-6/8/9/10 are Harmony's post-merge. I did not rebuild GATE-3 on the base (relied on gate3.out + the same-source argument in the lane report).
METADATA: reviewer=claude-sonnet-5-5, lens=coreaudio, round=1, date=2026-10-02
