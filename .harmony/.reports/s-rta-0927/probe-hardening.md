## BUILDER REPORT -- lane probe-hardening (s-rta-0927)

STATUS: DONE
RESULT: All four items (H1-H4) applied, one commit each, on `lane/probe-hardening-0927`. H1: the same `S.headers["Connection"] = "close"` fresh-connection fix from `c1-state-fix.md` (probe-deck-clock.py) applied to the other four `requests.Session`-based probes (probe-canvas.py, probe-fitmode.py, probe-outputs.py, probe-render-state.py); grepped for other pooled HTTP clients -- none found (details below). H2: probe-canvas.py's `wait_no_compiler` had the same invalid-regex `pgrep -x clang++` bug probe-outputs.py already fixed; escaped it the same way, proved live with a `sleep`-as-`clang++` scratch process. H3: probe-manual-bpm.sh chmod 644 -> 755. H4: two stale comments reworded to match current source (verified against the actual code, not assumed). GATE: all five touched probes run once against the MAIN app with the live lock -- probe-canvas GREEN (15/0), probe-fitmode GREEN (10/0), probe-outputs GREEN (10/0), probe-render-state GREEN (31/0), probe-manual-bpm.sh (invoked via `./`, now executable) 22/0 FAIL. Nothing merged or pushed.
FACTS: worktree `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w6`, branch `lane/probe-hardening-0927` (4 commits ahead of `main` at 8573006): `1041d9c` (H1), `82ac73a` (H2), `7b35b85` (H3), `1cc8783` (H4). Diffs: `git -C <WT> show 1041d9c 82ac73a 7b35b85 1cc8783`. Gate transcripts (verbatim, captured this run): `.harmony/probe-canvas.py`/`.sh` run in `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/hardening/{canvas-out,fitmode-out,outputs-out,render-state-out,manual-bpm-out}` (fresh mktemp dirs each, per-probe convention) and `manual-bpm.log`.
METHOD: (1) Read `c1-state-fix.md` for the exact fix line/comment to replicate verbatim. (2) grep'd every `.harmony/*.py`/`*.sh` for `requests.Session`, `http.client`, `urllib3.PoolManager`, and `curl` usage to find every pooled-client candidate and confirm scope. (3) Applied the identical one-liner + comment to the 4 named files (H1). (4) Found the exact prior fix commit for the clang++ regex bug (`git log -S"clang\+\+" -- probe-outputs.py`) and mirrored it into probe-canvas.py (H2), then proved the before/after behavior live in scratch with a `sleep` binary symlinked as `clang++` (no compiler needed) -- old pattern: `pgrep -x "clang++"` errors (exit 2, empty stdout); new: `pgrep -x 'clang\+\+'` matches the PID. (5) `chmod 755` + `git update-index --chmod=+x` on probe-manual-bpm.sh (H3). (6) Read both flagged comments, traced current callers/behavior in source (grep for `#include EmbeddedShaders.h` -> only Renderer.cpp/CompositorEngine.cpp; read `OutputWindow::Presenter::renderOpenGL` -> only calls `presentSharedFrame`, never `EffectChain::render()`) before rewording, then `git diff` confirmed only comment lines changed (H4). (7) Acquired the machine-wide live-app lock (polled `mkdir /tmp/audiodna-live.lock` every 20 s in the background until free, then wrote the owner file), symlinked `.venv`, ran each of the 5 touched probes once against the MAIN app bundle with `AUDIODNA_LOCK_OWNER=probe-hardening` and the app-path env var each script's header names, then released the lock and removed the `.venv` symlink before this final commit-free step.
CONFIDENCE+VERIFY: High. All five gate runs are fresh, this-session transcripts against the (unmodified) main app -- re-run: `AUDIODNA_LOCK_OWNER=<lane> CANVAS_APP=.../build/.../Audio-DNA.app bash <WT>/.harmony/probe-canvas.sh <out>` (swap `CANVAS_`->`FIT_`/`OUTP_`/`RSTATE_` for the other three; `MANUALBPM_APP=... MANUALBPM_PY=<main>/.venv/bin/python <WT>/.harmony/probe-manual-bpm.sh <out>` for the fifth) -- expect the same "GREEN" / "0 FAIL" lines. H2's regex proof is reproducible: `ln -s /bin/sleep clang++; ./clang++ 60 & pgrep -x "clang++"` (errors) vs `pgrep -x 'clang\+\+'` (matches).
UNKNOWNS/NOT-DONE: probe-crossfade.py and probe-effects-parity.py were read (not touched -- they use module-level `requests.get`/`requests.post`, not a Session, so no persistent-connection race) but I did not instrument them to independently reconfirm requests' module-level call creates a fresh connection every time; this rests on knowing `requests.get`/`.post` build a throwaway `Session` per call (documented requests behavior), not on a live probe of THIS codebase. probe-routines.* and probe-deck-clock.py were explicitly out of scope (owned by other lanes / already fixed) and untouched.
NUANCE: H1's grep swept `.harmony/*.py` and `.harmony/*.sh` for `requests.Session`, `Session()`, `http.client.HTTPConnection`, `urllib3.PoolManager`/`PoolManager(` -- five files use `requests.Session()` (deck-clock, already fixed; the four fixed here); no `http.client`/`urllib3.PoolManager` usage anywhere. Every `.sh` probe uses standalone `curl` invocations (one process per call, no connection reuse across calls) -- not exposed to this race. H4's EffectChain.h edit went slightly beyond the one quoted phrase ("now doubled by the second reader"): the same paragraph also said "unsynchronized against EITHER GL-thread reader" and "BOTH GL threads" a few lines later, both asserting the same now-false fact (two live GL-thread readers) the quoted phrase asserts -- leaving those uncorrected would have re-introduced the exact staleness the item asks to fix, one sentence later, in the same comment block. Reworded all three consistently; left the unrelated architecture/design-doc discussion in the rest of the note untouched.
HANDOFF-NEEDS: none.

