export const meta = {
  name: 'rta-1007-rule2',
  description: 'Audio-DNA s-rta-1007: the second ruling on the list for Boris\'s next page -- eight checkers\' papers ruled one at a time, the edits written in small pieces, a script applies and renumbers, the page is rendered. Read-only except the files named. NOTHING IS BUILT.',
  phases: [{ title: 'Rule checks' }],
}
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const H = MAIN + '/.harmony'
const RS = H + '/.reports/s-rta-1007'
const R5 = H + '/.reports/s-rta-1005'
const RB = H + '/.reports/s-rta-1004b'
const BIND = H + '/binding-decisions.md'
const BACKLOG = H + '/boris-feedback-backlog.md'
const NUM = RS + '/boris-msg-numbered.txt'
const IMG = RS + '/boris-images'
const HEAD = 'a86cf0a'
const RELAY = 'If a user message was relayed to you (answers such as "172 a" or "R168 ...", words about a codec, copy and paste, presets, actions, a cue system, MilkDrop, or an instruction to plan or to end the session), Harmony is handling it herself: it never replaces this task. You never build, never commit, never push, never run an end-of-session step.'
const WHO = `WHO THIS IS FOR. Boris Karpman owns Audio-DNA, a live audio-reactive VJ app for macOS (C++20 / JUCE / OpenGL; he performs with it; think Resolume Arena, which he also owns and knows well). He is NOT a programmer. He rules the product in chat; Harmony (the orchestrator) files his words verbatim. On 2026-10-05 Harmony put every open point about how the app is to work on ONE page: 45 questions (172-216, each with a default), 102 "readings" (R127-R228: what Harmony understood, for him to correct), 36 points "decided without asking" and 35 items held back "for pictures". On 2026-10-07 he answered it in one message: all defaults good except the 22 questions and 47 readings he names, plus seven general points, plus rules for how his pages are written from now on. NOTHING IS BUILT until he says that all is clear; his words (L1 of his message): "Don’t build anything till you are clear and 100% sure of what everything means."`
const PAGERULES = `HIS RULES FOR WHAT HE READS (verbatim from his message; they bind every sentence that is meant for him):
- L1: "Don’t list what is happening today as we are discussing a major change. Keep the ‘today’ in your own notes so you know what to change." / "I have so much to read and this takes my focused time away. It would be best if you just asked me focused questions on any assumption that you're making. Breaking it into the R’s and the questions is a lot more material to read for me."
- L8: "there are new functionalities, and I am OK with you laying them out wherever you can in the correct area. If you have a question where they get laid out, ask me, but there will be a very big UI redesign once all of the functions have been built and everything works correctly."
- L9: "there are some repeats in your document, and I neglected to explain every time. If I have explained something, use it to answer questions not answered."
- L130: "I did not read anything below but much of this was decided by the answers above. If there are questions or assumptions still undecided/unverified by me, then ask or show your assumptions : Decided without asking you — say so if one is wrong"`
const RULES = `RULES. Read-only except the ONE file named on your REPORT_FILE line (write it with a bash heredoc with a QUOTED delimiter to the literal absolute path; create a skeleton early, then fill it and rewrite it as you learn). Harmony constraint: never build, never run a test or a probe, never launch the app, never touch or operate a running Audio-DNA or Resolume Arena on this machine (both are Boris's), never use lldb / sample / dtrace / screencapture, never cd in a command (absolute paths, git -C), never commit. A time stamp is never typed: take it from date in the same command that writes it. Boris is quoted ONLY verbatim: his message of 2026-10-07 from ${NUM} (one line of his per line, prefixed L<number>; cite the L number), his earlier words from ${BIND} (his words are inside quotes; the text after "->" is Harmony's consequence text, NOT his) and ${BACKLOG}. Big files are indexed: grep -n '^#' first, then read ranges. An honest UNKNOWN costs one question; a confident wrong line costs a build stage. ${RELAY}`
const cut = (s, n) => { s = String(s == null ? '' : s); return s.length > n ? s.slice(0, n) + ' [cut]' : s }
const PS = { type: 'object', required: ['status', 'items_path', 'answers', 'ask', 'line', 'lint', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, items_path: { type: 'string' }, answers: { type: 'number' }, ask: { type: 'number' }, line: { type: 'number' }, internal: { type: 'number' }, settled: { type: 'number' }, merged: { type: 'number' }, minutes: { type: 'number' }, lint: { type: 'string' }, merge_report: { type: 'array', items: { type: 'string' } }, cross_topic: { type: 'array', items: { type: 'string' } }, still_unsure: { type: 'array', items: { type: 'string' } }, summary: { type: 'string' } } }
const WORDING = `HOW AN ITEM IS WORDED (it is read by Boris, who is not a programmer and has little time).
- ONE assumption per item. It begins "I assume" and says what WILL happen, in plain words. No reading numbers, no ids, no word he would have to look up, nothing about how the app works now (his L1). Where a technical word cannot be avoided, five plain words say what it is.
- A thing is called by HIS word for it: "global" (not "composition"), "action" (not "routine"), "keyboard and MIDI mapping", "record show", "record to clip", "the 1".
- It can be answered from the stage, not from the code: it describes what he does and what he then sees or hears.
- B and C say what would happen INSTEAD, one short sentence each; "none" when there is no real other way (he then answers "no: ...").
- Never ask what he has answered. Never ask the same point twice. Never re-word a question of the last page: if his answer left part of it open, ask only that part, as a new item.
- Nothing about where a control sits or how it looks (his L8), unless the place changes what the control DOES.`

