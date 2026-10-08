export const meta = {
  name: 'rta-1007-apply',
  description: 'Audio-DNA s-rta-1007: apply Boris\'s answers of 2026-10-07 to every item of the page of all open questions. Stage 1: one architect per topic writes how that part of the app will work after his words (plus the assumptions that are left), three seats answer his questions back, one reader writes the MilkDrop document. Stage 2: every paper is re-read by a blind checker. Read-only except the files named. NOTHING IS BUILT.',
  phases: [{ title: 'Apply' }, { title: 'Check' }],
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

const AS = { type: 'object', required: ['status', 'report_path', 'items', 'assume_yes', 'assume_line', 'assume_no', 'lint', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, items: { type: 'number' }, assume_yes: { type: 'number' }, assume_line: { type: 'number' }, assume_no: { type: 'number' }, answers: { type: 'number' }, names: { type: 'number' }, lint: { type: 'string' }, conflicts: { type: 'array', items: { type: 'string' } }, top_assumptions: { type: 'array', items: { type: 'string' } }, summary: { type: 'string' } } }
const QS = { type: 'object', required: ['status', 'report_path', 'answer_for_boris', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, answer_for_boris: { type: 'string' }, facts_verified: { type: 'number' }, unknowns: { type: 'array', items: { type: 'string' } }, question_for_him: { type: 'string' }, summary: { type: 'string' } } }
const CS = { type: 'object', required: ['verdict', 'paper_path', 'checked', 'findings', 'summary'], properties: { verdict: { type: 'string', enum: ['SOUND', 'SOUND_WITH_CORRECTIONS', 'UNRELIABLE'] }, paper_path: { type: 'string' }, checked: { type: 'number' }, findings: { type: 'array', items: { type: 'object', required: ['severity', 'item', 'issue', 'fix'], properties: { severity: { type: 'string', enum: ['MUST', 'SHOULD'] }, item: { type: 'string' }, issue: { type: 'string' }, fix: { type: 'string' } } } }, missed: { type: 'array', items: { type: 'string' } }, summary: { type: 'string' } } }

const TOPICS = [
  { key: 'A', name: 'Firing clips and the tempo row', sheets: ['area-tempo.md', 'facts-takes-bpm-fire.md', 'facts-resolume-emulate.md'],
    extra: `HIS QUESTION BACK that lands here: question 189 (L32). A separate architect answers it (file answer-tempo-auto.md); you file 189 as STATUS OPEN with RULE "waits on Harmony's answer to his question back (L32)" and write no @@ANSWER for it. Harmony's seed, to be TESTED, not adopted: his lines L12, L13, L17 and L31 read together may mean -- PAUSED and he fires a clip: the beat runs on from where it was held, and a BPM-mode clip then starts on the next "1"; STOPPED and he fires a clip: his press is the new "1" and the clip plays at once. Check whether every line of his fits that, say exactly where one does not, and make what is left an @@ASSUME. L17, L18 and L19 (R206 b, c, d) are looks in his Resolume that bear on your topic.` },
  { key: 'B', name: 'The cue system', sheets: ['area-cue-layers.md', 'facts-app-cue-today.md', 'facts-resolume-emulate.md'],
    extra: `L16 and L29 (R206 a and h) are looks in his Resolume that bear on your topic. His L34-L38 add new things (a transparency slider beside each cue button; a toggle between cue mode and preview mode; previews from the files window; "master cue"): each needs a RULE complete enough to build, and every edge his words leave open is an @@ASSUME.` },
  { key: 'C', name: 'Presets', sheets: ['facts-presets-delta.md', 'area-effects-signals.md', 'facts-resolume-emulate.md'],
    extra: `Pictures to LOOK at with the Read tool: ${IMG}/img-04-7dc96e027a08.png (R206 g, L22: Resolume's "Manage Presets" window), ${IMG}/img-06-ccaf044f8203.png (R154, L41: Resolume's effects tab with an effect double-clicked), and the two of 2026-10-05: ${R5}/boris-images/resolume-effects-tab-presets.png, ${R5}/boris-images/resolume-effect-panel-presets-menu.png. His L48 and L50 (presets are kept with the app AND with the show file; they travel with the show) replace his earlier words "these are saved with the app. always.": say exactly what follows -- where a preset lives, what happens when a show is opened on another computer, and what happens when one name exists in both places with different values -- and make an @@ASSUME of each part his words do not say.` },
  { key: 'D', name: 'Actions in a show', sheets: ['area-actions.md', 'facts-actions-open.md'],
    extra: `HIS QUESTION BACK: R129 point 4 (L66: "please explain in more detail about loading a preset onto an effect with an action playing") -> an @@ANSWER R129-4 that explains it in plain steps and says what you recommend. Picture to LOOK at: ${IMG}/img-08-85be4ebfacaf.png (R134, L56: the small row below a clip). His lines name a glide or fade several times -- L64 "Global glide slider", L66 "global glide back setting", L67 "master action fade back time", L70 "Global fade control", and his answer 213 b: say whether these are ONE control or several; if his words do not settle it, that is an @@ASSUME with ASK: YES. His L64 adds a loop toggle per action, L72 a global "stop actions" button, L7 an "ignore actions" toggle on the layer (G5), L73 a layer's bypass of global actions: say for each exactly what it does and how it sits with the others (is L7's toggle the same thing as L73's bypass?).` },
  { key: 'E', name: 'The review screen and recordings', sheets: ['area-recording.md', 'facts-takes-bpm-fire.md'],
    extra: `HIS QUESTION BACK: R184 (L87: he does not understand the sentence "To play a whole recording as a performance you make one action from its start to its end.") -> an @@ANSWER R184 that says in plain words what was meant, and whether it still holds after his other answers of this message. The NAME of this screen (L72, L114) is answered by the topic-J architect, not by you; use "the review screen" meanwhile. G4 (L6, the low-resolution show recording in chunks): you file the FUNCTION as an item with its RULE; a researcher answers what it costs. His L88 (a recording starts on the "1" and ends on an even grid line; no one-layer recording; the recording is the output of the layers, not the screen output) and L92 (record over with the MIDI controller, in real time, kept separately) change a lot: write each RULE in full.` },
  { key: 'F', name: 'The show file, decks and saving', sheets: ['area-show-decks.md'],
    extra: `HIS QUESTION BACK: R188 (L94: "Do we need snapshot?") -> an @@ANSWER R188-snapshot: say in plain words what the snapshot was for in R188, whether anything still needs it after his words (the show holds the layout; the app opens the last show), and what you recommend. G2 and G3 (L4, L5: copy and paste for any clip; Option-drag copies): write the RULE in full (what exactly is copied with a clip -- its effects, its actions, its mapping, its place in the deck? what happens to the clip in a cell that is pasted over? across decks? Cmd+C / Cmd+V / Cmd+X? undo?) and make an @@ASSUME of each part his words do not say. His L69 ("let's delete all the old show files and start from scratch") is a destructive act on his own files: the RULE says that it is done only when the build starts, with the files named to him first and moved to the Trash, never erased; one @@ASSUME lists which files Harmony takes "old show files" to mean (look up where shows, their backups and the app's settings live in the fact sheet; do NOT open, move or touch any of them).` },
  { key: 'G', name: 'Output screens', sheets: ['area-outputs.md'],
    extra: `His L96 (a show saved with the outputs connected remembers them when it is opened with the outputs connected) has to be squared with his earlier rule that the app never opens an output by itself (grep "never opens" and "Restore Last Outputs" in ${BIND} and read ${MAIN}/CLAUDE.md, the paragraph "Outputs"): say what follows and make the open part an @@ASSUME with ASK: YES if his words pull apart.` },
  { key: 'H', name: 'How a clip plays', sheets: ['area-clip-transport.md', 'facts-resolume-emulate.md'],
    extra: `G1 (L3, a codec of our own): file it as an item with STATUS OPEN and RULE "waits on the researcher's answer (answer-codec.md)"; do not answer it. L20 and L21 (R206 e and f) are looks in his Resolume that bear on your topic (Random; beats, Speed and Duration of a BPM-mode clip): the rules for 201, 202, R217 and R218 must agree with them, and where his words and the look differ that is a CONFLICT.` },
  { key: 'I', name: 'Effects, signals and what moves a slider by itself', sheets: ['area-effects-signals.md'],
    extra: `HIS QUESTION BACK: R221 b (L107: "Is this a good idea, help me think through this logically") -> an @@ANSWER R221-b that thinks it through in plain steps: what he and the audience see with each choice while the beat is paused, while it is stopped, and while the Master Signal is at 0; then your recommendation. His L106 moves One Shot and Looping, a threshold for buttons, and gain and falloff to the USER of a signal (the slider or button that uses it): write that RULE in full. His L111 (the keying and its slider go; only the transparency slider controls the layer's blend mode) and L112 ("Timeline" is what connects anything to the layer's playhead) need RULES in full, with an @@ASSUME for every part his words leave open.` },
  { key: 'J', name: 'The keyboard and MIDI mapping, menus and messages', sheets: ['area-controls.md'],
    extra: `HIS QUESTIONS BACK. (1) 207 (L120): he chose c and asks for an argument why there should be a key / pad hold setting and how it helps DJs or bands -> an @@ANSWER 207-hold: the honest argument FOR it with two or three stage examples, the argument against, and your recommendation. (2) The name of the review screen (L114: "Do you have a better name for this?"; L72: "We need a good name for this screen so it’s easy to remember and discuss") -> an @@ANSWER review-name with your pick and at most two alternatives, one line of reason each. Harmony's note, to be weighed, not adopted: the app will also have a "preview" mode in the cue system (L35), and "Review" and "Preview" sound alike when spoken; his own earlier word for it was "the replay window" (grep "replay window" in ${BIND}). (3) His L114 also sets a TASK, the list of what everything is called: a later step builds it from every paper's NAMES blocks; list yours carefully. His L116 ("we want mapping files") and L118 ("Spacebar is typically tap tempo") need RULES in full; what a mapping file holds, and what the spacebar does in the review screen (where an earlier answer of his made it stop the playback: grep "spacebar" in ${BIND}), are @@ASSUME blocks if his words leave them open.` },
  { key: 'K', name: 'Sources and the automatic features', sheets: ['area-sources-auto.md'],
    extra: `His L126: a dedicated document on how MilkDrop works now is being written by another agent in this run (file milkdrop-current.md); file R202 accordingly (nothing of MilkDrop is changed in the coming build; the smarter system is designed with him in a session of its own). His L128 ("plan all of these") answers question 215: all six things are planned. For each of the six say in the RULE what "planned" has to settle before it can be built, and write an @@ASSUME only where a real choice is his (for example the ORDER: Harmony's assumption is that the six come after the first builds -- firing and the tempo row, the cue system, presets, actions, the review screen -- unless he says otherwise).` },
]

const applyPrompt = T => `${WHO}

YOUR TASK: APPLY HIS ANSWERS to topic ${T.key}, "${T.name}". You are the architect who writes down how this part of the app will work after his words of 2026-10-07: the text the build will stand on, and the short list of what is still assumed.

READ, in this order, each one WHOLE:
1. ${NUM} -- his whole message (136 lines). Read ALL of it, not only the lines that name your items: he explains a thing once and expects it to be used wherever it applies (L9).
2. ${RS}/slice-${T.key}.md -- your topic's items exactly as they were shown to him, each with the lines of his that name it.
3. Fact sheets, for the TODAY lines only (each ends in a "## CORRECTIONS" block that wins over the lines above it): ${T.sheets.map(s => R5 + '/' + s).join(', ')}. grep -n '^#' first; read what your items need.
4. On demand only: ${BIND} (grep for a question number or a word; the section headed "2026-10-07 (s-rta-1007)" holds this message entry by entry), the page's record ${R5}/boris-clarify-all.md (grep -n '^## ' first; never whole), and another topic's slice ${RS}/slice-<letter>.md when one of your items leans on it (A firing clips and the tempo row, B the cue system, C presets, D actions, E the review screen and recordings, F the show file and decks, G output screens, H how a clip plays, I effects and signals, J the keyboard and MIDI mapping, K sources and the automatic features).

FOR THIS TOPIC: ${T.extra}

${DISCIPLINE}

${PAGERULES}

${FORMAT}

${RULES}

${CODE}

REPORT_FILE: ${RS}/apply-${T.key}.md
TURN BUDGET: you stop silently at 120 turns. Inputs read by your 14th tool call; a SKELETON of the report (every @@ITEM id of the slice, STATUS only) written by the 18th; then fill it part by part and rewrite the file about every 10 calls; final by the 80th. Before you return, run: python3 ${RS}/wf/lint_apply.py ${RS}/apply-${T.key}.md ${T.key} -- fix what it reports until its last line reads OK, and return that last line as "lint".
RETURN: status, report_path, items (count of @@ITEM blocks), assume_yes, assume_line, assume_no, answers, names, lint, conflicts (at most 8, each at most 220 characters), top_assumptions (the at most 6 most important ASK: YES texts, each at most 200 characters), summary (at most 500 characters).`

const loosePrompt = `${WHO}

YOUR TASK: the page's three LOOSE LISTS against his answers. The page of 2026-10-05 carried, on Harmony's side, a list of places where two statements disagreed (C1..), a list of things not established (N1..) and a list of things Harmony was still unsure of (U1..). His message of 2026-10-07 settles many of them. For EVERY entry say whether his words now settle it.

READ, each one WHOLE: 1. ${NUM} (his whole message, 136 lines). 2. ${RS}/slice-X.md (the three lists). 3. On demand: ${BIND} (grep; the section headed "2026-10-07 (s-rta-1007)" holds this message entry by entry), the topic slices ${RS}/slice-<letter>.md (the question or reading an entry was "handled as" is in one of them: A firing clips and the tempo row, B the cue system, C presets, D actions, E the review screen and recordings, F the show file and decks, G output screens, H how a clip plays, I effects and signals, J the keyboard and MIDI mapping, K sources and the automatic features), the fact sheets ${R5}/facts-*.md and ${R5}/area-*.md.

FOR EACH ENTRY one @@ITEM block. STATUS SETTLED = his words of 2026-10-07 settle it (HIS cites the lines; RULE says how). STATUS DROPPED = it no longer matters (say why: the item it was handled as is answered or its default is taken; a thing about Resolume he has now looked at under R206, L15-L22 and L29; a look that waits for the UI redesign, L8). STATUS OPEN = still open: RULE says what exactly is open, and an @@ASSUME block puts it to him. An N entry that only Harmony can settle by a look into the app or by a measurement when it is built is STATUS OPEN with ASK: NO on its @@ASSUME (technical), and TODAY names the cheapest way. There are no QUESTIONS BACK and usually no NAMES in this paper; keep those two headings, empty.

${DISCIPLINE}

${PAGERULES}

${FORMAT}

${RULES}

${CODE}

REPORT_FILE: ${RS}/apply-X.md
TURN BUDGET: you stop silently at 120 turns. Inputs read by your 12th tool call; a SKELETON (every id, STATUS only) by the 16th; rewrite about every 10 calls; final by the 75th. Before you return, run: python3 ${RS}/wf/lint_apply.py ${RS}/apply-X.md X -- fix what it reports until its last line reads OK, and return that last line as "lint".
RETURN: status, report_path, items, assume_yes, assume_line, assume_no, answers (0), names, lint, conflicts (at most 8, each at most 220 characters), top_assumptions (at most 6, each at most 200 characters), summary (at most 500 characters).`

const ANSWERFMT = `THE PAPER (markdown). Sections, in this order:
## ANSWER FOR BORIS  (plain words, the answer first, then why, then what you recommend; at most 150 words; no jargon he would have to look up -- where a technical word cannot be avoided, say in five words what it is; nothing about how the app's code is built)
## QUESTION FOR HIM  (ONE focused question if a choice is really his, with a default marked and at most two alternatives, each one short sentence; or the word "none")
## FACTS  (one bullet per fact; each labelled VERIFIED with its source -- a URL plus the quoted words, or file:line -- or INFERRED from what, or UNKNOWN with the cheapest way to find out)
## OPTIONS  (each option: what it is, what it costs to build in rough size S / M / L, what can go wrong, how one would know that it works)
## WHAT HAS TO BE MEASURED WHEN IT IS BUILT
## NOT DONE / UNSURE`

const QUESTIONS = [
  { key: 'codec', type: 'researcher', model: 'sonnet', effort: 'high', file: 'answer-codec.md',
    task: `HIS QUESTION (L3, verbatim): "- Is there any reason for us to build our own codec that is optimized for our system like resolume’s DVX 3.0? Is that something that you can do reliably?" ("DVX 3.0" as he typed it; Resolume's codec is DXV 3.)
ANSWER IT WITH FACTS. (1) What the app does with video now -- for your own understanding only: ${R5}/area-clip-transport.md (grep -n '^#' first), ${MAIN}/CLAUDE.md pitfalls 54, 56, 60, 62 and 64 with their full text in ${MAIN}/docs/claude/pitfalls.md, and the video classes under ${MAIN}/src (grep -rn "VideoPlayer\\|VideoRing\\|avcodec" ${MAIN}/src --include=*.h -l). (2) On the web, from primary sources where they exist: what DXV 3 is and why Resolume made it (Resolume's own pages and manual); HAP by Vidvox (its public specification, its variants -- Hap, Hap Alpha, Hap Q, Hap R / HDR --, its licence, which programs play it, FFmpeg's support for encoding and decoding it); NotchLC; ProRes 4444 for see-through video; FFmpeg's support for DXV. What such "performance codecs" buy a live video app: every frame stands alone (instant jumps, backwards play, scrubbing, random jumps on the beat -- all of which Boris's rules for BPM-mode clips, Random and the review screen lean on), the graphics card does the unpacking, see-through video, and the price (file size, a conversion step). (3) The real options for Audio-DNA: (a) invent and build a codec of our own from nothing; (b) adopt an existing open performance codec as the app's fast format, with a built-in "convert for performance" step, and keep playing ordinary files as now; (c) change nothing. For each: size, risks, how reliable the result can be made and how one would prove it. (4) "Can you do that reliably?": answer honestly for (a) and for (b).` },
  { key: 'lowres', type: 'researcher', model: 'sonnet', effort: 'high', file: 'answer-lowres-rec.md',
    task: `HIS POINT (L6, verbatim): "- I think it might be good to record a very low resolution show recording if that is possible and to record it in 10 or 20 minute chunks so that they are small and if something happens most of the recording is not lost. Maybe something like 1/4 or 1/8 the size so it is very minimal to use as a double check. How much comp resource would this take up?"
ANSWER HIS QUESTION -- how much of the computer it takes -- with numbers that are honest about being estimates. (1) What exists to build it from, for your own understanding only: ${R5}/area-recording.md (grep -n '^#' first), ${MAIN}/docs/claude/integration.md (the video recording part), ${MAIN}/docs/claude/recording.md, ${MAIN}/CLAUDE.md pitfalls 25 and 37, and the recorder's source (grep -rn "class VideoRecorder" ${MAIN}/src). What size is the composition canvas (the picture that would be shrunk)? Which encoders does the recorder know? Does it use the Mac's hardware encoder? (2) This machine: sysctl -n machdep.cpu.brand_string, sysctl -n hw.ncpu, system_profiler SPDisplaysDataType (read-only looks; nothing else). (3) On the web: what the Mac's hardware video encoder costs at small picture sizes; how a recording is written so that a crash loses at most the last few seconds (segmented files, fragmented MP4) -- FFmpeg's own documentation. (4) Work it out for a 1920 x 1080 canvas at 60 pictures a second: "1/4" and "1/8" read both ways (a quarter of each side = 480 x 270; a quarter of the area = 960 x 540; an eighth of each side = 240 x 135; an eighth of the area = about 680 x 382), at 30 and at 60 pictures a second: megabytes per 10 and per 20 minutes at a sensible quality, and the load on the processor and the graphics card as a RANGE, labelled ESTIMATE, with what it rests on. Say what would have to be measured on his Mac, with a show running, before anyone promises a number. (5) Does the sound go into it? (his words elsewhere: grep "record show" and "Video box" and "48 b" in ${BIND}).` },
  { key: 'tempo-auto', type: 'architect', model: 'opus', effort: 'high', file: 'answer-tempo-auto.md',
    task: `HIS QUESTION (L32, verbatim; it is his reply to question 189): "189 is it possible to correct the app listening to the music if it is off by two BPM's and keep it on automatic mode, or will that correction necessitate going back to manual mode? This is a question for you.  Same question if I correct, where the one is."
Question 189 as it was shown to him is in ${RS}/slice-A.md (grep -n "Question 189"); his other tempo words of this message are L12, L13, L14, L17, L18, L31 of ${NUM} (read the whole message once). ANSWER IT AS THE ARCHITECT OF THE TEMPO ROW. (1) How the app listens now and what a correction by hand does to it now, for your own understanding: ${R5}/area-tempo.md and ${R5}/facts-takes-bpm-fire.md (grep -n '^#' first; their "## CORRECTIONS" blocks win), ${MAIN}/docs/claude/analysis.md (rhythm), and the source they cite (BPMTracker and where the top bar's Manual tick box, Tap, the tempo steps, the nudge and Resync act on it). The adopted tempo-row rulings: the "HARMONY ADOPTION" blocks at the END of ${RB}/plan-nudge-row2.md (grep -n "HARMONY ADOPTION"; read only those blocks). (2) Then answer, separately for (a) a tempo that is off by about two BPM and (b) the place of the "1": CAN his correction hold while the app keeps listening? Describe each workable way in terms of what he would see and hear on stage -- for example: the app keeps listening but only close around his number; his "1" is kept as an offset that the listening carries; the listening is told the tempo is double or half -- and for each: what could go wrong on stage (it drifts back, it jumps, it fights him), how big the change to the listening is (S / M / L), and what must be measured with real tracks before it can be promised (he will bring three DJ tracks). (3) Your recommendation, and the ONE question that is really his.` },
]
const questionPrompt = Q => `${WHO}

YOUR TASK: ANSWER A QUESTION BORIS ASKED HARMONY in his message of 2026-10-07 (the whole message, one line per line: ${NUM}). ${Q.task}

${PAGERULES}

${ANSWERFMT}

${RULES}

${CODE}

REPORT_FILE: ${RS}/${Q.file}
TURN BUDGET: ${Q.type === 'architect' ? 'you stop silently at 120 turns. ' : ''}a skeleton of the paper within your first 10 tool calls; rewrite it about every 10 calls; final within 60.
RETURN: status, report_path, answer_for_boris (the section's text, at most 150 words), facts_verified (count), unknowns (at most 6, each at most 200 characters), question_for_him (the one question with its options, or "none"), summary (at most 400 characters).`

const milkPrompt = `${WHO}

YOUR TASK: WRITE THE MILKDROP DOCUMENT. His words (L126 of ${NUM}, verbatim): "R202 we will design a much smarter system for doing Milk drop and we will do that as a dedicated session where I will design the UI and how we will use it but not right now. I want you to create a dedicated document with how milk drop functions currently and that's it for this upcoming build." So: ONE dedicated document that says how MilkDrop works in Audio-DNA as it is NOW (this one document is the place where "today" belongs: it is what he asked for). It is the base for the later session in which he designs the smarter system. Nothing of MilkDrop is changed or built.

READ: ${R5}/area-sources-auto.md (grep -n '^#' first; its "## CORRECTIONS" block wins over the lines above), ${RS}/slice-K.md (R202, R212 and the other MilkDrop items as they were shown to him), ${H}/milkdrop-autoload-rootcause.md, ${MAIN}/CLAUDE.md pitfall 66 with its full text in ${MAIN}/docs/claude/pitfalls.md, ${MAIN}/docs/claude/*.md (grep -n -i "milkdrop\\|projectm"), his earlier words (grep -n -i "milkdrop\\|milk drop\\|jukebox" ${BIND}), and THE SOURCE: grep -rn -i -l "projectm\\|milkdrop" ${MAIN}/src ${MAIN}/CMakeLists.txt -- read every file it names that carries MilkDrop behaviour (the source class, its browser, the Jukebox, a clip's playlist, how presets are found on disk, what is saved with a show, the keys and mapping entries, the REST and OSC entries).

THE DOCUMENT (markdown), three parts:
# MilkDrop in Audio-DNA -- how it works now (written <stamp from date>, s-rta-1007; source at ${HEAD})
## PART 1 -- What you can do with it now  (FOR BORIS: plain words, no code words. Every control by the words it shows on screen: where it is, what it does, what it does not do although its label suggests it. How a MilkDrop picture gets into a cell; how its presets are found, browsed, changed by hand, by the Jukebox, by a clip's own playlist; Random and Bag; what follows the beat and what does not; what is kept when a show is saved; what the keyboard and MIDI mapping can reach. Then a short list "What is odd or broken now", each line one sentence.)
## PART 2 -- How it is built  (FOR THE BUILD: classes, files, threads, how it draws (pitfall 66), where presets and textures live on disk, settings keys, what is saved in the show file, the library and its version, every known debt; each line VERIFIED file:line, or INFERRED from what)
## PART 3 -- What is not known  (UNKNOWN lines, each with the cheapest way to find out; anything that could only be learned by running the app is listed here, not guessed)

${RULES}

${CODE}

REPORT_FILE: ${RS}/milkdrop-current.md
TURN BUDGET: a skeleton with the three parts within your first 12 tool calls; rewrite it about every 10 calls; final within 70.
RETURN: status, report_path, answer_for_boris (a summary of PART 1 in at most 100 words), facts_verified (count of VERIFIED lines), unknowns (at most 8, each at most 200 characters), question_for_him ("none"), summary (at most 400 characters).`

const checkApply = (key, name) => `${WHO}

YOUR TASK: RE-CHECK ONE PAPER, blind and hard. An architect has applied Boris's answers of 2026-10-07 to topic ${key}, "${name}", in the paper ${RS}/apply-${key}.md. The build and Boris's next page will stand on it. Your job is to find where it is WRONG: where a RULE says more, less or something else than his words; where an assumption of the architect is written as if it were his word; where an item is missing; where an assumption is put to him although his words already settle it; where a real gap is hidden.

DO IT IN THIS ORDER (re-derive, do not re-read):
1. Read ${NUM} WHOLE (his message, 136 lines) and ${RS}/slice-${key}.md WHOLE (the topic's items as they were shown to him, each with the lines of his that name it).
2. BEFORE you open the paper: for every line of his that names an item of this slice, write into your own file one sentence -- what does this line change, and in which part of the item? Do the same for every general point that lands here. This is your own reading; it is what you compare with.
3. Now read the paper WHOLE. Run: python3 ${RS}/wf/lint_apply.py ${RS}/apply-${key}.md ${key} and note its output.
4. Compare, item by item. A finding is MUST when: a RULE contradicts his words or leaves out something he said; a RULE holds a decision his words do not make and no @@ASSUME names it; a STATUS is wrong (for example STANDS although words of his elsewhere in the message change it -- his L9: an explanation given once answers the repeats); a quote of his is not verbatim or cites the wrong L number (check every quote against ${NUM}); a RULE or an assumption's TEXT describes the app as it is now (his L1 forbids "today" in what he reads); an @@ASSUME marked ASK: YES is already settled by words of his (cite them -- also look in ${BIND} for his earlier words: grep); an @@ASSUME that should be ASK: YES by the three tests (not settled by his words; seen on stage or costly to undo; about what the app does, not how it looks) is marked LINE or NO; an @@ANSWER does not answer what he asked, or is not plain. A finding is SHOULD when the paper is right but unclear, too long for him, or uses a word he would have to look up. Do not report matters of taste.
5. Look for what is MISSING: a line of his that lands in this topic and that no item carries; an edge that a builder would have to guess.
The discipline the architect was held to, so that you can hold the paper to it:
${DISCIPLINE}

${PAGERULES}

YOUR PAPER (markdown): ## MY OWN READING OF HIS LINES (step 2) / ## FINDINGS (one bullet each: severity, the item id, what is wrong with his words quoted and their L number, and the fix as the exact replacement text where you can give it) / ## MISSING / ## WHAT I CHECKED AND FOUND RIGHT (ids only) / ## LINT OUTPUT.

${RULES}

REPORT_FILE: ${RS}/check-${key}.md
RETURN: verdict (SOUND = nothing wrong; SOUND_WITH_CORRECTIONS = findings, the paper is usable with them; UNRELIABLE = so wrong that it should be written again), paper_path (your own file), checked (count of items compared), findings (every MUST and SHOULD; each field at most 500 characters), missed (at most 10, each at most 300 characters), summary (at most 400 characters).`

const checkAnswer = Q => `${WHO}

YOUR TASK: RE-CHECK ONE PAPER, hard. It answers a question Boris asked Harmony: ${RS}/${Q.file}. What he asked is in his message ${NUM} (read it whole once; the paper's task was: ${Q.short}). Harmony will put the paper's "ANSWER FOR BORIS" on his page almost as it stands, so a wrong fact or an over-promise there is costly.

DO: (1) Read the paper whole. (2) Re-check its load-bearing facts from the source, not from the paper: at least ten of them, and every one that the "ANSWER FOR BORIS" leans on -- open the URL and find the quoted words, or open the file at the cited line; a fact you cannot confirm is a finding. (3) Attack the answer: does it answer exactly what he asked? Does it promise what nobody has measured? Is a number given without saying that it is an estimate? Is there a simpler or safer option the paper missed? Is the recommendation the one you would stand behind, and if not, what is? (4) Is the "ANSWER FOR BORIS" plain -- no word he would have to look up, at most 150 words, the answer first?

${PAGERULES}

YOUR PAPER (markdown): ## FACTS RE-CHECKED (each: confirmed / wrong / could not confirm, with what you saw) / ## FINDINGS (severity MUST or SHOULD, what is wrong, the fix) / ## A BETTER "ANSWER FOR BORIS" (only if the paper's is wrong or not plain: your full replacement text, at most 150 words).

${RULES}

${CODE}

REPORT_FILE: ${RS}/check-${Q.key}.md
RETURN: verdict (SOUND / SOUND_WITH_CORRECTIONS / UNRELIABLE), paper_path (your own file), checked (count of facts re-checked), findings (every MUST and SHOULD; item = the paper's section; each field at most 500 characters), missed (at most 6), summary (at most 400 characters).`

const checkMilk = `${WHO}

YOUR TASK: RE-CHECK AND MEND THE MILKDROP DOCUMENT ${RS}/milkdrop-current.md. Boris asked for "a dedicated document with how milk drop functions currently"; a later session, in which he designs a smarter MilkDrop system, will stand on it. A wrong line there sends that design the wrong way.

DO: (1) Read the document whole. (2) Re-check from the SOURCE, not from the document: every line of PART 1 that says what a control does (find the control's on-screen words in the source and follow what it calls), and at least fifteen lines of PART 2 at their file:line. grep -rn -i -l "projectm\\|milkdrop" ${MAIN}/src ${MAIN}/CMakeLists.txt to see whether a file that carries MilkDrop behaviour was not read at all. (3) What is MISSING: a control, a setting, a saved field, a known debt (${H}/milkdrop-autoload-rootcause.md; ${MAIN}/docs/claude/pitfalls.md, pitfall 66; grep -n -i "milkdrop" ${H}/gotchas.md ${R5}/area-sources-auto.md). (4) PART 1 is for Boris: plain words, every control by the words it shows on screen, no class or file names. (5) MEND the document in place: this ONE document you may edit (replace a wrong line with the right one; add what is missing; move anything guessed into PART 3). Then add at its end a section "## RE-CHECK (s-rta-1007)" that lists each change you made in one line, and what you checked and found right.

${RULES}

${CODE}

REPORT_FILE: ${RS}/check-milkdrop.md (your findings; the document ${RS}/milkdrop-current.md is the only other file you may touch, and only for the mends named above)
RETURN: verdict (SOUND = nothing was wrong; SOUND_WITH_CORRECTIONS = you mended it and it is now right as far as you checked; UNRELIABLE = it needs writing again), paper_path (your own file), checked (count of lines re-checked), findings (each mend or open defect: severity, item = the part, issue, fix; each field at most 400 characters), missed (at most 8), summary (at most 400 characters).`

// ---------------- stage 1: apply + answers + the MilkDrop document (a FIXED literal order: the cache key of a resume depends on call order) ----------------
phase('Apply')
const S1 = [
  ...TOPICS.map(T => ({ kind: 'apply', key: T.key, run: () => agent(applyPrompt(T), { agentType: 'architect', model: 'opus', effort: 'high', schema: AS, phase: 'Apply', label: 'apply:' + T.key }) })),
  { kind: 'apply', key: 'X', run: () => agent(loosePrompt, { agentType: 'architect', model: 'opus', effort: 'high', schema: AS, phase: 'Apply', label: 'apply:X' }) },
  ...QUESTIONS.map(Q => ({ kind: 'answer', key: Q.key, run: () => agent(questionPrompt(Q), { agentType: Q.type, model: Q.model, effort: Q.effort, schema: QS, phase: 'Apply', label: 'answer:' + Q.key }) })),
  { kind: 'milk', key: 'milkdrop', run: () => agent(milkPrompt, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: QS, phase: 'Apply', label: 'doc:milkdrop' }) },
]
const r1 = await parallel(S1.map(s => s.run))
const one = S1.map((s, i) => ({ kind: s.kind, key: s.key, res: r1[i] }))
const back1 = one.filter(o => o.res && o.res.status !== 'BLOCKED')
log('stage 1: ' + back1.length + ' of ' + S1.length + ' papers back; ' + one.map(o => o.key + '=' + (o.res ? o.res.status : 'NONE')).join(' '))

