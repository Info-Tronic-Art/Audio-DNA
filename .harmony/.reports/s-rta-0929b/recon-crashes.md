# s-rta-0929b RECON: crash + launch-history analyst (read-only)

Stamped 2026-09-29 23:33 EDT (from `date`). Author: crash/launch analyst lane. Nothing was edited, built, committed or launched.
Labels: VERIFIED = I read the evidence line myself (file/log cited). INFERRED = derived from verified facts. ASSUMED = not checked.
Scratch evidence I generated (all under `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/bd9a1c96-8ac1-42b0-a088-3d8fb87be738/scratchpad/`, called `SP/` below):
`launchd-spawn.txt`, `spawns.json`, `playstate.txt`, `btdev.txt`, `errfiles.txt`, `parse*.py`, `spawn.py`, `errscan.py`.

## 0. Answer first

**The premise "the Bluetooth trigger is absent from the 09-24 and 09-29 crashes" is wrong for 09-29 (VERIFIED from the macOS unified log) and contradicted for 09-24 by Harmony's own work log.**
Both 09-29 crashes (20:58:15 and 20:59:17) were launches on which JUCE opened the Bluetooth headset "soundcore P31i" (BDA 34:09:C9:F5:E9:D6) as BOTH input and output at 16 kHz HFP, with coreaudiod logging `Negotiated Frame Size, HAL Request : 512 BTHAL Proposed 320`. That is exactly the JUCE 8.0.4 `CoreAudioInternal::reopen` "bodge" mismatch that the s-rta-0924b ASan run already pinned as a heap-buffer-overflow WRITE (`juce_CoreAudio_mac.cpp:795`, past a 1296-byte block from `allocateTempBuffers():361`).
Across the 663 Audio-DNA launches the unified log holds for today (12:40 to 23:25), exactly 2 opened the Bluetooth device and both crashed; the other 661 opened the built-in mic/speakers and none crashed.
So the heap signature is a deterministic function of "Bluetooth headset is the macOS default I/O at launch", not a rare race in app startup. A launch-count experiment on built-in audio cannot reproduce it (needs ~1000 to ~4000 launches per arm); with the headset connected 2 to 5 launches per arm suffice.

Strongest alternative (and why it loses): "the corruption comes from app code or from the CMIO/AVCapture enumeration and Bluetooth is a bystander". It loses because 661/661 non-BT launches with the identical startup path were clean, the mechanism is a source-verified, ASan-reproduced overflow with matching numbers (512 vs 320+4), and ReportCrash's malloc annotation shows `Corrupt value: 0x0` (zeros, i.e. silent audio samples), not a pointer or app data. What is NOT verified for 09-29 is the writer itself (no ASan on those two runs); that is INFERRED from the ASan repro on 09-24.

## 1. The 7 (now 8) Audio-DNA .ips files

