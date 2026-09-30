# ATTACK: plan-btguard gates (test-engineer seat, blind). Verified = read source today; inferred = reasoning.
Read: plan-btguard.md (all), ADM.h:405-412, ADM.cpp:193-232 + 709-760, AudioEngine.cpp:1-130, CMakeLists.txt:457-459,
MainComponent.cpp:230, ApiServer.cpp:1975-1990, binding-decisions.md:485-498.

## MUST
M1. No gate watches the INNER type's createDevice, so "a Bluetooth device got opened" is not caught. D2 asserts only that the
  decorator returns nullptr; M1 asserts only the FINAL setup. ADM's temp devices (plan :556-559) or a re-init could open the
  headset briefly and still end on built-in, and every case stays green. Fix: MockDeviceType records EVERY createDevice/open/start
  name (inner spy); every manager case (M1, M3, M4, new M5) asserts the spy saw no denied name, temp devices included; M2 (the
  unguarded control) must assert the spy DID see it.
M2. The device-list-change regression has no gate. The rig has no second device, so no live row fires a list change; ctest D3 only
  proves the decorator forwards. onDeviceManagerChanged / reapplying_ / lastReappliedSeq_ (4.4) live in AudioEngine, which is not
  in the test_device_policy target (4.8), so reconcile-with-manager is untested. If JUCE's fallback (ADM.cpp:193-232 ->
  initialiseFromXML -> default) regressed to the headset, all 20 cases pass. Fix: add M5: real GuardedAudioDeviceManager + mock
  {denied default, builtinA, builtinB}; open; drop the OPEN output from the mock; fireListChanged; pump the MessageManager;
  assert via the spy that the denied name is never created. Extract onDeviceManagerChanged into a linkable helper so it is
  covered.
M3. Live rows are toothless against a broken enumerator because Unknown transport fails OPEN. A2/A3 assert "transport not in
  {blue,blea,airp,ccwl}"; if enumerateCoreAudioDevices returned 0 for every device (wrong scope/element, error swallowed as 0,
  4.2), A2-A7 pass and a real headset would be allowed. E1 checks names only. Fix: A2/A3 and E1 assert the rig's built-in
  devices report transport == "bltn" exactly; ctest/probe fail if any rig device has transport 0.
M4. The mirrored fourcc constants (4.1) are never checked against the SDK. P1-P7 use the same constants, so a wrong literal is
  self-consistent and green. Fix: in CoreAudioDeviceInfo.cpp under JUCE_MAC, static_assert each mirrored value equals
  kAudioDeviceTransportType* (ifdef the macOS-13 Continuity ones or drop them).
M5. The env hook drives a different branch than production. ADNA_AUDIO_DENY_DEVICES -> "test-denied", so arms B/C prove hiding,
  labels and the no-input state, never the transport branch, and never "default denied while another allowed device exists"
  (rig has one mic and one speaker; arm C denies both). The headline behaviour (default = headset -> pick built-in) is proven only
  by mock M1 with an injected enumerator, never through the real CoreAudio type's getDefaultDeviceIndex (CA:2192-2216). Fix: say so
  in the plan; add a live row that denies only the default output when a second allowed output exists, and make the probe emit
  SKIP (not PASS) when the rig lacks one; a SKIP never counts toward 23/0.
M6. "Absent from a production build" is asserted, not gated. The hook is under #if AUDIODNA_TEST_SERVER (CMakeLists.txt:457-459,
  option default OFF at :62; the plan says main's build/ is ON). But Config::testDeniedNames and the "test-denied" reason are
  ungated (4.1), and DeviceGuard.cpp is also compiled into the test target where the define is absent. Fix: a check on a
  TEST_SERVER=OFF binary (strings: no ADNA_AUDIO_DENY_DEVICES, no /api/debug/audio_devices) plus a grep that every getenv of the
  variable sits inside the #if.
M7. Scope and attribution. binding-decisions.md:485-498: any fix is "at most a guard that keeps the app off Bluetooth devices";
  "never ... design for Bluetooth"; "hard wired ... or the onboard mic". Plan 2.3/pitfall text (section 8) present AirPlay/wireless
  Continuity denial, adopt-new-mic Reapply case (iii), the getDeviceStatus relabel and a JUCE-bump analysis under "Boris's rule";
  2.2 does say "the plan's reading" for AirPlay/ccwl but the pitfall text does not. Fix: default open question 1 to Bluetooth+BLE
  only, mark AirPlay/ccwl/adopt-mic/relabel as Harmony decisions, drop reconcile (iii) from this lane, and quote Boris only with
  the two binding-decisions quotes.
