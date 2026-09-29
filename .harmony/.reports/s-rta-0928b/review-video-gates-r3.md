# Reviewer Verdict — review-video-gates-r3
STATUS: DONE
VERDICT: APPROVE
REVIEWED: lane/video worktree .claude/worktrees/rta0928b-video, base 328301d, head c1120bf63b99c39fa6c671ae616e966478af9b67
FIX-ROUND DELTA REVIEWED: 90cdf55..c1120bf (05c8f75 W1/W2 probe measurement fix, df042c7 W3 FAILED-not-pending,
1fbd307 fix-round-1 report, 81dc7b0 pix_fmt guard, c1120bf fix-round-2 report). This round's own scope is chiefly
81dc7b0/c1120bf (the round-2 review's two MUSTs); the earlier commits were re-verified for regression only.

FINDINGS: none blocking.

SHOULD/NIT: none new. Round 2's SHOULD 1 (decode-error spin) was correctly STOPPED by the builder -- I verified the
premise independently by reading decodeNextFrame() (src/media/VideoPlayer.cpp:686-723): for a zeroed-payload stream,
avcodec_send_packet's failure hits the inner `continue` (line 713) and the loop keeps consuming packets from the SAME
file until av_read_frame itself returns EOF (atEof_ = true, line 699) -- decodeLoop's `if (!atEof_) continue` branch
(noteNoFirstFrame("a decode error")) is never reached for this failure mode. The report's claim is correct, not just
plausible.

VERIFIED POSITIVES (independently reproduced in $TMPDIR/scratchpad, never touching the reviewed tree):
- MUST 2 from r2 (pix_fmt crash) is fixed and the fix is REAL, not just claimed. I built my own GREEN check by running
  the lane's own compiled tests/test_video_player_open binary (copied build-lane, unmodified) -- "All tests passed
  (10 assertions in 2 test cases)". I then independently reproduced RED myself (not from the report's log): compiled
  the pre-fix VideoPlayer.cpp (the report's saved copy at fr2/VideoPlayer.cpp.1fbd307, diffed against HEAD's
  src/media/VideoPlayer.cpp -- the only difference is exactly the 11-line pix_fmt==AV_PIX_FMT_NONE guard, nothing
  else) into a fresh object file with the exact compiler invocation from build-lane/compile_commands.json, relinked
  it into a standalone test_video_player_open_RED binary against the same JUCE/Catch2/FFmpeg objects, and ran it:
  `REQUIRE( r.exited )` false, `r.signal := 6` (SIGABRT) -- a from-scratch mutation-repro of the crash, confirming
  the guard at src/media/VideoPlayer.cpp:163-169 is what stands between this fixture and an app abort, not an
  unrelated or coincidental change.
- ffprobe on the committed fixtures confirms the report's factual claims exactly: video_h264_cut_header.mp4 (1,539 B)
  -> codec_name=h264, pix_fmt=unknown, 1920x1080; video_h264_64x64.mp4 (3,464 B) -> pix_fmt=yuv420p, 64x64.
- freeFfmpeg() on the new guard's failure path is null-safe and follows the exact pattern already used by the
  adjacent (pre-existing) swsCtx_ failure branch two lines below it -- no leak, no pattern deviation.
- The forked-child ctest harness (tests/test_video_player_open.cpp) never calls a GL function before p.close(): open()
  never uploads a texture and never calls start() (decode thread), so releaseGL()'s `texture_ != 0` guard makes the
  destructor safe with no GL context in the child. Correct, minimal, matches existing test conventions
  (apply_sanitizers / catch_discover_tests used identically to every other test in tests/CMakeLists.txt).
- MUST 1 from r2 (no live GREEN evidence) is now satisfied and I re-verified the report's own quoted numbers against
  the actual on-disk logs byte-for-byte (this session's scratchpad still holds them, contrary to r2's session): ctest
  886/886 (scratchpad/video/ctest-fr2.log tail), the red-open.log/green-open.log SIGABRT->GREEN transcripts, w9 6/6
  PASS live (runs/fr2-g-w9.log), the w1 4-row fps table (b1/full1/full2/b3 .out files) and w2 (a)/(a2) printed rows,
  and the 3 regression probes GREEN (reg/fr2-{image-load,crossfade,deckclock}.log) all match the report's report.md
  quotes verbatim.
- w9's probe fixtures are HAP-only (probe-video.json "broken"); grepped probe-video.py/.json for any H.264
  cut-header live arm -- none exists, so the still-crashing H.264 path is correctly never exercised live (matches
  the "never a live arm" claim and the screen-safety concern from r1/r2).
- CLAUDE.md is 24,731 B (<=25,000). Full-lane docs diff (328301d..head) is additive: every removed line (tree entry,
  the video/rendering paragraphs, the render_frame table row, CLAUDE.md rule 5's NOTE) has a strictly-extending
  replacement at the same place -- nothing dropped without a replacement, matching ruling V6's explicit trim
  authorization for rule 5.
- git status in the worktree: only build-lane/ untracked (expected per-lane build dir, pre-existing convention, not
  part of this diff); no .venv symlink; no stray instrumentation or env-var hook added in this delta.
- Open items (w1 (a) / w2 (a2) not GREEN on this machine, found_not_fixed 1 the message-thread full-file read on a
  never-decodable stream) are transparently reported as needing a Harmony ruling, not silently shipped as GREEN --
  correct STATUS: PARTIAL self-report, and matches the FOCUS text's ask ("w1 5-run table incl. main's; w2 (a)/(a2)
  as ruled, printed" -- printed and ruled-upon, not asserted GREEN).

CONFIDENCE: HIGH -- the two things that most needed independent verification (the crash fix being real, and the live
evidence not being fabricated/stale) were both reproduced by me from scratch, not taken on the report's word. No MUST
found. Verdict is APPROVE; the two open fps rows and found_not_fixed 1 are correctly flagged as needing a Harmony
ruling, not a reviewer blocker.
METADATA: reviewer=reviewer-agent, builder_packet=video-gates-r3, date=2026-09-29
