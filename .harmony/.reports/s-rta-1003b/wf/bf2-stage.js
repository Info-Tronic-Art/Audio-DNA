export const meta = {
  name: 'rta-1003b-bf2-stage',
  description: 'Audio-DNA s-rta-1003b: ONE build stage of the sync dial lane from a ruled plan (one opus builder in the worktree named by args), then pinned sonnet reviews and at most one fix round',
  phases: [{ title: 'Build' }, { title: 'Review' }, { title: 'Fix' }, { title: 'Re-review' }],
}
// args: { key, lane, wt, branch, base, task, buildNote, reportName, lenses: [{ key, focus }], prevNotes }
const A = args || {}
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const MAIN_APP = MAIN + '/build/AudioDNA_artefacts/Release/Audio-DNA.app'
const RPT = '.harmony/.reports/s-rta-1003b'
const R3 = '.harmony/.reports/s-rta-1003'
const OLD = '.harmony/.reports/s-rta-1002b'
const SP = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/00e87ddd-eec9-42a5-94d1-5dc67e66ea7d/scratchpad'
const W = MAIN + '/.claude/worktrees/' + A.wt
const BF2 = MAIN + '/.claude/worktrees/bf2'
const BR = A.branch
const BASE = A.base
const KEY = A.key
const LANE = A.lane
const REPORT = W + '/' + RPT + '/' + A.reportName
const HRUL = MAIN + '/' + RPT + '/rulings-bf2.md'
const STOPS = MAIN + '/' + RPT + '/ruling-bf2-stops.md'
const STOPSPLAN = MAIN + '/' + RPT + '/plan-bf2-stops.md'
const RESTATED = MAIN + '/' + RPT + '/ruling-bf2-gates-restated.md'
const RULING = MAIN + '/' + R3 + '/ruling-bf2-delta.md'
const PLAN = MAIN + '/' + R3 + '/plan-bf2-delta.md'

