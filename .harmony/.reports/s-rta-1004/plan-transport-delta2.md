# PLAN -- lane "transport-delta2": the clip transport, SECOND DELTA on the ruled chain (s-rta-1004)

STATUS: DONE (plan authored; nothing built, nothing run; every number that needs a run is named in 5.6 and in NOT VERIFIED at the end)
Author: architect, 2026-10-04. Read-only. Pin checked: main HEAD = 185147b, `git status --short -- src tests docs CMakeLists.txt`
printed nothing, so every src line below is a plain-file read at 185147b. Worktrees: lane/bf2 = 740b6d6 (clean), lane/bf2-keys =
9eab9bd (clean); neither was read for code (H-17: parts only; this lane takes none, section 9).
Precedence after adoption: this delta > ruling-transport-delta1.md > ruling-transport.md > plan-transport.md's body.
Paths are relative to /Users/boriskarpman/projects/RealTimeAudio. "The chain" = those three files in .harmony/.reports/s-rta-1003b/.
A correction to the dispatch: it lists questions 47-50 as OPEN. binding-decisions.md:864-871 records all four answered at
12:41 ("47 default", "48 b", "49 default", "50 default"). This plan builds 47 as answered and still keeps it one function wide (TD3).

## 1 GOAL

Boris changed the ground of a ruled, unbuilt lane. In his words (binding-decisions.md, sections "2026-10-04 (s-rta-1004)"):
- "I want you to study these images and mimic exactly how resolume is doing. It's transport control."
- "lets do bars here not beats. I know I said beats before but lets do bars"
- "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples. Any other way will not
  work with music. Music and especially DJ music, is always in multiples of four."
