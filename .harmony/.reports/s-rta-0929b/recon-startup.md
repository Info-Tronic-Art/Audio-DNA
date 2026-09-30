# s-rta-0929b RECON: startup timeline + candidate heap corrupters (source recon, read-only)

Stamped 2026-09-29 23:40 EDT. Repo tree main d88d2ea (git status: only .harmony-version M, AGENTS.md ??). Nothing in the repo was edited, built or launched.
Labels: VERIFIED = I read the cited line / ran the cited read-only query; INFERRED = follows from verified facts; ASSUMED = not checked.
Sources used besides the source tree: the 7 .ips files (python parse), macOS unified log (`log show`, read-only), `system_profiler`, JUCE 8.0.4 sources under build/_deps/juce-src, the peer lane's prebuild.md.

## 0. Answer first

1. **The premise "the Bluetooth trigger is absent from the 09-29 crashes" is FALSE.** The unified log shows both 09-29 crashing launches ran with the Bluetooth headset `soundcore P31i` (BDA 34:09:C9:F5:E9:D6) as default input AND output, while every non-crashing launch in the same session used the built-in mic/speakers. 2 of 2 BT launches crashed; 0 of 9 built-in launches crashed (table in section 1). VERIFIED.
2. In both crashing launches coreaudiod logs the headset switching A2DP 44.1 kHz -> HFP 16 kHz **during app startup** and renegotiating the IO frame size from 512 to **320** ("Negotiated Frame Size, HAL Request : 512 BTHAL Proposed 512" then "Update Max Supported Frame Size 320" then "HAL Request : 512 BTHAL Proposed 320"). Audio-DNA/JUCE keeps requesting 512. VERIFIED (log lines quoted in section 1).
3. The malloc error text in the unified log says **"Corrupt value: 0x0"**: the freed block's free-list header was overwritten with zeros. This app outputs silence, so any over-run of an audio output buffer writes zeros. VERIFIED (log line quoted).
4. **Top candidate (INFERRED, strong):** JUCE 8.0.4 CoreAudioInternal keeps `bufferSize` = the REQUESTED 512 (`reopen()` overwrites it, juce_CoreAudio_mac.cpp:674-675) while the HAL delivers 320-frame cycles; `audioCallback` then loops `bufferSize` frames over HAL/temp buffers (lines 793-797, 827-832), writing 192 frames of zeros past the end of the output buffer per IO cycle. JUCE only corrects `bufferSize` from a 100 ms Timer on the MESSAGE thread (lines 848-852, 1118-1130), and the message thread is inside `MainComponent`'s constructor for the whole crash window (0.42-1.6 s), so the mismatch is never repaired until the run loop starts. This matches: BT-only, timing window, zero value, detection in unrelated frameworks on the message thread.
5. Second candidate: HAL notification thread runs `AudioIODeviceCombiner::restartAsync()` (close()) concurrently with message-thread device ops (no shared lock) and/or JUCE listener callbacks with dangling `inClientData`. Direct evidence: 09-23 211737 crashed INSIDE `restartAsync` on "HALC_ShellPlugIn Connection Queue" (BT era). Same BT trigger.
6. App-side code (AudioTap, AudioCallback, CombinedCallback, the asyncload witnesses) is NOT a startup writer: at startup the tap is not armed and `push()` returns before touching any buffer (section 4). VERIFIED by reading.

## 1. Evidence table: Bluetooth vs crash (unified log, 2026-09-29 20:56-21:03, `log show`, predicate process == "Audio-DNA" AND message CONTAINS "setPlayState Started")

| Audio-DNA pid | first start | device (input/output) | result |
|---|---|---|---|
| 1420, 1513, 1531, 1602 | 20:56:02 ... 20:57:32 | BuiltInMicrophoneDevice / BuiltInSpeakerDevice | alive |
| **1681** | 20:58:15.431 | **34-09-C9-F5-E9-D6:input / :output (BT)** | **CRASH 20:58:15.567 (.ips 205817)** |
| **1716** | 20:59:18.731 | **34-09-C9-F5-E9-D6:input / :output (BT)** | **CRASH 20:59:18.87 (.ips 205919)** |
| 1837, 1855, 1923, 1990, 2043 | 21:00:21 ... 21:02:56 | BuiltIn mic / speakers | alive |