function RULES(lane) {
  return `
RIG RULES (binding; breaking any one = lane FAIL):
- Your worktree WT=${W} (branch ${BR}). Work ONLY inside it with absolute paths (git -C ${W}). ${A.buildNote} Any configure: -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON plus -DFETCHCONTENT_SOURCE_DIR_<DEP>=${MAIN}/build/_deps/<dep>-src for each FetchContent dep (JUCE, HTTPLIB, CATCH2, MELATONIN_INSPECTOR, SYPHON; read-only use); the patched libprojectM at ~/.local/opt/projectm-4.1.1-fbo1 is found by the project's CMake (never rebuild or touch it); read ${BF2}/build-lane/CMakeCache.txt for the exact settings rather than guessing. df -h /System/Volumes/Data before any new build dir (stop and report if < 30 GB free). No build may outlive the stage that started it. NEVER edit the main checkout ${MAIN} or its build/, and never another worktree. NEVER cd in a command; multi-step sequences go in script files under ${SP}/${lane}/ run with bash <file>; capture a return code on its own line (rc=$? right after the command) -- a line like "$(date) ... rc=$?" prints 0 always. zsh does not split a command held in a variable. Stamp times from date, never estimate.
- OTHER LANES ARE ACTIVE on this machine in OTHER worktrees (a keys fix or a probe stage of this same lane; a keying audit that launches main's app). You share with them only the live-app lock and the ctest mutex below. Probes use the main checkout's python (${MAIN}/.venv/bin/python, read-only; never pip install into it; a .venv symlink in the worktree is removed before every commit).
- LIVE APP: at most ONE Audio-DNA on the machine. Use the helper: LANE=${lane} . ${SP}/lib/lock.sh (acquire_lock / release_lock / start_app APP OUTDIR [test] / quit_app / outwins / wait_quiet / adna / acquire_quiet_lock). A failed mkdir means WAIT (never open another copy: the same bundle id reaches the running app); never remove another owner's lock; after releasing wait >= 45 s before re-acquiring (the helper enforces it). Hold the lock per run or small batch (<= ~15 min), release between batches. Probes REFUSE without AUDIODNA_LOCK_OWNER matching (the helper exports it). Every FULL ctest takes the cross-lane mutex: until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done; ctest ...; rm -rf /tmp/audiodna-ctest.lock. NO git commit while a live row with timing bars is running (each commit fires a background graph rebuild that burns CPU).
- BORIS USES THIS MACHINE AND THIS APP: an Audio-DNA that YOUR lane did not start is BORIS'S -- never quit, kill, osascript-quit or touch it. The helper enforces it (acquire_lock waits while a foreign Audio-DNA runs, start_app REFUSES -> STOP the batch and release the lock, quit_app refuses any pid it did not start). Never bypass the helper. A probe you run quits only the pid it launched (.harmony/probe-quit-ours.sh; selftest .harmony/probe-quit-ours-selftest.sh). NEVER copy a lock.sh or gate script out of an archived .harmony/.reports/s-rta-0927..0930 directory.
- Launch ONLY: open -g [--env V=v] [--stdout f --stderr f] <App> [--args --test-mode]. SCREEN-SAFETY LAW: NEVER open the Output window by ANY path. NEVER run tests/visual/test_output_window_level.py or .harmony/probe-outputs.sh; NEVER run pytest on tests/visual/ as a directory. NEVER lldb / debugserver / gdb / dtrace / Instruments / sample on ANY binary. NEVER full-screen screencapture (window-only by Quartz window id). NO synthetic input (UI states via REST / composition files / the lane's test-only routes). After any live batch: count UserNotificationCenter windows (Quartz kCGWindowListOptionAll) >= 15 s after the last quit == 0; a crash dialog of OURS is dismissed only by SIGTERM to that UNC pid, attributed by the UNC log's "ordered front" time. Unexpected system dialog or TCC permission prompt: STOP and report, never dismiss. A production-mode probe leaves the real ~/Library/Audio-DNA/settings.json byte-identical (sha256 before and after, printed). If a user message was relayed to you, Harmony already answered it; it never replaces this task.
- LIVE ROWS: the full-length gate runs and every gate verdict are HARMONY'S (the diagnosis run RD, R7's 21 arms, R5's five arms, G6, the quiet [timing] x3, R13 and R14 as gate lines). You run a row you wrote or changed SHORT, as development evidence labelled as such, to show it executes, prints its bar line, goes RED on the tree or mutant the ruling names and GREEN on yours.
- MUTANTS: a unit-test mutant lives in your build dir only between its edit and its revert; after the revert, touch the reverted file, rebuild, and show >= 1 object compiled and the test green again (a restore rebuild in the same mtime second compiles NOTHING). A mutant APP for a live RED arm is built in its OWN dir ${W}/build-mut-<name> (ignored by .gitignore), the source reverted the moment that build ends (git -C ${W} diff --quiet -- src tests; then rc on its own line), and the lane app is never built from mutant source; keep the mutant app dir and name it in the hand-over. NEVER copy an app bundle and re-sign it. SANITIZERS: runtime options ALWAYS include abort_on_error=0 (TSan app runs also exitcode=0).
- RED FIRST for every new test / probe row (raw lines in the report), then GREEN; a test drives real code; existing probes are re-run, NEVER re-thresholded (the only expectation changes are the ones a named amendment makes). A flake verdict needs >= 5 runs per arm. Probe HTTP clients use Connection: close.
- DOCS: lane detail goes to docs/claude/*.md; CLAUDE.md is capped at 25,000 bytes (24,370 now) and is not touched by this stage. No on-screen text that announces an event or a failure is added by any stage.
- Report file ${REPORT} (in YOUR worktree; another stage writes its own file -- never edit bf2-delta.md from here), COMMITTED. The stash-guard hook treats a path that starts with a dot as a whole-tree add and scans the whole command line: stage report files as git -C ${W}/.harmony/.reports add -f s-rta-1003b/<file>, keep a git add / commit in its OWN Bash call, commit with explicit paths (git -C ${W} commit -m "<msg>" -- <path> <path>), never git commit -a / git add -A, never chain a check to the commit it gates with &&. Create the report within your first ~10 tool calls and append after EVERY item, so a successor can resume. One commit per item (message: <type>(s-rta-1003b bf2 ${KEY}): ...). Do NOT merge into main or into another lane branch, do NOT push. The report ENDS with a "HAND-OVER TO HARMONY" block: the head commit, which build dir holds which app, and for EVERY gate row of your stage the exact command line Harmony runs, the exact PASS line it prints, and what its RED arm is. Before returning: git status clean (except ignored build dirs), lock released, no app you launched running, outwins shows 0 Output-named windows.`
}

