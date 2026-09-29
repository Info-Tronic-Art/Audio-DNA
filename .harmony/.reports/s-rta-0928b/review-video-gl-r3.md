# Reviewer Verdict -- s-rta-0928b video lane, round 3 (GL lens)
STATUS: DONE
VERDICT: APPROVE

REVIEWED: worktree .claude/worktrees/rta0928b-video, branch lane/video, fix-round delta
90cdf55..c1120bf (commits 05c8f75, df042c7, 1fbd307, 81dc7b0, c1120bf), read via git show/diff only.
Whole-lane 328301d..c1120bf checked for fence/CLAUDE.md-size/docs-additive items.

FILE: src/media/VideoPlayer.h / .cpp, src/media/VideoRing.h (W3 -- df042c7)
  [OK] Correctness: firstFrameFailed_ is recomputed every uploadToTexture call only while
       !shown_.everShown, and the ring-pick branch (p.slot >= 0) returns the texture BEFORE that
       check runs -- so a frame that lands after the FAILED verdict still shows (VideoRing::judge
       returns New on picked=true regardless of the failed flag). Verified by reading the early
       return at VideoPlayer.cpp:446 vs the Failed check at :455-463.
  [OK] No new wait/lock on the GL/render thread: `firstDrawMs_`/`firstFrameFailed_` are plain
       members read/written only on the GL thread; `firstFrameGaveUp_` is the only cross-thread
       field and is a lock-free atomic (acquire/release), not a mutex. `thread_.wait(20)` after
       EOF is the decode thread's existing idle-poll pattern, unchanged in kind -- it does not
       spin and is joined/exited normally by close(). No leak: `noteNoFirstFrame` is idempotent
       (guarded by `everDecoded_ || firstFrameGaveUp_`) and the thread proceeds to its existing
       wait(20) loop, same as before this change.
  [OK] neverShown() (the C1 crossfade-pause provider) now excludes firstFrameFailed_
       (`VideoPlayer.h`), and Renderer's C3 render_frame gate reads `uploadToTexture`'s *pending*
       out-param, which the Failed case never sets (`return 0` before the switch's Pending arm) --
       traced the call chain Renderer.cpp:1793-1799 -> CompositorEngine::notePendingImage ->
       Renderer.cpp:2508's `pendingCaptureComplete_` check. Neither C1 nor C3 can hang on a FAILED
       player. video_pending_frames (VideoStats::pendingNow) is likewise untouched by the Failed
       arm (only the Pending case increments it), matching w9(c)'s "+0 pending" assertion.
  [OK] Retrigger recovery: `Renderer::openVideoForClip` (Renderer.cpp:1444) constructs a brand new
       `std::make_unique<VideoPlayer>()` per open -- a retrigger never reuses a failed player's
       state (firstFrameFailed_/firstFrameGaveUp_/firstDrawMs_ all start fresh), so "a later
       successful decode after a retrigger still recovers" holds structurally, not just by luck.
  [OK] Pure-function unit coverage (test_video_ring.cpp W3 case) exercises every branch of
       `firstFrameFailed` and `judge(..., failed=true)` including "never drawn -> no timeout" and
       "shown before -> never failed" -- re-ran it directly (ctest #880, `ctest --test-dir
       build-lane -I 880`): PASSED, independent of the report's own transcript.

FILE: src/media/VideoPlayer.cpp (MUST 3 guard -- 81dc7b0), tests/test_video_player_open.cpp,
tests/CMakeLists.txt
  [OK] The AV_PIX_FMT_NONE guard sits exactly before `sws_getContext`, is the narrowest fix for
       the reproduced SIGABRT (does not try to generalize to every possible sws_getContext
       failure -- that path is already handled by the existing null-check a few lines below).
  [OK] Gate is not toothless: `openInChild` forks a real child per open() and asserts
       `WIFEXITED`, so a regression (SIGABRT) fails the assertion in the PARENT/test process
       rather than crashing the test runner -- this is exactly the shape needed for a crash gate.
       Re-ran independently: `ctest --test-dir build-lane -I 884,885` -> both PASS; `ctest -N`
       confirms both are wired into the 886-test suite (#884, #885), matching the report's count.
  [OK] Control case (a valid 64x64 H.264) proves the guard doesn't over-reject.

FILE: .harmony/probe-video.py / .json / .sh (w9 row, fps-poll measurement change)
  [OK] w9_crossfade_onto_broken's rationale (HAP, not H.264, cut/zeroed) is documented in both the
       probe and the commit body with a concrete reason (H.264-cut-header aborts a build without
       the round-2 guard) -- a reasonable, disclosed deviation from the addendum's "e.g. mp4"
       phrasing, not a silent substitution.
  [OK] FOCUS claim "W2 poller change is measurement only" verified directly:
       `git show 05c8f75 --stat` touches only `.harmony/probe-video.{py,json,sh}` -- no `src/`
       changes. `git diff --stat 90cdf55..head` for the whole fix round confirms the only `src/`
       files touched anywhere in the round are VideoPlayer.h/.cpp and VideoRing.h.
  [OK] probe-video.json's new constants (fpsPollS, w1Repeats/w1PassMin, fps4kMin/
       fps4kBelowStillsMax, w9FadeS, the `broken` fixture specs) match the HARMONY ADOPTION
       ADDENDUM W1/W2/W3 rulings in plan-video.md verbatim (thresholds, run counts, rationale) --
       cross-checked against the addendum text directly (main repo's plan-video.md:889-909, the
       pinned spec; the worktree's own copy of the file predates the addendum, which is expected
       per the lane's V6 fence/rebase note).

FILE: docs/claude/pitfalls.md, rendering.md, testing-eyes.md, CLAUDE.md
  [OK] Spec/claim fidelity: each doc sentence added for W3 and the pix_fmt guard was checked
       against the actual code paths above (FAILED semantics, neverShown, render_frame answers,
       kFirstFrameTimeoutMs = 2000) and matches exactly; no overclaiming found.
  [OK] Docs are additive for this round (only the NN pitfall paragraph, rendering.md video
       paragraph and testing-eyes render_frame row grew); the CLAUDE.md rule-5 condensation is
       from an earlier commit in the lane (aec6285), out of this round's delta, and is a
       byte-budget rewrite that preserves the same information, not a content drop.
  [OK] `wc -c CLAUDE.md` = 24,731 <= 25,000 (V6 fence), verified directly.

FENCES / HYGIENE
  [OK] No edits to ImageSequence.*/SeqVram.h (V6 fence) -- `git diff --stat` for those globs
       across the whole lane is empty.
  [OK] No stray instrumentation/env-var hooks introduced (grepped the round's src/tests diff for
       getenv/ifdef-debug/instrument -- none). `git status --porcelain` on the worktree is clean
       except the untracked `build-lane/` build directory (not part of any commit).
  [OK] `found_not_fixed` items (open() reading a whole broken file synchronously on the message
       thread; w1(a)/w2(a2) fps variance) are correctly filed as open items for Harmony rather
       than silently re-thresholded or hidden -- checked decodeNextFrame's inner while(true) loop
       directly and confirmed it only returns at true EOF/an unrecoverable receive_frame error,
       supporting the SHOULD-4-STOPPED reasoning (the decode thread does not spin after that;
       it reaches the existing wait(20) idle poll).

SUMMARY: 3 code files + 2 test files + 1 fixture pair + 3 probe files + 3 docs files + CLAUDE.md
reviewed (14 files changed in the fix-round delta), 0 blocking issues. Independently re-ran the
new/changed tests (ctest #880 W3, #884/#885 the open() guard) against the worktree's own
build-lane and got the same GREEN the report claims; traced the C1/C3/pending-counter call chains
by hand rather than trusting the report's prose. No MUST found: the W3 state machine cannot hang
C1 or render_frame, the decode thread cannot leak, and a retrigger genuinely recovers because it
is always a fresh VideoPlayer object. The two open fps rows (w1(a), w2(a2)) are correctly flagged
as NOT GREEN and escalated rather than silently passed -- that is a hardware-variance finding for
Harmony to rule on, not a code defect in this delta.
CONFIDENCE: VERIFIED for all code-path claims above (read + independently executed); INFERRED for
the live-app w9/regression probe results (not re-run in this review -- no display/GL context
available here; relied on the report's transcripts, which are internally consistent with the code
read).
METADATA: reviewer=reviewer, builder_packet=s-rta-0928b-video-r3, date=2026-09-29
