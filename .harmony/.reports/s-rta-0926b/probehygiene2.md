# probehygiene2 -- s-rta-0926b

Lane: probehygiene2 | Worktree: `.claude/worktrees/wf_41d6317f-a47-2` | Branch: `lane/probe-hygiene2-0926b`
Base: `main` (fdf46b9, "docs(s-rta-0926b): wave 2 gate GREEN (ctest 599/599, all live probes green)")

## Scope (FENCE)

`.harmony/probe-*.{sh,py,json}`, `docs/claude/testing-eyes.md`, this report. Nothing in `src/`.

## What changed

### 1. Live-lock gate (all 14 `.harmony/probe-*.sh`)

Every probe now refuses (`exit 64`, clear message) unless `/tmp/audiodna-live.lock/owner` exists,
right after `set -u` and before any REST/launch activity:

```sh
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- ..."; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner ..."; exit 64; }
fi
```

Files: probe-crossfade.sh, probe-deck-path.sh, probe-downbeat-level.sh, probe-effects-parity.sh,
probe-finalize-loop.sh, probe-lane3.sh, probe-manual-bpm.sh, probe-mastersignal.sh,
probe-onset-render.sh, probe-render-state.sh, probe-resync.sh, probe-routines.sh, probe-step3.sh,
probe-tempo-silence.sh.

### 2. Exact process-name detection, not command-line substring

**Finding during implementation (documented in the header of every file, not just claimed):**
the packet's suggested `pgrep -x Audio-DNA` is ALSO exploitable -- macOS `pgrep`/`pkill` without
`-f` still match on the argv[0]-derived `comm` field, which a process can spoof via
`exec -a ".../MacOS/Audio-DNA" <anything>`. Verified live (see RED-FIRST 2 below): a `sleep`
process launched with `exec -a ".../Audio-DNA.app/Contents/MacOS/Audio-DNA" sleep 30` DOES match
both the old `pgrep -f 'MacOS/Audio-DN[A]'` bracket pattern AND a plain `pgrep -x Audio-DNA`.

The fix filters on `ucomm` -- the KERNEL's real exec-time process name (set from the actual binary
that was exec'd, not from a spoofable argv[0]) -- via three small helper functions defined in
every probe right after the lock gate:

```sh
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
adna_kill() { local p; p="$(adna_pids)"; [ -n "$p" ] && kill $p 2>/dev/null; }
```

Every `pgrep -f 'MacOS/Audio-DN[A]'` / `pgrep -f 'MacOS/Audio-DNA'` (already-running-refuse check,
teardown wait-loop, PID capture) and every `pkill -f '...'` is now `adna_running` / `adna_pids` /
`adna_kill`. Verified both directions live (see RED-FIRST below): (1) a fake linker command line
containing the app's `-o` path does NOT match (real ucomm is the interpreter, e.g. perl/clang);
(2) the `exec -a` argv0-spoofing dummy does NOT match either (real ucomm is the real exec'd
command, e.g. sleep). Stale header prose referencing the old "bracket trick"/"SELF-MATCH" framing
was rewritten in every file that had it (onset-render, step3, mastersignal, downbeat-level, resync,
finalize-loop, routines) to describe the new adna_pids/ucomm approach.

### 3. `cap()` delete-first + mtime guard ported into probe-crossfade.py and probe-render-state.py

Both files' `cap()` now (matching probe-effects-parity.py's cap(), commit 4f9e78d): delete any
pre-existing file at the target path before the request, then require the response's `ok:true`,
the file to exist, AND its mtime to be at/after the request time -- never decode a PNG the call
did not write. Row logic and thresholds elsewhere in both files are UNCHANGED (only `cap()`'s
body changed; call sites/signatures preserved: `cap(name)` in crossfade, `cap(name, size=None)`
in render-state).

### 4. Docs

`docs/claude/testing-eyes.md`: appended a short "Probe rig rules" note (lock gate, adna_pids/ucomm
approach, cap() hygiene) distinguishing the `.harmony/probe-*.sh` live rig from the `--test-mode`
Eyes harness documented above it.

## RED-FIRST (run against the live rig, not synthetic)

**1. No lock held.** Pre-change `probe-crossfade.sh` (git `main`, no lock-check code at all) with
no lock held and a bogus `XFADE_APP` still reaches its OWN "no app" check --
`REFUSE: no app at /tmp/does-not-exist.app (set XFADE_APP)`, exit 64 -- i.e. it never even
considers the lock. Post-change, same script, same conditions:
`REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && ...`, exit 64, BEFORE the app check.

