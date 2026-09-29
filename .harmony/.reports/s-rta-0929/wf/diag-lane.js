export const meta = {
  name: 'rta-0929-diag-lane',
  description: 'Audio-DNA s-rta-0929: one instrumentation-based diagnosis lane in its worktree (opus builder, temporary instrumentation, live runs under the shared lock), then an independent method audit',
  phases: [{ title: 'Diagnose' }, { title: 'Audit' }],
}
// args: { lane, wt, packet }
const A = args
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const MAIN_APP = MAIN + '/build/AudioDNA_artefacts/Release/Audio-DNA.app'
const RPT = '.harmony/.reports/s-rta-0929'
const REPORT = `${MAIN}/${RPT}/${A.lane}.md`
const SP = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad'
const W = MAIN + '/.claude/worktrees/' + A.wt
const L = SP + '/' + A.lane
const D = MAIN + '/build/_deps'

const RULES = `
RIG RULES (binding; breaking any one = lane FAIL):
- NOT an isolated worktree. Work ONLY inside W=${W} with absolute paths (git -C ${W} ...; cmake -S ${W} -B ${W}/build-lane; cmake --build ${W}/build-lane --target AudioDNA -j3). Configure: -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DFETCHCONTENT_SOURCE_DIR_JUCE=${D}/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=${D}/httplib-src -DFETCHCONTENT_SOURCE_DIR_CATCH2=${D}/catch2-src -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=${D}/melatonin_inspector-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=${D}/syphon-src (read-only use; a lane that must instrument JUCE copies juce-src into its scratch dir and points the JUCE source dir there). NEVER edit the main checkout ${MAIN} or its build/ — ONE exception: your report file ${REPORT} (the MINIMAL write fence allows .harmony/.reports/; scratchpad report.md writes are refused). Read-only use of ${MAIN_APP} = the unmodified app is allowed. NEVER cd in a command; multi-step sequences go in script files under ${L}/ run with bash <file>. zsh does not split a command held in a variable. Stamp times from date, never estimate.
- Python: use ${MAIN}/.venv/bin/python directly (never create a .venv symlink in the worktree).
- LIVE APP: at most ONE Audio-DNA on the machine (other lanes share it). Use the helper: put LANE=${A.lane} on its own line, then . ${SP}/lib/lock.sh (it pins LOCK_LANE by a real assignment; functions: acquire_quiet_lock (waits for no compiler BEFORE locking — use it for every perf batch) / acquire_lock / release_lock / start_app APP OUTDIR [test] / quit_app / outwins / wait_quiet / adna). A failed mkdir means WAIT (never open another copy: same bundle id reaches the running app); never remove another lane's lock; after releasing wait >= 45 s before re-acquiring (the helper enforces it). Hold the lock per launch or small batch (<= ~15 min), release between batches. Probes REFUSE without AUDIODNA_LOCK_OWNER matching (the helper exports it).
- Launch ONLY: open -g [--env V=v] [--stdout f --stderr f] <App> [--args --test-mode] (start_app does this). SCREEN-SAFETY LAW: NEVER open the Output window by ANY path (Output menu, Cmd+F, TopBar button, REST, deck/composition/settings file with an output) — no automated gate ever opens an output window; check outwins after every launch (must read Output-named 0). Quit via quit_app (osascript quit; kill only after 30 s). NEVER lldb/debugserver/gdb/dtrace/Instruments/sample/spindump on ANY binary. NEVER full-screen screencapture (window-only by Quartz window id via pyobjc with the main .venv python). NO synthetic input (UI states via REST / composition files / a TEMPORARY env-var hook). A probe that feeds broken media can CRASH the app -> a macOS "quit unexpectedly" dialog on Boris's screen: after any such batch count on-screen UserNotificationCenter windows (Quartz) == 0, and STOP and report any unexpected system dialog. NEVER write persistent system or user defaults (no defaults write). Do not change the display's refresh mode or any system setting.
- Perf numbers only when no compiler runs (acquire_quiet_lock) with the load average printed at start and end of each launch; other lanes may be compiling — wait, do not measure through it. A verdict needs >= 5 launches per arm (more where the packet says). Probe HTTP clients use a fresh connection per request (Connection: close).
- DIAGNOSIS ONLY: no product change is committed. All instrumentation is TEMPORARY: before reverting, save it as ${L}/instr.diff (git -C ${W} diff, plus new files) and keep your drivers/analyzers under ${L}/tools/ with paths fixed; revert with git -C ${W} apply -R of your own diff and delete the files you added (NEVER git checkout -- . / git stash / git reset --hard: the stash-guard hook blocks them); rebuild build-lane from the clean tree; strings check (strings <binary> | grep -c <your markers>) must be 0. End state: git -C ${W} status clean except build-lane/ (KEEP build-lane), lock released, no app you launched running, 0 Output-named windows, 0 UserNotificationCenter windows, any fixture files you put under ~/Documents or ~/Library moved to ${L}/ (say which).
- Label every claim VERIFIED (measured / read in source, cite the evidence file or file:line) / INFERRED / ASSUMED. A cause without a counterfactual run is INFERRED.
- Report: write ${REPORT} (BUILDER REPORT shape: STATUS, RESULT, FACTS, METHOD, CONFIDENCE+VERIFY, UNKNOWNS, then the deliverable sections, found_not_fixed, notebook lines for Harmony, PACKET QUALITY). Evidence under ${L}/runs/<tag>/; copy the tools + the aggregates the report cites (not raw runs) to ${MAIN}/${RPT}/${A.lane}-tools/.`

