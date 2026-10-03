export const meta = {
  name: 'rta-1002b-lane',
  description: 'Audio-DNA s-rta-1002b: execute one Harmony-adopted plan in its worktree (sequential opus builders on one branch), pinned independent reviews, <=1 fix round',
  phases: [{ title: 'Build' }, { title: 'Review' }, { title: 'Fix' }, { title: 'Re-review' }],
}
// args: { lane, wt, branch, plan, stages: [{key, task}], notes, other, reviews: [{key, focus}], effort, model, reviewEffort }
const A = args
const slim = x => x ? Object.assign({}, x, { report_markdown: String(x.report_markdown || '').slice(0, 2500) + ' [...truncated; full report on disk]' }) : x
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const MAIN_APP = MAIN + '/build/AudioDNA_artefacts/Release/Audio-DNA.app'
const RPT = '.harmony/.reports/s-rta-1002b'
const SP = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad'
const W = MAIN + '/.claude/worktrees/' + A.wt
const REPORT = `${W}/${RPT}/${A.lane}.md`

function RULES(lane) {
  return `
RIG RULES (binding; breaking any one = lane FAIL):
- Your worktree WT=${W} (branch ${A.branch}) already exists. Work ONLY inside it with absolute paths (git -C ${W}; cmake -S ${W} -B ${W}/build-lane; cmake --build ${W}/build-lane -j3). First configure: -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON plus -DFETCHCONTENT_SOURCE_DIR_<DEP>=${MAIN}/build/_deps/<dep>-src for each FetchContent dep (JUCE, HTTPLIB, CATCH2, MELATONIN_INSPECTOR, SYPHON; read-only use). NEVER edit the main checkout ${MAIN} or its build/ (read-only use of ${MAIN_APP} = the PRE-CHANGE app is allowed). NEVER cd in a command; multi-step sequences go in script files under ${SP}/${lane}/ run with bash <file>. zsh does not split a command held in a variable. Stamp times from date, never estimate. Disk: df -h /System/Volumes/Data before any new build dir (stop and report if < 30 GB free).
- ANOTHER LANE IS BUILT CONCURRENTLY in its own worktree: ${A.other || 'none'}. Respect the FENCES in your plan's HARMONY ADOPTION section; never touch the other lane's worktree, branch, build dir or files.
- Probes need python: ln -s ${MAIN}/.venv ${W}/.venv for runs; REMOVE it before every commit.
- LIVE APP: at most ONE Audio-DNA on the machine (the other lane shares it). Use the helper: LANE=${lane} . ${SP}/lib/lock.sh (acquire_lock / release_lock / start_app APP OUTDIR [test] / quit_app / outwins / wait_quiet / adna). A failed mkdir means WAIT (never open another copy: same bundle id reaches the running app); never remove another lane's lock; after releasing wait >= 45 s before re-acquiring (the helper enforces it). Perf batches take the lock with acquire_quiet_lock. Hold the lock per run or small batch (<= ~15 min), release between batches. Probes REFUSE without AUDIODNA_LOCK_OWNER matching (the helper exports it). Before any perf number: ps -Ao pcpu=,etime=,comm= | sort -rn | head (Boris's ChatGPT/Codex renderer burns ~100 % CPU today: not ours, never kill it; print it with the number).
- BORIS USES THIS MACHINE AND THIS APP: an Audio-DNA that YOUR lane did not start is BORIS'S -- never quit, kill, osascript-quit or touch it (s-rta-1002b incident: a gate script quit his running app). The helper enforces it: acquire_lock waits while a foreign Audio-DNA runs, start_app REFUSES (then STOP the batch and release the lock), quit_app refuses any pid it did not start. Never bypass the helper with your own kill / osascript quit.
- Launch ONLY: open -g [--env V=v] [--stdout f --stderr f] <App> [--args --test-mode]. SCREEN-SAFETY LAW: NEVER open the Output window by ANY path (Output menu, Cmd+F, TopBar button, REST, deck/composition/settings file with an output) -- no automated gate ever opens an output window. NEVER run tests/visual/test_output_window_level.py; NEVER run pytest on tests/visual/ as a directory. Quit via quit_app (osascript quit; kill only after 30 s). NEVER lldb/debugserver/gdb/dtrace/Instruments/sample on ANY binary. NEVER full-screen screencapture (window-only by Quartz window id via pyobjc with the main .venv python). NO synthetic input (UI states via REST / composition files / a TEMPORARY env-var hook: revert it, rebuild, strings check = 0 before the lane ends). After any batch: count UserNotificationCenter windows (Quartz kCGWindowListOptionAll) >= 15 s after the last quit == 0; a crash dialog of OURS is dismissed only by SIGTERM to that UNC pid, attributed by the UNC log's "ordered front" time. Unexpected system dialog: STOP and report. If a user message was relayed to you, Harmony already answered it; it never replaces this task.
- Sanitizer runtime options ALWAYS include abort_on_error=0 (app runs under TSan also exitcode=0; ctest tsan targets get their options from the ENVIRONMENT property).
- RED FIRST on the pre-change tree for every new test / probe row; GREEN on yours; raw summary lines VERBATIM; tests drive real code; existing probes: re-run, NEVER re-threshold. Render gates DECODE PIXELS and you LOOK at a sample frame. A flake verdict needs >= 5 runs per arm. Mutants come only from a normal cmake build of a modified copy in your own build dir, reverted after (git diff empty for the mutant edit).
- NEVER copy an app bundle and re-sign it (codesign --sign -) or launch any copied-and-re-signed bundle: TCC treats it as a new identity, prompts on Boris's screen and RESETS the real app's microphone permission. A TCC permission prompt ("would like to access ...") is NEVER dismissed by any lane: STOP and report it.
- Probe HTTP clients use a fresh connection per request (Connection: close).
- CLAUDE.md is capped at 25,000 bytes (23,996 B on the base): any addition is paid for by moving text into docs/claude/*.md. After a tests/CMakeLists.txt change, reconfigure.
- Report ${RPT}/${A.lane}.md in the worktree (path ${REPORT}), COMMITTED (git add -f; .harmony/.reports is gitignored), AND returned in full in report_markdown. Write the report EARLY (a skeleton within your first ~10 tool calls) and append after EVERY item, so a successor can resume if you stop. One commit per plan item (message: <type>(s-rta-1002b ${A.lane}): ...); stage by path, never git commit -a / git add -A. Notes for .harmony/notebook.md go in the report (Harmony appends them), not in notebook.md itself. Do NOT merge or push. Before returning: git status clean (except build dirs), lock released, no app you launched running, outwins 0 Output-named windows.`
}