- Headset connect: bluetoothd 20:57:55.579-.712 "ACL connected: 34:09:C9:F5:E9:D6", 20:57:56.006 "Nm 'soundcore P31i'" (between pid 1602's start and quit). Disconnects: 20:58:17.881 and 20:59:21.332 (after each crash: the app's SCO link ended). Between 20:59:21 and 21:00:21 the headset went away and the next launches used built-in again. `system_profiler SPBluetoothDataType` now: Bluetooth Controller State: Off (23:29). So "no Bluetooth audio" is true NOW, not at crash time. All VERIFIED.
- pid 1681 coreaudiod/Audio-DNA excerpts (20:58): 15.373-.374 "BluetoothHALPlugIn_register clientID=20998/20999 (PID=1681)"; 15.374 and 15.384 "Negotiated Frame Size, HAL Request : 512 BTHAL Proposed 512"; 15.385 "BTUnifiedAudioDevice: Setting sample rate 44100.000000 -> 16000.000000"; 15.386 "Update Max Supported Frame Size 320"; 15.388-.392 "HAL Request : 512 BTHAL Proposed 320" (dozens of lines, every client); 15.386-.411 flurry of PauseIO/ResumeIO on Audio-DNA's own IO context 20998; 15.431/.433 device #1 "setPlayState Started Input/Output 0xa"; 15.460/.493 Stopped (setSourceMode reopen); 15.548/.556 device #2 Started (0xb); 15.559 AVCaptureDevice (refreshCameraList); 15.566-.567 CMIO DAL init; **15.571 "Audio-DNA(1681,0x1ef7162c0) malloc: Incorrect checksum for freed object 0x124040e10: probably modified after being freed. Corrupt value: 0x0"**. pid 1716 shows the identical sequence at 20:59:18.674-.733 (Proposed 320, "Update Max Supported Frame Size 320").
- Rapid relaunch is NOT the trigger: pid 1602 (built-in) quit at 20:58:14.09 and pid 1681 launched at 20:58:15.15, but 1837..2043 launched ~45-60 s apart and were fine; the discriminator is the device, not the gap. (INFERRED; the built-in launches with short gaps, e.g. 1513 -> 1531, were 12 s apart and fine.)
- 09-23 crashes: the earlier diagnosis (.reports/s-rta-0923-startup-crash-diagnosis.md section 0) records the 16 kHz HFP `soundcore P31i` as default at 21:17-21:19 and the "device sample rate is 16000 Hz" warning in that session's logs. 09-24 124843: the unified log for 09-23/24 is purged (`log show` returns nothing), so device unknown. UNVERIFIED whether BT was on.
- One more piece of the same picture: AudioTap.cpp:246-258 (H1FIX comment) already documents "a BT/HFP block larger than the device's last-announced buffer size" (tests/test_bt_device_shapes.cpp): the authors saw numSamples exceed the announced size on BT before.

## 2. Startup timeline (execution order, file:line)

Process start -> first ~2 s. Crash-run overlay in brackets (pid 1681, launch 20:58:15.147).

A. Process / static init
1. dyld -> `main` from `START_JUCE_APPLICATION(AudioDNAApplication)` src/Main.cpp:99. JUCEApplicationBase::main -> initialiseApp -> `AudioDNAApplication::initialise` Main.cpp:11. [0.10-0.19 s: cfprefs, AppKit checkin, SkyLight]
2. Static initialisers: `static uint32_t s_nextClipId = 1000` MainComponent.cpp:15 (trivial). Regex grep of src/ (excluding test/) found no namespace-scope std::vector/map/string/juce::String globals and no `__attribute__((constructor))`, `JUCE_IMPLEMENT_SINGLETON`, `SharedResourcePointer` in app code. INFERRED clean (regex-limited). Shader sources are inline strings (EmbeddedShaders.h, compiled later on the GL thread).
3. Main.cpp:26 `make_unique<MainWindow>`; MainWindow ctor: DocumentWindow base ctor (TopLevelWindow; no peer yet), then Main.cpp:48 `new MainComponent(testMode,testPort)`.

