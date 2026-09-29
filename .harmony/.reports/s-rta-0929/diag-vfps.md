# s-rta-0929 diag-vfps -- video playback frame rate on the M1 Pro (Q1 "bimodal per launch", Q2 the 4 x 4K ceiling)

STATUS: DONE
RESULT: Q1 -- the split is per LOAD, not per launch. Each video's 30 fps frame clock starts on the render frame where its
clip is first drawn after the trigger. At 120 Hz a 30 fps clock delivers a new frame every 4 render frames. When all 4
clocks start on the same frame of those 4 ("bunch pattern 4"), all 4 new frames land on ONE render frame. That frame costs
6.1-6.5 ms of GL-thread CPU (4 x 1.4 ms texture uploads), and the render loop then drops display-link ticks: 104-117 fps.
Spread phases on the same binary read 117.6-120 fps. Counterfactual, 10 launches x 3 loads per arm, arms interleaved:
capping video uploads at 1 or 2 per render frame reads 119.9 / 119.8 fps median (min 119.1 / 116.9). Base reads 116.5 /
117.8 median (min 109.4 / 107.7). Lost display-link ticks fall from 3.5 / 2.1 per s to 0.2 per s.
Q2 -- most of the 4 x 4K gap is the GL-thread CPU cost of `glTexSubImage2D` from client memory: 0.86-0.96 ms per 4K frame,
bunched as in Q1. Without uploads the scene reads 111-116 fps; base reads 89.8-90.6. An IOSurface ring with one GPU blit
per new frame reads 105-107 fps (64-71 % of that gap back) and gives a byte-identical picture. The upload pixel format
(BGRA / 8_8_8_8_REV), a PBO and thread QoS each gain 0. Apple client storage is a trap: 28 fps. Even with no uploads the
scene stops at 111-116 fps, because the 4K composite takes 7.0-7.2 ms of GPU time per frame.
FACTS:
- (Q1) VERIFIED `/Users/boriskarpman/projects/RealTimeAudio/.harmony/probe-video.py:587-590` -- w1's "5 runs" are 5 fresh LOADS inside ONE app launch (`reps` loop in `steady()`; one launch per probe-video.sh). Loads in one launch differ, e.g. launch r1 108.3 / 104.0 / 113.9 and r2 118.9 / 118.3 / 114.1 (`/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/agg/q1-perlaunch.md`, base_w1)
- (Q1) VERIFIED fps follows the bunch pattern over 30 loads of the probe's own trigger sequence: pattern 4 reads 103.8-115.3 (n=20); spread patterns (3+1, 2, 2+1) read 117.6-119.5 (n=9); the ranges do not overlap (`/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/agg/q1-bunch-sequential.txt`). One /api/trigger_column gives pattern 4 on every load: 291/291 uncapped windows (`/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/agg/q1-table.txt`)
- (Q1) VERIFIED counterfactuals, 4 x 1080p w1c scene, 10 launches x 3 loads per arm, arms interleaved launch by launch -- median [min, max] fps: base 116.5 [109.4, 119.7] / 117.8 [107.7, 120.0]; upload cap 1 119.9 [119.1, 120.2]; cap 2 119.8 [116.9, 120.2]; GL thread USER_INTERACTIVE 119.0 [111.1, 120.1]; decoder stagger 117.8 [111.8, 119.8]; no upload 111.7 [105.9, 116.4] (`/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/agg/q1-table.txt`)
- (Q1) VERIFIED the unmodified MAIN app on w1c: fps 107.1-115.1 (15 loads, median 112.7) and 110.5-117.7 (15 loads, median 115.8). In the second set, where it was recorded, every load's median `peak_callback_ms` is 6.30-6.47 ms: the bunched frame, visible through the existing REST field (`/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/agg/main-reference.txt`)
- (Q1) VERIFIED the display is identical in all 218 instrumented launches (2,806 samples): max 120 fps, minimum refresh interval 8.333 ms, granularity 4.167 ms, mode 120 Hz, CVDisplayLink nominal period 8.333 ms, GL swap interval 1, app not active, window not key. The display link's outputTime grid is exactly 8.3333 ms in every window. The window was visible except in 6 launches (colbase r1-r5, one smoke), where it was occluded (appVisible 0, not my action); none are in the verdict sets (diag.tsv D lines / `/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/analyze.py`)
- (Q1/Q2) VERIFIED the GL render thread is JUCE's `std::thread` "OpenGL Renderer" at QoS DEFAULT (0x15), priority 31, in every launch. VideoDecode and FFmpeg's av:h264:df0/df1 threads are DEFAULT (`/Users/boriskarpman/projects/RealTimeAudio/src/media/VideoPlayer.h:207` Priority::normal -> QOS_CLASS_DEFAULT per JUCE `/Users/boriskarpman/projects/RealTimeAudio/build/_deps/juce-src/modules/juce_core/native/juce_Threads_mac.mm:51-58`); the message thread is USER_INTERACTIVE. E-cores = CPUs 0-1: a BACKGROUND calibration thread ran only there in 218/218 launches (USER_INTERACTIVE only on 2-9)
- (Q2) VERIFIED 4 x 4K video, 5 launches x 2 windows per arm, arms interleaved, two sets. Median fps: base 89.75 / 90.55; no upload 111.05 / 116.0; IOSurface + blit 104.95 / 106.85; cap 1 94.85; BGRA + 8_8_8_8_REV 89.95; BGRA + UNSIGNED_BYTE 89.5; PBO 89.25; client storage 28.0; QoS 90.05 (`/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/agg/q2-table.txt`, `/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/agg/q2b-table.txt`)
- (Q2) VERIFIED decoding the 4K fixture outside the app with ffmpeg (300 frames): software 14.2 ms CPU per frame; VideoToolbox 0.87 ms CPU per frame; 4 concurrent VideoToolbox streams 333 frames/s aggregate (`/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/agg/vtbench.txt`)
METHOD: I added TEMPORARY instrumentation to the lane worktree and to a PRIVATE JUCE copy, all saved in `instr.diff` /
`juce-instr.diff`:
- per-frame GL-thread records: entry / exit time, CPU number, thread CPU time, phase split, uploads, raw GL_TIME_ELAPSED
- JUCE display-link ticks (callback time, inNow, outputTime), flushBuffer wall time, render-thread waits
- decoder decode / sws / ring-wait records
- a 250 ms per-thread CPU / QoS / priority sampler, and display / window state every 2 s
- env-switched counterfactual arms

