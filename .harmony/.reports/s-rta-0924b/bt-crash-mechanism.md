# Bluetooth HFP startup crash — exact mechanism (JUCE 8.0.4 `CoreAudioInternal`)

Architect (Fable) diagnosis, 2026-09-24, s-rta-0924b. Read-only: no source edited, app not launched, no debugger.
Inputs: ASan log `.harmony/.reports/s-rta-0924b/asan-bt-startup-crash.log`; JUCE 8.0.4 source under `build/_deps/juce-src`
(byte-identical to `build-asan/_deps/juce-src`, verified with `cmp`; JUCE git HEAD `51d11a2be JUCE version 8.0.4`);
Researcher report `bt-crash-juce-history.md` (upstream fix history — not re-derived here).
File abbreviations below: `CA` = `build/_deps/juce-src/modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp`,
`ADM` = `build/_deps/juce-src/modules/juce_audio_devices/audio_io/juce_AudioDeviceManager.cpp`, `AE` = `src/audio/AudioEngine.cpp`.

## QUESTION
Which quantity disagrees between `allocateTempBuffers()` and `audioCallback()`, via which code path/thread, and what is the
cheapest experiment that discriminates the candidate explanations?

## ANSWER (mechanism) — VERIFIED from source, deterministic, not a race
**The disagreeing quantity is `CoreAudioInternal::bufferSize` itself.** The temp buffers are sized from the frame size the
device *reports back* after JUCE sets it (call it X); the callback copies the frame count JUCE *requested* (call it Y),
because `reopen()` overwrites `bufferSize` with the requested value AFTER the allocation without re-allocating. If
Y > X + 4 every input callback writes Y − (X + 4) floats past the heap block. Channel count, stride and `channelInfo` are
consistent (same `Stream` object, swapped atomically together with the allocation).

Chain, with cites:
1. `bufferSize` (`CA:1113`) is assigned in exactly two places (grep-verified): `CA:504` inside `updateDetailsFromDevice()`
   — under `callbackLock`, immediately followed by `allocateTempBuffers()` at `CA:512` (consistent) — and `CA:675` inside
   `reopen()`, the "bodge", NOT followed by an allocation and NOT under the lock. (`CA:1543` is a combiner local, unrelated.)
2. `reopen()` (`CA:645-684`): `stop(false)` → `setNominalSampleRate` → `AudioObjectSetPropertyData(kAudioDevicePropertyBufferFrameSize, Y)`
   (`CA:660-663`; treated as success on `noErr`, never re-checked) → `updateDetailsFromDevice(ins,outs)` (`CA:673`), which
   reads X = `getFrameSizeFromDevice()` (`CA:468` → `CA:444-447`), stores `bufferSize = X` (`CA:504`) and allocates
   `audioBuffer` = total_channels × (X + 4) floats (`CA:356-361`) → then `bufferSize = Y` (`CA:675`). JUCE's own comment
   (`CA:669-672`) says some devices "fail to correctly report their new settings until some random time in the future" —
   the code trusts the request for the frame count but leaves the allocation sized by the read-back.
3. `audioCallback()` (`CA:764-845`, HAL IO thread, under `callbackLock`): input copy `for (j = bufferSize …) *dest++ = *src`
   (`CA:793-797`) writes Y floats into `inStream->tempBuffers[i]`, which has only X + 4 floats (`CA:881-889`:
   `tempBuffers[i] = buffer + i*(X+4)`). It never consults `inInputData->mBuffers[].mDataByteSize` (the frames the HAL
   actually delivered). ASan's hit is exactly `CA:795`, thread T19 = the HAL IO thread started by
   `AudioIODeviceCombiner::start` → `CoreAudioInternal::start` → `AudioDeviceStart` (`CA:704-726`).
