# Reviewer Verdict — outputs-c1-conformance-r2
STATUS: DONE
VERDICT: PASS_WITH_NITS
SCOPE: git diff e14027ceef295f02cf9747ae68f33771fbcf4277..5e1b7a18e67f3baa0f2bd12c0072125feccb4e43 (Builder A's commit only; 79b4410 by the concurrent second executor is explicitly OUT of this review's scope per the packet)

## Summary
5e1b7a1 is a docs/evidence-only commit: no source, no test, no build file changed
(git diff --stat: 3 files, +73/-0 — fixround-720p-border-check.py, .txt, and an
appended section of outputs-c1.md). Because it touches zero source, the C1-scope,
fence, MUST-NOT-CHANGE, and "legacy OutputRenderer gone" checks are trivially
satisfied for this delta (nothing to violate). The substantive content is a
refutation of the critic's MUST (720p grey border) and a note on the SHOULD
(Release jassert no-op).

## Independent verification performed
- Reproduced the evidence script byte-for-byte in a $TMPDIR sandbox: extracted
  `fixround-720p-border-check.py`, the 5 referenced PNGs, and
  `media/P16_02_Screen_Split_2x2.png` via `git show <rev>:<path>` (never touched
  the worktree), then ran the script. Output was IDENTICAL, line for line, to
  the committed `fixround-720p-border-check.txt` and to the numbers quoted in
  the report — confirms the evidence is real and reproducible, not fabricated
  or cherry-picked.
- Verified `media/P16_02_Screen_Split_2x2.png` sha256 (8b82185a...) matches
  both `main` and the reviewed commit, supporting the "same file as main"
  claim.
- Opened `src/ui/OutputWindow.cpp` at base (e14027c) via `git show` and
  confirmed the SHOULD note's claim: the ctor's `jassert` is at the stated
  location and the Release-mode guarantee genuinely rests only on the
  `getDesktopWindowStyleFlags()` override — matches the report's description
  exactly (no source drift, no overclaim).
- Opened `TEETH-test_output_law-guards.txt` at base and confirmed the exact
  mutant the report cites ("flag only named in the jassert (dropped from the
  style flags)" → "1 failed") is present verbatim — the SHOULD note's evidence
  citation is accurate, not paraphrased into something stronger than what the
  teeth file shows.
- Attacked the MUST refutation for the strongest alternative explanation (a
  real capture-margin artifact that happens to coincide with the fixture's own
  gutter in size): the reported `d(capture, imageB stretched)` values (0.009,
  0.029 on a 0-255 scale) are near-zero across the WHOLE frame, not just the
  margin — a coincidental-color capture artifact overlaid on correctly-stretched
  content would not produce that. The control (`f0` vs image B, d=29.4) shows
  the metric correctly discriminates when content differs. The refutation
  holds.
- Checked precedent: standalone `.py` evidence/verification scripts committed
  under `.harmony/.reports/<slug>/<name>-evidence/` already exist elsewhere in
  this repo's history (`s-rta-0926b/routines-followup-evidence/stop-witness.py`,
  `mutation-3a.py`, `render-evidence/diag.py`) — this commit's placement and
  shape match established project convention, not a new pattern.

## Findings

[NIT] Readability/doc-hygiene — outputs-c1.md, appended section: a top-level
`## Fix round (outputs-c1-fix, round 1)` header is followed by `STATUS: PENDING`
and five unfilled stub headers (`### F1.` … `### F5.`), then immediately by a
second, differently-titled `### Fix round (outputs-c1-fix, critic round 1: ...)`
subsection carrying its own `STATUS: DONE` with the real content. The outer
`STATUS: PENDING` is never flipped and the five stub headers are never filled
or removed — a leftover skeleton draft left in the committed file. Not
blocking (no functional/gate impact — nothing in the repo parses this file's
STATUS token programmatically as far as I could find), but it leaves two
conflicting STATUS lines in the same section, which will confuse the next
human or agent reader. Fix: delete the F1-F5 stub block and the stale outer
`STATUS: PENDING` line, or merge the outer header directly into the filled-in
subsection.

No MUST or SHOULD findings. The MUST refutation and the SHOULD note are both
independently reproduced/verified accurate.

## Notes for Harmony (not part of the verdict, informational)
- The reviewed commit predates 79b4410 (the concurrent second executor) by
  ~1.5 minutes; the report's silence on that commit is expected and correct
  for this diff, not an omission.
- This round's builder correctly declined to add a new content-oracle probe
  row to gate `render_frame`, on the grounds that `render_frame` is pre-existing
  code outside C1's fence (MUST-NOT-CHANGE §14) and the offline decode already
  settles the claim — this is the right scope call, not scope-avoidance.
METADATA: reviewer=review-agent, builder_packet=outputs-c1-fix-round2, date=2026-09-27
