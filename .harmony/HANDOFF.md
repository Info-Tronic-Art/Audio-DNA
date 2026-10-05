# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony, SECONDARY lane, in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL live audio-reactive VJ app).
NEVER cd, from the first command: R=<repo> at the head of every command, absolute paths, git -C; a return code on its own line,
never after a pipe. CURRENT as of session s-rta-1004b (2026-10-04 20:49 → 23:15). READ, in this order: (1) .harmony/HANDOFF.md
(this file: WHERE WE ARE, the LOOSE-ENDS LEDGER, the SCREEN-SAFETY LAW — obey it, the session section); (2) .harmony/RIG-RULES.md
BEFORE launching any lane, probe or gate (binding; sections A, A2, A3, A4, B); (3) .harmony/.reports/s-rta-1003/board.md (the
rows under "s-rta-1004b LIVE STATE" are the newest); (4) my rulings of this session, short and binding:
.harmony/.reports/s-rta-1004b/rulings-one-save.md, rulings-outputs.md, rulings-nudge.md; (5) the ADOPTION blocks at the END of the
plans: .harmony/.reports/s-rta-1004/plan-one-save.md, plan-outputs.md, plan-nudge.md, plan-nudge-row.md, plan-transport-delta2.md,
plan-transport-answers.md, plan-effect-looks.md, plan-looks-answers.md, and .harmony/.reports/s-rta-1004b/plan-nudge-row2.md
(ADOPTED at 50 % of the context window: re-read it first) and plan-looks-answers2.md (ADOPTION OWED). A ruling is the builders'
spec: never load one whole in the main loop. History: .harmony/HANDOFF-ARCHIVE.md — never load it whole. Boris's rulings:
.harmony/binding-decisions.md, the nine sections headed "2026-10-04 (s-rta-1004)" and the FIVE headed "2026-10-04 (s-rta-1004b)"
(his words in quotes; the text after "->" is Harmony's); his messages verbatim: .harmony/boris-feedback-backlog.md (BF92-BF125),
byte-exact copies .harmony/.reports/s-rta-1004b/boris-msg-raw-1..5.txt, his pictures s-rta-1004b/boris-images/. Ultracode: use
workflows — .harmony/.reports/s-rta-1004b/wf/: stage.js (args {lane, key, base, prevNotes}: one opus-high builder -> three
pinned sonnet reviews -> <= 1 fix round; add the stage to its STAGES table first), plan2.js (args {key}: plan -> blind seats ->
ruling; a new lane gets a first question number AND a first reading number), recon2.js (args {key}: fact sheet + re-read),
open.js (his page), dry.js + check.sh (check and dry-run EVERY script before launch). Law #11: plans = architect, opus high;
rulings = architect, opus max while Fable is out (Boris 2026-10-02, verbatim: "just so you know, we are out of fable usage so
you will need to do all fable work with opus 5.5"); builders opus high; reviewers / seats / readers sonnet. BORIS USES THIS
MACHINE AND THIS APP: an Audio-DNA you did not start is his; yield the turn while lanes run; file his words verbatim the turn
they arrive (pulled from the session record by line and type; the stamp from `date` in the command that writes it); a turn that
carried a message of his launches nothing (a 40 s timer, then launch); a question already shown is never re-worded (next free
question 172, next free reading R127). NEVER load the update-config skill.

STATE: main = fbbf125 + this close's docs commit, pushed. MERGED today: the show-file protection (lane one-save, stage S1 =
MERGE 1, gated before and after; Pitfall 68; ctest on main 1278, serial). BUILT + GATED GREEN, NOT merged, nothing of them in
the app: outputs S1 (lane/outputs-core aa7bc8e, worktree outputs-a) and nudge S1 (lane/nudge a8afcfb, worktree nudge) — both
based on 8b464a6: the next stage's step 0 takes main in. lane/one-save 93b49ab (worktree onesave) goes on. Older worktrees: bf2,
bf2keys (read-only parts until nudge S4 is gated), keying (waits for his keying page). Pitfall numbers: next free 69. Boris
RE-MODELLED today: routines become ACTIONS (on / off buttons per clip, per layer, global; global supersedes layer through one
transition slider; an "ignore actions" switch on every control), a RECORDING REVIEW SCREEN (his reference picture), Quantize is
OUT of live use, a "look" is a PRESET, the tempo row follows Resolume's bar (stop = clips off + the beat stops; pause = the beat
and BPM-synced clips hold).

START HERE, in order:
1. HIS PAGE, opened for him at the close: .harmony/.reports/s-rta-1004b/boris-open.html — questions 150, 151, 154-171 (each has
   a default) and readings R109-R126; filing record boris-clarify-150-plus.md; content boris-open-items.json. File his answers
   first. Request still open: three tracks (he: "another session ... from a dj").
