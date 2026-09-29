# Reviewer Verdict — review-video-gates-r1 (lane/video, s-rta-0928b)
STATUS: DONE
VERDICT: PASS_WITH_NITS

REVIEWED_RANGE: base 328301de37dd56df2e83096089f9847fd0e51ff0 .. head 90cdf55214b50e6e587ddf84ee917b8cce1969a5
(worktree .claude/worktrees/rta0928b-video, branch lane/video), diff/git show only, never the working tree.

## Method (disk-verified, not recall)
- Read plan-video.md in full incl. HARMONY ADOPTION V1-V7 (overrides body).
- Read every changed file via `git show <head>:<path>` for the 23 changed files (2228+/255-).
- Opened src/media/VideoRing.h, VideoPlayer.h/.cpp, tests/test_video_ring.cpp, Renderer.h/.cpp,
  CompositorEngine.h/.cpp, src/api/ApiServer.cpp, src/test/TestServer.cpp, VideoStats.h, docs, CLAUDE.md.
- Independently RAN (read-only, on the worktree's already-built build-lane, no source mutation):
  `./build-lane/tests/test_video_ring` -> "All tests passed (105 assertions in 14 test cases)" (matches the report's
  claim exactly). `ctest -j4` in build-lane -> "100% tests passed, 0 tests failed out of 883" (matches the report's
  883/883 and the ctest -N total of 883 exactly).
- `wc -c CLAUDE.md` = 24731 (report claims 24,731 B; matches, under the 25,000 B cap).
- grep for stray env-var hooks (`AUDIODNA_VIDEO_`) in src/tests: zero hits, matches the report's
  "strings | grep -c AUDIODNA_VIDEO_ = 0" claim; no .venv symlink; `git status --porcelain` shows only the untracked
  build-lane/ dir (expected build output, not stray).
- grep confirmed `ffmpegMutex_` fully removed (R-10) and no new mutex added anywhere in the diff (only videoPlayerMutex_
  / retiredMediaMutex_, both pre-existing, reused for lookup-only scopes) -- satisfies "never add a new mutex".
- grep confirmed the old blocking `decodeFrameAtTime` / `convertFrameToRGBA` (the S1b re-seek loop + per-frame tempRow
  allocation) are fully deleted from VideoPlayer.cpp/.h, not merely bypassed.
- Confirmed `gl_video_decode_calls` / `gl_video_max_decodes_per_call` are declared in VideoStats.h and read in both
  /api/state handlers, but NO code path in the final VideoPlayer.cpp/Renderer.cpp increments them -- "0 by
  construction" is literally true (never touched), which is the intended regression witness, not dead code (the probe
  asserts on it) -- JUSTIFIED_KEEP.
- Confirmed `uploadToTexture(bool*)` has exactly one caller (Renderer.cpp:1794) after the signature change.
- Confirmed HARMONY ADOPTION rulings trace to real code, not just doc claims:
  - V1 (ShownState.everShown survives releaseGL, never cleared except by a new open): implemented in VideoRing.h
    (ShownState struct) and VideoPlayer.cpp releaseGL() (`shown_.onReleaseGL()` -- never touches everShown); ctest
    "V1: a GL release (context loss) keeps 'shown before'" passes.
  - V2 (idle check inside the ring-full wait too): `VideoRing::idleStep` checked both at decodeLoop's top and inside
    onDecoded's ring-full retry loop; `video_threads_awake` field wired into both /api/state handlers; ctest "V2: the
    idle rule parks..." passes; probe-video w4b (row read in full) computes its own timing to stay clear of the 10 s
    Loop wrap, matching commit 5's stated fix and the report's "clear of the Loop wrap" claim.
  - V3 (explicit CAS memory orders): `Ring::cas` uses acq_rel/acquire explicitly; comment states the rule.
  - V4 (retrigger mid-fade holds, does not pause): w6b in probe-video.py implements exactly the row described in the
    adoption text (retrigger the active column mid-fade, hold captures at +0.1/+0.25s asserting luma>20 and code not
    29/541, fade still reaches crossfadeProgress>=1).
  - V5 (kSkipNonRefInCatchUp default ON): `static constexpr bool kSkipNonRefInCatchUp = true;` in VideoPlayer.h,
    matching the ruling; the report's NONREF-on/off hold tables are consistent with this default.
  - V6 (rebase, no ImageSequence.*/SeqVram.h edits, pitfall "NN" placeholder, CLAUDE.md budget): confirmed diff touches
    no ImageSequence.h/.cpp or SeqVram.h; pitfalls.md/CLAUDE.md carry literal "NN" (expected -- Harmony assigns the
    number at merge per the seqvram precedent, commit 39d3986); CLAUDE.md 24,731/25,000 B.
  - V7 (GREEN = 4.7 + w4b + w6b): the report itself states plainly it is NOT fully GREEN by this definition (w2(a),
    w1(a)) and asks Harmony to rule -- an honest, disclosed gap rather than a silently-passed gate.

## Findings

