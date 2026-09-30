# Reviewer Verdict — btguard coreaudio r1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS; no MUST)
REVIEWED: lane/btguard head 647ad69 (base d88d2ea), read via git show/diff; JUCE 8.0.4 sources read from build/_deps/juce-src.
FILES: src/audio/{DevicePolicy,CoreAudioDeviceInfo,DeviceGuard,AudioEngine,CombinedCallback}.*, src/MainComponent.*, src/api/ApiServer.*, tests/test_device_policy.cpp (+CMake), .harmony/probe-btguard.sh, CLAUDE.md, docs/claude/*, APP-INVENTORY

VERIFIED (executed / read at source)
- Every app open path goes through the one GuardedAudioDeviceManager: git grep shows AudioEngine.cpp is the only caller of initialiseWithDefaultDevices / setAudioDeviceSetup; MidiHandler / MidiLearnOverlay use the manager for MIDI only. JUCE's list-change fallback (ADM::audioDeviceListChanged -> initialiseFromXML / initialiseDefault) reads the type's names/default, i.e. the decorator.
- CoreAudioDeviceInfo reproduces CoreAudioIODeviceType::scanForDevices (device order, wildcard scope, name read, per-direction channel gating, appendNumbersToDuplicates); AudioBufferList size test equals JUCE's ManagedAudioBufferList arithmetic (8 + 16n). E1/E2 pass on the real HAL (I ran build-lane/tests/test_device_policy: 30 cases / 162 assertions green).
- BG1 static_asserts compile against the SDK; read error fails closed; Unknown/Virtual allowed. BG2 aggregates fail closed on unresolved / unreadable / BT-UID-shaped members. BG3: setSourceMode untouched. BG4: M5 (JUCE alone recovers, XML branch) and M5b (stereo input, JUCE ends with no device) exist; reconciler acts only with no device object, 250 ms settle, 1 per 10 s, storm test R1.
- Audio callback untouched: CombinedCallback.h diff is a TEST_SERVER-only atomic counter in audioDeviceAboutToStart. No new mutex in production (deviceStatusMutex_ is inside #if AUDIODNA_TEST_SERVER). getenv only inside #if. No .venv symlink / stray files; git status clean apart from untracked build-lane/. CLAUDE.md = 23,818 B.
- Mutation runs in $TMPDIR copies (rebuilt objects, real test binary): caught = BLE deny removed (P7), createDevice refusal removed (D2), unmapped fail-open (P10), no CoreAudio wrapping (M1/M3), macOS-default preference removed (P2), reconciler switching a live device (R*), min-interval removed (R4). SURVIVED = decorator getDefaultDeviceIndex passthrough to inner; reconciler seq guard removed.

FINDINGS
SHOULD-1 tests/test_device_policy.cpp D1/M1/M3: GuardedDeviceType::getDefaultDeviceIndex (DeviceGuard.cpp:34) replaced by inner_->getDefaultDeviceIndex() -> all 30 cases still pass (162/162). Fixtures put the denied device at inner index 0, so inner and filtered defaults coincide. Add a case where a denied device precedes an allowed macOS-default USB (inner index != filtered index) and assert decorator index + manager opens the USB. Not a Bluetooth-open risk (out-of-range JUCE move() is a no-op), but it is the "respect a wired default" half of the policy.
NIT-2 DeviceGuard.cpp:180 seq guard: removing it survives; R4's 10 s min-interval masks it inside the test window. Beyond 10 s a failed re-apply would self-retry every 10 s from its own change messages. Untested (needs an injectable clock).
NIT-3 name-keyed mapping: rebuild() re-scans only for UNMAPPED names. A duplicate-name swap between the inner scan and the enumeration (BT "X" appears ahead of USB "X") maps a name to the wrong device without being unmapped. Window is microseconds (both back to back on the message thread) and the builder filed it (S2/S9 declined). Cheap hardening: also require inner and enumerated per-name channel counts to agree.
NIT-4 cached default: the decorator answers the default from the last scan; JUCE's type reads it live and only listens to kAudioHardwarePropertyDevices, so a default-only change is not seen until the next list change. Only affects which ALLOWED device a later re-open picks. INFERRED.
NIT-5 CA:1906-1951 handleAudioDeviceAboutToStart calls owner->audioDeviceListChanged() under the combiner callbackLock (wrapper rate mismatch only); the guard adds one HAL enumeration there (measured 0.25 ms per the lane). Not on the rig's 48k/48k path. INFERRED.
NIT-6 E1 asserts transport != Unknown and membership in a known set for EVERY device: a rig with a reported-Unknown virtual device fails E1 although the policy allows it. Rig-dependent test, not a product defect.
NIT-7 GuardedAudioDeviceManager::typeFactoryForTests is a public test seam in a production class (empty std::function in production).

RISK / NOT VERIFIED
- Real wireless transport classification, the aggregate branch, and "Bluetooth is default -> built-in opens" are unit-only (rig has no BT / aggregate); lane discloses this. I did not re-run the live probe (needs the live lock and would launch the app).
- BG9 (OFF-build strings) taken from the lane report; source inspection agrees (getenv, testDeniedNames, "test-denied", publishDeviceStatus all under #if).
- Onboard-mic parity vs base is by code identity (open sequence unchanged, same default) and A5/A7 rows, not a base-vs-lane numeric diff.

SUMMARY: 0 blocking, 7 suggestions (1 SHOULD, 6 NIT). Confidence: VERIFIED for the mutation results and the code-path reading; INFERRED for NIT-3/4/5.