### SUMMARY
H1: applied the c1-state-fix `Connection: close` fresh-connection fix (+ its explanatory comment) to probe-canvas.py, probe-fitmode.py, probe-outputs.py, probe-render-state.py -- the four other probes with a pooled `requests.Session()` exposed to cpp-httplib's 5 s keep-alive-close race. Grepped for other pooled HTTP clients; found none beyond the five Session-using files (one already fixed by c1-state-fix). H2: fixed probe-canvas.py's `wait_no_compiler` -- `pgrep -x clang++` is an invalid regex on macOS (bare `+` is a quantifier), silently never seeing a running clang++; escaped to `r"clang\+\+"`, matching probe-outputs.py's existing fix, proved live. H3: probe-manual-bpm.sh was mode 644; chmod 755 to match every other probe .sh. H4: reworded two stale comments (EmbeddedShaders.h's dead `OutputRenderer` reference; EffectChain.h's DEFERRED BOUNDARY note's "doubled by the second reader" / "BOTH GL threads" claims, both false since OutputWindow's second GL-thread reader was retired in outputs-c1) after verifying current truth against the source. GATE: all five touched probes run once against the main app with the live lock held -- all GREEN, 0 FAIL total across 88 rows (15+10+10+31+22).

### FILES CHANGED
- `.harmony/probe-canvas.py` -- H1 (fresh-connection fix + comment) and H2 (clang++ regex escape). Commits 1041d9c, 82ac73a.
- `.harmony/probe-fitmode.py` -- H1 (fresh-connection fix + comment). Commit 1041d9c.
- `.harmony/probe-outputs.py` -- H1 (fresh-connection fix + comment). Commit 1041d9c.
- `.harmony/probe-render-state.py` -- H1 (fresh-connection fix + comment). Commit 1041d9c.
- `.harmony/probe-manual-bpm.sh` -- H3 (mode 644 -> 755, no content change). Commit 7b35b85.
- `src/render/EmbeddedShaders.h` -- H4 (comment: `OutputRenderer` -> `CompositorEngine`, deletion noted). Commit 1cc8783.
- `src/effects/EffectChain.h` -- H4 (comment: three now-false "second reader"/"BOTH GL threads" claims reworded to the current single-reader-vs-writers state). Commit 1cc8783.

