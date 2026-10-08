# check-lowres: re-check of answer-lowres-rec.md (Boris L6, low-resolution show recording in 10/20 minute pieces)

VERDICT: SOUND_WITH_CORRECTIONS. Code and web facts hold up; the numbers are honest estimates but two unsourced ratios lean optimistic, the biggest live-show risk is not in the ANSWER, and the ANSWER is 164 words (limit 150) with Harmony jargon ("reading").

## FACTS RE-CHECKED
1. Machine M1 Pro, 10 cores, 16 GPU cores, 3456x2234: CONFIRMED (sysctl, system_profiler). Refresh rate not shown (paper right). The laptop panel is probably 120 Hz and project docs speak of "120 fps" drawing (rendering.md:77): see finding 4.
2. Canvas 1920x1080 when unset (Pitfall 37 text) and the recorder reads the canvas (Renderer.cpp:884-888): CONFIRMED.
3. Encoders libx264/prores_ks/mjpeg; H.264 ultrafast + zerolatency + crf; call sites hard-code H.264, 30 fps, quality 23, .mp4 (MainComponent.cpp:7289-7310): CONFIRMED.
4. No "videotoolbox"/hwaccel anywhere in src, CMakeLists, cmake: CONFIRMED (grep, no hit).
5. FindFFmpeg.cmake searches /opt/homebrew/opt/ffmpeg first: CONFIRMED. Homebrew ffmpeg on this Mac lists libx264, h264_videotoolbox, hevc_videotoolbox, prores_videotoolbox: CONFIRMED (ffmpeg -encoders, 2026-10-07).
6. submitFrame crops (min(config, canvas) from the bottom-left via glReadPixels): CONFIRMED (VideoRecorder.cpp:117-130).
7. No rate limit; time base {1,fps} (:241); pts_++ per encoded frame: CONFIRMED.
8. No movflags / segment / frag in VideoRecorder.cpp; avformat_write_header(..., nullptr): CONFIRMED.
9. VideoRecorder.h:34 "Audio is NOT included": CONFIRMED.
10. Triple buffer, drop not wait (:111-115): CONFIRMED.
11. FFmpeg formats page: "a normal MOV/MP4 is undecodable if it is not properly finished", frag_keyframe "start a new fragment at each video keyframe", frag_duration "Create fragments that are duration microseconds long", faststart "will not work in various situations such as fragmented output": CONFIRMED in the raw page (curl, 2026-10-07), not only the summary.
12. empty_moov wording: the CURRENT ffmpeg-formats page has NO empty_moov entry (0 hits; the movflags list shows cmaf, dash, default_base_moof, delay_moov, disable_chpl, faststart, frag_*...). The option exists in libavformat/movenc.c ("Make the initial moov atom empty", fetched 2026-10-07). PARTLY CONFIRMED (flag real, the paper's quote is from an old man page).
13. segment muxer: "outputs streams to a number of separate files of nearly fixed duration", segment_time default "2", example -segment_time 10 ... movflags=+faststart: CONFIRMED (raw page).
14. ffmpeg-user 2021-10 (M1 mini, 2560x1440): libx264 speed=2.61x fps 38; h264_videotoolbox speed=6.74x fps 98; "top suggests neither machine is CPU-bound": CONFIRMED. Added caveat: output was 175 MB vs 15 MB, so the two runs are not equal quality.
15. martin-riedl 2020: "4 times faster", "Only 20% of my CPU is used instead of 100%": CONFIRMED.
16. Apple forum 678210: "about 10% CPU and GPU" on M1 Pro 10-core (Dec 2021): CONFIRMED. The "MASSIVE artifacts on CQ<50" quote is a different poster on an M1 MAX (Jan 2024): same thread, not the M1 Pro user. cheatref "no true CRF mode, so quality tracks the bitrate you set": CONFIRMED.
17. Sound On Sound: M1 Pro/Max "Media Engine" with H.264/HEVC/ProRes hardware: CONFIRMED (page 2).
18. binding-decisions.md: "We can also record the video file of the show, which will take a lot of hard drive space" (:851, paper says 846-853), "48 b" three boxes (:867), "record to clip or record show" (:1187): CONFIRMED.
19. area-recording.md items 6, 7, 17: CONFIRMED. Same sheet: "the three Record boxes do not exist" -> finding 3.
20. Arithmetic of the table (px/s x seconds x bits per point / 8): CONFIRMED for every cell redone (480x270x30, 0.08 -> 23.3 MB, 0.25 -> 73 MB; 1080p30 370-1160 MB).
21. /api/state fields frame_time_ms, peak_frame_time_ms, peak_callback_ms, gpu_time_ms (TestServer.cpp:634-682) and getDroppedFrameCount (VideoRecorder.h:72): CONFIRMED.
22. Assumptions 18-19 (0.08-0.25 bits per point; ultrafast 4-8x cheaper than medium): COULD NOT CONFIRM; outside evidence leans the other way (finding 2).

## FINDINGS
- SHOULD: the ANSWER leaves out the one real risk. Fetching the small picture from the graphics card can stall the live drawing thread; the paper itself calls it "the most likely thing to be worse than my range" and UNKNOWN, yet the ANSWER says only "under 1% of the graphics card" and opens "It costs very little". Say plainly: the load is small, the hitch on the picture is unknown.
- SHOULD: assumptions 18-19 are optimistic. x264 ultrafast makes files about 2x the size of medium at the same CRF (2013 test: 5276 KB vs 2729 KB at CRF 20; thepostflow preset table: ultrafast "much larger" at matched CRF), and that guide says ultrafast saves ~55% of medium's time (about 2x, not 4-8x). zerolatency also removes B-frames. So 960x540@60 could be nearer 10-12% processor and the upper size bound about 2x higher. Still small; widen or flag.
- SHOULD: the ANSWER says the Audio box "already keeps the sound at full quality". The three Record boxes are not built, and "full quality" of the stored WAV is not verified. "About 10 MB more per 10 minutes" (128 kbps) has no fact behind it. Rephrase as a plain question without "already".
- SHOULD: render rate. The paper leaves the refresh "UNKNOWN", but rendering.md:77 speaks of "~100 fps" and "120 fps" drawing on a laptop with a probable 120 Hz panel, so today's 30-fps-stamped file is probably 3-4x slow motion. The plan (fixed 30-a-second clock) already covers it; say "probably 120".
- SHOULD: missed the cheaper mitigation for the read-back stall: asynchronous read-back (two pixel-buffer objects, read last frame's) after the shrink on the card. Add to option A.
- SHOULD: ANSWER is 164 words (limit 150) and uses "the big reading" (Harmony's R-jargon, which L1 says costs his focus). His "1/4 or 1/8 the size" (L6) is ambiguous (side vs area); the ANSWER silently shows both. Ask it as one focused question.
- SHOULD: "Then both ideas work together" and "a crash loses only the last few seconds" are promises; nothing was run (the crash test is only in the to-measure list). Word as a requirement.
- SHOULD (minor): the empty_moov quote is from an old man page, not the current docs (fact 12); say "the option exists". Matroska not checked (paper admits).

## A BETTER "ANSWER FOR BORIS" (replacement, 146 words)
It should be light on the computer, but nothing is measured yet. Every number is an estimate.
- 480 x 270, 30 pictures a second: about 1-3% of the processor, 25-100 MB per 10 minutes.
- 960 x 540, 60 a second: about 3-12% of the processor, 120-400 MB per 10 minutes.
- 240 x 135: under 25 MB per 10 minutes.
Not known: whether the small picture causes a tiny hitch while you perform. Only a test on your Mac shows that.
Pieces of 10 or 20 minutes are not enough alone: a video cut off by a crash cannot be played, so each piece must also be written to lose only the last seconds.
I suggest 480 x 270.
Decided without asking you: it is a size choice in the Video box.
Question: did you mean a quarter of each side, or of the area?
