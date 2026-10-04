# plan-transport-delta1 -- delta to ruling-transport after Boris's answers of 2026-10-03 23:46 (s-rta-1003b)

Author: architect. Read-only. Source read ONLY at main 34179a2 (`git show 34179a2:<path>`, `git grep ... 34179a2`).
Nothing built, run, launched or probed. Nothing under `.claude/worktrees/` read.
Labels: VERIFIED = read at 34179a2 at the cited line. INFERRED = reasoned from lines read, not run. ASSUMED = not read.
Boris is quoted only verbatim, from .harmony/binding-decisions.md:725-744 and .harmony/boris-feedback-backlog.md:288-309.
"Harmony constraint:" marks what Harmony requires. "Reading:" marks an interpretation of his words.
Base: .harmony/.reports/s-rta-1003b/ruling-transport.md (AM-1 .. AM-30) + the HARMONY ADOPTION at the end of
plan-transport.md. This file AMENDS that ruling: where they differ this file wins. Nothing is built yet.

QUESTION: five of Boris's answers change the ruled transport + undo-live design. What exactly changes?

APPROACH (stated first):
- X1 A BPM-synced clip gets a BEAT LOCK: a stateless phase lock that trims the clip's speed by at most 10 % so that
  the clip's own beat lines pass the playhead on the show's beats. It never seeks, so nothing ever jumps. Beat lines
  and the lock are computed from ONE pure header, so the lines cannot be somewhere the lock is not aiming at.
- X2 The hold (stage S3h) is built. A released hold plays on from the held frame; the lock then eases the clip onto
  the nearest beat (at most half a beat away).
- X3 The base already clamps every drag to in..out in the one function both surfaces call. Added: the Clip tab's own
  direct playhead writes go, and a marker dragged past the playhead carries the playhead with it.
- X4 The rule becomes a rule about FIELDS: no Undo or Redo writes a field the layer strip shows, on any layer that
  exists before and after the step. Bypass, solo, fold and layer order stop being Undo steps (two command classes are
  deleted). A layer move ends the Undo history (the older steps address rows by number). A whole layer may still
  come and go with Cmd+Z (he kept that); it comes back as it was, playing nothing, solo off.
- X5 The clip's rows already say "BPM" and "Beats"; the delta forbids the word "bar" in them, fixes the tooltips, and
  hands the bars lane one rule.
Stages: S0, S1, S2, (S2b only if FM-1 is over its bar), S3, S3h, S4a, S4c (new: beat lock engine), S4b, S5.

## 1 WHAT BORIS SAID (verbatim, with each question as asked)

His whole message (boris-feedback-backlog.md:291-298):
"answer to 12 questions. all defaults except for:
2 b
10 If we set the inpoint and endpoint on the timeline of the clip, I should not be able to drag outside of the points. It should be as if that is the extent of the timeline unless I let go and drag one of the in or out points.
4 For setting the clip beat marks, the clips are set with beats not bars. x2 or /2 are beat changes. bars will confuse this. A bar is for beats and is used in places where longer durations make sense and beats are used where exaggerations makes sense. We should be very clear where we are using beats and bars, but they are essentially the same thing like feet and inches
9 b
3 cmd z Does not change anything in the layer strip which is by default live based
12 b
One thing I didn't mention is that when the Video is in beats rather than adjust by speed mode, the beats are shown with lines in the play head area whereas if it was speed control, it's just the basic play head and with beats control there are lines for each beat in the play head area and the play head moves past them on time"

The questions as asked (the 12-question version; boris-feedback-backlog.md:300-306):
- Q2 "You keep the mouse down on the playhead and hold still. A (default) The clip keeps playing from under your
  hand. B The picture waits on that frame until you let go." -- he answered "2 b".
- Q10 "You drop the playhead beyond a clip's end marker. A (default) It stops at the end marker and the loop starts
  over. B It plays the part outside the markers once." -- he answered with his own rule (the line "10 ..." above).
- Q4 "How the BPM box and the Beats box belong together. A (default) ... two ways of saying one thing ... B Beats
  cuts the loop shorter while BPM stays." -- he picked no letter and ruled the unit (the line "4 ..." above).
  Harmony's reading (hers, INFERRED): default A stands for how the two boxes relate.
- Q9 "A BPM-synced clip left running for many minutes ... A (default) To line it up with the beat again, you fire
  it. B The app keeps nudging it back onto the beat by itself." -- he answered "9 b".
- Q3 "Cmd+Z and a layer that is playing. A (default) Bypass, solo, the order of the layers and effects are still
  undone, as today. B Cmd+Z leaves some of these alone while the layer plays -- say which." -- he answered with his
  own rule (the line "3 ..." above). It widens his rule of the afternoon: "Let's not allow control Z to change
  anything that is live in the layer strip. It changes anything else".
- Q12 -- "12 b". OPEN (the page's question 12 was rewritten after he opened it). It belongs to the notices lane.
  NOT in this lane; nothing here depends on it.
- Q1, Q5, Q6, Q7, Q8, Q11: "all defaults". Harmony's reading of what those defaults are (backlog :300): a paused clip
  plays from its beginning when fired; a removed layer comes back not playing; a reversed clip starts from its end;
  x2 next to Beats doubles the beats; a loaded deck's added layers stay; opening a show replaces the dial's number.
- The last sentence of his message is new feedback (BF35), not an answer to a question.

## 2 FACTS (verified at 34179a2 unless labelled)

The layer strip: every control and state it shows
- D1 VERIFIED src/ui/LayerStrip.cpp / .h. Controls:
  (a) X `clearBtn_` (.cpp:341-343) -> `onClearClip`; its tooltip says "(the clip comes back with Undo; a routine
      stop cannot be undone)" (:345-346).
  (b) B `bypassBtn_` writes `layer_->bypassed` (:347-352). (c) S `soloBtn_` writes `layer_->solo` (:353-358).
  (d) four transport buttons write the PLAYING clip's `reverse`, `playing`, `speed` (:370-394).
  (e) S fader writes the playing clip's `speed` (:404-413); the timer re-reads it (:841-850).
  (f) K fader writes `layer_->keyThreshold` (:423-426). (g) V fader writes `layer_->opacity` (:437-451); the timer
      re-reads it and sets the routine cue colour (:810-838).
  (h) V dropdown writes `layer_->keyingMode` or `layer_->blendMode` (:460-476).
  (i) F fader writes `layer_->transitionSpeed` (:485-488); F dropdown writes `layer_->transitionMode` (:497-502).
  States painted: (j) the thumbnail of the playing clip (:512-529, `updateThumbnail` :1018-1066); (k) routine bands
  (:877-917); (l) the transport rect: the in..out region and the playhead of the playing clip, taken from
  `transportViewOf` (:742-757; paint :533-563); a press or drag there scrubs (:958-963, :971-989);
  (m) the layer name (:565-576); (n) the playing clip's name -- `clip->name` for a source or an FX-only clip, the
  media file's name otherwise (:1067-1097); (o) the selection outline (:591-596); (p) the fold state (a folded row:
  `bandsShown`, LayerStrip.h bandsShown(); the flag is `Layer::folded`, src/model/Layer.h:211); (q) the ORDER of the
  strips = the order of `Composition::layers`.
  The strip shows nothing of a layer's effect stack. It only accepts an fx drop (LayerStrip.h:103-105).
- D2 VERIFIED the model fields behind D1: src/model/Layer.h:189 `name`, :190 `id`, :204 `opacity` (RelaxedFloat),
  :206 `bypassed`, :207 `solo`, :211 `folded`, :252 `blendMode`, :261 `keyingMode`, :262 `keyThreshold`,
  :282 `transitionMode`, :284 `transitionSpeed`; src/model/Clip.h:19 `name`, :127-128 `inPoint` / `outPoint`,
  :237-239 `playing`, `playheadPosition`, `beatsPlayed`.

Every Undo command that writes one of them today
- D3 VERIFIED the command classes (grep `class .*Cmd|class .*Command` over src): SetClipCmd, ToggleClipLockCmd,
  SwapClipsCmd (core/ClipCommands.h:80, :146, :188); SetColumnCountCmd, RemoveColumnCmd, ClearLayerClipsCmd,
  ClearActiveClipCmd, ToggleLayerFlagCmd, AddLayerCmd, RemoveLayerCmd, MoveLayerCmd, AddDeckCmd, InsertDeckCmd,
  RemoveDeckCmd, RenameDeckCmd (core/DeckCommands.h:54, :95, :309, :392, :435, :479, :538, :619, :675, :772, :917,
  :1012); EffectStackCmd (core/EffectCommands.h:82); TriggerClipCmd (core/TriggerCommands.h:62); CompositeCommand.
- D4 VERIFIED ToggleLayerFlagCmd writes `bypassed`, `solo` or `folded` (DeckCommands.h:438, :458-460). Its ONLY push
  sites: src/MainComponent.cpp:753 (bypass, from the strip), :762 (solo, from the strip), :6914 (fold, the menu).
- D5 VERIFIED MoveLayerCmd reorders the shared stack (DeckCommands.h:619-647). Its ONLY push sites:
  MainComponent.cpp:6934 (Move Layer Up), :6957 (Move Layer Down) -- menu commands on the selected layer.
  `LayerStrip::onLayerDragReorder`, `onBlendModeChanged` and `onFoldToggle` are declared (LayerStrip.h:94, :99,
  :100) and assigned nowhere in src (grep): a strip drag does not reorder layers at the pin.
