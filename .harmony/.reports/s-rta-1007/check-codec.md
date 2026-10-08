# check-codec: re-check of answer-codec.md
Checked 2026-10-07 (checker: Researcher). Sources opened again, not taken from the paper. Nothing built, run or probed.

VERDICT: SOUND_WITH_CORRECTIONS. The facts hold. The Boris-facing text over-promises in three places and misses a simpler option.

## FACTS RE-CHECKED
1. Resolume article 67 quotes (GPU codec; decompression on the video card; hardware accelerated only in Resolume; stores alpha): CONFIRMED, all four quotes seen verbatim.
2. CDM 2009-03-18 (bandwidth bottleneck; spatial not temporal compression; encoding slower): CONFIRMED, date 18 Mar 2009. The "bigger than Motion-JPEG in most cases" wording: the page text I got says "the file size is also larger than MJPEG"; "in most cases" NOT confirmed (minor).
3. Resolume blog 11923 (3 benefits; 25% smaller than DXV2; HQ doubles): CONFIRMED verbatim.
4. FFmpeg dxv.c is LGPL, decoder only, tags DXT1/DXT5/YCG6/YG10, no encoder: CONFIRMED (downloaded the file, grepped). "Resolume DXV" name also present in the local libavcodec.
5. github.com/Vidvox/hap (Free BSD, commercial use free, Tom Butterworth, VIDVOX 2012, GPU decompression): CONFIRMED.
6. hap.video ("over two dozen", natively or third-party, VDMX / TouchDesigner / disguise, CPU saving "substantially"): CONFIRMED. VDMX, TouchDesigner and disguise appear in case studies, not in a list of supporters. Fine.
7. HAP spec: "open video codec", fourcc table, Snappy/none, independent chunks, "Only one top-level section": CONFIRMED. The table also has HapA (alpha only); the paper's list omits it (minor).
8. Spec does not say "intra-frame" in words: CONFIRMED (paper rightly marks INFERRED). My search for an official Vidvox statement found none; only community posts. So "every frame stands on its own" for HAP is still unsourced.
9. HAP R benchmarks: BC7 + alpha, size like HAP Q, ProRes 422 level, "should NOT use ... in place of ProRes 422", VDMX 6 Sept 2024, Demolition Studios FFmpeg branch: CONFIRMED. The paper says the post date is not shown: the post is dated 7 Aug 2025, so over 6 months old (14 months); the FFmpeg-branch hope is likely stale.
10. FFmpeg hapdec.c: no BC7/BPTC, handles Hap1/5/Y/A/M, CPU decode via texturedsp, snappy: CONFIRMED by grep of the downloaded file. hapenc.c: name "Vidvox Hap", format hap/hap_alpha/hap_q, compressor none/snappy, chunks 1-64, input pixel format RGBA only, needs snappy-c.h: CONFIRMED. No Hap Q Alpha / Hap R: CONFIRMED.
11. Local FFmpeg 8.0_1: --enable-libsnappy / videotoolbox / gpl / version3 CONFIRMED in the ffmpeg binary. But the codec names "Vidvox Hap", "Resolume DXV", "NotchLC" are NOT in the ffmpeg binary: they are in /opt/homebrew/Cellar/ffmpeg/8.0_1/lib/libavcodec.62.11.100.dylib (1 match each). Paper's location is wrong (harmless). Homebrew formula today = 9.0.2, dependency list has x264/x265 and NO snappy: CONFIRMED. So a "brew upgrade" can silently remove the HAP encoder. App finds FFmpeg through cmake/FindFFmpeg.cmake, i.e. whatever is installed.
12. Apple forum 679660 (Apple "Graphics and Games Engineer", May 2021: "OpenGL would not 'understand' the format and the shaders would not be able to sample..."): CONFIRMED. Answer is to "Unfortunately no" for BC7-type data; fine as support, not a table of macOS extensions. DXT/S3TC on macOS GL still unverified (paper says so).
13. Apple ProRes page (4444 lossless alpha up to 16 bit; ~330 Mbps; only 4444 / 4444 XQ have alpha): CONFIRMED, and the page does not say "intra-frame" (paper says the same).
14. Forum sizes (HAP 1.61 / Q 3.20 GB/min at 4K60 alpha) and Vidvox "comparable to ProRes 422": NOT opened by me; could not confirm. The paper already labels them secondary.
15. Local code: VideoPlayer.cpp:117 avcodec_find_decoder; :229-230 sws to BGRA; :1819/1836 CGLTexImageIOSurface2D; :290-291 intraOnly_ list; :200-201 HAP alpha; VideoInfo.h HAP names; VideoPlayer.h:23: CONFIRMED. No hwaccel / videotoolbox / hw_device anywhere in src or CMakeLists: CONFIRMED. So HAP is decoded by FFmpeg on the CPU in this app.
16. pitfalls.md:121 (14 / 3.8 fps for 3.4 s, fixed by dropping frames), :135 (4.8 uploads/s GOP 250; cache min(2 GiB, RAM/16)), rendering.md:94 (HAP 1/600 reverse 0/270/273): CONFIRMED.
17. binding-decisions.md:650-651 and :680, backlog :115/:160/:205/:218, boris-msg L3, L20, L86, L101: CONFIRMED. L3 is "Is there any reason ... like resolume's DVX 3.0? Is that something that you can do reliably?"
18. Resolume DXV + HAP in Resolume: search shows Resolume Arena supports HAP (user-reported, Resolume 6) and recommends DXV. Secondary only.
19. ANSWER word count: 128 words. Within 150.