// ---------------- stage 2: one blind checker per paper that came back ----------------
phase('Check')
const SHORT = { codec: 'whether Audio-DNA should build a video codec of its own like Resolume\'s DXV 3, and whether that can be done reliably', lowres: 'how much of the computer a very low-resolution recording of the show in 10 or 20 minute chunks would take', 'tempo-auto': 'whether the app\'s listening can be corrected by hand (a tempo off by two BPM; the place of the "1") and stay in automatic mode' }
const NAMES = Object.fromEntries(TOPICS.map(T => [T.key, T.name]).concat([['X', 'the page\'s loose lists (statements that disagreed, things not established, things still unsure)']]))
const S2 = one.map(o => {
  if (!o.res || o.res.status === 'BLOCKED') return { key: o.key, kind: o.kind, run: null }
  if (o.kind === 'apply') return { key: o.key, kind: o.kind, run: () => agent(checkApply(o.key, NAMES[o.key]), { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check', label: 'check:' + o.key }) }
  if (o.kind === 'answer') { const Q = QUESTIONS.find(q => q.key === o.key); return { key: o.key, kind: o.kind, run: () => agent(checkAnswer({ key: Q.key, file: Q.file, short: SHORT[Q.key] }), { agentType: Q.key === 'tempo-auto' ? 'general-purpose' : 'researcher', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check', label: 'check:' + o.key }) } }
  return { key: o.key, kind: o.kind, run: () => agent(checkMilk, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check', label: 'check:milkdrop' }) }
})
const r2 = await parallel(S2.map(s => s.run ? s.run : async () => null))
const cut = (s, n) => { s = String(s == null ? '' : s); return s.length > n ? s.slice(0, n) + ' [cut]' : s }
const out = one.map((o, i) => {
  const c = r2[i]
  return {
    key: o.key, kind: o.kind,
    status: o.res ? o.res.status : 'NONE',
    lint: o.res && o.res.lint ? cut(o.res.lint, 120) : undefined,
    counts: o.res && o.kind === 'apply' ? { items: o.res.items, yes: o.res.assume_yes, line: o.res.assume_line, no: o.res.assume_no, answers: o.res.answers, names: o.res.names } : undefined,
    conflicts: o.res && o.res.conflicts ? o.res.conflicts.slice(0, 4).map(x => cut(x, 180)) : undefined,
    top: o.res && o.res.top_assumptions ? o.res.top_assumptions.slice(0, 3).map(x => cut(x, 160)) : undefined,
    summary: o.res ? cut(o.res.summary, 260) : undefined,
    check: c ? { verdict: c.verdict, checked: c.checked, must: (c.findings || []).filter(f => f.severity === 'MUST').length, should: (c.findings || []).filter(f => f.severity === 'SHOULD').length, missed: (c.missed || []).length, summary: cut(c.summary, 260) } : 'NONE',
  }
})
const papers = out.filter(o => o.status !== 'NONE' && o.status !== 'BLOCKED').length
const checks = out.filter(o => o.check !== 'NONE').length
log('stage 2: ' + checks + ' of ' + papers + ' checks back')
return { papers_sent: S1.length, papers_back: papers, checks_back: checks, complete: papers === S1.length && checks === papers, out }
