# PLAN — lane "btguard": Audio-DNA never opens a Bluetooth audio device

Architect (Fable), 2026-09-30 (written from s-rta-0929b), main HEAD d88d2ea. Read-only: no source edited, no app launched,
no system setting touched. One scratch rig ran ($TMPDIR/caenum: read-only CoreAudio property queries, no device opened).
Abbreviations: `CA` = `build/_deps/juce-src/modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp` (JUCE 8.0.4),
`ADM` = `build/_deps/juce-src/modules/juce_audio_devices/audio_io/juce_AudioDeviceManager.cpp`, `ADM.h` = its header,
`TYPE.h` = `.../audio_io/juce_AudioIODeviceType.h`, `AE` = `src/audio/AudioEngine.cpp`, `MC` = `src/MainComponent.cpp`,
`CC` = `src/audio/CombinedCallback.h`. Labels: VERIFIED (read/ran today) / INFERRED / ASSUMED.
Boris's words (binding-decisions.md:485-488, quoted only these): "we will never use bluetooth audio for any reason. it is
slow and bad. never use it again." and "we will only use hard wired sound input or the onboard mic".

## 0. ANSWER FIRST (the design in six lines)

1. Put the guard where JUCE looks up devices, not where the app opens them: `AudioDeviceManager` learns devices ONLY through
   an `AudioIODeviceType` (names, default index, `createDevice`; ADM:534-537, :764-774) and `createAudioDeviceTypes` is
   `virtual` (ADM.h:412). `GuardedAudioDeviceManager` wraps JUCE's CoreAudio type in a filtering decorator, so every open
   path -- first open, re-inits, the default-device lookup, any future picker -- sees a list with no Bluetooth device.
2. Classification = CoreAudio `kAudioDevicePropertyTransportType` (message thread only), deny {Bluetooth, BluetoothLE,
   AirPlay, wireless Continuity} and any aggregate containing one; everything else (built-in, USB, Thunderbolt, PCI,
   FireWire, HDMI, DisplayPort, AVB, wired Continuity, Virtual, Unknown) is allowed.
3. Default = the macOS default if allowed, else the built-in device, else the first allowed; no allowed input = output-only
   device + label "No wired mic found. Bluetooth is never used."; nothing allowed = no device + "No audio device found.
   Bluetooth is never used." -- the app launches and renders in both states.
4. One device open per launch: `setSourceMode` stops re-opening the device (its re-open changes nothing, VERIFIED below).
5. Hot-plug: JUCE ignores default-device changes while open; an earbud connect only re-reads details (no restart on
   built-in); the app reconciles on JUCE's change message (device gone -> re-apply policy; nothing else churns).
6. Gates: `tests/test_device_policy.cpp` (pure policy + decorator + real `juce::AudioDeviceManager` with mock types) and
   `.harmony/probe-btguard.sh` (TEST-ONLY env `ADNA_AUDIO_DENY_DEVICES` + `GET /api/debug/audio_devices`): predicted RED
   on main 13 FAIL / 10 PASS; no Bluetooth device, no system setting touched.

## 1. QUESTION

