# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony, SECONDARY lane, in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL live audio-reactive VJ app).
NEVER cd, from the first command: R=<repo> at the head of every command, absolute paths, git -C; a return code on its own line,
never after a pipe (a pipe gets no rc line at all). CURRENT as of session s-rta-1005 (2026-10-05 13:48 → 17:45).

THE FIRST RULE, BORIS'S OWN (binding-decisions.md, the section "2026-10-05 (s-rta-1005)"; BF126, BF152, BF153): NOTHING IS BUILT.
His words: "Don’t build anything till all questions are answered and you are 100% sure of what everything means." and "After
all is clear, then the following session you will start building the app and not before that." No builder of any lane runs, no
stage, no merge, until HE has said that all is clear. Until then every session is: his answers -> filed word for word -> what
they open -> his page -> end of session.

READ, in this order: (1) .harmony/HANDOFF.md (this file: WHERE WE ARE, the two LOOSE-ENDS LEDGERS, the SCREEN-SAFETY LAW — obey
it, the session section); (2) .harmony/RIG-RULES.md BEFORE launching any workflow (binding; sections A, A2, A3, A4, A5, B);
(3) .harmony/.reports/s-rta-1003/board.md, the rows under "s-rta-1005 LIVE STATE" (the newest); (4) what is in front of him:
.harmony/.reports/s-rta-1005/boris-clarify-all.md = every question AS ASKED and every reading — NEVER load it or
boris-all-items.json whole (110 KB / 400 KB): print one item by its number with python3 from the JSON (keys questions[n],
readings[r], decided_not_asked, design_page, still_unsure, notes_for_harmony). The adoption blocks at the END of the plans
(.harmony/.reports/s-rta-1004/plan-*.md, s-rta-1004b/plan-nudge-row2.md, plan-looks-answers2.md) are read only when their lane
is touched: every lane is ON HOLD and three are RE-OPENED on paper by his words (quantize-out; the tempo row;
presets — ruling-looks-answers2.md stays NOT adopted). History: .harmony/HANDOFF-ARCHIVE.md — never load it whole. His rulings:
.harmony/binding-decisions.md (his words in quotes; the text after "->" is Harmony's); his messages verbatim:
.harmony/boris-feedback-backlog.md (BF1-BF153), byte-exact copies .harmony/.reports/s-rta-1005/boris-msg-raw-1..3.txt, his
pictures s-rta-1005/boris-images/ and s-rta-1004b/boris-images/.

HIS PAGE, opened for him at the close: .harmony/.reports/s-rta-1005/boris-all-questions.html — 45 questions 172-216 and 102
readings R127-R228 in three parts (Part 1, what the first builds stand on: A firing clips and the tempo row, B the cue system,
C presets, D actions, E the review screen and recordings; Part 2: F the show file and decks, G output screens, H how a clip
plays, I effects and signals, J the keyboard and MIDI mapping, menus and messages; Part 3: K sources and the automatic
features), 36 points decided without asking, 35 items that "come next as pictures". He answers in chat: "172 a", "R193 c no:
...", "R130 off", "readings ok", "all defaults good". The page tells him that silence is NOT an answer this time. NONE is
answered yet. His earlier page (150, 151, 154-171) IS answered and filed.

WHEN HE ANSWERS: (a) the turn it arrives: pull his message from the session record by record line and type (python walk, assert
on its first words; pictures extracted and sized with sips), file it verbatim — backlog (next BF154), binding-decisions.md (a
section named by its HEADING, dated), the "## ANSWERS" block of boris-clarify-all.md; the stamp from date in the command that
writes it; (b) that turn launches nothing (a 40 s timer, then launch); (c) then a workflow: for every answer that is not a
letter and every corrected reading, what it opens -> new readings (from R229) and questions (from 217) -> blind seats, ONE
sheet or slice per seat, each writing a paper file -> a ruling (architect, opus max) -> his page -> a page check -> my own read
through a compact printer -> open it for him -> end of session. A list checked by fewer seats than were sent is NOT shown.
Scripts: .harmony/.reports/s-rta-1005/wf/ — samepage.js (fact sheets + a first pass), allq.js (args {leg}: sweep = ten areas of
the app, each re-read; merge = one list; verify = fifteen small seats -> second ruling -> page -> check), dry.js + check.sh
(check and dry-run EVERY script before launch; fix the scratch path inside check.sh). Fact base of this session, each sheet
ending in a "## CORRECTIONS" block that wins over the lines above it: s-rta-1005/facts-*.md (5: Resolume's real behaviour; the
app today for the cue system, presets, takes and BPM-mode fires; actions) and area-*.md (10: the whole app by area).
Law #11: plans = architect, opus high; rulings = architect, opus max while Fable is out (Boris 2026-10-02, verbatim: "just so
you know, we are out of fable usage so you will need to do all fable work with opus 5.5"); readers / seats / checkers sonnet;
builders (none until he says so) opus high. BORIS USES THIS MACHINE AND THIS APP: an Audio-DNA or a Resolume Arena you did not
start is his; yield the turn while workflows run; a question already shown is never re-worded (next free question 217, reading
R229, BF154, Pitfall 69). His name for the key and pad list: "keyboard and MIDI mapping". NEVER load the update-config skill.

STATE: main = this close's docs commits on top of 249a074, pushed; NO source file changed in s-rta-1005. Unchanged since
s-rta-1004b: the show-file protection is in the app (MERGE 1); outputs S1 (lane/outputs-core aa7bc8e, worktree outputs-a) and
nudge S1 (lane/nudge a8afcfb, worktree nudge) are built + gated and NOT merged; lane/one-save 93b49ab (worktree onesave);
worktrees bf2, bf2keys, keying. The build order of the s-rta-1004b close (outputs S2, one-save S7, quantize-out, nudge S1r,
presets, transport) is SUSPENDED by his rule and partly overtaken by his words: it is in HANDOFF-ARCHIVE.md and on the board,
and is re-made by architects after he says all is clear.

START HERE, in order:
1. HIS ANSWERS (he said he starts the next session with them). File them first. If he brings the three DJ tracks ("We will
   have the dj tracks later today": not arrived by the close), file where they are; the measurement itself is a build-side
   task and waits.
2. The next round: what his answers open -> his next page (as above). Part 1 first if he answered only part.
3. ONLY WHEN HE SAYS ALL IS CLEAR — and in the session AFTER that: (a) the pictures page (the 35 "comes next as pictures"
   items: where the action buttons sit, the review screen, the preview monitor, the preset button) THROUGH THE CRITIC PANEL
   (visual, UX, graphic, logic, interaction-logic) before he sees it — ask him earlier if he wants pictures while he answers;
   (b) architect deltas for what his words re-opened: quantize-out against "a clip that has BPM mode enabled will start
   playing on the next 1", the tempo row (a fire starts a stopped beat; a paused start), presets as in his Resolume pictures;
   plans for the new lanes (the cue system, actions, the review screen); (c) builders, at most 3 at once.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA's functions are being pinned down in full with Boris before anything more is built: question-and-answer
  sessions until he says all is clear, then the build.
SHIPPED: this session, on paper only — his answers to the 20-question page and three later messages filed word for word, with
  four Resolume pictures; fifteen fact sheets (what Resolume really does where he said "emulate"; the whole app by area), each
  re-checked by a second reader; ONE page with everything open, opened for him: 45 questions (172-216), 102 readings to
  correct, 36 points decided without asking, 35 items held for pictures. In the app: nothing new.
IN-FLIGHT: none (every started task is finished and filed).
NEXT: his answers, Part 1 first; file them; ask what they open; a new page; repeat. Building starts only in the session after
  he says all is clear.
BLOCKERS: his answers. The three DJ tracks for the clip-timing measurement ("later today"; not here yet).
YOU ARE HERE: question round 1 — the whole app's open points are on one page; 0 of 45 answered; nothing builds until he says
  all is clear.

## LOOSE-ENDS LEDGER — s-rta-1005 (CURRENT)
NOT RUN / NOT MET (reported as such, never as pass):
- MY OWN READ of the final list covered 9 of 45 questions in full (172, 173, 178, 180, 195, 202, 205, 211, 216), the heads of
  readings R214-R228, the intro, and one headless picture of the page's top. The other 36 questions (titles only), the bodies
  of the 102 readings, the 36 decided lines and the 35 picture items were checked by the seats and the page check, NOT by me.
- The second ruling's own texts (R214-R228; questions 202 and 216; the rewritten 180, 195, 205, 211) were seen by NO seat.
- Many readings carry a part marked "Mine:" (26 of them change what he sees on stage; the page lists them): his silence on
  those is not consent — the page says so.
- Five readings are over 320 words (R188, R193, R199, R191, R186), lettered so he can correct one line. The page's own guess:
  about 3 h 20 min for everything; Part 1 about 1 h 50 min.
- The nit "as you ruled" (8 times) is left on the page. My scripted replacement of "read in the code" ran on the list, the
  record and the page together and was re-verified (301 texts, 0 missing on the page).
- Resolume: several facts stay UNKNOWN from its manual (what Save's window starts with, what "Manage..." holds, the second
  "P", a bypassed layer in a monitor): asked of him as reading R206, eight looks in his own Arena. Found, with sources in
  facts-resolume-emulate.md: Resolume shows NO loaded-preset name; one of its monitors shows ONE layer (its team: several
  mixed is not possible) — his cue system goes beyond Resolume; Beat Snap and BPM Sync are separate there. NOT re-read by me.
- No test, gate or app launch ran this session (no build). Every count of the s-rta-1004b close is unchanged and NOT re-run.
- The first-pass page boris-open2.html was never opened for him: superseded, do not open it.
OPEN WITH BORIS:
- The page: 172-216, none answered; readings R127-R228, none confirmed. The pictures page: not offered a date. The tracks.
- Whether "all the documentation and notes about app function" meant the whole app (as done) or the open lanes only.
FILED DEBT (found by the sweeps in the app as it is; each with file:line in its sheet; NOT verified by me):
- Signals and macros have no save path (area-show-decks.md U-7). The Preview / Output tab buttons only change colour. The layer
  strip's transport buttons write the model, their callbacks are unwired (the old ledger says "do nothing"). Five tempo
  buttons (/4 /2 x1 x2 x4), the clip's Restart / Continue / Relative menu, the "Clip Position" source and an envelope's
  Looping / One Shot toggles change nothing. A take cannot start anywhere but 0 and replays through the live renderer. Keys
  and pads fire the deck that is SHOWN. Cheapest test for each: a unit case when its lane opens.
- The s-rta-1004b ledger below: nothing of it was worked; it stands whole.
SESSION / SYSTEM:
- WARN fable-usage-audit LAW11-LOG-GAP: this session's architect dispatches (5) have no DISPATCH_LOG row (a foreign lane
  cannot write it). Session index: skipped (foreign-repo lane, no transport yet).
- Context gauge read at the close: [CTX] 437,252 / 1,000,000 (43.7%) · prev turn (the off-ramp is about 40 %); the page and then the end of session were his explicit instruction.
- T26 STEP 3 NOT MET: this handoff is over 20 KB (two ledgers are carried).
DOUBTS:
- He may answer Part 1 only: the next page must carry forward what he did not reach without re-wording it.
- The cue system as he wants it (several layers mixed in a preview while the output runs) draws the same layers twice in a
  frame: facts-app-cue-today.md lists what that collides with. No promise on it before an architect has ruled.
- Two of his own answers pull apart in several places (the preset's name against Resolume's "P"; "will not play but will still
  display" against "136 b"; 1 beat against bars): each is a question or a reading on the page; the rulings wait on them.


## LOOSE-ENDS LEDGER — s-rta-1004b (STILL OPEN, UNCHANGED: no build ran in s-rta-1005)
NOT RUN / NOT MET (reported as such, never as pass):
- NOT re-run on merged main: the full-disk row OS-L21 and G-OS-HIS (both PASS on the lane's app at the merged code head); owed
  at MERGE 2. G-OS-HIS shows "the same show" only through GET /api/composition's 10.8 KB view, not the whole 43 KB file.
- outputs S1: "exactly its named case RED" is NOT MET as written by 9 of 22 arms (read as "the named case is RED", H-O3); U-P2's
  arm was replaced AFTER its result was seen (H-O2); "the app is unchanged" is INFERRED (no live row is ruled for S1).
- nudge S1: STOP-N1 (INFERRED, not run: the old bar label for up to a beat) and STOP-N2 (MEASURED by the builder: one repeated
  hop per beat at a small later nudge in Auto) wait for the architect; G-N0's RED arm is owed at S2.
- The publish rate (G0) was measured with Arena running: 93-99 a second on the 120 Hz screen. Quiet re-run owed before S5.
- one-save: MU-OS-21's live arm cannot bite (the unit row SW-2 carries the read-back); my first 185147b arm of the full-disk
  row was INVALID (the probe's fill step is not re-runnable on one image), the re-run on a fresh image is the valid one.
- ruling-nudge-row2.md was ADOPTED at 50 % context on his word "don't lose any decisions": I read its verdict, stage list,
  decisions and section 7, NOT sections 1-6 and 8-10. ruling-looks-answers2.md is NOT adopted.
- His page: the "what only you can check now" lines are INFERRED (nothing was looked at on screen; menu names not confirmed);
  the defaults of 157, 160, 164, 166 are the architect's guesses; question 163's option B may sit against his own rule on who
  owns a clip's sliders (left for him); I looked at the page's top only (one headless picture).
- The two fact sheets were re-read by a second reader, not re-derived by me; each lists UNKNOWN-NEEDS-A-RUN items.
- Every ctest verdict of mine is SERIAL. Parallel runs are known RED on main (debt below).
OPEN WITH BORIS:
- The page: 150, 151, 154-171. Readings told and not corrected (INFERRED consent): R85-R108 except where a later answer
  replaced one (R99 by his clarification; R87 / R100 = question 154 now); R109-R126 are on the page. Tracks requested.
- DONE AT HIS WORD: ~/Library/AudioDNA/Presets/fast_saves (FX_Save_1..9.json; checksums in the work log) moved to the Trash
  through Finder. His show's copy stays in .harmony/.reports/s-rta-1004/boris-show-backup/ (not committed).
FILED DEBT (found, not fixed; file:line where known is in s-rta-1004b-work.md and the stage reports):
- Until MERGE 1 main EMPTIED THE SHOW FILE on a full disk (measured). Fixed for the show. Four other writers are still
  unverified (a take's file, the audio store's list, the file browser's favourites, MilkDrop's data).
- "Save Deck As" / "Load Deck" then "Save Deck" can still write over a show with no copy (until one-save S4b). A test-mode app
  still lists his real library folders, read-only (until S4b). Collect Media drops a failed save silently. S1 writes the
  "keys" and "layout" blocks EMPTY until S2 / S3 (as ruled).
- main's AppSettings test cases collide when run side by side (4 of 5 runs RED at -j 4); the one-save stage renamed its
  fixture folder per process: whether main is now parallel-safe is NOT re-measured.
- The race detector reported nothing for a deliberately unsynchronised object on a test's own stack: existing [tsan] cases of
  that shape may have no teeth (tsan-r5).
- The top-bar Quantize menu does not follow a loaded show; a clip with its own Snap = Bar and no steady beat may wait for ever
  (UNKNOWN-NEEDS-A-RUN): both leave with quantize-out. The layer strip's three transport buttons do nothing. A take records
  neither a mouse-moved slider, nor the strip's B / S buttons, nor a preset load. APP-INVENTORY's test count is stale.
- Plus the s-rta-1004 ledger's debts (archived), unchanged except the full-disk line above.
SESSION / SYSTEM:
- WARN fable-usage-audit LAW11-LOG-GAP: this session's architect dispatches have no DISPATCH_LOG row (a foreign lane cannot
  write it). Session index: skipped (foreign-repo lane, no transport yet). T26 STEP 1-2 open; STEP 3 NOT MET: this handoff is about 21 KB (target 20).
- Worktrees: onesave, outputs-a, nudge (each with build-lane, some with build-tsan / build-mut-*), bf2, bf2keys, keying.
  289 GB free at the close.
DOUBTS:
- Twenty questions on one page may be more than one sitting; the ones that unblock a build come first (150, 151, 154).
- Reading R105 says "all layers" go quiet under a global action: his words name "the layer action"; it is question 155.
- Whether "stop clears all clips" should also stop a take that is replaying (question 170).
- The actions model is large and re-shapes recording, routines, the layer strip and the clip grid: do not plan any of them
  before the design page has been in front of him.

## SCREEN-SAFETY LAW — MANDATORY, EVERY SESSION, NO EXCEPTIONS

**Ratified by Boris 2026-08-03 after a gate session left a black overlay on his displays.**
This is a LAW, not a preference. It applies to Audio-DNA work in every session, primary or
secondary, and it applies to any agent you dispatch.

### Why it exists
Audio-DNA's output window is a REAL FULLSCREEN WINDOW ON BORIS'S ACTUAL MONITORS. It is not
a headless test artifact. Opening it in an automated gate has immediate, visible
consequences on the machine he is working on. Session 2026-08-03a opened and closed it three
times across two launches while gating C3, `pkill`ed the app repeatedly, and left it open —
and Boris ended up with a black overlay on every non-fullscreen screen that OUTLIVED a clean
exit of the app.

### The law
1. **NEVER end a session with the output window open.** Closing it is part of EOS, not an
   optional courtesy. Close via `Output > "All Outputs Off"` (or Cmd+Shift+Esc), then confirm.
2. **NEVER `pkill` / SIGKILL the app while the output window is open.** Close the window
   FIRST, let it tear down, THEN quit. Killing mid-fullscreen is the suspected trigger for
   the orphaned overlay.
3. **VERIFY THE SCREEN, NOT JUST THE PROCESS.** `pgrep` returning empty does NOT mean the
   screen is clean — this incident proves it. Before declaring a session safe to close,
   run `screencapture -x /tmp/eos-screen.png` and READ THE IMAGE. No CLI probe can see a
   black overlay, a TCC dialog, or a stuck window. This is the same class as the 2026-07-25
   TCC-prompt gotcha: the screen holds state that no socket or process check reveals.
4. **STATE THE APP STATE IN THE HANDOFF.** Every session that launched Audio-DNA must say
   explicitly, in its handoff, what state the app and its windows were left in, and whether
   the screen was visually verified clean.
5. **MINIMISE fullscreen output-window drive in automated gates.** If a gate needs the
   output window, open it, take what you need, close it immediately — do not leave it open
   across other work. Prefer probe states that do not require it when they answer the same
   question.
6. **IF BORIS REPORTS A SCREEN ARTIFACT, IT OUTRANKS THE LANE.** Stop, clean up, diagnose.
   His machine is not a test rig.

### EOS checklist addition (do this before writing "safe to close")
```
osascript -e 'tell application "System Events" to tell process "Audio-DNA" to click menu item "All Outputs Off" of menu 1 of menu bar item "Output" of menu bar 1'   # close every output window
pkill -f Audio-DNA ; sleep 2 ; pgrep -f Audio-DNA        # then quit, confirm gone
screencapture -x /tmp/eos-screen.png                      # AND LOOK AT IT
```
Report in the handoff: windows closed, process gone, screen visually verified.
(The black-overlay hypothesis and its fix notes: .harmony/HANDOFF-ARCHIVE.md "BLACK-OVERLAY BUG". Later practice, binding:
no full-screen capture — count Audio-DNA / Output / UserNotificationCenter windows with Quartz after every batch; launch
only with open -g; no gate ever opens an Output window. See .harmony/RIG-RULES.md.)

# >>> SESSION s-rta-1005 (2026-10-05 13:48 → 17:45, secondary) — START HERE <<<

## THE ONE-LINE VERSION
Boris answered the page of twenty questions, added a cue system and new rules for actions, presets and the review screen, and
stopped all building until every function is understood; this session filed his words, read the whole app's documentation and
rulings by area, and put everything still open on ONE page for him. Session log
.harmony/sessions/2026-10-05-s-rta-1005-secondary.md; work log .harmony/s-rta-1005-work.md; everything else
.harmony/.reports/s-rta-1005/.

## WHAT BORIS RULED TODAY (his words: binding-decisions.md, the section "2026-10-05 (s-rta-1005)")
- NOTHING IS BUILT until all is clear; question-and-answer sessions until then; the build starts the session after.
- Defaults accepted: 151, 155, 156, 157, 158, 165, 166, 167, 171. His own words on 150, 154, 159 b, 160, 161, 162 b, 163, 164,
  168, 169, 170; R111 replaced; R117 and R116 fall away ("Just replace with action."); R123 yes, an action follows the tempo.
- NEW: delete / copy / cut / paste for actions; saving one without its ignored controls; a small Save button on a changed
  preset; presets and the effects tab as in his Resolume pictures; the review screen is a screen of its own; a fire while the
  beat is stopped starts the beat; a CUE SYSTEM (a preview monitor, a cue button per layer, a click on a clip's name previews
  it); the key and pad list is called "keyboard and MIDI mapping".

## VERIFICATION — PROVEN, AND HOW
- By me: every quoted span of his in the filings asserted byte-exact against the session record's message before the write;
  each workflow script syntax-checked and dry-run before launch; the final list's structure by script (one default per
  question; numbers gapless); the page against the list (301 texts, 0 missing) after my one scripted wording change.
- By agents, not by me: 15 fact sheets, each re-read (wrong lines corrected in a block at the sheet's end); the first-pass
  list (3 seats, 135 lines ruled); the merged list (1 seat, then 15 small seats; 185 findings ruled, 0 rejected); two page
  checks (the last: SOUND, 629 item texts, 0 mismatches, his words verbatim).

## NOT VERIFIED — WHAT ONLY BORIS CAN CHECK
Everything on the page is his to answer or correct. Eight things only his own Resolume Arena can show (reading R206). Nothing
new is in the app to look at.

## MY OWN ERRORS THIS SESSION — recorded because no gate would surface them
1. A return code printed after a pipe, again. 2. My first workflow resume re-started four finished re-reads: a call's cache key
depends on call order (journal read; fixed by issuing calls in a fixed order). 3. I gave two checkers fifteen sheets each; both
returned nothing and the ruling ran on one paper — caught only by that ruling's honest PARTIAL; re-cut into fifteen small seats.
4. The turn that filed his long message cost 11 % of the context window (I read ruling sections myself). 5. I told him "roughly
an hour and a half to the page"; it took 2 h 55 min. Habits: RIG-RULES.md A5 and the notebook.

## SCREEN STATE AT CLOSE (screen-safety law #4)
This session and its agents launched NO Audio-DNA and touched no Resolume Arena (every prompt forbade it); no probe, no gate,
no Output window. At the close: no Audio-DNA process (pgrep: none). One page was opened in his browser in the background
(boris-all-questions.html). One headless browser picture of that page was taken by me (not a screen capture).

## COUNTS — run them, never inherit them
Nothing was run this session. From the s-rta-1004b close, NOT re-run: ctest on main 1278 (serial); lanes outputs-a 1272,
nudge 1276, onesave 1278.
