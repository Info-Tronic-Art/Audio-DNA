# ATTACK plan-btguard -- CoreAudio/JUCE device-lifecycle seat (blind). Labels: V verified in source today / I inferred.
Verdict: decorator-on-the-type is the right layer (V: ADM reaches devices only via the type). 4 MUST, 6 SHOULD, 3 NIT below.

## MUST
M1. Deny-list + fail-open lets unlisted transports through, against Boris's "EXACTLY hard-wired or onboard" (binding-decisions.md:485-488).
  V: SDK AudioHardwareBase.h:625 has a 3rd Continuity value `'ccap'` (ContinuityCapture, deprecated, macOS 13) that the plan lists in neither
  the deny nor allow set (plan 2.2, 4.1 transport ns); it falls through classify() -> allowed. The AutoAggregate 'fgrp' value is left to "the
  builder reads its value line" (plan 2.2) -- not in the policy table. Any future/unknown fourcc, and 'virt' (Loopback/BlackHole/Krisp/Zoom
  devices can front a Bluetooth route) and Unknown(0) are all allowed. Fix: ALLOWLIST {bltn, usb, thun, pci, 1394, hdmi, dprt, eavb, ccwd, +
  aggregate-by-members}; everything else (incl. unlisted, 'ccap', 0) hidden + logged. If Harmony wants 'virt', make it an explicit named choice.
  Also `static_assert`s in CoreAudioDeviceInfo.cpp (includes CoreAudio) that every mirrored fourcc == the SDK kAudioDeviceTransportType* constant;
  the fourcc() mirror in DevicePolicy.h is otherwise unverified (a typo'd 'blue' silently disables the whole guard; no gate would notice).
M2. Aggregate branch fails OPEN on the commonest real case. Plan 2.2 reads FullSubDeviceList then TranslateUIDToDevice; a Multi-Output /
  aggregate that lists earbuds which are currently NOT connected has an untranslatable UID -> member transport unknown -> "Unknown allowed" ->
  aggregate allowed and opened; when the earbuds reconnect the aggregate silently gains a 16 kHz member (drift-comp) mid-open. Fix: an
  unresolvable member UID = deny the aggregate (fail closed); additionally deny a member UID matching the BT UID shape (`XX-XX-XX-XX-XX-XX[:in|out]`).
  Add pure test P3b (aggregate {USB, unresolvable}) -> denied. (Plan risk 4 says mis-class only hides; true only for FALSE denies, not this.)
M3. Post-C3 the plan's own path-#4 analysis is stale and the safety net is untested. V: lastExplicitSettings is set only by
  setAudioDeviceSetup(...,true) (the re-open being deleted, ADM:736-746 updateXml when treatAsChosenDevice). After C3, ADM::audioDeviceListChanged
  (ADM:193-232) takes `createStateXml()==null` -> `initialiseDefault(preferredDeviceName, &currentSetup)` (ADM:223-226) with the VANISHED name still
  in currentSetup -> setAudioDeviceSetup returns "No such device: <name>" (ADM:764-772) after closeAudioDevice() -> no device. Plan 2.1 row 4
  ("fallback consults the decorator -> policy default") describes the XML branch that C3 removes. Outcome is still fail-closed, but recovery now
  rests solely on AudioEngine::onDeviceManagerChanged/reconcile, which has no test: R1-R5 are pure, M1-M4 never fire a list change with the open
  device removed, and probe arms never unplug. Fix: ctest M5 = real GuardedAudioDeviceManager + mock, open, remove the open device from the mock,
  fireListChanged, pump messages, run the exact onDeviceManagerChanged logic (factor it into a free function/static taking a manager so it is
  testable without AudioEngine) and assert a reopen on an allowed device and no reopen loop (count Reapply == 1). Rewrite row 4 accordingly.
M4. Reapply (closeAudioDevice + initialise on the message thread, plan 4.4) is issued exactly when the HAL is churning, while the plan admits
  the combiner restartAsync `close()` on the HAL queue runs without closeLock against message-thread open/start (CA:1637-1652 vs :1530-1580;
  the 09-23 SIGSEGV class, plan 2.5c). "Fewer opens" (2.4) is offset by adding a NEW close/open trigger on every list change that hides the open
  device. It also is not bounded when the reopened device itself fires a rescan (CA:1947 combiner rate reconcile -> audioDeviceListChanged ->
  seq++ -> a fresh Reapply allowed). Fix: coalesce Reapply through a >=250 ms message-thread timer re-evaluated after the last change (not inline
  in the ChangeListener), cap N Reapply per 10 s, and never Reapply while `currentAudioDevice` reports a pending restart/timer; add a gate that
  counts Reapply under a mock list-change storm (fire 20 changes, assert <=1 reopen).

## SHOULD
S1. Reapply changes device args, so AudioTap::prepare gets unequal args and a running perf take/recording is stopped (plan 2.4 only proved
  equal-args idempotence for the removed re-open). State it in MUST-NOT-CHANGE or defer Reapply while a take is armed/recording.
S2. Name vs UID: filter/createDevice key on JUCE's deduped NAME (CA:2238-2242 is name-keyed, unavoidable) but the dedupe suffix depends on id
  order: hidden BT "Scarlett" + wired "Scarlett" -> which one is "Scarlett" vs "Scarlett (2)" flips when the BT device (dis)connects, so the opened
  name can silently re-point to the other device on a rescan. Reconcile compares names only. Fix: DevicePolicy Lists carry the UID of the opened
  input/output; reconcile Reapply if the UID behind the opened name changed. Add P9b (rename flip). Test also two devices sharing one name.
S3. HAL property queries on the message thread: enumerate adds TransportType/UID/aggregate-list queries per device per scan (plan 4.2), issued while
  coreaudiod is mid-teardown of an HFP device -- HAL calls can block for seconds; the message thread also runs the UI. JUCE issues name/stream-config
  queries there already, so risk is incremental (I), but state a per-scan budget and log scans >100 ms; consider caching transport per AudioDeviceID.
S4. Gate cannot fail on a real Bluetooth classification: arms B/C use the env name deny (reason "test-denied"), ctest P* use synthetic DeviceInfo,
  E1 checks names only (plan 4.9). Nothing ties real HAL transport values to policy (see M1 static_assert) or exercises the enumerator path on a
  non-built-in device. Add E2: for every real device assert transport is in the known set and log it; add a CoreAudio-level ctest that classifies
  the rig's built-in pair as allowed AND asserts `transport::toString` round-trips the SDK constants.
S5. RED prediction is internally inconsistent: ctest text says "14 FAIL / 5 PASS" while listing 16 names and PASS {P2,P5,D3,M2,E1} (plan 5);
  E1 needs the real CoreAudioDeviceInfo, so it is not a "stub" pass. B5 (`rms == 0` fails on main "by noise floor") is not guaranteed to fail
  on a silent room (RED could read GREEN, or flake) -- assert `opened.input==""` instead, or drop B5 from the RED count.
S6. Default-index rule (3) "else first allowed" (plan 4.1) can pick a Virtual / HDMI-display device as the input when the Mac has no built-in mic
  (Mac mini/Studio) and defaultInput index 0 doubles as "unknown". Return -1-style "no default" -> prefer bltn -> then USB; document.

## NIT / attribution
N1. Attribution: plan quotes only the two Boris lines (good). But Risk 5 "hidden device Boris did not intend to use anyway" and 2.2 "Boris's rule"
  for BLE/AirPlay/Continuity attribute intent; AirPlay/wireless-Continuity denial and allowing 'virt'/Unknown are the PLAN's reading -- the second
  quote says "EXACTLY hard-wired input or onboard mic" (binding-decisions.md:487-488), which the allow-by-default policy contradicts (see M1).
N2. Label "Bluetooth is never used." / pitfall text cite "Boris's rule" -- fine, but the "2/2 vs 0/661" and crash lore are not Boris's words; keep separate.
N3. `guardedType()` Enumerate lambda + `productionConfig()` reading env under TEST_SERVER only: fine; note ADNA_AUDIO_DENY_DEVICES can hide the ONLY
  input on a real rig -- ensure it is compiled out of Release non-test builds (verify build/CMakeCache AUDIODNA_BUILD_TEST_SERVER=ON is the shipped app, plan 4.10 says yes: then it ships!).
## Checked and holds (V)
getDefaultDeviceIndex returns macOS default (CA:2192-2216); hardwareListenerProc ignores default changes (CA:1435-1447); combiner name = output name
(CA:2247); createDevice name-keyed with indexOf -1 -> ids[-1]==0 safe for ""; ADM insertDefaultDeviceNames on empty lists is safe (Array::move guarded);
empty names -> deleteCurrentDevice returns "" (ADM:742-751); audio callback untouched.