How does Audio-DNA (C++20 / JUCE 8.0.4 / CoreAudio, macOS 15 arm64) never open a Bluetooth audio device -- at launch, on a
source-mode switch, from any device menu, or when macOS changes its default -- and use the onboard mic / built-in output
(or a wired device) instead, with RED-first gates that need no Bluetooth device on the rig, inside the fence src/audio/*,
the device-selection call sites in src/MainComponent.cpp, ctests, .harmony probe rows and docs, without a JUCE bump?

## 2. WHAT THE PLAN DECIDES (the seven items, re-derived from source)

### 2.1 Every path that opens or re-opens an audio device today, and whether it can reach a Bluetooth device

| # | Path | Where | Reaches Bluetooth today? | Under the guard |
|---|---|---|---|---|
| 1 | AudioEngine construction | `AE:14 deviceManager_.initialiseWithDefaultDevices(2, 2)` -> `ADM:509-515` -> `initialise` (ADM:304-322) -> `initialiseDefault` (ADM:325-401) -> `insertDefaultDeviceNames` (ADM:518-606: `type->getDeviceNames(isInput)` + `deviceNames.move(type->getDefaultDeviceIndex(isInput), 0)` at :534-537; temp `type->createDevice` per candidate at :556-559; fills EMPTY names at :580-584) -> `setAudioDeviceSetup(setup, false)` (ADM:730-839; `deviceListContains` name check :764-772; `type->createDevice` :774; `open` :808; `start` :817). VERIFIED. | YES: `CoreAudioIODeviceType::getDefaultDeviceIndex` (CA:2192-2216) returns the index of `kAudioHardwarePropertyDefaultInputDevice` / `DefaultOutputDevice` -- the P31i when connected (recon-crashes.md 2.3). | The decorator's `getDeviceNames` excludes denied devices and `getDefaultDeviceIndex` answers the policy's choice; `deviceListContains` (ADM:294-301) and `createDevice` see only allowed names. Nothing at AE:14 changes. |
| 2 | Startup mode switch | `MC:303 setSourceMode(MicInput)` -> `AE:108-110 setAudioDeviceSetup(setup, true)`: `newSetup != currentSetup` (inputChannels {0,1} vs the stored active set {0}, ADM:735; `updateCurrentSetup` ADM:182-191) -> `stopDevice` :740 -> names unchanged so `needsNewDevice` false (:757-759) -> `updateSetupChannels` :795 (`useDefaultInputChannels` is true -> inputChannels := {0,1} again, ADM:709-727) -> `open` :808 -> `start` :817. A full stop/close/reopen/start on the SAME device object every launch (unified log: Started, Stopped, Started -- recon-startup.md section 2 item 12). VERIFIED. | Same device as #1 (names unchanged): Bluetooth only if #1 already was. | REMOVED (section 2.4): the mode becomes a pure atomic flip. |
| 3 | Any later mode switch (selectors `MC:304-339`, `MC:531-565`; `setAudioSourceModeSynced` `MC:5639-5652` from `POST /api/audio/source` `MC:2107`, perfRecord `MC:5672-5673`, perfPlay `MC:5773-5774`, stop-playback restore) | `AE:95-126` both branches call `setAudioDeviceSetup(setup, true)` | Same device as #1. File mode's `setup.inputChannels.clear()` (AE:121) is INEFFECTIVE: `updateSetupChannels` re-enables {0,1} because `useDefaultInputChannels` is true (ADM:709-727, :795) -- the mic stays open in both modes. VERIFIED. | REMOVED (section 2.4). |
| 4 | JUCE device-list changed re-init | `CoreAudioIODeviceType` listens to `kAudioHardwarePropertyDevices` (CA:2121-2126, :2301-2304 `triggerAsyncUpdate`) -> `audioDeviceListChanged` (CA:2266-2270: `scanForDevices` + `callDeviceChangeListeners`) -> `ADM::audioDeviceListChanged` (ADM:193-232): if the current device's NAME is in neither list -> `closeAudioDevice` + `initialiseFromXML(*e, true, ...)` (when `lastExplicitSettings` exists: set by every `setAudioDeviceSetup(..., true)`, i.e. today's setSourceMode) or `initialiseDefault(preferredDeviceName, &currentSetup)`; `initialiseFromXML` on failure calls `initialise(..., nullptr, false, preferredDefaultDeviceName)` (ADM:446-449) = `initialiseDefault` with EMPTY names -> `insertDefaultDeviceNames` -> the macOS DEFAULT. VERIFIED. Note the combiner's name is the OUTPUT device's name (CA:2247-2248), so only an output disappearance triggers this. | YES: after a wired OUTPUT vanished with the earbuds as default, the fallback picks the default. | The fallback consults the decorator -> the policy's default. Structurally closed. |
| 5 | JUCE restart after a device-detail change | per-device wildcard listener (CA:334) -> `deviceListenerProc` (CA:1150-1197) -> `deviceDetailsChanged` (CA:848-852, timer) -> `CoreAudioInternal::timerCallback` (CA:1118-1130: `updateDetailsFromDevice`, `owner.restart()` only if size/rate changed) -> `CoreAudioIODevice::restart` (CA:1372-1381) -> combiner `restartAsync` (CA:1637-1652: `close()` + timer) -> combiner `timerCallback` (CA:1728-1733) -> `restart(previousCallback)` (CA:1591-1635) = `open` + `start` on the SAME wrapper objects (by AudioDeviceID). VERIFIED. | Only the already-open device. | Unchanged; cannot switch devices. |
| 6 | HAL-thread restart | `deviceRequestedRestart` (CA:855-859) for `kAudioDevicePropertyDeviceHasChanged` / `kAudioObjectPropertyOwnedObjects` on the OPENED device -> `restartAsync` on the HAL queue (the 2026-09-23 SIGSEGV frame). VERIFIED (source); reachability under the guard: section 2.5. | Only the already-open device. | Unchanged; see 2.5. |
| 7 | System-object wildcard listener of each open CoreAudioIODevice | CA:1248 `hardwareListenerProc` (CA:1435-1447): acts ONLY on `kAudioHardwarePropertyDevices` -> `deviceDetailsChanged` (path #5). Default-device changes (`kAudioHardwarePropertyDefaultInputDevice/OutputDevice`) are IGNORED while a device is open. VERIFIED. | No. | Unchanged. |
| 8 | Combiner sample-rate reconciliation | `handleAudioDeviceAboutToStart` (CA:1906-1951): if the wrappers' rates differ it sets them and calls `owner->audioDeviceListChanged()` (CA:1947) -> path #4. VERIFIED. | Same as #4. | Same as #4. |
| 9 | `restartLastAudioDevice` (ADM:896-913), `setCurrentAudioDeviceType` (ADM:670-700: `insertDefaultDeviceNames`) | No app caller (grep VERIFIED: only `initialiseWithDefaultDevices`, `setAudioDeviceSetup`, `getAudioDeviceSetup`, `getCurrentAudioDevice`, `addAudioCallback` in `src/`). | -- | Guarded anyway (same type lookup). |
| 10 | Settings / device menu / picker | None: `src/ui/PreferencesDialog.cpp` handles MIDI output only (grep VERIFIED); `AppSettings` stores no audio device (grep VERIFIED); `TopBar` has an audio SOURCE selector (Mic/File), no device list. `MidiHandler::start` (src/midi/MidiHandler.cpp:13-27) and `MidiLearnOverlay` use the manager for MIDI only. | -- | A future `AudioDeviceSelectorComponent` would list through the decorator (hidden Bluetooth). |

### 2.2 Classifying a device as Bluetooth (reliably, on the message thread)

- Property: `kAudioDevicePropertyTransportType` (AudioHardwareBase.h:609-624; scope Global) on each `AudioDeviceID` from
  `kAudioHardwarePropertyDevices` -- the same list JUCE scans (CA:2151). VERIFIED on this rig with the scratch enumerator:
  `MacBook Pro Microphone` id 78 `bltn` uid `BuiltInMicrophoneDevice` in=1; `MacBook Pro Speakers` id 71 `bltn` uid
  `BuiltInSpeakerDevice` out=2; defaults 78 / 71; the names are byte-identical to JUCE's (`kAudioDevicePropertyDeviceNameCFString`,
  scope Wildcard, CA:2157) and `system_profiler SPAudioDataType` ("Transport: Built-in").
- Deny set: `'blue'` (Bluetooth), `'blea'` (Bluetooth LE) -- Boris's rule; plus `'airp'` (AirPlay) and `'ccwl'`
  (Continuity Capture wireless, macOS 13+) -- the plan's reading of "only hard wired sound input or the onboard mic"
  (both are wireless links with their own renegotiation). Allow: `'bltn'`, `'usb '`, `'thun'`, `'pci '`, `'1394'`,
  `'hdmi'`, `'dprt'`, `'eavb'`, `'ccwd'` (an iPhone by cable = hard-wired), `'virt'` (BlackHole / Loopback = the
  "system audio" input CLAUDE.md names; not a physical link), and `0` Unknown (fail OPEN, logged: denying an unknown would
  silently kill a fine wired interface; the harm model is specific to the wireless transports). Harmony may trim to
  Bluetooth-only (open question 1); the policy is one table.
- Aggregates (`'grup'`, and `kAudioDeviceTransportTypeAutoAggregate`, AudioHardwareBase.h:569 -- the builder reads its
  value line; treat identically) including Audio MIDI Setup's "Multi-Output Device": read
  `kAudioAggregateDevicePropertyFullSubDeviceList` (CFArray of sub-device UIDs) -> `kAudioHardwarePropertyTranslateUIDToDevice`
  -> each sub-device's transport; denied if ANY member is denied. The property calls compile and link (scratch rig);
  behaviour on a real aggregate is INFERRED (none on the rig; creating one is a system-setting change a gate may not
  make) -- the pure policy is ctested with `subTransports`.
- Name -> id mapping: JUCE resolves `createDevice` by NAME within its own lists (CA:2238-2242), so the enumerator
  reproduces JUCE's list construction exactly: same device order, names gated by channel count per direction
  (`kAudioDevicePropertyStreamConfiguration`, CA:2165-2175, :2284-2298), then `StringArray::appendNumbersToDuplicates(false, true)`
  per list (CA:2180-2181). The decorator keys transports by the deduped per-direction name; a JUCE name with no
  enumeration entry (a device that appeared between the two HAL queries) is HIDDEN (fail closed) after one re-scan,
  with one stderr line. On this rig the lists are equal (ctest E1 proves it on the real HAL).
- Where it runs: `scanForDevices` / the list-changed listener -- both message thread (`AsyncUpdater`, CA:2279-2282,
  :2301-2304; ADM's calls are message-thread). Never the audio callback (Sacred Rule 1 untouched: the callback code in
  `CC` / `AudioCallback.cpp` is not edited except a TEST-ONLY counter in `audioDeviceAboutToStart`, which runs on the
  message thread with the callback quiesced -- CC:172-193, ADM:1074-1101 per recon-startup.md 4.1).
- TEST-ONLY hook (question 6): `ADNA_AUDIO_DENY_DEVICES="<name>[;<name>...]"` (exact JUCE names), read once at the first
  scan in a `AUDIODNA_TEST_SERVER` build only (precedent: `ADNA_VIDEO_FORCE_FALLBACK`, `src/media/VideoPlayer.cpp:191`),
  classifies those names as `reason "test-denied"` (treated like Bluetooth). Absent from a production build.

### 2.3 The fallback rule, the no-device state, hidden vs shown-disabled

- Per direction, over the ALLOWED devices: (1) the macOS default if it is allowed (a wired interface Boris chose in
  System Settings is respected); (2) else the first device with transport `bltn`; (3) else the first allowed; (4) none ->
  that direction stays EMPTY. `insertDefaultDeviceNames` then behaves as today (fills only empty names from the filtered
  list, ADM:580-584; its sample-rate pairing :588-600 may pick another ALLOWED pair -- unchanged JUCE behaviour).
- No allowed input, an allowed output: JUCE opens output-only (`createDevice("MacBook Pro Speakers", "")`, CA:2233-2264;
  `updateSetupChannels` clears inputChannels for an empty name, ADM:709-727). `hasAudioDevice()` true; mic mode sees
  `numInputChannels == 0` -> `listened == nullptr` -> nothing analysed, AudioTap padded with silence (CC:143-159, already
  handled); file mode still works (the transport renders into the output scratch, CC:123-131). Label (startup, MC:485-498
  replacement): "No wired mic found. Bluetooth is never used." -- plain words, no modal (the repo's F3 rule).
- Nothing allowed: `setAudioDeviceSetup` with both names empty -> `deleteCurrentDevice()`, returns "" (ADM:742-751) ->
  no device, no error; `hasAudioDevice()` false; `sourceSampleRateCell` stays 0.0 -> `AnalysisResampler` bypass
  (src/analysis/AnalysisResampler.h:22, .cpp:28) and the analysis thread idles on an empty ring; rendering, REST, decks
  unaffected. Label: "No audio device found. Bluetooth is never used." (today's "No audio device found", MC:486,
  extended).
- A skipped device while a good one is open: NO label change (the show must not shout); stderr
  `[AudioEngine] skipped "soundcore P31i" (bluetooth; input, output)` once per scan change; the TEST-ONLY endpoint lists it.
- Hidden, not shown-disabled: JUCE's `AudioIODeviceType` interface (TYPE.h:89-124) has no "disabled" notion and
  `AudioDeviceSelectorComponent` has no disabled row; a hidden device is the only structural choice, and the app has no
  device list UI today (2.1 row 10). If a picker is ever added, it inherits the hidden list; the endpoint / stderr keep the
  audit trail of what was skipped and why.

### 2.4 Before the first open, and the redundant startup re-open

- The first open IS the guarded one: `initialiseWithDefaultDevices` (AE:14) -> `scanDevicesIfNeeded` (ADM:617-627) ->
  `createDeviceTypesIfNeeded` (ADM:137-153) -> the VIRTUAL `createAudioDeviceTypes` (ADM.h:412; default body ADM:247-262)
  -> our override installs the decorator -> `addAudioDeviceType` (ADM:264-275) registers JUCE's listener ON THE DECORATOR
  -> `type->scanForDevices()` -> `pickCurrentDeviceTypeWithDevices` -> `insertDefaultDeviceNames`. `deviceManager_` is
  an `AudioEngine` member (AudioEngine.h:70), fully constructed before the ctor body runs line 14, so the virtual dispatch
  is valid. No app code runs between JUCE's default lookup and the open, and none needs to: the lookup itself is guarded.
- Remove the device re-open from `setSourceMode` (AE:107-110, :119-122). Proof it changes nothing for wired / built-in
  devices (VERIFIED by reading, section 2.1 rows 2-3): open #1 requests inputChannels {0,1} (`initialise(2,2)` ->
  `updateSetupChannels` with `numInputChansNeeded = 2`), `chooseBestSampleRate(0)` = the device's current rate
  (ADM:842-869) and `chooseBestBufferSize(0)` = `getDefaultBufferSize()` = 512 (ADM:871-879, CA:1278-1289); the re-open
  passes `currentSetup.sampleRate/bufferSize` (the same values, `updateCurrentSetup` ADM:182-191) and {0,1} again; File
  mode's `clear()` is overridden back to {0,1}. Rate, buffer size, channels, the analysis path and R13 (`AudioCallback::
  audioDeviceAboutToStart` stores the rate cell at the ONE start, `src/audio/AudioCallback.cpp:53-57`) are identical.
  Side effects removed: one `audioDeviceStopped` + `audioDeviceAboutToStart` re-delivery (AudioTap::prepare with equal
  args -> no take stop), two extra IO threads created/destroyed, and `updateXml()` (`lastExplicitSettings`) -- which only
  fed path #4's XML branch, now guarded anyway. Fewer opens = fewer windows for the JUCE combiner races (research-heap.md
  1b forum 65502; recon-startup.md C2). The stderr lines `[AudioEngine] Switched to mic input mode` / `... file playback
  mode` (AE:112, :124) STAY (no probe greps them today -- grep VERIFIED -- but the 0929 recon used the mic line as a launch
  marker).
- Consequence worth stating: today's label "Mic: <getDeviceStatus()>" prints the COMBINER's name = the OUTPUT device
  (`AE:86 device->getName()`; CA:2247-2248 `combinedName = outputDeviceName`): "Mic: MacBook Pro Speakers @ 48000Hz".
  `getDeviceStatus()` will name the INPUT device (`getAudioDeviceSetup().inputDeviceName`) -- an intentional fix, listed
  under must-not-change as a text change.

### 2.5 Hot-plug while running (earbuds connect / disconnect mid-show)

- Nothing in the app follows the default device (2.1 rows 1-3 open by name once; no listener today). VERIFIED.
- JUCE: a connect changes `kAudioHardwarePropertyDevices` (and the defaults). (a) `CoreAudioIODeviceType` rescans and
  calls `ADM::audioDeviceListChanged` (path #4): the built-in output name is still listed -> no re-init; `sendChangeMessage`
  (ADM:232). (b) Each open `CoreAudioInternal` (input + output wrappers) gets `deviceDetailsChanged` via the system-object
  listener (CA:1435-1447) -> 100 ms timer on the MESSAGE thread -> `updateDetailsFromDevice` (CA:457-516: re-reads the
  built-in device's rate/frame size, re-allocates temp buffers under `callbackLock`) -> `restart()` ONLY if size or rate
  changed (CA:1126-1130) -- for built-in devices they do not. So a mid-show earbud connect costs two detail re-reads and
  no restart. VERIFIED (source); the "built-in details do not change on a Bluetooth connect" premise is INFERRED (not
  measurable on this rig without a Bluetooth device). (c) The HAL-thread `restartAsync` (path #6) needs
  `DeviceHasChanged` / `OwnedObjects` on the OPENED device object; the 09-23 SIGSEGV had the P31i itself open (its objects
  died on the HFP teardown, recon-crashes.md 1.2). With Bluetooth never opened, a Bluetooth connect/disconnect cannot
  fire it (per-device listener, CA:334); what remains is any macOS re-publication of a built-in / wired device's own
  objects (INFERRED rare; not Bluetooth-related). The race itself (CA:1637-1652 `close()` on the HAL queue vs message-thread
  `open`/`start` without `closeLock`, CA:1530-1580, :1664-1693) stays in every JUCE 8.x (research-heap.md 1b, forum
  67804; 8.0.15 L1744-1757) and is removed only by JUCE 9's combiner-free backend -> FILED as a loose end, not fixed here.
- What the guard does: `AudioEngine` registers as a `ChangeListener` on the manager and reconciles on every change
  message (message thread, asynchronous -- never inside JUCE's own callback): `devpolicy::reconcile` -> `Reapply` when
  (i) no device is open and something allowed exists, (ii) the opened input or output name is no longer in the filtered
  list (a wired mic unplugged: JUCE only stops the wrapper -- CA:1126-1128 `stopWithPendingCallback` when
  `isDeviceAlive` fails -- and never re-inits because the OUTPUT name still matches), or (iii) the app has NO input and
  an allowed input has appeared (Boris plugs in a mic); `Keep` otherwise -- a working device is never switched away
  from, even if macOS makes a new wired interface the default (stability during a show; open question 2). `Reapply` =
  `closeAudioDevice()` + `initialiseWithDefaultDevices(2, 2)` (guarded), at most once per decorator scan sequence
  (re-entrancy guard), bounded by events, no timer.

### 2.6 Gates -- section 5. 2.7 JUCE bump recommendation -- section 9.

## 3. TRADEOFFS CONSIDERED

- **A. Filtering decorator on the `AudioIODeviceType` inside a `GuardedAudioDeviceManager` (CHOSEN).** Guards every path
  in 2.1 by construction, including JUCE's own re-init fallback (#4) and any future picker; ~250 lines, all app-side, no
  JUCE edit; the pure policy is unit-testable and the decorator + real `juce::AudioDeviceManager` are testable with mock
  types (JUCE's own unit tests do exactly this, ADM:1549-1607, :1875). Cost: depends on the documented extension points
  (`createAudioDeviceTypes` virtual, `AudioIODeviceType` protected ctor + `callDeviceChangeListeners`, TYPE.h:188-191) and
  on reproducing JUCE's name construction (2.2).
- **B. Explicit device names chosen by the app before `initialise`** (`initialise(2, 2, nullptr, false, {}, &setup)`).
  Rejected: `insertDefaultDeviceNames` fills EMPTY names with the macOS default (ADM:580-584), so "no allowed input" cannot
  be expressed without also giving up `numInputChansNeeded` (and then `setAudioDeviceSetup` drops the input name forever,
  ADM:764-772 via `getSetupInfo`); path #4's fallback (`initialise(..., nullptr, ...)`, ADM:446-449) would still land on
  the default; an after-the-fact `ChangeListener` fix would open the Bluetooth device briefly -- the exact overflow window.
- **C. Deny by name pattern / by rate (16 kHz)** -- rejected: names are user-editable and localised; a 16 kHz check is
  a symptom, not the class (an A2DP-only headset is 44.1 kHz and still Bluetooth).
- **D. Keep the startup re-open** -- rejected: it changes nothing about the device (2.4) and doubles the races' window;
  keeping it would also make the "one open per launch" probe row meaningless.
- **E. JUCE patch / bump** -- outside this lane by constraint; section 9 gives the corrected recommendation.

## 4. DECISION / SPEC (exact changes; the builder executes as written)

### 4.1 `src/audio/DevicePolicy.h` / `.cpp` (NEW, pure: juce_core only, no CoreAudio, no device objects)

```cpp
namespace audiodna::devpolicy {
constexpr uint32_t fourcc(char a, char b, char c, char d) noexcept;   // (a<<24)|(b<<16)|(c<<8)|d -- no multichar literals (-Wpedantic, cmake/CompilerWarnings.cmake:13)
namespace transport {   // AudioHardwareBase.h:609-624 values, mirrored so the policy and its ctest need no CoreAudio header
  inline constexpr uint32_t Unknown = 0, BuiltIn = fourcc('b','l','t','n'), Aggregate = fourcc('g','r','u','p'),
    Virtual = fourcc('v','i','r','t'), PCI = fourcc('p','c','i',' '), USB = fourcc('u','s','b',' '), FireWire = fourcc('1','3','9','4'),
    Bluetooth = fourcc('b','l','u','e'), BluetoothLE = fourcc('b','l','e','a'), HDMI = fourcc('h','d','m','i'),
    DisplayPort = fourcc('d','p','r','t'), AirPlay = fourcc('a','i','r','p'), AVB = fourcc('e','a','v','b'),
    Thunderbolt = fourcc('t','h','u','n'), ContinuityWired = fourcc('c','c','w','d'), ContinuityWireless = fourcc('c','c','w','l');
  juce::String toString(uint32_t);   // "bltn" etc., "0" for Unknown (endpoint + stderr)
}
struct DeviceInfo {
    juce::String inputName, outputName;   // JUCE's per-direction names AFTER appendNumbersToDuplicates; empty = not in that list
    juce::String uid;
    uint32_t transport = 0;
    std::vector<uint32_t> subTransports;  // aggregate members; empty otherwise
    bool isDefaultInput = false, isDefaultOutput = false;
};
struct Config { juce::StringArray testDeniedNames; };   // TEST_SERVER only (ADNA_AUDIO_DENY_DEVICES); empty in production
struct Verdict { bool allowed = true; juce::String reason; };   // "", "bluetooth", "bluetooth-le", "airplay", "wireless-continuity", "aggregate:<member reason>", "test-denied"
Verdict classify(const DeviceInfo&, const Config&);
struct Lists {
    juce::StringArray inputs, outputs;        // the filtered lists JUCE sees: a subset of the inner lists, SAME order
    int defaultInput = 0, defaultOutput = 0;  // JUCE convention (CA:2215): 0 when unknown or empty
    struct Skipped { juce::String name, reason; bool input = false, output = false; };
    std::vector<Skipped> skipped;
    juce::StringArray unmapped;               // inner names with no DeviceInfo (hidden, fail closed)
};
Lists filter(const juce::StringArray& innerInputs, const juce::StringArray& innerOutputs,
             const std::vector<DeviceInfo>& scan, const Config&);
enum class Action { Keep, Reapply };
Action reconcile(bool haveDevice, const juce::String& openedInput, const juce::String& openedOutput, const Lists&);
}
```
Rules: `classify`: name in `testDeniedNames` (either direction) -> denied "test-denied"; transport Bluetooth / BluetoothLE /
AirPlay / ContinuityWireless -> denied; Aggregate (or any transport with non-empty `subTransports`) -> denied
"aggregate:<reason>" if any member classifies denied by transport; else allowed (Unknown allowed). `filter`: for each inner
name find the DeviceInfo whose `inputName` (or `outputName`) equals it; missing -> `unmapped` (hidden); denied -> `skipped`
(hidden); the rest keep inner order. Default index per direction: the allowed device flagged macOS default; else the first
allowed with transport BuiltIn; else 0. `reconcile`: `!haveDevice` -> `Reapply` iff any list non-empty; `openedInput`
non-empty and not in `inputs` -> `Reapply`; same for output; `haveDevice && openedInput.isEmpty() && !inputs.isEmpty()`
-> `Reapply` (and the output mirror); else `Keep`.

### 4.2 `src/audio/CoreAudioDeviceInfo.h` / `.cpp` (NEW; macOS enumeration; message thread only)

`std::vector<devpolicy::DeviceInfo> enumerateCoreAudioDevices();` -- `#if JUCE_MAC` real, else returns `{}`. Steps, each
the same address JUCE uses: ids = `kAudioHardwarePropertyDevices` (system object, scope Wildcard, element Main; CA:2151);
defaults = `kAudioHardwarePropertyDefaultInputDevice` / `DefaultOutputDevice` (CA:2200-2208); per id: name =
`kAudioDevicePropertyDeviceNameCFString` (scope Wildcard, CA:2157; skip the device if absent, as JUCE does); numIns /
numOuts = `kAudioDevicePropertyStreamConfiguration` per scope, summing `mNumberChannels` (CA:2284-2298); transport =
`kAudioDevicePropertyTransportType` (scope Global; error -> 0); uid = `kAudioDevicePropertyDeviceUID`; if transport is
Aggregate / AutoAggregate: `kAudioAggregateDevicePropertyFullSubDeviceList` -> for each UID `kAudioHardwarePropertyTranslateUIDToDevice`
(`AudioValueTranslation`) -> that id's transport into `subTransports`. Build `inputNames` (numIns > 0) and `outputNames`
(numOuts > 0) in id order, `appendNumbersToDuplicates(false, true)` on each (CA:2180-2181), write the deduped names back
into `inputName` / `outputName`. TEST_SERVER: `devpolicy::Config productionConfig()` in `DeviceGuard.cpp` reads
`ADNA_AUDIO_DENY_DEVICES` once (`;`-separated, trimmed) under `#if AUDIODNA_TEST_SERVER`. The scratch enumerator that
validated these calls is `$TMPDIR/caenum/caenum.cpp` (not part of the repo).

### 4.3 `src/audio/DeviceGuard.h` / `.cpp` (NEW: the decorator + the manager subclass)

```cpp
class GuardedDeviceType final : public juce::AudioIODeviceType, private juce::AudioIODeviceType::Listener {
public:
    using Enumerate = std::function<std::vector<devpolicy::DeviceInfo>()>;
    GuardedDeviceType(std::unique_ptr<juce::AudioIODeviceType> inner, Enumerate, devpolicy::Config);   // AudioIODeviceType(inner->getTypeName()); inner->addListener(this)
    ~GuardedDeviceType() override;                                    // inner_->removeListener(this) BEFORE inner_ is destroyed
    void scanForDevices() override;                                   // inner_->scanForDevices(); rebuild();
    juce::StringArray getDeviceNames(bool wantInputNames) const override;   // lists_.inputs / outputs
    int getDefaultDeviceIndex(bool forInput) const override;          // lists_.defaultInput / defaultOutput
    int getIndexOfDevice(juce::AudioIODevice* d, bool asInput) const override;   // i = inner_->getIndexOfDevice(d, asInput); i < 0 ? -1 : lists(asInput).indexOf(inner_->getDeviceNames(asInput)[i])
    bool hasSeparateInputsAndOutputs() const override;                // inner_
    juce::AudioIODevice* createDevice(const juce::String& out, const juce::String& in) override;   // "" always ok; a name not in lists_ -> nullptr + stderr "[AudioEngine] refused device \"<name>\""; else inner_->createDevice(out, in)
    struct Scan { devpolicy::Lists lists; std::vector<devpolicy::DeviceInfo> devices; uint64_t seq = 0; };
    Scan lastScan() const;                                            // message thread
private:
    void audioDeviceListChanged() override;                           // Listener from inner_: rebuild(); callDeviceChangeListeners();
    void rebuild();   // devices = enumerate(); lists = filter(inner names, devices, cfg); if (!lists.unmapped.isEmpty()) { inner_->scanForDevices(); re-enumerate; re-filter; }  ++seq; log skipped/unmapped when they changed since the previous scan
};
class GuardedAudioDeviceManager final : public juce::AudioDeviceManager {
public:
    explicit GuardedAudioDeviceManager(devpolicy::Config, GuardedDeviceType::Enumerate = &enumerateCoreAudioDevices);
    void createAudioDeviceTypes(juce::OwnedArray<juce::AudioIODeviceType>& types) override;   // (typeFactoryForTests ? it : base)(types); then on JUCE_MAC replace each type whose getTypeName() == "CoreAudio" by a GuardedDeviceType at the same index
    GuardedDeviceType* guardedType() const;                           // the wrapped type or nullptr
    std::function<void(juce::OwnedArray<juce::AudioIODeviceType>&)> typeFactoryForTests;   // set BEFORE initialise(); mocks name themselves "CoreAudio"
};
```
Ordering guarantee: JUCE's `CallbackHandler` is registered on the DECORATOR (ADM:264-275), and the decorator is the only
listener of the inner type, so `rebuild()` always precedes JUCE's `audioDeviceListChanged` -> JUCE never sees a stale
filtered list. Non-mac: no wrapping (the policy would hide everything without an enumerator).

### 4.4 `src/audio/AudioEngine.h` / `.cpp`

- `AudioEngine.h:70` `juce::AudioDeviceManager deviceManager_;` -> `GuardedAudioDeviceManager deviceManager_{ devguard::productionConfig() };`
  (`#include "DeviceGuard.h"`); `getDeviceManager()` (AudioEngine.h:26) keeps returning `juce::AudioDeviceManager&`.
- New public: `bool hasInputDevice() const` (= `getAudioDeviceSetup().inputDeviceName.isNotEmpty()` and a device is open);
  `struct DeviceStatus { juce::String inputName, outputName, state /*"ok"|"no-input"|"no-device"*/; double sampleRate = 0; int bufferSize = 0, inputChannels = 0, outputChannels = 0; bool haveDevice = false; GuardedDeviceType::Scan scan; int opens = 0; }`;
  `DeviceStatus deviceStatus() const` (copy under `std::mutex statusMutex_` -- taken on the message thread to publish and
  on the HTTP thread to read; never on the audio thread: the `RecorderHost::status()` pattern the critic blessed,
  `src/api/ApiServer.h:105-120`); `#if AUDIODNA_TEST_SERVER juce::var deviceStatusVar() const; #endif`.
