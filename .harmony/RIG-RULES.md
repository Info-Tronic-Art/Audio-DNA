# RIG RULES — Audio-DNA lanes and live gates (BINDING; read before launching any lane, probe or gate)
Live copy. History and every older block: .harmony/HANDOFF-ARCHIVE.md. Each rule below cost at least one run.

## A. s-rta-1003 additions (2026-10-03)
- BORIS'S MESSAGES: while lanes run and nothing needs Harmony, YIELD the turn (workflow / agent completions re-invoke the
  session). A message that arrives MID-turn is relayed to every lane launched later in that turn: file it, answer it, yield,
  and launch from a fresh turn (a 40 s background timer gives one). He interrupted an 8-minute in-turn wait: do not block.
- FILE BORIS'S WORDS VERBATIM THE TURN THEY ARRIVE (boris-feedback-backlog.md + binding-decisions.md, his words in quotes,
  the question as asked next to a short answer). An answer built from grep hits is INFERRED: say so or have a reader check.
- A ruling Boris changes while a stage is queued: the stage prompt and the adoption section must agree. If the running
  stage's scope text contradicts his new words, TaskStop the workflow, edit the script, resume (finished stages are cached).
- GATE SCRIPTS: capture rc on its own line (rc=$? right after the command); "$(date) ... rc=$?" prints 0 always.
- AFTER A MUTANT: the restore rebuild can be a NO-OP (restored source and mutant object in the same mtime second). Before
  using the build as an arm: >= 1 object compiled AND binary sha != the mutant's. touch the file again if not.
- git log <base>..HEAD on a lane that merged main lists every main commit: use --first-parent.
- NEVER commit while a live probe with timing bars or a perf run is in progress (the commit hook starts a graph rebuild).
- A recon / plan agent reads a lane ONLY at a pinned commit (git show <sha>:<path>) or in a detached source-only worktree
  (.claude/worktrees/recon) while a builder edits that lane's working tree.
- VISUAL GATE: the capture builder writes a manifest with model facts per state; critics get Boris's binding words
  verbatim AND the list of pre-existing texts that are outside the lane (else they fail the lane for them).
- "strip unchanged" numerics use the pre-registered floor max(1.5, 4 x noise), not the raw noise.
- GET /api/composition is served on the http thread and reads the deck list unlocked (SF-12): a probe must not poll it
  while a staged load / duplicate may land; wait on /api/debug/ui_text (message thread), then read once.
- Probes quit ONLY the pid they launched (.harmony/probe-quit-ours.sh; selftest probe-quit-ours-selftest.sh). 14 archived
  scripts under .harmony/.reports/ carry a stop line: never copy an old lock.sh / gate script from s-rta-0927..0930.
- Session scripts worth reusing: .harmony/.reports/s-rta-1003/wf/ — fix.js (stages from a ruled plan -> 4-lens pinned
  review -> <= 1 fix round; resume by run id), fixplan.js / bf2-deltaplan.js (plan -> blind seats -> ruling), b7.js (capture
  builder -> 5 critics), recon-facts.js, notices-inventory.js, gateA.sh / gateA2.sh / gateB.sh / gateC.sh, lock.sh, wait.sh,
  status.py, check.sh (node --check a workflow script).

