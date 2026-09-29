export const meta = {
  name: 'rta-0928b-fix-lane',
  description: 'Audio-DNA s-rta-0928b: Harmony-ruled fix round on an existing lane branch (opus builder continues from a named commit), pinned re-reviews, <=1 further fix round',
  phases: [{ title: 'Fix' }, { title: 'Re-review' }, { title: 'Fix 2' }, { title: 'Re-review 2' }],
}
// args: { lane, wt, branch, plan, notes, reviews: [{key, focus}], critic: string|null, effort }
const A = args
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const MAIN_APP = MAIN + '/build/AudioDNA_artefacts/Release/Audio-DNA.app'
const RPT = '.harmony/.reports/s-rta-0928b'
const SP = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad'
const W = MAIN + '/.claude/worktrees/' + A.wt

function RULES(lane) {
  return `
RIG RULES (binding; breaking any one = lane FAIL):
- NOT an isolated worktree. Work ONLY inside WT=${W} with absolute paths (git -C ${W}; cmake -S ${W} -B ${W}/build-lane; cmake --build ${W}/build-lane -j3). First configure: -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON plus -DFETCHCONTENT_SOURCE_DIR_<DEP>=${MAIN}/build/_deps/<dep>-src for each FetchContent dep (read-only use; names from CMakeLists.txt FetchContent_Declare). NEVER edit the main checkout ${MAIN} or its build/ (read-only use of ${MAIN_APP} = the PRE-CHANGE app for RED runs is required). NEVER cd in a command; multi-step sequences go in script files under ${SP}/${lane}/ run with bash <file>. zsh does not split a command held in a variable. Stamp times from date, never estimate.
- Probes need python: ln -s ${MAIN}/.venv ${W}/.venv for runs; REMOVE it before every commit.
- LIVE APP: at most ONE Audio-DNA on the machine (other lanes share it). Use the helper: LANE=${lane} . ${SP}/lib/lock.sh (acquire_lock / release_lock / start_app APP OUTDIR [test] / quit_app / outwins / wait_quiet / adna). A failed mkdir means WAIT (never open another copy: same bundle id reaches the running app); never remove another lane's lock; after releasing wait >= 45 s before re-acquiring (the helper enforces it). Hold the lock per run or small batch (<= ~15 min), release between batches. Probes REFUSE without AUDIODNA_LOCK_OWNER matching (the helper exports it).
- Launch ONLY: open -g [--env V=v] [--stdout f --stderr f] <App> [--args --test-mode]. SCREEN-SAFETY LAW: NEVER open the Output window by ANY path (Output menu, Cmd+F, TopBar button, REST, deck/composition/settings file with an output) — no automated gate ever opens an output window. NEVER run tests/visual/test_output_window_level.py; NEVER run pytest on tests/visual/ as a directory (Tier-1 = exactly test_sources.py test_effects.py test_audio_reactivity.py test_time_sweep.py test_performance.py). Quit via quit_app (osascript quit; kill only after 30 s). NEVER lldb/debugserver/gdb/dtrace/Instruments/sample on ANY binary. NEVER full-screen screencapture (window-only by Quartz window id via pyobjc with the main .venv python). NO synthetic input (UI states via REST / composition files / a TEMPORARY env-var hook: revert it, rebuild, strings check = 0 before the lane ends). Unexpected system dialog: STOP and report.
- RED FIRST on the pre-change app / tree for every new probe row / test; GREEN on yours; raw summary lines VERBATIM; tests drive real code; full ctest serial (ctest --test-dir ${W}/build-lane -j1) at the end. Existing probes: re-run, NEVER re-threshold. Render gates DECODE PIXELS and you LOOK at a sample frame. Perf numbers only when no compiler runs (wait_quiet) with the load average printed. Probe env: STEP3_BUILD_DIR / RESYNC_BUILD_DIR / DOWNBEAT_BUILD_DIR / MANUALBPM_BUILD_DIR / ROUTINES_BUILD_DIR point at the build dir under test (+ ROUTINES_RECORD_PAUSE=1.8 for probe-routines); T2 rows in probe-step3 fail if another app plays audio — re-run quiet. A flake verdict needs >= 5 runs per arm.
- Probe HTTP clients use a fresh connection per request (Connection: close): cpp-httplib drops a request that races its 5 s keep-alive close.
- CLAUDE.md is capped at 25,000 bytes (~24.5 KB now, ~500 B left): any addition is paid for by moving text into docs/claude/*.md. After a tests/CMakeLists.txt change, reconfigure.
- Report ${RPT}/${lane}.md in the worktree, COMMITTED (git add -f), AND returned in full in report_markdown. One commit per plan item (message: <type>(s-rta-0928b ${lane}): ...). Notes for .harmony/notebook.md go in the report (Harmony appends them), not in notebook.md itself. Do NOT merge or push. Before returning: git status clean (except build-lane/), lock released, no app you launched running, outwins 0 Output-named windows. KEEP build-lane.`
}

