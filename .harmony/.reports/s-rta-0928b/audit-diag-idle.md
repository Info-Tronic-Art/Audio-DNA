# Independent Method Audit — lane diag-idle
STATUS: DONE
VERDICT: SOUND_WITH_GAPS

## 0. Critical process finding: report.md does not exist on disk

`scratchpad/diag-idle/report.md` (the path the audit packet names, and the
path this lane's own tools/instrumentation are keyed to) **does not exist**.
`ls`/`find` across the whole scratchpad tree and the `rta0928b-idle` worktree
confirm no `report.md` anywhere. This is not a stale-cache issue — the
scratchpad's last-modified time (23:09:30) is minutes before this audit ran
(23:12:44), so the tree is current.

The content exists, but only inside the harness's own workflow journal:
`~/.claude/projects/-Users-boriskarpman-projects-RealTimeAudio/<session>/subagents/workflows/wf_5e0566ce-03d/journal.jsonl`,
`result` entry, field `report_markdown` (the full report text) plus a
structured `causes[]` array. The diagnosis agent's own `errors_or_deviations`
field states why: *"The report file: the harness refused a Write of
report.md from this subagent ('return findings as text'). The report is
this text."* — i.e. the Diagnose-phase agent tried to persist the deliverable
and was blocked by its own tool grant, then correctly fell back to returning
it as a structured result. The gap is downstream: whatever orchestration step
is supposed to take `result.report_markdown` and materialize it at
`report.md` never ran (or doesn't exist yet for the Diagnose phase). Sibling
lane diag-media has both a `report.md`-equivalent AND two audit reports
(`audit-diag-media.md`, `audit-diag-media-r2.md`) already in
`.harmony/.reports/s-rta-0928b/`, so the convention clearly expects a
persisted file; diag-idle is the one that silently didn't get it.

**Consequence**: any consumer that follows the documented protocol ("read
`report.md`") — this audit, Harmony's own tracking, a human — finds nothing
and could wrongly conclude the lane is incomplete or stalled. The work log
(`/Users/boriskarpman/projects/RealTimeAudio/.harmony/s-rta-0928b-work.md`
line 24) in fact still shows the last diag-idle status as *"lane still
bisecting"* (19:42) with no later completion entry, even though the journal
shows the lane finished (`status: DONE`) later that evening. This is exactly
the "prose is not delivery" failure mode called out in the Reviewer/Builder
disk-drop contract, applied here to a Diagnose-phase agent.

**MUST**: fix the harness step that consumes a Diagnose-phase `result` so
`report_markdown` is always written to the packet's named report path (or
change the packet instructions to point at the journal). Until then, treat
"no report.md" as ambiguous, not as "lane failed" — check the workflow
journal before concluding a diagnosis is missing.

I recovered the full report via the journal and audited its actual content
below, since the underlying diagnosis work is real and substantially
verifiable against the evidence directory, and blocking on the missing file
alone would waste a completed, well-evidenced diagnosis.

## 1. Counterfactual proof quality (task's core question)

Verified directly against `runs/<arm>/r*` directory counts (not taken on
faith from the report): every named counterfactual arm has exactly 5 clean
launch directories (`base, base_v1, base3, deflt, floor, many16,
many16metal, many16noanim, many16stat, metal, noanim, nohb, nols, nosig,
notop, nowave, testmode, activity` = 18 arms × 5), plus 4 probe arms
(`ctx, ctx16, grid, grid16`) × 3. Tainted launches exist and were correctly
excluded (`runs/{ctx,floor,stock,base,activity,many16,many16metal}/tainted-*`),
matching the report's claim that compiler-contaminated launches were
re-run. This is genuine >=5-launches-per-arm counterfactual work, not a
single-shot anecdote.

- **C1 (BODY union repaint) — PROVEN.** Arm `noanim` (5 launches):
  lag 258→4.1 ms/s, window-max 17.9→1.6 ms, busy 334→67 ms/s (card);
  `many16noanim`: 466→2.1, 17.8→1.0, 537→65. Numbers move by >90% in the
  predicted direction on two independent fixtures. VERIFIED against
  `summary/lagclass-base.txt` and `summary/lagclass-noanim.txt` — every
  number quoted in the report (e.g. "busy 241.1, 72.5%, 75.5%") is an exact,
  character-for-character match to the source file, not a paraphrase.
- **C2 (FULL-window TopBar collision) — PROVEN.** Arm `notop` (5 launches):
  FULL-pass rate 3.35→0.76/s, window-max 17.9→13.7 ms, lag 258→198 ms/s.
  Matches `summary/lagclass-notop.txt` exactly.
- **H1 (GL thread holding the message lock) — REFUTED, and independently
  re-checked by me**: I grepped kind-12/13 (lock-wait / paint-under-lock)
  records across *every* run directory in the lane (not just the 40 launches
  the report cites) and found **0** matches, corroborating the refutation
  with a wider sample than the report itself used.
- **H4 (delayed wake / App Nap)** — arm `activity` (beginActivity
  UserInitiated|LatencyCritical) shows no change (17.9 vs 17.9 ms,
  334 vs 334 busy) — a real counterfactual, correctly refuting the
  hypothesis rather than just asserting it.
- **C4, H1's "0 waits" figure, X1** are labeled VERIFIED without a dedicated
  counterfactual arm; the report itself flags this ("no counterfactual
  needed") because they are direct per-call/per-message attribution
  (profiler-style accounting), not causal hypotheses requiring an ablation.
  That's a legitimate third evidentiary category the task's PROVEN/INFERRED
  framing doesn't name explicitly — **NIT**: worth a one-line note in future
  reports distinguishing "directly measured" from "counterfactual-proven"
  from "INFERRED," since the report currently uses one VERIFIED label for
  both of the first two.
- **R1 (ClipInspector residual)** is honestly labeled **INFERRED** (no
  dedicated counterfactual, attributed by timeline only) — correct, not
  oversold.

## 2. Could the instrumentation itself have created or hidden the effect?

Addressed with two independent checks, not just assertion:
1. Arm `nohb` (heartbeat probe disabled): busy 338 vs base3 349 ms/s (~3%
   difference) — the heartbeat mechanism itself is not the source of the
   200-300 ms/s of measured busy time.
2. `ps -M` sampling of a genuinely **unmodified** stock build
   (`summary/ps-stock.txt`, median main 331.7 ms/s) vs the same sampling on
   the instrumented `base3` build (`summary/ps-base3.txt`, median main
   343.8 ms/s) — a ~3.5% overhead bound from a measurement method that does
   not depend on any of the report's own instrumentation. This is the right
   way to bound observer cost (an orthogonal measurement, not the same
   ruler measuring itself).

Drain/sampling I/O runs on a dedicated `diag.drain` background thread with
buffered `fprintf`, not on the message thread (confirmed by reading
`DiagIdle.mm`); the heartbeat and per-call `Scope`/`record()` hooks only do
an atomic fetch-add + struct write into a preallocated lock-free ring — no
allocation, no I/O, no lock, on the hot path. This matches the file's own
header comment ("Everything is inert unless ADNA_DIAG_FILE is set") and
Sacred-Rule-1-style hygiene. **VERIFIED** by reading `src/diag/DiagIdle.mm`
and `src/diag/DiagTrace.h` directly (via `instr.diff`) — the claimed design
matches the actual code.

## 3. Arithmetic

Checked `summary/lagclass-base.txt` line items against the report's cause
table: C1 busy 241.1 (72.5%/75.5% lag), C2 64.3 (19.3%/23.0%), C3 13.5
(4.1%/1.1%), C4→"sources0" 10.6 (3.2%/0.1%) all match exactly, and summing
all listed classes reproduces the file's own "TOTAL busy 332.8 ms/s"
(components sum to 332.7, rounding) and matches the report's stated total.
No arithmetic errors found in the primary attribution table.

One **SHOULD**-level looseness: the report states "106 clean launches in
all." Counting `runs/*/r*` directories directly gives 18 arms×5 (90) + 4
probe arms×3 (12) = 102, or 107 if `stock` (5) is added, or 110 if `smoke*`
(3×1) is also counted — none of these reconcile exactly to 106. Not
material to any causal claim (every individual arm's launch count is
independently verified above), but it means the report's own top-line
launch-count is not reproducible by directly counting its evidence
directory — a minor self-consistency slip worth fixing in the next report's
QA pass.

A second **NIT**: per-event median cost × rate does not equal the busy
ms/s share for C1 (7.86 ms × 26.32/s ≈ 207 vs the reported 241.1 ms/s).
This is not an error — busy ms/s is computed from actual per-event sums
(mean ≈ 9.16 ms, consistent with the right-skewed p90=13.39/max=27.39 the
report itself prints) — but a reader doing the "cost × rate" sanity check
the task prescribes will hit an apparent mismatch unless they notice the
report never claims median×rate=busy. Worth a one-line disclaimer in future
reports ("ms/s is a sum, not median×rate") to head off exactly this
false-positive.

## 4. Spot checks against raw evidence (not just the report's summaries)

- `grid-card.txt` / `grid-many16.txt`: TRUE dirty share = 1.00 for every
  BODY and FULL pass — confirms the "100% of the union dirty" claim
  verbatim.
- `rects-nosig-r1.txt` / `rects-nowave-r1.txt`: the exact rect tuples quoted
  in the report ("(4,246,378,771)", "(4,39,1720,347)") are present
  character-for-character in the raw files.
- `dispphase-metal.txt`: post-paint (CoreAnimation) cost 4.3 ms/s vs base's
  71.4 — exact match to the report's claim that metal removes the
  post-paint tax.
- `found_not_fixed #1` (597 ms one-off stall): I grepped
  `runs/many16metal/r3/diag.tsv` directly and found real `hb` (heartbeat)
  records with durations 570.2 ms and 489.6 ms — the 489.6 also appears
  verbatim as `win_max_max` for `many16metal` in `summary/arms-table.md`.
  This anomaly is real, not fabricated, and is correctly flagged as
  unresolved rather than swept into the main causal story.

No evidence file contradicted a report claim in anything I checked.

## 5. RED-able gates — are they real?

Yes. Current main (base/base_v1/base3, all built from main `b9c9ab2`) reads
window-max 17.9 / 21.4 / 22.6 ms and busy 334-350 ms/s against the proposed
`IDLE-HB-card` thresholds (<=8 ms, <=150 ms/s) — genuinely RED, confirmed
directly from `summary/arms-table.md`, not just asserted. `IDLE-HB-many16`
(17.8 ms / 537 ms/s vs the same thresholds) is also genuinely RED. The
report is honest that the gate requires the heartbeat+CFRunLoopObserver
harness (or an equivalent) to execute, not a bare `ctest` — it does not
oversell this as a drop-in existing-CI gate.

## 6. Scope/fidelity of the instrumentation vs its own description

`instr.diff` and `src/diag/DiagIdle.mm`/`DiagTrace.h` were read directly.
The METHOD paragraph's description (lock-free record ring drained by a
background thread, message-thread heartbeat via `callAsync`, two
`CFRunLoopObserver`s at `LONG_MIN`/`LONG_MAX`, per-thread CPU via mach
`THREAD_EXTENDED_INFO`) matches the actual code exactly. All five UI-side
diff hunks (`ClipCell.cpp`, `LayerStrip.cpp`, `SignalBar.cpp`, `TopBar.cpp`,
`WaveformDisplay.cpp`) are inert unless the corresponding `ADNA_DIAG_*` env
var is set to `"1"`, and default (unset) behavior is a no-op passthrough —
consistent with "everything is inert unless ADNA_DIAG_FILE is set." Tree
end-state (`git status` in the worktree = only `?? build-lane/`) confirms
the diff was reverted from the tracked tree as claimed.

## Findings summary

- **MUST** (process, not diagnosis content): `report.md` was never written
  to disk anywhere in the lane's scratchpad or worktree; the only copy of
  the report is `result.report_markdown` inside the workflow journal. Fix
  the harness's Diagnose-phase result-persistence step, or point future
  audit packets at the journal directly.
- **SHOULD**: the report's self-reported "106 clean launches" does not
  reconcile against a direct count of its own evidence directories
  (~102-110 depending what's included); tighten the launch-count bookkeeping
  in the next report.
- **NIT**: `VERIFIED` is used for both counterfactual-proven causes (C1, C2,
  H4) and directly-measured/no-counterfactual-needed causes (C4, H1, X1);
  a third label would remove ambiguity.
- **NIT**: per-event median×rate will not reproduce the reported busy ms/s
  (means, not medians, back the totals) — worth a one-line disclaimer to
  pre-empt a false "arithmetic doesn't add up" read.

## Verdict rationale

The diagnosis itself — once recovered from the journal — is methodologically
sound: real ablation counterfactuals at >=5 launches/arm on two fixtures,
an honest VERIFIED/INFERRED split, two independent instrumentation-overhead
bounds (nohb arm, stock-binary ps sampling) that directly answer the
"could the probe have caused this" question, and every quoted number I
spot-checked against raw evidence matched exactly with no fabrication or
contradiction found. That earns SOUND on content. But the report-delivery
failure is real, material to how this lane is meant to be consumed by
downstream automation/humans, and already caused the work log to go stale
("lane still bisecting" with no completion entry) — that keeps this out of
a bare SOUND.
