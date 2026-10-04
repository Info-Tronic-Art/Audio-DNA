# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony, SECONDARY lane, in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL live audio-reactive VJ app).
CURRENT as of session s-rta-1003b (2026-10-03 20:00 → 2026-10-04 ~01:20). READ, in this order: (1) .harmony/HANDOFF.md (this
file: WHERE WE ARE, the LOOSE-ENDS LEDGER, the SCREEN-SAFETY LAW — obey it, the session section); (2) .harmony/RIG-RULES.md
BEFORE launching any lane, probe or gate (binding; sections A, A2, B); (3) .harmony/.reports/s-rta-1003/board.md (the lane
board, updated in place) and .harmony/.reports/s-rta-1003b/rulings-bf2.md (Harmony's binding rulings H-1..H-16 for the sync
dial). History: .harmony/HANDOFF-ARCHIVE.md — never load it whole. Boris's rulings: BORIS_DECISIONS.md +
.harmony/binding-decisions.md (his words in quotes; read the 2026-10-03 sections incl. "s-rta-1003b"); his feedback verbatim:
.harmony/boris-feedback-backlog.md (BF1-BF37). Ultracode: use workflows (.harmony/.reports/s-rta-1003b/wf/bf2-stage.js runs
ONE stage by args: builder -> pinned reviews -> <= 1 fix round). Law #11: plans = architect, opus high; rulings = opus max
while Fable is out (Boris 2026-10-02, verbatim: "just so you know, we are out of fable usage so you will need to do all
fable work with opus 5.5"); builders opus high; reviewers / seats sonnet. BORIS USES THIS MACHINE AND THIS APP: an Audio-DNA
you did not start is his; yield the turn while lanes run; file his words verbatim the turn they arrive; a question already
shown to him is never re-worded in place. NEVER load the update-config skill (it costs ~13 % of the context window).

STATE: nothing was merged into main this session (main = 34179a2 + this close's docs commit; ctest 1252 not re-run). Three
unmerged lanes, each in its own worktree: lane/bf2 740b6d6 (.claude/worktrees/bf2: main merged in, S3f + S4 + fix round
built, the diagnosis instrument D0; build-lane holds the app as built from 68abc16's src; build-mut-r7 kept), lane/bf2-keys
9eab9bd (.claude/worktrees/bf2keys: the keys fix S4b, gated GREEN; own build-lane + build-mut-r13), lane/keying-audit e22ef2d
(.claude/worktrees/keying: audit probe + report, source only). Next free Pitfall: 68 (bf2; already written as 68 in the lane).

START HERE, in order:
1. SYNC DIAL (lane/bf2). Spec chain: rulings-bf2.md (H-1..H-16) > ruling-bf2-stops.md (section 4 stages, section 5 gate rows)
   > ruling-bf2-gates-restated.md (rows R11, G7, G1 for S5a / S5b) > the adoption at the end of
   .harmony/.reports/s-rta-1003/plan-bf2-delta.md > ruling-bf2-delta.md. DONE: M0 (Harmony gate), S3f, S4, D0, RD = outcome
   O1 "instrument artefact" (H-14: NMAX 1, EREF 1440, read its NOTE), S4b (Harmony gate H-15).
   NEXT: stage R7r (probe-only, one builder in worktree bf2) with STEP 0 = merge lane/bf2-keys INTO lane/bf2 (a code
   conflict in .harmony/probe-sync.py, two both-added regions: keep both), then SELFTEST + R13 / R14 again on the merged
   lane, then R7 / R7b on the measured origin as ruling-bf2-stops section 4 says. THEN Harmony's owed rows on a QUIET machine
   (no agent running, no video playing; run them from a background script): [timing] x3 (+ the loaded arm once as INFO,
   HD6), R5 x5, G6, R7's 21 arms + R7b, the mutant RED on build-mut-r7. THEN S6 (only after R7's verdict) -> S5a -> S5b.
   BEFORE S5b's packet: the architect rules H-16's three items (G7's ML-1 label bar is false on 15 existing buttons; the
   keyboard bind overlay has no test route; the learn title's em dash). S5b ends at the VISUAL GATE (five critic seats), then
   reviews, the final gate list, merge to main, Pitfall 68, remove the three worktrees as they merge.
2. FIRES + UNDO lane (after the sync dial merges): plan-transport.md + ruling-transport.md (30 amendments) +
   ruling-transport-delta1.md (DA-1..DA-10, built on Boris's answers; adoptions at the end of both plan files). Stages S0,
   S1, S2, (S2b), S3, S3h, S4a, S4c, S4b (visual gate), S5. S4c's builder reads Boris's answers to questions 20 and 24 first.
3. MESSAGES lane (after 2): plan-notices.md + ruling-notices.md (24 amendments; adoption at the end of the plan). Boris:
   "ok. the only fail message will be a failed save. remove all others". Default until he confirms question 12: every save he
   presses shows the box.
4. KEYING: Boris answers boris-keying.html (keep / fix / remove per entry) -> a plan for what he keeps (rulings-keying.md;
   audit on lane/keying-audit). The worktree keeps 97 MB of raw frames until he has seen the page.
5. Then: bf7 bars delta (+ BF37: beats for clips, units stated everywhere), bf45 envelopes, bf1 record, bf6 Timeline,
   ui-polish + Edit menu, deck names, bug X1, tsan-r5.
6. T26 (inbox): STEP 1 CLAUDE.md <= 8 KB and STEP 2 claudeMdExcludes — after the sync dial merges (it adds the Pitfall 68
   line; CLAUDE.md is 24,370 B on lane/bf2). Report up via idea-capture.
7. Pending gates from s-rta-1002b (quiet machine): mkvidx G6 / G7, bf10 G3 perf, bf10 G5 top-up; ui G1b only with Boris's OK.
BORIS'S PAGES (opened for him): .harmony/.reports/s-rta-1003b/boris-questions.html — 25 questions; 1-12 answered (12 to be
confirmed: I re-lettered it under him), 13-25 open, each with a default — and boris-keying.html. File every answer verbatim.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA feedback round (BF1-BF37): make the app behave the way Boris performs. Tonight: the sync dial.
SHIPPED: nothing new in his app tonight (no merge to main). On branches: sync dial brought up to today's app; the take-timing
  test finished; nudge on a key or pad; a knob can no longer be bound dead; the 10.7 ms take mystery diagnosed (a test
  artefact, not the app). Plans ruled: fires + Undo (with his answers), messages removal. Keying audit done, page for him.
IN-FLIGHT: none (every started task is finished and filed).
NEXT: merge the keys fix into the sync-dial branch -> re-state the take-alignment gate -> the long quiet test rows -> taps
  through the dial -> saved with the show -> the on-screen SYNC button (critic panel) -> merge. Then fires + Undo, messages.
BLOCKERS: none. The quiet rows need about 30 minutes with no video playing and no agent running.
YOU ARE HERE: 5 of the first 10 feedback items are in the app; the sync dial is about half-way through its remaining
  stages on its branch; the next three lanes are planned and ruled, none built.

## LOOSE-ENDS LEDGER — s-rta-1003b (CURRENT)
NOT RUN / NOT MET (reported as such, never as pass):
- The quiet [timing] x3: NOT RUN (machine never quiet). It failed once on a builder's run under load and passed in three
  full runs of mine; cause of the low readings NOT established (H-8; ruling-bf2-stops P4).
- Every sync live row as a GATE line: R1a, R4, R4b, R5 x5, R7 21 arms, R7b, G6, the R7 mutant RED — NOT RUN by Harmony
  (builders ran short development subsets only). G1-RED evidence for S3f / S4 was read in summaries, not re-derived.
- lane/bf2-keys is NOT merged into lane/bf2. A build WITHOUT the test server (the three debug routes compiled out): NOT built.
- RD: the earlier "3 of 3 shifted takes in one launch" pattern is NOT explained (in RD: 1 of 6 in two launches). H-14.
- Keying audit: I re-captured 87 frames (identical to the audit's); I did NOT re-derive the verdict arithmetic, and did NOT
  check the page's rows against the JSON myself (the page writer's script did). Sheet captions still call Max RGB an alias.
- Transport, delta and notices rulings: read-only work; NOTHING built or run; their facts FM-1..FM-6 are unmeasured.
- main's ctest (1252) not re-run this session; B6 / B3b / ASAN-LIVE of the deck change still as the s-rta-1003 ledger said
  (archived); pending s-rta-1002b gates unchanged.
OPEN WITH BORIS:
- Question 12 ("12 b"): I rewrote the question in place after he opened the page and its letters swapped meaning. Read as:
  a take or a routine that fails to save also shows the message (= the current default). He has been asked to confirm.
- Questions 13-25 unanswered (defaults stand). Question 20's default (a clip whose Beats is not whole is NOT beat-locked) is
  HARMONY'S default, not his word; at his "default all bpms to 120" most clips start that way.
- The keying page: every Remove is a proposal; nothing is built until he answers.
FILED DEBT (found, not fixed):
- The onset detector double-fires on click 69 of the probe's click file (the "late marker"; pre-existing).
- BeatLead's manual-Resync branch, originX_, lastResyncs_, Flags::resyncs are dead since D5 (HD3: kept as debt).
- "CC relative mode" is unreachable from any screen (CLAUDE.md lists it as a capability); MIDI learn makes every CC Absolute.
- Load Deck accepts a whole composition file (its shape check looks only for "layers"). GET /api/status frameTimeMs reads 0.0.
  R5 in AUTO printed odd tracker tempi (182.83 / 131.40 / 0.00) on a 120 BPM click; no bar reads it.
- Screen / Multiply / Darken / Lighten ignore opacity and a picture's alpha; 49 of 55 blends and 41 of 55 transitions are
  duplicates; the Keying slider and Threshold / Softness do nothing (the keying page).
- main's APP-INVENTORY said 1249 unit tests where its build lists 1252 (fixed on lane/bf2).
SESSION / SYSTEM:
- WARN fable-usage-audit LAW11-LOG-GAP: 13 architect dispatches, 0 DISPATCH_LOG rows (a foreign lane cannot write it).
- Session index: skipped (foreign-repo lane, no transport yet). T26 STEP 1-2 open.
- The TEMPORARY rm guard (installed at Boris's request for the unattended night) was REMOVED at close; script, settings
  snippet and decision log are in .harmony/.reports/s-rta-1003b/ (rm-guard.py, rm-guard-settings-snippet.json, rm-guard.log).
- Worktrees kept: bf2, bf2keys, keying (all unmerged). Disk: about 13 GB of build dirs across them.
DOUBTS:
- Whether Boris read questions 13-19 at all (he wrote "answer to 12 questions").
- Whether "a failed save" in his ruling includes a failed take save — the default now says yes (ruling-notices H-1).

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

# >>> SESSION s-rta-1003b (2026-10-03 20:00 → 2026-10-04 ~01:20, secondary) — START HERE <<<

## THE ONE-LINE VERSION
The sync dial moved through its merge-in and three stages on its branch, its one mystery was diagnosed by an instrumented
run, the next three lanes are planned and ruled, and the keying menus were audited pixel by pixel; nothing merged to main.
Session log .harmony/sessions/2026-10-04-s-rta-1003b-secondary.md; work log .harmony/s-rta-1003b-work.md; everything else
.harmony/.reports/s-rta-1003b/.

## VERIFICATION — PROVEN, AND HOW (Harmony ran every line below herself; logs in .harmony/.reports/s-rta-1003b/gate-*/)
- M0 (lane/bf2 c45b579): "100% tests passed, 0 tests failed out of 1321"; tsan 11 / 11, 0 reports; lints; quit sweep
  "SELFTEST 96 ok / 0 FAIL"; smoke health 200 + sync 200.
- A1 (lane/bf2 68abc16): 1330 / 1330; tsan 12 / 12; probe-sync-selftest "0 case(s) differ"; quit selftest 57 / 0.
- RD (lane/bf2 740b6d6, app as built): "RD outcome O1 (24 valid takes in 4 launches; c0 = 0 in 22, >= 1 block in 2; e' span
  64; C span 0; NMAX 1; EREF 1440)".
- S4b (lane/bf2-keys 9eab9bd): 1335 / 1335; "PASS  R13 the handler case moves the dial: 9 of 9 steps"; "PASS  R14 MIDI learn
  refuses a CC on a Sync target and changes nothing else; a note attaches: 8 of 8 states"; on the mutant app exit 1 with 8
  R13 FAIL lines and 5 R14 FAIL lines.
- Keying sample: 87 of 87 re-captured frames identical to the audit's (max difference 0); one contact sheet looked at.
Reviews (independent, pinned): merge / gates / realtime PASS_WITH_NITS; keys FAIL on lane/bf2 (closed by S4b: keys + gates
r2 0 MUST); D0 probe r2 0 MUST; keying method r2 0 MUST; the gate re-statement refuted twice, folded (round 3).

## NOT VERIFIED — WHAT ONLY BORIS CAN CHECK
- Nothing new is in his app tonight. His two pages carry what is his: 25 questions (taste and behaviour), and the keying
  keep / fix / remove proposals with pictures.
- When the sync dial lands: the feel of the dial against a real room; a held Sync key repeating; the SYNC button and the
  longer Gain (after the critic panel).

## MY OWN ERRORS THIS SESSION — recorded because no gate would surface them
1. Two hand-typed times on the lane board (corrected from the work log). 2. A silent 70,000-character cut of council papers
made the transport ruling PARTIAL (completion round run). 3. A quote of Boris stamped with the time I wrote a file, not the
time it was recorded (an architect caught it). 4. I re-worded question 12 on his page in place after he had it open; its
letters swapped meaning and his "12 b" had to be re-asked. 5. Loading the config skill cost about 13 % of the context
window. Habits are in RIG-RULES.md A2 and the notebook.

## SCREEN STATE AT CLOSE (screen-safety law #4)
Every launch was open -g through the lock helper or a probe that quits only its own pid; no gate opened an Output window;
0 Audio-DNA / Output / UserNotificationCenter windows after every batch and at close (Quartz count); no Audio-DNA process;
live + ctest locks free; no full-screen capture.

## COUNTS — run them, never inherit them
ctest 1252 on main (NOT re-run this session); 1330 / 1330 on lane/bf2 68abc16; 1335 / 1335 on lane/bf2-keys 9eab9bd.