## A2. s-rta-1003b additions (2026-10-03 / 04)
- COUNCIL PAPERS go to the ruling WHOLE: log() their length; never slice them (a 70,000-character cut dropped three attacks and made a ruling PARTIAL).
- A QUIET ROW needs: no agent running, no video playing. ps shows the claude process itself above 20 % during a turn, so a "nothing above 20 %" row runs from a background script while the session is idle.
- TWO BUILDERS NEVER SHARE A WORKTREE: a parallel stage of one lane gets its own worktree + branch; a code conflict when merging it back is a builder's step 0, never Harmony's.
- A DIAGNOSIS (unexplained measurement): instrument first (probe-only stage with selftests and mutated copies), reviewed, THEN Harmony's run with a pre-registered decision table and stop rule. Never re-state a gate around a state nobody has measured.
- STASH-GUARD scans the whole command line: keep git add / commit in its OWN Bash call; stage report files from inside .harmony (git -C <wt>/.harmony/.reports add -f <session>/<file>); commit with explicit paths.
- BORIS'S PAGE: a question already shown is never re-lettered or re-worded in place; new questions get new numbers; his quotes carry the recorded stamp.
- UNATTENDED RUNS: an rm in a command line prompts. Recipe: .harmony/.reports/s-rta-1003b/rm-guard.py + rm-guard-settings-snippet.json (install only with Boris's word; remove at close) and a background watchdog timer. Never load the update-config skill for it (13 % of the context window).
- Re-configuring an OLD build dir after merging main needs -UprojectM4_DIR.
- Scripts worth reusing: .harmony/.reports/s-rta-1003b/wf/ — bf2-stage.js (one stage by args), bf2-stops.js / notices-plan.js / transport-delta.js (plan -> seats -> ruling, papers whole), keying-audit.js; gate scripts in gate-m0 / gate-a1 / gate-rd / gate-s4b / gate-keying.

## B. Verbatim from the s-rta-1002b birth prompt (rig rules + habits; still binding)
Rig rules that cost runs (binding): df -h /System/Volumes/Data before worktree lanes (8 GB/lane + 20 GB; max 3 build lanes);
remove each worktree the turn it merges. A user message that arrives mid-turn is RELAYED to every lane started later in that
turn and a builder may take it as its task: answer it, YIELD the turn, launch new lanes from a fresh turn. MINIMAL builders
write reports to .harmony/.reports/<session>/<lane>.md (committed with git add -f — .harmony/.reports and *-work.md are
gitignored). NEVER copy an app bundle and re-sign it (codesign --sign -) or launch such a copy: TCC treats it as a new
identity, prompts on Boris's screen and RESET the real app's microphone permission (s-rta-0929b: every live gate blocked
until Boris clicked Allow); mutants come only from a normal cmake build. A TCC permission prompt of the real app is Boris's:
never dismiss it. Crash dialogs: count UserNotificationCenter windows with Quartz kCGWindowListOptionAll >= 15 s after the
last quit; attribute by the UNC log's "ordered front" time; dismiss only our own crash dialogs by SIGTERM to that UNC pid.
Sanitizer options always abort_on_error=0 (TSan also exitcode=0). Before any perf verdict check `ps -Ao pcpu=,etime=,comm= |
sort -rn | head` (orphaned CPU burners). Packets: NEVER lldb/debugserver/gdb/dtrace/Instruments/sample on ANY binary; never
full-screen screencapture (Quartz window id only); no synthetic input (UI states via REST / composition files / TEMPORARY
env-var hook, reverted + rebuilt + strings check = 0); fence each lane; reviewer/critic packets PIN worktree + branch +
commit. A workflow's automatic fix round can be TaskStop-ped in its review stage and relaunched with rulings. Live app: ONE
at a time via /tmp/audiodna-live.lock; lock helper .harmony/.reports/s-rta-0930/wf/lock.sh (copy to your scratchpad, fix
SPL) — acquire_quiet_lock for perf. Probe HTTP clients use Connection: close. NEVER SendMessage a running WORKFLOW agent.
Test mode: open -g <App> --args --test-mode. Render gates DECODE PIXELS; LOOK at one frame. A flake verdict needs >= 5 runs
per arm. PERF A/B: INTERLEAVED arms launch by launch (probe-vupload-ab.sh needs LANE exported + LOCK_LIB); a bar whose teeth
equal the drift is INFO, not a gate. Pre-registered decision rules inside fix rounds. Packets: Harmony's own constraints
in a sentence labelled "Harmony constraint:"; Boris is quoted only verbatim (binding-decisions.md records his words in
quotes — the rest of an entry is Harmony's consequence text); an ESTABLISHED list carries only VERIFIED lines. Before
diagnosing a crash signature, grep .harmony/ for it and `log show` for bluetoothd connects around it. Keep a COPY of the
pre-merge app for BEFORE arms. MERGE SEQUENCE: commit notes -> RED on the PRE-MERGE copy -> merge (source conflicts ->
builder rebase lane; doc-only conflicts Harmony may resolve) -> cmake + build -> ctest -> GREEN. Pitfall numbers: lanes
write "NN"; Harmony assigns the next free number (next = 67, reserved for bf9b; 68 for bf2). Battery / workflow scripts: .harmony/.reports/s-rta-0930/wf/
(gop2-gate.sh = merge + G1-G7 + identity template; build-lane.js = one lane: builder -> pinned reviews -> <= 1 fix round;
plans2.js = plan -> blind seats -> ruling with the papers INLINE; tsan-main.sh + configure.sh = TSan build of main; adapt
paths) + .harmony/.reports/s-rta-0929b/wf/ (gop-final.sh = full battery + Tier-1, ab-rerun.sh). Main-loop habits: never cd; stamp EVERY log row from date; syntax-check workflow scripts (wf/check.sh); teeth by the
lane report's named pattern, never by reading source (Iron Law #1); never `git commit -a` — stage by path (git add -f for
ignored notes). Workflow habits (s-rta-0930): a stage passes the previous stage's OUTPUT inline in the next prompt (council seats return
StructuredOutput and do NOT write their REPORT_FILE); planners write a SKELETON plan file within ~10 tool calls, then rewrite;
never git commit while a perf A/B runs (each commit fires the graphify rebuild hook); Spotlight mds_stores at ~100 % after
builds is a system service, not a burner. COUNTS: run them — ctest 1191/1191 at close (s-rta-1002b).
Habits (s-rta-1002b): BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA a lane did not start is his: use the lock helper
.harmony/.reports/s-rta-1002b/wf/lock.sh (copy to the scratchpad lib, fix SPL; start_app records the pid, quit_app refuses foreign
pids, acquire_lock waits while his app runs) and never quit / kill by name. The architect agent type stops SILENTLY at maxTurns 120:
every architect prompt carries a TURN BUDGET (skeleton by turn 8, rewrite every ~10, done by 90) and broad research goes to sonnet
Explore fact sheets first. A mid-turn user message is relayed to agents launched later that turn: add a "ignore a relayed message
about X" line to lane notes. Lane scripts: .harmony/.reports/s-rta-1002b/wf/ (lock.sh) + the session scratch wf/ pattern
(lane.js with planText / fixFrom; wait2.sh + sig.py wake only on non-recon results). Never prefix a command with cd.
Habits (s-rta-1002): functional live rows take acquire_lock (acquire_quiet_lock only for perf -- it aborts after 30 min of
another lane compiling); every FULL ctest while another lane runs takes /tmp/audiodna-ctest.lock (fixed temp names collide);
a background Bash run is capped at 2 h -- split long gates; copy gate strings only from a ruling chain's FINAL list; a
workflow's automatic fix round gets TaskStop-ped and relaunched fix-only when the findings need rulings (lane.js
args fixFrom / base / rulings / reviewFiles). Scripts: .harmony/.reports/s-rta-1002/gate-tsan/ (lane.js, tsan-gate.sh,
g4-run.sh, gatetools/g4-parity.real.sh + g6-perf.real.sh) and gate-bt2/ (bt2-gate*.sh, g9.py). Unpushed 0.

