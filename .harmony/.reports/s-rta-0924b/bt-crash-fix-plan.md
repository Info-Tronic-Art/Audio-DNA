# Bluetooth HFP startup crash — FIX PLAN (JUCE 8.0.4 CoreAudio temp-buffer overflow)

Architect (Fable), 2026-09-24, s-rta-0924b. Read-only: no source edited, app not launched, no debugger.
Inputs: `bt-crash-mechanism.md`, `bt-crash-juce-history.md`, `asan-bt-startup-crash.log` (this dir); JUCE 8.0.4 under
`build/_deps/juce-src` (re-read today, not trusted from the reports); app sources; `.harmony/s-rta-0924-work.md`,
`.harmony/probe-step3.sh`, `.harmony/notebook.md`. Abbreviations: `CA` = `build/_deps/juce-src/modules/juce_audio_devices/
native/juce_CoreAudio_mac.cpp`, `ADM` = `build/_deps/juce-src/modules/juce_audio_devices/audio_io/juce_AudioDeviceManager.cpp`,
`AE` = `src/audio/AudioEngine.cpp`, `MC` = `src/MainComponent.cpp`. Labels: VERIFIED (read/run today) / INFERRED / ASSUMED.

## RECOMMENDATION (first)
**Option A: bump JUCE `GIT_TAG 8.0.4` -> `8.0.8` (`CMakeLists.txt:30`), one line, gated fail-first.** B (local patch) is the
ready fallback, spec'd below. C (app-side) is rejected: nothing the app can do is provable from source, and the bug re-fires on
every device re-open path, not just startup.

## USER IMPACT — stated plainly (for Boris)
- **With the soundcore P31i connected as the Mac's default input+output, Audio-DNA cannot be launched.** Every launch overruns a
  JUCE heap buffer on the first headset audio cycle (~20 ms after the device starts) and the app dies with SIGABRT (malloc
  free-list checksum) or SIGSEGV (`AudioIODeviceCombiner::restartAsync`) — VERIFIED: ASan 1/1 today (`asan-bt-startup-crash.log:1-16`),
  3/4 plain launches on 0923 (`.harmony/s-rta-0923-work.md:18-20`). It blocked today's Harmony live gate at 12:48
  (`.harmony/s-rta-0924-work.md`, "12:48 LIVE GATE ... SIGABRT at launch").
- macOS makes a connected Bluetooth headset the default output and, for a headset with a mic, the default input (INFERRED from
  the 0923/0924 observations) — so "headset on before launch" = "app won't start".
- A launch that happens to survive is running on a corrupted heap: ~752 bytes (INFERRED: 512 requested vs 320 delivered) are
  written past a 1296-byte block on every 20 ms callback (`CA:793-797`), straight into the next allocation (`CA:883`). Any later
  crash or oddity in such a session is suspect.
- It is not only startup: the same overflow fires on every device re-open while the headset is the device — every
  `setSourceMode` in either mode (`AE:95-125`), a perf arm with an audio file (`MC:5092-5093`), play-with-audio
  (`MC:5172-5173`), stop-play restore (`MC:5205-5206`), and JUCE's own listener->restart chain (`CA:1118-1130`, `1372-1381`).
- **Workaround until fixed:** System Settings -> Sound: set BOTH Input and Output to the built-in devices (or disconnect the
  headset) BEFORE launching. Connecting the headset AFTER launch is safe: the app only re-initialises when its current device
  disappears (`ADM:193-221`, VERIFIED) and has no audio-device picker (`src/ui/PreferencesDialog.cpp` lists MIDI devices only,
  VERIFIED by grep) — it just keeps using the built-in mic/speakers.
- The 0923/0924 deprioritisation ("another time, not that important", `.harmony/HANDOFF.md:56`) was made when this was an
  unexplained 3/4 crash. It is now a known, deterministic bug in a dependency with an upstream fix; the fix is a one-line tag
  bump plus gates, and it sits on the critical path of the live gate whenever the headset is on. Recommend fixing now.

