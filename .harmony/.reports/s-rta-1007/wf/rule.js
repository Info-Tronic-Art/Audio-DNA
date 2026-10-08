export const meta = {
  name: 'rta-1007-rule',
  description: 'Audio-DNA s-rta-1007: rule on the twelve topic papers (Boris\'s answers of 2026-10-07 applied), make the topics agree, triage every assumption that is left, write the list for his next page and render it, check the page seven ways, rule on the checks, and write the list of what everything is called. Read-only except the files named. NOTHING IS BUILT.',
  phases: [{ title: 'Rule topics' }, { title: 'Rule page' }, { title: 'Check page' }, { title: 'Rule checks' }],
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
const CODE = `THE APP. The repository's main working tree is ${MAIN} (HEAD ${HEAD}; no builder is editing it: the build is on hold). Do NOT read the lane worktrees under ${MAIN}/.claude/worktrees. Project docs that index the code: ${MAIN}/CLAUDE.md (the pitfall index), ${MAIN}/docs/claude/*.md, ${H}/APP-INVENTORY.md.`
const FORMAT = `THE PAPER'S FORMAT (a script parses it: keep it exact). Plain markdown headings, and under them BLOCKS. A block opens with a marker line at column 0, holds one field per line as "FIELD: text" -- every field is ONE physical line, however long, with no line break inside it -- and closes with a line that reads @@END.

# APPLY <letter> -- <topic name> (s-rta-1007)
## SUMMARY
(at most 12 lines: what his words changed most in this topic)
## ITEMS
One block for EVERY id listed under "ITEM IDS OF THIS SLICE" in your slice, in the slice's order:
@@ITEM <id>
TITLE: <at most 12 words>
STATUS: <one of ANSWERED DEFAULT STANDS CORRECTED REPLACED SETTLED DROPPED OPEN> <then, optionally, a few words>
HIS: <the L numbers of his lines that decide it, for example "L11, L31"; or "none">
RULE: <how it works once built, after his words: plain words, complete enough to build from, as long as it needs; it never describes the app as it is now>
CHANGED: <which parts of the page's text (by their letters) his words replace, add to or remove, and what the page said there; or "nothing">
TODAY: <Harmony's own notes: what the app does now and what has to change -- cite the fact sheet's point id, or file:line you read yourself; or "not checked">
@@END
## ASSUMPTIONS
@@ASSUME <letter>-<running number>
ABOUT: <the item ids it belongs to>
TEXT: <the assumption as Boris will read it: one or two short sentences that begin "I assume", plain words, what WILL happen, at most 40 words; no reading numbers (no "R168"), no jargon, nothing about how the app is now>
WHY: <what in his words leaves this open, at most 30 words; cite L numbers>
ALT: <the other way(s) it could be meant, one short sentence each, as "b) ... c) ..."; or "none">
IF-WRONG: <STAGE = he or the audience would see it | REBUILD = costly to undo once built | SMALL> <then a few words>
ASK: <YES | LINE | NO> <then a few words why>
@@END
## QUESTIONS BACK
(only if this task names a question of his to answer)
@@ANSWER <key>
HIS: <L numbers>
ANSWER: <for Boris: plain words, the recommendation first, at most 130 words; nothing about how the app is now unless that is what he asked>
@@END
## NAMES
One block for every name that his words fix in this topic, and every name you had to pick for a new thing:
@@NAME <the name as it will read on screen and in talk>
MEANS: <what the thing is, one sentence>
SOURCE: <his words L.. (or an earlier line of binding-decisions.md) | Harmony's pick | the on-screen name now, which is replaced by ...>
@@END
## CONFLICTS
(his words against his words, or against an adopted ruling: one bullet each, both sides quoted with their L number or file line)
## NOT DONE / UNSURE
(what you could not settle, and the cheapest way to settle it)`
const DISCIPLINE = `HOW TO APPLY (the discipline; every status must be earned).
- His words win over the page's text, always. Quote him verbatim with the L number.
- A QUESTION he answered with a letter: STATUS ANSWERED; RULE restates that option in full, plus whatever words he added.
- A QUESTION he answered in words that fit no letter: STATUS ANSWERED; RULE = what his words say. If they leave part of the question open, that part becomes an @@ASSUME.
- A QUESTION marked "NOT NAMED by him": STATUS DEFAULT (his L1). RULE restates the default option in full. But test it against his WHOLE message: where words of his elsewhere say otherwise, STATUS REPLACED, cite the line, RULE = his words.
- A READING he named: he read it and corrected what was wrong. Only what his words touch changes (STATUS CORRECTED; CHANGED says which lettered parts are replaced, added or removed); the rest of that reading stands. "all good" / "yes" / "good" = STANDS for what he does not go on to change. Where his line opens with a letter ("R195 d ...", "R186 D) ...", "R206 A ..."), it is about that lettered part of the reading.
- A READING marked "NOT NAMED by him": STATUS STANDS (inferred consent: he went through the page in its order and corrected what was wrong). Test it against his whole message too: STATUS REPLACED where his words elsewhere say otherwise (his L9).
- D items (decided without asking) and P items (held for pictures): he did NOT read them (L130). Test each against all his words. SETTLED = his words now say the same (cite). REPLACED = his words say otherwise (cite; RULE = the new rule). Otherwise it is still Harmony's own: STATUS OPEN plus an @@ASSUME. For P items his L8 applies: Harmony lays the thing out where it fits in the correct area. A P item whose variants differ only in how it LOOKS or where it SITS gets STATUS DROPPED with RULE "laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8)". A P item whose variants differ in what the app DOES is a function question: STATUS OPEN plus an @@ASSUME.
- G items (general points of his message): STATUS ANSWERED; RULE = the function his words describe, complete enough to build; every edge his words leave open becomes an @@ASSUME.
- NEVER FILL A GAP SILENTLY. Whenever you must decide something his words do not say in order to write a RULE that can be built, write the RULE with your best reading AND an @@ASSUME that names the gap. Where his words can be read two ways, both readings go into TEXT and ALT. An assumption written into a RULE as if it were his word is the worst defect this paper can have.
- Typing slips are read kindly and flagged ("Lipp's" = clip's, "sell" = cell): say INFERRED in CHANGED.
- Where two statements of his pull apart (inside this message, or this message against his earlier words in ${BIND}), do not pick silently: one bullet under CONFLICTS and an @@ASSUME with ASK: YES.
- The words "as in Resolume" in a rule of his mean: what HIS Resolume does. Under R206 (L15-L22, L29) he reports eight looks he took in his own Arena at Harmony's request; they are evidence of what Resolume does, not rules by themselves.
- RULE lines never describe the app as it is now (his L1). What the app does now goes ONLY into the TODAY line: from the fact sheets (cite the point id), from the source when the sheets do not cover it (file:line), or "not checked". Do not spend your turns re-deriving "today": the job is his words.
- ASK on an @@ASSUME. YES only if all three hold: (1) no words of his, in any message, settle it; (2) a wrong guess would be seen on stage by him or the audience, or would cost more than a small change to undo once built; (3) it is about what the app DOES, not about where a control sits or how it looks. LINE = a real choice of Harmony's that he would most likely wave through: he gets it as one line that he can strike. NO = technical or internal: he would not care. Be strict, because he has too much to read; but never hide a real doubt: a focused question costs him ten seconds, a wrong build costs a stage.`

const TOPICS = [['A', 'Firing clips and the tempo row'], ['B', 'The cue system'], ['C', 'Presets'], ['D', 'Actions in a show'], ['E', 'The review screen and recordings'], ['F', 'The show file, decks and saving'], ['G', 'Output screens'], ['H', 'How a clip plays'], ['I', 'Effects, signals and what moves a slider by itself'], ['J', 'The keyboard and MIDI mapping, menus and messages'], ['K', 'Sources and the automatic features'], ['X', 'the page\'s loose lists (statements that disagreed, things not established, things still unsure)']]
const BACK = {
  A: { key: '189', paper: 'answer-tempo-auto.md', check: 'check-tempo-auto.md', item: '189', what: 'his question back on question 189 (L32): can the app\'s listening be corrected by hand -- a tempo that is off by two BPM, and the place of the "1" -- and stay in automatic mode' },
  E: { key: 'lowres', paper: 'answer-lowres-rec.md', check: 'check-lowres.md', item: 'G4', what: 'his question (L6): how much of the computer a very low-resolution recording of the show in 10 or 20 minute chunks would take' },
  H: { key: 'codec', paper: 'answer-codec.md', check: 'check-codec.md', item: 'G1', what: 'his question (L3): is there any reason to build a codec of our own like Resolume\'s DXV 3, and can that be done reliably' },
}
const cut = (s, n) => { s = String(s == null ? '' : s); return s.length > n ? s.slice(0, n) + ' [cut]' : s }
const TS = { type: 'object', required: ['status', 'report_path', 'accepted', 'partial', 'rejected', 'lint', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, accepted: { type: 'number' }, partial: { type: 'number' }, rejected: { type: 'number' }, own_findings: { type: 'number' }, blocks_replaced: { type: 'number' }, assumptions_dropped: { type: 'number' }, assumptions_added: { type: 'number' }, lint: { type: 'string' }, for_page: { type: 'array', items: { type: 'string' } }, summary: { type: 'string' } } }
const PS = { type: 'object', required: ['status', 'items_path', 'answers', 'ask', 'line', 'lint', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, items_path: { type: 'string' }, answers: { type: 'number' }, ask: { type: 'number' }, line: { type: 'number' }, internal: { type: 'number' }, settled: { type: 'number' }, merged: { type: 'number' }, minutes: { type: 'number' }, lint: { type: 'string' }, merge_report: { type: 'array', items: { type: 'string' } }, cross_topic: { type: 'array', items: { type: 'string' } }, still_unsure: { type: 'array', items: { type: 'string' } }, summary: { type: 'string' } } }
const CS = { type: 'object', required: ['verdict', 'paper_path', 'checked', 'findings', 'summary'], properties: { verdict: { type: 'string', enum: ['SOUND', 'SOUND_WITH_CORRECTIONS', 'UNRELIABLE'] }, paper_path: { type: 'string' }, checked: { type: 'number' }, findings: { type: 'array', items: { type: 'object', required: ['severity', 'item', 'issue', 'fix'], properties: { severity: { type: 'string', enum: ['MUST', 'SHOULD'] }, item: { type: 'string' }, issue: { type: 'string' }, fix: { type: 'string' } } } }, summary: { type: 'string' } } }
const NS = { type: 'object', required: ['status', 'report_path', 'names', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, names: { type: 'number' }, his: { type: 'number' }, picks: { type: 'number' }, summary: { type: 'string' } } }

const PAGEFMT = `THE ITEMS FILE ${RS}/page2-items.md (a script parses it and makes his page from it: keep it exact). Three kinds of block; a block opens with its marker line at column 0, holds one field per line as "FIELD: text" -- every field ONE physical line -- and closes with a line that reads @@END.
@@PAGE-ANSWER <key>
ASKED: <his own words, verbatim from ${NUM}, in straight double quotes, then the L number in square brackets, for example "Do we need snapshot?" [L94]>
TITLE: <at most 8 words>
TEXT: <the answer for him: plain words, the answer first, at most 130 words>
ITEM: <the number of the page item that carries his choice, where the answer ends in a choice; else none>
@@END
@@PAGE-ITEM <number>
TOPIC: <one letter A to K, or X>
KIND: <ASK | LINE>
TEXT: <the assumption: it begins "I assume"; ASK at most 40 words, LINE at most 25>
B: <another way it could be, one short sentence that says what would happen instead; or none>
C: <a third way; or none>
FROM: <the ids of the assumptions and items it carries, for example "D-4, D-9, R172">
@@END
@@TRIAGE <the id of one @@ASSUME block of assume-all.md>
TO: <a page item number | INTERNAL | SETTLED | MERGED <page item number>>
WHY: <at most 20 words; for SETTLED the L number of his words>
@@END
The nine answers he is owed have exactly these keys: codec, lowres, 189, R129-4, R184, R188-snapshot, R221-b, review-name, 207-hold. ORDER AND NUMBERS of the page items: first every ASK item, topics A to K then X; then every LINE item, topics A to K then X; numbered from 217 without a gap in the order of the file. EVERY @@ASSUME block of assume-all.md gets exactly one @@TRIAGE block.`
const WORDING = `HOW AN ITEM IS WORDED (it is read by Boris, who is not a programmer and has little time).
- ONE assumption per item. It begins "I assume" and says what WILL happen, in plain words. No reading numbers, no ids, no word he would have to look up, nothing about how the app works now (his L1). Where a technical word cannot be avoided, five plain words say what it is.
- A thing is called by HIS word for it: "global" (not "composition"), "action" (not "routine"), "keyboard and MIDI mapping", "record show", "record to clip", "the 1".
- It can be answered from the stage, not from the code: it describes what he does and what he then sees or hears.
- B and C say what would happen INSTEAD, one short sentence each; "none" when there is no real other way (he then answers "no: ...").
- Never ask what he has answered. Never ask the same point twice. Never re-word a question of the last page: if his answer left part of it open, ask only that part, as a new item.
- Nothing about where a control sits or how it looks (his L8), unless the place changes what the control DOES.`

const rulePrompt = ([key, name]) => {
  const B = BACK[key]
  return `${WHO}

YOUR TASK: RULE ON ONE TOPIC. An architect applied Boris's answers of 2026-10-07 to topic ${key}, "${name}" (the paper ${RS}/apply-${key}.md). A blind checker then went over that paper (${RS}/check-${key}.md). You are the ruling. A script lays your blocks over the paper; the result is what the build and Boris's next page stand on.

READ, each one WHOLE, in this order: 1. ${NUM} (his whole message, 136 lines). 2. ${RS}/slice-${key}.md (the topic's items as they were shown to him, each with the lines of his that name it). 3. ${RS}/apply-${key}.md. 4. ${RS}/check-${key}.md.${B ? ` 5. ${RS}/${B.paper} and ${RS}/${B.check} (a separate paper that answers ${B.what}, and its re-check).` : ''} On demand: ${BIND} (grep; his earlier words are in quotes), the fact sheets under ${R5}/ (facts-*.md, area-*.md).

RULE, in four passes:
(a) THE CHECKER'S FINDINGS. Every MUST and SHOULD and every line under its MISSING: ACCEPT, PARTIAL or REJECT. Re-derive each from his words (${NUM}, with the L number) -- never by who said it. For a finding about a quote, open ${NUM} and compare the words one by one.
(b) YOUR OWN PASS, because two readers can miss the same thing. Walk the lines of his that name this topic's items one by one (the slice lists them per item), and the general points that land here, and test the paper's RULE for each against his words. Then walk the items he did NOT name and test each against his whole message (his L9: an explanation given once answers the repeats).
(c) THE ASSUMPTIONS. Every @@ASSUME of the paper: is it real -- do no words of his settle it (also his earlier words: grep ${BIND})? Is its ASK right by the three tests? Can Boris read its TEXT in ten seconds and answer it from the stage? Is something a builder would have to guess still unnamed? Mend, drop or add.
${B ? `(d) THE QUESTION BACK. Write an @@ANSWER block with the id ${B.key}: the answer for Boris to ${B.what}. Build it from the answer paper as corrected by its re-check: plain words, the answer first, at most 150 words, a number that is an estimate is called an estimate, nothing promised that nobody has measured. If a choice is really his, one @@ASSUME (ASK: YES) carries it: TEXT = the way you recommend, written as an assumption; ALT = the other ways. Replace the @@ITEM ${B.item} block so that its RULE says how the matter stands now.` : '(d) THE QUESTIONS BACK. Every @@ANSWER block of the paper: does it answer exactly what he asked, in plain words, the answer first, in at most 130 words, without promising what nobody has measured? Mend it if not.'}

WRITE ${RS}/rule-${key}.md:
# RULING ${key} -- ${name} (s-rta-1007)
## FINDINGS RULED
(one line per finding of the checker: its item id | ACCEPT / PARTIAL / REJECT | your reason in at most 30 words, with his L number)
## MY OWN FINDINGS
(one line each: the item id | what was wrong | what you changed)
## REPLACEMENT BLOCKS
Every block of the paper that changes, written out IN FULL, in the paper's own format, with the SAME kind and the SAME id as the block it replaces. The script works like this: a block of yours with the same kind and id replaces the paper's block; a block with a new id is added; every block you do not write stays as the paper has it. A new assumption gets the next free number of the paper (${key}-<n>). To take an assumption out, write:
@@DROP ASSUME <its id>
WHY: <one line>
@@END
## FOR THE PAGE RULING
(at most 10 lines: what the ruling over ALL topics must know from this one -- a control, a name or a rule that another topic also touches; a contradiction with another topic; the assumptions that matter most)

THE PAPER'S FORMAT, which your replacement blocks keep:
${FORMAT}

${DISCIPLINE}

${PAGERULES}

${RULES}

REPORT_FILE: ${RS}/rule-${key}.md
TURN BUDGET: you stop silently at 120 turns. The four inputs read by your 12th tool call; a skeleton of the ruling (the findings table with verdicts) by the 20th; rewrite about every 10 calls; final by the 75th. Before you return, run: python3 ${RS}/wf/lint_rule.py ${RS}/rule-${key}.md ${key} -- fix what it reports until its last line reads OK, and return that last line as "lint".
RETURN: status (DONE; PARTIAL if you could not rule on every finding or an input was missing), report_path, accepted, partial, rejected, own_findings, blocks_replaced, assumptions_dropped, assumptions_added, lint, for_page (at most 6 lines, each at most 200 characters), summary (at most 400 characters).`
}

const pagePrompt = `${WHO}

YOUR TASK: THE RULING OVER ALL TOPICS, AND THE LIST FOR BORIS'S NEXT PAGE. Twelve topic papers were written, each was checked blind, and each was ruled. You now (1) make the topics agree with each other, and (2) decide for every assumption that is left whether Boris is asked about it, shown it as one line, or not troubled with it -- and you write the list his page is made from.

FIRST run: python3 ${RS}/wf/merge.py ${RS}
It lays each topic's ruling over its paper and writes ${RS}/spec-<letter>.md (the ruled blocks of a topic), ${RS}/assume-all.md, ${RS}/answers-all.md, ${RS}/names-all.md, ${RS}/today-notes.md and ${RS}/ledger.md; it prints one report line per topic: copy those lines into your ruling paper and return them. A topic with missing items or format problems: say so and go on. A topic with NO paper or NO ruling file: return PARTIAL and stop.

READ, each one WHOLE: 1. ${NUM} (his whole message, 136 lines). 2. ${RS}/assume-all.md (every assumption that is left, topic by topic, with each topic ruling's notes for you). 3. ${RS}/answers-all.md (the answers to the questions he asked Harmony). Then, on demand and by grep: an item's full rule in ${RS}/spec-<letter>.md (grep -n -A 7 "^@@ITEM <id>"), his earlier words in ${BIND}, the items as they were shown to him in ${RS}/slice-<letter>.md.

STEP 1 -- ACROSS TOPICS. A thing that more than one topic touches must be ONE thing, with ONE name and ONE rule. Look at least at these: the glide / fade-back slider of actions (his L64, L66, L67, L70 and 213 b); what a fire does while the beat is paused and while it is stopped (topics A, B, D, E; his L12, L13, L17, L31); "on the 1" for a clip, for a previewed clip (L36) and for an action; the preview monitor's two modes (L35) and "master cue" (L38); what a show file holds (presets L48 / L50, the layout L94, the outputs L96, the mapping files L116); stop (the tempo row's stop and "stop actions", L72); the spacebar (L118 against what it does in the review screen); copy and paste (clips L4 / L5, actions); the toggles on a layer that ignore or bypass (L7, L73); and every name. Where two topics' rules pull apart, his words decide if they decide it; if they do not, it is ONE page item of the kind ASK. Where your ruling changes an item's RULE, write the full replacement @@ITEM block into your ruling paper.

STEP 2 -- TRIAGE every @@ASSUME block of assume-all.md. First test it once more against his whole message (his L9: an explanation given once answers the repeats; grep ${BIND} for his earlier words too): an assumption that his words settle is SETTLED and is not put to him. Then:
- an ASK item, when all three hold: no words of his settle it; a wrong guess would be seen on stage by him or the audience, or would be costly to undo once built; it is about what the app DOES;
- a LINE item: a real choice of Harmony's that he would most likely wave through, but that is his to strike;
- INTERNAL: technical; or a look, or a place on screen (his L8: Harmony lays it out where it fits); or something only a measurement at build time can settle;
- MERGED: the same point as another assumption, in this topic or in another: one page item carries them all.
HIS TIME IS THE BUDGET. He wrote: "I have so much to read and this takes my focused time away." The whole page should take him 20 to 30 minutes. Expect something like 25 to 45 ASK items and at most about 60 LINE items; if you have more, test each one again. But never cut a real doubt to meet a number: there are no silent caps -- every assumption gets its @@TRIAGE block with the reason.

STEP 3 -- WRITE THE PAGE ITEMS.
${WORDING}

STEP 4 -- THE NINE ANSWERS he is owed. Take the ruled @@ANSWER blocks from answers-all.md (for codec, lowres and 189 the topic ruling's block wins over the answer paper's own section); mend an answer that is not plain, does not answer what he asked, or promises what nobody has measured. Where an answer ends in a choice that is his, that choice is an ASK item in its topic and the answer's ITEM names its number.

${PAGEFMT}

ALSO WRITE ${RS}/ruling-page.md: ## MERGE REPORT (the script's lines) / ## ACROSS TOPICS (each point: what the topics said, what you ruled, by which words of his) / ## REPLACEMENT BLOCKS FOR THE SPECS (each full @@ITEM block, with a line above it that reads "(topic <letter>)") / ## COUNTS (assumptions in; ASK, LINE, INTERNAL, SETTLED, MERGED out) / ## STILL UNSURE.

THEN run: python3 ${RS}/wf/lint_page.py ${RS} -- fix the items file until its last line reads OK. THEN run: python3 ${RS}/wf/render_page.py ${RS} -- it makes his page ${RS}/boris-page-2.html (and a plain-text copy boris-page-2.txt); read the text copy once from top to bottom as he would, and mend what does not read well.

${PAGERULES}

${RULES}

REPORT_FILE: ${RS}/page2-items.md (and your ruling paper ${RS}/ruling-page.md; the script's own output files are written by the script)
TURN BUDGET: you stop silently at 120 turns. merge.py run and the three inputs read by your 12th tool call; a skeleton of BOTH files by the 20th (the nine answers, and one @@TRIAGE block per assumption with a first verdict); rewrite about every 10 calls; lint OK and the page rendered by the 85th.
RETURN: status (DONE; PARTIAL if a topic was missing or the lint does not read OK), items_path, answers, ask, line, internal, settled, merged, minutes (the lint's estimate), lint (its last line), merge_report (the script's lines, each at most 260 characters), cross_topic (at most 8, each at most 220 characters), still_unsure (at most 6, each at most 220 characters), summary (at most 500 characters).`

const eyesPrompt = `${WHO}

YOUR TASK: READ BORIS'S NEXT PAGE AS BORIS. The page is ${RS}/boris-page-2.html; read its plain-text copy ${RS}/boris-page-2.txt from top to bottom, once, slowly, as a VJ who is not a programmer, has little time, and has just written (verbatim): "I have so much to read and this takes my focused time away. It would be best if you just asked me focused questions on any assumption that you're making." and "Don’t list what is happening today as we are discussing a major change." The list the page is made from is ${RS}/page2-items.md (blocks @@PAGE-ANSWER and @@PAGE-ITEM; cite an item by its number, an answer by its key). His message, for what he has already said: ${NUM}.

FIND, item by item: (1) an item he cannot answer in ten seconds: too long, two points in one, a word he would have to look up, an "it" that is unclear, an alternative that does not say what would happen instead; (2) anything that describes how the app works now; (3) two items that ask the same thing; (4) an item about where something sits or how it looks; (5) an answer in part 1 that does not answer what he asked, does not put the answer first, is not plain, or promises what nobody has measured; (6) an item whose "I assume" he could not picture on stage -- give the stage picture it needs; (7) the page as a whole: is anything in the intro wrong or unclear; would he know how to answer; is the order sensible? For every finding give the FIX as the exact replacement text. Do not report matters of taste; do not ask for more text.

${WORDING}

YOUR PAPER (markdown): ## FINDINGS (each: MUST or SHOULD | the item number or answer key | what is wrong | the exact replacement text) / ## THE TEN ITEMS I WOULD CUT OR SHORTEN FIRST / ## WHAT READS WELL.

${RULES}

REPORT_FILE: ${RS}/pcheck-eyes.md
RETURN: verdict (SOUND / SOUND_WITH_CORRECTIONS / UNRELIABLE), paper_path, checked (items read), findings (every MUST and SHOULD; item = the number or key; each field at most 500 characters), summary (at most 400 characters).`

const settledPrompt = (half, n) => `${WHO}

YOUR TASK: HAS HE ALREADY ANSWERED IT? Boris's next page is made from the list ${RS}/page2-items.md (blocks @@PAGE-ANSWER and @@PAGE-ITEM). He must never be asked what he has already said. You take the ${half} half of the @@PAGE-ITEM blocks: sort them by number and take ${n === 0 ? 'the first half (round up)' : 'the second half (the rest)'}; say in your paper which numbers you took.

FOR EVERY ITEM OF YOUR HALF: (1) read its TEXT, B and C; (2) search his message of 2026-10-07 (${NUM}: read it WHOLE once at the start, then search it per item) for words that settle the point, fully or in part; (3) search his earlier words: grep -n -i for two or three key words of the item in ${BIND} (his words are inside quotes; the text after "->" is Harmony's and settles nothing); (4) look whether the item re-words a question or a reading of the last page that he answered or did not correct: the FROM field names the ids, and each id is in ${RS}/spec-<its topic letter>.md (grep -n -A 7 "^@@ITEM <id>") and, as it was shown to him, in ${RS}/slice-<letter>.md. A finding is MUST when his words settle the item (quote them, with the L number or the file line) or contradict its "I assume"; SHOULD when his words settle part of it and the item should ask only the rest (give the narrower text). Say plainly "nothing found" for an item when that is so: most items should pass.

YOUR PAPER (markdown): ## THE NUMBERS I TOOK / ## FINDINGS (each: MUST or SHOULD | the item number | his words, quoted, and where | the fix: drop the item, or the exact narrower text) / ## ITEMS WITH NOTHING FOUND (numbers only).

${RULES}

REPORT_FILE: ${RS}/pcheck-settled-${n + 1}.md
RETURN: verdict (SOUND / SOUND_WITH_CORRECTIONS / UNRELIABLE), paper_path, checked (items searched), findings (every MUST and SHOULD; each field at most 500 characters), summary (at most 400 characters).`

const COVER = [['ABC', 'A, B and C'], ['D', 'D'], ['E', 'E'], ['FGHIJKX', 'F, G, H, I, J, K and X']]
const coverPrompt = ([letters, words]) => `${WHO}

YOUR TASK: WAS THE TRIAGE RIGHT? After Boris's answers of 2026-10-07 a list of assumptions was left (${RS}/assume-all.md: @@ASSUME blocks, topic by topic). A ruling decided for each one whether Boris is asked about it (an ASK item on his page), shown it as one line (a LINE item), or not troubled with it (INTERNAL, SETTLED by his words, or MERGED into another item). The decisions are the @@TRIAGE blocks of ${RS}/page2-items.md; the page items are its @@PAGE-ITEM blocks. You take the assumptions of topic${letters.length > 1 ? 's' : ''} ${words} (the ids that begin with ${letters.split('').map(l => l + '-').join(', ')}) and re-derive the triage for each.

READ WHOLE: ${NUM} (his message, 136 lines); the sections of ${RS}/assume-all.md for your topics (grep -n "^## TOPIC" first); ${RS}/page2-items.md.
FOR EVERY ASSUMPTION OF YOUR TOPICS: decide FIRST, yourself, where it belongs, by these tests -- an ASK item when all three hold: no words of his settle it; a wrong guess would be seen on stage by him or the audience, or would be costly to undo once built; it is about what the app DOES. A LINE item: a real choice of Harmony's that he would most likely wave through. INTERNAL: technical, or a look or a place on screen (his L8), or only a measurement at build time can settle it. THEN compare with its @@TRIAGE block. A finding is MUST when: an assumption that passes the three tests was made INTERNAL or was dropped without a block; a TRIAGE says SETTLED and the cited words of his do not settle it (open ${NUM} at that line); an assumption was MERGED into a page item that does not carry its point; a page item's TEXT says something else than the assumption(s) it comes from, or its B / C are not the real other ways; an assumption has no @@TRIAGE block at all. SHOULD when: an ASK item should be a LINE item or the other way round; an item should not be on the page at all because he would not care (say why).

YOUR PAPER (markdown): ## MY OWN TRIAGE (one line per assumption: its id | mine | the ruling's | agree or not) / ## FINDINGS (each: MUST or SHOULD | the assumption id and the page item number | what is wrong, with his words and their L number | the fix as exact text) / ## COUNTS.

${RULES}

REPORT_FILE: ${RS}/pcheck-cover-${letters}.md
RETURN: verdict (SOUND / SOUND_WITH_CORRECTIONS / UNRELIABLE), paper_path, checked (assumptions re-derived), findings (every MUST and SHOULD; item = the assumption id and page item number; each field at most 500 characters), summary (at most 400 characters).`

const namesPrompt = `${WHO}

YOUR TASK: THE LIST OF WHAT EVERYTHING IS CALLED. His words (L114 of ${NUM}, verbatim): "We need a solid list with what we call everything. You create and keep one and I will ask questions and you can give me the truth, which will be this doc. You will use this for comms with me and to name all features in the app, menus and manual(later)." Write the first version of that list. It is the ONE place that says what a thing is called; Harmony will keep it up to date.

SOURCES, in this order of authority: (1) HIS WORDS. His message of 2026-10-07: ${NUM} (read it whole). His earlier naming words: grep -n -i "call\\|name\\|naming\\|rename\\|term" ${BIND} and read each hit in its entry (his words are inside quotes; the text after "->" is Harmony's). (2) The names this session's papers fix or pick: ${RS}/names-all.md (@@NAME blocks: the name, what it means, where it comes from). (3) The words his next page uses: ${RS}/page2-items.md. (4) What is on screen in the app now, for the things that keep their names: the fact sheets ${R5}/area-*.md name each control by its on-screen words with file:line (grep -n '^#' first; read the parts that list controls), and ${H}/APP-INVENTORY.md. Do not read the source for more than a missing on-screen word.

THE LIST (markdown) -- for Boris: plain words, no class names, no file names:
# Audio-DNA -- what everything is called
(three lines: what this list is, in his own words; written <stamp from date>; how to read the "from" column)
Then one section per area, in this order: The show and its parts (show, deck, layer, clip, cell, column, global ...) / Firing clips and the tempo row / The cue system / Effects and presets / Signals / Actions / Recording and the review screen / Output screens / The keyboard and MIDI mapping / Sources and the automatic features / Words that are no longer used.
Each section is a table with four columns:
| Name | What it is | From | Replaces |
- Name: exactly as it reads on screen and as it is said.
- What it is: one plain sentence.
- From: "your words, <date>" when he named it (quote the words in the entry's sentence if they are short) | "my pick" when Harmony chose it and he has not confirmed it | "on screen now" when it is simply the name the app already shows.
- Replaces: the older name it takes the place of, if any.
The last section, "Words that are no longer used", lists each retired word with the word that replaces it (for example "routine" -> "action"; "composition" -> "global", by his L55).
RULES OF THE LIST: one name per thing and one thing per name -- where the sources use two names for one thing, his words decide; if he has not named it, pick the one the page uses and mark it "my pick". Where his own words use two names for what may be one thing (for example the glide slider of actions: L64, L66, L67, L70), list it under the name his next page uses and say in "What it is" that these are taken to be one thing. Do not invent things: every row is a thing the papers or the app have. Aim for completeness over polish: about 120 to 220 rows.

${RULES}

REPORT_FILE: ${RS}/names-draft.md
TURN BUDGET: a skeleton with every section and its first rows within your first 15 tool calls; rewrite about every 10 calls; final within 70.
RETURN: status, report_path, names (count of rows), his (rows from his words), picks (rows marked "my pick"), summary (at most 400 characters).`

const fixPrompt = (papers) => `${WHO}

YOUR TASK: THE SECOND RULING ON BORIS'S NEXT PAGE. The list his page is made from (${RS}/page2-items.md: @@PAGE-ANSWER, @@PAGE-ITEM and @@TRIAGE blocks) was written by a ruling over all topics. Seven checkers then went over it: one read the page as Boris; two searched whether he has already answered an item; four re-derived the triage of the assumptions (${RS}/assume-all.md). You rule on every finding and mend the list. After you, the page is shown to him.

READ, each one WHOLE: 1. ${NUM} (his whole message). 2. ${RS}/page2-items.md. 3. The first ruling's paper ${RS}/ruling-page.md. 4. The checkers' papers, every one: ${papers.map(p => RS + '/' + p).join(', ')}. On demand: ${RS}/assume-all.md, ${RS}/spec-<letter>.md (grep -n -A 7 "^@@ITEM <id>"), ${BIND}.

RULE on every MUST and SHOULD of every paper: ACCEPT, PARTIAL or REJECT, each re-derived from his words (the L number) or from the assumption's own text -- never by who said it, and never by how many said it. Where two checkers pull apart, his words decide; where his words do not decide, the item stays an ASK item. A finding that says "he has already answered this": open the cited line and read it yourself before you drop an item. A finding that says "this should be asked": apply the three tests yourself (no words of his settle it; seen on stage or costly to undo; about what the app does).

THEN MEND ${RS}/page2-items.md: rewrite the file whole. Removing or adding an item shifts the numbers: keep them running from 217 without a gap, first every ASK item (topics A to K then X), then every LINE item; update every @@TRIAGE block's TO and every @@PAGE-ANSWER's ITEM that names a number you moved. HIS TIME IS THE BUDGET: the page should take him 20 to 30 minutes; a mend that makes an item longer must make it clearer, too.

${WORDING}

${PAGEFMT}

WRITE ${RS}/ruling-page-2.md: ## FINDINGS RULED (one line each: the paper | the item | ACCEPT / PARTIAL / REJECT | the reason in at most 30 words) / ## WHAT CHANGED ON THE PAGE (each item added, dropped, re-worded or moved; old number -> new number where numbers moved) / ## COUNTS / ## STILL UNSURE.
THEN run: python3 ${RS}/wf/lint_page.py ${RS} -- fix until its last line reads OK. THEN run: python3 ${RS}/wf/render_page.py ${RS} and read ${RS}/boris-page-2.txt once from top to bottom as he would.

${PAGERULES}

${RULES}

REPORT_FILE: ${RS}/page2-items.md (and your ruling paper ${RS}/ruling-page-2.md)
TURN BUDGET: you stop silently at 120 turns. All inputs read by your 16th tool call; the findings table by the 28th; the mended items file by the 60th; lint OK and the page rendered by the 80th.
RETURN: status (DONE; PARTIAL if a checker's paper was missing or the lint does not read OK), items_path, answers, ask, line, internal, settled, merged, minutes, lint (its last line), merge_report (leave empty), cross_topic (what changed on the page: at most 10 lines, each at most 220 characters), still_unsure (at most 6, each at most 220 characters), summary (at most 500 characters: say how many findings you accepted, in part, and rejected).`

const namesCheckPrompt = `${WHO}

YOUR TASK: RE-CHECK AND MEND THE LIST OF WHAT EVERYTHING IS CALLED, ${RS}/names-draft.md. Boris asked for it (L114 of ${NUM}): "We need a solid list with what we call everything. You create and keep one and I will ask questions and you can give me the truth, which will be this doc." A wrong row there becomes a wrong word in the app.

DO: (1) Read the list whole and ${NUM} whole. (2) Every row whose "From" says "your words": find the words (in ${NUM} by L number, or in ${BIND} by grep) and confirm that he named THAT thing THAT way; a row that claims his words without them becomes "my pick". (3) Every naming word of his that is NOT in the list: grep -n -i "call\\|name\\|naming\\|rename\\|term" ${BIND}, and walk ${NUM} line by line for a thing he names (for example "master cue", "record to clip", "record show", "Review", "Macros", "global", "keyboard and MIDI mapping", "Timeline", the glide slider) -- add the missing rows. (4) One name per thing and one thing per name: two rows for one thing are merged; one name on two things is split. (5) The words of his next page (${RS}/page2-items.md, the TEXT fields) must be names of this list: where the page uses another word for a thing, report it as a finding (you do not edit the page). (6) Plain words, no class or file names. MEND the list in place (this ONE file you may edit) and add at its end a section "## Re-check (s-rta-1007)" with one line per change.

${RULES}

REPORT_FILE: ${RS}/check-names.md (your findings; the list ${RS}/names-draft.md is the only other file you may touch)
RETURN: verdict (SOUND / SOUND_WITH_CORRECTIONS = you mended it / UNRELIABLE), paper_path, checked (rows re-checked), findings (each mend, and each word of the page that is not a name of the list: severity, item, issue, fix; each field at most 300 characters), summary (at most 400 characters).`

// ---------------- stage 1: one ruling per topic (a FIXED literal order) ----------------
const leg = (args && args.leg) || 'all'
phase('Rule topics')
const r1 = await parallel(TOPICS.map(T => () => agent(rulePrompt(T), { agentType: 'architect', model: 'opus', effort: 'max', schema: TS, phase: 'Rule topics', label: 'rule:' + T[0] })))
const t1 = TOPICS.map((T, i) => ({ key: T[0], res: r1[i] }))
const ok1 = t1.filter(t => t.res && t.res.status === 'DONE')
const sum1 = t1.map(t => ({ key: t.key, status: t.res ? t.res.status : 'NONE', a: t.res && t.res.accepted, p: t.res && t.res.partial, r: t.res && t.res.rejected, own: t.res && t.res.own_findings, dropped: t.res && t.res.assumptions_dropped, added: t.res && t.res.assumptions_added, lint: t.res && cut(t.res.lint, 80) }))
log('topic rulings: ' + ok1.length + ' of ' + TOPICS.length + ' DONE; ' + sum1.map(s => s.key + '=' + s.status).join(' '))
if (ok1.length < TOPICS.length || leg === 'topics') return { stopped: ok1.length < TOPICS.length ? 'a topic ruling is missing or PARTIAL: the page ruling did not run' : 'leg topics only', topics: sum1 }

// ---------------- stage 2: the ruling over all topics; it writes the list and renders the page ----------------
phase('Rule page')
const pg = await agent(pagePrompt, { agentType: 'architect', model: 'opus', effort: 'max', schema: PS, phase: 'Rule page', label: 'rule:page' })
const sumP = pg ? { status: pg.status, answers: pg.answers, ask: pg.ask, line: pg.line, internal: pg.internal, settled: pg.settled, merged: pg.merged, minutes: pg.minutes, lint: cut(pg.lint, 100), merge_report: (pg.merge_report || []).slice(0, 14).map(x => cut(x, 260)), cross_topic: (pg.cross_topic || []).slice(0, 8).map(x => cut(x, 220)), still_unsure: (pg.still_unsure || []).slice(0, 6).map(x => cut(x, 220)), summary: cut(pg.summary, 500) } : null
if (!pg || pg.status !== 'DONE' || !/OK\s*$/.test(String(pg.lint || ''))) { log('the page ruling did not return DONE with a clean lint: the checks did not run; nothing is shown to Boris'); return { stopped: 'page ruling not DONE or lint not OK', topics: sum1, page: sumP } }

// ---------------- stage 3: seven checkers on the page, and the names list is written ----------------
phase('Check page')
const S3 = [
  { key: 'eyes', file: 'pcheck-eyes.md', run: () => agent(eyesPrompt, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check page', label: 'pcheck:eyes' }) },
  { key: 'settled-1', file: 'pcheck-settled-1.md', run: () => agent(settledPrompt('first', 0), { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check page', label: 'pcheck:settled-1' }) },
  { key: 'settled-2', file: 'pcheck-settled-2.md', run: () => agent(settledPrompt('second', 1), { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check page', label: 'pcheck:settled-2' }) },
  ...COVER.map(C => ({ key: 'cover-' + C[0], file: 'pcheck-cover-' + C[0] + '.md', run: () => agent(coverPrompt(C), { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check page', label: 'pcheck:cover-' + C[0] }) })),
]
const r3 = await parallel(S3.map(s => s.run).concat([() => agent(namesPrompt, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: NS, phase: 'Check page', label: 'names:list' })]))
const names = r3[S3.length]
const c3 = S3.map((s, i) => ({ key: s.key, file: s.file, res: r3[i] }))
const back3 = c3.filter(c => c.res)
const sum3 = c3.map(c => ({ key: c.key, verdict: c.res ? c.res.verdict : 'NONE', checked: c.res && c.res.checked, must: c.res ? (c.res.findings || []).filter(f => f.severity === 'MUST').length : undefined, should: c.res ? (c.res.findings || []).filter(f => f.severity === 'SHOULD').length : undefined }))
log('page checks: ' + back3.length + ' of ' + S3.length + ' papers back; ' + sum3.map(s => s.key + '=' + s.verdict + '/' + s.must + 'M').join(' ') + '; names list: ' + (names ? names.status + ' ' + names.names + ' rows' : 'NONE'))
if (back3.length < S3.length) return { stopped: 'fewer check papers came back than seats were sent: the second ruling did not run; the page is NOT to be shown', topics: sum1, page: sumP, checks: sum3, names: names ? { status: names.status, names: names.names, picks: names.picks } : null }

// ---------------- stage 4: the second ruling mends the list and renders the page; the names list is re-checked ----------------
phase('Rule checks')
const r4 = await parallel([
  () => agent(fixPrompt(S3.map(s => s.file)), { agentType: 'architect', model: 'opus', effort: 'max', schema: PS, phase: 'Rule checks', label: 'rule:page-2' }),
  () => names ? agent(namesCheckPrompt, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: CS, phase: 'Rule checks', label: 'check:names' }) : Promise.resolve(null),
])
const fx = r4[0], nc = r4[1]
return {
  topics: sum1, page: sumP, checks: sum3,
  page2: fx ? { status: fx.status, answers: fx.answers, ask: fx.ask, line: fx.line, internal: fx.internal, settled: fx.settled, merged: fx.merged, minutes: fx.minutes, lint: cut(fx.lint, 100), changed: (fx.cross_topic || []).slice(0, 10).map(x => cut(x, 220)), still_unsure: (fx.still_unsure || []).slice(0, 6).map(x => cut(x, 220)), summary: cut(fx.summary, 500) } : null,
  names: names ? { status: names.status, names: names.names, his: names.his, picks: names.picks, check: nc ? { verdict: nc.verdict, checked: nc.checked, findings: (nc.findings || []).length, page_words: (nc.findings || []).filter(f => /page/i.test(f.item + ' ' + f.issue)).slice(0, 6).map(f => cut(f.issue, 200)), summary: cut(nc.summary, 300) } : 'NONE' } : null,
  complete: !!(fx && fx.status === 'DONE' && /OK\s*$/.test(String(fx.lint || ''))),
}
