# diag-media (s-rta-0928b): media decoding and file I/O on the hot threads, ranked

STATUS: DONE
INBOX-RECHECK: none

RESULT: Video on the GL render thread (S1) is the biggest media cost by far, and it hurts in two ways.
- **S1b, a re-seek loop.** Any seek that lands more than ~3 s past a keyframe (retrigger, beat-snap, cue, outPoint loop, deck return) makes `VideoPlayer::decodeFrameAtTime` re-seek to that keyframe and decode 30 frames on every render frame, until the clock reaches the next keyframe.
  - For 3.3-3.5 s the whole output runs at 14 fps (1080p: 67 ms frames) or 3.8 fps (4K: 265 ms frames).
  - The video stays frozen for that time.
- **S1a, steady playback.** Convert + flip + a re-upload of the unchanged frame on every render frame.
  - 4 × 1080p: 93 fps at 120 Hz.
  - 4 × 4K: 36 fps.

The message-thread sites are bounded, per-event holds:
- **S2, video open + thumbnail.** 26 ms per 1080p clip and 99 ms per 4K clip: 0.42 s to load 16 × 1080p and 1.58 s for 16 × 4K. A media drop runs the open inside the GL fence, so the output goes black for 5-14 frames.
- **S3, sequence open + a second frame-0 decode for the thumbnail.** 128 ms for two 300-frame sequences.

S4 (a stat per image layer per GL frame, 0.02 ms per frame) and S5 (a stat per media-cell paint, 4.3 ms/s) are real but minor on a local SSD.

Every per-site number comes from ≥ 5 clean launches per arm, and each attribution has a counterfactual arm (the same binary with one behaviour switched off by an environment variable).

## FACTS (evidence root: SP = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media)

**Rig and baseline**
- Machine: M1 Pro, 10 cores, 32 GB. The built-in display runs at 120 Hz (VSync 8.33 ms). Measured idle frame interval: median 8.3 ms, 116-120 fps.
- Base: main b9c9ab2. Worktree branch diag/media, now clean again.
- Arms (tag prefix, environment flags, clean launches):
  - v0: base; 5.
  - v1: DIAG_VID_NODECODE + DIAG_NO_THUMB; 5 (v1_3 and v1_6 discarded because a compiler ran; v1_7 added).
  - v2: DIAG_VID_UPLOAD_ONLY_NEW; 5.
  - k0: seek; 5 (k0_3 discarded; k0_6 added).
  - i0: images; 5.
  - i1: DIAG_NO_EXISTS + DIAG_NO_PAINTSTAT; 5.
  - q0: sequences; 5.
  - q1: DIAG_NO_THUMB; 5.
- A background sampler checked for a compiler every 2 s during each launch; any sample voids the launch (`runs/*/compiler-samples.txt`).
- Load average at launch start was 3.1-6.8, printed in `runs/*/meta.txt`. Other lanes were active; no compiler ran during any counted launch.
- Same-launch no-media baseline (`idle_empty` phase, median of 5 launch values):
  - GL callback median 0.60 ms (p90 0.81-0.87).
  - Heartbeat max 13-21 ms; 0-2 stalls over 16 ms per 6 s.
- A grid holding media adds a background message-thread stall of 16-28 ms, 12-21 times per ~3 s. This is the diag-idle lane's stall.
- Message-thread holds below are therefore given as the site's own scope time (measured directly), with the heartbeat max next to it for comparison.

**S1b: the re-seek loop (VERIFIED)**
- Evidence: k0, 5 launches, `runs/seekstats.txt`. Fixture: a 10 s clip, x264 default GOP (keyframes at 0 and 8.33 s; 4K at 0 and 8.07 s), inPoint 0.5.
- Retrigger to 5.0 s:

| clip | frames in the loop | each frame | per event | output fps |
|---|---|---|---|---|
| 1080p | 47-48 | 66.7-68.5 ms (max 68-102) | 3254-3314 ms | ~14 |
| 4K | 14 | 263.6-268.9 ms | 3469-3559 ms | 3.8 |

- The playhead runs 0.50 → 0.83, which is the next keyframe. The picture stays on keyframe + 29 frames the whole time.
- Deck return (3.5 s away) enters the same loop: 67-69 ms per frame (1080p), 264-270 ms (4K).
  - Uninterrupted duration: 3673 ms (smoke3, 1 launch).
  - In the matrix the scripted retrigger cut it at 1.45-1.50 s and started a fresh 3.3-3.6 s loop.
- Control (GOP-30 clip, same launches): no loop. Deck return costs one 48.5-59.8 ms frame; a retrigger never exceeds 14 ms.
- The message thread is dragged in too. A trigger during the loop waits on `videoPlayerMutex_` in `getVideoPlayer`: 25.4-35.1 ms (1080p, 4 launches), 68-120 ms (4K, 5 launches). Outside the loop the wait is 1.0-1.2 ms.

**S1a: steady video playback (VERIFIED)**
- Evidence: pooled per-call values, 5 launches, steady windows, `runs/pool-v.txt`.
- Per-call cost (median / p90 / max):