4. Corollaries (INFERRED, same arithmetic): the *output* device's `CoreAudioInternal` is bodged identically; with
   Y > X_out + 4 the combiner's `outputAudioCallback` (`CA:1853-1896`, `std::fill`/`FloatVectorOperations::copy` of up to Y
   floats into `outStream->tempBuffers[i]`) is a second heap overflow on the output IO thread, and `CA:827-831` writes Y
   frames into the HAL output buffer that holds X_out frames (shared HAL memory — invisible to ASan). The input loop also
   over-READS the HAL input buffer by (Y − X) frames per cycle if the device really runs at X (`CA:788-797`; invisible to
   ASan: HAL IO buffers are not instrumented mallocs). Whichever IO thread fires first is the one ASan reports.

## THE NUMBERS
- ASan: 1296-byte block = 324 floats = total_channels × (X + 4), allocated by `calloc` (`HeapBlock::calloc`, `CA:361`);
  first out-of-bounds write is byte 1296 ("0 bytes after"), i.e. the copy ran off the end of the LAST-laid-out channel
  buffer (the loop at `CA:784` processes channel numInputChans−1 first).
- Channel count of that block: the combiner's input wrapper is `CoreAudioIODevice(name, inputID, 0)` (`CA:2253`) →
  `CoreAudioInternal(…, hasInput=true, hasOutput=false)` (`CA:1232-1235`, `CA:318-319`) → `outStream == nullptr`, so
  total = active input channels = requested {0,1} clipped to `chanNames.size()` (`CA:871-878`). HFP mic = 1 channel
  (task FACT; `system_profiler` today: "soundcore P31i … Input Channels: 1", 16000 Hz) → **X = 320** (= 20 ms @ 16 kHz).
  Enumeration for completeness: 1ch→X=320, 2ch→158, 3ch→104, 4ch→77 — only 320 is a value a device would report.
  (INFERRED-strong; E1/E2 below print the channel count and X directly.)
- Y: any Y ≥ 325 produces the identical ASan report, so ASan does not bound Y. Trace (VERIFIED code path, value INFERRED):
  `initialiseWithDefaultDevices(2,2)` (`AE:14`) → `initialiseDefault` builds a default `AudioDeviceSetup{sampleRate=0,
  bufferSize=0}` (`ADM:328`, header defaults) → `setAudioDeviceSetup` → `chooseBestBufferSize(0)` (`ADM:806`, `871-879`)
  → `getDefaultBufferSize()` = combiner `jmax(input default, output default)` (`CA:1520-1523`), each = first entry ≥ 512 of
  that device's `bufferSizes` list, else its largest (`CA:1278-1289`; list built at `CA:409-440`: min rounded to 16, then
  32..2048 step 32 inside the reported range, plus the current member value). The P31i OUTPUT is a *separate* CoreAudio
  device, 2 ch @ 44100 (system_profiler today) — an A2DP-style output whose range almost certainly reaches 512 (built-in
  devices measured today: range [15..4096], default 512). So **Y = 512** even if the HFP input can only do 320. Overflow =
  188 floats = 752 bytes per callback — enough to smash several neighbouring small-zone chunks (the very next allocation is
  `tempBuffers.calloc(channels+2)`, 24 bytes, `CA:883`), which is the classic source of the non-ASan signatures
  (`free_list_checksum_botch` on the message thread; SIGSEGV in `restartAsync` on the HAL listener queue). INFERRED.
- Nothing ever reports X to the app: `getCurrentBufferSizeSamples()` returns the bodged Y (`CA:762`, `CA:1273`), the
  combiner's `currentBufferSize` is the request (`CA:1554`, `1513`), and `ADM::updateCurrentSetup` (`ADM:182-191`) stores it.
  Worse, `getBufferSizesFromDevice()` adds the current member value to the list (`CA:432-433`), so once bodged, 512 stays
  "available" forever and `chooseBestBufferSize(512)` keeps returning 512 (self-perpetuating).