- D6 VERIFIED the tuple and the playing clip: ClearActiveClipCmd (pushed at MainComponent.cpp:735, :7061),
  TriggerClipCmd (:4979, :5105), ClearLayerClipsCmd, SetClipCmd, SwapClipsCmd -- all ruled by the base (AM-6, AM-8;
  the first two are deleted).
- D7 VERIFIED no command class writes `opacity`, `keyThreshold`, `keyingMode`, `blendMode`, `transitionSpeed`,
  `transitionMode` or a layer `name` (the class list of D3). The Layer tab writes them straight into the model with
  no Undo step: src/ui/LayerInspector.cpp:150, :178 (opacity), :169 (blend), :324 (fade time), :339 (key threshold),
  :19-25 (name). LayerInspector.cpp holds no `bypass` and no `solo` (grep: 0 lines).
- D8 VERIFIED whole-Layer values land only where a layer is CREATED by the step: RemoveLayerCmd's undo re-inserts
  the removed Layer (MainComponent.cpp:6838-6845 captures it; DeckCommands.h:538-610), AddLayerCmd's redo and
  InsertDeckCmd's redo re-add layers (base R19).
- D9 VERIFIED EffectStackCmd with a Layer scope writes `layer->layerEffects` (EffectCommands.h:31-34, :108-118); it
  is pushed by an fx drop on a strip (MainComponent.cpp:1086-1091). The strip does not show that stack (D1).
- D10 VERIFIED the layer-order plumbing that only MoveLayerCmd feeds: `Command::affectsLayerOrder`
  (core/Command.h:34), CompositeCommand.h:50-53, UndoManager.cpp:99, :105, `refreshAfterUndoRedo(bool)`
  (MainComponent.cpp:5350-5370: it clears the cell selection).
- D11 VERIFIED a precedent for ending the history at a change that is no Undo step: "Loading a composition is not
  itself undoable -- drop stale history" + `undoManager_.clear()` (MainComponent.cpp:3017-3018); the reason is
  written at :3157-3160 ("a stale command left alive could re-resolve, by coordinate, a valid-but-WRONG cell").
- D12 VERIFIED tests that name the two classes: tests/test_undo_commands.cpp:1362 (MoveLayerCmd case), :1394
  (ToggleLayerFlagCmd case), :1472, :1508-1514, :2549-2557 (the random walk's flag step), :2604;
  tests/test_show_model.cpp:865-882 (a SECTION).

The drag
- D13 VERIFIED the Clip tab writes the playhead itself, unclamped, before it calls `onCuepointJump` (base R15:
  src/ui/ClipInspector.cpp:1368-1375, :1395-1399); the in / out drags write only the point (:1387-1393).
  `writeBack` enforces the out point only (src/render/ClipTransportSync.h:56-71): nothing brings a playhead that
  lies before the in point back into the range.

The beat clock and the BPM Sync branch
- D14 VERIFIED `FeatureSnapshot::beatPhase` "[0, 1) smooth sawtooth ramp between beats" (src/analysis/
  FeatureSnapshot.h:41), `trackerState` (:42; LOCKED = 2, :148), `totalBeatCount` (:137) with its contract
  (:128-136): a tempo VALUE changes only the rate; a realign (Tap / Resync / confident detection) moves the phase.
  `trackerRequestSeq` (:144; Pitfall 48, docs/claude/pitfalls.md:105). Pitfall 42 (pitfalls.md:93) forbids reading
  the beatPhase WRAP as a clock or an edge.
- D15 VERIFIED the quantized release runs on the render thread from the frame's snapshot, on the `totalBeatCount`
  delta (src/model/Autopilot.cpp:72-88; base R32: one caller, src/render/Renderer.cpp:513).
- D16 VERIFIED `syncMedia`'s BPM Sync branch does its OWN `featureBus_.read()` (Renderer.cpp:1660 video, :1721
  sequence) and pushes the speed with `setSpeed` before `advanceFrame`. The frame already has one snapshot,
  `frameSnap_ = featureBus_.read()` (Renderer.cpp:402). INFERRED: `frameSnap_` is what the release gets at :513
  (the argument there is named `snap`; its definition was not read), and a second read can return a newer
  snapshot than the release used in the same frame.
- D17 VERIFIED the player's clock is continuous: `currentTime_ += dt * speed * direction`
  (src/media/VideoPlayer.cpp:416; direction :411-414); a paused player does not advance (:400-401); `setSpeed` is an
  atomic store (src/media/VideoPlayer.h:81); `getDuration()` (:70). The direction the clock really runs in is the GL
  member `reverseNow_` (VideoPlayer.h:219) with no accessor; `getReverse()` (:85) is the setting, which a PingPong
  ignores (Renderer.cpp:1647-1648). ImageSequence: `setSpeed` :51, `getDuration` :45, `pingPongForward_` :136.
- D18 VERIFIED test mode has no analysis thread; `/api/inject_features` sets the snapshot, `beatPhase` and
  `totalBeatCount` included (src/api/ApiServer.cpp:237, :992-995; MainComponent.cpp:1828).
- D19 VERIFIED the Clip tab's beat lines today (base R29: ClipInspector.cpp:1265-1277). The strip's transport rect
  draws no beat line (D1 (l)); what it paints is `TransportView` and it repaints only when that changes
  (LayerStrip.cpp:759-770; Pitfalls 57, 59).
