# Small safety recording of the show -- what it costs the computer (Boris, L6)

## ANSWER FOR BORIS
It costs very little. Every number below is an ESTIMATE, not a measurement.
- A quarter of each side (480 x 270) at 30 pictures a second: about 1-3% of the processor, under 1% of the graphics card, about 25-75 MB per 10 minutes.
- The big reading, a quarter of the area (960 x 540) at 60 a second: about 3-10% of the processor, about 120-380 MB per 10 minutes.
- An eighth is smaller still (240 x 135: 6-24 MB per 10 minutes).
Cutting into 10 or 20 minute pieces is not enough alone: a video file that is not closed properly cannot be played. It has to be written so a crash loses only the last few seconds. Then both ideas work together.
I recommend 480 x 270 at 30 a second. Only a test on your Mac with a show running gives a number I can promise.
Decided without asking you: this is a size choice inside the Video box.

## QUESTION FOR HIM
Should the small recording carry the sound?
- (default) No. The Audio box already keeps the sound at full quality, separately.
- Yes, add sound to the small file: about 10 MB more per 10 minutes.
- Both: a choice on the box.

## FACTS
(For Harmony's notes; "today" lines are kept here so the builder knows what to change, not for Boris.)
Machine
1. VERIFIED. Apple M1 Pro, 10 cores (`sysctl -n machdep.cpu.brand_string` = "Apple M1 Pro"; `sysctl -n hw.ncpu` = 10; `system_profiler SPDisplaysDataType`: "Total Number of Cores: 16", "Metal 3", built-in Liquid Retina XDR 3456 x 2234; refresh rate not shown -> UNKNOWN, cheapest: Displays settings screen).
The picture and the recorder
2. VERIFIED. The picture that would be shrunk is the composition canvas, 1920 x 1080 when unset, other shapes possible (CLAUDE.md Pitfall 37: "1920x1080 when unset"; docs/claude/rendering.md:158 presets 16:9 / portrait / square / 4:3 / Custom). The recorder reads that canvas (src/render/Renderer.cpp:884-888 `glBindFramebuffer(GL_READ_FRAMEBUFFER, canvasFBO_); videoRecorder_->submitFrame(canvas.w, canvas.h)`).
3. VERIFIED. Encoders known: H.264 `libx264`, ProRes `prores_ks`, MJPEG `mjpeg` (src/recording/VideoRecorder.cpp:206-208); H.264 is set to `preset ultrafast`, `tune zerolatency`, `crf` (quality 23) (:257-261); the two call sites hard-code H.264 (area-recording.md item 17). All three are software encoders.
4. VERIFIED. The app names no hardware encoder: grep "videotoolbox" over src/ and CMakeLists.txt = no hit. INFERRED: it does not use the Mac's hardware encoder today.
5. VERIFIED. The FFmpeg the build looks for is Homebrew's (cmake/FindFFmpeg.cmake:8-10 "/opt/homebrew/opt/ffmpeg"); that FFmpeg on this Mac lists `libx264`, `h264_videotoolbox`, `hevc_videotoolbox`, `prores_videotoolbox` (`ffmpeg -hide_banner -encoders`, run 2026-10-07). INFERRED (not checked): the app links that same library, so the hardware encoder is reachable by name. UNKNOWN for a packaged app: cheapest way = look at which libavcodec the built app loads.
6. VERIFIED (code read). `submitFrame` captures min(configured size, canvas size) from the bottom-left corner with `glReadPixels` (VideoRecorder.cpp:117-130): it CROPS, it does not shrink. INFERRED: a smaller size set today would record a corner; a shrink step on the graphics card must be added before the read.
7. VERIFIED (code read). `submitFrame` has no rate limit and each frame is stamped 1/fps (VideoRecorder.cpp:102-141, time base {1, fps} at :241). INFERRED, not run: with the draw loop above 30 a second the file plays too fast/slow; a fixed-rate step (30 a second) is needed.
8. VERIFIED (code read). No `movflags` and no segment option anywhere in VideoRecorder.cpp (header written with `avformat_write_header(formatCtx_, nullptr)` :300). INFERRED (from fact 11): a crash leaves today's file unplayable.
9. VERIFIED. The recorder carries no sound (VideoRecorder.h: "The recorder only captures video. Audio is NOT included in the recording").
10. VERIFIED. The glReadPixels hand-over is a lock-free triple buffer; frames are dropped, not waited for, when the encoder lags (Pitfall 25; VideoRecorder.cpp:111-115). A new step must keep that (no mutex on the picture thread).
How a recording survives a crash (FFmpeg's own documents)
11. VERIFIED. https://ffmpeg.org/ffmpeg-formats.html (fetched 2026-10-07): a normal MP4 is "undecodable if it is not properly finished"; "Writing a fragmented file has the advantage that the file is decodable even if the writing is interrupted"; `-movflags frag_keyframe` = "start a new fragment at each video keyframe"; `frag_duration` = "Create fragments that are duration microseconds long"; `faststart` "will not work in various situations such as fragmented output". (The fetch tool summarised the page; quotes should be re-checked in the page if a builder relies on a word.)
12. VERIFIED. `empty_moov`: "Write an initial moov atom directly at the start of the file, without describing any samples in it." (https://manpages.ubuntu.com/manpages/xenial/man1/ffmpeg-formats.1.html, 2026-10-07; an old release's page, same wording as ffmpeg-devel 2023 patch https://ffmpeg.org/pipermail/ffmpeg-devel/2023-April/308575.html). INFERRED: fragmented MP4 needs empty_moov + frag_keyframe (or frag_duration), and a key picture every ~2 s so a crash loses at most about that much plus the unwritten tail. The default key-picture spacing of libx264 (250 frames = about 8 s at 30/s) is from memory: ASSUMED.
13. VERIFIED. FFmpeg's `segment` muxer: "This muxer outputs streams to a number of separate files of nearly fixed duration"; `segment_time` default "2"; `reset_timestamps`; example `ffmpeg -i in.mkv -f segment -segment_time 10 -segment_format_options movflags=+faststart out%03d.mp4` (https://ffmpeg.org/ffmpeg-formats.html, 2026-10-07). "Nearly fixed": cuts fall on key pictures. The app drives libavformat itself, so it can instead close one file and open the next at a forced key picture: INFERRED, S-M.
What the hardware encoder costs
14. VERIFIED (community test, Oct 2021, base M1 Mac mini, 2560x1440 clip): software libx264 "speed=2.61x" (38 fps); `h264_videotoolbox` "speed=6.74x" (98 fps); "'top' suggests that neither machine is CPU-bound when the hardware encoders are invoked" (https://ffmpeg.org/pipermail/ffmpeg-user/2021-October/053727.html, 2026-10-07). Older than 6 months; different Mac; a file-to-file job, not live.
15. VERIFIED (blog, Dec 2020): "the videotoolbox variant is 4 times faster than the x264 software encoder ... Only 20% of my CPU is used instead of 100% for the software encoding" (https://www.martin-riedl.de/2020/12/06/using-hardware-acceleration-on-macos-with-ffmpeg/, 2026-10-07). That is a flat-out file encode, not a live one.
16. VERIFIED (user report, M1 Pro 10-core, https://forums.developer.apple.com/forums/thread/678210, 2026-10-07): "about 10% CPU and GPU" in the hardware encode; the same thread says the picture quality at a given bitrate is worse than libx264 ("MASSIVE artifacts on CQ<50"). Anecdotes, not Apple's numbers. https://cheatref.com/ffmpeg/compression/hardware-accelerated-encoding-with-videotoolbox (2026-10-07): the hardware H.264 encoder "has no true CRF mode ... quality tracks the bitrate you set".
17. VERIFIED (secondary). The M1 Pro has a separate Media Engine for video (https://www.soundonsound.com/reviews/apple-macbook-pro-m1-pro-max?page=2 via search, 2026-10-07). INFERRED: it does not take the shader cores; it does share memory with them.
Size and load figures (all my arithmetic)
18. ASSUMED (no source, the weakest link): bits per picture-point 0.08-0.25 for busy, moving, audio-reactive pictures with the fast x264 setting; 60 a second costs about 0.65 of that per picture (so ~1.3x the 30 a second size). Real value depends on the show: noise and strobing push to the top, calm clips to the bottom. Cheapest check: record one real show minute.
19. ASSUMED: libx264 `ultrafast` is 4-8x cheaper than FFmpeg's default `medium` speed (from memory, unsourced). Basis for the CPU range: fact 14 gives ~140 million points/s for software medium on an 8-core M1 at full machine load (INFERRED all cores busy). 960 x 540 at 60 = 31 million points/s -> ~3.7% of a full M1 Pro at 6x faster, range 2-7%, plus ~1-3% for the colour conversion and the read-back copy.
20. Disk figures. See table in OPTIONS. Disk space is no issue at these sizes; for contrast the same arithmetic gives 370-1,160 MB per 10 minutes for a full 1920 x 1080 at 30 a second (ESTIMATE, same assumption 18).
21. VERIFIED. Boris's words (binding-decisions.md:846-853, "44"): "We can also record the video file of the show, which will take a lot of hard drive space." and (:867) "48 b" -> "three boxes, each on its own: Parameters, Audio, Video". Lines 1187: "we will record to clip or record show. That's what the 2 recordings are called." So "record show" has a Video box (the full picture, large) and an Audio box (the sound); nothing of his says the small recording is a separate thing or whether it has sound -> the question above.
22. VERIFIED (area-recording.md items 6, 7). The sound of a take is kept in the Audio Store as a WAV file; a cut-off WAV can be repaired ("Repair Audio"); a take with audio refuses to start under 2 GB free. So the sound already survives a crash on its own path; the small picture file is the weak part.

## OPTIONS
Size/load table (ESTIMATE, rests on facts 18-19; canvas 1920 x 1080; megabytes, low-high)
| Shrunk size | Reading | 30 a s, per 10 min | 30 a s, per 20 min | 60 a s, per 10 min | 60 a s, per 20 min |
|---|---|---|---|---|---|
| 480 x 270 | 1/4 of each side | 23-73 | 47-146 | 30-95 | 61-190 |
| 960 x 540 | 1/4 of the area | 93-292 | 187-583 | 121-379 | 243-758 |
| 240 x 135 | 1/8 of each side | 6-18 | 12-36 | 8-24 | 15-47 |
| 679 x 382 | 1/8 of the area | 47-146 | 93-292 | 61-190 | 121-379 |
Processor, whole 10-core machine, software encoder, ESTIMATE: 240x135@30 under 1%; 480x270@30 1-3%; 480x270@60 or 679x382@30 2-5%; 960x540@30 2-6%; 960x540@60 3-10%. Hardware encoder: encoding itself near 0-3% (facts 14-16), the rest (shrink, read-back, conversion) the same. Graphics card, ESTIMATE: one shrink pass of the 1080p picture per recorded picture, under 1% of the card; the extra wait on the picture thread to fetch the small frame is the one real unknown: guess 0.2-1.5 ms per recorded picture (full-size 8.3 MB read would be several times that; no number exists in the project docs -> UNKNOWN). Memory: a few MB of buffers. Note the pitfall text gives a draw budget of 8 ms for the full effect chain, so 1 ms is a visible bite on a heavy show.

A. Reuse the recorder; add (1) a GPU shrink to the chosen size, (2) a fixed 30-a-second picture clock, (3) software H.264 as now. Cost M. Can go wrong: extra wait on the picture thread (read-back stall); a corner crop if the shrink is missed; colour/vertical flip bugs. Works if: a 10-minute test file plays at the right speed, shows the whole picture, and the app's `frame_time_ms`/`gpu_time_ms` (/api/state) stay within noise of recording off.
B. As A, but the Mac's hardware H.264 encoder (VideoToolbox). Cost M-L on top of A (a bitrate setting replaces quality 23; fallback to software if it fails to open). Gains: near-zero processor for the encode, but the encode is already small at these sizes, so the gain is 1-5 points of processor. Can go wrong: worse picture for the size, odd rate control at tiny sizes, differences between Macs. Works if: the same measurements as A show less processor and the file still reads on a 240 x 135 check. My view: not worth it for 480 x 270; revisit only if the measured processor cost is high.
Crash safety (pick one, combine with A or B)
C1. Closed pieces only (stop file, start next, every 10 or 20 minutes). Cost S-M. Loses up to a whole piece (the open one is unplayable). Does NOT meet his goal alone.
C2. Fragmented MP4 inside each piece (`frag_keyframe+empty_moov`, a key picture every ~2 s). Cost S on top of A. Loses seconds. Can go wrong: some players/editors dislike fragmented MP4; no thumbnail/length until the end in a few; verify in QuickTime, Resolume and the app's own clip loader. Works if: the app is force-quit mid-recording and the piece still plays up to the last few seconds.
C3. C1 + C2 (recommended): each 10-minute piece is also fragmented. Cost M total. Piece boundary should land on a forced key picture so no picture is dropped or doubled. One more guard: a free-space check at start and every piece (today there is none; disk guard of 2 GB exists only for take audio).

## WHAT HAS TO BE MEASURED WHEN IT IS BUILT
On his Mac, with a heavy real show (the busiest he plays: many layers, video clips, MilkDrop, feedback), 10 minutes each, recording off vs on, alternating:
1. Processor: Activity Monitor total plus the app's own number; the encoder thread's share. Pass: the extra is inside the ranges above.
2. Picture thread: `frame_time_ms`, `peak_frame_time_ms`, `gpu_time_ms`, `peak_callback_ms` (/api/state, already in the app) for off vs on; and `getDroppedFrameCount()`. Pass: no new late frames; drops 0.
3. Real file sizes of 480 x 270 and 960 x 540 at 30 and 60 per 10 minutes on a calm and on a busy show (replaces assumption 18).
4. Speed of the file: `ffprobe` duration equals wall-clock time (facts 7).
5. Crash test: force-quit mid-piece; the piece plays to within a few seconds of the quit.
6. Software vs hardware encoder, same size: processor, size, and a look at the picture.
7. Disk: a nearly full disk at start; and mid-recording.
8. A 2-hour show: piece boundaries have no gap or doubled picture; sound (if included) stays in step across boundaries.
9. Whether the app on his Mac draws above 60 a second (display refresh) -- decides how many pictures are skipped before the encoder.

## NOT DONE / UNSURE
- Nothing was run: no recording, no test, no app. Every processor and size number is an ESTIMATE from arithmetic on assumed ratios (facts 18-19); the web numbers are other Macs, other jobs (not live), older than 6 months.
- No source for libx264 `ultrafast` vs `medium` speed, nor for bits per point on audio-reactive pictures; both from general knowledge.
- The cost of the read-back wait on this app's picture thread is UNKNOWN; it is the most likely thing to be worse than my range, and the one that touches the live show.
- UNKNOWN whether the packaged app can reach the hardware encoder (which FFmpeg it ships).
- The FFmpeg doc quotes came through a page-summarising tool for the ffmpeg.org page; the empty_moov wording is from a 2016-era man page copy plus a 2023 patch that still shows it. 
- I decided, not asked: the small recording is a size setting inside the Video box (his L8 allows laying out where it fits). Matroska (survives truncation by design) as another container was not researched.
- Sound answer assumes the Audio box stays a separate full-quality file (his "48 b"); his words never say whether the small file has sound.
Wed Oct  7 22:48:22 EDT 2026
