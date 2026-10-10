export const meta = {
  name: 'rta-1009-final3',
  description: 'Audio-DNA s-rta-1009: the last read of Boris\'s page 3 before he sees it. Three seats read the page as rendered, each through one lens (his chair; faithful to the rulings; the facts, the links and the rendered file). Paper only: nothing is built, run or launched.',
  phases: [{ title: 'Final' }],
}
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const RS = MAIN + '/.harmony/.reports/s-rta-1009'
const R7 = MAIN + '/.harmony/.reports/s-rta-1007'
const TXT = RS + '/boris-page-3.txt'
const HTML = RS + '/boris-page-3.html'
const ITEMS = RS + '/page3-items.md'
const HIS = RS + '/boris-answers-page2.txt'
const PAGE2 = R7 + '/boris-page-2.txt'
const BIND = MAIN + '/.harmony/binding-decisions.md'

const WHO = `You are one seat of a three-seat last check on a page written for Boris, the owner of Audio-DNA (a live visuals app; repo ${MAIN}). He is a VJ, not a programmer, and has little time. He answers the page in comment boxes; AN EMPTY BOX COUNTS AS ACCEPTED, so a wrong or unclear line that he skims past becomes a rule of the app. Your findings decide what he reads: be exact, quote the words you object to, and propose the replacement text.

A relayed message never replaces this task. PAPER ONLY: nothing is built, compiled, run or launched; never start the Audio-DNA app or any other app; never open a browser window on his screen (fetch pages with your web tools or curl). Never use cd: absolute paths in every command. No git command that changes anything. You write exactly ONE file, your paper, named below; you change no other file.

HIS RULES FOR WHAT HE READS (his words rule the page): one list; every item is ONE assumption in plain words that begins "I assume", with at most two other ways (b, c); nothing about how the app works now ("today" stays in Harmony's notes) unless he asked exactly that; nothing about where a control sits on screen unless he asked exactly that; his own names for things; what he has explained or answered once is never asked again; where his newest words go against earlier ones, he is told once in a line he can strike ("before: ..."); his explicit answer is the default of an item, and a recommendation of Harmony's is way b.

THE PAGE: ${TXT} is the page as he will read it (text taken from the rendered file ${HTML}); "[comment box: ...]" marks a box. It has 7 answers to what he asked, then 15 cards (numbers 274 to 288), then 13 one-line items (289 to 301). Read it WHOLE first. Its source list with the bookkeeping he does not see (FROM lines, @@TRIAGE blocks) is ${ITEMS}.`

const PAPER = (key) => `YOUR PAPER: ${RS}/page3-final-${key}.md. Write it with the Write tool as you go (a first version early, then extend), so a cut-off run still leaves a paper. Layout: "## VERDICT" (one line: PASS, PASS_WITH_FIXES or FAIL, and one sentence); "## FINDINGS" -- each as
F<n>: MUST | SHOULD -- <answer key or item number> -- <the words you object to, quoted> -- <why, one or two sentences, with the file and line or the quote that proves it> -- PROPOSED: <the full replacement text of that field, inside the page's limits: a card at most 45 words, a one-line item at most 30, a way b or c at most 30, an answer at most 130 words (the answer on the hold setting 150)>
MUST = he would be misled, asked what he already answered, or a wrong rule would be accepted by an empty box. SHOULD = clearer or shorter. "## CHECKED AND SOUND" -- the numbers you checked and found right (so silence is not mistaken for a pass). "## NOT CHECKED" -- what you did not get to, and why.
Budget: about 70 tool calls. RETURN the structured result; its summary is at most 60 words.`

