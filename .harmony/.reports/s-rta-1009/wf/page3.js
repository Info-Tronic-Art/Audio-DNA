export const meta = {
  name: 'rta-1009-page3',
  description: 'Audio-DNA s-rta-1009: the list for Boris\'s page 3. One ruling across the topics writes the answers he is owed and one item per assumption his answers opened; six checkers read it, each through one lens (his words, coverage, plain words, his chair, logic, the truth of the answers); a second ruling writes edit blocks. Paper only: nothing is built.',
  phases: [{ title: 'List' }, { title: 'Check' }, { title: 'Rule' }],
}
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const H = MAIN + '/.harmony'
const RS = H + '/.reports/s-rta-1009'
const R7 = H + '/.reports/s-rta-1007'
const R5 = H + '/.reports/s-rta-1005'
const BIND = H + '/binding-decisions.md'
const BACKLOG = H + '/boris-feedback-backlog.md'
const NAMES = H + '/NAMES.md'
const ANS = RS + '/answers-by-item.md'
const PAGE = R7 + '/boris-page-2.txt'
const HEAD = '33989a8'

// Boris's words used in the prompts: every value is checked to be a verbatim piece of one of his messages before launch (wf/check_quotes.py)
const Q = {
  hold: "Don’t build anything till you are clear and 100% sure of what everything means.",
  today: "Don’t list what is happening today as we are discussing a major change. Keep the ‘today’ in your own notes so you know what to change.",
  focused: "It would be best if you just asked me focused questions on any assumption that you're making. Breaking it into the R’s and the questions is a lot more material to read for me.",
  layout: "there are new functionalities, and I am OK with you laying them out wherever you can in the correct area. If you have a question where they get laid out, ask me, but there will be a very big UI redesign once all of the functions have been built and everything works correctly.",
  repeats: "there are some repeats in your document, and I neglected to explain every time. If I have explained something, use it to answer questions not answered.",
  stopclips: "Stop removes all clips from all layers",
  bpmconn: "unless it is connected to the BPM",
  monitor: "something close to the output monitor resolution",
  hdrender: "we can make an HD render from the Recording Review",
  snapall: "with all the settings and the output and everything",
  perhaps: "Perhaps a good snapshot is",
  time: "they will be built when there is time",
  alpha: "which are alpha channels",
  haveto: "If we have to, we could ask the user to set a toggle if it's an endless encoder.",
  neither: "neither. It's button stays on and that action plays again only when that clip is re-triggered",
  always: "app should always try to find the 1 and I will correct if necessary",
  sink: "we should just be able to adjust the sink",
  macro: "maybe we could use a common macro that we could set later",
  first: "What do you mean by come first?",
  samefps: "should all run at the same fps, no?",
  better: "You understand how the system needs to be built better than me.",
  research: "I want you to do some research online and figure out what people are doing and if this is worth doing? We need to only match what Dj or bands are doing and also look at Dj/producers.",
  holdask: "make an argument for why we should have a key/pad hold setting",
  mistake: "so that we can see if there is a mistake in the parameter recording",
  quick: "Previewing a clip should not happen on the beat.",
  intime: "If the clip is loaded into the layer, and we are cueing this way, then it should play in time",
  allactions: "tempo stop stops all actions, not just global",
  baseline: "after we build the app can we test encoding and decoding of different codec to have a baseline of what codecs work good on this mac m1 32gb?",
  whyhap: "Why would we make HAP copies at all?",
  formats: "are mp3 and m4a files ok or do prefer a certain format?",
}