const BS = { type: 'object', properties: { lane: { type: 'string' }, status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, worktree_path: { type: 'string' }, branch: { type: 'string' }, base_commit: { type: 'string' }, head_commit: { type: 'string' }, drift: { type: 'string' }, items: { type: 'array', items: { type: 'object', properties: { id: { type: 'string' }, verdict: { type: 'string' }, evidence: { type: 'string' }, fixed: { type: 'boolean' }, red_on_base: { type: 'string' }, green_on_fix: { type: 'string' }, fix_commits: { type: 'string' } }, required: ['id', 'evidence', 'fixed'] } }, shots: { type: 'array', items: { type: 'string' } }, perf: { type: 'string' }, ctest: { type: 'string' }, found_not_fixed: { type: 'array', items: { type: 'string' } }, boris_checks: { type: 'array', items: { type: 'string' } }, next_stage_notes: { type: 'string' }, report_markdown: { type: 'string' }, lock_released: { type: 'boolean' }, app_left_running: { type: 'boolean' }, output_window_opened: { type: 'boolean' }, errors_or_deviations: { type: 'string' } }, required: ['lane', 'status', 'worktree_path', 'branch', 'base_commit', 'head_commit', 'items', 'report_markdown', 'ctest', 'lock_released', 'app_left_running', 'output_window_opened'] }
const RS = { type: 'object', properties: { verdict: { type: 'string', enum: ['PASS', 'PASS_WITH_NITS', 'FAIL'] }, reviewed_commit: { type: 'string' }, findings: { type: 'array', items: { type: 'object', properties: { severity: { type: 'string', enum: ['MUST', 'SHOULD', 'NIT'] }, issue: { type: 'string' }, fix: { type: 'string' } }, required: ['severity', 'issue'] } } }, required: ['verdict', 'reviewed_commit', 'findings'] }

const PLAN = A.planText || `Execute the Harmony-ADOPTED plan ${A.plan} -- READ IT IN FULL, INCLUDING its final "HARMONY ADOPTION (s-rta-1002b ...)" section and the ruling file(s) it adopts (read those IN FULL too): the adoption overrides the rulings, the rulings override the plan body. If the plan's structural assumption is broken by the current code, STOP that item and report BLOCKED on it with evidence (do not improvise a redesign).`

