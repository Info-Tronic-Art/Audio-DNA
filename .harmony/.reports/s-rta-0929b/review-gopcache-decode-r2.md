# Reviewer Verdict — gopcache decode r2
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES
REVIEWED: lane/gopcache 98994c6e6534859cc1cf2dd8800bc84fa1c66c7c vs a02c93c (git show / diff only)
VERIFIED: test_gop_cache_store (499 assertions, 20 cases) and test_video_decode_trace (510 assertions, 5 cases) run green from build-lane binaries built after the last source edit; CLAUDE.md 24703 B; no .venv / symlink / stray instrumentation in the diff; only env hook is the plan-blessed TEST_SERVER-only ADNA_GOPCACHE_BUDGET_MB; no new mutex, no audio-thread change.
MUST-1: reverse on a stream with pts-less packets (DivX-style AVI) stores/publishes the wrong picture (src/media/VideoPlayer.cpp onRunFrame: pts-less frame -> pts = ptsOfRel(r.target) -> rel == target; first output is the keyframe -> stored at the TARGET index -> rel >= target ends the run after one decode). Builder's own ISSUES 1 (120 seeks / 120 decodes vs main 73-77 / 833-916). Not in the plan, not documented, forward parity for the same file was just restored by F2.
SHOULD: live gates F3-F6 still unrun (dialog); VFR reverse distinct-frame regression vs main undocumented; misc nits in the structured output.
