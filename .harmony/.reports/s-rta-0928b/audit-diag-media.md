# Independent Method Audit — diag-media (s-rta-0928b)
STATUS: DONE (audit cannot proceed past the artifact-existence gate)
VERDICT: UNSOUND (as an audit-of-claims request — the artifact under audit does not exist)

## Headline finding (verified, not inferred)

`.../scratchpad/diag-media/report.md` — the file the task packet names as the
object of this audit — **does not exist**, on disk, anywhere. This was
checked three ways, all negative:

1. Direct read attempt on the exact path given in the task text: `File does not exist.`
2. `find` across the entire `diag-media` directory tree (top-level, `apps/`,
   `media/`, `runs/`, `tools/`) for any `*report*` file: zero hits.
3. `find` across **every** scratchpad session directory under
   `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/`
   (5 session UUIDs total) for `report.md` anywhere: the only hit is
   `.../scratchpad/diag-idle/report.md` (a *different* lane). No `diag-media`
   report exists under any session.
4. Cross-checked `.harmony/.reports/s-rta-0928b/` (the lane's presumed
   permanent-report destination) and the `rta0928b-media` worktree's own
   `.harmony/.reports/` tree: neither contains a diag-media synthesis
   document either — only pre-existing reports from earlier slugs
   (s-rta-0923, s-rta-0926, N12-rta) that predate this lane.

The evidence directory (`runs/`, `tools/`, `instr.diff`) is real and
substantial (raw per-launch JSON/log data for arms `v0/v1/v2` ×5,
`i0/i1` ×5, `k0` ×6, `q0/q1` ×5, plus `agg-*.txt`/`aggregate-*.json`
summaries and `seekstats.{txt,json}`, all last-modified 2026-09-28
17:22–20:08). What is missing is the **synthesis document** — the
write-up that would state "X causes Y," attribute a mechanism, and
propose RED-able gates. That document was never written (or was written
somewhere this audit could not locate despite an exhaustive search).

## Why this is disqualifying for the requested audit

The task asks this audit to judge, for each **claimed cause** in the report:
proven-vs-inferred, observer-effect confounds, arithmetic correctness,
evidence contradictions, unattributed claims, and RED-able-gate reality.
All six of those checks are checks *against a document*. With no document,
there is nothing to run them against — the audit's object-level task is
impossible to perform, not merely incomplete. This is a different failure
mode than "the report is thin" or "the report overclaims": there is no
claim to weigh at all.

**Most likely explanation** (inferred, not verified): the dispatch that
triggered this audit fired before the diag-media lane finished its
synthesis step. The evidence timestamps top out at 20:08 (the two
`aggregate-*.json` files and `agg-*.txt` summaries), consistent with the
lane having just finished computing aggregates when it was interrupted,
audited, or otherwise never got to the write-up. The `s-rta-0928b-work.md`
log (top-level work log) confirms diag-media (workflow `wp8qup9nt`,
worktree `rta0928b-media`) was launched at 17:21 alongside three sibling
lanes but the log's diag-media-specific narration stops before any
"report written" entry.

**Alternative explanation considered and rejected**: that the report was
written and then deleted/moved. There is no trace of this — no `.orig`,
no git history entry for a `report.md` under scratchpad (scratchpad isn't
git-tracked at all), and no reference to a completed diag-media report
anywhere in `s-rta-0928b-work.md` or the `.harmony/.reports/s-rta-0928b/`
directory's other files (`sampleat.md`, `plan-seqvram.md`, two attack
papers — none mention diag-media conclusions).

## What the raw evidence DOES support checking (partial credit, done anyway)

Since re-running the app was disallowed and the primary object was
missing, I spent the remaining budget attacking the two things that don't
require a report: (a) whether the *raw data* structurally meets the
audit's own bar, and (b) whether the *instrumentation* has an observer-effect
problem baked in regardless of what any future write-up would claim.