## FINDINGS
- MUST (ANSWER FOR BORIS): "the graphics card can do the unpacking" reads as a benefit Boris gets. In this app HAP is unpacked on the CPU by FFmpeg (fact 15 / paper fact 19), and the graphics-card unpack step is, by the paper's own "NOT DONE", not shown possible. Say it is what HAP is made for and that using it here is not proven.
- MUST (ANSWER FOR BORIS): "files several times bigger than ordinary video" is the paper's own INFERRED guess (no H.264 figure sourced; the 4K forum sizes were not opened). Written as fact. Say "much bigger (an estimate; I will measure on your clips)".
- MUST (ANSWER FOR BORIS): "jumping, playing backwards and landing on random beats are instant" is unmeasured and the intra-frame fact for HAP has no official sentence (spec silent). Reverse already works through the existing cache. Say "should be near-instant; to be measured".
- SHOULD (Option list / recommendation): a simpler option is missed. The app already treats ProRes, MJPEG, DNxHD as intra-only. "Convert for performance" could target an all-keyframe file of a codec it already plays, with no GPU step and no snappy dependency, then compare with HAP. HAP is not shown better in this app than that.
- SHOULD (Question for him): the default "yes, plan it" comes before the jump-wait on his real long-GOP clips is measured (paper's UNKNOWN). Better question: "Shall I measure first on 2 of your clips, then decide?" Also use his own words (L9): he asked to pick ONE codec that "decodes easily and plays well" (BF22, binding-decisions 650); tie the answer to it.
- SHOULD (fact 11): codec names are in libavcodec.62.dylib, not the ffmpeg binary; the real risk is that Homebrew 9.0.2 has no snappy, so the encoder may vanish on upgrade. The plan must say which FFmpeg it uses.
- SHOULD (fact 9): HAP R post is dated 2025-08-07; mark the FFmpeg-branch hope as stale.
- SHOULD (fact 2): drop "in most cases" or re-quote.

## A BETTER "ANSWER FOR BORIS"
No. A codec of our own is not worth building, and I would not call it something I can do reliably. Resolume made DXV in 2009. HAP, a free open format with the same purpose, has existed since 2012. Every HAP picture is stored on its own, so jumps and random beats should be near-instant (not yet measured on your clips). It can carry see-through video, and it is built for the graphics card to do the unpacking, which I have not yet shown we can use. The price is much bigger files (my estimate) and a one-time conversion. A codec of our own would play only here and needs new tools and long testing. HAP, or a simple all-pictures-stored format, is reliable because both already exist and can be checked.
(117 words)