2. START HERE — long task, begin at session start: the DESIGN PAGE for ACTIONS and the REVIEW SCREEN (he: "We need to really
   think about this to make sure it works very well and there's no confusion"). Inputs: BF96, BF98-BF100, BF111, BF114,
   BF116-BF120; readings R89-R108, R122-R126; his answers to 155-171; s-rta-1004b/facts-actions-today.md, facts-quantize.md,
   open-sweep-actions.md (16 undecided points, ranked), boris-images/review-screen-reference.png. A design workflow (several
   independent designs -> judges -> one HTML mock-up) THROUGH THE CRITIC PANEL (visual, UX, graphic, logic, and the
   interaction-logic critic) before he sees it; only then plan -> council -> ruling. Nothing of it is planned.
3. BUILD, at most 3 builders at once, each in its own worktree; every stage: builder -> pinned sonnet reviews -> my gate:
   a. OUTPUTS S2 (worktree outputs-a): the ruling's S2 row + rulings-outputs.md H-O1 (the clock joins S2's Owns), H-O2 (step 0:
      the mutant runner's table, M-P2 -> M-P2x), H-O4 (the tsan probe edit) and its bridge note; then S3; worktree B (S4, S6)
      from aa7bc8e. Owed before S5: the quiet re-run of the publish rate (s-rta-1004b/gate-g0/g0.sh; Arena was running).
   b. ONE-SAVE S7 (worktree onesave; step 0: take main in; fix probe-one-save's fill step), then S2, S3, S4a, S4b, S5, the
      visual gate, MERGE 2 (the full-disk row and G-OS-HIS again).
   c. QUANTIZE-OUT, a new lane ruled inside s-rta-1004b/ruling-nudge-row2.md: QO-P -> QO-0 (mine: the RED arms on main; QL1 a
      must fail on main, else STOP) -> QO-1 -> QO-2 -> VQ -> merge BEFORE nudge S2 and BEFORE transport S1. The clips' Snap
      VALUES stay in the files until he answers question 154 in words (HB-5).
   d. BEAT NUDGE: before S1r / S2 an ARCHITECT LINE on STOP-N1 and STOP-N2 (rulings-nudge.md); then S1r -> S2 (after
      quantize-out) -> my rows -> S2r -> S3a -> VG-0 -> S3m -> S3r -> S4 -> S4r -> VG -> S5. His 145 B and 146 B are named
      one-line changes (the adoption block). BPMTracker.cpp: nudge S1, S1r, then the transport lane's S4t.
   e. PRESETS (the lane called "looks"): ADOPT s-rta-1004b/ruling-looks-answers2.md first (read its section 7; decide
      HD-23..HD-31); the lane's base is main after one-save's MERGE 2.
   f. TRANSPORT: S0 may run when a slot is free; SM-a is BLOCKED until he brings the tracks; an architect delta before its S1
      (the rows that rest on Quantize; his Beat Repeat, Random and BPM Sync pictures; the step sizes).
4. NOT PLANNED: the messages lane re-stated; recording (three boxes); BeatLoopr -> "Beat Repeat" in every text; the manual.
   Older, re-read against binding-decisions.md first: bf7 bars (its Quantize menu is replaced), bf45 envelopes, bf6 Timeline,
   ui-polish + Edit menu, deck names, bug X1, tsan-r5 (+ today's finding: the race detector missed a race on stack locals),
   keying, T26 STEP 1-2, the s-rta-1002b pending gates.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA re-shaped around how Boris performs: one Save that cannot lose a show, per-screen output settings, a beat
  nudge and a Resolume-style tempo row, presets per effect, and now actions with a recording review screen.
SHIPPED: in his app today — the show-file protection: a failed Save leaves the show as it was; the first Save of an old show
  keeps the original in a "backups" folder. Built and tested, not in the app yet: the inner parts of the output Delay and of
  the nudge. On paper: every answer he gave filed word for word; the tempo row re-planned and ruled on his answers; presets
  re-planned; two fact sheets (what quantizes today; what routines and takes hold today); his page of 20 open questions.
IN-FLIGHT: none (every started task is finished and filed).
NEXT: his answers on the page; a design page for actions and the review screen (critics first, then him); then building:
  output stage 2, the old look buttons out, Quantize out, the nudge's wiring.
BLOCKERS: three real tracks from Boris for the clip-transport measurement. Nothing else.
YOU ARE HERE: 6 lanes ruled; 1 stage of one lane is in the app, 2 more stages are built and waiting; actions and the review
  screen are words and a reference picture, not yet a design.

## LOOSE-ENDS LEDGER — s-rta-1004b (CURRENT)
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

# >>> SESSION s-rta-1004b (2026-10-04 20:49 → 23:15, secondary) — START HERE <<<

## THE ONE-LINE VERSION
Three first build stages ran in parallel (show file, output history core, nudge arithmetic), each reviewed twice and gated by
me; the show-file protection was merged and pushed (MERGE 1); Boris answered every open question and re-modelled routines as
actions with a review screen; the tempo row and presets were re-planned on his answers; his page of open questions is open.
Session log .harmony/sessions/2026-10-04-s-rta-1004b-secondary.md; work log .harmony/s-rta-1004b-work.md; everything else
.harmony/.reports/s-rta-1004b/.

## WHAT BORIS RULED TODAY (his words: binding-decisions.md, the five sections "2026-10-04 (s-rta-1004b)")
- TEMPO ROW: stop = clips off + the beat stops, one press; pause = the beat and BPM-synced clips hold; Tap never starts,
  Resync starts; Resolume's layout; the nudge text between the nudge buttons; no "Bar 1..4" text; the three old buttons go.
- QUANTIZE leaves live use and returns in the review screen. PRESETS: a "look" is a "preset"; the name stays, no mark.
- ACTIONS replace routines: on / off buttons per clip, layer, global; global over layer through one transition slider; an
  "ignore actions" switch on every control; made only in the REVIEW SCREEN (his reference picture; rows, In / Out, a grid).

## VERIFICATION — PROVEN, AND HOW (all by me unless said)
- one-save S1 at 93b49ab: 1278 of 1278 serial; OS-L13 / L14 / L14b PASS with four RED arms (three mutant apps, the 185147b
  app); the full-disk row: new app keeps the show, the 185147b app empties it to 0 bytes (fresh image); K7 PASS with its RED
  arm; G-OS-HIS on two copies of his show; his folders' checksums before and after. MERGE 1: main rebuilt, 1278 serial, the
  live rows GREEN on main's app, pushed.
- nudge S1 at a8afcfb: the golden re-run on my own pristine 185147b tree, three runs equal to the recorded constants; 1276
  serial; 21 of 21 mutants RED, the stub arm RED.
- outputs S1 at aa7bc8e: 1272 serial; 20 stage cases; 22 mutants re-witnessed (21 RED, 1 equivalent and replaced); race
  detector 10 of 10. G0: publish rate and Syphon availability on main.
- By builders and reviewers, not by me: every unit case's RED-first; 18 review reports (0 MUST left); the two fact sheets.

## NOT VERIFIED — WHAT ONLY BORIS CAN CHECK
In the app now: open "test with harry", Save, and look for a "backups" folder beside it holding the original; Save again (no
second copy); re-open the saved show and see it is his show (the layers of his two decks are now one set: Deck 1's settings
win — the conversion main has done since 2026-10-03). The page lists these as do -> expect -> what wrong looks like.
Nothing else of today is visible to him.

## MY OWN ERRORS THIS SESSION — recorded because no gate would surface them
1. My first four commands were prefixed with cd (the rig rule was read after them); the shell's directory drifted. 2. A return
code read after a pipe, twice. 3. I told him output stage 2 would start "if my gate is green", then reached the off-ramp and
took it back. 4. My own gate clause for G-OS-HIS (a) demanded a key that route never carries: it printed FAIL on a met bar.
5. My first pull of his message from the session record returned my own command. 6. I told him readings R103-R108 while a
planning pass was numbering its own from R103. 7. The first full-disk RED arm failed on the row's precondition, not on the
show: read as RED it would have been a false arm. Habits: RIG-RULES.md A4 and the notebook.

## SCREEN STATE AT CLOSE (screen-safety law #4)
This session launched Audio-DNA in at least 17 runs of mine (a probe may launch more than once), always test mode with `open -g`, each quit by the run that started it; NO
Output window was opened by any path. At the close (2026-10-04 23:08:30): no Audio-DNA process; Quartz count 0 Audio-DNA windows, 0
Output-named, 0 UserNotificationCenter; live and ctest locks absent; no disk image attached. No full-screen capture was taken
(one headless browser picture of my own page). His own Audio-DNA was open for a while mid-session and was never touched; my
live rows waited until he closed it. One page was opened in his browser in the background (boris-open.html). Finder moved one
folder to the Trash at his word.

## COUNTS — run them, never inherit them
ctest on main: 1278 (serial, by me, after MERGE 1). Lanes: outputs-a 1272, nudge 1276, onesave 1278 (serial, by me).
