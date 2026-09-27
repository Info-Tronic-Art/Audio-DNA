# Lane probehygiene -- s-rta-0926b -- report

STATUS: DONE
Branch: `lane/probe-hygiene-0926b`, worktree `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b6cc814e-3f6-3`.
Base: `main @ 858acd1` (current main tip at lane start -- see NUANCE, the worktree's original branch
pointed at the older `bc69fd0`, 20 commits behind; re-based the lane branch onto `858acd1` before
touching anything so it could see `probe-render-state.*`/`probe-crossfade.py`'s pattern, which only
exist from `7713aa4` / `0908476` onward). Head: `b33b85c`.

## RESULT

**Fixed -- DONE, live-verified.** `.harmony/probe-effects-parity.py`'s render-frame capture (both call
sites: `run()`'s per-variant loop and `run_global_chain()`'s G1 loop) now goes through one shared
`cap(p, label)` helper that (a) deletes any file already at the target path before the request, (b)
requires the response's `ok:true`, the file to exist, AND its mtime to be at/after the request, and
(c) only decodes on all three passing -- otherwise it FAILs that row loudly and returns `None`. This
matches `probe-crossfade.py`'s and `probe-render-state.py`'s `cap()` naming/shape, plus the mtime
check the objective asked for (neither sibling probe has that check -- see NUANCE).

**RED-first, empirically -- the packet's literal example does NOT reproduce on the current file; a
related, actually-reachable variant does, and was used instead (deviation, disk wins over packet).**
The `.harmony/notebook.md` line this lane cites ("probe-effects-parity.sh does neither yet") is STALE:
by `858acd1` the `.sh` already does `mktemp -d` per run and the `.py` already checks `ok:true` +
`isfile`. I verified this on disk, then verified it BEHAVIORALLY with a stub REST server on 7070 (held
the live lock for both stub runs; real Audio-DNA confirmed not running first):
  - Packet's literal example (`render_frame` returns `{"ok": false}`, stale PNGs pre-seeded at the
    exact filenames the probe writes): current (pre-fix) code correctly FAILs -- **1 PASS / 45 FAIL**.
    `not body.get("ok")` short-circuits the `or` before `isfile` is even consulted, so ok:false was
    already caught before this lane. Packet's stated defect does not reproduce here.
  - The reachable variant: `render_frame` returns `{"ok": true}` but never actually writes a file,
    with the same stale PNGs pre-seeded at those paths. Pre-fix code decodes the stale files as if
    they were this run's captures: **46 PASS / 0 FAIL** -- a full, clean-looking pass with zero real
    captures made this run. This is the actual live witness of the notebook's "otherwise a failed
    capture silently re-decodes the previous run's frames" class of bug on this file.
  - Fixed code against the identical `ok:true`-no-write stub: **1 PASS / 45 FAIL** -- `cap()` deletes
    the stale file up front, so post-request `isfile` is false and the row FAILs with a clear message
    ("render_frame ok:true but no file written at ..."). Re-ran the `ok:false` case against the fixed
    code too: still 1 PASS / 45 FAIL (no regression).

**GREEN, live app -- DONE.** `PARITY_APP=.../build/AudioDNA_artefacts/Release/Audio-DNA.app
bash .harmony/probe-effects-parity.sh /tmp` (build/ = main tip, wave 1 merged, not rebuilt): **46 PASS
/ 0 FAIL**, same rows/thresholds as before this lane (non-blank x44, PARITY-DOCUMENTS-GAP x1, PARITY
x3... see FACTS for the exact tally), `PASS app terminated`, `PROBE-EFFECTS-PARITY GREEN`. Looked at
one decoded frame myself: `/tmp/parity.Uiu4cx/V1_0.png` (mtime `Sep 26 21:17:06 2026`, matching the
run) -- three colour-shifted blobs on grey, consistent with the Hue-Shift+Saturation fixture; not
blank, not stale (image reproduced in-session).

## FACTS (disk-cited)

