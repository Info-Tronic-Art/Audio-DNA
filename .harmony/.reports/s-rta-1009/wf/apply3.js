export const meta = {
  name: 'rta-1009-apply3',
  description: 'Audio-DNA s-rta-1009: apply Boris\'s answers to page 2 (33 comments; 31 items accepted as written). Stage 1: one architect per topic writes the rule now for every page-2 item, the changes to the old spec, the answers he is owed and what is still assumed; three researchers look online at the hold setting; one architect answers his question on the previews\' frame rate. Stage 2: a blind checker per paper. Stage 3: a ruling per topic and the answer on the hold setting. Paper only: nothing is built.',
  phases: [{ title: 'Apply' }, { title: 'Check' }, { title: 'Rule' }],
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

const FORMAT = `THE PAPER'S FORMAT (a script parses it: keep it exact). Plain markdown headings, and under them BLOCKS. A block opens with a marker line at column 0, holds one field per line as "FIELD: text" -- every field is ONE physical line, however long, with no line break inside it -- and closes with a line that reads @@END.

# APPLY <letter> -- <topic name> (s-rta-1009, page 2)
## SUMMARY
(at most 10 lines: what his answers changed most in this topic)
## ITEMS
One block for EVERY number listed under "PAGE-2 ITEMS" in your slice, in that order:
@@ITEM <number>
TITLE: <at most 12 words>
STATUS: <ANSWERED | ACCEPTED | OPEN | REPLACED> <then, optionally, a few words>
HIS: <the BF number(s) of the box(es) that decide it, each with at most 12 of his words in quotes; for an item he left empty: "box left empty = accepted as written">
RULE: <how it works once built: plain words, complete enough to build from, as long as it needs; it never describes the app as it is now; it never says "I assume">
CHANGED: <what his words change against the item as he read it and against the old blocks named under MADE FROM; "INFERRED" where you read a slip of dictation; or "nothing: accepted as written">
TODAY: <Harmony's own notes: what the app does now and what has to change -- from the old block's TODAY line, a fact sheet's point id, or file:line you read yourself (read, not run); or "not checked">
@@END
## AMENDMENTS
Changes to the RULE line of an OLD @@ITEM of the s-rta-1007 specs. You never re-type an old item: you name the piece that changes.
@@AMEND <old item id> <running number, from 1 for each old item>
OLD: <an exact piece of that old RULE line, copied from the file, found exactly ONCE in it -- as short as will do: the sentence or clause that no longer holds; or the single word APPEND to add text at the end of the RULE>
NEW: <the text that takes its place (or is added): plain words, complete, never "today">
HIS: <the BF number(s) with at most 12 of his words in quotes; or "item <number> accepted as written">
WHY: <one sentence: what was said there before and why it no longer holds>
@@END
## ASSUMPTIONS
@@ASSUME <letter>3-<running number>      (a NEW assumption; an old one that changes keeps its old id, for example F-10)
ABOUT: <the page-2 item numbers and old item ids it belongs to>
TEXT: <the assumption as Boris will read it: one or two short sentences that begin "I assume", plain words, what WILL happen, at most 40 words>
WHY: <what in his words leaves this open, at most 30 words; cite BF numbers>
ALT: <the other way(s) it could be meant, one short sentence each, as "b) ... c) ..."; or "none">
IF-WRONG: <STAGE = he or the audience would see it | REBUILD = costly to undo once built | SMALL> <then a few words>
ASK: <YES | LINE | NO> <then a few words why>
@@END
@@DROP ASSUME <old id>                   (an old assumption, listed INTERNAL in your slice, that his words now settle or contradict)
WHY: <which words of his (BF number) or which accepted item settles it, and how>
@@END
## QUESTIONS BACK
One block for every key listed under "ANSWERS OWED BY THIS TOPIC'S PAPER" in your slice:
@@ANSWER <key>
HIS: <BF number and his question in quotes>
ANSWER: <for Boris: plain words, the answer in the first sentence, at most 110 words; nothing about how the app is now unless that is what he asked; nothing measured is claimed>
@@END
## NAMES
@@NAME <the name as it will read on screen and in talk>
MEANS: <what the thing is, one sentence>
SOURCE: <his words (BF number) | Harmony's pick | replaces the name ... in ${NAMES}>
@@END
## REACHES OTHER TOPICS
(bullets: words of his in YOUR boxes that change a rule another topic owns -- the topic letter, the old item id if you know it, what changes; and words from another topic's box that you applied here)
## CONFLICTS
(his newest words against something he said or accepted before, or against an adopted ruling: one bullet each, both sides quoted, the new one with its BF number, the old one with its place in ${BIND})
## NOT DONE / UNSURE
(what you could not settle, and the cheapest way to settle it)`

const DISCIPLINE = `HOW TO APPLY (the discipline; every status must be earned).
- His words win over the page's text and over the old spec, always. Quote him verbatim with the BF number.
- A page-2 item whose box he LEFT EMPTY: STATUS ACCEPTED. RULE = the item's text turned into a rule, complete enough to build from: say it in full, and where the old blocks named under MADE FROM already rule the surroundings, say "with @@ITEM <id> as it stands" rather than shortening a long rule into the item's one sentence. But TEST it against all 33 boxes first: where words of his in ANOTHER box say otherwise (his rule: "${Q.repeats}"), STATUS REPLACED, cite the BF number, RULE = his words.
- A box that holds a letter ("b", "c"): STATUS ANSWERED; RULE restates that way of the item in full, plus whatever words he added.
- A box that holds his own words: STATUS ANSWERED; RULE = what his words say, in full. Do not stretch them: a word like "neither" rejects the two other ways and says no more. What they leave open becomes an @@ASSUME.
- A box in which he ASKS BACK: STATUS OPEN; RULE begins "Waits on his answer:" and then gives the reading that will be put to him again. He gets an @@ANSWER in plain words (when your slice lists the key as owed) and an @@ASSUME with ASK: YES that puts the item to him again in a form his question shows he can answer.
- AMENDMENTS. After the items, go through every old @@ITEM that the answered and accepted items are MADE FROM, and every old @@ITEM of your topic's spec whose RULE touches the same matter: where its RULE line no longer says what holds, write @@AMEND blocks. Do NOT amend a RULE only to note that an assumption was accepted when its text already says the same thing. Do NOT amend an old item that lives in ANOTHER topic's spec file unless it is named under MADE FROM of one of your items; for the others write one bullet under REACHES OTHER TOPICS.
- THE OLD ASSUMPTIONS STILL OPEN (listed INTERNAL in your slice: never shown to him). Test each against his 33 boxes and against the 57 items as they now stand. Settled or contradicted now: @@DROP ASSUME <id>, and where contradicted also an @@AMEND of the item it belongs to. Changed but still open: an @@ASSUME block with the SAME id and the new text. Untouched: write nothing.
- NEVER FILL A GAP SILENTLY. Whenever you must decide something his words do not say in order to write a RULE that can be built, write the RULE with your best reading AND an @@ASSUME that names the gap. Where his words can be read two ways, both readings go into TEXT and ALT. An assumption written into a RULE as if it were his word is the worst defect this paper can have.
- HIS EXPLICIT ANSWER IS THE DEFAULT. A recommendation of Harmony's never flips an answer he gave: where you would advise otherwise, his answer is the RULE and the assumption's TEXT, and your advice is its way b.
- NEWEST WORDS AGAINST EARLIER ONES. Where a box of this round goes against something he said or accepted before (find his earlier words: the old item's HIS line, grep in ${BIND}), his newest words win; one bullet under CONFLICTS with both quotes, so that it can be said to him once more in one line.
- Slips of dictation are read kindly and flagged INFERRED in CHANGED ("the sink" = the sync; "the beep" = the beat; "preview-cure" = preview-cue; "look out later" = look at later; "the safe snapshot" = the saved snapshot).
- RULE and NEW lines never describe the app as it is now. What the app does now goes ONLY into TODAY. Do not spend your turns re-deriving "today": the job is his words.
- ASK on an @@ASSUME. YES only if all three hold: (1) no words of his, in any message, settle it; (2) a wrong guess would be seen on stage by him or the audience, or would cost more than a small change to undo once built; (3) it is about what the app DOES, not about where a control sits or how it looks. LINE = a real choice of Harmony's that he would most likely wave through: he gets it as one line that he can strike. NO = technical or internal: he would not care. Be strict, because he has too much to read; but never hide a real doubt: a focused question costs him ten seconds, a wrong build costs a stage.
- WHO OWNS WHAT (so that two papers do not rule one thing twice). A: what the tempo bar's play, pause and stop do to the beat, to clips and to layers -- also his sentence "${Q.stopclips}" (in box 273). D: what they do to actions (box 226). K: the audio file under stop and pause (box 273), MilkDrop, the audio clips he will bring. I: the effects that read the beat by themselves, blend modes, keying, masks, envelopes. B: previews and cue. E: record show, record to clip, the low-resolution show recording, Studio's functions. H: how a clip plays, and codecs (also the test of codecs after the build; E refers to it). F: Snapshot, cells, copy and paste, decks, the show file. G: output screens. J: the keyboard and MIDI mapping, the NAME Studio, the hold setting (item 246). X: the loose lists of the old spec-X. A paper applies his words -- from ANY box -- inside its own items and its own old blocks; for a rule that another topic owns it writes a bullet under REACHES OTHER TOPICS.`

const AS = { type: 'object', required: ['status', 'report_path', 'items', 'amends', 'assume_yes', 'assume_line', 'assume_no', 'lint', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, items: { type: 'number' }, amends: { type: 'number' }, assume_yes: { type: 'number' }, assume_line: { type: 'number' }, assume_no: { type: 'number' }, drops: { type: 'number' }, lint: { type: 'string' }, summary: { type: 'string' } } }
const CS = { type: 'object', required: ['verdict', 'paper_path', 'paper_bytes', 'checked', 'must', 'should', 'summary'], properties: { verdict: { type: 'string', enum: ['SOUND', 'SOUND_WITH_CORRECTIONS', 'UNRELIABLE'] }, paper_path: { type: 'string' }, paper_bytes: { type: 'number' }, checked: { type: 'number' }, must: { type: 'number' }, should: { type: 'number' }, summary: { type: 'string' } } }
const RSC = { type: 'object', required: ['status', 'report_path', 'accepted', 'rejected', 'own', 'blocks', 'ask_yes', 'ask_line', 'lint', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, accepted: { type: 'number' }, rejected: { type: 'number' }, own: { type: 'number' }, blocks: { type: 'number' }, ask_yes: { type: 'number' }, ask_line: { type: 'number' }, lint: { type: 'string' }, summary: { type: 'string' } } }
const QS = { type: 'object', required: ['status', 'report_path', 'report_bytes', 'answer_for_boris', 'summary'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, report_bytes: { type: 'number' }, answer_for_boris: { type: 'string' }, unknowns: { type: 'array', items: { type: 'string' } }, summary: { type: 'string' } } }
const RES = { type: 'object', required: ['status', 'report_path', 'report_bytes', 'findings', 'sources', 'headline'], properties: { status: { type: 'string', enum: ['DONE', 'PARTIAL', 'BLOCKED'] }, report_path: { type: 'string' }, report_bytes: { type: 'number' }, findings: { type: 'number' }, sources: { type: 'number' }, headline: { type: 'string' } } }

const TOPICS = [
  { key: 'A', name: 'Triggering clips and the tempo bar', sheets: ['area-tempo.md', 'facts-takes-bpm-fire.md'],
    extra: `His boxes here: the one under the answer on correcting the listening (key "answer 189"), 218 (way b, and his word: "${Q.always}"), 250 (he asks back). OWED: @@ANSWER 250 -- say in plain words what item 250 meant by "shifts the beat": the difference between how FAST the beat runs (the tempo number), WHERE each beat falls in time (earlier or later, without changing which beat is the 1), and WHICH beat is the 1 -- with one small example, at most 110 words; then the @@ASSUME that puts 250 to him again in those plain words. 218: way b reads "until your first Resync"; his own words say "always" and that he corrects: rule what both together say and assume what they leave open (what happens to HIS 1 after he has pressed Resync: may the app move it again, and when?). His sentence in box 273, "${Q.stopclips}", is yours for the clips and the layers: find his earlier words on the tempo bar's stop (grep -n -i "stop" in the old spec-A.md and in the 2026-10-05 and 2026-10-07 sections of ${BIND}), say whether it is new against the old rule, rule exactly what the stop does after it, and make every edge it leaves open an @@ASSUME (the old items it changes: @@AMEND).` },
  { key: 'B', name: 'The cue system', sheets: ['area-cue-layers.md', 'facts-app-cue-today.md'],
    extra: `His boxes here: 220 (he CHANGES the item: "${Q.quick}" ... "${Q.intime}"), 222 (he asks back), 252 (he asks back). 220: his words separate a PREVIEW of a clip by its name (at once, never waiting for the beat, as many as he likes in a row) from CUEING a clip that is loaded into a layer (in time): apply them to item 220, to item 221 (which he left empty: test it against these words) and to every old block they touch (the old items R151, R179 and their neighbours). OWED: @@ANSWER 222 -- what the master cue is and where it is: start from HIS OWN earlier words (grep -n -i "master cue" ${BIND} ${BACKLOG}; the old item 177 and the old assumption B-4), say plainly what was his and what was Harmony's guess, at most 110 words; then the @@ASSUME (ASK: YES) that puts 222 to him again in a form his question shows he can answer. Where a button sits is NOT asked (his rule on layout) -- but he asks where the master cue is SHOWN: answer that. OWED: @@ANSWER 252 -- he asks "${Q.first}" and whether the three pictures "${Q.samefps}" and says "${Q.better}" A separate architect writes a technical paper on exactly this in this same run (${RS}/answer-252.md; it may not be finished when you write): write your own honest draft from the fact sheets; the ruling settles the final text with that paper.` },
  { key: 'C', name: 'Presets', sheets: ['facts-presets-delta.md'],
    extra: `His box here: 223 (way b). Item 253 was left empty.` },
  { key: 'D', name: 'Actions in a show', sheets: ['area-actions.md', 'facts-actions-open.md'],
    extra: `His boxes here: 226 ("${Q.allactions}"), 227 ("${Q.neither}"), 228 (way b). 226: the item spoke of layer and global actions; he says ALL actions: rule what that adds (the actions of clips) and assume what his words leave open (do the buttons go off, or do the actions stand still and start again?). His sentence in box 273, "${Q.stopclips}", says what the tempo stop does to clips; topic A owns that rule; rule what the stop does to actions with it in mind. 227: read "neither" strictly; his sentence speaks of a clip's action; what a play-once action on a LAYER or on GLOBAL does after it is not said by it. 228: his way b goes against the recommendation Harmony gave in the answer above it: his answer is the rule.` },
  { key: 'E', name: 'Studio and the recordings', sheets: ['area-recording.md', 'facts-takes-bpm-fire.md'],
    extra: `His boxes here: the one under the answer on the low-resolution show recording (key "answer lowres"), 231, 232 (way c), 256. OWED: @@ANSWER lowres -- he asks whether its size should be "${Q.monitor}": find what "output monitor" names (${NAMES}; his earlier words), say what will be built and what stays to be tested (which codec; 30 or 15 pictures a second), at most 110 words. He left 262 (a quarter as wide and as high; 10-minute pieces) and 261 empty = accepted, yet his box suggests another size and rate with "Maybe": do NOT pick silently -- one bullet under CONFLICTS and an @@ASSUME with ASK: YES. NEW in his words: "${Q.hdrender}" (a film in full quality, made afterwards from a show recording): check whether the old spec already rules it (grep -n -i "render" ${R7}/spec-*.md; his earlier "215 plan all of these" names Render); if not, it is a new function: RULE as far as his words go, the rest @@ASSUME. 231: he now says what the low-resolution recording is FOR ("${Q.mistake}"): rule what Studio must therefore be able to do with it as far as his words go, and assume the rest. The screen is called Studio (J owns the name; you use it). The test of codecs after the build is H's: refer to it.` },
  { key: 'F', name: 'The show file, decks and saving', sheets: ['area-show-decks.md'],
    extra: `His boxes here: the one under the Snapshot answer (key "answer R188-snapshot"), 233, 234, 236. He RE-DEFINES Snapshot against Harmony's answer and against item 236 ("${Q.perhaps}" a save of the show exactly where it is "${Q.snapall}", by a shortcut, to look at later): rule what his words say and make every edge an @@ASSUME -- what exactly is saved; where the snapshots are kept; how he looks at one later and what opening one does to the show that is open; whether a picture of the output goes with it; that taking one never disturbs the running show. Note his "Perhaps". 234 holds two statements: apply both (the second, on changing the deck, may already be the rule: then say which old block rules it). His words "it does not play": say which clip "it" can be, rule your best reading, assume the other. 233: he says both ways are right: write precisely which click does what, for an empty cell and for a cell that holds a clip, and assume what his words do not say.` },
  { key: 'G', name: 'Output screens', sheets: ['area-outputs.md'],
    extra: `His box here: 265 (way b). Square it with 237, 263 and 264 (left empty = accepted) and with his earlier rule that the app never opens an output by itself (grep -n -i "never opens" ${BIND}; ${MAIN}/CLAUDE.md, the paragraph "Outputs"): after All Outputs Off, what does opening a show do, and what does Output > Restore Last Outputs do? Assume what is open.` },
  { key: 'H', name: 'How a clip plays', sheets: ['area-clip-transport.md'],
    extra: `His boxes here: the one under the codec answer (key "answer codec": "${Q.baseline}"), 238 (way b), 239 ("${Q.whyhap}"). OWED: @@ANSWER 239 -- why HAP copies at all: the honest reason in at most 90 words (only if a jump, backwards play or a random beat is seen to wait on his ordinary clips; if nothing waits, never), in words that do not assume he remembers the last page. OWED: @@ANSWER codec -- yes or no to the test he asks for, what it would compare, and when; at most 90 words. Then rule 239 after both boxes: what is built, what is tested and when; page 2 said Harmony would measure "first" -- say where that measurement now stands (he asks for the test AFTER the build) and assume what is open. Promise no numbers. 238 way b: his answer cuts a new BPM-mode clip's end to whole groups of 4 bars; item 232 way c (topic E) does the like for a recorded clip: one bullet under REACHES OTHER TOPICS if the two must agree.` },
  { key: 'I', name: 'Effects, signals and what moves a slider by itself', sheets: ['area-effects-signals.md'],
    extra: `His boxes here: the one under the answer on effects that read the beat by themselves (key "answer R221-b"), 240, 241. 240: his words go AGAINST what he accepted in round 1 (find it: the old item 204 and its neighbours; grep -n -i "keying" ${BIND}) -- his newest words win; CONFLICTS carries both quotes. Rule what stays (all the blend modes and the keying: placeholders, "${Q.time}"), what is new (masks and moving masks, "${Q.alpha}"), and what that means for the coming build; assume the rest (item 240's own text, "There are no mask layers", is now against his words). 241: two kinds of envelope: rule both as far as his words go; assume the rest. The box under the answer: "${Q.sink}" (read: the sync) and "${Q.macro}" -- rule what is firm, mark what is "maybe", assume the rest; 243 (left empty) stands with it.` },
  { key: 'J', name: 'The keyboard and MIDI mapping, menus and messages', sheets: ['area-controls.md'],
    extra: `His boxes here: the one under the answer on the screen's name (key "answer review-name"), 244 (way b), 245 (way b), 246 (a task for Harmony: "${Q.research}"), 270 (way b, with words of his own). The screen is called STUDIO: one @@NAME Studio; and list under REACHES OTHER TOPICS every name in ${NAMES} that carries the old word (grep -n -i "review" ${NAMES}), so that the names list can be brought up to date. 246: three researchers look online in this same run and a separate seat writes the answer: file item 246 as STATUS OPEN ("waits on the research he ordered"), and write NO @@ANSWER and NO @@ASSUME about the hold setting itself. 270: both kinds of knob must work ("${Q.haveto}"): rule it as far as his words go; what the app must know about endless knobs is technical (TODAY: the old item's notes, the fact sheet).` },
  { key: 'K', name: 'Sources and the automatic features', sheets: ['area-sources-auto.md'],
    extra: `His boxes here: 247, 273, and the box "Anything else?" (key "general": "${Q.formats}"). OWED: @@ANSWER general -- answer from the program text, read not run: which audio file types the app's audio-file input opens (start at ${MAIN}/docs/claude/analysis.md and architecture.md, then the source that opens an audio file); at most 60 words. The honest floor: ffmpeg is installed on this Mac (/opt/homebrew/bin/ffmpeg) and converts either type, so both are fine whatever the app opens directly. Say in one clause what the clips are FOR (the test whether a correction by hand holds in automatic mode: items 217 and 218). 273 holds three statements: what Stop does to clips (A owns it), that the audio file would therefore stop, and that a pause does not pause it "${Q.bpmconn}": rule the audio file's side and assume what "connected to the BPM" leaves open.` },
  { key: 'X', name: 'The loose lists', sheets: [],
    extra: `You have NO page-2 item and NO box of your own: your ## ITEMS and ## QUESTIONS BACK sections stay empty. Your job is the old spec-X.md (73 items of the loose lists C / N / U; 28 assumptions): test EVERY old @@ITEM of spec-X.md, and every old assumption listed INTERNAL in your slice, against his 33 boxes and against the 57 items of page 2 as they now stand (answered or accepted). Where his words or an accepted item now change a RULE: @@AMEND. Where they settle or contradict an open assumption: @@DROP ASSUME, or a replacement block with the same id. Where nothing changes: write nothing. Under ## SUMMARY say how many old items and assumptions you tested and how many changed.` },
]
const NAME = Object.fromEntries(TOPICS.map(T => [T.key, T.name]))

const applyPrompt = T => `${WHO}

YOUR TASK: APPLY HIS ANSWERS TO PAGE 2 in topic ${T.key}, "${T.name}". You are the architect who writes down how this part of the app will work after his answers of 2026-10-09: the rule now for every page-2 item of this topic, the changes this makes to the old spec, the answers he is owed, and the short list of what is still assumed.

READ, in this order:
1. ${ANS} WHOLE -- all 33 boxes of his, also those of other topics: an explanation he gave once answers its repeats.
2. ${PAGE} WHOLE -- page 2 as he read it (the 9 answers at its top are Harmony's, written for him).
3. ${RS}/slice3-${T.key}.md WHOLE -- your slice: his boxes, your items with what each was made from, the old assumptions of this topic and where each went, and the id lists.
4. ${R7}/spec-${T.key}.md WHOLE -- the old spec of this topic (its ## ITEMS and ## ASSUMPTIONS; a RULE line can be several thousand characters: read it, do not skim it).
5. Every block named under MADE FROM that lives in another spec file: grep -n -A 7 "^@@ITEM <id>" ${R7}/spec-*.md (for an assumption: "^@@ASSUME <id>").
6. ${NAMES}: grep it for every name you use (grep -n -i "<word>" ...).
7. His earlier words wherever a box of his builds on them or goes against them: ${BIND} (grep -n '^## ' first; his words are inside quotes).
8. Only where a rule needs it: the fact sheets ${T.sheets.length ? T.sheets.map(s => R5 + '/' + s).join(', ') : '(none for this topic)'} (grep, then ranges), and the app's program text (read-only).

THIS TOPIC. ${T.extra}

${DISCIPLINE}

${PAGERULES}

${FORMAT}

${CODE}

TURN BUDGET. Your agent type stops SILENTLY at 120 turns. By turn 8 the file exists as a skeleton (all headings, every @@ITEM block of your slice with its STATUS). Rewrite or append every ~10 turns. Be done by turn 70. THE LAST STEP, always: run python3 ${RS}/wf/lint3.py ${RS}/apply-${T.key}.md ${T.key} -- fix every line it marks with "!" (an @@AMEND whose OLD is not found exactly once is the usual one: copy the piece again from the file) and run it again until its last line reads "LINT: OK"; lines marked "~" are notes.

${RULES}

REPORT_FILE: ${RS}/apply-${T.key}.md
RETURN: status (DONE only when the lint's last line begins "LINT: OK"), report_path, items (count of @@ITEM), amends (count of @@AMEND), assume_yes / assume_line / assume_no (counts of @@ASSUME by ASK), drops (count of @@DROP), lint (the lint's LAST line, verbatim), summary (at most 400 characters: what his answers changed most here).`

const p252 = `${WHO}

YOUR TASK: ONE TECHNICAL PAPER FOR ONE QUESTION OF HIS. Item 252 of page 2 read: "I assume the output always comes first: the preview may be smaller, less smooth and, in the first build, show trails (Echo, Freeze, Feedback) and MilkDrop a little differently." He wrote in its box (verbatim; BF number in ${ANS}, the block "@@BOX 252"): "What do you mean by come first? Playing something smaller or less smooth may create problems. What do you think? You understand how the system needs to be built better than me. Of course the output is more important, but the output preview screen which we have and the preview-cure screen below it should all run at the same fps, no?" ("preview-cure" is a slip of dictation for preview-cue.)
He hands the technical call to Harmony ("${Q.better}"). So this is Harmony's to DECIDE on the merits and to explain to him in plain words. You write the paper the decision stands on.

WHAT THE THREE PICTURES ARE. (1) The output screens: the projectors / displays the audience sees. (2) The output panel inside the app's window: a small copy of the output. (3) The preview panel below it (he also says cue monitor): under the planned cue system it shows a clip previewed by its name, or the layers whose cue buttons are on, mixed, while the output goes on untouched; and "master cue" (old item 177).

READ: ${RS}/slice3-B.md WHOLE (item 252, what it was made from: the old assumptions B-10, B-11, X-11 and the old items D8, D9 -- read each: grep -n -A 7 "^@@ASSUME B-10" ${R7}/spec-B.md and so on); ${R7}/spec-B.md, the items on the cue system (grep -n "^@@ITEM" first); ${R5}/facts-app-cue-today.md WHOLE (how the preview works now and what a second mix of the same layers costs: read, not run); ${R5}/area-cue-layers.md (grep, then ranges); ${MAIN}/docs/claude/rendering.md and ${MAIN}/CLAUDE.md (the 4-thread model: the render thread draws the composition once per frame, offscreen, at the display's rate; Pitfalls 13, 14, 35, 37, 57, 66); then the program text itself where the sheets do not answer (read-only; cite file:line).

ANSWER THESE, each with its evidence (file:line or fact-sheet point; "read, not run"):
1. With the app as it is planned, what decides the frame rate of each of the three pictures? Are they drawn in the same frame by the same thread? Can the preview panel run at a different rate than the output at all, and would that be simpler or harder to build than one rate for all?
2. What does a cued mix cost on top of the output's frame (the same layers drawn a second time, effects included), in what cases is it free, and in what cases could it push the frame over its budget? Nothing is measured: say what would have to be measured and how small a test would do.
3. What are the real ways to protect the output when the computer cannot keep up? (For example: the preview is drawn at a smaller size; the preview skips frames while the output keeps its rate; the preview leaves out effects that hold picture history; nothing is protected and everything slows together.) For each: what he would SEE in the preview, what it costs to build, and whether it can cause the "problems" he fears (a preview that lies about what the layer will look like on the output).
4. Trails (Echo, Freeze, Feedback) and MilkDrop in the preview: why item 252 said "a little differently", and whether that still has to be so.
5. YOUR RECOMMENDATION: one design, with the reason it beats the next best; and what stays unknown until a test.

THE PAPER (markdown): ## WHAT DECIDES THE FRAME RATE (points 1-2, with evidence) / ## WAYS TO PROTECT THE OUTPUT (point 3, a short table) / ## TRAILS AND MILKDROP (point 4) / ## RECOMMENDATION / ## UNKNOWN UNTIL MEASURED / ## ANSWER FOR BORIS (plain words, at most 120 words, the answer in the first sentence: what "the output comes first" means, whether all three run at the same frame rate, and what he would see if the computer could not keep up; no jargon: say "pictures a second", not fps, unless you quote him; nothing measured is claimed) / ## THE ASSUMPTION FOR HIS NEXT PAGE (one sentence beginning "I assume", at most 40 words, what WILL happen; then "b)" one other way).

TURN BUDGET: your agent type stops SILENTLY at 120 turns; skeleton by turn 8, done by turn 60.

${CODE}

${RULES}

REPORT_FILE: ${RS}/answer-252.md
RETURN: status, report_path, report_bytes (wc -c of your file), answer_for_boris (your ## ANSWER FOR BORIS, verbatim), unknowns (at most 6, each at most 200 characters), summary (at most 400 characters).`

const LENSES = [
  { key: 'dj', name: 'DJs and DJ / producers: their decks, controllers and software',
    look: `How do DJs and DJ / producers actually use a pad or a button that works ONLY WHILE HELD, as against one that is pressed once to switch on? Look at: the CUE button of a CDJ / club deck (hold = play from the cue point, release = back); hot cues and pad modes in Serato DJ, rekordbox, Traktor, Engine DJ (hold against latch / toggle for pad FX, "Pad FX" hold and release, "momentary" against "latch" FX on mixers such as Pioneer DJM, Allen & Heath Xone); Ableton Live's clip launch modes (Trigger, Gate, Toggle, Repeat -- Gate is exactly "plays while held") and how performers use Gate; Novation Launchpad, Akai APC and Ableton Push (momentary against toggle buttons, also in their MIDI settings); Native Instruments Maschine and MPC "note repeat" / hold. Also DJ / producers who play live with visuals (for example acts that trigger visuals from pads).` },
  { key: 'vj', name: 'VJ software and VJs',
    look: `What do VJ programs offer for a trigger that is on only while a key or pad is held, and do VJs use it? Look at: Resolume Arena / Avenue (clip trigger style and the shortcut modes for keyboard and MIDI: "Piano" against "Toggle" against normal; layer / clip "trigger style"; what the manual and the forum say they are for); VDMX; TouchDesigner; MadMapper; Modul8; CoGe; Synesthesia; HeavyM; Millumin; GrandVJ; NestDrop. Look for VJs describing how they play (forum threads, tutorials, interviews): flashes and strobes on a held pad, a logo held through a break, "piano mode" as the way to play visuals like an instrument -- and for the opposite: people who say they never use it, or that it gets in the way.` },
  { key: 'live', name: 'Bands and live shows: the people who run lights and visuals for them',
    look: `How do operators who run LIGHTS or VISUALS for bands and live acts use buttons that act only while held? Look at: lighting desks and software, where this is standard -- "flash" and "bump" buttons on grandMA, ChamSys MagicQ, Avolites, Hog, ETC, Obsidian Onyx, and "busking" a show (playing lights by hand to music one has never heard); how flash / bump buttons are used with live drums and hits; momentary against latch on those desks; video operators for bands and festival VJs using a MIDI pad the same way. Is a held button the normal tool for accents with a live band? Find working operators saying so, and anyone saying the opposite.` },
]
const researchPrompt = L => `YOU ARE ONE OF THREE RESEARCHERS looking online, each from a different side. Yours: ${L.name}.

WHY. Boris Karpman owns Audio-DNA, a live audio-reactive VJ app for macOS (think Resolume Arena, which he also owns). He performs with it for DJs and bands. Its keys and MIDI pads trigger clips, effects and actions. The open point: should every key or pad have ONE setting, "press" or "hold", where HOLD means: the thing is on only WHILE the pad is held and goes off when it is let go (press = one press switches it on and it stays). He had first answered "no hold setting", then asked Harmony (the orchestrator): "${Q.holdask}", read the argument, and now writes (verbatim): "${Q.research}"
So the question is not what is possible but what PEOPLE ACTUALLY DO, and whether a hold setting is worth building, measured only against what DJs, bands and DJ / producers do.

WHAT TO LOOK FOR. ${L.look}

HOW. Search the web (several searches; the makers' own manuals and forums first, then working performers: forum threads, interviews, tutorials; prefer sources of the last eight years). Open the pages and read them: a search snippet is not a source. For every finding keep the URL and a short quote (at most 25 words) that carries it. LOOK FOR THE OPPOSITE on purpose: evidence that the held mode is missing, unused, disliked, or replaced by something else. Do not pad: ten findings that hold beat thirty that might. What you could not find is a finding too: say so.

THE PAPER (markdown): ## HEADLINE (three sentences: what people on your side actually do) / ## FINDINGS (numbered; each: the claim in one sentence -- who does what with a held button -- then URL, the short quote, and HOW SURE: READ ON THE MAKER'S PAGE | READ FROM A PERFORMER | ONE FORUM VOICE | INFERRED) / ## WHAT IT IS USED FOR ON STAGE (the concrete uses, each with the finding numbers behind it) / ## AGAINST IT (the opposite evidence, the same way; or "looked for, found none" with the searches you ran) / ## HOW THE PROGRAMS NAME IT (the words used: momentary, gate, piano, flash, bump, hold, latch, toggle ... and by whom) / ## NOT FOUND.

RULES. Read-only except the ONE file named on your REPORT_FILE line (write it with a bash heredoc with a QUOTED delimiter to the literal absolute path; a skeleton early, then fill it). Never build, never launch or touch any app on this machine, never cd in a command, never commit. Do not read the repository's code: this is about the world outside. ${RELAY}

REPORT_FILE: ${RS}/research-hold-${L.key}.md
RETURN: status, report_path, report_bytes (wc -c of your file), findings (count), sources (count of different URLs you actually opened and read), headline (your ## HEADLINE, at most 400 characters).`

const checkApply = (key) => `${WHO}

YOUR TASK: RE-CHECK ONE PAPER, blind and hard. An architect has applied Boris's answers to page 2 in topic ${key}, "${NAME[key]}", in the paper ${RS}/apply-${key}.md. Boris's next page and, later, the build will stand on it. Your job is to find where it is WRONG: where a RULE says more, less or something else than his words; where an assumption of the architect is written as if it were his word; where an item he accepted is not carried as he read it; where an old rule that his words overturn was left standing; where an assumption is put to him although his words already settle it; where a real gap is hidden.

DO IT IN THIS ORDER (re-derive, do not re-read):
1. Read ${ANS} WHOLE (his 33 boxes), ${PAGE} WHOLE (page 2 as he read it) and ${RS}/slice3-${key}.md WHOLE (this topic's boxes, items, what each was made from, the old assumptions and where each went).
2. BEFORE you open the paper, write into your own file, under "## MY OWN READING": for every box of his that lands in this topic -- one or two sentences: what do these words decide, what do they leave open, and which old rule do they overturn? And for every item of this topic that he left empty: is there a box anywhere among the 33 whose words go against it? (His rule: "${Q.repeats}")
3. Now read the paper WHOLE. Run: python3 ${RS}/wf/lint3.py ${RS}/apply-${key}.md ${key} and copy its output into your file.
4. Compare, block by block.
   - @@ITEM: is the STATUS earned? Does the RULE say exactly what his words (or the accepted text) say -- no more, no less? Is it complete enough to build from? Check every quote of his against ${ANS}, letter by letter, and its BF number.
   - @@AMEND: open the old item (grep -n -A 7 "^@@ITEM <id>" ${R7}/spec-*.md) and read its RULE line whole. Is OLD the right piece? Does NEW say what his words say? And the hard part: is there ANOTHER sentence in that RULE, or in another old @@ITEM of ${R7}/spec-${key}.md, that his words now make false and that NO amendment touches? (grep the old spec for the key words of each box.)
   - @@ASSUME: is it really open -- do no words of his, in this round or earlier (grep ${BIND}; his words are inside quotes), settle it? Is the triage right by the three tests? Is TEXT one assumption, plain, at most 40 words, with nothing about how the app is now, no layout, his own names (${NAMES}), "Studio" and never "Review"? Is the default HIS answer wherever he gave one?
   - @@DROP ASSUME: do his words really settle it?
   - @@ANSWER: does it answer what he asked, in the first sentence, in plain words; is anything claimed that nobody measured; is it within its length?
   - CONFLICTS: is every clash between his newest words and his earlier ones named with both quotes?
5. Look for what is MISSING: a box or a sentence of a box that no block carries; an edge a builder would have to guess; an old assumption listed INTERNAL in the slice that his words now settle or contradict and that the paper left alone.
A finding is MUST when the paper would make Harmony build or ask the wrong thing; SHOULD when it is right but unclear, too long for him, or uses a word he would have to look up. Do not report matters of taste.
The discipline the architect was held to, so that you can hold the paper to it:
${DISCIPLINE}

${PAGERULES}

YOUR PAPER (markdown): ## MY OWN READING (step 2) / ## LINT OUTPUT / ## FINDINGS -- one line each, numbered F1, F2 ...: "F<n> | MUST or SHOULD | the block (for example ITEM 226, AMEND R151 2, ASSUME ${key}3-4, MISSING) | what is wrong, with his words quoted and their BF number | FIX: the exact replacement text where you can give it" / ## WHAT I CHECKED AND FOUND RIGHT (block ids only).

${CODE}

${RULES}

REPORT_FILE: ${RS}/check-${key}.md
RETURN: verdict (SOUND = nothing wrong; SOUND_WITH_CORRECTIONS = findings, the paper is usable with them; UNRELIABLE = so wrong that it should be written again), paper_path (your own file), paper_bytes (wc -c of your own file), checked (count of blocks compared), must (count), should (count), summary (at most 400 characters: the two or three findings that matter most).`

const check252 = `${WHO}

YOUR TASK: RE-CHECK ONE TECHNICAL PAPER, blind and hard. Boris asked, about the output and the previews (verbatim, his box under item 252; in ${ANS}, the block "@@BOX 252"): "${Q.first}" ... the three pictures "${Q.samefps}" and: "${Q.better}" An architect answered in ${RS}/answer-252.md. Harmony will DECIDE the design on it and tell him in plain words. Find where it is wrong.

DO IT IN THIS ORDER. (1) BEFORE opening the paper, form your own view from the evidence: read ${R5}/facts-app-cue-today.md WHOLE and ${MAIN}/docs/claude/rendering.md (grep -n '^#' first), and in the program text find for yourself where the output screens, the output panel and the preview panel get their picture and what paces them (read-only; cite file:line). Write under "## MY OWN VIEW": are the three drawn in the same frame by the same thread; can one of them run at another rate; what does a second mix of the same layers cost. (2) Read the paper WHOLE. (3) Check every file:line it cites by opening it: does the line say what the paper says? (4) Attack the recommendation: name the strongest other design and say whether the paper's reason against it holds. Could the recommended design make the preview LIE about what a layer will look like on the output (his fear: "Playing something smaller or less smooth may create problems")? (5) Read "## ANSWER FOR BORIS" as he would: is the answer in the first sentence, is every word plain, is anything claimed that nobody measured, is it true to the paper above it?

YOUR PAPER (markdown): ## MY OWN VIEW / ## CITATIONS CHECKED (each: file:line, CONFIRMED or WRONG with what the line really says) / ## FINDINGS (numbered F1 ...: MUST or SHOULD | what is wrong | the fix) / ## THE STRONGEST OTHER DESIGN / ## A BETTER ANSWER FOR BORIS (only if yours is truer or plainer: at most 120 words).

${CODE}

${RULES}

REPORT_FILE: ${RS}/check-252.md
RETURN: verdict (SOUND | SOUND_WITH_CORRECTIONS | UNRELIABLE), paper_path (your own file), paper_bytes (wc -c of it), checked (count of citations opened), must, should, summary (at most 400 characters).`

const checkHold = `YOUR TASK: RE-CHECK THREE RESEARCH PAPERS against their own sources. Boris Karpman (owner of Audio-DNA, a live VJ app; he performs for DJs and bands) ordered (verbatim): "${Q.research}" The point: should every key or pad have a setting "press" or "hold" (hold = on only while the pad is held). Three researchers looked online from three sides: ${LENSES.map(L => RS + '/research-hold-' + L.key + '.md').join(', ')}. Harmony will tell Boris what people actually do on the strength of these papers. Find what does not hold.

DO THIS. (1) Read the three papers WHOLE (a paper that is not there: say so and go on). (2) Pick the FIFTEEN findings the conclusion leans on most (across the three papers; always include every finding that says what working performers DO, and every finding under "AGAINST IT"). For each: open the URL yourself and read the page. Verdict: CONFIRMED (the page says it; give your own short quote), WEAKER (the page says less: say what), NOT FOUND (the page is gone or does not say it), WRONG (it says otherwise). (3) Search for yourself, three or four searches, for what all three may have missed: evidence AGAINST a held mode (performers who avoid it; programs that dropped it), and what the hold mode is called in the programs Boris knows (Resolume: "Piano"). (4) Say what the evidence supports, in plain words: is a held button a normal tool for DJs, for people who run visuals or lights for bands, for DJ / producers -- each of the three separately, each with HOW SURE.

YOUR PAPER (markdown): ## FINDINGS RE-CHECKED (a table: paper and finding number | verdict | your quote or what is wrong) / ## WHAT I FOUND MYSELF (with URLs and short quotes) / ## WHAT THE EVIDENCE SUPPORTS (DJs / bands / DJ-producers: one short paragraph each, with HOW SURE) / ## WHAT IT DOES NOT SUPPORT (claims in the papers to drop).

RULES. Read-only except the ONE file named on your REPORT_FILE line (bash heredoc, QUOTED delimiter, the literal absolute path; skeleton early). Never build, never launch or touch any app on this machine, never cd in a command, never commit. ${RELAY}

REPORT_FILE: ${RS}/check-hold.md
RETURN: verdict (SOUND | SOUND_WITH_CORRECTIONS | UNRELIABLE: of the three papers taken together), paper_path (your own file), paper_bytes (wc -c of it), checked (count of findings re-checked), must (count WRONG or NOT FOUND), should (count WEAKER), summary (at most 400 characters: what the evidence supports).`

const rulePrompt = (key, extra) => `${WHO}

YOUR TASK: RULE ON ONE PAPER. For topic ${key}, "${NAME[key]}", an architect applied Boris's answers to page 2 (${RS}/apply-${key}.md) and a blind checker went through it (${RS}/check-${key}.md). You are the last hand before Harmony lays this paper over the spec and makes Boris's next page from its assumptions. Decide every finding on the merits, find what both missed, and write ONLY the blocks that change. You do NOT rewrite the paper.

READ, in this order: (1) ${ANS} WHOLE -- his 33 boxes; his words are the measure of everything. (2) ${PAGE} WHOLE. (3) ${RS}/slice3-${key}.md WHOLE. (4) ${RS}/apply-${key}.md WHOLE. (5) ${RS}/check-${key}.md WHOLE (first: wc -c on it; if it is not there or empty, return status BLOCKED). (6) For every block a finding touches: the old item it amends (grep -n -A 7 "^@@ITEM <id>" ${R7}/spec-*.md -- read the RULE line whole), his earlier words (${BIND}; inside quotes), the names (${NAMES}).${extra}

HOW TO RULE.
- Every finding of the checker gets ONE verdict line: ACCEPT (the paper changes as the checker says), MODIFY (it changes, but differently: say how) or REJECT (the paper stands: say why, with his words). Decide by HIS WORDS, quoted with their BF number -- not by which of the two argued better.
- Then your own pass, independent of the checker: read every @@ITEM RULE against the box or the accepted text; every @@AMEND against the old RULE it changes; every @@ASSUME against the three tests for ASK; every @@ANSWER as Boris would read it. Your own findings are numbered H1, H2 ...
- THE TRIAGE IS YOURS TO SETTLE: after your ruling every @@ASSUME of this topic has its final ASK (YES / LINE / NO). He has too much to read: an assumption that words of his settle is dropped (@@DROP ASSUME <id>, WHY cites them); a technical one is NO; but a real doubt about what the app DOES that he or the audience would see is YES, however many there are.
- The discipline the paper was held to binds you too:
${DISCIPLINE}

${PAGERULES}

YOUR FILE (a script parses its blocks: the same block format as the paper).
# RULING ${key} -- ${NAME[key]} (s-rta-1009, page 2)
## VERDICTS
(one line per finding: "F3: ACCEPT -- one sentence" ... then your own: "H1: one sentence saying what was wrong and what you changed")
## REPLACEMENT BLOCKS
(ONLY what changes. A block with the same kind and id as a block of the paper REPLACES it, whole: write all its fields. A block with a new id is ADDED. To remove a block of the paper: "@@DROP <KIND> <id>" with a WHY line and @@END -- for example "@@DROP AMEND R151 2", "@@DROP ASSUME ${key}3-4". Block kinds and fields are exactly those of the paper: @@ITEM (TITLE, STATUS, HIS, RULE, CHANGED, TODAY), @@AMEND (OLD, NEW, HIS, WHY), @@ASSUME (ABOUT, TEXT, WHY, ALT, IF-WRONG, ASK), @@ANSWER (HIS, ANSWER), @@NAME (MEANS, SOURCE). Every field is ONE physical line.)
## FOR THE PAGE RULING
(at most 15 lines for the ruling that makes Boris's next page: which assumptions of this topic MUST be put to him and in what order of weight; which of his newest words go against earlier ones and should be said to him once more in one line; what in this topic depends on another topic's ruling)

HOW TO WRITE IT. Create the file with its three headings first. Then APPEND, in commands of at most about 120 lines each (bash heredoc with a QUOTED delimiter, >> to the literal absolute path): the verdict lines, then the replacement blocks a few at a time. Never put the whole ruling into one command, and never re-type a block that does not change: an answer that grows too long is cut off and lost. THE LAST STEP, always: python3 ${RS}/wf/lint3.py ${RS}/rule-${key}.md ${key} --rule -- fix every line marked "!" and run it again until the last line reads "LINT: OK".

TURN BUDGET: your agent type stops SILENTLY at 120 turns: the file with its headings by turn 6, the verdicts by turn 40, done by turn 80.

${CODE}

${RULES}

REPORT_FILE: ${RS}/rule-${key}.md
RETURN: status (DONE only when the lint's last line begins "LINT: OK"), report_path, accepted (count of ACCEPT and MODIFY), rejected (count), own (count of H findings), blocks (count of blocks under REPLACEMENT BLOCKS), ask_yes and ask_line (how many @@ASSUME of this topic end as YES and as LINE after your ruling), lint (the lint's LAST line, verbatim), summary (at most 400 characters: what you changed that matters most).`

const RULE_EXTRA = {
  B: ` (7) THE TECHNICAL PAPER on his question under item 252 and its re-check: ${RS}/answer-252.md and ${RS}/check-252.md, both WHOLE. With them you SETTLE @@ANSWER 252 (at most 120 words, plain, the answer in the first sentence, nothing measured claimed) and the @@ASSUME that puts 252 to him again: this is the technical call he handed to Harmony ("${Q.better}") -- decide the design on the merits of the two papers, say in the VERDICTS which design you ruled and why it beats the next best, and write both blocks under REPLACEMENT BLOCKS (replacing the architect's drafts). If the two papers are not there, rule the rest and say so under FOR THE PAGE RULING.`,
}

const holdPrompt = `${WHO}

YOUR TASK: THE ANSWER HE ORDERED ON THE HOLD SETTING. The history, all verbatim. Round 1, question 207: he chose "c: none -- a key or pad only presses" and asked: "${Q.holdask}" (and how it helps DJs or bands). Page 2 gave Harmony's argument (in ${PAGE}, the answer titled "Why a pad could have a hold setting") and item 246: "I assume your answer stands: no hold setting. A pad or a key only presses. b) Every pad and key gets one switch, press or hold, set to press until you change it, so nothing changes for a pad you leave alone." He wrote in its box (in ${ANS}, the block "@@BOX 246"): "${Q.research}"

THE RESEARCH. Three researchers looked online from three sides and a fourth re-checked their sources: ${LENSES.map(L => RS + '/research-hold-' + L.key + '.md').join(', ')} and the re-check ${RS}/check-hold.md. Read all four WHOLE. The re-check wins over a paper wherever they differ: a finding it marks NOT FOUND or WRONG is not used; one it marks WEAKER is used only as far as it holds.
ALSO READ: ${RS}/slice3-J.md (item 246 and what it was made from); the old item 207 and the old item D34 (grep -n -A 7 "^@@ITEM 207" ${R7}/spec-J.md; the same for D34); for Harmony's own notes only, what the app does with held keys now (${R5}/area-controls.md: grep -n -i "piano\\|momentary\\|hold"; read, not run).

WRITE (markdown, with blocks a script parses; every field ONE physical line):
# ANSWER -- the hold setting (item 246; s-rta-1009)
## WHAT PEOPLE DO (for Harmony: one short paragraph each for DJs, for the people who run visuals or lights for bands, for DJ / producers; every sentence with the finding it stands on, as "(dj 4)", "(live 2)", "(re-check)"; and what the research did NOT find)
## IS IT WORTH BUILDING (the judgement by his own measure -- "We need to only match what Dj or bands are doing": yes or no, the reason, the strongest argument against your verdict and why it does not win; what the smallest version would be)
## BLOCKS
@@ANSWER 246-hold
HIS: <the BF number of box 246, and his order in quotes>
ANSWER: <for Boris, at most 150 words, plain: the verdict in the first sentence; then what DJs do, what people who run visuals or lights for bands do, what DJ / producers do -- one sentence each, naming the two or three best-known tools and what THEY call it; then what you recommend. Up to three links as [label](https://...) to the clearest sources the re-check CONFIRMED. Nothing is claimed that the re-check did not confirm.>
@@END
@@ASSUME J3-hold
ABOUT: 246, 207, D34
TEXT: <ONE assumption beginning "I assume", at most 40 words. HIS EXPLICIT ANSWER IS THE DEFAULT: he answered "no hold setting" and has not taken it back; he asked whether it is worth doing. So TEXT keeps his answer -- unless you can show from his own words that this box withdraws it; your recommendation, if it differs, is way b.>
WHY: <at most 30 words>
ALT: <"b) ..." -- the other way; "c) ..." only if there is a real third>
IF-WRONG: <STAGE | REBUILD | SMALL> <a few words>
ASK: YES <a few words>
@@END
@@ITEM 246
TITLE: <at most 12 words>
STATUS: OPEN <a few words>
HIS: <BF number, his words in quotes>
RULE: <begins "Waits on his answer:" and then says, complete enough to build from, what each of the two ways would be: what a held pad does to a clip (in BPM mode and not), to an effect's button, to an action; what happens on release; what a key on the computer keyboard does>
CHANGED: <what this round changes against the old items 207 and D34>
TODAY: <Harmony's notes: what the app does with held keys and pads now, with the fact sheet's point or file:line; read, not run>
@@END

${PAGERULES}

TURN BUDGET: your agent type stops SILENTLY at 120 turns; skeleton by turn 6, done by turn 40.

${RULES}

REPORT_FILE: ${RS}/answer-hold.md
RETURN: status, report_path, report_bytes (wc -c of your file), answer_for_boris (the ANSWER line of @@ANSWER 246-hold, verbatim), unknowns (at most 5, each at most 200 characters: what the research could not show), summary (at most 400 characters: the verdict and the reason).`

// ---------------------------------------------------------------- run (every stage: parallel() over a FIXED list, so that a resume finds its cache)
const ok = r => r && r.status !== 'BLOCKED'
const trim = (s, n) => (s || '').slice(0, n)
const out = { apply: {}, check: {}, rule: {}, research: {}, p252: null, c252: null, chold: null, hold: null, skipped: [] }

phase('Apply')
const s1 = await parallel([
  ...TOPICS.map(T => () => agent(applyPrompt(T), { agentType: 'architect', model: 'opus', effort: 'high', schema: AS, phase: 'Apply', label: 'apply:' + T.key })),
  ...LENSES.map(L => () => agent(researchPrompt(L), { agentType: 'researcher', model: 'sonnet', effort: 'high', schema: RES, phase: 'Apply', label: 'research:hold-' + L.key })),
  () => agent(p252, { agentType: 'architect', model: 'opus', effort: 'high', schema: QS, phase: 'Apply', label: 'answer:252' }),
])
const NT = TOPICS.length, NL = LENSES.length
TOPICS.forEach((T, i) => { const r = s1[i]; out.apply[T.key] = r ? { status: r.status, items: r.items, amends: r.amends, yes: r.assume_yes, line: r.assume_line, no: r.assume_no, drops: r.drops, lint: trim(r.lint, 200), summary: trim(r.summary, 400) } : null })
LENSES.forEach((L, i) => { const r = s1[NT + i]; out.research[L.key] = r ? { status: r.status, bytes: r.report_bytes, findings: r.findings, sources: r.sources, headline: trim(r.headline, 400) } : null })
const r252 = s1[NT + NL]; out.p252 = r252 ? { status: r252.status, bytes: r252.report_bytes, answer: trim(r252.answer_for_boris, 900), unknowns: (r252.unknowns || []).slice(0, 6).map(u => trim(u, 200)), summary: trim(r252.summary, 400) } : null
const topicsOk = TOPICS.filter((T, i) => ok(s1[i]))
TOPICS.forEach((T, i) => { if (!ok(s1[i])) out.skipped.push('apply:' + T.key + ' came back ' + (s1[i] ? s1[i].status : 'null') + ' -- no check, no ruling for it') })
const nres = LENSES.filter((L, i) => ok(s1[NT + i])).length
log('Apply: ' + topicsOk.length + ' of ' + NT + ' topic papers; ' + nres + ' of ' + NL + ' research papers; the 252 paper ' + (ok(r252) ? 'is there' : 'is MISSING'))

phase('Check')
const doHold = nres >= 2
if (!doHold) out.skipped.push('fewer than 2 research papers came back -- no re-check, no answer on the hold setting')
if (!ok(r252)) out.skipped.push('the 252 paper is missing -- no re-check of it; the B ruling rules without it')
const s2 = await parallel([
  ...topicsOk.map(T => () => agent(checkApply(T.key), { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check', label: 'check:' + T.key })),
  ...(doHold ? [() => agent(checkHold, { agentType: 'researcher', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check', label: 'check:hold' })] : []),
  ...(ok(r252) ? [() => agent(check252, { agentType: 'general-purpose', model: 'sonnet', effort: 'high', schema: CS, phase: 'Check', label: 'check:252' })] : []),
])
const cs = r => r ? { verdict: r.verdict, bytes: r.paper_bytes, checked: r.checked, must: r.must, should: r.should, summary: trim(r.summary, 400) } : null
topicsOk.forEach((T, i) => { out.check[T.key] = cs(s2[i]) })
let j = topicsOk.length
const rHoldCheck = doHold ? s2[j++] : null; out.chold = cs(rHoldCheck)
const r252Check = ok(r252) ? s2[j++] : null; out.c252 = cs(r252Check)
const topicsRule = topicsOk.filter((T, i) => s2[i])
topicsOk.forEach((T, i) => { if (!s2[i]) out.skipped.push('check:' + T.key + ' did not come back -- no ruling for it (the paper stands unruled)') })
log('Check: ' + topicsRule.length + ' of ' + topicsOk.length + ' topic checks; hold re-check ' + (rHoldCheck ? rHoldCheck.verdict : 'none') + '; 252 re-check ' + (r252Check ? r252Check.verdict : 'none'))

phase('Rule')
const doHoldAnswer = doHold && !!rHoldCheck
if (doHold && !rHoldCheck) out.skipped.push('the re-check of the research did not come back -- no answer on the hold setting is written')
const s3 = await parallel([
  ...topicsRule.map(T => () => agent(rulePrompt(T.key, RULE_EXTRA[T.key] || ''), { agentType: 'architect', model: 'opus', effort: 'max', schema: RSC, phase: 'Rule', label: 'rule:' + T.key })),
  ...(doHoldAnswer ? [() => agent(holdPrompt, { agentType: 'architect', model: 'opus', effort: 'high', schema: QS, phase: 'Rule', label: 'answer:hold' })] : []),
])
topicsRule.forEach((T, i) => { const r = s3[i]; out.rule[T.key] = r ? { status: r.status, accepted: r.accepted, rejected: r.rejected, own: r.own, blocks: r.blocks, yes: r.ask_yes, line: r.ask_line, lint: trim(r.lint, 200), summary: trim(r.summary, 400) } : null; if (!r) out.skipped.push('rule:' + T.key + ' did not come back') })
const rHold = doHoldAnswer ? s3[topicsRule.length] : null
out.hold = rHold ? { status: rHold.status, bytes: rHold.report_bytes, answer: trim(rHold.answer_for_boris, 1200), unknowns: (rHold.unknowns || []).slice(0, 5).map(u => trim(u, 200)), summary: trim(rHold.summary, 400) } : null
log('Rule: ' + topicsRule.filter((T, i) => s3[i]).length + ' of ' + topicsRule.length + ' rulings; the hold answer ' + (rHold ? rHold.status : 'none'))
return out
