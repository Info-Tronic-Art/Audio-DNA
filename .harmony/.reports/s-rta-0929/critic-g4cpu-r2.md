# Critic Verdict — g4cpu round 2 (visual + UX + logic seat)
STATUS: DONE
VERDICT: APPROVE (no MUST)

## Scope note (important)
The dispatched task framed this as "the lane claims ... CPU drops." That framing is STALE: round-1 critic
MUST-1 already found "CPU drops" false for HEAD and the lane REFRAMED the claim away
(.claude/worktrees/rta0929-g4cpu/.harmony/.reports/s-rta-0929/g4cpu-fix.md §1 row 1). Verified on disk: commit
ad35de1 (c1, attribution counters) and 2df6876 (v5 identity test) are the only two commits since r1; both are
test-only (`git show --stat` on both — c1 touches UiPaintCounters.h/ApiServer.cpp/MainComponent.cpp/UI files
adding atomic counters + a debug-server ring buffer; 2df6876 adds only a `previewRect` geometry line, gated
`#if AUDIODNA_TEST_SERVER`). I reviewed against the ACTUAL current claim (pixel identity of a routine-playing
frame, not a CPU number), not the prompt's stale framing.

## Findings (disk-verified, not recalled)

1. [SHOULD] Task-framing drift: the dispatch text should be regenerated from the current lane report, not a
   pre-reframe snapshot, so future critic seats don't re-litigate an already-reframed claim.
   → fix: harness should read g4cpu-fix.md §1 before composing the round-2 critic packet.

2. [SHOULD] Bound-knob region is not covered by any of the 9 evidence frames — confirmed by direct inspection
   (no UniversalParamControl in any frame shows the chartreuse ROUTINE-cue hint; the only routine-driven controls
   pictured are the LayerStrip V faders). This is HONESTLY DISCLOSED in g4cpu-fix.md §3, with a source-level
   argument I independently verified on disk: `git show ad35de1 -- '*UniversalParamControl*'` shows the ONLY
   change to that file is one atomic-counter line placed strictly AFTER the existing `repaint()` call in
   `updateValueDisplay()` — no paint-logic touched. Given that, the disclosed gap is low-risk, not a hidden
   overclaim. → Harmony should make the explicit call named in the report's HANDOFF-NEEDS #2 (accept the
   source-level argument, or order the two-build `AUDIODNA_DEBUG_LAYER` hook arm) rather than leave it implicit.

3. [NIT] FPS counter text differs between paired captures (v5-before-P1 FPS:108 vs v5-after-P1 FPS:101; both
   P2 frames read FPS:101) — this is a live perf readout, correctly excluded from the identity gate via the
   "fps mask" per the lane's own tooling. Confirmed this is the ONLY source of difference in the P1 pair by an
   independent pixel diff (see Verification below) — not a rendering regression.

## Verification performed (executed, not eyeballed)
- `PIL.ImageChops.difference` on all 4 full-window before/after pairs:
  - before-P1 vs after-P1: bbox (536,92)-(3314,244), 0.0014% of pixels differ, max channel delta 91 — isolated
    to the "FPS:108"/"FPS:101" glyph region; the routine pad, strip column, and preview crop I cut out of that
    bbox are pixel-identical.
  - before-P2 vs after-P2: 0 px differ anywhere (byte-identical files).
  - This matches (and independently confirms) the lane's own `v5_routine_identity` PASS lines (0 px differ,
    max delta 0, at both P1 and P2) and its "cue" line (12213/14973/12213/14973 routine-cue px, before==after
    at each position).
- Temporal (same-build, P1 vs P2) diffs to test staleness/cadence: before-P1 vs before-P2 = 2.723% pixels
  differ; after-P1 vs after-P2 = 2.721% pixels differ — near-identical magnitude, confirming the candidate
  animates the SAME amount as main between the two injected beat positions (not stale, not a reduced cadence).
  Confirms "diff-v5-teeth*.png" is a P1-vs-P2 temporal diff (proving live motion: V-fader cue top, band
  hairline, pad sweep + digit), NOT a main-vs-candidate mismatch — I misread it as the latter until I re-derived
  it by computing the same diff myself from the labelled source frames.
- Regions crop (2x-scaled) confirms the magenta highlight in `v5-teeth-regions.png` sits exactly on: the top
  slice of each L1-L3 "V" fader fill (the routine's hand), the hairline under each "Sweep" band label, and the
  pad's teal sweep + beat digit — i.e., precisely the "routine pad / band hairlines / V faders (routine cue
  fill)" set the task asks about; nothing in the TopBar/knob row differs between P1 and P2 in either build.

## Checklist answers
- BEFORE vs AFTER identity at both beat positions: YES, pixel-identical outside the disclosed FPS-text noise.
- Anything stale / shifted / different cadence: NO — temporal diff magnitude matches between builds (2.72% vs
  2.72%), and the routine-cue pixel counts match exactly (before==after) at each position.
- Frames show a routine actually playing (non-trivial state): YES — pad "2/4"→"3/4", chartreuse V-fill +
  band name on L1-L3, cue pixel counts >=300 in all 4 frames.
- Pixel gate covers the regions Boris watches: routine pad, band hairlines, V faders — YES. Bound knobs — NO,
  not reachable by this fixture (disclosed, source-argument verified; not a hidden gap). No MUST because the
  gap is disclosed and the compensating source-level argument checks out on disk.

FILES REVIEWED:
- /private/tmp/.../scratchpad/g4cpu-fix/evidence/{v5-P1-regions,v5-P2-regions,v5-teeth-regions,v5-before-P1,
  v5-after-P1,v5-before-P2,v5-after-P2,diff-v5-teeth,diff-v5-teeth-preview}.png
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-g4cpu/.harmony/.reports/s-rta-0929/g4cpu-fix.md
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-g4cpu (git show ad35de1, 2df6876; src/ui/UiPaintCounters.h)

METADATA: reviewer=critic-panel(g4cpu,r2), builder_packet=lane/g4cpu@c1d216c, date=2026-09-29