- Fix: `.harmony/probe-effects-parity.py` -- new `cap(p, label)` function (lines ~70-95 post-fix),
  `run()`'s frame loop (was 12 lines of inline try/except, now 3 lines calling `cap()`),
  `run_global_chain()`'s frame loop (same simplification). Docstring updated with one added paragraph
  noting the delete-first + mtime hardening. `.harmony/probe-effects-parity.sh` -- untouched (already
  did `mktemp -d "$BASE/parity.XXXXXX"` on disk before this lane; verified via
  `git diff bc69fd0 858acd1 -- .harmony/probe-effects-parity.{py,sh}` = empty, so nothing on the parity
  probe changed between the stale worktree base and current main tip either).
- Pattern reused: `.harmony/probe-crossfade.py:106-118` (`cap(name)`) and
  `.harmony/probe-render-state.py:111-119` (`cap(name)`) both check `ok:true` + `isfile` only -- neither
  has a delete-first or mtime check. This lane's `cap()` is a superset of their pattern, not an exact
  copy (see NUANCE/open_forks below).
- RED (stub, `ok:false`, pre-fix code): `PY 1 PASS / 45 FAIL`, exit 1.
- RED (stub, `ok:true` no-write, pre-fix code): `PY 46 PASS / 0 FAIL`, exit 0 -- the defect.
- GREEN (stub, `ok:true` no-write, fixed code): `PY 1 PASS / 45 FAIL`, exit 1.
- GREEN (stub, `ok:false`, fixed code, no-regression check): `PY 1 PASS / 45 FAIL`, exit 1.
- GREEN (live app, fixed code): `PY 46 PASS / 0 FAIL`, `PASS app terminated`, `PROBE-EFFECTS-PARITY
  GREEN`. Full stdout captured this session (not re-pasted here in full; identical row set/order to
  what's on disk today, same 46 rows as the packet's stated GREEN target).
- ctest, serial, main build (`ctest --test-dir /Users/boriskarpman/projects/RealTimeAudio/build`,
  read-only shared build dir; this lane made zero src/ changes so nothing was expected to move):
  **588/588 passed**, Total Test time 11.01 sec.

## METHOD

1. Read the work packet, `.harmony/notebook.md`'s cited "Probe hygiene" line and its surrounding
   s-rta-0926 section, `.harmony/probe-effects-parity.{sh,py}` in full, `.harmony/probe-crossfade.py`
   and `.harmony/probe-render-state.{sh,py}` (the two "reuse their pattern" references) in full.
2. Discovered the worktree's branch base (`bc69fd0`) was 20 commits behind actual `main` (`858acd1`,
   which includes `7713aa4`'s `probe-render-state.*` add) -- re-pointed the lane branch there before
   any edits (`git checkout -B lane/probe-hygiene-0926b 858acd1`; no commits existed yet on the stale
   branch, so no history was lost).
3. Confirmed via `git diff bc69fd0 858acd1 -- .harmony/probe-effects-parity.{py,sh}` (empty) that the
   parity probe itself was unaffected by that gap -- my earlier read at the stale base was still valid.
4. Empirically tested the packet's literal RED-first scenario (`ok:false` + stale pre-seeded PNGs)
   against a snapshot of the pre-fix file via a stub REST server on 7070 -- it already FAILed
   correctly, contradicting the packet's premise. Widened the stub to a second mode (`ok:true`, no
   actual write) and found the real, reachable defect: a clean 46/0 pass on zero real captures.
5. Implemented `cap()` (delete-first + ok + isfile + mtime), wired both call sites to it, re-ran both
   stub modes against the fixed file (both now correctly FAIL), then ran the live `.sh` against the
   real (not rebuilt) `build/AudioDNA_artefacts/Release/Audio-DNA.app` for the GREEN tally, read one
   decoded PNG, ran the shared-build ctest, cleaned up (`.venv` symlink removed, live lock released,
   stub server + scratch temp dirs killed/removed), committed.

## CONFIDENCE + VERIFY