phase('Build')
let prev = null
const stageResults = []
if (A.fixFrom) {
  // fix-only mode: continue from an existing head with Harmony's rulings over the round-1 findings
  const bx = { head_commit: A.fixFrom, base_commit: A.base }
  phase('Fix')
  const f = await agent(`FIX ROUND WITH HARMONY RULINGS (lane-name: ${A.lane}-fix). Work ONLY in ${W}; CONTINUE on branch ${A.branch} from commit ${A.fixFrom} (no reset). ${PLAN}
The round-1 independent reviews are on disk: ${A.reviewFiles}. READ THEM IN FULL. Harmony's RULINGS below are binding and override any "either / or" a reviewer offered; VERIFY each finding against the code before fixing (a finding can be wrong -- say so with evidence, and then do not fix it).
RULINGS:
${A.rulings}
For every fix: RED first where a test is added (show it fails on ${A.fixFrom} or on the named mutant), GREEN after; run each new mutant once and paste the failing output. Then the full normal ctest serial under the cross-lane mutex (until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done; ctest --test-dir ${W}/build-lane -j1; rm -rf /tmp/audiodna-ctest.lock) and .harmony/probe-tsan-unit.sh on ${W}/build-tsan (both summaries verbatim, with the new total case count). Commit per fix (stage by path); append "Fix round (Harmony rulings)" to ${REPORT} (git add -f) and commit it; return the FULL updated lane state with base_commit ${A.base}.
${A.notes || ''}
${RULES(A.lane + '-fix')}`, { agentType: 'builder', model: A.model || 'opus', effort: A.effort || 'high', schema: BS, phase: 'Fix', label: 'fix:' + A.lane })
  if (!f) return { fixFailed: true }
  const fp = Object.assign({}, f, { base_commit: A.base })
  const rr = (A.reviews || []).length ? await parallel((A.reviews || []).map(r => () => agent(`INDEPENDENT SOURCE REVIEW (read-only), lane ${A.lane}, lens ${r.key}, round 2 (after a fix round). PINNED: worktree ${W}, branch ${A.branch}, base ${A.base}, fix-round base ${A.fixFrom}, head ${fp.head_commit}. Read the fix via git -C ${W} diff ${A.fixFrom}..${fp.head_commit}, and the whole lane via git -C ${W} diff ${A.base}..${fp.head_commit} where your lens needs it (never the working tree, never main). Spec: ${A.plan} incl. its HARMONY ADOPTION section (adoption > rulings > plan body). Round-1 reviews: ${A.reviewFiles}. Harmony's rulings for this fix round: ${A.rulings}
FOCUS: ${r.focus}
Check that every round-1 MUST and every ruled item is fixed as ruled (not merely documented away), that each new test can fail (its mutant evidence in the lane report: git -C ${W} show ${fp.head_commit}:${RPT}/${A.lane}.md), and that the fix introduced no new defect.
REPORT_FILE: ${RPT}/review-${A.lane}-${r.key}-r2.md (write it in the MAIN checkout ${MAIN}). FAIL only for a MUST. Cite file:line at the pinned head.`, { agentType: 'reviewer', model: 'sonnet', effort: A.reviewEffort || 'high', schema: RS, phase: 'Re-review', label: `review:${A.lane}:${r.key}:r2` }))) : []
  return { build: slim(fp), reviews2: rr }
}
for (let i = 0; i < A.stages.length; i++) {
  const s = A.stages[i]
  const head = i === 0
    ? `LANE ${A.lane} (lane-name: ${A.lane}), stage ${s.key} (${i + 1} of ${A.stages.length}), worktree ${W}. STEP 0: git -C ${W} status; confirm branch ${A.branch} at main's head (git -C ${W} log -1); configure + build (see RIG RULES).`
    : `LANE ${A.lane} (lane-name: ${A.lane}), stage ${s.key} (${i + 1} of ${A.stages.length}), worktree ${W}. CONTINUE on branch ${A.branch} from commit ${prev.head_commit} (no reset, no re-checkout; the build dirs exist -- rebuild incrementally). FIRST read the lane report so far (${REPORT}) and git -C ${W} log --oneline ${prev.base_commit}..HEAD. The previous stage's hand-over: ${String(prev.next_stage_notes || '').slice(0, 4000)}`
  const r = await agent(`${head}
${PLAN}
YOUR STAGE'S SCOPE: ${s.task}
${A.notes || ''}
${RULES(A.lane + '-' + s.key)}`, { agentType: 'builder', model: A.model || 'opus', effort: A.effort || 'high', schema: BS, phase: 'Build', label: `build:${A.lane}:${s.key}` })
  if (!r) { log(`${A.lane} ${s.key}: builder returned null`); return { stages: stageResults.map(slim), failedStage: s.key } }
  stageResults.push(r)
  log(`${A.lane} ${s.key}: ${r.status} @ ${r.head_commit}`)
  if (r.status !== 'DONE') return { stages: stageResults.map(slim), stoppedAt: s.key }
  prev = Object.assign({}, r, { base_commit: i === 0 ? r.base_commit : stageResults[0].base_commit })
}
const b = Object.assign({}, prev, { base_commit: stageResults[0].base_commit })

