# HARMONY'S RULINGS — lane bf2 (the sync dial), session s-rta-1003b (binding for every bf2 stage and review)
Precedence: Boris's verbatim words > these rulings > ruling-bf2-gates-restated.md (s-rta-1003b) > the HARMONY ADOPTION at the
end of plan-bf2-delta.md > ruling-bf2-delta.md > the plan body. Written 2026-10-03 21:03:06; later rulings are appended with a stamp.

## BORIS, VERBATIM (binding-decisions.md; quote him only from there)
- "every show remembers it's sync" (2026-10-03 14:55:32)
- "We don't need any text indicating what has happened or what has happened. That is something that happens online and is
  not necessary in this application. It is extra overhead and bloat. Please remove it cleanly and completely." (15:30:49)
- "remove the list entirely and cleanly" (15:42:01; texts that were in the app before, failure reports included)
- "ok. the only fail message will be a failed save. remove all others" (2026-10-03 20:59:50, s-rta-1003b; answering whether the two
  "Save failed" boxes stay as the one exception)

## RULINGS
H-1 "Pitfall NN" in history files. Files under .harmony/.reports/**, .harmony/HANDOFF-ARCHIVE.md and .harmony/*-work.md
    keep their "Pitfall NN" text: they are dated records of other lanes' placeholders. The M0 grep gate covers live
    files only (src, tests, docs, CLAUDE.md, APP-INVENTORY.md, live .harmony scripts). Harmony's check at c45b579:
    git grep -n 'Pitfall NN' HEAD -- . ':!.harmony/.reports' ':!.harmony/HANDOFF-ARCHIVE.md' ':!.harmony/s-rta-1002b-work.md'
    prints nothing.
H-2 G7 state C11 (a refused venue name). CONFIRMED as re-stated: the refusal caption is NOT built; a refused name leaves
    the panel as it was. Boris's fourth line above settles it (a refusal caption is a fail message that is not a failed
    save). A control that is greyed while its input cannot be accepted is a STATE display and is admissible; a caption or
    any text that reports the refusal is not. The interaction-logic critic judges in G7 whether the silent refusal reads
    as a fault.
H-3 The re-statement's additions are ADMITTED, each tightens a row: R11 (h), (i), (k); the copy A / path Z; the WAITS
    and TEXTS paragraphs; the dump's "texts" and "tooltip"; the test_staged_load list entry; G7's C13k.
H-4 No app-log line for a dial changed by a composition open (the architect's round-2 choice stands: unasked, ungated).
H-5 G7's "modalComponents" bar: 0 in every state without a menu, exactly 1 in C9 (the venue menu). ADMITTED: it is
    tighter than leaving C9 out and its reason is a JUCE fact, not taste. "Exactly 1" is INFERRED: the S5b builder
    MEASURES it on the first C9 dump; if the count with only the venue menu open is not 1, STOP and report (the bar is
    not adjusted by a builder).
H-6 The one question left for Boris (a way back to a number that an opened show replaced; default: no) goes on the
    Boris page, after check 6.12 so he sees the loss before he answers. Nothing is built for it.
H-7 Lane order: the sync dial merges BEFORE the lane that removes event texts app-wide. A "Loaded: <name>" file label
    in a capture is pre-existing, outside this lane, and recorded for that lane.
H-8 [timing] under load (fact from M0, c45b579): the case "[timing] the published lag moves 1 ms per 1 ms step and 150 ms
    at 150" read 0.65 - 0.82 per pair while a video player sat above 20 % CPU and 0.93 - 1.02 on a quiet run, against a
    0.7 .. 1.3 bar. The bar is NOT touched. S3f builds D8 exactly (RUN_SERIAL, margins always printed) and adds nothing
    else to the case; WHY the slope reads LOW (not noisy) under load is a question for the architect: the S3f builder
    reports what the per-arm lines show, and does not theorize in code.

## RULINGS of 2026-10-03 22:33:03 (after stages S3f and S4, the review rounds and the final fold of the re-statement)
H-9  The re-statement's round-3 questions. (Q-H1) A failed save of the VENUE LIST shows nothing on screen by default;
     lastError stays readable through REST. It follows question 12 of the Boris page (which saves count as "a failed
     save"; default: a show or a deck only). (Q-H2) The full-list line goes on the Boris page. (Q-H3) R11's LIVE RED
     (one scratch build with the two mutants) is run once. (Q-H4) The stamp of Boris's fourth line above is mended to
     the recorded time. The round-3 additions of section 8.5 are ADMITTED (each adds a way to fail). Side finding K7
     (Load Deck accepts a whole composition file) is filed for the deck lanes, not built here.
H-10 Stage S4's two new painted targets ("Sync -1 ms", "Sync +1 ms" in the bind overlay and the MIDI-learn overlay) are
     VISIBLE and nobody has looked at them: the S5b visual gate gains two states (bind overlay open, MIDI-learn overlay
     open) with measurable bars (each target inside the overlay, no overlap with its neighbours, text fits) and goes to
     the full five-seat panel with the rest.
H-11 Stale kill advice in .harmony/gotchas.md (two 2026-07 rule lines: pkill by name, sample <pid>) is replaced on main
     (Harmony's file; the lane copy merges). docs/claude/testing-eyes.md was fixed in the lane (dc59573).
H-12 OPEN after the fix round, each goes to an architect ruling (plan -> blind seats -> ruling: ruling-bf2-stops.md):
     the keys MUST (no way to make a Relative CC binding: a knob learned onto a Sync target does nothing); R7's
     one-device-block state at dial 0 (3 of 7 zero-dial takes read 41.333 ms instead of 30.667); the late marker at raw
     index 70 in every take; [timing] reading low once under load; no test drives the real SyncNudge handler case; the
     BeatLead manual-Resync branch that D5 left dead. NO further bf2 build stage (S6 included) starts before that ruling:
     S6's D9 changes the take start, the same code the R7 state may live in, and a diagnosis must not be confounded.
     R7's 21-arm run is NOT run until its instrument is understood.

## RULINGS of 2026-10-03 23:39:40 (the stop items: ruling-bf2-stops.md, 29 attacks ruled, 20 amendments A1..A20, STATUS DONE)
H-13 ADOPTED IN FULL: .harmony/.reports/s-rta-1003b/ruling-bf2-stops.md. Stages: D0 (probe-only diagnosis instrument) ->
     RD (Harmony's diagnosis run: 6 launches x 6 takes x 12 s at dial 0 on build-lane AS BUILT at 68abc16) -> R7r (R7 / R7b
     on the measured origin; only after RD = O1 or O5) -> the owed S3f rows (Harmony, quiet machine) -> S6 -> S5a -> S5b;
     S4b (the keys fix + rows R13, R14) beside D0 / RD; D1 only on RD = O2. Its section 5 replaces / adds gate rows.
     Harmony's decisions (the ruling's section 8):
     HD1 ALTERNATIVE: if RD does not reproduce the state in 6 launches (O5), SIX MORE launches run before R7r is built.
         Reason: the saved shifted takes cluster by launch (3 of 3 in one launch), which fits a state latched per launch;
         twelve minutes more of looking is cheaper than re-stating a gate around a state nobody has seen again.
     HD2 default: S6 does not start beside the owed R7 run; R7's verdict first.
     HD3 default: the dead BeatLead branch stays, filed as debt (the handoff's ledger).
     HD4 default: the learn overlay's title for a Sync target is built; it is a prompt (a state), and goes into G7.
     HD5 default: an Absolute-CC Sync entry in a hand-made bindings file loads inert (no screen can make one).
     HD6 ALTERNATIVE: the loaded [timing] arm is run ONCE as INFO (it costs a minute and tests "CPU load triggers the low
         regime"); it is never a verdict.
     HD7 NMAX and EREF are written here by Harmony with RD's outcome, before R7's gate run.
     HD8 yes, with one change: S4b is built in its OWN WORKTREE and branch (.claude/worktrees/bf2keys, lane/bf2-keys from
         68abc16), not merely its own build directory -- two builders never share a working tree or an index. Harmony
         merges lane/bf2-keys into lane/bf2 after its gate (R13, R14, G1, keys review r3).
     HD9 default: RD stops early once 2 takes with c0 >= one block and 2 with c0 = 0 are on record.
     HD10 default: the doubled-onset detector item and the [timing] regime record go to the handoff's ledger.
     BORIS QUESTIONS Q1 (a key or a pad is enough for Sync; default yes) and Q2 (a held Sync key keeps moving; default
     yes) are on his page as questions 13 and 14.

## RULINGS of 2026-10-04 00:22:14 (the diagnosis run RD; Harmony's own run; evidence .harmony/.reports/s-rta-1003b/gate-rd/)
H-14 RD OUTCOME O1 -- INSTRUMENT ARTEFACT, SHOWN. Run on lane/bf2 740b6d6, build-lane's app as built (binary of
     2026-10-03 22:17:41, sha 689a7ffebe9c), selftest "0 case(s) differ" first; 4 launches x 6 takes x 12 s at dial 0;
     the stop rule (2 takes with c0 >= one block and 2 with c0 = 0) was met after launch 4. The verdict line, verbatim:
     "RD outcome O1 (24 valid takes in 4 launches; c0 = 0 in 22, >= 1 block in 2; e' span 64; C span 0; NMAX 1; EREF 1440)".
     Every take: 11 x (c0 0, e 1440), 9 x (c0 0, e 1472), 2 x (c0 0, e 1504), 2 x (c0 512 = one block, e 1952, e' 1440);
     C 0 in all 24; no device re-open, no gap, no hop with an applied offset. The two shifted takes sat in launches 3 and
     4, each beside five unshifted takes with C span 0 -- both states in one launch, the rulers agreeing.
     MEANING: on some takes the click file starts one device block (512 samples, 10.667 ms) into the take; the take's
     markers and its audio agree; the probe's assumed origin was wrong, not the product. No product change.
     NMAX = 1. EREF = 1440 (HD7). NOTE for R7r: a 12 s take's median reads 1440 or 1472 (24 markers), R7's 60 s arms
     read 1472 (the D0 builder's S2); the verdict printed no WARN line; R7r's EREF clause must be stated with that in
     view, never loosened after a run.
     NOT EXPLAINED, filed: the earlier saved runs showed the shifted state in 3 of 3 takes of one launch; in RD it was 1
     of 6 in two launches. Established: the shift is exactly one block and is in where the file sits in the take.
     Ruled out by RD: a marker / audio disagreement (e' one-valued, span 64) and an analysis-side block (C span 0).
     Cheapest further test, if ever needed: log the arm-to-play gap per take beside c0.
     NEXT (not started, by Boris's instruction to finish and close): R7r (R7 / R7b on the measured origin), then the
     owed quiet rows (R5 x5, G6, R7's 21 arms + R7b, the mutant RED, [timing] x3), then S6. H-12's hold on S6 ends only
     with R7's verdict (HD2).

## RULINGS of 2026-10-04 00:44:10 (the keys fix, stage S4b)
H-15 GATE S4b GREEN (Harmony's own run on lane/bf2-keys 9eab9bd, worktree bf2keys; log .harmony/.reports/s-rta-1003b/gate-s4b/):
     build up to date; G1 "100% tests passed, 0 tests failed out of 1335"; test_binding_sync_nudge 84 assertions / 11
     cases; probe-sync-selftest "0 case(s) differ"; lane app: "PASS  R13 the handler case moves the dial: 9 of 9 steps",
     "PASS  R14 MIDI learn refuses a CC on a Sync target and changes nothing else; a note attaches: 8 of 8 states", R6
     settings sha unchanged; mutant app build-mut-r13: exit 1, R13 FAIL at 8 steps (first: "FAIL  R13 step 1: targetMs
     -1, want 1"), R14 10 FAIL line(s). After the batch: no Audio-DNA pid, 0 windows, 0 UNC. Reviews keys + gates r2:
     0 MUST. The keys MUST of review r1 / r2 on lane/bf2 is CLOSED by this branch.
     NOT DONE: the merge of lane/bf2-keys into lane/bf2 -- it conflicts in .harmony/probe-sync.py (code: two both-added
     regions, keep both), so a builder does it as step 0 of the next probe stage (R7r), then SELFTEST + R13 / R14 again
     on the merged lane. NOT RUN: a build without the test server (the three routes are read as compiled out, not built).
H-16 For S5b (from S4b's stop items; each needs the architect before S5b's packet is written): G7's ML-1 bar "every
     label_w <= its w - 4" is false today on 15 existing 32-px per-layer buttons -- re-state it (the two Sync targets
     pass); the keyboard bind overlay has no test route (H-10's state needs one); the standing learn title's em dash may
     be mojibake on screen (a visible text: its own small fix + capture).

## RULINGS of 2026-10-04 12:09:11 (session s-rta-1004; Boris replaces the sync dial)
BORIS, VERBATIM (binding-decisions.md, 2026-10-04 "The sync dial is REPLACED ..."):
- "I think we should copy what resolume does for delay. each output screen can be delayed and that is set on output display
  properties. Makes it simpler." (2026-10-04 12:08:21)
- "replace our sync with this" (2026-10-04 12:09:11)
H-17 THE LANE IS SUPERSEDED. His words outrank every ruling above (this file's precedence line). From this stamp:
     (a) NO further bf2 stage is started: R7r (incl. its step 0, the merge of lane/bf2-keys into lane/bf2), S6, S5a, S5b, and
         the architect ruling H-16 asked for. None was running at this stamp (R7r's script was not yet written; nothing to stop).
     (b) Harmony's owed rows -- the quiet [timing] x3, R5 x5, G6, R7's 21 arms + R7b, R7's mutant RED, R1a, R4, R4b -- are
         VOID WITH THE LANE: never run, neither passed nor failed. They are not carried as debt against main, because nothing
         of the lane merges.
     (c) lane/bf2 (740b6d6) and lane/bf2-keys (9eab9bd) do NOT merge into main as they stand. Branches and worktrees are KEPT
         (read-only, at these commits) as a source of parts until the replacement plan (a Delay per output screen, BF44) is
         ruled; the plan's recon lists what is carried and what is dropped; then the worktrees are removed. Pitfall 68 stays
         free (the lane's text for it is not merged).
     (d) H-14 (RD = O1, the take's one-block origin is the probe's, not the product's) stands as a FINDING about main's take
         recorder and the probe; it does not depend on the dial. The filed debts of the s-rta-1003b ledger that are about
         main (the doubled onset on click 69, "CC relative mode" unreachable from any screen, Load Deck accepting a whole
         composition file, frameTimeMs reading 0.0, APP-INVENTORY's 1249 against 1252) stay open on main.
     (e) What rode in the lane and is not the dial (the music-beat wheel of S5a, the Gain 140 px of S5b, the learn title's em
         dash of H-16, docs fixes such as testing-eyes.md's kill advice, the APP-INVENTORY count) goes to the replacement
         plan's recon as "carry or drop", each with its commit.
     Reason this is recorded as a ruling and not only as his quote: a successor reading H-1..H-16 alone would resume R7r.
