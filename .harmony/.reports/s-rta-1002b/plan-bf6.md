# PLAN bf6 -- Timeline (and Clip Position) follow the clip's playhead; a layer knob follows its playing clip

Architect (Fable), s-rta-1002b, 2026-10-02. Code read at main 5e47d17. Builds AFTER bf9b (pre-registered order
bf9 -> bf6; bf9 was re-planned as bf9b "decks are boxes of clips"). Input: .harmony/.reports/s-rta-1002b/diag-bf6.md
(builder diag6). Its root cause was re-checked here by reading the code.

## 1. GOAL

A knob set to Timeline (or Clip Position) moves with the play position of its own clip (clip knobs) or of the clip
playing in its layer (layer knobs). Composition controls do not offer either source.

Boris, verbatim: "Timeline control is not working for parameter control" (backlog BF6) and "when I set any clip
parameter to timeline it should be locked to the clips playhead, same with layer, not necessary for composition
controls" (binding-decisions.md:591-593).

There are two faults, both VERIFIED in section 2:
- (a) The picker turns "Timeline" into a 4-beat ramp on the global beat clock.
- (b) The connection engine never gives any connection a clip clock. It passes nullptr at every site it walks, so
  nothing that depends on a playhead can move.

The fix is the s166 "Lane 5" wiring that never landed (s166 spec :640-644). It is not a Timeline special case.

## 2. ESTABLISHED

VERIFIED = read at 5e47d17 (file:line). INFERRED = derived from the cited lines and not run.

- E1 VERIFIED -- The picker:
  - `sourceFromPicker(Timeline)` builds `Envelope{clock = Beats, cycleBeats = 4, points (0,0)-(1,1) Linear}`
    (src/connect/ConnPicker.cpp:90-100).
  - This is the ONLY place in src that creates `Kind::Envelope` (grep: ConnPicker.cpp:93 only).
  - `describeSource` names every Envelope "Timeline" (:122-126).
  - tests/test_conn_picker.cpp:62-72 pins the Beats clock.
- E2 VERIFIED -- The engine, `ConnectionEngine::tick` (src/connect/ConnectionEngine.cpp):
  - It passes nullptr as the clock for macros (:327), composition scalars (:335-336), global effects (:337), layer
    scalars (:343-344), layer effects (:345), clip scalars (:357-358), clip effects (:359) and source params (:376).
  - `evaluate` reads `clock ? clock->position() : 0` for Envelope(ClipPosition) (:139-143) and for Kind::ClipPosition
    (:147-151).
  - The Beats branch reads beatPhase / beatInBar / barsSinceResync (:101-102, :133-138).
- E3 VERIFIED -- The seam exists:
  - `struct ClipClock { virtual float position() const = 0; }` (src/connect/ConnectionEngine.h:21-25).
  - Its comment says "Lane 5 wires the real thing" (:12-20).
  - The engine runs on the message thread only (jassert, ConnectionEngine.cpp:294).
  - Its one call site is `MainComponent::tickFeaturePipeline` at 120 Hz, after the recorder and the routines
    (MainComponent.cpp:4076-4108).
- E4 VERIFIED -- The value wraps at the end:
  - `playbackXform` Forward = frac(x) (ConnectionShaper.cpp:10-13, :29-30), so exactly 1.0 becomes 0.
  - Backward = 1 - frac(x) (:31-32).
  - PingPong over one [0,1) pass equals Forward (:33-40).
- E5 VERIFIED -- The playhead:
  - `Clip::playheadPosition` is a RelaxedDouble in [0,1] of the WHOLE media file (Clip.h:238).
  - The GL thread writes it from the player on every synced frame (`ClipTransportSync::writeBack`,
    ClipTransportSync.h:47-48). That is called from `Renderer::syncMedia` for video (Renderer.cpp:1836) and for
    image sequences (:1903).
  - Player position = currentTime / duration (VideoPlayer.cpp:510-512).
  - A OneShot clip parks at exactly 1.0 (:470-473).
- E6 VERIFIED -- In and out points:
  - `inPoint` / `outPoint` are in [0,1] of the media (Clip.h:127-128).
  - writeBack enforces the forward out point: OneShot stops; Loop and PingPong seek to the in point
    (ClipTransportSync.h:55-72).
  - INFERRED from the same lines and VideoPlayer.cpp:476-493: reverse play is NOT stopped at the in point. It runs to
    the start of the file and wraps.
- E7 VERIFIED -- The dead "Clip Position" registry signal:
  - The in/out formula already exists in `ClipPositionSignal::updateFromClip` ((ph - in) / (out - in), clamped;
    ClipPositionSignal.h:33-41), but nothing calls it (grep).
  - So the registry signal "Clip Position" (registered hidden as Type::Audio, SignalRegistry.cpp:74-80) always reads 0.
  - It still shows in the picker's Audio submenu, because that loop filters only on Type::Audio
    (UniversalParamControl.cpp:421-440).
  - `getClipPositionSignal()` (SignalRegistry.cpp:191-199) has no caller. The header is listed in CMakeLists.txt:217.
- E8 VERIFIED -- Only video clips and image sequences have a playhead:
  - `Clip::isPlayable()` = Video or ImageSequence (Clip.h:248).
  - Images, sources (MilkDrop included), cameras and effects-only clips have no player, so nothing advances their
    playheadPosition.
- E9 VERIFIED -- Layers:
  - The trigger tuple holds activeClipColumn (incoming), previousClipColumn (outgoing) and crossfadeProgress
    (Layer.h:44-51).
  - It is read with ONE acquire load, `runtime()` (Layer.h:300; Pitfall 63 rule 3).
  - During a crossfade both clips are synced every frame: `applyTransition` renders the outgoing clip through
    getClipTexture -> syncMedia (CompositorEngine.cpp:1655-1697). So both playheads keep moving.
- E10 VERIFIED -- Starting a clip:
  - The activation tail writes `playheadPosition = inPoint` (Layer.h:535-544).
  - The retrigger path also seeks the player (MainComponent.cpp:4699-4723).
  - A NEW activation of a clip whose player already exists does not seek (:4737-4748). The next GL write-back then
    restores the position the player resumes from.
  - A beat-snapped trigger seeks to beatPhase of the whole file (:4677-4697).
  - The Clip inspector's timeline scrub seeks the player (ClipInspector.cpp:1243-1251, :1270-1274 -> onCuepointJump,
    MainComponent.cpp:1515-1528).
- E11 VERIFIED -- The picker menu and where controls are bound:
  - "Clip Position" (id 2) and "Timeline" (id 3) are offered the same way at every level
    (UniversalParamControl.cpp:510-514).
  - The widget does not know which level its owner is (UniversalParamControl.h:81-206).
  - Bind sites:
    - ClipInspector::bindScalarControls (ClipInspector.cpp:811-824) and buildSourceParamControls (:826-862).
    - LayerInspector::bindScalarControls (LayerInspector.cpp:753-767).
    - CompositionInspector::bindScalarControls (CompositionInspector.cpp:399-413).
    - EffectStackView rows (EffectStackView.cpp:369, :422-426). These know their chain's level through EffectScope
      Global / Layer / Clip (core/EffectScope.h:16-31; EffectStackView.h:41-42, :146).