const DS = { type: 'object', properties: {
  lane: { type: 'string' }, status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, worktree_path: { type: 'string' }, base_commit: { type: 'string' },
  headline: { type: 'string' },
  causes: { type: 'array', items: { type: 'object', properties: { id: { type: 'string' }, site: { type: 'string' }, thread: { type: 'string' }, trigger_frequency: { type: 'string' }, cost_ms: { type: 'string' }, share_or_rank: { type: 'string' }, proof: { type: 'string' }, label: { type: 'string', enum: ['VERIFIED', 'INFERRED', 'ASSUMED'] } }, required: ['id', 'site', 'cost_ms', 'proof', 'label'] } },
  unattributed: { type: 'string' }, fix_options: { type: 'string' }, gates: { type: 'string' },
  report_path: { type: 'string' }, tools_dir: { type: 'string' },
  lock_released: { type: 'boolean' }, app_left_running: { type: 'boolean' }, output_window_opened: { type: 'boolean' }, unc_windows_zero: { type: 'boolean' }, tree_clean: { type: 'boolean' }, instrumentation_strings_zero: { type: 'boolean' },
  errors_or_deviations: { type: 'string' } },
  required: ['lane', 'status', 'headline', 'causes', 'report_path', 'lock_released', 'app_left_running', 'output_window_opened', 'tree_clean', 'instrumentation_strings_zero'] }

phase('Diagnose')
const d = await agent(`${A.packet}\n${RULES}`, { agentType: 'builder', model: 'opus', effort: 'high', schema: DS, phase: 'Diagnose', label: 'diag:' + A.lane })
if (!d) return { diag: null }
log(`${A.lane}: ${d.status} — ${String(d.headline).slice(0, 200)}`)

phase('Audit')
const AS = { type: 'object', properties: { verdict: { type: 'string', enum: ['SOUND', 'SOUND_WITH_GAPS', 'UNSOUND'] }, findings: { type: 'array', items: { type: 'object', properties: { severity: { type: 'string', enum: ['MUST', 'SHOULD', 'NIT'] }, issue: { type: 'string' }, fix: { type: 'string' } }, required: ['severity', 'issue'] } } }, required: ['verdict', 'findings'] }
const audit = await agent(`INDEPENDENT METHOD AUDIT (read-only) of diagnosis lane ${A.lane}. Read the lane's report ${REPORT}, its evidence under ${L}/runs/, its tools under ${L}/tools/ and ${L}/instr.diff, and the source it cites (in ${W} or ${MAIN}; the tree is at main). Do NOT re-run the app. Judge: is each claimed cause PROVEN (a counterfactual with the required launches per arm, numbers that move as predicted) or only inferred; could the instrumentation itself have created or hidden the effect (observer cost, log I/O on a hot thread, timer-query stalls); is the arithmetic right (shares sum, per-event x rate); does any evidence file contradict the report; is anything left unattributed that the report calls attributed; are the RED-able gates it proposes real (would they read RED on current main?). Write your paper via Bash heredoc. REPORT_FILE: ${RPT}/audit-${A.lane}.md`, { agentType: 'reviewer', model: 'sonnet', effort: 'high', schema: AS, phase: 'Audit', label: 'audit:' + A.lane })
return { diag: d, audit }
