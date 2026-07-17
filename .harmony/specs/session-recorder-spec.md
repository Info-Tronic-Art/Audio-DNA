# Session Recorder — Full Implementation Spec (Wave 2)

Status: PLAN (2026-07-17). Boris-ratified scope: "IMPLEMENT fully (7 event types +
playback)" (triage decision 4). Build in next sessions per drain long-task rule.

All file:line cites verified against source this session, BEFORE Wave-0 lands its
deletes — Wave-0 touches none of the files below except MainComponent.cpp (menu/
ProgrammingMode removal), so **line numbers in MainComponent.cpp will drift; use the
symbol names given alongside every cite**.

## 0. Current state (verified)

- `SessionRecorder` (src/recording/SessionRecorder.h/.cpp): 7 event types, 7
  `record*` methods, JSON save/load (version 1), `advancePlayback(dt)` implemented
  but **never called** by anyone.
- Only capture call in production: `recordClipTrigger` inside
  `MainComponent::handleClipTrigger` (MainComponent.cpp:2472).
- RecordPanel (src/ui/RecordPanel.cpp): Record/Stop/Play/Save/Load work against the
  recorder object; Play calls `startPlayback()` but since nothing drives
  `advancePlayback`, nothing happens. `RecordPanel::refresh()` has **zero callers**
  (event-count label never updates). The three `onStartRecording/onStopRecording/
  onPlayRecording` callbacks are unwired (only `setSessionRecorder` is called, at
  MainComponent.cpp:931).
- `ApiServer` takes a `SessionRecorder&` (ApiServer.cpp:25,35) and never uses it.
- Timing today: wall-clock seconds since recording start
  (`juce::Time::getMillisecondCounterHiRes`, SessionRecorder.cpp:3-6).
- MainComponent runs a 30 Hz message-thread timer (`startTimerHz(30)`,
  MainComponent.cpp:247; `MainComponent::timerCallback` at :1771).
- All remote trigger paths already funnel to the message thread:
  ApiServer wraps callbacks in `MessageManager::callAsync` (ApiServer.cpp:319,340,475),
  OSC likewise (MainComponent.cpp:1128). MIDI/keyboard bindings dispatch through
  `MainComponent::handleBindingAction`.

## 1. Timing model decision: wall-clock primary, beat metadata recorded

**Keep wall-clock timestamps as the replay clock.** Rationale:

1. Deterministic. Beat-relative replay would depend on the live BPM tracker's state
   (searching/locking/locked, FeatureSnapshot.h:41), which is not reproducible
   between sessions even against the same audio file.
2. The VJ use case is "re-run my set against (the same) audio" — wall time matches
   the audio timeline.
3. Beat ALIGNMENT still happens for free: replayed ClipTriggers re-enter
   `MainComponent::handleClipTrigger`, whose beat-snap block reads the **live**
   FeatureBus and re-quantizes the playhead (MainComponent.cpp:2480-2503,
   `clip->beatSnap`). So replayed triggers lock to the live beat clock even if the
   wall-clock timestamp is a few ms off. This is the best of both models.

Additionally record per-event beat metadata (see schema v2): `bpm`, `beatPhase`,
`barPhase` sampled from the FeatureBus at capture time (pattern:
`analysisThread_.getFeatureBus().getLatestRead()`, as used in `handleClipTrigger`
MainComponent.cpp:2482-2485). These fields are inert in v2 playback; they enable a
future beat-relative replay mode without re-recording.

## 2. Capture design

### 2.1 The flood-control rule (load-bearing)

> **Record the user's input, never the output of automation.** A ParameterChange /
> MacroChange event is recorded only from a UI-gesture or binding-action call site.
> Automated writers are excluded by construction and reproduce themselves at replay
> time because the audio-analysis engines run live during playback.

