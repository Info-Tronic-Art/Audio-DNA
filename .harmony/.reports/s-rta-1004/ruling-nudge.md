# RULING nudge -- architect ruling on the blind council's attacks on plan-nudge.md (lane "nudge", the beat nudge; s-rta-1004)
Architect (opus, max effort; Fable is out of usage), 2026-10-04. Harmony decides after this; it is her working document.
Read-only. Pins checked: main HEAD = 185147b and `git status --short -- src tests docs CMakeLists.txt` printed nothing
(plain-file reads below are reads of 185147b); lane/bf2 = 740b6d6 clean; lane/bf2-keys = 9eab9bd clean. Nothing was built,
no test was run, no app or probe was launched. One thing WAS run: python arithmetic in the scratchpad (a paper model of
the tracker's rules as read, V22) -- it is my reading turned into numbers, never the app's code.
Shorthands: PL: = .harmony/.reports/s-rta-1004/plan-nudge.md. M: = main at 185147b (paths under
/Users/boriskarpman/projects/RealTimeAudio). BF2: / KEYS: = the two stopped worktrees (.claude/worktrees/bf2, bf2keys).
J: = build/_deps/juce-src/modules. SP: = the session scratchpad (/private/tmp/claude-501/-Users-boriskarpman-projects-
RealTimeAudio/914155b3-0073-45f6-a19f-910d2eb60a41/scratchpad). BD: = .harmony/binding-decisions.md. BL: =
.harmony/boris-feedback-backlog.md.
Papers (verbatim, 4 seats, 31 attacks, 39421 characters): .harmony/.reports/s-rta-1004/attack-nudge-papers.md.
Labels: VERIFIED (read at the pin, or computed), INFERRED, ASSUMED. Boris is quoted only verbatim, from BD: and BL:.
Precedence: Boris's verbatim words > this ruling > the plan body. Section 3 OVERRIDES the plan.

## 0 VERDICT
The plan NEEDS REVISION; its core stands. 31 attacks ruled: 21 ACCEPT, 9 PARTIAL, 1 REJECT. 24 amendments.

THE THREE THINGS ASKED FIRST
 WHERE the offset is applied: where the snapshot is PUBLISHED. One step (class BeatShift) on the analysis thread, after
   stage 14, immediately before `publishWrite()` (M:src/analysis/AnalysisThread.cpp:348). Not inside the tracker, not at
   the readers. Why every reader then agrees: no reader takes a beat value from the tracker -- the tracker object is
   reached outside the analysis thread in exactly two places, and neither reads a beat value (V2) -- every reader reads the nine beat
   fields off the bus, and the bus has one writer. A reader cannot see the un-shifted beat because it is never published.
 What AUTO's re-alignment does to it: it cannot erase it -- the shifted beat is re-derived from the tracker's position on
   every hop, so each re-alignment carries the shift with it. But it does two things the plan got wrong or understated:
   (1) a confident onset that arrives a few hops AFTER the tracker's own phase wrap steps the tracker's position back;
   the shifted beat steps back with it inside the beat, or is HELD for those few hops when the step would take back a
   beat already shown; (2) in that same case the tracker advances its beat COUNT at the wrap and its BEAT-IN-BAR only at
   the onset (V4). The plan's rule 3 adds one whole-beat carry to both, so the shifted beat-in-bar dips for those hops, the
   bar count steps back and the downbeat fires twice -- with either sign of the nudge (BE-1: upheld, V4, V22). Amendment
   A1 replaces rule 3: the bar fields are seated at the shifted beat's OWN edges, from the tracker's bar position
   corrected by how far the tracker's count runs ahead of it.
 Which way "+" points: "+" = LATER. "off beat by +12 ms" = the app's beat lands 12 ms AFTER the detected or tapped
   beat. How he reads it at a glance: a number line -- the "-" button sits left of the text (earlier), the "+" button
   right of it (later); each button's tooltip says the direction in words; the sign is always printed except at 0. His
   words ("forward or back") do not settle which way plus points: question 61, default A = later. One constant flips
   it everywhere (A8).

WHAT CHANGES
 - Six of the seven MUSTs change the design or the gates; the seventh removes a row.
   BE-1: the bar fields (above; A1). BE-2 + ST-2 + GA-4: a hand Resync is no longer "snap, then hold": with an earlier
   nudge of half a beat or more the plan freezes the beat for one or two whole beats and that bar never shows its beat 1
   (V12; in the paper model 166 of 680 press arms, worst wait 2 s). A hand Resync becomes a RESTART: the published beat is
   the tracker's own on the very hop of the Resync, at any nudge, with no count taken back and no beat skipped (A2); and a
   held snapshot carries the previous hop's request sequence, so Pitfall 48 stays true (A3).
   GA-1 + GA-2: the two rows that were to prove the Harmony constraint could not fail. L3 and L4 are now measured against
   the tracker's own beat line and must FAIL on an app whose BeatShift is bypassed (A10); the zero-identity proof is a
   golden recorded on 185147b by ONE test file that compiles unchanged on both trees -- the stopped lane already had
   that case and it is carried (A9, V17). GA-3: every capture state has a machine route or is dropped; F11 was wrong (the
   window opens maximized, V11). SC-7: row L5 could not fail and is dropped (A11).
 - Smaller: the setter becomes a TEST-SERVER route, no production write route (A12). Right-click = 0 is dropped; the typed
   number stays, because reading R26 ("a number can be typed") was told to him (A13). Key and pad stay on reading R27,
   and the plan's quote is re-sourced: it is his answer about the old Sync dial (A16). The glide stays (the one REJECT),
   but the only snap left is the hand Resync (A6); a loaded show slides to its number (A7). The take recorder's anchor
   is written when the nudge settles or a hold ends, never once per hop (A17). Running BPM-synced clips do not move
   with the nudge today and the plan must say so (A18).
 - The record outranks the dispatch: questions 49 and 50 are ANSWERED (BD:868, BD:879-881, BD:887-897). The dispatch
   listed them open; this ruling follows the record (A21).
 - Order: golden first (on 185147b), S1, S2, Harmony's rows, the hooks + baseline capture, the top bar, key and pad,
   Harmony's rows, the visual gate, docs. Other lanes need not wait (A23).
Nothing here loosens a pre-registered bar. Two plan bars were wrong as written and are replaced BEFORE any run: L4's
"0 violations" against a label that is refreshed 15 times a second (now with a stated lag), and the +1 / -1 ms live arms
(now INFO, because the instrument's teeth equal its drift; the 1 ms proof is the unit rows).
What no run has established: everything about the amended rule's behaviour in the app. It is derived by reading and by a
paper model. The unit cases of section 5 are RED-first for exactly that reason, and A24 says what a builder does when
one of them cannot be made green without a new rule: stop, and come back.

## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on, re-read or re-computed)
The publish point and the readers
 V1  VERIFIED. The pipeline is inline in `AnalysisThread::run()` (M:src/analysis/AnalysisThread.cpp:62-348):
     `acquireWrite()` :127, stage 5 copies twelve tracker values :203-214 (trackerRequestSeq at :213), stage 14 ends
     :332, `publishWrite()` :348. There is no per-hop function a test could call (GA-2 is right; PL:547 names one).
 V2  VERIFIED. `getBpmTracker()` is used at M:src/MainComponent.cpp:5766 (applyTempoCommand: requests) and :5896 (the
     take arm: `postedRequestSeq()`), nowhere else in src; `bpmTracker_->totalBeatCount()` is read only at
     AnalysisThread.cpp:212; `resetBeatPhase` has no caller outside BPMTracker. So every beat reader reads the bus.
 V3  VERIFIED. Readers at a count edge take the bar position from the SAME snapshot: M:src/model/Autopilot.cpp:74 (the
     count delta), :85-86 (`processPendingTrigger(snapshot.beatInBar, snapshot.barCount, ...)`); M:src/model/Layer.h:
     472-479 (Bar: beatInBar == 0; 2 Bar / 4 Bar: plus barCount parity). The count-delta consumer is
     `OnsetPulse::consume` (M:src/features/OnsetPulse.h:22-28): a backward jump re-baselines and returns 0.
 V4  VERIFIED (the lines) / INFERRED (the consequence; not run). The tracker's two clocks. Position: phase advances per
     hop and every whole beat crossed is counted at the WRAP (M:src/analysis/BPMTracker.cpp:221-232); a confident beat
     re-aligns to 0, +1 only from the second half (:235-236, :253-258). Bar position: `scoreBeat()` runs on the hop a
     beat is DETECTED (:343-346; gated on detection and on not being in the predicted regime, not on confidence) and
     steps beatInBar there (:380-382); bars are counted on the rising edge of the level, on that same hop (:516-523).
     In the predicted regime (Manual, held silence) both step together at the wrap (:243-246, :260-277). So in Auto,
     when the onset trails the wrap by k hops, the tracker publishes for k hops a count that has moved and a
     beat-in-bar that has not. BE-1's worked case re-computed: 120 BPM, one hop = 0.021333 beat; D = +30 ms gives
     delta = -0.06; on the wrap hop (phase 0.02) x = -0.04, w = -1, so PL:156's rule gives beatInBar' = (b - 1) mod 4
     where the hop before gave b: the dip. With b = 0: bw = -1, barCount' steps back, downbeat' goes true, false,
     true. The seat stated it for a later nudge; it holds for an earlier one too: D = -30, the hop before the wrap
     (phase 0.99) has w = +1 and gives b + 1; the wrap hop (phase 0.011, bar position still b) has w = 0 and gives b.
 V5  VERIFIED. The plan's guards name only the two counters (PL:164-174); T-N4 is Manual only (PL:517); T-N5 asserts
     counts only and accepts "200 +/- 1" (PL:522-525). Nothing asserts beatInBar', barCount' or the level in Auto.
 V6  VERIFIED. BF2's `BeatLead` met the same lag with a hold on the bar-position fold (BF2:src/analysis/BeatLead.cpp:
     119-124, :207-210): it froze all nine fields through every late-onset window. It never dipped; it stalled.
The request sequence, the take, the recorder clock
 V7  VERIFIED. The sequence's contract names the beat fields: "is reflected in bpm, beatPhase, totalBeatCount,
     trackerState and beatInBar here" (M:src/analysis/FeatureSnapshot.h:139-143). The take starts on the first snapshot
     whose sequence reaches the awaited one (M:src/recording/RecorderHost.cpp:592-600) and reads its bar position from
     that snapshot (:622-629). The resync branch is M:src/MainComponent.cpp:5783-5788; the replay return is :5798.
     BE-2's reading is right: a held snapshot would carry the new sequence over the old beat fields.
 V8  VERIFIED. M:src/recording/RecorderClock.cpp:14 (raw = count + phase), :56-64 ("reset" only when raw < lastRaw;
     the offset absorbs it), :68-72 ("bpm"), :78-79 ("periodic", 32 beats: RecorderClock.h:77). An anchor carries a
     std::string (M:src/recording/TempoMap.h:15-21); `beatAt` is linear at the anchor's bpm (:37-39); `tAt` /
     `sampleAt` are used by M:src/recording/Program.cpp:564-565. A hold is flat raw: no arm fires (BE-4 is right). The
     plan's citations "62-70" and "73-77" (PL:245-247) are lines 56-64 and 68-72 (SC-4 is right).
What the nudge cannot move today
 V9  VERIFIED. A running BPM-synced video only gets a speed (M:src/render/Renderer.cpp:1657-1667:
     `setSpeed(clip->videoBeats / clip->beatDivision)`; beatPhase is never read); a BPM-synced image sequence gets a
     frame rate from bpm and runs freely (:1718-1735). ST-1 is right: neither reads the beat clock.
The top bar, the window, JUCE
 V10 VERIFIED. M:src/ui/TopBar.cpp:289 (15 Hz), :297-311 (the timer copies six fields and repaints the wheel area),
     :529-634 (layout; the widths of PL F11 are right), :149-153 (the BPM field's Enter handler never gives focus back).
 V11 VERIFIED. The window does NOT open at 1280 x 800: M:src/Main.cpp:53 `setResizeLimits(1280, 720, 3840, 2160)`,
     :64-70 "Open maximized to fill the screen" (`setBounds(display->userArea)`), which runs after MainComponent's
     `setSize(1280, 800)` (M:src/MainComponent.cpp:2438). PL F11 marks the 1280 opening "VERIFIED": it is wrong
     (GA-3 is right). A permanent TEST-SERVER env hook already sits two lines below (MainComponent.cpp:2439-2441,
     ADNA_INSPECT_LAYER): the precedent for A14's window-size hook.
 V12 COMPUTED (not run). The plan's hold at a hand Resync. After the snap to 0 the target is the tracker's own count
     n; F.count = floor(old position + old delta). For |old delta| < 0.5 beat, n is F.count or F.count + 1: no hold,
     no burst. For an EARLIER nudge of half a beat or more, a press in the first half of the tracker's beat at phase p
     holds floor(p + old delta) whole beats, and when the hold ends the tracker is already on its beat 2 or 3: the
     published stream never carries beatInBar 0 for that bar. -500 ms at 120 BPM: one beat (500 ms) for every
     first-half press. -300 ms at 120 BPM pressed at p = 0.45: one beat = 500 ms, MORE than |D| + one hop -- the bar
     GA-4 and ST-2 propose ("<= |D| + 1 hop") is false for the plan's own rule. For a LATER nudge of half a beat or
     more the count must rise by 2 or 3 at once and the cap pays them on consecutive hops. Paper model (V22): of 680
     press arms, 166 do not show beat 1 on the Resync hop (all at |nudge| >= half a beat), 75 never show it within two
     beats, worst wait 2005 ms, up to 3 beat edges in 4 hops.
 V13 VERIFIED. J:juce_gui_basics/buttons/juce_Button.h:296-298: `setRepeatSpeed(initialDelay, repeatDelay,
     minimumDelay = -1)`. J:juce_gui_basics/native/juce_NSViewComponentPeer_mac.mm:918-940: `redirectKeyDown` has no
     repeat filter. That macOS sends repeats for his keys at his settings is ASSUMED (it is a system setting).
 V14 VERIFIED. A bound key fires on every key press (M:src/MainComponent.cpp:4100-4110); releases are polled only for
     Momentary bindings (:4120-4144).
Keys, MIDI, the stopped branches
 V15 VERIFIED. MIDI learn on main REMOVES the old bindings of a CC before it adds the new one (M:src/ui/
     MidiLearnOverlay.cpp:243 `isController`, :266 `removeBinding`, :282 `addBinding`). A CC learned onto a nudge
     target would delete a working binding and leave a dead one: SC-3's "harmless dead binding" is wrong on main.
     The standing title is a narrow literal with an em dash (:93-96).
 V16 VERIFIED. `Binding::Action` ends at TriggerRoutine, "APPEND ONLY: saved bindings store this enum as an int"
     (M:src/binding/Binding.h:24-48).
 V17 VERIFIED. BF2:tests/test_analysis_sync_thread.cpp:33-193 is a [golden] case that drives the REAL AnalysisThread
     in lockstep through its real inputs (`waitForTimestamp` on the bus, :132-145), hashes every snapshot field by
     field (FNV-1a-64, :78-105) and compares with a constant "Recorded by running THIS test ... on the lane's base
     commit ... Never re-recorded on the branch" (:126-130). It needs no seam in `run()`. KEYS: df98f78 is 11 files,
     +458 / -32; e15d1d0 is 8 files, +301 / -2. The BF2 tracker diff adds `predictedBeatRegime()`, `frameEpoch()` and
     `appliedResyncs()` (the last returns the existing private `resyncRequestsApplied_`).
Boris's record
 V18 VERIFIED. His words on the nudge: BD:836-838 (answer 41), BD:842-843 ("not the tempo"), BD:855-857 (Tap /
     Resync), BD:858 ("46 default"), BD:859-862 ("I mean everything."), BD:868 ("49 default" -> it reads 0), BD:879-881 (his
     changed answer to 50), BD:887-897 (81, 83, 85: the keys live in the show; opening a show switches them). "Key and pad is fine, knobs can skip the sync" is his answer to question 13, "Nudging Sync from a
     controller" (BD:769-770; BL:388, BL:414), and Harmony's own note says that answer "has nothing left to act on"
     (BL:443). READINGS told to him about the nudge and not corrected (.harmony/.reports/s-rta-1004/
     boris-clarify-45-46.md:20-24): R25 two controls and the text, always shown; R26 "1 ms per press, a held key keeps
     moving, a number can be typed, -500 to +500"; R27 "Earlier / later can be put on a key or a pad (no knobs), as he
     ruled for Sync"; R28 "The number is remembered with the show". Question 11's default ("opening a show replaces
     the dial's number", BL:420) was about the dial.
Rig rules
 V19 VERIFIED. .harmony/RIG-RULES.md:53-55 (no synthetic input; UI states via REST, composition files or a TEMPORARY
     env-var hook), :59-61 (a flake verdict needs >= 5 runs per arm; "a bar whose teeth equal the drift is INFO, not a
     gate"). M:docs/claude/testing-eyes.md:22-24: a TEST-ONLY route already drives a real text editor
     (`POST /api/debug/deck_rename` with ops begin / type / enter / escape / focus_lost) and counts closes that hand
     focus back (`focus_home_count`); :28: tooltip and menu snapshots needed a TEMPORARY hook build.
 V20 VERIFIED. M:src/analysis/FeatureSnapshot.h:193-198: trackerRequestSeq at offset 328, sizeof 384, bytes 332..383
     free; `clear()` zeroes the whole struct (:158-169). That the free bytes are zero in every PUBLISHED snapshot is
     INFERRED (no whole-struct assignment was found in the pipeline); A9's tail check decides it on 185147b.
 V21 VERIFIED. H-16 and H-17 (.harmony/.reports/s-rta-1003b/rulings-bf2.md:118-121, :128-146): the lane is
     superseded; the learn title's dash, the 15 old labels and the bind-overlay route go to the replacement plan as
     "carry or drop".
The paper model
 V22 COMPUTED (SP:sim3.py, sim4.py and the last run of this session; python; the tracker's rules of V4 and M:BPMTracker
     .cpp:486-578 as I read them + amendments A1-A3). Sweeps: 720 runs of 6000 hops (60 / 120 / 174 / 200 BPM; Manual,
     onsets 1-3 hops late, onsets 1-3 hops early; random nudges -500..+500, hand and replayed Resyncs, Taps, tempo
     steps; then again with 10 % missed onsets and with extra onsets). As ruled: 0 runs break any of -- count never
     back and never +2; bar position changes only at a count edge (or at a Resync's beat); bars never back; origin <=
     bars; the level true exactly on beat 1; position = tracker position + shift on every hop that is not held; a
     held hop keeps the old sequence; not engaged = equal to the tracker. The rule's parts are each load-bearing in
     the model: without the lag term the published bar position is wrong on 16500 of 108000 hops; without edge seating
     563 of 720 runs change it mid-beat; without the relabel 8240 of 196305 hops after a replayed Resync are wrong;
     with the plan's snap-then-hold see V12; without A3 the sequence moves during a hold in 164 runs. Longest holds
     seen, by cause: the tracker's own re-alignment 5 hops; a Tap 23 hops; a replayed Resync 24 hops (0.26 beat); a
     tempo step 31 hops. A model of my reading, not a run of the app: the unit cases of section 5 are the proof.

## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it)
| id | sev | verdict | decided by | amendment |
|---|---|---|---|---|
| BE-1 | MUST | ACCEPT | V4: count steps at the wrap, beat-in-bar at the onset; PL:156-161 carries one w into both. Re-computed for both signs; the paper model shows it (V22). The seat's fix (own bar state that steps with count', re-seated from the tracker) is adopted in the form "seated at the shifted beat's edges from the tracker's bar position plus its lag". | A1, A4, A22 |
| BE-2 | MUST | ACCEPT | V7: FeatureSnapshot.h:139-143 names the beat fields the sequence vouches for; RecorderHost.cpp:592-629 starts the take on it. Both of the seat's fixes are taken: a held snapshot keeps the old sequence, AND the hand Resync no longer holds at all. Word before request: adopted. | A2, A3, A10 |
| BE-3 | SHOULD | PARTIAL | V3: Layer.h:472-479 reads beatInBar / barCount at the count edge. ACCEPTED: the Auto unit case with a Bar snap, live arms for Next Downbeat and Bar snap. NOT as worded: "the landing beat is the same for D = 0 and D = +3" cannot be a bar -- at 0 the snapshot is main's, bit for bit, and main's own bar position lags the count in that sequence (side finding SF-1); the bar is "every D other than 0 lands on the tracker's downbeat beat", and D = 0 is printed. | A1, A10 |
| BE-4 | SHOULD | ACCEPT | V8: RecorderClock.cpp:56-64 fires only on raw < lastRaw; a hold is flat. An anchor on the first not-held tick, via the held bit. | A5, A17 |
| BE-5 | SHOULD | PARTIAL | PL:148-152 is true as the seat reads it. ACCEPTED: he is told the truth (2 s for a jump of 500, B3). REJECTED: a snap above 50 ms -- it trades a slide for a frozen beat or a burst of beat edges (V12's mechanics), and a mid-glide capture -- the text does not change during a glide; a row checks text = target while applied still moves. | A6 |
| BE-6 | NIT | ACCEPT | PL:190-193 gives the size, no test reaches it. Bound and arm added. The seat's "about 0.4 s" is 0.2 s at 200 BPM (0.667 beat). | A20 |
| BE-7 | NIT | ACCEPT | V18: BD:868, BD:879-881, BD:887-897. The dispatch to this ruling also lists 49 and 50 as open; the record outranks it. | A21 |
| GA-1 | MUST | ACCEPT | PL:595-601: both sides of L3's check come off the bus under test. The seat's remedy is taken whole: the tracker's line from the bracketing 0 arms, and a mutant app with BeatShift bypassed that must FAIL L2, L3 and L4. | A10 |
| GA-2 | MUST | ACCEPT | V1: no per-hop function exists; PL:547-550 compares the lane with itself. V17: the stopped lane's [golden] case drives the real thread and pins a constant from the base commit. Carried; recorded on 185147b; Harmony re-runs it there herself. The padding claim is decided by a tail-bytes check that runs on both trees. | A9 |
| GA-3 | MUST | ACCEPT | V11 (F11 is wrong), V19 (no synthetic input; the deck_rename route is the precedent for an editor; tooltips needed a temporary build). Each state gets a route or goes. | A14 |
| GA-4 | MUST | ACCEPT | PL:532-534 and PL:700-703 state no bar; M9 survives the arm as written. Under A2 the bar is stronger than the seat asks (no hold at a hand Resync). The seat's "<= |D| + 1 hop" would have been false for the plan's rule (V12). M9's arm is pinned with a precondition that proves it can go red. | A2, A22 |
| GA-5 | SHOULD | ACCEPT | TopBar.cpp:289: the label is 67 ms stale by design; PL:602-605 allows 0. Lag stated; text-vs-model row added; clamp and 400 arm added; L4 gets a live RED arm (the bypass app, once L4 is measured against the tracker's line). | A10 |
| GA-6 | SHOULD | ACCEPT | PL:580 grades N by the run it grades; PL:543, PL:545 name two unnumbered mutants; PL:524 accepts +/- 1. | A22 |
| GA-7 | SHOULD | ACCEPT | V19: RIG-RULES.md:59-61. The +1 / -1 arms are INFO live; L10 demands FAIL on the four large arms. | A10 |
| GA-8 | SHOULD | ACCEPT | V18: the quote is his answer 13 about the Sync dial; for the nudge the rule stands on reading R27, told to him and not corrected. The plan's section 1 and NB6 must say so. No new question: a reading told to him stands until he corrects it. The refusal stays behind one function. | A16 |
| ST-1 | SHOULD | PARTIAL | V9. ACCEPTED: the plan names it, the transport lane is told its bar lock must read the bus, and Boris is told (B12). REJECTED: a live row that passes when "the plan's own text says it does not" -- a row cannot gate on a sentence. | A18 |
| ST-2 | SHOULD | ACCEPT | V12. Under A2 a hand Resync has no hold; the bar is "the Resync hop itself is beat 1". | A2 |
| ST-3 | SHOULD | PARTIAL | PL:346-350 snaps on a load. ACCEPTED: a load slides (no snap), with a bar on the slide. The question: R28 was told to him, so the seat's "he was not asked" is half right -- it is asked once more as question 63 because it is behaviour he performs with, with R28 as the default. | A7 |
| ST-4 | SHOULD | ACCEPT | Right-click = 0 was never told to him (not in R25-R28) and zeroes silently. V10: the BPM field keeps focus. V19: the rename box's focus hand-back and its counter are the pattern. | A13 |
| ST-5 | SHOULD | PARTIAL | V13: JUCE forwards OS repeats, and stops when the app loses focus -- an app-side repeat polled from `isKeyCurrentlyDown` can stick when a key-up is lost and run the number to an end. ACCEPTED: named constants, the fallback ruled now, a model test of the schedule if the fallback is built. REJECTED for the first build: the app-side repeat and acceleration (a typed number is the big jump, R26). A held pad: Harmony's default, one step. | A15 |
| ST-6 | SHOULD | ACCEPT | PL encodes the sign in five places. One constant; the pure functions take it as a parameter and the tests run both values. The text itself cannot carry a direction word: his text is "off beat by [+/- X] ms". | A8 |
| ST-7 | NIT | PARTIAL | V10. ACCEPTED: one probe key for the lit quarter and a sentence to Boris. REJECTED: a faster tick -- Pitfall 57 (M:docs/claude/pitfalls.md:123) prices every timed repaint at the whole window; an own-layer wheel is its own piece of work. | A19 |
| ST-8 | NIT | ACCEPT | PL:245-248 with PL:148-152: one anchor per hop of a glide. | A17 |
| SC-1 | SHOULD | REJECT | Without the glide a jump of the target (a loaded show, a typed number, the route) is a frozen beat of up to |change| ms or two beat edges on consecutive hops (the hold and the cap, which the seat keeps). The glide is one clamp per hop. What the seat is right about shrinks it: the first hop and a load no longer snap; the snap sequence serves the hand Resync only. | A6 |
| SC-2 | SHOULD | PARTIAL | ACCEPTED: right-click = 0 goes (ST-4). REJECTED: cutting the typed number -- reading R26 told him "a number can be typed" (V18). | A13 |
| SC-3 | SHOULD | PARTIAL | ACCEPTED: the quote's source (GA-8). REJECTED: dropping the refusal -- V15: on main a learned CC first deletes that CC's working binding. The routes stay because rows L7, L8 and captures V14-V17 need them and synthetic input is forbidden; the dash becomes ASCII inside the title function the lane adds anyway; the 15 old labels are a baseline number, not work. | A16 |
| SC-4 | SHOULD | ACCEPT | V8; the plan's line numbers corrected. The anchor follows the SETTLED value and the end of a hold. | A17 |
| SC-5 | SHOULD | PARTIAL | ACCEPTED: the baseline capture leaves the front of the lane; other lanes branch from main now and take this lane's merge as their step 0. REJECTED: docs beside S3 -- two builders never share a worktree (RIG-RULES.md:34), and the docs describe S4's final names. | A23 |
| SC-6 | NIT | ACCEPT | A held pad is not his to be asked: it is one flag with a default. | A15 |
| SC-7 | MUST | ACCEPT | PL:75-77 with PL:606-607: no analysis thread runs in test mode, so the row cannot fail. Dropped; a lint of the single call site replaces it. | A11 |
| SC-8 | SHOULD | ACCEPT | PL:419-423 against PL:672: the plan refuses OSC because nobody asked and builds a production write route nobody asked for. The rows need a handle, not a product surface. | A12 |

## 3 AMENDMENTS (numbered; each OVERRIDES the plan body where they differ)
A1  THE RULE (BE-1, BE-3). REPLACES PL:139-180 (rules 1-5). PL:181-196 ("HOW 1 ms IS KEPT", "ACROSS A BAR LINE", "WHILE
    THE TEMPO CHANGES", "MANUAL / AUTO / LINK") stand and are read with this rule. Fork (b) of PL:129-137 stands.
    Class `BeatShift` (src/analysis/BeatShift.h/.cpp): analysis thread only; plain value state; no allocation, no lock, no
    system call; no juce include. Signature:
    `BeatShift::Adopt apply(FeatureSnapshot& snap, uint32_t word, const Flags& flags) noexcept;`
    Inputs per hop, after stage 14: S = the nine beat fields as stage 5 wrote them; S.seq = snap.trackerRequestSeq; bpm =
    snap.bpm; the word (A2: target D in whole ms, and a count of zero requests); `Flags` = { predicted =
    `predictedBeatRegime()`, locked = `downbeatLocked()`, phraseBars, resyncs = `appliedResyncs()` } read from the
    tracker for this hop (A4). barsAdvance = predicted or locked.
    State: Da (double, ms, the applied value); engaged; F (the nine fields and the sequence PUBLISHED on the previous
    hop); anchor (signed 64-bit beat index); the previous hop's S.beatInBar and resyncs; the last zero-request count
    consumed; relabelPending with its beat index; "a first hop was seen".
    Step 0, EVERY hop, engaged or not (reads only). If this is the first hop, or S.beatInBar differs from the previous
      hop's, or the tracker applied a Resync on this hop: anchor = S.totalBeatCount + (1 when S.beatPhase >= 0.5, else
      0) -- the beat the tracker's bar position names. (The tracker moves its bar position on the hop it scores an
      onset, V4: that is the beat already counted when the onset trails the wrap, and the beat about to be counted
      when it leads it.) lag = 0 when predicted; otherwise S.totalBeatCount - anchor, clamped to -1 .. +1.
    Step 1, IDENTITY. Not engaged and D == 0 and Da == 0: return. NO byte of the snapshot is written. F = S; the zero
      request is consumed; relabelPending is cleared.
    Step 2, RESTART (A2): engaged, a zero request pending, and the tracker applied a Resync on this hop. A pending
      zero request is consumed by the first hop on which the tracker applied a Resync, whether that hop restarts or
      not (so a later replayed Resync is never taken for a hand one).
    Step 3, GLIDE. Da moves toward D by at most 2.6667 ms per hop (0.25 ms per ms; a constant per processed hop, no wall
      clock) and lands exactly on D. Nothing else snaps: not the first hop, not a loaded show (A6, A7).
    Step 4, POSITION -- exact; this is what keeps the 1 ms whatever the hop. bpm not positive or not finite: the
      target is S, copied whole. Else delta = `shiftBeats(Da, bpm, plusMeansLater)` = -Da x bpm / 60000 beats when plus
      means later (A8). x = S.beatPhase + delta in double; w = floor(x); phase' = float(x - w), and a phase' that
      rounds to 1.0f is the next beat (phase' = 0, w + 1). count' = S.totalBeatCount + w in signed 64-bit.
    Step 5, GUARDS on the count, against F. HOLD when count' < F.totalBeatCount (or count' < 0): the snapshot gets
      F's nine fields AND F's sequence (A3), the held bit is set, the hop is done. CAP when count' > F.totalBeatCount
      + 1: count' = F.totalBeatCount + 1 and phase' = 0; the rest is paid one beat per hop.
    Step 6, BAR FIELDS -- seated at the shifted beat's OWN edges. They are RE-SEATED on a hop where count' >
      F.totalBeatCount (an edge); on the first engaged hop; and on the first not-held hop at or past a tracker Resync's
      beat (relabelPending is set, with resyncIndex = S.totalBeatCount, by any Resync the tracker applied that was not
      a restart -- set before step 5, so a hop that then holds sets it too; it is due when count' >= resyncIndex and is
      cleared by the re-seat). On EVERY OTHER hop beatInBar',
      downbeatDetected', barCount' and totalBarCount' are F's: they never change between edges.
      Re-seating: m = (count' - S.totalBeatCount) + lag. beatInBar' = (S.beatInBar + m) mod 4, always 0..3. bw =
      floor((S.beatInBar + m) / 4). When barsAdvance: barCount' = max(0, S.barCount + bw); totalBarCount' =
      max(F.totalBarCount, S.totalBarCount + bw); downbeatDetected' = S.downbeatDetected when m == 0, else (beatInBar'
      == 0). When not barsAdvance (Auto before the downbeat lock: the tracker counts no bars): both bar counts are S's
      (totalBarCount' still never below F's) and downbeatDetected' = S's when m == 0, else false.
      Always: resyncBarOrigin' = min(S.resyncBarOrigin, totalBarCount'); barPhase' and phrasePhase' are computed from
      the PUBLISHED beatInBar', barCount' and phase' with the tracker's two float formulas and clamps (M:BPMTracker.cpp:
      495-501, :550-557). When delta == 0 and count', beatInBar', the level and both bar counts all equal S's, the
      target is S copied whole -- nothing is recomputed, so the return to identity cannot hang on a float bit.
    Step 7, ENGAGED after the hop = not (D == 0 and Da == 0 and not held and the nine published fields equal S bit for
      bit). So after a return to 0 in Auto the step disengages on the first hop the tracker's own bar position has
      caught up with its count -- within one beat.
    WHAT IT GUARANTEES (each is a unit assertion in section 5): position' = position + delta on every hop that is not
    held or capped; count' never falls and rises by at most 1 per hop; beatInBar', the level and the bar counts change
    only at a count' edge, at a restart, or on the first hop at a Resync's beat; the level is true exactly while
    beatInBar' is 0 (when the tracker's own level follows its own beat-in-bar); totalBarCount' never falls;
    resyncBarOrigin' never exceeds it; a never-engaged session publishes the tracker's bytes untouched.
    WHAT IT DOES NOT PROMISE: when the tracker itself re-labels its bar (an onset it missed, a second onset inside one
    beat, a downbeat relock every 16 beats, an automatic phrase reset), the shifted beat follows at its NEXT edge: a
    bar position may repeat once, as the tracker's own does today; it never changes inside a beat.
    Carried from `BeatLead` (BF2: 5a46c07, 4ab00bd), re-typed: the nine-field `Beat`, `beatOf`, `writeBeat`,
    `kBeatFields` and their static_asserts, `nonBeatCrc` (now also zeroing trackerRequestSeq, A3), the float recompute
    of barPhase / phrasePhase, the hold. NOT carried: `frameEpoch_` and its seven sites, the excess counters, the fold,
    the lead slew in beats, `Diag`.
    Mutants: M1-M4, M9-M11 as PL names them, M21 (the lag term removed), M27 (no relabel after a tracker Resync), M28
    (the bar fields re-seated on every hop).
A2  A HAND RESYNC IS A RESTART (BE-2, ST-2, GA-4). REPLACES PL:325-330 and the "snap" of PL:149-150 and R6 (PL:700-703).
    Message thread, in `applyTempoCommand`'s "resync" branch, only when origin is not Replay, and BEFORE
    `tracker->requestResync()` (M:MainComponent.cpp:5787): composition_.beatNudgeMs = kNudgeAfterResync (0), then
    `void AnalysisThread::requestBeatNudgeZero() noexcept;` -- ONE compare-exchange on the word (ms = 0, zero-request
    count + 1), stored with release order. BeatShift loads the word with acquire order at its own step, which is after
    the tracker latched its request sequence and applied the Resync: a hop that applied the Resync has the request.
    A hop that falls between the two calls sees target 0 and no Resync: it glides one hop (<= 2.67 ms) and restarts on
    the next. The word: one `std::atomic<uint32_t>` on AnalysisThread, low 16 bits the signed ms, high 16 bits the
    zero-request count; `void AnalysisThread::setBeatNudgeTarget(int ms) noexcept;` replaces the ms and keeps the count
    (a compare-exchange loop, as `BPMTracker::postTempoRequest` does). No new mutex.
    Analysis thread (step 2): Da = 0. The snapshot gets S's own fields (beatPhase 0, beatInBar 0, the level true,
    barPhase 0, barCount 0, phrasePhase 0) with: totalBeatCount' = S.totalBeatCount clamped into [F.totalBeatCount,
    F.totalBeatCount + 1]; totalBarCount' = max(F.totalBarCount, S.totalBarCount); resyncBarOrigin' = totalBarCount';
    the sequence is S's; not held. When either counter differs from S's, `apply` returns them and AnalysisThread calls
    `void BPMTracker::adoptCounters(uint32_t totalBeats, uint32_t totalBars) noexcept;` before the publish
    (totalBeatCount_, totalBarCount_ and resyncBarOrigin_ take those values; nothing else changes; analysis thread
    only; this is its only caller) and BeatShift's anchor becomes the adopted count. From the next hop the published
    snapshot is the tracker's own, byte for byte, and the step is not engaged.
    When it bites. The BEAT counter is adopted only when the old nudge was half a beat or more (V12); below that the
    tracker's count is already F's or F's + 1. The BAR counter is adopted when the shifted view had already shown a
    bar line the tracker had not reached: an EARLIER nudge of any size, and a press inside the nudge's width before
    the tracker's bar line. (When the shifted view was a bar BEHIND -- a later nudge -- the published bar count takes
    the tracker's at the press and nothing is adopted: the bar the tracker had already counted shows at the press.)
    A REPLAYED Resync (a take or a routine) carries no zero request: the nudge stays, the tracker resyncs, and steps
    3-6 apply (a hold of at most the step back, then the relabel of step 6).
    If Boris ever wants the number to stay after Resync, ONE line changes: the two message-thread statements above are
    deleted and the Resync is then handled as a replayed one. kNudgeAfterResync stays a named constant.
    Mutants: M23 (the restart removed: Da snaps to 0, then the ordinary guards), M26 (app: the two message-thread
    statements removed).