## PATH / THREAD / WINDOW
- Both opens run on the message thread inside `MainComponent`'s constructor: `audioEngine_` is a member
  (`src/MainComponent.h:253`) → open #1 (`AE:14`) during member init; open #2 at `src/MainComponent.cpp:261` →
  `AE::setSourceMode` (`AE:108-110`) — only ~90 lines of pure UI setup in between, no message-loop pumping.
- Why open #2 is a *full* stop/close/reopen/start and not the early return at `ADM:735-738`: after open #1,
  `updateCurrentSetup` stored `inputChannels = {0}` (mono active set); `setSourceMode` sets `{0,1}` (`AE:109`) → setups differ.
  (Side finding: `AE:121 setup.inputChannels.clear()` in File mode is ineffective — `useDefaultInputChannels == true` makes
  `ADM:795 updateSetupChannels` re-enable {0,1}; every `setSourceMode` in either mode is a full device restart that re-runs
  the vulnerable `reopen()`.)
- Sequence of open #2: `ADM:740 stopDevice` → combiner `stop` → wrapper `stop` → `CA:1410 internal->stop(true)`
  (callback=nullptr, proc left running: callback now only zeroes output, `CA:835-840`) → `ADM:808 open` → combiner `open`
  → `close` → `CA:1318 internal->stop(false)` (sets `audioDeviceStopPending`, waits ≤2 s for the IO thread to call
  `AudioDeviceStop`, `CA:740-756`; a callback in this window returns at `CA:771-777` without copying) → per-device
  `reopen()` (set Y, read X, alloc X+4, bodge Y) → `ADM:817 start` → `CA:1682 d->start` → `AudioDeviceStart` → first real
  IO cycle → overflow. **Not a window: a persistent state** from `reopen()` return until the next
  `updateDetailsFromDevice()`; every callback in it overflows. No data race is involved (`CA:504` and `CA:769` both take
  `callbackLock`; `CA:675` runs with the device stopped).
- Why the ASan allocation stack points at open #2 and not open #1: open #1 was torn down within milliseconds, before a
  Bluetooth HFP device can deliver its first IO cycle (SCO link set-up), and the two stop phases above cannot copy. So the
  evidence is fully consistent with the mismatch existing at BOTH opens — E1/E2 log both. Removing the startup
  `setSourceMode` call would therefore only move the crash a few hundred ms later. (INFERRED.)
- The only self-healing path — `deviceListenerProc` (`CA:1150-1197`) on BufferFrameSize/NominalSampleRate/StreamFormat/…
  → `startTimer(100)` → `timerCallback` (`CA:1118-1130`) → consistent re-alloc with `bufferSize = X`, then
  `owner.restart()` → combiner `restartAsync` → `restart()` (`CA:1591-1635`) re-opens with the device's own X if it is in
  the common list — needs a property-change notification AND a running message loop before the first IO cycle. At startup
  the loop is blocked in the constructor, so it cannot run in time. (Note: this listener→`restartAsync` chain is the exact
  frame of the s-rta-0923 Crash A SIGSEGV — it fires on the HAL listener queue against already-corrupted heap.)
