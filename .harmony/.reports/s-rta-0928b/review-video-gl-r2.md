# Reviewer Verdict — review-video-gl-r2
STATUS: PARTIAL
VERDICT: FAIL (one MUST: live w9 GREEN evidence, required by W3's own ruling, was never captured on the fixed build)
FILES: src/media/VideoPlayer.{h,cpp}, src/media/VideoRing.h, tests/test_video_ring.cpp, tests/CMakeLists.txt,
  .harmony/probe-video.{json,py,sh}, docs/claude/{pitfalls.md,rendering.md,testing-eyes.md,architecture.md,
  effects.md,performance-controls.md}, CLAUDE.md
ISSUES:
  MUST -- W3's HARMONY ADOPTION ruling (plan-video.md line ~900-905) names TWO required pieces of evidence: "ctest on
    the judge/state" AND "a live row w9 ... crossfaded onto: the fade completes within its time + 0.5 s and
    render_frame answers". Only the ctest (verified GREEN by this reviewer) and a RED baseline on the OLD app
    (90cdf55) exist. w9 has never been run against the FIXED build (df042c7/1fbd307) -- the builder's own report says
    so explicitly ("w9 has not been run on the fixed app"). The pure ctest exercises only VideoRing::judge /
    firstFrameFailed (free functions); it does not exercise VideoPlayer's real decode thread, atomic ordering under
    real scheduling, CompositorEngine's C1 wiring, or the render_frame gate end-to-end -- exactly what this round's
    FOCUS asked to confirm ("no waits, no leak of the decode thread, C1 and the render_frame gate never hang").
    Blocked by a still-open system crash dialog (from an out-of-scope H.264 pix_fmt crash, correctly not run against
    the fix). Fix: dismiss the dialog, rebuild/relaunch the FIXED app, run probe-video.sh's w9_crossfade_onto_broken
    (both hap_cut / hap_zero arms) plus ideally the rest of the suite, and attach GREEN evidence before this round is
    called done.
  SHOULD (informational, not blocking) -- an untracked build-lane/ directory sits in the worktree, not covered by
    .gitignore (only /build/, /build-asan/, /build-tsan/, /cmake-build-*/, build-debug/ are ignored). Not committed,
    harmless, but worth a .gitignore entry so it doesn't tempt a stray `git add -A`.
METADATA: reviewer=reviewer-agent, builder_packet=video-gl-r2, date=2026-09-29