| clip | decoding call | sws | flip | upload | upload-only call |
|---|---|---|---|---|---|
| 1080p H.264 | 2.13 / 2.34 / 7.5 | 0.49 | 0.39 | 1.2-1.36 | 1.39 / 1.50 / 6.8 |
| 4K H.264 | 4.62 / 4.89 / 28.2 | 1.91 | 1.73 | 0.86 | 0.84 |
| ProRes 422 10-bit 1080p | 5.66 / 5.92 / 15.3 | 4.04 | 0.37 | 1.31 | 1.36 |

- On the 1080p decoding call, the packet feed + `avcodec_receive_frame` itself is only 0.045 ms (thread_count 2, `VideoPlayer.cpp:103`). The decode runs on FFmpeg's own worker threads.
- At 4 × 4K those workers fall behind and the GL thread blocks inside receive: decoding-call p90 20.6 ms, max 54 ms, up to 4 decodes per call.
- `frameReady_` is never cleared after an upload (`VideoPlayer.cpp:340`), so each 30-fps clip is uploaded 120 times a second. Measured: 603 uploads against 151 decodes.
- GL callback per frame (median of launch medians / p90 / fps), `runs/agg-v.txt`:

| state | base v0 | v1: no decode (uploads kept) | v2: upload only new frames |
|---|---|---|---|
| loaded, idle | 0.56 / 0.66 / 119.8 | — | — |
| 1080p × 1 | 2.07 / 2.91 / 119.8 | — | — |
| 1080p × 4 | 6.82 / 9.13 / 93.2 (150 frames over 8 ms per 5 s) | 6.33 / 6.52 / 117.2 (frames over 8 ms: 1) | 2.66 / 7.02 / 104.1 |
| 4K × 1 | 1.49 / 5.47 / 117.4 | — | — |
| 4K × 2 | 2.67 / 9.81 / 109.7 | — | — |
| 4K × 4 | 12.36 / 81.7 / 36.0 (70 intervals over 20 ms, 23 over 34 ms per 5 s) | 4.43 / 4.82 / 95.9 | 9.95 / 40.1 / 46.0 (no fix) |
| ProRes × 1 | 1.98 / 6.54 / 118.8 | — | — |

- Attribution from the two counterfactuals:
  - The decode+convert+flip path owns the frames over 8 ms and the 4K × 4 collapse.
  - The redundant re-upload owns most of the 1080p median.

**S2: video open + thumbnail on the message thread (VERIFIED)**
- Evidence: pooled n = 80 per size.
- Per clip (median / p90 / max):
  - 1080p: 25.7 / 26.9 / 37.7 ms. `VideoPlayer::open` 17.8 ms (first-frame decode+convert 10.9), `getThumbnail` 7.9 ms.
  - 4K: 99.0 / 101.2 / 103.0 ms. Open 66.6 (first frame 42.5), thumbnail 31.9.
  - ProRes: 39.5 ms (open 21.9, thumbnail 8.4).
- `loadComposition` over 5 launches:
  - 16 × 1080p: 423 ms (p90 435); heartbeat max 419-433.
  - 16 × 4K: 1583 ms (1577-1617); heartbeat 1577-1615.
  - Mixed 16 video + 16 image: 404 ms. The images add nothing, because their thumbnails decode off-thread.
- NO_THUMB counterfactual: 293 ms / 1108 ms, so the thumbnail is 31% / 30% of the load.
- The GL thread keeps rendering the old composition during the open (99-120 fps in the load phases). Every load then shows exactly 1 deck-less black frame at the swap (5/5).
- Swap costs rise when videos are playing (load_prores after the 4 × 4K phase):
  - Fence wait: 85 ms median (p90 93-142), against 4.1 ms with NODECODE.
  - `closeMediaForClip`: 4.0 ms median, p90 10.6, max 20.9 per player.
- Drops go through `applyFileDrop` inside `withDeckDetached`, where the renderer has no deck and renders black:

| dropped file | mutation inside the fence | black output frames | heartbeat max | with NO_THUMB |
|---|---|---|---|---|
| 4K video | 106.6 ms (open 75.8 + thumbnail 30.8) | 14 (13-14) | 117 ms | 71.7 ms, 10 frames |
| 1080p video | 28.9 ms | 5 (5-6) | — | 23.0 ms, 4 frames |
| 4K image | 0.016 ms | 2 (0-2) | — | — (the fence alone) |

**S3: sequence open + thumbnail on the message thread (VERIFIED)**
- Loading two 300-frame 1080p sequences (noisy PNG + JPEG): 128 ms (p90 134).
  - PNG clip 98.5 ms = open 58.2 (300 stats 1.5, frame-0 decode 55.7) + a second frame-0 decode for the thumbnail, 40.8.
  - JPEG clip 26.0 ms (frame-0 14.0, thumbnail 10.5).
  - NO_THUMB: 79.6 ms.
- Dropping 300 JPEGs: 27.1 ms inside the fence (thumbnail 13.0 + open 13.2) = 4-5 black frames. NO_THUMB: 14.8 ms, 3-4 frames.

**S4: stat() per frame on the GL thread (VERIFIED)**
- 4 image layers → 5 stats per frame (one per layer + one in `clipHasContent`).
  - 4.2 µs median / 6.8 p90 / 23.6 max per stat = 0.023 ms per frame, 2.8 ms/s.