Launches used `open -g`, production mode, under `acquire_quiet_lock`; a launch that saw a compiler was re-run. The verdict
tables come from INTERLEAVED arms (`run_mix.sh`), because sequential per-arm batches drifted about 5 fps in an hour on the
same scene.
CONFIDENCE+VERIFY: high for the Q1 cause (pattern-decided windows, a deterministic trigger, and a counterfactual with
30/30 windows >= 119.1, plus the main-app witness). High for the Q2 upload share (two interleaved sets agree within 1.6 fps
per arm). Medium for the GPU residual share, which is taken by difference. To re-check: apply
`diag-vfps-tools/instr.diff`, build with `build.sh cfg`, then
`bash run_mix.sh q1 w1col 3 10 "base=" "cap2=ADNA_VFPS_UPCAP=2"` and `python q1table.py runs/q1_base runs/q1_cap2`.
UNKNOWNS/NOT-DONE:
- What flushBuffer waits on inside macOS. The GL thread blocks there 5-19 ms using 0.24 ms of CPU; INFERRED swap back-pressure. Seeing it needs WindowServer tracing, which the rig forbids.
- The foreground (key window) case is untested: no synthetic input allowed.
- True zero-copy sampling is unpriced. An IOSurface cannot be bound as GL_TEXTURE_2D here (CGLError 10008 -> black picture), and sampling a rectangle texture needs a compositor shader variant.
- VideoToolbox is priced outside the app (ffmpeg), not built into it.
- No probe row is committed (diagnosis only); the RED-able row is specified in section 1.
NUANCE:
- (1) "per launch" is wrong: the split is per load.
- (2) The w2 (a2) "4 x 4K stills ceiling" is not a ceiling. The stills window reads 96-120 fps depending on how often the lightly loaded DEFAULT-QoS GL thread sits on an E-core: 40-67 % of frames in the slow windows, 6-21 % in the fast ones. The no-upload VIDEO scene beats it at 111-116.
- (3) The no-upload arm LOWERS Q1 fps to 111.7: a lighter GL thread migrates to E-cores (23 % of frames, wake-up p99 2.7 ms against 0.04). "Remove the upload" is therefore not a clean test of "remove the heavy frame" on this machine.
HANDOFF-NEEDS: Harmony -- a ruling on a Q1 fix lane (per-frame video upload budget; cap 2 measured) and a Q2 lane
(IOSurface ring + blit, +15-16 fps at 4K; VideoToolbox as its second stage); whether probe-video gains row w1c (section 1)
and w2 (a2) is replaced (section 2); the notebook lines below. The worktree branch `diag/vfps` (no commits, clean) is left
for Harmony to remove.
INBOX-RECHECK: none (no addendum message arrived during this run)

Lane window: 08:40:55 - 13:40 (date-stamped). Base: main adf9b8a. 218 instrumented launches (plus tainted re-runs) + 15 main-app launches,
evidence under `scratchpad/diag-vfps/runs/<tag>/` (833 MB, raw), aggregates + tools in `diag-vfps-tools/`.

---

## 1. Q1 verdict -- bimodal "per launch"

**Answer: the cause is the relative phase of the four players' frame clocks, which is fixed per LOAD. Pattern "4" (all
four new frames due on one render frame) loses display-link ticks. It reads 104-117 fps on the instrumented build and
107-118 on main; spread phases read 117.6-120.** VERIFIED by counterfactual (upload cap), 10 launches x 3 loads per arm.