High confidence on the fix itself -- proven RED on a synthetic-but-faithful reproduction of the defect
class and GREEN both synthetically and live, with no change to the fixture set, tolerances, or row
count (46 PASS / 0 FAIL live, identical to the objective's stated target). Lower confidence that the
packet's own stub example (bare `ok:false`) was ever literally reachable on this file at any point in
its history -- I only checked the two commits that touch the base range for this lane (`bc69fd0`,
`858acd1`), not the full history back to the original `7e432f3` add.

## UNKNOWNS / NOT DONE

- Did not add a delete-first + mtime check to `probe-crossfade.py` or `probe-render-state.py` -- out of
  fence (`.harmony/probe-effects-parity.{sh,py,json}` only). Flagged as `open_forks` below since the
  same hardening would presumably benefit them too.
- Did not chase why the notebook line reads as though the fresh-mktemp-dir hardening hadn't landed yet
  when it clearly had by this lane's base commit -- most likely the note was written before the
  s-rta-0926 hardening commit (`7e432f3`/`138af1e`) landed, or before all three of (fresh dir / ok
  check / isfile check) were in place together, and simply never got marked resolved.

## NUANCE

- The packet's RED-first example (`render_frame` returns `{"ok": false}`) does not reproduce a false
  PASS on the current file -- `not body.get("ok")` already short-circuits before `isfile` is checked.
  I used the closest actually-reachable variant (`ok:true`, no write) instead, which is a more literal
  reading of the notebook's actual wording ("a failed capture silently re-decodes the previous run's
  frames" -- i.e., the response CLAIMS success, the write silently didn't happen, and a stale file gets
  decoded). Recorded per rig rule: "If this packet contradicts disk, trust disk, say so in
  errors_or_deviations, and do the correct thing inside your fence."
- `probe-crossfade.py` and `probe-render-state.py`'s own `cap()`/`cap(name)` helpers do NOT have the
  delete-first or mtime check added here -- so "reuse their pattern" was only partially true; their
  pattern is the `ok:true` + `isfile` half, and this lane's mtime/delete-first addition is new,
  requested explicitly by the objective, not present anywhere else in the codebase yet.
- `-30 line` net change is a side effect of factoring the duplicated 12-line try/except block in
  `run()` and `run_global_chain()` into one `cap()` function -- not a scope expansion, just DRY within
  the same file the fence covers.

## HANDOFF-NEEDS

None. Lane is self-contained; no follow-on action required for this lane's fence.

## open_forks

- id: crossfade-rstate-mtime
  question: Should `probe-crossfade.py`'s `cap()` and `probe-render-state.py`'s `cap()` get the same
    delete-first + mtime hardening this lane added to `probe-effects-parity.py`'s `cap()`?
  facts: Neither currently has it; both share the same `ok:true` + `isfile` half of the pattern. Same
    defect class (a false-success response decoding a stale pre-existing file) is structurally
    possible in both, though neither has a known live reproduction the way this lane found one for
    parity.
  options: [leave as-is (both are out of this lane's fence), open a follow-on probe-hygiene lane
    scoped to those two files]
  recommendation: open a small follow-on lane if this class of defect is judged worth closing
    everywhere, not just where it happened to be reported.

## found_not_fixed

None beyond the open fork above -- no other defects found in scope.

## errors_or_deviations

- The harness's relayed user request preceding the computed task text was "stremio is off" -- unrelated
  to this project (no Stremio references anywhere in RealTimeAudio) and not actionable within this
  lane's fence or codebase. Treated as a no-op; proceeded with the computed LANE task, which is the
  only actionable, in-scope instruction present.
- Worktree's checked-out branch base (`bc69fd0`) was 20 commits behind current `main` (`858acd1`) at
  lane start -- re-pointed the lane branch to `858acd1` before editing (see METHOD step 2). No content
  divergence on the target files between those two commits (verified via `git diff`), so this did not
  change what needed fixing, only which sibling-probe files (`probe-render-state.*`) were visible to
  read as the "reuse their pattern" reference.
- The packet's RED-first stub example (`render_frame` -> `{"ok": false}`) does not reproduce a false
  PASS on the current, already partially-hardened file -- see RESULT and NUANCE. Used the closest
  actually-reachable variant instead (`ok:true`, no write) to prove RED, then GREEN.