**2. `exec -a` argv0-spoofing dummy (both directions of the fix, live, not just claimed).**
```
$ ( exec -a "/Users/.../Audio-DNA.app/Contents/MacOS/Audio-DNA" sleep 30 ) &
$ ps -p $PID -o ucomm=,args=
sleep   /Users/.../Audio-DNA.app/Contents/MacOS/Audio-DNA 30
$ pgrep -f 'MacOS/Audio-DN[A]'      # OLD pattern
<PID>                                 # MATCHES -- the bug
$ pgrep -x Audio-DNA                # packet's suggested fix
<PID>                                 # ALSO MATCHES -- not enough (macOS pgrep -x is comm-based)
$ ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'   # adna_pids
(empty)                               # correctly does NOT match
```
And direction 1 (fake linker/compiler invocation whose `-o` argument is the app path):
```
$ ( exec -a "clang++" perl -e 'sleep 25' -- -o "/Users/.../Audio-DNA.app/Contents/MacOS/Audio-DNA" ) &
$ pgrep -f 'MacOS/Audio-DN[A]'       # OLD pattern
<PID>                                  # MATCHES -- the bug FACT (a) describes
$ ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'
(empty)                                # correctly does NOT match (real ucomm is perl)
```
And a positive control -- a REAL compiled binary literally named `Audio-DNA` (not a script, not
argv0-spoofed) DOES show `ucomm=Audio-DNA` and IS matched by `adna_pids`, confirming the filter
still detects the real app.

**3. Stale-PNG stub (no-write stub on 7070, held lock, exits if bind fails).** A stub HTTP server
answered `POST /api/render_frame` with `{"ok": true}` and never wrote a file, with a pre-existing
stale PNG already at the target path. The PRE-change `cap()` logic (no delete-first, no mtime
check) happily decoded the stale file: `PASS xf_test: decoded a frame ... (BUG: this is the STALE
file, stub never wrote anything)`. The POST-change `cap()` (delete-first + mtime guard, same file
recreated) correctly rejected it: `FAIL render_frame xf_test: ok:true but no file written at ...`
(returns `None`, so the calling row correctly reports FAIL instead of a false PASS on stale data).

## GREEN battery (worktree's edited probes against the pre-built, unmodified
`/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app`, lock
held, `AUDIODNA_LOCK_OWNER=probehygiene2`)

Verbatim final summary line of every run, in order:

| Probe | Result |
|---|---|
| probe-crossfade.sh | `PY 35 PASS / 0 FAIL` ... `PROBE-CROSSFADE GREEN` |
| probe-render-state.sh | `PY 31 PASS / 0 FAIL` ... `PROBE-RENDER-STATE GREEN` (+ "no foreign render_frame traffic") |
| probe-effects-parity.sh | `PY 46 PASS / 0 FAIL` ... `PROBE-EFFECTS-PARITY GREEN` |
| probe-manual-bpm.sh | `18 PASS / 0 FAIL` |
| probe-resync.sh | `16 PASS / 0 FAIL` |
| probe-downbeat-level.sh | `14 PASS / 0 FAIL` |
| probe-routines.sh (`ROUTINES_BUILD_DIR=build ROUTINES_RECORD_PAUSE=1.8`) | `74 PASS / 0 FAIL` |
| probe-mastersignal.sh | `22 PASS / 0 FAIL` |
| probe-step3.sh | `94 PASS / 0 FAIL` (T2 row was fine first attempt, no re-run needed) |
| probe-deck-path.sh | `13 PASS / 0 FAIL` |
| probe-lane3.sh | `12 PASS / 0 FAIL` |
| probe-onset-render.sh | `13 PASS / 0 FAIL` |
| probe-finalize-loop.sh (N=5, smoke -- see note) | `5 cycles, 0 truncations. 8 PASS / 0 FAIL` |
| probe-tempo-silence.sh | `5 PASS / 0 FAIL` |

All 9 row-count-specified probes match the packet's expected counts EXACTLY: render-state 31/0,
crossfade 35/0, effects-parity 46/0, manual-bpm 18/0, resync 16/0, downbeat 14/0, routines 74/0,
mastersignal 22/0, step3 94/0. Zero FAILs anywhere. Every run's teardown reported
`app terminated`/`no process remains` and (where checked) `0 Audio-DNA Output windows`.