M8. Fail-open on Unknown contradicts "never for any reason". A HAL property error (device mid-teardown, exactly the HFP case)
  yields 0 -> ALLOWED (4.2 "error -> 0"). Fix: separate error from genuine kAudioDeviceTransportTypeUnknown; error => hide and
  rescan; only a genuine Unknown may stay allowed (or fail closed, Harmony's call).

## SHOULD
S1. RED numbers are inconsistent. 4.9 says "19 cases + E1" (=20); section 5 lists 16 names then "14 FAIL / 5 PASS"; a Keep-returning
  pass-through stub also passes R2 and R4 vacuously (absent from the PASS list). RED-vs-stub is not RED-on-main (the target does not
  exist on main). Fix: record measured counts, call it "RED = stub", and add a mutation table (flip a constant, drop unmapped hiding,
  default index 0, Keep-always) naming the case that dies for each.
S2. B5 (rms == 0 x5) is a weak witness. On main it passes if TCC denies the mic or the analysis noise-gates a quiet room, so it may
  not be RED on main; the plan simply assumes a noise floor. Use B2's opened.input_channels == 0 plus a listened-input witness.
S3. reapplying_ is not a re-entrancy guard: ADM's sendChangeMessage is asynchronous, so the change arrives after the
  ScopedValueSetter resets. Only lastReappliedSeq_ protects, and a flapping list (new scan seq each time) re-runs
  close+initialise on the live mic each event while (i)/(iii) hold. Fix: drop the claim, add a rate limit (N per minute) and a
  ctest (unopenable device + N list changes -> exactly one Reapply).
S4. A6/A7 (opens == 1) count audioDeviceAboutToStart, which also fires for JUCE-internal restarts (paths #5/#8, CA:1947): an
  unrelated built-in rate change makes them flaky-RED. Fix: also count AudioEngine's own setAudioDeviceSetup/initialise calls and
  gate on that; keep opens diagnostic.
S5. No live way to fire a list change: add a TEST_SERVER-only POST that triggers the decorator rebuild on the message thread so
  reconcile is observable without hardware; otherwise 2.5 stays INFERRED (plan labels 2.5(b) INFERRED).
S6. Thread: rebuild() adds per-device HAL calls (transport, uid, aggregate lists) plus a second scanForDevices() on the message
  thread inside JUCE's list-changed callback, at the moment a wireless device tears down; a stalled HAL blocks UI and the
  render callAsync. The claim that audioDeviceAboutToStart runs on the message thread with callback quiesced is INFERRED (the
  combiner reaches audioDeviceListChanged from its start path, CA:1947). Fix: cache transport per AudioDeviceID, log calls > 50 ms,
  confirm the thread of the CA:1947 path. The audio callback itself is untouched: no Sacred Rule violation found.
S7. Re-entrancy: audioDeviceListChanged can fire inside setAudioDeviceSetup->open (CA:1947), so rebuild() and
  ADM::audioDeviceListChanged (which may closeAudioDevice) run mid-open. Add a ctest whose mock fires list-changed from inside start().
S8. Dropping the setSourceMode re-open drops updateXml(); A7's "opened byte-identical" passes on main too (same device), so the
  whole source-mode-switch gate rests on the opens counter (S4).

## NIT
N1. Line cites for AudioEngine (AE:81-87, :107-110) are off by about one (getDeviceStatus starts at line 80); builder should locate by text.
N2. Fixture name "BT Headset" in ctests vs the ruling "never test/gate for Bluetooth": rename "Denied Wireless".
N3. A4 (skipped == []) turns RED whenever Boris's earbuds are on, i.e. when the guard works. Make it: every skipped entry has a
  denied reason and none equals opened.*.
N4. Held up under attack (verified): ADM.h:412 virtual; audioDeviceListChanged compares against the combiner/output name
  (ADM.cpp:193-232); updateSetupChannels re-enables default channels (ADM.cpp:709-727); ui_text default "No file loaded" (MC:230) so B3/C2
  fail on main; live RED 13/10 arithmetic matches the listed rows.