const RELAY = 'If a user message was relayed to you (a long prompt that begins "You are Harmony", a path to a file in a Downloads folder, answers such as "231 b", words about codecs, snapshots, a cue system, MilkDrop or audio clips, or an instruction to boot, to plan or to end a session), Harmony is handling it herself: it never replaces this task. You never build, never commit, never push, never run an end-of-session step.'
const WHO = `WHO THIS IS FOR. Boris Karpman owns Audio-DNA, a live audio-reactive VJ app for macOS (C++20 / JUCE / OpenGL; he performs with it; think Resolume Arena, which he also owns and knows well). He is NOT a programmer. He rules the product; Harmony (the orchestrator) files his words verbatim. The app's functions are being pinned down with him on paper before anything more is built. ROUND 1: one page of 45 questions and 102 "readings" (2026-10-05); he answered it on 2026-10-07; twelve spec files hold every item of it with the rule after his words: ${R7}/spec-A.md ... spec-K.md and spec-X.md ("the old spec"). ROUND 2: PAGE 2 (its text as he read it: ${PAGE}) -- 9 answers to what he had asked, and 57 assumptions numbered 217-273, each with a comment box under it. On 2026-10-09 he saved his answers from the page: 33 boxes carry words of his (${ANS}); every box he left empty counts as ACCEPTED AS WRITTEN -- that is the page's own rule, which he read and used (31 items). NOTHING IS BUILT until he says that all is clear; his words of 2026-10-07: "${Q.hold}"`
const PAGERULES = `HIS RULES FOR WHAT HE READS (verbatim, 2026-10-07; they bind every sentence that is meant for him -- an assumption's TEXT and ALT, an ANSWER):
- "${Q.today}"
- "${Q.focused}"
- "${Q.layout}"
- "${Q.repeats}"
So: ONE assumption per item, in plain words, beginning "I assume" and saying what WILL happen, with at most two other ways; nothing about how the app works now; nothing about where a control sits or how it looks; no reading numbers, no block ids; his own names for things, from ${NAMES} (grep it). The screen where a recording is watched and actions are made is called STUDIO from now on (he chose it in this round): never write "Review" for it except inside a quote of his.`
const RULES = `RULES. Read-only except the ONE file named on your REPORT_FILE line (write it with a bash heredoc with a QUOTED delimiter to the literal absolute path; create a skeleton early, then fill it and rewrite or append as you learn). Harmony constraint: never build, never run a test or a probe, never launch the app, never touch or operate a running Audio-DNA or Resolume Arena on this machine (both are Boris's), never use lldb / sample / dtrace / screencapture, never cd in a command (absolute paths, git -C), never commit. A time stamp is never typed: take it from date in the same command that writes it. Boris is quoted ONLY verbatim: his words of this round from ${ANS} (the HIS-WORDS lines; cite the BF number that stands in the same block), his earlier words from ${BIND} (his words are inside quotes; the text after "->" is Harmony's consequence text, NOT his) and ${BACKLOG}. Big files are indexed: grep -n '^## ' first, then read ranges; one block of a spec file: grep -n -A 7 "^@@ITEM <id>" <file>. NOTHING IS MEASURED: a fact about the app that you read in its program text is "read, not run"; an estimate is called an estimate. An honest UNKNOWN costs one question; a confident wrong line costs a build stage. ${RELAY}`
const CODE = `THE APP. The repository's main working tree is ${MAIN} (HEAD ${HEAD}; no builder is editing it: the build is on hold). Do NOT read the lane worktrees under ${MAIN}/.claude/worktrees. Docs that index the code: ${MAIN}/CLAUDE.md (the pitfall index), ${MAIN}/docs/claude/*.md, ${H}/APP-INVENTORY.md. Fact sheets on how the app works now (read, not run; s-rta-1005): ${R5}/area-*.md and ${R5}/facts-*.md.`

// ---- page 3: the list for Boris (this body is joined to the shared constants of apply3.js by wf/mk_page3.py)
const Q3 = {
  mastercue: "what and where is the master cue?",
  beep: "What does shifting the beep mean to you?",
}
const A3 = RS + '/assume3-all.md'
const ANS3 = RS + '/answers3-all.md'
const LED = RS + '/ledger3.md'
const L1 = RS + '/page3-round1.md'
const EDITS = RS + '/page3-edits.md'

