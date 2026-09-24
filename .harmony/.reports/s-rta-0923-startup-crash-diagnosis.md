# RTA Startup-Crash Diagnosis — Serialized Launch Experiment (2026-09-23, s-rta-0923)

**Verdict up front**: The experiment produced **0 crashes in 9 launches** (3 binaries × 3 launches: pre-s168 baseline, ASan HEAD, Release HEAD). This result **does not discriminate H1 vs H2** — the environment changed between the original crash session (21:17–21:19 EDT) and this experiment (21:42–21:45 EDT): the Bluetooth `soundcore P31i` (16 kHz HFP) is **no longer the default audio device**. All 9 launches ran on the built-in MacBook Pro mic/speakers at 48 kHz, i.e. the one audio path the analysis pipeline was actually built for. That is the single most important fact in this report — see "Critical caveat" below before reading anything else as exculpatory.

Re-parsing the three original `.ips` files directly (not trusting the earlier prose summary) turned up a detail the FACTS block didn't call out: **the three original crashes are not one signature, they are two.** One is a pure JUCE/CoreAudio-internal SIGSEGV with zero app-code frames (points at H2). The other two are malloc heap-corruption-detected-later SIGABRTs, discovered at unrelated allocator call sites (points at H1). See "Verdict on H1/H2/H3" for the reasoning — this is inferred from re-examined old evidence, not new evidence generated tonight, and is labeled as such throughout.

---

## 0. Pre-flight: audio device state (recorded before any launch)

```
$ system_profiler SPAudioDataType | grep -B2 -A3 Default
        MacBook Pro Microphone:
          Default Input Device: Yes
          ...
          Current SampleRate: 48000
        MacBook Pro Speakers:
          Default Output Device: Yes
          Default System Output Device: Yes
          ...
          Current SampleRate: 48000
```

`soundcore P31i` is still paired (visible in `system_profiler SPBluetoothDataType`) but is **not** the current default input/output — it was either disconnected or the Mac fell back to the built-in devices between the 21:19 findings and this 21:42 run. **Verified** via `system_profiler`, not assumed.

Every one of tonight's 9 launches logged `[AudioEngine] Switched to mic input mode` and none of the 9 launch logs printed the "device sample rate is 16000 Hz but the analysis pipeline assumes 48000 Hz" warning that the original session's logs showed — corroborating that tonight's runs used the 48 kHz built-in path, not the 16 kHz Bluetooth HFP path that was live when the original 3 crashes happened.

**Critical caveat**: because the trigger condition (Bluetooth HFP, 16 kHz, mono, mid-session device-list churn) was absent tonight, a clean launch on all three binaries is the *expected* result regardless of which hypothesis (H1/H2/H3) is correct. Zero crashes tonight is **not evidence against H1 or H2** — it is evidence only that all three binaries are stable on the untriggering path. Re-running this exact protocol with the Bluetooth headset reconnected and selected as default is the actual missing experiment.

---

## 1. Per-binary launch results

Protocol: serialized (pgrep-confirmed empty before each launch), detached via `nohup ... &`, 15 s wait, ALIVE/DEAD via `pgrep -f "MacOS/Audio-DNA"`, health via `curl -m3 127.0.0.1:7070/api/health`, then `pkill` + wait-until-gone. ASan runs used `ASAN_OPTIONS=abort_on_error=0:halt_on_error=1:detect_leaks=0`.

| Binary | Launch | Status | FPS (from /api/health) | Notes |
|---|---|---|---|---|
| Baseline (pre-s168, `3736f02^`) | 1 | ALIVE | 76.99 | clean |
| Baseline | 2 | ALIVE | 83.00 | clean |
| Baseline | 3 | ALIVE | 81.64 | clean |
| ASan HEAD (f5ae847, RelWithDebInfo) | 1 | ALIVE | 93.23 | no ASan report |
| ASan HEAD | 2 | ALIVE | 102.15 | no ASan report |
| ASan HEAD | 3 | ALIVE | 90.24 | no ASan report |
| Release HEAD (f5ae847, `build/`) | 1 | ALIVE | 76.99 | used for spare-survival BPM check |
| Release HEAD | 2 | ALIVE | 76.99 | clean |
| Release HEAD | 3 | ALIVE | 76.04 | clean |