- "Timeline only shows bars. The only place we see beats is in the circle with 4 positions in top bar that shows the 4 beats repeating."
- "build random and beatloopr"
- "when you pause a clip and then fire it, it stays, paused" and "stays in layer strip paused"
- "b can you program this reliably or should we change the plan?" (to: the clip slides until its bars sit on the music's bars)
- "doubles and halves and then smaller fractions to higher multiples, just like resolume does"
- "42 default" (every fire starts at second 0; no menu), "47 default" (4 bars is the least; 4, 8, 12, 16 ...; a short clip starts slow)
This file gives: one verdict for each of the 40 amendments, the panel, the bars arithmetic, the measurement that answers his
reliability question before anything depends on it, and the stage list that replaces ruling-transport-delta1.md section 4.
Harmony constraint: where his ruling differs from Resolume, HIS ruling is built and the difference is shown to him (section 6, D-1 .. D-9).

## 2 ESTABLISHED FACTS (only verified lines; INFERRED / ASSUMED are labelled where they are used, in section 3)

Code at 185147b (read by me):
- E1 Clip transport fields: `transportMode {Timeline, BPMSync}` Clip.h:118-119; `loopMode {Loop, PingPong, OneShot}` :120-121;
  `speed` :122; `reverse` :123; `inPoint` / `outPoint` :127-128; `beatDivision` :36 and `videoBeats` :41 (the two old BPM Sync
  numbers); runtime `playing`, `playheadPosition`, `beatsPlayed`, `hasBeenTriggered` :237-240 (Relaxed, not saved).
  Saved keys: Clip.cpp:47-48 (beatDivision, videoBeats), :65-67 (transportMode, loopMode as int, speed), :70 (inPoint).
- E2 Pause today is the CLIP's `playing`: the Clip tab's three buttons write `clip_->reverse` / `clip_->playing`
  (ClipInspector.cpp:37-53); the strip's four write the playing clip's (LayerStrip.cpp:373, :379, :385, :391 -- the fourth
  also doubles `speed`, capped at 4). There is no pause field on Layer (grep `paused` in Layer.h: none).
- E3 The fire tail: `Layer::applyActivationTail` sets the playhead to `inPoint`, and sets `playing = true` only for a new
  activation of a clip never triggered (Layer.h:548-558). The cell handler marks `hasBeenTriggered` and, for a Snap clip,
  copies `beatPhase` into the playhead (MainComponent.cpp:4866-4889).
- E4 BPM Sync today: a video's speed is `videoBeats / beatDivision`, tempo not used beyond `bpm > 0` (Renderer.cpp:1657-1665);
  a sequence's fps is derived from `beatDivision` and the tempo (:1718-1730); both branches do their own `featureBus_.read()`
  (:1660, :1721) although the frame's snapshot is `frameSnap_` (:402). Timeline speed = `effectiveClipSpeed(clip->speed,
  masterSpeed, false)` (:1670; Renderer.h:290).
- E5 ClipTransportSync.h (whole file, 74 lines): `pushIntent` reads `clip.playing` once and pushes it (:33-41); `writeBack`
  stores the playhead, compare-exchanges `playing`, and at `outPoint < 1` either stops a OneShot or seeks to `inPoint` (:45-73).
- E6 A video at the end of a OneShot stops on its last frame: `currentTime_ = duration_; playing_ = false` (VideoPlayer.cpp:431-434;
  backward :449-452). `setSpeed` stores any float, no clamp (VideoPlayer.h:81; ImageSequence.h:51). Both have `getDuration()`
  (VideoPlayer.h:70; ImageSequence.h:45).
- E7 Clip tab, Transport section today: mode combo with "Timeline", "BPM Sync" (ClipInspector.cpp:9-10); play-back / pause /
  play buttons (:31-53); Loop dropdown "Loop", "Ping Pong", "One Shot" (:56-58); a second dropdown "Restart", "Continue",
  "Relative" with NO onChange (:69-73: it does nothing); Speed slider 0..4, default 1 (:75-87); a Duration slider 0.1..300 with
  NO onValueChange (:90-98: it does nothing) and "/2", "x2" buttons (:102-103); the second row is PAINTED "Beats" in BPM Sync and
  "Duration" otherwise (:546); in BPM Sync two more rows: "Beats/ Cycle" dropdown (:204-234) and "Content Beats" slider
  (:237-275); a position readout painted as a 0..1 number with 2 decimals (:522-528); beat lines from `beatDivision` (:1262-1272);
  two direct playhead writes in the bar's handlers (:1372, :1396).
- E8 Layer strip: S fader 0..1 shown as `speed / 4`, default 0.25 (LayerStrip.cpp:396-411, :721, :841-849); a direct playhead
  write (:988); `TransportView` / `transportViewOf` (LayerStrip.h:58-66).
- E9 The beat clock in the snapshot: `bpm`, `beatPhase`, `trackerState` (FeatureSnapshot.h:40-42); `beatInBar` 0-3 and
  `barPhase` over 4 beats (:45-46); `totalBarCount` (:57); `resyncBarOrigin` (:126); `totalBeatCount` (:137);
  `trackerRequestSeq` (:144).
- E10 How the bar position is made: `barPhase_ = (beatInBar_ + phase_) / 4` (BPMTracker.cpp:486-499). `beatInBar_` advances
  (a) on a predicted phase wrap, only in the predicted regime (:243-246, :260-277), (b) on a DETECTED beat otherwise
  (`scoreBeat`, :343-346, :377-382), and (c) is re-derived when the downbeat detector re-locks (:438-452, checked every 16
  scored beats :385-388; first lock :474-481). `realignPhaseToZero` adds one to `totalBeatCount_` from the second half of a
  beat and never touches `beatInBar_` (:253-258). A confident detection realigns in Auto (:234-236). Resync sets
  `beatInBar_ = 0`, `barPhase_ = 0` (:566-577).
- E11 `/api/bpm` exists (ApiServer.cpp:215); `/api/inject_features` is a test-mode route that writes `barPhase` and
  `beatInBar` directly (:237, :969, :976-977); `/api/composition`-side dumps publish `barPhase`, `beatInBar` (:391, :843-845).
- E12 `Layer::clearActiveClip(rows, onlyIfActive, maxAttempts)` exists (Layer.h:492) and the activation path is CAS-based (:563-579).

Fact sheets (rows whose VERIFICATION did not overturn them):
- R1 Resolume, BPM Sync Speed: "the BPM Sync Speed is quantised to multiples of 2. So you can ramp a clip from 0, 1/8, 1/4,
  1/2, 1, 2, 4, 8 to 16 times as fast." (facts-resolume-transport Q3; VERIFICATION: CONFIRMED verbatim). What minus / plus do: NOT DOCUMENTED.
- R2 Resolume, Timeline: Speed slider non-linear, finer between 0 and 2, "towards 10"; typing a Duration changes the playback
  speed and "this doesn't affect the Speed slider!"; shortening by in / out adjusts the Duration (Q2, Q4; CONFIRMED). Steps of
  minus / plus on Speed and Duration: NOT DOCUMENTED.
- R3 Resolume's first Beats number: nearest power of 2 of beats, worked out at 120 BPM (Q3 + VERIFICATION refutation 1: docs v6 / v7,
  release note 4.1.4, staff Dec 2016). It changes the SPEED; nothing says an in / out point is moved.
- R4 Loop-style menu, v7: Loop, Ping Pong, Random (Interval, Distance; "measured in seconds or in beats, depending on which
  mode you're in"), Play Once and Eject, Play Once and Hold (Q6; CONFIRMED; "clears the layer" for Eject is NOT on the page).
- R5 Trigger-style menu: from the start (default) / pick-up / relative pick-up (Q7; CONFIRMED).
- R6 A paused clip triggered again: STAFF, v7.9: "If you pause a clip, and trigger it again, it will still be paused at the
  same position." (VERIFICATION refutation 3). In Resolume pause is a setting of the CLIP (same source: autopilot launches a
  paused clip and "it will stay paused").
- R7 After a scrub in BPM Sync Resolume documents (v4 / v6 text only) "out of phase" until the clip is triggered again; no
  automatic re-alignment is documented (Q8; CONFIRMED).
- R8 BeatLoopr: BPM Sync only; loops "over the relevant number of beats" from the current position, not stored; "Catch Up";
  Off button; the OPTION LIST is NOT DOCUMENTED (Q9; one thread title shows a "1/16" option exists; its body was not read).
- R9 Time readout: "Clicking on this number will switch to show you the remaining time."; format seconds:frames per STAFF
  (Zoltan, per the VERIFICATION's correction) (Q10).
- R10 Tap realigns the beat phase and leaves beat-in-bar alone; Resync resets beat-in-bar, bar and phrase; a tempo VALUE
  never touches phase or count; in Auto a confident detected beat realigns by itself; in Manual a detected beat never moves
  the phase (facts-beat-controls Q1, T1; VERIFICATION items 1-5, 10: CONFIRMED). There is no REST route for Tap (item 12).
- R11 The "/4 /2 x1 x2 x4" buttons of the top bar change nothing (facts-beat-controls Q1; VERIFICATION item 8).
Screenshots (observed by me, resolume-transport-1, -2, -7, -8.png):
- P1 Header "Transport" + mode menu at the right; under it the time readout at the top right ("00.14", "03.09"); the bar with
  a playhead flag above it and small in / out pointers at its ends; a row with play-backwards, pause, play-forwards (the active
  one filled) at the left and two small icon menus at the right; row "Speed": number, minus, plus, slider; row "Duration"
  ("8 s", "5.951 s") or "Beats" ("16"): number, minus, plus, "/2", "x2".
- P2 In BPM Sync the bar shows ticks ONLY between the in and out pointers, and in shots 2 and 7 the out pointer sits at about
  three quarters of the bar with the rest dark: a marked part shorter than the file.
- P3 The Speed slider's thumb: "1/4" at about 25 % of its travel (shot 2), "1" at about 50 % in BPM Sync (shot 7), "1" at
  about 25 % in Timeline (shots 1, 8). Nine evenly spaced detents fit shots 2 and 7 exactly (index 2 of 8, index 4 of 8).
- P4 The mode menu lists Timeline (marked), BPM Sync, SMPTE 1, SMPTE 2, Denon DJ, Pioneer DJ (shot 8).

## 3 ITEMS

New amendments are D2-1 .. D2-14 (named at each item). New forks F31 .. F47 (id | choice | runner-up) are listed at the end of section 3.

### TD1 RECONCILE

"Line" = Boris's 2026-10-04 words that decide the row, quoted from binding-decisions.md. "none" = he changed nothing there;
the row rides on "All defaults good except for these:" (11:59:06).

A. ruling-transport.md, AM-1 .. AM-30

| id | verdict | line | what replaces it / what stays |
|---|---|---|---|
| AM-1 queue-only transitions never run the tail | STANDS | "42 default" | unchanged |
| AM-2 restart stamp | STANDS | "42 default" | unchanged; a fire that does not restart (D2-6) mints no stamp |
| AM-3 "every fire plays" (F1) | AMENDED | "when you pause a clip and then fire it, it stays, paused"; "stays in layer strip paused" | D2-6: pause is the LAYER's; a fire sets the clip to play and never lifts the layer's pause. `hasBeenTriggered` is still deleted. FM-2 is still reported |
| AM-4 a held key fires once | STANDS | none | unchanged |
| AM-5 a fire on a layer that is not drawn | STANDS | none | unchanged |
| AM-6 the landing rule, `TransportState` | AMENDED | "lets do bars here not beats. I know I said beats before but lets do bars" | the rule stands; the field list loses `clipBpm`, gains `clipBars`, `syncSpeed`, `timelineRate`, the Random settings (D2-3, D2-4, D2-10) |
| AM-7 refused Redo | STANDS | none | unchanged |
| AM-8 one "live" test | STANDS | none | unchanged |
| AM-9 Load Deck's layers stay | STANDS | none (question 8 kept its default) | unchanged |
| AM-10 | DROPPED | (already replaced whole by DA-5 / DA-7) | stays replaced |
| AM-11 the drag, `scrubClip` | AMENDED | "You can click outside the timeline and In-N-Out points, but that won't do anything." | D2-7: a press outside in..out is ignored; a press inside moves the playhead there |
| AM-12 the hold record, `Intent` | AMENDED | "2 b" | stands; `Intent` gains `paused` (D2-6) |
| AM-13 a drop past the out marker | STANDS | "it cannot be dragged outside of the in and out points" | unchanged (it now arises only from a drag that began inside) |
| AM-14 `clipBpm` | DROPPED | question 30 not named: default "no BPM box on the clip" | `Relaxed<int> Clip::clipBars` (D2-3); its static_assert pin and TSan case take AM-14's place |
| AM-15 S4a additive; the old fields leave with the widgets | AMENDED | "mimic exactly how resolume is doing" | same shape; the test rows are re-aimed at `clipBars` (TD11) |
| AM-16 old shows | AMENDED | "always work with multiples of 4. If it is uneven, then move the outpoint in" | D2-5: an old BPM-synced clip is fitted on open (question 75) |
| AM-17 widgets (Reverse in the BPM row, B2, the "do" route) | AMENDED | "mimic exactly how resolume is doing" | no BPM row, no Reverse button (the play-backwards button is the reverse); the "do" route's actions change (D2-1) |
| AM-18 REST fields, beat lines, the inert combo | AMENDED | "Timeline only shows bars."; "42 default" | REST: `clipBars`, `syncSpeed` instead of `clipBpm`; bar lines (D2-2); the combo's removal is now his ("42 default": no menu) |
| AM-19 unit gates by name | STANDS | none | unchanged |
| AM-20 live rows | AMENDED | (all of the above) | section 5.3 here |
| AM-21 the stale frame, S2b | STANDS | none | unchanged |
| AM-22 rate, wobble, drift | AMENDED | "9 b"; "yes but this will be bars now" | the tempo store stands; FM-3 stands; FM-4 is measured against the BAR (5.6) |
| AM-23 no steady beat: a re-fire restarts at once | STANDS | none | unchanged |
| AM-24 S0's list | AMENDED | -- | section 4, S0 |
| AM-25 T6 corrections | AMENDED | -- | k10 stands; AM-15's rows as re-aimed |
| AM-26 the inert Duration row is removed | AMENDED | "mimic exactly how resolume is doing" (reading R9, told to him) | the dead widgets go; the row is REBUILT live: "Duration" in Timeline, "Bars" in BPM Sync (D2-1). Bar B3's "Duration in no state" is void |
| AM-27 no event text, no failure text | STANDS | "Nothing else." (BF41) and "display not enough HDD space to record." | this lane adds none of the allowed texts either; a greyed control, a number, "--" are states |
| AM-28 the visual gate | AMENDED | -- | five seats stay; states and bars as 5.5 here |
| AM-29 S1 may start before the sync-dial lane merges | DROPPED | "replace our sync with this" | the sync-dial lane does not merge (H-17); the lane's base is main; nothing waits on bf2 |
| AM-30 what no trim may cut | STANDS | "42 default" | unchanged |

B. ruling-transport-delta1.md, DA-1 .. DA-10

| id | verdict | line | what replaces it |
|---|---|---|---|
| DA-1 beat lock | AMENDED | "yes but this will be bars now"; "b can you program this reliably or should we change the plan?" | D2-8: the same law aimed at the bar; MEASURED first (TD7). `beatLockApplies` and Q-D1's three ways go: a clip's bars are always whole |
| DA-2 beat lines | AMENDED | "Timeline only shows bars." | D2-2: one line per bar, none per beat; thinning rule kept |
| DA-3 the hold | STANDS | "2 b" | unchanged |
| DA-4 the range is the timeline | AMENDED | "there are two conditions here. ..." | its bullet "A press or a drag in the dark part of either bar lands on the nearer point" is replaced for a PRESS (D2-7); the rest stands |
| DA-5 the strip rule | STANDS | "cmd-z does not affect anything in layer strip: play, play reverse, pause, transparency, bypass, solo, etc." | strip fields gain the layer's pause and the new clip fields (TD10) |
| DA-6 layers that come and go | STANDS | "If a layer strip is deleted, user can ctrl-z to get it back, but clip is not playing" | `restoredForUndo` also returns the layer not paused |
| DA-7 a layer move ends the history | STANDS | none (question 22 kept its default) | unchanged; S1m stays unbuilt |
| DA-8 units: beats | DROPPED | "lets do bars here not beats. I know I said beats before but lets do bars" | D2-3: the row is "Bars"; no caption, tooltip or painted string of the section holds "beat" |
| DA-9 the S fader is disabled in BPM Sync | DROPPED | "doubles and halves and then smaller fractions to higher multiples, just like resolume does" | D2-4: it steps through nine speeds |
| DA-10 labels and ids | AMENDED | -- | AM-3's rule is his again, in its new form; Q-D1 .. Q-D6 are closed: 20 by "we are going to use whole bars instead, so this doesn't happen", 21-23 by default, 24 by "yes but this will be bars now", 25 by his Speed sentence |

C. Gate rows of the chain

| rows | verdict | why |
|---|---|---|
| S1 cases TL-UC1 .. TL-UC22 | STAND; TL-UC3b's clause "its clip BPM" reads "its bars and sync speed"; TL-UC17's field list gains the layer's pause | TD10 |
| TL-U1, TL-U3 .. TL-U9b, TL-UC4 | STAND | AM-1, AM-2 |
| TL-U2 "a new activation of a paused, previously fired clip plays it" | AMENDED: "... on a layer that is not paused plays it; on a paused layer it waits on its first frame" | D2-6 |
| S3 TL-U10, TL-U13, TL-U14, TL-U15, TL-U15b; S3h TL-U11 .. TL-U12c | STAND; TL-U13 gains "a press outside in..out does nothing" | D2-7 |
| TL-U16 (rate = show BPM / clip BPM) | DROPPED -> TL-U32 | D2-3 |
| TL-U17, TL-U21 (beats math, beat lines) | DROPPED -> TL-U33, TL-U34 | D2-2, D2-3 |
| TL-U18a .. TL-U18d (old shows, no legacy key) | AMENDED: a / b / c re-registered as TL-U35a .. c; TL-U18d stands | D2-5 |
| TL-U19, TL-U19b | AMENDED -> TL-U36, TL-U36b | D2-3, D2-4 |
| TL-U20 (BPM and Beats rows) | DROPPED -> TL-U50 .. TL-U52 | D2-1 |
| TL-U22 .. TL-U28 (beat lock) | AMENDED -> TL-U40 .. TL-U48, same structure in bar units; TL-U25's "beats 14.6, 6.5, 2.5" and TL-U25b go | D2-8 |
| TL-U30 (strip lines) | AMENDED -> TL-U53 (bar lines) | D2-2 |
| TL-U31 (S fader disabled) | DROPPED -> TL-U54 | D2-4 |
| lints TL-L1, TL-L1b, TL-L3, TL-L4 | STAND. TL-L2 STANDS (+ `paused =` banned in commands). TL-L5 AMENDED: `ClipBeatGrid.h` reads `ClipBarGrid.h`, `beatLockTrim(` reads `barLockTrim(` | -- |
| mutants MU-1 .. MU-11, MU-16 .. MU-25, MU-27, MU-28, MU-38 .. MU-45, MU-48 .. MU-50, MU-H1 .. MU-H3 | STAND | -- |
| MU-12, MU-13, MU-14, MU-15, MU-26, MU-29, MU-30 .. MU-37, MU-46, MU-47 | DROPPED -> MU-60 .. MU-79 (5.1) | the code they mutate is replaced |
| live TR2 .. TR4, TR5b, TR6, TR6b, TR7, TR7b, TR7c, TR7d, TR8, TR11, TR12, TR13a, TR14 | STAND | -- |
| TR5 ended_oneshot_refire_plays | STANDS (loop mode "Play Once and Hold") | D2-10 |
| TR9, TR10, TR13b | AMENDED: `clipBpm` -> `clipBars` (5.3) | D2-3, D2-5 |
| TR15 .. TR18 | AMENDED -> TR20 .. TR23 in bar units (5.3) | D2-8, D2-2 |
| TR19 (INFO) | STANDS | -- |
| visual V1 .. V14, B1 .. B12 | DROPPED -> W1 .. W22, C1 .. C16 (5.5); the five seats and the packet rule stand | D2-1 |
| FM-1, FM-2 | STAND. FM-3 STANDS (measured in SM). FM-4, FM-5 AMENDED: bar units; both are measured in SM, before the lock is built (TD7) | -- |
| G-U1 .. G-U5, G-N1 | STAND with the pins of 5.2 here | -- |

D. Stages of the chain (ruling-transport-delta1.md section 4)

| stage | verdict | becomes |
|---|---|---|
| "started after the sync-dial lane (bf2) has merged" (H-3) | DROPPED | base = main; "replace our sync with this" |
| S0 | AMENDED | S0 (list in section 4) |
| -- | NEW | SM, the instrument, FIRST code stage |
| S1 | STANDS | S1 |
| S2, S2b | AMENDED (pause) / STANDS | S2, S2b |
| S3, S3h | AMENDED (press rule) / STANDS | S3, S3h |
| S4a BPM engine | AMENDED | S4a bars engine |
| S4c beat lock | AMENDED | S4c bar lock, built as SM's verdict says |
| -- | NEW | S4t, S4d, S4e |
| S4b widgets + lines + strip | AMENDED, split | S5a the panel, S5b the strip; one visual gate after S5b |
| S5 probe + docs | AMENDED | S6 |
| S1m | STANDS (not built: question 22 default) | S1m |

### TD2 THE PANEL (amendment D2-1; replaces AM-17, AM-26, V1-V14)

VERIFIED: P1-P4, E7, R1, R2, R4, R5, R9.
What it is, top to bottom (Audio-DNA's colours and fonts: reading R1, told to him, not corrected):
1. Header "Transport" with the MODE MENU at its right: "Timeline", "BPM Sync" live; "SMPTE 1", "SMPTE 2", "Denon DJ",
   "Pioneer DJ" listed and greyed ("leave a dropdown menu for those items and grey them out for now"). The existing
   `transportModeSelector_` gains four disabled items; a greyed item cannot be chosen and has no tooltip that announces anything.
2. TIME READOUT, top right, replacing the 0..1 number of E7: the playhead's time in the file, "SS.FF" (seconds, frames of the
   clip's own frame rate; "M:SS.FF" from one minute on -- the long form is INFERRED). A click switches to the time left to
   the out point, shown with a leading "-"; a click again switches back. View state of the inspector: not saved, not on the Clip.
3. THE BAR: the whole file; the marked part (in..out) lighter, the rest dark; small in / out pointers; the playhead flag above.
   In BPM Sync: bar lines inside the marked part only (D2-2, P2). In Timeline: no line.
4. BUTTON ROW: play backwards, pause, play forwards at the left, the active one filled; at the right ONE small menu, the
   loop style: "Loop", "Ping Pong", "Random", "Play Once and Eject", "Play Once and Hold" (R4). NO second menu ("42 default").
   When "Random" is chosen the row below the buttons shows "Interval" and "Distance" (TD9).
5. ROW "Speed": number, minus, plus, slider (TD4).
6. SECOND ROW: "Duration" (seconds; number, minus, plus, "/2", "x2") in Timeline; "Bars" (number, minus, plus, "/2", "x2")
   in BPM Sync (TD3). No BPM box (question 30's default).
7. BPM Sync only, under the second row: the BEATLOOPR row (TD9).
What of today's Clip tab it replaces (all E7): the second dropdown "Restart / Continue / Relative"; the dead Duration slider
and its pair; the painted "Beats" caption; the "Beats/ Cycle" dropdown and the "Content Beats" slider with their labels; the
"Reverse" text button (the play-backwards button is the reverse, as in P1); the 0..1 readout; the beat lines from
`beatDivision`. Kept and outside the lane: the cue-point row, the Snap selector, the autopilot rows, the sequence fps row.
Forks. F31 the Duration row is a factor of its own (`RelaxedFloat Clip::timelineRate`, default 1; Duration shown = marked
length / timelineRate; typing a Duration sets it; Speed multiplies on top; moving in / out leaves the rate and changes the
number) | runner-up: Duration writes `speed`. The runner-up is one field cheaper and loses to R2's "this doesn't affect the
Speed slider!" under "mimic exactly". F32 the Clip tab's three buttons act on the layer that plays this clip; when no layer
plays it, backwards / forwards set only the clip's direction and pause is greyed | runner-up: a pause stored on the clip (as
Resolume, R6) -- loses in TD5.
Controls (UI rules): the Speed slider is a `ResettableSlider` (default 1 in both modes); the numbers of Speed, Duration, Bars
are editable number boxes whose right-click gives the default (Speed 1; Duration = the marked length; Bars = the app's own fit,
TD3); the loop menu and the mode menu stay `juce::ComboBox` as today (no new PopupMenu; if the builder needs one it is
`showMenuAsync` with `.withParentComponent(getTopLevelComponent())`). The readout and the playhead join the inspector's
paint key by what is PAINTED (the text, the pixel x) -- Pitfalls 57, 59.
The layer strip: its back / pause / play buttons are the SAME three actions on the same fields as the panel's (D2-6); its
S fader shows the same value as the panel's Speed row for the mode the playing clip is in (TD4); its fourth button (">>",
E2) steps the speed one step up in BPM Sync and doubles it up to 4 in Timeline, as today (question 76). The strip's bar gets
the same bar lines (D2-2).
Test routes (test mode; they call the widgets' own handlers, no synthetic input): `GET /api/debug/clip_transport_ui` (bounds,
text, enabled, visible of every widget of the section; painted strings; line xs and stride; playhead x; readout text and
mode); `POST /api/debug/clip_transport_do {action, text}` with action = mode_timeline | mode_bpm | type_bars | bars_minus |
bars_plus | bars_half | bars_double | reset_bars | type_duration | dur_minus | dur_plus | dur_half | dur_double | type_speed |
speed_minus | speed_plus | reset_speed | loop_style | random_interval | random_distance | looper | looper_off | catch_up |
readout_click | back | pause | play; `GET /api/debug/strip_transport_ui?layer=N` (as DA-2, plus the S fader's value, step
index and the three buttons' lit state).
Where the documentation is silent, and the ONE screenshot or step in Boris's own Arena that settles each (none blocks a stage;
each default is named): (a) Speed minus / plus in both modes -- default: one step of the list in BPM Sync, 0.1 in Timeline;
settle: click plus once in each mode, read the number. (b) Duration minus / plus -- default 1 second; settle: click plus once.
(c) what Duration reads at Speed 2 -- default: unchanged (F31); settle: Timeline, Speed 2, a screenshot. (d) the readout's long
form and what "remaining" counts to -- default above; settle: a clip longer than a minute, both readout modes. (e) the
BeatLoopr options -- TD9; settle: one screenshot of the BeatLoopr row. (f) whether the playhead jumps on a click in the
bar -- built as reading R5 whatever Arena does.

### TD3 BARS (amendments D2-2, D2-3, D2-5; replace AM-14, AM-16, DA-2, DA-8)

VERIFIED: E1, E4, E9 (one bar = 4 beats: `beatInBar` 0-3, `barPhase` over 4 beats), R3, P2.
His rule against Resolume's: Resolume keeps the whole clip and changes its speed to a power-of-two number of BEATS (R3). He
ruled bars, groups of 4, and the out point moved in. P2 shows what he saw: a marked part that ends before the file does.
D2-3 THE NUMBERS. New pure header src/render/ClipBarGrid.h (no JUCE, no player; used by the renderer, both surfaces, the
tests). It takes the place of the chain's ClipBeatGrid.h.
- constants: `kBeatsPerBar = 4`, `kFitBpm = 120.0`, `kBarStep = 4`, `kMinBars = 4`, `kMaxBars = 4096`, `kFitTolSeconds = 0.05`.
- `int fitBars(double markedSeconds) noexcept;` -- 4 x floor((markedSeconds + kFitTolSeconds) / 8), at least 4, at most 4096.
- `float fitOutPoint(float inPoint, float outPoint, double fileSeconds, int bars) noexcept;` -- when the marked part is
  shorter than 8 s - tol: the out point unchanged (the clip is fitted WHOLE into 4 bars); else min(outPoint, inPoint +
  bars x 2 / fileSeconds).
- `int snapBars(double typed) noexcept;` -- the nearest multiple of 4 (a tie goes up), within 4 .. 4096.
- `double barSyncRate(double markedSeconds, int bars, double showBpm) noexcept;` -- markedSeconds x showBpm / (240 x bars).
- `double clipBarAt(double playhead, float inPoint, float outPoint, int bars) noexcept;`
- `struct BarLines { int count; int stride; double firstNorm; double stepNorm; };`
  `BarLines barLinesOf(float inPoint, float outPoint, int bars, double barWidthPx) noexcept;`
THE FIELD. `Relaxed<int> Clip::clipBars = 0` (saved; 0 = not fitted yet). Pin in tests/test_shared_field_types.cpp; a TSan case
(TL-U36b). F33 an int count of bars | runner-up: keep a float "beats" and show beats / 4 -- loses: every allowed value is a
whole multiple of 4 bars, and a float invites the 14.6 the last ruling had to lock around.
THE FIT (first switch to BPM Sync, and right-click on the Bars number). On the message thread, with L = the marked length in
seconds from the chain's length provider: `clipBars = fitBars(L)`, `outPoint = fitOutPoint(...)`. When the length is not
known yet (0) the clip stays `clipBars == 0`; MainComponent keeps the ids of such clips and fits each on the first timer tick
that knows its length (a bounded list on the message thread; no new thread, no lock). While `clipBars == 0` the renderer plays
the clip at `barSyncRate(L, fitBars(L), bpm)` and writes nothing. A later switch Timeline -> BPM Sync keeps the Bars number
and both points (only a never-fitted clip is fitted). Not an Undo step (transport is live: DA-5).
WHICH 120. `kFitBpm` is the constant 120, never the app's tempo at that moment. His words: "default all bpms to 120"; question
30's default as asked: "120 is used only to work out the first Bars number". F34 constant 120 | runner-up: the app's tempo --
loses: the same clip would get a different number on a different night, and Resolume itself uses 120 (R3).
WORKED EXAMPLES (at 120 one bar lasts 2.0 s; "speed" = how fast the video looks at a show tempo of 120, Speed row at 1):

| marked length | Bars | out point | speed at 120 | at 90 | at 174 |
|---|---|---|---|---|---|
| 2.0 s | 4 (whole) | unchanged | 0.25 | 0.1875 | 0.3625 |
| 7.3 s | 4 (whole) | unchanged | 0.9125 | 0.684 | 1.323 |
| 8.0 s | 4 | unchanged | 1.0 | 0.75 | 1.45 |
| 15.98 s (a "16 s" file as its container reports it) | 8 | unchanged | 0.99875 | 0.749 | 1.448 |
| 16.016 s (29.97 fps) | 8 | moved in to 16.0 s | 1.0 | 0.75 | 1.45 |
| 30 s | 12 | moved in to 24 s | 1.0 | 0.75 | 1.45 |
| 45 s | 20 | moved in to 40 s | 1.0 | 0.75 | 1.45 |
| 60 s | 28 | moved in to 56 s | 1.0 | 0.75 | 1.45 |
| 64 s | 32 | unchanged | 1.0 | 0.75 | 1.45 |

(45 s -> 20 bars, out at 40 s is the example of question 43 C as asked; 2 s -> a quarter speed is question 47 as asked.)
F35 the 0.05 s tolerance | runner-up: none -- loses: a file whose length reads 15.98 s would get 4 bars and lose half of
itself. INFERRED that such lengths occur (container durations were not sampled; S0 reads `getDuration()` of the fixtures).
With the tolerance a clip can run up to 0.6 % slow at 120: said to him as difference D-9.
THE ROW. Minus and plus step by 4 bars. "/2" is enabled only when half the number is itself allowed (the number is a multiple
of 8): at 8 it gives 4; at 4 and at 12 it is greyed (question 71). "x2" doubles; greyed above 2048. A typed number goes through
`snapBars` (6 -> 8, 5 -> 4, 1 -> 4, 13 -> 12); the box then shows the allowed number -- a state, no text (AM-27). Upper end
4096 bars (about 2 h 16 min at 120): INFERRED to be above any clip he plays; the fit clamps to it.
WITH THE NUMBER UNCHANGED, moving the in or out point changes L and so the rate: "If the user pulls the outpoint in, then
within that same amount of bars, it goes through less video, appearing to play slower". Example: 8 bars over 16 s (speed 1);
out point pulled to 12 s -> 0.75; pushed to 20 s -> 1.25. No fit runs on a marker drag.
D2-2 THE LINES. Both surfaces draw `barLinesOf(inPoint, outPoint, bars, barWidthPx)`: only in BPM Sync, only for a fitted clip
with a known length. One line per bar b, 1 <= b < bars, at inPoint + (outPoint - inPoint) x b / bars: a 4-bar clip has 3
lines. No line per beat anywhere. Lines closer than 3 px thin to every 2nd, 4th, 8th ... bar and never vanish (the chain's
F28, H-D2). One weight, one colour. Line b is where `clipBarAt` returns b, and the lock's error is computed from `clipBarAt`:
lines and playback cannot disagree (TL-U34). The lines do not depend on the Speed row: at Speed 2 the playhead crosses a line
every half bar of music and the lock keeps every second line on the music's "1" (TD4).
D2-5 OLD SHOWS (question 75). A saved clip with the old keys and `transportMode` BPM Sync loads with `clipBars = 0` and
`syncSpeed` = the step of the list nearest to `videoBeats / beatDivision` (video) or to 16 / `beatDivision` (sequence),
clamped to the list; it is then fitted like any clip (its out point may move in). A Timeline clip loads as it was. The two
old keys are read once and never written (TL-U18d). Said truthfully to him: an old BPM-synced video ignored the tempo (E4)
and now follows it; its end may be trimmed to whole groups of 4 bars.
IF HE EVER CHANGES 47 (short clips at 1 or 2 bars): `kMinBars`, `kBarStep` and the body of `fitBars` / `snapBars` -- one header,
TL-U33's table, question 71's rule.
Changes by file: src/render/ClipBarGrid.h (new); src/model/Clip.h/.cpp (`clipBars`, `syncSpeed`, `timelineRate`, load of the
old keys, `TransportState`); src/render/Renderer.cpp (`syncMedia`'s two BPM Sync branches read `frameSnap_`, push
`barSyncRate x syncSpeed`; Timeline pushes `effectiveClipSpeed(speed x timelineRate, master, false)`); src/MainComponent.cpp
(the fit, the pending-fit list, the length provider); src/api/ApiServer.cpp (`/api/composition` per clip: `transportMode`,
`clipBars`, `syncSpeed`, `timelineRate`, `inPoint`, `outPoint`, `loopMode`; per layer: `paused`; `/api/set_clip_param` gains
`transportMode`, `clipBars`, `syncSpeed`).

### TD4 SPEED (amendment D2-4; replaces DA-9)

VERIFIED: R1, R2, P3, E6 (`setSpeed` has no clamp), E7, E8.
BPM SYNC. `RelaxedFloat Clip::syncSpeed = 1` (saved), always one of `kSyncSpeedSteps = {0, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16}`
(data in ClipBarGrid.h). The rate pushed = `barSyncRate x syncSpeed` (x 1 + trim when locked). F36 its own field | runner-up:
reuse `speed`, snapped on a mode switch -- loses: a Timeline speed of 1.37 would be destroyed by one visit to BPM Sync.
- The panel's slider and the strip's S fader have nine evenly spaced detents (index 0 .. 8); a drag lands on the nearest
  detent, there is no value between two steps (P3: this is where his screenshots show the thumb). Default: index 4 ("1").
- Minus / plus: one step down / up; greyed at the ends. The number box shows "0", "1/8", "1/4", "1/2", "1", "2", "4", "8", "16";
  a typed number snaps to the nearest step (in ratio).
- 0: the picture stands on its frame; the clip is still playing (the play button stays lit, no pause is written); the lock is
  off. Stepping up again, the clip plays on from that frame and slides back onto the bar (TD6).
- The lock at a speed other than 1: the loop lasts B = 16 x bars / (4 x syncSpeed) = 4 x bars / syncSpeed beats, always a
  whole number for this list. `int lockPeriodBeats(int bars, float syncSpeed) noexcept;` = gcd(B, 4): 4 (the bar), 2 or 1
  beats; 0 at speed 0. Examples: 4 bars at 1 -> 16 beats -> bar lock; 4 bars at 8 -> 2 beats -> a 2-beat lock; 4 bars at 16
  -> 1 beat -> a beat lock; 12 bars at 8 -> 6 beats -> 2; any bars at 1/2 or below -> bar lock.
TIMELINE. Range 0 .. 4, linear, default 1, as today (E7, E8): "1" then sits at a quarter of the travel, where P3 shows it.
F37 keep 0 .. 4 | runner-up: Resolume's non-linear slider "towards 10" -- loses for now: its curve and its top are NOT
DOCUMENTED (R2), and nothing he said asks for more than 4 in Timeline (question 73). Minus / plus: 0.1. 0 = the picture stands.
THE PLAYER AT x8 AND x16 is not proven by reading: measured in SM (run M7, bar X6) before S4a is merged.
The master speed keeps applying to Timeline clips only (E4), as today.

### TD5 FIRES AND PAUSE (amendment D2-6; amends AM-3, AM-12)

VERIFIED: E2, E3, E5, E12, R6. Boris: "42 default"; "when you pause a clip and then fire it, it stays, paused"; "stays in
layer strip paused". Reading R16 (told to him, not corrected; its second sentence is INFERRED and is Boris check 6): the fired
clip stays on the frame where it was paused; a different clip fired on that paused layer shows its first frame and waits.
THE RULE. Pause is the LAYER's: `RelaxedBool Layer::paused = false` (runtime, not saved; a shared field: `Relaxed<T>`, a
static_assert pin, one TSan case -- Pitfall 63). `Clip::playing` keeps one meaning only: this clip is running (false = it
ended, or it was cleared). The intent pushed to the player is `clip.playing && !layer.paused`.
Why the layer (F38 | runner-up: the clip, as Resolume, R6). With pause on the clip the second sentence cannot hold (clip B has
its own setting), and the tail cannot tell "he paused it" from "a OneShot ended" or "the layer was cleared" -- the dead fire
the chain's AM-3 found (a clear, a momentary release and a OneShot end all leave `playing == false`). With pause on the layer
both sentences hold and the three cases stay apart. A clip belongs to one row, so "the clip's pause" and "its layer's pause"
differ only for another clip of the same row -- exactly the second sentence.
What each action does:

| action | layer not paused | layer paused |
|---|---|---|
| fire the clip the layer plays | restarts at its beginning (reversed: at its end), plays | NOTHING moves: it stays on its frame, paused; no stamp is minted |
| fire another clip of that row | it starts at its beginning, plays | it shows its first frame (reversed: its last) and waits; the stamp is minted, so the player seeks there |
| a column fire | every layer as the two rows above | the paused layer as the two cells above; the other layers start; the pause is not lifted |
| a queued (quantised) fire | released on its beat, then as above | the same, on its beat |
| autopilot, a routine, a take replay, REST, OSC, MIDI | as above (they enter the same two handlers: lint TL-L4) | as above |
| strip or panel: pause | sets `paused` | -- |
| strip or panel: play / play backwards | sets the direction | sets the direction and clears `paused`: the clip runs on from its frame |
| the layer's X (clear) | clears the layer | clears the layer AND the pause (question 74) |
| a OneShot that ended, fired again | plays again (chain check 6) | waits on its first frame |

`paused` is written by the strip's and the panel's buttons and by `clearActiveClip`'s caller on the message thread; no fire
writes it. The render sync: `Intent { bool wanted; bool held; bool paused; }`; `pushIntent` takes the layer's `paused` read once
(`pushIntent(clip, player, layerPaused)`), pushes `wanted && !held && !paused`; `writeBack` skips the play-state
compare-exchange and the OneShot stop while `held || paused` -- the hold's own rule (AM-12), so a pause can never be written
back as "ended". The lock is off while paused. `syncMedia` gets the layer's flag from its caller (S0 names the call sites that
know the layer; a clip drawn for a fade-out uses its own layer's flag).
The tail (AM-3 as amended): for a new activation or a re-fire that restarts: playhead to the start, `playing = true`, mint the
stamp. For a fire of the active clip on a paused layer: nothing. `hasBeenTriggered` is deleted; `triggerWillAutoPlay` reads:
the target exists, is not playing, and its layer is not paused.
Changes by file: model/Layer.h (`paused`; the tail takes the layer's flag), model/Clip.h, render/ClipTransportSync.h,
render/Renderer.cpp + the compositor call sites (the flag down to `syncMedia`), ui/LayerStrip.cpp and ui/ClipInspector.cpp
(the three buttons; lit state from the model on the timer, Pitfalls 41, 59), MainComponent.cpp (`triggerWillAutoPlay`, the
clear), api/ApiServer.cpp (`paused` per layer in `/api/composition`; `POST /api/debug/layer_pause {layer, paused}` test route).

### TD6 THE PLAYHEAD (amendments D2-7, D2-8; amend AM-11, DA-1, DA-4)

VERIFIED: E7 (:1372, :1396), E8 (:988), E9, E10. Boris: "there are two conditions here. 1 the in point and out point of the
timeline has not been set. In this condition, if you drag the play head, it cannot go beyond the edges of the time. 2 in
and/or out points have been moved. In this condition, if you drag the play head, it cannot be dragged outside of the in and
out points. You can click outside the timeline and In-N-Out points, but that won't do anything. The play head can only go to
the edges as they are defined."; "2 b"; "yes but this will be bars now"; question 33: B.
D2-7 THE PRESS AND THE DRAG (both surfaces, through the chain's one `scrubClip`):
- a press inside in..out moves the playhead there and takes the hold (reading R5);
- a press outside in..out (the dark part, or off the bar) does NOTHING: no scrub, no hold, nothing written. On the Clip tab a
  press on a pointer's own tab still grabs that pointer (TL-U13);
- a drag that began inside is clamped to [inPoint, outPoint - 1e-4] (DA-4, AM-13 unchanged);
- held still, the picture waits (DA-3); on release the clip plays on from there, never a seek back (DA-1's law).
D2-8 THE BAR LOCK = the chain's beat lock, aimed at the bar. In ClipBarGrid.h:
- `double showBarBeats(int beatInBar, double beatPhase) noexcept;` -- the music's place in its bar, 0 .. 4 beats;
- `double barLockErrorBeats(double clipBar, float syncSpeed, double showBarBeats, int periodBeats, bool backward) noexcept;`
  -- e = the clip's place (clipBar x 4 / syncSpeed, in show beats) minus the music's, wrapped into [-period/2, period/2);
- `float barLockTrim(double errorBeats) noexcept;` -- 0 inside `kBarLockDeadbandBeats` 0.03; else clamp(-e /
  `kBarLockTauBeats` 2.0, +/- `kBarLockMaxTrim` 0.06). The chain's three numbers, unchanged.
- In ClipTransportSync.h: `template <class Player> float barSyncSpeed(const Clip&, const Player&, const Intent&, float rate,
  double showBarBeats, bool clockLocked);` (the chain's `bpmSyncSpeed`, renamed). A trim only: never `seekTo`, never `restart`.
- Off (speed pushed = the rate exactly): not wanted, held, paused, tempo 0, tracker not LOCKED, length not known, speed step 0.
- ONE reader of the music's bar: `double Renderer::showBarBeats() const`, fed from `frameSnap_` (E4). When the beat-nudge lane
  has merged it returns the SHIFTED beat -- that one function body is the only line that changes.
THE ARITHMETIC (period 4 beats: at most 2 beats = half a bar to make up; 0.06 beat gained per beat until the error is
0.12 beat, then the 2-beat tail down to 0.03):
- worst case, 2 beats off: (2 - 0.12) / 0.06 + 2 x ln(0.12 / 0.03) = 31.3 + 2.8 = 34.1 beats.
- typical, 1 beat off (the mean of a blind drop): (1 - 0.12) / 0.06 + 2.8 = 17.4 beats.
- a quarter of a beat off: 4.9 beats (the chain's number, unchanged).

| tempo | worst (34.1 beats) | typical (17.4 beats) |
|---|---|---|
| 90 BPM | 22.7 s | 11.6 s |
| 120 BPM | 17.1 s | 8.7 s |
| 174 BPM | 11.8 s | 6.0 s |

WHAT HE SEES WHILE IT SLIDES: no jump. The picture runs up to 6 % fast (the clip was behind) or slow (ahead), always the
shorter way; the playhead meets each bar line a little nearer to the "1" than the line before; inside 0.03 beat the speed is
the exact rate again. Question 33 told him "up to about 15 seconds"; the law gives 17 at 120 and 23 at 90: difference D-6.
THE FALLBACK (F39): a JUMP ON THE NEXT BAR LINE | runner-up: a faster slide (12 %). The faster slide loses twice: if 6 % of
trim fails the smoothness bar, 12 % fails it harder; and it only halves a wait that a jump removes. The jump: no trim; on the
music's next "1" after a release, a realign or a wrap that finds the clip more than 0.1 beat off, ONE seek to the place the
clip would hold on the bar; then the steady rate. He would see: the clip plays on from where he dropped it, and on the next
"1" it cuts once, into time. Which of the two is built is SM's verdict (TD7), not this page's.

### TD7 THE MEASUREMENT FIRST (amendment D2-9; stage SM)

His question: "b can you program this reliably or should we change the plan?" Told to him (boris-clarify-42-44.md): B is
taken; it is measured on a test build before it is promised. Three things can make it unreliable, and reading settles none:
(i) THE BAR ITSELF. In Auto, `beatInBar` advances on DETECTED beats, not on time, and the downbeat detector may re-lock every
16 beats (E10). Each change of the "1" would send every synced clip on a slide of up to 34 beats. How often that happens on
real music is not known. In Manual the bar free-runs on phase wraps (E10 (a)) and should never slip (INFERRED).
(ii) THE PLAYER under a speed that changes every frame (the chain's FM-5, never run), and at x8 / x16 (E6: no clamp; not proven).
(iii) THE LAW on a real hop staircase and a wobbling tempo (the chain's FM-3, FM-4, never run).
WHAT IS BUILT, behind a test-only switch (process started with `--test-mode` AND the environment variable
`AUDIODNA_BARLOCK_PROBE=1`; with either absent no line of the product behaves differently -- lint TL-L6):
- src/render/ClipBarGrid.h, whole (pure; it is product code and stays whatever the verdict);
- src/render/BarLockProbe.h: a record (clip id, bars, speed step, enabled) and a ring of 131072 fixed-size samples, allocated
  once at start-up when the switch is on; the render thread writes one sample per frame (plain stores + one atomic index: no
  allocation, no lock, no system call); the http thread copies it out and drops the two newest samples;
- Renderer.cpp: in `syncMedia`'s video and sequence branches, one branch `if (probe && probe->names(clip->id))` that pushes
  `barSyncRate x step x (1 + trim)` instead of today's speed, and writes the sample;
- routes: `POST /api/debug/barlock {layer, column, bars, speed, enabled}`; `POST /api/debug/barlock_seek {pos}` (the seek the
  cue-point jump makes); `POST /api/debug/tempo {action, bpm}` (calls `applyTempoCommand`: there is no REST Tap, R10);
  `GET /api/debug/barlock_log?from=N`;
- `.harmony/probe-barlock.sh / .py` with a self-test on a recorded log and two mutated copies (RIG-RULES A2: instrument first).
A sample: the frame's time, `bpm`, `beatPhase`, `beatInBar`, `totalBeatCount`, `trackerState`, `trackerRequestSeq`,
`resyncBarOrigin`, the clip's playhead before the advance, the speed pushed, the trim, e, and flags (wrapped, held, paused).
Derived offline by the probe: c = (`totalBeatCount` - `beatInBar`) mod 4 (a change of c = the "1" moved against time); the
ANCHORED bar (follow a new c only after it has held for 8 beats in a row) and the e the lock would have had on it.
THE RUNS (Harmony's; her own test-mode instance, `open -g`, never an Output window, no synthetic input, own-pid quit; audio
from a FILE, which the app analyses without playing it, so nothing sounds on Boris's machine -- facts-beat-controls,
VERIFICATION C4). Fixture: the frame-coded ramp (10 s; marked 8 s = 4 bars) and one 1080p long-GOP clip.
- M1 exact clock: the injected beat driver at 120, 10 minutes, fired on a "1".
- M2 Manual: a typed tempo of 90, 120, 174 over a music file; 10 minutes each; fired on a "1" with Quantize on.
- M3 Auto: three music tracks near 90, 125 and 174 BPM, 10 minutes each (a 4-on-the-floor track, a breakbeat track, a track
  with a long breakdown). If the rig has no such files: BLOCKED, and Boris is asked for three tracks -- no verdict without it.
- M4 drops: Manual; 10 seeks at 120 (five about 1 beat off, five about 1.8 beats off, both signs), 4 each at 90 and 174; 44
  beats of log after each.
- M5 commands: Manual at 120: eight Taps at a 126 pace; a Resync in mid-bar; a tempo value 120 -> 128 -> 120; 44 beats after each.
- M6 a nudge, emulated: the driver steps its phase by +30 ms, later -30 ms (the nudge lane is not merged; its own row comes with it).
- M7 speed steps x8 and x16, both fixtures, 60 s each.
- M8 the fallback's own fact: ten seeks issued on a "1"; frames until the shown picture comes from the new place.
PRE-REGISTERED BARS (never loosened: met, or reported):
- X1 slips of the "1" in Auto (M3), outside the 8 beats after a command: at most 1 per 10 minutes on each track, raw. X1a: the same on the anchored bar.
- X2 holding (M1, M2, M3 with whichever bar passed X1): from 44 beats after the fire, |e| <= 0.10 beat in >= 95 % of frames (M1, M2: >= 99 %); in M2's fifth minute, 20 reads 3 s apart: >= 18 inside 0.10 beat (the chain's FM-4, in bar terms).
- X3 smoothness (M1, M2, M3): content rate over each 2 s window against the expected rate: >= 90 % of windows within 2 % (the chain's FM-5); M4b reported.
- X4 pull-in (M4): no playhead step back except across the loop point; the speed pushed within 6 % of the rate; |e| <= 0.10 beat by 40 beats after the seek in 22 of 22.
- X5 commands (M5, M6): no position step; settled as X4 by 40 beats (M6: by 8 beats); after the Taps c is reported (TD8).
- X6 x8, x16 (M7): content rate within 3 % of expected over 4 s windows; peak frame time <= 50 ms (probe-boxes k8's bar).
- X7 the tempo's wobble while LOCKED (M3): (max - min) <= 0.5 % of the mean over 60 s (the chain's FM-3).
- X8 (M8): the new picture is up within 3 frames in 10 of 10.
THE DECISION TABLE (pre-registered; the first row that matches decides):

| outcome | what is built | Harmony tells Boris |
|---|---|---|
| O1: X1 .. X7 met | S4c as ruled (the slide, on the raw bar) | "Yes: measured over 10 minutes at three tempos, a dropped clip slides back onto the bar in at most 17 seconds at 120 and stays there." |
| O2: X1 fails, X1a met, X2 .. X7 met on the anchored bar | S4c as ruled + the anchored bar (one small GL-thread record on the Renderer, `BarAnchor`; its own unit cases) | "Yes, with one condition: when the app itself moves its '1' in Auto, clips follow two bars later instead of at once." |
| O3: X1 and X1a fail (M2 met) | STOP AND ASK. Named fallback, built only on his word: the lock holds the clip on the BEAT in Auto (at most half a beat, 4.6 s at 120) and on the BAR in Manual / after a Resync | "In Auto the app moves its '1' too often for a clip to follow it. Either clips stay on the beat in Auto and on the bar when you tap and Resync, or we fix the '1' first. Which?" |
| O4: X2, X3 or X4 fails in M1 / M2 (the slide itself), X8 met | the fallback of TD6: a jump on the next "1" (its unit cases TL-U49j; TR21 / TR22 re-registered for it BEFORE it is built) | "The slow slide was not smooth enough. Instead the clip plays on from where you drop it and cuts once, on the next '1', into time." |
| O5: as O4 and X8 fails | STOP AND ASK; no change pre-decided | "Neither the slide nor the cut is clean on this player yet. A dropped clip keeps its tempo but stays off the bar until you fire it again (as Resolume), until we fix the player. OK for now?" |
| O6: X6 fails | STOP AND ASK; S4a waits | "The player cannot keep up at x16 (or x8). Stop the list at the highest speed that plays cleanly, or fix the player first?" |
| O7: X7 fails | STOP and report (the chain's FM-3 rule: a slew of the rate would fight the lock) | "The tempo the app reads wobbles more than the lock can take; I am looking at the tracker before building the lock." |

X5 failing alone is reported with the log and decided by an architect ruling (no pre-decided change). Harmony constraint: SM's
runs and verdict are Harmony's; S4c, S4d, S4e, S5a, S5b and S6 do not start before the verdict is written into the lane report.

### TD8 STAYING ON THE BAR (his "9 b")

Boris: "9 b" (the app keeps nudging it back by itself); "If we are shifted forward or back, everything that is connected to
BPM shifts forward or back. I mean everything."
WHAT CARRIES from the chain, unchanged: the lock is stateless and reads only this frame's snapshot and this frame's position
(no stored offset to go stale); a tempo VALUE changes only the rate, the phase is continuous (R10); a Tap, a Resync, a
confident detection re-place the beat and the clip SLIDES, never jumps; a Resync does not restart a clip; a trimmed loop's
lost frames at the wrap are pulled back (DA-1); a layer that is not drawn slides when it is drawn again; the tempo store
(AM-22).
WHAT "BAR" ADDS:
1. The beat-in-bar must be right, and today a Tap can make it wrong: a Tap in the second half of a beat completes the beat in
   `totalBeatCount` but leaves `beatInBar` where it was (E10; facts-beat-controls section 3 item 5), so the wheel -- and with it
   every synced clip -- loses one beat of the bar per such Tap. Amendment D2-11 (stage S4t): a realign that completes a beat
   on a Tap request also advances the bar's beat (the bookkeeping `advancePredictedBeat` already does for a predicted wrap).
   One function in BPMTracker.cpp, on the analysis thread, no allocation. A Resync already sets the "1" (E10). The detection
   path is left alone: on a detected beat `scoreBeat` advances it in the same hop (INFERRED from :234-236 and :343-346; SM's
   c column shows whether it holds).
2. In Auto the "1" is the detector's (TD7 (i)): measured, and the answer is O1 / O2 / O3.
3. A slide after a realign is up to 34 beats instead of 9: after a Resync in mid-bar every synced clip may run 6 % off for
   up to 17 s at 120. That is the cost of "slides, never jumps"; shown to him (check 14). A Resync that restarts synced clips
   (as Resolume's does, facts-resolume-transport Q8) is NOT in this lane and is not asked now.
4. The nudge: the clip reads the shifted beat through `Renderer::showBarBeats()` (TD6). A nudge of 30 ms at 120 is 0.06 beat:
   a slide of about 3 beats. The top bar's wheel shows the same shifted beat ("46 default"), so the playhead still meets the
   bar lines when the wheel shows "1".

### TD9 RANDOM AND BEATLOOPR (amendment D2-10; "build random and beatloopr")

VERIFIED: R4, R8, E5, E12.
LOOP STYLES. `Clip::LoopMode` keeps its saved numbers (Loop 0, PingPong 1, OneShot 2 -- E1) and gains Random 3,
PlayOnceEject 4. The menu reads "Loop", "Ping Pong", "Random", "Play Once and Eject", "Play Once and Hold"; today's "One Shot"
IS "Play Once and Hold" (it stops on its last frame, E6), so old shows keep their behaviour.
- Play Once and Eject: when the clip has ended (the layer's active clip, `playing == false`, its stamp seen, layer not paused)
  the layer is cleared as its X clears it, by `clearActiveClip(rows, onlyIfActive)` on the thread that already fires queued
  clips (S0 names the site; CAS, no lock). INFERRED from R4's "one shot samples that you want to punch in" that the layer
  empties; the page says only "eject".
- Random (R4): every Interval the playhead jumps to a random point at most Distance away, inside in..out, and plays on.
  `RelaxedFloat Clip::randomInterval`, `randomDistance` (saved). In Timeline both are seconds (number, minus, plus; defaults
  1 s and the marked length). In BPM Sync both are picked from one data list in ClipBarGrid.h, `kBarFractions = {1/16, 1/8,
  1/4, 1/2, 1, 2, 4}` bars (his own Quantize words: "Quantize 1 bar, 1/2 bar, 1/4, 1/8, 1/6"); the jump falls on the music's
  grid line of that Interval, and its length is a whole number of BEATS, so the clip stays on the beat; the lock's period is
  1 beat while Random is on. Pure helper `double randomJumpTarget(double playhead, float in, float out, double distanceNorm,
  uint32_t& rngState) noexcept;` (xorshift; GL thread; state on the player: one uint32 and one double, no allocation).
  What "Distance" means exactly (around the playhead, as built, or anywhere in the clip) is NOT DOCUMENTED beyond R4.
BEATLOOPR (BPM Sync only; R8). A row of buttons under the Bars row: "Off", then the options, then "Catch Up" (a toggle).
- The options are DATA: `kBeatLooprOptions` in ClipBarGrid.h, default = `kBarFractions` shown as "1/16" .. "4" with the
  row's caption "BeatLoopr (bars)". Resolume's own list is NOT DOCUMENTED: request RQ-1 asks Boris for one screenshot of the
  row in his Arena; the list then changes in that one array (and question 72 decides the unit shown).
- Pressing an option loops that length from where the playhead is now; pressing it again or "Off" ends it. Nothing is stored
  in the show ("It doesn't store loops start and end points permanently in the clip data", R8): `Relaxed<uint64_t>
  Clip::beatLoop` (runtime; option index, a press stamp, the Catch Up bit; 0 = off; pin + TSan case). The render sync loops
  [start, start + length] instead of in..out; a loop of a whole number of beats keeps the clip on the beat.
- Off without Catch Up: the clip plays on from where it is and slides onto the bar (his rule 33 B; Resolume leaves it there).
  Off with Catch Up: ONE seek to where the playhead would have been (the place the bar gives), then on -- the only seek the
  lock family makes, and only on his press.
- A fire, a scrub or a mode switch ends the loop (R8's old text: it turns off on a cue-point jump).
In BARS or beats: bars, as fractions -- "The only place we see beats is in the circle with 4 positions in top bar". F40 bar
fractions | runner-up: Resolume's beat numbers -- loses to that sentence; question 72 lets him say otherwise.

### TD10 UNDO

VERIFIED against his words of 2026-10-04: "cmd-z does not affect anything in layer strip: play, play reverse, pause,
transparency, bypass, solo, etc. if it is  the layer strip, cmd-z does not affect it. If a clip is triggered and plays, it is
not affected."; "default is ok. If a layer strip is deleted, user can ctrl-z to get it back, but clip is not playing";
"All defaults good except for these:" -- he named none of 21, 22, 23, so each took its default (Harmony's record line,
binding-decisions.md:789, not his words).
- Cmd+Z never touches the layer strip: STANDS (DA-5). His list names play, play reverse, pause: the strip fields gain the
  layer's `paused`; the playing clip's `TransportState` gains `clipBars`, `syncSpeed`, `timelineRate`, `randomInterval`,
  `randomDistance` and the two new loop styles (AM-6 as amended). Lint TL-L2 also bans `paused =` in src/core/*Commands.h.
- A removed layer comes back not playing: STANDS (DA-6); `restoredForUndo` returns it with an empty tuple, solo off, NOT paused.
- A fire is not an Undo step: STANDS (AM-6, TL-UC1).
- 21 (an added, empty layer goes with Cmd+Z): STANDS. 22 (a layer move ends the history): STANDS; S1m is not built.
  23 (an effect comes off with Cmd+Z, also while the layer plays): STANDS.
- New in this delta, all "no step" like every transport edit: the mode switch and its fit (the out point the app moved in is
  not put back by Cmd+Z), Bars, Speed, Duration, the loop style, Random's two numbers, BeatLoopr. Check 20 shows him.

### TD11 STAGES AND ORDER -- see section 4 (stages), 5 (gates). Lane boundaries:

- THE OUTPUTS LANE owns src/output/*, the output windows, Syphon's settings and the per-screen Delay. This lane touches none
  of them. The clip lock lives in the canvas's time: an output's Delay comes after it and is never read by it.
- THE NUDGE LANE owns the top bar (src/ui/TopBar.*), the shifted beat and where it is stored. This lane does not touch
  TopBar.*, FeatureSnapshot.h or AnalysisThread.*.
- BOTH OTHER LANES and this one edit src/MainComponent.cpp and the route table of src/api/ApiServer.cpp: whichever merges
  second rebases (S0's list is re-read). Neither other lane may touch render/ClipTransportSync.h, render/ClipBarGrid.h, the
  two BPM Sync branches of `Renderer::syncMedia`, or model/Layer.h's tail.
- WAITS FOR THE NUDGE LANE TO MERGE: stage S4t (it edits BPMTracker.cpp's realign path, which the nudge lane's plan may also
  claim: Harmony decision H-T1 -- default: S4t is built after the nudge lane has merged, or is handed to it); live row TR24
  (the nudge row); the body of `Renderer::showBarBeats()`. Nothing else waits: if this lane merges first, the nudge lane's
  stage list carries those three.

Forks of this delta (id | choice | runner-up):
F31 Duration is its own factor | Duration writes Speed. F32 the panel's buttons act on the playing layer | a clip-stored pause.
F33 `clipBars` an int | a float of beats. F34 the fit uses the constant 120 | the app's tempo then. F35 a 0.05 s fit
tolerance | none. F36 `syncSpeed` its own field | reuse `speed`. F37 Timeline Speed stays 0..4 | Resolume's "towards 10".
F38 pause on the layer | pause on the clip. F39 fallback = one jump on the next "1" | a faster slide. F40 Random / BeatLoopr
in bar fractions | in beats. F41 the instrument runs the real lock on a real player behind a switch | an offline simulation
on a logged beat clock only (loses: it cannot answer (ii), the player). F42 the lock's period = gcd(loop beats, 4) | bar lock
only at Speed 1 (loses: a 4-bar clip at x8 would have no lock at all). F43 "/2" greyed when half is not allowed | round down
to 4. F44 old shows fitted on open | old clips left as they are. F45 bar lines thin by powers of two | none when dense
(the chain's H-D2, kept). F46 a press outside does nothing | lands on the nearer point (his word decides). F47 S4t after
the nudge lane | built here at once.

## 4 STAGES + ORDER (replaces ruling-transport-delta1.md section 4)

ONE lane, based on main (nothing waits on the sync dial: H-17). One builder context per stage, in this order. Every stage:
its cases are written first and shown RED by id on the stage's base (a case that never failed is struck and reported), then
GREEN at its head. A builder never runs a live row, never gives a gate verdict, never launches the app outside the unit
binaries: every live row, every measurement and every verdict is Harmony's.

- S0 RE-BASE NOTES (no code). The chain's S0 list, minus everything about the sync-dial lane (BeatLead, the phase the release
  uses), plus: the call sites of `syncMedia` that know the LAYER (for the pause flag) and the GL-thread site that releases
  queued clips (for Eject); what `getDuration()` returns for each fixture (the fit's tolerance); whether PerfState's restore or
  a routine writes `playing` directly; whether the capture route can show an open combo popup (W3); the frame-time variable
  the instrument's sample uses; the pixel widths of both bars and the bar count from which lines thin; whether the nudge
  lane's plan claims BPMTracker's realign path (H-T1); the music files on the rig for M3 (H-T2); recount of the `^TL-` pin.
  Harmony constraint: freeze the RED arm (a copy of the pre-lane main app bundle); FM-2 on it before S2. Proves: the table.
- SM THE INSTRUMENT (TD7; the FIRST code stage). Owns: src/render/ClipBarGrid.h (new), src/render/BarLockProbe.h (new),
  src/render/Renderer.h/.cpp (the probe branch only), src/api/ApiServer.cpp (four test routes), src/MainComponent.cpp (the
  routes' wiring), tests/test_clip_bar_grid.cpp (new), tests/test_render_thread_lint.cpp (TL-L6), .harmony/probe-barlock.*.
  Proves: TL-U32, TL-U33, TL-U34, TL-U40 .. TL-U43, TL-L6; the probe's self-test and its two mutated copies.
  THEN HARMONY: runs M1 .. M8, writes X1 .. X8 and the outcome O1 .. O7 into the lane report, tells Boris the sentence of that
  row. S4c, S4d, S4e, S5a, S5b, S6 do not start before that line exists. S1 .. S4a may be built while she measures.
- S1 UNDO-LIVE + THE STRIP RULE. The chain's S1 (AM-6 .. AM-9, DA-5, DA-6, DA-7), files as the chain. `TransportState` is
  written with today's fields; S2 and S4a add theirs by the chain's clause rule. Proves: the chain's 27 S1 cases; lint TL-L2; G-U5.
- S2 FIRES + PAUSE. The chain's S2 (AM-1, AM-2, AM-4, AM-5) with D2-6. Owns: model/Clip.h, model/Layer.h,
  render/ClipTransportSync.h, media/VideoPlayer.*, media/ImageSequence.* (`restart`), render/Renderer.cpp and the compositor
  call sites (the flag), binding/BindingManager.*, ui/LayerStrip.cpp + ui/ClipInspector.cpp (the three buttons only),
  MainComponent.cpp (handlers, `triggerWillAutoPlay`, the clear), ApiServer.cpp (`paused`, the pause test route), tests.
  Proves: the chain's 15 S2 cases (TL-U2 as amended) + TL-U60 .. TL-U64; lints TL-L1 (S2 form), TL-L1b, TL-L4; TL-UC17 gains
  its pause clause; G-U4 at 7. Then Harmony measures FM-1.
- S2b (only if FM-1 is over its bar). As the chain.
- S3 DRAG. The chain's S3 (AM-11, AM-13, DA-4) with D2-7. Proves: TL-U10, TL-U13 (with its clause), TL-U14, TL-U15, TL-U15b,
  TL-U65; lints TL-L3, TL-L1 final.
- S3h HOLD. As the chain (AM-12 with `Intent::paused` already there from S2). Proves: TL-U11 .. TL-U12c; MU-H1 .. MU-H3; G-U4 at 8.
- S4a BARS ENGINE, additive (D2-3, D2-4, D2-5, F31). Owns: model/Clip.h/.cpp, render/Renderer.cpp (the two BPM Sync branches
  read `frameSnap_`; the rate), MainComponent.cpp (the fit, the pending list, the length provider), ApiServer.cpp (the REST
  fields), tests. The old widgets keep writing the two old fields, which nothing reads, for this stage only (AM-15's shape).
  Proves: TL-U35a .. c, TL-U36, TL-U36b, TL-U37, TL-U38, TL-U39; TL-UC3b gains its clause; G-U4 at 9. NOT merged to main
  before M7's bar X6 is met (O6).
- S4c THE LOCK, as SM's outcome says (D2-8). O1: `barSyncSpeed`, `movingBackward` on both players, the two branches. O2: the
  same + `BarAnchor`. O4: the jump (its cases registered first). O3 / O5 / O7: not built until ruled. Owns:
  render/ClipBarGrid.h, render/ClipTransportSync.h, render/Renderer.cpp, media/VideoPlayer.*, media/ImageSequence.*, tests.
  The probe branch of SM is DELETED here (the product path replaces it; `BarLockProbe`'s ring and routes stay, test mode
  only, as TR20 .. TR23's reader). Proves: TL-U44 .. TL-U49 (8), TL-U36 gains its clause, lint TL-L5; `.harmony/probe-video.sh`
  whole. Then Harmony re-runs M2 (one tempo) and M4 on the lane build: X2, X3, X4 again, same bars.
- S4t TAP KEEPS THE BAR (D2-11; after the nudge lane has merged, or handed to it: H-T1). Owns: src/analysis/BPMTracker.cpp,
  its test. Proves: TL-U70.
- S4d LOOP STYLES (D2-10). Owns: model/Clip.h/.cpp (`LoopMode`, the two Random fields), render/ClipTransportSync.h
  (the Random jump), media players (the mapping of the two Play Once styles), the GL-thread eject site, tests.
  Proves: TL-U71 .. TL-U74.
- S4e BEATLOOPR (D2-10). Owns: model/Clip.h (`beatLoop`), render/ClipTransportSync.h, render/ClipBarGrid.h (the list), tests.
  Proves: TL-U75 .. TL-U78; G-U4 at 10.
- S5a THE PANEL (D2-1, D2-2). Owns: ui/ClipInspector.h/.cpp, the two Clip-tab test routes, model/Clip.h/.cpp (the two old
  fields and their keys leave), the tests of AM-15's rows. Proves: TL-U50, TL-U51, TL-U52, TL-U18d.
- S5b THE STRIP (D2-2, D2-4). Owns: ui/LayerStrip.h/.cpp, ui/DeckView.* (the forwards), the strip dump route, tests.
  Proves: TL-U53, TL-U54. The stage ends at the VISUAL WORK GATE (5.5) for S5a + S5b together, not at a commit: a capture
  builder, then five critic seats, then Harmony; Boris sees nothing before it passes.
- S6 PROBE + DOCS. `.harmony/probe-transport.sh / .py / .json` (29 rows); docs: docs/claude/performance-controls.md (fires,
  the layer's pause, the strip rule, layer addressing), docs/claude/rendering.md (the bar fit, the lock, ClipBarGrid.h, the
  loop styles, BeatLoopr), docs/claude/architecture.md (the Clip and Layer field tables), docs/claude/testing-eyes.md (the
  new test routes), CLAUDE.md ("Transport state" under UI Patterns is false after S2 and is rewritten; Pitfalls 2 and 7
  re-worded in docs/claude/pitfalls.md), .harmony/APP-INVENTORY.md; three new Pitfalls (Harmony assigns the numbers; 68 is
  free): "a BPM-synced clip is trimmed, never sought; bars, lines and lock come from ClipBarGrid.h; one reader of the music's
  bar", "pause is the layer's; Clip::playing means running; a fire never lifts a pause", and the chain's "no Undo command
  writes a strip field ..."; Boris's page (sections 6 and 7 here). Proves: a full local run on the lane build and the RED table.
- S1m: not built (question 22 kept its default).

What Harmony runs herself: SM's M1 .. M8 and the verdict; FM-1, FM-2; every row of 5.2, 5.3, 5.4; the visual gate's ruling;
the re-run of M2 / M4 after S4c; six mutants of her choice (always MU-4, MU-18, MU-21, MU-48, MU-71, MU-74).
Harmony's decisions (each with a default): H-T1 S4t's owner -- default: built here after the nudge lane merges. H-T2 the three
tracks of M3 -- default: she names them in the gate script before the run; none on the rig = BLOCKED and Boris is asked.
H-T3 W3 (the open mode menu) is registered DUMP-ONLY if S0 finds the capture route cannot show a popup; it is then Boris
check 1. H-T4 the lane may merge in two parts (after S3h: undo, fires, pause, drag, hold; then the rest) -- default: one merge.

## 5 TESTS + GATE ROWS (pre-registered here; Harmony copies strings only from here and from the chain rows that STAND)

Harmony constraint (unchanged): RED arm = the frozen pre-lane main app; a flake verdict needs >= 5 runs per arm; a bar is
never loosened: met, or reported; a row whose bar sits inside 4 x the measured noise is BLOCKED until she waives it in writing.

### 5.1 Unit cases (TEST_CASE names begin with the id). RED arm of every case: the stage's base, by id, in the builder's log.

SM (RED arm: ClipBarGrid.h as a stub whose functions return 0):
- TL-U32 "bar sync rate: 8 s over 4 bars at 120 is 1, at 90 is 0.75; 16 s over 4 bars at 120 is 2; 2 s over 4 bars at 120 is 0.25"
- TL-U33 "the fit: 2.0 s gives 4 bars, out unchanged; 7.3 s gives 4, unchanged; 8.0 s gives 4; 15.98 s gives 8, unchanged;
  16.016 s gives 8, out at 16.0 s; 30 s gives 12, out at 24 s; 45 s gives 20, out at 40 s; 60 s gives 28, out at 56 s; a
  trimmed clip is fitted on its in..out length; typed 6 gives 8, 5 gives 4, 1 gives 4, 13 gives 12, 5000 gives 4096"
- TL-U34 "bar lines: line b sits where the clip bar is b; 3 lines for 4 bars, 11 for 12, none for a clip not fitted; lines
  closer than 3 px thin to every 2nd, 4th, 8th bar and never vanish (64 bars on a 150 px bar: every 2nd bar, 31 lines); a
  trimmed clip's lines lie inside in..out"
- TL-U40 "bar lock error: clip bar 1.25 against the music on its 1 is +1 beat; 3.75 is -1 beat; at speed 2 clip bar 2.5
  against beat 1 is 0; running backward mirrors it; never outside [-period / 2, period / 2)"
- TL-U41 "bar lock trim: zero inside 0.03 beat; minus half the error outside it; never beyond 0.06 either way"
- TL-U42 "lock period: 4 bars at speed 1 is 4 beats; 4 bars at 8 is 2; 4 bars at 16 is 1; 12 bars at 8 is 2; any bars at 1/2
  is 4; speed 0 is none"
- TL-U43 "a clip started 2 beats off the bar is within 0.03 beat (+ 1e-6) from 40 beats on, 1 beat off from 22 beats on;
  its position never steps back; the speed stays within 6 % of the rate"
- TL-L6 (lint) "`BarLockProbe` is constructed only where both `--test-mode` and AUDIODNA_BARLOCK_PROBE are checked; in
  src/render/Renderer.cpp every use of the probe is inside a null test; BarLockProbe.h holds no `new`, no `std::vector`
  growth call, no mutex, no file call in its write path"
S2: TL-U2 as amended (TD1 C), and
- TL-U60 "a fire of the clip a paused layer plays changes nothing: no stamp, the playhead and the play state stay"
- TL-U61 "another clip fired on a paused layer: its stamp is minted, its playhead is its start, the intent pushed is false,
  and after the sync the player sits on the start frame with the clip's play state still true"
- TL-U62 "a pause is never written back as ended: ten syncs of a paused layer leave the clip's play state true, a OneShot
  at its last frame is not stopped, and play carries on from the same frame"
- TL-U63 [tsan] "a message-thread pause against the GL-side read of the layer's pause"
- TL-U64 "a column fire leaves a paused layer paused and starts the other layers; clearing the layer lifts its pause"
S3: TL-U13 gains "a press outside in..out, off the pointer tabs, does nothing"; TL-U65 "layer strip: a press outside in..out
makes no scrub call and takes no hold; a drag that began inside is clamped at the points".
S4a:
- TL-U35a "an old BPM-synced video 8 / 4 loads not fitted, with sync speed 2" | TL-U35b "an old BPM-synced sequence over 4
  beats loads with sync speed 4, over 1 beat with 16, over a quarter beat with 16" | TL-U35c "an old Timeline clip loads as
  it was: not fitted, sync speed 1, timeline rate 1"
- TL-U36 "in BPM Sync the speed pushed is the bar rate times the speed step and ignores clip speed and master speed; a clip
  not fitted plays at the rate of its fit and the model is not written; in Timeline it is speed times timeline rate, with
  the master speed" (S4c adds: "; with the lock on it is that times one plus the trim, with it off exactly that")
- TL-U36b [tsan] "a message-thread bars write against the GL-side rate read"
- TL-U37 "the first switch to BPM Sync writes the bars and the out point once, at 120 whatever the show tempo (set to 90 in
  the case); a second switch keeps both; a length not known stays pending and is fitted on the first tick that knows it; a
  right-click on Bars fits again"
- TL-U38 "speed steps: plus and minus walk 0, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16 and stop at the ends; a typed 3 gives 4; a typed
  0.3 gives 1/4; at 0 the clip's play state stays true"
- TL-U39 "Duration: typing 4 on an 8 s clip gives timeline rate 2 and leaves Speed; moving the out point changes the
  number and not the rate; /2 then x2 gives the same number"
S4c (the fakes are the chain's, binding: a player whose seek is a request and whose wrap keeps the overshoot; the real
`pushIntent`, `barSyncSpeed`, `writeBack` in the renderer's order):
- TL-U44 "the lock is off and the speed pushed is exactly the rate: a paused layer, a held clip, a hold cleared between
  pushIntent and the push, a tempo of 0, a tracker that is not locked, a length not known, speed step 0; a tracker state
  that flips every frame keeps the speed within 6 % and never steps the position"
- TL-U45 "a tempo change from 120 to 90 on a locked clip: the error stays inside 0.05 beat" | TL-U45b "a realign of 2 beats:
  no position step; within 0.03 beat (+ 1e-6) from 40 beats on" | TL-U45c "a bar position published every 512 samples at
  48 kHz and read at 60 frames a second, at 90, 120, 174 and 200 BPM, on a locked 4-bar clip over 96 beats: in the last 64
  the speed pushed is exactly the rate in at least 95 % of the frames"
- TL-U46 "a trimmed loop, in 0.2, out 0.7, 4 bars, at 200 BPM through the real write-back and a player that seeks as the
  video does: from the second wrap on the error is within 0.03 beat (+ 1e-6) in the last beat before every wrap and never
  beyond 0.16; the trim is never below 0 nor above 0.06 (+ 1e-6)"
- TL-U47 "a reversed 4-bar clip meets its lines on the bar; a ping-pong keeps the lock through both turns with no position
  step; on the frame a clip is reversed the error is taken with the new direction"
- TL-U48 "after a drop the clip plays on from the drop point: the lock asks for no seek, and a quarter of a beat later the
  position is the drop point plus a quarter of a beat, within 6 %"
- TL-U49 "plus 4 bars, x2, /2, a moved out point and a speed step on a locked clip: no position step; within 0.03 beat
  (+ 1e-6) from 40 beats on"
- (O2 only) TL-U49a "the anchored bar follows a moved 1 only after it has held 8 beats; a Resync is followed at once".
  (O4 only) TL-U49j, registered in the lane report before S4c starts.
S4t: TL-U70 "a Tap in the second half of a beat advances the beat of the bar with the beat count; in the first half neither
moves; eight Taps leave (beat count - beat of the bar) mod 4 unchanged; a Resync still gives beat 0". RED arm: main.
S4d: TL-U71 "Play Once and Hold stops on the last frame and the layer keeps the clip; Play Once and Eject clears the layer
once, only when its clip ended by itself: not while paused, not while held, not for another clip" | TL-U72 "Random in
Timeline: one jump per Interval of clip time, every target inside in..out and within Distance; the same seed gives the same
jumps" | TL-U73 "Random in BPM Sync: a jump falls on the music's grid of the Interval and moves a whole number of beats; the
error against the beat is the same before and after" | TL-U74 "an old show's One Shot loads as Play Once and Hold; the two
new styles save and load".
S4e: TL-U75 "BeatLoopr: an option loops its length from the playhead at the press; the same option again, or Off, ends it;
a saved show holds no trace of it" | TL-U76 "Off without Catch Up asks for no seek; with Catch Up for exactly one, to the
clip's place on the bar" | TL-U77 [tsan] "a message-thread press against the GL-side read of the loop word" | TL-U78 "a
fire, a scrub and a mode switch end the loop".
S5a: TL-U50 "Clip tab: the mode menu has six items, four of them disabled; Timeline shows Speed and Duration, BPM Sync shows
Speed, Bars and the BeatLoopr row; no widget text or painted string of the section is Beats, BPM, Reverse, Restart, Continue
or Relative" | TL-U51 "Bars row: minus and plus step by 4; /2 is enabled only on a multiple of 8; a typed 6 shows 8; the
refresh sets a box only when its text changed and never takes the focus from a box being typed in" | TL-U52 "readout: seconds
and frames from the playhead and the frame rate; a click shows the time left with a minus sign; the paint key changes only
when the painted text changes".
S5b: TL-U53 "layer strip: in BPM Sync the transport view carries the bar lines and repaints only when a line or the playhead
moves a pixel; in Timeline none; the length is asked in refresh, when the playing clip changes, and at most on every 15th
tick while it is 0" | TL-U54 "layer strip: in BPM Sync the S fader has nine detents and the forward button steps one up; in
Timeline it runs 0 to 4 and the forward button doubles; the lit state of back, pause and play follows the layer's pause and
the clip's direction and is set only when it changes".
Counts: the chain's 77 minus 23 dropped (TD1 C) plus 43 new = 97 cases; lints 7 (TL-L1, L1b, L2, L3, L4, L5, L6).
Mutants (G-U3; each alone on the lane head turns the named case RED): MU-60 a fire lifts the pause -> TL-U61, TL-U64 (live:
TR25). MU-61 a fire of the active clip restarts on a paused layer -> TL-U60 (TR25). MU-62 the write-back runs while paused
-> TL-U62. MU-63 a press outside lands on the nearer point -> TL-U65, TL-U13. MU-64 the fit uses the show tempo -> TL-U37
(TR26). MU-65 the fit never moves the out point -> TL-U33 (TR26). MU-66 no fit tolerance -> TL-U33. MU-67 the rate ignores
the speed step -> TL-U36 (TR27). MU-68 `clipBars` a plain int -> TL-U36b and the static_assert. MU-69 the fit runs on every
switch -> TL-U37. MU-70 an old clip loads with sync speed 1 -> TL-U35a, TL-U35b (TR10). MU-71 the trim's sign flipped ->
TL-U41, TL-U43 (TR20, TR22). MU-72 no deadband -> TL-U41, TL-U45c. MU-73 no limit -> TL-U41. MU-74 the period is always one
beat -> TL-U40, TL-U43 (TR20, TR23). MU-75 the lock runs for a paused or held clip -> TL-U44. MU-76 a seek to the place on
the bar after a release -> TL-U48 (TR22). MU-77 the lock does its own `featureBus_.read()` -> TL-L5. MU-78 the lines use the
whole file -> TL-U34 (C10, TR23). MU-79 the Tap fix reverted -> TL-U70. MU-80 Eject ignores the pause -> TL-U71. MU-81 a
Random target outside in..out -> TL-U72 (TR28). MU-82 a Random jump of a fraction of a beat -> TL-U73. MU-83 Off always
seeks -> TL-U76 (TR29). MU-84 the loop word is saved -> TL-U75. MU-85 the refresh sets a box every tick -> TL-U51. MU-86
/2 enabled at 12 -> TL-U51 (C5). MU-87 the S fader stays continuous in BPM Sync -> TL-U54 (C14). MU-88 the lines never thin
-> TL-U34, TL-U53.

### 5.2 Unit gates
- G-U1 `ctest --test-dir build -R '^TL-' --no-tests=error --output-on-failure` -> `100% tests passed, 0 tests failed out of N`
  with N >= 104 (105 if S2b is built; 103 if S4t is handed to the nudge lane), AND each id appears as Passed. RED arm: the
  per-stage RED logs, by id.
- G-U1b, G-U2, G-U5, G-N1: as the chain (G-U1b's list gains test_clip_bar_grid). G-U3: the chain's standing mutants + MU-60 ..
  MU-88. G-U4 `.harmony/probe-tsan-unit.sh` -> exit 0 with `EXPECTED_TSAN_CASES` raised from 5 to 10 (TL-U8, TL-U11c, TL-U36b,
  TL-U63, TL-U77); RED: each field declared plain makes its case report a race (builder's log).
- G-P1 (new) the product is untouched by the instrument: the lane build started WITHOUT the environment variable answers
  `GET /api/debug/barlock_log` with 404, and TL-L6 passes. RED arm: the builder shows once that removing one null test turns
  TL-L6 RED.

### 5.3 Live rows -- `.harmony/probe-transport.sh`; final line `PROBE-TRANSPORT GREEN`; pin `EXPECTED_ROWS=29`
Launch rules, fixtures, tolerance, flake rule: the chain's. The beat driver: the chain's, also posting `beatInBar` (E11);
"driver bar position" = its beat-in-bar + phase, 0 .. 4. e = the clip's place minus the driver's, in beats, period 4.
STAND as written in the chain (19 rows with the three below): TR2, TR3, TR4, TR5, TR5b, TR6, TR6b, TR7, TR7b, TR7c, TR7d,
TR8, TR11, TR12, TR13a, TR14. AMENDED:
- TR9 bar_rate_follows_tempo. The old-key composition; the 10 s ramp (fitted: 4 bars, out 0.8). `set_bpm 120`; fire; r1 over
  2 s; `set_bpm 90`; 0.5 s; r2 over 2 s. Bar: r1 in [0.97, 1.03], r2 in [0.72, 0.78]. RED arm: r2 about 1.0.
- TR10 old_show_speeds. (a) the old sequence (8 frames, 2.5 fps, 4 beats) at `set_bpm 120`: `syncSpeed` == 4 (lane arm) and
  one cycle = 2.0 s +/- 5 % on BOTH arms. (b) the old video 8 / 4 on the 10 s ramp: `syncSpeed` == 2, `clipBars` == 4,
  `outPoint` in [0.799, 0.801] (lane arm); at 120 the rate is in [1.94, 2.06] on both arms; at 90 the lane arm's is in
  [1.45, 1.55] and the RED arm's stays in [1.94, 2.06].
- TR13b as the chain with `set_clip_param clipBars 8` for `clipBpm 240`: r1, r2, r3 in [0.47, 0.53], `clipBars` == 8 throughout.
NEW:
- TR20 bar_lock_pulls_in. Driver at 60; the fitted ramp (rate 0.5); Quantize off; fire at driver bar position [1.20, 1.30].
  Bars: (i) a read within 0.5 s of the fire has |e| >= 1.0 beat; (ii) no playhead read is below the one before it except
  across the wrap, and the content rate over every 2 s window is in [0.46, 0.54]; (iii) from 30 beats after the fire, five
  reads 0.5 s apart each have |e| <= 0.10. RED arm (frozen app, TR9's composition): (iii) fails, |e| about 1.25 (INFERRED;
  if it passes the row is a guard). Teeth: MU-71; MU-74 (the clip rests one beat off the bar).
- TR21 bar_lock_tempo_and_realign. From TR20's locked state. (a) the driver steps to 90, phase continuous: every read of the
  next 8 beats has |e| <= 0.15, after them <= 0.10; the rate over 2 s is in [0.72, 0.78]. (b) the driver adds 2 beats to its
  bar position: no read below the one before except across the wrap; the rate over every 2 s window within 8 % of 0.75; from
  40 beats later |e| <= 0.10. RED arm: (a)'s rate stays about 1.0.
- TR22 bar_lock_after_a_drop. TR20's fixture, fired on a "1", left 16 beats. Strip scrub down 0.35; four reads over 1 s
  within one frame of 0.35; up at a driver bar position that leaves the clip about 1 beat ahead. Bars: (i) the read 0.5 s
  after the up is in [0.365, 0.39]; (ii) a read within 0.3 s of the up has |e| >= 0.8; (iii) from 30 beats after the up,
  five reads 0.5 s apart each have |e| <= 0.10. GUARD (new routes). Teeth: MU-76 (i, ii); MU-71 (iii).
- TR23 lines_on_the_bar. The ramp with in 0.1, out 0.9 (8 s: 4 bars, 3 lines), fired at driver bar position [1.20, 1.30];
  wait 30 beats. Bars: (i) on both dumps the line xs equal `barLinesOf` of the model at that surface's width, +/- 1 px;
  (ii) at 8 instants where the driver's bar position is within 0.02 beat of a "1" the playhead x on each surface is within
  max(3 px, 0.04 of one bar's width) of a line or of the in pointer. GUARD. Teeth: MU-78 (i); MU-74 (ii).
- TR25 paused_fire. (a) fire a ramp; 1 s; pause the layer (test route); read p0; fire the same clip; 0.5 s: the playhead is
  within one frame of p0 and `paused` is true. (b) fire another clip of that row: two reads 0.3 s apart are equal and within
  one frame of its in point; `paused` true. (c) play: two reads 0.3 s apart differ by >= 0.01. GUARD (the field is new);
  RED arm for (a) on the frozen app with a clip-level pause through `set_clip_param`: reported, not judged. Teeth: MU-60, MU-61.
- TR26 fit_on_switch. The 10 s ramp, Timeline, `set_bpm 90`; switch to BPM Sync: within 0.5 s `clipBars` == 4 and `outPoint`
  in [0.799, 0.801]; the rate over 2 s in [0.72, 0.78]; to Timeline and back: both unchanged. GUARD. Teeth: MU-64 (the out
  point stays 1.0), MU-65.
- TR27 speed_steps. BPM Sync at `set_bpm 120`: `syncSpeed` 2 -> rate in [1.94, 2.06]; 1/2 -> [0.47, 0.53]; 0 -> two reads
  0.5 s apart equal and `playing` true; 16 -> [15.2, 16.8] over 2 s. GUARD. Teeth: MU-67.
- TR28 random_jumps. Timeline, Random, Interval 1 s, Distance 2 s, in 0.1, out 0.9, 10 s of reads at 20 Hz: at least 7 jumps
  (a step between two reads that the rate does not explain) and every read inside [0.1, 0.9]. GUARD. Teeth: MU-81.
- TR29 beatloopr. Driver at 60, locked; press option "1" at playhead p: every read of the next 3 bars is inside [p, p + one
  bar's width + one frame]; Off with Catch Up: exactly one step, then |e| <= 0.10 within 2 beats; again with Catch Up off:
  no step larger than one frame at Off. GUARD. Teeth: MU-83.
- TR30 play_once_and_eject. Out 0.3, style Play Once and Eject; fire: within 4 s the layer's `activeClip` is empty; the same
  on a paused layer: still there after 4 s. GUARD. Teeth: MU-80.
- (outside the pin until the nudge lane has merged) TR24 lock_follows_the_nudge: from TR20's locked state a nudge of +30 ms
  through the nudge lane's own route: within 8 beats |e| <= 0.10 against the SHIFTED bar. Whichever lane merges second runs it.
- Paths with no live row and what proves them: the lock on a sequence, reversed and ping-pong -- TL-U47; a trimmed loop at a
  high tempo -- TL-U46; the hop staircase -- TL-U45c; Random in BPM Sync -- TL-U73; the Tap fix -- TL-U70 and SM's c column;
  x2, /2, a moved point on a locked clip -- TL-U49; Undo and the new fields -- TL-UC3b, TL-UC14, TL-UC17.

### 5.4 Re-runs and idle paint: as the chain (probe-boxes with k10 re-registered; probe-video and probe-media-open rows;
gateC2.sh whole; probe-idle-paint with the Clip tab open on a playing BPM-synced clip: the readout repaints only when its
text changes -- TL-U52 is the gate).

### 5.5 VISUAL WORK GATE (after S5b; replaces V1-V14, B1-B12)
Captures by the app's own capture route at 1728 x 1117 and 1280 x 720; every bar is read from the two dump routes, never
from pixels alone; states are reached with REST, the "do" route and loaded compositions. A state that cannot be captured is
BLOCKED and the gate fails (W3: H-T3). The capture builder writes fixtures of 2 s, 10 s, 30 s and 128 s and a manifest.
States (PNG + dump): W1 Timeline, default. W2 Timeline, readout showing the time left. W3 the mode menu open. W4 BPM Sync,
just fitted (10 s: Bars 4, out at 0.8, 3 lines). W5 12 bars (30 s; "/2" greyed). W6 8 bars ("/2" live). W7 the 2 s clip
(Bars 4, out at the end). W8 Speed 1/4. W9 Speed 16 (plus greyed). W10 Speed 0 (minus greyed, play lit). W11 length not
known (a missing file: Bars "--", greyed). W12 the out point pulled in on W6 (Bars still 8). W13 the narrowest inspector.
W14 an image clip. W15 Random in Timeline. W16 Random in BPM Sync. W17 BeatLoopr on "1/2" with Catch Up. W18 a paused layer
(Clip tab + strip). W19 playing backwards (both). W20 the strip, BPM Sync, 4 bars. W21 the strip, Timeline. W22 the 128 s
clip (64 bars) on both surfaces.
Bars: C1 no widget of the section is cut; at 1728 x 1117 all lie in the visible area at scroll 0 (the chain's B1). C2 the
mode menu's items are exactly Timeline, BPM Sync, SMPTE 1, SMPTE 2, Denon DJ, Pioneer DJ, the last four disabled. C3 captions,
labels, tooltips and painted strings of the section: "Speed" once in every state; "Duration" once and no "Bars" in Timeline
states; "Bars" once and no "Duration" in BPM Sync states; in no state the words Beats, Beat, BPM, Reverse, Restart, Continue,
Relative ("BeatLoopr" is his own word and is allowed). C4 the Bars text is the model's `clipBars`, a multiple of 4, at least
4; `outPoint` is the fit's within 0.001 in W4, W5, W7. C5 "/2" `enabled` false in W4 and W5, true in W6; Bars minus false in
W4. C6 the Speed text is one of the nine strings and the thumb sits at index / 8 of its travel, +/- 2 px (W4: 4 / 8; W8:
2 / 8). C7 W9: plus disabled; W10: minus disabled and the play button lit. C8 no widget of a hidden row is visible. C9
nothing in the section's captions, labels or tooltips changes across any "do" action except numbers and lit / greyed states
(Harmony constraint: no text announces an event). C10 line xs equal `barLinesOf` +/- 1 px on both surfaces; neighbours at
least 3 px apart; one colour, one width; 3 lines in W4; at least one in W22. C11 no line in W1, W11, W14, W21. C12 the
readout is the playhead's time within one frame in W1 and begins with "-" in W2. C13 W18: pause lit and play not lit on
both surfaces, `paused` true; W19: backwards lit on both. C14 the strip's S fader: nine detents and the model's step in
W20; continuous in W21; enabled in both. C15 W17: exactly one option lit, Catch Up lit, Off not lit. C16 W1's Duration text
is the marked length / `timelineRate` with " s"; W12's Bars equals W6's.
C1-C16 are GUARDS (new routes); teeth shown once in the builder's log: MU-78 -> C10; MU-86 -> C5; MU-87 -> C14.
Five critic seats (visual-design, UX, graphic-design, logic, interaction-logic), each given the PNGs, the dumps, the
manifest, this section, Boris's four screenshots resolume-transport-1, -2, -7, -8.png, his sentences of section 1 verbatim,
D-1 .. D-10, and the list of what is in the section before the lane and outside it (cue points, Snap, autopilot rows, the
sequence fps row). Questions: visual-design -- set beside his screenshots, is every element of P1 present in the same order
and grouping, in our look? Are the bar lines told apart from the pointers and the playhead without colour alone? UX -- can
Timeline be told from BPM Sync from the bar alone? Does any control look usable and do nothing (the greyed "/2" at 12, the
greyed modes)? Is a paused layer told apart from a clip at Speed 0? graphic-design -- number formats ("1/4", "8 s",
"5.951 s", "12"), the readout, spacing against his screenshots. logic -- is there a state whose numbers pass and still tell
him something false (Bars 4 on a 2 s clip with no sign it runs slow; a fitted out point he did not move)? interaction-logic
(through the "do" route) -- switch modes and back; x2 then /2; minus at 4; typed 6; the readout click; pause then fire;
Speed to 0 and back. Pass = C1-C16 and no seat raising a MUST.

### 5.6 Facts Harmony must measure (none can be settled by reading)
X1 .. X8 of TD7 (stage SM), with the decision table. FM-1, FM-2: the chain's. FM-3 = X7. FM-4 = X2's fifth-minute clause.
FM-5 = X3. After S4c: M2 (one tempo) and M4 again on the lane build, same bars; not met: STOP and report.

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; on his screens, by his ear)

WHERE HIS RULING DIFFERS FROM RESOLUME (each is built HIS way; shown to him so he sees it):
- D-1 The second row reads "Bars", not "Beats"; one bar = 4 beats.
- D-2 Bar counts are 4, 8, 12, 16 ...; Resolume picks 1, 2, 4, 8, 16 ... BEATS (powers of two).
- D-3 On switching to BPM Sync the app MOVES THE OUT POINT IN to the last whole group of 4 bars; Resolume keeps the whole clip
  and changes its speed.
- D-4 A clip shorter than 4 bars is fitted whole into 4 bars and plays slow; Speed makes it fast.
- D-5 There is no second small menu (from the start / carry on / relative): every fire starts at the beginning.
- D-6 A clip dropped mid-bar SLIDES back onto the bar by itself: up to about 17 seconds at 120 (23 at 90, 12 at 174), running
  up to 6 % fast or slow meanwhile. He was told "up to about 15 seconds" in question 33. Resolume leaves it off the bar until
  it is fired again.
- D-7 Pause belongs to the layer: another clip fired on a paused layer waits on its first frame. In Resolume pause is the
  clip's own setting.
- D-8 The bar shows one line per BAR; Resolume's ticks are per beat (INFERRED from his screenshot 2: 16 segments for Beats 16).
- D-9 A file within 0.05 s of a whole group of 4 bars keeps its end and runs up to 0.6 % slow instead of losing 4 bars.
- D-10 Not from Resolume, ours: a press inside the bar moves the playhead there; Timeline Speed stops at 4 (question 73);
  Random and BeatLoopr lengths are fractions of a bar (question 72); the readout's long form.
CHECKS (the chain's checks 1-3, 5-10, 15-24, 35-39 stand as written there; 4, 11-14, 25-34 are replaced by these):
1. Open the mode menu. -> Timeline, BPM Sync, then SMPTE 1, SMPTE 2, Denon DJ, Pioneer DJ greyed. Wrong: a greyed one can be chosen.
2. Put the panel beside Resolume's. -> The same parts in the same places: time at the top right, bar, three buttons, one
   small menu, Speed row, Duration row. Wrong: a part of his screenshots is missing, or an extra box.
3. Click the time. -> It shows the time left, with a minus sign. Click again: the time played.
4. Pause a clip, then fire it. -> It stays on its frame, paused, in the strip. Press play: it runs on from there.
   Wrong: it jumps to its start, or it starts playing.
5. Fire a clip that is playing. -> It starts over from its beginning.
6. Pause a layer, then fire ANOTHER clip on it. -> Its first frame appears and waits. Press play: it runs. (This is my reading
   of "stays in layer strip paused"; say if the other clip should simply play.)
7. Pause one layer and fire a whole column. -> That layer stays paused; the others start.
8. Switch a 45-second clip to BPM Sync. -> Bars 20; the out pointer has moved in to 40 s; lines every bar. At 120 it looks
   exactly as fast as before. Wrong: a number that is not a multiple of 4, or the clip looks sped up.
9. Switch a 2-second clip to BPM Sync. -> Bars 4, the out pointer at the end, the picture in slow motion; Speed up two steps
   (4) and it runs at normal speed, once per bar.
10. Press minus, plus, /2, x2 on Bars. -> 4 at a time; /2 is grey at 4 and at 12; type 6: it shows 8.
11. With Bars unchanged pull the out pointer in, then out. -> Slower, then faster; Bars does not change.
12. Step Speed through its nine values (panel, then the S fader in the strip). -> 0, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16; the two
    controls move together; at 0 the picture stands still. Wrong: a value in between; stutter at 8 or 16.
13. With music playing and the beat steady, watch a synced clip for some minutes. -> Its playhead crosses a bar line each time
    the circle at the top shows "1". Wrong: it creeps off, or the picture visibly speeds up and slows down.
14. Drop the playhead in the middle of a bar; also press Resync in mid-bar. -> No jump; within 10 to 20 seconds the lines are
    back on the "1". Wrong: it jumps, it never comes back, or the wait feels too long (then say so: the other way is one cut
    on the next "1").
15. Tap a new tempo while it plays. -> The clip follows the tempo at once and the lines stay on the "1" the circle shows.
16. In Auto, on your own music, leave a synced clip alone for ten minutes. -> As 13. Wrong: every so often it wanders off the
    "1" and slides back (that would be the app moving its own "1").
17. Click in the dark part of the bar, outside the pointers. -> Nothing. Click inside: the playhead goes there.
18. Loop menu: Random; set Interval and Distance. -> The picture jumps about inside the marked part; in BPM Sync the jumps
    fall on the beat. Play Once and Eject: the layer empties at the end. Play Once and Hold: it stops on its last frame.
19. BeatLoopr: press "1/2". -> Half a bar repeats from where the playhead was. Press Off with Catch Up lit: it cuts to where
    it would have been. Without Catch Up: it carries on from there and slides back onto the bar.
20. Switch a clip to BPM Sync, press Cmd+Z. -> Nothing in the panel or the strip changes back (the out pointer stays moved).
21. Open one of your saved shows with BPM-synced clips. -> They are in BPM Sync with Bars set and the end trimmed to whole
    groups of 4 bars; a video that ignored the tempo before follows it now (question 75).

## 7 QUESTIONS FOR BORIS (new numbers 71 .. 76; each has a default A; nothing waits)

71. A clip is 12 bars long. Half of that is 6, which is not allowed.
    A (default) The /2 button is grey at 12 (and at 20, 28 ...). Use minus to go to 8.
    B /2 goes down to the nearest allowed number below (12 -> 4).
72. Random's two numbers and the BeatLoopr buttons, in BPM Sync:
    A (default) Fractions of a bar: 1/16, 1/8, 1/4, 1/2, 1, 2, 4.
    B Beats, as Resolume shows them.
73. Speed in Timeline mode goes from 0 to 4 today. Resolume's goes to about 10.
    A (default) Keep 0 to 4.
    B Go to 10.
74. A layer is paused. You clear it with X, then fire a clip on it.
    A (default) The clip plays: clearing a layer also ends its pause.
    B It waits on its first frame until you press play.
75. Your saved shows that have BPM-synced clips:
    A (default) When you open one, each such clip gets its Bars number and its end is trimmed to whole groups of 4 bars.
    B They open as plain speed clips, looking as they did; you switch the ones you want to BPM Sync yourself.
76. The fourth button in the layer strip (the double arrow):
    A (default) Stays: one Speed step up in BPM Sync, double the speed in Timeline.
    B Remove it; Resolume's panel has only three buttons.
REQUEST RQ-1 (not a question): one screenshot of the BeatLoopr row in your Arena (a clip in BPM Sync), so its buttons can be
copied. Optional, a minute each, in Arena: click plus once on Speed and on Duration and send the numbers; set Speed to 2 in
Timeline and send the Duration; a clip longer than a minute with the time shown both ways.

## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)

- K-1 THE STRONGEST. A lock with no memory chases whatever the app calls "1". In Auto that "1" is moved by a beat detector
  (E10): each move sends EVERY synced clip 6 % off speed for up to 17 s. Resolume's way (stay where dropped) can never do
  that; "mimic exactly" would have been safer. Why the plan still stands: he chose B knowing the wait, and the plan does not
  promise it -- stage SM measures the "1" on real music before any lock is built, and O2 / O3 are the pre-registered ways
  out. Cheapest refuting test: M3 on one track (10 minutes), X1.
- K-2 The lane is large: 14 stages, about 97 unit cases. One merge may sit for long while the nudge and outputs lanes move
  MainComponent.cpp under it. Counter: H-T4 (merge after S3h, then the rest). Test: S0's conflict list after each foreign merge.
- K-3 Pause on the layer rests on an INFERRED reading (R16's second sentence). If check 6 is answered "the other clip should
  play": the tail clears the pause on a fire of a DIFFERENT clip -- one line, TL-U61 and TL-U64 re-worded, D-7 gone. The
  same-clip rule is his own word and does not move.
- K-4 The fit moves the out point of his SAVED shows (question 75) and trims files he never trimmed. Test: check 21 on one
  of his shows before the merge; B is one branch in the load path.
- K-5 x8 and x16 on a long-GOP 1080p file: the decoder may not keep up (E6 says only that nothing clamps). Test: M7 / X6,
  before S4a merges.
- K-6 Eject clears a layer from the GL thread. It reuses the compare-exchange path the queued release uses (E12), but the
  exact site is S0's to name; if no site knows the layer AND the rows, Eject is requested through one atomic word consumed
  on the message thread's timer instead (up to one tick late). Test: TL-U71 under TSan.
- K-7 The instrument puts test code next to the product path. Counter: TL-L6, G-P1, and S4c deletes its branch. Test: G-P1's
  404 on a build started without the variable.
- K-8 The 0.05 s tolerance and `kMaxBars` are numbers chosen by reading (F35). Test: S0's `getDuration()` of real files;
  TL-U33's 15.98 row.
- K-9 Between a mode switch and the fit of a clip whose length is not yet known the clip plays at its fitted rate with its
  old out point (up to one bar-group "stretched") for a few ticks. Stated, not gated; visible only on a file still opening.
- K-10 Random's Distance and the BeatLoopr list are built from two sentences of documentation (R4, R8). Test: RQ-1; check 18, 19.
- K-11 The Duration row as a factor of its own (F31) is a reading of one sentence ("this doesn't affect the Speed slider!").
  If his Arena shows Duration changing with Speed, `timelineRate` is dropped and Duration writes `speed`: S5a only.
- K-12 The bar lock's worst slide at 90 BPM is 23 s -- longer than he was told. Said in D-6 and in O1's sentence; the jump
  fallback exists if he finds it too long (check 14).
- K-13 The chain's risks about Undo (K1), the stale frame (AM-21) and the trimmed loop's lost frames stand unchanged.

## 9 WHAT IS NOT IN THIS LANE

- SMPTE 1 / 2, Denon DJ, Pioneer DJ: listed greyed only ("add this to our build plan later after core elements are built and tested").
- The trigger-style menu and a BPM box on the clip (his "42 default"; question 30's default).
- A Resync that restarts synced clips; a rework of the downbeat detector (only if O3 is answered that way, in its own plan).
- Timeline Speed above 4 (question 73); cue points; the per-clip Snap menu and the Quantize menu's names (the bars lane).
- The top bar: the dead "/4 /2 x1 x2 x4" buttons (R11), the nudge and its text, Tap's doubled code -- the nudge lane.
- Every output's settings and Delay, Syphon -- the outputs lane. The quit window, the Record boxes, the disk-space line, the
  notices -- their lanes. The Edit menu; the keying audit.
- S1m (keep the Undo history across a layer move): not built.
- THE STOPPED SYNC-DIAL BRANCHES. Carried into this lane: NOTHING. lane/bf2 at 740b6d6 (BeatLead, the frame epoch in
  BPMTracker, `syncOffsetMs`, the delay line, the venues, the witness ring, probe-sync*) and lane/bf2-keys at 9eab9bd
  (SyncNudge, `bindingIsLive`, the Sync bind targets: e15d1d0, df98f78) are dropped with the dial (H-17; "replace our sync
  with this"). The chain's S0 items about them (does bf2 lead the phase the release reads; conflicts after the bf2 merge;
  AM-29's early S1) are void. Parts that are useful on main but not this lane's: the testing-eyes.md kill advice (dc59573),
  the architecture.md seqlock line (inside f14eb31), `selectAt` in the MIDI-learn overlay (df98f78), the take-origin finding
  (H-14) -- each is the outputs or the nudge plan's to carry or drop (facts-syncdial-parts.md T2). The injected beat driver
  this lane's rows use is main's own (probe-boxes k5), not the dial's rig.

NOT VERIFIED BY READING (each is a run or a file I did not open): how often the "1" moves in Auto (X1); the player at x8 and
x16 (X6); smoothness under a per-frame speed (X3); the seek's latency on a bar line (X8); what `getDuration()` returns for
real files (F35); that a detected beat advances beat-in-bar in the same hop as its realign (TD8 item 1); the GL-thread site
for Eject and the call sites that give `syncMedia` its layer (S0); whether the capture route can show an open menu (W3);
whether PerfState's restore or a routine writes `playing` directly; which music files exist on the rig (H-T2); the nudge
and outputs plans (skeletons at the time of writing: their file lists are ASSUMED from their headings); Resolume's BeatLoopr
list, the minus / plus steps and the readout's long form (NOT DOCUMENTED).

---------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (2026-10-04 13:57:32, session s-rta-1004)
ADOPTED IN FULL: .harmony/.reports/s-rta-1004/ruling-transport-delta2.md (status DONE; 31 attacks ruled: 17 ACCEPT, 11 PARTIAL,
3 REJECT; 17 amendments RA-1..RA-17, each OVERRIDES this plan's body; the earlier chain: 16 STAND, 19 AMENDED, 5 DROPPED).
Precedence for every stage, review and gate of the transport lane: Boris's verbatim words (binding-decisions.md, the later
line wins where one says REPLACES or REVERSES) > this adoption > ruling-transport-delta2.md > this plan's body >
ruling-transport-delta1.md > ruling-transport.md > their plans. Workflow run wf_eee587a8-bc3 (plan: architect opus high;
seats video-clock 8 attacks / 1 MUST, gates 8 / 3, stage-hands 7 / 0, scope 8 / 0 -- papers whole in
attack-transport-delta2-papers.md; ruling: architect opus max).
What I read myself before adopting: the ruling's returned verdict, stage list, decisions, measurements and strongest
counter-argument, and its section 7 (questions) in full. NOT read by me: sections 1-6 and 8-10 in the file -- the builders'
and reviewers' spec; a gate string is copied only from section 5 (and from the chain's final lists where section 5 is silent).
WHAT THE COUNCIL AND THE RULING FOUND THAT THE PLAN DID NOT: a reversed clip leaves in..out and a trimmed ping-pong never turns
(his fit makes a trimmed clip the normal case: new stage SR puts the range in the players); three gate rows that could not
fail; and -- no seat's, the ruling's own -- a --test-mode instance never starts the analysis thread, so the plan's tempo runs
could not have run in it: the first measurement SM-a runs in PRODUCTION mode.
HIS RELIABILITY QUESTION ("can you program this reliably or should we change the plan?"): answered by measurement, not by me.
SM-a (no product code) then SM-b (the instrument) come first; only stage S4c (the lock) waits for the verdict; the decision
table O1..O7 says what is built for each outcome. Bars: N1 at most 1 move of the "1" per 10 minutes per track (over = O3,
STOP AND ASK); R1 at most 1 % (over = the anchored reader, O2). No bar is loosened by anyone.
HARMONY'S DECISIONS (the ruling's section 8):
H-T1 default: S4t (a Tap keeps the bar) is built in this lane before SM-b; when the beat-nudge ruling lands, the two are
     reconciled so that TL-U70 is registered exactly once.
H-T2 the three tracks are Boris's: he is asked for them (request RQ-0). Without three real tracks: BLOCKED, no tracker row.
H-T3 default: W3 is dump-only if the capture cannot show a popup; then it is his check 1.
H-T4 default: two merges (merge 1 after S3h with VG-1 and probe-transport at 18 rows; merge 2 the rest).
H-T5 default: the production-mode instance follows the probe-manual-bpm.sh recipe exactly (a normal cmake build, never a
     copied bundle; the live lock; never while his app runs; open -g; his settings file byte-identical before and after).
H-T6 default: FM-7 on the fixtures plus at least 20 of his clips, read-only; FM-8 a read-only search of his show files. A
     first look at his one show (my copy of it): 15 clips carry "videoBeats": 4.0 and no key names a sync mode -- NOT a
     verdict (the key that marks a BPM-synced clip was not looked up); FM-8 is run at S0. Question 73 is held back until then.
H-T7 question 72 is asked now; default A if unanswered when S2 starts.
H-T8 default: an O3 or O7 from SM-a stops S4c only; SM-b is still built and run and the lane goes on to merge 1.
H-T9 Pitfall numbers are assigned at merge (68 is the outputs lane's).
ORDER: S0 -> SM-a (mine) -> S4t -> SR -> SM-b (then my runs M1..M8) -> S1 -> S2 (-> S2b) -> S3 -> S3h -> MERGE 1 -> S4a -> S4d ->
S4e -> S5a -> S5b (VG-2) -> S4c (per the verdict) -> S6 -> MERGE 2. NOT STARTED in this session: nothing of this lane is built.

## HARMONY ADOPTION, UPDATE ON BORIS'S ANSWERS (2026-10-04 14:06:21)
Boris, verbatim (binding-decisions.md, "Boris's answers to questions 71, 72, 74"): "71 b"; "new clip plays, the old clip is
permanently paused and if comp is saved, it is saved as paused. the clip is now paused until the user changes that
setting."; "74 b". His words outrank the ruling. Consequences, binding for every packet of this lane:
(1) 71 B: NO SLIDE IS BUILT. Stage S4c builds the ruling's one-cut path (the clip plays on and seeks once into time on the
    next "1"; the decision table's fallback row) as THE design, not as a fallback. The 6 % slide law, its pull-in bar X4 and
    the "eases onto the beat" text are void. SM-a (how often the "1" moves in Auto) still runs first and still decides: a
    moving "1" now means a CUT each time, and N1's bar and the STOP-AND-ASK of O3 stand.
(2) 72: PAUSE IS THE CLIP'S and is SAVED IN THE SHOW. The ruling's F38 (pause lives on the layer) and reading R16's second
    sentence are overruled. The ruling says its TransportPause.h "keeps question 72's B and C small": C is taken, and his
    words add what C did not say -- the pause persists until he changes it and is written to the show file.
(3) 74 B: Timeline Speed's range goes to 10 (the ruling's F37 runner-up).
OWED BEFORE the packets of S2, SM-b and S4c are written (NOT before S0, SM-a, S4t, SR, S1): an architect delta on these
three answers (opus max), attacked by two blind seats (gates, stage-hands) -- it re-states the affected stages, unit cases,
gate rows and Boris checks, and says what of SM-b's instrument is still needed without a slide. No builder re-designs them.

## POINTER (2026-10-04 15:36:21): Boris's answers 71 B, 72 and 74 B are re-stated in plan-transport-answers.md / ruling-transport-answers.md (ADOPTED); they amend this plan's stages and rows (TB-1..TB-14).