- Ctor (AE:3-22): `deviceManager_.addChangeListener(this);` before line 14; after line 19: `publishDeviceStatus(); logDevicePolicy();`
  (stderr: `[AudioEngine] audio devices: input "<in>" (<transport>), output "<out>" (<transport>); skipped: <none | "name" (reason; input, output), ...>`).
  Dtor (AE:24-30): `deviceManager_.removeChangeListener(this);` first.
- `setSourceMode` (AE:95-126): delete lines 107-110 and 119-122 (the two `setAudioDeviceSetup` blocks and the `setup`
  locals); keep `stop()`, the two `useInputForAnalysis` stores and both stderr lines. Add the comment: "one device open per
  launch (plan-btguard 2.4): the mode is an atomic read in CombinedCallback; a re-open changed nothing (JUCE re-enables the
  default input channels, ADM updateSetupChannels)".
- `getDeviceStatus()` (AE:81-87): no device -> `"no audio device (Bluetooth is never used)"`; input name empty ->
  `"no wired mic (Bluetooth is never used)"`; else `inputDeviceName + " @ " + rate + "Hz"`.
- `changeListenerCallback(source)` (AE:128-132): `if (source == &deviceManager_) { onDeviceManagerChanged(); return; }` then
  the transport branch. `onDeviceManagerChanged()` (message thread): `const auto scan = deviceManager_.guardedType() ? ->lastScan() : Scan{}`;
  `const auto setup = deviceManager_.getAudioDeviceSetup(); const bool have = deviceManager_.getCurrentAudioDevice() != nullptr;`
  `if (!reapplying_ && scan.seq != lastReappliedSeq_ && devpolicy::reconcile(have, setup.inputDeviceName, setup.outputDeviceName, scan.lists) == Reapply)`
  `{ juce::ScopedValueSetter<bool> s(reapplying_, true); lastReappliedSeq_ = scan.seq; deviceManager_.closeAudioDevice(); const auto err = deviceManager_.initialiseWithDefaultDevices(2, 2); if (err.isNotEmpty() && onError) onError("Audio device: " + err); }`
  then `publishDeviceStatus()` (also refreshes the label through `onDeviceStateChanged` if set -- see 4.6).