FILE: src/media/VideoRing.h
  [OK] Readability/Patterns: extensively commented with the exact ownership/memory-order invariants; matches the
       VideoRecorder / ImageDecode precedents cited in the plan (F16).
  [OK] Complexity: proportionate to a genuine lock-free SPSC-style ring with 3 slot states; no speculative generality
       (dropAheadSec defaults to infinity, only used by VideoPlayer's reverse-play call).

FILE: tests/test_video_ring.cpp
  [OK] Staged-test hygiene / correctness: 14 cases, 105 assertions, independently re-run GREEN (no synthetic-fixture-only
       risk: the stress case is a genuine two-thread run with owner/torn/order assertions, not just synthetic unit
       cases). Teeth for every rule are named in the report; spot-checked the ones with a ctest surface (V1/V2 cases
       exist and assert the specific behavior a wrong implementation would violate).

FILE: src/media/VideoPlayer.h / .cpp
  [OK] Real-time rules: GL-thread methods (advanceFrame/advanceClock/uploadToTexture/releaseGL) contain no heap
       allocation, no mutex, no FFmpeg call -- verified by reading every line of uploadToTexture/advanceFrame/
       advanceClock/releaseGL; matches "render thread never waits" and the audio-adjacent sacred-rules spirit (this
       lane never touches the audio callback at all).
  [OK] Lifecycle safety: close() never joins on a hot thread (R-9); destructor's stopThread(3000) + freeFfmpeg() is
       safe against the decode thread's own freeFfmpeg() at loop exit because freeFfmpeg() is genuinely idempotent
       (each pointer null-checked and nulled).
  [NIT] onDecoded()'s ring-full retry path returns without incrementing any stats counter when it bails out because
        gen_ changed (a stale-generation decode is simply dropped, uncounted) -- purely a diagnostics-completeness gap,
        not a correctness issue (video_frames_dropped already undercounts this one path). Not blocking.
  [SHOULD] found_not_fixed #3 (self-reported by the lane): a video whose first frame never decodes stays PENDING
           forever (a crossfade onto it and render_frame's gate wait indefinitely). This is a real behavior change
           from the old code's "black upload" fallback, could look like an app hang for a genuinely undecodable file,
           and has no ctest/probe fixture proving or bounding it. Recommend either a bounded-pending fallback or an
           explicit ticket before this ships to end users, but it does not block this review (already disclosed, not
           hidden, and the old behavior it replaces was itself a silent-wrong-picture bug per Pitfall 53's spirit).

FILE: src/render/Renderer.cpp / .h
  [OK] R-10 (mutex guards the lookup only) and R-9 (deferred destroy, threadDone gate) both verified against the
       actual diff, not just comments; no new mutex added; ffmpegMutex_ genuinely removed.
  [OK] C1/C3 pending generalization (mediaPendingFn_) correctly renamed and extended to Video with the same shape as
       the ImageSequence precedent; syncMedia's video branch correctly threads *pending through to
       compositor_.notePendingImage().

FILE: src/render/CompositorEngine.h / .cpp
  [OK] Clean rename (SequencePendingFn -> MediaPendingFn); no behavior change for the ImageSequence path.

FILE: src/api/ApiServer.cpp, src/test/TestServer.cpp, src/media/VideoStats.h
  [OK] Additive /api/state fields, identical shape in both servers, matches the report's field list verbatim.

FILE: CLAUDE.md, docs/claude/{pitfalls,rendering,architecture,effects,performance-controls,testing-eyes}.md
  [OK] Additive, consistent with the implementation (spot-checked the rendering.md "Video playback" paragraph and the
       new Pitfall NN entry against the actual code paths they describe -- no overclaiming found: "never waits",
       "0 by construction", "guards the lookup only" all verified true above).
  [NIT] Pitfall number is literally "NN" pending Harmony's assignment -- expected per the project's own precedent
        (commit 39d3986 assigned Pitfall 54 the same way) but flagged so it isn't forgotten at merge.

## Performance targets (not a code-quality defect; a product ruling for Harmony)
w1(a) and w2(a) (the plan's INFERRED >=110 fps bars) are not met on this machine; the lane's own w2(a) diagnostic table
shows the same scene with NO video plateaus at 108-110 fps too (a rendering ceiling unrelated to this lane's video
path), and w1(a) is measurably bimodal under the probe's poller but reads a stable ~120 fps under a cleaner poller in
the lane's own A/B hold. This is honestly surfaced with root-cause data in the report rather than hidden or
re-thresholded silently, so it is not scored as a MUST-level defect here — it is a business/perf-bar decision for
Harmony, explicitly named as HANDOFF-NEEDS in the report.

## SLIM
No EXCESS_DEAD/EXCESS_VESTIGIAL/EXCESS_DUP/EXCESS_SPEC found. gl_video_decode_calls / gl_video_max_decodes_per_call are
permanently-zero fields but are load-bearing regression witnesses read by probe-video (w1(c)/w2(c)/w8) —
JUSTIFIED_KEEP reason="asserted-on-zero regression witness proving no avcodec call reaches the GL thread; removing it
would silently drop the strongest correctness claim in the report".

## Verdict rationale
No MUST-level issue found: no defect that ships silently, no missed plan item concealed, no test that cannot fail.
Every checkable claim in the lane report (ctest counts, pass rates, byte counts, field names, mutex removal, adoption
rulings V1-V6) was independently re-derived from disk and matched. The two open items (w1/w2 perf bars, and the
found_not_fixed PENDING-forever edge case) are both already disclosed by the builder and correctly routed to Harmony
for a ruling rather than swept under a passing gate — that is the behavior a review should want to see, not a
violation. PASS_WITH_NITS reflects the two NIT/SHOULD items above (stats undercounting, the disclosed pending-forever
edge case) plus the still-open pitfall number.

SUMMARY: 23 files reviewed (source + tests + docs), 0 blocking issues, 3 non-blocking notes (1 NIT diagnostics gap,
1 SHOULD edge-case-hardening suggestion, 1 NIT pending pitfall-number assignment). Confidence: VERIFIED for every
claim listed as re-derived above (tests re-run, greps re-run, byte counts re-run); INFERRED for the two fps ruling
items (measurement disputed by the lane itself; Harmony's call, not a source-review finding).
METADATA: reviewer=claude-sonnet-5, builder_packet=lane-video-r1, date=2026-09-29