`ls ~/Library/Logs/DiagnosticReports | grep Audio-DNA` (VERIFIED). Parsed with `SP/parse.py`, `parse2.py`, `parse3.py`, `parse4.py` (line 1 JSON header, rest JSON body).
**An 8th report appeared after the task was written: `Audio-DNA-2026-09-29-232714.ips`** (pid 21091, procLaunch 23:26:54.5, capture 23:27:07.6). It is a TSan smoke build (`libclang_rt.tsan_osx_dynamic.dylib`, path /private/tmp/*/Audio-DNA.app) aborting in `__tsan::finalize` on quit (`NSApplication terminate:` -> `exit` -> tsan Die). Its sibling log `SP/prebuild/smoke-tsan/tsan.21091` has one report, a `std::ostreambuf_iterator` data race at `pad_and_output.h:54` (iostream logging). Not a heap signature; it was created by a sibling lane, not part of the heap set.

### 1.1 Header table (VERIFIED)

| report | procLaunch -> captureTime | dt | binary uuid | signal / site | procRole, coalition, parent | asi |
|---|---|---|---|---|---|---|
| 09-23 211737 | 21:17:23.874 -> 21:17:25.687 | 1.81 s | 19b2a171 | SIGSEGV KERN_PROTECTION_FAILURE 0x16f4f5c90 (pc == fault addr, inside thread 0's stack region) on HAL queue thread 1 | Foreground, com.audiodna.app, launchd | none |
| 09-23 211744 | 21:17:43.664 -> 21:17:44.606 | 0.94 s | 19b2a171 | SIGABRT malloc checksum in `opendir` under CMIO plug-in registration | Background, com.mitchellh.ghostty (responsible), launchd | `abort() called` |
| 09-23 211855 | 21:18:53.895 -> 21:18:54.942 | 1.05 s | 19b2a171 | SIGABRT malloc checksum in `free_small` <- CGRegionDeallocate, after MainComponent ctor returned | Background, ghostty, parent zsh | `abort() called` |
| 09-24 124843 | 12:48:34.601 -> 12:48:36.220 | 1.62 s | ca883857 | SIGABRT malloc checksum in `small_malloc_from_free_list` <- CFBasicHashRehash <- CFBinaryPlist... <- HIToolbox TSM, from `NSApplication run` | Foreground, com.audiodna.app, launchd | `abort() called` |
| 09-29 003404 | 00:33:23.181 -> 00:33:53.821 | 30.6 s | f65e8693 | SIGABRT libswscale in `Renderer::openVideoForClip` (known broken-file test). NOT heap. | Background | `abort() called` |
| 09-29 205817 | 20:58:15.147 -> 20:58:15.567 | 0.42 s | c8730a61 | SIGABRT malloc checksum in `free_small` <- CFBasicHashRehash <- CoreMedia `sbufAtom_InitializeKnownKeys` (pthread_once) <- CMIOExtension stream format <- CMIO plug-in init | Background, com.audiodna.app (coalition 30098), launchd | `abort() called` |
| 09-29 205919 | 20:59:17.879 -> 20:59:18.874 | 0.99 s | c8730a61 | SIGABRT malloc checksum in `small_malloc_from_free_list` <- `__CFStrAllocateMutableContents` <- NSString format <- CMIOExtensionPropertyAttributes <- CMIO plug-in init | Background, coalition 30098, launchd | `abort() called` |
| 09-29 232714 | 23:26:54.498 -> 23:27:07.622 | 13.1 s | (tsan build) | TSan finalize abort at quit. NOT heap. | Background | `abort() called` |

Header timestamps in the .ips (20:58:17, 20:59:19) are file-write time, ~2 s after capture. Crash reports carry no `vmRegionInfo` for the heap ones; `asi` holds only `abort() called`. The malloc message text (object address, "Corrupt value") is NOT in the .ips but IS in the unified log for the two 09-29 crashes (section 2.3). The `.ips` binary path is redacted (`/Users/USER/*/Audio-DNA.app`); tccd's unified-log line gives the real path for the 09-29 pair: `/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA` (VERIFIED, log 20:58:15.260, `binary_path=`).

The current build binary and its byte copy are both LC_UUID C8730A61, sha256 20b9b48f..., built 20:14:24; the pre-merge copy at `.../gate/pre-vu/Audio-DNA.app` is uuid 3B4F160C (VERIFIED, `dwarfdump --uuid`, `ls -laT`). So the "pre-merge copy vs main build" question is moot for the crashes: the 09-29 crashes ran the main-build path (c8730a61), not the pre-merge copy.

### 1.2 Full faulting-thread stacks (message thread, thread 0), app and system frames

**09-29 205817 (c8730a61), queue `com.apple.cmio.CMIOExtension_PlugIn_Extensions_Update`** (VERIFIED, 73 frames; middle collapsed):
`__pthread_kill / pthread_kill / abort / malloc_vreport / malloc_zone_error / free_list_checksum_botch / small_free_list_remove_ptr_no_clear / free_small / __CFBasicHashRehash / __CFBasicHashAddValue / CFBasicHashAddValue / CFDictionaryAddValue / CoreMedia sbufAtom_InitializeKnownKeys / __pthread_once_handler / _os_once / pthread_once / FigRemote_CreateFormatDescriptionFromSerializedAtomDataBlockBuffer / FigXPCMessageCopyFormatDescription2 / -[CMIOExtensionStreamFormat initWithXPCDictionary:] / +copyFormatsFromXPCArray: / xpc_array_apply / -[CMIOExtensionPropertyAttributes initWithXPCDictionary:] / -[CMIOExtensionPropertyState initWithXPCDictionary:] / +copyPropertyStatesFromXPCDictionary: / xpc_dictionary_apply / -[CMIOExtensionProviderHostContext pluginStates:] / -[CMIOExtensionSessionProvider initWithEndpoint:delegate:] / -[CMIODALExtensionPlugIn initWithPlugIn:endpoint:] / CMIO::DAL::CMIOExtension::PlugIn::Initialize / -[CMIODALExtensionSession updateStateWithExtensionsArrived:extensionsRemoved:] / dispatch_lane_barrier_sync / -[CMIODALExtensionSession init] / CMIO::DAL::CMIOExtension::PlugIn::InitializeExtensionPlugIns / CMIO::DAL::PlugInManagement::Initialize / CMIO::DAL::System::InitializeDevices / CMIO::DAL::System::CheckOutInstance / CMIOObjectGetPropertyDataSize / +[AVCaptureDALDevice _refreshDevices] / _ensureDeviceList / +[AVCaptureDevice_Tundra _devicesWithAllowIOSMacEnvironment:] / +[AVCaptureDeviceDiscoverySession_Tundra discoverySessionWithDeviceTypes:mediaType:position:]`
then **app frames**: `juce::CameraDevice::Pimpl::getAvailableDevices() +8517060` / `juce::CameraDevice::getAvailableDevices()` / **`MainComponent::refreshCameraList() +85312`** / **`MainComponent::MainComponent(bool,int) +62132`** / `AudioDNAApplication::MainWindow::MainWindow +53864` / `AudioDNAApplication::initialise +53224` / `JUCEApplicationBase::initialiseApp` / `JUCEApplication::initialiseApp` / `JUCEApplicationBase::main` (x2) / `dyld start`.

**09-29 205919 (c8730a61)** (VERIFIED, 71 frames): identical chain from `MainComponent::MainComponent +62132` down, and the same CMIO plug-in init chain up to `CMIOExtensionPropertyAttributes initWithMinValue:maxValue:validValues:readOnly:` -> `NSString initWithFormat:` -> `-[NSArray descriptionWithLocale:indent:]` -> `CFStringAppend` -> `__CFStrAllocateMutableContents` -> `szone_malloc_should_clear` -> `small_malloc_should_clear` -> `small_malloc_from_free_list` -> `small_free_list_remove_ptr_no_clear` -> `free_list_checksum_botch` -> abort. Same ctor offset +62132 in both (same binary).

**09-23 211744 (19b2a171)** (VERIFIED, 43 frames): `small_malloc_from_free_list <- szone_malloc_should_clear <- __opendir_common <- __opendir2 <- _CFIterateDirectory <- _CFBundleGetBundleVersionForURL <- _CFBundleCreate <- CFBundleCreateBundlesFromDirectory <- CMIO::DAL::PlugInManagement::OpenPlugInsInDirectoryURL <- RegisterPlugIns <- Initialize <- InitializeDevices <- CMIOObjectGetPropertyDataSize <- AVCaptureDALDevice _refreshDevices ... discoverySessionWithDeviceTypes <- juce::CameraDevice::Pimpl::getAvailableDevices <- CameraDevice::getAvailableDevices <- MainComponent::refreshCameraList +81840 <- MainComponent::MainComponent +63964 <- MainWindow ctor +56224 <- initialise +55568 <- ... main <- dyld start`.

**09-23 211855 (19b2a171)** (VERIFIED, 39 frames): `free_small <- __CGRegionDeallocate <- _CFRelease <- AppKit (2 unsymbolicated) <- -[NSWindow _setNeedsDisplayInRect:] <- -[NSView setNeedsDisplayInRect:] <- -[NSButton updateCell:] <- -[NSThemeFrame _updateButtons] <- -[NSWindow _cacheAndSetPropertiesForCollectionBehavior:] <- _effectiveCollectionBehavior <- canEnterFullScreenMode <- showsFullScreenButton <- -[NSThemeFrame _updateButtons] <- setRepresentedURL: <- _updateTitleProperties:animated: <- -[NSFrameView initWithFrame:styleMask:owner:] <- -[NSThemeFrame initWithFrame:...] <- -[NSWindow _commonInitFrame:...] <- -[NSWindow initWithContentRect:styleMask:backing:defer:] <- juce::NSViewComponentPeer::NSViewComponentPeer <- Component::createNewPeer <- Component::addToDesktop <- TopLevelWindow::setUsingNativeTitleBar <- MainWindow::MainWindow +56236 <- initialise <- ...`. MainComponent ctor had already returned.

**09-24 124843 (ca883857)** (VERIFIED, 40 frames): `small_malloc_from_free_list <- szone_malloc_should_clear <- __CFBasicHashRehash <- CFDictionarySetValue <- __CFBinaryPlistCreateObjectFiltered (x3) <- _CFPropertyListCreateFiltered <- CFBundleGetLocalInfoDictionary <- CFBundleGetValueForInfoDictionaryKey <- HIToolbox islPopulateKLPropertiesFromBundle <- islSetKLPropertyForInputSource <- islGetInputSourceProperty <- TSMGetInputSourceProperty <- _CreateKeyboardInputSourcesArray <- UpdateSourceIndicatorMode <- InitTSMFirstEventTime <- _FirstEventTime <- RunCurrentEventLoopInMode <- ReceiveNextEventCommon <- _DPSNextEvent <- -[NSApplication run] <- juce::JUCEApplicationBase::main() <- main <- dyld start`. No app frame except `main`; the run loop was already running.

**09-23 211737 (the SIGSEGV, not the heap signature)** (VERIFIED): thread 1 (queue `HALC_ShellPlugIn Connection Queue`, triggered): frame #0 = pc **0x16f4f5c90, a stack address** (`threadState.pc.matchesCrashFrame = 1`, `lr` in the app) <- `non-virtual thunk to juce::CoreAudioClasses::AudioIODeviceCombiner::restartAsync() +3237936` <- `CoreAudioInternal::deviceListenerProc` <- `HALObject::PropertiesChanged / ObjectsPublishedAndDied` <- `HALC_ShellPlugIn::Defer_AudioObjectsPublishedAndDied`. Thread 3 (queue `HALC_ShellObject_Listener Queue`) is blocked in `juce::CriticalSection::enter()` from the same `restartAsync` thunk. Message thread (thread 0) is at that instant INSIDE app code: `_platform_memmove <- std::vector<std::vector<float>>::assign <- AudioTap::prepare(double,int,int) <- CombinedCallback::audioDeviceAboutToStart <- AudioDeviceManager::addAudioCallback <- AudioEngine::AudioEngine(RingBuffer<float>&) <- MainComponent::MainComponent +59552`. i.e. the HAL was publishing/killing audio objects while the message thread was still starting the device in the AudioEngine ctor. A call through a corrupted/dangling object landing on a stack address (INFERRED: same BT device-churn window, see 2.4; a different manifestation than the malloc abort).

### 1.3 Thread inventory at crash time (VERIFIED, `SP/threads.txt`)

Base set present in ALL five heap reports and the SEGV: `JUCE Timer`, `CVDisplayLink`, `CoreMIDI MIDIInPortThread` (unnamed), `AudioReadAhead`, `AudioTapWriter`, `caulk.messenger.shared:17` and `:high`, several unnamed `start_wqthread` workers, and **two `com.apple.audio.IOThread.client`** (separate input and output CoreAudio devices, JUCE's combiner path).

| report | threads beyond the base set |
|---|---|
| 09-23 211737 | none (14 threads); plus HAL queue threads (the faulting one) |
| 09-23 211744 | none (14 threads): crash is inside the MainComponent ctor at refreshCameraList; no `AnalysisThread`, no HTTP, no media pools yet |
| 09-23 211855 | 28 threads: `AnalysisThread` (in `Thread::sleep`), `httplib` listener + 10 pool workers, `JUCE OSC server`, 2x `FilesBrowserThumbs` |
| 09-24 124843 | 28 threads: same as 211855 plus **`OpenGL Renderer`** actively running `juce::gl::loadFunctions()` (dyld `trieWalk`) via `OpenGLContext::CachedImage::initialiseOnThread` |
| 09-29 205817 | 20 threads: + 3x `ImageDecode`, `MediaPresence`, 2x `MediaOpen`; NO `AnalysisThread`, NO httplib/OSC, NO OpenGL Renderer |
| 09-29 205919 | 19 threads: same as 205817 |

Is any non-message thread INSIDE app code, not parked, at crash time? **No, with one exception.** Every app-code frame on a non-message thread is a park: `WaitableEvent::wait` (TimeSlice/ThreadPool/Timer), `AnalysisThread::run -> Thread::sleep -> nanosleep`, `httplib::Server::listen_internal` / `ThreadPool::worker` (accept/wait), `OSCReceiver::Pimpl::run -> waitForReadiness`. The two IO threads have NO app frames: `semaphore_wait_signal_trap <- caulk::mach::semaphore::wait_signal_or_error <- HALC_ProxyIOContext::IOWorkLoop`, i.e. waiting for the next IO cycle, not inside the JUCE callback. The exception is 09-24's `OpenGL Renderer` thread executing `gl::loadFunctions` (running app code, but allocation-light dlsym walking). The 09-23 SEGV has the message thread running `AudioTap::prepare` (the only report where the message thread itself is in app code other than the ctor call into CMIO).
**Interpretation (INFERRED):** the snapshot is one instant; an IO callback that runs a few microseconds every 20 ms (BT: 320 frames at 16 kHz) is parked ~99.9 % of the time, so "IO thread parked" does not exonerate it. The stacks say the detection site is collateral (5 different call chains: CMIO x3, NSWindow, TSM), consistent with an earlier out-of-bounds write.

What had already started (VERIFIED from threads plus stderr): the audio device and its IO threads (stderr's first line `[AudioEngine] Switched to mic input mode` precedes everything; `IOThread.client` x2 present in every report), AudioTapWriter, AudioReadAhead, JUCE Timer, MIDI. In the 09-29 pair the media pools exist; the analysis/API/OSC/GL threads do not (crash is in the MainComponent ctor, `refreshCameraList`, +62132).

## 2. 2026-09-29 FINAL battery: the 20:58:15 and 20:59:17 launches

### 2.1 Which run, which app, which args (VERIFIED)
- Driver: `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/gate/vupload-final.sh` (`run idle-paint IDLEPAINT_LAUNCHES=5 bash $H/probe-idle-paint.sh $O/idle-paint`, last probe of the battery; battery began 20:15:14, all probes green until idle-paint). Output: `.../gate/vupload-final.out` and `.../gate/final/idle-paint.log` (43 KB), per-launch dirs `.../gate/final/idle-paint/idlepaint.Is9JUU/{i1_idle_card/r1..r3, i2_idle_many16/r1}` each with `out.log`, `err.log`, `idle.json`.
- App = `IDLEPAINT_APP` default = `/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app` = **main build, uuid c8730a61** (log header `app:` line; tccd `binary_path` for pid 1681). `IDLEPAINT_APP_BEFORE` is the same path (so both arms are the same bundle).
- Launch code (`.harmony/probe-idle-paint.py:197-222`): `open -g --stdout <d>/out.log --stderr <d>/err.log <app>`; **no `--env`, no `--args`** (`env=[] test=False` on the log lines). Production mode, default composition/settings, no fixture loaded until `/api/health` answers (the crashed launches died before the REST server existed). Waits up to 60 x 1 s for `/api/health`, +2 s, then checks 1 process and 7070 listener.
- `run_arm` (lines 470-520): after a launch failure it calls `quit_app()` then `no("launch N did not come up")` and returns; `quit_app` = AppleScript quit then poll `ps ucomm` once per second.

### 2.2 Cadence and load (VERIFIED, launchd + probe log)
| event | time |
|---|---|
| i1 r2 (pid 1602) spawned / Quit AppleEvent / launchd `removing child pid/1602` | 20:57:32.128 / 20:58:13.886 / 20:58:14.114 |
| **i1 r3 (pid 1681) spawned by launchd** | **20:58:15.174** (gap 1.06 s after r2 was removed) |
| pid 1681 crash captured / ReportCrash ASI / launchd remove | 20:58:15.567 / 20:58:15.921 / 20:58:17.878 |
| probe declares r3 dead (60 s health wait) then `quit_app` (nothing to quit) | ~20:59:17 |
| **i2 r1 (pid 1716) spawned** | **20:59:17.907** (60.03 s after 1681 was removed) |
| pid 1716 crash captured / ReportCrash ASI | 20:59:18.874 / 20:59:18.945 |
| next launch g2 (pid 1837) | 21:00:20.614: built-in devices, clean |

The previous instance was fully gone before each crashing launch (launchd `removing child` precedes the next spawn; also the probe refuses to launch if any `Audio-DNA` ucomm exists). The two crashes share coalition id 30098 only because LaunchServices reuses the same launchd job label `application.com.audiodna.app.66438198.208594300` for every instance; this is NOT evidence of overlap (r2's label is identical, VERIFIED in the launchd log).
Machine load (from the probe log): 1-min load 4.1 before r3 (`load 4.11 4.48 5.03` at r2's end), 2.3 then 1.6 at the i2 header; `compilers()` == 0 (the probe refuses to launch otherwise). Battery load elsewhere was 4 to 12 with no crash. **Load and cadence do not discriminate**: across all 661 inter-launch gaps today the median gap is 1.24 s and 318 gaps were under 1.2 s (`SP/spawn.py`), i.e. the 1.06 s gap before pid 1681 is typical of clean launches.

### 2.3 The decisive fact: what the crashing launches did with audio (VERIFIED, unified log)
`/usr/bin/log show --start ... --predicate 'process == "Audio-DNA" OR process == "coreaudiod" ...'`:
- 20:57:55.712 `bluetoothd`: `ACL connected: 34:09:C9:F5:E9:D6` (the earbuds connect INTO the Mac; name `soundcore P31i`, `DvT Headset`), A2DP/AVRCP/HFP up by 20:57:56.7. r2 had been launched at 20:57:32 on `BuiltInMicrophoneDevice`/`BuiltInSpeakerDevice` and kept using them.
- 20:58:15.431 pid 1681: `setPlayState Started Input {34-09-C9-F5-E9-D6:input}`; 15.433 `Output {34-09-C9-F5-E9-D6:output}`; coreaudiod: `TriggerSCOAudio: Request eSCO creation`, `HFPStereo set ... SCOIsEnabled ... sample rate 16000.000000`, `Created a new in process converter ... 1 ch, 16000 Hz`.
- 15.460 to 15.494 IO stopped (JUCE reopen), then **15.538 `Negotiated Frame Size, HAL Request : 512 BTHAL Proposed 320`** (twice), 15.547 input IO restarted (`Injecting silent Audio started`), crash captured 15.567, i.e. within ~20 ms of the last IO restart (1 to 2 IO cycles).
- pid 1716: same: `Negotiated Frame Size, HAL Request : 512 BTHAL Proposed 320` (x6 between 18.674 and 18.840), `Setting sample rate 44100 -> 16000`, `Started Input/Output {34-09-C9-F5-E9-D6:...}` at 18.731/18.733 and again 18.850/18.858, ReportCrash `Parsing corpse` 18.890.
- ReportCrash ASI (VERIFIED, `log show --predicate 'process == "ReportCrash" AND eventMessage CONTAINS "ASI"'`): pid 1681 `malloc: Incorrect checksum for freed object 0x124040e10: probably modified after being freed. Corrupt value: 0x0`; pid 1716 `... object 0x12e028a00 ... Corrupt value: 0x0`. A zero written over a free-list header; 0x0 float = silence (BT SCO is being primed with "silent audio").
- 20:59:21.3 headset audio torn down; 20:59:26 to 20:59:30 ControlCenter activity, then `bluetoothd` `"A2DP Source" profile is disconnecting device` at 20:59:30.572 (INFERRED: the human disconnected the headset from Control Center). `system_profiler SPBluetoothDataType` now: controller State Off.

Mechanism as read in the build's own source (VERIFIED): `build/_deps/juce-src/modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp` (JUCE 8.0.4, `JUCE_BUILDNUMBER 4`): `reopen()` :645-684 sets the device buffer frame size, calls `updateDetailsFromDevice` (:673 -> :504 `bufferSize = X` and :512 `allocateTempBuffers()` = channels x (X+4) floats, :356-361), THEN :675 `bufferSize = bufferSizeSamples` (Y=512) with no reallocation; `audioCallback` :793-797 copies `bufferSize` floats per input channel into `tempBuffers[i]`. With X=320 (the value the HAL proposed) a 1-channel input block is 324 floats = 1296 bytes (small zone) and every callback writes 188 floats (752 bytes) of zeros past it. Same 1296-byte block and `:795` in the 09-24 ASan log `.harmony/.reports/s-rta-0924b/asan-bt-startup-crash.log` (which also shows the writer thread is the HAL `IOThread` and the allocation is in the main thread's `MainComponent::MainComponent` -> `AudioEngine::setSourceMode` -> `setAudioDeviceSetup`). Fix upstream in JUCE 8.0.8 (`f6df3e3`, per `bt-crash-juce-history.md`). So the "concurrent writer" in Harmony's reading is the IO thread of the combiner's INPUT device, live before `refreshCameraList` runs, and detection at random sites is expected.

### 2.4 Bluetooth on the other crash dates (VERIFIED in work logs; system state itself not recoverable, macOS log rotated)
- 09-23 21:17 to 21:19: `.harmony/s-rta-0923-work.md:18` "3/4 launches ... Default in+out = Bluetooth soundcore P31i, 16 kHz (HFP)"; the 9 clean launches at 21:42 to 21:45 ran only after the earbuds were off (`s-rta-0923-startup-crash-diagnosis.md` section 0, which itself notes the trigger was absent, so "0/9" was uninformative).
- 09-24 12:48: `.harmony/s-rta-0924-work.md:74` "merged main SIGABRT at launch ... soundcore P31i default @16k"; :75 the ASan run with the headset connected reproduced the `:795` write; :77 "BORIS RULING: never Bluetooth audio" (also `binding-decisions.md:485`).
So all 5 heap crashes (and, by INFERENCE, the 09-23 SEGV in `restartAsync`, which sits in the HAL device-publish path during the same window) coincide with the P31i as default I/O. The task's statement that the trigger was absent on 09-24 and 09-29 is not supported by the record.

### 2.5 stderr right before the crashes (VERIFIED)
`.../idle-paint/idlepaint.Is9JUU/i1_idle_card/r3/err.log` and `.../i2_idle_many16/r1/err.log` are both 41 bytes: `[AudioEngine] Switched to mic input mode` and nothing else. Surviving launches (e.g. `i1_idle_card/r2/err.log`) print next `WARNING: AVCaptureDeviceTypeExternal is deprecated ...` (that is `refreshCameraList`), then `[MilkDrop] Loaded 30 presets`, `[Analysis] source rate 48000 Hz -> 48 kHz path`, `[API] HTTP server listening on 127.0.0.1:7070`, ... So the app died between `AudioEngine` start and the end of the camera enumeration, consistent with the stacks. (The 09-23 logs that carried the "device sample rate is 16000 Hz" warning are gone; /tmp was cleared by the 09-26 12:41 boot, `sysctl kern.boottime`.)

## 3. Base rate, launch counts, clustering

### 3.1 Counts (numbers and their sources)
- **Unified log, 09-29 12:40 to 23:25 (VERIFIED, `SP/launchd-spawn.txt`, `SP/playstate.txt`, `SP/spawns.json`)**: 663 LaunchServices spawns of `Audio-DNA` (`Successfully spawned Audio-DNA[pid]`), 662 with a matching `removing child` (the last one is the running app). Lifetime under 3 s: exactly two (pid 1681 2.7 s, pid 1716 1.4 s). Per-pid audio device from `setPlayState Started Input/Output`: **663/663 pids have a record; 661 = BuiltInMicrophoneDevice + BuiltInSpeakerDevice, 2 = the P31i (pids 1681, 1716)**. The log store starts about 12:40 (`/var/db/diagnostics/Persist` oldest `.tracev3` is 09-29 12:48), so earlier days cannot be counted this way.
  Launches per hour (12h..21h): 35, 65, 35, 86, 82, 79, 67, 93, 61, 60.
- **Stderr archives, 09-26 to 09-29 (VERIFIED count method, ~+/-15 %, `SP/errscan.py`)**: files named err.log / app-err.log / *-err.log / err.* / stderr* under `/private/tmp` containing `[AudioEngine] Switched to mic input mode` (one per launch; birth-day buckets): 09-26 174, 09-27 638, 09-28 522, 09-29 1392, total 2726. Only two of them stop before the `[API] HTTP server listening` line in a single-launch file: the two crash logs above (three multi-launch appended files show more AudioEngine lines than API lines, `adna-step3-err.log`, `adna-step3row-err.log`, `.../cfd08720.../tempo/reruns/step3/app-err.log`; no crash report exists for any of those days, so I treat them as clean quits or test-mode cases). Over-count risks: direct-exec test launches and worktree/ASan/TSan builds are included; under-count: launches whose stderr went elsewhere.
- **09-23, 09-24, 09-25: not recoverable** (`/tmp` wiped by the 09-26 12:41 boot; no `.ips` for 09-25 to 09-28). Anecdotal from work logs: 09-23 4 launches with the headset (3 crashed) + 9 without (0 crashed); 09-24 12:48 one launch (crashed) + the ASan reproduction runs.
- **Binary c8730a61 only** (built 20:14:24, VERIFIED): 115 launches from 20:14:30 to 23:25 (launchd), 2 crashes, i.e. the 54th launch and the 55th-ish; the 113 built-in launches were clean.
- No other app's `.ips` contains `checksum_botch`: taken as given from the task, not re-checked.

### 3.2 Rates (Clopper-Pearson 95 %, computed in python)
| population | crashes / launches | rate | 95 % CI |
|---|---|---|---|
| all launches 09-29 12:40 to 23:25 | 2 / 663 | 0.30 % | 0.04 % to 1.1 % |
| ... built-in audio only | 0 / 661 | 0 % | upper 0.56 % |
| ... Bluetooth default I/O | 2 / 2 | 100 % | 16 % to 100 % |
| 09-26..09-29 stderr archive (all audio states, undercounts BT) | 2 / ~2726 | 0.07 % | 0.009 % to 0.27 % |
| BT-connected launches, all dates incl. 09-23/24 work-log counts (INFERRED: 09-23 3/4, 09-24 1/1, 09-29 2/2) | 6 / 7 | 86 % | 42 % to 99.6 % |

**Launches per arm needed to see >= 1 crash with 95 % probability, N = ceil(ln 0.05 / ln(1-p))**: p=0.86 -> 2; p=0.75 -> 3; p=0.5 -> 5; p=0.30 -> 9; p=0.0030 (unconditional today) -> 998; p=0.00073 (stderr archive) -> 4103. Use 99 %: p=0.86 -> 3; p=0.5 -> 7. **Design consequence**: an arm WITHOUT the headset connected will show ~0 crashes at any affordable N (0/661 already); an arm WITH the headset default in/out should crash on nearly every launch, so 5 launches per arm gives >= 95 % if the true rate is >= 45 %, and 3 launches if it is >= 63 %.

### 3.3 Clustering (VERIFIED where stated)
- **By Bluetooth window: yes, fully.** Today the only headset connection (20:57:55 to 20:59:30, 94 s, `SP/btdev.txt`) contains exactly the two launches that opened it, and both crashed; no other launch in 12 h of 661 did. 09-23 21:17:23, 21:17:43, 21:18:53 (3 crashes in 90 s) is the same shape (INFERRED: one headset window).
- By time of day: no evidence (crashes at 21:17, 12:48, 20:58, 20:59; launch volume peaks 19:00 with zero crashes).
- By machine load: no (load 1.6 to 4.1 at the crashes; loads up to 12 elsewhere without a crash).
- By launch cadence: no (gap 1.06 s before the first, 60 s before the second; median gap for clean launches 1.24 s).
- Test-mode vs production: both 09-29 crashes were production launches (`test=False`, no env). The launch mode is NOT the discriminator: the same production `open -g` path ran 661 times cleanly on built-in audio.
- By app version: crashes span three binaries (19b2a171, ca883857, c8730a61) over 6 days with the same JUCE 8.0.4 CoreAudio code, so it is not tied to a recent app change (INFERRED; the overflow lives in JUCE, not in files touched by s-rta-0929).

## 4. Risks and gaps in this analysis
1. **The 09-29 writer is inferred, not observed.** Nobody ran ASan on the two 09-29 launches; the match is: same device, same 512/320 mismatch (logged), same zero-valued corruption, same 1296-byte small-zone class of block, same source lines. Discriminating test (needs the headset default I/O, which Boris's ruling forbids without his say-so): launch the current build 3x with the P31i connected under ASan (expect `:795` write) and 3x with a JUCE 8.0.8 `CoreAudioInternal` patch or an app-level guard that refuses BT devices (expect 0/3). A no-Bluetooth reproduction would need a device that refuses the requested buffer size (any device with Y > X+4).
2. 6/7 is a small-sample composite; the 09-24 launch count and the 09-23 4th launch are prose in work logs, not machine records.
3. The launch counts under-cover 09-23 to 09-25 (no artifacts) and the stderr-archive count is +/-15 %; the unified-log window is only 09-29 12:40 onward.
4. The 09-23 SEGV (`restartAsync`, pc on a stack address) is attributed to the same headset window by inference only; it may be the H2 combiner race (device publish/die during start) rather than the overflow.
5. My claim "Bluetooth is not connected now" is from `system_profiler` (Bluetooth controller Off) at 23:2x, not a claim about any other time.
6. A sibling lane created a new .ips (TSan) at 23:27; if the set is re-parsed, exclude 232714 from the heap group.

## 5. Evidence index
- .ips: `~/Library/Logs/DiagnosticReports/Audio-DNA-2026-09-{23-211737,23-211744,23-211855,24-124843,29-003404,29-205817,29-205919,29-232714}.ips`
- Battery: `.../f33bd4ec-.../scratchpad/gate/vupload-final.sh|out`, `.../gate/final/idle-paint.log`, `.../gate/final/idle-paint/idlepaint.Is9JUU/`
- Probe: `/Users/boriskarpman/projects/RealTimeAudio/.harmony/probe-idle-paint.sh`, `.py` (launch 197-222, run_arm 470-520), `.json`
- Prior mechanism: `.harmony/.reports/s-rta-0924b/{asan-bt-startup-crash.log,bt-crash-mechanism.md,bt-crash-juce-history.md,bt-crash-fix-plan.md}`, `.harmony/s-rta-0923-work.md:18-34`, `.harmony/s-rta-0924-work.md:74-77`, `.harmony/binding-decisions.md:485-492`, `.harmony/.reports/s-rta-0923-startup-crash-diagnosis.md`
- JUCE source: `/Users/boriskarpman/projects/RealTimeAudio/build/_deps/juce-src/modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp` (:354-372, :490-516, :645-684, :784-797)
- Unified-log extracts: `SP/launchd-spawn.txt`, `SP/playstate.txt`, `SP/btdev.txt`, `SP/spawns.json`
