# usage: python3 mk_handoff.py  -- writes the s-rta-1009 handoff: the new sections, the SCREEN-SAFETY LAW kept byte-exact, the s-rta-1007 sections moved to HANDOFF-ARCHIVE.md
import re, os, sys
R = '/Users/boriskarpman/projects/RealTimeAudio'; H = R + '/.harmony/HANDOFF.md'; A = R + '/.harmony/HANDOFF-ARCHIVE.md'
old = open(H, encoding='utf-8').read()
assert 'CURRENT as of session s-rta-1007' in old, 'the handoff on disk is not the s-rta-1007 one: stop'
i = old.index('## SCREEN-SAFETY LAW'); j = old.index('## THE ONE-LINE VERSION')
law = old[i:j].rstrip('\n')
k1 = old.index("THE FIRST RULE, BORIS"); k2 = old.index('READ, in this order:')
rules = old[k1:k2].rstrip('\n')
assert rules.count('"Review"') == 1; rules = rules.replace('"Review"', '"Studio"')
fable = re.search(r'"just so\s+you know, we are out of fable usage so you will need to do all fable work with opus 5\.5"', old).group(0).replace('\n', ' ')
counts = old[old.index('## COUNTS'):].rstrip('\n')
P = R + '/.harmony/.reports/s-rta-1009'
new = f'''# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony, SECONDARY lane, in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL live audio-reactive VJ app).
NEVER cd, from the first command: R=<repo> at the head of every command, absolute paths, git -C; a return code on its own line,
never after a pipe -- when a return code matters use bash <R>/.harmony/.reports/s-rta-1009/wf/rc.sh <max chars> <command ...>
(it prints rc= first, then the end of the output). CURRENT as of session s-rta-1009 (2026-10-09 18:24 -> 21:55).

FIRST MOVE, BEFORE THE READS BELOW — A QUICK BOOT (his order of 2026-10-08; the format is standing). His page 3 has a box
under every answer and item and the button "Save my answers for Claude" at its bottom: it writes
~/Downloads/audio-dna-page3-answers.txt (the browser adds " (1)" on a second save) and copies the text to the clipboard; EVERY
BOX LEFT EMPTY COUNTS AS ACCEPTED, except item 277, which asks him to type yes, b or c. So: (1) read only this prompt, then
run python3 ~/Harmony_Main/scripts/boris-page.py read --project audio-dna --page page3 --page-file
{P}/boris-page-3.html --copy-to <R>/.harmony/.reports/<the new session's folder>   (make the folder first; --copy-to takes a
FOLDER). Exit 0 = found: it prints the file, when it was saved, its build against the page's, and every comment; 3 = no file
yet; 4 = not well-formed: read the file itself, never guess; 2 = a path is wrong. (2) No file yet: tell him in two lines --
type only where something is wrong or missing, press "Save my answers for Claude" at the bottom -- open the page for him
(open -g {P}/boris-page-3.html) and WAIT IN THE SAME TURN (the loop of the boris-page skill, step 5: the reader with --quiet,
5 s apart, at most 9 minutes per command; repeat while he works). If macOS asks whether the terminal may read the Downloads
folder, that prompt is his: say so first. If no file comes, he pastes (skill boris-page, "Pasted instead of saved").
(3) The file is there: read back to him in ONE line when it was saved and how many comments it holds; it IS his message --
file it word for word (backlog from BF273; binding-decisions.md, a dated section) as a chat message would be; a build that
differs from the page's, or an item listed as "left empty although the item asks a question" (277), is SAID to him, never
guessed. (4) Only then the reads below, and the work. Answers that came as a file inside a waiting turn are not a chat message:
that turn may launch (the 40 s timer is for a message that arrives in chat). THIS IS THE FIRST PAGE RENDERED BY THE SHARED TOOL:
its Save press in his browser was never exercised; if it misbehaves, he pastes and you file a defect.

{rules}

READ, in this order: (1) .harmony/HANDOFF.md (this file: WHERE WE ARE, the LOOSE-ENDS LEDGER, the SCREEN-SAFETY LAW — obey it,
the session sections; the s-rta-1004b ledger of BUILD-SIDE debts is in .harmony/HANDOFF-ARCHIVE.md under the heading
"LOOSE-ENDS LEDGER — s-rta-1004b (STILL OPEN; moved at the s-rta-1007 close)": read it before any build, merge or gate);
(2) .harmony/RIG-RULES.md BEFORE launching any workflow (binding; sections A to A7, B); (3) .harmony/.reports/s-rta-1003/board.md,
the rows under "s-rta-1009 LIVE STATE"; (4) what is in front of him: {P}/boris-page-3.html — read its text
boris-page-3.txt whole (about 3,000 words): 7 answers to what he asked, 15 assumptions that matter (274-288), 15 one-line ones
(289-303), a box under each (38 with "Anything else?"). Its list page3-items.md is MADE, never edited by hand: page3-round1.md
(the list ruling) + page3-edits.md (the second ruling, 131 blocks) + page3-edits2.md (my blocks after the last read; made by
wf/mk_edits2.py) + page3-frame.md, by wf/page3_build.py --build; the provisional ids (P1, NEW-2 ...) map to the numbers in
page3-items.md.map.json; each item's FROM line names the assumption blocks it is made from.
THE PLANNING TRUTH after his answers to page 2 (never load a file of it whole): {P}/spec-<topic letter>.md = every item with
THE RULE NOW, merged by wf/merge3.py from the s-rta-1007 specs (untouched) + this session's apply-<T>.md and rule-<T>.md (A
triggering clips and the tempo bar, B the cue system, C presets, D actions, E Studio and the recordings, F the show file and
decks, G output screens, H how a clip plays, I effects and signals, J the keyboard and MIDI mapping, K sources and the automatic
features, X the loose lists); one item: grep -n -A 9 "^@@ITEM <id>" spec-<letter>.md. ledger3.md = one line per item of page 2;
assume3-all.md = the 119 assumptions open after the rulings (16 to ask, 25 one-line, 78 internal: NOT on his page), with each
topic ruling's notes; answers3-all.md; names3-all.md (41 name blocks, already in .harmony/NAMES.md); today3-notes.md = MY notes
on what the app does now (never shown to him). His words: .harmony/binding-decisions.md (quotes are his; the text after "->" is
Harmony's; the section "2026-10-09 (s-rta-1009)" ends in the block "APPLIED": every item of page 2 with the rule now),
.harmony/boris-feedback-backlog.md (BF1-BF272), byte-exact {P}/boris-answers-page2.txt, by box answers-by-item.md. History:
.harmony/HANDOFF-ARCHIVE.md — never load it whole. docs/claude/milkdrop.md = how MilkDrop works now (he designs the new one
himself while the build runs: BF265; nothing of MilkDrop is planned or built before his design).

WHEN HE ANSWERS PAGE 3: (a) file it the turn it arrives -- wf/file_answers.py + an entries table by BOX (it asserts that every
content line of his is filed; it was written for page 2's file: copy the wf folder into the new session's folder and change
the page constants). (b) A chat message of his launches nothing in its own turn (40 s timer). (c) Apply with this session's
chain, all in {P}/wf/ (copy, then change "page2 / 217-273" to "page3 / 274-303" and the base specs to s-rta-1009's):
mkslices3.py -> apply3.js (12 architects high + blind checkers + rulings max; @@AMEND blocks change an old rule by an exact
piece found once; lint3.py) -> merge3.py (dry run first) -> page3.js (list ruling -> six checkers -> second ruling; made by
mk_page3.py from page3.body.js) -> page3_build.py --check / --build (set FIRST = 304) -> boris-page.py render --page page4 ->
final3.js (three seats read the RENDERED page) -> my own read -> one seat on my own edits -> open it -> end of session.
check.sh + dry.js on EVERY script (fix the scratch path in check.sh); check_quotes.py needs his files as arguments. A round
costs about 3.5 hours (two runs of about 82 minutes). A RULING NEVER REWRITES A LIST WHOLE (RIG-RULES A6).
IF HE SAVES WITH EVERY BOX EMPTY or writes "all good": that answers the page EXCEPT item 277 (the saved file lists it; say so);
it is NOT "all is clear". Tell him in a few lines what is still open, then let HIM say that all is clear: (1) NOT MEASURED --
how well the app finds the tempo and the 1 on his music, and whether a hand correction holds in automatic mode (waits for
the audio he will give: BF272, "3 10 min audio clips and a longer set"; MP3 is on the app's list, M4A is not: make WAV copies
with /opt/homebrew/bin/ffmpeg, his files untouched); the cost of the cue system on his heaviest show (frame time, master cue,
one previewed clip: needs HIS app running, so it is arranged with him, before the cue system is built); whether the
low-resolution show recording makes the live picture stutter; (2) WAITS FOR AFTER THE BUILD by his own words (BF240, BF241): the
codec test (jump wait, HAP copies or not, the low-resolution recording's codec, 30 or 15 pictures a second); (3) masks: what a
mask does is asked when masks are planned (item 301): "all clear" does not cover masks; (4) the items whose default follows his
newest words against earlier ones (274, 282, 292, 298, 300) and the three whose default is my reading (277, 287, 299);
(5) owed by Harmony before the build, not for him: the Preferences window, the store of recorded sound, the remote-control
port (page3-edits.md, "TO SAY IN CHAT", point 8); which MIDI controller he uses, and how endless knobs send their steps (a
Researcher task before the mapping is built: BF270).
Law #11: plans = architect, opus high; rulings = architect, opus max while Fable is out (Boris 2026-10-02, verbatim: {fable});
readers / seats / checkers sonnet; builders (none until he says so) opus high. BORIS USES THIS MACHINE AND THIS APP: an
Audio-DNA or a Resolume Arena you did not start is his; yield the turn while workflows run; an item already shown is never
re-worded in place (next free: page item 304, page id page4, BF273, Pitfall 69). NEVER load the update-config skill.

STATE: main = this close's docs commits on top of 37c151f (33989a8 filing, ce6d4ab apply + rulings + merged specs, 8e32299
page 3 first build, then the close), pushed; NO source file changed in s-rta-1009 (docs only: .harmony/NAMES.md brought up to
date; RIG-RULES A7). Unchanged since s-rta-1004b: the show-file protection is in the app (MERGE 1); outputs S1 (lane/outputs-core
aa7bc8e, worktree outputs-a) and nudge S1 (lane/nudge a8afcfb, worktree nudge) are built + gated and NOT merged; lane/one-save
93b49ab (worktree onesave); worktrees bf2, bf2keys, keying. Every lane is ON HOLD; the old build order is suspended and is
re-made by architects from the specs after he says all is clear.

START HERE, in order:
1. HIS ANSWERS to page 3 (the quick boot above). File them first. If he brings the audio, file where it is.
2. Apply them; ask only what they open (page 4, shorter still); or, if nothing is open, tell him so and what is unmeasured.
3. ONLY WHEN HE SAYS ALL IS CLEAR — and in the session AFTER that: first read the s-rta-1004b ledger in the archive (the
   build-side debts and what is owed at MERGE 2); then (a) the measurements that need no build (his audio; the cue system's
   cost with him); (b) "delete all the old show files" (item 235, accepted): list the files to him first, then move them to
   the Trash through Finder (RIG-RULES A3), never rm; (c) architect plans, lane by lane, from the s-rta-1009 specs (the tempo
   bar and triggering; the cue system; presets; actions; Studio and the recordings; then the rest; the blend / keying / mask
   lists last: BF260 "built when there is time"); (d) builders, at most 3 at once; (e) after the build, his codec test.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA's functions are being pinned down in full with Boris before anything more is built: question-and-answer
  rounds until he says all is clear, then the build.
SHIPPED: this session, on paper only — his 33 comments on page 2 filed word for word (BF240-BF272); every item of page 2 now
  has a rule (30 accepted, 21 answered, 6 that he asked back); 219 changes to 116 older rules; the research he ordered on the
  hold setting; answers to the 8 things he asked; the names list brought up to date (the screen is Studio); page 3, opened for
  him: 7 answers + 30 assumptions (15 that matter, 15 one-liners), about 25 minutes. In the app: nothing new.
IN-FLIGHT: none (every started task is finished and filed).
NEXT: his answers to page 3 (the file his Save button writes, picked up at boot) -> filed -> applied -> a shorter page 4 or
  "nothing is open". Building starts only in the session after he says all is clear.
BLOCKERS: his answers (item 277 needs a typed yes, b or c). His audio (3 ten-minute clips and a longer set, not here yet).
  Measurements nobody has made (tempo and the 1 on his music; the cue system's cost; the codec test, which he put after the build).
YOU ARE HERE: question round 3 — page 3 is in front of him, 0 of 30 answered; nothing builds until he says all is clear.

## LOOSE-ENDS LEDGER — s-rta-1009 (CURRENT)
NOT RUN / NOT MET (reported as such, never as pass):
- MY OWN READ covered the whole FIRST build of page 3 (7 answers, 28 items) before the last read. After the fold-in (26 blocks
  changed or new) I read the changed items as rendered, spot-checked the file (38 boxes, 1 must-answer box, 3 web anchors, 0 raw
  links) and looked at one headless picture of its top; I did not re-read the whole page. One seat read my 26 blocks and the
  frame: CLEAR TO SHOW, 0 MUST, 6 SHOULD. The six SHOULDs I then took (five wordings: 299, 300, answers 222, 252, 246-hold; one
  clause of the frame) were seen by NO seat: lint and render only.
- NOT read by me: the 12 apply papers, their 12 blind checks, the 12 rulings (returns and lint only; two pairs of amendments
  read for a clash: none), the merged specs, assume3-all.md (three blocks), the three research papers and the paper on item 252.
  They rest on the checkers and the rulings. 8 checker findings were rejected by rulings (topic B 1, J 2, X 2; page: cover F7,
  F11, logic F11 -- the last one re-opened by the final read: now item 291).
- Defaults of page 3 that follow his NEWEST words against something earlier, so that a quick "all good" builds the new reading:
  274 (the app may place the 1 again after the press that starts the beat; before: "click on play or any clip is the new 1"),
  282 (All Outputs Off lasts through a quit), 292 (a previewed clip's actions start at the click), 298 (a deleted clip leaves
  at once, also a removed column or deck), 300 (blend modes and keying stay). Defaults that are MY reading where rulings pulled
  apart: 277 (a clip's action buttons stay on at a stop: topic D's ruling had it the other way; he is asked outright), 299 (a
  Snapshot remembers the MilkDrop preset: turned by me at the last read, against the topic rulings, on his "the output and
  everything"), 287 (a knob that sends its position makes a slider jump: inferred consent, his "work smoothly" may mean no).
  274's main text is the middle reading of his "b" + "always" (the list ruling's choice; ways b and c carry the two letters).
- NOTHING IS MEASURED: every number on the page is an estimate and says so. Facts about the app in the answers are READ in the
  program by a seat, never run: MP3 is on the open-file lists, M4A is not (the Mac's own decoder probably reads it: not tried);
  no cue button exists; the S button is solo; Render is a disabled "coming" item. The three web links were fetched by a seat
  (HTTP 200, content matched, 2026-10-09), not by me.
- THE SHARED TOOL'S SAVE PRESS IS NOT EXERCISED: page 2's proven press was the project's own renderer. Defect found in the tool
  and reported up (idea ledger, id idea-2026-10-09-RealTimeAudio-17915970579955911095): an answer's TEXT is not run through
  its inline step, so links and bold in an answer print raw; worked around with @@PAGE-LINK blocks. Its line beside the Save
  button says flatly that every empty box counts as accepted; the page says three times that 277 does not.
- NAMES.md was edited by one sonnet seat from the 41 ruled name blocks (20 changed, 2 renamed, 12 added, 17 other rows touched)
  and checked by me by grep only; I took 6 block ids and a line of rule ids out of his copy and mended the Output menu row.
  Open: "Studio picture" lost its "resize by dragging" clause in the list (the rule stands in spec-E); "K slider" is neither
  retired nor ruled; my picks among the names are unconfirmed (the page says nothing in the list has to be read now).
- Not taken from the last read: the chair seat's merge of 301 into 300, its cut of 293, its cut list (the delta seat: harmless).
- No test, gate or app launch ran this session (no build). Every count of the s-rta-1004b close is unchanged and NOT re-run.
OPEN WITH BORIS:
- Page 3: 7 answers to read, items 274-303, none answered; 277 needs a typed answer. His audio (BF272). Which MIDI controller.
FILED DEBT (found in the app as it is; NOT verified by me):
- today3-notes.md and the s-rta-1007 today-notes.md (what the app does now, item by item); the MilkDrop document's "What is
  odd or broken now"; the s-rta-1004b ledger of build-side debts STANDS WHOLE (nothing of it was worked): HANDOFF-ARCHIVE.md,
  heading "LOOSE-ENDS LEDGER — s-rta-1004b (STILL OPEN; moved at the s-rta-1007 close)". The s-rta-1007 ledger moved to the
  archive at this close ("[s-rta-1007] LOOSE-ENDS LEDGER"); of it STILL OPEN: the readings he did not name stand by inferred
  consent; Collect Media says nothing when files could not be copied (one question when that lane is planned); first-round
  items 256, 265, 274, 266 dropped as settled can each return as one line; Ableton Link's licence is not looked up.
SESSION / SYSTEM:
- Idea ledger +2 records for the primary (the tool defect above; the proof that the real Save press works + what stays local),
  ids ending 5911095 and 0130175. The primary's routed inbox item (shared tool at page 3) is DONE.
- WARN fable-usage-audit LAW11-LOG-GAP: 28 architect dispatches this session have no DISPATCH_LOG row (a foreign lane cannot
  write it). Session index: skipped (foreign-repo lane, no transport yet).
- Context gauge read before the close writes (scripts/ctx-now.sh): [CTX] 271,202 / 1,000,000 (27.1%) · prev turn. The
  conversation was compacted once in mid-session (after the apply run was launched).
DOUBTS:
- He may take "all good" for "all is clear": the page's last paragraph keeps them apart; say it again in chat.
- 30 items may still be more than he wants; the 15 one-liners can become mine to decide if he says so.
- Item 277 forces a typed answer; if he saves without it, the reader lists it: ask it in chat in one line, never guess.
- The cue system as he wants it draws layers twice in a frame; answer 252 promises the same fps and names the limit (a preview
  that needs its own drawing freezes first); nobody has run it.

{law}

## THE ONE-LINE VERSION
Boris answered page 2 on the page itself (33 comments, the rest accepted); this session filed his words, applied them to every
item (one architect per topic, a blind checker and a ruling for each), had the hold setting researched as he ordered, answered
the 8 things he asked back, and put what his answers opened on a shorter page 3 (7 answers, 30 assumptions), checked by six
lenses, ruled, re-read as rendered by three seats, and opened for him. Session log
.harmony/sessions/2026-10-09-s-rta-1009-secondary.md; work log .harmony/s-rta-1009-work.md; everything else
.harmony/.reports/s-rta-1009/.

## WHAT BORIS RULED (his words: binding-decisions.md, the section "2026-10-09 (s-rta-1009)"; backlog BF240-BF272)
33 comments, each filed with its box and line; the 31 items and 3 answers he left empty are accepted as written. In short (the
rule in full: the "APPLIED" block and spec-<letter>.md): the screen is Studio; the app always tries to find the 1 and he
corrects; a previewed clip never waits for the beat; tempo stop stops all actions and removes all clips from all layers; a
clip's play-once action keeps its button on and plays again only when the clip is re-triggered; a Snapshot is a save of the
whole show exactly where it is; pasting over or deleting a clip removes it from its layer; all blend modes and keying stay as
placeholders and masks are added later; two kinds of envelope (along the beat; along the playhead in Timeline mode); MilkDrop
stays as it is and he designs its new system himself; both kinds of knob must work, endless ones preferred; the codec test
comes AFTER the build; he asked what master cue is, what "shifting the beat" and "come first" meant, why HAP copies at all,
and ordered research on the hold setting; he will give 3 ten-minute audio clips and a longer set.

## VERIFICATION — PROVEN, AND HOW
- His answers: the saved file copied byte-exact (cmp rc 0; sha256 189a4e9d43a39ce4...), 33 of 33 content lines filed (asserted by
  wf/file_answers.py), every quote in a workflow prompt verbatim against his files (wf/check_quotes.py: 30 of 30, 32 of 32).
- The apply run: 43 of 43 papers on disk; wf/lint3.py OK on all 12 papers and all 12 rulings, run by me.
- The merge: dry run, then real: 57 of 57 page items ruled, 33 of 33 boxes cited, 8 of 8 answers, 219 amendments applied and 0
  not applied; the s-rta-1007 specs untouched (git status: 0 lines).
- Page 3: wf/page3_build.py LINT OK (0 problems; 8 word-count notes under the hard limits); boris-page.py render and lint OK;
  the new texts' quotes asserted verbatim by script (8 of 8); in the rendered file 38 boxes, 1 must-answer box (277), 3 web
  anchors, 0 raw links; one headless picture of the top looked at.
- NOT a proof of anything in the app: nothing was built, run or measured.

## NOT VERIFIED — WHAT ONLY BORIS CAN CHECK
Everything on page 3 is his to strike or to leave; item 277 needs his typed yes, b or c. Whether the Save button of this page
writes the file in his browser (first page made with the shared tool). Whether the three links at the bottom open for him.
Whether the names in the list read right to him (Studio, Snapshot). Nothing new is in the app to look at.

## MY OWN ERRORS THIS SESSION — recorded because no gate would surface them
- I told him "mp3/m4a both fine" before the check came back; M4A is not on the app's list of sound files. Corrected on the
  page and in chat. Habit (RIG-RULES A7, notebook): an answer says only what is verified at that moment.
- A return code after a pipe, again (the render line, the two idea-capture lines): the outputs proved the runs, the rc lines
  proved nothing. Fourth session with this slip; the rule alone did not hold, so the fix is a tool: wf/rc.sh (prints rc= on
  its own line, then the end of the output), named in the birth prompt's first lines.
- check_quotes.py run without his files reported 32 false alarms and cost a turn; it now refuses to run without them.
- --copy-to of the reader given a file name instead of a folder (boot): FileNotFoundError, fixed at once.
- The first build of page 3 had passed six checkers and two rulings and still held 8 must-fix points (an unreadable
  must-answer item, a default against his words, quotes used too boldly, dead-looking links): found only because three seats
  read the RENDERED page afterwards. That read is now a rule (RIG-RULES A7).

## SCREEN STATE AT CLOSE (screen-safety law #4)
This session and its agents launched NO Audio-DNA and touched no Resolume Arena (every prompt forbade it); no probe, no gate,
no Output window. One page was opened in his browser in the background (boris-page-3.html, 2026-10-09 21:50). One headless
Chrome picture of that page was taken by me (its own process, stopped by its own id; not a screen capture). Audio-DNA
processes at the close: none (pgrep count 0).

{counts}
'''
arch_src = old[old.index('## NEXT-HARMONY'):i] + old[j:]
arch_src = re.sub(r'(?m)^## ', '## [s-rta-1007] ', arch_src); arch_src = re.sub(r'(?m)^### ', '### [s-rta-1007] ', arch_src)
arch = open(A, encoding='utf-8').read()
assert '[s-rta-1007] NEXT-HARMONY' not in arch, 'the archive already holds the s-rta-1007 handoff'
open(A, 'w', encoding='utf-8').write(arch.rstrip('\n') + '\n\n# s-rta-1007 HANDOFF, as it stood (moved at the s-rta-1009 close; the SCREEN-SAFETY LAW stayed in HANDOFF.md)\n\n' + arch_src.rstrip('\n') + '\n')
open(H, 'w', encoding='utf-8').write(new)
print('HANDOFF.md: %d lines, %d bytes (was %d lines); archive +%d lines; law kept: %s; his rule quotes kept: %s' % (new.count('\n'), len(new.encode()), old.count('\n'), arch_src.count('\n'), law in new, 'Don’t build anything till you are clear' in new))