Excluded writers (verified automated, per-frame or per-refresh):
- `mappingEngine_.processFrame` — GL thread, every frame (Renderer.cpp:210).
- `routingEngine_.processFrame` — GL thread, every frame (Renderer.cpp:198).
- `EffectStackView::refresh()` signal/macro-driven block — writes
  `fx.paramValues[p]` and fires `onParamChanged` at inspector-refresh rate
  (EffectStackView.cpp:199-208). **Do not wire the existing `onParamChanged` for
  recording** — it multiplexes user and automated writes. Add a new callback (2.2#7).
- `EffectsRackPanel::timerCallback` knob refresh — uses `dontSendNotification`
  (EffectsRackPanel.cpp:220), so it never fires knob callbacks. Safe.
- `UniversalParamControl::setParamValue` — `dontSendNotification`
  (UniversalParamControl.cpp:284), so `onValueChanged` is inherently user-only.
- `MacroPanel::refresh()` — `dontSendNotification` (MacroPanel.cpp:87,95), so
  `onMacroValueChanged` (MacroPanel.cpp:19) is inherently user-only.

### 2.2 Call-site list — all 7 event types

All `record*` calls live in MainComponent (owner of `sessionRecorder_`,
MainComponent.h:208), fed by existing or new callbacks. "NEW cb" = add a
`std::function` member and fire it at the cited site.

1. **ClipTrigger** — DONE: `MainComponent::handleClipTrigger`
   (MainComponent.cpp:2472). All trigger paths funnel here: deck UI
   (MainComponent.cpp:559), bindings (:3625), REST (:1117), OSC (:1128).

2. **ColumnTrigger** — `MainComponent::handleColumnTrigger`, one line after
   `deck->triggerColumn(column)` (MainComponent.cpp:2567):
   `sessionRecorder_.recordColumnTrigger(column);`
   Note: `handleColumnTrigger` does NOT route through `handleClipTrigger`, so
   there is no double-record.

3. **MacroChange**
   a. `MacroPanel::onMacroValueChanged` (fired MacroPanel.cpp:19; currently has
      zero consumers — verified). MacroPanels live inside ClipInspector /
      LayerInspector / CompositionInspector, all pointing at the single
      `globalMacroBank_` (MainComponent.cpp:901; "Global-only MacroBank" is a
      known deferred item). Forward each inspector's panel callback up through
      one `InspectorPanel::onMacroChanged(int idx, float v)` (NEW cb) →
      `recordMacroChange(idx, v)`.
   b. Binding path `Binding::Action::AdjustMacro`
      (`MainComponent::handleBindingAction`, MainComponent.cpp:3839-3845) — add
      record call next to the `manualValue = value` write.

4. **TransportChange** (see schema v2 for the action vocabulary)
   a. ClipInspector transport controls — all mutate `clip_` directly with no
      callback out today: playBtn_/pauseBtn_/playBackBtn_
      (ClipInspector.cpp:36-52), reverseBtn_ (:118-123), speedSlider_ (:83-85),
      halfSpeedBtn_/doubleSpeedBtn_ (:104-113). NEW cb
      `ClipInspector::onTransportAction(Clip*, const std::string& action, float value)`
      fired in each handler; MainComponent wires it →
      `recordTransportChange(action, value, clip->id)` (signature gains clipId —
      see schema v2).
   b. Binding `LayerTransport` (MainComponent.cpp:3796-3815, toggles
      `clip->playing`) → record "toggle_play" with clip id.
   c. Binding `GlobalPlayPause` / `GlobalStop` (MainComponent.cpp:3766-3779,
      `audioEngine_.play()/stop()`) → record "global_play"/"global_pause"/
      "global_stop", clipId 0.
   d. **Deck switch** — `MainComponent::handleDeckSwitch` (MainComponent.cpp:2707)
      → record action "deck_switch", value = deckIndex. Required for replay
      correctness: ClipTrigger events are active-deck-relative.
   e. Wave-1 D wires TopBar Play/Pause/Stop — when that lands, its handlers are
      also capture sites. Sequencing: this wave builds after Wave-1.

5. **EffectToggle**
   a. Global rack: `EffectsRackPanel` enableToggle onClick
      (EffectsRackPanel.cpp:355-364). NEW cb
      `onEffectToggled(int effectIndex, bool enabled)`; MainComponent (rack
      constructed at MainComponent.cpp:373) wires it →
      `recordEffectToggle(chainEffectName, enabled)` with scope=Global,
      effectIndex recorded (names come from `Effect::getName`, Effect.h:52).
   b. Clip/layer stacks: `EffectStackView` bypassBtn fires `onBypassChanged`
      (EffectStackView.cpp:266-272) — currently zero consumers (verified).
      Forward through ClipInspector/LayerInspector → InspectorPanel →
      MainComponent → record with scope=Clip/Layer + owner id + effectIndex
      (clip stacks can contain duplicate effect names, so name alone is
      insufficient — schema v2).
   c. Binding `ToggleEffectBypass` (MainComponent.cpp:3817-3837) → record
      (scope=Clip, active clip id, `binding.targetEffectIndex`).

6. **CuepointJump** — the jump handler already funnels to MainComponent:
   `ClipInspector::onCuepointJump` fired at ClipInspector.cpp:305-306, consumed at
   MainComponent.cpp:911-924. Change the callback signature to
   `(Clip*, int cuepointIndex, double pos)` (index is in scope at the fire site,
   ClipInspector.cpp:285) and record in the MainComponent handler:
   `recordCuepointJump(clip->id, index, pos)`.

7. **ParameterChange** — user-originated only, coalesced (2.3):
   a. Global rack knobs: knob `onValueChange` (EffectsRackPanel.cpp:467-476) is
      user-only (see 2.1). NEW cb `onParamEdited(int effectIndex, int paramIndex,
      float value)` fired there → record scope=Global.
   b. Clip/layer effect stacks: NEW cb `EffectStackView::onUserParamChanged`,
      fired ONLY from the per-control `pc->onValueChanged` lambda
      (EffectStackView.cpp:337-343) and the dry/wet `dwc->onValueChanged`
      (EffectStackView.cpp:298-302, record as paramIndex = -1). Do NOT fire it
      from `refresh()` (EffectStackView.cpp:206). Forward through the inspectors
      like 5b → record scope=Clip/Layer + owner id + effectIndex.
   c. Source params (`ClipInspector::onSourceParamsChanged`,
      MainComponent.cpp:905-908): stretch — the callback currently passes only
      `Clip*`, not which param; plumb index or skip in v1 (documented gap).

Known v1 capture gaps (documented, not blocking): Randomize
(`MainComponent::randomizeAllEffects` MainComponent.cpp:2284, `beatSyncRandomize`,
rack per-effect "R" EffectsRackPanel.cpp:404-450) mutates params AND mappings;
mappings are not an event type, so a set that leans on Randomize will not replay
identically. Stretch: burst-record the resolved global-chain param values +
enable states after each randomize (bounded: effects × params events). Layer
toggles (solo/mute/bypass/opacity, MainComponent.cpp:3680-3719, :3785-3794) and
preset/deck loads are also uncaptured — out of scope v1, listed in §8.

### 2.3 Coalescing rule (ParameterChange, MacroChange, speed)

Knob drags emit per-mouse-move (can exceed 100 Hz). In
`recordParameterChange`/`recordMacroChange`/`recordTransportChange("speed")`:
if the **last** event in `events_` has the same type and same target key
(scope, targetId, effectIndex, paramIndex) and `now - last.timestamp <= 0.05 s`,
overwrite its value and timestamp in place instead of appending. Effects:
≤ 20 events/s per continuously-dragged control, final value always preserved,
gesture shape kept at ≥ UI refresh fidelity. Never coalesce ClipTrigger /
ColumnTrigger / EffectToggle / CuepointJump / non-speed transport.

## 3. Schema v2 (JSON, version field 2; v1 files still load)

Why v1 is insufficient (verified against SessionRecorder.h:37-57):
ParameterChange can't distinguish global-chain vs clip-effect vs dry/wet targets;
EffectToggle has no owner identity or effect index (duplicate names possible);
TransportChange has no clip identity; deck switches aren't captured at all.

Event struct additions (serialize only when meaningful for the type; keep all
existing v1 field names so ClipTrigger/ColumnTrigger v1 files parse unchanged):

- `uint8_t scope` — 0=Global, 1=Layer, 2=Clip (ParameterChange, EffectToggle).
- `int effectIndex` — position in the owner's chain/stack (ParameterChange,
  EffectToggle). `paramIndex == -1` on a Clip/Layer-scope ParameterChange means
  dry/wet.