- With videos decoding, a stat costs 26-30 µs median, with single outliers of 1.0-1.55 ms.
- NO_EXISTS counterfactual: callback median 0.475 → 0.464 ms, which is inside the noise.

**S5: stat() per cell paint on the message thread (VERIFIED)**
- Every visible ClipCell repaints at the 30 Hz UI rate in every phase, idle included (29-30 paints/s per cell).
- With 24 visible media cells: 700 paints/s and 700 stats/s.
  - 5.1 µs median / 8.2 p90 / 42.7 max per stat = 4.3 ms/s.
  - Paint 0.083 ms per cell (p90 0.092), 59.4 ms/s in total.
- NO_PAINTSTAT counterfactual: 55.2 ms/s (−4.2 ms/s, the stat time). The heartbeat did not change.

**ImageSequence memory (VERIFIED, RSS by `ps`; unified memory counts GL textures)**
- One pass of the 300-frame PNG sequence: 231.7 → 2961 MB (+2.73 GB).
- Both sequences: 5354 MB.
- 558 MB after the composition swap.
- Per-frame costs: upload 2.04 ms median on the GL thread; off-thread decode 44.6 ms per frame (noisy PNG), 12.9 ms (JPEG).

## METHOD
- **Instrumentation (temporary).** Saved as `SP/instr.diff` (tracked files + the new DiagTrace.h). The generators are `SP/tools/instr_{vp,rd,seq,mc}.py`. It adds:
  - `[GF]`: one line per GL frame, printed on every return path. It carries the whole-callback ms, frame-window ms, `compositeDeck` ms, deckActive, and per-frame accumulators for video, stat and sequence work.
  - `[GV]`: one line per video `syncMedia` call, with the split into packet+receive, seek, sws, flip and upload.
  - `[CP]`: one line per ClipCell paint, with stat count and ms.
  - Message-thread scopes on loads, opens, thumbnails, sequence open, drops, the fence and triggers. Mutex-wait scopes on `getVideoPlayer`, `closeMedia` and the open-insert lock.
  - The s-rta-0928 heartbeat (a callAsync ping every 2 ms, latency logged when ≥ 2 ms).
  - A TEMPORARY `/api/diag/media` hook, active only when `DIAG_MEDIA_HOOK=1`. It posts phase marks and calls the same handlers a Finder drop reaches (`handleFileDrop` / `handleMultiFileDrop`). No synthetic input was used.
  - Five counterfactual switches read once from the environment: `DIAG_VID_NODECODE`, `DIAG_VID_UPLOAD_ONLY_NEW`, `DIAG_NO_THUMB`, `DIAG_NO_EXISTS`, `DIAG_NO_PAINTSTAT`.
- **Fixtures** (`tools/fixtures.sh`, ffmpeg testsrc2):
  - 1080p30 and 4K30 H.264, 10 s, default GOP 250, plus a 1080p GOP-30 control.
  - ProRes 422 HQ-profile-2 10-bit 1080p.
  - 300-frame 1080p PNG (noise alls=12) and JPEG sequences.
  - 16 distinct 4K JPEG images.
  - 16 video cells each for 1080p and 4K, as APFS clones.
- **How a sequence is recognised.** A multi-file Finder drop of 3 or more images becomes an ImageSequence (`ClipCell::filesDropped` → `applyMultiFileDrop`). A folder drop is ignored, and `openDirectory` is unused. Compositions store `sequenceFiles` explicitly.
- **Driver** (`tools/drive.py`, REST 7070, `Connection: close`). Four scenarios, each starting with a same-launch empty-composition baseline:
  - video: 16 × 1080p and 16 × 4K grids at 1 / 4 and 1 / 2 / 4 playing layers, ProRes × 1;
  - seek: retrigger, deck away/back and a trigger during the catch-up, for GOP-250 1080p, GOP-30 1080p and GOP-250 4K;
  - images: a 32-cell grid (16 × 4K JPEG + 16 × 1080p video), triggers, 20 refreshes, forced repaints;
  - seq: two 300-frame sequences, then drops of a 300-file sequence, a 4K video, a 1080p video and a 4K image.
- **Launches.** Only through `open -g` under the lock helper, in batches of up to 8 launches (~11 min per hold). `tools/batch2.sh` and `tools/matrix.sh` interleave the arms. `outwins` read "Output-named 0" after every launch.
- **Analysis.**
  - `tools/analyze.py`: per-phase stats for one launch.
  - `tools/aggregate.py`: median across launches, per-launch values listed, contaminated launches excluded.
  - `tools/pool.py`: per-call distributions pooled across clean launches.
  - `tools/seekstats.py`: the loop events.
  - Outputs: `SP/runs/agg-{v,k,iq}.txt`, `pool-{v,k,iq}.txt`, `seekstats.{txt,json}`, `aggregate-*.json`.
- **Revert.** `git apply -R` of my own diff, then removed `src/diag/`, rebuilt build-lane from the clean tree, and ran a strings check (0 for every marker).

