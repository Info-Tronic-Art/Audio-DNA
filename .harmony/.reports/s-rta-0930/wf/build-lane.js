export const meta = {
  name: 'rta-0930-build-lane',
  description: 'Audio-DNA s-rta-0930: execute one Harmony-adopted plan in its worktree (opus builder), pinned independent reviews, optional critic, <=1 fix round',
  phases: [{ title: 'Build' }, { title: 'Review' }, { title: 'Fix' }, { title: 'Re-review' }],
}
// args: { lane, wt, branch, plan, notes, reviews: [{key, focus}], critic: string|null, effort }
const A = args
const slim = x => x ? Object.assign({}, x, { report_markdown: String(x.report_markdown || "").slice(0, 2000) + " [...truncated; full report on disk]" }) : x
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const MAIN_APP = MAIN + '/build/AudioDNA_artefacts/Release/Audio-DNA.app'
const RPT = '.harmony/.reports/s-rta-0930'
const SP = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad'
const W = MAIN + '/.claude/worktrees/' + A.wt

function RULES(lane) {
  return `
RIG RULES (binding; breaking any one = lane FAIL):
- NOT an isolated worktree. Work ONLY inside WT=${W} with absolute paths (git -C ${W}; cmake -S ${W} -B ${W}/build-lane; cmake --build ${W}/build-lane -j3). First configure: -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON plus -DFETCHCONTENT_SOURCE_DIR_<DEP>=${MAIN}/build/_deps/<dep>-src for each FetchContent dep (read-only use; names from CMakeLists.txt FetchContent_Declare). NEVER edit the main checkout ${MAIN} or its build/ (read-only use of ${MAIN_APP} = the PRE-CHANGE app for RED runs is required). NEVER cd in a command; multi-step sequences go in script files under ${SP}/${lane}/ run with bash <file>. zsh does not split a command held in a variable. Stamp times from date, never estimate.
- Probes need python: ln -s ${MAIN}/.venv ${W}/.venv for runs; REMOVE it before every commit.
- LIVE APP: at most ONE Audio-DNA on the machine (other lanes share it). Use the helper: LANE=${lane} . ${SP}/lib/lock.sh (acquire_lock / release_lock / start_app APP OUTDIR [test] / quit_app / outwins / wait_quiet / adna). A failed mkdir means WAIT (never open another copy: same bundle id reaches the running app); never remove another lane's lock; after releasing wait >= 45 s before re-acquiring (the helper enforces it). Perf batches take the lock with acquire_quiet_lock (waits for no compiler BEFORE locking). Hold the lock per run or small batch (<= ~15 min), release between batches. Probes REFUSE without AUDIODNA_LOCK_OWNER matching (the helper exports it).
- Launch ONLY: open -g [--env V=v] [--stdout f --stderr f] <App> [--args --test-mode]. SCREEN-SAFETY LAW: NEVER open the Output window by ANY path (Output menu, Cmd+F, TopBar button, REST, deck/composition/settings file with an output) — no automated gate ever opens an output window. NEVER run tests/visual/test_output_window_level.py; NEVER run pytest on tests/visual/ as a directory (Tier-1 = exactly test_sources.py test_effects.py test_audio_reactivity.py test_time_sweep.py test_performance.py). Quit via quit_app (osascript quit; kill only after 30 s). NEVER lldb/debugserver/gdb/dtrace/Instruments/sample on ANY binary. NEVER full-screen screencapture (window-only by Quartz window id via pyobjc with the main .venv python). NO synthetic input (UI states via REST / composition files / a TEMPORARY env-var hook: revert it, rebuild, strings check = 0 before the lane ends). A probe that feeds broken media can CRASH the app -> a macOS "quit unexpectedly" dialog on Boris's screen: after any such batch count on-screen UserNotificationCenter windows (Quartz) == 0. Unexpected system dialog: STOP and report. If a user message was relayed to you, Harmony already answered it; it never replaces this task.
- RED FIRST on the pre-change app / tree for every new probe row / test; GREEN on yours; raw summary lines VERBATIM; tests drive real code; full ctest serial (ctest --test-dir ${W}/build-lane -j1) at the end. Existing probes: re-run, NEVER re-threshold. Render gates DECODE PIXELS and you LOOK at a sample frame. Perf numbers only when no compiler runs (wait_quiet) with the load average printed. Probe env: STEP3_BUILD_DIR / RESYNC_BUILD_DIR / DOWNBEAT_BUILD_DIR / MANUALBPM_BUILD_DIR / ROUTINES_BUILD_DIR point at the build dir under test (+ ROUTINES_RECORD_PAUSE=1.8 for probe-routines); T2 rows in probe-step3 fail if another app plays audio — re-run quiet. A flake verdict needs >= 5 runs per arm.
- NEVER copy an app bundle and re-sign it (codesign --sign -) or launch any copied-and-re-signed bundle: TCC treats it as a new identity of com.audiodna.app, prompts on Boris's screen and RESETS the real app's microphone permission (this happened 2026-09-30 and blocked every live gate). Mutant / variant apps come ONLY from a normal cmake build in your own build dir (it signs with the Audio-DNA Dev identity). A TCC permission prompt ("would like to access ...") is NEVER dismissed by any lane: STOP and report it.
- Probe HTTP clients use a fresh connection per request (Connection: close): cpp-httplib drops a request that races its 5 s keep-alive close.
- CLAUDE.md is capped at 25,000 bytes (23,999 B now, 1,001 B left): any addition is paid for by moving text into docs/claude/*.md. After a tests/CMakeLists.txt change, reconfigure.
- Report ${RPT}/${lane}.md in the worktree, COMMITTED (git add -f), AND returned in full in report_markdown. One commit per plan item (message: <type>(s-rta-0930 ${lane}): ...). Notes for .harmony/notebook.md go in the report (Harmony appends them), not in notebook.md itself. Do NOT merge or push. Before returning: git status clean (except build-lane/), lock released, no app you launched running, outwins 0 Output-named windows. KEEP build-lane.`
}