## QUESTION
Which of (A) JUCE upgrade, (B) local JUCE patch via FetchContent `PATCH_COMMAND`, (C) app-side mitigation fixes the
`CoreAudioInternal::audioCallback` heap overflow on the 16 kHz HFP headset, at what risk/blast radius, and how is each PROVEN?

## MECHANISM RECAP (re-verified from source today; full derivation in `bt-crash-mechanism.md`)
- `reopen()` (`CA:645-684`) sets the requested frame size Y (`CA:660-663`), then `updateDetailsFromDevice()` (`CA:673`) reads
  back X (`CA:463-466` -> `444-447`), stores `bufferSize = X` (`CA:504`) and allocates `total_channels * (X + 4)` floats
  (`CA:512` -> `356-361`), then the "bodge" overwrites `bufferSize = Y` (`CA:675`) without re-allocating. VERIFIED.
- `audioCallback()` (`CA:764-845`) copies `bufferSize` (= Y) frames per channel into the (X+4)-sized temp buffers
  (`CA:793-797`) and never consults `mDataByteSize` — the only use of that field in the whole file is the no-callback zeroing
  branch at `CA:839` (VERIFIED by grep). ASan hit = `CA:795`, thread T19 = HAL IO thread (`asan-bt-startup-crash.log:2-11`).
- Why Y > X here: the combiner asks each wrapper for `jmax(inDefault, outDefault)` (`CA:1520-1523`); a device's default is
  its first listed size >= 512 else its largest (`CA:1278-1289`); lists come from `BufferFrameSizeRange` plus the CURRENT member
  value (`CA:409-440`, `432-433`). The A2DP output offers 512, the HFP input (1 ch, 16 kHz) tops out at 320 -> the input is asked
  for 512, reads back 320 (X=320 from 1296 B = 1 x (320+4) x 4 B; VERIFIED arithmetic, value INFERRED). After open #1 the
  bodged 512 is appended to the input's own list (`CA:432-433`), so from then on JUCE reports 512 as "available" — this is
  what defeats any app-side discovery via `getAvailableBufferSizes()`.