- E12 VERIFIED -- Save and load:
  - An Envelope saves `"clock":"beats"|"clipPosition"` plus its points (ConnSerialization.cpp:120-137) and loads both
    (:204-238).
  - A Kind::Signal saves its name (:105-108).
  - A clip-clock envelope already round-trips (tests/test_connection.cpp:655-673).
  - Clip transport settings are saved: loopMode, speed, reverse, in/out, transportMode, beatSnapMode (Clip.cpp:40-48).
    Probe fixtures can therefore set them.
- E13 VERIFIED (read-only scan run during planning, 2026-10-02) -- No saved file on this machine holds an envelope,
  clipPosition or "Clip Position" connection. Folders scanned:
  - ~/Library/AudioDNA: compositions (1), Decks (4), Presets (12).
  - ~/Library/Audio-DNA (1).
  - ~/Documents to depth 4 (1,790 .json).
  - ~/Desktop (0).
  - The app's composition and deck folders are ~/Library/AudioDNA/{compositions,decks} (CompDecksBrowser.cpp:322-332).
- E14 VERIFIED -- The connection's own playback and loop settings (`ConnShape::playback`, `loop`) are not reachable
  from any UI (grep outside src/connect: none).
- E15 VERIFIED -- Undo:
  - Changing a knob's source in the picker is not an undo step (grep: no listener of `onSourceChanged`).
  - Undoing an effect-stack edit copies the connections, which clears grip and state (ParamConnection.h:156-166).
- E16 VERIFIED -- Routines and takes:
  - A recorded lane is a WRITER through the grip path, not a source (ParamConnection.h:20-33).
  - PerfState captures manual values and the playhead. It restores neither the playhead nor any connection source
    (PerfState.h:14-50; the playhead is only parsed, at PerfState.cpp:72).
- E17 VERIFIED -- REST:
  - /api/composition returns each clip's playheadPosition, the clip / layer / composition `live` scalars and each clip
    effect param's effective value (ApiServer.cpp:398, :430, :444, :454, :459-478).
  - It does not return live values for layer effects or source params.
  - Test mode starts no AnalysisThread (MainComponent.cpp:1859-1863), so the beat clock is frozen unless POST
    /api/inject_features drives it (ApiServer.cpp:237).
- E18 VERIFIED -- Design history:
  - s166 L5 already specified: "ClipPosition per scope: clip = own playheadPosition; layer = its active clip;
    composition = disabled until a master layer exists (D5) ... Retire ClipPositionSignal"
    (.harmony/specs/s166-universal-connection-architecture.md:640-644; D5 at :699-700).
  - The lane-3 plan made Timeline a 4-beat ramp as a stopgap (s-rta-0923-lane3-plan.md:403-406).
- E19 VERIFIED -- Rulings:
  - Ruling 8 "BUILD THE DRAWABLE TIMELINE CURVE" (binding-decisions.md:258).
  - Ruling 13(a) "A TIMELINE IS A CONNECTION SOURCE" (:304-308).
  - Ruling 22: a routine lane can be printed onto a knob as a curve source (:386-392).
  - Rulings 9/14: "Do not migrate anything until answered", about the v1 preset conversion (:260-261, :312-316).
  - Decks are boxes of clips and the layers are one shared playing stack (BORIS_DECISIONS.md:350-356;
    binding-decisions.md:598-606). So bf9b reshapes layers BEFORE bf6 builds.
  - BORIS_DECISIONS.md:393 rejects a "Timeline view". That means an arrangement view, not this source.
- E20 INFERRED -- Lag: the engine reads the playhead the GL thread wrote on the previous frame, and the GL thread
  renders with the value from the previous tick. So a Timeline value trails the picture by about one frame (8-17 ms).
  Every connection already has this lag.

How the fix answers each case the task listed:
- **Timeline vs Clip Position after the fix:**
  - Both read the same clip clock.
  - Clip Position = the straight position itself.
  - Timeline = the curve `ConnSource::env.curve` laid over the clip (x = the clip from its in point to its out point).
  - The picker's Timeline curve is a straight ramp, so until the curve can be drawn the two behave the same (Q2
    decides whether both stay in the menu).
  - They do NOT merge in the model. Kind::ClipPosition stays (old files, the plain source). Kind::Envelope with the
    ClipPosition clock is the drawable one.