NOT read: BPMTracker (whether a typed or tapped tempo reports LOCKED and keeps `beatPhase` running -- already on the
base's S0 list); the sync-dial lane (bf2); ImageSequence.cpp; the GOP cache.

## 3 ITEMS

### X1 BEAT LOCK and BEAT LINES (his "9 b" and the beat-lines sentence)

THE BASE TODAY. Fork F9 = "rate from the tempo"; nothing is derived from the beat clock (plan T4 "THE MODEL");
AM-22: "F9 stands", FM-4 measures the drift and "no build change without his answer". Beat lines: AM-18's second
bullet (the Clip tab only).
BORIS. "9 b" (to: "B The app keeps nudging it back onto the beat by itself."). And: "when the Video is in beats
rather than adjust by speed mode, the beats are shown with lines in the play head area whereas if it was speed
control, it's just the basic play head and with beats control there are lines for each beat in the play head area
and the play head moves past them on time".

WHAT IS LOCKED TO WHAT
- The clip's beat coordinate: x = (playhead - inPoint) / (outPoint - inPoint) x beats, with beats = L x clipBpm / 60
  as in the base (L = the in..out length in seconds). A beat line sits at every whole x.
- The show's beat phase: the snapshot's `beatPhase`. It equals the fractional part of
  `totalBeatCount + beatPhase`, the continuous beat time of Pitfall 42.
- The lock holds the two fractional parts equal: a whole x under the playhead when the show's beat falls. Its error
  e = wrap(frac(s x x) - beatPhase) into [-0.5, 0.5), s = +1 while the clock runs forward, -1 while it runs
  backward. e > 0: the clip is ahead. e < 0: the clip is behind.
- It is a lock modulo ONE beat. It does not say WHICH line is on which beat, it does not put the loop's start on a
  bar, and it has no memory. Reading: "lines for each beat ... the play head moves past them on time" asks for
  exactly this; "bars will confuse this" speaks against a bar lock.
- Pitfall 42 cannot bite: the lock compares two phases. It never detects a wrap, never counts a beat and keeps no
  baseline, so a tick gap or a catch-up burst loses nothing.

HOW THE CORRECTION IS APPLIED: a bounded rate trim, never a seek
- speed pushed = bpmSyncRate(show BPM, clipBpm) x (1 + trim).
- trim = 0 while |e| <= 0.03 beat (the deadband); else trim = clamp(-e / 2, -0.10, +0.10).
  So: the error shrinks with a time constant of 2 show beats near the lock, and by at most 0.10 beat per beat far
  from it. The worst case (half a beat off) is back inside the deadband in about 6.8 beats (3.4 s at 120).
  One clip beat passes per show beat at trim 0, so the law is the same at every tempo.
- Why these numbers (INFERRED, by reading; FM-5 measures them): the snapshot moves in analysis hops of ~10.7 ms =
  0.021 beat at 120 BPM (D14, D16), so a phase read is up to that stale; a deadband of 0.03 beat keeps that noise
  from moving the speed at any tempo up to 168 BPM, and above it the trim it causes is under 2 %. 0.03 beat at 120
  is 15 ms, less than one frame. A 10 % speed change is continuous motion; it is not a position step.
- The trim is a pure function of this frame's position and this frame's snapshot. No state on the Clip, none on the
  player, none in the renderer beyond what the base's F19 store already keeps. No new shared field (Pitfall 63).
- The render thread does it: computed in `syncMedia`'s BPM Sync branch from the player's position read BEFORE
  `advanceFrame`, pushed through the existing `setSpeed`. It never waits, never decodes, never seeks.
- The lock is OFF (speed pushed = the rate, exactly) when any of these holds: the mode is Timeline; the clip's
  intent is not playing; the clip is held (X2); the tempo is unknown (`bpm` 0) or the tracker is not LOCKED; the
  length is not known; `beatLockApplies(beats)` is false (below).
- ONE snapshot per frame: the lock reads the snapshot the quantized release read in that frame (D15, D16:
  `frameSnap_`, or whatever :513 is given), not a second `featureBus_.read()`; the two reads in `syncMedia` go. Else a clip released on a beat can start with an error the release did not see.
  S0 re-reads this on the merged main, and whether the sync-dial lane leads the phase the release uses: the lock
  uses the same phase as the release, whatever that is by then.

SIGNATURES (new header src/render/ClipBeatGrid.h; pure; includes no player, no JUCE; used by the renderer, the
Clip tab, the layer strip and the tests)
- `double clipBeatAt(double playhead, float inPoint, float outPoint, double beats) noexcept;`
- `struct BeatLines { int count; double firstNorm; double stepNorm; };`
  `BeatLines beatLinesOf(float inPoint, float outPoint, double beats) noexcept;` -- line b (1 <= b < beats - 0.001)
  at inPoint + (outPoint - inPoint) x b / beats; count 0 unless 2 <= beats <= 64 (the base's rule, AM-18).
  `beatLineCount` of AM-18 becomes `beatLinesOf(...).count`.
- `double beatLockError(double clipBeat, double showBeatPhase, bool backward) noexcept;`
- `float beatLockTrim(double error) noexcept;`
- `bool beatLockApplies(double beats) noexcept;`
- constants `kBeatLockDeadband = 0.03`, `kBeatLockTauBeats = 2.0`, `kBeatLockMaxTrim = 0.10`.
- `bool VideoPlayer::movingBackward() const noexcept;` and the same on ImageSequence (GL thread only: the direction
  the clock runs in now, D17).
LINES AND PLAYBACK CANNOT DISAGREE because line b is, by definition, where `clipBeatAt` returns b, and the lock's
error is computed from `clipBeatAt`. Both surfaces draw `beatLinesOf`. TL-U21 pins the identity.

HOW IT LIVES WITH HIS OTHER RULINGS
- Every fire restarts ("restart"). A fire is never moved to fit the beat: the clip starts at its start (reversed: at
  its end) at once, or on the bar under Quantize. A clip fired BETWEEN beats with Quantize off then slides onto the
  NEAREST beat: if the beat fell less than half a beat ago the clip runs up to 10 % fast until it has caught up, else
  up to 10 % slow until the next beat catches it. The base's removal of the per-clip beat-phase seek stands (a seek
  into the clip is not "from its beginning").
- Under Quantize the clip starts on the frame of the beat: its first error is under one frame (0.03-0.05 beat), at
  most a 2.5 % trim for a moment.
- The playhead drag: "I wanted to keep playing at the same speed, but play from wherever I drop the play head." The
  lock never seeks, so a clip plays on from the drop point, always. In speed mode nothing else happens. In beats
  mode the lock then eases it onto the nearest beat: at most half a beat, at most 10 % off its speed, for a few
  beats. Reading: his "9 b" and "moves past them on time" are later and are about beats mode; the two together are
  met by "from the drop point, then onto the beat without a jump". It is Boris check 26.
- The hold (X2): a held clip is not trimmed (its player is paused). See X2.
- A tempo change (typed, Tap tempo value, Link, REST): only the rate changes (D14); the phase is continuous; the
  error stays inside the deadband or just outside it. The lock reads only the snapshot, so it follows a command one
  hop late (Pitfall 48); no code of this lane acts on "the tempo I just sent".
- A Resync, a Tap, a confident detection: the phase is realigned (D14). The clip does NOT jump: it sees a new error
  of at most half a beat and slides. A Resync does not restart a clip (NOT in this lane).
- Reverse: the clock runs backward, x falls, s = -1: the same lines are crossed on the beat.
- PingPong: `movingBackward()` gives s for the leg; at a turn with a whole beats number the phase is continuous.
- Tempo unknown, or the tracker not LOCKED: no lock; the rate comes from the base's last-known tempo (F19, AM-22's
  store). When the lock returns, the clip slides. If S0 finds that a typed or tapped tempo does not report LOCKED,
  the condition becomes "`bpm` > 0 and `beatPhase` moved since the last frame" and S4c says so in its report.
- x2, /2, a typed BPM or Beats, a moved in or out point: x changes scale, so the error steps by at most half a beat
  and the clip slides. Nothing seeks. (x2 from a locked clip stays locked: twice a whole number is whole.)
- A Beats number that is not whole. At each loop wrap x drops from beats to 0, so the phase steps by the distance of
  beats from a whole number. `beatLockApplies(beats)` = that distance <= kBeatLockMaxTrim x beats / 2: the step can
  be pulled back in under half a loop. 16.016 (a 29.97 fps file): yes, and the trim stays under 1 %. 14.6: yes --
  after every wrap the clip runs up to 10 % slow for about 6 beats, then sits on the beat for the rest. 2.5 or 6.5:
  no -- the lock is off, the clip keeps tempo and its lines pass off the beat. The lines are drawn either way.
  This is question D1 for Boris.
- A layer that is not drawn (bypassed, hidden, muted by solo): `syncMedia` does not run (base R7), so nothing is
  trimmed; when it is drawn again the clip slides.

FORKS
- F20 What is locked. CHOICE: the beat phase, modulo one beat, stateless. RUNNER-UP: an anchored position lock
  (clip beat = show beat count minus an offset taken at the fire or the drop). Why it loses: an offset taken between
  beats keeps the clip off the beat for good, so the offset would have to be rounded -- which is the phase lock plus
  a stored offset that every drag, Resync, x2, wrap and value copy must repair; and it reads the beat COUNT, which
  is Pitfall 42's ground.
- F21 How the correction is applied. CHOICE: a bounded speed trim. RUNNER-UP: a seek to the aligned position when
  the error is large. Why it loses: Boris, on the drag: "I do not want it to jump back to where it was or where it
  should be playing before I grabbed it." A seek is that jump, and a seek costs a decode round trip (Pitfall 56).
- F22 A Beats number that is not whole. CHOICE: the same lock where the step can be recovered, none where it cannot.
  RUNNER-UP: round the automatic number to a whole beat when a clip is switched to beats. Why it loses as the
  default: "default all bpms to 120" -- rounding the beats changes the BPM box away from 120. It is D1's other way.
