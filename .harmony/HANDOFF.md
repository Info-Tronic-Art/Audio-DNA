# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony, SECONDARY lane, in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL live audio-reactive VJ app).
CURRENT as of session s-rta-1003 (2026-10-03 13:24 → ~19:15). READ, in this order: (1) .harmony/HANDOFF.md (this prompt's
file: WHERE WE ARE, the LOOSE-ENDS LEDGER, the SCREEN-SAFETY LAW — obey it, the session section); (2) .harmony/RIG-RULES.md
BEFORE launching any lane, probe or gate (binding; every rule cost a run); (3) .harmony/.reports/s-rta-1003/board.md (the
lane board: order, state, next step). History is .harmony/HANDOFF-ARCHIVE.md — never load it whole (grep '^#', read ranges).
Boris's rulings: BORIS_DECISIONS.md + .harmony/binding-decisions.md (his words in quotes; read the 2026-10-03 sections
before touching decks, undo, notices, sync, clips, recording, envelopes); his feedback verbatim: .harmony/boris-feedback-backlog.md
(BF1-BF33). CLAUDE.md is 24,224 B of its 25,000-byte cap and is due to be cut to <= 8 KB (inbox task T26 STEP 1). Ultracode:
use workflows. Law #11: plans = architect, opus high; rulings = opus max while Fable is out (Boris 2026-10-02, verbatim:
"just so you know, we are out of fable usage so you will need to do all fable work with opus 5.5"); builders opus high.
BORIS USES THIS MACHINE AND THIS APP: an Audio-DNA you did not start is his; yield the turn while lanes run (he interrupts
long waits); file his words verbatim the turn they arrive.

STATE: the deck change (decks are boxes of clips; lane bf9b) is MERGED into main (17574d1) and gated; ctest 1252 / 1252.
Boris gave about 35 rulings in one afternoon (BF11-BF33 + answers), all filed. Sync dial: ruling adopted, nothing built
today. Fact sheets exist for every other item (.harmony/.reports/s-rta-1003/facts-*.md). Next free Pitfall: 68 (bf2).

START HERE, in order:
1. bf2 sync dial — worktree .claude/worktrees/bf2 (lane/bf2 4a1f240). Stages per .harmony/.reports/s-rta-1003/ruling-bf2-delta.md
   (22 amendments) + the HARMONY ADOPTION at the end of plan-bf2-delta.md (items 1-7: every show remembers its sync incl. 0;
   no notice text; Gain twice as long, same scale): M0 merge main INTO lane/bf2 -> S3f -> S4 -> S6 -> S5a -> S5b (visual
   gate) -> reviews -> merge (Pitfall 68). Quiet-machine rows (R7 21 arms, G6, [timing]) are Harmony's.
2. PLAN TOGETHER (architect -> blind council -> ruling): transport + undo-live — every fire restarts a video; a column
   re-fire restarts; the playhead drag plays on from the drop; Cmd+Z never changes what is live; a BPM-synced clip has a BPM
   box (starts at 120) and a beats box with x2 and /2. Facts: facts-transport.md. Re-registers K10 (ii), T6h, the trigger-undo tests.
3. Notices lane — remove every event and failure text app-wide (facts-notices.md: 23 + 21). State displays must stay
   truthful (the Record panel must not say "Recording" after a failed take); probe-asan-live's waits move to a model fact
   FIRST; Boris may still answer "keep failures" (default: they go).
4. Keying / blend pixel audit on the merged app -> a works / fix / remove table for Boris (facts-keying.md).
5. Then: bf7 bars delta (Quantize 1, 1/2, 1/4, 1/8, 1/16 needs sub-beat release edges; MilkDrop labels in bars, no seconds);
   bf45 envelopes delta (NO big view; trim handles that snap); bf1 record delta (alpha, no auto end, live cell, prominent
   REC; codec measured in-app); bf6 Timeline; ui-polish + the Edit menu (BF33); deck names (bad characters -> "-"); bug X1;
   tsan-r5 (+ SF-12 the http-thread reader).
6. T26 (inbox): STEP 1 CLAUDE.md <= 8 KB (pin-scan tests first), STEP 2 claudeMdExcludes for ~/.claude/CLAUDE.md (layer0
   supplied in-project), re-measure against .harmony/.reports/s-rta-1003/t26-step0.md, report up via idea-capture.