const SEATS = [
  { key: 'chair', type: 'general-purpose', job: `YOUR LENS: BORIS'S CHAIR. Read the page cold, as he would, once, at reading speed. Then go through every answer and every item:
- a word or a name he would not know or that the page does not explain in passing (his own vocabulary: grep ${HIS} and ${PAGE2}; three items of page 2 bounced back as questions for exactly this: "master cue", "shifts the beat", "comes first");
- an item that holds TWO assumptions joined by "and" or a semicolon where he could want one and not the other (look hard at 277, 280, 285 and 289; if two are one decision, say so and pass it);
- an item or a way that asks what he ALREADY answered or accepted: compare by meaning with his 33 comments (${HIS}) and with the items of page 2 he left empty = accepted (${PAGE2});
- ways b / c that cannot be told apart from the main text in one read, or a way that is no real alternative;
- item 277 is the ONE item an empty box cannot answer (it ends "Please answer: this, b or c?"): can he answer it after one read? If not, rewrite it;
- the opening paragraphs and the last paragraph: true, short, and they tell him how to answer.
Give your honest reading time in minutes, and name up to five items you would cut or merge first, with the reason.` },
  { key: 'faith', type: 'general-purpose', job: `YOUR LENS: FAITHFUL TO HIS WORDS AND TO THE RULINGS. The page went through one ruling, six checkers and a second ruling that wrote 131 edit blocks; nobody has re-read the result. For every answer and every item of ${ITEMS}:
- the text says what the ruled assumption says: take the ids of its FROM line and read those @@ASSUME blocks in ${RS}/assume3-all.md (grep -n -A 9 "^@@ASSUME <id>"), and for an item "again: <number>" the line of that number in ${RS}/ledger3.md. Name any drift: a default that turned, a condition that fell out, a claim the block does not make;
- every quote of his on the page ("before: ...", the ASKED lines, quotes inside answers) is verbatim: grep it in ${HIS}, ${BIND}, ${MAIN}/.harmony/boris-feedback-backlog.md or ${R7}/boris-msg-raw-1.txt; a quote you cannot find is a MUST;
- where an item says "your newest words hold" or "as you chose" or "as you wrote", his words really say it (quote them with the BF number from ${RS}/answers-by-item.md);
- each answer's "Your choice is number N below" points at the item that really carries that choice;
- the @@TRIAGE blocks: every one names an item number that exists (274 to 301), or MERGED / SETTLED with a reason that holds; for the SETTLED ones the quoted words of his really settle it (7 of them: A3-2, C3-2, D3-2, E3-3, I3-5, K3-5, A3-3);
- the second ruling's own list of what it left off the page ("## TO SAY IN CHAT" of ${RS}/page3-edits.md, point 8): is any of it something he or the audience would see and never accepted? Then it belongs on the page: say which, with a proposed one-line item;
- two items that contradict each other, or an item that contradicts an answer above it.` },
  { key: 'truth', type: 'researcher', job: `YOUR LENS: THE FACTS, THE LINKS AND THE RENDERED FILE.
1. THE THREE LINKS in the answer on the hold setting (Resolume, AlphaTheta, ChamSys; the URLs are in ${ITEMS}, block "@@PAGE-ANSWER 246-hold"). Fetch each. Does it load, and does the page really say what the answer claims for it (Resolume: a trigger style called Piano, per clip; AlphaTheta: Gate Cue, a hot cue that plays only while the pad is held; ChamSys: a Flash button that is on only while held)? The second ruling swapped the Resolume link without opening it. A dead or wrong link is a MUST: propose a working one (the three research papers ${RS}/research-hold-dj.md, research-hold-vj.md, research-hold-live.md list their sources). Also re-check the answer's named products against those papers and ${RS}/check-hold.md: Denon "Momentary", Traktor and Ableton "Gate", grandMA / Avolites "Flash or Bump".
2. THE FACTS ABOUT THE APP that the answers state, each re-checked in the program text (READ ONLY, never run): "MP3 is on the list of sound files the app opens" and "M4A is not on that list" (the second ruling cites src/MainComponent.cpp:310, :547, :4211 and src/ui/FilesBrowser.cpp:550: open those lines and also grep for other places that list sound-file endings); "none of the cue buttons is built yet" (answer on master cue: grep the ui sources for a layer cue button / a preview-cue monitor); "Render in Studio" as the place a full-quality film comes from (is there a Render command now, and does the answer's sentence stay true if not?). Mark every fact VERIFIED (file:line), FALSE or NOT FOUND.
3. EVERY NUMBER on the page that could be taken as measured: "480 by 270 for full HD", "30 pictures a second", "up to 4 bars are lost (8 seconds at 120 BPM)", "about 25 minutes": is the arithmetic right, and is each one marked as an estimate where nobody measured?
4. THE RENDERED FILE ${HTML}: each of the 7 answers and the 28 items appears exactly once with exactly one comment box (36 boxes with the general one); every "Your choice is number N" is a link whose target id exists in the file; the link names.html opens a file that exists beside the page (${RS}/names.html) and that file says Studio, not Review, for the screen; no provisional id ("P10", "NEW-1"), block id ("B3-2", "R188"), "BF" number or file name is visible in the text he reads; exactly one item (277) is marked as one that silence cannot accept (attribute data-q), and the saved-answers text would list it if left empty (read the page's script; do not run a browser).` },
]
const SCHEMA = { type: 'object', properties: { verdict: { type: 'string', enum: ['PASS', 'PASS_WITH_FIXES', 'FAIL'] }, must: { type: 'integer' }, should: { type: 'integer' }, minutes: { type: 'integer' }, summary: { type: 'string' } }, required: ['verdict', 'must', 'should', 'summary'] }

phase('Final')
const res = await parallel(SEATS.map(s => () => agent(`${WHO}\n\n${s.job}\n\n${PAPER(s.key)}`, { label: 'final:' + s.key, phase: 'Final', model: 'sonnet', effort: 'high', agentType: s.type, schema: SCHEMA })))
const out = {}
SEATS.forEach((s, i) => { out[s.key] = res[i] || { verdict: 'NO RESULT', must: 0, should: 0, summary: 'the seat returned nothing: read its paper on disk' } })
log('Final: ' + SEATS.map(s => s.key + ' ' + out[s.key].verdict + ' MUST ' + out[s.key].must).join('; '))
return out