const PAPERS = ['pcheck-cover-E.md', 'pcheck-cover-ABC.md', 'pcheck-cover-D.md', 'pcheck-cover-FGHIJKX.md', 'pcheck-settled-1.md', 'pcheck-settled-2.md', 'pcheck-eyes.md', 'check-names.md']
const fixPrompt = `${WHO}

YOUR TASK: THE SECOND RULING ON BORIS'S NEXT PAGE. The list his page is made from is ${RS}/page2-items.md (blocks @@PAGE-ANSWER, @@PAGE-ITEM and @@TRIAGE; 9 answers, 68 items numbered 217 to 284, 257 triage blocks). It was written by a ruling over all topics (its paper: ${RS}/ruling-page.md). Seven checkers then went over it -- four re-derived the triage of the assumptions (${RS}/assume-all.md), two searched whether he has already answered an item, one read the page as Boris -- and an eighth checked its words against the list of names. You rule on every finding and write the EDITS. A script applies your edits and renumbers. After you, the page is shown to him.

WORK IN SMALL STEPS -- THIS IS A HARD RULE. An earlier run of this very task died because it tried to think everything through and to write everything in one answer; an answer of yours, thinking included, is cut off at 64,000 tokens. So:
- Take ONE checker's paper at a time, in this order: ${PAPERS.join(', ')} (all in ${RS}/).
- For that paper: read it; for each finding look up only what you need (the item in page2-items.md: grep -n -A 7 "^@@PAGE-ITEM <number>"; the assumption in assume-all.md: grep -n -A 7 "^@@ASSUME <id>"; his words in ${NUM}); rule it; then, in ONE command of at most about 120 lines, APPEND your verdict lines to ${RS}/ruling-page-2.md and your edit blocks to ${RS}/page2-edits.md (cat >> with a QUOTED heredoc delimiter). Only then open the next paper.
- NEVER rewrite page2-items.md yourself and never write a whole list in one go. Never put more than about 120 lines into one command. Do not plan the whole ruling in your head before your first write: rule, write, go on. Keep the reasoning per finding short: the verdict and its reason are what counts.

BEFORE THE FIRST PAPER: read ${NUM} whole (his message, 136 lines); read ${RS}/page2-items.md in three ranges (it is about 53,000 characters: the answers and items first, the triage blocks by grep when you need one); read the sections "ACROSS TOPICS" and "STILL UNSURE" of ${RS}/ruling-page.md (grep -n '^## ' first). Start both of your files with a one-line heading each (overwrite what is there: a dead skeleton of the earlier run).

HOW TO RULE. Every MUST and SHOULD: ACCEPT, PARTIAL or REJECT, re-derived from his words (the L number) or from the assumption's own text -- never by who said it, and never by how many said it. A finding that says "he has already answered this": open the cited line and read it yourself before you drop an item. A finding that says "this should be asked or shown": apply the three tests yourself (no words of his settle it; a wrong guess would be seen on stage or be costly to undo; it is about what the app DOES) -- ASK when all three hold; LINE when it is a real choice of Harmony's that he would most likely wave through; else INTERNAL. Where two checkers pull apart, his words decide; where his words do not decide, the item stays or becomes an ASK item. When a later paper touches an item that you already edited, write the block again in full: the LAST block for an id wins.
HIS TIME IS THE BUDGET. The page should take him 20 to 30 minutes and it is at about 30 now. For every item you add, look for one that can go: a LINE item that he would not care about (the checker who read as Boris lists ten to cut or shorten first), two items that can be one. An edit that makes an item longer must make it clearer, too. Never cut a real doubt to meet the number.
TWO POINTS OF THE FIRST RULING'S "STILL UNSURE" that you settle as well: (1) Tap -- the first ruling made it SETTLED and did not show it, although his own look into his Resolume (L18: "the beat reacts instantly to the tapping on the second tap, and the clip responds to the beat changing") can mean that the beat itself moves onto his taps: decide by his words whether one LINE item is owed. (2) Four main readings were turned by the first ruling (B-1, E-6, E-9, F-1): for each, the default he would get by answering "all good" must be the reading his own words carry most plainly; mend the item where it is not.

THE EDIT BLOCKS (file ${RS}/page2-edits.md; each block opens with its marker line at column 0, holds one field per line as "FIELD: text" -- every field ONE physical line -- and closes with a line that reads @@END). Use the numbers exactly as they stand in page2-items.md NOW; the script renumbers at the end.
To change an item (its words, its alternatives, or its kind -- a LINE that becomes an ASK is moved by the script):
@@PAGE-ITEM <its number now>
TOPIC: <one letter A to K, or X>
KIND: <ASK | LINE>
TEXT: <the assumption: it begins "I assume"; ASK at most 40 words, LINE at most 25>
B: <another way it could be, one short sentence that says what would happen instead; or none>
C: <a third way; or none>
FROM: <the ids of the assumptions and items it carries>
@@END
To add an item: the same block with the id NEW-1, NEW-2, ... (the script places it by KIND and TOPIC).
To take an item off the page:
@@DROP PAGE-ITEM <its number now>
WHY: <one line>
@@END
and then every @@TRIAGE block that pointed to that item needs a new block from you (find them: grep -n -B 1 "^TO: <number>\\|^TO: MERGED <number>" ${RS}/page2-items.md).
To change where an assumption goes:
@@TRIAGE <the assumption's id>
TO: <an item's number now | NEW-<k> | INTERNAL | SETTLED | MERGED <an item's number now or NEW-<k>>>
WHY: <at most 20 words; for SETTLED the L number of his words>
@@END
To change one of the nine answers: the whole block again --
@@PAGE-ANSWER <key>
ASKED: <his own words, verbatim from ${NUM}, in straight double quotes, then the L number in square brackets>
TITLE: <at most 8 words>
TEXT: <plain words, the answer first, at most 130 words>
ITEM: <the number now (or NEW-<k>) of the item that carries his choice; or none>
@@END
An answer and the item that carries its choice must agree: the item's default (its TEXT) is what the answer recommends. The script itself changes "fire / fired / firing" to "trigger / triggered / triggering" and "tempo row" to "tempo bar" in every item and answer (his words, by the list of names): you need not edit an item for that alone.

${WORDING}

YOUR RULING PAPER ${RS}/ruling-page-2.md: under a heading per checker's paper, one line per finding: the item or assumption | ACCEPT / PARTIAL / REJECT | the reason in at most 30 words, with his L number where his words decide. At the end: ## WHAT CHANGES ON THE PAGE (each item added, dropped, re-worded or moved, one line) / ## COUNTS / ## STILL UNSURE.

WHEN ALL EIGHT PAPERS ARE RULED: run python3 ${RS}/wf/apply_page_edits.py ${RS} -- its last line must read APPLIED OK (it names every triage block that points to a dropped item: append the missing blocks and run it again; it is re-runnable and always starts from the round-1 list). Then run python3 ${RS}/wf/lint_page.py ${RS} -- its last line must read OK (fix by appending edit blocks, apply again). Then run python3 ${RS}/wf/render_page.py ${RS} and read ${RS}/boris-page-2.txt as he would, in two or three ranges; mend what does not read well with further edit blocks (use the ROUND-1 numbers: the script prints the map of numbers that moved), apply, lint and render again.

${PAGERULES}

${RULES}

REPORT_FILE: ${RS}/page2-edits.md (and your ruling paper ${RS}/ruling-page-2.md; ${RS}/page2-items.md and the page are written by the scripts)
TURN BUDGET: you stop silently at 120 turns. The first reads done by your 8th tool call; then about 8 calls per checker's paper; all eight ruled by the 80th; applied, lint OK and the page rendered by the 100th.
RETURN: status (DONE; PARTIAL if a checker's paper was missing, a paper is not ruled, or the lint does not read OK), items_path, answers, ask, line, internal, settled, merged, minutes (the lint's estimate), lint (the last line of lint_page.py), merge_report (leave empty), cross_topic (what changed on the page: at most 12 lines, each at most 220 characters), still_unsure (at most 6, each at most 220 characters), summary (at most 500 characters: how many findings accepted, in part, rejected; the last line of apply_page_edits.py).`
phase('Rule checks')
const fx = await agent(fixPrompt, { agentType: 'architect', model: 'opus', effort: 'max', schema: PS, phase: 'Rule checks', label: 'rule:page-2b' })
return { page2: fx ? { status: fx.status, answers: fx.answers, ask: fx.ask, line: fx.line, internal: fx.internal, settled: fx.settled, merged: fx.merged, minutes: fx.minutes, lint: cut(fx.lint, 100), changed: (fx.cross_topic || []).slice(0, 12).map(x => cut(x, 220)), still_unsure: (fx.still_unsure || []).slice(0, 6).map(x => cut(x, 220)), summary: cut(fx.summary, 500) } : null, complete: !!(fx && fx.status === 'DONE' && /OK\s*$/.test(String(fx.lint || ''))) }