7. Pending gates (quiet machine): mkvidx G6 / G7, bf10 G3 perf, bf10 G5 evidence top-up; ui G1b on-screen probe only with
   Boris's OK (NOT given).
Boris page: .harmony/.reports/s-rta-1003/boris-checks.html (opened at close; his test show is in
~/Documents/AudioDNA-deck-check/). One open question to him: "keep failures" (see 3). Unpushed 0.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA app-evaluation feedback round (BF1-BF33): make the app behave the way Boris performs.
SHIPPED: deck change merged (decks = boxes of clips; switching decks never changes the picture; no deck marks, no Undo
  Remove button, no deck messages); freed-memory fault fixed + memory gates; Undo of a new deck keeps a playing clip;
  33 probe scripts can no longer quit his app; MilkDrop plays across deck switches; Layer tab survives Remove Deck.
IN-FLIGHT: none (sync dial S1a-S3 built on its branch from earlier sessions; its ruling is adopted, no builder started).
NEXT: sync dial build -> fires + Undo rules -> remove all event messages -> keying / blend audit -> bars, envelopes,
  record to clip, Edit menu.
BLOCKERS: none. One question to Boris with a default (keep failure messages or not).
YOU ARE HERE: 5 of the first 10 feedback items are in the app (codec info, deck rename, MKV reverse, MilkDrop size, decks);
  the afternoon's 23 new items are planned or fact-sheeted, none built.

## LOOSE-ENDS LEDGER — s-rta-1003 (CURRENT)
NOT RUN / NOT MET (reported as such, never as pass):
- K5 with Ableton Link ON: NOT RUN (waiver in the work log: the app is built with Link OFF). SF-6: a Link build + toggle route.
- B7 graphic-design seat answered NO: ~185 pt empty top bar where "Fade:" was. Harmony's decision: left for the sync-dial
  top-bar pass; on the Boris page as a look item.