const LISTFORMAT = `THE LIST'S FORMAT (a script parses it and renders his page from it: keep it exact). Blocks; a block opens with a marker line at column 0, holds one field per line as "FIELD: text" -- every field ONE physical line -- and closes with a line that reads @@END.

@@PAGE-ANSWER <key>          (an answer he is owed; shown first on the page, in the order of the list)
ASKED: <his own words, verbatim, in quotes -- from ${ANS}>
TITLE: <at most 8 words>
TEXT: <the answer: plain words, the answer in the first sentence, at most 130 words (the one on the hold setting: 150, and it may carry up to three links as [label](https://...))>
ITEM: <the provisional ids of the items it leads to, for example "P3, P4"; or none>
@@END
@@PAGE-ITEM P<n>             (one assumption; P1, P2 ... are provisional ids: a script gives the final numbers from 274)
TOPIC: <one letter A to K>
KIND: <ASK = a card of its own with other ways b / c: a wrong guess would be seen on stage or be costly to undo | LINE = one line in the small list at the end>
TEXT: <ONE assumption: begins "I assume", says what WILL happen, plain words; ASK at most 45 words, LINE at most 30. A question mark in TEXT makes the item one that silence can NOT accept: use "?" only where he must really tell Harmony something (which one, how many)>
B: <another way it could be, one sentence, at most 30 words; or none>
C: <a third way only if it is real; or none>
FROM: <the assumption ids it is made from (for example B3-2, F3-1); and "again: 222" when it is an item of page 2 put to him again>
@@END
@@TRIAGE <assumption id>     (one for EVERY @@ASSUME block of ${A3} whose ASK is YES or LINE)
TO: <P<n> = it is on the page as that item | MERGED = said by another item (name it in WHY) | SETTLED = words of his settle it (quote them in WHY with the BF number or the line of ${BIND}) | INTERNAL = technical, a look, or a measurement: not his to read>
WHY: <one sentence>
@@END`

const PAGE3RULES = `WHAT PAGE 2 TAUGHT (binding for page 3).
- THREE ITEMS OF PAGE 2 CAME BACK AS QUESTIONS because they used words he did not know or that were Harmony's coinage: "master cue" (222: "${Q3.mastercue}"), "shifts the beat" (250: "${Q3.beep}"), "comes first" (252: "${Q.first}"). An item of page 3 uses ONLY words he uses himself, or says in passing what a word means ("the 1 -- the first beat of the bar"). A name that is Harmony's pick (marked so in ${NAMES}) is never used bare.
- HIS EXPLICIT ANSWER IS THE DEFAULT of every item; a recommendation of Harmony's is way b and the answer above it says so.
- NOTHING HE HAS ANSWERED IS ASKED AGAIN. His 33 boxes and his earlier words (${BIND}) settle what they settle; an explanation he gave once answers its repeats ("${Q.repeats}").
- AN ITEM OF PAGE 2 PUT TO HIM AGAIN gets a NEW provisional id and says the thing itself in plain words; it never points back ("see 222").
- WHERE HIS NEWEST WORDS GO AGAINST SOMETHING HE SAID OR ACCEPTED BEFORE, he is told once, as a LINE item he can strike: "I assume your newest words hold: ... (before, you had ...)".
- NOTHING IS MEASURED. No answer and no item claims a number or a fact about the app that nobody has run; "an estimate", "as far as I can see without running it", "not measured".
- THE FEWEST ITEMS THAT HIDE NO REAL DOUBT. He wrote: "${Q.focused}" Two assumptions that are one decision are one item. A technical choice, a look, a place on screen, a measurement: not on the page (TRIAGE INTERNAL). But a real doubt about what the app DOES that he or the audience would see is on the page, however many there are.`