**Crash count by binary: 0/3 baseline, 0/3 ASan HEAD, 0/3 Release HEAD.** No new `~/Library/Logs/DiagnosticReports/Audio-DNA-*.ips` files were created during the 21:42:25–21:45:08 EDT experiment window (verified with `find ... -newermt "-70 min"` — only the three pre-existing 21:17–21:18 reports from the earlier session are on disk).

### ASan report (per task instructions: quote fully if produced)

**None produced.** `grep -l "AddressSanitizer" /private/tmp/rta-launch-asan-*.log` matched nothing in any of the 3 ASan-instrumented runs. The binary is confirmed ASan-instrumented (linked against `libclang_rt.asan_osx_dynamic.dylib`, per the build task's own verification), so this is a true "did not trigger" result under tonight's conditions, not an instrumentation failure. There is therefore no allocated-by/freed-by stack to report — ASan needs the app to actually execute the code path where the s168 AudioTap fan-out is exercised, which the current mono/16kHz-vs-48kHz-mismatch trigger condition wasn't present to exercise.

### Spare-survival BPM/bar check (owed check, Release HEAD launch #1)

```
BPM_T1 release #1 (t=0s):  bpm=154.90  beatPhase=0.0047  barPhase=0.251  phrasePhase=0.281  beatInBar=1  barCount=2   totalBarCount=2
BPM_T2 release #1 (t=+10s): bpm=154.90  beatPhase=0.8896  barPhase=0.722  phrasePhase=0.090  beatInBar=2  barCount=8   totalBarCount=8
```

`totalBarCount` **is present and increasing** (2 → 8 over 10 s). Sanity check: at 154.9 BPM (2.58 beats/s) and 4 beats/bar, 10 s ≈ 25.8 beats ≈ 6.5 bars, consistent with the observed 6-bar increase. This owed check **passes**.

---

## 2. Original crash reports — independently re-parsed (not new evidence, re-derived from the 21:17–21:19 session)

Parsed directly with Python (`.ips` = one JSON header line + one JSON body) rather than trusting the earlier prose summary.

### Crash A — `Audio-DNA-2026-09-23-211737.ips`
```
Exception: EXC_BAD_ACCESS / SIGSEGV, subtype KERN_PROTECTION_FAILURE at 0x000000016f4f5c90
Faulting thread: "HALC_ShellPlugIn Connection Queue" (thread index 1, NOT the message thread)
Top frames:
 #00 <unknown>
 #01 Audio-DNA   non-virtual thunk to juce::CoreAudioClasses::AudioIODeviceCombiner::restartAsync() +56
 #02 Audio-DNA   juce::CoreAudioClasses::CoreAudioInternal::deviceListenerProc(unsigned int, unsigned int, AudioObjectPropertyAddress const*, void*) +536
 #03 CoreAudio   HALObject::PropertiesChanged(...) +1920
 #04 CoreAudio   HALObject::ObjectsPublishedAndDied(...) +548
 #05 CoreAudio   HALSystem::AudioObjectsPublishedAndDied(...) +200
 #06 CoreAudio   invocation function for block in HALC_ShellPlugIn::Defer_AudioObjectsPublishedAndDied(...) +40
 #07 libdispatch.dylib  _dispatch_call_block_and_release +32
 #08 libdispatch.dylib  _dispatch_client_callout +16
 #09 libdispatch.dylib  _dispatch_lane_serial_drain +740
 #10 libdispatch.dylib  _dispatch_lane_invoke +440
 #11 libdispatch.dylib  _dispatch_workloop_invoke +1612
```
**Every frame is JUCE-internal, CoreAudio-internal, or libdispatch-internal.** No AudioTap or other Audio-DNA analysis-thread symbol appears anywhere in this stack. The fault fires on a HAL-owned dispatch queue reacting to a device publish/die event (i.e. a Bluetooth device dropping/reappearing), landing inside JUCE's own `AudioIODeviceCombiner::restartAsync()`.

### Crash B — `Audio-DNA-2026-09-23-211744.ips`
```
Exception: EXC_CRASH / SIGABRT ("Abort trap: 6")
Faulting thread: "JUCE v8.0.4: Message Thread" (thread index 0)
Top frames:
 #00 libsystem_kernel.dylib  __pthread_kill +8
 #01 libsystem_pthread.dylib pthread_kill +296
 #02 libsystem_c.dylib       abort +124
 #03 libsystem_malloc.dylib  malloc_vreport +892
 #04 libsystem_malloc.dylib  malloc_zone_error +100
 #05 libsystem_malloc.dylib  free_list_checksum_botch +40
 #06 libsystem_malloc.dylib  small_free_list_remove_ptr_no_clear +964
 #07 libsystem_malloc.dylib  small_malloc_from_free_list +512
 #08 libsystem_malloc.dylib  small_malloc_should_clear +176
 #09 libsystem_malloc.dylib  szone_malloc_should_clear +120
 #10 libsystem_c.dylib       __opendir_common +356
 #11 libsystem_c.dylib       __opendir2 +72
```
`free_list_checksum_botch` fires **inside an unrelated `opendir()` call** (almost certainly `AVCaptureDevice` enumeration, per the FACTS block, since that's the code path in this app that calls `opendir`-family functions around startup) — i.e. malloc's free-list metadata was already corrupted *before* this allocation, and the corruption is only detected here as collateral damage.

### Crash C — `Audio-DNA-2026-09-23-211855.ips`
```
Exception: EXC_CRASH / SIGABRT ("Abort trap: 6")
Faulting thread: "JUCE v8.0.4: Message Thread" (thread index 0)
Top frames:
 #00 libsystem_kernel.dylib  __pthread_kill +8
 #01 libsystem_pthread.dylib pthread_kill +296
 #02 libsystem_c.dylib       abort +124
 #03 libsystem_malloc.dylib  malloc_vreport +892
 #04 libsystem_malloc.dylib  malloc_zone_error +100
 #05 libsystem_malloc.dylib  free_list_checksum_botch +40
 #06 libsystem_malloc.dylib  small_free_list_remove_ptr_no_clear +964
 #07 libsystem_malloc.dylib  free_small +632
 #08 CoreGraphics            __CGRegionDeallocate +56
 #09 CoreFoundation           _CFRelease +296
 #10 AppKit                  <unknown symbol>
 #11 AppKit                  <unknown symbol>
```
Same `free_list_checksum_botch` signature as Crash B, this time surfaced during an AppKit `CGRegion` deallocation (`_CFRelease` → `CGRegionDeallocate` → `free_small`). Again: the corruption was detected far from wherever it was actually introduced.

---

## 3. Verdict on H1 / H2 / H3

**H1 (s168 AudioTap fan-out corrupts the heap)** — **PLAUSIBLE, not confirmed tonight.** Crashes B and C share a `free_list_checksum_botch` signature, which is the classic delayed-detection symptom of an earlier out-of-bounds write or double-free somewhere unrelated to where the abort fires (`opendir` internals in one case, AppKit `CGRegionDeallocate` in the other — two completely different call sites converging on the same corrupted-metadata detector). That pattern — same detector, different innocent trigger sites — is much more consistent with "something wrote past a heap buffer earlier and the damage is discovered later" than with a live race at the abort site itself. s168's AudioTap is the one piece of code in this diff range that both (a) is new since the last successful run and (b) touches the audio-callback fan-out with buffer/channel-count assumptions that the 16 kHz mono Bluetooth HFP path could plausibly violate. This is **inferred**, not proven: I did not reproduce a crash tonight, so there is no ASan allocation/free stack pointing at AudioTap code — the discriminating evidence would be exactly that ASan stack, and tonight's environment (48 kHz built-in device) never exercised the code path that would produce it.

**H2 (JUCE 8.0.4 CoreAudio combiner race, independent of our code)** — **PLAUSIBLE and separately supported.** Crash A is a clean counter-example to H1 as the sole explanation: it is a SIGSEGV with an unbroken JUCE→CoreAudio→libdispatch call chain and **zero app-code frames**, firing on a HAL-internal dispatch queue in direct response to a device publish/die event (i.e., the Bluetooth device's CoreAudio object disappearing/reappearing, a known trigger class for JUCE `AudioIODeviceCombiner` issues on macOS when the default device set changes under the app). This crash cannot be explained by heap corruption from our own code without a much longer inferential chain, and is the more natural fit for "the JUCE 8.0.4 combiner has a known race on Bluetooth profile/device transitions."

**Combined reading**: the three original crashes are not one bug, they're at least two — H1 and H2 both have direct evidential support **from the original 21:17–21:19 crash reports**, not mutually exclusive. **H3 (something else)** is not needed to explain any of the three signatures; both A and (B,C) map cleanly onto H1/H2 respectively, so I'm not invoking a third cause.

**What would actually discriminate further**: re-run this exact 9-launch protocol with `soundcore P31i` reconnected and selected as the default input/output (reproducing the original 16 kHz HFP condition). If the SIGABRT heap-corruption signature (B/C) reproduces under ASan and yields an "allocated by" stack inside `AudioTap`/`AudioCallback`, that confirms H1. If instead only the SIGSEGV-in-`restartAsync()` signature (A) reproduces, and it reproduces identically on the **pre-s168 baseline** (which has no AudioTap), that would confirm H2 as sufficient on its own and let H1 be downgraded. Tonight's run could not perform this test because the triggering device was not present — **this is the single owed follow-up**, and it requires reconnecting the Bluetooth headset, which was out of scope for tonight's launch-only protocol.

---

## 4. Secondary observations (not part of the H1/H2/H3 question, noted because they showed up in all 9 logs identically)

- `source_julia_set` shader **fails to compile at startup** on all three binaries (baseline, ASan HEAD, Release HEAD) — `source_julia_set_3d` compiles fine. This matches the pre-existing notebook finding (unrelated to s168/AudioTap, present even in the pre-s168 baseline, so it predates s168 and is not a regression from this investigation).
- No ASan errors, warnings, or leak reports of any kind appeared in any of the 3 ASan launch logs.

---

## 5. Guardrails honored

- Never more than one `Audio-DNA` process alive at a time (`pgrep`-confirmed empty before every launch, `pkill` + wait-until-gone after every launch).
- `/Users/boriskarpman/projects/RealTimeAudio/build` untouched — `mtime` unchanged (Sep 6 11:50) before and after this session.
- No Output window / fullscreen ever opened — every launch was detached, unattended for 15 s, and only queried via HTTP (`/api/health`, `/api/bpm`); no UI automation of any kind touched the running app.
- Final process check at end of experiment and again at report time: **no Audio-DNA process running.**
- No git commits made (`HEAD` still `f5ae847`, working tree diff unchanged from session start).
- No writes to `~/Harmony_Main`.

---

## Appendix: raw result log

```
PRE-LAUNCH-CHECK baseline #1: empty, OK
RESULT baseline #1 start=2026-09-24T01:42:25Z start_epoch=1790214145 status=ALIVE pid=36726 health=RESPONDED: {
  "ok": true,
  "status": "ready",
  "version": "0.1.0",
  "fps": 76.994041442871094,
  "effects_count": 135
}
POST-KILL-CHECK baseline #1: empty, confirmed gone
---
PRE-LAUNCH-CHECK baseline #2: empty, OK
RESULT baseline #2 start=2026-09-24T01:42:40Z start_epoch=1790214160 status=ALIVE pid=42925 health=RESPONDED: {
  "ok": true,
  "status": "ready",
  "version": "0.1.0",
  "fps": 82.998092651367188,
  "effects_count": 135
}
POST-KILL-CHECK baseline #2: empty, confirmed gone
---
PRE-LAUNCH-CHECK baseline #3: empty, OK
RESULT baseline #3 start=2026-09-24T01:42:56Z start_epoch=1790214176 status=ALIVE pid=50648 health=RESPONDED: {
  "ok": true,
  "status": "ready",
  "version": "0.1.0",
  "fps": 81.643867492675781,
  "effects_count": 135
}
POST-KILL-CHECK baseline #3: empty, confirmed gone
---
PRE-LAUNCH-CHECK asan #1: empty, OK
RESULT asan #1 start=2026-09-24T01:43:11Z start_epoch=1790214191 status=ALIVE pid=56974 health=RESPONDED: {
  "ok": true,
  "status": "ready",
  "version": "0.1.0",
  "fps": 93.229095458984375,
  "effects_count": 135
}
POST-KILL-CHECK asan #1: empty, confirmed gone
---
PRE-LAUNCH-CHECK asan #2: empty, OK
RESULT asan #2 start=2026-09-24T01:43:26Z start_epoch=1790214206 status=ALIVE pid=61659 health=RESPONDED: {
  "ok": true,
  "status": "ready",
  "version": "0.1.0",
  "fps": 102.147987365722656,
  "effects_count": 135
}
POST-KILL-CHECK asan #2: empty, confirmed gone
---
PRE-LAUNCH-CHECK asan #3: empty, OK
RESULT asan #3 start=2026-09-24T01:43:41Z start_epoch=1790214221 status=ALIVE pid=67818 health=RESPONDED: {
  "ok": true,
  "status": "ready",
  "version": "0.1.0",
  "fps": 90.240570068359375,
  "effects_count": 135
}
POST-KILL-CHECK asan #3: empty, confirmed gone
---
PRE-LAUNCH-CHECK release #1: empty, OK
RESULT release #1 start=2026-09-24T01:43:57Z start_epoch=1790214237 status=ALIVE pid=76522 health=RESPONDED: {
  "ok": true,
  "status": "ready",
  "version": "0.1.0",
  "fps": 76.998634338378906,
  "effects_count": 135
}
BPM_T1 release #1: {
  "ok": true,
  "bpm": 154.896743774414062,
  "beatPhase": 0.00466775894165,
  "barPhase": 0.251166939735413,
  "phrasePhase": 0.281395852565765,
  "beatInBar": 1,
  "barCount": 2,
  "totalBarCount": 2
}
BPM_T2 release #1: {
  "ok": true,
  "bpm": 154.896743774414062,
  "beatPhase": 0.889645397663116,
  "barPhase": 0.722411334514618,
  "phrasePhase": 0.090301416814327,
  "beatInBar": 2,
  "barCount": 8,
  "totalBarCount": 8
}
POST-KILL-CHECK release #1: empty, confirmed gone
---
PRE-LAUNCH-CHECK release #2: empty, OK
RESULT release #2 start=2026-09-24T01:44:22Z start_epoch=1790214262 status=ALIVE pid=90491 health=RESPONDED: {
  "ok": true,
  "status": "ready",
  "version": "0.1.0",
  "fps": 76.994316101074219,
  "effects_count": 135
}
POST-KILL-CHECK release #2: empty, confirmed gone
---
PRE-LAUNCH-CHECK release #3: empty, OK
RESULT release #3 start=2026-09-24T01:44:37Z start_epoch=1790214277 status=ALIVE pid=92956 health=RESPONDED: {
  "ok": true,
  "status": "ready",
  "version": "0.1.0",
  "fps": 76.044654846191406,
  "effects_count": 135
}
POST-KILL-CHECK release #3: empty, confirmed gone
---
EXPERIMENT COMPLETE
Final process check: none running
```