A3  A HELD SNAPSHOT KEEPS THE OLD REQUEST SEQUENCE (BE-2). ADDS to PL:164-168. During a hold the snapshot's
    trackerRequestSeq is F's. A snapshot then never claims a request its beat fields do not show (Pitfall 48;
    M:FeatureSnapshot.h:139-143), and the take arm waits the length of the hold -- which is bounded (A20). BeatShift
    therefore writes ten fields while it holds, and `nonBeatCrc` zeroes ten. bpm and trackerState are NOT held: they
    are not beat fields and the sequence's promise about them is only delayed, never broken. Mutant M22.
A4  THE TRACKER: TWO READ-ONLY VIEWS, ONE WRITE (BE-1, BE-2). REPLACES PL:217 ("`predictedBeatRegime()` only").
    src/analysis/BPMTracker.h: `bool predictedBeatRegime() const` and `uint32_t appliedResyncs() const` (returns the
    existing `resyncRequestsApplied_`, BPMTracker.h:289) -- both as the stopped lane typed them (V17), no behaviour
    change. src/analysis/BPMTracker.h/.cpp: `adoptCounters` (A2). Nothing else in the tracker changes; `frameEpoch_`
    is not carried.
A5  TWO SNAPSHOT FIELDS (BE-4, GA-1). REPLACES PL:177-179 and PL:218-219. `float beatNudgeAppliedMs` at offset 332 and
    `uint8_t beatShiftState` at offset 336 (bit 0 = engaged, bit 1 = this hop is held); static_asserts for both
    offsets; sizeof stays 384. AnalysisThread writes both on every hop, after `apply` (0.0f and 0 at rest: the same
    zero bits the free tail holds today -- A9's tail check proves that on 185147b). They are sync tokens, like
    trackerRequestSeq: no UI readout. The un-shifted beat is NOT published anywhere, in any build.
A6  THE GLIDE STAYS; ONE SNAP IS LEFT (SC-1 rejected; BE-5). Rule 1 of PL:148-152 stands as step 3 of A1, with two
    changes: nothing snaps on the first hop, and nothing snaps on a load or New (A7). The only instant change is A2's
    restart. The text shows the TARGET the moment it changes (PL:269-270 stands). What Boris is told (B3 of section 6,
    and the manual): a typed jump slides at a quarter of real time -- 100 ms takes 0.4 s, 500 ms takes 2 s -- and the
    beat runs a quarter slower or faster while it slides. PL:636's "about half a second" is struck. Mutants M5, M7.
A7  A LOADED SHOW SLIDES TO ITS NUMBER (ST-3). REPLACES the `snap = true` of PL:346-349. After the model swap (Open) and
    in the New path the hook calls `void MainComponent::setBeatNudgeMs(int ms);` with the model's value: the target
    changes, the applied value glides. No picture jump. Whether an opened show's number replaces the one set in the
    room at all is question 63 (default A: it does, as reading R28 told him). If he answers B, ONE line changes: the
    hook is deleted, the number moves out of the show (to the app's settings), and "beatNudgeMs" in a show file is
    read and ignored.
A8  ONE SIGN (ST-6). src/model/BeatNudge.h (new; no juce include): `inline constexpr bool
    kNudgePlusMeansLater = true;`, kNudgeMinMs / kNudgeMaxMs, kNudgeAfterResync, `int clampNudge(int ms)`,
    `double shiftBeats(double appliedMs, double bpm, bool plusMeansLater)`, `int nudgeAfterTempoCommand(...)` as
    PL:335. The constant is read in exactly three places: (1) BeatShift's one call of `shiftBeats`; (2) the two button
    tooltips; (3) the two overlay labels -- (2) and (3) decide which of "-" / "+" says "earlier". Nothing else. `offBeatText` and the binding step do
    not depend on it ("+" always raises the number). T-N1 and T-U5 run both values. If he answers 61 B, that
    constant is the one line.
A9  THE GOLDEN: ONE TEST FILE, RECORDED ON 185147b (GA-2). REPLACES T-N10 (PL:546-550) and the last sentence of rule 5.
    tests/test_analysis_nudge_golden.cpp, carried from BF2:tests/test_analysis_sync_thread.cpp:33-193 (its [golden]
    case: the signal, the FNV-1a-64 field-by-field hash, `waitForTimestamp`, the lockstep loop), re-typed. It uses only
    what main has (`AnalysisThread(ring, nullptr)`, `getFeatureBus().createWriter()`, `setFeatureBusWriter`,
    `getBpmTracker()`: M:AnalysisThread.h:55-77, M:src/features/FeatureBus.h:97) and it never names a field this lane
    adds -- so the SAME file compiles and runs on 185147b and on the lane. Two cases, 1,968 hops each:
    [golden-auto] no command (the tracker locks by itself: Auto); [golden-commands] the same signal with requests posted
    between lockstep hops: hop 300 `setManualMode(true)` + `followExternalTempo(120)`, hop 700 `requestResync()`, hop
    1000 `setManualBPM(126)`, hop 1400 `setManualMode(false)`. Each case checks (1) the hash of every field that exists
    at 185147b, in declaration order, equals its constant, and (2) bytes 332..383 of every published snapshot are zero.
    On main (2) is the free tail; on the lane it is the two new fields at rest plus the rest of the tail -- which is
    what "bit-identical at 0" means for the tail. (3) a RAW hash of all 384 bytes of every published snapshot equals
    its own constant. If on 185147b the raw hash differs between the three recording runs while the field hash does
    not, check (3) is dropped and that is reported (R10): fields and tail are then the proof.
    How the constants are made: the builder's step 0, in a scratch worktree at 185147b with ONLY this file and its
    two CMake lines added: three runs, the three printed hashes of each case must be equal (if they are not, or if
    check (2) fails on main, STOP and report: the snapshot is not reproducible there and the gate must be re-ruled).
    The file is then committed on the lane with those constants and is never re-recorded on the lane.
    Who proves it: Harmony, row G-N0 -- her own pristine worktree of 185147b, the file copied in, the run.
A10 LIVE ROWS MEASURED AGAINST THE TRACKER'S OWN LINE; MUTANT APPS (GA-1, BE-3, GA-5, GA-7). REPLACES rows L1-L4, L9, L10
    of PL:583-624; the rows are written out in section 5. The principle: a row about "the shifted beat" must compare
    with something the shift cannot move. In Manual at a fixed tempo the tracker's line (beat position against the
    probe's clock) is measured in a nudge-0 arm before and after the nudged arm; the nudged arm's readings are placed
    on that line. Four scratch apps, each a normal cmake build in its own directory (RIG-RULES: never a copied or
    re-signed bundle; after each, the restore rebuild compiles >= 1 object and the binary's sha differs):
    build-mut-sign (M1); build-mut-bypass (M24: AnalysisThread passes a word of 0 to `apply`, whatever was set);
    build-mut-resync (M26 + M15: no zero request; "beatNudgeMs" not written to the show file -- disjoint rows L9, L6);
    build-mut-keys (M16 + M25 + M6: `bindingIsLive` always true; the binding step's sign ignored; the analysis step
    reads the target as D + 1 -- disjoint rows L8, L7, L1).
    The +1 / -1 ms arms of L2 are INFO (RIG-RULES.md:59-61); the 1 ms proof is T-N2, T-N3, T-N16. A FAIL is a FAIL: a
    "flake" needs >= 5 runs per arm before anyone says the word.
A11 ROW L5 IS DROPPED (SC-7). PL:606-607 and the "analysisRunning" key go. The rule it stood for stays as Pitfall 68's
    last sentence and as a lint (T-N14): `BeatShift::apply(` and `adoptCounters(` each occur exactly once in src
    outside their own class's files, both in AnalysisThread.cpp.
A12 THE SETTER IS A TEST-SERVER ROUTE (SC-8). REPLACES PL:419-423. `GET /api/debug/beat_nudge` -> {"ok", "ms",
    "minMs": -500, "maxMs": 500}; `POST /api/debug/beat_nudge` {"ms": N} sets through `setBeatNudgeMs` on the message
    thread and answers the GET body; a body without a whole-number "ms" is 400 and changes nothing. Compiled out
    without AUDIODNA_TEST_SERVER. No "delta" form (L7 steps through `binding_action`). Production gains only two
    read-only keys in `GET /api/features`, from the same coherent snapshot: "beatNudgeAppliedMs", "beatShiftState".
    A probe that needs "the snapshot shows my set" waits for features.beatNudgeAppliedMs == ms.
A13 THE TOP BAR: NO RIGHT-CLICK, A TYPED NUMBER, AND THE KEYS COME BACK (ST-4, SC-2). REPLACES PL:274-279.
    Right-click on the text does nothing. Tooltip on the text: "How far the beat is moved from the detected or tapped
    beat. Click to type a number." The typed number stays (R26). Enter commits (PL:277-278's parse stands); Esc and a
    loss of focus cancel (a `juce::Label` commits on a loss of focus unless `setEditable`'s third argument,
    lossOfFocusDiscardsChanges, is true: J:juce_gui_basics/widgets/juce_Label.h:244-246; the test route below drives
    `showEditor` / `hideEditor`, :263, :273); ALL THREE end the edit and hand keyboard focus back to MainComponent, by the call the deck
    tab's rename box makes when it closes (the builder names the line; `focus_home_count` is its existing witness,
    M:docs/claude/testing-eyes.md:22). TopBar exposes `std::function<void()> onNudgeEditEnded`; MainComponent assigns
    the focus hand-back to it. TEST-SERVER: `POST /api/debug/nudge_edit` {"op": "begin" | "type" | "enter" | "escape" |
    "focus_lost", "text"} -- the same shape and the same real code paths as `deck_rename`; `GET /api/debug/ui_text`
    gains "nudge": {"text", "tooltip_text", "tooltip_minus", "tooltip_plus", "editor_open", "focus_home_count", "x",
    "y", "w", "h", "label_w"} and "wheel_beat" (A19). No slider is added by this lane; no popup menu.
A14 THE VISUAL GATE'S STATES EACH HAVE A ROUTE (GA-3). REPLACES PL:303-312; PL F11's window sentence is struck (V11).
    Hooks (all TEST-SERVER, permanent, compiled out otherwise): env ADNA_WINDOW_SIZE=<W>x<H> read in src/Main.cpp --
    the window opens at that size instead of maximized (W >= 1280, the window's own minimum); the routes of A12 and
    A13; `bind_overlay` and `midi_learn` (A16); and a route for the top bar's Manual toggle ONLY if none exists (the
    builder looks first and names what he found). States: V1 "off beat by 0 ms" (Auto, LOCKED); V2 "+12"; V3 "-7"; V4
    "+500"; V5 "-500"; V6 the editor open, number selected (nudge_edit begin); V7 Manual on with "-500" at 1280; V8 no
    tempo ("---") with "+12"; V9 the default build's disabled Link beside it; V10 the window at 1280 x 800 and V11 the
    DEFAULT launch -- which on his machine is the size he runs -- both with Manual on; V14 the keyboard bind overlay
    with the two targets; V15 the MIDI-learn overlay with them; V16 a nudge target selected; V17 the standing learn
    title. DROPPED as captures: V12 (the three tooltips: read as TEXT from ui_text, exact strings; a tooltip window
    cannot be raised without a pointer) and V13 (a pressed button: JUCE's standard look, nothing of this lane).
    BASELINE, taken before the top bar is touched (stage VG-0): V18 today's top bar at 1280 and at the default size,
    Manual off and on; V19 today's learn overlay. The manifest carries every top-bar widget's bounds: it settles
    whether the bar overflows at 1280 (PL F11's arithmetic) and what width he runs.
A15 HELD BUTTON, HELD KEY, PAD (ST-5, SC-6). REPLACES PL:271-273, PL:382-384 and question 63 of the plan.
    On-screen buttons: `setRepeatSpeed(kNudgeRepeatDelayMs = 400, kNudgeRepeatIntervalMs = 50)`, no acceleration
    (named constants in TopBarModel.h; one line each to change). A held KEY moves by the Mac's own key repeat: JUCE
    forwards repeats (V13) and macOS stops sending them when the app loses focus (INFERRED: key events go to the key
    window), so the number cannot run away on a lost key-up. Its speed is the Mac's setting, not ours: nothing in the plan or the manual may state a number for it. A
    PAD: one hit = 1 ms, a held pad does nothing more (Harmony's default H-2; one flag).
    The fallback is ruled NOW, so a fix round needs no new ruling: if Boris's check B8 shows a held key moves once,
    the key repeat moves to the app's own tick -- a pure schedule in src/model/BeatNudge.h (first repeat after 400 ms,
    then every 50 ms, a count of steps due per tick), started by the first press, ignoring OS repeats while held,
    stopped when `KeyPress::isKeyCurrentlyDown` is false OR the app is not the foreground process, with a model test of
    the schedule and of the foreground stop.
A16 KEY AND PAD: WHERE THE RULE COMES FROM, AND WHY THE REFUSAL STAYS (GA-8, SC-3). In PL:20 and PL:359 the quote is
    re-sourced: it is his answer to question 13 about the Sync dial; for the nudge the rule is reading R27, told to him
    and not corrected (V18). Section 1 of the plan lists it under readings, not under his words on the nudge.
    NB6 stands as written, with: (a) the refusal stays because main's learn path deletes before it adds (V15); it sits
    behind ONE predicate, `bool bindingIsLive(const Binding&)`, so "let knobs through" is one line; (b) the standing
    learn title is written in ASCII inside `titleText()`, which the lane adds anyway -- the dash is not a separate
    piece of work; (c) H-16's label bar stands as PL:389-392 restates it (the count of old failing labels is printed
    by the first run and pinned; it was 15 in H-16's count, not re-read here). Mutants M16, M17, M25.
A17 THE TAKE RECORDER'S ANCHOR (BE-4, ST-8, SC-4). REPLACES PL:245-248 and T-R1. `RecorderClock::tick` writes one
    anchor, why = "nudge", (a) on the first tick whose snapshot is not held after one or more that were (bit 1 of
    beatShiftState), and (b) when beatNudgeAppliedMs differs from the value at the last "nudge" anchor AND has been
    unchanged for kNudgeSettleTicks = 12 consecutive ticks (0.1 s at 120 Hz). Never while unmetered; at most one
    anchor per tick. A glide or a held key therefore writes one anchor when it stops, not one per hop. With the nudge
    at 0 and never moved the two inputs are 0 on every tick and the anchor list is exactly today's. The line numbers
    the plan cites are 56-64 and 68-72. Mutant M18.
A18 WHAT DOES NOT MOVE WITH THE NUDGE TODAY (ST-1). ADDS to NB2, after PL:241: a BPM-synced clip that is ALREADY
    RUNNING takes only a speed from the tempo (V9); it has no beat phase, so the nudge cannot move it. A clip FIRED
    quantised starts on the nudged beat. Moving a running loop onto the beat is the clip-transport lane's bar lock
    (his answer "9 b"), and that lock must read the published bus. Proposed for Harmony to hand the transport lane as
    its constraint (H-10): the bar lock reads FeatureSnapshot, never BPMTracker. Boris is told in plain words (B12).
A19 THE WHEEL (ST-7). ADDS to PL:294-299: `GET /api/debug/ui_text` gains "wheel_beat" (0..3, the quarter the wheel last
    lit); row L4 checks it against the published beatInBar with the label lag. B10 tells him the circle is a coarse
    sign and the number is the instrument. The timer stays at 15 Hz (Pitfall 57).
A20 HOW LONG A HOLD CAN LAST (BE-6). ADDS to PL:164-168. A hold lasts until the shifted position has made up the step
    back, and no longer: the tracker's own re-alignment and a Tap, at most half a beat; a tempo step, at most |D| x
    |change in BPM| / 60000 beats; both together add. The tracker's position advances on every hop once it has a
    tempo, so no hold is unbounded. T-N7 gains the arm 120 -> 200 at D = +500 with a stated bar (section 5).
A21 49 AND 50 ARE SETTLED (BE-7). PL:666-667 is struck. 49: "49 default" -- after a Resync the number reads 0; A2
    builds it; the plan's "default A" wording goes. 50 and its follow-ups (81, 83, 85): key and MIDI settings live in
    the show and on the computer, and opening a show switches them. For this lane that means one thing: the two nudge
    targets are ordinary bindings and travel wherever the saves lane puts bindings; `Action::BeatNudge` appended last
    and "targetNudgeStepMs" in toVar / fromVar (absent -> +1) keep every saved copy loadable (T-B5, M17).
A22 COUNTS, MUTANT NUMBERS, EXACT BARS (GA-6, GA-4). G-N1's N = N_main + N_new: N_main is measured by Harmony on
    185147b before S1 (row G-N0); N_new is the number of new ctest entries each stage report declares, re-counted by
    her with `ctest -N`. Mutants are numbered M1-M28 in section 5; PL's two unnumbered ones are M19 (count' in
    uint32) and M20 (a std::vector member). T-N5's "+/- 1" is replaced by per-hop steps of 0 or 1 and an exact
    position identity. M9's arm is pinned and carries a precondition that proves it can go red (T-N8b).
A23 ORDER (SC-5). REPLACES PL:490. S0 as a first stage is struck; the baseline capture becomes VG-0, taken on the
    lane just before the top bar is touched (S1 and S2 do not touch it). Other lanes do not wait for this lane: they
    branch from main now, and whichever merges second takes main in as its builder's step 0. What this lane claims
    for itself: snapshot offsets 332 and 336; `Binding::Action`'s next slot (a lane that merges later appends after
    BeatNudge); Pitfall number 68.
A24 THE STOP RULE (from section 10's first risk). If a unit case of section 5 that states a guarantee of A1-A3 cannot
    be made green without a rule that is not written in A1-A3, the builder STOPS and reports the hop sequence that
    fails; he does not add a rule. The same holds for G-N0: a golden that will not reproduce on 185147b is reported,
    not masked.

## 4 FINAL STAGES + ORDER (one builder context per stage; Harmony runs every live row, never a builder)
One worktree + branch for the lane (lane/nudge from main 185147b or its successor); no two builders in it at once. A
builder builds, runs unit tests, writes probes and their self-tests; he never runs a live row and never gives a gate
verdict. REPLACES PL:436-493.
| key | who | scope and the files it owns | needs | exit, and what it proves |
|---|---|---|---|---|
| G-N0 | Harmony | (a) `ctest -N` on a build of 185147b: N_main, written into the lane notes. (b) After S1's step 0: her OWN pristine worktree of 185147b + the one golden file and its two CMake lines; three runs. | nothing; (b) needs S1 step 0 | Row G-N0. Proves the constants are main's, and that the free tail of a published snapshot is zero on main. |
| S1 | builder | THE ARITHMETIC. Step 0 (A9): tests/test_analysis_nudge_golden.cpp recorded in a scratch worktree at 185147b, committed first. Then: src/analysis/BeatShift.h/.cpp (new), src/model/BeatNudge.h (new), src/analysis/BPMTracker.h/.cpp (the two views and `adoptCounters`, A4), tests/test_beat_shift.cpp (new), CMakeLists.txt + tests/CMakeLists.txt (the new sources and targets only). Nothing is wired: the app is unchanged after S1. | this ruling | T-N1..T-N9, T-N11..T-N16 RED on a stub `apply` that writes nothing, then GREEN; unit mutants M1-M11, M19-M23, M27, M28 each RED; the golden green on the lane. Proves the rule on real-tracker sequences, both regimes. |
| S2 | builder | WIRING, SAVE, ROUTE, ANCHOR, PROBE. src/analysis/AnalysisThread.h/.cpp (the word and its two setters, the member, the one `apply` call just before `publishWrite()`, the `adoptCounters` call, the two snapshot writes); src/analysis/FeatureSnapshot.h (A5); src/model/Composition.h ("beatNudgeMs": saved beside "bpmMultiplier", absent or not a number -> 0, out of range clamped, New -> 0); src/MainComponent.h/.cpp (`setBeatNudgeMs`, the two statements of A2 in the resync branch, the load / New hook of A7, the debug callbacks); src/api/ApiServer.h/.cpp (the two features keys; `/api/debug/beat_nudge`); src/recording/RecorderClock.h/.cpp (A17); tests: test_analysis_nudge_thread.cpp (new: T-N10c, T-N10d), test_composition.cpp (T-C), test_beat_nudge_model.cpp (new: T-M), the recorder clock's test file (T-R1; the builder names it); .harmony/probe-nudge.sh, probe-nudge.py, probe-nudge-selftest.py (rows L1-L4, L6, L9; it sources probe-quit-ours.sh and takes the live lock as probe-manual-bpm.sh does; no kill line is copied from an old probe); the build scripts of build-mut-sign, -bypass, -resync. | S1, G-N0 | The unit cases; the probe self-test prints "0 case(s) differ". Then Harmony: G-N1, L1, L2, L3, L4, L6, L9, L10. Proves the wired app: every reader on the shifted beat, zero untouched, Resync exact. |
| S3a | builder (short) | HOOKS ONLY, no top-bar change: src/Main.cpp (ADNA_WINDOW_SIZE); `GET /api/debug/ui_text` keys "topbar" (every top-bar widget: name, x, w, visible) and "wheel_beat"; the Manual-toggle route if none exists; docs/claude/testing-eyes.md lines for these. | S2's rows green | Builds; a `strings` check that a build WITHOUT the test server holds none of the new route names. |
| VG-0 | capture builder | BASELINE: V18, V19 with the manifest (every widget's bounds at 1280 and at the default launch size). Window-only captures of the lane's own app. | S3a | The manifest: is the bar over its width at 1280 today; what size he runs; how the learn title's dash paints today. If the bar is NOT over its width, PL:282-292's numbers are re-read by Harmony before S3b; the design does not change. |
| S3b | builder | TOP BAR. src/ui/TopBar.h/.cpp (the two buttons, the editable text, the group's bounds in `resized()` for the tempo cluster only, M:TopBar.cpp:553-602; the five multiplier buttons `setVisible(false)` and no width under question 62's default; the timer line that sets the text only when what it would paint differs); src/ui/TopBarModel.h; tests/test_topbar_model.cpp; MainComponent.cpp (the callback assignments beside :587-601, `onNudgeEditEnded`); ApiServer (the `nudge_edit` route, ui_text "nudge"). | VG-0 | T-U1..T-U6; M12, M13. |
| S4 | builder | KEY AND PAD, as PL:462-466 with A15, A16: src/binding/Binding.h, BindingManager.h/.cpp, src/ui/BindingOverlay.h/.cpp, src/ui/MidiLearnOverlay.h/.cpp, MainComponent.cpp (`buildBindableTargets`, the `handleBindingAction` case, debug callbacks), ApiServer (`binding_action`, `midi_learn`, `bind_overlay`), tests/test_binding_beat_nudge.cpp (new), probe rows L7, L8, L11, the build script of build-mut-keys. Runs after S3b in the same worktree, never beside it. | S3b | T-B1..T-B6; M16, M17, M25. Then Harmony: G-N1 again, L7, L8, L11. |
| VG | capture builder, then five critic seats; Harmony's verdict | States V1-V11, V14-V17 against V18, V19; a manifest with the model facts per state (the nudge value, the text, each widget's bounds, label width against its box); Boris's words verbatim; the list of texts outside the lane (every top-bar text that is not the nudge group; the old overlay labels). | S4's rows green | Row VG. Boris sees nothing before it is green. |
| S5 | builder | DOCS + MANUAL, as PL:470-489 with: Pitfall 68 = "The published beat is the NUDGED beat: one step (BeatShift) just before the publish; a beat reader reads FeatureSnapshot, never BPMTracker; the bar position is seated at the published beat's edges; a held snapshot keeps the old request sequence; a hand Resync restarts it and may set the tracker's two counters; an injected snapshot is published as given -- before adding a beat reader, a second publisher or a beat field"; docs/claude/integration.md gets the two features keys only (no production write route); testing-eyes.md gets every new debug route and the env hook; the manual's step 4 takes its "earlier / later" from the sign Boris chose; its step 5 says "Resync sets it back to 0; Tap leaves it; opening a show slides to that show's number" (per question 63's answer); no sentence states a speed for a held key; one sentence says a clip that is already running in BPM sync does not move with the nudge (until the transport lane's lock exists). APP-INVENTORY: routes, the action, the test count RE-COUNTED with `ctest -N`. | VG green | Harmony reads the manual against the built app (every control name on screen as written) before Boris sees it. |
Order: G-N0(a) -> S1 (step 0 -> G-N0(b)) -> S2 -> Harmony's rows -> S3a -> VG-0 -> S3b -> S4 -> Harmony's rows -> VG -> S5 ->
merge by RIG-RULES' merge sequence (RED on the pre-merge copy, merge, build, ctest, GREEN).
Reviews: the project's pinned review after S2 and after S4. Real-time lens on S1 / S2: no allocation, lock or system
call in `BeatShift::apply`, the two setters or the per-hop path; the two CAS loops are lock-free; `adoptCounters` has one
caller (T-N14).
What this lane and the others must not both touch: PL:495-502 stands, with A23's three claims added.
WHAT IS NOT IN THIS LANE: PL section 9 (PL:724-760) stands, and with it: a production write route and OSC for the
nudge (H-1, H-4); an app-side key repeat, unless B8 fails (A15); a repeating pad (H-2); moving a clip that is already
running in BPM sync (the transport lane, A18); fixing SF-1 to SF-5 on main; a faster beat wheel; re-laying the top
bar. From the stopped branches, one more part is carried than PL:738-747 lists: the [golden] case of
BF2:tests/test_analysis_sync_thread.cpp (A9) and the accessor `appliedResyncs()` (A4).

## 5 TESTS + GATE ROWS (pre-registered; a bar is met or reported, never loosened; each has its RED arm)
REPLACES PL section 5. Harmony copies gate strings ONLY from here.
UNIT, tests/test_analysis_nudge_golden.cpp (S1 step 0): A9. T-N10a [golden-auto], T-N10b [golden-commands]. Each prints
 "[golden] <case> publishes=1965 hash=0x<16 hex digits> raw=0x<16 hex digits>". RED arm on the lane: M6 (through the
 real thread).
UNIT, tests/test_beat_shift.cpp (S1). Sequences come from a REAL BPMTracker driven hop by hop (512 samples at 48 kHz)
 the way tests/test_bpm_stabilization.cpp and tests/test_downbeat_detector.cpp do; its nine fields are copied into a
 snapshot, then `BeatShift::apply`. "pos" = totalBeatCount + beatPhase as doubles. "truth[i]" = the tracker's
 beatInBar after the hop on which it scored the onset of beat i, i being its totalBeatCount after that hop. RED arm for
 the whole file before the class exists: a stub `apply` that writes nothing.
 T-N1  SIGN + SIZE. Manual 120, D = +250, plus means later: pos' - pos = -0.5 within 1e-4 on every hop after the glide;
       D = -250: +0.5. The same two with plusMeansLater = false: the signs swap. M1 RED.
 T-N2  1 ms. bpm in {60, 120, 174, 200}, D in {-500, -37, -1, +1, +37, +500}, settled: pos' - pos = -D x bpm / 60000
       within 2e-5 on every hop not held; pos'(D + 1) - pos'(D) = -bpm / 60000 within 2e-5. M6 RED.
 T-N3  EDGE TIME. Manual 123, 1000 beats, D in {-500, -37, +37, +500}: the mean over beats of (hop of the count' edge
       - hop of the tracker's edge of the same beat) x 10.6667 ms is within 1.0 ms of D; each one within one hop. M1 RED.
 T-N4  BAR LINE, MANUAL. Manual 120, D = +250 and -250, 64 beats: beatInBar' changes on exactly the hops count'
       changes and runs 0, 1, 2, 3, 0; both bar counts rise by one on exactly the hops beatInBar' goes 3 -> 0; the
       level is true exactly while beatInBar' == 0; barPhase' and phrasePhase' equal the two float formulas bit for
       bit. M3, M4 RED.
 T-N5  AUTO, THE ONSET AND THE WRAP APART (replaces PL's T-N5). The tracker in Auto with the downbeat locked. Arm
       LATE: every beat a confident onset 2 hops after the wrap. Arm EARLY: 1 hop before it. D in {+250, -250, +30,
       -30, +3, -3}; 200 beats after the glide. (a) count' never falls and rises by 0 or 1 per hop; pos' - pos =
       delta within 2e-5 on every hop that is not held. (b) beatInBar' changes only on a hop where count' rose, and
       then by +1 mod 4. (c) the level is true exactly while beatInBar' == 0; totalBarCount' rises by one on exactly
       the hops beatInBar' goes 3 -> 0; barCount' never falls. (d) at every count' edge, beatInBar' == truth[that
       beat]. (e) no hold is longer than 4 hops. Arm MISSED (LATE with every tenth onset dropped): (a), (b), (c).
       M2 RED on (a); M21 RED on (d) (D = +3, LATE); M28 RED on (b) (arm MISSED).
 T-N5b A BAR SNAP IN AUTO (BE-3). T-N5's LATE and EARLY sequences with a pending Bar-snap trigger consumed as
       Autopilot does (a count rise, then `Layer::processPendingTrigger(beatInBar, barCount, ...)` of the same
       snapshot; the real function if a test can build a Layer, as tests/test_layer_runtime does, else its two-line
       rule, and the report says which). For every D in {+3, +30, +250, -30}: the trigger fires on a hop where count'
       rose and beatInBar' == 0, and that beat is a beat with truth == 0. For D = 0 the test PRINTS "INFO  T-N5b D=0
       lands <k> beat(s) after the downbeat" and asserts nothing (SF-1). M21 RED.
 T-N6  NONE SKIPPED. A Tap from 60 to 200 with D = -500, and from 200 to 60 with D = +500: count' rises by at most 1
       on any hop. M10 RED.
 T-N7  TEMPO CHANGE. `followExternalTempo` 120 -> 126 at D = +500: after any hold (<= 4 hops) pos' - pos = -500 x 126
       / 60000 within 2e-5; a ramp 120 -> 130 in 100 steps: beatPhase' never steps back, count' never falls. NEW ARMS
       (A20): 120 -> 200 at D = +500 (delta goes from -1.0 to -1.667 beat). Applied on a hop where the tracker's phase
       is 0.1: a hold follows, between 15 and 17 hops (0.567 beat at 200 BPM is 15.9 hops), and count' never falls.
       Applied at phase 0.8: no hold; beatPhase' steps back inside the beat and count' does not change on that hop.
       M8 RED.
 T-N8  THE RESTART (A2). Manual; D in {+40, -40, +250, -250, +500, -500} x the tracker's phase at the press in {0.2,
       0.7} x bpm in {120, 200}; the word carries a zero request and the tracker applies a Resync on the same hop. On
       THAT hop: not held; beatPhase' == 0.0f; beatInBar' == 0; the level true; barCount' == 0; resyncBarOrigin' ==
       totalBarCount'; totalBeatCount' - F's is 0 or 1; totalBarCount' >= F's; the sequence is S's; the applied value
       is 0.0. Adoption is returned exactly when S.totalBeatCount lies outside [F.count, F.count + 1] or
       S.totalBarCount < F.totalBarCount; the test asserts that at least one arm adopts (-500 at phase 0.2) and at least
       one does not (+40, -40). ONE MORE ARM, the bar counter: D = -40, the press while the tracker is at beatInBar 3,
       phase 0.95 (the shifted view has crossed the bar line, the tracker has not): adoption is returned with bars
       = F.totalBarCount and beats = S.totalBeatCount. After the test applies an adoption to the tracker: on the next hop the nine fields
       equal the tracker's bit for bit and `engaged()` is false. M23 RED ("held on the Resync hop", -500 at 0.2).
 T-N8b A REPLAYED RESYNC, THE ORIGIN, THE RELABEL. D = +250 kept, a Resync with NO zero request while the tracker is at
       beatInBar 0, phase 0.2 (the shifted view is still in the bar before). PRECONDITION asserted first: on at least
       one hop S.resyncBarOrigin > the un-clamped totalBarCount' (the arm can go red). Then: resyncBarOrigin' <=
       totalBarCount' on every hop; the first not-held hop at or past the Resync's beat has beatInBar' == 0; from
       there beatInBar' == (count' - that beat) mod 4 for 16 beats; no hold longer than half a beat + 1 hop.
       M9 RED; M27 RED.
 T-N8c THE SEQUENCE DURING A HOLD (A3). Manual 120, D = +40, `setManualBPM(120)` posted when the tracker's phase is
       0.3. PRECONDITION: the hop that applies it is held. On every held hop the published trackerRequestSeq equals
       the previous hop's; the first not-held hop carries the Tap's. M22 RED.
 T-N8d THE WORD'S ORDER (A2). (i) the zero request one hop before the Resync: one glide hop, then the restart; (ii)
       both on one hop: the restart; (iii) a Resync with no zero request: no restart, the nudge kept.
 T-N9  ONLY THE BEAT. On every hop of T-N4, T-N5, T-N8c: `nonBeatCrc` (the nine fields and trackerRequestSeq zeroed)
       before == after. M11 RED.
 T-N11 GLIDE. 0 -> +500: the applied value moves 2.6667 ms per hop (within 1e-9) and reaches 500.0 exactly; no hold;
       beatPhase' never steps back; a FIRST hop with D = +500 also glides. M7 RED.
 T-N12 BACK TO ZERO. +10 then 0, Manual and Auto (T-N5 LATE): once the applied value is 0.0, within one beat
       `engaged()` is false and every later snapshot equals the tracker's bit for bit. M5 RED.
 T-N13 A LATER NUDGE BEFORE THE FIRST LOCK. D = +500 fully applied while bpm is 0, then the tracker locks: count'
       stays 0 and beatPhase' 0 until the tracker's position reaches the shift; no unsigned wrap. M19 RED.
 T-N14 REAL-TIME TEXT GATE. BeatShift.cpp / .h hold none of: "new ", "malloc", "std::vector", "std::string",
       "std::mutex", "lock_guard", "printf", "std::cout", "juce::". And: `BeatShift::apply(` and `adoptCounters(` each
       occur exactly once in src outside their own class's files, both in AnalysisThread.cpp. M20 RED.
 T-N15 NEVER ENGAGED = UNTOUCHED. D == 0 from the start, T-N5's LATE sequence: after every `apply` all 384 bytes of
       the snapshot equal their value before it. M6 RED.
 T-N16 A HELD KEY NEVER LAGS. Manual 120, the target raised by 1 every 3 hops for 60 hops: pos' - pos = -(the target
       of that hop) x 120 / 60000 within 2e-5 on every hop.
UNIT, tests/test_analysis_nudge_thread.cpp (S2; the real thread in lockstep, the lane's APIs):
 T-N10c the target set to +10 at hop 300 and back to 0 at hop 500: the hash of hops 800..1964 equals the hash of the
       same hops of a never-nudged run made in the same test. M5 RED.
 T-N10d the published beatNudgeAppliedMs goes 0.0 -> 10.0 in four hops and beatShiftState's bit 0 follows `engaged()`.
MODEL:
 T-U1  `offBeatText`: 0 -> "off beat by 0 ms"; 12 -> "off beat by +12 ms"; -7 -> "off beat by -7 ms"; 500 -> "off beat
       by +500 ms"; -500 -> "off beat by -500 ms"; every character ASCII. M12 RED.
 T-U2  `parseOffBeat`: "12" -> 12; "+12" -> 12; "-7" -> -7; "off beat by -7 ms" -> -7; " 30 ms" -> 30; "900" -> 500;
       "-900" -> -500; "abc" -> false; "" -> false; "1.9" -> 1. M13 RED.
 T-U3  `clampNudge`: -501 -> -500; 501 -> 500; 0 -> 0.
 T-U4  `nudgeTextChanged(painted, ms)`: "off beat by +12 ms" with 12 -> false; with 13 -> true (Pitfall 59).
 T-U5  the tooltips, plus means later: "-" = "Move the beat 1 ms earlier. Hold to keep moving."; "+" = "Move the beat
       1 ms later. Hold to keep moving."; the text = "How far the beat is moved from the detected or tapped beat.
       Click to type a number." With the sign constant false the words "earlier" and "later" swap.
 T-U6  the text's editor: Enter, Esc and a loss of focus each call `onNudgeEditEnded` exactly once; only Enter
       changes the value (a JUCE component is invisible by default: Pitfall 34).
 T-M1  `nudgeAfterTempoCommand("resync", false, 40)` == 0.  T-M2 ("resync", true, 40) == 40. M14 RED.  T-M3 "tap",
       "manual", "auto", "link" (replay or not, 40) == 40.
 T-C1  save with beatNudgeMs 37 -> the file has "beatNudgeMs": 37 -> load -> 37. M15 RED.  T-C2 no key -> 0; 9999 ->
       500; "x" -> 0.  T-C3 New: 37 -> 0.
 T-R1  RecorderClock (A17). (a) applied 0 then 5, steady: exactly one "nudge" anchor, on the 12th steady tick. (b)
       applied changing on every tick for 240 ticks: none while it changes, one after it settles. (c) 20 held ticks,
       then released: one "nudge" anchor on the first not-held tick, and `beatAt` of that tick's time equals the
       clock's beat within 1e-9. (d) applied 0.0 and state 0 throughout: no anchor whose why is "nudge". M18 RED.
 T-B1  a key press on a BeatNudge binding with step +1 -> +1; step -1 -> -1; a release -> 0. M25 RED.
 T-B2  a MIDI note-on -> the step; note-off -> 0.
 T-B3  a MIDI CC, Absolute and Relative, any value -> 0; `bindingIsLive` false for both. M16 RED.
 T-B4  `learnMidiCC` with a BeatNudge candidate: false; the binding list is byte-equal before and after; an older
       binding on that CC is still there. M16 RED.
 T-B5  toVar / fromVar: "targetNudgeStepMs" round-trips -1 and +1; absent -> +1; BeatNudge's saved number is the
       last one and every older action keeps its number. M17 RED.
 T-B6  `learnMidiCC` with any other candidate does what the overlay's inline loop did (the CC's old binding removed,
       the new one added).
MUTANTS: M1 delta's sign flipped. M2 the hold removed. M3 beatInBar' = S's. M4 the bar counts not carried. M5 the
 glide stops 0.01 ms short. M6 the target read as D + 1. M7 no glide. M8 delta from the first hop's bpm. M9 the origin
 clamp removed. M10 the cap removed. M11 `apply` also writes onsetCount. M12 no "+". M13 the parse does not clamp. M14
 the replay guard removed. M15 the key not written. M16 `bindingIsLive` always true. M17 BeatNudge inserted before
 TriggerRoutine. M18 the "nudge" anchor removed. M19 count' in uint32. M20 a std::vector member. M21 the lag term
 removed. M22 the sequence not held. M23 the restart removed (the applied value snaps to 0, then the ordinary guards).
 M24 (app) `apply` always given a word of 0. M25 the binding step's sign ignored. M26 (app) the two message-thread
 statements of A2 removed. M27 no relabel after a tracker Resync. M28 the bar fields re-seated on every hop.

GATE ROWS (Harmony; strings exact). All live rows: the lane's test-server build, the real app (NOT test mode: the
analysis thread must run), a 120 BPM click file as the source, Manual 120 unless the row says otherwise; `open -g`;
the live lock; the probe quits only the pid it launched. "The tracker's line" = a straight line fitted to (probe time,
totalBeatCount + beatPhase) over an arm of 8 s at nudge 0; held polls (beatShiftState bit 1) are never fitted or
judged; a row that uses the line has a nudge-0 arm before AND after, and is VALID only when the two arms' median
residuals differ by <= 1.0 ms (else "INVALID: drift" -- it blocks and goes to the architect; it is not re-run until
green). A FAIL is a FAIL; "flake" needs >= 5 runs per arm.
G-N0 THE GOLDEN IS MAIN'S. On Harmony's pristine worktree of 185147b with only the golden file and its CMake lines
     added: three runs print the same "[golden] auto publishes=1965 hash=0x... raw=0x..." and the same "[golden]
     commands publishes=1965 hash=0x... raw=0x..."; they equal the constants in the file (the raw constant only if it
     was stable when recorded, A9); "100% tests passed, 0 tests failed out of 2"
     for that target. And N_main = the total of `ctest -N` there. RED arm: the lane's M6 build fails the same file.