const BS = { type: 'object', properties: {
  stage: { type: 'string' }, status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] },
  head_commit: { type: 'string' }, commits: { type: 'array', items: { type: 'string' } },
  items: { type: 'array', items: { type: 'object', properties: { id: { type: 'string' }, verdict: { type: 'string' }, evidence: { type: 'string' } }, required: ['id', 'verdict', 'evidence'] } },
  ctest: { type: 'string' }, tsan: { type: 'string' },
  found_not_fixed: { type: 'array', items: { type: 'string' } }, stop_items_for_harmony: { type: 'array', items: { type: 'string' } },
  facts_measured: { type: 'array', items: { type: 'string' } }, harmony_commands: { type: 'array', items: { type: 'string' } },
  next_stage_notes: { type: 'string' }, report_path: { type: 'string' }, summary: { type: 'string' },
  lock_released: { type: 'boolean' }, app_left_running: { type: 'boolean' }, output_window_opened: { type: 'boolean' },
  errors_or_deviations: { type: 'string' } },
  required: ['stage', 'status', 'head_commit', 'items', 'report_path', 'summary', 'lock_released', 'app_left_running', 'output_window_opened'] }
const RS = { type: 'object', properties: { verdict: { type: 'string', enum: ['PASS', 'PASS_WITH_NITS', 'FAIL'] }, reviewed_commit: { type: 'string' }, report_path: { type: 'string' }, findings: { type: 'array', items: { type: 'object', properties: { severity: { type: 'string', enum: ['MUST', 'SHOULD', 'NIT'] }, issue: { type: 'string' }, fix: { type: 'string' } }, required: ['severity', 'issue'] } } }, required: ['verdict', 'reviewed_commit', 'findings'] }

const CONTEXT = `CONTEXT. Lane bf2 = the SYNC DIAL of Audio-DNA: one signed number of milliseconds (-500..+500, per venue) that moves the picture LATE (a delay line) or EARLY (BeatLead) against the sound. This session merged main into the lane (M0) and built stages S3f and S4 and a review fix round; the lane head is 68abc16. That work left STOP ITEMS, and an architect ruled them after a blind council: ${STOPS} -- THE SPEC FOR YOUR STAGE. READ IT IN FULL: section 0 verdict, section 1 facts, section 3 amendments A1..A20, section 4 FINAL STAGES + ORDER (your stage's row and, for a probe stage, the diagnosis run's DECISION TABLE), section 5 the gate rows (exact strings), section 9 side findings. It overrides its plan ${STOPSPLAN} (read the plan body only where the ruling points into it).
PRECEDENCE: Boris's verbatim words > Harmony's rulings ${HRUL} (read it FIRST; H-13 is her adoption of the stops ruling and her decisions HD1..HD10) > ${STOPS} > the re-stated gate rows ${RESTATED} > the HARMONY ADOPTION at the end of ${PLAN} > ${RULING} > the earlier chain ${MAIN}/${OLD}/ruling-bf2.md and plan-bf2.md.
THE LANE'S REPORTS: ${BF2}/${RPT}/bf2-delta.md (this session: M0, S3f, S4, the fix round -- every hand-over block, the DRIFT list) and ${BF2}/${OLD}/bf2.md (S1a-S3). The builders' saved run logs: ${SP}/bf2-S3f/, ${SP}/bf2-S4/, ${SP}/bf2-R1/ (read-only).
Boris's words are in ${MAIN}/.harmony/binding-decisions.md (quote him only verbatim).
If a structural assumption of the ruling is broken by the code you find, STOP that item and report it in stop_items_for_harmony with evidence (do not improvise a redesign). A STOP clause of the ruling is binding.`