const BS = { type: 'object', properties: { lane: { type: 'string' }, status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, worktree_path: { type: 'string' }, branch: { type: 'string' }, base_commit: { type: 'string' }, head_commit: { type: 'string' }, drift: { type: 'string' }, items: { type: 'array', items: { type: 'object', properties: { id: { type: 'string' }, verdict: { type: 'string' }, evidence: { type: 'string' }, fixed: { type: 'boolean' }, red_on_base: { type: 'string' }, green_on_fix: { type: 'string' }, fix_commits: { type: 'string' } }, required: ['id', 'evidence', 'fixed'] } }, shots: { type: 'array', items: { type: 'string' } }, perf: { type: 'string' }, ctest: { type: 'string' }, found_not_fixed: { type: 'array', items: { type: 'string' } }, boris_checks: { type: 'array', items: { type: 'string' } }, report_markdown: { type: 'string' }, lock_released: { type: 'boolean' }, app_left_running: { type: 'boolean' }, output_window_opened: { type: 'boolean' }, errors_or_deviations: { type: 'string' } }, required: ['lane', 'status', 'worktree_path', 'branch', 'base_commit', 'head_commit', 'items', 'report_markdown', 'ctest', 'lock_released', 'app_left_running', 'output_window_opened'] }
const RS = { type: 'object', properties: { verdict: { type: 'string', enum: ['PASS', 'PASS_WITH_NITS', 'FAIL'] }, reviewed_commit: { type: 'string' }, findings: { type: 'array', items: { type: 'object', properties: { severity: { type: 'string', enum: ['MUST', 'SHOULD', 'NIT'] }, issue: { type: 'string' }, fix: { type: 'string' } }, required: ['severity', 'issue'] } } }, required: ['verdict', 'reviewed_commit', 'findings'] }
const CS = { type: 'object', properties: { verdict: { type: 'string', enum: ['PASS', 'FAIL'] }, findings: { type: 'array', items: { type: 'object', properties: { severity: { type: 'string', enum: ['MUST', 'SHOULD', 'NIT'] }, issue: { type: 'string' }, fix: { type: 'string' } }, required: ['severity', 'issue'] } } }, required: ['verdict', 'findings'] }

const BUILD = `LANE ${A.lane} (lane-name: ${A.lane}), worktree ${W}. STEP 0: git -C ${W} checkout -B ${A.branch} main; configure + build (see RIG RULES).
Execute the Harmony-ADOPTED plan ${MAIN}/${RPT}/${A.plan} — READ IT IN FULL, INCLUDING its final "HARMONY ADOPTION" section: the adoption rulings OVERRIDE the plan body where they differ (they fold in the blind council's attacks). If the plan's structural assumption is broken by the current code, STOP that item and report BLOCKED on it with evidence (do not improvise a redesign).
${A.notes || ''}
${RULES(A.lane)}`