- `targetId` reused as owner id: clip id or layer id per `scope`; also set on
  TransportChange (0 = global transport). Matches the system's canonical
  `RouteTarget` addressing (Route.h:44-52).
- Beat metadata (all types): `bpm`, `beatPhase`, `barPhase` floats (§1).
- Root object: `version: 2`, `recordedAt` ISO string, `events` array (as today).

TransportChange `action` vocabulary: `play`, `pause`, `play_reverse`,
`reverse_toggle`, `speed` (value = multiplier), `toggle_play`, `global_play`,
`global_pause`, `global_stop`, `deck_switch` (value = deck index).

Loader: `version` 1 → defaults (scope=Global, effectIndex=0, no beat metadata).
No real v1 files exist in the wild beyond clip-trigger-only sessions, so this is
cheap insurance, not a migration.

API changes to SessionRecorder (keep all existing names):
- `recordTransportChange(action, value, uint32_t targetId = 0)`.
- `recordEffectToggle(effectName, enabled, uint8_t scope, uint32_t ownerId, int effectIndex)`.
- `recordParameterChange(scope, ownerId, effectIndex, paramIndex, value)`.
- `startRecording()/startPlayback()` return bool and refuse when the other mode
  is active (§5). New: `getPlaybackTime()`, optional injectable clock for tests
  (§7). Add the lock to `getNumEvents/getDuration/saveToFile` (currently
  lock-free reads racing HTTP-thread capture — SessionRecorder.cpp:144-153,165).