### Mechanism, step by step (each step: evidence, label)
1. **Clocks start where the clip is first drawn.** A triggered clip's player advances `currentTime_ += realDt` only on
   frames that draw it (`src/render/Renderer.cpp` syncMedia -> `VideoPlayer::advanceFrame`, `src/media/VideoPlayer.cpp:273-306`).
   Every player adds the same `realDt`, so the clocks' relative phases never change during a load.
   A 30 fps clip's new frame is due every 33.3 ms = 4 render frames at 120 Hz. VERIFIED: the uploads-per-frame
   histograms are exact, e.g. `{0: 394, 4: 151}` = 151 frames carrying all 4 uploads in 5 s (`agg/q1-bunch-sequential.txt`).
2. **The probe's trigger sequence sets the phase.** Trigger, then `wait_active` polling per layer
   (`.harmony/probe-video.py:569-571`) usually lands all four starts on the same frame mod 4. Pattern 4 occurred in 20/30
   loads, 3+1 in 6, 2 in 2, 2+1 in 1, 4+3 in 1. A single `/api/trigger_column` makes it 4 on every load. VERIFIED
   (`agg/q1-table.txt`, col_w1 / colbase / q1_* / q1b_*).
3. **Bunched frame cost.** Each 1080p `glTexSubImage2D` (GL_RGBA / GL_UNSIGNED_BYTE from the ring slot,
   `src/media/VideoPlayer.cpp:435-437`) costs 1.40 ms of GL-thread CPU (per-upload median, 1.40-1.43 across four arms). The 4-upload frame's
   renderOpenGL takes 6.1-6.4 ms (p90), against 2.1 ms (cap 1) and 3.5 ms (cap 2). VERIFIED (per-frame records, kind 10/11).
4. **Lost ticks.** JUCE triggers each frame from the CVDisplayLink callback by setting a `pendingRender` FLAG
   (`juce_OpenGLContext.cpp` renderFrame / refreshDisplayLinkConnection). Two ticks during one frame + flushBuffer therefore
   collapse into one frame. After `renderOpenGL` the GL thread blocks in `[NSOpenGLContext flushBuffer]` (swap interval 1):
   5.98 ms median wall time at 0.24 ms CPU over 495 windows, so it is waiting, not working. A heavy frame pushes that wait
   past a second tick.
   - Lost display-link ticks per s, median over windows: pattern-4 base 2.1-8.2; cap 1 0.2; cap 2 0.2. VERIFIED.
   - Where they fall: 4-6 ms after the heavy frame's start (the tick arriving during it) and 12-14 ms after it (the next
     tick, during the following flush). VERIFIED (`agg/q1-lostpos.txt`).
   - What flushBuffer waits on inside macOS: INFERRED swap back-pressure (not observable without forbidden tracing).
5. **JUCE's own frame limiter never fires.** In `NativeContext::swapBuffers` the underrun sleep ran 0 times in 495
   windows (minSwapTimeMs 8). VERIFIED.

### Counterfactual table (w1c scene = 4 x 1080p a1080, ONE `/api/trigger_column`; 10 launches x 3 loads per arm; arms interleaved)
| arm (env) | windows (pattern) | fps median [min, max] | loads >= 117 | lost ticks/s | render p90 ms | GL on E-core | GL wake-up p99 ms | uploads / 5 s |
|---|---|---|---|---|---|---|---|---|
| set 1 base | 30 (4) | 116.5 [109.4, 119.7] | 12 | 3.5 | 6.36 | 0.2 % | 0.037 | 605 |
| cap 1 (`ADNA_VFPS_UPCAP=1`) | 30 (1) | **119.9 [119.1, 120.2]** | 30 | 0.2 | 2.08 | 0 | 0.039 | **586** (skips ~3 %) |
| decoder stagger 8 ms (`DECSTAGGER=8`) | 30 (4) | 117.8 [111.8, 119.8] | 17 | 2.3 | 6.30 | 0 | 0.036 | 605 |
| GL UI + decoders UI-initiated (`QOS=1`) | 30 (4) | 117.9 [104.1, 119.7] | 20 | 2.1 | 6.34 | 0 | 0.017 | 603 |
| no upload (`NOUPLOAD=1`) | 30 (4) | 111.7 [105.9, 116.4] | 0 | 8.3 | 2.14 | **23 %** | **2.68** | 605 |
| set 2 base | 30 (4) | 117.8 [107.7, 120.0] | 21 | 2.1 | 6.13 | 0.1 % | 0.036 | 604 |
| cap 2 (`UPCAP=2`) | 30 (2) | **119.8 [116.9, 120.2]** | 29 | 0.2 | 3.48 | 0.2 % | 0.036 | **604** (none skipped) |
| GL UI only (`QOS=4`) | 30 (4) | 119.0 [111.1, 120.1] | 26 | 1.1 | 6.09 | 0 | 0.018 | 605 |
| stagger + QOS=1 | 30 (4) | 119.1 [100.7, 119.9] | 25 | 0.9 | 6.13 | 0 | 0.020 | 605 |
| no upload + GL UI (`NOUPLOAD=1,QOS=4`) | 30 (4) | 118.2 [113.7, 119.2] | 21 | 1.9 | 1.29 | 21 % | 0.32 | 604 |
video_late_frames delta 0 in every window of every arm. Every launch's per-load fps, bunch pattern, swap-interval shares
and lost ticks/s: `agg/q1-perlaunch.md` (all 15 arm sets). Two arms in full:

set 1 base, launch -> load (pattern) fps [lost ticks/s]:
r1 113.4 [6.7] / 109.7 [10.2] / 116.5 [3.6]; r2 116.9 [3.0] / 113.2 [6.8] / 117.1 [2.8]; r3 119.7 [0.2] / 119.5 [0.4] / 115.5 [4.6];
r4 119.7 [0.4] / 113.8 [6.2] / 115.6 [4.4]; r5 115.8 [4.2] / 118.8 [1.2] / 117.9 [2.2]; r6 117.7 [2.2] / 119.2 [1.0] / 112.1 [7.7];
r7 116.8 [3.4] / 119.3 [0.8] / 116.2 [3.8]; r8 113.9 [5.9] / 116.5 [3.4] / 109.4 [10.7]; r9 117.8 [2.2] / 119.1 [1.0] / 116.5 [3.6];
r10 114.9 [5.0] / 118.0 [2.2] / 115.1 [4.7] (all pattern 4).
set 2 cap 2: r1 116.9 [3.2] / 119.6 [0.4] / 119.5 [0.6]; r2 120.0 / 119.5 / 119.6; r3 119.5 / 119.9 / 119.8; r4 120.0 / 119.6 / 119.7;
r5 117.7 [2.4] / 118.8 [1.2] / 119.6; r6 120.1 / 120.2 / 120.2; r7 120.2 / 119.4 / 119.9; r8 119.9 / 120.2 / 120.0;
r9 120.1 / 120.1 / 120.0; r10 118.3 [1.8] / 119.9 / 119.3 (all pattern 2; unlabeled loads <= 0.6 ticks/s).
Histogram note: swap-to-swap intervals are bursty (flushBuffer returns in pairs), so 13-35 % of intervals are >= 2
vblanks even at 120 fps. Lost ticks, not the histogram, are the frame-loss measure.

### Hypotheses
- **H1 (cost near budget + something fixed per launch) -- PARTLY RIGHT, VERIFIED:** the "something" is fixed per LOAD and
  is the clock phase (step 1-2). The near-budget cost is the bunched frame, 6.1-6.4 ms against 8.33.
