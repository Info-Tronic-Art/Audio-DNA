# RULING nudge-row -- architect ruling on the blind council's attacks on plan-nudge-row.md (lane "nudge-row": the delta on Boris's answers 61, 62, 111; s-rta-1004)
Architect (opus, max effort; Fable is out of usage), 2026-10-04. Harmony decides after this; this is her working document.
Read-only. Pins checked: main HEAD = 185147b and `git status --short -- src tests docs CMakeLists.txt` printed nothing
(plain-file reads below are reads of 185147b); lane/bf2 = 740b6d6 clean; lane/bf2-keys = 9eab9bd clean. Nothing was built,
no test was run, no app or probe was launched. Every width below is arithmetic on cited lines, not pixels.
Shorthands: M: = main at 185147b (paths under /Users/boriskarpman/projects/RealTimeAudio). PL: = plan-nudge-row.md, the
plan ruled on (NR1..NR5 its items, F1..F24 its facts, MR1..MR19 its mutants). RN: = ruling-nudge.md, the ruling this
delta amends (A1..A24). RT: = ruling-transport-delta2.md. BD: / BL: = .harmony/binding-decisions.md / boris-feedback-
backlog.md. FB: / FR: = facts-beat-controls.md / facts-resolume-screen-delay.md. J: = the JUCE copy in the build tree
(build/_deps/juce-src) -- NOT part of the pin, read for one API fact.
Labels: VERIFIED (read at the pin by me), COMPUTED, INFERRED, ASSUMED.
Precedence: Boris's verbatim words > Harmony's adoption > THIS RULING > PL > RN > plan-nudge.md's body. Amendments
NA-1..NA-18 override PL where they differ; where both are silent, RN stands.
Papers: .harmony/.reports/s-rta-1004/attack-nudge-row-papers.md (2 of 2 seats, 15 attacks; 16389 characters as dispatched
and as transcribed; the array is closed).
Harmony constraint (nudge row), as dispatched: the row is HIS list in HIS order -- beat wheel, play, pause, stop, BPM
number, BPM minus, BPM plus, nudge back, nudge forward, /2, *2, tap, resync; nothing is added to it and nothing left out;
Manual, Link and Quantize stay to its right (reading R69). Every control in it does something. A stopped or paused BPM
timer is a STATE he can see in the row; "nothing moves" covers every beat-driven reader at once; what is not BPM-based
is untouched. The engine stages of RN (G-N0, S1, S2) and the key / pad stage S4 are not re-opened except where the
stopped timer needs a field.

## 0 VERDICT
The plan NEEDS REVISION; its core stands. 15 attacks ruled: 12 ACCEPT, 3 PARTIAL, 0 REJECT. 18 amendments (NA-1..NA-18):
11 answer the attacks, 6 come from re-reading the code the plan rests on (no seat raised them: NA-12..NA-17), 1 is
bookkeeping (NA-18).

RULED FIRST

1. WHAT A PAUSED AND A STOPPED BPM TIMER PUBLISH
 The nine beat fields are beatPhase, beatInBar, barPhase, downbeatDetected, barCount, totalBarCount, totalBeatCount,
 phrasePhase, resyncBarOrigin (V1).
 PAUSED  all nine keep, bit for bit, the values of the hop that applied the pause, on every hop until play. No edge.
         The tempo (bpm) and the tracker's state word are published LIVE: in Auto the detector keeps listening and the
         tempo keeps tracking; in Manual it is the hand tempo. Bit 2 of `beatShiftState` is set.
 STOPPED beatPhase 0.0, beatInBar 0, barPhase 0.0, downbeatDetected true, barCount 0, phrasePhase 0.0, resyncBarOrigin
         = totalBarCount. totalBeatCount and totalBarCount do NOT change at the stop: a stop is never an edge. Held so
         until play. The tempo published is the tempo it will run at -- never 0, because bpm 0 already means "no
         tempo" to every reader (V13). What the NUMBER shows while stopped is question 126. Bit 3 is set.
 PLAY    from pause: the beat runs on from the held place; nothing is counted. From stop: that hop is beat 1 --
         totalBeatCount + 1 (ONE edge), beatInBar 0, the downbeat level true, the phase starts at 0. No bar is counted
         (NA-16).
 Everything that is not a beat field is published live throughout: onsets, levels, bands, the request sequence.
 Where it is done: inside the tracker (its phase accumulator IS the BPM timer), at FIVE gated sites, not the plan's
 four (NA-12), with a rule for the start hop (NA-13). With a nudge engaged the publish step holds the same values (PL
 steps 2b / 2c, with NA-14).

2. WHAT EACH BEAT-DRIVEN READER THEN DOES
 A QUANTISED CLIP FIRE is queued (V8) and drains only on a totalBeatCount edge (V9). Held timer: no edge: it WAITS.
   Play from stop: the one edge has beatInBar 0 and barCount 0, so Beat, Bar, 2 Bar and 4 Bar snaps all land on his
   press. Play from pause: the next beat (Beat) or the first beat of the next bar (Bar). Quantize Off: at once, as
   always. This is told to him as reading R81; the plan's draft question 129 is not asked (NA-10).
 A BPM-SYNCED CLIP never reads the beat today; it only takes a SPEED (V10). It HOLDS its frame while the timer is not
   running, through ONE seam at its advance (NA-1); its own play / pause setting is not written; on play it goes on from
   the held frame. A clip that is not in BPM Sync is not touched.
 A ROUTINE runs on a beat clock (V11). A RUNNING routine waits in place and goes on from where it stood, with no jump
   at play -- provided the recorder clock's change is two lines, not the plan's one (NA-15). A routine FIRED while the
   timer is held waits for a BAR LINE (V12): from pause the next one; from stop the first one after play, 4 beats after
   his press. That is the routine engine's own design (its lead-in ends on that line) and it is NOT changed (NA-16;
   reading R83). So the plan's sentence "every fire that waited lands on his press" is true of clips, not of routines.
 AUTOPILOT in beat mode counts totalBeatCount deltas (V9): it waits; play from stop counts one beat. Its end-of-clip
   mode is not beat-driven: a free clip still ends and advances; a held BPM clip never reaches its end, so it waits.
 ALSO: slideshow, randomize and MilkDrop playlist steps (count deltas) wait; the beat, bar and phrase phases in shaders,
   mappings, signals and connections stand still (on the "1" when stopped); the routine clock stands, and so does the
   beat a take is being RECORDED against. A take that is being REPLAYED is not beat-driven -- it follows its own audio or the
   wall clock (V31) -- so it plays on, untouched, as his rule for what is not BPM-based says.
   What follows the SOUND (onsets, loudness, bands) is untouched. Two honest exceptions, both said to Boris: the source
   "Structural Landscape" scales time by the tempo NUMBER and keeps drifting (V14; B-R4); a preset selector may step
   once at a stop pressed in the second half of a bar (SF-R4).

3. WHAT /2, x2, BPM MINUS AND BPM PLUS DO, AGAINST THE 60..200 FOLD
 TODAY, as facts (V4, V5, V6): every tempo REQUEST -- Tap, typed, REST, OSC, Link -- is folded into 60..200: halved
   while above 200, doubled while below 60. So wired as plain requests: x2 changes the tempo only from 60..100 (128 x 2
   = 256 folds back to 128); /2 only from 120..200; between 100 and 120 neither does anything; minus at 60 jumps to 118
   (59 doubles); plus at 200 jumps to 100.5. In Auto a request locks at once and the detector replaces it after about 2
   s of consistent estimates (200 hops), or keeps a half / double of it as "the locked octave".
 RULED (PL NR3 stands): a tempo set BY HAND IN THE ROW -- the typed number, minus, plus, /2, x2 -- is CLAMPED into
   30..400 and never folded; the detector, Tap, REST, OSC and Link keep the fold.
   /2    half the tempo; greyed below 60 (the result would be under 30).
   x2    double the tempo; greyed above 200 (the result would be over 400).
   minus the next whole number below; greyed at 30.   plus the next whole number above; greyed at 400.
   None of the four moves the "1". IN AUTO any of the five switches Manual ON and applies (the Manual toggle shows it:
   NA-17); back to Auto folds a tempo outside 60..200 on the first hop. UNDER LINK the five are greyed.
   The tempo they step from is not the 15 Hz label: it is the published tempo once the row's own last request has been
   applied, and that request's tempo until then (NA-2). The ceiling stays 400 (ST-8 PARTIAL; NA-11).

4. THE ROW'S WIDTH AT THE SIZE HE RUNS
 He runs 1728 wide, maximized (V15); the bar's inner width is 1720. COMPUTED from the named constants: the row --
 wheel, play, pause, stop, the BPM number, "-", "+", "<<", the nudge text, ">>", "/2", "x2", Tap, Resync -- is 498 px,
 from x = 326 to x = 824. Left of it: the Audio block 240, the routines stop 36, "Bar N" 46. Right of it, in this
 order: the LOCKED word 62, Manual 82, Link 56, Quantize 161 -- to x = 1185. Then the right block (Master Signal,
 Master, Outputs, FPS, DSP) 522. Sum 1703 of 1720: it fits with 17 px to spare (the plan said 31; "<<" / ">>" and
 "R[]" cost 14: NA-5). The 17 rests on two run-time widths that are ASSUMED (the Master Signal label 86, the nudge text
 box 88); it is now a unit bar with the real fonts (NA-6) and a number Harmony reads (FM-R1). At 1512: DSP, FPS, the
 "Quantize:" caption and the LOCKED word are shed. At 1280: all six sheddable items. The row, Manual, Link, the Quantize
 combo and the Outputs button are never shed.

WHAT CHANGES
 - The two proofs the plan's headline rested on could not fail. "BPM-synced clips hold" was guarded by a lint that one
   token anywhere in Renderer.cpp satisfies, by a live clause allowed to print INFO, and by no mutant; a BPM-synced
   VIDEO was never measured (GA-1, ST-1). The hold is now one pure function at the clip's advance, called at all three
   advance sites, pinned by a lint on the call lines, with unit cases, three mutants and a live clause that is PASS or
   FAIL for a sequence AND a video -- the position is already on a route of main, so no route is built (NA-1). "The
   buttons are the BPM timer" was proved only through a debug route that never touched a button (GA-2): there is now
   one entry in MainComponent for every row control, a widget case that clicks each control in-process, and from stage
   S3r on the debug route presses the real control and says so (NA-2).
 - The plan contradicted itself on Open (ST-2): ruled -- Open and New never touch the timer; only a launch starts it
   running (NA-8).
 - The step's base was unspecified and the 15 Hz label would have made a held "+" stutter and two quick presses
   collapse (ST-5): the base follows the request sequence (NA-2). LR4 could not run where it was scheduled (GA-3): it is
   two rows (NA-3). The timer's absence from a take gets a machine proof (GA-4: NA-4). No two buttons of the top bar
   read the same; the nudge text's tooltip says which way plus goes (GA-5, ST-7, ST-4: NA-5). The fit at 1728 is a
   unit bar (GA-6: NA-6). LR2's count is exact (GA-7: NA-7).
 - Two PARTIALs keep the plan's choice and change who decides: the nudge at a stop stays 0 by default but becomes
   question 129, because his words name only Resync (ST-3: NA-9); the ceiling stays 400 because 200 would grey x2 at
   every tempo he plays, and he is told what fast tempos do (ST-8: NA-11).
 - Six corrections no seat raised, found re-reading the tracker and the clocks: a fifth writer moves the bar count
   while paused in Auto (NA-12); in Auto an onset on the play hop turns "beat 1" into beat 2 at once (NA-13); a Resync
   while held, with a later nudge engaged, would publish a beat edge on a stopped timer (NA-14); the recorder clock
   needs two lines or it writes an anchor on every held tick and jumps a beat at play, and that clock is the routines'
   too (NA-15); routines are not clips at play (NA-16); the Manual toggle must follow the mode, or a pad's "Tempo x2"
   in Auto leaves it showing Auto (NA-17).
 - Questions: 125, 126, 127 and 128 as the plan drafted them (127 A names the new label); 129 is re-assigned to the
   nudge at a stop. None of the five has been shown to him yet. The open questions of the dispatch: 49 is answered in
   the record ("49 default", BD:868) and this lane keeps RN A2's two statements as its one line; 50's answer moves no
   line here (the nine row targets are ordinary bindings: "targetTempoRowOp" absent -> 0); 47 and 48 are not touched.