B. MainComponent member initialisation (declaration order, src/MainComponent.h; all run BEFORE the ctor body)
4. lookAndFeel_ (h:277), presetManager_ (h:~293, MilkDrop preset manager, scan only), milkDropBaseDirs_.
5. `RingBuffer<float> ringBuffer_{16384}` h:302: `new float[16384]{}` = 65,536 B (RingBuffer.h:17-21). Freed only at destruction.
6. **`AudioEngine audioEngine_{ringBuffer_}` h:303 -> AudioEngine::AudioEngine (src/audio/AudioEngine.cpp:3-22)**, exact order:
   - member init (h:70-84 AudioEngine.h): deviceManager_, formatManager_, sourcePlayer_, transportSource_, readAheadThread_ (not started), audioCallback_(ringBuffer), combinedCallback_(sourcePlayer_, audioCallback_) which owns AudioTap (AudioTap.h:221 `TimeSliceThread flushThread_` not started; gapStorage_ 64x16 B).
   - AudioEngine.cpp:7 registerBasicFormats; :8 transport addChangeListener; :9 `sourcePlayer_.setSource(&transportSource_)` (source now NON-null, so the File-mode branch of AudioSourcePlayer runs on every callback until Mic mode is set); **:10 `readAheadThread_.startThread` = "AudioReadAhead"**.
   - **:14 `deviceManager_.initialiseWithDefaultDevices(2,2)`** -> AudioDeviceManager::initialise -> scanDevicesIfNeeded -> initialiseDefault (juce_AudioDeviceManager.cpp:325-401) -> insertDefaultDeviceNames (:518-606) creates and destroys two TEMPORARY CoreAudioIODevice objects (`createDevice(out,"")`, `createDevice("",in)`, :556-559) just to read sample rates; then setAudioDeviceSetup (:730-839): createDevice -> AudioIODeviceCombiner (input and output are different AudioDeviceIDs on this Mac: built-in mic and speakers, and the BT input/output pair) -> `open()` -> `start(callbackHandler)` (:817) which starts BOTH CoreAudio IOProcs => two "com.apple.audio.IOThread.client" threads (device #1). The callback list is EMPTY at this point: the IO callback only zeros the output (AudioDeviceManager.cpp:1049-1053).
   - **:21 `deviceManager_.addAudioCallback(&combinedCallback_)`** (juce_AudioDeviceManager.cpp:971-985): calls `CombinedCallback::audioDeviceAboutToStart` on the message thread WITHOUT audioCallbackLock and BEFORE the callback is in the list (:980-984), so the IO thread cannot reach it yet. Chain: AudioSourcePlayer::prepareToPlay; AudioCallback::audioDeviceAboutToStart (`monoBuffer_.resize(bufferSize)`, rate cell store); **AudioTap::prepare (CombinedCallback.h:192 -> AudioTap.cpp:35-103): allocates pendingStorage_ (channels x rate*2 floats) and silenceStorage_ (channels x maxBlock floats), and starts "AudioTapWriter" (AudioTap.cpp:101-102)**. Then `callbacks.add` under the lock. From here every IO cycle runs CombinedCallback.
7. `AnalysisThread analysisThread_{...}` h:308: ctor pre-allocates FFT/aubio/etc. (AnalysisThread.cpp:26-53); NOT started (started at MainComponent.cpp:1853).
8. UI members; `PreviewPanel previewPanel_` h:333 -> Renderer -> `ImageDecode::Decoder imageDecoder_{3}` (Renderer.h:303 -> ImageDecode.h:120-126) = **3 "ImageDecode" pool threads**; PreviewPanel.cpp:20 `renderer_.attachTo(glHost_)` (Renderer.cpp:52-58): OpenGLContext::attachTo only creates an Attachment watcher (juce_OpenGLContext.cpp:1317-1325); the CachedImage / GL thread are created only when the component is showing in a peer (:1129-1193), i.e. later.
9. mappingTickTimer_ h:355 (not started yet), effectLibrary_, `output::OutputManager outputs_` h:393, composition_ h:412, `MediaPresenceSweeper presence_` h:437 -> ThreadPool ctor = **"MediaPresence" thread** (MediaPresence.h:155; the 1 Hz timer starts later, cpp:664), `MediaOpener mediaOpener_` h:455 -> **2 "MediaOpen" threads** (MediaOpener.cpp:66-71, kThreads=2 h:35), then apiServer_ (null), oscHandler_, midiOutputHandler_, videoRecorder_ h:662, syphonOutput_, recorderHost_ h:670, routineEngine_.

C. MainComponent ctor body (src/MainComponent.cpp:210-2310), top-level statements, trivial ones collapsed
10. :212-246 setLookAndFeel, installAsDefault, addAndMakeVisible x6, labels.
11. :283-298 fast-save dir scan (findChildFiles).
12. :299-303 audioSourceSelector_ items; **:303 `audioEngine_.setSourceMode(MicInput)`** -> AudioEngine.cpp:95-111: transport stop, `useInputForAnalysis=true`, `setup.inputChannels.setRange(0,2,true)`, **`deviceManager_.setAudioDeviceSetup(setup,true)`**. The built-in mic has 1 input channel (`system_profiler SPAudioDataType`: Input Channels: 1) so the requested {0,1} differs from the current {0} => the device is stopped, closed, re-opened and restarted (juce_AudioDeviceManager.cpp:735-820): combiner.stop/close/open/start, IOProcs destroyed and recreated (juce_CoreAudio_mac.cpp:734-759), `audioDeviceStopped` then `audioDeviceAboutToStart` delivered again to CombinedCallback (AudioTap::prepare second call, AudioCallback resize), 2 NEW IO threads (device #2). VERIFIED for the code path; the "reopen happens every launch" part rests on the 1-channel mic (VERIFIED system_profiler) plus the log showing Started 0xa then Stopped then Started 0xb in EVERY launch (VERIFIED, table above).
13. :304-370 selector/gain wiring, labels (no audio calls except lambdas).
14. **:372 `refreshCameraList()`** (impl :4313-4322) -> `juce::CameraDevice::getAvailableDevices()` -> AVCaptureDeviceDiscoverySession -> first in-process CoreMediaIO initialisation (DAL plug-in scan, CMIOExtension XPC). This is the first big burst of small-zone allocations after device start, which is why 3 of 5 aborts are detected here.
15. :440-452 fps/cpu labels, `startTimerHz(30)` (:448), `mappingTickTimer_.startTimerHz(120)` (:452). JUCE timers post to the MESSAGE thread (juce_Timer.cpp:109,264-271), so no timer callback runs until the ctor returns and the run loop starts.
16. :479-499 onError lambda, `hasAudioDevice/getCurrentSampleRate` reads (message thread).
17. :501-664 effectLibrary_.registerDefaults, EffectsRackPanel, TooltipWindow, `composition_.initDefault`, signalRegistry_, renderer wiring, TopBar, SignalBar, DeckView, **`presence_.start` :664**.
18. :667-1786 genre/structural callbacks, DeckView lambdas, InspectorPanel (:1456), BrowserPanel (:1522, creates FilesBrowser -> "FilesBrowserThumbs" x2, FilesBrowser.h:92), TimingWindow (:1787), AudioDNAMenuBar (:1791), OutputManager settings (:1799), BindingOverlay/MidiLearnOverlay (:1834-1839).
19. :1842-1843 `MidiHandler::start(audioEngine_.getDeviceManager())` (CoreMIDI callbacks via the device manager).
20. **:1852-1853 `analysisThread_.startThread(high)` ("AnalysisThread")** (skipped in test mode).
21. :1857-1883 TestServer (test mode only; std::thread TestServer.cpp:82).
22. **:1892 `apiServer_` created, :2184 `apiServer_->start()`** (ApiServer.cpp:93 std::thread = httplib listener + worker pool threads).
23. :2273-2274 **`oscHandler_.startListening(8000)`** ("JUCE OSC server").
24. :2288-2301 key listener, OverlayWatch, `NativeLayerHost::attach` x2 (VBlankAttachment/display-link registration, NativeLayerHost.mm:101), :2309 `setSize(1280,800)`. ctor returns.
25. Back in MainWindow ctor: Main.cpp:50 `setUsingNativeTitleBar(true)` -> addToDesktop -> NSViewComponentPeer -> NSWindow init (crash 211855 site), :51 setContentOwned, :58 setMacMainMenu, :69 setBounds, :75 `setVisible(true)`; JUCE creates the OpenGL CachedImage and starts the "OpenGL Renderer" thread now (juce_OpenGLContext.cpp:1184-1193) (present only in the 09-24 dump).
26. JUCEApplicationBase::main enters the run loop: first 30 Hz / 120 Hz timers fire; the 100 ms CoreAudioInternal/combiner timers that were armed by HAL notifications during the ctor fire here (this is where JUCE finally re-reads the real buffer size and restarts the device).

### Crash-run wall clock (pid 1681)
launch .147 | .336 first CoreAudio use (AudioComponentRegistrar) | .373 our IO contexts registered with the BT HAL plug-in | .385-.392 BT A2DP->HFP, frame size 512->320 for all clients | .431 device #1 started | .460-.493 stopped (setSourceMode reopen) | .548/.556 device #2 started | .559 refreshCameraList | **.571 malloc abort (+0.42 s)**. Total ctor time up to the crash: ~0.24 s. Everything in the window is message thread ctor code + IO threads + HAL threads.

## 3. Threads existing at crash time: where started, before refreshCameraList (cpp:372)?

| Thread (name in .ips) | Started at | before 372? | Basis |
|---|---|---|---|
| JUCE Message Thread | process main | yes | Main.cpp:99 |
| JUCE Timer | first `Timer::startTimer` (JUCE TimerThread singleton); tid is lower than every app thread | yes | tid order; creator not identified (INFERRED: window/LookAndFeel init) |
| AudioReadAhead | AudioEngine.cpp:10 | yes | VERIFIED, idle: no reader source, no TimeSliceClient |
| caulk.messenger.shared:17/:high, GCD wq threads, HAL "HALC_ShellPlugIn" queues | created by CoreAudio/HAL on first use (AudioEngine.cpp:14) | yes | tid order; Apple-owned |
| com.apple.audio.IOThread.client x2 | HAL at AudioDeviceStart: device #1 AudioEngine.cpp:14 (juce_AudioDeviceManager.cpp:817), device #2 MainComponent.cpp:303 -> AudioEngine.cpp:110 | yes | the two present in the 09-29 dumps have tids > MediaOpen, i.e. device #2 (VERIFIED by tid order + log 0xa/0xb) |
| AudioTapWriter | AudioTap.cpp:101-102 via AudioEngine.cpp:21 | yes | idle (no ThreadedWriter, no clients) |
| ImageDecode x3 | Renderer.h:303 / ImageDecode.h:120-126 | yes | idle until a decode job is queued |
| MediaPresence | MediaPresence.h:155 (pool in member presence_) | yes (pool); sweeps start cpp:664 (after) | idle |
| MediaOpen x2 | MediaOpener.cpp:66-71 | yes | idle |
| CVDisplayLink | not created by app or JUCE code before a peer exists: JUCE's PerScreenDisplayLinks is instantiated by OpenGLContext::CachedImage (juce_OpenGLContext.cpp:1050, created only on attach :1187) or NSViewComponentPeer (:1764,1826) | creator unidentified | its tid position varies between dumps (early in 211744/09-29, last in 211855/124843). Harmless to the analysis |
| AnalysisThread | MainComponent.cpp:1853 | **no** | absent from every dump that crashed in refreshCameraList |
| httplib listener + pool | ApiServer.cpp:93 via cpp:2184 | no | present only in 211855/124843 |
| JUCE OSC server | cpp:2274 | no | same |
| FilesBrowserThumbs x2 | FilesBrowser.h:92, BrowserPanel at cpp:1522 | no | same |
| OpenGL Renderer | juce_OpenGLContext.cpp:1184-1193 at setVisible (Main.cpp:75) | no | 124843 only |

Consequence (VERIFIED for the 09-29 pair, where AnalysisThread/httplib/OSC/FilesBrowserThumbs are absent and the stack ends in refreshCameraList): the corrupting write happened BEFORE cpp:372. Only these code streams ran in that window: message-thread member init and ctor body, the two IO threads (CombinedCallback chain), HAL callback threads (JUCE listeners), the (idle) pools. The idle pools run no app code. INFERRED: the writer is in the IO chain or the HAL-listener chain, or is message-thread code triggered by them.

## 4. Audio path in depth

### 4.1 Device manager calls and their order
- `initialiseWithDefaultDevices(2,2)` once (AudioEngine.cpp:14), member-init phase of MainComponent (h:303). Two temporary devices (insertDefaultDeviceNames), one combiner, start with an EMPTY callback list.
- `addAudioCallback(&combinedCallback_)` once (AudioEngine.cpp:21). AboutToStart runs unlocked but pre-registration (juce_AudioDeviceManager.cpp:980-984). No member is constructed after `addAudioCallback`: everything the callback touches (ringBuffer_, audioCallback_, combinedCallback_, sourcePlayer_, transportSource_, AudioTap) was built before the device even started. VERIFIED: the design is order-safe.
- `setAudioDeviceSetup` a second time at MainComponent.cpp:303 (setSourceMode) with the callback ALREADY registered: stop -> close -> open -> start under live IO. All later AboutToStart/Stopped deliveries go through AudioDeviceManager::audioDeviceAboutToStartInt/StoppedInt under audioCallbackLock (juce_AudioDeviceManager.cpp:1074-1101), so CombinedCallback's own buffers are quiesced while it resizes. VERIFIED.
- MidiHandler adds a MIDI callback to the same manager at cpp:1843 (after 372).
- Not audio-thread hazards but relevant: AudioSourcePlayer runs (source non-null) in file mode from device start until cpp:303 sets `useInputForAnalysis`; it copies mic input into the output scratch and can allocate on the IO thread (`tempBuffer.setSize`, juce_AudioSourcePlayer.cpp:114) when inputs > outputs; AudioDeviceManager::tempBuffer is allocated on the IO thread on the first callback after registration (juce_AudioDeviceManager.cpp:1020). Allocation on the IO thread is thread-safe; it is not a corruption source by itself.

### 4.2 What each callback writes (allocation site, size, who can realloc/free)
Sizes assume the default 512-frame request (app never sets a size; NOTE CLAUDE.md says 128, the code path gives 512) and 2 output channels. "small zone": the peer's prebuild.md measured a 512-byte quantum on this arm64; the upper threshold on a 32 GiB machine is ASSUMED (15 KB classic, possibly ~127 KB on large-memory Macs, recalled not verified).

| Buffer | alloc site | size | written by | realloc/free after IO can run | small zone? |
|---|---|---|---|---|---|
| RingBuffer buffer_ | RingBuffer.h:17-21 (member init) | 65,536 B | IO thread push (AudioCallback.cpp:46-49), clamped to available | never until dtor | maybe (ASSUMED) |
| AudioCallback::monoBuffer_ | AudioCallback.cpp:55 vector::resize in aboutToStart | 512*4 = 2,048 B | IO thread, `min(numSamples, size)` (AudioCallback.cpp:25) | aboutToStart only, quiesced by audioCallbackLock | yes |
| AudioTap silenceStorage_ | AudioTap.cpp:90-91 | ch x maxBlock*4 = 2 x 2,048 B (+ptr vectors) | never written after prepare; read by push pad path | prepare() only (message thread, quiesced) | yes |
| AudioTap pendingStorage_ | AudioTap.cpp:78-79 | ch x rate*2*4 = 2 x 384,000 B @48k (2 x 128,000 B @16k) | push() spill path only when a take is running | prepare() only, quiesced | no (large) |
| AudioTap gapStorage_ + fifo | AudioTap.h:308-309 | 64 x 16 = 1,024 B | push() only after a gap in a running take | never | boundary |
| Witnesses (asyncload) `lastTicks_`, `gapMaxTicks_`, `callbacks_`, `periodSamples_`, `overruns_` | CombinedCallback.h:207-215, AudioCallback.h:39-41 | 8-32 B inside the object, no allocation | IO thread relaxed atomics + one plain int64 `lastTicks_` (IO thread; reset in aboutToStart while quiesced) | none | n/a |
| AudioSourcePlayer tempBuffer | juce_AudioSourcePlayer.cpp:114,190 | (numIn-numOut) x numSamples x4; (2,8) at stop | IO thread | `audioDeviceStopped` reallocs it (:190) under audioCallbackLock via the manager | small |
| AudioDeviceManager::tempBuffer | juce_AudioDeviceManager.cpp:1020 | 2 x 512 x 4 = 4 KB | IO thread | grows on IO thread only | yes |
| CoreAudioInternal::audioBuffer + tempBuffers | juce_CoreAudio_mac.cpp:354-366 (called from updateDetailsFromDevice :512) | (bufferSize+4) x 4 x channels: 2,064 B (1 ch) / 4,128 B (2 ch) per internal | IO thread copies IN (:793-797), reads OUT (:821-832) | realloc under callbackLock in updateDetailsFromDevice (:496-513) | yes |
| Combiner scratchBuffer | juce_CoreAudio_mac.cpp:1576 | numOuts x bufferSize x4 ~ 4.1 KB | IO thread (the callback's outputChannelData) | open() only | yes |
| Combiner fifo | :1575 | numOuts x (targetLatency + 2*bufferSize) x4, latency-dependent, ~25-100 KB (BT latency is large) | IO threads (accessFifo :1840,1886) | open()/close() clear | maybe |

AudioTap at startup (VERIFIED, AudioTap.cpp:150-192): `push()` sets the busy flag, `armed_.exchange(false)` is false (start() is only called by the recorder), `running_` is false, returns 0. No buffer is read or written. `AudioTap::start()` is the only writer of `threadedWriter_/activeWriter_`. So AudioTap cannot corrupt the heap at startup; H1 "AudioTap fan-out" is not supported by the startup path. The only thing it does is realloc-on-prepare on the message thread under the manager lock.

### 4.3 JUCE 8.0.4 CoreAudio facts (juce_audio_devices/native/juce_CoreAudio_mac.cpp)
1. `audioCallback` (:764-845) copies exactly `bufferSize` frames in and out using the member `bufferSize`, never `mDataByteSize` of the HAL buffers: input loop :793-797, output loop :827-832 (`*dest = *src++; dest += stride`). tempBuffers are sized from `bufferSize+4` at the last `allocateTempBuffers` (:354-366).
2. `reopen()` (:645-684): sets nominal rate and `kAudioDevicePropertyBufferFrameSize` to the requested value (:660-663), calls `updateDetailsFromDevice` (:673) which stores the DEVICE-reported size and allocates temp buffers for it (:504,:512), then **overwrites `bufferSize = bufferSizeSamples` (:675) without the lock and without reallocating**. JUCE's own comment (:669-672): "some devices fail to correctly report their new settings until some random time in the future". If the device really runs a different cycle length than the requested one, the loops in point 1 run with the wrong count.
3. `getBufferSizesFromDevice` (:409-440) adds the current `bufferSize` to the "available" list even if outside the device's range (:432-433), so `chooseBestBufferSize` (juce_AudioDeviceManager.cpp:871-878) keeps re-requesting 512 for a device whose range is now 320.
4. The re-sync path: HAL property notification (`deviceListenerProc` :1150-1197, runs on a HAL queue) -> `deviceDetailsChanged` -> `startTimer(100)` (:848-852) -> Timer callback on the MESSAGE thread (:1118-1130) -> `updateDetailsFromDevice` + `owner.restart()`. With the message thread stuck in the ctor this cannot happen before the ctor returns.
5. `deviceListenerProc` also calls `deviceRequestedRestart()` (:855-859) directly on the HAL thread for kAudioDevicePropertyDeviceHasChanged / kAudioObjectPropertyOwnedObjects, which runs `AudioIODeviceCombiner::restartAsync()` (:1637-1652) = `close()` (:1581-1589: stop, `fifo.clear()`, `active=false`, `d->close()` for both wrappers) on the HAL thread under `closeLock` only. Message-thread `open()/start()/close()` (:1530-1589, :1664-1693) do NOT take `closeLock`, so they race with it (fifo.setSize :1575, scratchBuffer.setSize :1576 vs fifo.clear()/callback swap on the other thread).
6. Listener lifetime: CoreAudioInternal registers a wildcard listener with raw `this` (:334) and removes it in the destructor (:347) after `stopTimer()`; CoreAudioIODevice registers on the system object (:1248) with `internal.get()`. Removal does not wait for an in-flight callback. `insertDefaultDeviceNames` creates and destroys two devices in the first milliseconds (juce_AudioDeviceManager.cpp:556-559), and `deleteCurrentDevice` destroys a combiner on every setup change. `intern.xruns += n` (:1162) executes on EVERY notification, `startTimer(100)` (:851) on detailsChanged.
7. `handleAudioDeviceAboutToStart` (:1906-1951) forwards the individual sub-device (not the combiner) to `callback->audioDeviceAboutToStart(device)` (:1949-1950) whenever `callback != nullptr`; CombinedCallback.h:187-192 then calls `AudioTap::prepare(rate, channels=<sub-device active output channels>, ...)`. For the input sub-device that is 0 channels: the tap silently disables itself (functional bug, not a heap writer; low rank).
8. The 09-23 211737 fault: `restartAsync +56` on "HALC_ShellPlugIn Connection Queue" with PC in unknown memory (KERN_PROTECTION_FAILURE at 0x16f4f5c90, exec of a stack-range address), triggered from `ObjectsPublishedAndDied -> deviceListenerProc`, while the message thread was inside `AudioEngine::AudioEngine -> addAudioCallback -> AudioTap::prepare -> vector::assign` (a long memmove; likely just what the sampler caught). A jump to a data address from inside restartAsync means a virtual call through a freed/overwritten object (VERIFIED: stack; INFERRED: cause).

### 4.4 Why the detection sites look unrelated
The corrupted block is a FREE small-zone block whose header (checksum/prev/next, 16-24 B) was zeroed ("Corrupt value: 0x0", unified log). libmalloc only checks the header when it unlinks that block (small_free_list_remove_ptr_no_clear) from a malloc or when coalescing on free. The first heavy allocator users after device start are CMIO (refreshCameraList), NSWindow creation and TSM: that is where it trips. The writer runs on other threads (IO cycle every 20 ms on BT, every ~10 ms on built-in) and keeps re-zeroing the same bytes, so the window ends only when the device is resynced/restarted or the process dies.

## 5. Other early threads: any object they touch that the message thread frees/reallocs during construction?
- AudioReadAhead (TimeSliceThread, AudioEngine.cpp:10): no client registered until loadFile (transportSource_.setSource, AudioEngine.cpp:49); idle. VERIFIED.
- AudioTapWriter: no ThreadedWriter until `AudioTap::start` (recorder); idle. VERIFIED.
- ImageDecode x3, MediaOpen x2, MediaPresence: idle pools; jobs only from decode requests / begin() / the 1 Hz sweep started at cpp:664; they hold `WeakReference`/shared_ptr mailboxes. Nothing queued in the window. INFERRED (no job source before 372: no image/video load at startup).
- JUCE Timer thread: only posts CallTimersMessage to the message thread (juce_Timer.cpp:109). No app object touched.
- CVDisplayLink: callbacks only fan out to registered factories; none registered before a peer exists (PerScreenDisplayLinks :212-236 with an empty factory list). INFERRED harmless.
- CoreAudio IO threads and HAL queues: see sections 4.3-4.4. These are the only threads that write app/JUCE memory in the window.

## 6. Ranked candidates

### C1 (most likely). CoreAudio IO-cycle length vs JUCE `bufferSize`: BT HFP renegotiates to 320 frames while JUCE loops 512
- Site: juce_CoreAudio_mac.cpp:673-675 (bodge) + 793-797, 827-832 (loops); resync only via Timer (:848-852, :1118-1130). App request path: AudioEngine.cpp:14 and :105-110 request the default size (512) and never a fixed size.
- Mechanism (INFERRED): at startup the BT device is A2DP (Proposed 512). JUCE scans (ranges), picks 512, opens; during open/start the headset switches to HFP: max frame 320. HAL now cycles 320 frames. JUCE's `bufferSize` is 512 (either because it read 512 before the switch, or because :675 forces it) and JUCE keeps looping 512: each cycle it writes 192 extra frames (768 B per mono stream, 1,536 B per stereo) of zeros (our output is silence, CombinedCallback.h:168-169) beyond the HAL output buffer, and reads 192 frames past the HAL input buffer. Two IO threads => ~100 stomps/s until the message thread finally runs the 100 ms resync timer. Overrun zeros land on whatever follows (small-zone neighbours: 1.3-2.6 KB HAL buffers or 2 KB temp buffers).
- Threads: input IOThread + output IOThread (writers), message thread (victim, blocked in the ctor so it cannot resync), HAL thread (property notification).
- Fit: BT-only (2/2 vs 0/9, section 1); window 0.42-1.62 s = until the message thread reaches the run loop; zero value; detection collateral on the message thread; intermittency = headset connectivity/profile (headset reconnects on its own, seen 20:57:55); no crash on built-in where request == cycle (512 == 512). Also consistent with H1FIX's earlier BT/HFP over-read finding in AudioTap.
- Cheapest discriminating test (needs the headset connected as default I/O, ~2 min): launch the CURRENT Release binary under Guard Malloc: `DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib` (peer validated it stops at the writer; env-only MallocCheckHeap does not). Expected: SIGSEGV whose top frame is `CoreAudioInternal::audioCallback` at the output copy loop (juce_CoreAudio_mac.cpp ~827-832). Second: the ASan app (`build-asan/.../Audio-DNA.app`, JUCE instrumented) with the headset on: an ASan `heap-buffer-overflow WRITE ... audioCallback` if the HAL buffer is malloc-backed; silence means it is not (then gmalloc/vm guard is the only detector). Control: same builds with Bluetooth off, expect 0 crashes. No-BT repro (INFERRED to work): a scratch program (outside the repo) that keeps the built-in devices running at 128 frames makes the HAL cycle shorter than JUCE's 512 the same way.
- Caught by ASan? Only if the overrun destination was allocated through malloc (ASan interposes malloc for the whole process including CoreAudio-made buffers, and the store is in instrumented JUCE code). If the HAL IO buffer is vm/shared memory: ASan sees nothing; Guard Malloc sees only malloc'd memory too, so a vm buffer would show as EXC_BAD_ACCESS/silent instead. The observed small-zone corruption implies malloc-backed neighbours, so gmalloc is the safest first tool.

### C2. HAL notification thread vs message thread: `restartAsync()`/`close()` race and dangling listener `inClientData`
- Site: juce_CoreAudio_mac.cpp:1150-1197 (deviceListenerProc on HAL queue), :855-859, :1637-1652 (restartAsync = close() on HAL thread, closeLock only), message-thread open/start :1530-1589,:1664-1693 without closeLock; listener add/remove :334/:347/:1248/:1260; temp devices juce_AudioDeviceManager.cpp:556-559.
- Mechanism (INFERRED): a BT profile change publishes HAL objects (crash A: ObjectsPublishedAndDied) => `restartAsync` runs `fifo.clear()` (zero-fill of the fifo, no lock vs `fifo.setSize` on the message thread) or a virtual call on a device being torn down; or an in-flight listener callback dereferences a CoreAudioInternal/CoreAudioIODevice freed by `insertDefaultDeviceNames`/`deleteCurrentDevice` (writes: `xruns +=`, `Timer::startTimer` -> timer fields and a dangling pointer in TimerThread). Zero-fill and small header stomps fit the checksum signature.
- Threads: HAL "HALC_ShellPlugIn Connection Queue" vs message thread (+ IO threads reading `callback`/fifo).
- Fit: verified direct hit in 211737 (HAL thread inside restartAsync with a wild PC, message thread in the AudioEngine ctor); BT era; timing 0.4-1.6 s (device churn); intermittent by nature (narrow race). Weaker than C1 for the 09-29 pair because there is no direct trace of it in those dumps, but C1 and C2 have the same trigger and may both exist.
- Cheapest test: same BT-on launches under the ASan app: an ASan `heap-use-after-free` naming `Timer::startTimer`/`deviceListenerProc`/`restartAsync` with alloc/free stacks in CoreAudioIODevice/AudioIODeviceCombiner = C2; TSan build (already built) with BT flapping for a data race between `AudioIODeviceCombiner::close` and `open` (note: TSan cannot see Apple HAL internal synchronisation, expect noise).
- ASan catches this (all objects are JUCE C++ compiled in): yes if the race fires; gmalloc also traps a write to a freed page.

### C3. Ordinary startup churn when the device set changes mid-startup (any device, not only BT)
- Same mechanisms as C1/C2 but triggered by other HAL events (e.g. default-device change, aggregate device, Continuity). Included because the BT correlation does not exclude other triggers. `kAudioHardwarePropertyDevices` notifications also `triggerAsyncUpdate` a device-list rescan (juce_CoreAudio_mac.cpp:2301-2305) that runs after the ctor.
- Test: repeat N launches on built-in while forcing a HAL change (switch default output in System Settings during launch). Low priority until C1 is settled.

### C4. Deterministic overflow in uninstrumented third-party code (aubio, libav*, libprojectM) laid out next to a free block only sometimes
- aubio objects are created in AnalysisThread's ctor (message thread, member init) but not run before cpp:1853; libav/projectM not used before 372. So they cannot explain the three refreshCameraList aborts. Possible only for the two post-ctor aborts. ASan does not instrument these libraries (peer prebuild.md: libaubio, libav*, swscale not instrumented); gmalloc would.
- Fit: poor (does not explain BT-only). Rank low.

### C5. Apple framework bug (CMIO/AVF/AppKit/TSM) or a third-party DAL plug-in
- /Library/CoreMediaIO/Plug-Ins/DAL holds only plugins-info.txt (VERIFIED): no third-party DAL plug-in. The unified log says "will load 3rd party plug-ins: F". Only Apple extensions load. The heap error is detected in Apple frames but the corrupt value zero and the BT-only correlation point elsewhere. Rank very low. Only gmalloc could catch it.

### Not supported: H1 "AudioTap fan-out"
- push() is a no-op at startup (section 4.2). The H1FIX comment is the BT signal, not a writer.

## 7. Which tool sees which

| Candidate | ASan (JUCE + app instrumented) | TSan | Guard Malloc (whole process) | MallocCheckHeap env |
|---|---|---|---|---|
| C1 overrun of tempBuffers/HAL buffer | yes if destination is malloc-backed | no | yes (traps at the writer) | only detects later, at the metadata |
| C2 race / UAF in JUCE objects | yes when fired | data race report (noisy) | yes for UAF writes | later only |
| C4 aubio/libav overflow | only via libc interceptors | no | yes | later only |
| C5 Apple bug | no | no | yes | later only |
Peer-validated: gmalloc gives SIGSEGV at the bad write; MallocScribble/GuardEdges/CheckHeap do not (prebuild.md, gmtest.c results).

## 8. Risks and what I did not verify
- I did not observe the frame-size mismatch directly. Evidence chain: coreaudiod "Proposed 320" + JUCE code reading (:673-675, :793-832) + zero value + BT-only correlation. Alternative reading: only the HAL notification storm (C2) matters and the size mismatch is harmless. The gmalloc/ASan run with the headset on separates them.
- `getFrameSizeFromDevice()` value at each step and the actual `mDataByteSize` of the IO buffers were not read (no instrumentation allowed). A TEMPORARY env-var hook that logs, in the first 20 callbacks, JUCE's `bufferSize`, `inInputData->mBuffers[0].mDataByteSize/4/channels` (requires a JUCE-side print) would settle it in one run.
- Small-zone threshold on this machine is ASSUMED. The 512-byte quantum is from the peer (prebuild.md).
- 09-24 124843: device unknown (log purged). If BT was off then, C1 does not explain that crash and C2/C3 or another trigger is needed.
- Concurrent TSan .ips (Audio-DNA-2026-09-29-232714.ips) is the peer's smoke run (exit-time TSan abort from a std::cerr race), unrelated.
- CVDisplayLink and JUCE Timer thread creators are not identified by source (harmless).
- Quantitative crash rate under BT is small-sample (2/2, plus the 09-23 BT session).