### TESTS (the GATE -- each probe run ONCE against the main app, lock held)
- **probe-canvas** (`CANVAS_APP=<main app>`): `PY 15 PASS / 0 FAIL`, `PASS no foreign render_frame traffic during the run`, `PASS app terminated` -- **`PROBE-CANVAS GREEN`**.
- **probe-fitmode** (`FIT_APP=<main app>`): `PY 10 PASS / 0 FAIL`, `PASS no foreign render_frame traffic during the run`, `PASS app terminated` -- **`PROBE-FITMODE GREEN`**.
- **probe-outputs** (`OUTP_APP=<main app>`): `PY 10 PASS / 0 FAIL`, `PASS no foreign render_frame / output_probe traffic during the run`, `PASS app terminated`, `PASS after quit: 0 Audio-DNA windows in the FULL Quartz window list` -- **`PROBE-OUTPUTS GREEN`**.
- **probe-render-state** (`RSTATE_APP=<main app>`): `PY 31 PASS / 0 FAIL`, `PASS no foreign render_frame traffic during the run`, `PASS app terminated` -- **`PROBE-RENDER-STATE GREEN`**.
- **probe-manual-bpm.sh** (invoked directly via `./`, now executable; `MANUALBPM_APP=<main app>` `MANUALBPM_PY=<main>/.venv/bin/python`): 22 rows, all PASS (AUTO sanity, M poller/bpm-stays/non-vacuous/free-run/rate/bar-length, S1/S2/S3 set_bpm and OSC rows, R resync rows, teardown/screen-safety rows) -- **`22 PASS / 0 FAIL`**.
- Total: 88 PASS / 0 FAIL across all five gates, verbatim summary lines above.
- No build was run: H1-H3 are Python/bash/file-mode changes exercised end-to-end by the gate itself; H4 is comment-only in two `.h` files, confirmed via `git diff` that only comment lines changed (no build required per the work packet, but I re-read both files after editing to confirm no stray syntax).

### ISSUES
- None found beyond what's in UNKNOWNS/NUANCE above.

### SKILL_PROPOSALS
- None -- this reused the existing http-flake-triage pattern from c1-state-fix.md and a straightforward comment/mode fix; no new reusable procedure.

### RISKS
- Low: H4's EffectChain.h reword touches 3 sentences instead of the 1 literally quoted in the packet, because all 3 assert the same retired-second-reader fact and leaving 2 of them stale would have visibly contradicted the fixed one line later in the same comment block. If a narrower edit was wanted, the other two sentences ("unsynchronized against either GL-thread reader", "BOTH GL threads") are called out explicitly in NUANCE above for review.
- Low: probe-crossfade.py / probe-effects-parity.py were read and reasoned about (module-level `requests.get`/`.post`, not `Session`) but not independently instrumented to reconfirm requests' fresh-connection-per-call behavior in this specific runtime (see UNKNOWNS).

### METRICS
- Self-check: 5/5 gate probes GREEN, 0 FAIL (88 rows total). `git status` clean, live lock released, no Audio-DNA process running, `.venv` symlink removed.
- Tool calls: ~55. Files read: ~20.

### KNOWLEDGE CONTEXT
- Tools used: grep, git log -S, git diff, live probe runs. No KNOWLEDGE_TOOLS block in the work packet; no graphify graph query needed (comment/probe fixes, no impact-scope question).
- Impact authority: none needed -- H1-H3 are probe-tooling only (no app code); H4 is comment-only, verified by reading the actual current call sites, not by tool inference.
- Risk level: NORMAL.

### PACKET QUALITY
- Clarity: CLEAR.
- Missing context: none -- the packet named the exact prior fix commits/comments to mirror, which made both H1 and H2 mechanical once located.
- Unused context: none.
- Self-assembly: no DEPARTMENT field -- legacy protocol followed.
- Self-brief files: `c1-state-fix.md` (required read) was the load-bearing reference for H1's exact fix text; the project CLAUDE.md was read per the framework's standing requirement but not specifically needed for this probe-tooling task.

### STATUS
DONE. All four items applied (one commit each), all five touched probes GREEN with 0 FAIL against the main app under the live lock, tree clean, lock released, no app running.

### NEXT ACTION
Harmony: review the four commits (especially H4's 3-sentence reword vs. the 1 literally-quoted phrase, flagged in RISKS/NUANCE), and merge `lane/probe-hardening-0927` if accepted. Not merged or pushed by this lane.

INBOX-RECHECK: 0 addenda folded (none received)