Nothing here loosens a pre-registered bar. LR2's tolerance {c, c + 1} is REPLACED before any run by an exact count
(tighter); LR3's INFO arm is REMOVED (tighter); LR4's "13 of 13" becomes LR4a "16 of 16" and LR4b "8 of 8".
What no run has established: everything. Every rule here is from reading 185147b. The cases of section 5 are RED first
for that reason, and NA-18 says what a builder does when one cannot be made green without a new rule: stop.

## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on, re-read or re-computed)
V1  VERIFIED M:src/analysis/AnalysisThread.cpp:192-214: per hop `feedSilenceDetection`, `process`, `feedDownbeatFeatures`
    (with the PREVIOUS hop's structural state, :198-201), then twelve tracker values into the snapshot: bpm,
    trackerState, trackerRequestSeq and the nine beat fields named in section 0.
V2  VERIFIED M:src/analysis/BPMTracker.cpp. One hop, in order: the request sequence is latched (:74); a pending tempo
    request is applied (:79-84); in Manual `updatePhase(false, 0)` and return (:99-104); in Auto the pipeline, then
    `updatePhase(beat, conf)` (:106-210). `updatePhase` (:213-247): free run, every whole beat crossed is counted
    (:226-232), a confident onset realigns (:235-236), a predicted wrap advances the bar's beat (:243-246).
    `realignPhaseToZero` (:253-258): + 1 when the phase is >= 0.5, then 0. Then `feedDownbeatFeatures` (:330-363):
    `scoreBeat()` when a beat was detected outside the predicted regime (:343-346), `updateBarPhase()`, `updatePhrase()`,
    and LAST a pending Resync (:357-362, `applyResync` :566-578).
V3  VERIFIED M:src/ui/TopBar.h:100-102, M:TopBar.cpp:33-39: three TextButtons ">", "||", "[]"; the "[]" tooltip is "Stop
    all routines". M:src/MainComponent.cpp:611-622: `onPlay` / `onPause` call `applyClipPlaying` for every shared
    layer's playing clip; :623-628 `onStop` calls `routineEngine_.stopAll()` and nothing else. M:tests/
    test_topbar_link_toggle.cpp:95-121 pins that tooltip and that one click is one `onStop`. The seats' and PL F3's
    citations hold.
V4  VERIFIED M:BPMTracker.h:41-42 (60, 200), M:BPMTracker.cpp:281-291 (`foldBPMToRange`), :607-622 (`applyTempoRequest`
    folds EVERY request at :614 and sets state LOCKED with the hysteresis counter full), :119 (the detector's raw
    estimate is folded too). PL F9 holds.
V5  COMPUTED from V4: 240 -> 120; 256 -> 128; 201 -> 100.5; 59 -> 118; 50 -> 100; 30 -> 60. PL F10 holds.
V6  VERIFIED M:BPMTracker.h:48 (200 hops), M:BPMTracker.cpp:147 and :293-309 (an estimate near half or double the locked
    tempo is "corrected" to the locked octave), :170-188 (a different median must persist 200 hops). PL F14 holds
    (its consequence, a sticky octave after Manual off, stays INFERRED).
V7  VERIFIED M:BPMTracker.cpp:504-558 (`updatePhrase`, run every hop): at :535-546, when the structural state changes
    to "drop" or leaves "breakdown" and the hop is NOT in the predicted regime, `barCount_ = 0`. In Auto with real
    onsets the regime is not predicted (:139, :209). This writer of a beat field is NOT among the plan's four sites
    (PL:201-207): paused in Auto, a detected drop would zero the published barCount and jump phrasePhase and the "Bar
    N" text (M:TopBar.cpp:525). Stopped, barCount is already 0. The only other writers of the nine fields are inside
    `updatePhase`, `realignPhaseToZero`, `advancePredictedBeat`, `scoreBeat` / `analyzeDownbeatPosition`, `applyResync`,
    `updateBarPhase` and `updatePhrase`'s bar edge (:516-523) -- each is covered by the plan's sites 1, 2 and 4 or is a
    pure function of fields that then do not move. (Grep of every assignment in the file; a randomised case now
    closes the list by test: T-G4c.)
V8  VERIFIED M:MainComponent.cpp:36-43 and :4848-4850: a fire is queued unless Quantize is Off or the tracker is not
    LOCKED. A hand tempo request sets LOCKED (V4), so in Manual a held timer still queues.
V9  VERIFIED M:src/model/Autopilot.cpp:74-89: pending triggers are processed only when `beatCrossings_.consume(
    snapshot.totalBeatCount)` is not 0; M:src/model/Layer.h:466-481: Beat fires on any beat; Bar when beatInBar == 0;
    2 Bar / 4 Bar when also barCount % 2 / % 4 == 0. :108-109: the beat mode adds the same delta to the clip's count.
V10 VERIFIED M:src/render/Renderer.cpp:1657-1667: a BPM-synced VIDEO's speed is `videoBeats / beatDivision` -- no
    tempo term; :1673 `player->advanceFrame(dt)` whatever the beat does. :1718-1735: a BPM-synced IMAGE SEQUENCE
    takes its fps from bpm, then :1734 `seq->advanceFrame(dt)`; :1738 the non-BPM sequence advance. Outside src/media
    these three are the ONLY calls of `advanceFrame(` (grep). Both players compute `currentTime_ += dt * speed *
    direction` (M:src/media/VideoPlayer.cpp:398-416; M:src/media/ImageSequence.cpp:114-148) and both serve a pending
    seek BEFORE that, so dt = 0 holds the clock and still lets a seek land. `VideoPlayer::advanceFrame` with dt 0
    still posts the wanted time and refreshes the draw stamp (:379-395). M:src/render/ClipTransportSync.h:44-53:
    `writeBack` compare-exchanges the PLAYER's play state into the clip's `playing` -- so stopping the player would
    write "not playing" into the show.
V11 VERIFIED M:src/recording/RoutineEngine.h:172 and M:RoutineEngine.cpp:532: the routine clock is a `RecorderClock`
    ticked every tick from app start; the take clock is the same class (M:src/recording/RecorderClock.cpp:11-87).
V12 VERIFIED M:RoutineEngine.cpp:228-239 (`dueNow`: Beat = a beat edge, Bar = a bar edge), :542-543 (the bar edge is a
    change of totalBarCount, the beat edge a change of totalBeatCount), :250-263 and :478 (the boundary a fired routine
    aims at is clock beat + the beats to the next bar line -- 4 when the beat stands on the "1"), :265-270 (its
    restore glides over the one beat that ENDS on that boundary). M:MainComponent.cpp:4279-4281 and :6248-6254: the two
    sites that pass "tracker LOCKED and bpm > 0".
V13 VERIFIED M:TopBar.cpp:332-336 ("---" at bpm 0), M:RecorderClock.cpp:32-33 (unmetered at bpm <= 0), M:Renderer.cpp:
    1661 and :1722 (at bpm 0 a BPM-synced clip keeps its own speed). PL's fork (c) is rightly rejected.
V14 VERIFIED M:src/render/EmbeddedShaders.h:10755-10756 and :10793: the one shader that scales `u_time` by `u_bpm / 120`
    is the procedural source "Structural Landscape". It has no beat position.
V15 VERIFIED M:TopBar.cpp:529-631 (`resized()`; inner width = width - 8 at :531; the Master Signal label is measured from
    its text at :628-630). COMPUTED: today's left block 1031 / 1095 px, right block 436 + the label (PL F1, F2 hold).
    VERIFIED M:tests/test_master_signal_link.cpp:15-17 and :289-290 (1728 is "this app's real maximized-window width"),
    :352-354 (the bar does not fit at 1280 today), :318-320 (the headless harness measures a label's text width);
    M:src/Main.cpp:53 (minimum 1280 x 720). The new sums are in section 0 item 4 and NA-6.
V16 VERIFIED PL:574-576 against PL:275-276: T-G12 is "every file ... is either on the whitelist ... or contains
    `beatRunning(`"; the rule it claims is "in the same function". PL:609-614: no mutant removes a clip gate. PL:636-
    638: clause (c) may print INFO. M:src/api/ApiServer.cpp:498-509: `GET /api/composition` already publishes every
    clip's "playheadPosition" and "playing"; `writeBack` stores the playhead every frame (M:ClipTransportSync.h:47-48).
    So a position route EXISTS on main for a video and for a sequence.
V17 VERIFIED M:MainComponent.cpp:5764-5817: `applyTempoCommand` posts requests, then captures a "tempo" point unless
    the origin is Replay; :1980 a replayed point re-applies it. Its callers are :588, :597, :601, :1896, :1900, :1980,
    :2354, :2358, :4355, :7841, :7849 (grep) -- none is in a load or New path.
V18 VERIFIED from V17's caller list and M:TopBar.cpp:9-290: today Open and New change neither the tempo, the mode nor
    the phase.
V19 VERIFIED M:TopBar.cpp:289 (15 Hz), :297-305 (the timer copies bpm and five beat values into `displaySnap_`). PL:338
    says "base = the published tempo" and names no source. M:BPMTracker.cpp:74 and :592-605, M:BPMTracker.h:173: a
    posted request raises the sequence after its write; the hop latches it before reading; `postedRequestSeq()` is
    readable from any thread (Pitfall 48).
V20 VERIFIED M:EmbeddedShaders.h:2221-2226: the Strobe effect's own rate is "1.0 + u_strobe_rate * 15.0; // 1 to 16 Hz".
    So the app already flashes at up to 16 Hz with no tempo involved. 68 lines of that file name `u_beatPhase`; they
    were NOT each read.
V21 VERIFIED M:RecorderClock.cpp:32-33 and :82: `wasUnmetered` is `lastBpm_ <= 0` and `lastBpm_` is set from snap.bpm
    every tick. While the timer is held the tempo stays positive, so with the plan's ONE changed term (PL:297) every
    held tick is "newly unmetered": an "unmetered" anchor per tick (:38-39), no "lock" anchor on leaving (:45-52), and
    at play from stop the count's + 1 passes into the clock as a jump of one beat.
V22 VERIFIED PL:226-252 against RN A1 / A2 (RN:239-241, :293-296): RN's step 2 (RESTART) publishes a count clamped
    into [F, F + 1]. The plan's step 2b uses F's count exactly but is entered only on `stopApplied`; a Resync applied
    while the timer is held raises only `appliedResyncs()` (PL:229-231, T-G6 at PL:537-538). With a LATER nudge engaged
    the published count is one behind the tracker's for part of every beat, so that Resync would publish F + 1: a
    beat edge on a stopped timer, with beatInBar 0 -- a waiting quantised fire would land.
V23 VERIFIED M:BPMTracker.cpp:343-346 and :377-382: on a hop with a detected beat in Auto, `scoreBeat()` advances the
    bar's beat. A start from stop is applied at the START of a hop (PL:223-225); `scoreBeat()` runs later in the same
    hop. INFERRED: an onset on the play hop -- likely, since he presses play ON the kick -- leaves that hop published
    with the count + 1 and beatInBar 1. (A Resync avoids exactly this by being applied last: :354-356.)
V24 VERIFIED M:tests/test_topbar_link_toggle.cpp:33-39, :83-86, :117-120: the project's widget tests click a control
    by handing it a constructed MouseEvent (mouseDown + mouseUp) -- in-process, no OS input; :80-86 shows a disabled
    control does nothing under it. J:modules/juce_gui_basics/buttons/juce_Button.cpp:359-362 and :390-399:
    `triggerClick()` only posts a message, and the click is dropped when the button is not enabled.
V25 VERIFIED M:docs/claude/recording.md:110 (the routine cue is reserved for routines; "Stop (routines only)" is one of
    the ways a routine leaves); M:src/ui/LookAndFeel.h:36 (`kRoutineCue`); M:docs/claude/pitfalls.md:21 (Pitfall 6:
    ASCII glyphs "<", ">", "||" for small buttons). M:TopBar.cpp:545-550 and :589-598: "||" and "[]" sit in 24 px
    buttons, "/2" and "x2" in 26 px, today.
V26 VERIFIED M:MainComponent.cpp:7584-7588 and :7852-7862: the bindable "Play / Pause" runs the AUDIO FILE; "Stop" stops
    routines. PL F4 holds.
V27 VERIFIED M:src/sources/PresetSelector.cpp:79: a bar crossing is "barPhase < last and last > 0.5".
V28 VERIFIED RT:404-409 (the transport lane's `transportSpeed(...)` with `TempoView { bpm, showBarBeats, clockLocked }`
    replaces both `setSpeed(` lines of `syncMedia`), RT:426-429 (its lint pins `setSpeed(`), RT:492-498 and RT:768-769
    (S4t edits BPMTracker.cpp; "whichever lane builds it registers TL-U70 once").
V29 VERIFIED BD:910-920 and BL:632-650: his words on 61, 62, 63 and 111, as quoted in section 7. BD:855-862, BD:868 and
    boris-clarify-47-49.md:19-21: his words on Tap, Resync and "49 default"; question 49 as asked reads "A (default)
    0. Resync is a fresh start: the "1" is where you pressed, nothing on top." boris-clarify-111.md: readings R69..R73
    as told to him. None of his eleven screenshots was opened except resolume-transport-3.png, which is the clip
    panel; by their names none shows Resolume's tempo row (INFERRED).
V30 VERIFIED FR:70-80 and FR:160-176: Resolume documents Tap, Resync, plus / minus, Nudge (a TEMPORARY tempo change),
    x2 "to accentuate a musical climax", Pause; it documents no step size, no BPM "play" or "stop", and no range
    beyond one 2009 staff post (2 to 500). "Similar to resolume" is settled by HIS list.
V31 VERIFIED M:src/recording/RecorderHost.cpp:550-567: a REPLAYED take's position is the audio transport's sample
    position (with audio) or the wall time since play -- never the beat; :469: the RecorderClock is ticked while a take
    is RECORDED. So a held timer freezes the beat a recording take stamps (NA-15) and does not hold a replaying take. A
    replaying ROUTINE runs on the beat clock (V11) and does wait.

## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it)
| id | sev | verdict | decided by | amendment |
|---|---|---|---|---|
| GA-1 | MUST | ACCEPT | V16, V10: T-G12 is per file while its own rule says per function; both BPM branches are in Renderer.cpp; no clip mutant exists; clause (c) may print INFO. And the position route exists on main, so INFO was never needed. | NA-1 |
| GA-2 | MUST | ACCEPT | V3: the three widgets are bound to clip play / pause and `stopAll()` today; PL:497 and PL:423 disagree about what the route calls; no case clicks play, pause or stop. | NA-2 |
| GA-3 | SHOULD | PARTIAL | ACCEPTED: `bpm_edit` exists only from S3r (PL:501), so LR4 cannot run at S2r. On the guard the seat's arithmetic is wrong for double (an unguarded handler asks 480, the clamp gives 400, LR4 wants 240: it FAILS) and right in effect for half at the floor (15 clamps to 30, the guarded result). A half arm at 50 is added and the route answers "refused". | NA-2, NA-3 |
| GA-4 | SHOULD | ACCEPT | V17: tempo commands ARE captured; nothing gates `setBeatTimer`; T-R2 tests the clock only; B-R12 was the only check. | NA-4 |
| GA-5 | SHOULD | ACCEPT | PL:163-167 rejects two "-" "+" pairs and ships two ">"; distinct texts are machine-checkable. | NA-5 |
| GA-6 | SHOULD | ACCEPT | V15: the label is measured at run time; T-RB4 is fed 86; the headless harness can measure it. With NA-5 the spare is 17 px. | NA-6 |
| GA-7 | NIT | ACCEPT | V2: a realign from the second half adds 1, so MR2 lands inside {c, c + 1}. PL:427 "thirteen" against PL:582-584's fourteen. PL:656-659 counts 10 for 8 + 1 + 1 and leaves learn uncounted. | NA-7 |
| ST-1 | MUST | ACCEPT | as GA-1; V10: the video's speed has no tempo term and its advance never looks at the beat. | NA-1 |
| ST-2 | MUST | ACCEPT | PL:300-302 says both things. V18: no load or New path touches the tracker today. Ruled: Open and New never touch the timer. | NA-8 |
| ST-3 | SHOULD | PARTIAL | ACCEPTED: his zeroing words name Resync only (V29), so it is his to say -> question 129. REJECTED as the default: question 49 as asked reads "the "1" is where you pressed, nothing on top" and he took it; play from stop is that gesture; and a kept nudge of a beat or more makes play show two beats at once (RN A1 step 5). The seat's "gone with no text" is wrong: the row shows "nudge 0 ms". | NA-9 |
| ST-4 | SHOULD | ACCEPT | PL:169-170: the text's tooltip names no direction. One sentence is added; B-R9 asks whether back = later reads right. | NA-5 |
| ST-5 | SHOULD | ACCEPT | V19: the label is a 15 Hz copy and PL:338 names no source. The seat's expected numbers (256, 400, 200) assume a clamp the plan does not have; the arm is 100 -> 200 -> 400 -> 200. | NA-2 |
| ST-6 | SHOULD | ACCEPT | V8, V9; BD:859-862. A stays, is told as reading R81 and named in B-R5; B stays one line (PL:291-293). The plan's draft question 129 is not asked. | NA-10 |
| ST-7 | SHOULD | ACCEPT | as GA-5; V25. The routines stop reads "R[]" (the seat's "Rtn" was an example). | NA-5 |
| ST-8 | SHOULD | PARTIAL | REJECTED: a ceiling of 200 greys x2 above 100 BPM -- at every tempo he plays; x2 at a climax is the documented use (V30). ACCEPTED: the readers were not read. Read now: Strobe already flashes at 1 to 16 Hz by its own knob (V20), so 400 BPM (6.7 Hz) adds no new class. He is told (R77); the ceiling is one constant (HR-8). | NA-11 |
Reconciliation: GA-1 and ST-1 are one attack from two seats and get one fix. GA-5 and ST-7 agree. No two seats conflict.

## 3 AMENDMENTS (numbered; each OVERRIDES the plan body where they differ)
The real-time rules hold in every amendment: nothing touches the audio callback; the analysis thread gains branches
and plain members only (no allocation, no lock, no system call); the render thread reads one byte more of the snapshot
it already reads and never waits; no mutex is added; the two message-thread members of NA-2 are read and written on the
message thread only. No slider and no popup menu is added. No text announces an event or a failure: a refused step is
a greyed control, a held timer is a lit button.

NA-1  THE CLIP HOLD IS ONE SEAM, AT THE ADVANCE (GA-1, ST-1). REPLACES PL:264-271 (the first two bullets of "THE RATE
      READERS"), PL:275-276 and T-G12 (PL:574-576).
      - src/render/ClipTransportSync.h gains ONE pure function:
        `inline double syncDt(const Clip& clip, double dt, bool beatRunning) noexcept;`
        -- 0.0 when the clip is in BPM Sync and `beatRunning` is false; otherwise dt. No player, no bus.
      - `Renderer::syncMedia`: each of the three advances (M:Renderer.cpp:1673, :1734, :1738) reads
        `advanceFrame(ClipTransportSync::syncDt(*clip, dt, running))`. `running` is `beatRunning(snap.beatShiftState)`
        from the bus read the BPM branch already makes (:1660, :1721); a clip that is not in BPM Sync passes true and
        reads nothing more than today.
      - Why the dt and not the player's play state: V10 -- stopping the PLAYER would write "not playing" into the
        show through `writeBack`, and pause is the clip's own, saved setting (BD:904-906). With dt = 0 the player
        stays a playing player on a standing clock, and a seek still lands. INFERRED, not run: the shown frame stays
        and nothing goes "pending" (Pitfall 56). FM-R2 measures it; both outcomes are ruled there.
      - THE LINT (replaces T-G12; in tests/test_render_thread_lint.cpp; `//` comments stripped first): (a) outside
        src/media, `advanceFrame(` occurs on exactly three lines, all in Renderer.cpp, and each of those lines contains
        `ClipTransportSync::syncDt(`; (b) `syncDt(` is defined once, in ClipTransportSync.h. A token elsewhere in the
        file, or in a comment, cannot satisfy it. The `.bpm` whitelist of PL:575-576 is struck: the hold no longer
        hangs on who reads the tempo, so the transport lane's rewrite of the speed lines (V28) cannot un-gate a clip.
      - Unit cases T-G12a..d; mutants MR20, MR21, MR22. LR3's clause (c) becomes (c1) a sequence and (c2) a video,
        both PASS or FAIL, read from `GET /api/composition` (V16). No route is built.
      - "Structural Landscape" (V14) stays ungated and is said to Boris (B-R4): holding it needs a time uniform of
        its own, which is not this lane.
      - Harmony constraint (proposed, for the transport lane): its bar lock applies no correction while `beatRunning`
        is false, and it keeps the three `syncDt` call lines; whichever of the two lanes merges second re-applies its
        own edit on the other's lines as its builder's step 0 (HR-1).
NA-2  ONE ENTRY FOR THE ROW; THE BUTTONS ARE PROVED TO CALL IT; WHAT A STEP STEPS FROM (GA-2, GA-3, ST-5). REPLACES
      PL:303-304, PL:338 ("base = the published tempo"), PL:423-426 and T-RW4 (PL:602).
      - src/model/BeatTimer.h gains
        `enum class TempoRowOp : uint8_t { Play, Pause, Stop, BpmMinus, BpmPlus, Half, Double, NudgeBack, NudgeForward };`
        (the first seven, in this order, are the saved "targetTempoRowOp" 0..6) and two pure functions:
        `float handTempoBase(float publishedBpm, int32_t postsNotYetApplied, float lastPostedBpm) noexcept;`
        -- lastPostedBpm when postsNotYetApplied > 0; else publishedBpm when it is > 0; else 120.
        `bool nextHandTempo(TempoRowOp op, float base, float& out) noexcept;`
        -- false = refused (the result would leave 30..400; minus at 30; plus at 400). It is `canHalve`, `canDouble`,
        `bpmStepUp`, `bpmStepDown` behind one door.
      - `bool MainComponent::tempoRowOp(TempoRowOp op, Origin origin);` -- the ONE entry. Play / Pause / Stop ->
        `setBeatTimer`. The four tempo steps -> base = `handTempoBase(` the bus's bpm, `int32_t(lastHandPostSeq_ -`
        the bus's trackerRequestSeq`)`, `lastHandPostBpm_)`; then `nextHandTempo`; then `applyTempoCommand("manual",
        bpm, origin)`; then lastHandPostSeq_ = `tracker->postedRequestSeq()` and lastHandPostBpm_ = bpm (two plain
        members, message thread only; Pitfall 48's recipe, V19). The two nudges -> RN's `setBeatNudgeMs` with the
        number + 1 (forward) or - 1 (back). It returns false when refused (a range end; Link on, for the four tempo steps). The typed
        number goes through the same post-and-remember helper.
        Callers: TopBar's `std::function<void(TempoRowOp)> onTempoRowOp` (ONE callback for the nine controls; `onPlay`
        and `onPause` leave TopBar.h under question 127 A; `onStop` is renamed `onStopRoutines`); `handleBindingAction`'s
        TempoRow and BeatNudge cases; the debug route at S2r. `setBeatTimer(` has exactly one call site, inside
        `tempoRowOp` (lint T-G14).
      - THE DEBUG ROUTE SAYS HOW IT ACTED. `POST /api/debug/tempo_row` {"op": "play" | "pause" | "stop" | "bpm_minus" |
        "bpm_plus" | "half" | "double" | "nudge_back" | "nudge_forward"} answers {"ok", "via": "handler" | "button",
        "clicked": true | false, "refused": "" | "range" | "disabled" | "link"}. At S2r it calls `tempoRowOp` ("via":
        "handler"). From S3r on it presses the row's OWN control through
        `bool TopBar::pressRowControlForTest(const juce::String& rowName);`
        (false when the control is disabled or not showing; else the control's own `onClick`) and answers "via":
        "button". Its "clicked" / "refused" answer is what `tempoRowOp` recorded for that press (one test-server
        member), so a press refused at a range end reads "range" whether or not the 15 Hz display had already greyed
        the control. Every live row states the "via" it expects. LR1, LR2, LR3 and LR4a are run at S2r (handler) and
        again after S3r (button). It is never synthetic input.
      - THE WIDGET CASE T-RW5 (S3r; the harness of V24, in-process, no OS input): one left click on each of play,
        pause, stop, "-", "+", "<<", ">>", "/2", "x2" calls `onTempoRowOp` exactly once with that control's own op and
        calls nothing else; one click on Resync calls `onResync` once; two on Tap call `onTapTempo` once; one on the
        routines stop calls `onStopRoutines` once; a click on a control disabled at a range end calls nothing.
        Mutants MR23 (TopBar: play's click calls the routines-stop callback) and MR24 (app: MainComponent's
        `onTempoRowOp` ignores Play: RED on LR5 (g)).
      - A held "-" or "+" (400 ms, then every 50 ms: RN A15's constants) therefore steps once per repeat, and two
        quick x2 are two doublings: T-G13, LR4a step 13. Mutants MR26 (`nextHandTempo` never refuses), MR28 (the base
        is always the published tempo).
NA-3  LR4 IS TWO ROWS (GA-3). REPLACES PL:641-648 and PL:497's "(LR4's UI clauses wait for S3r)". LR4a (after S2r: the
      steps, the refusals, the no-wait triple; Manual is set by `POST /api/set_bpm`, which exists) and LR4b (after
      S3r: the typed number, the greyed ends, Auto, back to Auto). Section 5.
NA-4  THE TIMER IS NOT IN A TAKE, AND A MACHINE PROVES IT (GA-4). ADDS to PL:294-299. Lint T-G14 (the body of
      `MainComponent::setBeatTimer` holds no `capture(`); live row LR7 (two takes compared, then a replay); mutant
      MR25. B-R12 becomes a one-line confirmation. A ROUTINE that is playing waits with a held timer (its clock is
      the beat: NA-15); a replaying TAKE does not -- it follows its own audio or the wall clock (V31) and is not
      BPM-based. Both are said in R75.
NA-5  NO TWO BUTTONS READ THE SAME; THE TEXT'S TOOLTIP SAYS THE DIRECTION (GA-5, ST-7, ST-4). REPLACES PL:163-170 and
      the label in PL:186-189.
      - Nudge back reads "<<", nudge forward ">>" (24 px each, as "||" and "[]" today: V25). Play stays ">", pause
        "||", stop "[]". The routines stop -- outside the row, under question 127 A -- reads "R[]" (30 px) in
        `kRoutineCue`; its tooltip "Stop all routines" is unchanged.
      - Tooltips, exact: "<<" = "Move the beat 1 ms later. Hold to keep moving."; ">>" = "Move the beat 1 ms earlier.
        Hold to keep moving."; the nudge text = "How far the beat is moved from the detected or tapped beat. Plus
        moves the beat earlier, minus later. Click to type a number." With the sign constant true the words "earlier"
        and "later" swap in all three (T-U5's rule).
      - `rowTexts()` in TopBarModel.h is the one source of the button texts. T-RB3b: the texts of the row's eleven
        buttons and of the routines stop are pairwise distinct. Mutant MR32.
      - VG, pre-registered MUST: two buttons of the top bar with the same text; a button text wider than its box.
      - Cost: + 14 px (section 0 item 4). The key / pad labels stay "Nudge forward" / "Nudge back".
NA-6  THE FIT AT 1728 IS A UNIT BAR (GA-6). ADDS to T-RW1 (PL:594-596) and to PL RR6.
      - T-RW1 gains: with the app's LookAndFeel as the default (as M:tests/test_topbar_link_toggle.cpp:47-48) and the
        bar at 1728 x 40: the shed list is EMPTY; no row button's text, measured with the font the LookAndFeel
        gives that button, is wider than its box (INFO prints each width against its box; main's own buttons were never
        measured, so no tighter bar is set here -- the visual gate judges a squeezed text); INFO prints "margin <n> px" (1720 minus the sum of the
        kept widths). At 1512 and at 1280 the bar's shed list equals `fitTopBar(inner, the MEASURED label width).shed`.
      - If the 1728 clause is RED the builder STOPS and reports the margin. Remedies in this order, each one constant,
        no new ruling needed: (1) the nudge text box takes its measured width + 8, with no 88 floor; (2) the gap after
        Resync 6 -> 2 and the gap after "x2" 4 -> 2. Still short: HR-10. Never: a row control narrower than its text.
      - VG-0's manifest records the Master Signal label's width on today's bar (FM-R1).
NA-7  LR2'S COUNT IS EXACT; THE COUNTS ARE RESTATED (GA-7). LR2 (section 5): the stop is posted at a chosen phase, the
      count must equal c exactly, and MR2 is a second RED arm. PL:427 reads "the fourteen names". LR6's total is an
      enumerated list.
NA-8  OPEN AND NEW NEVER TOUCH THE TIMER; A LAUNCH STARTS IT RUNNING (ST-2). REPLACES the second sentence of PL:300-302.
      The timer's state lives in the tracker and in the published byte; it is not in the show and not in settings.
      Every LAUNCH starts Running. Open and New neither read it nor write it: a stopped timer stays stopped (the lit
      stop shows it), a paused one paused. Reasons: the timer is the app's clock, as the tempo and Manual are, and
      neither of those is touched by a load today (V18); and "stop, open the next show, play on the 1" is a gesture
      he should have. The opened show's nudge amount still takes over (his words: "good (this is just nudge amount)")
      and glides while the timer is held (PL step 2c).
      Proof: lint T-G14 (one call site of `setBeatTimer(`); LR2 (e); mutant MR33. Reading R82.
NA-9  THE NUDGE AT A STOP IS HIS TO SAY: QUESTION 129 (ST-3). REPLACES the first half of reading R74 (PL:726-727).
      Default A is the plan's: the two message-thread statements run in the Stop path (human origin only), so the
      nudge reads 0 from the stop. B: they are deleted and Stop never touches the number; steps 2b and 2c already
      carry a kept nudge, and T-N19 is then the live path. Either way ONE place changes: those two statements in
      `setBeatTimer`. Pause never touches the number (R74). Boris's check B-R3 is done at nudge 0 first.
NA-10 A QUANTISED FIRE WAITS: A READING, NOT A QUESTION (ST-6). The plan's draft question 129 (PL:723-725) is not
      asked. His words settle the default: a fired clip "is not affected unless it's set to be quantized" -- a
      quantised fire follows the beat, and the beat is stopped. It is told as reading R81 and checked in B-R5. B
      (fire at once while held) stays the one line of PL:291-293. The visual gate captures the waiting state (R9b)
      and the seats answer in words whether a waiting pad reads as "waiting", not "dead"; a cue is added only if a
      seat shows a mis-read.
NA-11 THE HAND RANGE STAYS 30..400 (ST-8). PL:324-327 stands. Added: V20; reading R77 tells him what fast tempos do;
      S5's manual says it in one sentence; `kHandMaxBPM` is one constant and HR-8 is Harmony's lever. Still NOT read:
      each of the 68 shader lines that take the beat phase.
NA-12 THE FIFTH SITE (own finding; V7). ADDS to PL:201-207. While the timer is not Running, `updatePhrase`'s
      structural reset (M:BPMTracker.cpp:539-541) does not zero `barCount_`. One condition, analysis thread. Cases
      T-G4b and T-G4c; mutant MR29. T-G4c is the answer to "did the reading miss a sixth writer": it does not
      enumerate writers, it drives the tracker with everything the detector can do and watches the nine fields.
NA-13 THE START HOP (own finding; V23). ADDS to PL:227-228. On the hop that applies a start from stop, `scoreBeat()`
      is not called: that hop always publishes beatInBar 0 and the level true, in Auto too. One bool, set by
      `startFromOne()`, cleared at the end of `feedDownbeatFeatures`. Case T-G3b; mutant MR30. What it does NOT
      promise: in Auto the detector may score the same kick a hop or two AFTER the press and move the bar's beat on
      -- a hand Resync has that exposure on main today. It is measured, not guessed (FM-R3).
NA-14 A RESYNC WHILE THE TIMER IS HELD IS A STOP FOR THE PUBLISH STEP (own finding; V22). ADDS to PL:204-205 and
      PL:235-240. In the tracker, `applyResync` while not Running runs `applyStop()` -- the "1" block, no count, state
      Stopped -- and raises BOTH `appliedResyncs()` and `appliedStops()`. BeatShift therefore takes step 2b on that
      hop (PL already ranks it above step 2): the "1" block with F's count EXACTLY, never RN's [F, F + 1]; a hand
      Resync's zero request is consumed there, so the nudge reads 0 as after any hand Resync. With
      `kGesturesStartTimer` true (128 B) the Resync runs `startFromOne()` instead and the hop is a start. Cases T-G6
      (amended), T-N17b; mutant MR27.
NA-15 THE RECORDER CLOCK: TWO LINES, AND IT IS THE ROUTINES' CLOCK TOO (own finding; V21, V11). REPLACES "one term in
      M:RecorderClock.cpp:33" (PL:296-298). `RecorderClock` keeps the LAST tick's verdict in a bool; `wasUnmetered` and
      `isUnmetered` both come from ONE predicate: the tempo is not positive, OR the timer is not running. Then a held
      timer gives what the plan wanted -- one "unmetered" anchor on entry, none while held, one "lock" anchor on
      leaving -- and the beat goes on from its held value: the + 1 of a start from stop is absorbed (M:RecorderClock.
      cpp:45-52), so neither a take being recorded nor a RUNNING ROUTINE jumps a beat at play. T-R2 (restated); mutants MR18, MR31;
      live clause LR2 (d).
NA-16 "EVERY FIRE THAT WAITED LANDS ON HIS PRESS" IS TRUE OF CLIPS, NOT OF ROUTINES; NO BAR IS COUNTED AT PLAY (own
      finding; V9, V12; closes PL RR3). REPLACES the last sentence of PL:193-197. A waiting CLIP lands on the play
      press. A waiting ROUTINE starts on the first bar line after play. `startFromOne()` counts a beat and NOT a bar,
      and that is now a rule, not a risk: a bar edge at play would start the routine four beats before the boundary
      its own restore glide was scheduled to end on (V12). Live clauses LR2 (c), (d). Reading R83; B-R5 says it. If
      he wants a routine on the press, that is a change of the routine engine's lead-in, for the architect -- not one
      statement in the tracker.
NA-17 THE MANUAL TOGGLE FOLLOWS THE MODE (own finding; needed by NA-2 and by S4r). ADDS to PL:349-352.
      `void TopBar::setManualShown(bool on);` -- sets the toggle (no notification), the mode flag and the LOCKED
      word's visibility, and only when the value differs (Pitfall 59). `applyTempoCommand` calls it: "manual" and
      "link" -> true, "auto" -> false. So a row step, a pad's "Tempo x2", REST `set_bpm`, OSC, Link and a replayed
      point all leave the toggle showing the mode the tracker is in (after a REST `set_bpm` it is stale today:
      FB T1, INFERRED there). Case T-RW6; live LR4b step 7.
NA-18 STAGES, MUTANT APPS, COUNTS, THE STOP RULE.
      - The stages keep the plan's keys and order; their scopes are section 4's.
      - Mutants MR20..MR34 are added (section 5). Mutant APPS, each a normal cmake build in its own directory under
        RN A10's rules: build-mut-gate (MR1 + MR20), build-mut-edge (MR2 + MR3 + MR25 + MR26 + MR28), build-mut-fold
        (MR7 + MR12 + MR18) at S2r; build-mut-row (MR24 + MR33) at S3r. Inside each app the mutants fail DISJOINT
        clauses (section 5 names them). The plan's pairing "MR1 + MR3" is struck: with MR1 the beat moves while
        stopped and LR2's clauses cannot be told apart.
      - G-N1's N: as RN A22 -- each stage report declares its new ctest entries; Harmony re-counts with `ctest -N`.
      - RN A24 holds for every case of section 5 that states a guarantee of PL NR2 / NR3 or of NA-12..NA-15: if it
        cannot be made green without a rule that is not written here, the builder STOPS and reports the hop sequence.

## 4 FINAL STAGES + ORDER (one builder context per stage; Harmony runs every live row, never a builder)
One worktree and branch for the whole nudge lane (RN section 4); no two builders in it at once. A builder builds, runs
unit tests, writes probes and their self-tests; he never runs a live row and never gives a gate verdict. REPLACES PL
section 4's table; PL:506-513 (order, reviews, the real-time lens, what two lanes must not both touch) stands with HR-1.
| key | who | scope and the files it owns | needs | exit, and what it proves |
|---|---|---|---|---|
| G-N0 | Harmony | as RN | -- | as RN (unchanged) |
| S1 | builder | as RN, with `kNudgePlusMeansLater = false` and PL NR5-A's literals | this ruling adopted | as RN |
| S1r | builder | THE TIMER AND THE HAND TEMPO, unwired. src/model/BeatTimer.h (new, no juce: `BeatRun`, the state bits, `beatRunning`, the command word's pack / unpack, kHandMinBPM / kHandMaxBPM, `canHalve`, `canDouble`, `bpmStepUp`, `bpmStepDown`, `TempoRowOp`, `handTempoBase`, `nextHandTempo`, kGesturesStartTimer, kNudgeAfterStop); src/analysis/BPMTracker.h/.cpp (`requestRun`, the queue word, the FIVE gated sites, `applyStop`, `startFromOne` with the start-hop flag, a Resync while held = `applyStop`, the three views, `setHandTempo` + `kTempoExact`, the Auto re-fold); src/analysis/BeatShift.h/.cpp (the three Flags; steps 0 / 2b / 2c); tests/test_beat_timer.cpp (new: T-G1..T-G11, T-G3b, T-G4b, T-G4c, T-G13); tests/test_beat_shift.cpp (T-N17, T-N17b, T-N18..T-N20, T-N7b); CMake lines for the new test only. | S1 green; Harmony's order against the transport lane's S4t (HR-1) | The cases RED on a tree where `requestRun` and `setHandTempo` do nothing, then GREEN; MR1-MR12 and MR26-MR30 each RED; the golden still green (the gate is 0 at rest). Proves the timer, the range and the base rule on a REAL tracker. |
| S2 | builder | as RN | S1, G-N0 | as RN; then Harmony: G-N1, L1-L4, L6, L9, L10 with PL NR5-A's values |
| S2r | builder | WIRING. src/analysis/AnalysisThread.cpp (the three Flags; bits 2, 3 into the state byte); src/render/ClipTransportSync.h (`syncDt`) and src/render/Renderer.cpp (the three advance lines -- nothing else of `syncMedia`); src/MainComponent.h/.cpp (`tempoRowOp`, `setBeatTimer`, the post-and-remember helper, the "manual" branch -> `setHandTempo`, the debug callbacks); src/recording/RecorderClock.h/.cpp (NA-15) + its test (T-R2); src/api/ApiServer.h/.cpp ("beatTimer" in features; `POST /api/debug/tempo_row`, via "handler"); tests/test_clip_transport_sync.cpp (T-G12a..d); tests/test_render_thread_lint.cpp (the T-G12 lint, T-G14); .harmony/probe-nudge-row.sh / .py / -selftest.py (LR1, LR2, LR3, LR4a, LR7; it sources probe-quit-ours.sh, takes the live lock, quits only its own pid, deletes the takes it made); the fixture shows and media the rows name; the build scripts of build-mut-gate, build-mut-edge, build-mut-fold. | S1r, S2's rows green | Unit green; MR18, MR20-MR22, MR25, MR31, MR33 RED; the probe self-test prints "0 case(s) differ". Then Harmony: G-N1, LR1, LR2 (a)-(d), LR3, LR4a, LR7, L10's lines; and `.harmony/probe-video.sh` and `.harmony/probe-seq-vram.sh` still GREEN on the lane (FM-R2). Proves the wired app: one stopped beat for every reader, clips included; the tempo steps move the real tempo; a take never holds the timer. |
| S3a | builder (short) | as RN (hooks only, no top-bar change), incl. the Manual-toggle route if none exists | S2r's rows green | as RN |
| VG-0 | capture builder | as RN, plus V18b (1512 wide); the manifest carries the Master Signal label's width | S3a | the manifest: PL F2's arithmetic against pixels; the size he runs; FM-R1 |
| S3m | builder (short) | THE ROW'S MODEL, no widget. src/ui/TopBarModel.h (`nudgeText`, `parseNudge`, `nudgeTextChanged`, `bpmText`, `parseBpm`, the width constants, `rowOrder()`, `rowTexts()`, `fitTopBar`, the tooltip strings); tests/test_topbar_model.cpp (T-U1, T-U2, T-U4, T-U5 as amended; T-RB1..T-RB5, T-RB3b). | VG-0 | RED on stubs, then GREEN; MR13-MR15, MR32, M12, M13 RED. Proves every string and the layout arithmetic before a pixel moves. |
| S3r | builder | THE ROW. src/ui/TopBar.h/.cpp (the cluster from the routines stop to Quantize in `resized()`; the right block laid by `fitTopBar`; `onTempoRowOp`; the three timer buttons lit from the published byte; the BPM label and its editor; "-" "+"; "<<" the text ">>"; "/2" "x2"; `setManualShown`; `pressRowControlForTest`; the five-button code, `handleMultiplierButton`, the BPM field, `onPlay` and `onPause` removed; `updateBpmDisplay` rewritten; every timer-driven set only when what it would paint differs); src/model/Composition.h (bpmMultiplier: the member, its reset and its save line removed; the load line reads and drops the key) + tests/test_composition.cpp (T-C4); src/MainComponent.cpp (the callback assignments beside :587-628; `applyTempoCommand`'s `setManualShown` call; `onNudgeEditEnded`, `onBpmEditEnded`); src/api/ApiServer (`nudge_edit`, `bpm_edit`, `tempo_row` via "button", ui_text "tempo_row" and "nudge"); tests/test_topbar_row.cpp (new: T-U6, T-RB6, T-RW1..T-RW3, T-RW5, T-RW6); the existing top-bar tests kept green (test_master_signal_link.cpp; test_topbar_link_toggle.cpp -- its Link bounds and its routines-stop case re-pinned to the new layout and to "R[]", per question 127's answer); the build script of build-mut-row. | S3m | Unit green; MR19, MR23, MR34 RED. Then Harmony: G-N1, LR4b, LR5, and LR1 / LR2 whole (with (e)) / LR3 / LR4a again, now via "button". Proves the buttons ARE the timer and the steps. |
| S4 | builder | as RN, labels "Nudge forward" / "Nudge back" | S3r | as RN; then Harmony: L7, L8, LR5 |
| S4r | builder (short) | THE SEVEN TARGETS, as PL:503, with the handler case calling `tempoRowOp(op, Origin::Human)`. Same worktree, after S4. | S4's rows green; HR-2 | Unit green; MR16, MR17 RED. Then Harmony: LR6. |
| VG | capture builder, five critic seats; Harmony's verdict | R1-R20 and R9b against V18, V18b, V19, with the manifest of PL NR4 | S4r's row green | Row VG. Boris sees nothing before it is green. |
| S5 | builder | DOCS + MANUAL as PL:505, with: the pitfall's sentence on rate readers replaced by "a BPM-synced clip's advance takes its dt from `ClipTransportSync::syncDt`; the timer is gated at five sites; a Resync while held is a stop; Open and New never touch it; the recorder clock treats a held timer as unmetered"; one manual sentence on fast tempos (R77); recording.md: a routine fired while stopped starts on the first bar line after play. | VG green | Harmony reads the manual against the built app. |
Order: G-N0 -> S1 -> S1r -> S2 -> S2r -> Harmony's rows -> S3a -> VG-0 -> S3m -> S3r -> Harmony's rows -> S4 -> S4r ->
Harmony's rows -> VG -> S5 -> merge by RIG-RULES' merge sequence. S1r may instead follow S2 if Harmony wants the nudge's
own rows green first; it must precede S2r.
WHAT HARMONY RUNS HERSELF, AND WHEN: G-N0 before S1 / at S1's step 0 (RN). G-N1 after every builder stage. After S2r:
LR1, LR2 (a)-(d), LR3, LR4a, LR7 (via "handler"), the three mutant apps of S2r, the two existing media probes. After
S3a: VG-0. After S3r: LR4b, LR5, LR1-LR3 and LR4a again (via "button"), LR2 (e), build-mut-row. After S4: L7, L8 and LR5 (a) on L7's sets. After S4r:
LR6. Then VG (the capture builder, five critic seats, her verdict), then S5's manual read against the built app. Every
live batch: the lane's test-server build, the real app, `open -g`, the live lock, own-pid quit; no Output window, no
full-screen capture, no synthetic input; never while Boris's own Audio-DNA runs.
NOT IN THIS LANE: PL section 9 stands, and with it: a time uniform for "Structural Landscape"; a routine that starts on
the play press (NA-16); PresetSelector's wrap test (SF-R4); holding the detector's scoring after a start or a Resync in
Auto (FM-R3); another hand ceiling (HR-8); un-folding Tap / REST / OSC / Link; reading the 68 beat-phase shader lines;
re-wording the routines' surfaces beyond the one button's label and colour. From the stopped sync-dial branches nothing
new is carried (PL:797-803 stands; the two worktrees were pin-checked, not searched).

## 5 TESTS + GATE ROWS (pre-registered; a bar is met or reported, never loosened; each has its RED arm)
RN section 5 stands with PL NR5-A's replacements. PL section 5's new cases stand EXCEPT where this section restates
them: T-G6, T-G12, T-R2, T-RB4, T-RW1, T-RW4 (struck), LR1..LR6. Harmony copies gate strings ONLY from here and from RN.
UNIT, tests/test_beat_timer.cpp (S1r; a REAL BPMTracker driven hop by hop, 512 samples at 48 kHz). RED arm for the
 file: `requestRun` and `setHandTempo` stubbed to do nothing.
 T-G1, T-G2, T-G3, T-G4, T-G5, T-G7..T-G11: as PL:519-552.
 T-G3b THE START HOP IN AUTO (NA-13). Auto, the downbeat locked, Stopped. Play is posted; on the applying hop arm 1
       feeds a confident beat (beat true, confidence 0.9), arm 2 none. In both, that hop reads totalBeatCount + 1,
       beatInBar 0, the level true. MR30 RED (arm 1 reads beatInBar 1).
 T-G4b PAUSED UNDER A DROP (NA-12). Auto, the downbeat locked, barCount >= 3. Pause; then 400 hops of confident
       onsets during which the structural state goes 0 -> 2, then 3 -> 0: barCount, phrasePhase and the other seven
       never change. The same while Stopped. MR29 RED.
 T-G4c HELD UNDER EVERYTHING THE DETECTOR CAN DO (NA-12). A fixed-seed sequence of 20,000 hops -- raw tempos 0 and
       60..200, confidences 0..1, beat flags, structural states 0..3, silence in and out -- once Paused (from a locked,
       mid-bar state), once Stopped: the nine accessors never change; `bpm()` or `trackerState()` changes at least
       once (the detector lives). MR1, MR4, MR29 RED.
 T-G6  RESYNC WHILE HELD (PL:537-538, amended by NA-14). Paused at beatInBar 2, phase 0.7; `requestResync()`: the "1"
       block of T-G2, the count unchanged, the state Stopped, `appliedResyncs()` + 1 AND `appliedStops()` + 1.
       Stopped: the same; only the two counters change.
 T-G13 THE BASE AND THE DOOR (NA-2). `handTempoBase(128, 0, 999)` == 128; `(128, 1, 256)` == 256; `(0, 0, 999)` ==
       120. With NO snapshot progress between presses: from 100, Double, Double, Half give 200, 400, 200; twenty
       BpmPlus from 120 give 140. `nextHandTempo`: Double at 240 -> false; Half at 50 -> false; Half at 60 -> 30;
       BpmMinus at 30 -> false; BpmPlus at 400 -> false; BpmPlus at 399.5 -> 400. MR26, MR28 RED.
UNIT, tests/test_beat_shift.cpp additions (S1r): T-N17, T-N18, T-N19, T-N20, T-N7b as PL:554-569 (T-N17's zero-request
 arm is question 129 A's path; T-N19 is B's).
 T-N17b A RESYNC WHILE HELD, THE NUDGE ENGAGED (NA-14). Manual 120; D = -250 applied; Paused when the tracker's phase
       is 0.2. PRECONDITION asserted first: the published count is one behind the tracker's. Then a hand Resync (the
       zero request and the tracker's Resync on one hop). On that hop: the "1" block; totalBeatCount' == F's EXACTLY;
       the applied value 0.0; not held; adoption returned. A second arm: D = +250 at phase 0.7. MR27 RED (arm 1
       shows an edge).
UNIT, tests/test_clip_transport_sync.cpp additions (S2r; the file's fake player). RED arm: a stub `syncDt` returning dt.
 T-G12a BPM Sync, the beat not running: `syncDt` == 0.0.   T-G12b BPM Sync, running: == dt.
 T-G12c not in BPM Sync, the beat not running: == dt. MR22 RED.
 T-G12d the fake player synced 100 times (pushIntent, `advanceFrame(syncDt(...))`, writeBack) with the beat not
       running: its playhead does not move and the clip's `playing` is still true; then running: it moves on from
       the held place.
LINT, tests/test_render_thread_lint.cpp additions (S2r):
 T-G12 NA-1's lint, clauses (a) and (b). RED arm: the tree before S2r (three bare advances); MR20, MR21.
 T-G14 the body of `MainComponent::setBeatTimer` holds no `capture(`; `setBeatTimer(` is called on exactly one line of
       src, inside `tempoRowOp`; `requestRun(` occurs once in src outside BPMTracker's own files. MR25, MR33 RED.
UNIT, the recorder clock's test (S2r):
 T-R2  (PL:571-573, restated by NA-15). bpm 120 throughout. The state byte goes Running -> Stopped for 240 ticks ->
       Running, and the first running tick carries the count + 1 with phase 0 (a start from stop): exactly ONE
       "unmetered" anchor, on entry; NONE on the 239 ticks after it; ONE "lock" anchor on leaving; `beat` does not
       advance while held and on the first running tick is within one tick's advance of the held value (no + 1).
       The same through Paused. With the byte 0 throughout the anchor list equals today's. MR18 RED (no anchor; the
       beat jumps by 1). MR31 RED (an anchor on every held tick).
MODEL, tests/test_topbar_model.cpp (S3m): T-RB1, T-RB2, T-RB3 as PL:578-584 (fourteen names).
 T-RB3b `rowTexts()`: play ">", pause "||", stop "[]", "-", "+", "<<", ">>", "/2", "x2", "Tap", "Resync", and the
       routines stop "R[]": pairwise distinct; every character ASCII. MR32 RED.
 T-RB4 `fitTopBar` with a signal label of 86: inner 1720 -> nothing shed, 17 px left; 1504 -> DSP, FPS, the caption,
       the state word; 1272 -> all six. With a label of 104 at inner 1720: DSP is shed. For every inner width from
       1272 to 2200: the kept widths sum to <= the width; the shed list is a prefix of DSP, FPS, caption, state word,
       Master Signal, Master; the row, Manual, Link, the combo and Outputs are never in it.
 T-RB5 the tooltip strings of PL NR4 with NA-5's three, exactly; every character ASCII.
WIDGET, tests/test_topbar_row.cpp (S3r; Pitfall 34): T-U6, T-RB6, T-RW2, T-RW3 as PL:591-601.
 T-RW1 PL:594-596, plus NA-6's clauses: at 1728 x 40 with the app's LookAndFeel the shed list is empty, no row
       button's text is wider than its box, and "margin <n> px" is printed; at 1512 and 1280 the shed list equals
       `fitTopBar`'s for the measured label. MR34 RED (the nudge text box 200 px wide).
 T-RW5 NA-2's widget case: nine row controls, each its own op once; Resync once; Tap once for two clicks; the
       routines stop once; a control disabled at a range end: nothing. MR23 RED.
 T-RW6 `setManualShown(true)` turns the toggle on, hides the LOCKED word and does NOT call `onManualBpmChanged`; a
       second call with the same value requests no repaint. RED arm: a stub that does nothing.
 T-C4, T-B7..T-B10: as PL:603-608.
MUTANTS (new; PL's MR1-MR19 and RN's M1-M28 stand): MR20 the video advance takes the raw dt. MR21 a sequence advance
 takes the raw dt. MR22 `syncDt` ignores the transport mode. MR23 (TopBar) play's click calls the routines-stop
 callback. MR24 (app) MainComponent's `onTempoRowOp` ignores Play. MR25 (app) `setBeatTimer` captures a point. MR26
 `nextHandTempo` never refuses. MR27 a Resync while held is taken by step 2. MR28 the base is always the published
 tempo. MR29 the structural reset not gated. MR30 `scoreBeat` runs on the start hop. MR31 `wasUnmetered` left on
 `lastBpm_`. MR32 nudge forward's text is ">". MR33 (app) the Open path calls `setBeatTimer` with Running. MR34 the
 nudge text box is 200 px wide.

GATE ROWS (Harmony; strings exact). The rig is RN's: the lane's test-server build, the real app (not test mode), the
120 BPM click file, `open -g`, the live lock, the probe quits only the pid it launched; no Output window, no
full-screen capture, no synthetic input. "Applied" = the first /api/features poll (every 5 ms) whose trackerRequestSeq
has reached the posted value (Pitfall 48). Every `tempo_row` answer is checked for the "via" the run expects: "handler"
after S2r, "button" after S3r; a wrong "via" is "INVALID: via" and blocks. A FAIL is a FAIL; "flake" needs >= 5 runs.
LR1  PAUSE HOLDS, PLAY RUNS ON. Manual 120. ARM 0 (no nudge was ever set): `tempo_row` pause; from the applied poll,
     400 polls: the nine beat fields identical in all, "beatTimer": "paused", bpm 120. Then play: over the next 8 s the
     line (probe time, totalBeatCount + beatPhase) has a slope of 2.00 beats/s within 1 %, and its value at the
     applied poll is within 0.06 beat of the held value. Three rounds at pseudo-random times (fixed seed). ARM N (the
     nudge engaged): nudge -40 set and waited for; pause; 200 polls identical; while paused the nudge is set to +100:
     200 more polls, the nine fields still identical, and beatNudgeAppliedMs reaches 100.0; play: totalBeatCount never
     falls over 8 s. "PASS  LR1 pause holds and play runs on: 3 of 3 rounds, 1200 of 1200 polls held; nudged arm 400
     of 400". RED arms: build-mut-gate (MR1): "FAIL  LR1: beat moved while paused"; build-mut-fold (MR12): "FAIL  LR1
     arm N: beat moved while paused".
LR2  STOP IS THE 1 AND NOT AN EDGE; PLAY IS THE EDGE; A WAITING CLIP LANDS ON IT; A ROUTINE WAITS FOR ITS BAR; OPEN
     LEAVES IT STOPPED. Manual 120, Quantize "Next Downbeat", the fixture show (a clip to fire; one saved routine, bar
     snap, looping) and a second show file whose "beatNudgeMs" is 37. Rounds 1-3 run before any nudge is set in
     this launch (the publish step is not engaged: the published count is the tracker's own); rounds 4-5 set the nudge
     to +40 first and wait for it.
     Arm S posts `tempo_row` stop on the first poll whose beatPhase is in 0.65..0.85, arm F in 0.10..0.30; c = that
     poll's totalBeatCount.
     (a) STOP. From the applied poll, 400 polls: beatPhase == 0.0, beatInBar == 0, barCount == 0, downbeatDetected
         true, totalBeatCount == c EXACTLY, all nine fields constant, "beatTimer": "stopped"; `/api/debug/beat_nudge`
         "ms" == 0 (in rounds 4-5 that is question 129 A; under B: 40).
     (b) A CLIP WAITS. `POST /api/trigger_clip`: for 1.0 s the layer's playing clip is not the new one (reading R81).
     (c) A ROUTINE WAITS. `POST /api/routine/fire`: for 1.0 s `GET /api/routine/status` shows it waiting
         (its slot's "state") and "clockBeat" (M:MainComponent.cpp:6383) does not change.
     (d) PLAY. `tempo_row` play: in the applied poll totalBeatCount == c + 1, beatInBar == 0, downbeatDetected true,
         totalBarCount unchanged; the new clip is the playing one within 150 ms; the routine is NOT running 1.5 s
         after the applied poll and IS running 2.15 s after it (4 beats at 120 are 2.0 s; NA-16); "clockBeat" at the
         first poll after play is within 0.25 of its held value (NA-15).
     Five rounds: S, F, S, F, S.
     (e) OPEN, once, from S3r on: stop (arm S); `POST /api/load_composition` of the second show; after the load, for
         2.0 s "beatTimer" is "stopped" and the nine fields are constant; "ms" == 37 and beatNudgeAppliedMs reaches
         37.0 while stopped; then play: totalBeatCount == the held count + 1 and beatInBar == 0 in the applied poll.
     INFO arm (FM-R3), Auto on the click file, LOCKED: ten rounds of stop then play with the play posted 30 ms before
         the next expected onset, ten with it posted 30 ms after one: "INFO  LR2 Auto: beatInBar left 0 within half
         a beat in <k> of 10 (before the kick), <j> of 10 (after it)". No bar.
     "PASS  LR2 stop is the 1 and play is the edge: 5 of 5 rounds; open leaves it stopped". RED arms: build-mut-edge:
     MR2 -> "FAIL  LR2a: count c+1, want c" (rounds 1 and 3: with a nudge engaged the stop hop re-seats the count, so only
     the un-nudged S rounds can show it), MR3 -> "FAIL  LR2d: no edge at play"; build-mut-fold
     (MR18) -> "FAIL  LR2d: routine clock jumped"; build-mut-row (MR33) -> "FAIL  LR2e: beatTimer running after open".
LR3  NOTHING SET TO BPM MOVES; THE REST DOES. Manual 120, the click file playing, the fixture show: a BPM-synced image
     sequence, a BPM-synced video (each taking >= 8 s per loop at 120) and a Timeline video of length L >= 20 s (L
     pinned in the probe), all playing. A clip's position is its "playheadPosition" in `GET /api/composition`
     (M:ApiServer.cpp:507), read every 100 ms. PRECONDITION while running: each of the three positions differs between
     two reads 300 ms apart (else "INVALID: clip not moving"). Stop for 5 s; the window starts 200 ms after the
     applied poll.
     (a) the nine beat fields constant over >= 900 polls (5 ms).
     (b) onsetCount rises by >= 8 and rms is above 0 in >= 90 % of polls (the analysis listens).
     (c1) the image sequence: every read in the window equals the first, and its "playing" is true in all.
     (c2) the BPM video: the same two clauses.
     (d) the Timeline video: its position x L advances by the elapsed time within 10 %.
     Then play: for (c1) and (c2), within 1.0 s the position has changed and the first changed read is within 0.05
     of the held one (it goes on; it does not jump).
     (e) the same with pause for 2 s in place of stop: (c1), (c2), (d).
     "PASS  LR3 one stopped beat: beat held, sequence held, video held, free clip ran". RED arms: build-mut-gate: (a)
     FAILS (MR1) and "FAIL  LR3c2: video moved" (MR20) while (c1) passes; the lane at the S2 commit: "FAIL  LR3c1" and
     "FAIL  LR3c2".
LR4a THE TEMPO STEPS (after S2r via "handler"; again after S3r via "button"). Each step is read from `/api/bpm` after
     it is applied, exact to 0.01, and its "clicked" / "refused" answer is checked: a refused step
     answers "clicked": false with "refused": "range" (or "disabled" when the control was already greyed). Seeds are
     `POST /api/set_bpm` (Manual on), waited for.
     A (seed 120): 1 double -> 240. 2 double -> refused, 240. 3 half -> 120. 4 half -> 60. 5 half -> 30. 6 half ->
       refused, 30. 7 bpm_minus -> refused, 30.
     B (seed 100): 8 half -> 50. 9 half -> refused, 50.
     C (seed 127.6): 10 bpm_plus -> 128. 11 bpm_plus -> 129. 12 bpm_minus -> 128.
     D (seed 100): 13 double, double, half posted back to back with no wait -> 200 once the third is applied.
     E (seed 200): 14 double -> 400. 15 bpm_plus -> refused, 400. 16 double -> refused, 400.
     And at every double and half the beat goes on: between the two polls around the applied one, totalBeatCount +
     beatPhase advances by no less than dt x the slower tempo / 60 - 0.1 and no more than dt x the faster tempo / 60
     + 0.1 beat, and never falls (a tempo value never realigns).
     "PASS  LR4a tempo steps: 16 of 16". RED arms: build-mut-fold (MR7): "FAIL  LR4a step 1: 120, want 240";
     build-mut-edge (via "handler"): MR26 -> "FAIL  LR4a step 2: 400, want 240" and "step 9: 30, want 50"; MR28 ->
     "FAIL  LR4a step 13: 50, want 200".
LR4b THE NUMBER, THE ENDS, AUTO (after S3r; via "button").
     1 `bpm_edit` "127.6" enter -> bpm 127.6, bpm_text "127.6", tempo_row.manual true.
     2 "500" enter -> 400, "400".   3 "abc" enter -> 400, the editor closed.   4 "12" enter -> 30, "30".
     5 at 30, once ui_text shows it: enabled.minus and enabled.half false, plus and double true; `tempo_row` half ->
       "refused": "disabled", 30.
     6 "240" enter, once ui_text shows enabled.double false: `tempo_row` double -> "refused": "disabled", 240.
     7 Manual off by the toggle route; LOCKED on the click file; `tempo_row` bpm_plus -> tempo_row.manual true within
       200 ms and bpm is the next whole number above the bpm polled just before the press.
     8 "256" enter, then Manual off: in the applied poll bpm == 128.
     "PASS  LR4b the number, the ends, Auto: 8 of 8". RED arm: the lane at the S3a commit ("FAIL  LR4b: 404").
LR5  THE TEXTS FOLLOW THE MODEL; BOTH EDITORS GIVE THE KEYS BACK; THE BUTTONS ARE THE TIMER (replaces RN L11; via
     "button"). (a) after every set in L2, L7, L9: ui_text nudge.text equals T-U1's string within 200 ms. (b)
     `nudge_edit`: begin -> editor open; type "12", enter -> "ms" 12, "nudge +12 ms", closed, focus_home_count + 1;
     type "x9", escape -> 12, + 1; focus_lost -> 12, + 1. (c) the same three closes through `bpm_edit`. (d)
     tempo_row.lit is "play", "pause", "stop" within 200 ms of each op. (e) tempo_row.order equals T-RB3's fourteen
     names with strictly increasing x, and tempo_row.shed is empty at the default size. (f) every tooltip equals
     T-RB5's. (g) each of the nine `tempo_row` ops answers "via": "button", "clicked": true; "beatTimer" follows play,
     pause and stop; across play and pause no clip's "playing" in `GET /api/composition` changes; across stop a
     running routine is still running in `GET /api/routine/status`.
     "PASS  LR5 texts follow the model: <n> of <n>; editors: 6 of 6 closes hand focus home; buttons: 9 of 9". RED
     arms: the lane at the S3a commit ("FAIL  LR5: no tempo_row in ui_text"); build-mut-row (MR24): "FAIL  LR5g:
     beatTimer stopped after play".
LR6  THE SEVEN TARGETS. `binding_action` {"action": "tempoRow", "op": k}, Manual 120, in this order: 1 stop ->
     "stopped". 2 play -> "running", the count + 1. 3 pause -> "paused". 4 play -> "running". 5 double -> 240. 6
     half -> 120. 7 bpm_plus -> 121. 8 bpm_minus -> 120. 9 a release (value 0) -> nothing changes. 10 a CC-typed
     binding -> nothing changes. Then learn: 11 MIDI learn refuses a CC on a row target and the binding list is
     byte-equal; 12 a note attaches. "PASS  LR6 the seven targets: 10 of 10 steps; learn: 2 of 2". RED arm: the lane
     at the S4 commit ("FAIL  LR6: 400").
LR7  THE TIMER IS NOT IN A TAKE (NA-4). Manual 120. Take A: `POST /api/perf/record`; through `tempo_row`: stop, play,
     pause, play, double; `POST /api/perf/stop`. Take B: record; double; stop. BAR 1: A's points, as (scope, control,
     action) triples without their times, are exactly B's. BAR 2 (the tempo seeded back to 120 before each replay):
     take A replayed with the timer running: "beatTimer" is "running" in every poll during and after the replay,
     and bpm reaches 240 (the tempo point IS replayed). BAR
     3: take A replayed with the timer stopped: "beatTimer" stays "stopped" throughout, the nine beat fields do not
     move, the replay's position ADVANCES (a take follows its own audio, not the beat: V31) and bpm reaches 240
     (R75). The probe deletes the takes it made. "PASS  LR7 the timer is not in a take: points equal, replay leaves
     it running, a stopped timer stays stopped under a replay". RED arm: build-mut-edge (MR25):
     "FAIL  LR7: take A has <n> more point(s) than take B".
L10  gains one line per new RED arm above.
VG   Five critic seats on R1-R20 and R9b against V18, V18b, V19: 0 MUST. States: PL:433-439's R1-R20 with "<<", ">>"
     and "R[]" in place; R9b = STOPPED with a quantised clip fired and waiting. Pre-registered MUSTs: the eight of
     PL:661-664, plus: two buttons of the top bar with the same text; a button text wider than its box; anything shed
     at the default size. Named questions the seats answer in words (a SHOULD unless a seat shows a mis-read): can
     ">" (play) be told from ">>" at arm's length; can "R[]" be told from "[]"; is the lit one of play / pause / stop
     readable as "the beat is stopped"; in R9b, does the waiting pad read as waiting, not dead; is a greyed "x2"
     legible as "cannot", not "broken"; do "<<" = later and ">>" = earlier read the right way round beside "nudge
     +12 ms".
After every live batch: RN's closing line stands (no pid, no window left; no Output window; no full-screen capture;
no synthetic input).

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; replaces PL section 6)
RN's B5, B10, B11, B12, B13 stand.
B-R1  Music on, the beat locked or tapped. Press ">>" (nudge forward) a few times, then "<<" -> the flashes and cuts
      come EARLIER, then later, against the music; the text counts "nudge +1 ms", "+2" ...; the tempo number does
      not move. WRONG: forward makes it later; the tempo changes; only some things move.
B-R2  Press pause (||) -> everything that follows the beat freezes where it is: the circle, beat pulses, BPM-synced
      clips, autopilot steps, a routine that is playing. A clip that just plays at its own speed keeps playing;
      things that follow the SOUND (loudness, hits) keep reacting. Press play -> it all runs on from where it stood.
      WRONG: a free clip freezes; a BPM clip keeps running or goes black; the picture jumps when you press play.
B-R3  With the nudge at 0 and Manual on: press stop ([]) -> the circle sits on beat 1 and stays; stop is lit; the
      tempo number still shows the tempo. Wait for a "1" in the music and press play ON it -> beat 1 is exactly your
      press. WRONG: the circle sits on another beat; play starts somewhere else in the bar; the number shows 0
      (question 126). With the app listening (Manual off) its own listening takes the beat over again right after
      your press; if the "1" then slips to "2", tell us (we are measuring how often).
B-R4  One honest limit: the "Structural Landscape" source drifts at a speed set by the tempo NUMBER, not by the beat,
      so it keeps drifting while stopped. Tell us if that bothers you.
B-R5  Stopped, with Quantize on, fire a clip -> it waits; press play -> it starts on that press. Press a routine pad
      while stopped -> it waits too, and starts one bar after you press play (its lead-in needs that bar). With
      Quantize off a clip fires at once, stopped or not. WRONG: the clip never starts or starts a bar late; a waiting
      pad looks dead; you want the routine on the press too (tell us).
B-R6  Stopped, tap a new tempo -> the number changes, the beat stays stopped; Resync -> still stopped, on the "1"
      (question 128). After stop the nudge text reads "nudge 0 ms" (question 129). Tell us if you want it kept.
B-R7  With the app listening press "x2" -> the number doubles, Manual switches on by itself, the beat runs twice as
      fast from where it was, no jump. "/2" twice from 128 -> 64, then 32; a third "/2" is grey. "+" from 127.6 -> 128
      -> 129 (question 125). Two quick "x2" from 100 -> 400, not 200. Click the number, type 140, Enter -> 140, and
      your clip keys work again at once. WRONG: x2 at 128 shows 128; two quick presses count once; the number shows
      what you typed, not what runs; keys type into the box.
B-R8  Put "Beat stop" and "Beat play" on two pads, "Tempo x2" on a third -> they do what the buttons do; with the
      app listening, the Manual switch turns on when the pad doubles the tempo; a knob does not attach to them.
B-R9  At arm's length on your screen: is the row in the order you gave; can you tell play ">" from nudge forward
      ">>", and the row's stop "[]" from the routines' "R[]" to its left; do "<<" = later and ">>" = earlier read the
      right way round to you; is "x2" the label you want (you wrote "*2"); is "nudge +12 ms" readable where it sits;
      can you see at a glance that the beat is stopped?
B-R10 Make the window narrow -> the row never loses a control; FPS / DSP go first, then the "Quantize:" word, the
      LOCKED word, then the two Master faders (still on the Composition tab). Is that the right order to give up?
B-R11 Hold "+" (tempo) and hold ">>" (nudge) with the mouse -> both run steadily and stop at their ends. Is the speed
      right? At very fast tempos beat-driven looks flash fast: 400 is almost 7 flashes a second.
B-R12 (a confirmation; a machine row proves it) Record a take while you stop and start the beat, replay it -> the
      replay does not stop your beat.
B-R13 Stop the beat, open another show, press play -> it was still stopped after opening, and starts on your press,
      with that show's nudge amount.

## 7 BORIS QUESTIONS (125..129; each has a default A; nothing waits; none has been shown to him yet)
His words this delta rests on, verbatim: "61 b but lets call it "nudge X ms""; "do a similar to resolume: beatWheel play
pause stop bpm# bpm- bpm+ nudgeBack nudgeForward /2 *2 tap resync"; "good (this is just nudge amount)"; "just the bpm
timer. If most of the show is set up to BPM, and the BPM goes stop, the BPM goes to zero nothing moves. If there are
clips that are not BPM based, then they play just as they were and are unaffected"; "If I tapped the tempo again, to set
the tempo, the time does not change. If I press re-sync, then it does re-sync and that changes by how far off the beat
we are."; "49 default".
125. The tempo "-" and "+" buttons -- how big is one step?
     A (default) Whole numbers: from 127.6, "+" goes to 128, then 129; "-" goes to 127. For anything finer you type it.
     B Small steps of 0.1: 127.6, 127.7, 127.8 ...
126. The BPM timer is stopped. What should the tempo number show?
     A (default) The tempo it will run at when you press play (so you can still tap, type, halve and double while it
       is stopped). The lit stop button and the still circle show that it is stopped.
     B 0, until you press play.
127. Today there are three small buttons left of the beat circle: play and pause run every clip on every layer, and
     the square stops all routines. Your row now has its own play, pause and stop for the BPM timer.
     A (default) The old play and pause for all clips leave the top bar (each layer keeps its own). The "stop all
       routines" button stays, to the left of the row, labelled "R[]" in the routines' green so it cannot be taken
       for the row's stop.
     B All three old buttons stay, to the left of the row.
     C All three go; routines are stopped from their own bands and pads only.
128. The BPM timer is stopped or paused, and you tap a tempo or press Resync.
     A (default) The tempo you tapped is taken, Resync puts it on the "1" -- but it stays stopped until you press play.
     B Tapping or Resync starts it running again.
129. You have set a nudge, say "nudge +12 ms". You press stop, and later play. What does the nudge read?
     A (default) 0. Stop then play is a fresh start by hand, like Resync: the "1" is where you press play, nothing on top.
     B Still +12. Stop and play never touch the nudge; only Resync puts it back to 0.
Readings to tell him with the questions (he corrects only what is wrong):
 R74 Pause never touches the nudge number.
 R75 A take does not record the timer's play / pause / stop, and replaying a take never stops your beat. While the
     timer is held, a routine that is playing waits with it; a take that is replaying follows its own audio and plays on.
 R76 With the app listening, "-", "+", "/2", "x2" or a typed tempo switch Manual on.
 R77 The tempo you set by hand can go from 30 to 400; what the app hears by itself stays between 60 and 200. Above
     about 200, beat-driven looks flash fast (400 is almost 7 times a second).
 R78 "/2" and "x2" never move the "1": the beat just runs half or twice as fast.
 R79 The "Bar 1..4" text sits just left of the circle; the LOCKED word sits beside Manual.
 R80 The nudge buttons read "<<" (back = later = minus) and ">>" (forward = earlier = plus), because "-" and "+" are
     now the tempo's.
 R81 Stopped or paused with Quantize on: a clip you fire waits and starts when you press play -- from stop on your
     press, from pause on the next beat or bar line. With Quantize off it fires at once.
 R82 Opening a show, or New, never starts or stops the BPM timer. The app always launches with it running.
 R83 A routine pad pressed while the timer is stopped starts on the first bar line after play: one bar after your
     press.
 R84 Resync while stopped or paused: the beat goes to the "1" and the nudge reads 0, as after any Resync.
Not asked, because his words or a reading settle them: the plan's draft 129 (a quantised fire while stopped: R81,
NA-10); 49 ("49 default"); whether Open restarts the timer (R82, NA-8).
The line that changes for each B: 125 -> `bpmStepUp` / `bpmStepDown` (and T-G11's, T-G13's and LR4a group C's numbers).
126 -> one branch in the label's text source (the five tempo controls still act on the real tempo). 127 -> B: two
buttons and their two callbacks stay left of the row (+ 50 px: DSP is shed at 1728); C: the routines stop and its
tooltip case go (- 36 px). 128 -> `kGesturesStartTimer`. 129 -> the two statements in `setBeatTimer`'s Stop path are
deleted; T-N19 is then the live path and LR2 (a) reads 40 in rounds 4-5.

## 8 HARMONY'S DECISIONS (each has a default)
HR-1  Order against the transport lane. DEFAULT: BPMTracker.cpp -- S1, then S1r, then the transport lane's S4t; if S4t
      is already merged, S1r takes it in as its step 0 and TL-U70 is registered once (RT H-T1). Renderer.cpp's
      `syncMedia` -- this lane owns the three advance lines, the transport lane the speed lines; the lane that merges
      second re-applies its edit as its step 0, and BOTH lints (T-G12 here, TL-L7 there) are green after it.
HR-2  Stage S4r (keys and pads for the seven). DEFAULT built. ALTERNATIVE deferred: those seven are mouse-only.
HR-3  The overlay target "Stop" is renamed "Stop routines". DEFAULT yes.
HR-4  The Link-on greyed state is unit-tested, not captured (no Link in the default build). DEFAULT as stated.
HR-5  No production route and no OSC for the timer or the hand tempo beyond today's `/api/set_bpm`. DEFAULT none.
HR-6  A second pitfall number for the timer (RN H-12's rule: assigned at merge).
HR-7  Tap, REST `/api/set_bpm`, OSC and Link keep the 60..200 fold. DEFAULT unchanged.
HR-8  The hand ceiling `kHandMaxBPM`. DEFAULT 400. ALTERNATIVE: a lower value is one constant; the cases and steps
      that name 240, 256 or 400 follow it (T-G9, T-G11, T-G13, T-N7b, T-RB2, T-RW3, LR4a groups A and E, LR4b steps 2,
      6 and 8, R11, B-R7). 200 is ruled out (ST-8).
HR-9  A routine fired while stopped starts on the first bar line after play (NA-16). DEFAULT as ruled. ALTERNATIVE:
      an architect line on the routine engine's lead-in, if Boris wants it on the press.
HR-10 If the bar still does not fit at 1728 after NA-6's two remedies. DEFAULT: DSP is shed at his width (the shed
      order's first item) and he is told in B-R10. ALTERNATIVE: propose question 127 C to him (- 36 px).
HR-11 The routines stop's label "R[]" (NA-5). DEFAULT as ruled. After the visual gate she may change that one string;
      it must stay distinct (T-RB3b).
HR-12 The mutant apps (NA-18). DEFAULT four new builds, one after another in one scratch directory (RN R9; `df -h`
      first). She may merge two apps only if their failing clauses stay disjoint.
HR-13 Readings R74..R84 go to Boris with questions 125..129, on one page. DEFAULT yes.
HR-14 FM-R3's outcome (Auto, play from stop). DEFAULT: reported and said in B-R3; any fix is an architect line.

## 9 SIDE FINDINGS
SF-R1 MAIN: in Manual the FPS and DSP labels stop updating and the number shows typed text (VERIFIED M:TopBar.cpp:
      322-329, :365-369). S3r's rewrite removes it as a consequence.
SF-R2 MAIN: the bindable "Play / Pause" runs the audio file, not clips (V26). Not this lane.
SF-R3 The dispatch lists 47..50 as open; the record has all four answered (BD:864-897), 50 answered and then changed.
      The record wins, as RN A21 ruled.
SF-R4 MAIN: PresetSelector reads any backward step of barPhase from above 0.5 as a bar crossing (V27). A Stop pressed
      in the second half of a bar is such a step, so a preset selector steps once AT the stop -- as it does at a
      Resync today. Not fixed here (the fix is a counter read in that source).
SF-R5 MAIN: after a REST `set_bpm` the Manual toggle is stale (FB T1, INFERRED there). NA-17 removes it.
SF-R6 PLAN: "one term" in the recorder clock is wrong (V21); "four sites" misses a fifth (V7); "every fire that
      waited" over-claims for routines (V12); PL:300-302 contradicts itself; PL:427 says thirteen; PL:497 disagrees
      with PL:423 and schedules LR4 before `bpm_edit` exists.
SF-R7 SEATS: both seats re-checked their citations, and every line re-read here was where they said. Three claims
      inside accepted attacks are wrong and change nothing in the verdicts: GA-3's "also gives 240" (it gives 400);
      ST-5's "256, 400 (clamped), 200" (the plan disables, it does not clamp); ST-3's "gone with no text" (the row
      shows "nudge 0 ms").
SF-R8 A take recorded across a held stretch carries an "unmetered" anchor (tempo 0). M:src/recording/RoutineSlice.cpp:
      67 tests for such anchors when a routine is cut from a take; what it then does was not read. For S5's recording
      doc and for whoever next touches routine saving.
SF-R9 A replaying take is not on the beat (V31). PL:294-296's reason for keeping the timer out of a take ("a replayed
      routine whose own clock is the beat ...") is right for routines; for a take the reason is that the timer is the
      stage's, not the recording's. An earlier draft of this ruling had a replaying take wait; the code says it plays on.
SF-R10 The quote in PL F3 and in M:MainComponent.cpp:624 (his 2026-09-26 ruling that Stop is for routines only) is
      NOT in BD or BL (grep); this ruling cites the code comment and does not quote it as his words.

## 10 RISKS (the strongest counterargument first)
R1  STRONGEST: "Hold at the publish step instead. Nothing the tracker does could then leak into a held beat -- the
    plan's own step 2c IS that hold -- and this ruling had to add a fifth site, a start-hop rule and a Resync rule the
    plan missed: proof that gating a tracker from the inside is a list nobody knows to be complete. And the Harmony
    constraint said the engine is not re-opened." It loses because (1) pause must go on from the held place: above a
    tracker that ran on, that needs an offset in BEATS that grows with every pause (a minute is 120 beats), while RN
    A1's bar seating and count guards were built for a shift of at most 3.33 beats and pin the published bar count
    never to fall -- after a long pause no bar edge would be published until the tracker's count caught up; (2) play
    from stop must write the tracker in any design, or the published count runs ahead of it for good; (3) in Auto the
    detector must keep following the music while the beat stands, which only a live tracker does; (4) "a list nobody
    knows to be complete" is answered by a case that does not use the list (T-G4c) and by three mutants. What would
    change the ruling: T-G4 or T-G4c cannot be made green without disturbing the detector's own state -- then the
    builder stops (NA-18) and the publish-step hold comes back to the architect with that hop sequence.
R2  NOTHING WAS RUN. Section 0 is from reading 185147b; NA-12..NA-15 are four more readings of the same files by the
    same reader. A wrong reading of the order inside one hop passes this paper and fails the app. Refuting tests:
    T-G1..T-G6, T-G3b, T-N17b, T-R2 on the real classes, RED first; RN A24.
R3  THE DEFAULT OF 129 MAY BE THE WRONG WAY ROUND. If he uses the nudge as his room's number, every stop throws it
    away. The default follows the one choice on record ("49 default") and keeps "play is exactly your press" true at
    any nudge; the other answer is two statements deleted, and he is asked before anything he could lose exists.
R4  400 BPM. One x2 at 160 gives 320: beat-driven looks flash 5.3 times a second; the ceiling is almost 7. Strobe
    already reaches 16 by its own knob (V20), so it is not a new class, but x2 makes it one press. He is told (R77,
    B-R11); the ceiling is one constant (HR-8). The 68 beat-phase shader lines were not read.
R5  THE FIT AT 1728 RESTS ON 17 PX AND TWO ASSUMED WIDTHS. T-RW1's new clause turns a miss into a RED before a pixel
    is shown; NA-6 orders the remedies; HR-10 is the last resort. Question 127 B adds 50 px and sheds DSP at his width.
R6  THE HOLD BY dt = 0 IS INFERRED. If a held video goes black or pending (FM-R2) the lane stops at S2r's rows; the
    named fallback (the player paused, with the write-back skipped while held) is an architect line, not pre-built.
R7  IN AUTO "PLAY IS THE 1" IS CERTAIN FOR ONE HOP (NA-13) and then belongs to the detector. If it scores the kick he
    pressed on, the bar reads "2". Main's Resync has the same exposure; FM-R3 measures how often.
R8  TWO LANES IN THREE PLACES: BPMTracker.cpp (S1, S1r; the transport lane's S4t), Renderer.cpp's `syncMedia` (three
    advance lines here, the speed lines there), TopBar / MainComponent / ApiServer (RN R11). HR-1's order and the two
    lints are the control.
R9  A ROUTINE STARTS A BAR AFTER PLAY (NA-16). It is coherent with its lead-in and it will still look late beside a
    clip that starts on the press. R83 and B-R5 say it before he meets it.
R10 FOUR MORE MUTANT APPS (eight with RN's): disk and build time; RN R9's rule applies (HR-12).
R11 A TAKE RECORDED ACROSS A STOP has an unmetered stretch: events inside it share one beat value and keep their wall
    time (PL RR8; SF-R8).
R12 THE DEBUG ROUTE HAS TWO BODIES ("handler" at S2r, "button" from S3r). The answer's "via" and each row's stated
    expectation keep a row from passing on the wrong one.
R13 QUESTION 129 WAS RE-ASSIGNED. The plan's draft 129 is told as a reading. If Harmony would rather ask it, it needs
    a number above 129; nothing in the build changes either way.

FACTS HARMONY MUST MEASURE (each named where it is used; both outcomes ruled)
 FM-R1 The margin at 1728 with the real fonts: T-RW1 prints "margin <n> px"; VG-0's manifest gives the Master Signal
       label's width on today's bar. n >= 0: as ruled. n < 0: NA-6's remedies, then HR-10.
 FM-R2 A held BPM-synced clip in the real app: LR3 (c1), (c2), (e), and `.harmony/probe-video.sh` and `.harmony/
       probe-seq-vram.sh` GREEN on the lane. Held and never pending: as ruled. Black, pending, or a probe RED: STOP
       after S2r; R6's fallback goes to the architect.
 FM-R3 Auto, play from stop pressed on a kick: LR2's INFO arm. k = j = 0: nothing. Otherwise B-R3's last sentence
       stands and the hop log goes to the architect (HR-14); nothing is built here.
 FM-R4 N_main and the golden on her pristine 185147b (RN FM-1, FM-3): unchanged.
 FM-R5 The rig's drift (RN FM-7), for LR1's slope clause.
 FM-R6 The take file's shape for LR7: she opens one take the probe made and checks the builder's point test by eye
       before the row is trusted.
 FM-R7 (Boris, not Harmony: no synthetic input) the look at arm's length (B-R9) and the held-button speed (B-R11).

SUMMARY: 15 attacks ruled (12 ACCEPT, 3 PARTIAL, 0 REJECT); 18 amendments (NA-1..NA-18); stages G-N0, S1, S1r, S2, S2r,
S3a, VG-0, S3m, S3r, S4, S4r, VG, S5; 5 Boris questions (125..129) with defaults and 11 readings (R74..R84); 14 Harmony
decisions with defaults; 7 facts to measure.

STATUS: DONE
