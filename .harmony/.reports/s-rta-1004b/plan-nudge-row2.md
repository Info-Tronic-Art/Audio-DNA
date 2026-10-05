# PLAN nudge-row2 -- the beat-nudge lane, SECOND DELTA on Boris's answers (125-129, 135-137; readings R81, R85)
Architect (opus), 2026-10-04, session s-rta-1004b. A POSITION for a blind council; an architect ruling follows; Harmony decides.
Read-only. Pin: `git -C /Users/boriskarpman/projects/RealTimeAudio diff --stat 7bc6df7 -- src tests docs CMakeLists.txt`
printed nothing (plain-file reads below are reads of 7bc6df7). lane/bf2 = 740b6d6 clean, lane/bf2-keys = 9eab9bd clean
(pin-checked, not searched). The nudge worktree (stage S1, being built) was NOT read. Nothing built, run or launched.
Shorthands: M: = main at 7bc6df7. RN = ruling-nudge.md. RR = ruling-nudge-row.md (NA-1..NA-18, HR-1..HR-14, LR1..LR7,
R74..R84). PL = plan-nudge-row.md. FQ = s-rta-1004b/facts-quantize.md (its VERIFICATION section overrides its body).
RA = ruling-transport-answers.md. BD = .harmony/binding-decisions.md (line numbers of today's file).
Labels: VERIFIED (read at the pin by me, or a sheet row its VERIFICATION confirmed), COMPUTED, INFERRED, ASSUMED.
NOT READ by me (turn budget), and nothing below rests on them except where it says so: boris-feedback-backlog.md (his
words are quoted from binding-decisions.md only), plan-nudge.md's body, RR sections 1, 2 and 10 beyond the lines
cited, RN sections 1-3 beyond A18's and T-N5b's lines, ruling-transport-delta2.md, ruling-bf2-stops.md (the section
list here is the dispatch's), RIG-RULES.md, HANDOFF.md and docs/claude/pitfalls.md (their rules are applied as the
dispatch and CLAUDE.md's index state them), facts-actions-today.md and facts-beat-controls.md.
Precedence asked for: Boris's verbatim words > Harmony's adoption blocks > THIS DELTA (amendments NB-1..NB-8) > RR > RN >
the plans. Where this delta is silent, RR stands; where RR is silent, RN stands. Every line replaced is named.

## 1 GOAL
Re-state every stage of the beat-nudge lane after S1 on his answers of 21:03:09 and 21:33:30, so that a builder reads one
stage row and one list of rows and never designs. His words this delta rests on, verbatim (BD:951-959, 967-972,
1007-1013, 1021-1022): "125 whole numbers"; "126 and 127 stop clears all clips from layer strips, pause stops them, tempo
setting stays the same. Make thee new buttons as I asked"; "128 tapping tempo does not start anything but when click
resync, that is the 1 and it begins on that button push"; "129 b"; "135 the beat stops but tempo is not lost, just not
playing"; "136 b"; "137 the plus moves the main bpm 1 bpm number regardless of bpm or timeline mode. the plus moves 1
beat in the clip that is in bpm mode"; "This is not for live usage. We should remove it from the top bar and use it in
the recording review screen which we have yet to create."; "r85 go with the defined stop play pause that we discussed
in 135 and place them at the top. use this similar layout across the top row where it fits: [Image #8]".
Which earlier rules of his these replace: his answer to 111 (BD:918-920, "just the bpm timer ...") is REPLACED for STOP
(stop now also takes the clips off) and KEPT for PAUSE by "136 b" (what is not BPM-based plays on). BF20's live Quantize
menu is REPLACED (BD:976). RR's question 127 default A (the routines stop stays as "R[]") is REPLACED by R85 confirmed.
RR's question 128 default A and 129 default A are REPLACED by "128 ..." and "129 b".

THE POSITION IN TEN LINES
1 STOP = stop every routine, take the clip off every layer (the layer's own clear: a cut, the queue and a fade in
  flight gone with it), then stop the beat as RR ruled it (the "1", no edge, tempo live). No Undo step. NB-1.
2 PAUSE = RR's pause, unchanged: the beat holds; a BPM Sync clip gets dt 0 at its advance; nothing is written into any
  clip, so "held by the row" is not a stored state at all. The old play-all / pause-all buttons, which DID write clips, go. NB-2.
3 A Tap never starts; a Resync while stopped or paused IS a start from the "1" on that hop. One constant becomes two. NB-3.
4 Stop and play never touch the nudge; the two statements and `kNudgeAfterStop` are deleted. NB-4.
5 "-" / "+" = 1 BPM, whole numbers, every mode: RR stands. NB-5.
6 Live Quantize is removed in a LANE OF ITS OWN, "quantize-out", merged to main before the packet of this lane's S2 and
  before the transport lane's S1: the top-bar combo, the clip's Snap, both gates, the drain and the legacy beat-phase
  seek go; the pending slot's bits stay in the layer word, provably never set; a routine's own quantize stays. NB-6.
7 The nudge still moves every reader of the published beat; the live rows that used a quantised fire as their reader
  use an autopilot beat step (and a routine pad's bar start, while pads exist). NB-7.
8 The row is his thirteen cells, square, edge to edge, in his order, play / pause / stop lit by the published state;
  it fits at 1728 with 33 px to spare (COMPUTED on two ASSUMED widths), and at 1280 with compact cells. NB-8.
9 Stages: quantize-out (QO-1, QO-2) first; then S1r, S2, S2r, S3a, VG-0, S3m, S3r, S4, S4r, VG, S5 as re-stated in
  section 4. BPMTracker.cpp order S1 -> S1r -> transport S4t holds.
10 No STOP for S1: nothing he answered makes an S1 item wrong. One NOTE for Harmony on S1's case T-N5b (section 4).

## 2 ESTABLISHED FACTS (verified lines only)
E1  VERIFIED M:src/model/Layer.h:380-390 `clearedNext`: previous = the active ref, active = none, crossfadeProgress =
    1.0, and the pending slot emptied -- in ONE tuple word. A clear is a cut; a fade in flight ends; a queued trigger dies.
E2  VERIFIED M:Layer.h:492-503 `clearActiveClip` -> `applyClearTail` (:587-592): the cleared clip's runtime `playing`
    is set false. VERIFIED: the string "playing" does not occur in M:src/model/Clip.cpp (grep): play state is not saved today.
E3  VERIFIED M:src/MainComponent.cpp:5726-5749 `applyClearActiveClip(int layerIndex, Origin, int deckIndex)`: clears the
    shared layer, refreshes the deck view, and unless the origin is Replay captures one point (control "activeClip",
    v = -1). It pushes NO Undo command. A replay calls it at :1965.
E4  VERIFIED M:MainComponent.cpp:723-748 the layer strip's X: `routineEngine_.stopOnLayer(layerIdx)`, then
    `clearActiveClip`, then a `ClearActiveClipCmd` pushed (an Undo step today), then `refreshPreviewFromShow()`
    (the 2026-07-30 fix: without it a shader source keeps rendering through the empty-compositor fallback). The
    lambda holds no `capture(`: an X is not recorded in a take today. `applyClearActiveClip` (E3) has ONE caller in
    src, the replay at :1965 (grep).
E5  VERIFIED M:src/recording/RoutineEngine.cpp:793-806 `stopAll()` stops every running routine and resets every pad;
    :808-817 `stopOnLayer` stops only RUNNING routines that hold that layer (it reads `running_` only).
E6  VERIFIED M:MainComponent.cpp:612-628: the three old buttons. `onPlay` / `onPause` call `applyClipPlaying` on every
    layer's playing clip (they WRITE clips); `onStop` calls `routineEngine_.stopAll()` and nothing else.
E7  VERIFIED M:src/ui/TopBar.h:98-100 (`playButton_{">"}`, `pauseButton_{"||"}`, `stopButton_{"[]"}`), :117-119 (the
    "Quantize:" label and combo), :62 (`quantizeSelectorBoundsForTest`); M:src/ui/TopBar.cpp:529-631 `resized()`: inner
    area = bounds reduced by (4, 2); Audio block 38+90+4+30+70+6+2 = 240; the three buttons 24+1+24+1+24+6+2 = 82;
    wheel 26+2; "Bar N" 44+2; Quantize 55+100+6 = 161; right block DSP 55, FPS 45, 6, Outputs 100, 4, Master 90+42, 4,
    Master Signal 90 + its measured label.
E8  VERIFIED M:src/model/Clip.h:118-119 `enum class TransportMode : uint8_t { Timeline, BPMSync }`, default Timeline.
    "BPM-synced" today = `clip.transportMode == Clip::TransportMode::BPMSync`.
E9  VERIFIED M:src/render/Renderer.cpp:1657-1673 (video) and :1718-1738 (sequence): a BPM Sync clip takes a SPEED from
    the tempo and then `advanceFrame(dt)`; three advance calls (:1673, :1734, :1738). No phase lock exists on main.
E10 VERIFIED M:Layer.h:410-433 `triggerClip(ref, rows, forcedSnap, ...)`: a fire is only QUEUED when `forcedSnap != Off
    || target->beatSnapMode != Off || target->beatSnap` and the ref is not the active one; else `immediateNext`.
    :449-486 `processPendingTrigger` is the drain. FQ Q2 (CONFIRMED, VERIFICATION A.7-A.13).
E11 VERIFIED M:MainComponent.cpp:36-43 `quantizeModeToForcedSnap`; its four uses :4280, :4849, :5078, :6255 (two clip
    handlers, the routine tick, the routine fire); :2012-2015 a take's preamble writes `composition_.quantizeMode`.
E12 VERIFIED M:MainComponent.cpp:4866-4870 and following: after a fire, `if (clip->beatSnap && clip->isPlayable())` the
    playhead is set from the beat phase (the legacy seek). His rule "42 default" (BD:844-845: a fired clip starts at
    its beginning, every time) is broken by this line for a clip with Snap on.
E13 FQ Q1a, Q3 (CONFIRMED A.1-A.3): the combo writes `Composition::quantizeMode` directly, never follows the model;
    the only three writers of the global value are the combo, a loaded show, a take's preamble. No binding, REST, OSC
    or settings field sets Quantize or a clip's snap (FQ Q3; A.6). `AppSettings.h` has two keys only.
E14 FQ Q3 + C-3 (VERIFIED by grep there): the take checkpoint stores `quantizeMode` and the two pending fields;
    nothing restores the pending pair; `Program.cpp:292-295` emits the preamble row `comp/quantize`.
E15 FQ section 3 item 11 (CONFIRMED): `Routine::quantize` is typed `Clip::BeatSnapMode` (Routine.h:21, default Bar) and
    `RoutineSnap` is a static-asserted copy of that enum (RoutineEngine.cpp:12-17). FQ E.6: the two lines that couple
    routines to the top-bar value are M:MainComponent.cpp:4280 and :6255.
E16 FQ Q4 + C-1: the tests and probes a removal turns red (listed by name in NB-6).
E17 VERIFIED M:src/binding/Binding.h:24-47: 21 actions, append only; `GlobalStop` and `GlobalPlayPause` exist; no
    action or field names a snap.
E18 VERIFIED RA:361-381 (TB-7): after the transport lane a clip's OWN pause is `paused` / `pausedAt`, written only
    through src/model/TransportPause.h; "NOTHING ELSE writes the pause: no fire, no column fire, no clear of a layer
    ... no beat timer"; `applyClipPlaying` is "the ONE entry of the top bar's Play and Pause (for as long as they exist".
E19 VERIFIED RR section 0 item 1 and PL:193-262 (read whole): the paused / stopped / play publication, the request
    queue word, steps 0 / 2b / 2c, and that "a start from stop (`startApplied`) is treated by steps 0 and 6 exactly as
    a tracker Resync that is not a restart".
E20 VERIFIED the picture resolume-tempo-bar.png (looked at): 1200 x 82 px, thirteen flush cells -- circle, play (lit:
    filled mint), pause, stop, "BPM 256", "-", "+", two arrow-and-bar glyphs, "/2", "x2", "TAP", "RESYNC". No nudge
    text, no bar text.
E21 VERIFIED BD:688-689 and :764 (his Undo rule): "Let's not allow control Z to change anything that is live in the
    layer strip. It changes anything else"; "cmd-z does not affect anything in layer strip: play, play reverse, pause,".

## 3 ITEMS
The real-time rules hold in every amendment: nothing touches the audio callback; the analysis thread gains branches
only; the render thread reads the byte RR already gave it and never waits; no mutex is added; every new function below
runs on the message thread. No slider and no popup menu is added. No text announces an event or a failure: a stopped
beat is a lit button and empty strips.

### RB1 STOP (126 / 127 / 135)  -- amendment NB-1
VERIFIED: E1-E6, E19, E21.
FORKS. (a) what "clears" is: A1 the layer's own clear on every layer [CHOSEN]; A2 a new "fade out" clear. A2 loses: no
tuple function fades to nothing today (E1), his word is "clears", and the X he knows cuts. (b) routines: B1
`routineEngine_.stopAll()` [CHOSEN]; B2 `stopOnLayer` per layer (exactly what X does); B3 no routine call. B3 breaks
question 135's option A as it was shown to him, which Harmony read his answer as choosing (the question's words, not
his: "Play starts the beat again; the layers stay empty until you fire clips"): a routine fires clips, its clock is
the beat (RR NA-15), so it would put clips back on the first beats after play. B2 misses a pad that is WAITING for its
bar (E5: `stopOnLayer` reads `running_` only). B1 is one existing call -- the very call the old "[]" button made (E6),
so the button's leaving loses nothing. Harmony constraint "no routine clause is built in this lane" is met in the sense
that no routine code is written; this ONE existing call is named here for her (HB-1: she may take B3 and reading R103
then tells him a running routine refills the layers). (c) Undo: C1 no Undo step [CHOSEN, E21]; C2 one composite step.
C2 breaks E21. (d) recorded in a take: D1 nothing is captured, as the strip's X captures nothing today (E4) [CHOSEN];
D2 one "activeClip = none" point per layer through `applyClearActiveClip` (E3). D2 loses: it is a new take clause
(Harmony constraint: none in this lane), and takes are being re-modelled. The cost of D1 is said: a take replayed
later shows the clips he took off with Stop -- as it already does for X today (side finding SF-B1).
THE RULE. `void MainComponent::clearAllLayers(Origin origin);` (message thread), in this order:
`routineEngine_.stopAll()`; for every shared layer `layer->clearActiveClip(composition_.rowClips(l))` (the X's own
call, E4, without its Command); one `refreshPreviewFromShow()` (E4); one deck-view refresh. It pushes no Command and
captures no point (`origin` only decides nothing today; it is kept in the signature for the binding path).
`setBeatTimer(BeatRun::Stopped, origin)` calls `clearAllLayers(origin)` FIRST, then `requestRun(Stopped)`. It is the
only caller (lint T-G14, amended). A Stop while already Stopped runs both again (layers fired since are taken off; the
tracker's Stopped + Stop is Stopped, PL:226).
WHAT EACH THING DOES AT A STOP. A queued trigger: none can exist after quantize-out; before it, E1 cancels it. A
crossfade in flight: ends at once, both clips off (E1). A retired deck's clip: no layer names it any more; it is reaped
by the next fenced edit as after any clear (M:MainComponent.cpp:1785-1787; INFERRED, not run). A playing routine and a
waiting pad: stopped (B1). A take being REPLAYED: not stopped -- its next recorded fire puts a clip back (parked for
the actions lane; reading R104). A take being RECORDED: gets no point from a Stop -- neither the timer (RR NA-4
stands whole) nor the clears (D1). The output: every Output window and Syphon show the empty
composition (what the layers' effects and the global stack make of nothing); no Output window is touched.
Autopilot, bypass, solo, opacity, effects, the shown deck: untouched. A clip's own pause: untouched (E2, E18).
WHAT THE BEAT PUBLISHES WHILE STOPPED: RR section 0 item 1 STANDS whole (the "1" block, no count change, tempo live
and never 0, bit 3). PLAY afterwards: RR's start from stop STANDS (one edge, beatInBar 0, no bar counted); the layers
stay empty until he fires. A clip FIRED while stopped: RB2.
REPLACES: RR section 0 item 2's first paragraph (RR:49-52, a quantised fire waits) and the routine paragraph's gate
use (RR:56-60); RR NA-16's first two sentences; RR LR5 (g)'s last clause ("across stop a running routine is still
running"); RR B-R3's first sentence; PL:193-197's last sentence.
RED-FIRST. Unit T-S1..T-S4 (section 5); mutants MS1, MS5, MS7; live LR2 (a), (b), (d).
THE GUARD against a mis-press is his call: question 144 (default: none, one press, as Resolume).

### RB2 PAUSE (127 / 136 B)  -- amendment NB-2
VERIFIED: E8, E9, E18; RR NA-1 (the seam `ClipTransportSync::syncDt`).
THE RULE: RR's pause STANDS as ruled, word for word -- "136 b" is the rule RR already built on his answer to 111.
- "BPM-synced" per clip: today `transportMode == BPMSync` (E8); after the transport lane, the same field shown as the
  panel's mode "BPM Sync" (RA; the panel of resolume-bpm-sync-panel.png). `syncDt` reads that one field.
- WHERE "held by the row" LIVES: nowhere in the model. It is computed on the render thread every frame from two facts
  -- the published byte (`beatRunning(snap.beatShiftState)`) and the clip's transport mode -- and it only replaces
  the dt of that frame's advance by 0.0. No clip field, no layer field, no show key, no settings key, no Undo step.
  So play restores exactly what ran before (nothing was changed), a clip he paused himself stays paused (its own
  `playing` today, `paused` after the transport lane, is never written by the row: E18's TB-7 names "no beat timer"),
  and nothing of it is saved.
- What a held clip shows: its frame, standing. The strip and the cell show nothing new; the lit pause in the row is
  the state display (the visual gate asks the seats whether that reads; question none).
- A clip FIRED while paused or stopped: every fire lands at once (after quantize-out no fire waits). A Timeline clip
  plays. A BPM Sync clip becomes the layer's clip at once, is `playing`, and stands on its first frame until the beat
  runs (dt 0); from STOP the play press is its "1", so it starts in time; from PAUSE it runs on with the beat (and,
  once the transport lane's S4c exists, takes that lane's one cut on the next "1" if it is off the bar -- RA TB-2's
  "fire off the 1" row; the transport lane's lock applies no correction while the beat is not running: RR NA-1's
  proposed constraint STANDS). Reading R105. INFERRED, not run: a clip fired with dt 0 shows its first frame and is
  not "pending" (Pitfalls 53, 56) -- LR3 (e) and FM-R2 measure it; if it shows black the lane STOPS at S2r's rows.
- The row's PLAY: `requestRun(Running)` and nothing else. It starts no clip.
- THE OLD BUTTONS' CODE GOES (E6): `TopBar::onPlay`, `onPause` and their two lambdas -- the only top-bar path that
  wrote a clip's play state. `applyClipPlaying` stays for the strip, the `LayerTransport` pad and replays (E18).
REPLACES: nothing of RR's pause. ADDS: LR3 is re-stated on PAUSE (a stop now empties the layers, so "stop for 5 s"
can no longer prove a hold); clauses (e) fired-while-paused and (g) his own pause survives. Mutant MS6.
Runner-up for where the hold lives -- a per-layer "held" flag set at the pause and cleared at play -- loses: it is a
second copy of the published byte that a fire, a load or an Undo between pause and play would have to keep right.

### RB3 TAP AND RESYNC (128)  -- amendment NB-3
VERIFIED: E19; RR NA-14; PL:204-205 (site 3: a tempo request applies the tempo and skips its realign while not Running).
THE RULE. `kGesturesStartTimer` is REPLACED by two constants in src/model/BeatTimer.h:
`inline constexpr bool kTapStartsTimer = false;`   `inline constexpr bool kResyncStartsTimer = true;`
- A TAP while Stopped or Paused: the tempo is taken (PL site 3); the state does not change; the nine beat fields do
  not move; nothing is counted. (RR's rule; now a constant of its own.)
- A RESYNC while Stopped OR Paused: the tracker runs `startFromOne()` -- state Running, `++totalBeatCount_` (ONE
  edge), the "1" block, the start-hop flag of RR NA-13 -- and raises `appliedResyncs()` AND `appliedStarts()`; it does
  NOT raise `appliedStops()`. A Resync while Running is main's Resync, unchanged.
- THE PUBLISH STEP on that hop: a start (E19: steps 0 and 6). A HAND Resync's zero request is consumed on the same
  hop (RN A2 stands: every hand Resync zeroes the nudge), so the published beat is the tracker's own: count' = F's
  count + 1 EXACTLY, beatInBar' 0, the level true, applied 0.0, not held; `adoptCounters` is returned when the
  tracker's count differs from that. INFERRED from PL:235-250, not run: RN A24 holds -- if T-N17b cannot be made green
  without a rule not written here, the builder STOPS with the hop sequence.
- A REPLAYED Resync (a take, a routine; RN H-3) while held: starts the beat too and leaves the nudge (H-3 unchanged).
REPLACES RR NA-14 whole ("a Resync while held is a stop for the publish step"), T-G6, T-N17b, mutant MR27, reading R84,
B-R6's first sentence, the constant in RR's S1r row. RR's question 128 is closed by his words.
R84 RE-STATED (reading R109): "Resync while the beat is stopped or paused: your press is the 1, the beat runs from it,
and the nudge reads 0 as after any Resync. Tapping a tempo while it is stopped changes the number and starts nothing."
Every row that read the old constant: S1r's file list; T-G6; T-N17b; LR2 (new clause f); B-R6; S5's pitfall sentence
("a Resync while held is a stop" -> "a Resync while held is a start from the 1"). Checked: LR1, LR3, LR4a, LR4b, LR6,
LR7 do not press Tap or Resync while held.
Runner-up -- Resync from PAUSE only seats the "1" and stays paused -- loses to his words: "when click resync, that is
the 1 and it begins on that button push" names no exception.

### RB4 THE NUDGE ACROSS STOP AND PLAY (129 B)  -- amendment NB-4
THE RULE. RR NA-9's B: the two message-thread statements in the Stop path are NOT written; `kNudgeAfterStop` is not
declared (it leaves S1r's BeatTimer.h list). Stop, pause and play never read or write the nudge number. T-N19 (PL:563-
566) is the live path: the "1" block with F's count on the stop hop, the kept shift after play.
WHAT ELSE READ THE ZEROING (each checked): T-N17 (PL:554-558) -- its zero-request arms are STRUCK for the stop hop
(a Stop never carries a zero request now); the stop hop with a kept nudge is T-N19's; T-N17 keeps its never-engaged
arm (no byte written) and its count clause "totalBeatCount' == F's EXACTLY". PL step 2b's sentence "A pending zero
request is consumed and Da = 0" stays true only for RB3's hop, which is no longer a stop hop: a zero request on a
stop hop cannot occur from the UI; if both arrive in one hop (a Resync then a Stop inside 10.7 ms) the queue applies
them in order and the hop is a stop with the nudge zeroed by the Resync -- T-N17 keeps ONE such arm. LR2 (a): "ms" is
what it was before the stop (40 in rounds 4-5). B-R6's second sentence and B-R3's "With the nudge at 0" (kept: it is
still the clean first check). RR risk R3: moot. Mutant MS4 (Stop zeroes the nudge).
What "play is the 1" means with a kept nudge (E19, PL:247-250): at nudge 0 the edge is on his press; with a plus
(earlier) nudge the edge is on his press and the phase starts at the shift; with a minus (later) nudge the edge comes
that many ms after the press. B-4 says it.

### RB5 THE STEPS (125 A, 137)  -- amendment NB-5
RR section 0 item 3 STANDS whole: "-" / "+" = the next whole number below / above, in every mode, clamped 30..400,
never folded; "/2" "x2" as ruled; the base rule of NA-2. His 137 is reading R96 (a BPM Sync clip's Beats "-" / "+" =
1 beat is the TRANSPORT lane's panel, not this lane). Nothing changes.

### RB6 QUANTIZE OUT (his words on R81; readings R86, R87, R100)  -- amendment NB-6
VERIFIED: E10-E16.
WHERE IT IS BUILT. Forks: W1 a stage of this lane before S2; W2 a lane of its own, "quantize-out", branch
lane/quantize-out from main, merged to main FIRST [CHOSEN]; W3 only hide the combo. W3 loses on the code: the clip's
own Snap queues a fire with the combo gone, an old show with `quantizeMode` set queues with no control on screen (E13),
and the legacy seek breaks "42 default" (E12). W1 loses: the transport lane's S1 and S2 rest on the same lines (FQ
per-stage list: AM-1, AM-8, TL-U3*, TR6) and would have to wait for this whole lane -- its visual gate and Boris's
checks -- to get a removal that has nothing to do with the nudge. W2 costs one more worktree and one small visual
gate; both lanes then take main in as a step 0.
WHAT GOES (lane quantize-out; no code is designed beyond these lines):
 Q-a The top-bar "Quantize:" label and combo, `onQuantizeChanged`, `quantizeSelectorBoundsForTest` (E7). The four
     layout tests of tests/test_master_signal_link.cpp that use that seam (:191, :302, :342, :386) are re-pinned to the
     widget that is then the Master Signal label's left neighbour ("x4"), through one seam
     `juce::Rectangle<int> TopBar::rightOfTempoBoundsForTest() const;` (S3r re-points it once more).
 Q-b `Composition::quantizeMode`, the enum `QuantizeMode`, its reset and its save line. The LOAD reads the key
     "quantizeMode" and drops it. An old show loads.
 Q-c The clip's Snap: the Clip inspector's combo (`beatSnapSelector_`, ClipInspector.cpp:161-175, :746-751, :1428),
     the fields `Clip::beatSnap` and `Clip::beatSnapMode`, their save lines; the LOAD reads "beatSnap" and
     "beatSnapMode" and drops them. Rests on readings R87 / R100 (INFERRED consent: he answered R87 with the picture
     of a panel that has no snap; BD:1025-1027). If he corrects it: Q-c and the clip half of Q-d come back as they
     were; nothing else of this delta changes.
 Q-d Both gates: `Layer::triggerClip` loses its `forcedSnap` parameter and its queue branch (always `immediateNext`);
     `Composition::fire` / `triggerColumn` lose theirs; `quantizeModeToForcedSnap` is deleted; M:MainComponent.cpp:
     4849 and :5078 fire plainly; Autopilot's three `triggerClip` calls drop the argument.
 Q-e The drain: `Layer::processPendingTrigger`, its call in `Autopilot::processFrame` (the beat-crossing consume
     itself STAYS: autopilot counts with it, Pitfall 38), the counter `render_pending_fired` and its two REST keys.
 Q-f The legacy beat-phase seek (E12).
 Q-g A take's preamble row `comp/quantize` is no longer emitted (Program.cpp:292-295) nor applied (M:MainComponent.
     cpp:2012-2015). `PerfStateCapture` writes 0 and an empty pending pair.
 Q-h The two routine lines (E15) pass `RoutineSnap::Off` as the forced snap: a routine's start follows its OWN
     quantize only.
WHAT STAYS:
 - The pending slot's three fields inside the layer's tuple word, its pack / unpack, and every cancel line that
   writes them empty (`clearedNext`, `immediateNext`, `releaseMomentary`, core/DeckCommands.h's helpers). No bit of
   the word moves (Pitfall 63: no re-pack). They are provably never set: no parameter, no function and no caller that
   could set them exists (the compiler), plus lint T-Q5. Their excision is NOT in any lane yet (section 9).
 - The enum type `Clip::BeatSnapMode` (routines are typed by it, E15).
 - A ROUTINE'S OWN QUANTIZE: the field, the pad's "Quantize" submenu, the REST fields, the engine, its tests.
   Untouched until the actions lane. Does leaving it break a rule of his? No: his words are about "the top bar" and
   "button pushes" for recording clean-up; R86 says "every clip you fire"; a pad's wait for its bar is the routine
   engine's lead-in. Whether the pad menu should go now is his: question 147.
 - "the snap override of a binding": there is none to remove -- no Binding carries a snap (E17, E13); the only
   "override" is the tuple's `pendingTriggerSnapOverride`, which stays packed and always Off.
 - The checkpoint struct `PerfState` and its JSON keys (a take's file format is not touched; old takes load).
 - `bpmMultiplier` and the five buttons: this lane's S3r (RR's row stands).
WHAT A REPLAYED FIRE DOES: a take's or a routine's recorded fire goes through the same handler (FQ headline 4), which
no longer has a gate: it lands at its recorded time (press time is all a take ever stored, FQ Q6). An OLD take whose
checkpoint carries `quantizeMode` 1 or 2 replays with every fire at its recorded press time, not on the line it
landed on when it was performed (the landed time was never stored). Said to him (reading R110).
TESTS THAT TURN RED AND WHAT BECOMES OF THEM (E16; the builder deletes or rewrites exactly these, and reports each):
 DELETED (they pin the queue being armed): tests/test_undo_commands.cpp :2717, :2731, :2760, :2776, :2656 (its queue
 arm), :3024, and the queue arms of :1646, :1824, :2815, :2844, :2875; tests/test_layer_runtime.cpp :341, :457, :474;
 tests/test_show_model.cpp T5 (:474) and the queue arms of T6, T6e, T7.
 REWRITTEN: test_undo_commands :2927 ("clear cancels a pending trigger" -> on a hand-built tuple); test_layer_runtime
 :245 and the pack cases :66, :92, :177 (the word still packs the fields: kept, values hand-built);
 test_layer_runtime_race.cpp R1 and R-bf9b (no snap argument; the race is trigger vs clock vs autopilot);
 test_composition.cpp round trip (the three fields leave; T-Q-C1..C3 added); test_program_preamble.cpp (no
 `comp/quantize` row); test_take.cpp :1076-1100; test_recorder_host.cpp :616 / :738 (8 -> 7 discrete entries);
 test_layer_strip_source_deck.cpp:22 (the alias); test_shared_field_types.cpp:19 (the two Clip fields leave its list).
 PROBES re-stated by QO-2: probe-tsan.sh / .py scenario d (validity = `render_autopilot_advances >= 1` only);
 probe-boxes.py K5 `k5_queue_link_off` (struck: the behaviour it pins is gone; Harmony strikes it from the boxes
 row's count in writing); probe-step3.sh:732 (>= 7); probe-tsan-analyze.py:35-36, 68-70 (field list); fixtures that
 write the keys keep them (they are now the "old show" fixtures).
GATES: section 5, QL1..QL5 and VQ. `.harmony/probe-tsan-unit.sh` is REQUIRED (Pitfall 63) and is Harmony's.
EVERY ROW OF RN AND RR THAT TABLE Q5 (with the VERIFICATION's added rows) NAMES:
 | row | disposition |
 |---|---|
 | RN:77-78 V3 / A1 (readers at a count edge) | STANDS as arithmetic; its example reader is Autopilot's beat step, not `processPendingTrigger` |
 | RN:189 BE-3 | AMENDED: the Auto unit case stays (T-N5b); its two live arms are VOID |
 | RN:524-529 T-N5b | AMENDED: it uses "its two-line rule" (a count rise, then `beatInBar == 0` of the same snapshot) -- the alternative RN itself allows -- and is renamed "A BAR READER IN AUTO"; bars unchanged. See the NOTE in section 4 |
 | RN:512 T-N4, RN:545 | STAND |
 | RN:639-655 L3a, L3b, L3c, L3d | L3a, L3b, L3c AMENDED (section 5); L3d VOID |
 | RN:436-441 A18 | AMENDED: "A clip FIRED quantised starts on the nudged beat" is struck; the rest stands |
 | RN:700-701 B1, :707-708 B4, :711-713 B6, :723-725 B11, :726-728 B12, :729-730 B13 | B1, B4, B6, B11, B12 AMENDED (section 6); B13 VOID |
 | RN:738 question 62 B | CLOSED by his row (BD:913-914); moot |
 | RN:759-763 H-7, RN:780-786 SF-1, RN:824-826 R4 | MOOT for clip fires (no bar-quantised fire exists); the PresetSelector sentence of SF-1 stands as filed; FM-2 is not run |
 | RN:789-791 SF-3 | MOOT for clip fires; stands as filed for a routine pad waiting for its bar |
 | RR:17-18 the dispatch constraint | AMENDED: "Manual and Link stay to its right"; Quantize is gone |
 | RR:49-53 rule 2, first paragraph | VOID |
 | RR:56-62 (a routine waits; autopilot waits) | STAND as engine facts; no gate row or Boris check of this lane uses the routine half (parked) |
 | RR:91-96 item 4 (the widths) | AMENDED: NB-8 |
 | RR:164-166 V8, V9 | V8 is true of 7bc6df7 only; V9 stands (autopilot) |
 | RR:219 V22 ("a waiting quantised fire would land") | STANDS with the reader "an autopilot beat step"; its remedy (NA-14) is replaced by NB-3 |
 | RR:390-395 NA-10 | VOID (R81 is void by his words; state R9b is void) |
 | RR:424-428 NA-16 | AMENDED: "`startFromOne()` counts a beat and NOT a bar" STANDS as a rule; the clip sentence is VOID; LR2 (c) and the routine half of (d) are VOID; reading R83 and HR-9 are PARKED |
 | RR:460 S3r's row; RR:527-538 T-RB1..T-RB5, T-RW1 | AMENDED: section 4 and section 5 |
 | RR:566-575 LR2 | AMENDED: section 5 |
 | RR:693-696 B-R5, RR:710-711 B-R10 | B-R5 VOID (replaced by B-3); B-R10 AMENDED |
 | RR:757-760 R81, R83; RR:785-786 HR-9 | R81 VOID; R83 and HR-9 PARKED for the actions lane |
 | RR:348-350 (NA-4's routine sentence), RR:748-749 R75, RR:763 | R75 AMENDED: its first sentence stands; its routine sentence is PARKED |
 The TRANSPORT lane's rows of TABLE Q5 (AM-1, TL-U3 / U3b / U3c, TR6 / TR6b, AM-8 / F13, TL-UC14 / UC15, K6, AM-21, AM-23,
 MU-3 / MU-4, RA:253-254's first fire row, TR23, D-6, checks 3, 5, 28, R2:304's "fired on a 1 by its own Snap") are
 NOT ruled here: they are that lane's delta, owed before its S1 (its adoption block of 21:33:30, item 5). This delta
 only fixes what main will be when that delta is written: no queue can be armed.
SIDE FINDING SF-1 OF RN: MOOT (table above).

### RB7 WHAT THE NUDGE STILL MOVES  -- amendment NB-7
His words stand whole (BD:859-860): "everything that is connected to BPM shifts forward or back. I mean everything.
If the user twist the knob in real time, or triggers a clip, that is not affected unless it's set to be quantized" --
and nothing can be set to be quantised live any more, so every fire and every knob is "not affected".
THE READERS OF THE PUBLISHED (SHIFTED) BEAT THAT REMAIN: the beat circle and "Bar N"; the beat, bar and phrase phases
in shaders, mappings, signals and connections; autopilot's beat steps and per-type timing; the slideshow, the beat
randomise and the MilkDrop playlist counters; the routine clock, a pad's bar start and a routine's loop return
(today, until the actions lane); the beat a take is recorded against; and, when the transport lane's S4c lands, a BPM
Sync clip's one cut on the next "1" (RN A18: its lock reads the bus -- STANDS). NOT readers: a fire, a knob, what
follows the sound.
THE LIVE ROWS OF S2 RE-STATED (full text in section 5): L3a's reader becomes an AUTOPILOT BEAT STEP (a layer on
"On Beat", 1 beat, between two image clips): each advance must land on the SHIFTED beat. L3b becomes "a fire never
waits, nudged or not". L3c's reader becomes a ROUTINE PAD's bar start (own quantize Bar, global forcing gone), the
one bar-line starter left on main; when the actions lane removes the pads it re-states L3c. L4 (the bar turns with
the shifted beat) is untouched and remains the reader-independent proof of the bar. RED arm for all three: RN's
build-mut-bypass, on the tracker's-line clause.

### RB8 THE THREE OLD BUTTONS AND THE ROW'S PLACE (R85 confirmed)  -- amendment NB-8
VERIFIED: E6, E7, E20. Reading R101 (told to him, not corrected: INFERRED consent) names the thirteen cells and their
order; if he corrects it, `rowOrder()` and `rowTexts()` in TopBarModel.h are the two lines that change.
WHAT GOES: `playButton_`, `pauseButton_`, `stopButton_` of today and their three callbacks (E6); RR's "R[]" routines
stop (NA-5's third bullet, HR-11, T-RB3b's twelfth text, T-RW5's routines-stop clause, MR23's wording); the Quantize
cluster (by quantize-out); the five multiplier buttons and the Manual BPM field (RR's S3r, stands).
WHAT STAYS IN THE TOP BAR, left to right: the Audio block (source, gain) | "Bar N" | THE ROW | the state word
(LOCKED / SEARCHING) | Manual | Link | flexible space | Master Signal | Master | Outputs | FPS | DSP.
THE ROW (one name per cell; `rowOrder()`): wheel, play, pause, stop, bpm, bpmMinus, bpmPlus, nudgeBack, nudgeText,
nudgeForward, half, double, tap, resync -- his thirteen in his order, plus the text his answer 61 asked for ("nudge X
ms"), between the two nudge cells as RR placed it. Where that text sits is question 145.
THE CELLS. The bar is 40 high; the inner area is 36 (E7). A cell is SQUARE, 36 x 36, cells are 1 px apart, corners
square, as the picture. Widths: wheel 36, play 36, pause 36, stop 36, bpm 96 (the caption "BPM" and the number in ONE
cell; a click on it opens RR's editor), "-" 36, "+" 36, "<<" 36, the nudge text 88, ">>" 36, "/2" 36, "x2" 36, "TAP"
48, "RESYNC" 68; thirteen gaps of 1. COMPUTED: 360 + 96 + 88 + 48 + 68 + 13 = 673.
Texts (`rowTexts()`; ASCII; Pitfall 6): ">" "||" "[]" "-" "+" "<<" ">>" "/2" "x2" "TAP" "RESYNC" -- pairwise
distinct (T-RB3b, now eleven). RR NA-5's tooltips stand. Upper-case TAP / RESYNC and the caption "BPM" are R101's.
THE FIT (COMPUTED on E7 and RR section 0 item 4; ASSUMED widths: the Master Signal label 86, the nudge text box 88;
taken from RR and not re-derived by me: the state word 62, Manual 82, Link 56):
 at 1728 (inner 1720): 240 + 46 + 673 + 6 + 200 + 522 = 1687: nothing shed, 33 px to spare.
 `fitTopBar` gives up width in THIS order (REPLACES RR's six-item list; the "Quantize:" caption no longer exists):
 1 DSP (55)  2 FPS (51)  3 COMPACT CELLS (cell 36 -> 30, bpm 96 -> 84, TAP 48 -> 40, RESYNC 68 -> 58: 90)
 4 the state word (62)  5 Master Signal (180)  6 Master (136).
 at 1512 (inner 1504): steps 1-3 -> 1491: the state word and both faders stay.
 at 1280 (inner 1272): steps 1-5 -> 1249: Master stays (RR shed all six).
 Never given up: a row cell, Manual, Link, Outputs; never: a cell narrower than its text (RR NA-6's rule and its two
 remedies STAND, re-based: remedy 2 is "the gap after the row 6 -> 2").
WHAT THE LEAVING FREES, against today's bar: Quantize 161, the three old buttons 82, the five buttons 140, the BPM
field 64 when Manual; against RR's layout: Quantize 161 and "R[]" 36 -- which is what pays for square cells (+ 175).
THE LIT STATE (a STATE display, set from the published byte only when it differs: Pitfalls 41, 59): exactly one of
play / pause / stop is lit -- Running: play; Paused: pause; Stopped: stop. Lit = the cell filled with the app's accent
and dark text, as the picture's play; unlit = the bar's cell colour. The colour constant is the builder's choice among
the existing AudioDNALookAndFeel colours EXCEPT `kRoutineCue` (reserved). The beat circle is RR's: it turns with the
published beat, stands where it is when paused, sits on beat 1 when stopped; it fills its 36 x 36 cell; its repaint
rect stays `getWheelRepaintBounds()` (Pitfall 57).
VG-0 AND THE VISUAL STATES: section 5 (VG-0 is still needed: the two ASSUMED widths and "what the bar looks like
before the row" are its job; its baseline is now the bar AFTER quantize-out).

### RB9 STAGES  -- section 4.

### RB10 WHAT BORIS CHECKS, AND HIS QUESTIONS  -- sections 6 and 7.

## 4 STAGES + ORDER
One builder context per stage; a builder builds, runs unit tests, writes probes and their self-tests; he never runs a
live row and never gives a gate verdict. Every live row, every mutant app's RED line and every verdict is Harmony's.
Every live batch: the lane's test-server build, the real app, `open -g`, the live lock, own-pid quit; no Output
window, no full-screen capture, no synthetic input; never while Boris's own Audio-DNA runs (RIG-RULES; the SCREEN-
SAFETY LAW). REPLACES RR section 4's rows from S1r down; RR's G-N0 and S1 rows are untouched.

LANE quantize-out (new; its own worktree and branch from main; merges to main by RIG-RULES' merge sequence)
| key | who | scope and the files it owns | needs | exit, and what it proves |
|---|---|---|---|---|
| QO-0 | Harmony | The RED arms on main 7bc6df7's test-server build: QL1, QL2, QL3 each print their FAIL line; `ctest -N` total noted. | nothing | Row QO-0: the rows can fail. |
| QO-1 | builder | ENGINE + MODEL (NB-6 Q-b, Q-d, Q-e, Q-f, Q-g, Q-h and the model half of Q-c). src/model/Layer.h (`triggerClip`'s parameter and queue branch; `processPendingTrigger`), src/model/Composition.h, src/model/Clip.h/.cpp, src/model/Autopilot.cpp, src/render/Renderer.h/.cpp (the counter only), src/MainComponent.cpp (the helper, the four call lines, the preamble apply, the legacy seek), src/recording/Program.cpp, src/recording/PerfStateCapture.cpp, src/api/ApiServer.cpp + src/test/TestServer.cpp (the `render_pending_fired` keys); the tests NB-6 lists, deleted or rewritten; tests/test_quantize_out.cpp (new: T-Q1..T-Q6); tests/test_composition.cpp (T-Q-C1..C3). The Clip inspector's combo is only made to compile (its `onChange` body emptied), not removed. | QO-0 | The new cases RED on the tree before, then GREEN; MQ1-MQ4 RED; every deleted / rewritten case reported by name with its reason. Proves no fire can wait and an old show loads. |
| QO-2 | builder | SURFACES + PROBES + DOCS (Q-a, the UI half of Q-c). src/ui/TopBar.h/.cpp (the label, the combo, the two layout lines, the seam), src/ui/ClipInspector.h/.cpp (the combo and its row slot; the row closes up), tests/test_master_signal_link.cpp (the four re-pins); .harmony/probe-quantize-out.sh / .py / -selftest.py (QL1-QL4; sources probe-quit-ours.sh, takes the live lock, deletes the takes it made) and the fixtures (an OLD show carrying "quantizeMode": 2, "bpmMultiplier": 2, clips with "beatSnap": true and "beatSnapMode" 2 and 3; an OLD take whose checkpoint has "quantizeMode": 2); the probe edits NB-6 lists; docs/claude/performance-controls.md (:48, :73), architecture.md (:94), recording.md, effects.md if it names Snap, CLAUDE.md (the capability "per-clip beat snap granularity" struck), .harmony/APP-INVENTORY.md (:56, :58, :186, :188; counts RE-COUNTED with `ctest -N`). | QO-1 | Unit green; the probe self-test prints "0 case(s) differ". Then Harmony: unit count, QL1-QL5, the boxes and routine-display probes, then VQ. |
| VQ | capture builder, five critic seats; Harmony's verdict | Q1 the top bar at 1728, 1512, 1280 (Manual off and on); Q2 the Clip inspector's Autopilot section for a video clip and an image clip; each against the same view of main. Window-only captures of the lane's own app. | QL rows green | Row VQ. Boris sees nothing before it is green. |

LANE nudge (one worktree and branch, as RN; no two builders in it at once)
| key | who | scope and the files it owns | needs | exit, and what it proves |
|---|---|---|---|---|
| S1 | builder | AS RULED (being built; not re-opened). | -- | as RN |
| S1r | builder | RR's S1r row STANDS with these changes only: src/model/BeatTimer.h declares `kTapStartsTimer` and `kResyncStartsTimer` in place of `kGesturesStartTimer`, and does NOT declare `kNudgeAfterStop`; src/analysis/BPMTracker.cpp: a Resync while not Running = `startFromOne()` (NB-3), not `applyStop()`; tests: T-G6 as amended, T-G6b new, T-N17 and T-N17b as amended (section 5). Step 0 only if S1 used the real `Layer::processPendingTrigger` in T-N5b: swap it for the two-line rule (see NOTE). | S1 green; Harmony's order against the transport lane's S4t (HR-1) | As RR, with MR27 replaced by MR35 and MR36 added. |
| S2 | builder | RN's S2 row STANDS (the wiring, the save key, the route, the anchor, the probe) with: step 0 = main (with quantize-out merged) taken in; `setBeatNudgeMs` unchanged; the probe's L3 rows as re-stated in section 5 (no route sets Quantize: none exists). | S1r, G-N0, quantize-out MERGED | As RN; then Harmony: G-N1, L1, L2, L3a-c, L4, L6, L9, L10. |
| S2r | builder | RR's S2r row STANDS with: src/MainComponent.h/.cpp also owns `clearAllLayers` (NB-1) and `setBeatTimer`'s Stop branch calls it; NO nudge statement in the Stop path (NB-4); the probe's rows are LR1, LR2 (a), (b), (d), (f), LR3, LR4a, LR7 as re-stated; the fixture show of LR2 / LR3 (section 5); tests/test_stop_clears.cpp (new: T-S1..T-S4); the build scripts of build-mut-gate, build-mut-edge, build-mut-fold and build-mut-stop. | S1r, S2's rows green | Unit green; MR18, MR20-MR22, MR25, MR31, MR33, MS1, MS4, MS5, MS7 RED; self-test "0 case(s) differ". Then Harmony: G-N1, LR1, LR2 (a)(b)(d)(f), LR3, LR4a, LR7, L10's lines, probe-video.sh and probe-seq-vram.sh still GREEN (FM-R2). Proves: stop empties the layers and is the "1"; pause holds only BPM Sync clips and writes nothing; Resync starts; a Tap does not. |
| S3a | builder (short) | RN's row STANDS; the ui_text "topbar" dump no longer lists a Quantize widget. | S2r's rows green | as RN |
| VG-0 | capture builder | BASELINE on the lane after S3a: V18 the bar at 1728, 1512 and 1280, Manual off and on (it has no Quantize; it still has the three old buttons and the five); V19 the learn overlay; V20 Boris's picture resolume-tempo-bar.png carried into the set as the reference. The manifest: every top-bar widget's bounds, the Master Signal label's measured width (FM-R1), the bar's inner height. | S3a | The manifest: are 86 and 36 true; if the inner height is not 36 or the label is over 119 (86 + the 33 spare), Harmony re-reads NB-8's sums before S3m -- the design does not change. |
| S3m | builder (short) | RR's row STANDS with: `rowOrder()` = NB-8's fourteen names; `rowTexts()` = NB-8's eleven; the width constants and the SIX-STEP `fitTopBar` of NB-8 (it returns the shed list AND `compact`); T-RB3b, T-RB4 as amended. | VG-0 | as RR; MR32 re-worded (below). |
| S3r | builder | RR's row STANDS with: the cluster laid in `resized()` is "from the Audio block's end to Link"; the cells are NB-8's (square, flush, the lit fill); the three OLD buttons, `onPlay`, `onPause`, `onStop` and their lambdas in MainComponent.cpp:612-628 are REMOVED (no "R[]"); `rightOfTempoBoundsForTest` re-pointed to Link; T-RW1, T-RW5 as amended; test_topbar_link_toggle.cpp's routines-stop case is deleted (the button is gone) and its Link bounds re-pinned. | S3m | Unit green; MR19, MR34, MS6, MS8 RED. Then Harmony: G-N1, LR4b, LR5, LR2 (e), and LR1 / LR2 / LR3 / LR4a again via "button"; build-mut-row. |
| S4 | builder | RN's row as RR amended it. STANDS. | S3r | as RN; then Harmony: L7, L8, LR5 (a). |
| S4r | builder (short) | RR's row STANDS: the seven targets Play, Pause, Stop, BPM minus, BPM plus, /2, x2 -> `tempoRowOp(op, Origin::Human)`. The "Beat stop" target therefore empties the layers too. The old bindable "Stop" (`GlobalStop`: stops all routines) and "Play / Pause" (the audio file) are NOT touched. | S4's rows green; HR-2 | as RR; then Harmony: LR6 as amended. |
| VG | capture builder, five critic seats; Harmony's verdict | States T1-T19 of section 5 against V18-V20. | S4r's row green | Row VG. Boris sees nothing before it is green. |
| S5 | builder | RR's row STANDS with: the pitfall sentence reads "the timer is gated at five sites; a Resync while held is a START from the 1; a Tap never starts; stop takes every clip off through `clearAllLayers` and is never an Undo step; the row's pause writes no clip; Open and New never touch it; the recorder clock treats a held timer as unmetered"; the manual says stop / pause / play / Resync / Tap as B-1..B-6 word them; NO sentence on Quantize, on a waiting fire or on a routine pad's start; recording.md is not touched by this lane. | VG green | Harmony reads the manual against the built app. |
ORDER: QO-0 -> QO-1 -> QO-2 -> Harmony's rows -> VQ -> merge quantize-out. In parallel, in the nudge worktree: S1 ->
S1r (neither touches a file of quantize-out). Then S2 (step 0 takes main in) -> Harmony's rows -> S2r -> Harmony's rows
-> S3a -> VG-0 -> S3m -> S3r -> Harmony's rows -> S4 -> S4r -> Harmony's rows -> VG -> S5 -> merge.
NOTE FOR HARMONY ON S1 (not a STOP). RN:524-529 lets S1's builder call the real `Layer::processPendingTrigger` in
T-N5b "if a test can build a Layer ... else its two-line rule, and the report says which". quantize-out deletes that
function. If S1's report says "the real function", S1r's step 0 swaps it for the two-line rule (bars unchanged) BEFORE
the lane takes main in at S2. If S1's packet can still be told: "the two-line rule". Nothing else of S1 reads Quantize,
the stop, the pause or the two constants (S1's row, RR:453; `kNudgePlusMeansLater` and NR5-A's literals are untouched).
BPMTracker.cpp ACROSS LANES: nudge S1, then S1r, then the transport lane's S4t -- HR-1 STANDS. Harmony constraint
(proposed, for S4t): "a Tap keeps the bar" must not move a beat field and must not start the timer while it is not
Running; S4t's exit re-runs tests/test_beat_timer.cpp whole (T-G4c and T-G6b are the cases that would catch it).
FENCES.
 - one-save lane (BindingManager's serializer, AppSettings.h): this delta adds NOTHING to either. The timer's state
   and the "held" state are in no file (RR NA-8 stands). S4 / S4r append the two binding fields RN / RR named, after
   BeatNudge, as before. quantize-out edits Composition.h's and Clip.cpp's save / load lines (three keys leave the
   writer): it merges BEFORE one-save's builder touches those functions, or one-save takes main in as its step 0.
 - outputs lane: the Outputs button keeps its place and is never given up by `fitTopBar`; no gate opens an Output
   window. Nothing else is shared.
 - transport lane: (1) Renderer.cpp `syncMedia` -- this lane owns the three advance lines, that lane the speed lines
   (HR-1 stands). (2) ClipInspector.cpp -- quantize-out removes the Snap combo first; that lane's S5a rewrites the
   panel with no snap. (3) MainComponent's two fire handlers -- quantize-out first; that lane's S2 then edits handlers
   with no gate. (4) TransportPause.h -- this lane never includes it and never assigns `paused` / `pausedAt` (E18;
   that lane's lint TL-L4 is the proof on its side; LR3 (g) and LR5 (g) on this side). (5) `applyClipPlaying` loses
   two callers here (the old buttons): RA TB-7's "for as long as they exist" is then over, and that lane's S0 table of
   call sites is counted after this lane's S3r or says which came first.
WHAT HARMONY RUNS HERSELF, AND WHEN: QO-0 before QO-1. After QO-2: unit count, QL1-QL5 (incl. probe-tsan-unit.sh),
VQ. G-N1 after every nudge builder stage. After S2: L1, L2, L3a-c, L4, L6, L9, L10. After S2r: LR1, LR2 (a)(b)(d)(f),
LR3, LR4a, LR7 via "handler", the four mutant apps, the two media probes. After S3a: VG-0. After S3r: LR4b, LR5, LR2
(e), LR1-LR3 and LR4a via "button", build-mut-row. After S4: L7, L8. After S4r: LR6. Then VG, then S5's manual read.

DISPOSITION OF EVERYTHING ELSE IN RR (a line not named here or in section 3 STANDS)
 NA-1 STANDS (the seam; LR3's clauses re-stated).  NA-2 STANDS; "`onStop` is renamed `onStopRoutines`" VOID (it is
 removed); the debug route gains no op (Resync is pressed through today's `POST /api/resync` at S2r and through
 `pressRowControlForTest("resync")` from S3r).  NA-3 STANDS.  NA-4 STANDS (neither the timer nor a
 stop's clears are in a take: NB-1 D1); its routine sentence PARKED.  NA-5 AMENDED: the "R[]" bullet VOID,
 texts per NB-8.  NA-6 STANDS, re-based on NB-8.  NA-7 STANDS (LR2's exact count).  NA-8 STANDS.  NA-9 CLOSED by "129
 b" (NB-4).  NA-10 VOID.  NA-11 STANDS.  NA-12 STANDS.  NA-13 STANDS, and now also covers a Resync while held.
 NA-14 REPLACED by NB-3.  NA-15 STANDS (it is the recorder's clock; its live clause is LR2 (d)'s "clockBeat").
 NA-16 AMENDED (RB6's table).  NA-17 STANDS.  NA-18 STANDS; build-mut-stop is added; MR23, MR27, MR32 re-worded.
 HR-1, HR-2, HR-4..HR-8, HR-10, HR-12, HR-14 STAND.  HR-3 STANDS (the overlay's old "Stop" must not read like the
 row's; its words are the naming lane's).  HR-9 PARKED.  HR-11 VOID.  HR-13 SUPERSEDED (asked and answered).
 R74 STANDS.  R75 first sentence STANDS, rest PARKED.  R76-R80 STAND.  R81 VOID.  R82 STANDS.  R83 PARKED.  R84
 REPLACED by R109.  Questions 125-129: closed by his words.
 RN: A2's two statements stay in the RESYNC branch only.  RN's S3b was already replaced by S3m / S3r.  RN L11 was
 already replaced by LR5.  RN B5, B10 STAND.

## 5 TESTS + GATE ROWS (pre-registered; a bar is met or reported, never loosened; each has its RED arm)
RN section 5 and RR section 5 STAND except the rows re-stated here. Harmony copies gate strings from here, then RR,
then RN, in that order of precedence.

UNIT, lane quantize-out (QO-1). RED arm for each: the tree before QO-1 (main 7bc6df7).
 T-Q1  `Layer::triggerClip` of a clip that is not the active one makes it the active ref in that call, for a clip
       whose JSON carried "beatSnap": true and "beatSnapMode": 2, and for one without the keys; the pending ref is
       invalid after it.
 T-Q2  100 fires through `Composition::fire` alternating two clips, with a snapshot stream of beat edges fed to
       `Autopilot::processFrame` between them: after every fire the layer's active ref is the fired one; the pending
       ref is never valid.
 T-Q3  Autopilot's advance onto a clip loaded from JSON with "beatSnapMode": 3 lands in the frame that decides it.
 T-Q4  A fired playable clip whose JSON carried "beatSnap": true has `playheadPosition == inPoint` after the fire,
       with a snapshot whose beatPhase is 0.6 (the legacy seek is gone; "42 default"). MQ1 RED.
 T-Q5  LINT (comments stripped): in src, none of `forcedSnap`, `quantizeModeToForcedSnap`, `processPendingTrigger`,
       `beatSnapMode`, `beatSnap`, `QuantizeMode`, `render_pending_fired` occurs; `quantizeMode` occurs only in
       src/recording/PerfState.h / .cpp, PerfStateCapture.cpp and the one load line of Composition.h; every line of
       src/model/Layer.h and src/core/DeckCommands.h that assigns `pendingTriggerColumn` assigns `-1`. MQ2 RED.
 T-Q6  `RoutineEngine`: with the forced snap Off, a routine whose own quantize is Bar waits for the bar edge and one
       whose own is Off starts inside `fire` (the two existing behaviours, pinned as this lane's promise to leave
       routines alone). RED arm: MQ4 on the live row QL4 (the engine is not changed; this case is GREEN before and
       after and is listed as a guard, not as a RED-first case).
 T-Q-C1 a show JSON with "quantizeMode": 2, "bpmMultiplier": 2 and a clip with "beatSnap": true, "beatSnapMode": 3
       loads: same decks, clips, layers as the same JSON without those keys (compared by `toVar`).
 T-Q-C2 `toVar` of any composition has no key "quantizeMode"; no clip has "beatSnap" or "beatSnapMode". MQ3 RED.
 T-Q-C3 a take checkpoint with "quantizeMode": 2 and a pending pair parses; `Program::compile`'s preamble has no
       `comp/quantize` row; the four take fixtures still load.
 MUTANTS: MQ1 the legacy seek left in. MQ2 `Composition::fire` keeps a snap parameter that one handler passes. MQ3
 `toVar` still writes "quantizeMode". MQ4 (app) the routine tick passes Bar as the forced snap.
UNIT, tests/test_beat_timer.cpp (S1r), amended and new:
 T-G6  RESYNC WHILE HELD IS A START (NB-3; replaces RR's T-G6). Paused at beatInBar 2, phase 0.7; `requestResync()`:
       on the applying hop the state is Running, totalBeatCount + 1, beatInBar 0, beatPhase 0.0, the level true,
       totalBarCount unchanged, `appliedResyncs()` + 1, `appliedStarts()` + 1, `appliedStops()` unchanged; the next
       100 hops advance the phase at the tempo. The same from Stopped. In Auto with a confident beat fed on that hop
       (RR T-G3b's arm 1): still beatInBar 0. MR35 RED (the old rule: the state stays Stopped).
 T-G6b A TAP NEVER STARTS (NB-3). Stopped, then Paused: a tap tempo request 120 -> 126: `bpm()` 126, the state
       unchanged, the nine accessors unchanged over 200 hops. MR36 RED (a tap request runs `startFromOne()`).
UNIT, tests/test_beat_shift.cpp (S1r), amended:
 T-N17 THE STOP HOP (PL:554-558, amended by NB-4): the arms WITHOUT a zero request are the rule -- D in {+250, -250,
       +40, -40} applied, phase in {0.2, 0.7}: the "1" block, totalBeatCount' == F's EXACTLY, not held, the applied
       value UNCHANGED (D), the sequence S's; the never-engaged arm writes no byte. ONE arm keeps a zero request
       (a Resync and a Stop queued in one hop): applied 0.0. MR11 stands.
 T-N17b A RESYNC WHILE HELD, THE NUDGE ENGAGED (NB-3; replaces RR's). Manual 120; D = -250 applied; Paused at
       tracker phase 0.2; PRECONDITION asserted: the published count is one behind the tracker's. A hand Resync
       (zero request + Resync on one hop): on that hop the "1" block, totalBeatCount' == F's + 1 EXACTLY, applied
       0.0, not held, the state bits Running; over the next 400 hops count' never falls and rises by 0 or 1 per
       hop. A second arm: D = +250 at phase 0.7. MR27' RED (the hop is treated as a stop hop: no edge).
 T-N19 STANDS and is now the live path. T-N18, T-N20, T-N7b STAND.
UNIT, tests/test_stop_clears.cpp (S2r; a Composition with 3 layers, clips on each, the headless MainComponent seam
 the project's command tests use -- the builder names it; if `clearAllLayers` cannot be reached headless he moves its
 body to a free function over (Composition&, RoutineEngine&, capture sink) in src/model and tests that). RED arm: a
 stub `clearAllLayers` that does nothing.
 T-S1  after it every layer's active ref is invalid, the pending ref invalid, crossfadeProgress 1.0 -- with one
       layer mid-crossfade (progress 0.4) and one whose clip is from a deck that is not the shown one. MS1 RED.
 T-S2  the Undo stack's size and top are the same before and after; a following Undo does not make any layer's
       active ref valid. MS5 RED (a `ClearActiveClipCmd` pushed).
 T-S3  no clip's `playing` is true afterwards for the cleared clips, and a clip that was NOT active keeps its
       `playing` as it was (after the transport lane this case also asserts `paused` unchanged; that lane adds it).
 T-S4  with a recorder capturing: NO point is captured by a Stop; a running routine and a waiting pad are both
       gone from the engine's status. MS7 RED (routines not stopped).
LINT T-G14 (RR), amended: `setBeatTimer(` is called on one line of src, inside `tempoRowOp`; `clearAllLayers(` is
 called on one line of src, inside `setBeatTimer`; the body of `setBeatTimer` holds no `capture(` and no
 `beatNudgeMs`; `requestRun(` occurs once outside BPMTracker's files; `applyClipPlaying(` is not called from any
 TopBar callback. MR25, MR33, MS4, MS6 RED.
MODEL / WIDGET (S3m, S3r), amended:
 T-RB3  `rowOrder()` == NB-8's fourteen names.   T-RB3b `rowTexts()`: the eleven texts of NB-8, pairwise distinct,
       ASCII. MR32' RED (nudge forward's text is ">").
 T-RB4 `fitTopBar` with a signal label of 86: inner 1720 -> nothing given up, 33 left; 1504 -> DSP, FPS, compact;
       1272 -> DSP, FPS, compact, the state word, Master Signal. With a label of 120 at 1720: DSP is given up. For
       every inner width from 1272 to 2200: the kept widths sum to <= the width; what is given up is a prefix of
       NB-8's six steps; a row cell, Manual, Link and Outputs are never in it.
 T-RW1 RR's, re-based: at 1728 x 40 nothing is given up and the cells are 36 x 36 (the bpm, text, TAP, RESYNC cells
       36 high); at 1512 and 1280 the result equals `fitTopBar`'s for the measured label; no text wider than its
       cell at 36 or at 30; "margin <n> px" printed. MR34 stands.
 T-RW5 RR's with: thirteen clickable controls; one click on RESYNC calls `onResync` once; two on TAP call
       `onTapTempo` once; the routines-stop clause is struck; TopBar has no member `onPlay`, `onPause`, `onStop`
       (a compile-time `requires` check). MR23' RED (TopBar: stop's click sends the Pause op). MS8 RED (the lit
       cell does not follow the byte: T-RW7).
 T-RW7 (new) the byte Running / Paused / Stopped lights play / pause / stop and exactly one; the same byte twice
       requests no repaint (Pitfall 59). RED arm: a stub setter.
 T-RW6, T-U6, T-RB1, T-RB2, T-RB5, T-RB6, T-RW2, T-RW3, T-C4, T-B7..T-B10, T-G1..T-G5, T-G7..T-G13, T-G3b, T-G4b,
 T-G4c, T-G12a-d, T-R2: STAND.
MUTANTS, new: MS1 (app) `clearAllLayers` skips the layer loop. MS4 (app) the Stop path zeroes the nudge. MS5 the
 Stop pushes a Command. MS6 (app) the row's play also calls the old play-all loop. MS7 `clearAllLayers` does not call
 `stopAll`. MS8 the lit cell is set from the last click, not from the byte. MR35, MR36 as above. MUTANT APPS:
 build-mut-stop (MS1 + MS4 + MS6) at S2r -- disjoint clauses: LR2 (a) layers, LR2 (a) "ms", LR3 (g). RR's four stand
 (MR27 leaves build lists: it was unit-only).

GATE ROWS, lane quantize-out (Harmony; strings exact; the rig above; Manual 120 on the 120 BPM click file)
QL1  A FIRE LANDS ON THE PRESS. The OLD-show fixture loaded (a clip with "beatSnapMode": 2, one with 3, the show's
     "quantizeMode": 2). 30 fires by `POST /api/trigger_clip` at pseudo-random times (fixed seed), alternating the
     two clips; `GET /api/composition` polled every 5 ms. "At once" = the first poll in which the layer's playing
     clip is the fired one comes <= 150 ms after the POST returned. BAR: 30 of 30 at once; "pendingClip" is null in
     every poll of the row; at least 20 of 30 land in a poll whose beatInBar != 0 or beatPhase >= 0.15 (they are not
     on a line); for the playable clip, "playheadPosition" in that poll is within 0.05 of its in-point.
     "PASS  QL1 fires land on the press: 30 of 30 within 150 ms, 0 queued". RED arm (QO-0, main): "FAIL  QL1: <k> of
     30 within 150 ms" with k <= 10 (a bar at 120 is 2.0 s).
QL2  AN OLD SHOW LOADS AND IS SAVED CLEAN. `POST /api/load_composition` of the OLD-show fixture: ok; the deck, clip
     and layer counts equal the fixture's; `POST /api/debug/save_composition`: the file has no "quantizeMode", no
     "beatSnap", no "beatSnapMode"; it still has "bpmMultiplier" (this lane's S3r drops that one). "PASS  QL2 an old
     show loads and saves without the quantise keys". RED arm (main): "FAIL  QL2: quantizeMode in the saved file".
QL3  A REPLAYED TAKE'S FIRES LAND AS RECORDED. Record a take of 8 fires at pseudo-random times, stop, replay: for
     each fire, (the poll time the layer's clip changed, minus the replay's start) differs from the point's recorded
     `t` in the take file by <= 150 ms: 8 of 8. Then the OLD-take fixture (checkpoint "quantizeMode": 2) replayed:
     its 6 fires the same way, 6 of 6. "PASS  QL3 replayed fires land as recorded: 14 of 14". RED arm (main, the old
     take): "FAIL  QL3: <k> of 6 within 150 ms" with k <= 3.
QL4  ROUTINES ARE AS THEY WERE. A saved routine with quantize "bar" fired by `POST /api/routine/fire` starts on a
     totalBarCount edge (not before it); one with "off" is running within 150 ms. `.harmony/probe-routine-display.sh`
     prints its PASS lines unchanged. "PASS  QL4 routines keep their own quantize". RED arm: MQ4's app ("FAIL  QL4:
     the off routine waited").
QL5  THE LAYER WORD UNDER TSAN. `.harmony/probe-tsan-unit.sh`: its own PASS line, 0 reports. probe-tsan.sh scenario
     d with the re-stated validity line. RED arm: none new -- this row is a required regression gate of Pitfall 63,
     and is declared as such (its RED arm is the probe's own self-test).
VQ   Five critic seats on Q1, Q2: 0 MUST. Pre-registered MUSTs: the word "Quantize" or "Snap" anywhere in the window;
     a gap or a mis-aligned control where the combo was; any top-bar control that moved other than by closing the gap.

GATE ROWS, lane nudge, re-stated (everything not re-stated STANDS: G-N0, G-N1, L1, L2, L4, L6, L7, L8, L9, L10, LR4a,
LR4b, LR6 -- LR6's step 1 additionally reads "every layer empty")
L3   THE SHIFTED BEAT IS WHAT BEAT-DRIVEN STARTS LAND ON; A FIRE NEVER WAITS. (replaces RN L3a-L3d)
     L3a nudge +250; a layer with autopilot "On Beat", 1 beat, two image clips: 30 successive advances; t_f = the
       first 5 ms poll in which the layer's playing clip changed. BAR 30 of 30: totalBeatCount in that poll is one
       more than at the previous advance; beatPhase < 0.15; the tracker's line at t_f has a fractional part between
       0.40 and 0.70. "PASS  L3a autopilot steps land on the shifted beat: 30 of 30".
     L3b nudge +250, then -250: 30 fires each by `POST /api/trigger_clip` at pseudo-random times: 60 of 60 show the
       new clip within 150 ms and at least 40 of 60 at beatPhase >= 0.15. "PASS  L3b a fire never waits: 60 of 60".
     L3c after a Resync at nudge 0 (n as RN), nudge +250: 6 fires of a saved routine whose own quantize is Bar
       (`POST /api/routine/fire`, then stopped before the next): the first poll of `GET /api/routine/status` that
       shows it running is taken within 150 ms after a poll with beatInBar == 0 and beatPhase < 0.15, and ((the
       tracker's line at that poll) - n) / 4 has a fractional part between 0.10 and 0.18. "PASS  L3c bar starts land
       on the shifted downbeat: 6 of 6".
     RED arm: build-mut-bypass -- L3a and L3c FAIL on the tracker's-line clause; L3b's RED arm is QL1's (main).
LR1  STANDS (pause holds, play runs on), plus: across the row no clip's "playing" changes in `GET /api/composition`.
LR2  STOP EMPTIES THE LAYERS AND IS THE 1; PLAY IS THE EDGE; THE LAYERS STAY EMPTY; A FIRE WHILE STOPPED LANDS;
     RESYNC STARTS, A TAP DOES NOT; OPEN LEAVES IT STOPPED. (replaces RR's LR2)
     Manual 120. The fixture show: layer 0 a BPM Sync image sequence, layer 1 a Timeline video (transition 2.0 s),
     layer 2 an image with autopilot "On Beat" 4 beats and a second clip, two saved routines (own quantize Bar,
     looping, each fires a clip on layer 2); a second show file whose "beatNudgeMs" is 37. Rounds 1-3 before any
     nudge is set; rounds 4-5 at nudge +40. In rounds 2 and 4 a second clip is fired on layer 1 100 ms before the
     stop (a fade in flight); in rounds 3 and 5 the routine is running and a second pad is waiting.
     Arm S posts `tempo_row` stop on the first poll whose beatPhase is in 0.65..0.85, arm F in 0.10..0.30; c = that
     poll's totalBeatCount.
     (a) STOP. From the applied poll, 400 polls: RR's beat clauses (phase 0.0, beatInBar 0, barCount 0, the level
         true, totalBeatCount == c EXACTLY, nine fields constant, "beatTimer": "stopped"); `/api/debug/beat_nudge`
         "ms" == its value before the stop (0, or 40 in rounds 4-5); within 150 ms of the POST and in every later
         poll of the window, NO layer of `GET /api/composition` has a playing clip and "pendingClip" is null on
         all; `GET /api/routine/status` shows no slot running or waiting.
     (b) FIRE WHILE STOPPED. `POST /api/trigger_clip` of the Timeline video: the layer's clip within 150 ms, and
         its "playheadPosition" differs between two reads 300 ms apart. Of the BPM Sync sequence: the layer's clip
         within 150 ms, "playing" true, its position equal in every read for 1.0 s.
     (d) PLAY. `tempo_row` play: in the applied poll totalBeatCount == c + 1, beatInBar == 0, the level true,
         totalBarCount unchanged; the sequence's position has changed within 1.0 s and its first changed read is
         within 0.05 of the held one; layer 2 (never fired, autopilot on) is STILL empty 2.5 s after play and no
         routine is running; "clockBeat" (M:MainComponent.cpp:6383) at the first poll after play is within 0.25 of
         its held value (RR NA-15).
     (f) RESYNC AND TAP, once after the rounds. Stop; `binding_action` "tapTempo" twice 500 ms apart: bpm within 120
         +/- 2, "beatTimer" still "stopped", the nine fields constant over 200 polls. Then Resync (S2r: `POST
         /api/resync`; from S3r: the row's own RESYNC through `pressRowControlForTest`): in the applied poll
         "beatTimer": "running", totalBeatCount == the held count + 1, beatInBar == 0, "ms" == 0 and
         beatNudgeAppliedMs 0.0. The same from pause (held mid-bar): "running", the count + 1, beatInBar 0.
     (e) OPEN, once, from S3r on: as RR's (e), unchanged.
     Five rounds: S, F, S, F, S. INFO arm (FM-R3): RR's, unchanged.
     "PASS  LR2 stop empties the layers and is the 1; play is the edge: 5 of 5 rounds; resync starts, tap does not;
     open leaves it stopped". RED arms: build-mut-stop: MS1 -> "FAIL  LR2a: layer <n> still plays"; MS4 -> "FAIL
     LR2a: ms 0, want 40"; build-mut-edge: MR2 -> "FAIL  LR2a: count c+1, want c" (rounds 1 and 3), MR3 -> "FAIL
     LR2d: no edge at play"; build-mut-fold (MR18) -> "FAIL  LR2d: routine clock jumped"; build-mut-row (MR33) ->
     "FAIL  LR2e: beatTimer running after open"; the lane at the S2 commit -> "FAIL  LR2f: 404".
LR3  PAUSE HOLDS WHAT IS SET TO BPM; THE REST PLAYS; NOTHING IS WRITTEN. (replaces RR's LR3)
     RR's fixture (a BPM Sync sequence, a BPM Sync video, a Timeline video of pinned length L) plus a second BPM
     Sync sequence and a second Timeline clip not yet fired, and a Timeline clip on a fourth layer that is PAUSED
     BY HAND before the row (through the `LayerTransport` `binding_action`). RR's PRECONDITION stands. PAUSE (not
     stop) for 5 s; the window starts 200 ms after the applied poll.
     (a) the nine beat fields constant over >= 900 polls.   (b) onsetCount rises by >= 8; rms above 0 in >= 90 %.
     (c1) the sequence, (c2) the BPM video: every position read equals the first; "playing" true in all.
     (d) the Timeline video: its position x L advances by the elapsed time within 10 %.
     (e) FIRED WHILE PAUSED: the second BPM Sync sequence is the layer's clip within 150 ms, "playing" true,
         position constant for 1.0 s; the second Timeline clip is the layer's clip within 150 ms and moves.
     Then play: (c1), (c2), and (e)'s sequence: within 1.0 s the position has changed and the first changed read
     is within 0.05 of the held one.
     (g) HIS OWN PAUSE SURVIVES: the hand-paused clip's "playing" is false and its position equal in every read
         from before the row's pause to 2.0 s after the row's play.
     "PASS  LR3 pause holds BPM clips only: beat held, sequence held, video held, free clip ran, fired clips
     right, his pause kept". RED arms: build-mut-gate: (a) FAILS (MR1) and "FAIL  LR3c2: video moved" (MR20);
     build-mut-stop (MS6): "FAIL  LR3g: his paused clip plays"; the lane at the S2 commit: "FAIL  LR3c1".
LR5  STANDS with: (d) lit follows the byte (unchanged); (e) the fourteen names of NB-8; (g) "thirteen" in place of
     "nine" where it counts cells; its last clause reads "across play and pause no clip's "playing" changes; after
     stop no layer has a playing clip". The string "buttons: 9 of 9" stays (the nine `tempo_row` ops).
LR7  STANDS (RR's three bars and strings), with ONE change of setup: two layers are playing when take A is
     recorded, so that its stop takes clips off. BAR 1 is unchanged and now also proves a stop's clears are not in
     a take. BAR 3's "the replay's position ADVANCES" stands (a replaying take is not stopped: reading R104).
VG   Five critic seats on T1-T19 against V18-V20: 0 MUST. (replaces RR's state list R1-R20, R9b)
     STATES, each with its model facts in the manifest (tempo, nudge, the byte, each layer's playing clip, every
     top-bar widget's bounds, each text's width against its cell, what `fitTopBar` gave up):
     T1 Running, Auto LOCKED, 128, "nudge 0 ms", 1728 (play lit)      T2 "nudge +12 ms"      T3 "nudge -500 ms"
     T4 the nudge editor open      T5 the BPM editor open, the number selected
     T6 PAUSED mid-bar, a BPM Sync clip and a Timeline clip on two layers; the deck and strips in frame (pause lit)
     T7 STOPPED from T6: stop lit, the circle on beat 1, every strip empty, the number still 128
     T8 STOPPED, then a Timeline clip and a BPM Sync clip fired (one moves, one stands on its first frame)
     T9 Manual 127.6      T10 Manual 400 ("+" and "x2" greyed)      T11 Manual 30 ("-" and "/2" greyed)
     T12 no tempo ("---", SEARCHING)      T13 1280 wide, Manual on, stopped (compact cells; Master kept)
     T14 1512 wide (compact cells)      T15 1728, Manual on
     T16 the keyboard bind overlay with the nine targets      T17 the MIDI-learn overlay with them
     T18 a row target selected in learn      T19 the standing learn title.
     Pre-registered MUSTs: RR's (the eight of PL:661-664; two top-bar buttons with the same text; a text wider than
     its cell; anything given up at 1728) plus: more or fewer than one of play / pause / stop lit in any state; the
     word "Quantize" or a button "R[]" or any of the three old buttons anywhere; a row cell that is not flush with
     its neighbour at 1728; a strip that still shows a clip in T7.
     Named questions the seats answer in words: does the row read as the picture V20 (cells, order, the lit play);
     in T7, can "stopped, layers empty" be told from "the app has nothing loaded"; in T6 and T8, does the standing
     BPM clip read as held, not broken; can ">" be told from ">>"; is a greyed "x2" legible as "cannot"; does the
     nudge text between "<<" and ">>" break the row's rhythm (question 145).
After every live batch: RN's closing line stands.

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; on his real screens and by his ear)
RR's B-R1, B-R4, B-R7, B-R8, B-R11, B-R13 STAND (B-R8's "Beat stop" pad now also empties the layers). RN's B5, B10
STAND. REPLACED here: RR's B-R2, B-R3, B-R5, B-R6, B-R9, B-R10, B-R12; RN's B1, B4, B6, B11, B12. VOID: RN's B13.
B-1  (after quantize-out) Open one of your old shows. Fire clips with the mouse, your keys and your pads, on and off
     the beat -> every clip starts the instant you press, from its beginning. The top bar has no "Quantize"; the clip
     panel has no "Snap". WRONG: a clip waits for a beat or a bar; a clip starts part-way in; the show does not open.
B-2  Music on, a few layers playing, some clips in BPM Sync and some not. Press pause (||) -> pause lights; the beat
     circle stands; the BPM Sync clips stand on their frame; the other clips keep playing; loudness and hits keep
     reacting. Press play (>) -> it all runs on from where it stood, no jump. A clip you had paused yourself on its
     layer is still paused. WRONG: a free clip freezes; a BPM clip runs on or goes black; your own paused clip
     starts; the picture jumps at play.
B-3  Press stop ([]) -> every layer strip is empty and the picture is empty AT ONCE (a cut, like a layer's X); stop
     lights; the circle sits on beat 1; the tempo number still shows the tempo; your routines have stopped. Press
     Cmd+Z -> nothing comes back. Fire a clip that is not in BPM Sync -> it plays. Fire a BPM Sync clip -> it
     appears on its first frame and waits. Wait for a "1" in the music and press play ON it -> beat 1 is your
     press and the waiting clip starts with it; layers you did not fire stay empty. WRONG: a clip stays on a
     strip; clips come back by themselves after play; the number shows 0; Cmd+Z refills the layers.
B-4  Set a nudge ("nudge +12 ms"). Stop, then play -> it still reads +12. Only Resync puts it back to 0. With a
     nudge set, "play is the 1" is moved by that nudge: that is what the number means. WRONG: stop or play
     changes the number.
B-5  Stopped: tap a new tempo -> the number changes, nothing starts, stop stays lit. Press Resync on a "1" you hear
     -> the beat starts from 1 on that press, play lights, the nudge reads 0. Do the same from pause -> the same.
     WRONG: tapping starts the beat; Resync leaves it stopped; Resync starts somewhere else in the bar.
B-6  At arm's length on your screen, beside your picture of Resolume's bar: is the row in your order, are the cells
     the size and shape you meant, can you see at a glance which of play / pause / stop is on, can you tell play
     ">" from nudge forward ">>", is "nudge +12 ms" readable where it sits (question 145), is "Bar 2" left of the
     circle wanted (question 146)?
B-7  Make the window narrow -> the row never loses a cell; FPS / DSP go first, then the cells get a little
     narrower, then the LOCKED word, then Master Signal, then Master. Is that the right order to give up?
B-8  (the nudge, replaces RN B1 / B4 / B11 / B12) Music on, Resync on the "1". Press ">>" a few times, then "<<" ->
     flashes, autopilot steps and everything that follows the beat come earlier, then later, against the music; the
     tempo number does not move. Fire a clip and turn a knob at any nudge -> both act the instant you do it. A clip
     ALREADY running in BPM Sync does not move with the nudge yet (it will once the clip transport's bar lock is
     built). WRONG: only some beat-driven things move; a fire waits; the tempo changes.
B-9  (a confirmation; a machine row proves it) Record a take while you pause, play and stop the beat, replay it ->
     the replay does not pause or stop your beat. A take you replay after pressing stop keeps replaying and puts
     its clips back (reading R104): tell us if stop should end a replay too.

## 7 QUESTIONS FOR BORIS (144..147; each has a default A; nothing waits. 148 and 149 are not used)
144. Stop empties every layer at once. One wrong press in a show empties the screen.
     A (default) One press, at once -- as in Resolume.
     B Stop only acts when you hold the button (or the pad) for half a second; a short press does nothing.
145. The text "nudge +12 ms". Your picture has the two nudge buttons side by side, with no text.
     A (default) The text sits between the two nudge buttons: "<<"  nudge +12 ms  ">>".
     B The two nudge buttons sit side by side as in your picture, and the text comes after RESYNC.
146. Today the words "Bar 1" .. "Bar 4" sit just left of the beat circle. Your picture has no such text.
     A (default) Keep it.
     B Take it out; the circle alone shows the beat.
147. Each routine pad has its own "Quantize" in its right-click menu and waits for the next bar by default. Quantize
     has left the top bar and the clips. Until actions replace the pads:
     A (default) Leave the pads as they are.
     B Take that menu out now; a pad starts the instant you press it.
The line that changes for each B: 144 -> one branch in `TopBar`'s stop cell and in the "Beat stop" binding case (a
press arms a 500 ms timer; the release before it cancels) + T-RW5's stop clause and LR5 (g)'s stop op. 145 ->
`rowOrder()` (the text's name moves to the end) + T-RB3, T-RW1. 146 -> one name leaves the bar's fixed-left list
(- 46 px) + T-RB4's sums. 147 -> NOT one line: the routine engine's lead-in (RR NA-16, HR-9) -- it goes to the
actions lane's architect, not to a builder here.
READINGS told with them (he corrects only what is wrong; next free reading after these: R111):
 R103 Stop also stops every routine, playing or waiting -- otherwise a routine would put clips back on the layers.
 R104 A take that is replaying is not stopped or paused by the row; it follows its own sound.
 R105 A BPM Sync clip you fire while the beat is paused or stopped appears on its first frame and starts moving when
      the beat runs (from stop: on your play press, the "1"). A clip that is not in BPM Sync plays at once.
 R106 Stop is a cut: the clips go at once, like a layer's X. No fade.
 R107 Cmd+Z never brings back what stop took off (your rule for the layer strip).
 R108 The row's pause never changes a clip's own pause: a clip you paused yourself stays paused after play, and
      nothing about the row's pause is saved in the show.
 R109 Resync while the beat is stopped or paused: your press is the "1", the beat runs from it, the nudge reads 0.
      Tapping a tempo while it is stopped changes the number and starts nothing. (replaces R84)
 R110 An old show's Quantize and Snap settings are dropped the next time it is saved. An old recording made with
      Quantize on replays each press at the moment you pressed, not on the line it landed on then.
Rules of this delta that rest on a reading he did NOT correct (INFERRED consent), and the line that changes if he
does: R87 / R100 (the clip's Snap goes) -> NB-6 Q-c and the clip half of Q-d. R101 (the thirteen cells, the caption
"BPM", upper-case TAP / RESYNC) -> `rowOrder()` / `rowTexts()`. R86 (every fire at once) -> none: his own words on R81
carry it. R97 (stop / pause / play in one sentence) -> none: it restates 135 and 136. R79 (the Bar text) -> question 146.

## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)
K1  STRONGEST: "quantize-out as a lane of its own, FIRST, puts engine work on the layer's trigger functions (Pitfall
    63) in front of a lane whose S1 is already being built, for a control he could simply stop using. Hide the combo
    in S3r and be done." It loses on three verified facts: the clip's own Snap queues with the combo gone (E10); an
    old show with the global value set queues with no control on screen (E13); the legacy seek breaks "42 default"
    (E12). And every live row of this lane and the transport lane that fires a clip needs "at once" to be true of
    ANY show, not of a fixture with the keys at 0. Cheapest refuting test: QO-0 -- if QL1's RED arm on main does NOT
    fail with the old-show fixture, the removal is not needed and W3 comes back.
K2  THE PENDING SLOT STAYS IN THE WORD, DEAD. A later caller could arm it again. Answer: there is no parameter,
    function or caller left to arm it; T-Q5 pins the tokens. The alternative (excise the bits now) re-packs the CAS
    word and rewrites the Undo snapshots the transport lane's S1 is about to rewrite. Refuting test: T-Q2 / QL1's
    "pendingClip is null in every poll".
K3  STOP CALLS `stopAll()` -- one routine call in a lane told to build no routine clause. Without it a routine refills
    the layers after play and question 135's option A, as read, is false. HB-1 is Harmony's lever. Refuting test: LR2 (d) round 3 with MS7.
K4  A STOP'S CLEARS ARE NOT IN A TAKE (D1). A take recorded across a stop replays with clips he had taken off. It is
    main's behaviour for X today (SF-B1); the review screen is where takes get re-modelled. Refuting test: none
    needed -- it is a stated limit (B-9, R104).
K5  THE HOLD BY dt = 0 IS INFERRED, and now carries more: a BPM Sync clip FIRED while held must show its first frame
    (Pitfalls 53, 56). If it shows black or "pending", the lane stops at S2r's rows; RR's fallback (the player
    paused, write-back skipped while held) is an architect line. Refuting test: LR3 (e), LR2 (b), FM-R2, state T8.
K6  A MIS-PRESSED STOP EMPTIES THE SCREEN, by mouse, key or pad (S4r). Default A is his own model (Resolume);
    question 144 asks before anything is built. Refuting test: his answer.
K7  THE FIT RESTS ON 33 PX AND TWO ASSUMED WIDTHS, and on RR's three widths I did not re-derive. T-RW1 turns a miss
    into a RED before a pixel is shown; VG-0 measures; `fitTopBar`'s compact step exists for exactly this.
K8  RESYNC STARTS THE BEAT FROM PAUSE TOO. In Auto the detector may score the kick he pressed on a hop later and the
    bar reads "2" (RR R7, FM-R3: unchanged exposure, now reachable from two more states). Measured, said in B-5.
K9  T-N17b's NEW RULE (count' = F + 1 exactly on a held Resync with the nudge engaged) is INFERRED from PL:235-250
    and RN A2 as RR quotes them; I did not re-read plan-nudge.md's A1 / A2 steps. RN A24 holds: the builder stops.
K10 R87 MAY BE CORRECTED after shows have been saved without their Snap values. He answered it with a panel that has
    no snap; the fields come back by reverting Q-c, the values in re-saved shows do not. Said in R110.
K11 NOTHING WAS RUN. Every rule here is from reading 7bc6df7 and the two rulings. Every new case is RED first.

## 9 WHAT IS NOT IN THIS LANE
- THE ACTIONS RE-MODEL and everything of routines and takes that touches it. PARKED for the actions lane, by name:
  RR's routines-stop button and its label (NA-5, HR-11); R75's routine sentence; R83 and HR-9 (a pad pressed while
  stopped); LR2's old clauses (c) and the routine half of (d); whether stop ends a REPLAYING take (R104); recording
  a stop's clears or an X in a take (SF-B1); the pad's Quantize submenu and a routine's own quantize (question 147);
  re-wording "routine" on any surface (the naming lane).
- A recording review screen and its Quantize (BD:971-978; a design is owed to him first).
- Excising the pending slot's bits from the layer word, the cancel helpers and `PerfState`'s three fields (K2).
- The transport lane's own delta for its rows of TABLE Q5 (RB6's last paragraph); a clip's saved pause
  (TransportPause.h); the BPM panel modelled on resolume-bpm-sync-panel.png; Beats "-" / "+" = 1 beat; the cut on
  the next "1"; "a Tap keeps the bar" (S4t).
- A fade-out stop; a visible "held" mark on a strip or a cell; a production route or OSC for the timer (HR-5);
  another hand ceiling (HR-8); a time uniform for "Structural Landscape"; PresetSelector's wrap test (SF-R4);
  holding the detector's scoring after a start (FM-R3); un-folding Tap / REST / OSC / Link.
- The one-save lane's keys and the outputs lane's Delay: nothing of either is read or written here.
- FROM THE STOPPED SYNC-DIAL BRANCHES (rulings-bf2.md H-17: the lane is superseded): this delta carries NOTHING new.
  What RN already carries stands: from lane/bf2 at 740b6d6, the [golden] case of tests/test_analysis_sync_thread.cpp
  and the accessor `appliedResyncs()` (RN:493-494). From lane/bf2-keys at 9eab9bd: nothing (PL:797-803 stands).
  Dropped with them: the sync dial itself, its top-bar control, its key targets and its owed gate rows (void by
  H-17). Both worktrees were pin-checked here, not searched.
SIDE FINDINGS (for Harmony's ledger)
 SF-B1 MAIN: the strip's X pushes an Undo step but captures no take point (E4), while a replay can put a clear back
       (E3, :1965). So a clear by X is in no take. Also: X IS an Undo step today, against his rule (E21); the
       transport / undo-live lane owns that.
 SF-B2 MAIN: `applyClearActiveClip` with a human origin has no caller (E4).
 SF-B3 RR's S2r row lists `kNudgeAfterStop` and `kGesturesStartTimer` in S1r's header file; both names are gone
       (NB-3, NB-4). A packet written from RR alone would re-create them.
HARMONY'S DECISIONS THIS DELTA ADDS (each with a default): HB-1 Stop calls `stopAll()` (default yes; alternative:
no routine call, reading R103 replaced by "a running routine puts clips back"). HB-2 quantize-out is a lane of its
own, merged before S2's packet and before the transport lane's S1 (default yes; alternative W1: stages QO-1 / QO-2
inside the nudge worktree before S2). HB-3 probe-boxes K5 `k5_queue_link_off` is struck from the boxes row (default
yes). HB-4 the S1 builder is told "T-N5b: the two-line rule" if his packet can still be told (default yes).

STATUS: DONE

## HARMONY ADOPTION (2026-10-04 22:40:14, session s-rta-1004b)
ADOPTED IN FULL: .harmony/.reports/s-rta-1004b/ruling-nudge-row2.md (status DONE; verdict "NEEDS REVISION; the plan's position stands"; 16 attacks ruled: 13 ACCEPT, 3 PARTIAL, 0 REJECT; 19 amendments NC-1..NC-19, each OVERRIDES this delta plan's body). It is the SECOND DELTA on the beat-nudge lane: precedence for the lane is now Boris's verbatim words > the adoption blocks at the end of s-rta-1004/plan-nudge.md and s-rta-1004/plan-nudge-row.md > this adoption > ruling-nudge-row2.md > s-rta-1004/ruling-nudge-row.md > s-rta-1004/ruling-nudge.md > the plans. Workflow run wf_afba4dd7-c9c (plan: architect opus high; seats gates 8 attacks / 2 MUST, stage-hands 8 / 1 -- papers whole (17,809 characters) in attack-nudge-row2-papers.md; ruling: architect opus max).
What I read myself before adopting: the ruling's returned verdict, stage list, decisions HB-1..HB-11, the facts to measure, the head of its strongest counter-argument, and its section 7 (questions, readings, the lines that change) in full. NOT read by me: sections 1-6 and 8-10 in the file -- the builders' and reviewers' spec; a gate string is copied only from section 5. Adopted with the context gauge at [CTX] 501,518 / 1,000,000 (50.2%), on Boris's word "don't lose any decisions": re-read this block first next session.
THE RULING'S CAVEAT, kept in view: "Nothing was run"; the top bar's widths are ASSUMED until the baseline capture VG-0 (FM-B2).
RULED: STOP clears every layer's tuple and writes no clip field (a clip fired again plays; his own pause survives), stops the routines on those layers as a layer's X does, then stops the beat on the "1"; never an Undo step; not captured in a take. PAUSE is the hold the first delta built (BPM Sync clips get dt 0; nothing stored). A Tap never starts; a hand Resync is a run-queue command that starts a held beat. The nudge survives stop and play. LIVE QUANTIZE LEAVES IN A LANE OF ITS OWN, "quantize-out", merged before this lane's S2 packet and before the transport lane's S1; the clips' two Snap keys stay in the file, unread. The row: thirteen flush cells; the top bar is 34 px high (cells 30).
BORIS'S ANSWERS (verbatim, binding-decisions.md, recorded 2026-10-04 22:40:14): "144 a"; "145 b"; "146 b"; "147 a". 144 A and 147 A are the ruling's defaults. 145 B: the text "nudge X ms" sits BETWEEN the two nudge buttons -- the ruling's section 7 names the change ("`rowOrder()` (one name moves) + T-RB3"); the ruling's own reading R112 is VOID. 146 B: the "Bar 1" .. "Bar 4" text is taken out -- the ruling names the change ("one name leaves the fixed list (- 46 px) + T-RB4's sums"). No architect delta is needed: the packets of S3m / S3r carry this block and those names.
HARMONY'S DECISIONS: HB-1..HB-11 at their defaults. Notes -- HB-1: stopOnLayer for every layer. HB-2: quantize-out is its own lane. HB-4 is moot: S1 is built and gated; its T-N5b uses Layer's two-line Bar rule (the builder's S-6), which is what the ruling's note asks. HB-5: the clip's two Snap fields stay until Boris confirms reading R100 in words. HB-6: 34 px until the visual gate. HB-9: question 147 was asked with 144-146 and answered A; FM-B4 is still measured and reported.
READINGS: the ruling numbers its readings R103..R114; Harmony's own R103-R108 were already told to Boris under those numbers, so the ruling's twelve are told to him as R109..R120 (R112 -> R118 is void by 145 B), on the page boris-open.html of this session's close. Cite the ruling's own as "nudge-row2 R103" etc. Next free reading: R121.
NOT COVERED BY THIS RULING (it ran beside S1's reviews): STOP-N1 and STOP-N2 of rulings-nudge.md. An architect line on both is OWED before the packets of S1r and S2.
STAGES. Lane quantize-out: QO-P (probe + fixtures) -> QO-0 (mine: the RED arms on main; if QL1 a does not fail on main: STOP) -> QO-1 (engine + model) -> QO-2 (surfaces, probes, docs) -> VQ (visual gate) -> merge. Lane beat nudge: S1 (DONE, my gate GREEN at a8afcfb) -> S1r -> S2 (step 0 takes main with quantize-out merged) -> my rows -> S2r -> S3a -> VG-0 -> S3m -> S3r -> S4 -> S4r -> VG -> S5 -> merge. BPMTracker.cpp: nudge S1, S1r, then the transport lane's S4t.
NOT STARTED in this session: nothing after S1 is built.