## CONFIDENCE + VERIFY
- **High:** the S1b mechanism and its costs (5/5 launches and a same-launch GOP control), the S1a split (two counterfactual arms), the S2/S3 scope costs (these are the site's own code timed directly; per-launch spread is small), and the black frames during drops.
- **Medium:** S4 and S5. Their costs are tiny on a local APFS SSD, and the counterfactual deltas sit at the noise floor for S4. The slow-volume risk is INFERRED.
- **To re-verify:**
  1. `git -C W apply SP/instr.diff` (DiagTrace.h is included as a new file).
  2. Build.
  3. `bash SP/tools/matrix.sh 1 5` with `APP` pointing at the build.
  4. `python SP/tools/aggregate.py v0_ v1_ v2_` (and pool.py, seekstats.py).

## UNKNOWNS / NOT MEASURED
- ffmpeg/x264 default GOP is 250. The share of real VJ files whose keyframe spacing exceeds ~3 s is ASSUMED to be large; camera and phone files often use 0.5-2 s GOPs and would not loop.
- 4K GOP-30 deck-return hitch: not measured. INFERRED ~4× the 1080p 55 ms.
- Camera frames (`texMgr_.uploadImage(pendingCameraFrame_)`, `Renderer.cpp:323`, convert + upload on GL) and MilkDrop preset load (`projectm_load_preset_file` on GL, `ProjectMSource.cpp:139`) were read in code but not measured (no camera use; no preset fixtures).
- With 2+ active video layers the message-thread heartbeat median rises from 7 to 16 ms even with decode disabled (v1) or uploads trimmed (v2). It is not attributable to any site here, so it is handed to diag-idle.
- q1_5 had two GL frames of 237 and 253 ms with no media work in them, while ~5 GB of sequence textures were resident. Unattributed; INFERRED driver/VRAM pressure (1 of 10 launches).
- The report file was not written, because the harness refuses report-file writes. The full text is here.

## Ranked table

**Scoring.** Score = C × F × V.
- C = measured cost per event in ms. For per-frame sites: extra GL ms per frame over the loaded-idle 0.56 ms; per stat for S4/S5.
- F = events per minute. These are ASSUMED session models:
  - Moderate show: 2 × 1080p video layers playing 50% of the time, 2 image layers, 1 long-GOP seek/return per minute, 1 composition load (16 video cells) per 20 min, 1 media drop per 10 min, 24 visible media cells.
  - Heavy show: 4 video layers playing 100% of the time.
- V = visibility:
  - 3: the output picture breaks (frozen, black, or under 30 fps);
  - 2: visible stutter (dropped frames at 120 Hz);
  - 1: UI freeze only (the outputs keep rendering);
  - 0.3: invisible, headroom only.

| rank | site | file:line | thread | trigger & frequency | measured cost (med / p90 / max) | impact vs 8 ms budget / 8.33 ms (120 Hz) and 16.67 ms (60 Hz) VSync | user-visible symptom | score (arithmetic) |
|---|---|---|---|---|---|---|---|---|
| 1 | S1b re-seek / catch-up loop | VideoPlayer.cpp:441, :448, :469 via Renderer.cpp:1619 | GL | seek > ~3 s past a keyframe: retrigger, beat-snap, cue, outPoint, deck return; F = 1/min | 1080p 67.1 / 68.5 / 102 ms per frame × 47-48 frames = 3.3 s; 4K 264 / 269 / 319 ms × 14 = 3.5 s | 8× (1080p) and 32× (4K) the 8.33 ms period; misses the 16.67 ms VSync too | the whole output runs at 14 fps / 3.8 fps for ~3.4 s with the video frozen; message-thread triggers stall 25-120 ms | 1080p 3300 × 1 × 3 = **9,900**; 4K 3500 × 1 × 3 = 10,500 |
| 2 | S1a steady decode + convert + flip + per-frame re-upload | VideoPlayer.cpp:537, :544-555, :340/:362; Renderer.cpp:1577-1642 | GL | every render frame × playing video layers (30 decoding + 90 upload-only calls/s per clip) | 1080p decoding call 2.13 / 2.34 / 7.5, upload-only 1.39; 4K 4.62 / 4.89 / 28.2; ProRes 5.66 / 5.92 / 15.3; callback 1080p × 4 6.82 / 9.13 / 25.8; 4K × 4 12.4 / 81.7 / 91.8 | 1080p × 4: 150 frames over 8 ms per 5 s (93 fps); 4K × 4: 51 frames over 16.67 ms per 5 s (36 fps) | judder at 4 × 1080p; 4 × 4K is unplayable (36 fps) | moderate 2 × 1080p: (2 × 1.51) × 3600 × 0.3 = **3,240**; heavy 4 × 1080p: 6.26 × 7200 × 2 = 90,144; 4 × 4K: 11.8 × 7200 × 3 = 254,880 |
| 3 | S2 video open + thumbnail at load | MainComponent.cpp:2913-2944 → VideoPlayer.cpp:56-179, :389-402 | message | each video clip at load / append / duplicate / undo; F = 1 load per 20 min | per clip 1080p 25.7 / 26.9 / 37.7, 4K 99.0 / 101.2 / 103.0; 16 × 1080p load 423 / 435; 16 × 4K 1583 / 1617 | UI frozen 0.42-1.6 s (heartbeat 426 / 1582 ms against an idle max of 13-21 ms); the GL output keeps running; 1 black frame at the swap | UI freeze on every composition / deck load | 16 × 4K 1583 × 0.05 × 1 = **79**; 16 × 1080p 423 × 0.05 × 1 = 21 |
| 4 | S5 cell-paint stat | ClipCell.cpp:206 | message | each visible media cell × 30 Hz = 700/s at 24 cells | 5.1 / 8.2 / 42.7 µs; 4.3 ms/s | 0.43% of the message thread | none on SSD (a slow volume would stall paints, INFERRED) | 0.0051 × 42,000 × 0.3 = **64** |
| 5 | S2-drop: video open inside the GL fence | MainComponent.cpp:4847-4900 inside :4910 (UndoService.cpp:66, :89) | message; the GL renders deck-less | each video drop / Replace Content; F = 0.1/min | 4K 106.6 / 109.0 ms mutation → 14 black frames; 1080p 28.9 / 33.3 → 5 black frames | ~117 ms / ~42 ms of black on every output | **the projector goes black** when media is dropped mid-show | 4K 117 × 0.1 × 3 = **35**; 1080p 42 × 0.1 × 3 = 12.5 |
| 6 | S4 GL stat per image layer | CompositorEngine.cpp:1076, 1186, 1234, 1238, 1342, 1583 | GL | 1 per Image layer per frame + 1 per deck scan (600/s at 4 layers) | 4.2 / 6.8 / 23.6 µs (images); 26-30 µs med, max 1.55 ms with video decoding | 0.023 ms per frame = 0.3% of 8.33 ms | none on SSD | moderate (3 per frame): 0.0042 × 21,600 × 0.3 = **27**; 4 layers 45 |
| 7 | S3 sequence frame-0 decode (× 2) + stats | ImageSequence.cpp:32, :57; MainComponent.cpp:2940, :4957 (fenced at :5026) | message | each sequence at load / drop; F = load 0.05 + drop 0.1 per min | 2 × 300-frame load 128 / 134 ms; PNG clip 98.5, JPEG 26.0; drop 27.1 / 31.5 → 4-5 black frames | UI 128 ms; drop ~42 ms black | UI hitch at load; brief black on a drop | drop 42 × 0.1 × 3 = **12.5**; load 128 × 0.05 × 1 = 6.4 |

Rank-order note: S1 is #1 under any weighting. Under the moderate model the loop (S1b) leads; under any heavy model the steady cost (S1a) leads. S2-drop scores low only because drops are rare. Per event, it is the only site that blacks the output, so it is a correctness defect for a live show whatever its score.

## Per-site detail: fix options (not implemented), constraints, expected gain, RED-able gate

**S1a / S1b: video on the GL thread (F16)**

Fix options:
- **(a) Decode off the GL thread, one serial decode context per playing player.** Use a dedicated thread per player, or a pool with a per-player serial queue.
  - FFmpeg thread affinity: the `AVFormatContext`/`AVCodecContext` is used by exactly one thread. Today `ffmpegMutex_` guards only open/close (`VideoPlayer.h:173-174`).
  - The decode thread converts into a pre-allocated ring of 2-3 RGBA frames tagged with pts. It can write bottom-up by passing sws a negative stride, which removes the 0.39 / 1.73 ms flip.
  - The GL thread keeps the clock (`advanceTransport` / `advanceClock`, so Rule 15 is unchanged). Each frame it picks the newest ring frame with pts ≤ clock and uploads only when that frame changed. It never waits: an atomic index or try_lock.
  - Seeks become requests to the decode thread, which decodes forward at its own pace. Meanwhile the GL thread shows the last frame. That is a hold, never 0 / no media (Pitfall 53, the FX-only trap).
  - A player that has never shown a frame counts as pending for the render_frame gate and for the C1 crossfade pause, as sequences already do.
  - Pitfall 35: during a crossfade the incoming and outgoing clips each have their own player and thread, so both chains stay live.
  - `closeMediaForClip` signals the thread and joins off the message thread (the retire list). A running frame decode is not interruptible, so the join is bounded by one decode.
  - `videoPlayerMutex_` must no longer be held across the decode (`Renderer.cpp:1577`). That removes the 25-120 ms trigger waits and the 85 ms fence waits.
- **(b) Cheaper independent steps:**
  - (b1) Upload only a newly converted frame. Measured gain, same launches: 1080p × 4 median −4.2 ms (6.82 → 2.66), 93 → 104 fps. No gain at 4K × 4.
  - (b2) Fix `decodeFrameAtTime`. Remember the pending seek target and keep decoding forward across calls instead of re-seeking while the decoded pts advances toward it; or bound the per-call decode work by time (e.g. 4 ms). This removes S1b: 3.3 s at 14 fps / 3.8 fps becomes a held picture that catches up over a few frames.
  - (b3) VideoToolbox hardware decode + IOSurface → GL texture (`CGLTexImageIOSurface2D`). Zero-copy, but macOS-only. The app is cross-platform, so the software path stays.

Expected gain, from the counterfactuals:
- (a): 4 × 4K from 36 to ≥ 96 fps (v1 gives 95.9 fps even with the per-frame uploads still in). Combined with (b1), 1080p × 4 goes to roughly 1.3-2.7 ms median. S1b moves off the GL thread entirely.

Gates:
- **m1_video_steady_4k** (S1a). A new TEST_SERVER `/api/state` counter `gl_video_decode_calls`: avcodec send/receive calls on the render thread, reset on read. Play 4 × 4K for 5 s.
  - PASS: `gl_video_decode_calls == 0` AND fps ≥ 110 AND p90 of `peak_callback_ms` polls ≤ 8.3 ms.
  - Reads today: about 120 decode calls/s (4 clips × 30), 36.0 fps, p90 81.7 ms.
- **m2_retrigger_midgop** (S1b). 1080p GOP-250 clip, inPoint 0.5: trigger, wait 3 s, retrigger, watch 3 s.
  - PASS: every render callback ≤ 16.7 ms AND a new counter `gl_video_max_decodes_per_call` ≤ 2.
  - Reads today: 30 decodes per call, callbacks of 67-69 ms for 3.3 s. The 4K variant reads 264-269 ms.
  - A deck-return variant (switch away 3.5 s, then back) reads the same today.

**S2: video open + thumbnail on the message thread (load, append, duplicate, drop, Replace Content, undo reconnect)**

Fix options:
- **(a) Asynchronous open.** A media-open job on a pool does `avformat_open_input` / `find_stream_info` / codec open / first frame, and makes the thumbnail with sws straight to 90×72, with no per-pixel `setPixelColour` loop.
  - It posts the ready player to the message thread, keyed by (clip id, composition generation). Until then the clip is pending (hold / skip, never FX-only).
  - Constraint from `MainComponent.cpp:3008-3014`: thumbnails and dims are written into `incoming` BEFORE the swap, because a post-swap write into a live Clip races the GL thread. An asynchronous delivery must write through a side table or under the fence.
  - Feel call for Boris: today a load keeps showing the OLD composition until everything is open. With (a) the new one appears at once, with pending clips holding.
- **(b) Cheap step.** Make `getThumbnail` sws-scale to 90×72. Measured share: 31% of a 1080p load, 30% of a 4K load (NO_THUMB: 423 → 293 ms, 1583 → 1108 ms).
- **(c) Drops.** Open the video BEFORE `withDeckDetached` and fence only `setClip`. A 4K drop goes from 14 black frames to the fence's own 1-2. Asynchronous open (a) makes the drop itself instant.

Gates:
- **m3_load_16x4k**: a TEST-ONLY `peak_message_stall_ms` in `/api/state`, fed by a heartbeat like the diag one. PASS ≤ 50 ms across `load_composition` of 16 × 4K clips. Reads today 1577-1617 ms.
- **m4_drop_video_black**: a TEST-ONLY counter `frames_without_deck` (render frames with `activeDeck_ == nullptr` while the composition has decks), plus a TEST-ONLY drop endpoint mirroring `DeckView::onFileDropped` (like `/api/debug/stall_message_thread`). PASS: delta == 0 over a 4K video drop. Reads today 13-14.

**S3: ImageSequence open + thumbnail (load, append, drop, undo)**

Fix:
- Take the dimensions from the first asynchronous decode result instead of decoding frame 0 in `open()` (`ImageSequence.cpp:57`).
- Move the 300 stats into the decoder: a missing frame becomes Failed and the last frame repeats (R-7 semantics).
- Get the sequence thumbnail from `ClipThumbnails`, keyed by the first file path + mtime, which is already off-thread. Delete the second frame-0 decodes (`MainComponent.cpp:2940`, `:4957`).

Constraints:
- `firstFramePending` / C1 / C3 semantics are unchanged: the dims are not needed to draw, since the texture carries its own size (Pitfall 39: the fit reads the texture size).

Gain:
- Load 128 → ~2 ms (the stats + JSON only; INFERRED from the scope split).
- Drop 27 ms → ~0; black frames 4-5 → fence only.

Gate:
- **m5_seq_open**: a counter `msg_image_decodes` (`ImageFileFormat::loadFrom` on the message thread). PASS == 0 over the two-sequence load and the 300-file drop.
- Reads today 4 per load and 2 per drop; heartbeat 124-135 ms (load) and 31-43 ms (drop).

**S4: stat() per image layer per GL frame**

Fix:
- Cache existence. The decoder already reads the stamp {mtime, size} off-thread (`ImageDecode.h:72`), so "resident" implies "exists".
- A 1 Hz off-thread sweep (the decoder or `ClipThumbnails` pool) of the composition's media paths maintains an atomic "missing" flag per path. The GL thread and `ClipCell` read the flag.

Constraints:
- Today a file deleted mid-show turns that clip into no media / FX-only on the next frame (`CompositorEngine.cpp:1076`, `clipHasContent`). With the cache, the latency is ≤ 1 s. Document it.

Gain:
- 0.02 ms per frame on SSD; removes the 1-1.5 ms outliers seen during video decode.
- Insurance for network or external volumes (INFERRED).

Gate:
- A counter `gl_file_stats` (stat calls on the render thread). PASS == 0. Reads today 600/s with 4 image layers.

**S5: stat() per cell paint**

Fix:
- Read the same cached missing flag; repaint on change.

Gain:
- −4.2 ms/s of message-thread time (measured counterfactual).

Gate:
- A counter `paint_file_stats`. PASS == 0. Reads today 700/s with 24 visible media cells.

## Sweep: other decoding and file I/O by thread (code read = VERIFIED path; unmeasured = INFERRED cost)

**(a) Per frame on the GL thread**
- The 6 compositor stats (S4) and the video path (S1).
  - Plus a per-decoded-frame `std::vector` allocation in `convertFrameToRGBA` (`VideoPlayer.cpp:545`, a heap allocation on the render thread).
- `ImageSequence::getCurrentTexture` allocates a `std::vector` per call and uploads up to one new frame per frame (2.04 ms median, measured).
- Camera: `texMgr_.uploadImage(pendingCameraFrame_)` (`Renderer.cpp:323`) converts + uploads on the GL thread per camera frame. Not measured.
- MilkDrop: `projectm_load_preset_file` on the GL thread when a preset change is pending (`ProjectMSource.cpp:139`): file read + parse + shader compile. Not measured.
- A grep of src/render, src/media and src/sources for the named APIs found no other per-frame file I/O.

**(b) Per paint, timer tick, trigger or refresh on the message thread**
- `ClipCell.cpp:206` (S5, 30 Hz × visible media cells).
- `ClipThumbnails::get` does 2 stats per Image cell on every `DeckView::refresh` (`ClipThumbnails.h:49` + `ThumbnailCache.h:30`). A refresh of the 32-cell grid measured 0.03-0.07 ms.
- `handleClipTrigger`:
  - `existsAsFile` per image/video trigger (`MainComponent.cpp:4331`, `:4343`);
  - `getVideoPlayer` (`:4295`, `:4321`) waits on the GL decode (25-120 ms measured in the loop);
  - `openVideoForClip` (`:4348`) runs a full open on the message thread if the player is missing.
- `onClipSelected` does `existsAsFile` (`:718`).
- The fence drain `UndoService.cpp:89` normally takes 3-10 ms; 85 ms median during 4 × 4K decode.
- The slideshow advance has been O(1) since R1.3.
- The FilesBrowser / CompDecksBrowser / PresetManager directory listings run on user action only.

**(c) At composition / deck load or media drop**
- `openMediaForDeck` (S2 / S3), `applyFileDrop` and `applyMultiFileDrop` inside the fence (S2 / S3).
- Replace Content (`MainComponent.cpp:6550-6557`): open + thumbnail. Code read only.
- `makeClipMediaHook` (undo/redo): opens on the message thread.
- Collect Media (`:6076-6150`): file copies + a recursive `findChildFiles`.
- `swapCompositionModel` → `closeMediaForClip` per orphaned player: 4.0 ms median, up to 20.9 ms while the GL thread decodes.
- `Composition::loadFromFile` (`Composition.h:677`): JSON read, small.

## ImageSequence notes for the bounded-texture-window plan (record, not a design)
- **Memory today.** Every visited frame stays resident until `releaseGL` / retire (`ImageSequence.cpp:331`). Measured RSS on unified memory, where textures count: +2.73 GB after one pass of a 300-frame 1080p sequence; 5.35 GB with two; back to 558 MB after the swap.
- **Upload cost.** The first upload of a frame is a `glTexImage2D` of a new texture: 2.04 ms median / 2.66 max on the GL thread.
  - The per-frame budget cap is 8 MiB (`ImageTexCache.h:261`), which fits exactly one 1080p frame (8,294,400 B).
  - A 4K frame (33 MB) takes the budget alone (the first take is always granted).
  - A window re-uploads on every loop: +60 ms/s of GL time per 1080p30 sequence (30 × 2.04 ms), which is 0 today after the first pass.
- **Decode cost.** Re-decoding happens on the shared 3-thread `ImageDecode::Decoder` (compositor images use it with prefetch P = 1, and so does the legacy path). Measured 44.6 ms per noisy 1080p PNG frame: one PNG sequence at 30 fps needs 1.34 CPU-s/s (~45% of the pool); two would saturate it and starve demand decodes and prefetch. JPEG 12.9 ms per frame.
- **Invariants a window must keep:**
  - Never evict the frame on screen or `lastShown_` (the hold).
  - Reset `requested_[i]` (and `outstanding_`) when frame i is evicted, or it is never requested again.
  - `failed_` stays sticky.
  - The `openGen_` tags stay.
  - Look-ahead (`kLookAhead` 3, `kMaxOutstanding` 4) is direction-aware (PingPong, reverse, OneShot).
  - `firstFramePending()` feeds both C1 (the crossfade pause) and the render_frame gate (`notePendingImage`, C3).
  - GL deletes happen on the GL thread only. Retire goes through `drainRetiredMedia`, and `releaseGL` at context loss clears everything.
- **Inactive decks.** An off-screen deck only advances the clock (`syncMedia` with decode = false, no requests). Today a deck return finds every visited frame resident; with a window it shows `lastShown_` (stale) until the look-ahead refills. This interacts with Rule 15.
- **Spill memory.** `ready_` keeps decoded results (8.3 MB each at 1080p) whenever the budget is exhausted.
- **Dimensions.** `open()` still decodes frame 0 on the message thread just for the dimensions (S3). A window design could take them from the first decode result instead.

## found_not_fixed
1. Every `withDeckDetached` fence renders at least one deck-less (black) frame on every output. `UndoService.cpp:66` nulls `activeDeck_` before the GL drain at `:89`. Measured: 1 per load, 2 per image drop, 5-14 per video drop.
2. `videoPlayerMutex_` is held across the whole GL video decode and upload (`Renderer.cpp:1577-1642`). Message-thread `getVideoPlayer` / `closeMediaForClip` / open-insert wait behind it: 25-120 ms in the loop, up to 20.9 ms per close at a swap.
3. A beat-snapped trigger seeks a video or sequence to `beatPhase ∈ [0,1)` as a normalized position of the WHOLE clip (`MainComponent.cpp:4283-4302`). It jumps to an arbitrary point and, on long-GOP files, enters S1b.
4. The composition JSON field `speed` is loaded without a presence check (`Clip.cpp:215`). A file without it loads video and sequence clips at speed 0, i.e. frozen. It hit my first fixture; probe-deck-clock notes the same.
5. `VideoPlayer::getThumbnail` reads `frameBuffer_` on the message thread while `convertFrameToRGBA` can write it on the GL thread, if the id's player is live (the Replace Content path). A data race, INFERRED.
6. With 2+ triggered video layers the message-thread heartbeat median goes from 7 to 16 ms, even with decode off (v1) or uploads trimmed (v2). Not a media site; handed to diag-idle.
7. `convertFrameToRGBA` allocates a `tempRow` vector per decoded frame on the GL thread (`VideoPlayer.cpp:545`).
8. Two unattributed 237-253 ms GL frames with ~5 GB of sequence textures resident (q1_5).

## Notebook lines for Harmony
- A composition clip without `"speed"` loads at speed 0 (`Clip.cpp:215`, no presence check): always write `speed` 1.0 in probe fixtures, or videos and sequences never advance.
- A diag phase mark sent through `callAsync` is ordered with the action's own `callAsync`: post the mark BEFORE the action, or the action's cost lands in the previous phase.
- `pgrep -f <pattern>` in a `while` waiter matches the waiter's own command line and never exits: wait on a pid instead.
- Wait for no compiler BEFORE taking the live lock (holding it while `wait_quiet` spins blocks the other lanes). Sample for compilers every 2 s during a launch and discard any launch with a hit.
- x264's default GOP (250) + `VideoPlayer::decodeFrameAtTime`'s "re-seek when > 2 s behind, decode ≤ 30" rule = a mid-GOP seek spins 30 decodes per render frame until the next keyframe. Use `-g 30` fixtures as the control.
- Every ClipCell repaints at the 30 Hz UI rate even at idle: per-paint work in `ClipCell::paint` costs × 30 × the visible cells.
- On this M1, `ps -o rss` of Audio-DNA includes GL textures (unified memory): a usable VRAM proxy (+2.7 GB per 300-frame 1080p sequence).

## Deviations and rig notes
- **`cd` rule broken once.** One command used `cd <worktree> && git status` (read-only), which the rig forbids. Nothing else used `cd`.
- **Lock hold while waiting.** My first smoke batch held the live lock ~10 min (17:36-17:47) while waiting for a compiler. I fixed it with quiet-before-lock (`tools/batch2.sh`). That script later grabbed and released the lock 3 times in 2 minutes while another lane's compiles flapped (19:44-19:46).
- **Contaminated launches.** Three launches ran with a compiler active (v1_3, v1_6, k0_3). They are excluded and replaced by v1_7 and k0_6.
- **Stray waiters.** Two self-matching waiter shells (pgrep) were left running; I killed them.
- **Report file.** The harness refused the report-file write, so `report.md` does not exist; this text is the report.

## PACKET QUALITY
- **Clarity:** CLEAR.
- **Missing context:**
  - REST has no drop path; I added a temporary environment-gated hook.
  - The old drive.py targets routines.
  - The `speed` JSON gotcha.
  - Lock contention and compiles from 3 lanes (the matrix took ~2 h of wall time for ~40 min of launches).
- **Unused context:** none.
- **Self-brief files** (all useful): plan-renderleft.md (FACTS F1-F16, rulings, FILED), renderleft.md / renderleft-fix.md (FOUND NOT FIXED), restore-tools (DiagTrace.h, drive.py, run1.sh), probe-image-load.{py,sh}, probe-deck-clock.py (the `speed` note), lock.sh.
- **Self-assembly:** no DEPARTMENT field.
- **Knowledge tools:** grep only; impact authority is not authoritative, so I took a conservative posture.

### Evidence index
- runs/{v0_1..5, v1_1,2,4,5,7, v2_1..5, k0_1,2,4,5,6, i0_1..5, i1_1..5, q0_1..5, q1_1..5}: app-err.log, drive.out, drive-log.json, meta.txt, analysis.json.
- runs/smoke1-5: setup runs; only smoke3 is cited, for one number.
- Aggregates: runs/agg-{v,k,iq}.txt, pool-{v,k,iq}.txt, seekstats.{txt,json}.
- tools/ holds every driver and analyzer. instr.diff is the instrumentation.

NEXT ACTION: Harmony. Plan S1 (off-GL-thread video decode, plus the two cheap steps b1/b2, which can land first with gates m1/m2), and S2(c) (open before the fence), which ends the black output on drops.