const listPrompt = `${WHO}

YOUR TASK: MAKE THE LIST FOR BORIS'S PAGE 3. His answers to page 2 are applied, checked and ruled topic by topic. What is left: the answers he is owed, and the assumptions his answers opened. You rule ACROSS the topics and write the one list his next page is rendered from: the answers first, then one item per open assumption that he must see, and a triage line for every assumption that does not get an item. He will read every word of it; he is not a programmer; he has little time.

READ, in this order, each WHOLE unless a size is given:
1. ${ANS} -- his 33 boxes (his words are the measure).
2. ${PAGE} -- page 2 as he read it: its tone and length are the model; its three failures are named below.
3. ${A3} -- every assumption that is open after the rulings, topic by topic, with each topic ruling's notes "for the page ruling", the CONFLICTS (his newest words against earlier ones) and what reaches across topics. Run wc -c on it first; if it is over 150,000 characters read it topic by topic (grep -n '^## TOPIC').
4. ${ANS3} -- the answers he is owed, as ruled (the @@ANSWER blocks), the technical paper's recommendation on item 252 and the paper on the hold setting.
5. On demand: ${LED} (one line per item of page 2 with the rule now: grep by number), ${RS}/spec-<letter>.md (one block: grep -n -A 7 "^@@ITEM <id>"), ${NAMES} (grep every name you use; it is being brought up to date while you work, so the names this round fixed -- ${RS}/names3-all.md, 41 blocks -- win over it wherever the two differ: the screen is Studio, never Review; a Snapshot is a save of the whole show, no longer a still picture), ${BIND} (his earlier words, inside quotes).

WHAT TO WRITE.
A. THE ANSWERS. One @@PAGE-ANSWER for each thing he asked or ordered in this round, in this order: what the master cue is and where (key 222); what "shifting the beat" meant (250); what "the output comes first" meant and whether the three pictures run at the same frame rate (252); why HAP copies at all (239); the test of codecs after the build (codec); the size and rate of the low-resolution show recording (lowres); the research he ordered on the hold setting (246-hold); mp3 and m4a (general). Start from the ruled @@ANSWER blocks; make each one true, plain and short; where an answer leads to an item, ITEM names it. If two answers are one thought (239 and codec), they may be ONE block under the key of the first.
B. THE ITEMS. Go through ${A3} topic by topic. For every @@ASSUME with ASK: YES or LINE decide: an item of its own (KIND ASK or LINE), merged into another item, settled by his words, or internal. Across topics, find the same doubt raised twice (what the tempo stop does appears in A, D, I and K; the name Studio in several) and make it ONE item in the topic that owns it. Order: by topic A to K; inside a topic the ASK items first.
C. THE TRIAGE. One @@TRIAGE block for every @@ASSUME with ASK: YES or LINE -- none may vanish.
D. Under the heading "## TO SAY IN CHAT" at most 8 lines for Harmony: what is still unmeasured and now waits for after the build, by his own words; anything he must be told that is not an item.
E. Under "## COUNTS": answers, ASK items, LINE items, and your honest estimate of his reading time in minutes.

${PAGE3RULES}

${PAGERULES}

${LISTFORMAT}

HOW TO WRITE IT. Create the file with the heading "# PAGE 3 -- the list, round 1 (s-rta-1009)" first. Then APPEND in commands of at most about 120 lines each (bash heredoc with a QUOTED delimiter, >> to the literal absolute path): the answers, then the items topic by topic, then the triage blocks, then the two last sections. Never put the list into one command: an answer that grows too long is cut off and lost. THE LAST STEP: python3 ${RS}/wf/page3_build.py --lint ${L1} -- fix every line marked "!" and run it again until its last line begins "LINT: OK".

TURN BUDGET: your agent type stops SILENTLY at 120 turns: the file with the answers by turn 25, all items by turn 60, done by turn 85.

${RULES}

REPORT_FILE: ${L1}
RETURN: status (DONE only when the lint's last line begins "LINT: OK"), report_path, answers (count), ask (count of KIND ASK), line (count of KIND LINE), triage (count of @@TRIAGE), minutes (his reading time, your estimate), lint (the lint's LAST line, verbatim), summary (at most 400 characters).`

const LS = { type: 'object', required: ['status', 'report_path', 'answers', 'ask', 'line', 'triage', 'minutes', 'lint', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, answers: { type: 'number' }, ask: { type: 'number' }, line: { type: 'number' }, triage: { type: 'number' }, minutes: { type: 'number' }, lint: { type: 'string' }, summary: { type: 'string' } } }
const PC = { type: 'object', required: ['verdict', 'paper_path', 'paper_bytes', 'must', 'should', 'summary'], properties: { verdict: { type: 'string', enum: ['PASS', 'PASS_WITH_FIXES', 'FAIL'] }, paper_path: { type: 'string' }, paper_bytes: { type: 'number' }, must: { type: 'number' }, should: { type: 'number' }, summary: { type: 'string' } } }
const R2 = { type: 'object', required: ['status', 'report_path', 'accepted', 'rejected', 'edits', 'lint', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, accepted: { type: 'number' }, rejected: { type: 'number' }, edits: { type: 'number' }, papers_ruled: { type: 'number' }, lint: { type: 'string' }, summary: { type: 'string' } } }

