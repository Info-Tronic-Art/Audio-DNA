# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony, SECONDARY lane, in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL live audio-reactive VJ app).
CURRENT as of session s-rta-1004 (2026-10-04 11:53 → ~16:20). READ, in this order: (1) .harmony/HANDOFF.md (this file: WHERE WE
ARE, the LOOSE-ENDS LEDGER, the SCREEN-SAFETY LAW — obey it, the session section); (2) .harmony/RIG-RULES.md BEFORE launching
any lane, probe or gate (binding; sections A, A2, A3, B); (3) .harmony/.reports/s-rta-1003/board.md (the lane board); (4) the
ADOPTION blocks at the END of each plan in .harmony/.reports/s-rta-1004/ (they are short and they bind): plan-one-save.md,
plan-outputs.md, plan-nudge.md + plan-nudge-row.md, plan-transport-delta2.md + plan-transport-answers.md, plan-effect-looks.md +
plan-looks-answers.md. Each plan's ruling-<lane>.md is the builders' spec: never load one whole in the main loop. History:
.harmony/HANDOFF-ARCHIVE.md — never load it whole. Boris's rulings: .harmony/binding-decisions.md, the nine sections headed
"2026-10-04 (s-rta-1004)" (his words in quotes; the text after "->" is Harmony's); his messages verbatim with each question as
asked: .harmony/boris-feedback-backlog.md (BF38-BF91) and .harmony/.reports/s-rta-1004/boris-clarify-*.md. Ultracode: use
workflows — .harmony/.reports/s-rta-1004/wf/plan-lane.js (args {key}: architect plan -> blind seats -> ruling; the lanes table
is inside), recon-topic.js (args {key}: a fact sheet + an adversarial re-read), facts.js; for a BUILD stage adapt
.harmony/.reports/s-rta-1003b/wf/bf2-stage.js (one stage by args: builder -> pinned reviews -> <= 1 fix round) with a new
scratchpad path. Law #11: plans = architect, opus high; rulings = opus max while Fable is out (Boris 2026-10-02, verbatim:
"just so you know, we are out of fable usage so you will need to do all fable work with opus 5.5"); builders opus high;
reviewers / seats / readers sonnet. BORIS USES THIS MACHINE AND THIS APP: an Audio-DNA you did not start is his; yield the turn
while lanes run; file his words verbatim the turn they arrive, the stamp taken from `date` in the command that writes it; a
question already shown to him is never re-worded in place (new numbers; the next free one is 135). NEVER load the
update-config skill.