- `publishDeviceStatus()`: fills `DeviceStatus` from `getAudioDeviceSetup()`, `getCurrentAudioDevice()` (rate, buffer,
  `getActiveInputChannels/OutputChannels().countNumberOfSetBits()`), the scan, `opens` (4.5), `state`.
- `std::function<void()> onDeviceStateChanged;` (message thread; MainComponent sets it to refresh the label in the
  no-input / no-device states).

### 4.5 `src/audio/CombinedCallback.h` (TEST-ONLY witness only)

Under the existing `#if AUDIODNA_TEST_SERVER` (CC:41-54, :207-215): `std::atomic<int> opens_{0}` (`static_assert` lock-free
like its neighbours) incremented in `audioDeviceAboutToStart` (CC:172-193; message thread, callback quiesced) and
`int opens() const`. `AudioEngine::deviceOpens()` exposes it (AudioEngine.h:61-67 block). Nothing in
`audioDeviceIOCallbackWithContext` changes (Sacred Rule 1).

### 4.6 `src/MainComponent.cpp`

- Replace MC:485-498 with: `if (!audioEngine_.hasAudioDevice()) setFileLabel("No audio device found. Bluetooth is never used.");`
  `else if (!audioEngine_.hasInputDevice()) setFileLabel("No wired mic found. Bluetooth is never used.");` `else { <the R13 log block unchanged> }`.
  Add `audioEngine_.onDeviceStateChanged = [this] { <the same three-way rule, but only overwrite the label when the state is no-input / no-device or when it just recovered (state "ok" and the label currently shows one of the two sentences)> };`
  next to `audioEngine_.onError` (MC:479-483).