G-N1 UNIT. "100% tests passed, 0 tests failed out of <N>", N = N_main + N_new (A22). Each unit mutant (M1-M23, M25,
     M27, M28) built in build-mut-<n> prints at least one FAILED case of the test named for it above.
L1   ZERO. No nudge was ever set. 500 polls of /api/features: beatNudgeAppliedMs == 0.0 and beatShiftState == 0 in
     all; `GET /api/debug/beat_nudge` "ms": 0. "PASS  L1 zero: 500 of 500 polls not engaged".
     RED arm: build-mut-keys (M6): "FAIL  L1 zero".
L2   THE BEAT MOVES BY D. Arms in the order 0, +250, 0, -250, 0, +500, 0, -500, 0, +1, 0, -1, 0; each 8 s after
     features.beatNudgeAppliedMs equals the arm's ms; /api/features every 5 ms; t = the poll's send / receive midpoint.
     For each arm the line is fitted to its two neighbouring 0 arms; the arm's median residual r (beats) gives
     measured = -r x 60000 / bpm. BAR, the four large arms: |measured - D| <= 3.0 ms, each arm >= 1000 polls. "PASS
     L2 arm <D>: measured <m> ms". The +1 and -1 arms print "INFO  L2 arm <D>: measured <m> ms" and have no bar.
     RED arms: build-mut-sign (FAIL on the four, measured near -D); build-mut-bypass (FAIL on the four, measured near 0).