- F23 Where the lines are drawn. CHOICE: the Clip tab's bar AND the layer strip's transport rect. RUNNER-UP: the
  Clip tab only (today's loop, R29). Why it loses: "the play head area" -- the strip is where a playing layer's
  playhead is watched during a show; the Clip tab shows only the inspected clip. Cost: `TransportView` gains the
  lines; they change only when beats, in or out change, so the strip repaints no more often than today.

AMENDMENT TEXT
AM-31 BEAT LOCK. REPLACES fork F9's "rate only" and the plan's sentence "The POSITION is integrated from the rate;
nothing is derived from beatPhase or totalBeatCount"; REPLACES AM-22's first line "F9 stands" and its FM-4 bullet;
CLOSES Q9.
- In BPM Sync the speed pushed to a video or a sequence is the base's rate x (1 + trim), with the error, the trim,
  the deadband, the limit and the off conditions written above, from src/render/ClipBeatGrid.h.
- The lock adds no field to Clip, Layer, VideoPlayer or ImageSequence except the two `movingBackward()` accessors.
  It calls `setSpeed` only: no `seekTo`, no `restart`, no write of `playing` or `playheadPosition`.
- It reads the frame's one snapshot, the one the release read (S0 pins the lines).
- AM-22's store (the last tempo read while LOCKED) and its FM-3 slew stand. FM-4 becomes a bar the lock must meet
  (5.6 below), not a report.
- TL-U19's name gains "times one plus the trim; with the lock off it is exactly the rate".
AM-32 BEAT LINES. REPLACES AM-18's second bullet.
- Both surfaces draw `beatLinesOf(inPoint, outPoint, beats)`: only in BPM Sync, only when the length is known, and
  only when two neighbouring lines are at least 3 px apart on that surface. One weight for every line: no heavier
  line every fourth beat (X5). In Timeline mode neither surface draws a line.
- Clip tab: the loop of R29 fed from `beatLinesOf`. Nothing else of the bar's painting changes.
- Layer strip: `TransportView` gains `int beatLines; int firstLineX; float lineStepPx` (what paint() draws, in
  pixels, so the change test compares what is painted: Pitfall 59); `transportViewOf(const Clip*, bounds, double
  mediaSeconds)`. The length comes from `std::function<double(const Clip&)> LayerStrip::mediaSeconds`, forwarded by
  DeckView and assigned in MainComponent to the base's length provider. The strip asks it in `refresh()` and, while
  the answer is 0, at most once per timer tick -- never in paint().
- Test route `GET /api/debug/strip_transport_ui?layer=N` (test mode): the transport rect, the in and out x, the
  playhead x, the line xs. `GET /api/debug/clip_transport_ui` also reports the bar's line xs and playhead x.

TESTS, RED first (stage S4c unless marked)
- TL-U21 "beat grid: line b sits where the clip beat is b; 19 lines for 20 beats, 14 for 14.6, 1 for 2, none below
  2 or above 64; a trimmed clip's lines lie inside in..out"
- TL-U22 "beat lock error: clip beat 3.25 against show phase 0 is +0.25; 3.75 is -0.25; running backward mirrors
  it; the error is never outside [-0.5, 0.5)"
- TL-U23 "beat lock trim: zero inside 0.03 beat; minus half the error outside it; never beyond 0.10 either way"
- TL-U24 "render level, a fake player and a fake beat clock: a clip started half a beat off is inside 0.03 beat
  after 12 beats; its position never steps back; the speed pushed stays within 10 % of the rate"
- TL-U24b "a tempo change from 120 to 90 on a locked clip: the error stays inside 0.05 beat"
- TL-U24c "a realign of half a beat (Resync, Tap): no position step; inside 0.03 beat within 12 beats"
- TL-U25 "the lock is off and the speed pushed is exactly the rate: Timeline mode, a paused clip, a held clip, a
  tempo of 0, a tracker that is not locked, a length that is not known, beats 2.5 and 6.5"
- TL-U25b "beats 14.6 in a Loop: after each wrap the error is inside 0.03 beat before the next wrap; beats 16.016:
  the trim never exceeds 1 %"
- TL-U26 "a reversed 16-beat clip crosses its lines on the beat; a ping-pong keeps the lock through both turns with
  no position step"
- TL-U27 "x2 and /2 on a locked clip: no position step; inside 0.03 beat within 12 beats"
- TL-U28 "after a drop the clip plays on from the drop point: no seek is requested by the lock and a quarter of a
  beat later the position is the drop point plus a quarter of a beat, within 10 %"
- TL-U30 (S4b) "layer strip: in BPM Sync the transport view carries the grid's lines and the rect repaints only when
  a line or the playhead moves a pixel; in Timeline it carries none; lines closer than 3 px are not drawn"
- Lint TL-L5 "src/render/ClipBeatGrid.h includes no player header and holds no `seekTo`, `restart(`, `setPlaying`
  or `playheadPosition`; in src/render/Renderer.cpp `beatLockTrim(` is used only as a factor inside a `setSpeed(`
  argument (2 lines)"
- Mutants: MU-30 the trim's sign flipped -> TL-U23, TL-U24 (live: TR15). MU-31 no deadband -> TL-U23. MU-32 no
  limit -> TL-U23, TL-U24. MU-33 `backward` ignored -> TL-U22, TL-U26. MU-34 the lock runs for a held or paused
  clip -> TL-U25. MU-35 the lines use the whole file, not in..out -> TL-U21 (visual gate: B10). MU-36 a seek to the
  aligned position after a release -> TL-U28 (live: TR17). MU-37 `beatLockApplies` always true -> TL-U25.
  MU-43 the lock does its own `featureBus_.read()` -> no unit case can see it, so TL-L5 also pins: "between
  `GLuint Renderer::syncMedia(` and the next function no `featureBus_.read()` occurs" (2 lines today, D16).
GATE ROWS: TR15 .. TR18, FM-4, FM-5, bars B10, B11 (section 4).

STAYS A QUESTION FOR BORIS: D1 (a Beats number that is not whole). Default: F22's choice.

### X2 THE HOLD (his "2 b")

THE BASE TODAY. AM-11: the drag is built without a hold. AM-12: "The hold, ONLY if Boris answers Q2 'wait while I
hold' -- stage S3h".
BORIS. "2 b" (to: "B The picture waits on that frame until you let go.").
NO FORK is left open by his words; AM-12 already specifies the stage and its gates.
AM-33 THE HOLD IS BUILT. REPLACES the words "ONLY if Boris answers Q2" in AM-12 and "(only if ...)" in section 4's
S3h, 5.1's "Conditional stages" (for S3h), G-U1, G-U4 and 5.3's TR8; CLOSES Q2; retires risk K2.
- S3h is a normal stage, after S3. AM-12's record, `Intent`, its end conditions and its cases stand word for word.
- AM-11's fourth bullet reads: "While the mouse is down the picture waits on the frame under it. No seek on
  mouseUp: after a drag the clip plays on from the last position." (AM-12 already wrote this switch.)
- The hold and the beat lock (X1): while a clip is held the lock is off (TL-U25). At Up the clip plays on from the
  held frame -- no seek, no jump (TL-U11, TL-U28) -- and a BPM-synced clip then slides onto the nearest beat. A
  speed-mode clip is not trimmed at all.
- The hold and the clamp (X3): a clip held past the out marker waits just inside it; at Up a Loop starts over and a
  OneShot ends (AM-13).
- A marker drag is not a hold (X3).
Gates: TL-U11, TL-U11b, TL-U11c [tsan], TL-U12, TL-U12b, TL-U12c; MU-H1 .. MU-H3; live row TR8 -- all unconditional.

### X3 THE DRAG CLAMP (his answer to 10)

THE BASE TODAY. AM-11: `scrubClip` "clamps to [inPoint, outPoint - 1e-4]"; both the strip and the Clip tab go
through it. AM-13: a drop past the out marker is a drop at the END of the range. Q10 asked the other way.
BORIS. "If we set the inpoint and endpoint on the timeline of the clip, I should not be able to drag outside of the
points. It should be as if that is the extent of the timeline unless I let go and drag one of the in or out points."
CHECK OF THE BASE AGAINST IT
- During the drag, both surfaces: MET by AM-11 -- every press and every drag event goes through `scrubClip`, which
  clamps before it seeks and before it writes the model. The strip paints the model's playhead (D1 (l)).
- GAP 1 (D13): the Clip tab still writes `clip_->playheadPosition` itself, unclamped, before the callback. AM-11
  says its handlers "call the same callback" but does not strike the direct write, and lint TL-L3 guards only
  LayerStrip.cpp. For one frame the playhead would be drawn outside the points.
- GAP 2 (D13): when a point is dragged past the playhead, the playhead is left outside the range -- before the in
  point nothing brings it back; past the out point a OneShot stops where it is, outside.
FORKS
- F24 The bar's scale. CHOICE: the bar keeps showing the whole file with the in..out region; the playhead cannot
  leave the region. RUNNER-UP: the bar rescales so in..out fills it. Why it loses: "unless I let go and drag one of
  the in or out points" -- on a rescaled bar both points sit on its ends and cannot be dragged outward. Reading.
- F25 A point dragged past the playhead. CHOICE: the point carries the playhead (the playhead is clamped into the
  new range at once). RUNNER-UP: leave the playhead until it next wraps. Why it loses: "as if that is the extent of
  the timeline" -- a playhead drawn outside the extent, and a OneShot resting outside it.
AM-34 THE RANGE IS THE TIMELINE. ADDS to AM-11 and AM-13; CLOSES Q10 (AM-13's outcome is his rule's outcome: the
end marker is the end of the timeline).
- src/ui/ClipInspector.cpp has no direct playhead write: the handlers only call the scrub callback. Lint TL-L3
  reads: "`onScrub` is assigned in src/MainComponent.cpp and forwarded in src/ui/DeckView.cpp; src/ui/LayerStrip.cpp
  and src/ui/ClipInspector.cpp have no `playheadPosition =`".
- In the Clip tab's in and out drags: after the point is written, when the clip's playhead is outside
  [inPoint, outPoint - 1e-4] the handler calls the scrub callback with the clip's id and its current playhead; the
  clamp lands it on the moved point. No hold is taken for a marker drag: from the in point the clip plays on inside
  the range; at the out point a Loop starts over and a OneShot ends, inside the range.
- A press or a drag in the dark part of either bar lands on the nearer point. Nothing is written on screen.
- NOT in this lane: reverse PLAY leaving a trimmed range (base SF-5); a point set by anything but the Clip tab's
  drag.
Tests: TL-U15 stands. New TL-U15b (S3) "a point dragged past the playhead carries it: the in point lands the
playhead on it; the out point puts it at the end of the range, and two frames later a Loop is at its start and a
OneShot has ended; after every marker drag event the playhead is inside in..out". Mutant MU-42 no carry -> TL-U15b
(live: TR7d). MU-10 (no clamp) stays.
Gate rows: TR7c gains "strip down 0.05 (before the in point 0.2): no read is below 0.2"; new TR7d (section 4).

### X4 CMD+Z AND THE LAYER STRIP (his answer to 3)

THE BASE TODAY. AM-10: "Q3 is widened; its default is today's behaviour" -- bypass, solo, layer order and effects of
a playing layer are still undone. AM-6's "live" = the tuple and the playing clip's media and transport. Section 10's
table: "MoveLayerCmd, ToggleLayerFlagCmd, RenameDeckCmd, ToggleClipLockCmd, EffectStackCmd | as today".
BORIS. "cmd z Does not change anything in the layer strip which is by default live based". Harmony's reading (hers):
"Cmd+Z changes NOTHING in the layer strip, playing or not (the strip is live by nature)."

THE STRIP RULE (reading, by FIELD, not by widget). A strip field is any model value the strip shows or sets (D1,
D2): of the LAYER -- name, bypass, solo, fold, opacity, key threshold, keying mode, blend mode, fade time, fade mode,
its position in the stack, and its tuple (what it plays, what fades out, what is queued); of the clip it PLAYS --
which media, its name, and its whole transport (AM-6's `TransportState`).
No Undo and no Redo writes a strip field of a layer that exists both before and after the step.

WHICH EDITS STOP BEING UNDO STEPS
| edit | today | after |
|---|---|---|
| a fire, a layer's X | a step (D6) | no step (base, AM-6 / TL-UC1) |
| Bypass, Solo (strip buttons) | ToggleLayerFlagCmd (D4) | no step |
| Fold (menu) | ToggleLayerFlagCmd (D4) | no step |
| Move Layer Up / Down (menu) | MoveLayerCmd (D5) | no step; the Undo history ends there (below) |
| opacity, key, blend, keying, fade time, fade mode, layer name | no step (D7) | no step -- unchanged |
| the strip's transport buttons, S fader, playhead, in / out points | no step | no step -- unchanged |
`ToggleLayerFlagCmd` and `MoveLayerCmd` are deleted with their tests (D12). The three handlers keep their live
write and their refresh, and push nothing. The plumbing only MoveLayerCmd fed (D10) is removed with it if the
builder's grep finds no other producer.

A LAYER MOVE ENDS THE UNDO HISTORY. The older steps name rows by number (base R17). Today a move is a step, so an
Undo unwinds it before it reaches them. Once a move is no step, an older Undo would land on another layer's row.
- CHOICE (F26): ONE function `void MainComponent::moveLayerNoUndo(int from, int to)` -- the fenced move, then
  `undoManager_.clear()`. Both menu handlers call it. The precedent and its reason are in the tree (D11).
- RUNNER-UP: keep the history and re-number -- `virtual void Command::layerMoved(int from, int to)` called on every
  step in the history, each command re-numbering its rows and the rows of a deck it holds by value. Why it loses
  for now: about ten commands, three of which hold whole decks, inside the lane's most intricate stage (base K1),
  for a rare set-up action. It is pre-specified as stage S1m if Boris answers D3 the other way.
- REJECTED: keep the step and refuse or spend its Undo. A refused step blocks every Cmd+Z behind it for good (the
  rule is unconditional, so it never clears); a spent one shifts the rows under older steps (AM-10 said so).
- Silent (AM-27): the menu's Undo item is disabled after a move -- a state, not an event text.

WHICH STEPS SKIP THE STRIP PART
- SetClipCmd, SwapClipsCmd, ClearLayerClipsCmd on a live cell: AM-6's landing rule, unchanged. ONE addition: in
  outcome 3 (same id, same media) the live clip's `name` is put back with its transport -- the strip shows it for a
  source or FX-only clip (D1 (n)). `Clip::TransportState` is unchanged; `landClipInCell` keeps the name beside it.
- AddDeckCmd, InsertDeckCmd, RemoveDeckCmd: no tuple write except the base's one (AM-8), which touches only a QUEUED
  trigger -- the strip shows nothing of the queue (base R8).
- EffectStackCmd: STAYS a step in all three scopes. Reading: a layer's effect stack is shown in the Layer tab, not
  in the strip (D1, D9), so it is "anything else" -- also when the effect was dropped on the strip. Question D4.
- ToggleClipLockCmd, RenameDeckCmd, SetColumnCountCmd, RemoveColumnCmd: no strip field; as the base rules them.

THE SAME FIELD EDITED ELSEWHERE. The rule is about the field. Opacity, blend, key threshold, fade time and the layer
name set in the Layer tab are not Undo steps today (D7) and never become one in this lane; the same holds for a
write by REST, MIDI, OSC, a routine or a take. No later lane may add an Undo command that writes a strip field
(lint TL-L2 below; docs).

A LAYER THAT COMES AND GOES (reconciled with what he kept)
- He kept, by "all defaults": a removed layer comes back with Cmd+Z, not playing (Q5); a loaded deck's added layers
  stay when its clip plays (Q8). Reading: he treats adding and removing a whole layer as an edit of the show; the
  strip rule protects what an EXISTING strip shows.
- So AddLayerCmd, RemoveLayerCmd and InsertDeckCmd stay steps, with the base's rules (AM-7, AM-9): a live layer is
  never removed (spent Undo, refused Redo); an idle one may go.
- The state a restored layer's strip comes back with: every setting it had when it was removed (name, bypass, fold,
  opacity, key, keying, blend, fade time and mode, its effects), an EMPTY tuple (base F15), and SOLO OFF. The same
  for a layer re-added by Redo of Add Layer or of Load Deck.
- Why that is not "changing the strip": no strip that is on screen changes -- a strip that was not there appears,
  as he left it. Solo is the one setting dropped because it is the one that reaches OTHER layers: a restored layer
  that solos would black out every layer that plays (F27; runner-up: restore solo as it was -- loses: Cmd+Z would
  change the live picture, the thing both of his rules forbid).
- `Layer restoredForUndo(Layer captured)` (core/DeckCommands.h): the empty tuple and solo off, used by the three
  restore paths.
- Open by his words: whether Cmd+Z may REMOVE an idle layer row at all (Undo of Add Layer; Undo of a Load Deck with
  nothing live). Default: yes, as the base. Question D2.

THE BASE'S LANDING RULE, RE-STATED FOR THE WIDER RULE. AM-6 stands whole (four outcomes, first execute never
filtered, the disposal rule, the column floor) with the name added to outcome 3. AM-7 stands (a refused Redo for
Remove Layer and Remove Column). AM-8 stands. AM-9 stands and is no longer ASSUMED: he kept its default. F13 reads:
"live" = AM-6's three slots and the clip's media, name and transport; "strip field" = the list above, live or not.