- Sample-rate side note (not the mechanism): the combiner opens BOTH devices at one common rate (`CA:1553-1561`,
  `chooseBestSampleRate` starts from the input's 16000, `ADM:850`); it also re-sets the other device's nominal rate in
  `handleAudioDeviceAboutToStart` (`CA:1937-1944`). Forcing the A2DP output to 16 kHz is what flips the headset's profile
  and generates the property-change storm. E1/E2 print the rates so this is recorded, but it does not change the overflow.

## RULED OUT (and why)
- Channel-count change between alloc and callback: `inStream` (channels, channelInfo, tempBuffers) and the allocation are
  swapped as one unit under `callbackLock` (`CA:496-513`); a new `Stream` always brings a fresh allocation. A count mismatch
  could not produce this report anyway: `tempBuffers` has channels+2 calloc'd (null) slots (`CA:883`) → a null-pointer
  write near address 0, not "0 bytes after a 1296-byte region".
- Stride/`dataOffsetSamples` mismatch: `dest` advances by 1 per frame regardless of stride (`CA:795-796`) — that would
  over-read the HAL buffer, never over-write JUCE's heap.
- Sample-rate change altering frames-per-cycle: IO frame size is in frames, rate-independent; a device-side frame-size
  change fires the listener path above, which re-allocates consistently.
- App code (AudioTap/CombinedCallback, s-rta-0923 H1): the writing thread has zero app frames; the app callback runs only
  after the overflowing copy (`CA:813-816`). H1 is not needed to explain any of the three signatures.

## OPEN: clamped vs stale (what the experiment decides)
(a) **Range clamp**: HFP input `BufferFrameSizeRange` max < 512; HAL returns `noErr` and clamps (JUCE treats `noErr` as
    success) → X = 320 persistently; device runs at 320 → JUCE heap overflow AND HAL over-read every cycle.
(b) **Stale read-back**: range includes 512, the BT HAL plugin applies asynchronously → read-back 320 for a while, then
    512; device runs at 512 (or 320 first) → heap overflow; HAL over-read only while it still runs at 320. This is the case
    the JUCE forum thread describes (Researcher report).
(c) Device fixed at 320 for both directions. App-visible outcome of a/b/c is identical; the difference matters for a local
    patch choice, not for the upstream fix (which reads the delivered sizes).
Either way the expected read-back for the ASan numbers is **324 / inputChannels − 4 = 320**.

## EXPERIMENTS (exact steps; Harmony runs them — headset must be default in AND out)
Pre-flight for both: `system_profiler SPAudioDataType | grep -B1 -A6 "soundcore"` must show P31i as Default Input
(1 ch, 16000) and Default Output. VERIFIED today that this state is volatile: at ~13:05 it was default in+out; minutes later
the device had left the bus entirely (probe saw only Little Bot Microphone / MacBook Pro Microphone / MacBook Pro Speakers).
This also explains the 0-crash s-rta-0923 run. Re-check immediately before each run.

### E1 — cheapest and most discriminating (~2 min, no app, no JUCE, no rebuild)
A ~120-line CoreAudio probe: dump the default input device using JUCE's exact property addresses, set BufferFrameSize=512
the way `reopen()` does (`{BufferFrameSize, ScopeGlobal, ElementMain}`, `CA:660-663`), read back with JUCE's read address
(`{BufferFrameSize, ScopeWildcard, ElementMain}`, `CA:444-447`) at +0/+50 ms/+500 ms/+2 s, install a wildcard property
listener (timestamps any BufferFrameSize/NominalSampleRate/StreamConfiguration/DeviceHasChanged notification), then start an
IOProc for 20 cycles and print `mBuffers[0].mDataByteSize / (4 × mNumberChannels)` (frames actually delivered) plus the
property value read from inside the callback. Run once for input and once for output (`output` arg), and once with 320.
The read-only half of this probe was compiled and run today (built-in devices, numbers above); the SET+IOProc half is
deliberately not run from this session (it starts the mic).
Build & run (from a terminal that already has microphone permission — the one the ASan app ran from):
```
clang++ -std=c++17 -framework CoreAudio -framework CoreFoundation ca_probe_set.cpp -o /tmp/ca_probe_set
/tmp/ca_probe_set 512 input | tee /tmp/e1-in-512.log ; /tmp/ca_probe_set 512 output | tee /tmp/e1-out-512.log
```
Source (`ca_probe_set.cpp`):
```cpp
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach_time.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
static const AudioObjectPropertyElement kMain = kAudioObjectPropertyElementMain;
static double t0; static double nowMs(){ static mach_timebase_info_data_t tb; if(!tb.denom) mach_timebase_info(&tb);
    return mach_absolute_time()*(double)tb.numer/tb.denom/1e6; }
static UInt32 getU32(AudioObjectID id, AudioObjectPropertySelector s, AudioObjectPropertyScope sc){ AudioObjectPropertyAddress a{s,sc,kMain};
    UInt32 v=0, sz=sizeof v; AudioObjectGetPropertyData(id,&a,0,nullptr,&sz,&v); return v; }
static Float64 getF64(AudioObjectID id, AudioObjectPropertySelector s, AudioObjectPropertyScope sc){ AudioObjectPropertyAddress a{s,sc,kMain};
    Float64 v=0; UInt32 sz=sizeof v; AudioObjectGetPropertyData(id,&a,0,nullptr,&sz,&v); return v; }
static UInt32 frameSize(AudioObjectID id){ return getU32(id,kAudioDevicePropertyBufferFrameSize,kAudioObjectPropertyScopeWildcard); } // JUCE CA:444-447
static void printRange(AudioObjectID id){ AudioObjectPropertyAddress a{kAudioDevicePropertyBufferFrameSizeRange,kAudioObjectPropertyScopeWildcard,kMain};
    AudioValueRange r{}; UInt32 sz=sizeof r; if(AudioObjectGetPropertyData(id,&a,0,nullptr,&sz,&r)==noErr)
    printf("[%.1f ms] BufferFrameSizeRange [%.0f..%.0f]\n",nowMs()-t0,r.mMinimum,r.mMaximum); }
static int channels(AudioObjectID id, bool input){ AudioObjectPropertyAddress a{kAudioDevicePropertyStreamConfiguration,
    input?kAudioDevicePropertyScopeInput:kAudioDevicePropertyScopeOutput,kMain}; UInt32 sz=0; if(AudioObjectGetPropertyDataSize(id,&a,0,nullptr,&sz)!=noErr) return 0;
    char* raw=(char*)calloc(1,sz); auto* bl=(AudioBufferList*)raw; int n=0; if(AudioObjectGetPropertyData(id,&a,0,nullptr,&sz,bl)==noErr)
    for(UInt32 i=0;i<bl->mNumberBuffers;++i) n+=bl->mBuffers[i].mNumberChannels; free(raw); return n; }
static std::atomic<int> cycles{0}; static bool gInput=true; static const int kMax=20;
static OSStatus ioProc(AudioObjectID dev,const AudioTimeStamp*,const AudioBufferList* in,const AudioTimeStamp*,AudioBufferList* out,const AudioTimeStamp*,void*){
    int c=cycles.fetch_add(1); const AudioBufferList* bl=gInput?in:out;
    if(c<kMax){ UInt32 nb=bl?bl->mNumberBuffers:0, by=nb?bl->mBuffers[0].mDataByteSize:0, ch=nb?bl->mBuffers[0].mNumberChannels:0;
        printf("[%.1f ms] IO cycle %2d: buffers=%u bytes0=%u ch0=%u => FRAMES DELIVERED=%u | property BufferFrameSize now=%u\n",
               nowMs()-t0,c,nb,by,ch,ch?by/(4*ch):0,frameSize(dev)); }
    if(out) for(UInt32 i=0;i<out->mNumberBuffers;++i) memset(out->mBuffers[i].mData,0,out->mBuffers[i].mDataByteSize);
    return noErr; }
static OSStatus listener(AudioObjectID dev,UInt32 n,const AudioObjectPropertyAddress* pa,void*){
    for(UInt32 i=0;i<n;++i){ UInt32 s=pa[i].mSelector, sc=pa[i].mScope;
        printf("[%.1f ms] PROPERTY CHANGED '%c%c%c%c' scope '%c%c%c%c' -> BufferFrameSize=%u nominalRate=%.0f inCh=%d outCh=%d\n",nowMs()-t0,
               (int)(s>>24),(int)(s>>16),(int)(s>>8),(int)s,(int)(sc>>24),(int)(sc>>16),(int)(sc>>8),(int)sc,
               frameSize(dev),getF64(dev,kAudioDevicePropertyNominalSampleRate,kAudioObjectPropertyScopeGlobal),channels(dev,true),channels(dev,false)); }
    return noErr; }
int main(int argc,char** argv){
    UInt32 req=argc>1?(UInt32)atoi(argv[1]):512; gInput=!(argc>2&&!strcmp(argv[2],"output")); t0=nowMs();
    AudioObjectPropertyAddress da{gInput?kAudioHardwarePropertyDefaultInputDevice:kAudioHardwarePropertyDefaultOutputDevice,kAudioObjectPropertyScopeGlobal,kMain};
    AudioObjectID dev=0; UInt32 sz=sizeof dev; AudioObjectGetPropertyData(kAudioObjectSystemObject,&da,0,nullptr,&sz,&dev);
    CFStringRef name=nullptr; AudioObjectPropertyAddress na{kAudioDevicePropertyDeviceNameCFString,kAudioObjectPropertyScopeGlobal,kMain}; sz=sizeof name;
    AudioObjectGetPropertyData(dev,&na,0,nullptr,&sz,&name); char nb[128]="?"; if(name){ CFStringGetCString(name,nb,sizeof nb,kCFStringEncodingUTF8); CFRelease(name); }
    printf("device %u '%s' (%s)  nominalRate=%.0f  inCh=%d outCh=%d\n",dev,nb,gInput?"default INPUT":"default OUTPUT",
           getF64(dev,kAudioDevicePropertyNominalSampleRate,kAudioObjectPropertyScopeGlobal),channels(dev,true),channels(dev,false));
    printRange(dev); printf("[%.1f ms] BufferFrameSize BEFORE set: %u\n",nowMs()-t0,frameSize(dev));
    AudioObjectPropertyAddress wild{kAudioObjectPropertySelectorWildcard,kAudioObjectPropertyScopeWildcard,kAudioObjectPropertyElementWildcard};
    AudioObjectAddPropertyListener(dev,&wild,listener,nullptr);
    AudioObjectPropertyAddress bfs{kAudioDevicePropertyBufferFrameSize,kAudioObjectPropertyScopeGlobal,kMain}; // JUCE reopen() CA:660-663
    Boolean settable=0; AudioObjectIsPropertySettable(dev,&bfs,&settable);
    OSStatus st=AudioObjectSetPropertyData(dev,&bfs,0,nullptr,sizeof req,&req);
    printf("[%.1f ms] SET BufferFrameSize=%u: settable=%d status=%d(%s) readBack immediately=%u\n",nowMs()-t0,req,(int)settable,(int)st,st==noErr?"noErr":"ERROR",frameSize(dev));
    for(int ms:{50,500,2000}){ usleep(ms*1000); printf("[%.1f ms] readBack: %u\n",nowMs()-t0,frameSize(dev)); }
    AudioDeviceIOProcID pid=nullptr; st=AudioDeviceCreateIOProcID(dev,ioProc,nullptr,&pid); printf("CreateIOProcID status=%d\n",(int)st);
    st=AudioDeviceStart(dev,pid); printf("[%.1f ms] AudioDeviceStart status=%d\n",nowMs()-t0,(int)st);
    for(int i=0;i<200&&cycles.load()<kMax;++i) usleep(50*1000);
    AudioDeviceStop(dev,pid); AudioDeviceDestroyIOProcID(dev,pid);
    printf("[%.1f ms] stopped after %d cycles; final BufferFrameSize=%u nominalRate=%.0f\n",nowMs()-t0,cycles.load(),frameSize(dev),
           getF64(dev,kAudioDevicePropertyNominalSampleRate,kAudioObjectPropertyScopeGlobal));
    AudioObjectRemovePropertyListener(dev,&wild,listener,nullptr); return 0; }
```
Reading the output:
| Observation (input device, req=512) | Verdict |
|---|---|
| readBack 320 at every timestamp AND frames delivered = 320 | (a)/(c) clamp or fixed; JUCE alloc 324, copies 512 → 752-byte heap overflow + HAL over-read every cycle |
| readBack 320 at +0 (and/or +50 ms) then 512; delivered 512 (maybe 320 first) | (b) stale read-back — the JUCE-forum case; heap overflow on every cycle, HAL over-read only while at 320 |
| readBack 512 immediately and delivered 512 | mechanism as derived would NOT fire with the device in this state → the app's open must have caught the device mid profile-switch (listener lines will show it); run E2 |
| inCh ≠ 1 | recompute X = 324/inCh − 4 (2ch → 158); mechanism unchanged |
Also compare `nominalRate`: if the output probe shows 44100 while the input shows 16000, note that JUCE forces both to one
common rate at open (`CA:1553-1561`) — the profile flip happens under JUCE's feet during open #1/#2.

### E2 — in-situ confirmation on the app's own path (~5-10 min incremental rebuild of the scratch ASan tree)
Patch ONLY `build-asan/_deps/juce-src/modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp` (scratch copy; `build/`
stays clean). Revert afterwards by re-materialising the pristine file from the JUCE 8.0.4 git object:
`cd build-asan/_deps/juce-src && git show HEAD:modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp > modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp`.
Do NOT reconfigure with `--fresh` (would re-populate JUCE). Four insertions:
1. Line 34 (before `namespace juce {`): `#include <cstdio>`
2. In `allocateTempBuffers()` after `CA:361`:
   `dbgTempBufSize = tempBufSize; fprintf (stderr, "[JUCE-DBG alloc] dev=%u tempBufSize=%d totalCh=%d bytes=%zu\n", (unsigned) deviceID, tempBufSize, total, (size_t) total * (size_t) tempBufSize * sizeof (float));`
3. In `reopen()` between `CA:673` (`updateDetailsFromDevice (ins, outs);`) and `CA:674`:
   `fprintf (stderr, "[JUCE-DBG reopen] dev=%u requested=%d readBack=%d inCh=%d outCh=%d rateReq=%.0f rateDev=%.0f\n", (unsigned) deviceID, bufferSizeSamples, bufferSize, getChannels (inStream), getChannels (outStream), newSampleRate, sampleRate);`
   (`bufferSize` here is X because `CA:504` just stored it; the bodge to Y follows at `CA:675`.)
4. In `audioCallback()` as the first statement inside `if (callback != nullptr) {` (`CA:782-783`), i.e. BEFORE the copy loop so it prints even though ASan aborts on the copy:
   `if (dbgCallbackCount < 5) { ++dbgCallbackCount; const auto* bl = numInputChans > 0 ? inInputData : outOutputData; const auto nb = bl != nullptr ? bl->mNumberBuffers : 0u; const auto by = nb ? bl->mBuffers[0].mDataByteSize : 0u; const auto ch = nb ? bl->mBuffers[0].mNumberChannels : 0u; fprintf (stderr, "[JUCE-DBG callback] dev=%u bufferSize=%d inCh=%d outCh=%d tempBufFloatsPerCh=%d halBuffers=%u halBytes0=%u halCh0=%u => halFramesDelivered=%u\n", (unsigned) deviceID, bufferSize, numInputChans, numOutputChans, dbgTempBufSize, nb, by, ch, ch ? by / (4u * ch) : 0u); }`
   plus two members next to `CA:1113`: `int dbgCallbackCount = 0; int dbgTempBufSize = 0;`
Build/run (notebook recipe: raw binary, ASan log_path, wait for exit on its own):
```
cmake --build build-asan --target AudioDNA -j8
ASAN_OPTIONS=halt_on_error=1:detect_leaks=0:log_path=/tmp/asan-e2 \
  ./build-asan/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app/Contents/MacOS/Audio-DNA 2>/tmp/e2-stderr.log ; echo "exit=$?"
grep JUCE-DBG /tmp/e2-stderr.log ; cat /tmp/asan-e2.*
```
Expected if the diagnosis is right: two `reopen` lines per open (input dev and output dev), the input one reading
`requested=512 readBack=320 inCh=1 outCh=0` at open #2 (and very likely at open #1 too), an `alloc … tempBufSize=324
totalCh=1 bytes=1296`, then `callback … bufferSize=512 … tempBufFloatsPerCh=324 … halFramesDelivered=320|512`, then the
ASan report at `CA:795`. `halFramesDelivered` decides clamp (320) vs stale (512) inside the app's own process — this is the
single number E1 cannot give for the app's exact open sequence. Refuting outcome: `readBack == requested` at both opens
with the crash still occurring → the mechanism is elsewhere and this report is wrong (see RISKS).

### E3 — fix validation (not a mechanism discriminator)
Rebuild against JUCE ≥ 8.0.8 (Researcher: commit f6df3e3 "CoreAudio: Respect buffer size passed to audio callback" —
sizes the callback from the delivered buffers and chunks when larger) and rerun the E2 launch with the headset default:
no ASan report and the app alive for 60 s. That the upstream fix covers this exact path is INFERRED from its description
(source not on disk here); E3 is what makes it VERIFIED.

## FIX OPTIONS (decision-level; implementation is a separate lane)
1. **Bump `GIT_TAG 8.0.4` → ≥ 8.0.8** (`CMakeLists.txt:30`) — recommended; audit the two breaking changes the Researcher
   listed (`OpenGLFrameBuffer::readPixels/writePixels` RowOrder param in 8.0.9; `AudioTransportSource::hasStreamFinished`
   semantics in 8.0.5). Handles clamp and stale alike because it uses `mDataByteSize`.
2. If a bump is blocked: local backport via `FetchContent PATCH_COMMAND` — in `audioCallback` clamp the copied frame count
   to `min(bufferSize, framesDelivered, allocatedFramesPerChannel)` (store `tempBufSize` as a member) and pass that count
   to the app callback. Merely calling `allocateTempBuffers()` again after `CA:675` fixes only the JUCE-heap overflow, not
   the HAL over-read/over-write in the clamp case — insufficient on its own.
3. No reliable app-side mitigation exists: the app cannot observe X (`getCurrentBufferSizeSamples()` returns Y). Querying
   CoreAudio directly for the input device's `BufferFrameSizeRange` and requesting `min(512, max)` helps only in case (a).
   Skipping the startup `setSourceMode` does not help (open #1 has the same defect). Opening input-only (no combiner)
   does not help (single-device `reopen()` has the same bodge).

## RISKS / strongest counterargument
- Strongest counter: "X is not 320 — the device reported 512 and something else shrank the block." Rebuttal: the block's
  size is computed from `bufferSize` at `CA:356-361` and nothing else; 324 = 1 × (320 + 4) is the only physically sensible
  factorisation; and E2's `alloc`/`reopen` lines settle it directly. If E2 shows `readBack == requested` at both opens yet
  ASan still fires at `CA:795`, this report is wrong and the remaining suspect is a `Stream` whose `channels` exceeds the
  count used at allocation (would need a `Stream` swap without re-allocation, which the source does not contain).
- Y = 512 is inferred from the output device's expected range; it could be any value ≥ 325 without changing the mechanism.
- Clamp-vs-stale is undetermined here; only E1/E2 decide it. The recommended fix (upgrade) does not depend on the answer.
- E1 starts the mic from a CLI tool: macOS may show a one-time microphone permission prompt for the terminal app if it has
  never had mic access (the ASan app run got mic data, so the launching terminal very likely already has it — INFERRED).
- The headset's presence is volatile (seen today); a "no crash" result without the pre-flight check proves nothing.
- Separate BT hazard noticed, not this crash: if the IO thread never runs a callback within `stop(false)`'s 2 s wait
  (`CA:745-752`), `audioDeviceStopPending` stays true and the NEXT start's first callback executes `AudioDeviceStop`
  (`CA:771-777`) — silent, stuck audio. Worth a line in the fix lane's test plan.

STATUS: COMPLETE — mechanism VERIFIED from source; X=320/Y=512 INFERRED pending E1/E2; headset absent at report time.