const slimB = r => Object.assign({}, r, { summary: String(r.summary || '').slice(0, 1400), next_stage_notes: String(r.next_stage_notes || '').slice(0, 1500), errors_or_deviations: String(r.errors_or_deviations || '').slice(0, 1200), items: (r.items || []).map(x => ({ id: x.id, verdict: String(x.verdict || '').slice(0, 140), evidence: String(x.evidence || '').slice(0, 300) })), found_not_fixed: (r.found_not_fixed || []).map(x => String(x).slice(0, 300)), stop_items_for_harmony: (r.stop_items_for_harmony || []).map(x => String(x).slice(0, 500)), facts_measured: (r.facts_measured || []).map(x => String(x).slice(0, 320)), harmony_commands: (r.harmony_commands || []).map(x => String(x).slice(0, 400)) })
const slimR = rs => rs.map(x => ({ lens: x.lens, verdict: x.verdict, reviewed_commit: x.reviewed_commit, findings: (x.findings || []).map(f => ({ severity: f.severity, issue: String(f.issue || '').slice(0, 500), fix: String(f.fix || '').slice(0, 250) })) }))

const LENSES = A.lenses || []
const review = (lens, base, head, n, extra) => agent(`INDEPENDENT SOURCE REVIEW (read-only), lane bf2 (the sync dial), stage ${KEY}, lens ${lens.key}, round ${n}. PINNED: worktree ${W}, branch ${BR}, base ${base}, head ${head}. Read the change ONLY through git objects: git -C ${W} diff ${base}..${head} -- <paths>, git -C ${W} log --first-parent --oneline ${base}..${head}, git -C ${W} show ${head}:<path>, git -C ${W} grep -n '<pattern>' ${head} -- <paths> (NEVER the working tree -- a builder may be editing it -- and never main's checkout for lane code).
SPEC: Harmony's rulings ${HRUL} (binding, read first; a point she has ruled is not a finding), then ${STOPS} (the architect ruling on the stop items: section 3 amendments A1..A20, section 4 stages incl. the diagnosis decision table, section 5 gate rows), then the chain it amends (${RESTATED}, ${RULING}, the HARMONY ADOPTION at the end of ${PLAN}). The stage's report at the head: git -C ${W} show ${head}:${RPT}/${A.reportName}.
FOCUS: ${lens.focus}
${extra || ''}
Always also check: ruling conformance item by item for your lens; each new test or self-test case RED before and GREEN after per the report, and driving real code; nothing stray (no mutant, no instrumentation left, no .venv link); no on-screen text that announces an event or a failure added; docs as the ruling says. You may run read-only git commands and text greps; do NOT build, run tests, launch the app or run a probe.
REPORT_FILE: ${RPT}/review-bf2-${KEY}-${lens.key}-r${n}.md (write it in the MAIN checkout: ${MAIN}/${RPT}/review-bf2-${KEY}-${lens.key}-r${n}.md). Verdict FAIL only for a MUST (a defect that ships, a missed ruling item, a gate or test that cannot fail, a script that can quit an app it did not start). Cite file:line at the pinned head for every finding; label each VERIFIED (read / grepped by you) or INFERRED.`, { agentType: 'reviewer', model: 'sonnet', effort: 'high', schema: RS, phase: n === 1 ? 'Review' : 'Re-review', label: 'review:bf2:' + KEY + ':' + lens.key + ':r' + n }).then(x => x ? Object.assign({ lens: lens.key }, x) : { lens: lens.key, verdict: 'NO_RESULT', reviewed_commit: head, findings: [] }).catch(() => ({ lens: lens.key, verdict: 'NO_RESULT', reviewed_commit: head, findings: [] }))

