# facts-transport -- clip TRANSPORT (read-only fact sheet, s-rta-1003)

Source read: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon (commit a7491d4, lane/bf9b). Every `file:line` below is relative to that root unless it starts with `/Users/...`.
Labels: VERIFIED = I read the code at the cited line. INFERRED = reasoned from code I read, not run, not followed end to end. UNKNOWN-NEEDS-A-RUN = named with the cheapest discriminating test.
Nothing was built, run, launched or edited. Boris is quoted only from the task text.

Boris (verbatim, from the task): "Firing a column that is already playing should restart its videos" / "When I grab the play head and move the timeline, I do not want it to jump back to where it was or where it should be playing before I grabbed it. I wanted to keep playing at the same speed, but play from wherever I drop the play head." / "As far as a video clip is concerned, it is in two categories, either BPM synced throughout the whole thing, or just playing with a speed control. If it is BPM synced, regardless of the length, it is synced to the current playing BPM and it has bars that the user can set. We automatically set bars for a cliff, but the user can change the bars in it by amount." / "Regardless of the clip being BPM or speed controlled, if we are in Qantize mode, it is triggered on time by the Qantize method."

Two doc/code contradictions up front (both VERIFIED, both change a plan):
- Pitfall 7 says `hasBeenTriggered` distinguishes first activation from a return. It is set ONLY by the cell path (`MainComponent.cpp:4763`, in handleClipTrigger). handleColumnTrigger never sets it (grep of `src/`: the only writer is :4763). See Q1.
- docs/claude/performance-controls.md line 48 says "a never-played clip starts at its in-point". The code seeks the player to the in-point only on a RETRIGGER (`MainComponent.cpp:4787-4811`) or a beat-snapped clip (:4765-4786); a first activation does not seek (see Q2). The probe comment `.harmony/probe-video.py:28` says the same as the code: "A FIRST trigger plays from 0 (only a retrigger seeks to the in-point ...)". probe-boxes k10 (i) cannot tell the two apart (its ramp clip has inPoint 0).

---------------------------------------------------------------------------------------------------
## 1  QUESTIONS ANSWERED

### Q1  Cell trigger vs column trigger of a clip that is already its layer's playing clip

Answer: a CELL re-fire restarts the video (the message thread seeks the player to the in-point). A COLUMN re-fire does not: the layer's tuple is left alone and only the MODEL playhead is reset, and the render thread overwrites that from the still-running player on the next frame, so the video never restarts.