const SEATS = [
  { key: 'words', name: 'HIS WORDS', job: `Hold every answer and every item against HIS WORDS. Read ${ANS} WHOLE first and write, before you open the list, one line per box: what it decides. Then for each @@PAGE-ITEM: is it already settled by a box of his, or by earlier words of his (grep ${BIND}; his words are inside quotes)? Then it must not be asked: MUST. Is its TEXT -- the default he accepts by leaving the box empty -- HIS answer wherever he gave one, or does it smuggle in Harmony's preference? Does it stretch his words (he said "neither": does the item make him say more)? For each @@PAGE-ANSWER: is ASKED verbatim (check it letter by letter against ${ANS})? Where his newest words go against earlier ones: is he told once, and are both sides stated truly?` },
  { key: 'cover', name: 'COVERAGE', job: `Nothing may vanish. Read ${A3} WHOLE and list every @@ASSUME with ASK: YES or LINE. For each: is there a @@TRIAGE block, and does its reason HOLD? TO: SETTLED -- open the quoted words and check they really settle it. TO: MERGED -- read the item it was merged into: does that item really put THIS doubt to him? TO: INTERNAL -- would he or the audience see a wrong guess on stage? Then it is not internal: MUST. Then the other way round: every thing he asked or ordered in his 33 boxes (${ANS}) has its @@PAGE-ANSWER; every item of page 2 that the ledger ${LED} marks OPEN is put to him again or settled; every bullet under CONFLICTS in ${A3} is said to him once.` },
  { key: 'plain', name: 'PLAIN WORDS AND HIS PAGE RULES', job: `Read the list as an editor who knows his rules by heart. For every item: ONE assumption, not two joined by "and"; begins "I assume"; says what WILL happen; nothing about how the app works now (no "today", "currently", "at the moment", "still", "no longer" that implies now); nothing about where a control sits or how it looks; no reading numbers, no block ids, no file names; within its length (ASK 45 words, LINE 30, an answer 130; the hold answer 150). Every name: grep ${NAMES} -- his own name for the thing? The screen is Studio, never Review. Is b a REAL other way, or a straw man? Is a "?" used only where he must tell Harmony something? For every sentence longer than 25 words give a shorter one that says the same.` },
  { key: 'chair', name: "BORIS'S CHAIR", job: `Sit in his chair. He is a working VJ who knows Resolume Arena well; he is not a programmer; he dictates and reads fast; he has said the pages cost him focused time. Three items of page 2 came back as questions because of words he did not know ("master cue", "shifts the beat", "comes first"). Read ${PAGE} first to hear how page 2 spoke to him and where it lost him (his boxes: ${ANS}). Then read the list top to bottom ONCE at his speed and mark every place where he would stop: a word he never used himself (grep ${BIND} and ${BACKLOG} for it inside his quotes: if he never said it and the item does not explain it, MUST); a sentence he must read twice; an item where he cannot tell what he would SEE on stage; an answer whose first sentence does not answer; an item he would answer with a question. For each give the wording that would not stop him. Then give your honest reading time for the whole list in minutes, and say which five items you would cut first if he asked for a shorter page.` },
  { key: 'logic', name: 'LOGIC', job: `Find contradictions. (1) Between two items of the list (could he accept both defaults and get a rule that cannot be built?). (2) Between an item and something he ANSWERED or ACCEPTED on page 2: read ${LED} WHOLE (one line per item 217-273 with the rule now) and test every item of the list against it. (3) Between an answer and an item (the answer recommends one thing, the item's default says another, and nothing says why). (4) Inside an item: do TEXT and b exclude each other, and together cover the real choices? (5) Follow each default into a show: he presses the tempo bar's stop while a clip plays, an action runs and an audio file plays; he previews a clip by its name while the tempo is paused; he saves a snapshot in the middle of a show -- do the list's defaults, together with the ledger's rules, say what happens at every step, or is there a step nobody ruled? Name each such gap as MISSING with the item that should carry it.` },
  { key: 'truth', name: 'THE ANSWERS ARE TRUE', job: `Check every @@PAGE-ANSWER against the paper it stands on. 252: ${RS}/answer-252.md and ${RS}/check-252.md (both WHOLE) -- does the answer say what the papers support, is anything claimed that nobody measured, does it answer all three things he asked (what "comes first" means; whether the three pictures run at the same frame rate; what Harmony thinks)? 246-hold: ${RS}/answer-hold.md and ${RS}/check-hold.md (both WHOLE) -- is every claim about what DJs, bands and DJ / producers do backed by a finding the re-check CONFIRMED; open each link in the answer and confirm the page says it; is the verdict the paper's? general (mp3, m4a): find in the program text which audio file types the app opens (read-only; cite file:line; "read, not run") and compare. 239, codec, lowres, 222, 250: against the ruled @@ANSWER blocks in ${ANS3} and, for 222, against HIS earlier words on the master cue (grep -n -i "master cue" ${BIND} ${BACKLOG}): does the answer say truly what was his and what was Harmony's guess?` },
]
const seatPrompt = S => `${WHO}

YOUR TASK: CHECK THE LIST FOR HIS PAGE 3 through ONE lens: ${S.name}. The list (${L1}) holds the answers he is owed and the assumptions his answers opened; his page is rendered from it by a script, word for word. Other checkers hold other lenses; stay in yours and go deep.

THE LIST: read ${L1} WHOLE. Its blocks: @@PAGE-ANSWER (ASKED, TITLE, TEXT, ITEM), @@PAGE-ITEM P<n> (TOPIC, KIND ASK or LINE, TEXT, B, C, FROM), @@TRIAGE <assumption id> (TO, WHY).

YOUR LENS. ${S.job}

${PAGE3RULES}

${PAGERULES}

A finding is MUST when the page would mislead him, ask what he has answered, hide a real doubt, or stop him cold; SHOULD when it is right but could be plainer or shorter. No matters of taste. Give the FIX as the exact replacement text of the field wherever you can.

YOUR PAPER (markdown): ## WHAT I DID (five lines) / ## FINDINGS -- one line each, numbered F1, F2 ...: "F<n> | MUST or SHOULD | the block (PAGE-ITEM P7, PAGE-ANSWER 252, TRIAGE B3-4, MISSING) | what is wrong, with the evidence (his words quoted with the BF number, or file and line) | FIX: the exact replacement text" / ## FOUND RIGHT (block ids only).

${RULES}

REPORT_FILE: ${RS}/page3-check-${S.key}.md
RETURN: verdict (PASS | PASS_WITH_FIXES | FAIL), paper_path (your own file), paper_bytes (wc -c of it), must (count), should (count), summary (at most 400 characters: the findings that matter most).`