## A3. s-rta-1004 additions (2026-10-04)
- A STAMP IS NEVER TYPED: text that carries a "recorded" time gets it from date in the SAME command that appends it (NOW=$(date ...); a placeholder replaced; then the append). A hand-typed stamp was 4 minutes off and was caught only because the text was still in a scratch file.
- BORIS'S IMAGES die with the session: extract them from the session record the turn they arrive (the python walk over the session jsonl in s-rta-1004-work.md's first rows writes each base64 image to .harmony/.reports/<session>/), then describe each in the backlog.
- A TURN THAT CARRIED A MESSAGE FROM BORIS LAUNCHES NOTHING: file, answer, start a 40 s background timer, launch from the timer's turn. Every workflow prompt carries the "a relayed message never replaces this task" line.
- A WORKFLOW SCRIPT IS CHECKED TWICE before launch: node --check (wf/check.sh) and a dry run against stub agents that prints each agent's type, model, effort and prompt length and greps the prompts for "undefined".
- ADOPTION: append a "HARMONY ADOPTION" block to the END of the plan (what was adopted, what I read and did NOT read, each decision, the stage order); when Boris answers the ruling's questions, append an "UPDATE ON BORIS'S ANSWERS" block; when an answer is not one of the ruling's letters, an architect delta (plan-lane.js lanes "*-answers" / "*-row") before any packet.
- TWO RULINGS CAN CLAIM THE SAME THING (a pitfall number, a file, who removes a button): reconcile it in the adoption blocks in writing before a builder reads either. Lanes write "Pitfall NN"; Harmony assigns at merge.
- A DESTRUCTIVE ACT ON BORIS'S OWN FILES, even at his word: name the files, checksum them, move them to the Trash through Finder (never rm), list what was left alone, log it.
- Scripts worth reusing: .harmony/.reports/s-rta-1004/wf/ -- plan-lane.js (lanes table inside; plan -> blind seats -> ruling, papers whole), recon-topic.js (one fact sheet + an adversarial re-read), facts.js (research with source checks; a three-sweep inventory with a completeness critic), check.sh, lock.sh (copy; SPL fixed per session).