STATE: nothing was built and nothing was merged this session (main = 185147b + this close's docs commit; ctest not run).
Boris re-ruled the product in one sitting (his rules BF38-BF91, 11 screenshots). THE SYNC DIAL IS DEAD (rulings-bf2.md H-17): lane/bf2
740b6d6 and lane/bf2-keys 9eab9bd do not merge; their worktrees (.claude/worktrees/bf2, bf2keys) stay READ-ONLY as a source
of parts until the beat-nudge lane's stage S4 is gated, then they are removed. lane/keying-audit e22ef2d (worktree keying)
waits for his keying page. FIVE LANES ARE PLANNED, COUNCILLED, RULED AND ADOPTED, three of them re-stated on his later
answers. Pitfall numbers: lanes write "Pitfall NN"; Harmony assigns at each merge (next free: 68).

START HERE, in order:
1. HIS OPEN QUESTIONS (each has a default; nothing waits): 125-129 (the tempo row: boris-clarify-125-129.md), 131-134 (looks:
   boris-clarify-131-134.md), 73 held back until FM-8 is run. His page, opened for him: .harmony/.reports/s-rta-1004/
   boris-open.html. REQUESTS to him: three real tracks of about 10 minutes (RQ-0: the transport lane's FIRST measurement is
   BLOCKED without them), three Arena screenshots (Beat Repeat's row; Random's Interval / Distance; plus once on Speed and on
   Duration). "fast_saves" (9 old files) is question 134.
2. BUILD, at most 3 build lanes at once, each in its own worktree from main, df first (302 GB free):
   a. ONE-SAVE stage S1 (the show file: "version": 2, one verified writer, backups/ before any old-shape file is overwritten)
      -> my rows G-OS1, G-OS-HIS, G-OS4-0 -> MERGE 1 early: it protects his one show. Then S7 (the old look buttons go), S2,
      S3, S4a, S4b, S5, the visual gate, MERGE 2.
   b. OUTPUTS: G0 first (mine, on main, no code: M1 the canvas publish rate, M2 whether Syphon is available), then worktree A
      (S1, S2, S3) beside worktree B (S4, S6), then S5, S7. Delay 0..100 ms (his "51 default is good").
   c. BEAT NUDGE: G-N0 (mine) -> S1 -> S1r -> S2 -> S2r -> S3a -> VG-0 -> S3m -> S3r -> S4 -> S4r -> VG -> S5.
      ONE builder at a time in BPMTracker.cpp across lanes: nudge S1, S1r, THEN the transport lane's S4t.
   d. TRANSPORT: S0 -> SM-a (mine, production mode, three tracks) -> S4t -> SR -> SM-b -> S1 -> S2 -> S3 -> S3h -> MERGE 1 ->
      S4a -> S4d -> S4e -> S5a -> S5b -> S4c -> S6 -> MERGE 2. No slide exists any more: a clip out of time is CUT once on the
      next "1" (his "71 b").
   e. LOOKS: after one-save S7. S1a -> S1b -> SE -> S2 -> S3 -> the visual gate.
   Every stage: builder -> pinned sonnet reviews -> my behavioural gate; every live row and every verdict is mine; every
   visible change goes through the visual gate (capture builder, five critic seats) before he sees it.
3. NOT YET PLANNED (each needs a fact sheet, then plan-lane.js): the MESSAGES lane re-stated (BF41, BF42; ruling-notices.md of
   s-rta-1003b is stale against today's words; the quit window moved to one-save); RECORDING (three boxes Parameters / Audio /
   Video; record a clip into the nearest open cell, always also to a folder: BF52, BF58, BF62; bf1); NAMES (routines ->
   "actions" BF80; BeatLoopr -> "Beat Repeat" BF91: every on-screen and documented use, and whether "action" already means
   something on screen); the TAKE GAP (a take records neither mouse-moved effect sliders nor a look load); the manual
   (docs/manual; its first entry is the nudge lane's S5).
4. Older lanes, now mostly re-shaped by today's words — re-read each against binding-decisions.md before touching it: bf7 bars
   (clips are in BARS now, BF40 / BF61), bf45 envelopes, bf6 Timeline, ui-polish + Edit menu, deck names, bug X1, tsan-r5,
   keying (his page boris-keying.html is unanswered), T26 STEP 1-2 (CLAUDE.md <= 8 KB), the s-rta-1002b pending gates.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA re-shaped around how Boris performs: Resolume-style transport in bars, per-screen output settings, a beat
  nudge, one Save, looks per effect.
SHIPPED: nothing in his app today (no build, no merge). On paper: every answer he gave today filed word for word; 8 verified fact
  sheets; 5 lanes planned, attacked by blind critics, ruled and adopted; 3 of them re-stated on his later answers; the sync
  dial stopped the minute he replaced it; his list of everything the app saves (a page).
IN-FLIGHT: none (every started task is finished and filed).
NEXT: build. First the show-file protection (his one old show gets a backup before any Save), then output settings, the beat
  nudge, transport, looks. The transport lane's first step is a measurement on three real tracks.
BLOCKERS: three real tracks from Boris for that measurement. Nothing else.
YOU ARE HERE: 5 of the first 10 feedback items are in the app (unchanged since Oct 3); the next five lanes are fully ruled and
  none is built.

## LOOSE-ENDS LEDGER — s-rta-1004 (CURRENT)
NOT RUN / NOT MET (reported as such, never as pass):
- NOTHING was built, run or measured this session: no ctest, no app launch, no gate. Every "ruled" behaviour is a reading of
  185147b by an architect. Three rulings say so themselves: the nudge's arithmetic "rests on a python paper model"; the tempo
  row "Nothing was run", two top-bar widths ASSUMED; the looks delta's after-write hook ASSUMED.
- I adopted each ruling after reading its verdict, stage list, decisions and section 7 (questions) — NOT its facts, attack
  table, amendments or gate rows (each adoption block says so). Builders and reviewers read those in full.
- The fact sheets were each re-read by a second sonnet reader (verdicts in the work log); I did not re-derive them. One line
  on Boris's page (the video recording has no sound) was read once. Resolume facts with no source are marked NOT DOCUMENTED.
- Held back / owed: question 73 (FM-8); main's ctest count (1252 vs APP-INVENTORY's 1249) still unchecked; the s-rta-1002b
  pending gates unchanged; the sync dial's owed rows are VOID with the lane (H-17), not passed.
OPEN WITH BORIS:
- 125-129 and 131-134 unanswered (defaults stand: A is built). Three tracks and three Arena screenshots requested.
- Told to him as readings and not corrected (INFERRED consent, each in a boris-clarify file): my R1-R77 and the tempo-row
  ruling's own R74-R84 (the numbers overlap: cite those as "tempo-row R74"). One I got wrong and
  corrected to him (R41: his old deck files are NOT loadable); one I withdrew (R39).
- DONE AT HIS WORD: "prestest 2.json" and "test 1.json" moved from ~/Library/AudioDNA/Presets to the Trash (Finder rc 0; the
  Trash could not be listed from here). His one show was copied to .harmony/.reports/s-rta-1004/boris-show-backup/ (same
  sha256; NOT committed; his Library untouched).
FILED DEBT (found on main by the fact sheets, not fixed; the full list with file:line is in s-rta-1004-work.md, rows "DEBT
FOUND"): the Output-menu video recording can never report failure and has no disk check; a failed take save prints "Could not
stop the take: " with no reason; key / MIDI bindings are written nowhere automatically; Save Routine says "saved" with nothing
on disk; a plain Save of an old-shape show overwrites the original with no backup; Load Deck of a new-shape show makes an
empty deck that Save Deck would write over the show; settings.json drops other keys after one unreadable read; JUCE's
replaceWithText can report success on a full disk (INFERRED); the top-bar "/4 /2 x1 x2 x4" buttons are inert; momentary keys
and CC-relative knobs cannot be set from any screen (CLAUDE.md lists both as capabilities); a structural effect undo
overwrites later slider edits; the Link macros and user signals have no serializer; at nudge 0 in Auto a bar-quantised fire
can land a beat late (nudge ruling SF-1); a take records neither mouse-moved effect sliders nor a look load; Collect Media can
re-point a clip to a same-named file (read once). Plus the s-rta-1003b ledger's debts on main (archived), unchanged.
SESSION / SYSTEM:
- WARN fable-usage-audit LAW11-LOG-GAP: 16 architect dispatches, 0 DISPATCH_LOG rows (a foreign lane cannot write it).
- Session index: skipped (foreign-repo lane, no transport yet). T26 STEP 1-2 open.
- Worktrees kept: bf2, bf2keys (parts, read-only), keying. About 13 GB of build dirs. No new worktree was made.
DOUBTS:
- Whether Boris wants looks to UNPLUG signals (question 131's default) — the critics called it the riskiest default.
- Whether "delete them. this is a new build" (106) was meant wider than the two files I named: I kept it narrow.
- "Routines" -> "actions": "action" is already a word in the code and maybe on screen (binding actions): unchecked.
- The context of this session ran to 68 %: the last three adoptions were made at TIGHT. Re-read their adoption blocks first.

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

# >>> SESSION s-rta-1004 (2026-10-04 11:53 → ~16:20, secondary) — START HERE <<<

## THE ONE-LINE VERSION
Boris answered the 25 questions and then re-ruled the product in one sitting; the sync dial was stopped, eight fact sheets
were written and re-read, and five lanes were planned, attacked, ruled and adopted (three re-stated on his later answers);
nothing was built. Session log .harmony/sessions/2026-10-04-s-rta-1004-secondary.md; work log .harmony/s-rta-1004-work.md;
everything else .harmony/.reports/s-rta-1004/.

## WHAT BORIS RULED TODAY (short; his words are in binding-decisions.md)
- TRANSPORT mimics Resolume's panel (Timeline / BPM Sync; SMPTE and DJ modes listed greyed). Clips in BARS: 4, 8, 12, 16 ...;
  the out point moves in, a clip is never stretched (a clip shorter than 4 bars is fitted whole and plays slow). Every fire
  starts at the beginning. NO slide: a clip out of time is cut once on the next "1"; fired between two "1"s it cuts back to
  its beginning; a Resync cuts at once. Pause is the CLIP's, saved with the show. Timeline Speed to 10. Random and
  "Beat Repeat" (his name for BeatLoopr) are built. The timeline shows bars only.
- THE SYNC DIAL IS REPLACED by a Delay per output screen (0..100 ms) in an "Output Screens" window, with Opacity, Brightness,
  Contrast, Red, Green, Blue; Syphon is an output with the same settings; remembered with the screen.
- BEAT NUDGE: "nudge X ms", plus = earlier, shifts everything connected to the BPM; Tap leaves it, Resync zeroes it; saved
  with the show; key or pad. The tempo row: beat wheel, play, pause, stop (the BPM timer only), BPM number, BPM -, BPM +,
  nudge back, nudge forward, /2, *2, tap, resync.
- ONE SAVE: the show holds decks, keys and MIDI, window layout, "actions" (his name for routines). Decks are taken from the
  list of shows. A show's keys take over when it is opened; a new show takes the most recent show's. Quit asks (Return =
  Save & Quit). Collect Media and Snapshot stay commands.
- LOOKS PER EFFECT, kept by the app: values, Dry / Wet and the signals on the sliders; a changed look is a new look, save-over
  offered; a name box holding "Look X". The old Save / Load / FX Save / ten slots go. Nothing ships; he makes looks later.
- MESSAGES: only an unsaved clip recording, an unsaved show recording, a failed Save, no disk space to record, and the quit
  window. RECORDING: three boxes (Parameters, Audio, Video); a clip recorded into a cell always also goes to a folder.
- Cmd+Z never touches the layer strip; a removed layer comes back not playing.

## VERIFICATION — PROVEN, AND HOW
- Nothing was run. What was CHECKED, and by whom: each fact sheet by an independent sonnet re-read (8 of 8 SOUND or
  SOUND_WITH_CORRECTIONS; each sheet's VERIFICATION section overrides its body); each plan by blind seats whose citations the
  ruling re-derived; each workflow script by node --check and a dry run against stub agents before launch (prompt lengths of
  running lanes compared after every edit of the shared script).
- By me: boot state against the s-rta-1003b handoff (heads, cleanliness, the merge conflict); his two preset files read before
  deleting; sha256 of his show before and after my copy; the screen (below).

## NOT VERIFIED — WHAT ONLY BORIS CAN CHECK
Nothing new is in his app. When the lanes land, each ruling's section 6 lists his checks: outputs B-0..B-10 (a slider drags in
the never-key window; the delay against a phone's slow motion; the right screen after a re-plug; colours against Resolume);
the nudge's and the tempo row's (a held key repeats; the row at his window width); transport's (the cut on the "1" by eye;
bars against his own clips; Speed 10); one-save's (his old show opens and its first Save leaves a backup; the quit window);
looks' (signals re-plugged; the name box while keys are bound).

## MY OWN ERRORS THIS SESSION — recorded because no gate would surface them
1. A "recorded" stamp typed by hand (4 minutes off), caught before it was appended; stamps now come from `date` by
substitution (notebook). 2. Reading R41 told him his old deck files were loadable; they are an older format the app already
refuses (corrected to him). 3. I told him the slide took "about 15 seconds"; it was 17 to 34 (the ruling corrected it; he then
dropped the slide). 4. I told him the Delay ran to 500 ms before Resolume's 100 was known (overruled, asked as 51).
5. Question 36's option named things "the app keeps by itself" that it does not keep (the audio input, bindings). 6. One
dry-run "failure" was my own harness, not the script (re-run, logged). 7. Reading R39 (the computer's keys stay live) was
withdrawn after his next message read the other way. Habits: RIG-RULES.md A3 and the notebook.

## SCREEN STATE AT CLOSE (screen-safety law #4)
This session launched NO Audio-DNA, no probe and no gate. At close: no Audio-DNA process; Quartz count 0 Audio-DNA windows,
0 Output-named, 0 UserNotificationCenter; live + ctest locks absent; no full-screen capture was taken. Two pages were opened
in his browser in the background (boris-saves.html, boris-open.html). Finder moved two files to the Trash at his word.

## COUNTS — run them, never inherit them
ctest on main: NOT run this session (1252 by the s-rta-1003b lane's count, 1249 in APP-INVENTORY). Nothing else was counted.