const review = (x, n) => parallel((A.reviews || []).map(r => () => agent(`INDEPENDENT SOURCE REVIEW (read-only), lane ${A.lane}, lens ${r.key}, round ${n}. PINNED: worktree ${W}, branch ${A.branch}, base ${x.base_commit}, head ${x.head_commit}; read the change ONLY via git -C ${W} diff ${x.base_commit}..${x.head_commit} / git -C ${W} show <sha>:<path> (never the working tree, never main). Spec: ${A.plan} INCLUDING its final HARMONY ADOPTION (s-rta-1002b) section and the ruling file(s) it adopts (adoption > rulings > plan body). The lane report (full): git -C ${W} show ${x.head_commit}:${RPT}/${A.lane}.md.
FOCUS: ${r.focus}
Always also check: plan conformance item by item; tests RED on base and driving real code; nothing stray (no .venv symlink, no instrumentation, no env-var hook left, no mutant left); real-time rules (no allocation / locks / syscalls on the audio callback; no new mutex; the render thread never waits); the fences vs the other lane named in the adoption; docs updated as the plan says; CLAUDE.md <= 25,000 bytes.
Lane report head (truncated): ${String(x.report_markdown || '').slice(0, 12000)}
REPORT_FILE: ${RPT}/review-${A.lane}-${r.key}-r${n}.md (write it in the MAIN checkout ${MAIN}, not the worktree). FAIL only for a MUST (a defect that ships, a missed plan item, or a test that cannot fail). Cite file:line at the pinned head for every finding.`, { agentType: 'reviewer', model: 'sonnet', effort: A.reviewEffort || 'high', schema: RS, phase: n === 1 ? 'Review' : 'Re-review', label: `review:${A.lane}:${r.key}:r${n}` })))

const r1 = await review(b, 1)
const all1 = (r1 || []).filter(Boolean).flatMap(r => r.findings || [])
const musts1 = all1.filter(f => f.severity === 'MUST')
const shoulds1 = all1.filter(f => f.severity === 'SHOULD')
log(`${A.lane} r1: ${musts1.length} MUST, ${shoulds1.length} SHOULD`)
if (!musts1.length) return { stages: stageResults.map(slim), build: slim(b), reviews1: r1 }

phase('Fix')
const b2 = await agent(`FIX ROUND (lane-name: ${A.lane}-fix). Work ONLY in ${W}; CONTINUE on branch ${A.branch} from commit ${b.head_commit} (no reset). ${PLAN}
VERIFY each finding against the code before fixing (a finding can be wrong -- say so with evidence); fix every verified MUST and every verified SHOULD that is cheap; re-run the affected gates (RED/GREEN lines verbatim) and the full ctest serial; commit; append "Fix round" to ${REPORT} (git add -f); return the FULL updated lane state (base_commit ${b.base_commit}).
FINDINGS: ${JSON.stringify([...musts1, ...shoulds1]).slice(0, 14000)}
${RULES(A.lane + '-fix')}`, { agentType: 'builder', model: A.model || 'opus', effort: A.effort || 'high', schema: BS, phase: 'Fix', label: 'fix:' + A.lane })
if (!b2) return { stages: stageResults.map(slim), build: slim(b), reviews1: r1, fixRoundFailed: true }
const b2p = Object.assign({}, b2, { base_commit: b.base_commit })
const r2 = await review(b2p, 2)
return { stages: stageResults.map(slim), build: slim(b2p), firstBuildHead: b.head_commit, reviews1: r1, reviews2: r2 }