- MC:303 stays (`setSourceMode(MicInput)` is now a flag flip). The label sites MC:309, :536, :5649 stay (new text from 4.4).
- MC:2133-2134 block: `#if AUDIODNA_TEST_SERVER apiServer_->setAudioDevicesProvider([this] { return audioEngine_.deviceStatusVar(); }); #endif`
  (before `start()`, like the other providers).
- MC:4075-4079 (TEST_SERVER xruns sample) unchanged.

### 4.7 `src/api/ApiServer.h` / `.cpp` (TEST-ONLY route, production port, no `--test-mode`)

`server_.Get("/api/debug/audio_devices", ...)` inside the existing `#if AUDIODNA_TEST_SERVER` block (ApiServer.cpp:298-312);
`setAudioDevicesProvider(std::function<juce::var()>)` beside `setLoadWitnessProvider` (ApiServer.h:171-175, provider
member :288-290); handler: 503 `jsonError("audio devices not wired")` if unset, else `JSON::toString(provider())`.
Reads ONLY the mutex-guarded copy (never the manager). JSON:
`{"ok":true,"scan_seq":N,"state":"ok|no-input|no-device","opened":{"input":"…","output":"…","sample_rate":48000,"buffer_size":512,"input_channels":1,"output_channels":2},"opens":1,"lists":{"inputs":[…],"outputs":[…],"default_input":0,"default_output":0},"devices":[{"name_in":"…","name_out":"…","uid":"…","transport":"bltn","sub_transports":[],"default_input":true,"default_output":false,"allowed":true,"reason":""}],"skipped":[{"name":"…","reason":"…","input":true,"output":true}],"unmapped":[],"test_denied":[…]}`.

### 4.8 CMake

