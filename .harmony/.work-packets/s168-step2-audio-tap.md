# WORK PACKET — s168 Step 2: the audio tap, its sample-exact counter, and T1

PROJECT: ~/projects/RealTimeAudio (Audio-DNA, C++20/JUCE)
SPEC (authoritative, READ IT IN FULL BEFORE WRITING ANYTHING):
  `.harmony/specs/s167-performance-log-and-routines.md`
  — §2 **D10 (AUDIO IS PART OF THE TAKE — the tap, the sync contract, the test, the cost)**,
    §5 Build order **row 2**, §6 risks **R12, R14, R15**.
  D10.3 states the EXACT tolerances T1 must meet. Do not invent your own.

## WHY THIS MATTERS (the owner said it, verbatim)
"Depending on what was used for the track, it was either the audio from the microphone or from
the sound card or from whatever. It gets recorded along with the log **and we will test it to make
sure it stays in time**." The sync test is a DELIVERABLE HE NAMED, not an afterthought. A
recording is `{lanes + audio}` and must be self-contained — an offline video render must not need
the DJ's original track file.

## THE HAZARD THAT KILLS THE NAIVE VERSION
There is ALREADY a full-rate PCM ring buffer hanging off the audio callback. **It is
SINGLE-CONSUMER, owned by the analysis thread, and `src/audio/AudioCallback.cpp` IGNORES the
return value of `push`** — so it drops samples silently under a stall, with no trace (spec R12).
Do NOT reuse it and do NOT add a second consumer to it. This packet needs a SECOND, INDEPENDENT
TAP off the same callback.

## TASK — row 2 of the spec's build order, and nothing beyond it
1. `AudioTap` — a second fan-out in the device callback that writes captured input to disk as
   `audio.wav` inside the take folder. Non-blocking on the audio thread; the file write happens
   off it.
2. A **delivered-sample counter** (`deliveredSamples_`) that is the take's timebase origin, plus
   **gap detection**. Per D10 the counter, not wall clock, is what lanes align against.
3. **Lift `CombinedCallback` to `src/audio/CombinedCallback.h`** so the T1 test can construct one
   directly. This is a MOVE for testability — do not change its behaviour while moving it.
4. `.adna-take` folder save/load completion: `audio.wav` sits beside `take.json`. `Take::save`/
   `Take::load` already take a directory as the real contract (Lane A) — extend, do not redesign.
5. **T1 — `tests/test_audio_tap_sync.cpp`**, registered in `tests/CMakeLists.txt`. It must pass at
   D10.3's stated tolerances: 0 samples exact, <= 1 block late, **across a dropout, a restart, and
   a FIFO stall**. R14: `hostTimeNs` is ASSUMED non-null on CoreAudio; if it is null, gap detection
   is off and the take must SAY SO (`audio.gapDetection:false`). **T1 must cover BOTH branches.**
6. R15: `RecordPanel` should refuse to start on a volume with < 2 GB free. If that check belongs in
   the tap rather than the panel, put the mechanism here and leave the UI call for step 4 — say
   which you chose and why.

## HARD CONSTRAINTS
- **Nothing that can block, allocate, lock, or log on the AUDIO THREAD.** A priority inversion here
  is a dropout in front of an audience. State in your report, per function, what runs on the audio
  thread and why each call is safe there.
- Do NOT change the existing analysis ring buffer's behaviour or its single-consumer ownership.
  If you find yourself editing it, stop — you are on the wrong path.
- Do NOT touch: `src/signal/*`, `src/analysis/*`, `src/connect/*`, `src/api/*`,
  `src/test/TestServer.cpp`, `tests/test_bpm_stabilization.cpp`,
  `tests/test_oscillator_bar_fold.cpp` — another lane owns all of those RIGHT NOW.
  `src/ui/RecordPanel.*` belongs to a later step; leave it alone.
- You MAY touch: `src/audio/*`, `src/recording/AudioTap.*`, `src/recording/Take.{h,cpp}` (extension
  only), `tests/test_audio_tap_sync.cpp`, `tests/fixtures/*`, `CMakeLists.txt`,
  `tests/CMakeLists.txt`.
- **USE YOUR OWN BUILD DIRECTORY: `cmake -S . -B build-s2 -DCMAKE_BUILD_TYPE=Release` and
  `cmake --build build-s2 --config Release -j6`, then `cd build-s2 && ctest`.** Another lane is
  building concurrently and a shared `build/` produces plausible-but-wrong results. Do NOT build
  in `build/`. Do not delete `build/`.
- Do NOT launch the app. Do NOT commit. Do NOT push.

## SUCCESS CRITERIA
- Build exits 0 in `build-s2`.
- `cd build-s2 && ctest` — all pass, count strictly greater than 296, no regression.
  **296/296 is the MEASURED baseline from a clean forced rebuild. Re-run it; never inherit it.**
- T1 exists, is registered, and passes at D10.3's tolerances — including the dropout, the restart,
  the FIFO stall, and BOTH `hostTimeNs` branches.
- The stated arithmetic identity holds in the test: a recording's WAV frame count equals
  `deliveredSamples - firstSample`.

## REPO GOTCHAS (each cost a prior session real time)
- Line numbers DRIFT here. Re-grep by anchor text; never trust a cited number, including any above.
- A negative grep is only as strong as its pattern. State your pattern; try an alternative spelling
  before believing a "does not exist anywhere" answer.
- `.harmony/` is gitignored with force-tracked files; `git add` there warns and still stages,
  breaking `&&` chains — use `;`.
- Confirm you are in `~/projects/RealTimeAudio`, NOT `~/projects/RealTimeAudio copy` (stale Jul 11).
- Analysis hard-codes 48 kHz with no resampling (R13). The owner's machine reports 48000 today, so
  do not "fix" it here — but do not build a NEW 48 kHz assumption into the tap. Take the rate from
  the device.

## REPORT — TO DISK, MANDATORY
Write to `.harmony/.reports/s168-step2-audio-tap.md` before returning, then also return it.
Fields: WHAT / WHERE (files + symbols) / WHAT RUNS ON THE AUDIO THREAD AND WHY EACH CALL IS SAFE /
HOW T1 SIMULATES THE DROPOUT, THE RESTART AND THE STALL / HOW VERIFIED (exact commands, before and
after ctest counts, which build dir) / WHAT IS NOT VERIFIED / ASSUMPTIONS / RISKS LEFT OPEN / STATUS.