L3   A QUANTISED FIRE LANDS ON THE SHIFTED BEAT; A FIRE BY HAND DOES NOT WAIT.
     L3a nudge +250, Quantize "Next Beat": 30 fires by `POST /api/trigger_clip` at pseudo-random times (the probe's
       fixed seed); for each, the first poll (5 ms) in which the layer's playing clip is the new one, at time t_f.
       BAR, 30 of 30: totalBeatCount == the count at the request + 1; beatPhase < 0.15; and the tracker's line at
       t_f has a fractional part between 0.40 and 0.70 (half a beat after the tracker's own beat). "PASS  L3a
       quantised fires land on the shifted beat: 30 of 30".
     L3b Quantize "Off": 30 fires; 30 of 30 show the new clip within 150 ms and at least 20 of 30 at beatPhase >=
       0.15. "PASS  L3b fires by hand do not wait: 30 of 30".
     L3c after a Resync at nudge 0 (n = totalBeatCount of the first poll that carries the Resync's sequence), nudge
       +250: 6 fires with Quantize "Next Downbeat" and 6 of a clip whose own snap is Bar. BAR, 12 of 12: the landing
       poll has beatInBar == 0 and beatPhase < 0.15, and ((the tracker's line at t_f) - n) / 4 has a fractional part
       between 0.10 and 0.18 (the tracker's bar line plus half a beat). "PASS  L3c bar fires land on the shifted
       downbeat: 12 of 12".
     L3d INFO, Auto (the click file, LOCKED), nudge 0 then +3: "INFO  L3d Auto: beatInBar trails the count on <k> of
       <n> beats, median <m> ms" and the beatInBar each of 6 Bar-snap fires landed on, at 0 and at +3. No bar.
     The routes that set Quantize and read the playing clip are the ones probe-boxes.sh uses (the builder names
     them). RED arm: build-mut-bypass -- L3a and L3c FAIL on the tracker's-line clause.
L4   THE BAR TURNS WITH THE SHIFTED BEAT. A Resync at nudge 0 (n as in L3c), then +500; 70 s of polls. (a) beatInBar
     changes only in a poll where totalBeatCount changed; (b) totalBarCount rises by one exactly when beatInBar goes
     3 -> 0; (c) neither counter falls; (d) ui_text's "Bar N" and "wheel_beat" equal the values of a features poll
     taken in the 150 ms before the ui_text read; (e) at every poll where beatInBar goes 3 -> 0, ((the tracker's line
     at t) - n) / 4 has a fractional part between 0.24 and 0.28 (one beat after the tracker's bar line). BAR: 0
     violations of (a)-(d) in >= 130 beats and (e) on every one of >= 30 bar lines. "PASS  L4 the bar turns with the
     shifted beat: 0 violations in <n> beats". RED arm: build-mut-bypass -- (e) FAILS.
L6   SAVED WITH THE SHOW, AND A LOAD SLIDES. Set 37; `POST /api/debug/save_composition` to a temp path; set 0; load
     that file; wait for the load on ui_text. BAR: "ms" == 37 on the first read after the load; beatNudgeAppliedMs goes
     from 0.0 to 37.0 with no two successive 5 ms polls more than 5.4 ms apart, within 400 ms; New -> "ms" 0 and the
     applied value back to 0.0 the same way; a fixture without the key -> 0; no new text in ui_text's notice keys.
     "PASS  L6 saved with the show and slid on load". RED arm: build-mut-resync (M15): "FAIL  L6: ms 0 after load".
L7   KEY / PAD ACTION. `POST /api/debug/binding_action` {"action": "beatNudge", "step": +1} x 3, {"step": -1} x 5, a
     release (value 0), a CC-typed binding x 1: "ms" after each = 1, 2, 3, 2, 1, 0, -1, -2, -2, -2; then "ms" set to
     499 and step +1 twice: 500, 500. And the route: {"ms": 9999} -> 500; {"ms": "x"} -> HTTP 400, "ms" unchanged.
     "PASS  L7 the handler case moves the nudge: 12 of 12 steps". RED arm: build-mut-keys (M25).
L8   LEARN REFUSES A KNOB. As PL:614-618, with A16(c)'s baseline count. "PASS  L8 MIDI learn refuses a CC on a nudge
     target and changes nothing else; a note attaches: 8 of 8 states". RED arm: build-mut-keys (M16).
L9   TAP AND TEMPO LEAVE THE NUMBER; RESYNC ZEROES IT AND IS BEAT 1 AT ONCE. (a) set +40; `binding_action` "tapTempo"
     twice 500 ms apart -> "ms" 40, bpm 120 +/- 2; `POST /api/set_bpm` 126 -> "ms" 40. "PASS  L9 Tap and tempo leave
     the number". (b) for each start value +40, -250, -500, +500 (Manual 120): set it and wait for it; note the last
     polled totalBeatCount c; `POST /api/resync`. BAR for each: "ms" == 0 at once; in the first features poll whose
     trackerRequestSeq has reached the posted value (read as probe-manual-bpm.sh reads it): beatInBar == 0, beatPhase
     < 0.2, beatShiftState's held bit clear, beatNudgeAppliedMs == 0.0, totalBeatCount between c and c + 2; and
     totalBeatCount never falls between two successive polls of the row. "PASS  L9 Resync zeroes and is beat 1 at
     once: 4 of 4 arms". RED arm: build-mut-resync (M26): "FAIL  L9 resync: ms 40, want 0".
L10  THE MUTANT APPS. One line per RED arm above: "RED  <row> on build-mut-<name>: <the FAIL line it printed>"; a
     mutant app that prints PASS on its row FAILS this row.
L11  THE TEXT FOLLOWS THE MODEL; THE EDITOR GIVES THE KEYS BACK. (a) after every set in L2, L7, L9: ui_text's
     nudge.text equals T-U1's string for "ms" within 200 ms. (b) `nudge_edit`: begin -> editor_open true; type "12",
     enter -> "ms" 12, the text "off beat by +12 ms", editor_open false, focus_home_count + 1; begin, type "x9",
     escape -> "ms" 12, + 1; begin, focus_lost -> "ms" 12, + 1. (c) the three tooltip strings equal T-U5's. "PASS
     L11 the text follows the model: <n> of <n>; the editor: 3 of 3 closes hand focus home". RED arm: the lane at
     the S3a commit, before the top bar is touched ("FAIL  L11: no nudge in ui_text").
VG   Five critic seats on V1-V11, V14-V17 against V18, V19: 0 MUST. Pre-registered MUSTs: a nudge text not exactly as
     T-U1; the text clipped at any state; a top-bar control overlapping another at 1280 with Manual on that did not
     overlap in V18; a non-ASCII glyph in the group or in the learn title; a tooltip string that is empty or differs
     from T-U5 (read from ui_text).
After every live batch: no Audio-DNA pid of the probe's is left, none of its windows; no Output window was opened; no
full-screen capture was taken; no synthetic input was sent.

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; replaces PL:630-653)
B1  In the room, music playing, Manual or Auto locked, Resync on the "1". Stand where you watch from. Press "+" a few
    times, then "-" -> the flashes and cuts slide later, then earlier, against the music; the BPM number does not
    move; the text counts 1 ms per press. WRONG: the tempo number changes; the picture does not move; only some
    things move (a quantised clip still fires on the old beat while the effects pulse on the new one).
B2  Hold "+" with the mouse -> the number runs up steadily, about 20 a second, and stops at +500. Is that speed right
    for you? WRONG: it jumps, or the picture stutters while it runs.
B3  Click the text, type 120, press Enter -> it shows +120 at once and the beat slides there in about half a second
    (a jump of 500 takes 2 seconds, and the beat runs a little slow or fast while it slides). Then press one of your
    clip keys -> the clip fires. WRONG: a freeze or a double flash; your keys do nothing after typing.
B4  Fire a clip with Quantize Off, turn a knob -> both act the instant you do it. Set Quantize to Next Beat and fire ->
    the clip starts on the nudged beat. WRONG: the fire by hand waits; the quantised fire lands on the old beat.
B5  Loud hits (a snare, a drop) -> what follows loudness or hits still lands on the sound, not on the nudged beat.
    WRONG: hit-driven flashes drift with the nudge.
B6  Tap the tempo -> the number stays. Set a big number (-300), then press Resync on the "1" you hear -> it reads 0
    and the "1" is exactly where you pressed. WRONG: Tap changes the number; after Resync the circle hesitates, or a
    clip waiting for the bar starts a bar late.
B7  Save the show at +30, open another show, open the first again -> +30 is back and the beat slides to it, no jump
    (question 63). WRONG: 0; a message appears; a visible jump.
B8  Put "Beat earlier" / "Beat later" on two keys and hold one -> it keeps moving, at the speed your Mac repeats a
    held key. Put them on two pads -> one hit = 1 ms. Try to learn a knob on them -> it does not take, and the knob
    keeps the job it had. WRONG: a held key moves only once (tell us: the fix is already decided); a knob attaches.
B9  At a glance from arm's length: is "off beat by +12 ms" readable where it sits, and are "-" and "+" where your hand
    expects them? Is "+" = later the way round you think of it (question 61)?
B10 The beat circle: with +250 at 120 BPM its lit quarter turns half a beat after the music's beat. It is a coarse
    sign (it is redrawn 15 times a second): a few ms cannot be seen on it. The number is what you read.
B11 An honest limit to judge by eye: things that START on a beat (a quantised clip, an autopilot step) land within
    about one hundredth of a second of the nudged beat, as they do today around the un-nudged one; smooth beat-driven
    movement follows the nudge to the millisecond.
B12 A clip that is ALREADY RUNNING in BPM sync does not move when you nudge: today it only follows the tempo. A clip
    you fire quantised starts on the nudged beat. The running clip will follow once the clip transport's bar lock is
    built. Tell us if that is not what you meant by "everything".
B13 With music in Auto, a clip set to start on the bar: with any nudge set it starts on the "1". At exactly 0 the
    app behaves as it does today; if bar starts feel a beat late at 0, tell us (side finding SF-1).

## 7 BORIS QUESTIONS (numbers 61-63; each has a default A; nothing waits)
61. Which way is plus? "off beat by +12 ms" means:
    A (default) The app's beat comes 12 ms LATER than the beat it found or you tapped. "+" moves it later, "-" earlier.
    B The other way round: "+" moves the beat earlier.
62. The five buttons "/4 /2 x1 x2 x4" next to the tempo do nothing today, and the nudge needs their space.
    A (default) Take them out. (They can come back when they do something.)
    B Keep them and make them halve and double the tempo; the nudge then sits further right, after Quantize.
63. You have set the number for the room you are in. Then you open another show.
    A (default) That show's own number takes over, and the beat slides to it. (What I told you before: the number is
      remembered with the show.)
    B The number you set in this room stays, whatever show you open. Shows do not carry it.
Not asked, because his words or a reading told to him already settle them: 49 (after a Resync the number reads 0); 50
and its follow-ups (where key and MIDI settings live); a key or a pad, no knob (reading R27); one ms per press, a held
key keeps moving, a number can be typed, -500 to +500 (reading R26); the text and the two controls beside the tempo,
always shown (reading R25). The plan's own question 63 (a held pad) is Harmony's (H-2).
The line that changes for each B: 61 -> the constant of A8. 62 -> the group moves to the row's flexible place by one
constant and the five buttons become visible again (PL:286-288). 63 -> A7's last sentence.

## 8 HARMONY'S DECISIONS (each has a default)
H-1  The setter is a TEST-SERVER route; production gets no write route for the nudge (A12). DEFAULT: as ruled.
     ALTERNATIVE: a production `POST /api/beat_nudge` beside `/api/set_bpm` -- then OSC should be decided with it.
H-2  A held pad: one hit = 1 ms, no repeat (A15). DEFAULT: as ruled. ALTERNATIVE: it repeats, built with the fallback
     schedule of A15 and the note-off already polled for Momentary bindings.
H-3  A replayed Resync (a take, a routine) resyncs the beat and leaves the nudge (A2; PL:331-332). DEFAULT: as ruled.
H-4  OSC for the nudge is not built. DEFAULT: not built.
H-5  The wheel stays at 15 Hz (A19). DEFAULT: stays; a faster wheel in its own layer is an idea, not this lane.
H-6  A held key rides the Mac's key repeat (A15). DEFAULT: as ruled; the fallback is built only if B8 fails.
H-7  SF-1 (at nudge 0 in Auto a bar-quantised fire can land a beat late; with any nudge it does not). DEFAULT: filed
     against main, not fixed here, because fixing it at 0 changes main's published bytes in Auto and the Harmony
     constraint forbids that in this lane. ALTERNATIVE, her constraint to change: run A1's step 6 at 0 as well in Auto
     -- one condition in step 1 -- and give up "bit-identical at 0" for the hops where the tracker's bar position
     trails its count; the golden then needs a second constant recorded with that rule. Measure FM-2 before deciding.
H-8  Tooltips are judged as text, not as captures (A14). DEFAULT: text. ALTERNATIVE: one TEMPORARY hook build, as the
     cell-tooltip precedent, reverted and re-built with a strings check.
H-9  The restart may set the tracker's two counters (A2, `adoptCounters`): the beat counter only when the old nudge
     was half a beat or more, the bar counter when an earlier nudge had already shown a bar line the tracker had not
     reached. DEFAULT: as ruled -- after every hand Resync the published beat is the tracker's own, byte for
     byte. ALTERNATIVE if the real-time review will not have a write into the tracker: BeatShift keeps the two
     differences itself and adds them on every later hop; the tracker is never written, and in that corner the
     published counters stay a constant apart from the tracker's until the app quits (the step stays engaged; "at 0
     identical to the tracker" then holds for every field except those two). Either way T-N8's bars stand.
H-10 Other lanes branch from main now; the lane that merges second takes main in as step 0 (A23). DEFAULT: yes. Told
     to the output-settings and transport lanes: offsets 332 and 336 are taken; append any new Binding action AFTER
     BeatNudge; the transport lane's bar lock reads the bus (A18).
H-11 The manual's home is docs/manual/ (PL:480-487). DEFAULT: as the plan.
H-12 The next free Pitfall number, 68, goes to this lane. DEFAULT: yes.

## 9 SIDE FINDINGS
SF-1  MAIN, Auto: a fire quantised to the bar (Bar / 2 Bar / 4 Bar snap, Next Downbeat) can land one beat AFTER the
      downbeat. When the onset trails the tracker's phase wrap, the count edge is published with the old beat-in-bar
      (V4) and Autopilot reads both from that snapshot (V3); the bar position turns a few hops later, with no count
      edge, so the fire waits for the next beat. INFERRED from the lines; how often it happens is not measured (FM-2).
      Under A1 it cannot happen with any nudge other than 0. The same lag makes main's barPhase step back a quarter
      bar for those hops, and `PresetSelector`'s wrap test (M:src/sources/PresetSelector.cpp:79, "barPhase < last and
      last > 0.5") reads ANY backward step above 0.5 as a bar crossing (INFERRED; PL R2 names it).
SF-2  MAIN, Auto before the downbeat lock: a Resync's bar position is overwritten at the next scored onset
      (M:BPMTracker.cpp:393-400, beatInBar = beats scored mod 4). INFERRED.
SF-3  MAIN: a Resync pressed in the FIRST half of a beat restarts that beat with no beat edge (M:BPMTracker.cpp:253-258,
      :566-578), so a fire queued for the next downbeat waits a whole bar. INFERRED. A2 keeps this behaviour for the
      published beat (0 or 1 edge at the press, as the tracker); it is not made better or worse here.
SF-4  MAIN: the BPM field keeps the keyboard after Enter (V10); bound keys then type into it (INFERRED, JUCE focus
      rules, not run). A13 gives the keys back for the nudge text only. The BPM field's one-line fix is not in this lane.
SF-5  MAIN: a BPM-synced video's speed line has no tempo term (M:Renderer.cpp:1665); whether `videoBeats` carries the
      tempo was not read. For the clip-transport lane.
SF-6  PLAN: F11 says the window opens at 1280 x 800 and marks it VERIFIED; it opens maximized (V11). F15 and PL:666-667
      call questions 49 and 50 open; both are answered (V18). The dispatch to this ruling repeats that; the record wins.
SF-7  PLAN: the RecorderClock citations (PL:245-247) are off by six lines (V8). SEAT: stage-hands returned
      "citations_rechecked": false; ST-8 cites "RecorderClock.cpp:95-98" in a file of 87 lines. Every line its
      attacks rest on was re-read here (V8, V9, V10, V13, V18); its attacks stand or fall on those.
SF-8  SEATS: the bar "hold <= |D| + 1 hop" (GA-4, ST-2) would have been a false bar for the plan's rule (V12).
SF-9  MAIN: a Tap that completes a beat adds a beat without moving the bar position (facts-beat-controls.md section 3
      item 5). Under A1 the published bar position then repeats once at the next edge, as main's does at the Tap.
SF-10 PROCESS: one scratch python run of the paper model was refused by the command runner's removal check (the
      script removes nothing); the same arithmetic was run from a file. Nothing was deleted anywhere.

## 10 RISKS (the strongest counterargument first)
R1  STRONGEST: "This is BeatLead again. The plan threw its machinery out for one signed rule; this ruling puts a lag
    term, edge seating, a relabel, a hold, a cap and a restart with a write into the tracker back in. A delay ring for
    LATER (PL R1) needs none of it." It loses because (1) a ring cannot do EARLIER, so two mechanisms would still meet
    at 0; (2) for LATER a ring replays the tracker's own gap between count and bar position D ms late -- it keeps
    SF-1 at every nudge, where A1 removes it at every nudge but 0; (3) his rule that Resync zeroes the number means a
    ring must be emptied at a Resync, which is the restart problem again; (4) each piece of A1-A3 answers a failure
    that was derived from the code and then reproduced in the model with that piece removed (V22) -- none is taste.
    What would change the ruling: a unit case of A1-A3 that cannot be made green without a new rule (A24: the builder
    stops), or FM-2 showing that on real music the tracker's bar position often trails its count by more than one beat
    (then the lag's clamp of one beat is wrong and the rule comes back to the architect).
R2  NOTHING WAS RUN. The guarantees rest on reading 185147b and on a python model of that reading. A wrong reading of
    the order inside one hop (`updatePhase`, `scoreBeat`, `updatePhrase`, `applyResync`) would pass the model and fail
    the app. Cheapest refuting test: T-N5 and T-N8, written against the REAL BPMTracker, RED first.
R3  THE WRITE INTO THE TRACKER (`adoptCounters`). It is the first one from the publish step. It is reachable only at a
    hand Resync while a nudge was applied (A2 says when), it sets three counters whose origin is arbitrary,
    and the lint pins its one caller. If the real-time review will not have it, H-9's alternative is already ruled.
R4  0 AGAINST 1 MS (H-7). With any nudge the published bar position is seated at the beat's edge; at 0 it is main's.
    In Auto, when onsets trail the wrap, a bar-quantised fire can land on a different beat at 0 than at +1. Main is
    the odd one; FM-2 says how often; fixing it at 0 is Harmony's constraint to lift, not this ruling's.
R5  THE GLIDE AGAINST "not the tempo". While the number slides the beat runs up to a quarter slow or fast: one hop
    for a 1 ms press, 2 s for a typed 500. He is told (B3). The alternative is a jump -- a frozen beat or two beat
    edges at once -- not "no effect".
R6  THE TOP BAR'S WIDTH is unmeasured until VG-0. If the bar is already over its width at 1280, the group makes it 22
    px worse under 62 A; this lane does not re-lay the whole bar.
R7  A HELD KEY rides a system setting (V13). B8 is the only check; the fallback is ruled (A15).
R8  THE LIVE ROWS lean on the tracker's line being straight over about 25 s in Manual; the audio clock against the
    probe's clock can drift by parts in ten thousand. The VALID clause measures it every run; INVALID blocks.
R9  FOUR MUTANT APPS cost disk and build time (RIG-RULES: `df -h` first). They can be built one after another in one
    scratch directory; the restore-rebuild rule applies after each.
R10 THE GOLDEN covers every field and bytes 332..383, and a raw hash of all 384 bytes where that is reproducible on
    main (A9). If the raw hash is NOT reproducible on 185147b, alignment padding between fields is outside the proof;
    no reader can reach it through a field. The row reports which of the two it was.
R11 TWO LANES in TopBar.cpp / MainComponent.cpp / ApiServer.cpp (PL:495-502): a merge conflict is a builder's step 0.

FACTS HARMONY MUST MEASURE (each named where it is used)
 FM-1 N_main: `ctest -N` on a build of 185147b (G-N0; A22).
 FM-2 In Auto, on the click file and on real music: on how many beats beatInBar turns later than totalBeatCount, and
      by how many ms (L3d INFO). Decides H-7 and tests the lag's one-beat clamp (R1).
 FM-3 The golden reproduces on her pristine 185147b; the tail is zero; whether the raw 384-byte hash is stable (G-N0).
 FM-4 The window size he runs: the default launch on his display (VG-0's manifest).
 FM-5 Whether today's top bar is over its width at 1280 (VG-0's manifest).
 FM-6 How the learn title's dash paints on main (V19).
 FM-7 The live rig's drift: the spread between the two nudge-0 arms around each arm (printed by L2, L3, L4).
 FM-8 The count of old overlay labels wider than their buttons (L8's first run; H-16 counted 15).
 FM-9 (Boris, not Harmony: no synthetic input) whether a held key repeats on his Mac (B8).

SUMMARY: 31 attacks ruled (21 ACCEPT, 9 PARTIAL, 1 REJECT); 24 amendments (A1-A24); stages G-N0, S1, S2, S3a, VG-0,
S3b, S4, VG, S5; 3 Boris questions (61-63) with defaults; 12 Harmony decisions with defaults; 9 facts to measure.

STATUS: DONE