THE COMMAND TABLE (replaces section 10's last-but-one row; every other row stands)
| command | Undo | Redo |
|---|---|---|
| ToggleLayerFlagCmd, MoveLayerCmd | deleted: bypass, solo, fold and a layer move push no step; a layer move clears the history | -- |
| SetClipCmd, SwapClipsCmd, ClearLayerClipsCmd | as section 10; outcome 3 also keeps the live clip's name | same |
| RemoveLayerCmd | the layer returns through `restoredForUndo`: its settings, an empty tuple, solo off | REFUSED over a live layer (AM-7) |
| AddLayerCmd | as section 10 | re-adds what the Undo erased, through `restoredForUndo` |
| InsertDeckCmd | as section 10 | as section 10; re-added layers through `restoredForUndo` |
| RenameDeckCmd, ToggleClipLockCmd, EffectStackCmd | as today | as today |

THE 200-STEP WALK (TL-UC16), re-stated. Its name gains: "...; no step changed a strip field of a layer that exists
before and after it". Its random actions gain the edits that push no step (bypass, solo, fold, an opacity and a
blend write, the transport buttons) between the Undo and Redo steps; one of the three seeds moves a layer twice and
asserts the history is empty right after each move. The strip fields are compared through a TEST-side helper
`StripState stripStateOf(const Composition&, uint32_t layerId)` (tests only; no src type).

TESTS, RED first (S1)
- TL-UC17 "no command's Undo or Redo changes a strip field of a layer that exists before and after the step: name,
  bypass, solo, fold, opacity, key, keying, blend, fade time, fade mode, position, tuple, the playing clip's name,
  media and transport (every Command class)" -- the table of TL-UC14, with the strip fields.
- TL-UC18 "Undo of Remove Layer brings the layer back with its settings, an empty tuple and solo off; no other
  layer's strip field changes; Redo of Add Layer and of Load Deck re-add their layers the same way".