- Call path: `AE:14 initialiseWithDefaultDevices(2,2)` (open #1, in the AudioEngine ctor) then `MC:261 setSourceMode(MicInput)`
  -> `AE:110 setAudioDeviceSetup` -> `ADM:808 open` -> `CA:1559` -> `CA:1307` -> `reopen`; `ADM:817 start` -> first IO cycle ->
  overflow (`asan-bt-startup-crash.log:13-27`, VERIFIED). Both opens run inside `MainComponent`'s constructor with the message
  loop blocked, so JUCE's only self-heal (listener -> timer -> consistent re-alloc -> restart) cannot run first.

## TRADEOFFS CONSIDERED
### A — bump JUCE to the first tag containing the upstream fix (8.0.8)  [RECOMMENDED]
- Fix: `f6df3e3` "CoreAudio: Respect buffer size passed to audio callback", first in tag 8.0.8 (Researcher, VERIFIED against
  GitHub). Its code is NOT on this machine — all four local JUCE checkouts are 8.0.4 (VERIFIED today: `build`, `build-asan`,
  `~/Documents/RealTimeAudio/build{,-debug}`, `Archived/...`). Coverage of THIS path is therefore INFERRED from the commit
  message: the callback size is taken from the incoming `AudioBufferList` and chunked when larger than the prepared buffer ->
  both the clamp case (device stays at 320) and the stale case (device later delivers 512) are bounded. Gate 6a makes it VERIFIED.
- Blast radius: every JUCE module the app links (12, `CMakeLists.txt:541-557`) and the test targets (`tests/CMakeLists.txt:884-887,
  935-938, 970`); full JUCE re-clone (shallow, ~200 MB) + recompile per build dir (`build`, `build-asan`, `build-gate`).
- Compile risk: BREAKING_CHANGES 8.0.5-8.0.15 relevant to linked modules = `AudioTransportSource::hasStreamFinished` (8.0.5)
  and `OpenGLFrameBuffer::readPixels/writePixels` RowOrder (8.0.9): ZERO call sites in `src/` and `tests/` (VERIFIED by grep
  today). No private/native JUCE headers included (VERIFIED). JUCE CMake API used = `juce_add_gui_app` only (`CMakeLists.txt:77`;
  stable across 8.0.x — ASSUMED). Any other compile break is loud, not silent.
- Behaviour risk: non-breaking changes in `juce_opengl`/`juce_gui_basics`/`juce_audio_devices` between 8.0.4 and 8.0.8 are not
  researched. Bounded by the gates: ctest (406), probe-step3 (63 rows incl. the Output-window/screen-law row), Eyes render
  pipeline. Bonus: also picks up `1616c0e` "Ensure devices are restarted correctly after changing sample rate" (Researcher) —
  relevant to this headset's HFP/A2DP rate flips (INFERRED).
- Why 8.0.8 and not 8.0.15: smallest step that contains the fix; identical compile-break audit result; smallest behavioural
  surface for the renderer/UI. Going further is a separate decision that needs a Researcher pass over CHANGE_LIST 8.0.9-8.0.15
  for `juce_opengl`/`juce_gui_basics`/`juce_audio_devices` — not required to fix this crash.
- Proof: fail-first ASan launch reproduces `CA:795` -> bump -> N clean ASan launches on the headset + ctest + probe-step3 63/0.

### B — carry a local patch to JUCE via FetchContent `PATCH_COMMAND`  [FALLBACK]
- B1: backport upstream `f6df3e3` as a `.patch` (Builder fetches the commit; it may not apply cleanly onto 8.0.4 — the file
  changed between 8.0.4 and 8.0.8, e.g. `1616c0e`; try `git apply --3way`, hand-resolve). B2: hand-written minimal clamp+chunk
  patch (~50 lines, spec in Appendix B).
- Blast radius: one JUCE file, the RT audio callback; every build dir re-patches at configure. The patch step MUST be idempotent
  (ExternalProject re-runs it after any update; CMake here is 4.2.3, VERIFIED) — apply via a CMake script that `git apply
  --check --reverse` first (Appendix B).
- Risk: we then own RT-thread code in a file we do not maintain; a mistake there is the same crash class; the patch must be
  dropped at the next JUCE bump (the apply script fails loudly then — acceptable). B2 leaves `getCurrentBufferSizeSamples()`
  reporting the bodged 512 while callbacks deliver 320 — the app copes (see RISKS, "reverse mismatch").
- Proof: identical gate to A. Choose B only if A fails to compile with non-trivial changes or a gate regresses for a
  JUCE-side reason not fixable inside the lane; the decision costs at most one build cycle.

### C — app-side mitigation in AudioEngine  [REJECTED — nothing provable from source]
- C1 request a fixed/explicit buffer size: `chooseBestBufferSize` (`ADM:871-879`) honours it only if it is in the combiner's
  COMMON list (`CA:1502-1507`); if the HFP range excludes 512 the fallback is `getDefaultBufferSize()` = `jmax` -> 512 anyway
  (`CA:1520-1523`) — the exact bug path. A smaller value (256/320) is honoured only if inside BOTH devices' ranges, which the
  app cannot learn: open #1 happens inside `initialiseWithDefaultDevices` (`AE:14`) before the app can look, and afterwards the
  input's list already contains the bodged 512 (`CA:432-433`). Only a direct CoreAudio `BufferFrameSizeRange` query (macOS-only
  app code duplicating JUCE internals) could pick a value, and only if the device CLAMPS — clamp-vs-stale is undetermined
  (mechanism report, needs E1). Does not cover the stale case at all.
- C2 open one device only (input-only in mic mode) to dodge the combiner's `jmax`: a single device derives Y from its own list
  (`CA:1291-1297`, `1278-1289`), so Y is in range — fixes clamp/fixed cases only, not stale. And it changes app semantics: with
  `numOutputChannels == 0` the mic-mode gain path analyses nothing (`src/audio/CombinedCallback.h:70-78`, `chans = min(in,out)`)
  and AudioTap is prepared with ACTIVE OUTPUT channels = 0 (`CombinedCallback.h:154-158`) -> takes record silence in mic mode.
  Blast radius = the recording subsystem the step-3 gate just certified. Rejected.