- B6 perf was run at 4137f60, not re-run at the final head fc51063 (delta = message-thread inspector code only).
- B3b / ASAN-LIVE / H1 / U1 not re-run on merged main (src / tests / probe diff fc51063..17574d1 = 0).
- Pending from s-rta-1002b: mkvidx G6 / G7, bf10 G3 perf, bf10 G5 evidence, ui G1b (Boris has NOT OK'd the on-screen strip).
NOT BUILT (ruled, filed):
- BF31 Undo never changes what is live (T6h pins today's exception: Undo of a Load Deck that added layers stops its clips).
- Every fire restarts (bf9b's C3 "resume" + K10 (ii) still as built). Column re-fire, playhead drag, BPM box.
- BF32 notices app-wide: "Loaded: / Saved: / Loaded deck:" and 40 more still show. BF33 Edit menu (Undo is in "Composition").
- Remove Deck still empties the Clip tab (the effect scope carries a deck index). After Add / Remove Column the Clip tab goes empty.
- Composition::routineLoadNote has no reader and no log line. The drawable Timeline curve is assigned to NO lane.
- SF-12: GET /api/composition reads the deck list on the http thread unlocked (ASan container-overflow once) -> tsan-r5.
- ui-polish: a 20-character deck name is cut without "…"; a strip shows the file name, not the clip name; rebuildCells split (SF-3).
GATES / PROBES HYGIENE:
- probe-boxes-perf.sh exits 0 whatever B6(ii) says (read the B6 lines). probe-asan-live waits on three event texts.
- AS8 (Layer tab after Remove Deck) has no failing arm; its teeth are lint B4k + the live rows. m9b [H2] proven by a mutant only.
- probe-quit-ours.sh honours QUIT_OURS_UCOMM from the environment (selftest seam). probe-lane3.sh / probe-step3.sh launch without -g.
- APP-INVENTORY "242 embedded shaders" / "41 REST routes" not re-derived. performance-controls.md overclaims "Undo / Redo
  leave the Clip tab emptied".
SESSION / SYSTEM:
- T26: only STEP 0 (measurement) and STEP 3 (this slim handoff + archive + RIG-RULES.md) done; STEP 1-2 open.
- fable-usage-audit WARN LAW11-LOG-GAP: 3 architect dispatches, 0 DISPATCH_LOG rows (a foreign lane cannot write it).
- Session index: skipped (foreign-repo lane, no transport yet).
- 156 visual-gate captures are on disk in .harmony/.reports/s-rta-1003/bf9b-shots/ (gitignored); 12 + the manifest are committed.
- Worktrees: .claude/worktrees/bf2 kept (unmerged lane); bf9b and recon removed. Branch lane/bf9b kept.
DOUBTS:
- "Remove the list entirely and cleanly" includes failure messages: a failed save will show nothing. Told to Boris once; no answer.
- The Clip-tab / Layer-tab behaviour after Undo of a Remove Deck was checked live only for the Layer tab.

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

# >>> SESSION s-rta-1003 (2026-10-03 13:24 → ~19:15, secondary) — START HERE <<<

## THE ONE-LINE VERSION
The deck change is merged and gated; Boris fed back all afternoon (about 35 rulings, filed verbatim); the sync dial and
every other item are planned or fact-sheeted, none built. Session log .harmony/sessions/2026-10-03-s-rta-1003-secondary.md;
work log .harmony/s-rta-1003-work.md; everything else .harmony/.reports/s-rta-1003/ (board.md first).

## VERIFICATION — PROVEN, AND HOW (Harmony ran every gate)
Lane bf9b: main merged INTO the lane (M1), by-name-quit sweep (M2, selftest 88 / 0), MilkDrop deck-switch row (M3), FIX-1
memory, FIX-2 undo, FIX-3 screen, FIX-4 probes, FIX-5 Layer tab; reviews: 4 lenses at 4137f60 + the delta at fc51063, all
PASS_WITH_NITS, 0 MUST. Gates at the final head fc51063: build rc 0; ctest serial 1252 / 1252; TSan unit 5 / 5, 0 warnings;
"PROBE-ASAN-UNIT GREEN (10 cases, 0 reports)"; "PROBE-ASAN-LIVE GREEN (7 steps, ...)" and "RED (step L1)" on the wiring
mutant; deck probes 63 PASS / 0 FAIL / 1 BLOCKED with the exact BLOCKED-1 line, rc 3, RED on the pre-merge app in exactly
the 19 expected rows; MilkDrop m9b PASS x3 (RED-OK x3 pre-merge), 23 / 0; deck tabs 54 / 0 (51 / 3 pre-merge on the
build-count clause only); F5 live rows 10 / 0; perf at 4137f60: "B6(ii) PASS ... = -0.016 ms" (teeth +37 ms under the
mutant), B6(i) +0.034 ms; visual gate: 156 captures, critics 4 YES / 1 NO (Fade gap) + 2 YES on the fix. Post-merge on
main 17574d1: build rc 0 (355 objects), ctest 1252 / 1252, deck probes same line rc 3, TSan 5 / 5.

## NOT VERIFIED — WHAT ONLY BORIS CAN CHECK (page .harmony/.reports/s-rta-1003/boris-checks.html, opened for him)
- Clicking through 20 decks while clips play: nothing on screen changes, by feel. The top bar's empty stretch: taste.
- His own show "test with harry" opens with his layers as they were. Remove Deck / Cmd+Z by hand. Load Deck of a wider deck.

## MY OWN ERRORS THIS SESSION — recorded because no gate would surface them
1. Told Boris where the Timeline curve "will be drawn" from two grep hits, as fact; no plan says it. Corrected to him; habit
   in RIG-RULES. 2. Read his "default and bpms for clips to 120" as "my default stands"; he meant every clip's BPM starts at
   120. 3. gateA.sh printed every rc as 0 ("$(date) ... rc=$?"). 4. Trusted a restore rebuild that compiled nothing (the
   mutant app ran again as "restored"). 5. An 8-minute in-turn wait Boris had to interrupt. 6. One git log without
   --first-parent (80 lines of noise) and one cd in a subshell.

## SCREEN STATE AT CLOSE (screen-safety law #4)
Every launch was open -g through the lock helper or a probe that quits only its own pid; no gate opened an Output window;
0 Audio-DNA / Output / UserNotificationCenter windows after every batch and at close; no Audio-DNA process running; live +
ctest locks free; no full-screen capture (window-id captures only). The ASan app ended on reports without a crash dialog.

## COUNTS — run them, never inherit them
ctest 1252 / 1252 on main 17574d1. Unpushed: see git status at boot.