## 4. Playback design

### 4.1 Drive mechanism

`MainComponent::timerCallback` (30 Hz message-thread timer, MainComponent.cpp:1771)
gets, at its top:

```
if (sessionRecorder_.isPlaying())
{
    const double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    const double dt = now - lastPlaybackTick_;   // measured, not fixed 1/30
    lastPlaybackTick_ = now;
    for (const auto* e : sessionRecorder_.advancePlayback(dt))
        applySessionEvent(*e);
    // auto-stop is detected by RecordPanel::refresh polling isPlaying()
}
```

`lastPlaybackTick_` initialized when playback starts. Measured dt sums to true
elapsed wall time → zero systematic drift; worst-case event lateness is one timer
period (~33 ms) plus message-thread jitter, acceptable for visuals and masked for
triggers by live beat-snap re-quantization (§1). Rejected alternative: driving
from the GL render thread — every dispatched handler is a message-thread API
(`handleClipTrigger` touches JUCE components), so a GL driver would immediately
hop back via callAsync, adding latency variance rather than removing it.

### 4.2 Dispatch — `MainComponent::applySessionEvent(const SessionRecorder::Event&)`

Replayed events re-enter the NORMAL paths (same functions user input funnels to):

- ClipTrigger → `handleClipTrigger(layerIndex, columnIndex)` — includes beat-snap,
  preview/output load, deck refresh. Its `recordClipTrigger` call self-gates on
  `recording_` (false during playback per §5), so no feedback loop.
- ColumnTrigger → `handleColumnTrigger(columnIndex)`.
- MacroChange → `globalMacroBank_.getMacro(paramIndex).manualValue = value`;
  the routing engine and `MacroPanel::refresh` (10 Hz) pick it up exactly as for
  a live knob turn.
- TransportChange → switch on `action`; clip-targeted actions resolve the clip via
  a new `MainComponent::findClipById(uint32_t)` helper (extract the existing
  lookup at MainComponent.cpp:3584-3601) and set `playing/reverse/speed`;
  `global_*` → `audioEngine_.play()/stop()`; `deck_switch` →
  `handleDeckSwitch((int)value)`.
- EffectToggle → scope Global: match `previewPanel_.getEffectChain()` effect by
  name → `setEnabled(enabled)`; scope Clip/Layer:
  `owner->effects[effectIndex].bypassed = !enabled`.
- CuepointJump → `findClipById`; set `clip->playheadPosition = value` and seek the
  video player / image sequence — extract the body of the existing
  `onCuepointJump` handler (MainComponent.cpp:911-924) into a shared
  `seekClipTo(Clip&, double)` used by both live and replay paths.
- ParameterChange → scope Global:
  `previewPanel_.getEffectChain().getEffect(effectIndex)->setParamValue(paramIndex, value)`;
  scope Clip/Layer: write `effects[effectIndex].paramValues[paramIndex]`
  (`paramIndex == -1` → `dryWet`).

UI feedback during replay is mostly free: deckView refreshes inside
`handleClipTrigger`; the rack timer syncs knobs from the chain
(EffectsRackPanel.cpp:205-236); `inspectorPanel_->refresh()` runs at 10 Hz
(MainComponent.cpp:1799). One S-size gap: the rack timer does not sync the enable
toggles — add `enableToggle->setToggleState(effect->isEnabled(), dontSendNotification)`
to `EffectsRackPanel::timerCallback` so replayed EffectToggles are visible.

## 5. State machine + RecordPanel

States (mutually exclusive, owned by SessionRecorder): **Idle / Recording /
Playing**. `startRecording()` returns false if playing; `startPlayback()` returns
false if recording or `events_` empty. Hard invariant: `events_` is never mutated
while Playing (`advancePlayback` hands out raw pointers into `events_`,
SessionRecorder.cpp:120-140 — mutation would dangle them). Enforced by the state
guards plus UI gating below; add a debug assertion in `loadFromFile`/`clear`/
`startRecording` that `!playing_`.