- `CMakeLists.txt:99-105` (Audio block): add `src/audio/DevicePolicy.h/.cpp`, `src/audio/CoreAudioDeviceInfo.h/.cpp`,
  `src/audio/DeviceGuard.h/.cpp`. CoreAudio / CoreFoundation come transitively from `juce::juce_audio_devices` (the app
  and `test_audio_tap_sync` already compile JUCE's CoreAudio code that way, tests/CMakeLists.txt:457-465).
- `tests/CMakeLists.txt`: new target after `test_audio_tap_sync` (:437-481 is the template): `add_executable(test_device_policy
  test_device_policy.cpp ${SRC_DIR}/audio/DevicePolicy.cpp ${SRC_DIR}/audio/CoreAudioDeviceInfo.cpp ${SRC_DIR}/audio/DeviceGuard.cpp)`,
  same include dir, `Catch2::Catch2WithMain juce::juce_core juce::juce_events juce::juce_audio_basics juce::juce_audio_devices`,
  the same compile definitions / `-Wno-*` options / `apply_sanitizers` / `catch_discover_tests`. Target count 107 -> 108.

### 4.9 `tests/test_device_policy.cpp` (NEW; 19 cases + E1)

Fixtures: `DeviceInfo` builders; a `MockDeviceType : juce::AudioIODeviceType` named "CoreAudio" (names given at construction,
`fireListChanged()` calls `callDeviceChangeListeners()`, `createDevice` returns a `MockDevice` whose `open` succeeds and
`start` calls `audioDeviceAboutToStart` -- the `FakeAudioIODevice` shape of `tests/test_bt_device_shapes.cpp:60-98`);
`juce::ScopedJuceInitialiser_GUI gui;` in the manager cases (ADM's `sendChangeMessage` needs a MessageManager; precedent
`tests/test_clip_inspector_paint_key.cpp:32`).
P1 Bluetooth default in+out with built-in present -> inputs/outputs lack the headset, defaults point at the built-in,
`skipped` has 2 entries reason "bluetooth". P2 wired USB default -> USB stays default. P3 aggregate {USB, built-in}
allowed; aggregate {USB, Bluetooth} denied "aggregate:bluetooth". P4 only Bluetooth devices -> empty lists, defaults 0.
P5 no devices -> empty, 0. P6 AirPlay + ContinuityWireless denied, ContinuityWired / Virtual / Unknown allowed. P7 BLE denied.
P8 `testDeniedNames` hides a built-in by exact name, reason "test-denied". P9 duplicates: "Scarlett" (USB) kept,
"Scarlett (2)" (Bluetooth) hidden -- mapping by the deduped name. P10 an inner name with no DeviceInfo -> in `unmapped`,
hidden. R1 reconcile: opened input gone -> Reapply. R2 all present -> Keep. R3 no device, an allowed device exists ->
Reapply. R4 no device, nothing allowed -> Keep. R5 device open with no input, an input appears -> Reapply.
D1 decorator over a mock listing {headset, built-in}: `getDeviceNames` filtered, `getDefaultDeviceIndex` = built-in.
D2 `createDevice("", "BT Headset")` -> nullptr; `createDevice("", "Built-in Mic")` -> non-null. D3 `inner.fireListChanged()`
reaches a listener registered on the decorator AFTER `lastScan().seq` advanced.
M1 `GuardedAudioDeviceManager` (typeFactoryForTests -> the mock, enumerator -> a lambda) `initialiseWithDefaultDevices(2, 2)`
returns "" and `getAudioDeviceSetup().inputDeviceName == "Built-in Mic"`, `outputDeviceName == "Built-in Speakers"` with
the headset as the mock's default. M2 CONTROL: plain `juce::AudioDeviceManager` with the same mock opens "BT Headset"
(documents the JUCE behaviour the guard exists for; GREEN before and after). M3 no allowed input -> device open,
`inputDeviceName.isEmpty()`. M4 nothing allowed -> `getCurrentAudioDevice() == nullptr`, error "".
E1 (`#if JUCE_MAC`): `enumerateCoreAudioDevices()` vs a real `juce::AudioIODeviceType::createAudioIODeviceType_CoreAudio()`
after `scanForDevices()`: every JUCE input/output name has exactly one DeviceInfo with that `inputName`/`outputName`
(read-only HAL property queries; no device is opened; proves the name algorithm on the real HAL).

### 4.10 `.harmony/probe-btguard.sh` (NEW; scaffolding = probe-resync.sh / probe-async-load.sh verbatim)

Live-lock gate (probe-step3.sh:113-124), `adna_pids`/`adna_running` (ucomm), REFUSE if running, `open -g --stdout --stderr`
with `--env` (probe-async-load.sh:52-61), health wait <= 60 s, `ok`/`no` counters, graceful `osascript` quit with a 30 s
wait and a `.ips` diff (probe-async-load.sh:68-70), never the Output window, no synthetic input. Env: `BTGUARD_APP`
(default `<root>/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app`; the RED run passes main's `build/` app -- both are
`AUDIODNA_BUILD_TEST_SERVER=ON` Release: `build/CMakeCache.txt:31,255` VERIFIED), `BTGUARD_OUT`. TCC: the first launch of
a rebuilt binary may raise the mic prompt -- `screencapture -x` and LOOK (probe-step3.sh header rule). Three arms, one
launch each, production mode (no `--test-mode`):
Arm A (no env): A0 launch + `/api/health`; A1 `GET /api/debug/audio_devices` 200 + `ok:true`; A2 `opened.input` non-empty
and its `devices[]` row `allowed:true` with transport not in {blue, blea, airp, ccwl}; A3 same for `opened.output`;
A4 `skipped == []` and `unmapped == []`; A5 `/api/features sourceSampleRate == opened.sample_rate`; A6 `opens == 1`;
A7 `POST /api/audio/source {"mode":"file"}`, 1 s, `{"mode":"input"}`, 1 s -> `opened` byte-identical and `opens == 1`;
A8 stderr contains `[AudioEngine] Switched to mic input mode` exactly 2x (launch + A7) -- the marker is preserved;
A9 graceful quit <= 30 s, no new `~/Library/Logs/DiagnosticReports/Audio-DNA*.ips`.
Arm B (`--env 'ADNA_AUDIO_DENY_DEVICES=MacBook Pro Microphone'`): B0 launch + health; B1 the mic row `allowed:false`,
`reason:"test-denied"`, `test_denied` echoes the name; B2 `opened.input == ""`, `opened.output` non-empty, `state == "no-input"`;
B3 `GET /api/debug/ui_text` (ApiServer.cpp:318) == "No wired mic found. Bluetooth is never used."; B4 `/api/bpm` readable
+ alive 10 s; B5 `/api/features rms == 0` on 5 reads 1 s apart (no input is listened to); B6 quit clean.
Arm C (`--env 'ADNA_AUDIO_DENY_DEVICES=MacBook Pro Microphone;MacBook Pro Speakers'`): C0 launch + health; C1 `state == "no-device"`,
both `opened` names empty; C2 `ui_text` == "No audio device found. Bluetooth is never used."; C3 `POST /api/render_frame`
(ApiServer.cpp:268) answers ok (the app renders with no device); C4 `POST /api/audio/source` file then input -> 200, alive;
C5 quit clean. 23 rows.

### 4.11 Docs (text in section 8)

## 5. GATES (RED first; numbers predicted)

- **ctest (RED)**: on main the target cannot exist (no policy). RED demonstration in the worktree, uncommitted: build
  `test_device_policy` against a pass-through stub (`filter` returns the inner lists, defaults 0; `reconcile` returns
  Keep; decorator/manager wired to it) -> predicted 14 FAIL (P1, P3, P4, P6, P7, P8, P9, P10, R1, R3, R5, D1, D2, M1, M3, M4
  minus the ones that pass vacuously on a stub: P4/P5 shapes -- record the actual) / 5 PASS (P2, P5, D3, M2, E1). Then the
  real implementation: 20/20. ctest total 967 -> 987 (107 -> 108 targets); serial `ctest` (the `-j` flake, notebook).
- **Live (RED)**: `.harmony/probe-btguard.sh` with `BTGUARD_APP=<main build d88d2ea>`: predicted **13 FAIL / 10 PASS**
  (FAIL: A1-A7, B1, B2, B3, B5, C1, C2 -- A1-A7/B1/B2/C1 by the missing endpoint (404), B3/C2 by the label reading
  "No file loaded" (MC:230), B5 by the open mic's noise floor; PASS: A0, A8, A9, B0, B4, B6, C0, C3, C4, C5). After commit
  C2 (endpoint + witnesses, re-open still present): 21/2 (A6 reads 2, A7 reads 3). After C3: 23/0, run >= 5 times (flake
  rule); every run 23/0.
- **Stay GREEN** (built-in devices, `build-lane`): probe-step3 94/0 (HANDOFF.md:3155; T2 p95 ~9 ms), probe-resync 16/0,
  probe-onset-render 13/0, probe-downbeat-level 14/0, probe-manual-bpm 18/0 (HANDOFF.md:3154-3155 baselines). Optional
  (audio witnesses): probe-async-load's a2 row (`audio_callback_gap_max_ms` bounds only tighten with one open).
- Sacred-rule check: `git diff` of `CombinedCallback.h` / `AudioCallback.cpp` touches only the `#if AUDIODNA_TEST_SERVER`
  `audioDeviceAboutToStart` counter; no new code in `audioDeviceIOCallbackWithContext`.

## 6. MUST NOT CHANGE

- The audio callback (`CC:56-170`, `AudioCallback.cpp:8-51`): byte-identical. Analysis path, R13 rate cell, ring buffer:
  untouched. No new inter-thread channel on a hot path (the status mutex is message <-> HTTP only).
- On built-in / wired devices: opened names, sample rate, buffer size, active channels identical to today (rows A2-A5, A7
  witness them); `/api/audio/source` semantics; `inputSource` mirror (MC:4031); perf recorder arm/play/stop paths.
- No JUCE source or tag change (`CMakeLists.txt:30` stays 8.0.4); no settings.json key; no Output window; no default
  device or system setting written by any code or gate; no Bluetooth device connected for any gate.
- stderr lines `[AudioEngine] Switched to mic input mode` / `file playback mode` kept.
- Intentional text changes only: `getDeviceStatus()` names the INPUT device (was the output/combiner name); the two
  plain-language labels of 2.3; today's "No audio device found" gains the second sentence.

## 7. RISKS (strongest counterargument first)

1. **"A decorator on JUCE's device type is a dependency on JUCE internals; a bump could route around it."** The extension
   point is public and documented (ADM.h:404-412 "You can override this if your app needs to do something specific, like
   avoid using DirectSound devices"; TYPE.h:38-40 "To create a new device type, implement this class"); the manager reaches
   devices only through the type (2.1). JUCE 9's rewrite is inside the CoreAudio TYPE (research-heap.md 1: aggregate
   devices instead of the combiner) and keeps the ADM API (INFERRED; not on disk). Teeth: M1 drives the REAL
   `juce::AudioDeviceManager`, so a bump that changes its consultation path fails the ctest loudly, and E1 fails if a bump
   changes the name construction. Why it beats B: B leaves path #4's fallback unguarded and opens Bluetooth briefly.
2. **Name-keyed mapping race** (a device appears between the inner scan and ours): mitigated by one re-scan, then fail
   closed; the built-in devices never come and go, and any hidden device returns on the next list change. E1 proves the
   algorithm matches on the real HAL.
3. **Removing the re-open changes startup timing** (one `audioDeviceAboutToStart`, no second IO-thread pair): AudioTap's
   `prepare` is idempotent for equal args (AudioTap.cpp per test_bt_device_shapes.cpp:302-310 comments); the async-load
   gap witness only shrinks. Verified by the five GREEN probes.
4. **Aggregate branch unexercised on the rig** (INFERRED behaviour; no aggregate exists and a gate may not create one):
   bounded by the pure policy tests; a real aggregate of wired devices that mis-classifies would only be HIDDEN (fail
   closed, visible in `skipped`), never opened -- the safe direction.
5. **Deny-list breadth** (AirPlay, wireless Continuity beyond Boris's literal Bluetooth): trimming is a one-table change
   (open question 1); the harm of over-denying is a hidden device Boris did not intend to use anyway.
6. **`initialise` twice on Reapply** (`initialiseWithDefaultDevices` re-runs `initialise`, resets `lastExplicitSettings`,
   ADM:509-515): JUCE supports re-initialise (its own tests call it after setups); bounded by the `lastReappliedSeq_`
   guard; a failed re-open leaves "no device" (label + onError), never a loop.
7. **A Mac with no `bltn` output and only Bluetooth speakers** would run with no output device: harmless (the app outputs
   silence, CC:166-169) except file mode needs output channels as scratch (CC:123-131) -> file mode analyses nothing there.
   Not this rig; documented.
8. Residuals NOT fixed here (filed): the HAL-thread `restartAsync` race (2.5 c); temp devices in `insertDefaultDeviceNames`
   still register/unregister HAL listeners (ADM:556-559, CA:1248/1260 -- the listener-lifetime class, recon-startup.md
   4.3 item 6); the JUCE overflow itself for any wired interface whose read-back frame size differs from the request
   (section 9).

## 8. DOCS TEXT (verbatim for the builder)

- `docs/claude/pitfalls.md` (append; Harmony assigns NN):
  "NN. **The app never opens a Bluetooth audio device -- JUCE follows the macOS default device, so the guard lives in the device TYPE, not at the open call**: `AudioDeviceManager` learns devices only through an `AudioIODeviceType` (names, default index, `createDevice`) and re-reads the DEFAULT device on every path -- `initialiseWithDefaultDevices` (`insertDefaultDeviceNames`), the device-list-changed re-init and its `initialiseFromXML` fallback, `setCurrentAudioDeviceType` -- so a name check at one call site leaves the others open. `GuardedAudioDeviceManager` (`src/audio/DeviceGuard.h`) wraps JUCE's CoreAudio type in `GuardedDeviceType`: it hides every device whose CoreAudio transport is Bluetooth / Bluetooth LE / AirPlay / wireless Continuity (or an aggregate containing one), answers the default index with the macOS default when allowed, else the built-in device, and refuses a hidden name in `createDevice` (`src/audio/DevicePolicy.h` is the pure table; `CoreAudioDeviceInfo.cpp` reproduces JUCE's name list -- same order, channel gating and `appendNumbersToDuplicates`). Boris's rule (binding-decisions.md 2026-09-24): never Bluetooth audio; wired or the onboard mic only. Why: JUCE 8.0.4 `CoreAudioInternal::reopen` (juce_CoreAudio_mac.cpp:673-675) sizes its temp buffers from the device's read-back frame size and the callback copies the REQUESTED count (:793-797); a Bluetooth HFP headset (16 kHz, 320 frames) overflows the heap on its first callback -- every launch with the earbuds as macOS default crashed (2026-09-23/24/29; ASan `.harmony/.reports/s-rta-0924b/asan-bt-startup-crash.log`; 2/2 vs 0/661 in the 09-29 unified log). Upstream fix = JUCE >= 8.0.9 (f6df3e3), NOT 8.0.8 (the file is byte-identical 8.0.4..8.0.8). States: no allowed input -> output-only device and the label "No wired mic found. Bluetooth is never used."; nothing allowed -> no device and "No audio device found. Bluetooth is never used." -- the app launches and renders either way; a skipped device is a stderr line, never a modal. One device open per launch: `setSourceMode` is an atomic flag flip (its old re-open changed nothing -- JUCE re-enables the default input channels -- and doubled the combiner-race window). While running, `AudioEngine` reconciles on the manager's change message: a vanished device or a first allowed input re-applies the policy; a working device is never switched away from. Never open the device by name from app code, never add a picker that bypasses the manager, and never read the manager on the HTTP thread (`deviceStatus()` is the mutex-guarded copy). Guards: `tests/test_device_policy.cpp` (M2 documents the unguarded JUCE behaviour; E1 checks the name algorithm on the real HAL); live `.harmony/probe-btguard.sh` (TEST-ONLY `ADNA_AUDIO_DENY_DEVICES`, `GET /api/debug/audio_devices`)."
- `CLAUDE.md` Common Pitfalls Index (after line 240 "60."): "NN. The app never opens a Bluetooth audio device (the guard is in the device TYPE; one device open per launch) -- before touching AudioEngine's device open, GuardedAudioDeviceManager, setSourceMode, or adding any audio device picker." Pay for it (CLAUDE.md is 24,522 B of 25,000): MOVE the "### Latency Budget" table (CLAUDE.md:36-56, 1,085 B) to `docs/claude/architecture.md` as a new "### Latency Budget" section right after "Lock-Free Communication Chain" (before line 113 "## Technology Stack"), leaving in CLAUDE.md: "### Latency Budget\n\nStage-by-stage table (audio buffer delivery -> swap, ~15-25 ms audio-to-visual): `docs/claude/architecture.md` \"Latency Budget\".\n\n---" ; add "the latency budget" to the Trigger Table row for architecture.md. Net ~ -750 B (builder records `wc -c CLAUDE.md`, must stay < 25,000).
- `docs/claude/architecture.md`: Source Tree (line 152-154 block) add `DevicePolicy.h/cpp` ("pure no-Bluetooth device policy: classify / filter / reconcile"), `CoreAudioDeviceInfo.h/cpp` ("macOS device enumeration: JUCE-identical names + transport types + aggregate members"), `DeviceGuard.h/cpp` ("GuardedDeviceType decorator + GuardedAudioDeviceManager"); "Debugging Audio Issues" (line 345) item 4: "No sound / features flat: read `GET /api/debug/audio_devices` (TEST_SERVER build) or the startup stderr `[AudioEngine] audio devices:` line -- a Bluetooth / AirPlay / wireless device is hidden by policy (Pitfall NN); the label says so when nothing wired is available."
- `docs/claude/testing-eyes.md` (after the gl_context_cycle paragraph, line 15): "**Audio device policy witnesses, test-server builds (s-rta-0929b btguard)**: `GET /api/debug/audio_devices` (production port, no `--test-mode`) returns the last device scan (every CoreAudio device with its transport, allowed flag and reason, the filtered lists and defaults), the opened devices (names, rate, buffer, channels), `opens` (device starts since launch: 1 after a normal launch) and `state` (`ok` / `no-input` / `no-device`), read from a mutex-guarded copy published on the message thread. Env `ADNA_AUDIO_DENY_DEVICES=<name>[;<name>]` (read once at the first scan) treats those exact device names as Bluetooth -- `.harmony/probe-btguard.sh` arms B / C. Never connect a Bluetooth device for a gate."
- `.harmony/APP-INVENTORY.md`: counts row line 31: "967 unit tests / 107 Catch2 targets" -> "987 unit tests / 108 Catch2 targets" (builder substitutes the real `ctest -N` count); the TEST-ONLY paragraph (line 204-222): add `GET /api/debug/audio_devices` + `ADNA_AUDIO_DENY_DEVICES` (one sentence each, same style).
- `.harmony/HANDOFF.md` ledger item 14 (Harmony's edit, text offered): "Carried: JUCE >= 8.0.9 (NOT 8.0.8: f6df3e3 is first tagged 8.0.9; the CoreAudio file is byte-identical 8.0.4..8.0.8) before any wired interface, with the #1601 separate-in/out regression risk; the Bluetooth guard (s-rta-0929b btguard) closes the crash's trigger, so the bump is off the crash's critical path."

## 9. RECOMMENDATION ONLY -- the carried "JUCE bump before any wired interface" deferral

- Corrected minimum: **JUCE 8.0.9** (research-heap.md 0.1, RE-VERIFIED by Harmony: blob 143d626a at 8.0.4 and 8.0.8,
  f6df3e3 + ff99341179 land 8 commits after 8.0.8; s-rta-0929b-work.md:13). Any ledger / plan text saying 8.0.8 is wrong.
- With the guard in place the ONLY remaining trigger of the 8.0.4 overflow is a device whose read-back frame size is
  smaller than the requested 512 (clamp) or reported late (stale) at `reopen` (bt-crash-mechanism.md OPEN a/b): built-in
  devices offer 512 (range [15..4096] measured 09-24); class-compliant USB / Thunderbolt interfaces normally accept 512
  (INFERRED, not measured -- none on the rig). So: no bump now; TRIGGER = the first wired interface: run bt-crash-mechanism.md
  E1 (`ca_probe_set 512 input|output`, read-back at +0/+50/+500/+2000 ms and frames delivered) on THAT interface before
  a show; read-back == 512 and delivered == 512 -> the 8.0.4 path is safe for it; otherwise pick one of:
  | option | delta | fixes | risk |
  |---|---|---|---|
  | (a) cherry-pick f6df3e3 + ff99341179 onto 8.0.4 (FetchContent `PATCH_COMMAND`, bt-crash-fix-plan.md Appendix B mechanics) | one file (the patch base == our file, VERIFIED by hash equality; that the two commits touch only juce_CoreAudio_mac.cpp is INFERRED -- confirm with `gh api` compare before applying) | the temp-buffer overflow (C1) | carries whatever in that change causes #1601 (INFERRED; JUCE could not reproduce); we own a patched RT file; must be dropped at the next bump |
  | (b) tag 8.0.9 .. 8.0.15 | ~1,200 commits of GUI/GL/audio churn; RowOrder API break with zero call sites (bt-crash-fix-plan.md, grep VERIFIED then) | C1 | open issue #1601: "glitchy breakup with differing input and output devices" -- this app's mic + speakers combiner; the restartAsync race stays (8.0.15 L1744-1757) |
  | (c) JUCE 9.0.x (9.0.3 current) | major: new CoreAudio backend (aggregate devices, no `AudioIODeviceCombiner`), licence banner, unevaluated OpenGL/GUI changes | C1 AND the HAL-thread restartAsync class (C2) | largest regression surface for the renderer / Output window; needs its own lane with the full probe battery and Eyes |
  Order of preference when the trigger fires: (a) for a show-critical fix window (smallest, revertible, one-file), soak
  >= 5 min on the combiner with `audio_callback_gap_max_ms` / `audio_xruns` (`/api/state` load{}) as the #1601 witness;
  (c) as the strategic move when a GL/GUI regression budget exists; (b) only as a stepping stone to (c). The decorator of
  this lane survives all three (it sits above the type).

## 10. COMMIT SEQUENCE (each builds, ctest green, the five probes green)

- **C1 `feat(btguard): no-Bluetooth device policy -- GuardedAudioDeviceManager, reconcile, labels (+ test_device_policy)`**:
  4.1-4.4 (except the TEST-ONLY status var), 4.6 label + onDeviceStateChanged, 4.8, 4.9, pitfalls/CLAUDE.md/architecture
  docs. Rig behaviour unchanged (built-in devices are allowed and default). ctest 987/987 serial; probes GREEN.
- **C2 `test(btguard): TEST-ONLY device witnesses -- /api/debug/audio_devices, ADNA_AUDIO_DENY_DEVICES, opens counter; probe-btguard.sh`**:
  4.5, 4.7, the env hook, MC provider, probe + testing-eyes/inventory docs. probe-btguard 21/2 (A6/A7 RED: `opens` 2 / 3 --
  the real-code RED for C3), probes GREEN.
- **C3 `fix(btguard): one device open per launch -- setSourceMode no longer re-opens the device`**: AE:107-110/:119-122
  removal + comment. probe-btguard 23/0 x5; step3/resync/onset/downbeat/manual-bpm GREEN; `[AudioEngine] Switched` lines
  present (A8).
- Harmony (not the builder): ledger item 14 text (section 8), Pitfall number, `HANDOFF` loose ends: HAL-thread restartAsync
  race (JUCE 8.x, only JUCE 9 removes it); temp-device listener lifetime; aggregate branch unexercised; E1 probe on the
  first wired interface.

## 11. OPEN QUESTIONS FOR HARMONY (decide at adoption; defaults stated)

1. Deny breadth: Bluetooth + BLE (Boris) + AirPlay + wireless Continuity (this plan's reading of "hard wired or onboard
   mic"). Default: as planned; trimming = one table.
2. Adopt a newly plugged wired mic while running only when the app has NO input (planned); never switch away from a
   working device even if macOS makes the new one default. Default: as planned (show stability).
3. Label sentences (F3: whole words, no jargon, never a modal): "No wired mic found. Bluetooth is never used." /
   "No audio device found. Bluetooth is never used." Default: as planned.
4. Fix the pre-existing mislabel ("Mic: MacBook Pro Speakers @ 48000Hz" names the OUTPUT device): planned yes.
5. Unknown transport (0) allowed (fail open, logged) -- or fail closed? Default: allowed.

## COMPACT

- DESIGN: `GuardedAudioDeviceManager` (subclass, `createAudioDeviceTypes` override ADM.h:412) wraps JUCE's CoreAudio
  `AudioIODeviceType` in `GuardedDeviceType`; the pure `devpolicy` table hides Bluetooth / BLE / AirPlay / wireless
  Continuity (+ aggregates containing one) from `getDeviceNames`, answers `getDefaultDeviceIndex` with the macOS default
  if allowed else built-in, refuses hidden names in `createDevice`; transports from `kAudioDevicePropertyTransportType`
  keyed by JUCE-identical names (VERIFIED on rig: both built-in devices `bltn`, names byte-equal). Every JUCE open path
  (2.1 rows 1-9) consults the type -> structurally closed, incl. JUCE's list-changed fallback to the default (ADM:446-449).
- STARTUP: guard active at AE:14 (first open) by construction; `setSourceMode` re-open removed (VERIFIED no-op for the
  device config: ADM:709-727 re-enables {0,1}; rate/buffer identical) -> one open per launch.
- STATES: no allowed input -> output-only + "No wired mic found. Bluetooth is never used."; nothing -> no device + "No
  audio device found. Bluetooth is never used."; app launches and renders (resampler bypass at rate 0). Hidden, not
  shown-disabled (no picker exists; JUCE has no disabled row).
- HOT-PLUG: JUCE ignores default changes while open (CA:1435-1447); earbud connect = detail re-read, no restart on
  built-in (CA:1126-1130); app reconciles on the change message (device gone / first input -> Reapply, else Keep);
  HAL-thread restartAsync race unreachable via Bluetooth (per-device listener CA:334) but FILED (JUCE 8.x, fixed only by 9).
- GATES: ctest `test_device_policy` (20 cases incl. real-ADM M1/M2 control/E1 real HAL names; RED vs stub ~14 FAIL);
  `.harmony/probe-btguard.sh` 23 rows, RED on main 13/10, C2 21/2, C3 23/0 x5; step3 94/0, resync 16/0, onset 13/0,
  downbeat 14/0, manual-bpm 18/0 stay GREEN; no Bluetooth, no system setting.
- JUCE: minimum 8.0.9 (not 8.0.8); no bump this lane; trigger = first wired interface -> E1 probe; then cherry-pick
  f6df3e3+ff99341179 (small, #1601 risk) or JUCE 9 (removes the combiner + race).
- FENCE kept: src/audio/* (3 new files + AudioEngine + TEST-ONLY counter), MC label/provider lines, ApiServer TEST-ONLY
  route, tests, .harmony probe, docs; CLAUDE.md paid by moving the Latency Budget table to architecture.md.

STATUS: COMPLETE -- plan ready for the builder; 5 open questions defaulted; no code edited.

## HARMONY ADOPTION (s-rta-0929b, 2026-09-30) — OVERRIDES the plan body where they differ
Plan authored on Fable (tier verified). Attacked blind: attack-btguard-coreaudio.md (4 MUST), attack-btguard-vj.md (4 MUST),
attack-btguard-gates.md (8 MUST). Rulings below are HARMONY DECISIONS unless a line quotes Boris; Boris's words on this subject
are ONLY the two quotes in binding-decisions.md 485-488. SHOULDs: the builder adopts each unless it conflicts with a ruling here or
the fence, and records "adopted / declined + reason" per SHOULD.

BG1 (Q1, Q5, coreaudio M1, vj M1, gates M3/M4/M7/M8) DENY SET = the wireless transports: Bluetooth, BLE, AirPlay, wireless
    Continuity ('ccwl') and the deprecated Continuity Capture ('ccap', wired-ness unknowable). This is HARMONY's reading of
    "we will only use hard wired sound input or the onboard mic"; docs / pitfall text attribute only the two quotes to Boris and
    name the rest as the app's rule. Virtual ('virt') and a genuine reported Unknown (0) stay ALLOWED + logged (system-audio
    routing uses virtual devices). A property READ ERROR is not Unknown: it fails CLOSED (hidden, logged, rescanned next change).
    static_assert every mirrored fourcc == its SDK kAudioDeviceTransportType* constant in CoreAudioDeviceInfo.cpp. E1 and live
    rows A2/A3 assert the rig's built-in devices read transport == 'bltn' EXACTLY (0 = FAIL); a ctest injects a read error.
BG2 (coreaudio M2) Aggregates / multi-output: denied if ANY member is denied, has an unresolvable UID, or has a UID of the
    Bluetooth shape (XX-XX-XX-XX-XX-XX[:input|:output]). Pure test P3b ({USB, unresolvable} -> denied).
BG3 (vj M3 + coreaudio M3) C3 (removing setSourceMode's re-open) is DROPPED from this lane and filed: it changes the normal mic
    path and JUCE's list-change fallback (lastExplicitSettings stays null -> "No such device"). Today's open sequence stays
    byte-for-byte; the guard works through the guarded device type on every open, including that re-open.
BG4 (coreaudio M3/M4, gates M2, Q2) NO app-level Reapply / reconcile in this lane by default: JUCE's own list-change fallback
    runs through the guarded type. ctest M5 (real GuardedAudioDeviceManager + mock {denied default, builtinA, builtinB}: open,
    remove the OPEN device, fireListChanged, pump the MessageManager) must show JUCE re-inits onto an ALLOWED device and the spy
    never sees a denied name. ONLY if M5 proves JUCE ends with no device while an allowed one exists: implement reconcile case
    (i) "device gone" alone, coalesced through a >= 250 ms message-thread timer, <= 1 Reapply per 10 s, never while a restart is
    pending, with a storm ctest (20 list changes -> <= 1 reopen). Case (iii) "adopt a newly plugged mic" is filed, not built.
BG5 (gates M1) MockDeviceType is an inner SPY recording every createDevice / open / start name, JUCE's temporary devices
    included; every manager case asserts no denied name was ever created; control M2 (unguarded) asserts the spy DID see it.
BG6 (vj M2, Q3) The no-device / no-input state is a PERSISTENT status indicator independent of setFileLabel (every other label
    write leaves it visible) while the state != ok. Sentences as planned (plain words, never a modal). Gate: arm B, trigger a
    label-writing action via REST, the sentence is still shown (UI text endpoint). This is a visual change: one critic seat
    (visual + UX + logic hats, micro-artifact proportionality) judges window-only captures of it before merge.
BG7 (vj M4) No-device (rate 0) is proven beyond render_frame: arm C rows arm + stop a performance take, start + stop a video
    recording (offscreen; never an Output window), switch the audio source both ways -> HTTP 200, app alive, 0 new .ips; clamp
    deviceRate / CaptureFacts.rate where a 0 reaches arithmetic.
BG8 (gates M5) Say plainly that the env hook drives the hiding branch, not the transport branch. Add a live row that denies only
    the DEFAULT output while a second allowed output exists; the probe prints SKIP when the rig has no second output; a SKIP never
    counts toward the pass total.
BG9 (gates M6) Production absence is gated: build a TEST_SERVER=OFF app once; `strings` shows no ADNA_AUDIO_DENY_DEVICES and no
    /api/debug/audio_devices; a grep shows every getenv of the variable inside #if AUDIODNA_TEST_SERVER; Config::testDeniedNames
    and the "test-denied" reason compile only under that define.
Q4 (the "Mic: MacBook Pro Speakers" mislabel) ADOPTED as a Harmony decision: it is on the line this lane rewrites.
Commits: C1 policy + guard + indicator + tests + docs -> C2 TEST-ONLY witnesses + probe; C3 dropped. Probe totals are recomputed
    for the dropped C3 (no "opens 2/3" row as a RED for C3; keep an opens counter as INFO).
Fence (Harmony constraint): src/audio/* (new DevicePolicy / CoreAudioDeviceInfo / DeviceGuard + AudioEngine), the label /
    indicator lines in src/MainComponent.cpp (+ the UI component that shows the indicator), src/api/ApiServer TEST-ONLY route,
    RecorderHost / perf-arm deviceRate clamps (BG7 only), tests, .harmony/probe-btguard.sh, docs/claude/*.md, CLAUDE.md (paid).
