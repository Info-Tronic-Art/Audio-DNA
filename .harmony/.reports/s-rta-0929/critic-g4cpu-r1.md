# Critic Verdict -- lane g4cpu, round 1
STATUS: PARTIAL
VERDICT: FAIL
HEAD: 42e3c93ae76df9782a5448e99319db60183bec1b (worktree .claude/worktrees/rta0929-g4cpu)

## Verdict
FAIL (MUST). The claim under review -- "routine pad / band hairlines / V faders
(routine cue fill) / bound knobs look and move EXACTLY as on main while CPU
drops" -- is not supported: (a) no CPU-reducing code is actually committed at
this HEAD, and (b) the single evidence frame supplied is not the before/after
pair at injected beat positions the task asks the critic to judge, and is not
even from this lane's own routine-identity test.

## Findings

[MUST] Spec/claim fidelity -- "CPU drops" is false for this HEAD.
`git diff --stat ad35de1 HEAD` on the pinned worktree shows the only commit
after `ad35de1` is a 254-line doc-only report (`.harmony/.reports/s-rta-0929/g4cpu.md`,
0 code changes). `ad35de1` itself is described by the lane's own report as
"TEST-ONLY witnesses, with no behaviour change" (g4cpu.md:50-51) -- pure
counters/instrumentation gated under `AUDIODNA_TEST_SERVER`. The one candidate
that actually changed CPU (`c2`, a RoutinePad paint-key patch) was run once as
an uncommitted experiment and explicitly reverted (g4cpu.md:9, "I measured c2
once... I did not commit it, and I reverted it"), then STOPPED by the plan's
own pre-registered rule 3.3(iii) (RED margin 7.7 < 8, g4cpu.md:124-125,144).
There is no CPU-reduction change in the tree for a critic to check a visual
side effect against -- the premise handed to this seat does not match the
lane's own disk-recorded result. Fix: re-issue the look request only after a
CPU-affecting change actually lands (options a/b/c in g4cpu.md section 6 need
a Harmony ruling first), or correct the task framing to "test-only
instrumentation, no CPU change, verify zero visual delta" if that is what is
actually intended.

[MUST] Staged-test hygiene / evidence sufficiency -- the "two injected beat
positions (v5)" before/after pair does not exist for this lane. The one PNG
supplied (`look-v1-after-S2.png`) is, by the report's own account, a reused
frame from the pre-existing `probe-idle-paint` v1 test ("I looked at a sample
frame (v1 after-S2, test mode, card fixture): normal UI, the ROUTINES row, no
Output window" -- g4cpu.md:200-201), taken only for the screen-safety check
(no stray Output window), not a purpose-built routine-identity capture. The
report's own UNKNOWNS section lists v5 as not run: "UNKNOWNS-NOT-DONE: c2
(commit, ctest, g4 gate, v5)... are all not done (STOP)" (g4cpu.md:35), and
lists `v5_row.py` as a scratchpad draft only, itself blocked on an unmerged
`recordUiGeometry` change ("It needs `preview_rect` added to recordUiGeometry,
to mask the preview's opacity render" -- g4cpu.md:216-217). There is therefore
no BEFORE frame, no AFTER-at-injected-beat-2 frame, and no diff image to run
the requested identity check against -- one static frame cannot answer
"is anything stale / shifted / updating at a different cadence."

[SHOULD] Frame content does not clearly show a routine playing. In the
supplied frame, L1/L2 rows show pad thumbnails (`solid_color`, `checkerboard`)
and routine columns 1/2, but neither V fader shows the chartreuse routine-cue
fill CLAUDE.md documents for a playing band ("its V fill turns the routine cue
while a routine's hand grips opacity" -- CLAUDE.md "Routine pads and bands"),
and no band hairline is visibly highlighted. This reads as an idle/pre-trigger
layout, not "non-trivial state" (a routine actually playing). Cannot confirm
from this frame alone; would need a frame captured mid-playback (ideally the
missing v5 pair) to verify.

[Not independently gradeable] Pixel-gate coverage of "the regions Boris
watches" -- no pixel-diff artifact for the routine pad / hairline / V-fader
regions was supplied for this lane (the existing `probe-idle-paint` G10
re-run in g4cpu.md section 5 is a general idle-paint regression, not scoped to
these routine-cue regions specifically). Cannot assess without the v5
diff output.

## What would flip this to PASS
1. Harmony's ruling on g4cpu.md section 6 lands (c2 committed, or another
   CPU-saving change lands) so "CPU drops" is true of HEAD.
2. The v5 before/after pair (both injected beat positions) plus a diff image
   scoped to the routine pad / band hairline / V-fader / bound-knob regions
   is actually produced and supplied to this seat.
3. At least one frame in evidence visibly shows the chartreuse routine-cue
   state (a routine actually playing), not an idle layout.

METADATA: reviewer=critic-panel(visual+ux+logic), lane=g4cpu, round=1,
worktree_head=42e3c93ae76df9782a5448e99319db60183bec1b, date=2026-09-29