**(a) Sample-size hygiene, structurally: PASS.** The run directories show
≥5 launches per arm exactly as the task's bar requires: `v0_1..v0_5`,
`v1_1,v1_2,v1_4,v1_5,v1_7` (5 of 7 attempts — 2 presumably discarded/failed),
`v2_1..v2_5`, `i0_1..i0_5`, `i1_1..i1_5`, `k0_1..k0_6` (6), `q0_1..q0_5`,
`q1_1..q1_5`. `runs/agg-v.txt` shows per-arm summaries labelled
`== v0_ 5 launches [...]` confirming `aggregate.py` itself enforces/reports
the arm size. This is good methodology hygiene — but it says nothing about
whether any eventual causal claim is correct, since no claim exists yet.

**(b) Observer-effect risk in the instrumentation itself: FOUND, verified
by reading `instr.diff` directly (not inferred).** `DiagTrace.h`
(`.claude/worktrees/rta0928b-media/src/diag/DiagTrace.h`) and its call
sites in `src/render/Renderer.cpp` contain two unconditional (no minimum-
duration gate) `fprintf(stderr, ...)` calls on the **GL/render thread**,
the exact thread this project's own CLAUDE.md calls "sacred" for the audio
callback and budget-constrained (<8ms) for render:

- `DgFrame`'s destructor (Renderer.cpp `renderOpenGL()`, one `[GF]` line
  **every frame**, no threshold — unlike `Scope`, which only prints when
  `dur >= minMs`).
- `DgVid`'s destructor (`Renderer::syncMedia`, one `[GV]` line **every
  decode/advance call**, i.e. once per video clip per frame while any
  video clip is active) — also unconditional.

Both prints fire *after* the frame's own `frameMs` metric is captured
(`frameMs` is computed via `high_resolution_clock` before `dgFrame`'s
destructor runs), so the stderr I/O cost is invisible to any
`frameMs`-based conclusion but still consumes real wall-clock render-
thread time before the next frame can start. Concretely: if a future
report uses `frameMs` (or the `[DG]`-scope timings) as ground truth for
"video decode costs Xms," the fixed per-call/per-frame stderr-write
overhead is a confound that scales with clip count (more video clips →
more `[GV]` lines/frame) and would be indistinguishable, from `frameMs`
alone, from a genuine per-clip decode cost. The `agg-v.txt` data is
consistent with this concern in shape (not proof) — e.g. `v4kx4` shows
`fps=36.0` alongside `cb_max=91.823`, `iv_max=96.026` — a real stall, but
the aggregate doesn't separate "decode cost" from "N × stderr write cost"
anywhere in the tool outputs inspected (`aggregate.py`, `seekstats.py`
were not read in full for time; flagged as a gap, not a proven bug).

This is a **MUST**-level flag for whoever eventually writes the diag-media
report: any claim of the form "video decode/media checks cost N ms" must
first confirt it isn't (partly) measuring `[GF]`/`[GV]` stderr-write
overhead, since those two print sites are the only unconditional
(non-thresholded) I/O in the diff, on the one thread this project's own
rules call sacred-adjacent (render thread budget <8ms).

## Verdict

**UNSOUND** — not because any specific causal claim is wrong (none exist
to check), but because the audit's precondition — a completed report
articulating claimed causes — is false. The instrumentation and raw data
are real, well-structured (≥5 launches/arm, matches task's own bar), and
plausibly usable, but nothing has synthesized them into a claim yet, so
none of the requested checks (proven-vs-inferred, arithmetic, evidence
contradiction, unattributed claims, RED-able gate reality) can be
performed. Recommend: re-dispatch the diag-media lane to completion (or
locate wherever its write-up actually landed, if it exists outside the
paths this audit could reach) before requesting this audit again — and
when it lands, apply the observer-effect check above to any `frameMs`-
based media-decode claim first, since that is the one confound this audit
could establish independent of the missing report.

## Scope note

Per instructions, the app was not re-run; only static inspection of
report.md's (non-)existence, the `runs/` evidence tree, `instr.diff`, and
`s-rta-0928b-work.md` was performed. `tools/*.py` (aggregate.py, analyze.py,
drive.py, pool.py, seekstats.py) were listed but not read line-by-line —
if the report later appears, an audit of those scripts' arithmetic
(shares summing to 1, per-event×rate = busy-ms/s) is still owed and was
not done here since there was no report to check it against.