- **Ruling 8, the drawable curve:** NOT built here. This is the contract for whoever builds the editor. bf45's editor
  component is the natural host: the s167-l2 ruling is one struct / one evaluator / one editor for AutomationCurve.
  - It edits `ConnSource::env.curve` of the knob's own connection.
  - x axis = the clip from its in point (left) to its out point (right), labelled in the clip's time (seconds, or
    bars and beats for a BPM-synced clip).
  - A vertical line at `clipTimelinePosition()` (this lane's one formula).
  - y = the knob's range after RANGE / INVERT.
  - For a layer knob the axis is "the playing clip" and the line is that clip's.
  - It is not offered where the picker greys out or leaves out Timeline.
  - It is a small per-knob curve, not the rejected "Timeline view" (BORIS_DECISIONS.md:393).
  - An edit must be an undo step. Picker changes are not undoable today (O3).
- **Rulings 13(a) and 22, recorded lanes:** unchanged.
  - A take or routine lane stays a writer (grip) over whatever source the knob has.
  - Printing a lane onto a clip knob's Timeline later means re-timing the take onto one pass of the clip.
  - The Beats clock stays in the model for a beat-timed print. Nothing here blocks either.
- **Loop modes:**
  - Loop wraps the playhead, so the value wraps with it.
  - PingPong turns the playhead, so the curve is read backwards.
  - OneShot parks at the end, so the value holds the curve's end. This needs the no-wrap fix (F3).
- **Reverse / speed:**
  - Reverse runs the playhead down, so the curve is read backwards.
  - Speed, BPM-synced transport and master speed only change how fast the playhead moves. Nothing to do here.
- **Retrigger:** the playhead goes back to the in point, so the value goes to the curve start.
- **Scrub / cue jump:** the value follows, also while the clip is paused.
- **In / out points:** x = (playhead - in) / (out - in), clamped. Moving the in or out point rescales where the curve
  sits.
- **Crossfade (Pitfall 35):**
  - Each clip's own knobs follow that clip's own playhead (both playheads move during the fade).
  - A layer knob follows the INCOMING clip from the moment it is triggered.
- **No playhead** (still image, source, camera, effects-only clip, empty layer):
  - The connection publishes nothing, so the knob shows its hand value.
  - The clip-level picker greys out both items. Q1 offers the alternative.
- **Save / load:**
  - New picks save `"clock":"clipPosition"` plus a format marker.
  - An old Timeline (the exact pre-bf6 picker output, with no marker) loads clip-clocked.
  - An old "Clip Position" registry-signal connection loads as Kind::ClipPosition.
  - No existing file is affected (E13).
- **Routine and take replay:**
  - They fire clip triggers and move knobs as writers.
  - A Timeline knob follows whatever the clip's playhead does during the replay.
  - A routine gesture on a Timeline knob grips it and hands it back as for any source.
  - No routine data changes.
- **Undo:** Timeline is a pure function of the playhead and keeps no phase state. Any undo that restores a connection
  or a layer's active clip is followed at once.

## 3. DESIGN FORKS

- **F1 -- How the playhead reaches the engine (the root cause).**
  - A (CHOSEN): wire the existing ClipClock seam in ConnectionEngine::tick, with one clock object per clip and one per
    layer.
  - B: revive the registry signal ClipPositionSignal, written by the GL thread. Loses: a registry signal holds ONE
    value for the whole app, but every clip has its own playhead (N clips, N positions). It would also add a
    GL-to-registry write.
  - C: evaluate clip-clocked sources on the GL thread at draw time (no lag). Loses: it puts a second evaluator next to
    "the one evaluator" (ConnectionEngine.h:27-30), while grips, glide and smoothing state live on the message thread.
    That is two evaluators to keep identical, to save about one frame of lag that every connection already has (E20).
- **F2 -- Which clip a LAYER knob follows during a crossfade.**
  - A (CHOSEN): the incoming (active) clip, from the moment it is triggered. That is "the clip the layer plays now",
    and the rule is the same for a cut and a fade.
  - B: blend the outgoing and incoming values by fade progress. Loses: it produces a value no playhead produced, and a
    cut and a fade would behave differently.
  - C: the outgoing clip until the fade ends. Loses: it follows a clip that is leaving.
- **F3 -- The end of the clip.**
  - A (CHOSEN): clamp to [0,1] and never wrap. A OneShot clip parked at its end holds the curve's end.
  - B: keep frac(). Loses: exactly 1.0 snaps the value back to the start (diag [E]).
- **F4 -- Composition controls.**
  - A (CHOSEN): not offered. Boris: "not necessary"; also s166 D5's recommendation.
  - B: keep the beat clock there. Loses: "Timeline" would mean two different things.
  - C: follow the master or top layer's clip. Loses: an invention he did not ask for.
- **F5 -- Clips with no playhead, and empty layers.**
  - A (CHOSEN default; Q1): publish nothing, so the knob stays at its hand value. The clip-level menu greys out the
    items with "video clips only".
  - B (Q1's alternative): a virtual playhead (bars since the clip started). Needs a length setting.
  - C: hold the curve start (what a null clock gives today). Loses: a ramp on opacity hides the clip, so a layer whose
    opacity is on Timeline would vanish whenever a still plays.
- **F6 -- Timeline and Clip Position.**
  - A (CHOSEN default; Q2): keep both. They are identical until the curve can be drawn; then Timeline follows the
    drawn shape.
  - B: remove Clip Position now. Rejected: a visible removal he did not ask for.
  - C: make them differ now with an invented default shape. Loses.
- **F7 -- The drawable curve (ruling 8).**
  - A (CHOSEN): not in bf6. The editor is one component on AutomationCurve (bf45's); the contract is in section 2.
  - B: build a Timeline-only editor now. Loses: a second editor for one struct, replaced when bf45 lands.
  - C: hold bf6 until the editor exists. Loses: the bug stays through five lanes.
- **F8 -- Old saved Timeline knobs.**
  - A (CHOSEN): upgrade on load inside ConnSerialization::fromVar.
    - It matches ONLY the exact pre-bf6 picker output saved without a format marker.
    - bf6 writes the marker on every envelope it saves, so a future beat-clocked envelope (bf45) is never touched.
    - No existing file is affected (E13). G0 re-scans just before merge.
  - B: a post-pass in Clip::fromVar / Layer::fromVar / Composition::fromVar that knows the level and keeps
    composition-level ones on beats. Loses: three edits in bf9b's busiest files, for a case with zero instances.
  - C: no upgrade. Loses: a Timeline knob saved before the merge stays a beat ramp labelled "Timeline", so the bug
    stays in his file.
  - CONFLICT FLAGGED: rulings 9/14 say "do not migrate anything until answered". They are about the v1 preset
    conversion, and I read them as not applying here. Because their words are absolute, G0 re-scans his folders just
    before merge. Any hit sends Q4 to Boris before the merge (default: upgrade).
- **F9 -- The dead "Clip Position" registry signal.**
  - A (CHOSEN): retire it (s166 L5 named this), and load an old Signal("Clip Position") connection as
    Kind::ClipPosition, which now works.
  - B: wire it. Impossible per clip (see F1-B).
  - C: only hide it from the Audio submenu. Loses: a dead object stays.
- **F10 -- How the picker learns what to offer.**
  - A (CHOSEN): a provider function on the widget, called when the menu opens, set by each owner. It stays right if
    the clip's media is replaced without a rebind.
  - B: a flag set at bind time. Loses: stale after a media replace on the same clip.
  - C: the widget looks up its owner in the model. Loses: ties the UI to the model.

**Strongest counterargument to the whole plan:** "Timeline should BE the drawable curve. Shipping a straight clip ramp
makes Timeline a copy of Clip Position and may teach Boris that Timeline means a straight line."

It loses for three reasons:
- His complaint is about the LOCK: the knob followed nothing he could see.
- The shape belongs to the editor he already approved (ruling 8), and that editor uses exactly the axis this lane
  fixes.
- Waiting for the editor keeps a broken control in front of him for five lanes.

The duplicate menu entry is a tidiness question (Q2), not a correctness problem.

## 4. ITEMS + BUILD STAGES

Load-bearing piece: Item 1's walk (TC1, TC5). Verify it first.

**P0 -- Precondition (the Stage 1 builder's first action).**
- bf9b must be merged.
- Re-read ConnectionEngine::tick's walk, Layer.h and Composition.h on the merged tree.
- Find how bf9b names "the clip playing in this layer" (the incoming one during a fade).
  - If bf9b kept Layer::runtime().activeClipColumn and getClipAt: the layer clock is
    `layer.getClipAt(layer.runtime().activeClipColumn)`, with one runtime() load.
  - If bf9b moved the playing clip elsewhere (for example, a shared layer pointing into a deck box): use bf9b's
    accessor. The rule does not change.
- Write TC5 and TC6 against the merged model.
- Report which accessor was used.

### ITEM 1 -- The clip clock (engine). Stage 1.

**Files:**
- src/connect/ConnectionEngine.h, ConnectionEngine.cpp, ConnectionShaper.h, ConnectionShaper.cpp.
- tests/test_connection.cpp (one section).
- New tests/test_timeline_clock.cpp, with a target in tests/CMakeLists.txt that uses the same sources as
  test_connection (:602-618).

**Changes:**
- 1a ConnectionEngine.h:
  - Rewrite the ClipClock comment (:12-20): position() returns [0,1], or NaN meaning "no playhead".
  - Declare `float clipTimelinePosition(const Clip& clip);`:
    - NaN if !clip.isPlayable().
    - Otherwise (playheadPosition.load() - inPoint) / (outPoint - inPoint), clamped to [0,1].
    - 0 if the in/out span is <= 1e-6.
  - Declare `struct ModelClipClock final : ClipClock { const Clip* clip = nullptr; float position() const override; };`
    position() returns NaN when clip is nullptr.
  - Update the evaluate() doc (:59-73): it also returns NaN for a clip-clocked source with no playhead.
- 1b ConnectionShaper: add `float clipXform(float pos01, ConnShape::Playback pb)`.
  - Clamp to [0,1].
  - Backward -> 1 - p.
  - Forward and PingPong -> p, because the clip's own transport already turns the playhead.
  - playbackXform is unchanged (beats sources still use it).
- 1c ConnectionEngine::evaluate (:139-151): for Envelope with Clock::ClipPosition and for Kind::ClipPosition:
  - `float p = clock ? clock->position() : kNan; if (std::isnan(p)) return kNan;`
  - then clipXform instead of playbackXform.
  - Grip handling stays before this, unchanged.
- 1d ConnectionEngine::tick (:339-384):
  - Per layer: ONE `layer.runtime()` load -> `ModelClipClock layerClock{ <the layer's playing clip> }`, passed to the
    layer scalars and layer effects.
  - Per clip: `ModelClipClock clipClock{ &clip }`, passed to the clip scalars, clip effects and source params.
  - Macros, composition scalars and global effects keep nullptr.
  - Replace the "Lane 5" comment (:353-356).

**Behaviour:**
- Clip knobs on Timeline or Clip Position follow their clip's position between its in and out points, and never wrap
  at the end.
- Layer knobs follow the clip the layer plays (the incoming one during a fade).
- No playhead: the knob shows its hand value.
- Composition knobs are unchanged, except that a clip-clocked source there now shows the hand value instead of the
  bottom of the range.

**RED-first tests** (tests/test_timeline_clock.cpp, tag [timeline]):
- Each drives the real `ConnectionEngine::tick` or `evaluate`. Here RED means "fails on main 5e47d17 / the bf9b merge
  base", with the reason given.
- A test `FakeClock : ClipClock` compiles on main. Hand-built clip-clock connections are used so every RED test
  compiles on main.

- TC1 -- Clip sweep.
  - Setup: deck 0 / layer 0 / column 0 = a Video clip (media type only; no file needed). Clip opacity, one clip effect
    param and that effect's dry/wet get an Envelope with the ClipPosition clock and the (0,0)-(1,1) ramp. Beat clock
    frozen.
  - Check: at playhead 0, .1, ... .9 each value == playhead +-0.001.
  - RED: main publishes 0 (diag [A]/[D]).
- TC2 -- Beats ignored.
  - Setup: playhead held at 0.25; the beat clock runs from 0 to 4.5.
  - Check: the value stays 0.25.
  - RED: main reads 0.
- TC3 -- In / out points.
  - Setup: in 0.2, out 0.6.
  - Check: playhead 0.4 -> 0.5; 0.1 -> 0; 0.7 -> 1.
  - RED: 0.
- TC4 -- The end holds.
  - Setup: evaluate() with FakeClock{1.0}, for an Envelope(ClipPosition) ramp and for Kind::ClipPosition.
  - Check: Forward -> 1.0; Backward -> 0.0.
  - RED: main gives 0 and 1, because frac(1.0) == 0.
- TC5 -- Layer clock.
  - Setup: layer opacity on Timeline; Video clips at columns 0 and 1 with playheads 0.8 and 0.3.
  - Check:
    - active = 1 -> 0.3.
    - Crossfade {active 1, previous 0, progress 0.5}: layer 0.3; clip 0's own knob 0.8; clip 1's own knob 0.3.
    - Cleared layer (active -1): twin NaN, so layer.eff() == the manual value.
  - RED: main reads 0.
- TC6 -- A layer effect param on Timeline follows the active clip. RED: 0.
- TC7 -- No playhead.
  - Setup: an Image clip with a stale playheadPosition of 0.7 and opacity on Timeline; and a Source clip with a source
    param on Timeline.
  - Check: both twins are NaN, so eff() == manual.
  - RED: main publishes 0, which is not the manual value. This test also guards against a fix that skips
    isPlayable().
- TC8 -- Composition.
  - [pin] A Beats-clock Envelope on a composition scalar still follows beat/4.
  - A clip-clocked connection at composition level -> NaN (hand value). RED: main gives 0.
- TC9 -- Kind::ClipPosition on a clip scalar and on a layer scalar both follow the playhead. RED: 0.
- TC10 -- [pin] Grip and hand-back on a Timeline knob.
  - Gripped -> NaN.
  - Released -> glides from the hand value to curve(playhead) over handBackGlideMs.
  - Master Signal depth 0.5 halves the reach (unchanged rule).
- TE1 (green only after Item 2) -- `sourceFromPicker(Timeline)` -> tick -> the value follows the playhead. RED on main
  (Beats clock and null clock).
- test_connection.cpp:1014-1018 (the depth section that uses ClipPosition with nullptr):
  - Change it to a FakeClock at 0.0; the section's subject is the depth math.
  - Add a pin: evaluate(Kind::ClipPosition, nullptr) == NaN.
  - This is an intended change in what a null clock means ("position 0" becomes "no playhead"), not a weaker test.

**GREEN bar:** all [timeline] tests pass; test_connection and test_conn_picker pass; full ctest shows no failure that
the merge base does not also have.

**Risk:** the layer accessor under bf9b (P0); a returning clip (R2).

### ITEM 2 -- Timeline uses the clip clock. Stage 1.

**Files:** src/connect/ConnPicker.cpp (`case PickerChoice::Kind::Timeline` :90-100; describeSource's Envelope comment
:122-126); tests/test_conn_picker.cpp (:62-72).

**Change:** `sourceFromPicker(Timeline)` -> `Envelope{ clock = Envelope::Clock::ClipPosition, points (0,0)-(1,1)
Linear }`. cycleBeats keeps its default and is unused. describeSource is unchanged ("Timeline").

**Tests:**
- TP1 (rewrite of :62-72): clock == ClipPosition, 2 points, eval(0.5) == 0.5. RED: Beats.
- TP2: picker output -> ConnSerialization::toVar -> fromVar -> clock ClipPosition. RED: Beats.
- Plus TE1 (Item 1).

### ITEM 3 -- Old files load with the new meaning. Stage 1.

**Files:** src/connect/ConnSerialization.cpp (toVar envelope :120-137; fromVar signal :180-184 and envelope
:204-238); tests in test_timeline_clock.cpp.

**Changes:**
- 3a toVar: every envelope source also writes `"envFormat": 2` (bf6's marker).
- 3b fromVar: an envelope that has NO "envFormat", clock "beats", cycleBeats 4 (+-0.001) and exactly two points,
  (0,0,linear) and (1,1,linear) (+-1e-6), is the pre-bf6 Timeline pick. Set its clock to ClipPosition. Anything else
  loads as written.
- 3c fromVar: kind "signal" named "Clip Position" (the retired registry signal) becomes Kind::ClipPosition, with
  signalName cleared.

**Tests:**
- TU1 -- A legacy Timeline JSON loads clip-clocked. RED: Beats.
- TU2 -- [pin] Near misses stay Beats:
  - cycleBeats 8;
  - three points;
  - points (0,1)-(1,0);
  - the same ramp WITH envFormat 2 (guards a future beat envelope from bf45).
- TU3 -- Signal "Clip Position" loads as Kind::ClipPosition. RED: it stays Signal.
- TU4 -- End to end: Clip::fromVar of a clip JSON whose opacity connection is in the legacy form, then tick with
  playhead 0.5 -> 0.5. RED: 0.

**Note:** a composition-level legacy Timeline (none exist, E13) becomes clip-clocked and shows the hand value. This is
documented.

### ITEM 4 -- Retire the dead "Clip Position" registry signal. Stage 1.

**Files:** src/signal/SignalRegistry.cpp (block :74-80 in initDefaults; getClipPositionSignal :191-199);
src/signal/SignalRegistry.h (include :6, declaration :45); delete src/signal/ClipPositionSignal.h; CMakeLists.txt:217.

**Behaviour:**
- The Audio submenu and the SignalBar "+" menu no longer list a "Clip Position" that never moved.
- It is the LAST registry entry, so the picker ids (100 + index) of every other signal stay the same.
- Signal ids are never saved (ParamConnection.h:37-38).

**Test:** TR1 -- SignalRegistry::initDefaults() has no signal named "Clip Position" (via getSignalByName,
SignalRegistry.cpp:136). RED: it is present.

### ITEM 5 -- Menus offer Clip Position / Timeline only where there is a playhead. Stage 2.

**Files:**
- src/ui/UniversalParamControl.h/.cpp, src/ui/EffectStackView.h/.cpp.
- src/ui/ClipInspector.cpp, src/ui/LayerInspector.cpp, src/ui/CompositionInspector.cpp.
- New tests/test_timeline_picker.cpp, with a target using the sources of test_effect_stack_binding
  (tests/CMakeLists.txt:2158-2176) plus the inspectors' sources.

**Changes:**
- 5a UniversalParamControl.h:
  - `enum class PlayheadSources : uint8_t { Offered, NoPlayhead, NotOffered };`
  - A public `std::function<PlayheadSources()> playheadSources;`. Empty means Offered, which is today's menu.
  - A test seam, `juce::PopupMenu sourcePickerMenuForTest();`, that returns the menu buildSourcePickerMenu builds.
- 5b buildSourcePickerMenu (:510-514 only):
  - Offered -> as today.
  - NoPlayhead -> both items shown, disabled, labelled "Clip Position -- video clips only" and "Timeline -- video clips
    only". Final wording after the critic panel.
  - NotOffered -> both items left out.
  - handleSourcePickerResult is unchanged.
- 5c EffectStackView:
  - The same public provider member, copied to every param control and dry/wet control when rows are built (near :369
    and :424).
  - If it is empty: scope Global -> NotOffered; Layer and Clip -> Offered.
- 5d ClipInspector: the provider `[this]{ return clip_ && clip_->isPlayable() ? Offered : NoPlayhead; }` is set on:
  - the six scalar controls (bindScalarControls);
  - each source-param control (buildSourceParamControls);
  - effectStackView_ (in the constructor).
- 5e LayerInspector: Offered on its controls and on effectStackView_ (set explicitly).
- 5f CompositionInspector: NotOffered on its controls and on effectStackView_.

**RED-first sequence:**
- Commit 1 adds only the seam and the empty provider member, so the menus are unchanged. UI2, UI3a, UI5a and UI5c fail
  there at their assertions.
- Commit 2 makes them pass.

**Tests** (headless JUCE under ScopedJuceInitialiser_GUI; controls found by `boundConnection()`; menus walked with
juce::PopupMenu::MenuItemIterator, as tests/test_output_menu_model.cpp:27 does):
- UI1 -- [pin] Empty provider -> items 2 and 3 present and enabled.
- UI2 -- CompositionInspector: on every control bound to composition_->scalarConns, items 2 and 3 are absent.
- UI3 -- ClipInspector:
  - (a) Image clip -> the opacity control shows items 2 and 3, disabled.
  - (b) [pin] Video clip -> enabled.
- UI4 -- [pin] LayerInspector -> enabled.
- UI5 -- EffectStackView:
  - (a) setEffects(&comp.globalEffects, EffectScope::global()) -> absent.
  - (b) [pin] Layer scope -> enabled.
  - (c) the Clip-scope view inside ClipInspector with an Image clip -> disabled.
- UI6 -- [pin] The Audio submenu has no "Clip Position" item (Item 4).
- UI7 -- [pin] The provider is read when the menu opens: change a Video clip's media type to Image without
  rebinding, and the next menu shows the items disabled.
- Named fallback: if a headless CompositionInspector needs more than about 10 extra sources to link, replace UI2 with
  UI5a plus gate row V4.

**GREEN bar:** all UI tests pass, and test_effect_stack_binding, test_right_click_reset,
test_param_control_routine_cue and test_clip_inspector_paint_key still pass.

### ITEM 6 -- Live witness: .harmony/probe-timeline.{sh,py,json}. Stage 1.

- The Stage 1 builder writes it, then runs it FAILING on the merge-base build and PASSING on the lane build.
- R9's menu half is covered by the UI tests and gate G5. No REST endpoint shows menus.
- The .sh copies probe-crossfade.sh's lock gate and the adna_pids / ucomm process match. The app path comes from env
  TIMELINE_APP.
- Fixtures:
  - Grey 320x240 H.264 clips made with ffmpeg lavfi in the run directory: grey4.mp4 (4 s) and grey6.mp4 (6 s).
  - The image media/P16_01_baseline.png.
  - Compositions written by the .py in the shape of .harmony/probe-lane3-clip.json (clip "conns" keyed by scalar).
- Definitions: n(p) = clamp((p - in) / (out - in)).
- Sampling: /api/composition every 100 ms, with Connection: close.
- Drift vs bar:
  - Lag is at most one GL frame plus one tick, about 25 ms, which is about 0.006 on a 4 s clip.
  - Bar 0.03 is about 5x that drift.
  - Main's mean error is about 0.5, so the bar has real teeth.

Rows (all in ONE launch in test mode under the lock; fixtures loaded one after another with /api/load_composition):
- R1 -- Sweep.
  - Setup: L0C0 = grey4, Loop. Clip opacity, Ripple p0 and layer opacity on new-format Timeline. Clip positionX on
    Clip Position.
  - Bar: over 5 s, on at least 95% of samples, |clip.opacity - n(p)| <= 0.03, and the same for fx p0 and
    layer.opacity.
  - Bar: positionX == ClipScalar PosX toModel(n(p)) (ScalarParams.h) +-2% of its span.
- R2 -- Beats ignored. During R1, drive /api/inject_features with the beat clock going 0 -> 4.5 in 0.5 steps. R1's
  bars must still hold.
- R3 -- Retrigger. When p > 0.6, POST /api/trigger_clip on the same cell. Within 150 ms: p <= 0.05 and
  clip.opacity <= 0.06.
- R4 -- Wrap. Opacity >= 0.85 is seen before the loop point. The first sample with p < 0.1 after the wrap has
  opacity <= 0.13.
- R5 -- OneShot end. Variant: OneShot. After p >= 0.99 and playing is false, clip.opacity >= 0.97 for 2 s.
- R6 -- In / out. Variant: in 0.25, out 0.75, Loop. |opacity - (p - 0.25) / 0.5| <= 0.03 on at least 95% of samples.
  After the seek back to the in point, opacity <= 0.05.
- R7 -- Crossfade.
  - Setup: layer 0 transition 2 s; column 0 = grey4, column 1 = grey6. Trigger column 0; at p0 of about 0.5 trigger
    column 1.
  - Bar: on every sample with 0 < crossfadeProgress < 1, |layer.opacity - n(p1)| <= 0.03 AND
    |clip0.opacity - n(p0)| <= 0.03. At least 5 such samples.
- R8 -- Reverse. Variant: reverse = true, Loop. Over 2 s: Spearman(opacity, time) <= -0.9 and
  |opacity - n(p)| <= 0.03.
- R9 -- No playhead. Layer 1 column 0 = the image, clip opacity on Timeline (written directly), manual opacity 1.0.
  live.opacity == 1.0 +-0.001 for 2 s.
- R10 -- Picture.
  - Separate fixture: only clip opacity on Timeline; layer opacity manual 1; black background.
  - At least 6 render_frame captures across one pass, each paired with the playhead read just after.
  - Bar: decoded mean luma vs n(p) has Pearson r >= 0.98, and luma at n >= 0.9 is >= 4x luma at n <= 0.2.
- R11 -- Legacy file. The R1 fixture with clip opacity in the PRE-bf6 form (clock "beats", no envFormat). R1's
  clip-opacity bar must hold.
- R12 -- Rig hygiene:
  - 0 windows named "Output";
  - the app is gone after quit;
  - the lock is released;
  - 0 UserNotificationCenter windows (Quartz kCGWindowListOptionAll) 20 s after quit.
- R13 -- INFO, the R2 risk. Play column 0 to p of about 0.5, trigger column 1, then trigger column 0 again. Sample
  every 10 ms for 200 ms and report how many samples read p == inPoint before p jumps to about 0.5. Expected 1-3.
  Owner: bf9b.

Pre-registered RED on the merge-base build: R1, R3, R4, R5, R6, R7, R8, R9, R10 and R11 FAIL (main reads 0, or the
frozen beat clock).

### ITEM 7 -- Docs. Stage 2. See section 6.

### TEMP -- A hook for the visual gate. Stage 2; separate commit; REVERTED on the lane branch before merge.

- Env var `ADNA_BF6_SHOT=<clip_video|clip_image|layer|comp|sweep>`.
- It polls (500 ms) until a composition with at least one clip is loaded. Then it:
  - selects layer 0 / column 0 in the inspector (layer 1 for clip_image);
  - opens the named tab;
  - except for `sweep`, opens the picker once on the Opacity control (Master for comp).
- It synthesises no input.
- A grep row in G1 proves it is gone.

### BUILD STAGES

| Stage | Items | Depends on | Ships alone? |
|---|---|---|---|
| 1 engine + data + witness | P0, 1, 2, 3, 4, 6 | bf9b merged | Yes, mergeable alone. Timeline and Clip Position work at clip and layer level. The composition menu still lists them; picking one there now just leaves the knob at its hand value, which is harmless. Do not show Boris until Stage 2. |
| 2 menus + docs | 5, 7, TEMP | Stage 1 | Ships to Boris together with Stage 1 |

## 5. GATES (Harmony, after each merge)

- **G0** (read-only, just before merging Stage 1). Re-run the E13 scan:
  `find ~/Library/AudioDNA ~/Library/Audio-DNA ~/Documents ~/Desktop -maxdepth 4 -name '*.json' -size -20M -print0 2>/dev/null | xargs -0 grep -l -E '"envelope"|"Clip Position"' 2>/dev/null`
  - No hit -> proceed.
  - Any hit -> open the file. If it holds a pre-bf6 Timeline or "Clip Position" connection, Q4 goes to Boris before
    the merge (default: upgrade).
- **G1** (unit tests), on the lane build:
  - `ctest -R "timeline|conn_picker|connection|effect_stack_binding|right_click_reset|param_control_routine_cue|clip_inspector_paint_key" --output-on-failure`
    -> all pass.
  - Full ctest -> no test fails that passes on the merge base (compare the two failure lists).
  - On the merge commit, `git grep -n ADNA_BF6_SHOT` -> 0 lines and `git grep -n ClipPositionSignal src` -> 0 lines.
- **G2** (RED proof):
  - The builder's report shows each RED test's failing output on the merge base (UI tests: on the seam-only commit).
  - Harmony spot-checks TC1 and TU1 by building test_timeline_clock against the merge-base sources.
- **G3** (live). One launch, test mode, under the lock: `.harmony/probe-timeline.sh <out>` on the lane build, every
  row PASS. Run it once on the merge-base build too: R1 and R3-R11 FAIL, as pre-registered.
- **G4** (rig hygiene): row R12.
- **G5** (VISUAL WORK GATE, after Stage 2). Captures by Quartz window id only, decoded PNG:
  - V1: Clip tab, video clip, picker open on Opacity. Clip Position and Timeline are enabled.
  - V2: image clip. Both items greyed out, "video clips only".
  - V3: Layer tab picker.
  - V4: Composition tab picker. Neither item is present.
  - V5a / V5b: Clip tab with the Timeline-driven Opacity slider, about 2 s apart (it moved).
  - V6: R10's three render_frame pictures.
  - Critic panel: visual-design, UX, graphic-design and logic critics, plus an interaction-logic critic. What the menu
    offers depends on level and media, so greyed vs absent must read consistently with the app's other menus.
  - Bar: no open MUST.
  - Then the artifact page for Boris: the captures, the section 8 checks and the section 9 questions.
- **G6** INFO, not a gate -- tick cost.
  - The walk gains one atomic load per layer and one small stack object per clip (grows with the clip count; no
    allocation).
  - If Harmony wants a number: interleaved A/B, at least 5 launches per arm, a 20-deck fixture, `peak_message_stall_ms`
    with /api/debug/heartbeat on.
- **G7** INFO -- TSan.
  - No shared-field type changes: the clock reads the RelaxedDouble playheadPosition and does one tuple load on the
    message thread.
  - So probe-tsan-unit.sh is not required (Pitfall 63's rule covers changes TO a shared field). Run it if convenient.

## 6. DOCS

- **docs/claude/rendering.md** -- add a new "### Timeline and Clip Position follow the clip (bf6)" after the Master
  Signal paragraph (:43-47). It covers:
  - the clock rule (clip / layer / composition) and the formula;
  - no wrap at the end;
  - no playhead = hand value;
  - what the menus offer;
  - envFormat and the two load upgrades;
  - the ~1-frame lag;
  - Master Signal applies (until Q3 says otherwise);
  - guards: tests/test_timeline_clock.cpp, tests/test_timeline_picker.cpp, .harmony/probe-timeline.sh.
- **docs/claude/pitfalls.md** -- a new "Pitfall NN". Harmony assigns the number (next free is 64 unless taken). Text:
  "NN. **Timeline / Clip Position read ONE clip clock -- never nullptr at a clip or layer site, never frac() at the
  end**: `clipTimelinePosition(clip)` = (playheadPosition - inPoint) / (outPoint - inPoint), clamped to [0,1]; NaN
  when the clip has no playhead (`!isPlayable()`: image, source, camera, effects-only). `ConnectionEngine::tick` hands
  clip-owned connections their own clip's `ModelClipClock`, layer-owned ones the layer's playing clip (the INCOMING
  one during a crossfade, Pitfall 35), composition / macro / global-effect ones none; a NaN clock publishes nothing
  (the knob shows its hand value). `ConnectionShaper::clipXform` clamps: `frac(1.0) == 0` would snap a OneShot clip
  parked at its end back to the curve start. The playhead is GL-written (RelaxedDouble): `.load()` it on the message
  thread, never cache it across ticks. Guards: tests/test_timeline_clock.cpp, tests/test_timeline_picker.cpp; live
  .harmony/probe-timeline.sh."
- **docs/claude/architecture.md:235** -- remove ClipPositionSignal from the list of signal types.
- **CLAUDE.md** -- one index line of at most 150 bytes: "NN. Timeline / Clip Position read ONE clip clock (own clip;
  layer = its playing clip; none = hand value) -- before touching ConnectionEngine::tick's walk." CLAUDE.md is at
  24,002 of 25,000 bytes; Harmony decides who gets the remaining space across lanes. Nothing else changes in CLAUDE.md.
- **.harmony/APP-INVENTORY.md:**
  - :105 picker row -> "Cascading popup: Manual / Audio signals / BPM-sync / Oscillators / Envelopes / Clip Position /
    Timeline / Macros -- Clip Position and Timeline follow the play position of the knob's clip between its in and out
    points (a layer knob: the clip playing in the layer); greyed 'video clips only' on stills / sources; not offered on
    Composition controls (bf6); right-click = reset; [-]/[+]; Invert + Range".
  - :122 GL-thread row: remove "ClipPositionSignal playhead".
  - :281-286: re-count the registry signals from code and remove ClipPositionSignal.
  - Removed-items table (in the :356 style): "ClipPositionSignal | RETIRED bf6 (never wired; Kind::ClipPosition reads
    the owning clip)".
- **BORIS_DECISIONS.md "Playback Behaviour"** -- add: "- **Timeline follows the clip (2026-10-02, s-rta-1002b,
  verbatim):** "when I set any clip parameter to timeline it should be locked to the clips playhead, same with layer,
  not necessary for composition controls" -> a knob set to Timeline (or Clip Position) follows its clip's play
  position between the in and out points (it loops, reverses, restarts on retrigger and holds at the end with the
  clip); a layer knob follows the clip playing in that layer; Composition controls do not offer it." Add the answers
  to Q1-Q4 when they arrive.
- **Code comments:** ConnectionEngine.h :12-20 and :59-73; ConnPicker.cpp :122-126; ConnectionEngine.cpp :353-356.

## 7. RISKS (each with the cheapest test that would catch it)

- **R1** -- bf9b changes how a layer knows its playing clip. Check: P0; run TC5 on the merged tree.
- **R2** -- A clip coming back after another clip played.
  - Activation writes the model playhead = in point, but the player resumes from where it was (E10).
  - So a Timeline knob shows its curve start for 1-2 frames on a cut. A fade hides it, because the incoming clip
    starts at fade 0.
  - Check: probe row R13 (INFO).
  - Owner: bf9b. It should decide whether a re-fired clip restarts or resumes, and make the model match:
    - either seek the player at activation, as the retrigger path does (MainComponent.cpp:4712-4722),
    - or stop writing the in point at Layer.h:539.
- **R3** -- About one frame of lag (E20). Only visible once curves have sharp edges, i.e. when they can be drawn.
  Check: R1's tolerance; INFO.
- **R4** -- A null clock changes meaning from "position 0" to "no playhead". Grep finds only one test that relies on
  the old meaning (test_connection.cpp:1014-1018), and Item 1 changes it. Check: G1 full ctest.
- **R5** -- The load upgrade could catch a future beats envelope. Prevented by the envFormat marker. Check: TU2 pins it.
- **R6** -- Master Signal turns Timeline down. Boris decides: Q3.
- **R7** -- Menu wording, and whether "greyed" and "absent" read consistently. Check: the interaction-logic critic in
  G5.
- **R8** -- A SignalBar column or a macro wired to the retired "Clip Position" signal. It reads 0 today and keeps
  reading 0, because an unknown id gives 0 (SignalRegistry.cpp:181-189). Clip and layer connections to it now WORK
  through TU3. No saved file uses it (E13).
- **R9** -- Reverse play with an in point above 0 runs below the in point (E6, already true before this lane). The
  knob sits at its curve start there. Logged as O2.
- **R10** -- tsan-r5 (later) turns inPoint / outPoint into Relaxed<T>. clipTimelinePosition must then `.load()` them
  (Pitfall 63 rule 1). tsan-r5 owns that.
- **R11** -- BF2's sync dial must not shift Timeline or Clip Position: they are locked to the picture, not the sound.
  Note sent to bf2.

**Strongest alternative root cause:** "the playhead is not advancing". Refuted twice:
- diag live run 1: the playhead went 0.04 -> 0.99 while the value stayed 0;
- diag [E]: given a clock, the value follows the playhead.

## 8. WHAT ONLY BORIS CAN CHECK (on the live rig)

1. On a video clip, set Opacity (or an effect knob) to Timeline:
   - the knob rises from low to high as the clip plays and drops back at each loop;
   - retrigger sends it back to the start;
   - dragging the clip's timeline bar moves the knob, even while paused.
   Does it feel locked to the clip? Is the drop at the loop fine, or would a short glide feel better?
2. A layer knob on Timeline restarts with each new clip at the moment that clip starts, also during a fade. Is that
   fine, or should it blend across the fade?
3. Invert and Range on a Timeline knob reverse and narrow the sweep as expected.
4. Menus:
   - "video clips only", greyed out, on pictures and sources;
   - no Timeline in the Composition tab;
   - no "Clip Position" under Audio.
5. On a OneShot clip, the knob stays at the top when the clip ends.

## 9. QUESTIONS FOR BORIS (plain words; each has a default, so the build never waits)

- **Q1 Pictures and sources.** "A still picture or a generated source (like MilkDrop) has no play position. If you
  set a knob on one of those to Timeline, should it (a) be greyed out there, and the knob stays where you set it
  [DEFAULT], or (b) sweep once over a length you set in bars, starting when the clip starts?"
- **Q2 Two entries.** "After this fix, 'Clip Position' and 'Timeline' do the same thing: the knob goes from its low
  end to its high end as the clip plays. Once you can draw the Timeline curve (you approved that earlier), Timeline
  will follow your drawing and Clip Position will stay a straight line. Keep both in the menu [DEFAULT], or remove
  Clip Position to keep the menu short?"
- **Q3 Master Signal.** "The Master Signal fader turns down how much signals move the knobs. Should it also turn down
  knobs that follow a clip's Timeline? (a) Yes, Timeline counts as a signal [DEFAULT, how it works today]. (b) No, a
  Timeline knob always follows its clip fully."
- **Q4 Old shows** (ask ONLY if G0 finds a hit). "A show you saved before this fix has knobs set to Timeline. From
  now on they will follow their clip like new ones. OK? [DEFAULT yes]"

## 10. FILES SHARED WITH OTHER LANES (exact blocks, for merge order)

| File | bf6 touches | Same file, other lane |
|---|---|---|
| src/connect/ConnectionEngine.cpp | evaluate() :139-151; tick() deck/layer/clip loop :339-384 | bf9b (the layer walk, if layers become shared); bf6 rebases on it |
| src/connect/ConnectionEngine.h | ClipClock block :12-25; evaluate doc :59-75; new declarations | -- |
| src/connect/ConnectionShaper.h/.cpp | new clipXform (appended) | bf2 may touch beatsNow (beat-clock shift): different function |
| src/connect/ConnPicker.cpp | case Timeline :90-100; describeSource comment :122-126 | bf7: kBpmDivisions :16, formatBeats :22-36 |
| src/connect/ConnSerialization.cpp | toVar envelope :120-137; fromVar signal :180-184, envelope :204-238 | bf45, if it serializes envelopes through here |
| src/signal/SignalRegistry.{h,cpp}; CMakeLists.txt:217 | remove block :74-80, accessor :191-199, include / declaration | bf45 (BF5 sample envelopes) may add signals near the Mod 1 / Mod 2 block :63-72 |
| src/ui/UniversalParamControl.{h,cpp} | new enum / provider / seam; buildSourcePickerMenu :510-514 | bf7: BPM Sync block :442-466, handleSourcePickerResult :598-617 |
| src/ui/EffectStackView.{h,cpp} | provider member; row build near :369, :424 | -- |
| src/ui/ClipInspector.cpp | constructor (effectStackView_ provider); bindScalarControls :811-824; buildSourceParamControls :826-862 | ui (BF3 codec / Show in Finder); bf7 (beat selectors in the constructor) |
| src/ui/LayerInspector.cpp | constructor (provider); bindScalarControls :753-767 | bf9b (Persistent removed); bf7 (autopilot beats) |
| src/ui/CompositionInspector.cpp | constructor (provider); bindScalarControls :399-413 | bf7 (autopilot duration); maybe bf2 |
| tests/CMakeLists.txt | two new targets (appended) | every lane (append only) |
| docs: rendering.md, pitfalls.md, architecture.md:235, CLAUDE.md index, APP-INVENTORY.md :105/:122/:281-286, BORIS_DECISIONS.md | see section 6 | the CLAUDE.md byte cap is shared by all lanes |

Notes for other lanes:
- **bf9b:**
  - the returning-clip activation mismatch (R2);
  - bf6 will call bf9b's accessor for a layer's playing clip.
- **bf45:**
  - the Timeline editor contract (section 2);
  - the envFormat marker;
  - describeSource currently names EVERY Envelope "Timeline". A beat-clocked envelope from bf45 needs its own name.
- **bf2:** do not shift clip-clocked sources.
- **tsan-r5:** clipTimelinePosition reads inPoint / outPoint / mediaType on the message thread.

## 11. OBSERVATIONS FOR HARMONY (already true before this lane; not built here)

- O1 -- Beat-snapped triggers seek to beatPhase x the whole file and ignore the in point (MainComponent.cpp:4677-4697).
- O2 -- Reverse play is not bounded by the in point. ClipTransportSync.h:55-72 only enforces the forward out point.
- O3 -- Changing a knob's source in the picker cannot be undone (no onSourceChanged listener). s166 D10 recommended
  that Cmd-Z removes a connection.
- O4 -- A control bound to a connection loaded from a file does not tick its current source in the picker.
  bindConnection sets the button text but not sourceMode_ (UniversalParamControl.cpp:126-146).
- O5 -- s166 L5's "loop=false restarts on clip trigger" for beat-clocked sources is not built, and nothing can reach
  it (no UI sets ConnShape::loop, E14).
- O6 -- The clip transport mode is also called "Timeline" (ClipInspector.cpp:9-15), so a "Timeline" knob on a
  "BPM Sync" clip reads oddly. A naming question for the UI rewrite.
- O7 -- The Audio submenu lists hidden registry signals, because there is no visibility filter
  (UniversalParamControl.cpp:426-437).

STATUS: DONE

## HARMONY ADOPTION (s-rta-1002b, 2026-10-02 16:48:04) — overrides the ruling, which overrides the plan body
1. ADOPTED: ruling-bf6.md IN FULL (AM-1..AM-18). NOT buildable until bf9b merges with contracts C1 (playing-clip
   accessor), C2 (the walk) and C3 (playhead == player at activation) — relayed into bf9b's plan prompt; Stage 1 starts
   at P0 on bf9b's merged tree. Stage 2 waits for Boris's Q1.
2. BORIS QUESTIONS — defaults until he answers: Q1 (a) Timeline greyed out "video clips only" on pictures / sources
   (asked in chat); Q2 keep both "Clip Position" and "Timeline"; Q3 Master Signal also turns down Timeline knobs. Q4 only
   if the pre-merge scan finds an old file.
3. G1 per AM-16: run the test binaries by path and check --list-tests (a ctest -R on file names can select 0 tests).
4. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA your lane did not start is his (s-rta-1002b incident): never quit / kill / touch it; the lock helper waits for it; if start_app refuses, stop the batch and release. Visual work: Harmony's critic panel on decoded captures before Boris sees it. MERGE by Harmony; rebase onto whatever main is at launch.
