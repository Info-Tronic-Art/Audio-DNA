# PLAN -- lane "bt2": one device open per launch (C3) + the reconciler adopts an allowed device when the app has none (case iii) and re-applies when the open mic is gone

Architect, 2026-09-30, session s-rta-0930, main HEAD 655d232. Read-only on source: no file in the tree edited, no app launched,
no system setting touched, no Bluetooth device. Scratch rigs only (paths below); no mock case opened a real device.
Abbreviations: AE = src/audio/AudioEngine.cpp, AEH = src/audio/AudioEngine.h, DG / DGH = src/audio/DeviceGuard.cpp / .h,
DP / DPH = src/audio/DevicePolicy.cpp / .h, CC = src/audio/CombinedCallback.h, MC = src/MainComponent.cpp,
AS / ASH = src/api/ApiServer.cpp / .h, TDP = tests/test_device_policy.cpp, PB = .harmony/probe-btguard.sh,
ADM = build/_deps/juce-src/modules/juce_audio_devices/audio_io/juce_AudioDeviceManager.cpp (JUCE 8.0.4),
CA = build/_deps/juce-src/modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp (JUCE 8.0.4),
SBX = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/bt2
(evidence: x_bt2_red.cpp, x_bt2_new.cpp, x_c3_engine.cpp, proto/audio/*, rig.sh, rig2.sh, rig3.sh, mut.sh).
Labels: VERIFIED (read or ran today) / INFERRED.

## 0. ANSWER FIRST

1. C3: delete the two `setAudioDeviceSetup(setup, true)` blocks in `AudioEngine::setSourceMode` (AE:138-141, AE:150-153).
   The source mode is already a per-block atomic read in the callback (CC:87); the re-open changed nothing (JUCE re-enables
   the default input channels, ADM:711-728) but ran JUCE 8.0.4's CoreAudio open again: `opens` 2 -> 1 at launch, +2 -> +0
   per file <-> input round trip. Its one side effect -- JUCE's `lastExplicitSettings` is no longer written, so a vanished
   open device takes JUCE's no-XML path ("No such device") for the mono built-in mic too -- is ALREADY recovered by the
   merged DeviceReconciler (VERIFIED in scratch: mono, no re-open -> 1 re-apply -> the other allowed output).
2. Adoption: `DeviceReconciler` gets a pure rule table `devpolicy::reconcile(haveDevice, openedInput, lists)` ->
   None / NoDevice / AdoptInput / InputLost. The action stays the ONE existing re-apply (close + JUCE's default open
   through the guarded type = the launch choice re-made). Gates: today's coalescing (250 ms after the LAST change message,
   <= 1 per 10 s) plus "never twice on the same device scan -- the launch's own scan included" (`openDefaultDevices()`
   records it), so only a device plugged in AFTER launch triggers anything. A still-listed (working) input is never
   switched away from.
3. InputLost (loose end 1, separable): JUCE stops the WHOLE combined device when its input dies and never re-inits it
   (the combiner's name is the output's), so today a pulled wired-mic cable leaves the app deaf, state "ok", no notice,
   and re-plugging does nothing. Re-applying falls back to the macOS default / built-in mic, else output-only + the notice.
4. A re-apply that fails while a device was open puts back what is still listed of it (output-only): never worse off.
   While the app listens to the mic, every re-apply writes "Mic: <input> @ <rate>Hz" to the existing file label.
5. Teeth: ctest -- 3 new TDP cases RED on 655d232 (VERIFIED: 7 failed assertions), 2 guards green on main, 6 new-API
   guards (6/6 mutants killed on a prototype that also passes all 36 existing cases), and a new target with the REAL
   AudioEngine (every real device denied, nothing opened) proving `setSourceMode` never touches the device manager (VERIFIED
   RED on 655d232, GREEN on a C3 scratch copy). Live -- TEST-ONLY `POST /api/debug/audio_deny {"names": [...]}` swaps the
   denied set at runtime and runs the guard's device-list-change path (the plug / unplug stand-in); probe-btguard gains 12
   gated rows: predicted RED on the pre-merge copy 25 PASS / 12 FAIL / 2 SKIP, GREEN 37 / 0 / 2 on 5 of 5 runs.

## 1. GOAL

Open the audio device once per launch, and make the notice's "plug one in" true -- adopt an allowed device plugged in after
launch when the app has no allowed input (and recover a lost mic) -- without ever opening a Bluetooth / wireless device or
switching away from a working mic.

## 2. ESTABLISHED

### 2.1 VERIFIED -- read today

Launch and C3
- V1 The launch open is `deviceManager_.initialiseWithDefaultDevices(2, 2)` (AE:24), before `addAudioCallback`
  (AE:31). ADM:971-985 `addAudioCallback` sends a callback added while a device runs a synthetic `audioDeviceAboutToStart`,
  so the TEST-ONLY `opens` counter (CC:179) reads 1 for the launch open (the real start at AE:24 predates the callback).
- V2 MC:303 calls `setSourceMode(MicInput)` at startup; other callers MC:308, :313, :540, :545, :3976, :5676
  (`setAudioSourceModeSynced`, reached from `POST /api/audio/source` MC:2111-2112 and the perf paths MC:5706, :5811, :5868).
  AE:138-141 (mic) and AE:150-153 (file) call `setAudioDeviceSetup(setup, true)`. ADM:735-738: a differing setup sends a
  change message and proceeds; with the same names it stops, re-opens and restarts the SAME device (ADM:740, :755-759,
  :795, :808, :817). `updateSetupChannels` (ADM:711-728) re-enables the default input channels (useDefaultInputChannels
  is true), so File mode's `inputChannels.clear()` (AE:152) never closed the mic. Every re-open runs
  `CoreAudioIODevice::open` -> `CoreAudioInternal::reopen` (CA:1291-1311) for each wrapper of the combiner (CA:1530-1580).
- V3 btguard measured `opens` = 2 after launch and 4 after a file -> input switch on this rig (btguard.md:29; INFO rows
  PB:163, PB:171).
- V4 `updateXml` (ADM:915-931, reached from `setAudioDeviceSetup(..., true)` at ADM:748 / :800 / :831) writes
  `lastExplicitSettings`. ADM:219-227: a vanished open device takes JUCE's XML branch when it exists, else
  `initialiseDefault(preferred, &currentSetup)`, which re-opens the VANISHED names -> "No such device" (ADM:764-772) -> no
  device. ADM:509-516 `initialiseWithDefaultDevices` resets `lastExplicitSettings` (ADM:512).
- V5 MIDI writes it too: `MidiHandler::start` enables every available MIDI input (src/midi/MidiHandler.cpp:13-27, called
  at MC:1848); `setMidiInputDeviceEnabled` -> `updateXml()` (ADM:1117-1138, :1136). After C3 the XML exists iff a MIDI
  input existed at launch.
- V6 File playback needs no restart: AE:9 makes the transport the player's source before the device starts;
  `AudioSourcePlayer::audioDeviceAboutToStart` prepares it (juce_AudioSourcePlayer.cpp:166-180); `AudioTransportSource::
  setSource` prepares a new source when already prepared (juce_AudioTransportSource.cpp:92-98).
- V7 The callback reads `useInputForAnalysis` every block (CC:87) -- a mode switch needs no device operation.

Reconciler and hot-plug
- V8 DG:153-163: the reconciler arms its 250 ms settle only when no device exists and one existed before (`hadDevice_`);
  DG:165-199 it re-applies (`closeAudioDevice` + `initialiseWithDefaultDevices`) only while `getCurrentAudioDevice() ==
  nullptr`. So: nothing allowed at launch -> a device plugged in later is never opened; output-only (no allowed input)
  -> a mic plugged in later is never adopted (review-btguard-coreaudio-r2.md:14 X1 / X2, executed; re-run by me, 2.2).
- V9 ADM:193-233: JUCE re-inits only when the open device's NAME left the lists; a combiner's name is the OUTPUT's
  (CA:2247-2248); an output-only device is a plain CoreAudioIODevice (CA:2259); ADM:195 does nothing with no device open.
- V10 Input-lost chain: `CoreAudioInternal::timerCallback` calls `owner.stopWithPendingCallback()` when
  `updateDetailsFromDevice` fails (CA:1118-1127), which it does for a device that is not alive (CA:449-462); ->
  `stopAndGetLastCallback` (CA:1408-1416) -> the combiner's input `DeviceWrapper::audioDeviceStopped` (CA:1996) ->
  `handleAudioDeviceStopped` -> `shutdown({})` (CA:1953, CA:1735-1752): BOTH wrappers stop, the ADM's callback is told
  `audioDeviceStopped` (-> change message, ADM:1091-1093); the ADM keeps the dead combiner and its names; nothing restarts
  it (`CoreAudioIODevice::stop` clears the pending callback, CA:1334-1340).
- V11 `hasInputDevice()` is name-based (AE:96-99): with a dead input the state still reads Ok -> no notice (MC:3075-3090).
- V12 Every ADM change message republishes the TEST-ONLY status and calls `onDeviceStateChanged` (AE:159-169) ->
  `refreshAudioDeviceNotice(true)` (MC:490): the notice follows any device change with no new wiring.
- V13 Every device start and stop sends a change message (ADM:1074-1089 at :1088; ADM:1091-1101 at :1093).
- V14 App-side device-lifecycle callers: AE:24, AE:141, AE:153, DG:193-194 only (grep src/).
- V15 TEST-ONLY patterns: env `ADNA_AUDIO_DENY_DEVICES` read once under `#if AUDIODNA_TEST_SERVER` (DG:203-217); debug
  POSTs marshal with `callAsync` and answer at once (AS:2088-2098); `GET /api/debug/audio_devices` reads the
  mutex-guarded copy (AS:2077-2086, AE:267-285); TEST-ONLY routes registered at AS:298-324. AS:2075's asyncload comment is
  orphaned above `handleDebugAudioDevices` (review-btguard-coreaudio-r2.md NIT-3).
- V16 `CoreAudioInternal::stop(false)` waits up to 40 x 50 ms for the IO thread to acknowledge (CA:734-757, loop
  CA:745-751), on the calling (message) thread.
- V17 TDP:721-728 `appOpenSequence` replicates today's startup (initialise + the setSourceMode re-open); M5 (TDP:736-753)
  documents JUCE's XML branch, M5b (TDP:755-780) the no-XML branch.
- V18 tests/CMakeLists.txt:483-518 is test_device_policy (AUDIODNA_TEST_SERVER=1, JUCE_MODAL_LOOPS_PERMITTED=1);
  :1142-1170 test_bt_device_shapes links AudioCallback.cpp + AudioTap.cpp with juce_audio_formats / juce_audio_utils --
  the link template for an AudioEngine test. `ctest --test-dir build -N` = 1049 today.
- V19 CLAUDE.md = 23,999 B (wc -c). Its index line CLAUDE.md:224 already routes "AudioEngine's device open,
  GuardedAudioDeviceManager, setSourceMode" to Pitfall 61. docs/claude/integration.md has no audio-device section (grep).
- V20 Boris's words (binding-decisions.md:485-491): "we will never use bluetooth audio for any reason. it is slow and bad.
  never use it again." and "we will only use hard wired sound input or the onboard mic".
- V21 The rig's CoreAudio list today = exactly "MacBook Pro Microphone" + "MacBook Pro Speakers" (read-only enumeration
  in the SBX AE1 run).
- V22 In-app precedent for "a device returns when plugged back": the Outputs hot-plug (docs/claude/integration.md:25);
  plan5 Q6 shipped as a default, not Boris's words (.harmony/s-rta-0927-work.md:187).

### 2.2 VERIFIED -- ran in SBX (tree untouched; no app; the mock cases open no real device)

- S1 Main's reconciler (objects of build/tests/CMakeFiles/test_device_policy.dir built from 655d232 at 08:06; SBX/rig.sh
  + x_bt2_red.cpp): adopt-input (output-only launch, "USB Mic" appears) FAIL: reapplies 0, input ""; adopt-device
  (nothing at launch, a built-in pair appears) FAIL: reapplies 0, no device; input-lost ("USB Mic" open, vanishes, output
  stays) FAIL: reapplies 0, input still "USB Mic"; keep (working built-in mic, a USB mic appears and becomes the mock
  default) PASS; mono-no-reopen (launch WITHOUT the re-open, mono mic, open output vanishes) PASS: JUCE alone -> no
  device, reconciler -> 1 re-apply -> "Speakers B"; opens replica PASS: starts launch 1 / mic 2 / file 3 / mic 4, XML set,
  active inputs after "file" = 1. Totals: 6 cases, 3 failed, 7 failed assertions.
- S2 Main's REAL AudioEngine (AE + AudioCallback + AudioTap + DG + DP + CoreAudioDeviceInfo compiled with
  test_bt_device_shapes' flags + AUDIODNA_TEST_SERVER=1; SBX/rig2.sh + x_c3_engine.cpp): every real device denied through
  the env after a precheck that the guard lists nothing -> `setSourceMode(MicInput)`: JUCE's XML SET, 1 change message
  (2 after file -> input) -> 3 failed assertions. The same test against a scratch copy of AE with the two blocks deleted:
  all 8 assertions pass.
- S3 A prototype of this design (SBX/proto/audio/*, SBX/rig3.sh): TDP's 36 existing cases 36/36 (201 assertions, E1 / E2
  on the real HAL included), the 6 S1 cases 6/6, 6 new-API cases 6/6.
- S4 Mutants of the prototype (SBX/mut.sh), each KILLED: no AdoptInput -> adopt-input, failed-adoption, table, round trip,
  storm; no InputLost -> input-lost, table, round trip; "switch when > 1 input is listed" -> keep, R2; no restore ->
  failed-adoption; launch scan not recorded -> launch-scan, failed-adoption, R4b; no settle -> R1, storm.

### 2.3 INFERRED (not established)

- I1 A really unplugged device reports DeviceIsAlive = 0 and JUCE's per-device listener fires (V10's HAL premise).
- I2 Closing a DEAD device on the message thread stalls it up to 2 s (V16 + I1: its IO proc no longer runs).
- I3 Plugging a device fires no DeviceHasChanged / OwnedObjects on an OTHER, already-open device (so no HAL-thread
  `restartAsync`, CA:1637-1652, races an adoption).
- I4 macOS may publish a new default input after the device-list notification; the guard answers the default from its
  last scan (review-btguard-coreaudio-r1.md NIT-4).
- I5 The runtime deny is the policy-level twin of a plug: the same code from `GuardedDeviceType::rebuild` down (decorator,
  JUCE's list-change handler, reconciler), not the HAL side (per-device detail timers, a dead input's combiner shutdown).

## 3. DESIGN FORKS

F1 -- what happens when the app has no allowed input and one appears
- (a) CHOSEN: automatic re-apply of the launch policy (the existing close + `initialiseWithDefaultDevices` through the
  guarded type). It is the code the launch and the merged device-gone path already run (M5b, R1-R5), and
  `insertDefaultDeviceNames` pairs input and output by a common sample rate across ALL allowed devices (ADM:518-606).
- (b) input-only adoption (`setAudioDeviceSetup` with the current output + the policy's input). Loses: a second open path;
  it pins the output even when the new mic shares no rate with it (the pairing is bypassed -> an open failure loses the
  working output); keeping the output buys nothing -- the app's output is silence (CC:169-172) and the device object is
  re-created anyway (needsNewDevice, ADM:755-788).
- (c) reword the notice ("... and restart the app", review r2 option 1). Loses: F3's deaf-after-unplug state has no notice
  to reword; a relaunch mid-show costs far more than one device re-open.
- (d) a "Rescan" button. Loses: new UI; Harmony constraint: no device-picker UI.

F2 -- the trigger
- (a) CHOSEN: the existing ADM change message + the 250 ms settle timer (event-driven; the guard rebuilds before JUCE's
  listeners hear of a change, DGH:15-18).
- (b) a polling timer -- periodic work duplicating the HAL notification. (c) a HAL listener on
  kAudioHardwarePropertyDefaultInputDevice -- a second listener outside JUCE; the list change already carries the event.

F3 -- which "no allowed input" states re-apply
- (a) CHOSEN: no device (plan-btguard 2.5 case i), no input (iii), and the open input vanished (ii, input side = btguard
  loose end 1).
- (b) (i) + (iii) only (the literal goal). Loses: V10 / V11 -- a pulled wired-mic cable stops the whole device with state
  "ok" and no notice, and re-plugging never restarts it; (iii) cannot see that state (its input name is not empty).
  Kept separable (item I4).

F4 -- C3's teeth
- (a) CHOSEN: probe A6 / A6b on `opens` (RED on main: 2 / 4) + ctest AE1 on the REAL AudioEngine with every device denied
  (RED on main VERIFIED S2; zero device side effects).
- (b) an AudioEngine test constructor injecting the mock type. Loses: a production API change to test a deletion; cannot
  compile on main (no RED on main).
- (c) a ctest opening the real speakers output-only. Loses: device side effects in a unit test, rig-dependent.

F5 -- the live stimulus
- (a) CHOSEN: TEST-ONLY `POST /api/debug/audio_deny {"names": [...]}` -- replaces the denied set, runs the guard's
  list-change path on the message thread; the probe decides when.
- (b) an env var that denies for N ms. Loses: races the <= 60 s health wait; cannot do "lose, then re-plug".
- (c) real hardware / a system audio setting. Forbidden.

F6 -- the "plugged in after launch" baseline
- (a) CHOSEN: `DeviceReconciler::openDefaultDevices()` is the app's ONE device choice (launch + every re-apply) and records
  the scan seq it ran on; a rule acts only on a newer scan.
- (b) the first change message's seq. Loses: misses a device plugged in between the launch scan and that message.
- (c) keep `hadDevice_`. Loses: cannot express "nothing was ever open, now something is allowed".

F7 -- telling the performer
- (a) CHOSEN: while the source is the mic, each successful re-apply writes "Mic: <input> @ <rate>Hz" (or the no-wired-mic
  status) into the existing file label; the notice stays the persistent indicator (BG6).
- (b) rewrite the label only when it already reads "Mic: ...". Loses: a fall-back to the onboard mic is silent whenever
  the label shows something else (vj S1 "silent swap").
- (c) a new status element. Loses: new UI surface; Boris Q2.

## 4. ITEMS (commit order K1 -> K4; each builds and passes ctest; each item's RED is recorded before its GREEN)

### I1 -- TEST-ONLY runtime device deny + the probe rows (K1; no behaviour change)
Files / functions
- DGH / DG `GuardedDeviceType::setTestDeniedNames(const juce::StringArray&)` (`#if AUDIODNA_TEST_SERVER`):
  `config_.testDeniedNames = names; inner_->scanForDevices(); audioDeviceListChanged();` (rebuild +
  `callDeviceChangeListeners` = the path `CoreAudioIODeviceType::audioDeviceListChanged` takes, CA:2266-2270, minus the
  HAL trigger); stderr `[AudioEngine] TEST-ONLY audio_deny: <names | none>`.
- DGH / DG `GuardedAudioDeviceManager::setTestDeniedNames(...)` (#if): updates `config_` (so `policyConfig()` and the
  status field `test_denied` follow) and forwards to `guarded_` when non-null.
- AEH / AE `void debugSetDeniedDevices(const juce::StringArray&)` (#if; message thread) -> `deviceManager_.setTestDeniedNames`.
- ASH: in the `#if AUDIODNA_TEST_SERVER` public block (ASH:204-208) `std::function<void(const juce::StringArray& names)>
  onDebugAudioDeny;`; private decl beside ASH:272 `void handleDebugAudioDeny(const httplib::Request&, httplib::Response&);`.
- AS: route after AS:323 `server_.Post("/api/debug/audio_deny", ...)`; handler after `handleDebugAudioDevices`
  (AS:2077-2086): 400 `jsonError("names (an array of device names) required")` unless `names` is an array of strings
  (`[]` allowed); 503 when unwired; `juce::MessageManager::callAsync([this, names] { onDebugAudioDeny(names); })`; 200
  `jsonOk()`. Move the orphaned comment at AS:2075 down to `handleDebugCancelLoad` (same hunk; r2 NIT-3).
- MC: inside the existing `#if AUDIODNA_TEST_SERVER` block MC:2148-2150: `apiServer_->onDebugAudioDeny = [this](const
  juce::StringArray& names) { audioEngine_.debugSetDeniedDevices(names); };`
- PB: A6 / A6b become gated; new rows below; helpers `deny(names)` (POST, Connection: close) and a poll of
  `/api/debug/audio_devices` every 0.25 s with a deadline; `last_reapply` read as `d.get('last_reapply', '')`; header
  (PB:9-15) lists the rows and says plainly that the runtime deny drives the policy's list-change path, not the HAL.
Pre-registered row bars (IN / OUTN = arm A's `opened{}`; NOTICE_IN / NOTICE_DEV as PB:136-137; MIC(r) = "Mic: " + IN +
" @ " + str(int(r)) + "Hz"; NOMIC = "Mic: no wired mic (Bluetooth is never used)"; every row that POSTs requires 200):
- A6 `opens == 1` after launch. A6b `opens == 1` after A7's file -> input.
- A10 deny [IN] -> within 3 s: reapplies 1, last_reapply "input-lost", state "no-input", opened.input "", opened.output
  == OUTN, audio_notice == NOTICE_IN, file_label == NOMIC.
- A11a deny [] -> at +2 s reapplies still 1 (the 10 s bound). A11 within 13 s of that POST: reapplies 2, last_reapply
  "adopt-input", state "ok", opened.input == IN, opened.output == OUTN, audio_notice "", file_label ==
  MIC(opened.sample_rate), `/api/features` sourceSampleRate == opened.sample_rate.
- A12 (coalescing guard; starts >= 10.5 s after A11's re-apply): deny [IN], 0.05 s, deny [] -> after 1.5 s reapplies
  still 2 and opened{} byte-identical to A11's. INFO A6c: opens after A12 (expected 3).
- B7 (arm B after B4 / B5; B3b left the source on input) deny [] -> within 3 s: reapplies 1, last_reapply "adopt-input",
  state "ok", opened.input == IN, input_channels >= 1, audio_notice "", file_label == MIC(rate). B7b `opens == 2`.
  B8 after 2 more s: reapplies still 1.
- C7 (arm C after C5b) deny [IN] -> within 3 s: reapplies 1, last_reapply "no-device", state "no-input", opened.output
  == OUTN, opened.input "", audio_notice == NOTICE_IN. C8a deny [] -> at +2 s reapplies still 1. C8 within 13 s:
  reapplies 2, last_reapply "adopt-input", state "ok", opened.input == IN, audio_notice "".
- A10-A12 print SKIP (never counted) when arm A opened no input (mirrors D1 / D2).
RED (the lane's K1 build, a normal cmake build): predicted 26 PASS / 11 FAIL / 2 SKIP -- FAIL A6 (2), A6b (4), A10
(reapplies 0, state "ok"), A11a, A11, B7 (stays "no-input" = X2 live), B7b (opens 3), B8, C7 (stays "no-device" = X1 live),
C8a, C8; A12 passes vacuously there (a guard row). Record the actual table.
GREEN: A6 / A6b at K2, the rest at K3.
Risk: I5 (policy twin, not the HAL) -- stated in the probe header and the pitfall.

### I2 -- C3: one device open per launch (K2)
Files / functions
- AE `setSourceMode` (AE:126-157): delete AE:138-141 and AE:150-153 (both `setup` locals and `setAudioDeviceSetup`
  calls). Keep `sourceMode_`, `stop()`, both `useInputForAnalysis` stores and both stderr lines (PB A8 counts "Switched to
  mic input mode"). Comment: "bt2 C3 (Pitfall 61): the source mode is an atomic flag CombinedCallback reads every block --
  never a device call here. The old setAudioDeviceSetup re-open changed nothing (JUCE re-enables the default input
  channels, updateSetupChannels) and sent the device through JUCE 8.0.4's CoreAudio open again at launch and on every
  switch."
- MC:5697-5699 (perfRecord comment only): "(a mode change can restart the device at a different rate/channel count)" ->
  "(since bt2 C3 a mode change never restarts the device; reading after the switch stays correct)".
- NEW tests/test_audio_engine_devices.cpp, whole body `#if JUCE_MAC`, case AE1 (= SBX/x_c3_engine.cpp): enumerate real
  names (read-only) -> `setenv("ADNA_AUDIO_DENY_DEVICES", all names joined by ';')` -> SAFETY precheck: a
  `GuardedDeviceType` over `createAudioIODeviceType_CoreAudio()` with `devguard::productionConfig()` lists NO input and NO
  output (REQUIRE -- otherwise no engine is built: nothing may open, no TCC prompt) -> construct `AudioEngine` ->
  `unsetenv` -> REQUIRE no device -> `getDeviceManager().dispatchPendingMessages()` -> add a counting ChangeListener ->
  `setSourceMode(MicInput)` -> dispatch -> CHECK `createStateXml() == nullptr` and 0 change messages -> File, MicInput,
  dispatch -> CHECK still 0.
- tests/CMakeLists.txt: new target `test_audio_engine_devices` right after TDP's block (after :518): sources the test +
  audio/AudioEngine.cpp, audio/AudioCallback.cpp, recording/AudioTap.cpp, audio/DeviceGuard.cpp, audio/DevicePolicy.cpp,
  audio/CoreAudioDeviceInfo.cpp; link as test_bt_device_shapes (:1147-1155); defs `_USE_MATH_DEFINES JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0 AUDIODNA_TEST_SERVER=1`; the same -Wno-* options, `apply_sanitizers`,
  `catch_discover_tests`. Targets 111 -> 112.
- TDP: `appOpenSequence` (TDP:721-728) -> `appLaunch(adm)` = initialise only (the app's launch since C3). M5 keeps its
  explicit-settings step INLINE, retitled "JUCE's XML branch (explicit settings: a MIDI input enabled at launch --
  MidiHandler::start -- or any treatAsChosenDevice setup) lands on an ALLOWED device"; M5b retitled "(no explicit
  settings -- the app's launch since C3)"; NEW M5c = M5b with a MONO mic (S1 "mono-no-reopen"; PASS on main -- C3's safety
  guard).
Behaviour change: opens 2 -> 1 at launch and +2 -> +0 per file <-> input round trip; a mode switch sends no change message
and writes no explicit settings; the mono built-in mic's vanished-device recovery moves from JUCE's XML branch (immediate)
to the reconciler (250 ms) unless a MIDI input existed at launch.
RED: AE1 on 655d232 FAILs 3 assertions (S2); probe A6 (2) / A6b (4) FAIL on the pre-merge copy.
GREEN: AE1 passes (S2 scratch copy: 8/8); A6 == 1 and A6b == 1 on 5 of 5 runs; A7 / A8 unchanged; the audio battery (G8).
Risks: R5, R6.

### I3 -- the reconciler adopts an allowed device when the app has none (K3)
Files / functions
- DPH / DP (pure, juce_core only): `enum class Reapply { None, NoDevice, AdoptInput, InputLost };`
  `Reapply reconcile(bool haveDevice, const juce::String& openedInput, const Lists&);`
  `juce::String toString(Reapply);` -> "", "no-device", "adopt-input", "input-lost". Table:
  haveDevice && openedInput non-empty -> `lists.inputs.contains(openedInput) ? None : InputLost`;
  haveDevice && openedInput empty -> `lists.inputs.isEmpty() ? None : AdoptInput`;
  !haveDevice -> `(inputs and outputs empty) ? None : NoDevice`.
- DGH / DG `DeviceReconciler` (validated shape: SBX/proto/audio/DeviceGuard.cpp):
  - public `juce::String openDefaultDevices()`: close the device if one is open; `initialiseWithDefaultDevices(numIns_,
    numOuts_)`; THEN `lastAttemptSeq_ = guardedType()->lastScan().seq` (after the open: a rescan the open itself caused --
    the combiner's rate reconcile, CA:1947 -- is not "new"); return the error.
  - `changeListenerCallback`: always `startTimer(settleMs_)` (a restart: evaluated after the LAST change message).
  - `timerCallback`, in this order:
    ```
    stopTimer(); guarded == nullptr -> return
    action = dp::reconcile(device != nullptr, setup.inputDeviceName, scan.lists); None -> return
    scan.seq == lastAttemptSeq_ -> return                      // this scan was acted on (or opened at launch)
    reapplies_ > 0 && now - lastReapplyMs_ < minIntervalMs_ -> startTimer(remaining); return   // unchanged bound
    lastReapplyMs_ = now; ++reapplies_; lastAction_ = action
    cerr "[AudioEngine] re-applying the device policy (<code>)"
    lists = copy of scan.lists; previous = getAudioDeviceSetup(); hadDevice = device != nullptr
    error = openDefaultDevices()
    if (hadDevice && no device now): keep = previous with each name cleared unless in lists;
        if a name remains: setAudioDeviceSetup(keep, false); cerr "the re-apply failed (<error>); kept the previous
        device[ -- that failed too: <e>]"
    onReapplied(error)
    ```
  - remove `hadDevice_` and `lastReappliedSeq_`; add `uint64_t lastAttemptSeq_ = 0;`, `Reapply lastAction_ = None;`,
    `Reapply lastAction() const noexcept`. Rewrite the class comment (DGH:80-88) to the table and gates above ("acts
    while a device exists only for adopt-input / input-lost").
- AEH / AE: AE:24 -> `auto result = deviceReconciler_.openDefaultDevices();`; the `onReapplied` lambda (AE:14-18) also
  calls `if (onDevicesReapplied) onDevicesReapplied(error);`; AEH beside `onDeviceStateChanged` (AEH:47-48):
  `std::function<void(const juce::String& error)> onDevicesReapplied;` (message thread); `publishDeviceStatus` adds
  `"last_reapply"` = `toString(deviceReconciler_.lastAction())`.
- TDP: `appLaunch` -> `reconciler.openDefaultDevices()` wherever a reconciler exists (M5b, M5c, R1-R5, the new cases).
Tests (TDP), RED on 655d232 with main's API only (source: SBX/x_bt2_red.cpp):
- R6 adopt a mic: {Denied Wireless (default in + out), Built-in Speakers} -> output-only; add "USB Mic" (USB, 1 in),
  `fireListChanged`, pump 800 -> reapplies 1, input "USB Mic", output "Built-in Speakers", `!spy.saw(kDenied)`.
  Main: FAIL (reapplies 0, input "").
- R7 adopt a device: {Denied Wireless} -> no device; add Built-in Mic + Built-in Speakers -> reapplies 1, a device,
  input "Built-in Mic". Main: FAIL (3 assertions).
- R9 a working mic is never switched: {Built-in Mic, Built-in Speakers} open; add "USB Mic" and make it the mock default
  -> reapplies 0, input unchanged, no new start. Main: PASS (a guard; kills "switch" mutants).
New-API guards (source: SBX/x_bt2_new.cpp; mutation-verified S4):
- P14 the reconcile table (8 rows).
- R10 the launch's scan is never re-tried by itself: failOpen the speakers -> `openDefaultDevices()` fails -> pump 800
  -> reapplies 0; clear failOpen; `fireListChanged` -> reapplies 1, a device.
- R11 a failed adoption puts the output back: output-only; failOpen "USB Mic"; add it; fire -> reapplies 1, device open,
  output "Built-in Speakers", input ""; +800 ms still 1; clear; fire -> 2, input "USB Mic".
- M8 the launch (`openDefaultDevices`) starts the device once and writes no explicit settings (1 start,
  `createStateXml() == nullptr`, reapplies 0).
Behaviour change: when no device is open, a device plugged in after launch is opened; when the open device has no
input, a mic plugged in after launch is adopted; never on the launch's own scan; the notice clears through V12.
GREEN: TDP all pass (36 existing + 11 new); live B7 / B7b / B8 / C7 / C8a / C8 on 5 of 5 runs.
Risks: R1, R8, R9, R11.

### I4 -- the open mic is gone -> re-apply (btguard loose end 1; K3, separable)
Files: the InputLost row of `dp::reconcile` -- nothing else.
Tests: R8 input lost ({Denied Wireless, "USB Mic" default in, Built-in Mic, Built-in Speakers}; "USB Mic" open; remove
it; fire -> reapplies 1, input "Built-in Mic"; main: FAIL, reapplies 0, input still "USB Mic"); R12 round trip with no
onboard mic (lost -> output-only; the mic returns -> adopted; reapplies 1 -> 2; new API). Live A10 / A11a / A11 / A12.
Behaviour change: a vanished (or newly hidden) open input re-applies the policy -> the macOS default / built-in mic, else
output-only + the notice. Dropping I4 = drop R8, R12, A10-A12 (A11 needs A10).
Risks: R2, R3.

### I5 -- the Mic label follows a re-apply (K3)
MC next to MC:490: `audioEngine_.onDevicesReapplied = [this](const juce::String& error) { if (error.isEmpty() &&
audioEngine_.getSourceMode() == AudioEngine::SourceMode::MicInput) setFileLabel("Mic: " + audioEngine_.getDeviceStatus()); };`
(`setFileLabel` keeps the staged-load diversion, MC:3068-3073; a failed re-apply still reaches the label through
`onError`, AE:16-17.)
Tests: live A10 / A11 / B7 `file_label` bars (MainComponent has no unit seam here).
Behaviour change: after every automatic device change while listening to the mic, the file label names the mic now in
use. Text in an existing label only; Harmony may run the visual critic seat on `BTGUARD_SHOTS=1` window-only captures.
Risk: R7.

### I6 -- docs (K4): section 6.

## 5. GATES -- Harmony, after the merge (pre-registered decision rules)

- G1 Build clean; no new warning in a touched file.
- G2 `ctest --test-dir build -j1`: 100% pass; expected 1049 + 12 = 1061 cases (TDP +11, test_audio_engine_devices +1),
  112 targets (the builder records the real `ctest -N`). One failure = RED; a flake verdict needs >= 5 runs.
- G3 RED re-check on 655d232's sources (Harmony or a reviewer seat, the SBX/rig.sh method): R6, R7, R8 and AE1 FAIL;
  R9 and M5c PASS. A predicted-RED case that passes on 655d232 is toothless -> back to the builder.
- G4 Mutation table on the real code (source copies -> scratch objects -> relinked test binaries; never an app): no
  AdoptInput, no InputLost, switch-a-working-mic, no restore, launch seq not recorded, no settle -- each killed by the
  cases named in S4.
- G5 TCC / dialog tripwire before the first live run of the merged binary: one launch, `log show` tccd AUTHREQ for
  com.audiodna.app in that window = none new; UserNotificationCenter windows (kCGWindowListOptionAll) 0 at >= 15 s after
  the quit. Any prompt -> STOP (Boris's screen; never dismiss it).
- G6 probe-btguard RED on the pre-merge app COPY (655d232): 25 PASS / 12 FAIL / 2 SKIP, the FAILs exactly {A6, A6b, A10,
  A11a, A11, A12, B7, B7b, B8, C7, C8a, C8} (A6 reads 2, A6b 4, the rest the missing route). Another FAIL = rig problem
  -> investigate before merging; a predicted FAIL that passes = toothless row -> fix the row.
- G7 probe-btguard GREEN on merged main, 5 of 5 runs: 37 PASS / 0 FAIL / 2 SKIP, every quit clean (0 .ips, 0 dialogs).
  Any FAIL -> investigate. If A6 ever reads opens > 1 with reapplies 0: a C3 regression reads 2 on EVERY run, a JUCE
  detail restart (CA:1118-1130) only sometimes -> 5 more runs before a verdict.
- G8 Audio battery on merged main, one run each (first `ps -Ao pcpu=,etime=,comm= | sort -rn | head` for orphaned
  burners): step3 94/0, manual-bpm 22/0, resync 16/0, downbeat-level 14/0, onset-render 13/0, tempo-start 10/0,
  finalize-loop 8/0 (40 cycles), routines 109/0, async-load (its audio witness rows). A FAIL -> 5 interleaved runs
  merged-vs-pre-merge-copy before attributing it to bt2.
- G9 Production absence: compile AE, DG, DP, AS and MC from compile_commands with -DAUDIODNA_TEST_SERVER removed -> they
  build; `strings` of those objects: 0 x "audio_deny", 0 x "ADNA_AUDIO_DENY_DEVICES"; grep: every new debug symbol sits
  inside `#if AUDIODNA_TEST_SERVER`.
- G10 Sacred rules: `git diff 655d232..HEAD -- src/audio/CombinedCallback.h src/audio/AudioCallback.cpp` is empty; no new
  std::mutex outside `#if AUDIODNA_TEST_SERVER`; the reconciler and the hook are message-thread only.
- G11 CLAUDE.md still 23,999 B (this plan adds nothing there).
- No perf gate. INFO only: launch -> /api/health time, interleaved A/B >= 5 runs per arm (C3 removes one device open).

## 6. DOCS

- docs/claude/pitfalls.md Pitfall 61 (pitfalls.md:131): replace the text from "The startup `setSourceMode` re-open stays"
  through "a newly plugged device is not adopted." with (verbatim):
  "One device open per launch (s-rta-0930 bt2): `setSourceMode` is an atomic flag flip -- CombinedCallback reads the mode
  every block; its old `setAudioDeviceSetup(setup, true)` re-open changed nothing (JUCE re-enables the default input
  channels, so File mode kept the mic open anyway) but sent the device through JUCE 8.0.4's CoreAudio open again at launch
  and on every source switch; never add a device call to it. `DeviceReconciler::openDefaultDevices()` is the app's ONE
  device choice -- at launch and on every re-apply (close, then JUCE's default open through the guarded type) -- and
  records the device scan it ran on. The reconciler re-applies it 250 ms after the LAST device change message, at most
  once per 10 s, never twice on the same scan (the launch's included, so only a device plugged in AFTER launch counts), in
  exactly three cases (`devpolicy::reconcile`): no device is open and an allowed one is listed (the open device vanished
  and JUCE's own re-init re-opened the vanished name -- "No such device"; JUCE takes its XML branch instead only when
  `lastExplicitSettings` exists, which since bt2 means a MIDI input was enabled at launch, `MidiHandler::start` -- or
  nothing was allowed at launch and a device was plugged in); the open device has no input and an allowed input is listed
  (the notice's "plug one in"); the open INPUT vanished while its output stays (JUCE's combiner stops the whole device and
  never re-inits it -- its name is the OUTPUT's -- so the app sat deaf with no notice): the policy falls back to the macOS
  default / built-in mic, else output-only + the notice. A still-listed (working) input is NEVER switched away from, even
  when macOS makes a newly plugged mic the default. A re-apply that fails while a device was open puts back what is still
  listed of it (output-only) -- it never leaves the app worse off. While the app listens to the mic, each re-apply writes
  "Mic: <input> @ <rate>Hz" (or the no-wired-mic status) to the file label."
  and append to its "Guards:" list: "`tests/test_device_policy.cpp` M5c (a mono mic without the old re-open: JUCE alone
  ends with no device, the reconciler re-applies once), M8 (the launch starts the device once, no explicit settings),
  P14 (the reconcile table), R6 / R7 / R8 (adopt a mic / a device, the open mic gone -- RED on 655d232), R9 (a working mic
  is never switched), R10 (the launch's scan is never re-tried by itself), R11 (a failed adoption puts the output back),
  R12 (lost -> output-only -> re-plugged -> adopted); `tests/test_audio_engine_devices.cpp` AE1 (the REAL AudioEngine,
  every device denied so nothing opens: `setSourceMode` sends no change message and writes no explicit settings); live
  `.harmony/probe-btguard.sh` A6 / A6b (opens == 1 after launch and after file -> input), A10-A12, B7-B8, C7-C8 --
  TEST-ONLY `POST /api/debug/audio_deny {"names": [...]}` replaces the denied set at runtime and runs the guard's
  device-list-change path: the policy-level stand-in for plugging / unplugging, not the HAL's own notifications."
- docs/claude/testing-eyes.md:19, append: "`POST /api/debug/audio_deny {"names": ["<name>", ...]}` (same build path; `[]`
  = none) replaces that set at runtime and runs the guard's device-list-change path on the message thread (inner rescan
  -> rebuild -> JUCE's listeners -> the reconciler) -- the probe's stand-in for plugging / unplugging a device; the status
  gains `last_reapply` (`no-device` / `adopt-input` / `input-lost`, "" before the first re-apply)."
- docs/claude/architecture.md:376 (item 4), append: "; a device plugged in while the notice shows is adopted about 0.3 s
  later (at most one device re-open per 10 s), and a working mic is never switched away from (Pitfall 61)".
- .harmony/APP-INVENTORY.md: row 31 counts (the real `ctest -N` and target count); the btguard paragraph (:230-234) +
  "s-rta-0930 bt2: TEST-ONLY `POST /api/debug/audio_deny {"names": [...]}` swaps the denied set at runtime (the plug /
  unplug stand-in); `/api/debug/audio_devices` += `last_reapply`; ctest +1 Catch2 target (`test_audio_engine_devices`)."
- CLAUDE.md: NO change (0 B; CLAUDE.md:224 already sends "AudioEngine's device open ... setSourceMode" to Pitfall 61). No
  new pitfall number (Pitfall 61 grows; 63 stays free).

## 7. RISKS (strongest counterargument first; each with the cheapest discriminating test)

- R1 "Adoption (and InputLost) acts while a device EXISTS -- the exact close / open on the message thread BG4 fenced off
  ('only while the manager has no device object, so no JUCE restart can be pending'); rewording the notice would add zero
  CoreAudio passes." Why it loses: a reword cannot fix V10 (a pulled cable leaves the app deaf with NO notice); JUCE's
  own message-thread restarts (CA:1118-1130, CA:1372-1381, CA:1728-1733) run on the same thread as the reconciler's timer
  and each start re-arms the 250 ms settle (V13); the HAL-thread `restartAsync` needs DeviceHasChanged / OwnedObjects on
  the OPEN device, which plugging a different device does not fire (I3) -- the same exposure JUCE's own device switch
  has; and net passes go DOWN (C3 removes one open per launch and two per mode round trip; adoption adds one per plug
  event, <= 1 per 10 s). Test: G7's A10 / A11 cycle on 5 runs = 10 real re-applies of the built-in devices, 0 .ips,
  0 dialogs.
- R2 Closing a DEAD device (InputLost after a real unplug) may stall the message thread up to 2 s (V16 + I2) -- the same
  stall JUCE's own output-unplug re-init has today; the GL thread keeps drawing (INFERRED). Not measurable on the rig
  (the runtime deny keeps the device alive) -> Boris check B2.
- R3 After a drop-out the app stays on the onboard mic even when the wired mic returns (default "do not switch") -> Boris
  Q1; the Outputs already bring a display's window back (V22).
- R4 The live stimulus is the policy twin of a plug (I5): the HAL side (per-device timers, a dead input's shutdown) is
  covered only by the JUCE source chain (V10) and the mock cases. Named in the probe header and the pitfall.
- R5 C3 moves the mono built-in mic's vanished-device recovery from JUCE's XML branch (immediate) to the reconciler
  (250 ms) when no MIDI input existed at launch. Test: M5c (VERIFIED green on main's reconciler, S1).
- R6 MIDI presence changes JUCE's fallback path (V5): with a controller plugged in, JUCE re-inits by itself (M5 covers
  that branch lands on an allowed device); without one, the reconciler does (M5b / M5c).
- R7 I5 overwrites a file / image name shown in the label when a re-apply happens in mic mode -- intended (the label is
  the transient status line, BG6), only on device events -> Boris Q2.
- R8 An adoption re-makes the launch choice for BOTH directions, so the OUTPUT can change if macOS's default output
  changed since launch or the rate pairing picks another pair -- silent (CC:169-172). Test: A11 / B7 assert
  opened.output == OUTN on the rig.
- R9 The unchanged 10 s bound delays a second re-apply (e.g. a quick re-plug on a Mac with no onboard mic) by up to 10 s.
  Kept: it is the storm bound. Pre-registered live in A11a / C8a.
- R10 AE1 builds a REAL AudioEngine on the real HAL: a device appearing between the precheck and the engine's own scan
  (milliseconds) would be opened -> if it were an input, a TCC prompt. Mitigation: the precheck REQUIRE; `#if JUCE_MAC`;
  the residual race is named. (A `*` deny-all hook would remove it but cannot run on 655d232, which AE1's RED needs.)
- R11 The guard answers the default from its last scan (I4): with SEVERAL new inputs at once the policy may pick the
  built-in / first USB rather than a just-published macOS default. Rare; allowed devices only.
- R12 Two same-named devices, the open one unplugged and the other renamed to its name (dedupe swap): the open name stays
  listed -> Keep while the combiner is dead. Unit-uncovered; INFERRED rare.
- R13 A re-apply failure puts JUCE's own error text in the label through `onError` (pre-existing behaviour of the gone
  path; not plain words). NIT, not changed here.
- R14 probe-btguard gets ~50 s longer (two 10 s bounds + one 10.5 s coalescing wait + settles).

## 8. WHAT ONLY BORIS CAN CHECK

- B1 A normal launch looks and feels the same (the app now opens the sound device once instead of twice).
- B2 Only if he uses a wired mic / interface (the rig has none, so the tests simulate it): (a) with the "No wired mic found
  - plug one in" note showing (a Mac with no built-in mic in use), plug it in -> within about a second the note goes and
  the visuals react; (b) pull its cable while the app runs -> within about a second the app listens on the MacBook's own
  mic (the text line reads "Mic: MacBook Pro Microphone @ 48000Hz" when the source is the mic) -- and whether the controls
  pause for a moment (up to ~2 s possible, R2); (c) plug it back -> by default the app stays on the MacBook mic until a
  relaunch (Q1).
- B3 Whether the "Mic: ..." line after an automatic change reads right (Q2).

## 9. QUESTIONS (Boris's; each has a default so the build does not block)

- Q1 Drop-out: "If your wired mic drops out mid-show, the app switches to the MacBook's own mic within about a second.
  When you plug the wired mic back in, what should happen?" (a) DEFAULT: stay on the MacBook mic until you relaunch -- the
  app never switches away from a mic that works; (b) go back to the wired mic by itself (like an output window that comes
  back when its display is plugged back); (c) never fall back to the MacBook mic: show the "plug one in" note and wait for
  the wired mic.
- Q2 Visibility: "When the app changes the mic by itself, the text line beside the audio controls says 'Mic: <name>'
  (while the app listens to the mic), and the yellow note appears if no mic is left. Is that enough, or do you want
  something more visible?" DEFAULT: enough -- no pop-up, no new element.
Harmony decisions requested (defaults): H1 ship I4 (default yes); H2 the restore-on-failure in I3 (default yes); H3 the
label write I5 (default yes); H4 the 10 s-bound and coalescing live rows A11a / A12 / C8a (default yes).

## 10. CROSS-LANE TOUCHES (for merge sequencing)

- src/MainComponent.cpp: bt2 touches ONLY MC:~490 (one `onDevicesReapplied` lambda beside `onDeviceStateChanged`),
  MC:2148-2150 (one wiring line inside the existing `#if AUDIODNA_TEST_SERVER` block) and MC:5697-5699 (comment only, in
  `perfRecord`). Lane tsan's MainComponent regions (the TSan table's trigger / deck-switch writers at MC:4614, :4820,
  :5113, :5492) do not overlap.
- src/api/ApiServer.{h,cpp}: TEST-ONLY hunks only -- route list AS:~323, handler after AS:2086 (+ the AS:2075 comment
  move), ASH:204-208 and ASH:~272. Neither tsan nor gop2 lists ApiServer.
- tests/CMakeLists.txt: one target appended after :518 (append-only; other lanes' targets merge trivially).
- .harmony/APP-INVENTORY.md row 31 counts and docs/claude/pitfalls.md (bt2 edits Pitfall 61 in place; other lanes append
  63+): doc-only conflicts Harmony resolves.
- std::cerr: bt2 adds message-thread stderr lines in DG (the class of lane tsan's cerr races); if tsan introduces a
  serialized logger, sweep DG's lines after both merges.
- Nothing in gop2's territory (VideoPlayer*, GopCache*, video probes).

## 11. LEARNINGS (for Harmony to capture; not logged from here -- outside this dispatch's write fence)

- A real AudioEngine is unit-testable headlessly with zero device side effects: deny every real device through the
  TEST-ONLY env and REQUIRE, before constructing the engine, that a GuardedDeviceType over the real CoreAudio type with
  `productionConfig()` lists nothing (no device opens, no TCC prompt).
- `ChangeBroadcaster::dispatchPendingMessages()` delivers a pending AudioDeviceManager change message synchronously --
  count change messages in a test without JUCE_MODAL_LOOPS_PERMITTED.
- JUCE 8.0.4's combiner stops the WHOLE device when its input dies and the ADM never re-inits (the name is the output's):
  a pulled input cable leaves an app deaf with its setup names intact.
- `setMidiInputDeviceEnabled` writes the ADM's `lastExplicitSettings` (updateXml): MIDI presence changes JUCE's audio
  list-change fallback path.

## COMPACT

- C3: delete AE:138-141 / AE:150-153 -> opens 2 -> 1 at launch, +0 per source switch; teeth = probe A6 / A6b (RED on main
  2 / 4) + ctest AE1 (the real AudioEngine, every device denied, nothing opened: RED on 655d232 VERIFIED, GREEN on a C3
  scratch copy); its fallback side effect is already covered by the merged reconciler (VERIFIED, M5c).
- Reconcile: pure `devpolicy::reconcile` -> None / NoDevice / AdoptInput / InputLost; the one existing re-apply (close +
  `initialiseWithDefaultDevices` through the guarded type); settle 250 ms after the last change, <= 1 per 10 s, never
  twice per scan, the launch's scan recorded by `openDefaultDevices()`; a working (listed) mic is never switched; a
  failed re-apply restores the still-listed half; the Mic label follows.
- Tests: TDP +11 (R6 / R7 / R8 RED on 655d232 VERIFIED; R9 / M5c guards; P14 / R10 / R11 / R12 / M8 new API; 6 / 6 mutants
  killed on a prototype that also passes the 36 existing cases); new target test_audio_engine_devices (AE1).
- Live: TEST-ONLY `POST /api/debug/audio_deny`; probe-btguard +12 gated rows; RED on the pre-merge copy 25 / 12 / 2,
  GREEN 37 / 0 / 2 on 5 of 5 runs; the audio battery stays green.
- Boris: Q1 drop-out re-plug (default: stay on the MacBook mic), Q2 visibility (default: the Mic line + the note).
- Fence: src/audio (AudioEngine, DeviceGuard, DevicePolicy), MC ~490 / 2148-2150 / 5697-5699, ApiServer TEST-ONLY hunks,
  tests (+1 target), probe-btguard, docs; CLAUDE.md untouched.

STATUS: COMPLETE -- plan ready for the builder; 2 Boris questions and 4 Harmony decisions defaulted; no source edited; scratch evidence in SBX.