- C3 delay/defer: there is no seam — `open` and `start` are both inside `setAudioDeviceSetup` (`ADM:808`, `817`) and the overflow
  is on the first IO cycle after start; the self-heal needs a device notification AND a running message loop (`CA:1150-1197`
  -> `1118-1130`), and the constructor blocks the loop. Rejected.
- C4 detect after the fact (compare JUCE's size to a CoreAudio read after start, then stop): races the first IO cycle; the heap
  is already corrupted. Rejected.
- Structural: the bug is in the dependency and re-fires on every re-open path listed under USER IMPACT; app-side code would
  have to wrap all of them and would still be a guess.

## DECISION / SPEC (Option A)
Lanes: Builder (edits, builds, ctest); Harmony (live gates — the party that builds never verifies); Boris (connect/reconnect
the headset; `blueutil` is not installed, VERIFIED). Screen-safety law and the no-debugger/no-GUI-input rule
(`.harmony/gotchas.md:445-448`) apply to every live step.

0. Pre-flight (before EVERY launch, both fail-first and pass runs): `system_profiler SPAudioDataType | grep -B1 -A6 soundcore`
   must show the P31i as `Default Input Device: Yes` (Input Channels: 1, 16000 Hz) AND `Default Output Device: Yes`. Right now
   it does NOT (built-in mic/speakers are default — VERIFIED at report time), and the mechanism report saw the device leave the
   bus within minutes: a "no crash" without this check proves nothing.
1. FAIL-FIRST (Harmony, before any edit): the existing `build-asan` binary IS the unfixed binary (built today on merged main
   `0f34480`; `git status` shows only `.harmony/` files changed since — VERIFIED). Launch it per the notebook recipe
   (`.harmony/notebook.md:1300`): raw binary, `ASAN_OPTIONS=halt_on_error=1:abort_on_error=1:detect_leaks=0:log_path=/tmp/asan-ff`,
   wait for the process to exit on its own. Expect the report with `juce_CoreAudio_mac.cpp:795`. If it does not reproduce,
   STOP — re-check step 0; no pass run counts until the fail-first reproduces in the same session.
2. Builder: `CMakeLists.txt:30` -> `GIT_TAG        8.0.8`. No other code change. Reconfigure `build/` and `build-asan/`; if the
   shallow-clone tag update errors, `rm -rf <dir>/_deps/juce-src <dir>/_deps/juce-subbuild <dir>/_deps/juce-build` and
   reconfigure (fresh shallow clone). Positive checks that the fetched source is the fixed one (record in the lane report):
   `grep JUCE_BUILDNUMBER build/_deps/juce-src/modules/juce_core/system/juce_StandardHeader.h` -> 8;
   `git -C build/_deps/juce-src log -1 --format='%H %s'` -> "JUCE version 8.0.8";
   `grep -n mDataByteSize build/_deps/juce-src/modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp` -> MORE than the single
   8.0.4 hit (`CA:839`), with new hits inside `audioCallback`. Then READ the new `audioCallback` and note: (i) the frame count
   is derived from the buffer lists (upgrades "fix covers this path" from INFERRED to VERIFIED-by-reading), (ii) the chunk
   upper bound — `bufferSize` or the prepared allocation (decides the AudioCallback hardening in RISKS).
3. Builder: build `build/` (Release) rc 0. Fix only trivial API renames, each reported; anything non-trivial -> stop and report.
   ctest SERIAL (`ctest` without `-j`; a known flake under `-j6`, notebook): expect 406/406 (last count,
   `.harmony/s-rta-0924-work.md`) — compare counts; a SKIP is not a pass.
4. Builder: rebuild `build-asan` with its EXISTING configuration (`CMAKE_CXX_FLAGS=-fsanitize=address -fno-omit-frame-pointer`,
   `CMAKE_BUILD_TYPE=RelWithDebInfo`, `AUDIODNA_BUILD_TEST_SERVER=OFF`, `ADNA_SANITIZE` empty — VERIFIED from
   `build-asan/CMakeCache.txt`; RelWithDebInfo, not Debug, because Debug breaks `OutputWindow.cpp:273`, notebook 2026-07-30).
   The global flags instrument the JUCE module sources compiled into the app target — the fail-first report proves that for this
   config. Do not change the config between fail-first and pass runs.
5. Builder docs: `CLAUDE.md:161` JUCE version 8.0.4 -> 8.0.8; add Common Pitfall #31: "JUCE < 8.0.8 CoreAudio copies the
   REQUESTED frame count into temp buffers sized from the device's READ-BACK frame count (`reopen()` bodge) — Bluetooth HFP
   devices (16 kHz, 320-frame) overflow the heap on the first callback; never pin JUCE below 8.0.8 (upstream f6df3e3)".
   `.harmony/APP-INVENTORY.md` has no version string to update (VERIFIED by grep). `HANDOFF.md:56` loose-end #5 closes on gate
   PASS (Harmony's call).
6. Harmony gate (all of a-d must pass; e recommended; f informational):
   a. **ASan launches, headset default, N = 5** = 3 steady-state (headset connected > 1 min) + 2 fresh-connect (launch within
      ~10 s of Boris (re)connecting the headset — the worst case for the stale read-back window). Per launch: step 0; launch the
      raw ASan binary with `ASAN_OPTIONS=halt_on_error=1:abort_on_error=1:detect_leaks=0:log_path=/tmp/asan-fix-<i>`, stderr to
      `/tmp/asan-fix-<i>-stderr.log`; wait for `GET http://127.0.0.1:7070/api/health` (IPv4, production API is always on; <= 60 s;
      the first launch of a rebuilt binary may raise the TCC mic prompt — `screencapture -x` and LOOK, never dismiss); hold
      60 s (covers the listener/timer restart chain and the 2 s stop hazard). During the hold, machine checks that the app is
      REALLY on the headset and audio is flowing: `/api/features` -> `sourceSampleRate == 16000` and `bandValidMask == 63`
      (bit 6 clear at 16 kHz, CLAUDE.md FeatureSnapshot table); arm a 10 s live-input take via `/api/perf/record`, then
      `/api/perf/status` -> `deviceRate == 16000` and `sample` grows ~16000/s (+-10 %), `/api/perf/stop`. Then exercise one
      mid-session re-open: `/api/perf/play` withAudio on `~/Documents/Audio-DNA/Takes/step3gate2.adna-take` (exists, VERIFIED)
      -> switches to File mode (`MC:5172-5173`), wait 5 s, `/api/perf/stop_play` -> restores mic mode (`MC:5205-5206`); both are
      `setSourceMode` re-opens of the vulnerable path. Graceful quit `osascript -e 'tell application "Audio-DNA" to quit'`,
      wait for exit with the bracket pgrep `MacOS/Audio-DN[A]`, exit code 0, and NO `/tmp/asan-fix-<i>.*` file. PASS = 5/5.
      Any ASan report (any location) = FAIL; attach the log. N rationale: the defect is deterministic per launch once the device
      is in the reproducing state (1/1 today, 3/4 on 0923), so five state-confirmed clean launches — three of them with the
      re-open exercise, two on a fresh HFP link — is strong evidence; more launches add little unless the device state differs.
   b. **Release launch on the headset x2** (`build/` app via `open`, the user's real path): survives 60 s, the file label reads
      "Mic: soundcore P31i @ 16000Hz" (`AudioEngine::getDeviceStatus`, `AE:81-87`), graceful quit rc 0.
   c. **ctest** (Release, serial): 406/406.
   d. **probe-step3** on a fresh forced `build-gate/` (its own rule, `probe-step3.sh:14-15`) with the BUILT-IN devices default —
      the configuration of the 63/0 baseline (run 8, `.harmony/s-rta-0924-work.md`): expect 63 PASS / 0 FAIL, same row count.
      (The `bandValidMask == 127` row is 48 kHz-conditional, `probe-step3.sh:238-240`, but the baseline is defined on built-in.)
   e. Recommended (juce_opengl changed): `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_render_pipeline.py -v` against `build/`
      (`AUDIODNA_BUILD_TEST_SERVER=ON` there, VERIFIED) — screen-safety law applies.
   f. Informational: probe-step3 with the headset default (no baseline; record the count and any FAIL rows for R13 follow-up).
7. DONE = 6a-6d pass. Commit: `build(juce): 8.0.4 -> 8.0.8 - fixes CoreAudio temp-buffer heap overflow on Bluetooth HFP
   devices (JUCE f6df3e3)` + docs. Close `HANDOFF.md:56`; point `.harmony/.reports/s-rta-0923-startup-crash-diagnosis.md` at
   this dir. If 6a fails with the SAME stack (`audioCallback` temp-buffer overflow): the upstream fix does not cover this path ->
   switch to B2 (Appendix). If it fails with a DIFFERENT stack: new bug, own lane, do not fold in.

## RISKS
- **Strongest counterargument to A:** "You are changing the whole UI/GL framework mid-session to fix a 50-line bug; the
  renderer and the Output window are the app's most fragile surfaces, and a quiet rendering regression is exactly what a
  pass/fail gate can miss." Why A still wins: B's patch is RT-thread code we would write and own inside a file we do not
  maintain, with the same crash class if wrong, and it must be re-derived at the next bump anyway; A's risk is bounded and
  loud (compile breaks) or gated (ctest 406, probe-step3's 63 rows including the Output-window row, Eyes render pipeline), and
  it is one line to revert; A also fixes the output-side corollary (`CA:1853-1896`, mechanism report) and the sibling
  restart-after-rate-change bug, which B2 does not. B stays one build cycle away.
- Upstream coverage is INFERRED until step 2's reading and gate 6a; the 8.0.4 arithmetic (X=320, Y=512) is INFERRED (E1/E2 in
  the mechanism report would pin it, but the recommended fix does not depend on the answer).
- **Reverse mismatch after the fix (app-side):** if 8.0.8 chunks to the prepared allocation rather than to `bufferSize`, the app
  can receive a block LARGER than `getCurrentBufferSizeSamples()` when the read-back exceeds the request. AudioTap already
  chunks to `maxBlock_` (`src/recording/AudioTap.cpp:245-261`; `tests/test_bt_device_shapes.cpp:175-353`, VERIFIED), but
  `AudioCallback` silently DROPS the excess (`src/audio/AudioCallback.cpp:24`, buffer sized at `:49`) -> analysis clock skew.
  Builder decides at step 2(ii): if the chunk bound can exceed `bufferSize`, loop `AudioCallback` in `monoBuffer_`-sized chunks
  (5 lines) with a test mirroring `test_bt_device_shapes`; otherwise leave it alone.
- After the fix the headset callback size becomes 320 (or chunks) while `getCurrentBufferSizeSamples()` still reports the
  bodged 512 (`CA:675`, `762`, `1273`; upstream did not touch the bodge — INFERRED). App consumers of that value —
  `AudioCallback.cpp:49` (mono buffer) and `CombinedCallback.h:157` (AudioTap `maxBlock`) — are only over-sized by it; the
  analysis hop (512) is decoupled by the ring buffer. No app change needed for that.
- Device volatility: the headset was default at 12:48 and 13:05 today and is NOT default now (VERIFIED). Every launch needs
  step 0; a run without it is void. Fresh-connect launches need Boris at the headset.
- TCC: the first launch of each rebuilt binary can prompt for microphone access; look at a screenshot, never dismiss. The
  terminal that ran today's ASan binary already has mic access (INFERRED from the report — the callback received input data).
- Separate BT hazard, NOT fixed by A (ASSUMED — no evidence 8.0.8 touched it): `stop(false)` waits <= 2 s for the IO thread
  (`CA:740-756`); if the HFP link delivers no cycle in that window, `audioDeviceStopPending` stays set and the NEXT start's
  first callback executes `AudioDeviceStop` (`CA:771-777`) -> silent, stuck audio. Gate 6a's `sample`-grows check catches it;
  if seen, log it as its own loose end.
- Product note, unchanged by this fix: with the headset as device the combiner forces both directions to one common rate
  (`CA:1553-1561`, `1927-1944`) — the A2DP output is pushed to 16 kHz while the app runs; file-mode analysis then runs from a
  16 kHz device (R13's resampler handles it; Brilliance band gated). The R13 plan's Boris product questions remain open.
- Network: FetchContent will re-download JUCE (~200 MB shallow) once per build dir.

## Appendix B — fallback patch spec (only if A is blocked; execute as written, no re-derivation)
Files: `cmake/patches/juce-8.0.4-coreaudio-respect-delivered-frames.patch` (unified diff against
`modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp`), `cmake/patches/apply-git-patch.cmake`, and in
`FetchContent_Declare(JUCE ...)` (`CMakeLists.txt:27-32`) add
`PATCH_COMMAND ${CMAKE_COMMAND} -DPATCH_FILE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/patches/juce-8.0.4-coreaudio-respect-delivered-frames.patch -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patches/apply-git-patch.cmake`.
Script: `execute_process(git apply --check --reverse ${PATCH_FILE})` -> rc 0 means already applied: message + return;
else `execute_process(git apply --whitespace=nowarn ${PATCH_FILE})`, rc != 0 -> `FATAL_ERROR`. ExternalProject runs
`PATCH_COMMAND` in the source dir (ASSUMED; if not, pass `-DSRC_DIR=<SOURCE_DIR>` and use `WORKING_DIRECTORY`). Post-configure
check in every build dir: `grep -c AUDIODNA-PATCH <dir>/_deps/juce-src/modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp` == 1.
Patch content (class `CoreAudioInternal`):
1. Member `int tempBufferFrames = 0;` beside `int bufferSize = 0;` (`CA:1113`). In `allocateTempBuffers()` after `tempBufSize`
   is computed (`CA:356`): `tempBufferFrames = tempBufSize;`.
2. In `audioCallback()` (`CA:764-845`), inside `if (callback != nullptr)`: compute the frames the HAL actually delivered —
   `framesAvailable(list, stream)` = min over the stream's `channelInfo` entries with `dataStrideSamples != 0` of
   `list->mBuffers[info.streamNum].mDataByteSize / (sizeof (float) * (UInt32) info.dataStrideSamples)`; `INT_MAX` when the
   stream is null/empty or the list is null. `delivered = jmin (framesAvailable (inInputData, inStream), framesAvailable
   (outOutputData, outStream))`; if still `INT_MAX`, use `bufferSize`. `maxChunk = jmax (1, jmin (bufferSize, tempBufferFrames))`
   — never hand the app more than it was told at `audioDeviceAboutToStart`, never more than the allocation.
3. Replace the single-pass body with `for (offset = 0; offset < delivered; offset += maxChunk) { n = jmin (maxChunk, delivered -
   offset); input copy with src = base + info.dataOffsetSamples + offset * stride, j = n; discontinuity check only when offset
   == 0; callback->audioDeviceIOCallbackWithContext (..., n, context); output copy with dest = base + info.dataOffsetSamples +
   offset * stride, j = n; }`. Keep the no-callback zeroing branch (`CA:835-840`) unchanged. `previousSampleTime += delivered`
   (was `bufferSize`, `CA:843`). Marker comment: `// AUDIODNA-PATCH(bt-hfp): respect delivered frame count (f6df3e3 semantics)`.
4. Chunk, never truncate: AudioTap counts `numSamples` per callback (`CombinedCallback.h:130-135`); dropping frames would
   time-compress a take during the stale window. Proof = the same gate 6a-6d (JUCE internals are not unit-testable here).

STATUS: COMPLETE — recommendation A (JUCE 8.0.4 -> 8.0.8), fail-first gate defined (N=5 ASan headset launches + 2 Release +
ctest 406 + probe-step3 63/0); B spec'd as fallback; C rejected with cites. Headset not default at report time.