- **H2 (thread placement / QoS) -- NOT the per-load decider, VERIFIED; a minor lever.** In base, GL frames on E-cores are
  0-0.6 % in both 104-fps and 119-fps loads. UI QoS for the GL thread halves lost ticks (2.1 -> 1.1/s, median +1.2 fps)
  but leaves the tail (min 111.1). The E-core effect is real for a LIGHT GL thread (no-upload arm, section 2's stills).
- **H3 (ProMotion cadence per launch) -- RULED OUT for the background window, VERIFIED:** display and window state
  identical in 218/218 launches; exact 8.333 ms outputTime grid; cap 1 reaches 119.9 in the same `open -g` window. The
  foreground case is UNTESTED (no synthetic input). INFERRED: Boris's foreground use goes through the same in-app
  mechanism, since a key window changes neither the clock phases nor the upload bunching.
- **H4 (something in the video path set per load) -- CONFIRMED, VERIFIED:** it is the decode/clock start phase. Ring depth
  and NONREF play no part (late 0 in every steady window).
- **H5 (something else):** decoder bursts in sync add a little (stagger +1.3 median, INFERRED minor). A light GL thread's
  E-core migration costs up to 8 fps (no-upload arm). The environment drifts about 5 fps per hour (the same w1c scene read
  111.8 then 117.3 median an hour apart). UNATTRIBUTED (section 4).

**Boris's use (INFERRED):** a column / scene launch, or beat-quantized triggers (per-clip beat snap), starts every clock
on the same frame: pattern "all", the worst case, by design. The unmodified main app reads 107-116 fps on it (15 loads).
Mixed frame rates (24/25/30 fps) make the pattern drift over time, so the dips come and go.

### RED-able row (proposed for probe-video; reads RED on main)
`w1c_column_trigger_1080x4`: fresh load of 4 layers of a1080 (1080p canvas), ONE `POST /api/trigger_column {column 0}`,
wait until all 4 are active, 2 s, then a 5 s window polled every 50 ms; 3 loads per launch, 2 launches.
- PASS (a): in every load, the median over the window of `peak_callback_ms` is <= 4.0 ms. The bunched frame is gone.
  - Main: 6.30-6.47 ms in 15/15 loads (`agg/main-reference.txt`), so RED every load.
  - Prototypes, measured as renderOpenGL p90 (the same function scope): cap 2 3.48 ms, cap 1 2.08 ms (INFERRED GREEN).
- PASS (b): the median over the 6 loads of each load's median fps is >= 118.5.
  - Main: 112.7 and 115.8 in two 15-load sets.
  - Cap 1 / cap 2: 120.0 / 120.0 (poll-fps medians).
- PASS (c): `video_uploads` per 5 s window >= 595 and `video_late_frames` delta 0, so no content frame is skipped. A naive
  cap 1 fails this (586), which is the point.

## 2. Q2 attribution table -- the 4 x 4K ceiling (4K canvas, 4 layers of a4k H.264 30 fps)

Reference points (median of 10 windows, 5 launches; two interleaved sets): base **89.75 / 90.55**, no upload (c1)
**111.05 / 116.0**, display rate 120. The packet's "stills 116-120" figure is not a stable ceiling (see NUANCE): here the
stills window read 97-104 fps median in most arms and 119.5 in one.

| # | cause | where | ms per frame (median / p90) | share of the base -> 120 gap (set 1 / set 2) | counterfactual |
|---|---|---|---|---|---|
| A | GL-thread CPU of the upload: `glTexSubImage2D` from client memory (the driver copies 33 MB on the GL thread) | `src/media/VideoPlayer.cpp:435-437` (+ create path :427) | 0.86-0.96 / 1.55 ms per 4K upload; 1.35-1.46 ms per frame on average; the frame that carries the bunched uploads: renderOpenGL p90 4.64-4.95 vs 0.65-0.70 with no upload | **15.2 / 16.3 fps = 50 % / 55 %** | c4a IOSurface ring + blit: per-upload CPU 0.08-0.10 ms, fps 104.95 / 106.85, GL thread 204 -> 91-98 ms/s. VERIFIED |
| B | GPU side of getting the pixels into a sampled texture (the driver's transfer / the c4a blit) | GPU driver (Metal-backed GL); c4a: `glBlitFramebuffer` rect -> 2D | GPU p50 base 7.16-7.53 vs no-upload 7.02-7.19 (+0.14-0.34), p90 8.89-9.03 vs 7.77-7.92 (+1.1); c4a GPU p50 7.71-7.76 | **6.1 / 9.2 fps = 20 % / 31 %** (c4a -> no-upload residual) | by difference, INFERRED. No clean arm exists: GL_TEXTURE_2D IOSurface binding fails (CGLError 10008, VERIFIED), so the no-blit path could not be priced |
| C | the 4K composite itself (4 layers x 4K texture fetch + passes; not video-specific) | compositor GPU passes (`CompositorEngine` compositeDeck) | GPU 7.0-7.2 ms p50 with no upload (stills 6.7-7.1) against an 8.33 ms budget | **9.0 / 4.0 fps = 30 % / 14 %** (no-upload -> 120) | the invalid IOSURF=2 arm (empty textures) ran GPU 5.41 ms and 120.0 fps, so texture fetch of 4 real 4K layers is ~1.6 ms. INFERRED |
| (A') | the bunching of A (Q1 mechanism) | as Q1 | 4 x 0.96 ms on one render frame | overlaps A | cap 1: 94.85 (+5.1), pattern 1, render p90 2.11. VERIFIED |
| -- | upload pixel format / CPU swizzle (packet hypothesis c2) | -- | per-upload 0.96 / 0.97 ms in BGRA + REV / BGRA + UBYTE vs 0.96 base | **0** | bgra1 89.95, bgra2 89.5. VERIFIED not a cause |
| -- | PBO double buffer on the GL thread (c3b) | -- | memcpy 1.5 ms per frame on the GL thread + the same driver upload 1.0 ms | **0** | pbo 89.25. VERIFIED no gain |
| -- | Apple client storage + shared hint (c3) | -- | upload call 0.01 ms, but renderOpenGL costs 25.2 ms CPU EVERY frame (the driver re-reads client memory at draw) | **-62 fps** | client 28.0. VERIFIED trap |
| -- | decode threads (4 x 4K30 software H.264 + sws) | `VideoPlayer::decodeLoop` + FFmpeg frame threads | FFmpeg frame threads 1.73-1.78 cores; VideoDecode 0.47-0.64 cores (sws 4.1-4.7 ms per 4K frame); about 2.5 of 8 P-cores for the whole app | **0 at the measured load** | GL thread not descheduled (wake-up p99 0.03 ms, on E-cores 1 %); QOS=1 90.05. VERIFIED not a cause |

Shares add to 100 % per set (A + B + C = 30.25 / 29.45 fps). A and the c4a gain are VERIFIED counterfactuals, B is by
difference, C is the residual to the display rate. Everything is in `agg/q2-table.txt` / `agg/q2b-table.txt`.
Correctness of c4a: the canvas captured through `render_frame` is byte-identical to the shipped path at the same frame
(codes 211 and 212: max |diff| 0), 3 captures. VERIFIED.

## 3. Fix options (NOT implemented), ranked by fps gained per unit of risk

1. **Q1 -- per-frame video upload budget (spread the picks across render frames).** Measured by the cap 2 prototype.
   - Gain: +2.0 / +3.4 fps median; minimum 107.7 -> 116.9; lost ticks 2.1 -> 0.2/s; 0 frames skipped at cap 2. At 4K,
     cap 1 adds +5.1.
   - Risk: low.
   - Constraints:
     - Pitfall 56: the render thread never waits. A deferred player HOLDS its shown frame (never 0, never pending). A
       never-shown player is exempt, so first frames and the C1 / C3 gates are untouched.
     - The budget must adapt to demand: cap 1 has zero slack at 4 x 30 fps / 120 Hz and skipped ~3 % of content frames.
       Use a floor of ceil(new frames per render frame) + 1 and rotate priority so no layer starves.
     - Share or mirror the existing per-frame `uploadBudget_` used for images (`Renderer.cpp`, `uploadBudget_.reset()`
       at the frame top; Pitfall 53).
     - Pitfall 35: a crossfading layer has two live players; both count, neither starves the other.
   - Gate:
     - the new `w1c` row: RED on main (peak_callback 6.30-6.47 ms) -> GREEN;
     - w1 (a), w5 frame accuracy, w6 (b) upload count 130-180, w3 / w4 holds;
     - a pure ctest for the budget / rotation.
2. **Q2 -- IOSurface-backed ring slots + one GPU blit per new frame (c4a).** Measured.
   - Gain: +15.2 / +16.3 fps at 4 x 4K (90 -> 105-107); GL thread CPU halves; byte-identical picture.
   - Risk: medium.
   - Constraints:
     - GL upload stays on the GL thread (the blit), only when the frame is new, and never waits. A slot returns to the
       writer only when its blit fence signals (`glClientWaitSync` with timeout 0 polled at the pick), so the writer may
       see one slot fewer for ~1 frame.
     - The effect chain keeps its sampler2D texture: GL_TEXTURE_RECTANGLE is only the blit SOURCE (OpenGL 4.1 core:
       `glBlitFramebuffer`, `glFenceSync`).
     - Per-player GL objects stay bounded: 3 rect textures + 3 FBOs + 1 destination FBO (the Pitfall 54 rule).
     - `releaseGL` / context loss must delete them and release the fenced slots (V1 "shown before" unchanged).
     - sws writes into a locked IOSurface: decode-thread sws per 4K frame goes from 4.1-4.4 ms to 4.7-5.4 ms.
     - Memory is equal to today's 3 malloc'd slots.
   - Gate:
     - probe-video w2 (a) with a raised bar (>= 100);
     - w5 frame accuracy + the video lane's byte-identity row (S-vid == S-img);
     - w3 / w3b / w4 / w4b / w6 / w6b / w9 unchanged;
     - ctest (VideoRing + a fence-release unit), TSan.
3. **Q1 / general -- GL render thread QoS USER_INTERACTIVE** (`pthread_set_qos_class_self_np` on the first
   `renderOpenGL`: JUCE creates the thread as a plain `std::thread`).
   - Gain: +1.2 median on w1c; wake-up p99 0.036 -> 0.018 ms. It does NOT fix the tail and does NOT keep a light GL thread
     off the E-cores: stills windows under QOS=1 still had 40-50 % E-core frames.
   - Risk: very low.
   - Gate: w1c + idle-paint rows (message-thread CPU unchanged).
4. **Q2 stage 2 -- VideoToolbox hardware decode to NV12 IOSurfaces + a YUV -> RGB shader blit.** Priced outside the app
   (INFERRED in-app).
   - Gain: frees ~2.2 cores at 4 x 4K (software 14.2 ms CPU per frame vs VideoToolbox 0.87; media engine 333 4K frames/s
     aggregate for 4 streams). No fps gain at the measured load, since decode is not a cause (table row "decode threads");
     the gain is headroom under a busy machine / more layers.
   - Risk: medium-high:
     - HAP / ProRes 4444 alpha / 10-bit stay software;
     - VT session errors need a software fallback;
     - mid-GOP seek + NONREF catch-up semantics change (Pitfall 56 hold rules must still hold);
     - two-plane textures need a shader instead of `glBlitFramebuffer`.
   - Gate: as option 2, plus w3 / w3b hold lengths re-baselined.
5. **Q2 -- sample the IOSurface rectangle texture directly (no blit).**
   - Gain: at most option 2's residual B (6-9 fps, INFERRED upper bound).
   - Risk: high: every clip-entry pass needs a sampler2DRect variant with pixel coordinates (Pitfall 39: `layer_transform`
     is one program shared by clip and layer transforms). GL_TEXTURE_2D binding is impossible here (CGLError 10008).
6. **Q1 alternative -- offset the clock phases at trigger** (e.g. player i starts i/4 of a content frame late).
   - Gain: INFERRED equal to option 1 for same-rate clips.
   - Risk: mixed frame rates drift back into coincidence, and it shifts cue timing by up to 1 render frame.
**Do not:** BGRA / 8_8_8_8_REV (0), a PBO filled on the GL thread (0), Apple client storage (-62 fps), decoder-only QoS
changes (0 at 4K, +1.3 at 1080p).

## 4. What remains unattributed
- **Q1, within pattern 4:** per-load fps still spans 104-120. Heavy frames that start ~0.25 ms after a vblank (right after
  a flushBuffer return) go with the lowest fps (`agg/q1-heavyphase-base_w1.txt`). This is a correlation, not tested by a
  counterfactual.
- **Environment drift:** the same w1c scene read 111.8 then 117.3 median an hour apart (other lanes' load avg 3.5-10.8,
  WindowServer / GPU activity outside the app). Handled by interleaving; its cause is unattributed. colbase r1-r5 ran
  with the window occluded (appVisible 0), but their median roughly equals the visible r6-r10 (~117.5 vs ~117.2), so
  occlusion is not the explanation (INFERRED).
- **flushBuffer:** what it blocks on (5-19 ms at 0.24 ms CPU): INFERRED swap / drawable back-pressure.
- **Q2 B:** the GPU-side upload cost is by difference; no arm isolates it.
- **Q2 C:** why the 4K composite needs 7 ms GPU (per-pass split not measured; texture fetch ~1.6 ms INFERRED).
- **Stills placement:** why the scheduler keeps a light GL thread on E-cores 40-67 % of frames in some stills windows
  (96-101 fps) and 6-21 % in others (110-120). Correlation only; a UI QoS does not prevent it.

## 5. Reusable tools (paths fixed to this session's scratchpad; `diag-vfps-tools/README.md`)
`/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0929/diag-vfps-tools/`:
- Instrumentation: `patch_juce.py` (CRLF-safe JUCE hooks), `patch_app.py` / `patch_app1b.py` / `patch_app2.py` /
  `patch_app3.py` / `patch_app4.py`, and `instr.diff` + `juce-instr.diff`, which are authoritative.
- Build and launch: `build.sh`, `fixtures.sh`.
- Runners: `run_arm.sh` (sequential) and `run_mix.sh` (INTERLEAVED arms; use this one for any A/B on this shared rig).
- Driver: `drive.py` (w1 / w1c / w2 / w2v / still1080 scenes, CLOCK_UPTIME_RAW window stamps, optional render_frame
  capture + code-band decode).
- Analyzers: `analyze.py`, `timeline.py`, `bunch.py`, `heavyphase.py`, `lostpos.py`, `q1table.py`, `q2table.py`,
  `perlaunch.py`.
- Pricing and cleanup: `vtbench.sh` (ffmpeg software vs VideoToolbox under the live lock), `cleanup.sh`.
- Aggregates cited above: `agg/`.

## Rig compliance / deviations
- Worktree only, absolute paths, no `cd` in commands, and scripts under `scratchpad/diag-vfps/tools/`. JUCE was
  instrumented in a PRIVATE copy (`scratchpad/diag-vfps/juce-src`); the main checkout's `build/_deps` was used read-only.
- One Audio-DNA at a time, launched `open -g`, always under my lock (`acquire_quiet_lock`). Output-named windows were 0
  after every launch; UserNotificationCenter windows were 0 after every launch (no crash, no dialog).
- Never used: lldb / dtrace / sample / Instruments / spindump, screen capture, synthetic input, or defaults writes.
- Perf numbers only without compilers: tainted launches were re-run (q1 1, q1b 1). Load average is printed per launch in
  the run meta files.
- **Deviations:**
  - (1) I stopped two of my own sequential batches while they waited for quiet, without holding the lock, and replaced
    them with interleaved runs after finding ~5 fps/hour drift. The sequential results (base_w1, cap1_w1, noup_w1, col_w1,
    colbase; first binary) are kept only as supporting data.
  - (2) Four rebuilds of build-lane during the lane may have made other lanes wait for quiet.
  - (3) Removed `build-lane/_deps/juce-build` once (my own build dir): it cached the private JUCE path and blocked the
    clean reconfigure.
- **End state (13:36:29):**
  - `git -C .../rta0929-vfps status` = `?? build-lane/` only;
  - build-lane rebuilt from the clean tree against the main checkout's JUCE (binary 18,251,072 B, the same size as main's
    app); `strings | grep -c -E 'DIAG-VFPS|ADNA_VFPS_|DiagVfps'` = 0;
  - my lock released;
  - no app I launched is running (the Audio-DNA then running was asyncload's, under their lock);
  - 0 Output-named, 0 UserNotificationCenter windows.
  - All fixtures and captures were written under `scratchpad/diag-vfps/` (media/, runs/); nothing went under ~/Documents
    or ~/Library.

## found_not_fixed
1. probe-video w1 (a) is really per load: its trigger sequence decides the bunch pattern, so its 3/5 "flake" is the
   product behaviour, not noise. w2 (a2) compares against a stills ceiling that floats 96-120 with GL-thread placement.
   Neither row can RED a fix; section 1's w1c can.
2. JUCE's display-link pacing drops a tick whenever renderOpenGL + the flushBuffer wait spans two vblanks (flag, not
   counter). Any frame spike costs a frame, not just a late frame.
3. The GL render thread runs at QoS DEFAULT (JUCE `std::thread`). With a light load it spends 20-67 % of frames on
   E-cores, with wake-up p99 1-7 ms (stills window, no-upload arm): about -10 to -20 fps on light 4K scenes. INFERRED
   risk for any light scene.
4. A naive upload cap skips content frames at zero slack (cap 1: 586 of ~605 uploads per 5 s).
5. `CGLTexImageIOSurface2D` onto GL_TEXTURE_2D fails on this Mac (CGLError 10008). IOSurface zero-copy must use
   rectangle textures.
6. Apple client storage (`GL_UNPACK_CLIENT_STORAGE_APPLE` + `GL_STORAGE_SHARED_APPLE`) makes every draw re-read client
   memory: 25 ms CPU per frame.

## Notebook lines for Harmony (`.harmony/notebook.md`)
- `## 2026-09-29 multi-video fps is set per LOAD by clock phase | 4 videos at 30 fps on 120 Hz: if their clocks start on the same render frame mod 4 (column trigger, beat-quantized triggers) all 4 uploads land on one frame (6.3 ms GL CPU) and ~2-8 display-link ticks/s are lost (104-117 fps); spread = 119-120; peak_callback_ms median 6.3-6.5 is the REST-visible witness | discovered: src/media/VideoPlayer.cpp:435, .harmony/.reports/s-rta-0929/diag-vfps.md`
- `## 2026-09-29 JUCE mac GL pacing = display-link FLAG + blocking flushBuffer | pendingRender is a flag, so two CVDisplayLink ticks during one renderOpenGL + flushBuffer collapse into one frame; flushBuffer blocks 5-19 ms at ~0.2 ms CPU (swap back-pressure) -- measure lost ticks, not swap-interval histograms (bursty) | discovered: juce_OpenGLContext.cpp renderFrame / refreshDisplayLinkConnection`
- `## 2026-09-29 GL render thread is QoS DEFAULT and drifts to E-cores when light | JUCE's OpenGL Renderer is a std::thread (QoS 0x15, pri 31); with little work (stills, no uploads) 20-67 % of its frames run on CPUs 0-1 (E) with wake-up p99 1-7 ms and fps drops 10-20; UI QoS halves lost ticks but does not prevent E placement | discovered: diag-vfps diag.tsv C lines`
- `## 2026-09-29 4K upload cost is GL-thread CPU, not format | glTexSubImage2D RGBA/UBYTE of a 4K frame = 0.86-0.96 ms GL CPU; BGRA/8_8_8_8_REV, PBO and QoS gain 0; client storage = 25 ms/frame trap; IOSurface ring + glBlitFramebuffer = +15-16 fps byte-identical; IOSurface cannot bind to GL_TEXTURE_2D (CGLError 10008) | discovered: src/media/VideoPlayer.cpp:435`
- `## 2026-09-29 A/B perf on the shared rig drifts ~5 fps/hour | run arms INTERLEAVED launch by launch (diag-vfps-tools/run_mix.sh), never as sequential per-arm batches | discovered: col_w1 111.8 vs colbase 117.3, same scene`

## PACKET QUALITY
- Clarity: HAD_TO_INFER. The packet's ESTABLISHED "per-launch not per-frame" was contradicted by the probe's own source:
  w1's 5 runs are 5 loads in one launch. I treated it as a hypothesis and tested it (per load, VERIFIED).
- Missing context: that w1's repeats share a launch (`probe-video.py:587-590`); that the stills ceiling itself depends on
  GL-thread placement; that JUCE's mac GL loop is display-link-flag paced (read from juce-src).
- Unused context: the old e9ff9dc6 tools were used as templates only (the DiagIdle recorder / sampler pattern, run_arm
  structure); diag-media tools were not needed.
- Self-brief files: probe-video.{sh,py,json}, video.md (s-rta-0928b), s-rta-0928b / s-rta-0929 work logs,
  docs/claude/pitfalls.md (35, 53, 54, 56), CLAUDE.md, lock.sh. All existed and were useful. `.harmony/notebook.md` was
  not read (size guard; not needed for a diagnosis).
- Knowledge tools: none in the packet; grep + source reads; no deletions.