Cell path (VERIFIED):
- ClipCell click -> `deckView_->onClipTriggered` -> `handleClipTrigger(layer, col)` (`MainComponent.cpp:697-699`, `ui/DeckView.cpp:197-198`).
- `Composition::fire` -> `Layer::triggerClip(ref, rows, forcedSnap)` (`MainComponent.cpp:4747`, `model/Composition.h:422-435`).
- `Layer::triggerClip` (`model/Layer.h:410-433`): `snapEnabled` is true if a forced snap (global Quantize) or the clip's own beat snap is on, BUT the queue branch is `if (snapEnabled && ref != r.activeRef())` (:424). A re-fire (ref == active ref) therefore NEVER queues, even with Quantize on: it goes to `immediateNext` (:361-374), which returns the tuple unchanged (`if (ref == r.activeRef()) return r;` after clearing the pending slot). No CAS happens.
- `activate()` (:564-580): "a retrigger with nothing queued changes no tuple field (no CAS), so its tail runs once after" -> `applyActivationTail` (:548-557): model `playheadPosition = inPoint`, `beatsPlayed = 0`. `playing` is set true only if `to.activeRef() != from.activeRef() && !hasBeenTriggered` -- a retrigger never changes `playing`.
- Back in handleClipTrigger: `wasRetrigger = (t.before.activeRef() == ref)` (:4755); `hasBeenTriggered = true` (:4763); then the branch chain (:4765 / :4787 / :4812):
  1. `clip->beatSnap && playable` (legacy bool, true whenever the clip's Snap combo is not "Snap Off", ClipInspector.cpp:225-230 area `beatSnapSelector_.onChange`): model `playheadPosition = snap.beatPhase`, `player->seekTo(beatPhase)` / `seq->seekTo(beatPhase)` (:4768-4783). It takes precedence over the retrigger branch. INFERRED oddity: beatPhase (0..1 inside one beat) is used as a normalized FILE position. Also `clip` here is `composition_.clipAt(t.after.activeRef())` (:4758), which for a QUEUED trigger is the OLD active clip.
  2. `else if (wasRetrigger && playable)`: `player->seekTo(clip->inPoint)` / `seq->seekTo(clip->inPoint)` (:4787-4811). THIS is what restarts a video on a cell re-fire. `playing` untouched (a paused clip stays paused).
  3. `else if (t.after.activeRef() == ref && playable)`: `syncActivatedPlayhead(*clip)` (:4812-4817) = contract C3 (Q2).
- No undo entry for a retrigger (:4865-4880: pushed only when the tuple or target-`playing` changed).
- Capture: the activeClip point carries `p.retrigger = wasRetrigger` (:4892) (cell path only).

Column path (VERIFIED):
- `deckView_->onColumnTriggered` / REST `trigger_column` (`MainComponent.cpp:699`, :1878) / bindings (:7654) -> `handleColumnTrigger(column)` (:4935).
- One forced-snap decision for the whole column (:4975), then `composition_.triggerColumn(deck, column, forcedSnap, &transitions)` (:4977; `model/Composition.h:440-457`) = `layer.triggerClip(ref, rowClips(i), forcedSnap)` on every layer without `ignoreColumnTrigger` (`Layer::triggerClip` as above). A layer already playing that ref: no CAS, model playhead reset by the tail, nothing else.
- Per-layer loop (:4985-5017): `syncActivatedPlayhead` only for `t.after.activeRef() == ref && !(t.before.activeRef() == ref)` (:4996-4999) -- a NEW activation. A re-fire is neither. **There is no `seekTo` anywhere in handleColumnTrigger** (grep `seekTo(` across src: only MainComponent.cpp:1472/1477 (cue jump), :4778/4783 (beat snap), :4804/4809 (cell retrigger), ClipTransportSync.h:69 (out-point)).
- Why the model reset is invisible: `Renderer::syncMedia` -> `ClipTransportSync::writeBack` stores `player.getPlayheadPosition()` into `clip.playheadPosition` every frame the clip is drawn (`render/ClipTransportSync.h:45-48`; called `Renderer.cpp:1676`, :1753). The comment at `MainComponent.cpp:4789-4795` says exactly this ("the renderer overwrites clip->playheadPosition FROM the player's actual position every frame"). The cell path was fixed for it in 2026-07-30; the column path never got the fix (the column path was written/extended later; INFERRED history).
- Column re-fire with Quantize on: also immediate (same `ref != r.activeRef()` rule), so a re-fire is never "triggered on time by the Qantize method" today.
- `hasBeenTriggered` is NEVER set by a column fire (VERIFIED, only writer is :4763). Consequence (INFERRED from Layer.h:555 + :4763): a clip that has only ever been column-fired still counts as "never triggered", so EVERY new activation of it by a column fire runs the first-activation rule and sets `playing = true` -- a clip the user paused comes back playing after the next column fire; the cell path "returning clips keep their state" does not hold for column fires. `triggerWillAutoPlay` (:196) reads the same flag for the capture of the auto-play.
- Mixed row on a column fire (VERIFIED, docs + code): layers whose cell in the column is empty are cleared (F12; `Layer::triggerClip` with `target == nullptr` -> `clearActiveClip`); layers playing another deck's/column's clip activate the column's clip (resume, C3); layers already on the ref: the no-op above. So "restart its videos" must define the per-layer behaviour of all three cases.

Pitfall 7 / Pitfall 2 mapping (VERIFIED): Pitfall 7 = the `hasBeenTriggered` rule above (`Layer.h:555`); Pitfall 2 = `ClipTransportSync` (`playing` is the message thread's intent, the GL thread writes back the player's state with a CAS; a OneShot that stopped itself clears the intent; `ClipTransportSync.h:1-72`).

### Q2  A video that played, was replaced, and is fired again: resume or restart? Where decided (C3)

Answer: it RESUMES from where it stopped. Decided in `MainComponent::syncActivatedPlayhead` (`MainComponent.cpp:4903-4917`), called from handleClipTrigger (:4812-4817) and handleColumnTrigger (:4996-4999) for a newly activated, non-queued playable clip. The decision is really "do nothing to the player": the player's clock froze when the clip left the layer; C3 only copies the player position into the model.

- VERIFIED: a clip that is neither a layer's active nor previous ref is not drawn, so `syncMedia` is not called for it and its player clock does not advance (docs/claude/performance-controls.md line 48: "its playhead freezes, its decode thread parks and its ring trims after 1 s; its playing flag is left as it was. Fired again it RESUMES"; code: the only call sites of the video frame provider are in the compositor's draw of the layer's active clip and the crossfade chain, `render/CompositorEngine.cpp:1115-1119`, :1222-1226, :1447; a hidden/bypassed layer is skipped before the clip is read, :1068-1071). A layer that is `visible=false`, `bypassed`, or not soloed while another is, also stops its clip clock the same way (INFERRED from :1068-1071; not run).
- VERIFIED: before C3 runs, `applyActivationTail` has already set the MODEL playhead to the in-point (Layer.h:553) and C3 then overwrites it with `player->getPlayheadPosition()`. So the in-point value never survives a resume. The player itself is not seeked on a first/new activation (no `seekTo` in that branch).
- VERIFIED: the `playing` flag of a replaced clip is left as it was (only an explicit CLEAR runs `applyClearTail`, `Layer.h:~583-590`, which sets the old active clip `playing = false`).
- VERIFIED: a never-played clip with `inPoint > 0` therefore starts at the player's position 0, not at its in-point, on first fire (the player is opened at `currentTime_ = 0`, `media/VideoPlayer.cpp:243-248`; nothing seeks it). The only "restart at the in-point" gestures: cell retrigger (:4787), the out-point wrap (`ClipTransportSync.h:60-70`, `seekTo(inPoint)`), the Loop wrap does NOT go to the in-point when `outPoint == 1` (VideoPlayer::advanceTransport wraps at `duration_` with fmod -> 0, `media/VideoPlayer.cpp:434-438`); INFERRED consequence: a Loop clip with `inPoint > 0` and `outPoint == 1` loops back to 0, not to its in-point.
- Pause-state: because `hasBeenTriggered` is set (cell path) the returning clip keeps its play/pause state; via the column path it is auto-played (Q1).
- Image sequences: same (`ImageSequence::advanceFrame` is called only from syncMedia, `Renderer.cpp:1700-1740`).
- "Restart" selector: ClipInspector has a `triggerDropdown_` (Restart / Continue / Relative, `ui/ClipInspector.cpp:69-73`) with NO onChange and no model field -- VERIFIED inert (grep `triggerDropdown_` = declaration + addItem + setBounds only).
- Doc claim to check: performance-controls.md line 48 ends "a never-played clip starts at its in-point" -- not what the code does (see header).

### Q3  The playhead drag

Components that handle a drag (VERIFIED):
1. Clip tab timeline bar: `ClipInspector::mouseDown/mouseDrag/mouseUp` (`ui/ClipInspector.cpp:1223-1283`), painted by `paintTimeline` (:1106-1215).
2. Layer strip transport bar: `LayerStrip::mouseDown` (:984-989) / `mouseDrag` (:998-1003) -> `scrubPlayhead` (`ui/LayerStrip.cpp:1068-1078`).
3. Cuepoint trigger buttons (`ClipInspector.cpp:~300-308`) and the Set button (reads the playhead, :322) behave like a click on (1).

What each writes:
- (1) Clip tab. mouseDown (:1223-1250): the hit test checks the in marker first (`abs(mx - inX) < 8`), then the out marker, otherwise Playhead. So grabbing the playhead within 8 px of the in or out marker (the default state: playhead 0 == inPoint 0, or outPoint 1) grabs the MARKER and drags the in/out point instead (VERIFIED :1236-1241). A click anywhere else is "click = scrub": the playhead is not hit-tested, any press in the bar moves it to that x. During a Playhead drag (mouseDown :1247-1249, mouseDrag :1271-1273) it writes `clip_->playheadPosition = norm` (the model) and calls `onCuepointJump(clip_, norm)`, wired at `MainComponent.cpp:1466-1479` to `renderer.getVideoPlayer(clip->id)->seekTo(pos)` or `getImageSequence(id)->seekTo(pos)`. `seekTo` is a request (`media/VideoPlayer.cpp:366-371`, `media/ImageSequence.cpp:105-112`); `advanceFrame` applies it on the GL thread (`VideoPlayer.cpp:380-387`: `currentTime_ = target * duration_`, playhead stored, generation bump). mouseUp (:1281) only clears `currentDrag_`: nothing is written on release; the player keeps its `playing`/speed/reverse, so playback continues from the drop point at the same speed. No undo, no pause.
  Value domain: normalized [0,1] of the WHOLE file (not of the in..out range). The in/out drags do not seek the player.
- (2) Layer strip. `scrubPlayhead` writes ONLY `clip->playheadPosition = normalized` for `playingClip()` (:1068-1078). There is no seek callback on LayerStrip (all its `std::function` members listed `LayerStrip.h:76-137`; none seeks). VERIFIED.

Every writer of the clip's model `playheadPosition` (VERIFIED unless marked), by transport mode:
| writer | where | Timeline (speed) | BPM Sync |
|---|---|---|---|
| render write-back, every frame the clip is drawn: model <- player position | `render/ClipTransportSync.h:48` via `Renderer.cpp:1676` (video), :1753 (sequence) | yes | yes |
| player's own clock: `currentTime_ += dt * speed * dir`, wraps/ping-pong/OneShot | `media/VideoPlayer.cpp:388-480` (video), `media/ImageSequence.cpp:154-200` | speed = `clip.speed * masterSpeed` (`Renderer.cpp:1670`, `effectiveClipSpeed` `Renderer.h:290`) | video: speed = `videoBeats / beatDivision`, only when `snap.bpm > 0` (`Renderer.cpp:1657-1666`); sequence: fps recomputed from bpm, speed = `clip.speed` (:1705, :1718-1735) |
| out-point enforcement: `ph >= outPoint` -> `seekTo(inPoint)` + model <- inPoint (Loop/PingPong) or stop (OneShot) | `ClipTransportSync.h:60-71` | yes | yes |
| activation tail: model <- inPoint, beatsPlayed <- 0 (any thread, before the CAS; also on a no-op retrigger) | `model/Layer.h:548-557` | yes | yes |
| C3: model <- player position on a new activation | `MainComponent.cpp:4903-4917` | yes | yes |
| cell retrigger: `player->seekTo(inPoint)` | `MainComponent.cpp:4787-4811` | yes | yes |
| beat-snapped clip trigger: model <- beatPhase, `seekTo(beatPhase)` | `MainComponent.cpp:4765-4786` | yes | yes |
| Clip tab drag / click / cuepoint jump: model <- norm + `seekTo(norm)` | `ClipInspector.cpp:1247-1249, 1271-1273, 305-307` -> `MainComponent.cpp:1466-1479` | yes | yes |
| Layer strip scrub: model <- norm, NO seek | `LayerStrip.cpp:1077` | yes | yes |
| applyClipPlaying "stop": `playing=false`, model <- inPoint, NO seek | `MainComponent.cpp:6455` | yes | yes (the next write-back restores the old player position if the clip is drawn; INFERRED) |
| replaceContent / clear: model <- 0 | `model/Clip.h:331, 373` | n/a | n/a |
No writer derives the position from the BEAT CLOCK or beat phase except the beat-snap trigger above; neither mode phase-locks the playhead to the beat grid (Q4).

Candidates for "jump back" (Boris: "I do not want it to jump back to where it was or where it should be playing before I grabbed it"):
- C-A (VERIFIED code path; INFERRED that this is what he hit): grabbing the LAYER STRIP bar. `scrubPlayhead` moves only the model; the player never moves; the very next `writeBack` stores the player's real position back into the model. The drawn playhead jumps back (mouse-drag at the UI rate, overwritten at 60 Hz), the picture never changes. This is exactly "jump back to where it was". Needs no decode-side explanation.
- C-B (VERIFIED code; INFERRED effect): Clip-tab bar on a clip that is not the layer's playing clip. It seeks the PARKED player of the selected cell (the ruling-bf6 AM-2 decision, a parked clip's knob follows its parked playhead, /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-1002b/ruling-bf6.md:24-29); what plays on the layer is a different clip and is untouched.
- C-C (VERIFIED code; INFERRED effect): grabbing near the in/out marker drags the marker, not the playhead (:1236-1241) -- the playhead never moves. With the default in 0 / out 1 this is the case at both ends of the bar.
- C-D (VERIFIED code): dropping the playhead at or beyond `outPoint` (when `outPoint < 1`) -> next frame `seekTo(inPoint)` (`ClipTransportSync.h:60-70`): a snap back to the in-point. Dropping BEFORE the in-point is not enforced (plays from there until the end, then wraps to 0 when `outPoint == 1`).
- C-E (INFERRED from Pitfall 56 + `VideoPlayer.cpp:380-387, 810-830`): each drag event is a seek = a generation bump; the decode thread re-seeks to the keyframe at or before the target and catches up while the ring HOLDS the last shown frame (`video_late_frames`). On a long-GOP file the PICTURE can stay on the old frame during the whole drag and land at release; the model playhead is already at the new place. Looks like "not following", not a snap back.
- C-F (INFERRED): a sequence in BPM Sync: `ImageSequence::getDuration()` = `files / fps` and fps is recomputed every frame from bpm (`Renderer.cpp:1722-1731`), so the normalized playhead (`currentTime_ / dur`) moves when bpm changes while `currentTime_` is held. A drop is converted with the duration at the frame the request is consumed. Small, only sequences.
- NOT a candidate (VERIFIED): refresh/inspector timers do not write the playhead (ClipInspector.cpp:502, 1006, 1193 are reads); no beat-clock or routine/restore writer exists (grep of `playheadPosition =`/`.playhead` in src: the table above is complete).
- "where it should be playing": no code computes a "should be" position from the beat clock for video. The closest writers are C-D and the activation tail (inPoint). Whether Boris means the BPM-synced phase (a thing the app does not do today) is a question for him, not for the code.

