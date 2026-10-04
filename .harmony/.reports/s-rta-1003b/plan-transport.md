# plan-transport -- the TRANSPORT + UNDO-LIVE lane (s-rta-1003b)

Author: architect. Read-only. Source read ONLY at main 34179a2 (`git show 34179a2:<path>`, `git grep ... 34179a2`).
Nothing was built, run, launched or probed. Nothing under `.claude/worktrees/` was read.
Labels: VERIFIED = read at 34179a2 at the cited line. INFERRED = reasoned from lines read, not run. ASSUMED = not read.
Boris is quoted only verbatim (sources: `.harmony/boris-feedback-backlog.md` from "Boris feedback of 2026-10-03",
`.harmony/binding-decisions.md` from the first "## 2026-10-03"). "Harmony constraint:" marks what Harmony requires.

## 1 GOAL

One lane, five build stages, that makes these true on main after the sync-dial lane (bf2) has merged:

- G-A Every fire of a clip starts its video (or image sequence) from its start and plays it -- a cell, a column, a
  key / MIDI / OSC / REST binding, a routine, a take replay, a quantized release, an Autopilot advance. One rule, one
  place in the code (the layer's activation tail + the render thread), so no path can be forgotten again.
- G-B A column fired again restarts the videos it already plays (BF11).
- G-C A playhead dragged on either bar (Clip tab, layer strip) shows the frame under the mouse while held and plays
  on from the drop point at the same speed and direction (BF25).
- G-D A BPM-synced clip has a BPM box (120 on every clip) and a Beats box with /2 and x2; its speed is
  show BPM / clip BPM and follows the tempo (BF15, answers 5, 6, 7 + follow-up).
- G-E Cmd+Z (and Redo) never starts, stops, replaces or rewinds what a layer plays; it undoes everything else (BF31).

NOT in this lane (explicit): the Quantize value list and sub-beat release edges (bars lane bf7); the Edit menu (BF33,
ui-polish); removing on-screen notices (BF32 lane) -- this lane adds none; the Timeline curve (bf6); the dead
"Duration" row (bf7 ruling AM4 owns its removal; see 4.4); phase-locking a clip to the bar grid (rejected, T4 fork
F9); an Undo step for bypass / solo being dropped (question Q3); reverse play inside a trimmed in..out range
(existing gap, section 9 R8); a REST play / pause route; `applyClipPlaying("stop")` (model-only rewind, untouched);
Quantize of an empty-cell clear in a column fire (immediate today, untouched).

## 2 ESTABLISHED FACTS (each VERIFIED at 34179a2 unless labelled)

Firing
- E1 Two message-thread entries: `MainComponent::handleClipTrigger(int layerIndex, int column, Origin, int deckIndex,
  bool immediate)` src/MainComponent.cpp:4789 and `handleColumnTrigger(int column, Origin, int deckIndex)` :5038.
  Callers: deck grid :688 / :691; REST :1886 / :1887; OSC :2319; bindings :7716 (TriggerClip; the 3 targeting modes are
  resolved before this line into resolvedLayer / resolvedColumn / resolvedDeck, :7694-7698) and :7753 (TriggerColumn);
  take / routine replay :1970 (`Origin::Replay`; `immediate` = a checkpoint-0 restore, :1941); a routine's own snap
  uses `quantizeModeToForcedSnap` :4280, :6255.
- E2 GL-thread entries that never pass the handlers: the quantized release `Layer::processPendingTrigger`
  (src/model/Autopilot.cpp:85, on a beat crossing, Pitfall 42) and the Autopilot advance `Layer::triggerClip`
  (Autopilot.cpp:326, :355, :417). The advance never names the active column (`nextCol != currentCol` :325; candidates
  exclude the current column :339-345). No other caller of `triggerClip` / `triggerClipImmediate` / `fire` /
  `triggerColumn` exists in src (grep at 34179a2: MainComponent.cpp:4839, :4850, :5080; Composition.h:433-434, :453;
  Autopilot.cpp as above). No separate "structural scene" trigger function exists (grep
  `sceneTrigger|structuralTrigger|onSectionChange|triggerScene` = 0 lines): INFERRED it is the Autopilot advance.
- E3 Every activation ends in `Layer::applyActivationTail` src/model/Layer.h:548-558: model playhead = in-point,
  `beatsPlayed = 0`, and `playing = true` only when `to.activeRef() != from.activeRef() && !clip->hasBeenTriggered`
  (:555). `activate()` runs it also for a retrigger that changes no tuple field (:576-578); the quantized release
  runs it at :484-487.
- E4 A re-fire of the active ref is never queued: `if (snapEnabled && ref != r.activeRef())` Layer.h:424; and
  `immediateNext` returns the tuple unchanged for the active ref (Layer.h:366-367).
- E5 The only writer of `Clip::hasBeenTriggered` is handleClipTrigger (MainComponent.cpp:4866); readers: Layer.h:555
  and `triggerWillAutoPlay` MainComponent.cpp:196. The column path never sets it.
- E6 handleClipTrigger's player seeks, all on the message thread: the clip's own legacy `beatSnap` branch seeks to
  `snap.beatPhase` as a file position (:4868-4889); a retrigger seeks to the in-point (:4890-4914); a new activation
  runs C3 `syncActivatedPlayhead` (:4915-4920; body :5006-5019: model playhead <- player position, no seek).
  handleColumnTrigger has no `seekTo`; its only transport step is C3 for a new activation (:5099-5102).
  All `seekTo(` call sites in src: MainComponent.cpp:1470, :1475, :4881, :4886, :4907, :4912;
  render/ClipTransportSync.h:69.
- E7 The render thread overwrites the model playhead from the player on every frame a clip is drawn
  (`ClipTransportSync::writeBack` ClipTransportSync.h:44-48, called from `Renderer::syncMedia` Renderer.cpp:1614 ff.),
  so a model-only reset is invisible. `pushIntent` (:33-42) pushes `clip.playing` to the player; the write-back of
  the play state is a compare-exchange on the intent (:50-53). Out-point: `ph >= outPoint` -> `seekTo(inPoint)` or a
  OneShot stop (:56-71).
- E8 `VideoPlayer::seekTo` stores a request (media/VideoPlayer.cpp:354-359); `advanceFrame` consumes it on the GL
  thread BEFORE the transport step, playing or paused (:366-376), and bumps the generation (:382-385). A seek touches
  neither `pingPongForward_` nor `reverseNow_`. `advanceTransport` (:397-470): `currentTime_ += dt * speed *
  direction`; a OneShot stops at either end and stores `playing_ = false` (the two OneShot cases inside the boundary
  switches, :419-453).
- E9 A column fire: `Composition::triggerColumn` -> `Layer::triggerClip` on every layer without `ignoreColumnTrigger`
  (Composition.h:440-453); an empty cell clears the layer (`triggerClip` -> `clearActiveClip`, Layer.h:416-418); the
  clear tail stops the old clip (`playing = false`, `applyClearTail`, Layer.h:586-592).
- E10 Capture for takes: an `activeClip` point per fire with `p.retrigger` (cell path only, inside the capture block MainComponent.cpp:4985-4999)
  and a `playing` "resume" point when the fire auto-plays (`captureAutoPlay` :5025-5036).

Playhead drag
- E11 Layer strip: `LayerStrip::scrubPlayhead` writes ONLY `clip->playheadPosition` (ui/LayerStrip.cpp:979-989); mouse
  down / drag call it (:959-963, :971-976). No seek. With E7 the next frame puts the old position back.
- E12 Clip tab: `ClipInspector::mouseDown` (ui/ClipInspector.cpp:1349-1376) grabs the in marker when the press is
  within 8 px of it, else the out marker within 8 px, else scrubs: model playhead + `onCuepointJump` (:1373-1374),
  wired at MainComponent.cpp:1464-1477 to `player->seekTo(pos)` / `seq->seekTo(pos)`. `mouseDrag` repeats it
  (:1394-1398); `mouseUp` only clears the drag target (:1404-1407). Both markers have a drawn tab (:1285-1314).
- E13 Nothing holds the clip while the mouse is down: between drag events the player keeps advancing (E8).

BPM sync
- E14 Model: `Clip::TransportMode { Timeline, BPMSync }` model/Clip.h:118-119; `speed` :122, `reverse` :123,
  `inPoint` / `outPoint` :127-128, `beatDivision = 4` :36, `videoBeats = 4` :41, `sequenceFps = 2.5` :32; runtime
  `playing` :237, `playheadPosition` :238, `beatsPlayed` :239, `hasBeenTriggered` :240. `beatDivision` / `videoBeats`
  are saved (model/Clip.cpp:47-48) and loaded under `hasProperty` guards (:189-192); `replaceContent` copies them from
  the new content (Clip.h:294-295); `clear()` resets them (:347-348). No clip-BPM field exists.
- E15 Video, BPM Sync: `player->setSpeed(videoBeats / beatDivision)` when `snap.bpm > 0` (Renderer.cpp:1657-1667):
  the tempo is only a gate. Timeline: `effectiveClipSpeed(speed, masterSpeed, false)` (:1670; Renderer.h:290-293).
- E16 Sequence, BPM Sync: fps = frames * bpm / (beatDivision * 60) recomputed every frame (:1718-1733), speed =
  `clip.speed` (:1705-1706). Timeline: fps = `sequenceFps` (:1709).
- E17 Clip tab widgets: mode combo "Timeline" / "BPM Sync" (ClipInspector.cpp:9-10); "Beats/ Cycle" combo, 7 presets
  0.25..16 (:204-234); "Content Beats" slider 1..64 snapped to powers of two (:237-275); an inert "Restart / Continue
  / Relative" combo (:69-73, bounds :634) and an inert "Duration" slider (:90-98, bounds :660). The strings "Loop
  length", "Content length", "Image every" do not occur in src/ui (grep = 0): the bars lane (bf7) is NOT in 34179a2.
- E18 `VideoPlayer::getDuration()` returns a plain `double duration_` (media/VideoPlayer.h:70, :170);
  `ImageSequence::getDuration()` exists (ImageSequence.h:45). `FeatureSnapshot::bpm` = "stabilized/locked tempo
  estimate (BPM), 0 if unknown" (analysis/FeatureSnapshot.h:40).

Undo
- E19 Commands (all `: public Command`; grep): TriggerClipCmd (core/TriggerCommands.h:62); SetClipCmd,
  ToggleClipLockCmd, SwapClipsCmd (core/ClipCommands.h:80, :146, :188); SetColumnCountCmd, RemoveColumnCmd,
  ClearLayerClipsCmd, ClearActiveClipCmd, ToggleLayerFlagCmd, AddLayerCmd, RemoveLayerCmd, MoveLayerCmd, AddDeckCmd,
  InsertDeckCmd, RemoveDeckCmd, RenameDeckCmd (core/DeckCommands.h:54, :95, :309, :392, :435, :479, :538, :619, :675,
  :772, :917, :1012); EffectStackCmd (core/EffectCommands.h:82); CompositeCommand (core/CompositeCommand.h:15).
  Push sites: MainComponent.cpp lines listed by `git grep -n 'pushCommands(\|make_unique<[A-Za-z]*Cmd>' 34179a2 -- src`.
- E20 A fire is an Undo step: "Trigger Clip" (:4976-4983), "Trigger Column" (:5104-5125), Human origin only.
  TriggerClipCmd undo / redo store the tuple and the target's `playing` (TriggerCommands.h:76-85, :118-128).
- E21 ClearActiveClipCmd (layer X button, pushed :735-737, and inside the clear-cell composite :7061) restores the
  tuple on undo (DeckCommands.h:411, :420). ClearLayerClipsCmd restores `state.runtime` with `setRuntime`
  (:337-339). RemoveLayerCmd's undo re-inserts the captured `Layer removed_` value (:580-590) -- INFERRED: with the
  tuple it had when removed. AddLayerCmd's undo erases the layer unconditionally (:506-513). InsertDeckCmd's undo:
  no added layers -> `retireOrEraseDeck` (:849-856); added layers -> the deck is erased and the last k layers are
  erased by position, playing or not (:858-864). SetClipCmd / SwapClipsCmd value-copy a whole `Clip` into a cell
  (`cell = *state`, ClipCommands.h:112-122, and `applyCell`), runtime fields included. RemoveColumnCmd's redo calls
  `deck->removeColumn` with no look at a tuple (DeckCommands.h:121-127).
- E22 `POST /api/debug/undo {redo}` exists (api/ApiServer.cpp:340, :2367-2379). `/api/set_clip_param` :192.

Pinned today (tests / probes that this lane re-registers; names in T6)
- E23 tests/test_layer_runtime.cpp:280 SECTION "retrigger: the tuple is unchanged, the clip restarts from its
  in-point and keeps its play state" (asserts `playing == false` :299). tests/test_show_model.cpp:706 "T6h THE
  EXCEPTION, pinned: ..." (asserts 3 layers, deck erased, `playingClip(0) == nullptr` :727-732).
  .harmony/probe-boxes.py:1485 `k10_fresh_and_resume` ((ii) "the re-fired clip resumes where it left" :1523-1525).
  `ruling-bf9b-merge.md:29`: "Harmony constraint: K10 and the C3 resume contract stay as built in this lane"; :537-538
  hands the change to this lane.
- E24 Gate scripts at 34179a2: `.harmony/probe-asan-unit.sh` (last line `PROBE-ASAN-UNIT GREEN (<n> cases, 0
  reports)`, `EXPECTED_ASAN_CASES=10`), `.harmony/probe-tsan-unit.sh` (`EXPECTED_TSAN_CASES=5`),
  `.harmony/probe-boxes.sh` (`EXPECTED_ROWS=25`, final line `PROBE-BOXES GREEN`).

Fact sheet (facts-transport.md, written at a7491d4) vs 34179a2: every behaviour I re-read is unchanged; line numbers
moved (handleClipTrigger 4686 -> 4789; the hasBeenTriggered write 4763 -> 4866; beat-snap seek 4765 -> 4868; retrigger
seek 4787 -> 4890; C3 call 4812 -> 4915; syncActivatedPlayhead 4903 -> 5006; handleColumnTrigger 4935 -> 5038; column
C3 4996 -> 5099; onCuepointJump 1466 -> 1464; REST wiring 1878 -> 1886; binding fires 7654 -> 7716 / 7753;
applyClipPlaying 6440 -> 6540; routine snap 4176 / 6156 -> 4280 / 6255; LayerStrip scrub 1068 -> 979; ClipInspector
mouse handlers 1223-1283 -> 1349-1407; VideoPlayer seekTo 366 -> 354). NOT re-read (use the sheet as INFERRED):
ImageSequence.cpp, the decode-thread seek (VideoPlayer.cpp ~800-830, ~1320), RoutineEngine.cpp, TopBar.cpp.

## 3 ITEMS

### T1 Every fire restarts a video

TODAY (E3-E7). A cell re-fire of the playing clip seeks its player to the in-point and keeps play / pause. A clip
that was replaced and is fired again RESUMES (C3). A first fire starts at file position 0, not at the in-point
(nothing seeks a fresh player). A clip whose own Snap is on is seeked to the beat phase as a file position. A
quantized release and an Autopilot advance never seek (GL thread, no handler). A OneShot that ended has
`playing == false` and a cell re-fire leaves it so (INFERRED from E3 + E8: it does not play again).

BORIS. Asked "A video you played, replaced, then fire again: continue where it left off (today) or restart?":
"restart". And: "Regardless of the clip being BPM or speed controlled, if we are in Qantize mode, it is triggered on
time by the Qantize method." And (2026-09-26): "this is a playing app. there is no stop buttons anywhere."

THE RULE (one sentence). A fire that makes a clip its layer's active clip -- or names the clip that already is --
restarts it: playhead to its start, playing on. Where it is decided: `Layer::applyActivationTail` (every path ends
there, E3). Where it is carried out: the render thread, on the first frame that draws the clip.

Forks
- F1 Play state on a fire. CHOICE: every fire plays (the tail sets `playing = true` always). RUNNER-UP: keep
  today's rule (a paused clip stays paused; only a never-fired clip auto-plays). Why it loses: a OneShot that ended
  would never play again on a re-fire (E8); the cell and column paths already disagree (E5: a column fire auto-plays
  every new activation because the flag is never set); and a restarted clip that sits frozen on its first frame is not
  a restart on stage. Cost: CLAUDE.md "Transport state" and Pitfall 7 are replaced (T9), `hasBeenTriggered` is
  deleted. Put to Boris as Q1 (default = the choice).
- F2 A re-fire under Quantize. CHOICE: it is queued like any other fire and restarts on its beat / bar. RUNNER-UP:
  immediate, as today (E4). Why it loses: Boris's sentence above has no exception; a column re-fire would restart
  some videos now and switch the other layers on the bar.
- F3 Mechanism. CHOICE: a restart counter on the clip, bumped by the tail, consumed by the render thread.
  RUNNER-UP: seek from the two handlers (today's shape, extended to the column path). Why it loses: the quantized
  release and the Autopilot advance run on the GL thread and never reach a handler (E2); a player that is not open
  yet cannot be seeked from the message thread (first fire at file 0, E6); three seek branches stay to be kept in
  step.
- F4 Start of a reversed clip. CHOICE: a clip with `reverse` on restarts at its END (just inside the out-point); a
  PingPong restarts on its first leg at the in-point. RUNNER-UP: always the in-point. Why it loses: a reversed
  OneShot restarted at the in-point stops on the next frame (E8), a reversed Loop wraps at once.

THE CHANGE (no code)
- model/Clip.h: new runtime field `restartSeq` (`Relaxed<uint32_t>`, not saved, Pitfall 63); delete
  `hasBeenTriggered`; new pure `double Clip::restartPosition() const` (in-point; `outPoint - 1e-4` when `reverse`).
  New `void Clip::carryLiveTransportFrom(const Clip& live)` (playing, playheadPosition, beatsPlayed, restartSeq,
  scrubHeld -- used by T5 so a value copy never rewinds or re-restarts a live clip).
- model/Layer.h `applyActivationTail`: playhead = `restartPosition()`, `beatsPlayed = 0`, `playing = true`,
  `restartSeq.fetchAdd(1)`. `triggerClip`: drop `&& ref != r.activeRef()` from the queue test (F2); the release
  (`processPendingTrigger`) already runs the tail when the pending ref becomes / is the active ref (:484-487).
- render/ClipTransportSync.h: new `applyRestart(const Clip&, Player&)` called by `Renderer::syncMedia` before
  `pushIntent` in both branches: when `clip.restartSeq` differs from the player's last seen value, call the player's
  `restart(position)` and store the seen value. Player concept gains `uint32_t restartSeen()`, `void
  restart(double normalized)` (GL thread only: the existing seek request + first PingPong leg). A fresh player's seen
  value is 0, so a clip fired before its player opened starts at its start on its first drawn frame.
- media/VideoPlayer.h/.cpp, media/ImageSequence.h/.cpp: `restart`, `restartSeen_`.
- MainComponent.cpp handleClipTrigger: delete :4866 and the three branches :4868-4920; handleColumnTrigger: delete
  :5099-5102; delete `syncActivatedPlayhead` (:5006-5019, MainComponent.h:557-558). `triggerWillAutoPlay` (:192-198)
  becomes "the target exists and is not playing" (the "resume" capture point then records every fire that turns a
  clip on). The preview / label block (:4922-4958) and the capture block stay.
- What stays untouched: a still image (no transport); a procedural source, MilkDrop, the camera (no start: only
  `beatsPlayed` resets, as today -- "For Milk drop there is no timeline, but there are effects."); the crossfade of a
  re-fired clip keeps running (`immediateNext` returns the tuple, E4); a deck switch fires nothing; a momentary
  release clears (Layer.h:509-526); `applyClipPlaying` play / pause / resume / stop / reverse (MainComponent.cpp:
  6548-6555) are not fires.

EVERY PATH (after the lane; "restart" = start position + playing, on the frame the clip is first drawn)

| path | entry | restart happens | notes |
|---|---|---|---|
| cell click | handleClipTrigger :688 | at once, or on its beat / bar when Quantize or the clip's Snap is on | re-fire included (F2) |
| column fire | handleColumnTrigger :691 | same, every non-ignoring layer | T2 |
| REST trigger_clip / trigger_column | :1886 / :1887 | same | |
| OSC | :2319 | same | |
| key / MIDI binding, ByPosition / ThisItem / Selected | :7716 / :7753 | same | Selected on a retired deck's clip fires nothing (:7708, unchanged) |
| piano / momentary press | same lines | same | release = clear (or cancel the queue), no restart |
| take replay, routine replay | :1970 (Origin::Replay) | same rule; a replayed fire restarts | no Undo step, no capture (unchanged) |
| routine / take checkpoint-0 restore | :1970 with `immediate` | immediate; a layer already on that clip restarts (as today's retrigger seek did) | never queued |
| quantized release | Autopilot.cpp:85 (GL) | on the beat / bar, same frame | NEW: today no seek |
| Autopilot / smart advance | Autopilot.cpp:326, :355, :417 (GL) | at the advance | NEW: today resumes; never names the active clip (E2) |
| deck switch | handleDeckSwitch | fires nothing | unchanged |

TESTS, RED first (each fails at 34179a2 for the reason in brackets)
- U1 test_layer_runtime: "a fire of the active clip plays it and bumps restartSeq" [playing stays false; no field].
- U2 test_layer_runtime: "a new activation of a paused, previously fired clip plays it" [keeps paused].
- U3 test_layer_runtime: "with a forced snap a re-fire of the active clip is queued; the release restarts it and
  leaves the fade alone" [immediate].
- U4 test_layer_runtime: "a reversed clip restarts just inside its out-point" [in-point].
- U5 test_clip_transport_sync (fake player): "a changed restartSeq restarts the player once, before the intent is
  pushed; an unchanged one never does" ; "a fresh player (seen 0) under a fired clip (seq 1) starts at the start".
- U6 test_clip_transport_sync: "restart resets a PingPong to its first leg".
- U7 test_composition "Retrigger resets playhead" (:129) keeps passing and also checks the bump.
- U8 [tsan] test_layer_runtime_race: message-thread fires vs the GL-side consume of `restartSeq` (no report).
- Lint L1 (tests/test_render_thread_lint.cpp, new case): `seekTo(` occurs in src/MainComponent.cpp only inside the
  one scrub function of T3 (count pinned); `hasBeenTriggered` occurs nowhere in src.
- Mutants: MU-T1a the tail does not bump -> U1 RED (and live row TR2); MU-T1b `applyRestart` runs after the
  player advanced -> U5 "before the intent is pushed" RED; MU-T1c the queue test keeps `ref != activeRef` -> U3 RED; MU-T1d the tail keeps the `hasBeenTriggered`-style
  condition -> U2 RED.
GATE: section 5 rows TR1, TR3, TR4, TR5, TR6.

### T2 A column re-fire restarts its videos (BF11)

TODAY (E6, E9). The column path never seeks: layers already playing the column's clip keep running; the tail's model
reset is overwritten on the next frame (E7).
BORIS. "Firing a column that is already playing should restart its videos"
FORK F5. CHOICE: no rule of its own -- a column fire is a fire of each non-ignoring layer's cell, and T1's rule
applies per layer. RUNNER-UP: a column-only "restart the whole column" step in handleColumnTrigger. Why it loses: it
is the per-path seek T1 removes, and it would miss a quantized column fire (released on the GL thread).
Per layer, after the lane: already playing the column's clip -> restarts (on the bar with the others when Quantize is
on, F2); playing another clip -> the column's clip comes in from its start (was: resumed); empty cell -> the layer is
cleared at once, as today (E9; Boris's default Q5 of 2026-10-02 in BORIS_DECISIONS.md:363); Ignore Column -> untouched.
THE CHANGE: none beyond T1 (the C3 call at :5099-5102 goes).
TESTS: U9 test_show_model "T4b a column fired twice restarts every layer that plays it: each clip's restartSeq is
bumped, the empty-cell layer stays cleared, the Ignore Column layer's clip is not bumped" [no bump field / playing
untouched]. Mutant MU-T2: `Composition::triggerColumn` skips a layer whose active ref is the column's -> U9 RED.
GATE: TR2.

### T3 The playhead drag (BF25)

TODAY. Three causes, all VERIFIED in code, none run:
- C-A (the jump "back to where it was"): the layer strip's bar writes only the model (E11); the next frame's
  write-back restores the player's position (E7). The picture never moved.
- C-B (the jump after a drop near the end): a drop at or past the out-point is sent to the in-point on the next
  frame (ClipTransportSync.h:56-70).
- C-C (the playhead does not come along): a press within 8 px of the in or out marker drags the marker (E12) -- with
  the default in 0 / out 1 that is both ends of the bar.
- C-D (while the mouse is down): nothing holds the clip (E13); it plays on from each drag event, so when the hand
  rests the playhead slides away and the next movement pulls it back.
No decode-side cause was found for a jump AFTER the drop: a seek is one request, consumed on the GL thread, one
generation bump; it leaves the direction and the PingPong leg alone (E8). INFERRED for reverse / PingPong (the GOP
cache re-plan of Pitfall 62 was not re-read, not run).
Per transport mode after a drop today (Clip tab) and after the lane (both bars): free-running: plays on at
`speed`; BPM-synced: plays on at its rate (T4: a rate, never a position -- there is no "where it should be"); reverse:
plays on backwards; PingPong: plays on in the leg it was in; OneShot: plays on to its end and stops; a stopped or
paused clip stays stopped at the drop point.

BORIS. "When I grab the play head and move the timeline, I do not want it to jump back to where it was or where it
should be playing before I grabbed it. I wanted to keep playing at the same speed, but play from wherever I drop the
play head. Does this make sense?"

Forks
- F6 While the mouse is down. CHOICE: scrub -- the picture shows the frame under the mouse and the clip holds
  there; on release it plays on. RUNNER-UP: the output keeps playing untouched while a marker follows the mouse, and
  the clip jumps once at the drop. Why it loses: it changes what the Clip tab does today (it already seeks live), and
  the drop point could not be seen before letting go. It is the safer choice for an audience, so it is Q2 for Boris
  (default = scrub).
- F7 A drop outside the in..out range. CHOICE: the drag is clamped to the range, so no drop can be sent elsewhere.
  RUNNER-UP: allow any position and suspend the out-point rule until the playhead re-enters. Why it loses: a second
  transport state for one gesture.

THE CHANGE
- model/Clip.h: runtime `scrubHeld` (`RelaxedBool`, not saved).
- render/ClipTransportSync.h: `pushIntent` pushes `wanted && !scrubHeld` to the player and still returns the intent
  read; `writeBack` skips the play-state compare-exchange and the OneShot stop while held (the player is paused by
  the hold, not by its own stop), keeps the playhead store.
- MainComponent: ONE function `void MainComponent::scrubClip(Clip& clip, double normalized, ScrubPhase phase)`
  (Down / Move / Up): clamps to [inPoint, outPoint - 1e-4], sets `scrubHeld` on Down, seeks the player / sequence
  (the only `seekTo` left in MainComponent.cpp, lint L1), writes the model playhead, clears `scrubHeld` on Up.
  `onCuepointJump` (:1464-1477) becomes Down + Up in one call (a cue jump holds nothing).
- ui/ClipInspector.cpp: mouseDown / mouseDrag / mouseUp call the scrub callback with the phase; a marker is grabbed
  only on its drawn tab (:1285-1314), the rest of the bar scrubs; `setClip()` and the destructor clear a hold they
  own. ui/LayerStrip.cpp + .h: new `std::function<void(Clip*, double, int phase)> onScrub`; `scrubPlayhead` calls it
  (no direct model write); `mouseUp` ends the hold; a strip that is re-pointed or destroyed mid-drag ends it.
- api (test mode only): `POST /api/debug/scrub {layer, pos, phase}` calls `scrubClip` on the layer's playing clip.

TESTS, RED first
- U10 test_clip_transport_sync: "the layer strip's old gesture (a model write with no seek) is undone by the
  write-back" -- documents C-A; then "scrub Down + Move seeks the player and the write-back keeps the position"
  [no seek].
- U11 "while scrubHeld the player is paused, `clip.playing` stays true, the write-back does not clear it; after Up
  the player plays" [no field].
- U12 "a OneShot held at its last frame is not stopped by the hold".
- U13 headless ClipInspector: "a press at x = bar left + 3 px, below the in tab, scrubs; a press on the in tab drags
  the marker" [marker grabbed]; "setClip(other) mid-drag clears the hold of the first clip".
- U14 headless LayerStrip: "a press in the transport bar calls onScrub(Down) with the clamped position; mouseUp calls
  Up; no direct write to playheadPosition" [direct write].
- U15 pure: "scrub clamps to in..out: a drop at 0.95 with out 0.8 lands below 0.8 and the next write-back does not
  send it to the in-point".
- Mutants: MU-T3a `scrubClip` skips the seek -> U10 RED; MU-T3b Up does not clear `scrubHeld` -> U11 RED; MU-T3c no
  clamp -> U15 RED; MU-T3d setClip leaves the hold -> U13 RED.
GATE: TR7, TR8 (live, test route) + Boris's hand (section 7).

### T4 A BPM-synced clip (BF15; answers 5, 6, 7 and the follow-up) -- VISUAL deliverable

TODAY (E14-E17). Two fields in beats (`beatDivision`, `videoBeats`), a 7-preset combo and a snapped slider; a video's
rate is `videoBeats / beatDivision` and never follows the tempo; a sequence follows it; no clip BPM; no automatic
length; the Speed slider is shown but inert for a BPM-synced video.

BORIS. "As far as a video clip is concerned, it is in two categories, either BPM synced throughout the whole thing, or
just playing with a speed control. If it is BPM synced, regardless of the length, it is synced to the current playing
BPM and it has bars that the user can set. We automatically set bars for a cliff, but the user can change the bars in
it by amount." / "There should be no problem playing clips that are set up for BPM and clips that are set up for speed
at the same time. Their speeds are just controlled differently." / unit: "beats" / number or buttons: "both" /
automatic length from the current BPM: "not current bpm but have a bpm and beats input so user can do it by numbers
and a x2 and /2 control to double or half easily" / "7 follow up: default and bpms for clips to 120" / "default all
bpms to 120" / "For setting a clips beats, these are done with beats and not bars."

THE MODEL
- ONE new saved field: `float Clip::clipBpm = 120` (key "clipBpm"). `beatDivision` and `videoBeats` leave the struct;
  their keys are read once, at load, for conversion, and are not written any more.
- Length L (seconds) of what plays: video = (outPoint - inPoint) x file length; sequence = (outPoint - inPoint) x
  frames / sequenceFps.
- Beats is NOT stored: beats = L x clipBpm / 60. The two boxes are two views of one number. Typing beats N sets
  clipBpm = 60 x N / L. "The automatic length" is what the Beats box shows at 120: 2 x L (not rounded).
- Rate in BPM Sync, video and sequence alike: rate = show BPM / clipBpm. Sequence fps stays `sequenceFps`; its speed
  is the rate. `clip.speed` and the composition's master speed are not applied in BPM Sync ("Their speeds are just
  controlled differently."). Timeline mode is untouched (E15, E16).
- Show BPM = `FeatureSnapshot::bpm` of the frame's snapshot; when it is 0 (unknown) the last value above 0 the
  renderer saw, 120 before any (F19; runner-up: freeze the clip -- loses: a black-out of motion in a silence).
- Following the tempo: the rate is read every drawn frame, so a Tap, a typed tempo, Link or the tracker moves the
  speed within one analysis hop (Pitfall 48). The POSITION is integrated from the rate; nothing is derived from
  beatPhase or totalBeatCount, so Pitfall 42 is not in play and a dragged playhead stays where it was dropped (T3).
  Alignment to the bar comes from the fire: Quantize starts the clip on the bar (T1 / F2), the rate keeps it there.
- x2 doubles clipBpm (twice the beats, half the speed); /2 halves it. Range of clipBpm: 20 .. 999. A button whose
  result would leave the range is disabled and does nothing (F12; runner-up: clamp -- loses: x2 then /2 would not
  return to the same number). A typed value is clamped to the range; a typed Beats value is clamped to the beats that
  20 and 999 give for this L.
- `replaceContent` gives the cell the new content's `clipBpm` (120 for a fresh drop), as it does for the two old
  fields today (E14); `clear()` -> 120.
- OLD SHOWS (no "clipBpm" key): a clip saved in Timeline mode -> 120. A clip saved in BPM Sync keeps its speed:
  video -> 120 x beatDivision / videoBeats (same speed at a show BPM of 120; 120 for the untouched 4 / 4);
  sequence -> 60 x beatDivision x sequenceFps / (frames x speed) (the same cycle length at every tempo); clamped to
  the range; 120 when frames is 0. A save writes "clipBpm" and drops the two old keys; load -> save -> load -> save
  gives equal files.

Forks
- F8 What the two boxes are. CHOICE: linked through the clip's length (above). RUNNER-UP: independent -- BPM = the
  clip's own tempo, Beats = how much of it loops (a crop). Why it loses: "regardless of the length, it is synced" and
  "so user can do it by numbers" read as two ways to state one speed; the in / out markers already crop. If Boris
  means the crop, only the Beats box's setter changes (it would move the out-point); Q4 asks him.
- F9 How it follows. CHOICE: rate from the tempo. RUNNER-UPS: (i) advance by the beat clock's delta per frame --
  loses: the snapshot moves in analysis hops, so the clip's clock would jitter by up to a hop per frame and a Resync
  would jump it; (ii) lock the position to the bar -- loses: it is exactly "where it should be playing" that Boris does
  not want after a drop.
- F10 Which length. CHOICE: the in..out range. RUNNER-UP: the whole file. Why it loses: trimming the loop would leave
  the Beats box naming a length that no longer plays.
- F11 Old shows. CHOICE: convert BPM-synced clips, 120 for the rest. RUNNER-UP: 120 for all. Why it loses: a saved
  BPM-synced sequence would change speed on open with nothing on screen to say why (BF32 forbids a note).

THE CHANGE
- model/Clip.h, Clip.cpp (field, serialization, conversion as a pure function `float clipBpmFromLegacy(...)`).
- render/Renderer.cpp syncMedia :1657-1671 and :1705-1736: one pure helper in Renderer.h beside `effectiveClipSpeed`,
  `static float bpmSyncRate(float showBpm, float clipBpm)`; a GL-thread member for the last known BPM.
- ui/ClipInspector.cpp/.h: remove the "Beats/ Cycle" combo and the "Content Beats" slider (E17) and the inert
  "Restart / Continue / Relative" combo (it offers "Continue", which Boris's "restart" rules out); in BPM Sync show two
  rows; hide the Speed row (slider, /2, x2) in BPM Sync and the two new rows in Timeline. The bar's beat lines
  (today `int(beatDivision)`) become one line per whole beat of the loop, none above 64 beats.
- A length provider for the Beats box: `std::function<double(const Clip&)> ClipInspector::mediaSeconds`, wired in
  MainComponent to the clip's open player. ASSUMED: `duration_` is written before the player is published under
  `videoPlayerMutex_`; the builder verifies it, else the read becomes an atomic. 0 = not known yet.
- api: `/api/set_clip_param` gains `transportMode`, `clipBpm`, `beats`; `/api/composition` reports per clip
  `transportMode`, `clipBpm`, `speed`, `inPoint`, `outPoint`, `loopMode`, `reverse`; test mode `GET
  /api/debug/clip_transport_ui` dumps the widgets (bounds, text, enabled, visible).

THE WIDGETS (Clip tab, Transport section, BPM Sync mode)
- Row 1: label "BPM", a number box. Row 2: label "Beats", a number box, then "/2" and "x2" (the same button pair,
  order and size as the Speed row's pair today, so the two modes line up).
- Both boxes are `ResettableSlider` with an editable text box and `setDefaultValue` (Pitfall 5): BPM default 120;
  Beats default = the beats at 120. Right-click on either resets the clip to 120. BPM shows 2 decimals with trailing
  zeros cut ("120", "127.5"); Beats shows up to 2 decimals. Scroll wheel off (as the sliders beside them).
- Length not known yet (media pending, Pitfall 53): the Beats box shows "--" and is disabled; BPM, /2, x2 work.
- Model-driven (Pitfalls 41, 59): the inspector's existing refresh compares the TEXT each box paints with the text
  the model gives now and sets the box (no notification) only on a difference, never while that box is being typed
  in. The Beats text follows an in / out drag. `PaintKey` gains the two texts and the buttons' enabled state.
- No timer repaint of the window (Pitfall 57): a box repaints itself, only on a change.
- Harmony constraint: no text announces an event. Clamping a typed value shows the clamped number, nothing else.

TESTS, RED first
- U16 pure `bpmSyncRate`: 120 / 120 = 1; 90 / 120 = 0.75; 120 / 240 = 0.5; show 0 -> the last known; first frame 120.
- U17 pure beats <-> clipBpm with L: L 7.3 s at 120 -> 14.6 beats; typing 16 -> 131.5068 BPM; x2, /2 exact inverses;
  the buttons are disabled at the limits and change nothing.
- U18 test_composition: round-trip of `clipBpm`; "an old BPM-synced video 8 / 4 loads as 60"; "an old BPM-synced
  sequence of 8 frames at 2.5 fps, 4 beats, speed 1 loads as 75"; "an old Timeline clip loads as 120"; "load, save,
  load, save: the two saves are equal and hold no beatDivision / videoBeats key".
- U19 test_clip_transport_sync-style render test with a fake player: in BPM Sync the speed pushed is the rate and
  ignores `clip.speed` and the master speed; in Timeline it is unchanged.
- U20 headless ClipInspector (visible, Pitfall 34): the two rows exist only in BPM Sync; the Speed row only in
  Timeline; typing Beats sets the model BPM; a model change (in-point moved) changes the Beats text on the next
  refresh without a repaint when nothing changed (test_clip_inspector_paint_key gains the inputs); right-click = 120;
  "--" when the provider returns 0.
- Mutants: MU-T4a rate = clipBpm / show -> U16 RED; MU-T4b beats use the whole file -> U17 (trimmed case) RED;
  MU-T4c the legacy conversion returns 120 always -> U18 RED; MU-T4d the refresh sets the box every tick -> the
  paint-key case RED; MU-T4e x2 clamps instead of refusing -> U17 RED.
GATE: TR9, TR10; the VISUAL WORK GATE (section 5.4).

### T5 Undo never changes what is live (BF31)

BORIS. "Let's not allow control Z to change anything that is live in the layer strip. It changes anything else" /
"I don't wanna see an under removed button at all. We just use control Z. The only place that we will see undo
remove, will be in the top edit menu."

"LIVE", precisely (F13). For every layer in the stack: its trigger tuple (the active clip, the previous clip while a
fade runs, the queued clip) and the transport of those clips (`playing`, playhead, direction, `restartSeq`).
Undo and Redo never write any of these, never take away the cell, column or layer that holds them, and never bring
a layer back playing. NOT live -- still undone on a playing clip or layer: its effects, its lock, bypass / solo,
layer order, names, every cell that is not playing. RUNNER-UP: also freeze the look (effects, bypass) of anything
playing. Why it loses: nearly every edit is made on what is playing; Cmd+Z would stop working exactly where he
edits. "It changes anything else". Bypass / solo is the one doubtful member: Q3.

THE COLLISION RULE (F14). When a step, undone or redone, would take away something live, the live part stays where
it is and the rest of the step is done; the step is spent either way. A deck leaves the tab row and its playing clip
keeps playing (the retired deck of the deck change, already built); a layer, a column or a cell that is live simply
stays. RUNNER-UP: refuse the whole step until the clip is replaced. Why it loses: every older step is blocked behind
it while the show runs, and with no notice allowed (BF32) Cmd+Z looks dead. Redo follows the same rule (F16;
runner-up "Redo is free": loses -- Cmd+Shift+Z would stop a clip that Cmd+Z may not).

EVERY COMMAND (E19-E21). (a) = today it can change what plays; (b) = it cannot.

| command (menu text) | today | after: Undo | after: Redo |
|---|---|---|---|
| TriggerClipCmd ("Trigger Clip", "Trigger Column") | (a) restores the tuple and `playing` | never pushed; class deleted. A fire is not a step | -- |
| ClearActiveClipCmd ("Clear Layer Clip", layer X; child of the clear-cell composite :7061) | (a) the cleared clip plays again | never pushed; class deleted. The composite keeps its SetClipCmd child | -- |
| ClearLayerClipsCmd ("Clear Layer Clips", "Clear Deck Clips") | (a) restores / clears the tuple (:337-339) | the clips return to the grid; nothing starts (no tuple write) | cells that are not live are cleared; a live cell stays |
| SetClipCmd (drop, clear, paste, per-cell edits) | (a) a value copy over a live cell replaces the clip or its `playing` | same clip id landing: the edit is applied, live transport carried (`carryLiveTransportFrom`); another clip or empty landing on a live cell: that cell is left | same |
| SwapClipsCmd ("Swap Clips", "Move Clip") | (a) the tuple names the cell, so the layer plays whatever lands there | skipped whole when either cell is live | same |
| RemoveColumnCmd ("Remove Column") | undo (b); redo (a) removes the column under a live clip | re-inserts the cells; nothing starts | skipped when any tuple names that deck's column |
| SetColumnCountCmd ("Add Column", children of drops) | (a) a shrink can cut a live column | a shrink stops above the highest live column of that deck | same |
| AddLayerCmd ("Add Layer") | (a) erases the layer, playing or not (:506-513) | the layer stays when it is live (kept: its Redo then does nothing); else erased | re-adds only if the undo erased it |
| RemoveLayerCmd ("Remove Layer") | (a) the layer returns with its old tuple (INFERRED, E21); redo erases a live layer | the layer and its clips return; the tuple is empty -- nothing plays on it until fired (F15) | skipped when the layer is live |
| InsertDeckCmd ("Load Deck", "Duplicate Deck") | no added layers: (b) retire; added layers: (a) T6h | nothing of it live: as today (deck and added layers go). Anything live -- a tuple names the deck, or an added layer plays anything: the deck is retired, ALL added layers stay (F18) | restores the retired deck if it is still there, else re-inserts it; re-adds layers only if the undo erased them |
| AddDeckCmd ("Add Deck") | (b) retire (AM-7) | unchanged | unchanged |
| RemoveDeckCmd ("Remove Deck") | (b) retire; undo moves it back with its playheads | unchanged | unchanged |
| MoveLayerCmd, ToggleLayerFlagCmd, RenameDeckCmd, ToggleClipLockCmd, EffectStackCmd | (b) nothing starts or stops | unchanged | unchanged |
| CompositeCommand | per child | per child | per child |

Forks inside the table
- F15 Undo of Remove Layer. CHOICE: back with its clips, playing nothing. RUNNER-UP: back and playing as it was.
  Why it loses: a picture appearing on the output from Cmd+Z is a change of what is live. Q5 for Boris.
- F18 Load Deck that added layers. CHOICE: two outcomes only (all gone / deck retired and every added layer kept).
  RUNNER-UP: erase the idle added layers one by one and keep the playing ones. Why it loses: erasing a layer erases
  that row in the retired deck too (rows == layers), so Redo could not bring the deck back whole.
- Where "keep it playing" meets "un-add the thing it plays on" (the layer itself): the layer stays, its Redo is
  spent. RUNNER-UP: move the clip to another layer. Loses: it changes what is live on that other layer.

THE CHANGE
- core/DeckCommands.h: three pure helpers next to `tupleNamesDeck` (:226): `bool layerIsLive(const Layer&)`,
  `bool cellIsLive(const Composition&, uint32_t deckId, int row, int column)`, `int highestLiveColumn(const
  Composition&, uint32_t deckId)` (active, pending, and previous while `crossfadeProgress < 1`). Each command asks
  them INSIDE its fence (the GL thread is held there, so the tuple cannot move under the test -- Pitfall 55).
  ClearLayerClipsCmd / LayerClipsSnapshot lose the tuple; ClearActiveClipCmd is deleted; AddLayerCmd, RemoveLayerCmd,
  InsertDeckCmd, RemoveColumnCmd, SetColumnCountCmd get the rule of their row. InsertDeckCmd erases its added layers
  by layer ID, never "the last k" (:863-864) -- a layer another step kept may sit above them.
- core/ClipCommands.h: SetClipCmd, SwapClipsCmd take a "is this cell live" hook (same pattern as the fence hook,
  headless tests pass a lambda) and use `carryLiveTransportFrom`.
- core/TriggerCommands.h: deleted. MainComponent.cpp: the pushes at :735-737, :4976-4983, :5104-5125 and the
  ClearActiveClipCmd child at :7061 go, with the `playBefore` / `playAfter` capture that fed them.
- Lint L2 (new case, tests/test_render_thread_lint.cpp): in src/core/*Commands.h `setRuntime(` occurs only as
  `setRuntime(LayerRuntimeSnapshot{})`; the strings `->playing =` and `playheadPosition =` occur nowhere; the string
  `TriggerClipCmd` occurs nowhere in src.

TESTS, RED first (tests/test_undo_commands.cpp, tests/test_show_model.cpp)
- U-C1 "a fire pushes no Undo step": the handler-level seam used by the existing trigger-undo cases now expects an
  empty history [one entry].
- U-C2 "Undo of Clear Layer Clips puts the clips back and leaves the tuple empty; Redo leaves a re-fired cell in place
  and clears the others" [tuple restored].
- U-C3 "SetClipCmd undo onto a live cell: another clip id -> the cell is left, the same Clip object keeps playing;
  the same id -> the edit lands, `playing`, playhead, `restartSeq` are the live ones" [value copy].
- U-C4 "a restored clip never restarts: `restartSeq` after an Undo of a same-id edit equals the value before".
- U-C5 "SwapClipsCmd undo with a live cell is skipped; with idle cells it swaps back".
- U-C6 "RemoveColumnCmd redo under a live clip is skipped; `SetColumnCountCmd` shrink stops above the live column".
- U-C7 "AddLayerCmd undo while the layer plays keeps the layer, Redo adds nothing; idle: erased, Redo re-adds".
- U-C8 "RemoveLayerCmd undo brings the layer back with an empty tuple; Redo of a live layer is skipped and its Undo
  is then a no-op" [old tuple].
- U-C9 T6h re-registered (T6). U-C10 "T6h-b: nothing live -> deck and added layers go, cells disposed once" (today's
  T6h body minus the fire). U-C11 "T6h-c: an added layer plays ANOTHER deck's clip -> deck erased or retired by its
  own tuple test, added layers stay". U-C12 "T6h-d: after the keep, an older RemoveLayerCmd undo and an older
  InsertDeckCmd undo still resolve their own layers (by id), rows == layers".
- U-C13 [asan] the AS6 shape with a kept layer: the Layer and Clip inspectors bound across a keep-undo (no report).
- Mutants: MU-T5a the fire still pushes -> U-C1; MU-T5b ClearLayerClipsCmd writes the tuple -> U-C2 and lint L2;
  MU-T5c no carry -> U-C3, U-C4; MU-T5d AddLayerCmd ignores the test -> U-C7; MU-T5e InsertDeckCmd erases by
  position -> U-C12; MU-T5f the previous clip of a running fade is not counted live -> a U-C3 variant mid-fade.
GATE: TR11, TR12, TR13.

### T6 Expectation changes (complete list; no other expectation changes)

| what, by name | today | new expectation | Boris |
|---|---|---|---|
| tests/test_layer_runtime.cpp SECTION "retrigger: the tuple is unchanged, the clip restarts from its in-point and keeps its play state" (:280) | `playing == false` kept | renamed "... restarts from its start and plays"; `playing == true`; `restartSeq` bumped | "restart" |
| tests/test_shared_field_types.cpp static_assert on `Clip::hasBeenTriggered` (:33-34) | field is RelaxedBool | assert removed; asserts for `restartSeq`, `scrubHeld` added | "restart" |
| tests/test_show_model.cpp dump helper (:72-89, used by "T1 every deck-switch path ...") | dumps `hasBeenTriggered` | dumps `restartSeq` instead; the T1 cases keep "byte-identical" | -- (field gone) |
| tests/test_layer_runtime_race.cpp writes of `hasBeenTriggered` (:288) and its TriggerClipCmd thread (:211) | present | the write goes; the Undo thread performs a ToggleLayerFlagCmd instead | "Let's not allow control Z to change anything that is live in the layer strip. It changes anything else" |
| tests/test_undo_commands.cpp, every case named "TriggerClipCmd: ..." (:2182, :2227, :2264, :2310, :2347, :2444, :2656, :2776 and the three at :2991-3041) and the TriggerColumnCmd composite case (:2383 block) | a fire is undone / redone / merged | deleted; replaced by U-C1 | same sentence |
| tests/test_undo_commands.cpp "ClearActiveClipCmd: X-button clear restores layer runtime on undo" (:1431), the clear-cell composite case (:1168-1192), the stale-layer sub-step (:1516), the first-execute case (:3047-3066) | the X clear is undone | deleted; the composite case keeps its SetClipCmd half and expects no tuple write | same sentence |
| tests/test_show_model.cpp "T6h THE EXCEPTION, pinned: Undo of a Load Deck that ADDED layers takes the deck and those layers back even while its clip plays ..." (:706) | 3 layers, deck erased, clip stops, 3 disposals | renamed "T6h Undo of a Load Deck that added layers while its clip plays: the deck is retired, the SAME Clip keeps playing, the added layers stay; redo restores the deck under its id": 5 layers, 1 retired deck, `playingClip(0)` the same object, 0 disposals, redo: 2 decks, 5 layers, no reconnect | same sentence |
| tests/test_show_model.cpp T7 SECTION "RemoveLayerCmd: undo restores the layer and every live and retired deck's row" (:826) | (INFERRED) the layer's tuple returns | rows as before; the tuple is empty | same sentence |
| tests/test_render_thread_lint.cpp B4f (pinned counts for DeckCommands.h, ruling-bf9b-merge AM-7 :301) | counts of today's file | re-pinned once, at the head of stage S1 | -- |
| .harmony/probe-boxes.py `k10_fresh_and_resume` (ii) "the re-fired clip resumes where it left" (:1523-1525) | t within `resumeTol` of where it left | row renamed `k10_fresh_and_restart`; (ii) "the re-fired clip restarts": t within `resumeTol` of its in-point + elapsed; "(ii) its REST playhead stays still while it is in no layer" stays; `EXPECTED_ROWS=25` unchanged | "restart" |
| contract C3 (ruling-bf9b amendment 5; `syncActivatedPlayhead`) | a new activation resumes; the model copies the player | retired: after a fire the model playhead is the start, and the player is there on its first drawn frame | "restart" |
| .harmony/probe-video.py header (:31) "A FIRST trigger plays from 0 (only a retrigger seeks to the in-point ...)" and the "prime" double trigger in `retrigger()` (:730), w10 (:1130) | needed | comment corrected; the prime stays (harmless: a re-fire restarts again); bars unchanged | "restart" |
| Boris page of the deck change, step 8.6 (ruling-bf9b-merge.md:607-608: fire "D2 C1", click 5 deck tabs, one Cmd+Z) | "the D2 C1 you fired is undone" | the D2 C1 clip keeps playing; deck clicks are still not Undo steps | same Undo sentence |
| docs: CLAUDE.md "Transport state", Pitfall 7, performance-controls.md:48 | resume / keep pause | T9 | both |

Unchanged on purpose: probe-media-open `m6_retrigger_seek` and probe-video `w3_`, `w3b_`, `w6b_` (a re-fire still
lands on the in-point; only the thread that seeks changed); `w4_deck_return_1080` (a deck switch fires nothing);
tests/test_composition.cpp "Retrigger resets playhead"; T6f, T6g, T6i, T6j; test_program_preamble's "resume" point.
ASSUMED (builder greps before S1): no probe posts `/api/debug/undo` right after a fire expecting the fire undone,
other than probe-boxes' `k9c_remove_undo` (Remove Deck: unchanged).

## 4 BUILD STAGES (T8)

ONE lane (F17). Why not two: both halves rewrite the same two handlers (:4789-5000, :5038-5133) -- the Undo half
deletes the capture / push code the transport half would otherwise have to carry; `carryLiveTransportFrom` (T5) is
what keeps T1's `restartSeq` from restarting a live clip on an Undo; and one gate list runs once. Runner-up: two
lanes merged in sequence. Loses: the second lane re-bases over the first's handler rewrite and Harmony runs the
unit, TSan, ASan and live gates twice.

One builder context per stage, in this order; each stage ends with ctest green for the suites it names.

- S0 RE-BASE NOTES (no code; first thing after bf2 has merged). Re-read on the merged main and correct every cited
  line in this plan: src/MainComponent.cpp (the two handlers, :1464, :1886, the `apiServer_` wiring block),
  src/api/ApiServer.cpp route table (:163-344) and `/api/composition` writer, src/analysis/FeatureSnapshot.h (does
  bf2 add a second tempo or change what `bpm` means for the render thread?), src/render/Renderer.cpp syncMedia,
  tests/test_render_thread_lint.cpp pinned counts. Freeze the RED arm: a copy of the pre-lane main app bundle at
  `<scratch>/red/Audio-DNA.app` (Harmony constraint: RED arm = a frozen copy of the pre-merge main app).
  Proves: the plan's file:line table for the merged tree, written into the lane report.
- S1 UNDO-LIVE (T5 + its T6 rows). Files: core/DeckCommands.h, core/ClipCommands.h, core/TriggerCommands.h
  (deleted), model/Clip.h (`carryLiveTransportFrom` without the two new fields yet), MainComponent.cpp push sites,
  tests. Proves before S2: U-C1..U-C13 RED on the stage's base and GREEN at its head; lint L2; `[undo]` and `[show]`
  suites; `.harmony/probe-asan-unit.sh`.
- S2 RESTART (T1 + T2). Files: model/Clip.h, model/Layer.h, render/ClipTransportSync.h, media/VideoPlayer.*,
  media/ImageSequence.*, render/Renderer.cpp (two calls), MainComponent.cpp handlers, tests. Proves: U1-U9, lint L1
  (with the scrub exception registered for S3), `.harmony/probe-tsan-unit.sh`, probe-boxes k10 re-registered.
- S3 DRAG (T3). Files: model/Clip.h (`scrubHeld`), render/ClipTransportSync.h, MainComponent.cpp/.h (`scrubClip`),
  ui/ClipInspector.*, ui/LayerStrip.*, api test route. Proves: U10-U15; TSan unit gate again (new shared field).
- S4a BPM ENGINE (T4 model + render + REST, no widget). Files: model/Clip.h/.cpp, render/Renderer.h/.cpp,
  api/ApiServer.cpp. Proves: U16-U19; old shows load (U18).
- S4b BPM WIDGETS (T4, visual). Files: ui/ClipInspector.*, MainComponent.cpp (provider), api dump route. Proves:
  U20; the stage ends at the VISUAL WORK GATE (5.4), not at a commit.
- S5 PROBE + DOCS. `.harmony/probe-transport.sh/.py/.json` (rows TR1-TR13), docs (section 6), the Boris page
  (section 7), the lane report. Proves: a full local run of the probe on the lane build and the RED arm table.

4.1 Collisions with the sync-dial lane (bf2: TopBar, ApiServer routes, MainComponent providers, BeatLead):
MainComponent.cpp (bf2 adds providers and route wiring near :1886 -- this lane edits the handlers and :1464, and
adds one provider in S4b: textual conflicts only if both touch the wiring block; S0 re-reads it);
api/ApiServer.cpp (both add routes and fields: additive, S0 re-reads the table and the `/api/composition` writer);
FeatureSnapshot / BeatLead (this lane READS `bpm` on the render thread only: S0 confirms the field and that the
dial does not shift it); TopBar: not touched by this lane.
4.2 What the bars lane (bf7) needs from this one: a re-fire is queued under Quantize like any fire (F2), so its new
value list applies to every fire; the release clock is still whole beats here; the per-clip legacy `beatSnap` seek
is gone; the rows "Beats/ Cycle" and "Content Beats" no longer exist, so ruling-bf7's relabels of them ("Loop
length", "Content length") are void; a clip's length is typed in beats ("For setting a clips beats, these are done
with beats and not bars.").
4.3 What the Timeline lane (bf6) needs: after a fire the model playhead IS the start (C3 retired) and a parked
clip's playhead still freezes where it left until its next fire; `Clip::restartPosition()`; `scrubHeld` (a held clip
does not advance); the length provider and `clipBpm` (beats per loop = L x clipBpm / 60) for a curve drawn in beats.
4.4 The "Duration" row: bf7's ruling AM4 removes it. If this lane lands first it leaves the row alone; if bf7
landed first, S4b lays its two rows out without it. Either order is safe; S0 records which.

## 5 HARMONY'S GATE LIST (T7; pre-registered)

Harmony constraint: RED arm = the frozen pre-lane main app; a bar whose teeth equal the noise is INFO, not a gate; a
flake verdict needs >= 5 runs per arm. Rows marked GUARD pass on both arms by design (they protect, they do not
prove the change) and are reported as INFO for the RED arm.

5.1 Unit (run by Harmony on the lane head)
- G-U1 `ctest --output-on-failure -R "layer_runtime|clip_transport_sync|show_model|undo_commands|composition|clip_inspector|layer_strip|render_thread_lint|shared_field_types"`
  -> 100% passed, 0 failed. RED arm: the same command on the stage bases recorded in the lane report shows U1-U20 and
  U-C1..U-C13 failing by name (the builder's RED log per stage; a test that never failed is struck from the list).
- G-U2 full `ctest` -> 0 failed; the count of test cases is >= the pre-lane count minus the deleted TriggerClipCmd /
  ClearActiveClipCmd cases plus the new ones (the lane report gives the three numbers).
- G-U3 mutants: each MU-* named in T1-T5 applied alone to the lane head turns the named test RED (builder's table;
  Harmony re-runs three of her choice).
- G-U4 `.harmony/probe-tsan-unit.sh` -> exit 0, with `EXPECTED_TSAN_CASES` raised from 5 to 6 (U8). RED arm for the
  new case: U8 with `restartSeq` declared as a plain `uint32_t` reports a race (builder's log).
- G-U5 `.harmony/probe-asan-unit.sh` -> last line `PROBE-ASAN-UNIT GREEN (11 cases, 0 reports)` (10 -> 11: U-C13).

5.2 Live rows -- `.harmony/probe-transport.sh` (final line `PROBE-TRANSPORT GREEN`, row-count pin `EXPECTED_ROWS=13`,
`PROBE-TRANSPORT BLOCKED <n>` if a bar had no driver). Launch rules: `--test-mode`, `open -g`, never an Output
window, no synthetic input, quit by own pid. Fixtures: probe-video's frame-coded ramp clips (decoded pixels give the
frame number; 30 fps, 10 s), in-point 0.5 unless said. "t" = decoded content time from pixels; "el" = seconds since
the fire's HTTP reply. Tolerance tol = 0.25 s (probe-boxes' `resumeTol` class; the lane report states the measured
noise from 5 runs -- a row whose teeth are not >= 4 x that noise is downgraded to INFO).
- TR1 cell_refire_restart (GUARD). Fire; 2 s; fire again; capture at el 0.3. Bar: |t - (5.0 + el)| <= tol.
- TR2 column_refire_restart. Two layers, column 0 = two ramps; `trigger_column`; 2 s; `trigger_column`; capture at
  el 0.3. Bar: both layers |t - (5.0 + el)| <= tol. RED arm: t ~ 7.3 s (fails by ~2 s).
- TR3 replaced_refire_restart. Fire A; 2 s; fire B on the same layer; 1 s; fire A; capture at el 0.3.
  Bar: |t - (5.0 + el)| <= tol. RED arm: t ~ 7 s (resumes).
- TR4 first_fire_at_start. A never-fired clip, in-point 0.5; fire; capture at el 0.3. Bar: frame code in [150, 170].
  RED arm: code < 30.
- TR5 ended_oneshot_refire_plays. OneShot ramp, out-point 0.7; fire; wait until `/api/composition` shows `playing`
  false; fire; at el 0.5: `playing` true and two playhead reads 0.2 s apart differ by >= 0.01. RED arm: `playing`
  false (INFERRED; if the RED arm passes, the row becomes GUARD).
- TR6 quantized_refire. Composition with `quantizeMode` = next downbeat, a locked tempo (probe-boxes k5's driver);
  fire; after a downbeat fire again mid-bar; read the playhead every 50 ms. Bar: no drop to the in-point before the
  next downbeat; a drop within 100 ms after it. RED arm: the drop comes within 100 ms of the fire.
- TR7 scrub_drop_plays_on (GUARD for the RED arm: the route is new). `/api/debug/scrub` Down 0.6, Move 0.7, Up 0.7
  on a playing ramp; capture 0.5 s after Up. Bar: |t - (7.0 + 0.5)| <= tol and the REST playhead never below 0.69
  after Up. Its teeth: mutant MU-T3a on the lane build fails it (builder's log).
- TR8 scrub_hold (GUARD, same reason). Down 0.4; four playhead reads over 1 s; Up. Bar: all four within one frame of
  0.4, `playing` true throughout, and the read 0.5 s after Up is > 0.42. Teeth: MU-T3b.
- TR9 bpm_rate_follows_tempo. Composition (old keys: `transportMode` 1, 4 / 4, so both arms load it); `set_bpm 120`;
  fire; rate r1 = content seconds per second over 2 s; `set_bpm 90`; 0.5 s; rate r2 over 2 s.
  Bar: r1 in [0.97, 1.03], r2 in [0.72, 0.78]. RED arm: r2 ~ 1.0 (teeth 25% against 3%).
- TR10 old_show_keeps_speed. Old BPM-synced sequence (8 frames, 2.5 fps, 4 beats), `set_bpm 120`.
  Bar: `/api/composition` `clipBpm` == 75 (lane arm only) and one cycle = 2.0 s +/- 5% on BOTH arms (GUARD half).
- TR11 undo_leaves_the_fire. Fire a ramp; 1 s; `POST /api/debug/undo`; 0.5 s. Bar: the layer's `activeClip` is
  unchanged, `playing` true, t advanced by >= 0.3 s. RED arm: the layer is empty after the undo.
- TR12 undo_load_deck_keeps_playing. 3-layer show; `/api/debug/load_deck` a 5-row deck; fire its row-0 clip (a
  ramp); 1 s; undo. Bar: 5 layers, the deck absent from `decks`, decoded pixels still the ramp and advancing; redo:
  the deck is back under its id, the ramp never restarted (t monotonic). RED arm: 3 layers, the ramp gone.
- TR13 undo_edit_never_restarts. A playing ramp; drop a file into ANOTHER cell (`/api/debug/drop_files`); undo; redo.
  Bar: t monotonic across both, no step > tol. (GUARD.)
- Also re-run, unchanged scripts: `.harmony/probe-boxes.sh` -> `PROBE-BOXES GREEN` with k10 re-registered (T6);
  `.harmony/probe-video.sh` rows w3_retrigger_midgop_1080, w6b_retrigger_mid_fade, w4_deck_return_1080 and
  `.harmony/probe-media-open.sh` row m6_retrigger_seek -> their own bars (the seek moved to the render thread).
- Flake rule: any row that fails once is re-run 5 times on each arm before a verdict.

5.3 Idle paint (INFO unless it regresses): `.harmony/probe-idle-paint.sh` with the Clip tab open on a BPM-synced
playing clip -> its existing bars; the two boxes add no repaint while their text is unchanged (U20 is the gate).

5.4 VISUAL WORK GATE for T4's widgets (stage S4b). Captures by the app's own capture route, 1728 x 1117 and
1280 x 720, light checks read from `GET /api/debug/clip_transport_ui` (the dump), never from pixels alone.
States (PNG + dump each): V1 Timeline mode (Speed row, no BPM rows); V2 BPM Sync, default (BPM "120", Beats = 2 x
L); V3 after x2 (BPM "240"); V4 at the upper limit (x2 disabled); V5 at the lower limit (/2 disabled); V6 length not
known ("--", Beats disabled); V7 a typed BPM "127.5"; V8 trimmed in / out (Beats smaller, BPM the same); V9 the same
clip in the narrowest inspector width the window allows; V10 an image clip (no transport rows at all).
Measurable bars (from the dump): B1 no row is clipped: every widget's bounds lie inside the inspector at both
window sizes; B2 the /2 and x2 buttons have the same bounds as the Speed row's pair in V1 (+/- 1 px); B3 label text
is exactly "BPM" and "Beats", button text exactly "/2" and "x2"; B4 V2 Beats text == 2 x L to 2 decimals for the
fixture's known L; B5 V4 / V5: the named button `enabled == false`, the other true; B6 no dump field of a hidden row
is `visible == true`; B7 no label, tooltip or painted string in the Transport section changes between two dumps taken
before and after x2 other than the two numbers (Harmony constraint: no text announces an event).
Five critic seats, each given the PNGs, the dump, this section, BORIS_DECISIONS.md "Inspector Grammar" and "Rejected":
- visual-design: do the two rows read as one group with the mode combo? Is anything misaligned against the rows
  above and below? Is the disabled Beats box ("--") distinguishable from an enabled one without colour alone?
- UX: can a first-time user tell which box to type in to make the clip twice as slow? Is it clear that the two
  numbers move together? Is there any state where a control looks usable and does nothing?
- graphic-design: type sizes, number formatting ("120" vs "120.00"), spacing of "/2" "x2" against the box; does it
  match the Speed row it replaces?
- logic: for V2-V8, is every pair (BPM, Beats) consistent with L in the dump (Beats = L x BPM / 60 within 0.01)?
  Does any state show a number the model does not hold?
- interaction-logic: type in Beats -> BPM changes; x2 then /2 returns to the same text; right-click on either box
  -> "120"; Tab moves BPM -> Beats; a drag of the out marker changes Beats while BPM stays; nothing steals the
  keyboard focus from a box being typed in when the refresh runs.
Pass = all bars B1-B7 and no seat raising a MUST; a MUST is fixed and the whole gate re-run.

## 6 DOCS (T9)

- CLAUDE.md "Transport state" paragraph is false after S2. Replacement (two lines, the detail lives in docs):
  "Transport state: every fire restarts the clip from its start and plays it (the layer's activation tail bumps
  `Clip::restartSeq`; the render thread carries it out). `Clip::playing` is `mutable` (the render thread writes it for
  a OneShot stop). Undo never changes what plays. Rules: `docs/claude/performance-controls.md` 'Firing' and 'Undo'."
  Pitfall index line 7 becomes "Every fire restarts and plays -- before touching `applyActivationTail`, a trigger
  handler or a player seek." (CLAUDE.md is being cut by another task: only these two edits are asked of it.)
- docs/claude/pitfalls.md: entry 7 rewritten (the `hasBeenTriggered` rule is gone; never seek a player from a trigger
  handler; GL-thread fires exist). Entry 2 gains: the restart is applied before the intent is pushed; a held scrub
  pauses the player without touching the intent. New "Pitfall NN: Undo / Redo never write a tuple or a transport
  field; a command that copies a Clip over a cell carries the live transport; a step never removes a live cell,
  column or layer (lint L2)". New "Pitfall NN: a BPM-synced clip is a RATE (show BPM / clip BPM), never a position;
  Beats is derived from the length -- before adding a beat-clock read to clip transport". New "Pitfall NN: one scrub
  function owns `seekTo` on the message thread; a hold must end on mouseUp, setClip and destruction".
- docs/claude/performance-controls.md: :45 "Firing" (every fire restarts; a re-fire queues under Quantize; the
  per-path table of T1); :48 "A clip that leaves its layer ... Fired again it RESUMES ... a never-played clip starts
  at its in-point" -> "Fired again it restarts from its start"; :65 guards list (new tests, probe-transport); :81 the
  Undo paragraph gains BF31's rule, the definition of live and the collision rule; the AM-7 exception sentence goes.
- docs/claude/architecture.md: :117 the clip runtime field list (`hasBeenTriggered` -> `restartSeq`, `scrubHeld`);
  the `beatDivision` / `videoBeats` rows (~:91-92 per the fact sheet, not re-read) -> `clipBpm`. Finding, not this
  lane's: :265 calls UndoManager "DEAD: perform() never called", which is false at 34179a2 (MainComponent.cpp:5330).
- docs/claude/rendering.md: the syncMedia paragraph gains the restart step and the BPM-sync rate.
- docs/claude/recording.md: where a routine or a take fires a clip: a replayed fire restarts the clip; the
  checkpoint-0 restore restarts a clip a layer already plays; a replayed fire is never an Undo step (already true).
- docs/claude/integration.md + testing-eyes.md: the new `/api/set_clip_param` keys, `/api/composition` fields, the
  two test routes. `.harmony/APP-INVENTORY.md`: the Clip tab's BPM rows; the removed combos.
- BORIS_DECISIONS.md "Playback Behaviour": three entries with his words (restart; the drag; Undo and what is live).

## 7 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)

1. Fire a video, let it run, fire another clip on the same layer, fire the first again. -> It starts from its
   beginning. Wrong: it carries on from where it was.
2. Fire a column, wait, fire the same column again. -> Every video in it starts over together. Wrong: some keep
   running.
3. Turn Quantize on and fire the playing column again in the middle of a bar. -> Nothing moves until the bar, then
   they all start over on it. Wrong: they jump at once, or never.
4. Pause a clip, then fire it. -> It plays from its beginning. Wrong: it sits on its first frame.
5. Grab the playhead in the Clip tab and in the layer strip, drag, hold still, let go. -> The picture follows your
   hand, waits while you hold, and plays on from where you let go at the same speed. Wrong: it springs back, or runs
   away under your hand.
6. Do the same on a reversed clip and a ping-pong clip. -> It carries on in the direction it had. Wrong: it turns
   round.
7. Set a clip to BPM Sync. -> BPM shows 120 and Beats shows the clip's length in beats at 120. Press x2: the clip
   plays at half speed and both numbers double; /2 brings it back. Tap a faster tempo: the clip speeds up with it.
   Wrong: the speed does not follow your taps, or the numbers disagree with what you see.
8. Type a number into Beats (for example 16). -> BPM changes to match and the clip now loops in exactly 16 beats.
9. Play a BPM-synced clip and a speed clip side by side and change the tempo. -> Only the BPM-synced one changes.
10. Open one of your saved shows that has BPM-synced clips. -> They move as they did before.
11. Fire a clip, then press Cmd+Z. -> The clip keeps playing; Cmd+Z undid your last edit instead (or nothing).
    Wrong: the clip stops or the old one comes back.
12. Load a deck that adds layers, fire one of its clips, press Cmd+Z. -> The deck leaves the tab row, the clip keeps
    playing, the added layers stay. Wrong: the picture goes away.
13. Remove a layer by mistake, press Cmd+Z. -> The layer and its clips are back, not playing until you fire one.

## 8 QUESTIONS FOR BORIS (each has a default; nothing waits)

- Q1 You paused a clip and then fire it. Should it play from its beginning (default), or restart and stay paused?
- Q2 While you drag the playhead, should the output show the frames you drag over (default), or keep playing
  untouched and jump once when you let go?
- Q3 Bypass and solo on a layer: should Cmd+Z still undo them (default), or are they "live" like a fired clip and
  left alone?
- Q4 BPM and Beats: we made them two ways of saying one thing -- change one and the other follows, and the whole
  clip is stretched to fit (default). Or should Beats cut the loop shorter while BPM stays?
- Q5 You remove a layer by mistake and press Cmd+Z: it comes back with its clips but not playing (default). Or
  should it come back playing what it was playing?
- Q6 A reversed clip that is fired starts from its end (default). Right?

## 9 RISKS (strongest counterargument first; cheapest refuting test for each choice)

- R1 THE STRONGEST: the keep-live rule (F14) leaves half-undone steps -- a deck gone from the tabs with its layers
  still there, a cell that an Undo skipped and no later Undo will ever remove -- and it does so silently. A refused
  step would be simpler to reason about and cannot corrupt the stack. Why the plan still holds: every command
  restores by value and by id, a skipped part changes nothing (the model after the skip equals the model before it),
  and the retired deck -- the same idea -- is already built and gated. Cheapest refutation: U-C12 (older steps undone
  after a keep, rows == layers, ids resolve) plus a 200-step random Undo / Redo walk with a fire between steps
  asserting `rowsEqualLayers` and that no tuple changed (add it to U-C12 if the council asks). If it goes RED the
  lane falls back to "refuse the step" for layers and columns and keeps the rule for cells and decks.
- R2 F1 (every fire plays) reverses a documented rule (Pitfall 7, CLAUDE.md "Transport state") that someone once
  asked for. Refute: Q1; one line in the tail restores the old behaviour for non-OneShot clips.
- R3 `restartSeq` and value copies: any path that copies a Clip over a live one with a different counter restarts
  it. Covered for commands (U-C3, U-C4). NOT read: composition load, take restore (`PerfState`), Duplicate Deck.
  Cheapest test: TR13, plus a unit case that loads a take state over a playing clip and checks the counter the player
  saw. If a path cannot carry the counter, the consume side can ignore a counter that went DOWN.
- R4 The quantized re-fire (F2) puts the ACTIVE ref in the pending slot: any reader that assumes pending != active
  (the pad's queued look, `releaseMomentary` -- it tests active first, Layer.h:515) is affected. Cheapest test: U3 plus
  a headless ClipCell case "active and pending at once paints the queued mark".
- R5 Rate, not phase (F9): over minutes a BPM-synced loop can drift against the beat (render clock vs audio clock,
  tracker phase corrections). Boris may expect it to stay locked without re-firing. Cheapest test: TR9 extended to
  5 minutes at a fixed tempo, drift read from pixels; if it exceeds a quarter beat, runner-up (i) with a smoothed
  beat clock is the fix -- and T3 must then say what a drop does.
- R6 F8 may be the wrong reading of "a bpm and beats input". Cheapest test: Q4 and step 8 of section 7; the other
  reading changes one setter and no saved field.
- R7 The scrub hold can stick (a strip rebuilt mid-drag, a lost mouseUp) and freeze a live clip. Cheapest test:
  U13, U14; live: TR8's last read. Fallback if it still worries the council: the render thread drops a hold that saw
  no scrub call for 10 s.
- R8 Reverse inside a trimmed range is not enforced today (the out-point rule is forward-only,
  ClipTransportSync.h:56); F4's "just inside the out-point" works around it, it does not fix it. Not this lane.
- R9 `VideoPlayer::duration_` read from the message thread is ASSUMED safe (T4). Cheapest test: the TSan unit case
  for the provider; else make it atomic.
- R10 The RED arm cannot run TR7 / TR8 (new route) -- they are GUARDs with mutant teeth, which is weaker than a
  frozen-app RED. Only Boris's hand (section 7, steps 5-6) proves BF25 against the old app.
- R11 Old-show conversion (F11) rests on E15 / E16 as read; a show saved with `speed` != 1 on a BPM-synced VIDEO was
  ignoring it and still is. Cheapest test: TR10 on one of Boris's own saved shows (file needed from him).
- R12 Line numbers move when bf2 merges; S0 exists for that. A stage that starts without S0's table builds on stale
  cites.

STATUS: DONE

## HARMONY ADOPTION (s-rta-1003b, 2026-10-03 22:17:01) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1003b/ruling-transport.md IN FULL (STATUS DONE after the completion round and the fix
   round on its completeness check: 45 of 45 attacks ruled, 30 amendments AM-1..AM-30; papers whole in
   attack-transport-papers.md / attack-transport-papers-full.json). A builder builds from the plan body + the ruling, the
   ruling overriding; the ruling's section 10 is the reading map. Stages: S0, S1, S2, (S2b only if FM-1 is over its bar),
   S3, (S3h only if Boris answers Q2 "wait while I hold"), S4a, S4b (VISUAL WORK GATE), S5. Section 5 is the ONLY source
   of gate strings; sections 6-7 are the Boris page.
2. HARMONY'S DECISIONS H-1..H-4 (the ruling's section 4): all four take the ruling's default.
   H-1 the bars lane's two ClipInspector relabels are withdrawn; S4b deletes the two widgets by member name.
   H-2 S4b removes the inert Duration row if it is still in the tree; the bars lane's item I13 is struck.
   H-3 S1 does NOT start before the sync-dial lane (bf2) has merged. Reason (Harmony): one build lane at a time on this
       machine -- the sync dial's quiet-machine rows cannot run beside a compiling lane, and parallel opus build lanes
       ended s-rta-1002b on a usage stop. Revisit when bf2 reaches its visual gate (no bf2 builder compiling).
   H-4 the inert "Restart / Continue / Relative" combo is removed in S4b; note N2 tells Boris.
   H-5 (AM-27's open line) "--" in a disabled Beats box for a length that is not known is a STATE display and stays --
       the app already shows "---" for a tempo it does not know (facts-notices.md table C, kept). It is not a failure text.
3. BORIS QUESTIONS Q1..Q10: each keeps the ruling's default until he answers. They went to him on one page on 2026-10-03 22:17:01:
   .harmony/.reports/s-rta-1003b/boris-questions.html (with notes N1, N2). An answer that differs from a default is filed
   verbatim (binding-decisions.md) and overrides the ruling at the named amendment; S3h is built only on his Q2 answer.
4. Boris, verbatim, binding for every stage: "Let's not allow control Z to change anything that is live in the layer
   strip. It changes anything else" (BF31); "restart" (clarifying answer 2); "default all bpms to 120"; "We don't need any
   text indicating what has happened or what has happened. That is something that happens online and is not necessary in
   this application. It is extra overhead and bloat. Please remove it cleanly and completely."; "ok. the only fail message
   will be a failed save. remove all others". This lane adds no on-screen text that announces an event or a failure.
5. FACTS FM-1..FM-4 are Harmony's to measure on the running app (FM-2 on the frozen pre-lane app before S2).
6. PITFALLS: the lane writes "Pitfall NN"; Harmony assigns numbers at the merge (68 is the sync dial's). Docs go to
   docs/claude/*.md; CLAUDE.md's "Transport state" paragraph is replaced by a pointer (CLAUDE.md is being cut to <= 8 KB).
7. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP -- the lock helper, own-pid quits only, no Output window, no
   synthetic input. S4b is a visual deliverable: five critic seats (visual-design, UX, graphic-design, logic,
   interaction-logic) on decoded captures before Boris sees it. MERGE by Harmony.
