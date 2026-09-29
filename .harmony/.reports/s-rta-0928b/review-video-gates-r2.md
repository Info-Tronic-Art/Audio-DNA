# Reviewer Verdict — review-video-gates-r2
STATUS: PARTIAL
VERDICT: FAIL
REVIEWED: lane/video worktree .claude/worktrees/rta0928b-video, base 328301d, head 1fbd30734755d252e40b2ccfaeb414e6ab8e63b5
FIX-ROUND DELTA REVIEWED: 90cdf55..1fbd307 (VideoPlayer.h/.cpp, VideoRing.h, test_video_ring.cpp, tests/CMakeLists.txt,
probe-video.{sh,py,json}, docs/claude/{pitfalls,rendering,testing-eyes}.md)

FINDINGS:
MUST 1 (blocks merge as "gates" complete): the round's own report
(.harmony/.reports/s-rta-0928b/video.md "Fix round 1", STATUS: PARTIAL) and the task's FOCUS list both require live
GREEN evidence -- w9 GREEN after the fix, the w1 5-run table incl. main's 5 runs, w2 (a)/(a2) printed, the full
probe-video GREEN x2, and the 3 regression probes re-run -- and NONE of it exists. Only code + ctest + the RED-on-90cdf55
w9 log are done; a crash + system dialog stopped all live work per the rig's "unexpected dialog: STOP" rule, and the
scratchpad run logs the report cites (fr1-red-h90-*.log, ctest-fr1.log) no longer exist on disk (session-scratchpad
cleanup) so I could not re-inspect them directly -- only the report's verbatim quotes. This round cannot be called
gate-complete.
MUST 2 (pre-existing, correctly filed by the builder, not a regression of this round, but ships unmitigated in the
reviewed HEAD): a video file whose pixel format is unknown before any frame decodes (e.g. an H.264 .mp4 cut to its
header, a partial download/copy) SIGABRTs the whole app: VideoPlayer::open() -> sws_getContext(AV_PIX_FMT_NONE) trips a
libswscale assertion. VERIFIED independently (not from the report's word): read
~/Library/Logs/DiagnosticReports/Audio-DNA-2026-09-29-003404.ips directly -- SIGABRT, thread stack
Renderer::openVideoForClip(uint,juce::File const&) <- MainComponent::openMediaForDeck <- MainComponent::loadComposition
<- ApiServer::handleLoadComposition lambda <- juce::MessageQueue::runLoopCallback (message thread), crashing frame's
image = libswscale.9.1.100.dylib -- an exact match to the report's claim. Code confirms the call is still unguarded at
HEAD (src/media/VideoPlayer.cpp:157-165, no pix_fmt == AV_PIX_FMT_NONE check). This is a real, user-reachable crash that
ships in the reviewed HEAD (also present on main per the builder's inference, not independently checked there). The
builder did the right thing (filed it, did not silently fix out-of-scope, swapped the w9 fixture to HAP so no future
run can trip it) but it needs a Harmony ruling + a fix before this feature is safe to ship to real users with imperfect
media files.

SHOULD 1: VideoPlayer::decodeLoop's decode-error branch (`if (!atEof_) { noteNoFirstFrame(...); continue; }`,
src/media/VideoPlayer.cpp ~583-587) has no backoff or attempt cap, unlike the old decodeFrameAtTime's 30-attempt bound.
A stream that fails to decode every packet (the w9 hap_zero arm: a zeroed mdat) spins the decode thread at ~100% of one
core until it reaches real EOF. Bounded by file length so not infinite, and off the GL thread, but worth a
thread_.wait() or an attempt cap for a long corrupted file.

NIT 1: build-lane/ (the fix round's build dir, .claude/worktrees/rta0928b-video/build-lane) is not in .gitignore's
build-dir list (/build/, /build-asan/, /build-tsan/, /cmake-build-*/, build-debug/) and shows as the only untracked
path in git status. Harmless (expected per-lane build dir) but worth adding to .gitignore.

VERIFIED POSITIVES (independently reproduced, not taken on the report's word):
- Rebuilt tests/test_video_ring.cpp against the lane's VideoRing.h in the existing build-lane/ (cmake --build
  --target test_video_ring): 118 assertions / 15 test cases, ALL PASS -- matches the report's ctest claim exactly.
- Reproduced one of the report's "teeth" mutations in an isolated $TMPDIR copy (never touched the reviewed tree): a
  standalone compile of VideoRing.h + test_video_ring.cpp against the same Catch2 libs, with firstFrameFailed's
  `gaveUp ||` branch removed. Result: 3 assertions fail at test_video_ring.cpp:202/203/205 with expansions
  `false` / `false` / `3 == 4` -- the identical lines and expansions the report's teeth table cites. The W3 gate is
  real, working coverage, not a synthetic-fixture-only claim.
- git diff 328301d..HEAD -- docs CLAUDE.md: additive only (grep of removed vs added lines shows every "-" line has a
  strictly-extending "+" replacement; nothing dropped). CLAUDE.md is 24,731 B (<= 25,000, matches the report).
- git status in the worktree: clean except build-lane/ (expected); no .venv symlink, no stray instrumentation.
- W3's GL-thread change (uploadToTexture: firstDrawMs_ stamp + firstFrameFailed_ re-judge) adds no lock, mutex or wait
  to the render thread; the decode-thread side adds one atomic<bool> + one log line. Sacred-rule / X1 intact.
- VideoRing::judge() call sites and VideoPlayer::neverShown() correctly wire the new Shown::Failed verdict through to
  C1 (neverShown() now false once failed, so the crossfade pause provider stops waiting on it) exactly as the ADOPTION
  ADDENDUM W3 ruling specifies.
- kSkipNonRefInCatchUp is `true` at src/media/VideoPlayer.h:206, matching the docs' "(on)" claim and ruling V5.

CONFIDENCE: HIGH on everything independently reproduced above (code review, rebuild, mutation repro, crash-report
read, docs diff, git status). INFERRED (not independently re-run) on the report's own live-run numbers for W2/w1's
5x90cdf55-app runs and the ctest-884 total, since the session scratchpad evidence files are gone; these numbers are
plausible and consistent with everything I could verify, but are the report's word, not mine.
METADATA: reviewer=reviewer-agent, builder_packet=video-gates-r2, date=2026-09-29