const BS = { type: 'object', properties: { lane: { type: 'string' }, status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, worktree_path: { type: 'string' }, branch: { type: 'string' }, base_commit: { type: 'string' }, head_commit: { type: 'string' }, drift: { type: 'string' }, items: { type: 'array', items: { type: 'object', properties: { id: { type: 'string' }, verdict: { type: 'string' }, evidence: { type: 'string' }, fixed: { type: 'boolean' }, red_on_base: { type: 'string' }, green_on_fix: { type: 'string' }, fix_commits: { type: 'string' } }, required: ['id', 'evidence', 'fixed'] } }, shots: { type: 'array', items: { type: 'string' } }, perf: { type: 'string' }, ctest: { type: 'string' }, found_not_fixed: { type: 'array', items: { type: 'string' } }, boris_checks: { type: 'array', items: { type: 'string' } }, report_markdown: { type: 'string' }, lock_released: { type: 'boolean' }, app_left_running: { type: 'boolean' }, output_window_opened: { type: 'boolean' }, errors_or_deviations: { type: 'string' } }, required: ['lane', 'status', 'worktree_path', 'branch', 'base_commit', 'head_commit', 'items', 'report_markdown', 'ctest', 'lock_released', 'app_left_running', 'output_window_opened'] }
const RS = { type: 'object', properties: { verdict: { type: 'string', enum: ['PASS', 'PASS_WITH_NITS', 'FAIL'] }, reviewed_commit: { type: 'string' }, findings: { type: 'array', items: { type: 'object', properties: { severity: { type: 'string', enum: ['MUST', 'SHOULD', 'NIT'] }, issue: { type: 'string' }, fix: { type: 'string' } }, required: ['severity', 'issue'] } } }, required: ['verdict', 'reviewed_commit', 'findings'] }
const CS = { type: 'object', properties: { verdict: { type: 'string', enum: ['PASS', 'FAIL'] }, findings: { type: 'array', items: { type: 'object', properties: { severity: { type: 'string', enum: ['MUST', 'SHOULD', 'NIT'] }, issue: { type: 'string' }, fix: { type: 'string' } }, required: ['severity', 'issue'] } } }, required: ['verdict', 'findings'] }

// args: { lane, wt, branch, from_commit, base_commit, plan, rulings, reviews: [{key, focus}], model, effort, reviewEffort }
const FIX = (from, findings, round) => `FIX ROUND ${round} (lane-name: ${A.lane}-fix${round}). Work ONLY in ${W}; CONTINUE on branch ${A.branch} from commit ${from} (no reset, no STEP 0, no rebase). Read the lane report ${RPT}/${A.lane}.md in the worktree and the plan ${MAIN}/${RPT}/${A.plan} INCLUDING every HARMONY ADOPTION section — the newest addendum OVERRIDES older text.
Implement the Harmony rulings below; VERIFY each against the code first (a ruling can rest on a wrong premise — say so with evidence and STOP that item rather than improvise). RED first for every new or changed gate (on the app built from ${from} — keep a copy of it before your first change — and, where the ruling names it, 5 runs per arm), GREEN on yours, raw lines VERBATIM. Commit per ruling group; append "Fix round ${round}" to ${RPT}/${A.lane}.md (git add -f) with a table ruling -> commit -> RED line -> GREEN line; return the FULL updated lane state.
RULINGS / FINDINGS:
${findings}
${RULES(A.lane)}`
const reviewR = (x, n) => parallel((A.reviews || []).map(r => () => agent(`INDEPENDENT SOURCE REVIEW (read-only), lane ${A.lane}, lens ${r.key}, round ${n}. PINNED: worktree ${x.worktree_path}, branch ${x.branch}, base ${A.base_commit}, head ${x.head_commit}; the fix round started from ${A.from_commit}. Read the change ONLY via git -C ${x.worktree_path} diff / git show (never the working tree, never main): the fix-round delta ${A.from_commit}..${x.head_commit} in full, and the whole lane ${A.base_commit}..${x.head_commit} where needed. Spec: ${MAIN}/${RPT}/${A.plan} incl. every HARMONY ADOPTION section (the newest addendum overrides). FOCUS: ${r.focus}
Always also check: each ruling implemented as written (or a verified, evidenced deviation); gates RED on the named arm and able to fail; nothing stray (no .venv symlink, no instrumentation, no env-var hook left); real-time rules (render thread never waits, no new mutex); docs additive (git diff ${A.base_commit}..${x.head_commit} -- docs CLAUDE.md removes nothing unintended); CLAUDE.md <= 25,000 bytes.
Lane report (truncated): ${String(x.report_markdown || '').slice(0, 14000)}
REPORT_FILE: ${RPT}/review-${A.lane}-${r.key}-r${n}.md. FAIL only for a MUST (a defect that ships, a missed ruling, or a test that cannot fail).`, { agentType: 'reviewer', model: 'sonnet', effort: A.reviewEffort || 'high', schema: RS, phase: n === 2 ? 'Re-review' : 'Re-review 2', label: `review:${A.lane}:${r.key}:r${n}` })))

phase('Fix')
const f1 = await agent(FIX(A.from_commit, A.rulings, 1), { agentType: 'builder', model: A.model || 'opus', effort: A.effort || 'high', schema: BS, phase: 'Fix', label: 'fix1:' + A.lane })
if (!f1) return { fix1: null }
log(`${A.lane} fix1: ${f1.status} @ ${f1.head_commit}`)
if (f1.status === 'BLOCKED') return { fix1: f1 }
const r2 = await reviewR(f1, 2)
const musts = (r2 || []).filter(Boolean).flatMap(r => r.findings || []).filter(f => f.severity === 'MUST')
const shoulds = (r2 || []).filter(Boolean).flatMap(r => r.findings || []).filter(f => f.severity === 'SHOULD')
log(`${A.lane} r2: ${musts.length} MUST, ${shoulds.length} SHOULD`)
if (!musts.length) return { fix1: f1, reviews2: r2 }
phase('Fix 2')
const f2 = await agent(FIX(f1.head_commit, JSON.stringify([...musts, ...shoulds]).slice(0, 12000), 2), { agentType: 'builder', model: A.model || 'opus', effort: A.effort || 'high', schema: BS, phase: 'Fix 2', label: 'fix2:' + A.lane })
if (!f2) return { fix1: f1, reviews2: r2, fix2Failed: true }
const r3 = await reviewR(f2, 3)
return { fix1: f1, reviews2: r2, fix2: f2, reviews3: r3 }