RecordPanel changes (src/ui/RecordPanel.cpp):
- Delete the shadow `recording_` bool (RecordPanel.h:35) — derive all button/label
  state from `recorder_->isRecording()/isPlaying()/getNumEvents()` in `refresh()`.
- Button gating: Record enabled only in Idle; Stop enabled in Recording OR Playing
  (Stop stops either); Play enabled only in Idle with events; Save/Load enabled
  only in Idle (Load during playback = dangling pointers; Save during recording =
  torn file).
- `refresh()` currently has no caller: make RecordPanel a `juce::Timer` at 4 Hz
  (start/stop in `visibilityChanged` or parent tab switch,
  BrowserPanel.cpp:146) — it must poll because playback auto-stops inside
  `advancePlayback` (SessionRecorder.cpp:136-137) with no callback.
- Status: "Ready" / "Recording… N events" / "Playing… 12.3 / 45.6 s" (uses new
  `getPlaybackTime()`) / "Stopped (N events)". Play button flips to resume-from-
  start semantics (playback always restarts at 0 in v1; scrubbing is out of scope).
- Wire `onPlayRecording` → MainComponent sets `lastPlaybackTick_` (§4.1). The
  unwired `onStartRecording/onStopRecording` remain available for the video-sync
  future ("Video (Future)" combo item) — leave them.

Optional S (recommended): wire the dormant `ApiServer` reference —
`POST /api/record/start|stop|play` + `GET /api/record/status` — trivial with the
existing endpoint pattern and makes the deterministic gate scriptable end-to-end.

## 6. Undo interaction (for the Wave-2 undo builder)

**Replayed events are NOT undoable. Recommended and by-construction:**
1. Undo v1 is Boris-scoped to *structural* edits only (triage decision 3); none of
   the 7 performance event types is structural — the sets are disjoint by design.
2. A replay can emit hundreds of events; pushing them through a 500-entry history
   (UndoManager.h:44) would evict the user's real structural history.
3. Precedent: DAWs exclude automation playback from undo.

Integration point to state in BOTH specs: performance-path functions —
`handleClipTrigger`, `handleColumnTrigger`, `handleBindingAction`,
`applySessionEvent`, and every §2.2 capture site — must never call
`undoManager_.perform()`. Today none do (verified: `undoManager_` is referenced
only at MainComponent.cpp:1642-1644 and :2754-2757, the undo/redo key handlers).
If undo later wraps UI param edits in Commands, replay must keep writing to the
model/engines directly, bypassing the command layer.

## 7. Build order

| # | Step | Size | Verify (independent pass/fail) |
|---|------|------|-------------------------------|
| 1 | SessionRecorder core v2: schema fields + v1-compat loader, coalescing, state-machine guards (bool returns), `getPlaybackTime`, lock on `getNumEvents/getDuration/saveToFile`, injectable clock hook for tests | S | new `test_session_recorder` (Catch2, links SessionRecorder.cpp + juce_core, per tests/CMakeLists.txt pattern e.g. test_composition lines 159-185): round-trip, coalescing, guards, advancePlayback determinism |
| 2 | Playback drive: timer hook + `applySessionEvent` + `findClipById`/`seekClipTo` extraction + ClipTrigger/ColumnTrigger/deck_switch dispatch + the one-line `recordColumnTrigger` | S | app: record clip+column triggers → Play → visible retriggers in order |
| 3 | Transport + cuepoint + macro capture: `onTransportAction` cb, `onCuepointJump` signature + record, `handleDeckSwitch` record, binding-path records (AdjustMacro, LayerTransport, GlobalPlayPause/Stop, ToggleEffectBypass), MacroPanel→InspectorPanel forwarding; dispatch for TransportChange/CuepointJump/MacroChange | M | app: record speed/reverse/cue/macro gestures → replay reproduces them; JSON shows correct action strings + clip ids |
| 4 | Param + toggle capture: rack `onParamEdited`/`onEffectToggled`, stack `onUserParamChanged` + `onBypassChanged` forwarding chain, dispatch for ParameterChange/EffectToggle, rack enable-toggle sync in timer | M | app: 5 s knob drag → file has ≤ ~100 coalesced events for that param, final value exact; replay moves the knob visibly |
| 5 | RecordPanel state machine: shadow-bool removal, button gating, 4 Hz refresh timer, playback progress, auto-stop detection | M | click-through matrix: every button in every state does the gated thing; auto-stop returns UI to Idle |
| 6 | Docs: APP-INVENTORY §2 (SessionRecorder PARTIAL→REAL, Record panel partial→yes) + §8 dead-playback row | S | inventory rows match app behavior |
| 7 | Stretch: `/api/record/*` endpoints; randomize burst-capture; source-param capture | S+M+M | REST probe; randomize→replay visual match |