phase('Build')
const b1 = await agent(`LANE ${LANE}, stage ${KEY}, worktree ${W}, branch ${BR} (base ${BASE}). STEP 0: git -C ${W} status (clean) and git -C ${W} log --first-parent --oneline -6; if ${REPORT} exists read it first (you are resuming an interrupted stage: continue it, do not redo finished items).
${CONTEXT}
YOUR STAGE'S SCOPE: ${A.task}
${A.prevNotes ? 'NOTES FROM EARLIER STAGES: ' + String(A.prevNotes).slice(0, 4000) : ''}
RETURN: the structured result; summary <= 1200 characters (full detail lives in the report file); items = one entry per ruling item of your stage with its verdict and the key evidence line; facts_measured = each fact you measured with its raw line; harmony_commands = the exact command lines Harmony runs for your stage's gate rows; next_stage_notes <= 1500 characters.
${RULES(LANE)}`, { agentType: 'builder', model: 'opus', effort: 'high', schema: BS, phase: 'Build', label: 'build:' + LANE })
if (!b1) return { failedStage: KEY }
log(LANE + ': ' + b1.status + ' @ ' + b1.head_commit)
if (b1.status === 'BLOCKED' || !LENSES.length) return { stage: slimB(b1) }

phase('Review')
const r1 = await Promise.all(LENSES.map(l => review(l, BASE, b1.head_commit, 1, '')))
const all1 = r1.flatMap(x => (x.findings || []).map(f => Object.assign({ lens: x.lens }, f)))
const musts1 = all1.filter(f => f.severity === 'MUST')
const shoulds1 = all1.filter(f => f.severity === 'SHOULD')
log(KEY + ' r1: ' + musts1.length + ' MUST, ' + shoulds1.length + ' SHOULD; verdicts ' + r1.map(x => x.lens + '=' + x.verdict).join(' '))
if (!musts1.length && !shoulds1.length) return { stage: slimB(b1), head: b1.head_commit, reviews1: slimR(r1) }
if (b1.status !== 'DONE' && !musts1.length) return { stage: slimB(b1), head: b1.head_commit, reviews1: slimR(r1) }

phase('Fix')
const b2 = await agent(`LANE ${LANE}, stage ${KEY}, FIX ROUND after the pinned reviews, worktree ${W}. CONTINUE on branch ${BR} from commit ${b1.head_commit} (no reset). FIRST read your stage report ${REPORT}.
${CONTEXT}
The round-1 review reports are ${MAIN}/${RPT}/review-bf2-${KEY}-<lens>-r1.md (lenses ${r1.map(x => x.lens).join(', ')}): READ THEM IN FULL. VERIFY each finding against the code before fixing (a finding can be wrong -- say so with evidence, and then do not fix it). Fix every verified MUST; fix a verified SHOULD only when it is small, local and needs no design choice; anything that needs a design choice or changes what Boris sees goes to stop_items_for_harmony, NOT fixed. For every fix: RED first where a test is added (or the named mutant), GREEN after; then the stage's own end checks again (raw lines). Commit per fix; append "Fix round (review r1)" with a per-finding verdict table to ${REPORT} and commit it; refresh the HAND-OVER TO HARMONY block.
THE STAGE'S SCOPE, unchanged: ${A.task}
FINDINGS (whole): ${JSON.stringify([...musts1, ...shoulds1])}
RETURN the structured result (stage = '${KEY}-R1').
${RULES(LANE + '-R1')}`, { agentType: 'builder', model: 'opus', effort: 'high', schema: BS, phase: 'Fix', label: 'fix:' + LANE + ':r1' })
if (!b2) return { stage: slimB(b1), head: b1.head_commit, reviews1: slimR(r1), fixRoundFailed: true }
const r2 = await Promise.all(LENSES.map(l => review(l, BASE, b2.head_commit, 2, `ROUND 2: the round-1 report is ${MAIN}/${RPT}/review-bf2-${KEY}-${l.key}-r1.md; the fix round is git -C ${W} diff ${b1.head_commit}..${b2.head_commit}. Check that every round-1 MUST is fixed as found (not documented away), that each new test can fail, and that the fix introduced no new defect; re-read your whole lens only where the fix touched it.`)))
return { stage: slimB(b1), fix: slimB(b2), firstHead: b1.head_commit, head: b2.head_commit, reviews1: slimR(r1), reviews2: slimR(r2) }