const rule2Prompt = keys => `${WHO}

YOUR TASK: THE SECOND RULING ON THE LIST FOR HIS PAGE 3. The list ${L1} was checked through ${keys.length} lenses; the checkers' papers are ${keys.map(k => RS + '/page3-check-' + k + '.md').join(', ')}. Rule every finding and write EDIT BLOCKS for what changes. You NEVER rewrite the list: a script lays your edit blocks over it, numbers the items from 274 and renders his page.

HOW.
1. Read ${ANS} WHOLE (his 33 boxes: the measure), ${PAGE} WHOLE (page 2 as he read it) and the list ${L1} WHOLE.
2. Then take ONE checker's paper at a time, in the order given above. Read it WHOLE; for every finding decide ACCEPT (the list changes as the checker says), MODIFY (it changes, differently) or REJECT (the list stands) -- by HIS WORDS, by the evidence the finding cites (open it when the finding is a MUST) and by the rules below; then APPEND to your file, before you open the next paper: the verdict lines of that paper ("words F3: ACCEPT -- one sentence") and the edit blocks they need. Two checkers often want the same block changed in different ways: before you write an edit block, look in your own file whether you already replaced that block, and if so write the block ONCE MORE, whole, with both changes (the LAST block with a given id wins).
3. EDIT BLOCKS (the same block format as the list; every field ONE physical line):
   - a block with the id of a block of the list REPLACES it, whole (write every field);
   - a NEW item: "@@PAGE-ITEM NEW-<k>" (k = 1, 2 ...); a new answer: "@@PAGE-ANSWER <new key>"; a new triage line: "@@TRIAGE <assumption id>";
   - to remove a block: "@@DROP <KIND> <id>" with a line "WHY: ..." and @@END (for example "@@DROP PAGE-ITEM P12"); when you drop an item, also write the @@TRIAGE blocks that say where its assumptions went.
4. After the last paper: read your own edit blocks once against each other (the same item changed twice? an answer's ITEM field that names a dropped item?), fix by appending the corrected block, and append "## TO SAY IN CHAT" (at most 8 lines for Harmony: what he must be told in chat that the page does not carry; what is unmeasured and now waits for after the build) and "## COUNTS" (accepted, rejected, edit blocks).
5. THE LAST STEP: python3 ${RS}/wf/page3_build.py --check ${L1} ${EDITS} -- it lays your blocks over the list in memory and lints the result; fix every line marked "!" by appending corrected blocks, and run it again until its last line begins "LINT: OK".

${PAGE3RULES}

${PAGERULES}

${LISTFORMAT}

HOW TO WRITE IT. Create ${EDITS} with the heading "# PAGE 3 -- the second ruling: verdicts and edit blocks (s-rta-1009)" first. APPEND after each paper, in commands of at most about 120 lines (bash heredoc with a QUOTED delimiter, >> to the literal absolute path). Never hold several papers' worth of blocks back for one big command: an answer that grows too long is cut off and lost.

TURN BUDGET: your agent type stops SILENTLY at 120 turns: about 12 turns a paper; done by turn 95.

${RULES}

REPORT_FILE: ${EDITS}
RETURN: status (DONE only when the check's last line begins "LINT: OK"), report_path, accepted (ACCEPT + MODIFY), rejected, edits (count of edit blocks), papers_ruled (how many of the checkers' papers you ruled), lint (the check's LAST line, verbatim), summary (at most 400 characters: what changed most).`