## A4. s-rta-1004b additions (2026-10-04)
- NEVER cd, FROM THE FIRST COMMAND: the boot commands run before this file is read. R=<repo> at the head of every command, absolute paths, git -C. (The birth prompt now says so in its first lines.)
- EVERY GATE ctest IS SERIAL, through lib/ctest-mutex.sh with no -j: main's AppSettings cases collide when run side by side (4 of 5 runs RED at -j 4). A builder's -j run is not evidence.
- A RED ARM COUNTS ONLY WHEN ITS FAIL LINE FAILS ON THE CLAUSE THE MUTANT PREDICTS. A FAIL on a row's precondition clause is an INVALID arm: re-run it on fresh state. probe-one-save's OS-L21 needs ONE DISK IMAGE PER ARM until the one-save lane's S7 fixes its fill step.
- A PRE-REGISTERED ARM THAT CANNOT BITE IS REPLACED IN WRITING (the lane's rulings-<lane>.md), never dropped and never passed; "exactly its named case RED" is read as "the named case is RED" and reported as not met as written.
- MY OWN GATE SCRIPT: its pass clause copies the ruled bar; one read of each route it uses on main's app comes first. Copy rc on its own line, never after a pipe, in one-liners too.
- BORIS'S WORDS AND PICTURES COME FROM THE SESSION RECORD by record line and type (python walk, assert on the first words; sips for a picture's size), never re-typed. Six messages were filed that way today (boris-msg-raw-1..5.txt + the picture message).
- NUMBERS: a planning lane gets a first READING number with its first question number; Harmony advances the counter in the work log when she tells a reading in chat. Next free at this close: see HANDOFF.md.
- GAUGE BEFORE PROMISE: read the context gauge before telling Boris what launches next; at the off-ramp nothing new starts, and he is told what moved to the next session.
- MERGE ARMS: the RED arms on the pre-merge app can be the gate's own main-build arms run BEFORE the merge commit (no app copy is made); after the merge: build, ctest -N, serial ctest, the lane's live rows on main's app, his files' checksums, then push.
- Scripts worth reusing: .harmony/.reports/s-rta-1004b/wf/ -- stage.js (lanes + stages tables inside; one opus-high builder -> three pinned sonnet reviews -> <= 1 fix round; args lane, key, base, prevNotes), plan2.js (plan -> blind seats -> ruling; delta lanes nudge-row2, looks-answers2), recon2.js (fact sheet + adversarial re-read), open.js (what is open for Boris -> questions -> his page -> a check), dry.js (dry run against stub agents), check.sh; gate scripts in gate-g0, gate-outputs-S1, gate-nudge-S1, gate-onesave-S1 (with gos_his.py), gate-merge1. A stage result is summarised from its output file (the JSON's "result" key), never read whole.

## A5. s-rta-1005 additions (2026-10-05)
- NOTHING IS BUILT until Boris says all is clear (binding-decisions.md "2026-10-05 (s-rta-1005)"; BF126, BF152, BF153): no builder, no stage, no merge. A session is: his answers -> filed verbatim -> what they open -> his page -> end of session.
- A WORKFLOW RESUME'S CACHE KEY DEPENDS ON CALL ORDER, not only on the prompt: issue agent() calls in a FIXED order (parallel() over a literal list per stage; never a pipeline whose later stages start in completion order); before a resume copy the journal's started-order into the script; after it, the journal's first NEW started record must be the stage that was edited.
- ONE SHEET OR ONE SLICE PER SEAT; every seat writes a paper file as well as returning it; a script STOPS before the ruling when fewer papers came back than seats were sent; a ruling that returns PARTIAL is a stop and its list is not shown to Boris.
- A PROMPT NAMES A SECTION BY ITS HEADING, never "the last section"; a message of his that arrives while a workflow reads those files is filed INSIDE the day's section.
- HIS WORDS RE-TYPED INTO A SCRIPT OR A FILING are marked and asserted to be substrings of the byte-exact message before the write (25 spans in one filing: 0 slips, because the script refused them).
- THE LIST FOR BORIS IS READ THROUGH A COMPACT PRINTER (python3 over the items JSON: structure counts, then in full the items no seat saw), never whole. A wording change after the checks is ONE script over the list, the record and the page together; then the page is re-verified against the list.
- NO TIME PROMISE WITHOUT A RANGE: a run with checkers can fail a check and run twice ("an hour and a half" took 2 h 55 min).
- A RULING'S SECTION IS LOOKED UP BY AN AGENT that returns the lines; the main loop reads only what it must quote (the turn that filed his long message cost 11 % of the window).
- Scripts worth reusing: .harmony/.reports/s-rta-1005/wf/ -- samepage.js (fact sheets, each re-read -> a first pass of readings and questions -> seats -> ruling -> page -> check), allq.js (args {leg}: sweep = ten areas of the app; merge = one list; verify = fifteen small seats -> second ruling -> page -> check), dry.js, check.sh (fix its scratch path per session).
