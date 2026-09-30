# Reviewer Verdict — vupload gates r2
STATUS: DONE
VERDICT: PASS_WITH_NITS
LANE: vupload (s-rta-0929), lens gates, round 2 (fix round: 3a9e166..b930908)
BASE: 3e15613  HEAD: b9309081133757d72782602d6fff1b5a2fc7c940
WORKTREE: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vupload

FILES REVIEWED (fix-round delta 3a9e166..b930908, all 10):
.harmony/.reports/s-rta-0929/vupload.md, .harmony/probe-video-w10-all.sh (NEW), .harmony/probe-video.py,
.harmony/probe-vupload.py, .harmony/probe-vupload.sh, docs/claude/rendering.md, docs/claude/testing-eyes.md,
src/render/GLThreadQos.h (DELETED), src/render/Renderer.cpp, src/ui/OutputWindow.cpp.
Also traced whole-lane 3e15613..HEAD for CLAUDE.md/docs additivity and the round-1 SHOULD items this round answers.

## Ruling-by-ruling (ADDENDUM 2, VU14-VU17) -- each checked against code AND re-derived from raw run data

- VU14 (kWriterLookAhead, 90b2ca9): adopted as a documented override of plan section 6's "MUST NOT CHANGE" line
  (pitfalls.md / rendering.md both now read "measured from the writer's look-ahead"). No code change this round;
  this is a doc-only ruling as the packet states. VERIFIED (git show 90b2ca9 predates this round; docs already
  carried the corrected text before 3a9e166).
- VU15 (QoS revert, 8952f97): re-derived from the BUILDER's own raw scratch data in this session's shared
  scratchpad (`.../scratchpad/vupload/fix1/vu15.out`, `vu15-summary.txt`), not from the commit message alone.
  Confirmed: 3 arms (MAIN/FINAL/NOQOS), rotated round-robin start per round (r1 MAIN,FINAL,NOQOS; r2 FINAL,NOQOS,MAIN;
  r3 NOQOS,MAIN,FINAL, ...), 10 launches/arm, "tainted 0" (quiet). Per-launch values match the commit message
  exactly. Decision arithmetic re-checked: FINAL-NOQOS=9>=4 AND NOQOS(2)<=MAIN(2)+2=4 -> rule (a) fires; rule (b)
  false (|9|>=4); rule (c) false (11>5=MAIN+3). This is the literal, pre-registered arithmetic, not a re-framed
  post-hoc justification. Revert commit correctly removes GLThreadQos.h + both call sites (Renderer.cpp,
  OutputWindow.cpp) with no dangling include/symbol (`grep -rn GLThreadQos src/ tests/` empty after the revert).
  Doc claims (rendering.md, testing-eyes.md) were updated in the SAME commit to say QoS is DEFAULT/21 and the
  raise was "tried and reverted" -- no stale "raised to USER_INTERACTIVE" claim survives anywhere in the tree.
  qos_check() -> qos_info() in probe-vupload.py: u2 and u4a(f) are now unconditionally INFO (never judged),
  matching the ruling exactly.
- VU16 (u7 hold_no_texture assertion, 4769fe8): confirmed against real logs
  (`.../scratchpad/vupload/fix1/vu16/{main-3e15613,final-3a9e166}.log`): pre-lane app -> "FAIL ... video_hold_no_texture
  absent" x4 scenes (PY 0/4), lane app -> "PASS ... delta ... 0 == 0" x4 scenes (PY 4/4). `pv.delta()` itself
  already FAILs on an absent counter (`no(f"{tag}: {k} absent...")` inside `delta()`), so the u7 code's
  `if h is not None: check(...)` does not silently skip the absent case -- traced this to rule out a
  toothless/fail-open path (the obvious gotcha for a "some apps predate the field" guard). VERIFIED.
- VU17 (probe-video-w10-all.sh, c5dfcf9): confirmed against real logs
  (`.../scratchpad/vupload/fix1/vu17-{main-3e15613,final-3a9e166}/wrapper.log`): main 3e15613 -> all 3 arms
  "witness -- 0/10 Opened lines say upload=<path>" (the arm did not take its path) -> PROBE-VIDEO-W10-ALL RED;
  lane app -> all 3 arms 10/10 witnessed, PY 34/34 pass, 30/30 captures max|diff| 0 -> GREEN. The wrapper's
  witness check (`opened -ge 1 && opened -eq right`) genuinely requires every Opened line to name the right path,
  not just one -- would catch a build that silently ignores the env hook on later opens. No existing threshold
  was touched (probe-video.json / probe-vupload.json unchanged in this fix round).