const out = { list: null, seats: {}, rule2: null, skipped: [] }
const trim = (s, n) => (s || '').slice(0, n)
const leg = (args && args.leg) || 'all'

phase('List')
const rl = await agent(listPrompt, { agentType: 'architect', model: 'opus', effort: 'max', schema: LS, phase: 'List', label: 'page3:list' })
out.list = rl ? { status: rl.status, answers: rl.answers, ask: rl.ask, line: rl.line, triage: rl.triage, minutes: rl.minutes, lint: trim(rl.lint, 200), summary: trim(rl.summary, 400) } : null
if (!rl || rl.status === 'BLOCKED') { out.skipped.push('the list did not come back -- no checks, no second ruling'); return out }
if (leg === 'list') return out

phase('Check')
const sr = await parallel(SEATS.map(S => () => agent(seatPrompt(S), { agentType: S.key === 'truth' ? 'researcher' : 'general-purpose', model: 'sonnet', effort: 'high', schema: PC, phase: 'Check', label: 'page3:check-' + S.key })))
SEATS.forEach((S, i) => { const r = sr[i]; out.seats[S.key] = r ? { verdict: r.verdict, bytes: r.paper_bytes, must: r.must, should: r.should, summary: trim(r.summary, 400) } : null; if (!r) out.skipped.push('check ' + S.key + ' did not come back') })
const back = SEATS.filter((S, i) => sr[i]).map(S => S.key)
log('Check: ' + back.length + ' of ' + SEATS.length + ' papers; MUST ' + SEATS.reduce((n, S, i) => n + ((sr[i] && sr[i].must) || 0), 0))
if (back.length < SEATS.length) { out.skipped.push('fewer papers came back than seats were sent (' + back.length + ' of ' + SEATS.length + ') -- the second ruling is NOT run: Harmony decides'); return out }

phase('Rule')
const r2 = await agent(rule2Prompt(back), { agentType: 'architect', model: 'opus', effort: 'max', schema: R2, phase: 'Rule', label: 'page3:rule2' })
out.rule2 = r2 ? { status: r2.status, accepted: r2.accepted, rejected: r2.rejected, edits: r2.edits, papers: r2.papers_ruled, lint: trim(r2.lint, 200), summary: trim(r2.summary, 400) } : null
return out