- TL-UC19 "a same-clip landing on a live cell keeps the live clip's name".
- Lint TL-L2 gains: "`ToggleLayerFlagCmd`, `MoveLayerCmd` and `affectsLayerOrder` nowhere in src; in
  src/core/*Commands.h no `bypassed =`, no `folded =`, no `opacity`, no `blendMode =`, no `moveLayer(`, and
  `solo =` only as `solo = false` inside restoredForUndo; in src/MainComponent.cpp `moveLayer(` only inside
  moveLayerNoUndo, which holds `undoManager_.clear()`".
- Mutants: MU-38 a restore keeps solo -> TL-UC18. MU-39 outcome 3 drops the name -> TL-UC19, TL-UC17. MU-40
  moveLayerNoUndo without the clear -> TL-L2. MU-41 a command that writes `bypassed` on Undo -> TL-UC17, TL-L2.
- T6 (expectation changes) gains: tests/test_undo_commands.cpp:1362 and :1394 deleted (their classes are gone);
  :1472, :1508-1514, :2604 lose their ToggleLayerFlagCmd / MoveLayerCmd lines; the random walk's flag step
  (:2549-2557) becomes a direct write with no step; tests/test_show_model.cpp:865-882 drives
  `Composition::moveLayer` directly. G-U2's "cases deleted" counts them.
ALSO (D1 (a)): the X button's tooltip promises "the clip comes back with Undo". After the base that is false. It
reads: "Clear this layer's clip. Also stops every routine playing on this layer. This cannot be undone." -- a
standing description, not an event text (AM-27). S1.

AM-35 THE STRIP RULE. REPLACES AM-10 whole and the table row named above; ADDS the name to AM-6's outcome 3; ADDS
`restoredForUndo` to RemoveLayerCmd, AddLayerCmd, InsertDeckCmd; CLOSES Q3; relabels AM-9 (kept by Boris) and AM-3
(Q1's default is now his: "all defaults" -- Harmony's reading of which default, backlog :300; AM-3's narrow-rule
fallback is dead, FM-2 is still measured and reported, nothing depends on it).

### X5 UNITS (his answer to 4)

THE BASE TODAY. The rows are captioned "BPM" and "Beats" with "/2" and "x2" (AM-17, B3); x2 doubles the beats (Q7's
default, which he kept). Beats = L x clipBpm / 60, two views of one number (F8; Harmony's reading of his answer: A
stands).
BORIS. "For setting the clip beat marks, the clips are set with beats not bars. x2 or /2 are beat changes. bars will
confuse this. A bar is for beats and is used in places where longer durations make sense and beats are used where
exaggerations makes sense. We should be very clear where we are using beats and bars, but they are essentially the
same thing like feet and inches".
NO FORK: the base already builds beats. What the delta pins:
AM-36 UNITS. ADDS to AM-17, AM-18 and bar B3.
- The clip's length row is captioned exactly "Beats"; the tempo row exactly "BPM". "/2" and "x2" sit on the Beats
  row and change the beats (AM-17, Q7 closed).
- Tooltips (standing text): Beats box "Length of the loop in beats"; BPM box "The clip's own tempo, in beats per
  minute"; "/2" "Half as many beats: the clip plays twice as fast"; "x2" "Twice as many beats: the clip plays half
  as fast".
- No caption, tooltip or painted string of the BPM row, the Beats row or the two bars holds the word "bar" in any
  form (B3). The beat lines are one per BEAT, all of one weight; nothing marks a bar on a clip (AM-32).
- The per-clip Snap setting and the Quantize menu name bars today; they launch a clip, they do not set it, and they
  are outside this lane.
NOTE FOR THE BARS LANE (bf7) -- not built here:
1. A clip is never set in bars. No bars lane widget on a clip's rows may take or show bars.
2. Every number box, menu or readout that shows a musical length carries its unit in its caption or beside the
   number ("beats" or "bars"). A caption that names no unit ("Loop length", "Content length" -- ruling-bf7 AM6,
   already withdrawn by H-1) does not meet his rule.
3. Beats and bars are one measure at two scales (1 bar = 4 beats at the pin, FeatureSnapshot.h:46): a place that
   shows bars may say so once; it must not show a bare number that could be either.
4. The bars lane lists every place that shows a musical length with its unit, as a table, before it builds.

## 4 STAGES + GATES (X6)

### 4.1 Which of ruling-transport's amendments change
| base | what happens to it | by |
|---|---|---|
| AM-3 | relabelled: F1 is Boris's now (Q1 default, "all defaults"; Harmony's reading of which default). The narrow-rule fallback is dead. FM-2 is still measured and reported | AM-35 |
| AM-6 | stands; outcome 3 also keeps the live clip's name | AM-35 |
| AM-9 | stands; no longer ASSUMED (Q8 default kept) | AM-35 |
| AM-10 | REPLACED whole | AM-35 |
| AM-11 | its fourth bullet REPLACED (the picture waits while the mouse is down) | AM-33 |
| AM-12 | the words "ONLY if Boris answers Q2" struck; the rest stands word for word | AM-33 |
| AM-13 | stands; added to (direct writes struck, the marker carry) | AM-34 |
| AM-17 | stands; added to (captions, tooltips, no "bar") | AM-36 |
| AM-18 | its second bullet (the beat lines) REPLACED; the rest stands | AM-32 |
| AM-22 | "F9 stands" and the FM-4 bullet REPLACED; the store and FM-3 stand | AM-31 |
| section 4 | the stage list below replaces it; S3h is unconditional; S4c is new | this section |
| section 5 | pins and rows changed below; every row not named here stands | this section |
| section 6, 7 | checks 7, 10, 20 replaced; checks 24-32 added; Q1-Q10 closed; D1-D4 new | sections 5, 6 |
| section 9 | K2 (no hold) and K5 (rate, not position) retired; K1 grows (X4) | section 7 |
| section 10 | one table row replaced, three amended (X4); a stage row for S4c | X4, below |
Untouched: AM-1, AM-2, AM-4, AM-5, AM-7, AM-8, AM-14 .. AM-16, AM-19 .. AM-21, AM-23 .. AM-30; H-1 .. H-5.
Plan forks after this delta: F6 = the hold (AM-12's form). F9 = the beat lock (AM-31). F13 widened (AM-35).
New: F20 .. F27. All other forks as the ruling left them.

### 4.2 Stages (one lane; one builder context per stage; RED first, by id, as the base)
- S0 RE-BASE NOTES. The base's list, plus: where the frame's snapshot is read and what the release at
  Renderer.cpp:513 is given (D16); whether the sync-dial lane leads the phase the release uses; whether a typed, a
  tapped and a Link tempo report LOCKED and keep `beatPhase` moving; what `trackerState` and `beatPhase` read in
  test mode after `set_bpm` with no injected beats (if LOCKED with a phase that stands still is reachable, the lock
  gets one more off-condition -- "the beat time did not advance since the last drawn frame", one GL-thread member --
  and TL-U25 one more clause; S0 says which); `reverseNow_` and the sequence's leg; every push site of
  ToggleLayerFlagCmd and MoveLayerCmd on the merged main (D4, D5) and every producer of `affectsLayerOrder` (D10);
  whether `onLayerDragReorder` is still unassigned (D5); whether `/api/bpm` reports `beatPhase` (FM-4); whether a
  REST route can run a menu command (TR19); the width of the strip's transport rect at both window sizes.
- S1 UNDO-LIVE + THE STRIP RULE. The base's S1 with AM-35. Files added to the base's list: MainComponent.cpp (the
  three flag handlers, `moveLayerNoUndo`, the removal of D10's plumbing), core/DeckCommands.h (the two classes
  deleted, `restoredForUndo`), ui/LayerStrip.cpp (the X tooltip), the tests of D12.
  Proves: the base's 21 S1 cases + TL-UC17, TL-UC18, TL-UC19 (24); lint TL-L2 as widened; G-U5.
- S2 RESTART. As the base (AM-3 in its F1 form).
- S2b (only if FM-1 is over its bar). As the base.
- S3 DRAG. AM-11 (with AM-33's bullet), AM-13, AM-34. Proves: TL-U10, TL-U13, TL-U14, TL-U15, TL-U15b; lint TL-L3
  as widened; lint TL-L1 in its final form.
- S3h HOLD (built). AM-12, AM-33. Files: render/ClipTransportSync.h, MainComponent.cpp, ui/LayerStrip.*,
  ui/ClipInspector.*, tests. Proves: TL-U11, TL-U11b, TL-U11c, TL-U12, TL-U12b, TL-U12c; MU-H1 .. MU-H3; G-U4 at 7.
- S4a BPM ENGINE. As the base, and the beats math and `beatLinesOf` live in the new src/render/ClipBeatGrid.h.
  Proves: the base's 7 cases + TL-U21 (8); G-U4 at 8.
- S4c BEAT LOCK (new; after S3h and S4a, before S4b). AM-31. Files: render/ClipBeatGrid.h, render/Renderer.cpp
  (`syncMedia`'s two BPM Sync branches; the two `featureBus_.read()` go), media/VideoPlayer.h,
  media/ImageSequence.h (the accessor), tests. Proves: TL-U22, TL-U23, TL-U24, TL-U24b, TL-U24c, TL-U25, TL-U25b,
  TL-U26, TL-U27, TL-U28 (10); TL-U19 gains its trim clause (the base's clause rule: the same TEST_CASE, shown RED
  by id in S4c's log); lint TL-L5; `.harmony/probe-video.sh` whole (`PROBE-VIDEO GREEN`: a speed that changes every
  frame under the reverse GOP cache, Pitfall 62).
- S4b BPM WIDGETS + BEAT LINES. The base's S4b with AM-32 and AM-36. Files added: ui/LayerStrip.*, ui/DeckView.*
  (the forward), the strip dump route. Proves: TL-U18d, TL-U20, TL-U30. Ends at the VISUAL WORK GATE.
- S5 PROBE + DOCS. 23 rows, 5 measurements; the docs of the base plus: the strip rule and the history end at a
  layer move (docs/claude/performance-controls.md), the beat lock and ClipBeatGrid.h (docs/claude/rendering.md),
  one new Pitfall ("a BPM-synced clip is trimmed, never sought; lines and lock come from ClipBeatGrid.h"), one more
  ("no Undo command writes a strip field; a layer move ends the history"). Harmony assigns the numbers.
- S1m (only if Boris answers D3 "keep the history"): `Command::layerMoved(int from, int to)` on every step in the
  history; `moveLayerNoUndo` calls it and no longer clears. Its own small plan first; its cases are not registered
  here.
What the other lanes get, added to the base's list: bars lane -- the note of X5; Timeline lane -- ClipBeatGrid.h is
the only source of a clip's beat positions; Edit menu -- the Undo item is disabled after a layer move.

### 4.3 Unit gates: the changed pins
- 5.1's heading reads "71 cases + 6 lint cases; conditional stage S2b: + 1" (49 + 6 of S3h + 16 new; 5 lints + TL-L5). New ids, with their stage: TL-UC17,
  TL-UC18, TL-UC19 (S1); TL-U15b (S3); TL-U11, TL-U11b, TL-U11c, TL-U12, TL-U12b, TL-U12c (S3h, no longer
  conditional); TL-U21 (S4a); TL-U22, TL-U23, TL-U24, TL-U24b, TL-U24c, TL-U25, TL-U25b, TL-U26, TL-U27, TL-U28
  (S4c); TL-U30 (S4b); lint TL-L5 (S4c). Names: section 3. Changed names: TL-UC16 (X4), TL-U19 (AM-31), lint TL-L2
  (X4), lint TL-L3 (AM-34).
- G-U1 `ctest --test-dir build -R '^TL-' --no-tests=error --output-on-failure` -> `100% tests passed, 0 tests failed
  out of N` with N >= 77 (78 if S2b is built), AND each of the 77 ids appears as Passed. RED arm: the per-stage RED
  log, by id (S4c's log also lists TL-U19 for its trim clause).
- G-U1b adds any new binary (the beat grid's) to the list run whole by path. GUARD, no RED arm.
- G-U2 as the base; "cases deleted" also counts the two cases of D12.
- G-U3 adds MU-30 .. MU-43 (section 3) and makes MU-H1 .. MU-H3 unconditional. Harmony re-runs five of her choice,
  always MU-4, MU-18, MU-21, MU-30, MU-39.
- G-U4 `.harmony/probe-tsan-unit.sh` -> exit 0 with `EXPECTED_TSAN_CASES` raised from 5 to 8 (TL-U8, TL-U11c,
  TL-U19b). By stage: 6 at S2's head, 7 at S3h's, 8 at S4a's. RED: as the base, and the hold record as a plain
  `uint64_t` makes TL-U11c report a race.
- G-U5, G-N1: as the base. (The X tooltip is a `setTooltip` text; it is not one of G-N1's five strings and it
  announces no event.)

### 4.4 Live rows: `.harmony/probe-transport.sh`, pin `EXPECTED_ROWS=23`
The base's 17 rows stand with these changes and additions. Launch rules and the flake rule as the base.
THE BEAT DRIVER (TR15 .. TR18): the probe posts `/api/inject_features` at >= 50 Hz with `bpm`, `trackerState` 2 and
`beatPhase` + `totalBeatCount` taken from its own monotonic clock (the driver of probe-boxes k5 and of TR6). Show
tempo 60 unless a row says otherwise (one post is then at most 0.02 beat old). The probe computes
e = wrap(frac(x) - driver phase at the reply), x from the reply's `playheadPosition`, `inPoint`, `outPoint`,
`clipBpm` and the fixture's length (on the frozen arm, which reports none of the last three: the fixture's own
numbers). The lane report states the noise of e on a locked clip over 5 runs; a bar inside 4 x that noise is
BLOCKED, as the base rules.
- TR7c gains: "strip down 0.05 (before the in point 0.2), up: no read is below 0.2". GUARD; teeth MU-10.
- TR7d marker_carries_the_playhead. In 0 / out 1, a Loop ramp. Wait for a playhead read in [0.3, 0.4]. Clip tab:
  press the in marker's tab, drag to the x of 0.6, up. Bar: `inPoint` in [0.59, 0.61] and a playhead read within
  0.2 s of the up is >= 0.59. GUARD: no RED arm (the route is new). Teeth: MU-42 -- the read is below 0.45.
- TR8 scrub_hold. As the base wrote it; no longer conditional.
- TR15 beat_lock_pulls_in. BPM-synced ramp, `clipBpm` 60 (10 beats), Quantize off; fire while the driver's phase is
  in [0.45, 0.55]. Bars: (i) a read within one beat of the fire has |e| >= 0.3 (the fire did not seek); (ii) no
  playhead read is below the one before it except across the loop's wrap, and the content rate over every 2 s
  window is in [0.88, 1.12]; (iii) from 12 beats after the fire, five reads 0.5 s apart each have |e| <= 0.08.
  RED arm (frozen app, TR9's old-key composition, rate 1.0): (iii) fails with |e| about 0.5 (INFERRED; if the RED
  arm passes, the row is reported as a guard and MU-30 is its teeth).
- TR16 beat_lock_tempo_and_realign. From TR15's locked state. (a) the driver steps to 90 with its phase continuous:
  every read of the next 8 beats has |e| <= 0.12, after them <= 0.08, and the rate over 2 s is in [1.44, 1.56].
  (b) the driver adds half a beat to its phase in one step: no read is below the one before it except across the
  wrap; the rate stays in [1.32, 1.68]; from 12 beats later |e| <= 0.08. RED arm: (a)'s rate stays about 1.0.
- TR17 beat_lock_after_a_drop. Locked clip at 60. `/api/debug/strip_scrub` down 0.35; four reads over 1 s within
  one frame of 0.35; up while the driver's phase is in [0.45, 0.55]. Bars: the read 0.5 s after the up is in
  [0.39, 0.41]; from 12 beats after the up |e| <= 0.08. GUARD: no RED arm (the route is new). Teeth: MU-36 -- the
  read after the up is 0.45 or more; MU-30 -- the last clause.
- TR18 lines_on_the_beat. Ramp with in 0.2, out 0.7, `clipBpm` 96 (8 beats, 7 lines), inspected in the Clip tab,
  locked at 60. Bars: on both dumps (`/api/debug/clip_transport_ui`, `/api/debug/strip_transport_ui?layer=0`) the
  line xs equal `beatLinesOf` of the model, +/- 1 px; at 8 instants where the driver's phase is in [0, 0.02] the
  playhead x on each surface is within max(2 px, a tenth of the line spacing) of a line x or of the in marker's x.
  GUARD: no RED arm (the routes are new). Teeth: MU-35 turns the first bar RED, MU-30 the second.
- TR9 runs with no beat driver. What the lock does then is S0's finding (4.2); the row's bars do not change.
- (INFO, outside EXPECTED_ROWS, only if S0 finds a REST route that runs a menu command) TR19
  move_layer_ends_history: drop a file on an idle cell; Move Layer Up; `POST /api/debug/undo`: the cell still holds
  the file and the layer order is the moved one.
- Paths with no live row, and what proves them: bypass, solo and fold push no step -- the class is deleted, lint
  TL-L2; a restored layer's state -- TL-UC18; the lock on a sequence, a reversed and a ping-pong clip -- TL-U26.
- Measurements, printed as `MEASURE <name> <value>`: M1 (FM-1), M2 (FM-3), M3 (FM-4), M4 (FM-5), M5 (the noise
  of e).

### 4.5 Re-runs, idle paint, the visual gate, the facts to measure
- 5.4 gains: `.harmony/probe-idle-paint.sh` with a BPM-synced clip playing in a strip -> its existing bars (INFO
  unless it regresses); TL-U30 is the gate for "the lines add no repaint".
- 5.5 gains states V12 the layer strip, BPM Sync, 8 beats (7 lines); V13 the same clip in Timeline mode, strip and
  Clip tab (no line); V14 64 beats (the strip draws no line when they would be under 3 px apart). Every dump of a
  BPM Sync state now carries the line xs of both surfaces.
  B3 gains: no caption, tooltip or painted string of the BPM row, the Beats row or either bar holds "bar" in any
  letter case; the four tooltips are AM-36's strings exactly. B10 in every BPM Sync state with a known length the
  line xs of both surfaces equal `beatLinesOf` of the model, +/- 1 px, and all lines of a surface have one colour
  and one width. B11 no line on either surface in V1, V6, V10, V13. Pass = B1-B11 and no seat raising a MUST.
  B10 and B11 are GUARDS (new routes); teeth on the lane build: MU-35 turns B10 RED in V8.
  Seats, added questions. visual-design: on both surfaces, at both window sizes, are the beat lines told apart
  from the in and out markers and from the playhead without colour alone? UX: can beats mode be told from speed
  mode from the bar alone? Does anything in the two rows or the two bars suggest bars? interaction-logic (given
  TR18's numbers): does the playhead meet a line when the beat falls; after x2 are there twice the lines?
- 5.6 changes. FM-4 becomes a bar: music file, a BPM-synced ramp fired on a downbeat with Quantize on; in the fifth
  minute 20 reads 3 s apart: |e| <= 0.1 beat in at least 18. Not met: STOP and report -- no change is pre-decided.
  New FM-5 the trim on real music (lane build): the same clip for 60 s while locked; the content rate over each 2 s
  window against show BPM / clip BPM. Bar: at least 90 % of the windows within 2 %. Not met: STOP and report (the
  three constants are named; which one moves is decided on the numbers).

## 5 WHAT ONLY BORIS CAN CHECK (changes to ruling-transport section 6; do -> expect -> wrong)

Checks 1-6, 8, 9, 11-19, 21-23 stand. Replaced:
7. Grab the playhead in the layer strip, and then in the Clip tab; drag; let go. -> The picture follows your hand
   and the clip plays on from where you let go. If you keep the mouse down and hold still, the picture waits on
   that frame until you let go. Wrong: it springs back, or it slides away under a resting hand.
10. On a clip with a trimmed start and end, drag the playhead past either marker, in the strip and in the Clip tab.
    -> It stops at the marker and goes no further. Let go at the end marker: the loop starts over. Wrong: the
    playhead goes into the dark part.
20. Bypass a layer, solo another, move its V fader, then press Cmd+Z a few times. -> None of the three changes
    back; Cmd+Z undoes your last edit outside the layer strip instead. Wrong: the bypass or the solo flips back.
Added:
24. Drag the in marker to the right, past the playhead. -> The playhead rides along on the marker.
25. Set a video to BPM Sync. -> Lines appear in its playhead bar, in the Clip tab and in the layer strip, one per
    beat. Set it back to speed. -> The lines are gone.
26. With music playing and the beat steady, watch a BPM-synced clip. -> The playhead crosses a line on every beat,
    for as long as you leave it. Wrong: it crosses between the beats and stays that way.
27. Drag that clip's playhead somewhere and let go between two beats. -> It plays on from where you let go, with no
    jump, and within a few beats it is crossing the lines on the beat again.
28. With Quantize off, fire a BPM-synced clip between two beats. -> It starts at once from its beginning and eases
    onto the beat within a few beats. Wrong: it starts part-way in, or it never lines up.
29. Press Resync, or tap a new tempo, while a BPM-synced clip plays. -> The clip does not jump; it eases onto the
    new beat.
30. Type 14.6 into Beats. -> Each time the loop comes round the clip runs a little slow for a few beats, then sits
    on the beat (question D1). Type 15 and it runs evenly.
31. Remove a layer that had bypass, solo and a lowered fader; press Cmd+Z. -> It is back with its clips, its bypass
    and its fader as they were, playing nothing, and its solo is off.
32. Move a layer up, then press Cmd+Z. -> Nothing happens; the Undo item in the menu is grey (question D3).
33. Add an effect to a layer, press Cmd+Z. -> The effect goes (question D4).

## 6 QUESTIONS FOR BORIS (each has a default; nothing waits)

Closed by his message: Q2 ("2 b"), Q3, Q9 ("9 b"), Q10 (his own rules), Q4 (he ruled the unit; Harmony's reading: A
stands), Q1, Q5, Q6, Q7, Q8 ("all defaults"). None of the base's ten is left.
- D1 A clip whose Beats number is not whole (the app's own number usually is not: a 7.3-second clip shows 14.6).
  The app still pulls it onto the beat, so each time the loop comes round it runs a little slow or fast for a few
  beats (default). Or should the app round its own number to a whole beat when you switch a clip to beats -- the
  BPM box then shows a number near 120 instead of 120?
- D2 You add a layer and press Cmd+Z: the empty layer goes away again (default). Or should Cmd+Z never take a layer
  row away?
- D3 After you move a layer up or down, Cmd+Z can no longer step back to edits you made before the move (default).
  Or should those older steps stay reachable (more work; a later stage)?
- D4 An effect you put on a layer: Cmd+Z takes it off again, also while the layer plays (default). Or should Cmd+Z
  leave a layer's effects alone too?

## 7 RISKS (the strongest counterargument first; the cheapest refuting test)

- KD-1 THE STRONGEST. The lock changes a clip's speed by itself, on numbers chosen by reading. The base rejected
  exactly this family (F9: jitter from a clock that moves in hops; "where it should be playing"). On real music the
  tracker realigns its own phase ("confident detection", D14): every realign is a slide, and a tracker that
  realigns often would make a BPM-synced clip breathe by up to 10 % -- visible on footage of steady motion. Why the
  delta still builds it: "9 b" and "the play head moves past them on time" cannot be met by a rate alone (render
  clock against audio clock, FM-4), a seek is the jump he ruled out, and the deadband sits above the hop noise.
  Cheapest refutation: FM-5 (one minute of real music, before S4b starts). Unit-side: TL-U24 run again with a
  recorded phase trace instead of the ideal clock, if FM-5 fails.
- KD-2 Most clips will not have a whole Beats number at the default 120 (F22). For them the lock re-pulls on every
  loop, or is off for short loops with a half-beat remainder. Boris may read either as "not synced". D1, check 30.
  The other way (round the automatic number) is one line in the widget's first-switch path and contradicts "default
  all bpms to 120" -- only he can trade those.
- KD-3 The lock is modulo one beat (F20). After a drop, a Resync or an unquantized fire the loop's first beat can
  sit on beat 2, 3 or 4 of the bar, and stays there. His words ask for lines on beats, not for the loop on the bar.
  Refutation: checks 26-28. A bar lock would need the stored offset F20 rejects.
- KD-4 A layer move ends the Undo history (F26), silently. "It changes anything else" is narrowed for every edit
  before a move. Why it holds for now: the precedent is in the tree for the same reason (D11); the runner-up
  re-numbers about ten commands inside the lane's most intricate stage (base K1). Refutation: D3, check 32; S1m is
  named.
- KD-5 X4 reads "anything in the layer strip" as a list of fields (D1). Two edges are readings he has not seen: a
  layer's effects stay undoable (D4), and a whole layer may still go with Cmd+Z (D2). If either is wrong the change
  is small and local (EffectStackCmd's Layer scope pushes no step; AddLayerCmd pushes no step and InsertDeckCmd's
  Undo always keeps its layers).
- KD-6 Solo off on a restored layer (F27) is a setting Cmd+Z does not bring back. It is the smaller wrong: the
  other way blacks out the playing layers. Check 31 shows it to him.
- KD-7 A LOCKED tracker with a phase that stands still (test mode after `set_bpm`; perhaps manual tempo) would make
  the lock chase a fixed phase and swing the speed by 10 % each way. Not read (BPMTracker). S0 settles it before
  S4c; the off-condition is pre-specified (4.2). TR9 would show it: its rate bar is 3 %.
- KD-8 The lock follows the snapshot, which is up to a hop old, and the picture reaches the screen a frame or more
  later: the clip sits a constant few milliseconds behind the sound. That offset is the sync-dial lane's subject;
  this lane only promises the same phase the quantized release uses (S0).
- KD-9 The strip's beat lines put a length lookup on the strip (AM-32). Asked per paint or per tick it would take
  the player map's lock 30 times a second per layer against the render thread's lookup. The rule (in `refresh()`,
  and once per tick only while unknown) is pinned by TL-U30's provider call count.
- KD-10 The hold freezes the output for as long as he holds (his choice, "2 b"). Base RE-2 and RE-3 live here;
  AM-12's cases are the guard. A speed that changes every frame under reverse play is new load on the GOP cache
  (Pitfall 62): probe-video is re-run whole at S4c.
- KD-11 The beat driver posts at 50 Hz; at 120 BPM that staircase alone is 0.04 beat, over the deadband. The rows
  run at 60 for that reason. If the route cannot take 50 Hz, the rows are BLOCKED, not loosened; TL-U24 .. TL-U28
  remain the proof.
- KD-12 Line numbers are those of 34179a2; the sync-dial lane moves them. S0 exists for that.

NOT IN THIS LANE: a Resync that restarts clips; a lock to the bar; rounding the automatic Beats number; the
re-numbering of history after a layer move (S1m); reverse play leaving a trimmed range (base SF-5); rescaling the bar
to in..out; wiring the strip's drag reorder (unassigned at the pin, D5); greying a refused Redo (Edit-menu lane); any
bars-lane label; "12 b" and every failure text (notices lane); the Snap and Quantize menus.

FORKS (id | choice | runner-up)
F20 | beat-phase lock modulo one beat, stateless | anchored position lock with a stored offset
F21 | bounded speed trim (10 %, deadband 0.03 beat, 2-beat time constant) | seek to the aligned position
F22 | non-whole Beats: lock where the wrap step is recoverable, else rate only | round the automatic Beats to a whole beat
F23 | beat lines in the Clip tab AND the layer strip | Clip tab only
F24 | bar keeps the whole file, playhead clamped to in..out | bar rescaled so in..out fills it
F25 | a point dragged past the playhead carries it | playhead left until it wraps
F26 | a layer move is no step and ends the Undo history | keep the history, re-number every step (S1m)
F27 | a restored layer returns with its settings, empty tuple, solo off | solo restored as it was

STATUS: DONE


## HARMONY ADOPTION (s-rta-1003b, 2026-10-04 01:06:07) — overrides the delta ruling, which overrides ruling-transport, which overrides the plan
1. ADOPTED: .harmony/.reports/s-rta-1003b/ruling-transport-delta1.md IN FULL (STATUS DONE: 22 attacks ruled, 10 delta
   amendments DA-1..DA-10). A builder builds from plan-transport.md's body + ruling-transport.md + ruling-transport-delta1.md
   (the delta overriding). Stages, in order: S0, S1, S2, (S2b only if FM-1 is over its bar), S3, S3h (BUILT: Boris "2 b"),
   S4a, S4c (the beat lock), S4b (widgets + beat lines; VISUAL WORK GATE), S5; S1m only if Boris answers question 22 "B".
   Gate strings: the delta's section 5 where it replaces a row, ruling-transport's section 5 otherwise.
2. HARMONY'S DECISIONS: H-D1 default A (a loop that is not a whole number of beats is not locked) -- HARMONY'S default,
   not Boris's word; the ruling's own strongest counterargument is that his first clip at 120 will not be locked until he
   types a whole number of beats; it is question 20 on his page and S4c's builder reads his answer at stage start.
   H-D2 default (beat lines thin to every 2nd / 4th / 8th beat when closer than 3 px). H-D3 done: questions 20-25 are on
   his page with his own sentences beside them. H-D4 default (FM-5 right after S4c, read before S4b; over its bar: STOP).
   H-1..H-5 of the first adoption stand (S1 does not start before the sync-dial lane has merged).
3. Boris's answers that this delta builds (verbatim in binding-decisions.md, the s-rta-1003b section): "2 b", his rule for
   10, his rule for 4, "9 b", his rule for 3, and the beat-lines sentence. Questions 1, 5, 6, 7, 8, 11: defaults.
4. Harmony constraint, unchanged: no on-screen text that announces an event or a failure; own-pid quits, no Output window,
   no synthetic input; S4b is a visual deliverable with five critic seats. MERGE by Harmony.