## Re-derivation, not recall

- Ran `cmake --build build-lane` (incremental) at HEAD: builds clean, no dangling GLThreadQos symbol.
- Ran `ctest --test-dir build-lane -j1`: 962/962 pass (27.8s), matching the claimed count exactly.
- Read the raw per-launch VU15/VU16/VU17 run artifacts left in this session's shared scratchpad (same session id
  as the builder's, so the data is first-party, not the builder's own summary) and cross-checked every printed
  number against the commit messages and the lane report table -- all matched.

## Scope / hygiene checks

- Docs additive only: `git diff 3e15613..HEAD --stat -- docs/** CLAUDE.md` shows insertions only (CLAUDE.md +1
  line, pitfalls.md +2, rendering.md / testing-eyes.md net additive text swaps, no content removed beyond the
  stale QoS-raise sentence the revert correctly retracted).
- CLAUDE.md: 24,522 bytes, under the 25,000 cap.
- `git diff --name-only 3a9e166..HEAD`: exactly the 10 files above -- no stray .venv symlink, no leftover
  instrumentation script, no env-var hook left over from the VU15 diagnosis (rule (a) fired, so the plan's
  rule-(b) TEMPORARY instrumentation arm was correctly never committed).
- Real-time rules: the revert only removes a QoS-raise call from two `newOpenGLContextCreated` overrides
  (message-thread/GL-thread setup callbacks, not the render hot path) -- no new mutex, no waiting primitive added.
- Gates are RED-able: both VU16 and VU17 were independently observed RED on the pre-lane app and GREEN on the
  lane app in the raw logs above, not merely asserted in prose.

## Findings

- [NIT] docs/claude/pitfalls.md's new entry is still numbered "NN (vupload)" (placeholder). Pitfall 59 is
  already taken by g4cpu on main (per this session's git log), so this lane's entry will collide with 59 if
  merged as-is. The builder's own NEXT ACTION already flags this ("needs 60 or the next free number") as a
  merge-time rebase step, so this is not a fix-round omission -- just re-confirming it must be resolved (renumber
  + re-grep any cross-references) in the same commit/step as the main-rebase, before this lane lands, or the
  pitfall index will have two "59"s / a dangling "NN".

No MUST-level defect found in this fix round: every VU14-VU17 ruling is implemented exactly as pre-registered,
the decision arithmetic for VU15 was re-derived from raw first-party run data (not recalled from the commit
message), the new/changed gates (VU16, VU17) are demonstrably RED-able and were observed RED on the correct
pre-lane baseline, docs are additive and match the reverted code with no overclaiming, ctest is green at 962/962,
and no stray artifacts were left in the tree.

SUMMARY: 10 files in the fix-round delta reviewed (plus whole-lane docs/CLAUDE.md hygiene pass), 0 blocking
issues, 1 NIT (pitfall number placeholder, already tracked for merge-time resolution).
METADATA: reviewer=reviewer-vupload-gates-r2, builder_packet=vupload, lens=gates, round=2, date=2026-09-29

## Cross-check with sibling lens=gl round-2 review

The parallel gl-lens r2 review (`review-vupload-gl-r2.md`) flags a SHOULD I independently confirm: the lane
report's NUANCE (a) / table row "VU14 kWriterLookAhead adopted" says "Plan section 6's line now reads 'reseek
geometry measured from the writer's look-ahead...'" -- I read plan-vupload.md:481 directly and that line (part of
the original MUST-NOT-CHANGE body) is UNCHANGED; the "measured from the writer's look-ahead" wording exists only
in ADDENDUM 2 (plan-vupload.md:726-727), which overrides the body per the addendum's own header, not as an edit
to section 6 itself. This is a wording overclaim in the lane report (conflating "the addendum overrides section 6"
with "section 6 was rewritten") -- does not affect gates, code, or the VU14 ruling's validity (which is correctly
adopted and documented in rendering.md/pitfalls.md), so it does not change my verdict, but I agree it should be
corrected in the lane report before merge. Adding as a second NIT rather than duplicating the sibling's finding.

FINDINGS (consolidated):
1. [NIT] docs/claude/pitfalls.md "NN (vupload)" placeholder collides with Pitfall 59 (taken by g4cpu on main) --
   already tracked as a merge-time rebase step in the lane report's NEXT ACTION.
2. [NIT] Lane report NUANCE (a) overclaims that "plan section 6's line now reads..." -- the body line is unchanged;
   only ADDENDUM 2 carries that wording. Cosmetic (report accuracy), not a gate/code defect. Cross-confirmed with
   the sibling gl-lens r2 review.