Seek on the decode side (Pitfalls 56, 62, 64; VERIFIED in `media/VideoPlayer.cpp`): `seekTo` only stores a target + flag (:366-371). `advanceFrame` (GL thread) consumes it BEFORE the transport math (:380-387): `currentTime_ = target * duration_`, playhead stored, `gen_` bumped (:395-407). The decode thread sees the new generation (:807-830): ends the current run, forward play calls `seekToTimestamp(want)` (keyframe at or before `want`, `av_seek_frame` AVSEEK_FLAG_BACKWARD :1570-1590) and decodes up to `want`; reverse play plans its own runs from the GOP cache; direction changes are generation bumps too. The reader picks the newest ring frame with pts <= clock; none ready = HOLD the shown frame, never black (Pitfall 53/56). The seek is applied even when the clip is paused. The ImageSequence seek is the same shape (`ImageSequence.cpp:105-130`, consumed BEFORE the playing check). Pitfall 64 (frame seek aims at the middle of the frame; keyframe index = demuxer's live one) is in the project CLAUDE.md index only: the recon tree's docs/claude/pitfalls.md ends at 62/63/"NN" (no entry 64-66), so the full text of 64 was not available to me; the code of runStep's seek is at `VideoPlayer.cpp:1323-1330`.

### Q4  Transport modes, "BPM Sync", beat clock, speed mode

Enum (VERIFIED): `Clip::TransportMode { Timeline, BPMSync }` (`model/Clip.h:~115`), default Timeline; `Clip::LoopMode { Loop, PingPong, OneShot }`; fields `speed` (1.0), `reverse`, `inPoint`/`outPoint`, `beatDivision` (4.0, "BPM Sync: beats per playback cycle"), `videoBeats` (4.0, "how many beats the source content contains"), `beatSnapMode`/`beatSnap`. All persisted (`model/Clip.cpp:46-48, 66-75`; load :189-213) and preserved by `replaceContent` except `beatDivision`/`videoBeats` which COPY from the new content (Clip.h `replaceContent`: `beatDivision = newContent.beatDivision; videoBeats = newContent.videoBeats`). The UI label for `TransportMode::Timeline` is "Timeline"; for BPMSync "BPM Sync" (`ClipInspector.cpp:7-10`). "Timeline" mode IS Boris's "speed control".

Video, Timeline mode (VERIFIED `Renderer.cpp:1668-1671`): `player->setSpeed(clip.speed * masterSpeed)` (masterSpeed = composition `CompScalar::Speed`). Reverse via `setReverse` (not PingPong). Loop modes via the player.

Video, BPM Sync (VERIFIED `Renderer.cpp:1657-1667`): `if (snap.bpm > 0 && beatDivision > 0) player->setSpeed(videoBeats / beatDivision)`.
- The BPM value is used ONLY as an on/off gate. The speed does not depend on the tempo, so a video does NOT follow tempo changes: it plays at a fixed multiple of its native speed. "Synced to the current playing BPM" is true only if the file's native tempo equals the live tempo and `videoBeats` was set right (INFERRED from the formula; nothing in the code reads bpm into a rate).
- `clip.speed` and `masterSpeed` are IGNORED for video in this mode (the speed slider in the Clip tab is still shown and does nothing for a video; VERIFIED by reading the branch).
- With `snap.bpm <= 0` the branch does not call `setSpeed`: the player keeps the last speed it had (initial `speed_ = 1.0f`, `VideoPlayer.h:225`) (VERIFIED).
- There is NO phase lock: nothing aligns the content position to the bar/beat grid on trigger or while playing (the position is whatever the player's clock says). "Bars" appear nowhere in the model: `beatDivision` and `videoBeats` are beats.
- Reverse and loop modes still apply.

Image sequence, BPM Sync (VERIFIED `Renderer.cpp:1704-1735`): `seq->setSpeed(clip.speed)` (masterSpeed skipped, `effectiveClipSpeed(... isBpmSynced=true)`); fps = `numFrames * bpm / (beatDivision * 60)` recomputed every frame, so ONE cycle of all frames = `beatDivision` beats at the live bpm. Here the tempo IS followed (rate follows bpm). `videoBeats` is not used for sequences. Image sequence, Timeline: fps = `clip.sequenceFps` (slider 0..6 images/sec, default 2.5).

When BPM changes: video BPM Sync: nothing changes in rate (see above); sequence BPM Sync: fps changes immediately, `currentTime_` held, so the normalized playhead jumps (C-F). `Composition::bpmMultiplier` (TopBar x1/x2/..., `ui/TopBar.cpp:402-403`) has no reader outside save/load in `src/` (grep) -- INFERRED not applied to `snap.bpm`.

What the UI offers (VERIFIED, `ui/ClipInspector.cpp`):
- Mode combo "Timeline" / "BPM Sync" in the Transport header (:6-18).
- Timeline mode: Speed slider 0..4 step 0.01 + ÷2 / ×2 buttons + Reverse (:77-120); Loop dropdown (Loop / Ping Pong / One Shot); for sequences an "Images/ Sec" slider (:178-201, 0..6).
- BPM Sync mode (any playable clip): combo "Beats/ Cycle" with the presets `1/4 Beat, 1/2 Beat, 1 Beat, 2 Beats, 4 Beats (1 Bar), 8 Beats (2 Bars), 16 Beats (4 Bars)` -> beatDivision 0.25..16 (:204-234); "Content Beats" slider 1..64, editable text box, snaps to {1,2,4,8,16,32,64} (:237-275) -> videoBeats. No free number box for the loop length; no "bars" unit; max 16 beats.
- Timeline bar draws `int(beatDivision)` division lines in BPM Sync (:1140-1152).
- Automatic length: NONE. VERIFIED: `videoBeats`/`beatDivision` are written only by the two UI controls, `Clip::fromVar`, `replaceContent`, `clear` (grep of src); `VideoPlayer::getDuration()` has no caller outside `src/media/` (grep); a new clip starts at 4 beats / 4 beats whatever its length or the tempo. Image sequences get `sequenceFps = 2.5` at creation (`MainComponent.cpp:5474`).
- Both modes at once on different clips: nothing prevents it; the branch is per clip inside syncMedia (VERIFIED). Their speed rules differ as above.

### Q5  Quantize: where a trigger is queued and released; modes alike?

Queue (VERIFIED):
- Global control: `Composition::QuantizeMode { Off, NextBeat, NextDownbeat }` (`model/Composition.h:144-145`), TopBar combo "Off / Next Beat / Next Downbeat" (`ui/TopBar.cpp:166-186`). Per-clip: `Clip::beatSnapMode { Off, Beat, Bar, TwoBar, FourBar }` (Clip tab combo "Snap Off/Beat/Bar/2 Bar/4 Bar", `ClipInspector.cpp:~169-185` area, onChange sets `beatSnapMode` and legacy `beatSnap`).
- `quantizeModeToForcedSnap` (`MainComponent.cpp:36-42`): Off -> Off; tracker not LOCKED -> Off (an honest immediate trigger); NextBeat -> Beat; NextDownbeat -> Bar. Used by cell (:4746), column (:4975), routines (:4176, :6156).
- `Layer::triggerClip`: `snapEnabled = forcedSnap != Off || target->beatSnapMode != Off || target->beatSnap` (`Layer.h:420-421`); if snapEnabled and `ref != activeRef` the ref goes into the layer's single PENDING slot (`pendingTriggerColumn/DeckId/SnapOverride`, `Layer.h:424-429`), otherwise immediate. One pending slot per layer (a new queued trigger replaces it).
- It never inspects `transportMode`: BPM Sync and Timeline clips are treated alike (VERIFIED: `transportMode` appears only in Renderer.cpp:1657/1706/1718, ClipInspector, Clip.cpp serialization).
- A re-fire of the active ref is never queued (Q1) -- the only trigger Quantize does not delay.

Release (VERIFIED):
- GL thread, once per frame inside the `deckActive` gate: `showAutopilot_.processFrame` (`Renderer.cpp:508-520`) -> `Autopilot.cpp:76-90`: on a BEAT crossing (`beatCrossings_.consume(snapshot.totalBeatCount)`, Pitfall 42) for every layer with `pendingTriggerColumn >= 0` -> `Layer::processPendingTrigger(beatInBar, barCount, rows)` (`Layer.h:449-490`). Snap mode: a forced override wins, else the pending clip's own `beatSnapMode`, else Beat. Beat/Off: fire on every beat crossing; Bar: `beatInBar == 0`; TwoBar: also `barCount % 2 == 0`; FourBar: `% 4 == 0`. The fire is `immediateNext` + `applyActivationTail` (model playhead <- inPoint, `playing <- true` only for a never-triggered clip); the player is not seeked (INFERRED + Q2): a queued BPM-synced clip starts at its parked position, not at bar 1 of its loop.
- Granularity today = whole beats and whole bars up to 4 bars. Boris's "Quantize 1 bar, 1/2 bar, 1/4, 1/8, 1/6" (verbatim from binding-decisions line 639) has sub-beat values; the release clock only fires on a beat crossing (`consume(totalBeatCount)` -> 0 or more whole beats), so 1/8 and 1/16 values have no edge to fire on today (INFERRED; the TopBar combo and `QuantizeMode`/`BeatSnapMode` enums stop at Beat/Bar/2/4 bars).
- Released only while the tracker keeps a lock and beats keep crossing; a cancel (clear, momentary release before the beat) is a CAS on the same word (`Layer.h:492-525`).

### Q6  What the UI shows for a clip's length today, and the removed "Beats/Duration" row

VERIFIED (recon tree, `ui/ClipInspector.cpp`):
- The Clip tab shows NO length in seconds, frames or bars for a video. The Transport header has a position readout `juce::String(pos, 2)` = the normalized playhead fraction (e.g. "0.37"), no unit (:500-508). The timeline bar has no tick labels.
- BPM Sync: "Beats/ Cycle" combo (beats, bar equivalents in the three largest labels) + "Content Beats" slider (beats). Timeline: nothing for a video; "Images/ Sec" for a sequence.
- The "Duration" row (:90-103, layout :628-638, label painted :525-528): a `durationSlider_` 0.1..300 (default 8.0), `durHalfBtn_` "/2", `durDoubleBtn_` "×2". The label reads "Duration" in Timeline mode and "Beats" in BPM Sync mode. It is INERT: no `onValueChange`, no `onClick` on the buttons, no model field (grep of `durationSlider_|durHalfBtn_|durDoubleBtn_` over src/ shows only declaration, constructor, bounds). It is STILL PRESENT in the recon tree.
- plan-bf7 (/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-1002b/plan-bf7.md:112, 269) left it "NOT touched (findings)"; ruling-bf7 AM4 (/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-1002b/ruling-bf7.md:254-265) orders its removal (members, constructor code :89-98 and :102-103, layout :628-638, painted label :525-528; rows below move up by one `kRowHeight`) and the relabels "Beats/ Cycle" -> "Loop length" and "Content Beats" -> "Content length" (ruling-bf7.md:85, 274-276). The recon tree contains none of the bf7 strings ("Loop length", "Content length", "Image every": grep = 0), so bf7 is NOT in this tree. Whether bf7 lands before or after the transport work is a sequencing fact the planner must check.
- Two more clip-tab facts from the same file: the "Restart / Continue / Relative" trigger dropdown is inert (Q2); the Speed slider is inert for a BPM Sync video (Q4).
- The deck cell shows no length either; its tooltip for a sequence reads "N images at X images/sec" (`ui/ClipCell.cpp:380-381`).

---------------------------------------------------------------------------------------------------
## 2  TABLES

### 2a  Re-fire of the clip a layer already plays

| gesture | tuple | player | model playhead | `playing` | undo | with Quantize on |
|---|---|---|---|---|---|---|
| CELL click (handleClipTrigger) | unchanged (no CAS) | `seekTo(inPoint)` (:4787-4811) unless the clip's Snap is on (then `seekTo(beatPhase)`, :4765) | reset by tail, then follows the player | kept | none | immediate (never queued) |
| COLUMN fire (handleColumnTrigger), layer already on that ref | unchanged | NO seek | reset by tail, overwritten by write-back next frame | kept | none | immediate |
| COLUMN fire, layer on another clip | active <- ref, crossfade | NO seek (resumes, C3) | = player position | set true if never "triggered" (and the column path never sets that flag) | one composite | queued if Quantize / Snap on |
| COLUMN fire, empty cell | layer cleared | stops being drawn | n/a | old clip `playing <- false` | composite | not queued (clear is immediate) |

### 2b  Transport behaviour per mode (video / image sequence)

| | Timeline ("speed control") | BPM Sync |
|---|---|---|
| video rate | `clip.speed * masterSpeed` | `videoBeats / beatDivision`; tempo never enters (gate `bpm > 0`); `clip.speed` and `masterSpeed` ignored |
| video, `bpm <= 0` | n/a | rate left as last set |
| sequence rate | fps = `sequenceFps`, speed = `clip.speed * masterSpeed` | fps = `numFrames * bpm / (beatDivision * 60)`, speed = `clip.speed` |
| follows tempo change | no | sequence yes, video no |
| phase to the beat grid | none | none |
| UI | Speed slider 0-4, ÷2, ×2, Reverse, Images/Sec (sequences) | Beats/ Cycle combo (0.25..16 beats), Content Beats (1..64, snapped) |
| Quantize / Snap | same code path (never reads the mode) | same |

### 2c  Pitfall map for this area

| Pitfall | what it fixes | where |
|---|---|---|
| 2 | intent/write-back of `playing`, no player-state race | `render/ClipTransportSync.h` |
| 7 | `hasBeenTriggered` = first activation vs return | `Layer.h:555`, `MainComponent.cpp:4763` (cell path only) |
| 56 | decode off the GL thread; seek = request + generation bump; HOLD not black | `media/VideoPlayer.cpp:366-407, 807-830` |
| 62 | reverse from a GOP cache; direction change = discontinuity | `VideoPlayer.cpp:430-470` |
| 64 | frame seek aims at frame middle; live keyframe index | text not in the recon docs; code `VideoPlayer.cpp:1323-1330` |

---------------------------------------------------------------------------------------------------
## 3  WHAT EXISTS TODAY vs WHAT BORIS ASKED (plain gaps)

1. "Firing a column that is already playing should restart its videos": today the cell re-fire restarts (player seek to in-point), the column re-fire does not seek anything (no `seekTo` in handleColumnTrigger). Also today's re-fire is never quantized, and the column path leaves `hasBeenTriggered` unset (auto-play on every new activation).
2. Playhead drag: two UIs; the layer-strip one only writes the model and is overwritten by the next render write-back; the Clip-tab one seeks the player and keeps playing but grabs the in/out marker instead of the playhead near the ends, and a drop at/after `outPoint` snaps to `inPoint`. No code computes a "should be" position from the beat clock.
3. "Two categories": the enum has exactly two modes (Timeline, BPMSync), so the category split exists, but BPM Sync for a VIDEO is not synced to the tempo (fixed speed ratio), the Speed control is ignored in it, and sequences are the only clips that follow bpm.
4. "it has bars that the user can set. We automatically set bars for a cliff": no bars anywhere in the model or UI (beats only; presets 0.25..16 beats; content beats 1..64 snapped), no automatic length from file duration or tempo (video duration is unused outside src/media), defaults 4/4 for every new clip.
5. "Regardless of the clip being BPM or speed controlled, if we are in Qantize mode, it is triggered on time": the queue/release code is mode-blind (good), but a re-fire is never queued, a queued fire does not align the clip's content phase to the bar, and the release clock is whole beats / bars (sub-beat values Boris listed have no edge).
6. "no problem playing clips that are set up for BPM and clips that are set up for speed at the same time": per-clip branch in `syncMedia`; works structurally.
7. The dead Duration row and the inert Restart/Continue/Relative combo are still in the tree; bf7 (not in this tree) removes the first.

---------------------------------------------------------------------------------------------------
## 4  UNKNOWN-NEEDS-A-RUN

U1. Column re-fire really leaves a playing video running (VERIFIED by code, not run). Cheapest test: 8080 or 7070 app, a deck with one video per layer; `POST /api/trigger_column {"column":0}`; sample `GET /api/composition` -> `decks[].layers[].clips[].playheadPosition` (and `playing`) at t0 and t0+2 s; fire `trigger_column` again; sample at +50 ms and +250 ms; expect a continuing value (no drop to `inPoint`). Compare with `POST /api/trigger_clip {"layer":L,"column":0}` twice (expect a drop to `inPoint`, as probe-media-open m6 / probe-video w3 assert). Use inPoint 0.5 so a restart is distinguishable from a wrap.
U2. Resume after replacement: fire A, wait 2 s, fire B in the same layer (same column of another deck, or another column), wait 2 s, fire A; read A's `playheadPosition` before leaving, while away (should be frozen), and after return (continues from the frozen value). Probe `.harmony/probe-boxes.py` k10 covers the resume half.
U3. Which drag jumps back: no REST route seeks, plays or sets speed (`/api/set_clip_param` accepts only `fitMode`, `ApiServer.cpp:700-720`), and the drag is mouse input, so this needs either Boris's hand with a log of `playheadPosition` via `/api/composition` while dragging in (a) the Clip tab bar and (b) the layer strip bar, or a new test hook. Cheapest ctest without the app: a `ClipTransportSync` case with a fake player (tests/test_clip_transport_sync.cpp already does this) that writes `clip.playheadPosition` as `scrubPlayhead` does (no player seek) then runs `writeBack` -> expect the model reverts.
U4. Picture lag during a Clip-tab drag on a long-GOP file (C-E): drag in a run with `/api/state` `video_seeks` / `video_late_frames` sampled before/after; no REST action to seek, same limitation as U3.
U5. Whether a BPM-Sync video really does not track a tempo change: `POST /api/set_bpm` (manual) while a BPM Sync clip plays (clip authored in a composition JSON with `transportMode: 1`, `beatDivision`, `videoBeats`; load it with `/api/load_composition {"path"}`); sample `playheadPosition` rate before/after -> expect identical per-second advance. (Composition JSON is the only way to author `transportMode`; `/api/composition` does not report `transportMode`, `speed`, `beatDivision` or `videoBeats` -- VERIFIED by grep of ApiServer.cpp.)
U6. A queued trigger of a paused/never-played clip: whether the tail's `playing <- true` and the unseeked player show the expected picture at the bar (read `playing` and `playheadPosition` before/after the bar with Quantize NextDownbeat; requires a LOCKED tracker, e.g. the audio file source `POST /api/audio/source`).
U7. The beat-snap branch (`clip->beatSnap`, :4765) writes `beatPhase` as a file position: confirm with a clip whose Snap combo is "Beat": cell-click twice, read `playheadPosition` after the second click (expect a value in [0,1) unrelated to the in-point).

---------------------------------------------------------------------------------------------------
## 5  FILES A BUILDER WOULD TOUCH (paths only)

/Users/boriskarpman/projects/RealTimeAudio/src/MainComponent.cpp  (handleClipTrigger ~4686, handleColumnTrigger ~4935, syncActivatedPlayhead ~4903, quantizeModeToForcedSnap ~36, onCuepointJump ~1466, applyClipPlaying ~6440)
/Users/boriskarpman/projects/RealTimeAudio/src/MainComponent.h
/Users/boriskarpman/projects/RealTimeAudio/src/model/Layer.h  (triggerClip, activate, applyActivationTail, processPendingTrigger)
/Users/boriskarpman/projects/RealTimeAudio/src/model/Composition.h  (fire, triggerColumn, QuantizeMode)
/Users/boriskarpman/projects/RealTimeAudio/src/model/Clip.h and Clip.cpp  (TransportMode, beatDivision, videoBeats, serialization, replaceContent)
/Users/boriskarpman/projects/RealTimeAudio/src/model/Autopilot.cpp and Autopilot.h  (the queued-trigger release on the beat crossing)
/Users/boriskarpman/projects/RealTimeAudio/src/render/Renderer.cpp  (syncMedia BPM branches ~1615-1760)
/Users/boriskarpman/projects/RealTimeAudio/src/render/Renderer.h  (effectiveClipSpeed)
/Users/boriskarpman/projects/RealTimeAudio/src/render/ClipTransportSync.h  (the write-back and the out-point wrap)
/Users/boriskarpman/projects/RealTimeAudio/src/media/VideoPlayer.cpp and VideoPlayer.h  (seekTo, advanceFrame, advanceTransport: Loop wrap and in-point)
/Users/boriskarpman/projects/RealTimeAudio/src/media/ImageSequence.cpp and ImageSequence.h
/Users/boriskarpman/projects/RealTimeAudio/src/ui/ClipInspector.cpp and ClipInspector.h  (timeline bar mouse handlers, mode combo, Beats/ Cycle, Content Beats, Duration row, trigger dropdown)
/Users/boriskarpman/projects/RealTimeAudio/src/ui/LayerStrip.cpp and LayerStrip.h  (scrubPlayhead)
/Users/boriskarpman/projects/RealTimeAudio/src/ui/TopBar.cpp  (Quantize combo)
/Users/boriskarpman/projects/RealTimeAudio/src/core/TriggerCommands.h  (undo of a trigger; only if the retrigger gets an undo entry)
/Users/boriskarpman/projects/RealTimeAudio/src/recording/RoutineEngine.cpp  (routine snap/quantize; its own restart semantics)
/Users/boriskarpman/projects/RealTimeAudio/src/api/ApiServer.cpp  (only if a REST route for seek/mode/speed or the missing /api/composition fields is added)
Docs a change touches: CLAUDE.md Pitfall 7 line, docs/claude/performance-controls.md (lines 43-51 and "Beat Snap Granularity"), docs/claude/architecture.md (beatDivision/videoBeats rows ~91-92), .harmony/APP-INVENTORY.md.

---------------------------------------------------------------------------------------------------
## 6  EXISTING TESTS / PROBES, and REST routes that can drive this area

ctest cases (names are file/TEST_CASE; read, not run):
- tests/test_layer_runtime.cpp: SECTION "retrigger: the tuple is unchanged, the clip restarts from its in-point and keeps its play state" (:280); "a queued trigger cancelled by cancelPendingInto never fires"; "releaseMomentary: press A, press B, release A leaves B queued"; tests/test_layer_runtime_race.cpp.
- tests/test_composition.cpp "Retrigger resets playhead" (:129) -- model only, proves the tail, does not prove a player seek.
- tests/test_show_model.cpp T3 "the same column from another deck is a new clip: a crossfade, not a retrigger (bf9b)".
- tests/test_clip_transport_sync.cpp (4 cases: trigger inside the sync window; OneShot stop; pause inside the window; intent changed across an out-point crossing).
- tests/test_clip_inspector_paint_key.cpp (PaintKey fields incl. transportMode, playhead, beatDivision), tests/test_layer_strip_transport_view.cpp (strip playhead repaint), tests/test_layer_transport_reverse.cpp, tests/test_take_v1_transport.cpp.
- tests/test_autopilot.cpp (pending-trigger release on beat crossings), tests/test_undo_commands.cpp (TriggerClipCmd).
- tests/test_video_ring.cpp, test_gop_cache*.cpp, test_video_decode_trace.cpp (decode side of a seek).
- No ctest drives handleColumnTrigger or handleClipTrigger (grep: only test_undo_commands.cpp and test_layer_strip_source_deck.cpp name them) and none covers the Clip tab drag or `LayerStrip::scrubPlayhead`.

Probes (all REST + captures, none drag):
- .harmony/probe-media-open.py/.sh: m6_retrigger_seek (sequence, cell retrigger -> playhead in [0.5, 0.8)); m8_speed_default.
- .harmony/probe-video.py/.sh: w3_retrigger_midgop_1080 / w3b (cell retrigger, prime), w4_deck_return_1080, w6b_retrigger_mid_fade, w10 frame-accuracy rows.
- .harmony/probe-boxes.py/.sh: k10_fresh_and_resume (a re-fired clip resumes; REST playhead still while in no layer), k1b_switch_video, k3_autopilot.
- .harmony/probe-vupload.py (trigger_column rows u4b/u8/u11), probe-crossfade, probe-idle-paint (strip playhead repaints).

REST/Eyes routes that drive this without synthetic input (VERIFIED route list `ApiServer.cpp:163-344`): `POST /api/trigger_clip {layer, column}`; `POST /api/trigger_column {column}` (the SHOWN deck's column); `POST /api/switch_deck`; `POST /api/set_bpm`, `GET /api/bpm`, `POST /api/resync`, `POST /api/inject_features` (8080 test mode; fake bpm/beat phase); `POST /api/audio/source` (file source for a locked tracker); `GET /api/composition` (per clip: `playing`, `playheadPosition`, ids, layer `activeClip/previousClip/pendingClip` refs; NOT transportMode/speed/loop/beatDivision/videoBeats/inPoint/outPoint); `GET /api/state` (video_seeks, video_late_frames, seq_late_frames, fence counters); `POST /api/render_frame {output_path, time}` and `/api/snapshot` for pictures; `POST /api/load_composition {path}` and test-only `POST /api/debug/load_deck`, `/api/debug/drop_files` to author clips with transport fields (the only way to set `transportMode`, `speed`, `inPoint`, `beatDivision`); `POST /api/perf/*` and `/api/routine/*` for recorded/routine fires. There is NO REST route to seek, play/pause, set speed, mode, loop, bars or quantize mode (`/api/set_clip_param` accepts only `fitMode`); quantize mode and the clip tab are reachable only by the UI or by a loaded composition (`quantizeMode` is saved in the composition JSON, `Composition.h:742`).