**Deviation, disclosed:** `probe-finalize-loop.sh` defaults to `N=40` mic-input record/stop cycles
(not in the packet's row-count list). Ran it with `N=5` as a smoke check of the lock-gate +
adna_pids fix on that file specifically (its own oracles: `finalizeErrors==0`, disk frame-count
agreement, no truncation stderr line) rather than the full 40-cycle statistical bound, to keep
total lane runtime reasonable. All 3 of its independent oracles passed on all 5 cycles.

**`ctest`:** not run. This lane's fence is `.harmony/probe-*.{sh,py,json}` + one doc; no `src/`
change was made, so the existing `ctest 599/599` from the prior wave-2 gate (commit fdf46b9) is
unaffected by this lane's diff. Re-running the full C++ suite would not exercise anything this
lane touched.

## Files changed

```
 .harmony/probe-crossfade.py      | 20 +++++++++++++++--
 .harmony/probe-crossfade.sh      | 33 +++++++++++++++++++++----
 .harmony/probe-deck-path.sh      | 27 ++++++++++++++++++--
 .harmony/probe-downbeat-level.sh | 40 +++++++++++++++++++++++-------
 .harmony/probe-effects-parity.sh | 33 +++++++++++++++++++++++----
 .harmony/probe-finalize-loop.sh  | 34 ++++++++++++++++++++++----
 .harmony/probe-lane3.sh          | 33 +++++++++++++++++++++++----
 .harmony/probe-manual-bpm.sh     | 39 +++++++++++++++++++++++++-------
 .harmony/probe-mastersignal.sh   | 43 ++++++++++++++++++++++++++--------
 .harmony/probe-onset-render.sh   | 41 +++++++++++++++++++++++++--------
 .harmony/probe-render-state.py   | 18 ++++++++++++++-
 .harmony/probe-render-state.sh   | 33 +++++++++++++++++++++++----
 .harmony/probe-resync.sh         | 40 +++++++++++++++++++++++++-------
 .harmony/probe-routines.sh       | 42 ++++++++++++++++++++++++++--------
 .harmony/probe-step3.sh          | 49 ++++++++++++++++++++++++++++------
 .harmony/probe-tempo-silence.sh  | 27 ++++++++++++++++++++--
 docs/claude/testing-eyes.md      | 13 +++++++++++
 17 files changed, 465 insertions(+), 100 deletions(-)
```

No `.json` fixture changes needed (grepped -- none of them reference pgrep/pkill/lock/cap()).

## Rig discipline followed

- Checked out `lane/probe-hygiene2-0926b` from `main` in the worktree first; never touched the
  main checkout.
- `.venv` and `build` symlinks created inside the worktree for the run (needed since several
  probes hardcode `$ROOT/build/...` or `$ROOT/$BUILD_DIR/...` with no absolute-path override), both
  **removed** before this commit -- confirmed absent via `ls` and `git status` (clean, only the
  intended 17 files + this report).
- `build-lane/` (pre-existing, untracked, real cmake build from an earlier lane's occupancy of this
  worktree) untouched throughout -- kept per rig rule, never built into, never deleted.
- Live lock acquired via `mkdir /tmp/audiodna-live.lock` + owner file `probehygiene2 <pid> <epoch>`,
  held for the entire RED-FIRST + GREEN sequence, `AUDIODNA_LOCK_OWNER=probehygiene2` set for every
  probe invocation. Lock released (`rm -rf /tmp/audiodna-live.lock`) only after confirming ownership
  and that no Audio-DNA process remained.
- Every app launch used the documented `open -g`/`--stdout`/`--stderr` pattern already in each
  probe; no `lldb`/`gdb`/screen capture beyond what each probe already did; no unexpected system
  dialogs seen.
- No other lane's live-app usage or build was disrupted: the stub-server RED-FIRST test bound only
  to 7070 while the lock was held, and was torn down before any real probe ran; observed a genuinely
  concurrent lane (`wf_156f9cfa-a36-2`) compiling in the background throughout -- confirmed via
  `ps` that its clang/clang++ invocations never satisfy `ucomm=="Audio-DNA"`, so the new detection
  is provably inert against it (this is exactly the FACT (a) scenario, observed live, not
  hypothesized).

## Verdict

STATUS: DONE. All three DO items applied to all 14 shell probes + the two named Python `cap()`s.
All three RED-FIRST cases shown failing pre-change / passing post-change, live, with real processes
(not just prose). Full GREEN battery run on all 14 probes against the unmodified build; all 9
row-count-specified probes match exactly; zero FAILs anywhere.

**Found-not-fixed (out of fence, flagged for Harmony):** none. The packet's own suggested fix
(`pgrep -x`) was insufficient against argv0-spoofing; this was caught and corrected within scope
(same files, same DO item) rather than shipped as designed and reported as a gap.