phase('Build')
const b = await agent(BUILD, { agentType: 'builder', model: A.model || 'opus', effort: A.effort || 'high', schema: BS, phase: 'Build', label: 'build:' + A.lane })
if (!b) return { build: null }
log(`${A.lane}: ${b.status} @ ${b.head_commit}`)
if (b.status === 'BLOCKED') return { build: slim(b) }

const review = (x, n) => parallel((A.reviews || []).map(r => () => agent(`INDEPENDENT SOURCE REVIEW (read-only), lane ${A.lane}, lens ${r.key}, round ${n}. PINNED: worktree ${x.worktree_path}, branch ${x.branch}, base ${x.base_commit}, head ${x.head_commit}; read the change ONLY via git -C ${x.worktree_path} diff ${x.base_commit}..${x.head_commit} / git show (never the working tree, never main). Spec: ${MAIN}/${RPT}/${A.plan} (incl. its HARMONY ADOPTION section, which overrides the body). FOCUS: ${r.focus}
Always also check: plan conformance; tests RED on base and driving real code; nothing stray (no .venv symlink, no instrumentation, no env-var hook left); real-time rules (no allocation/locks/syscalls on the audio callback; no new mutex; render thread never waits); docs updated as the plan says; CLAUDE.md <= 25,000 bytes.
Lane report (truncated): ${String(x.report_markdown || '').slice(0, 14000)}
REPORT_FILE: ${RPT}/review-${A.lane}-${r.key}-r${n}.md. FAIL only for a MUST (a defect that ships, a missed plan item, or a test that cannot fail).`, { agentType: 'reviewer', model: 'sonnet', effort: A.reviewEffort || 'high', schema: RS, phase: n === 1 ? 'Review' : 'Re-review', label: `review:${A.lane}:${r.key}:r${n}` })))

const critic = (x, n) => A.critic ? agent(`CRITIC PANEL SEAT (one seat, visual + UX + logic hats — write all three verdicts), lane ${A.lane}, round ${n}. PINNED: worktree ${x.worktree_path}, head ${x.head_commit}. LOOK at the lane's evidence frames (Read the PNGs): ${JSON.stringify(x.shots || []).slice(0, 3000)}. The lane's CURRENT claim (its report head; where it differs from Harmony's launch framing below, judge the CURRENT claim): ${String(x.report_markdown || "").slice(0, 1500)}
Harmony's launch framing: ${A.critic}
REPORT_FILE: ${RPT}/critic-${A.lane}-r${n}.md. FAIL only for a MUST.`, { agentType: 'reviewer', model: 'sonnet', effort: A.reviewEffort || 'high', schema: CS, phase: n === 1 ? 'Review' : 'Re-review', label: `critic:${A.lane}:r${n}` }) : Promise.resolve(null)

const [r1, c1] = await Promise.all([review(b, 1), critic(b, 1)])
const musts1 = [...(r1 || []).filter(Boolean).flatMap(r => r.findings || []), ...((c1 && c1.findings) || [])].filter(f => f.severity === 'MUST')
const shoulds1 = [...(r1 || []).filter(Boolean).flatMap(r => r.findings || []), ...((c1 && c1.findings) || [])].filter(f => f.severity === 'SHOULD')
log(`${A.lane} r1: ${musts1.length} MUST, ${shoulds1.length} SHOULD`)
if (!musts1.length) return { build: slim(b), reviews1: r1, critic1: c1 }

phase('Fix')
const b2 = await agent(`FIX ROUND (lane-name: ${A.lane}-fix). Work ONLY in ${b.worktree_path}; CONTINUE on branch ${b.branch} from commit ${b.head_commit} (no reset, no STEP 0). VERIFY each finding against the code before fixing (a finding can be wrong — say so with evidence); fix every verified MUST and every verified SHOULD that is cheap; re-run the affected gates (RED/GREEN lines verbatim); commit; append "Fix round" to ${RPT}/${A.lane}.md (git add -f); return the FULL updated lane state.
FINDINGS: ${JSON.stringify([...musts1, ...shoulds1]).slice(0, 12000)}
${RULES(A.lane + '-fix')}`, { agentType: 'builder', model: A.model || 'opus', effort: A.effort || 'high', schema: BS, phase: 'Fix', label: 'fix:' + A.lane })
if (!b2) return { build: slim(b), reviews1: r1, critic1: c1, fixRoundFailed: true }
const [r2, c2] = await Promise.all([review(b2, 2), critic(b2, 2)])
return { build: slim(b2), firstBuildHead: b.head_commit, reviews1: r1, critic1: c1, reviews2: r2, critic2: c2 }