Steps 1-2 are the demoable core (playback alive); 1-6 ≈ two build sessions.
Sequence AFTER Wave-1 lands (Wave-1 D adds TopBar transport handlers that become
capture sites; Wave-1 B rewires OSC in MainComponent — avoid merge churn).

## 8. Test strategy

Unit (`tests/test_session_recorder.cpp`, Catch2 v3, no GL/audio deps —
SessionRecorder is pure juce_core):
- Round-trip: build event list via `record*` (injected clock) → save → load →
  events identical field-by-field, order preserved.
- v1 file (hand-written fixture) loads; defaults applied.
- Coalescing: 100 `recordParameterChange` on one target inside 50 ms → bounded
  count, final value preserved; interleaved different-target events never merge.
- Guards: `startRecording` during playback fails; `startPlayback` with zero
  events fails; record calls while not recording are no-ops.
- **Deterministic replay**: with injected clock, record events at known times;
  drive `advancePlayback` with a scripted dt sequence (fixed 33 ms steps, then a
  seeded-random jitter sequence) → assert the fired sequence is exactly the
  recorded sequence, each event fires at >= its timestamp and within one dt step,
  auto-stop flips `isPlaying` after the last event. Same fired-sequence for both
  dt profiles = determinism proof.

Behavioral gate (Harmony, app-level): build + ctest green; scripted smoke — start
record (REST if step 7 lands, else UI), trigger 3 clips + drag a rack knob +
toggle an effect + jump a cuepoint, stop, save; inspect JSON (all captured types
present, param events coalesced, beat metadata populated when audio playing);
load, play — clips visibly retrigger in recorded order and knob/toggle states
change. Record-during-playback button is disabled (visual check).

## 9. Risk register

1. **Double-trigger during playback while user also performs.** Intentional: user
   input stays live during replay (VJ layers on top). Clip retrigger is benign
   (restart semantics). Param fights are last-writer-wins per frame; the rack
   knob guard (`isMouseButtonDown`, EffectsRackPanel.cpp:218) keeps drags visually
   stable. No suppression in v1 — document in panel tooltip.
2. **Timing drift / stalls.** Measured-dt eliminates systematic drift. macOS
   message-thread stalls (menu tracking, window drag, modal dialogs) pause the
   timer → events fire in a correct-order burst on resume. Mitigation: Save/Load
   disabled during playback; document. Trigger jitter is masked by live beat-snap.
3. **Dangling event pointers.** `advancePlayback` returns pointers into `events_`;
   any mutation while Playing is a use-after-free. Guarded by state machine + UI
   gating + debug assertions (§5). This is the highest-severity quiet failure —
   reviewer should verify every `events_` mutation site checks `!playing_`.
4. **Thread safety of capture.** All current capture paths land on the message
   thread (ApiServer/OSC hop via callAsync — ApiServer.cpp:319,
   MainComponent.cpp:1128). The recorder's CriticalSection covers any future
   off-thread caller; playback/dispatch is message-thread-only by design.
5. **Recording flood regression.** The boundary in §2.1 is convention, not
   compiler-enforced. A future contributor hooking `Effect::setParamValue` or
   `onParamChanged` would capture GL-thread automation at frame rate. The spec
   rule + a unit test asserting coalescing caps help; reviewer checklist item.
6. **Deck-layout sensitivity.** ClipTrigger stores layer/column (not clip id) —
   editing the deck between record and replay triggers whatever now occupies the
   cell (Resolume-style; acceptable, document). `deck_switch` capture keeps the
   active-deck context correct mid-set.
7. **Different audio at replay.** Beat-snap re-quantizes triggers to the live
   clock — visuals stay musical but timestamps shift up to one snap unit from the
   recording. Feature, documented.
8. **Wave collisions.** Wave-0 edits MainComponent.cpp (line drift — symbols cited
   throughout); Wave-1 B/D rewire OSC + TopBar transport in MainComponent. Build
   this after Wave-1; TopBar handlers join the §2.2#4 capture list then.
9. **Unbounded session size.** 8-hour set with heavy knob work ≈ 20 ev/s peak but
   only while dragging; realistic sessions are 10^3-10^4 events (~1 MB JSON). No
   cap in v1; if needed later, cap + warn at 250k events.
