# Facts: what quantises, snaps or waits for a beat today, and which adopted rows rest on it (s-rta-1004b)

READ-ONLY FACT SHEET. No design. Written by a fact-finder seat; nothing built, run or launched.

Pin check: `git -C /Users/boriskarpman/projects/RealTimeAudio diff --stat 7bc6df7 -- src tests docs CMakeLists.txt` printed nothing -> VERIFIED. Every code line below is a plain-file read of main (code = 7bc6df7). Paths relative to /Users/boriskarpman/projects/RealTimeAudio.

Labels: VERIFIED = I read it (file:line). INFERRED = reasoned from read lines, not run. UNKNOWN-NEEDS-A-RUN = named with the cheapest test.
Short names: RT = .harmony/.reports/s-rta-1003b/ruling-transport.md, RD = s-rta-1003b/ruling-transport-delta1.md, R2 = s-rta-1004/ruling-transport-delta2.md, RA = s-rta-1004/ruling-transport-answers.md, RN = s-rta-1004/ruling-nudge.md, RR = s-rta-1004/ruling-nudge-row.md, RB7 = s-rta-1002b/ruling-bf7.md.

HEADLINE (five facts a planner needs first)
1. A control labelled "Quantize:" with a combo "Off / Next Beat / Next Downbeat" IS in the top bar today. VERIFIED src/ui/TopBar.cpp:173-187.
2. The top bar's "/4 /2 x1 x2 x4" buttons are NOT quantise controls and do nothing: they write `composition_.bpmMultiplier` (saved in the show) and nothing in src reads it. VERIFIED (grep over src: only TopBar.cpp:401-426, Composition.h:142/229/741/879).
3. There are TWO live gates on a fire: the global `Composition::quantizeMode` (top-bar combo) and the per-clip `Clip::beatSnapMode` (Clip inspector combo "Snap Off / Beat / Bar / 2 Bar / 4 Bar"). A third, separate one belongs to routine pads (each routine's own `quantize`, default Bar, which waits for a bar even with global Quantize Off).
4. A take replay / routine replay of a clip fire goes through the SAME quantising handler (Origin::Replay, immediate=false), so a recorded press is quantised AGAIN when it is replayed. Only the checkpoint-0 restore fires "immediate".
5. The take recorder stores ONE time for a press: the moment of the press (stamped at the recorder's last tick). It never stores the time the quantised clip actually landed. (Q6.)

---------------------------------------------------------------------------------------------------

## 1 QUESTIONS ANSWERED

### Q1 ON SCREEN today

**Q1a. The top bar has a control named "Quantize". Yes.**
- VERIFIED src/ui/TopBar.h:117-119: `juce::Label quantizeLabel_{"", "Quantize:"}; juce::ComboBox quantizeSelector_;`
- VERIFIED src/ui/TopBar.cpp:172-187: items "Off" (id 1), "Next Beat" (id 2), "Next Downbeat" (id 3); default Off; `onChange` sets `composition_.quantizeMode = static_cast<Composition::QuantizeMode>(id - 1)` and calls `onQuantizeChanged` if set.
- VERIFIED src/ui/TopBar.cpp:601-603 (layout): label 55 px wide, combo 100 px, placed after the five multiplier buttons and before the right block (Master Signal / Master / Outputs / FPS / DSP).
- VERIFIED the model enum `Composition::QuantizeMode { Off, NextBeat, NextDownbeat }`, default Off (src/model/Composition.h:144-145; reset in `clear` at :230).
- VERIFIED nothing assigns `TopBar::onQuantizeChanged` (grep over src: declared TopBar.h:29, called TopBar.cpp:186-187, no assignment in MainComponent.cpp). The combo writes the model directly.
- VERIFIED the combo does NOT follow the model: its only `setSelectedId` is the constructor's (TopBar.cpp:181) (grep of "quantizeSelector_" over src). So a show or take that sets `quantizeMode` to 1 or 2 runs quantised while the combo still reads "Off" (INFERRED consequence; RB7 line 714 "H2" records the same defect, and RB7 AM8/T7 planned a fix that is NOT on main: main still has "Next Downbeat").
- VERIFIED the global value is also written by: loading a show (Composition.h:880) and a take replay's checkpoint-0 row (src/recording/Program.cpp:292-295 emits preamble row `comp/quantize` with `v = quantizeMode`; src/MainComponent.cpp:2012-2015 applies it).

**Q1b. The "/4 /2 x1 x2 x4" buttons.**
- VERIFIED TopBar.h:111-115 (`multDiv4Button_{"/4"}`, `multDiv2Button_{"/2"}`, `multX1Button_{"x1"}`, `multX2Button_{"x2"}`, `multX4Button_{"x4"}`), TopBar.cpp:156-170 and :401-427: a click sets `composition_.bpmMultiplier` to -4/-2/1/2/4, repaints the cyan highlight, and calls `onBpmMultiplierChanged` (never assigned).
- VERIFIED the field is persisted (Composition.h:741 write, :879 read; tests/test_composition.cpp:157,194 round trip) and has no reader anywhere else in src (also RN 1004 ruling Q62 text: "do nothing today"; ruling-one-save.md:887 SF-2 "`bpmMultiplier` likewise").
- VERIFIED the highlight is not re-synced from the model either (no setter besides the click), same as the Quantize combo.
- They do not quantise, snap or delay anything. They only share a row with the Quantize control.

**Q1c. Every other on-screen control that quantises / snaps / waits (label as painted, where, what it writes, values):**

| # | control | where | writes | values / notes |
|---|---|---|---|---|
| 1 | "Quantize:" combo | top bar | `Composition::quantizeMode` (saved in show: key `quantizeMode`, Composition.h:742) | Off / Next Beat / Next Downbeat. Forces a snap granularity on EVERY clip and column fire (Beat or Bar) but only while the tracker is LOCKED (MainComponent.cpp:36-43). Also the routine pad override (Q2). |
| 2 | Clip "Snap" combo (shown "Snap Off" when Off; tooltip "Quantize clip trigger to next beat/bar boundary") | Clip inspector, Autopilot section, in the row right of the autopilot-duration combo (ClipInspector.cpp:161-175, :746-751) | `Clip::beatSnapMode` and legacy `Clip::beatSnap = (sel > 1)` (ClipInspector.cpp:171-173) | "Snap Off", "Beat", "Bar", "2 Bar", "4 Bar" (ClipInspector.cpp:163-167). Always visible when a clip is inspected (no setVisible on it: grep). Saved per clip (Clip.cpp:72-73). Follows the model on refresh (ClipInspector.cpp:1428). |
| 3 | "Quantize" submenu of a routine pad's right-click menu (rows Off / Beat / Bar / 2 Bar / 4 Bar, ticked) | routine pad (ROUTINES row above the column numbers) (RoutineDeckView.h:243-262, DeckView.cpp:910-940) | `Routine::quantize` of that pad's routine (via REST-equivalent `perfRoutineSet`, MainComponent.cpp:6286-6288); saved in show (Routine.h:124) | Default per routine = Bar (Routine.h:21), so a pad press waits for the next bar with global Quantize Off. |
| 4 | Pad tooltip / waiting state "Starting on the next beat / two-bar line / four-bar line", "Restarting from the top ..." | routine pad (RoutineDeckView.h:106-120) | read-only text from the engine's `startsOn` | display only |
| 5 | Autopilot duration combo "Layer Determined / 1 Beat / 2 Beats / 4 Beats / 8 Beats / 16 Beats / 32 Beats" (ClipInspector.cpp:138-150) and the Layer's own default | Clip inspector, layer inspector | `Clip::autopilotDuration` | An auto-advance that lands on a beat crossing (Q2). Not a quantiser of a user fire. |
| 6 | "Random FX on Beat" toggle + beat-count combo 1/2/4/8/16/32; "Sync" button | MainComponent main window (MainComponent.cpp:200-232) | `beatRandomToggle_`, `beatRandomCount_`, `beatCounter_` (not saved) | Counts beats, then randomises effects (Q2). The "Sync" button resets the beat counter. |
| 7 | Slideshow "beats per image" (`slideshowBeats_`, default 8, MainComponent.h:342) | folder-slideshow (image loader) | in-memory | counts beat crossings (MainComponent.cpp:4518-4542) |
| 8 | Clip "Beats"/BPM-sync transport (Timeline vs BPM Sync, `beatDivision`, `videoBeats`) | Clip inspector | `Clip::transportMode/beatDivision/videoBeats` | A RATE only: video speed = videoBeats / beatDivision (Renderer.cpp:1657-1667); no phase lock, no wait. |
| 9 | TopBar "Tap" / "Resync" / "Manual" / "Link" | top bar | tempo | They move the beat grid every wait is measured against; they do not delay a fire. |

- No Preferences control, no menu-bar item, no layer-strip or deck-view control, and no binding-overlay field quantises. VERIFIED: grep of "quantiz|snap" over src/ui/PreferencesDialog.cpp, MenuBarModel.cpp, LayerStrip, ClipCell, DeckView (only the routine pad submenu, DeckView.cpp:921-938), BindingOverlay.cpp, MidiLearnOverlay.cpp, src/binding/*, src/midi/*, src/osc/* finds none. `AppSettings.h` has two keys only (`milkDropPresetDir`, `outputs`).
- Momentary pads: a binding's trigger mode "Toggle / Momentary" (src/binding/Binding.h:53-58) is not a quantiser, but a Momentary release acts on a QUEUED trigger (cancels it) (Q2).
- No UI shows a queued clip. VERIFIED: no "pending" in ClipCell/DeckView/LayerStrip; RT:140 (R8) and RB7:710 (H1) say the same. The only surfaces of the queue are REST `GET /api/composition` per-layer `pendingClip` (ApiServer.cpp:401-455) and the counter `render_pending_fired` in `/api/state` (ApiServer.cpp:1555; TestServer.cpp:691).

### Q2 THE ENGINE today (every path that makes an act wait for, or land on, a beat or bar line)

**The one queue.** All clip fires reach `Layer::triggerClip(ref, rows, forcedSnap)` (src/model/Layer.h:410-433).
- VERIFIED rule: `snapEnabled = forcedSnap != Off || target->beatSnapMode != Off || target->beatSnap`. If snapEnabled AND `ref != active ref`, the trigger is only QUEUED: one pending slot per layer (`pendingTriggerColumn`, deck id, snap override) in the layer's atomic tuple word (Layer.h:421-431, :34-53). Otherwise `immediateNext`.
- VERIFIED a re-fire of the clip that is already active is NEVER queued today (`ref != r.activeRef()` in the test): it retriggers at once even with Quantize on (Layer.h:423, :361-375). (RT AM-1 planned to change this; not on main.)
- VERIFIED drain: `Autopilot::processFrame`, on the render thread every frame, `beats = beatCrossings_.consume(snapshot.totalBeatCount)`; if beats == 0 it returns; else for every layer with a pending column it calls `processPendingTrigger(beatInBar, barCount, rows)` ONCE with the frame's snapshot (src/model/Autopilot.cpp:74-92; called from src/render/Renderer.cpp:513). `render_pending_fired` counts calls that changed the tuple (Autopilot.cpp:87, Renderer.h:569-572).
- VERIFIED what decides the line (Layer.h:449-482): a forced snap (global Quantize) wins; else the clip's own `beatSnapMode`; and if both are Off but the legacy `beatSnap` bool is true the queue uses Beat. Beat/Off: next beat crossing. Bar: `beatInBar == 0`. TwoBar: `beatInBar == 0 && barCount % 2 == 0`. FourBar: `... % 4 == 0`. A pending Off is treated as Beat (Layer.h:468-470).
- VERIFIED the global Quantize maps NextBeat -> Beat, NextDownbeat -> Bar; Off -> no forcing; and it is Off whenever `snap.trackerState != STATE_LOCKED` (MainComponent.cpp:36-43).
- VERIFIED a PER-CLIP snap has NO tracker-lock guard: only the global path has one. A clip whose own Snap is Bar, fired with no steady beat, is queued and no beat crossing may ever come, so it never drains (INFERRED consequence of Layer.h:421 + Autopilot.cpp:74-80; MainComponent.cpp:23-30 comment says exactly this hazard for the global path).
- VERIFIED cancels: `clearedNext` (clear, X) cancels the pending slot (Layer.h:376-390); `releaseMomentary` cancels a pending trigger of its own ref if released before the beat (Layer.h:509-526; MainComponent.cpp:7674-7688); a Remove Deck cancels pending into that deck (core/DeckCommands.h:195-210); a deck SWITCH cancels nothing (bf9b).
- VERIFIED the render thread tries at most 16 CASes, then waits for the next qualifying beat (Layer.h comment :330-340, Autopilot.cpp kRenderTriggerAttempts).
- INFERRED: Bar/2Bar/4Bar landing reads `beatInBar`/`barCount` of the frame that consumes the crossing; if a stall makes `beats > 1`, one call is made with the current beatInBar, so a bar line inside the gap can be missed until the next qualifying crossing (Autopilot.cpp:74-86). RN SF-1 (RN:780-783) and SF-3 (RN:789) record related lag cases (INFERRED there too).

**Every trigger path (all enter MainComponent::handleClipTrigger / handleColumnTrigger, which pass the same forced snap):**

| path | where | quantised by global Quantize? | by clip's own Snap? |
|---|---|---|---|
| mouse click on a cell / column header | deckView_->onClipTriggered/onColumnTriggered, MainComponent.cpp:687-692 | yes (handleClipTrigger :4846-4850; handleColumnTrigger :5075-5080) | yes (inside triggerClip) |
| computer-keyboard key and MIDI note/CC binding (TriggerClip, TriggerColumn) | handleBindingAction, MainComponent.cpp:7700-7760 -> same handlers | yes | yes |
| REST POST /api/trigger_clip, /api/trigger_column | MainComponent.cpp:1886-1887 -> same handlers (Origin::Human) | yes (RT:126 R5) | yes |
| OSC trigger clip | MainComponent.cpp:2318-2320 (callAsync -> handleClipTrigger) | yes | yes |
| autopilot advance | Autopilot.cpp:326/355/417 `triggerClip(..., BeatSnapMode::Off, ...)` | NO (forced Off) | YES: `triggerClip` still queues if the TARGET clip's own snap is on (Layer.h:420-421), then it drains on a later beat crossing (INFERRED: the code reads `target->beatSnapMode` regardless of caller) |
| routine pad press (perfRoutineFire) | MainComponent.cpp:6252-6256 -> RoutineEngine::fire | the routine's START waits on global Quantize if on, else the routine's own quantize (default Bar). See routines below. |
| a routine's / take's clip events while playing | `recorderHost_.dispatch.fire` "activeClip" -> handleClipTrigger(Origin::Replay, deck, immediate) (MainComponent.cpp:1960-1971) | YES, the same handler: a replayed fire is quantised again (immediate=false) | yes |
| checkpoint-0 restore (take preamble, routine restore) | `immediate = f.p.origin == Origin::Preamble` (MainComponent.cpp:1941) -> `triggerClipImmediate` / `clearActiveClip` (:4834-4841) | NO (bypassed on purpose, "not a performance trigger") | NO |
| drop of a file onto a cell / other model writes | not a trigger | n/a | n/a |

**Other things that wait for or land on a beat/bar line:**
- Routine pad (restore-then-replay on the next bar). VERIFIED src/recording/RoutineEngine.cpp:
  - `effectiveSnap(forced, own)` = the global forced snap if on, else the routine's own `quantize` (default Bar) (:181, :228-231; Routine.h:21). `dueNow` Off: now; Beat: totalBeatCount edge; Bar: totalBarCount edge; TwoBar/FourBar: bar edge plus `barCount % N == 0` (:233-244).
  - `fire`: if the tracker is not locked (`beatAvailable` false) the routine starts AT ONCE with a notice (:757-766); with an effective Off it starts inside the call; else it is PENDING until the tick that finds `dueNow` (:564-570). A fire while running = restart at the next boundary; while pending = no-op (:683-704).
  - The restore half-glide is scheduled to END on the boundary (`beatsUntilBoundary`, :246-258; `scheduleGlides`) and a Jump routine restores in one call ON the boundary.
  - Edits of the routine's Quantize while pending re-time it (`resyncPending`, :489-523).
  - Routine's loop return lands on its own `lengthBeats` (whole bars, RoutineSlice.cpp:291-294).
  - With the setting at its smallest (Off) it starts inside `fire` (:769-770); with global Quantize Off the routine is NOT immediate unless its own quantize is Off.
- Autopilot beat mode and end-of-clip: lands on beat crossings (Autopilot.cpp:74-140): `beatsPlayed += beats`; when `>= targetBeats` the advance fires on that frame; per-type timing (`getPerTypeBeats`) too. The advance itself is immediate (snap forced Off) -- unless the target clip's own Snap queues it (above). The per-clip duration is a constant list (1/2/4/8/16/32 beats), a per-layer default, or per-type.
- Transitions (clip-to-clip fade): NOT beat-aligned. `Layer::transitionSpeed` is a DURATION in seconds; `LayerClock::advanced` steps by dt/seconds (render/LayerClock.h:10-25); `<= 0` falls back to 0.5 s; `<= 0` in `immediateNext` = a cut (Layer.h:372). VERIFIED.
- Beat-synced randomisation: `beatSyncRandomize()` consumes beat deltas, `beatCounter_ += beats`, randomises when `>= beatRandomCount_` (combo 1/2/4/8/16/32, default 4) (MainComponent.cpp:4683-4700), driven only `if (beatRandomToggle_.getToggleState())` in the 30 Hz timer (:4363-4364). Not tied to Quantize. VERIFIED.
- Slideshow: `advanceSlideshow` counts beat deltas, default 8 (MainComponent.cpp:4518-4542). VERIFIED.
- A momentary key's release: Momentary binding release -> `releaseMomentaryRefs` -> `Layer::releaseMomentary` (one CAS): clip active -> cleared (at once, not on a beat); clip still queued -> its queue cancelled (MainComponent.cpp:7674-7688, :7726-7731, :7764-7771). The release is NOT recorded (no capture there). VERIFIED.
- BPM-synced video (the "beat lock"): there is NO lock on main. A BPM-synced clip gets a RATE only: video `setSpeed(videoBeats / beatDivision)` when `snap.bpm > 0`; sequence fps = numFrames * bpm / (beatDivision * 60) (src/render/Renderer.cpp:1657-1667, :1718-1736). Its phase is whatever the fire/drop left. The "lock", "cut on the next 1" and "slide" in the rulings are planned (transport lane S4c: RD DA-1, RA TB-1/TB-2), not code. VERIFIED by reading Renderer.cpp.
- A legacy beat-phase SEEK on trigger: `if (clip->beatSnap && clip->isPlayable())` sets `clip->playheadPosition = snap.beatPhase` and seeks the video/sequence to that fraction (MainComponent.cpp:4866-4889). It runs on the clip that is ACTIVE after the trigger (`t.after.activeRef()`), so when the fire only QUEUED, it runs on the previously active clip if that clip has beatSnap. VERIFIED reading; consequence INFERRED. (RT:972 and RD:346 call it "the per-clip beat-phase seek" the transport lane removes.)
- `hasBeenTriggered` is written only in handleClipTrigger for the clip active after the press (MainComponent.cpp:4866 region); a queued clip that drains later is activated by `applyActivationTail` with `hasBeenTriggered` still false, so it auto-plays on every drained activation (INFERRED from Layer.h:549-556 and RT:R4; not run).

**What "off" / "smallest" does:**
- Global Quantize Off (default) -> forcedSnap Off -> no queue unless the target clip's own Snap is on.
- Clip Snap "Snap Off" and `beatSnap` false -> no queue from the clip. Smallest = "Beat" -> next beat crossing (up to 1 beat, 0.5 s at 120).
- Routine quantize Off -> starts inside `fire`; smallest queued = Beat.

### Q3 SAVED STATE AND ROUTES

**Stored fields (all VERIFIED unless stated):**

| where | field | write / read | notes |
|---|---|---|---|
| show file (Composition::toVar) | `quantizeMode` int 0/1/2 | src/model/Composition.h:742 / :880 | reset to Off on `clear` (:230); read with no hasProperty guard |
| show file | `bpmMultiplier` int | Composition.h:741 / :879 | does nothing (Q1b) |
| show file, per clip | `beatSnap` bool, `beatSnapMode` int 0..4 | src/model/Clip.cpp:72-73 / :229-233 | legacy upgrade: `beatSnap && mode == Off` -> Beat. `Clip::clearRuntime/reset` keep or zero it (Clip.h:267,317,357-358) |
| show file, per routine | `quantize` string off/beat/bar/2bar/4bar | src/model/Routine.h:124 / :159-160 (default Bar; unknown string = Bar) | saved with the composition |
| take checkpoint 0 (PerfState) | `quantizeMode` int; per layer `pendingTriggerColumn`, `pendingTriggerSnapOverride` | src/recording/PerfState.cpp:86-87,121-122,192,225; PerfStateCapture.cpp:53,70 | the take restores the global Quantize at replay start (Program.cpp:292-295, MainComponent.cpp:2012). A queued trigger at record start is stored in the checkpoint; whether the preamble restores a pending slot: UNKNOWN (no read of Program.cpp's pending handling) |
| take lanes | the global `comp/quantize` is NOT a recorded lane; no capture site writes it (grep) | RoutineSlice.cpp:36 drops `quantize`/tempo/audio/activeDeck lanes by name for routines | VERIFIED |
| settings.json | none | model/AppSettings.h keys: `milkDropPresetDir`, `outputs` only | VERIFIED |
| presets/bindings | none (no Binding action or field for quantize) | src/binding/Binding.h:24-48 | VERIFIED |
| runtime only | layer pending slot (not in show; in take checkpoint), `beatRandomCount_`, `slideshowBeats_` | | `beatCounter_` etc. not saved |

**Routes that set or read one:**
- REST `POST /api/routine/save` and `POST /api/routine/set` take `quantize` = off|beat|bar|2bar|4bar (ApiServer.cpp:2498-2542, :2627-2633; MainComponent.cpp:6206-6207, :6286-6288). `POST /api/routine/fire` fires a pad (quantise by routine/global). `GET /api/routine/status` reads `quantize`/`startsOn` (MainComponent.cpp:6412).
- REST `GET /api/composition` reads per-layer `pendingClip` (ApiServer.cpp:455). `/api/state` reads `render_pending_fired` (ApiServer.cpp:1555; test server 691). It does not print `quantizeMode`, `bpmMultiplier` or `beatSnap` (grep).
- REST `POST /api/load_composition` (and Open) sets all the show-file fields. `POST /api/set_clip_param` accepts only `fitMode` (R2 V7, ApiServer.cpp:729-733).
- OSC and MIDI/key bindings: NO route sets or reads Quantize or a clip's snap. OSC has routine fire (MainComponent.cpp:2370-2373). VERIFIED by grep over src/osc, src/midi, src/binding.
- Only three writers of the global Quantize exist: the top-bar combo, a loaded show, a replayed take's preamble.
- INFERRED: probes set Quantize via a loaded show's `quantizeMode` or beatSnapMode in the composition JSON (probe-boxes.py:1074, probe-tsan.py:119-128); nothing in src/test exposes a quantize setter (grep).

### Q4 TESTS AND PROBES that pin quantised or snapped behaviour (what a removal turns red)

Ctest (tests/):
- tests/test_undo_commands.cpp: `Layer::triggerClip: forced snap queues a non-active column instead of firing immediately` (:2717); `Layer::processPendingTrigger: forced override picks granularity independent of the clip's own beatSnapMode` (:2731); `Composition::triggerColumn: forced snap queues on every non-ignoring layer, skips ignoring ones` (:2760); `TriggerClipCmd: undo of a queued forced-snap trigger restores pendingTriggerSnapOverride too` (:2776); `Layer::clearActiveClip: cancels a pending trigger too` (:2927); `D1c a trigger's first perform after its queued trigger fired keeps it fired` (:3024); `RemoveDeckCmd/AddDeckCmd/InsertDeckCmd ... queued trigger` (:1646, :1824, :2815, :2844, :2875); `TriggerClipCmd: pendingTriggerColumn-only change pushes, merges, round-trips` (:2656). All assert the forced-snap/queue API (`Beat`, `Bar` arguments, `pendingTriggerSnapOverride`). A removal of the forced snap or the queue turns these red or uncompilable.
- tests/test_layer_runtime.cpp: `a queued trigger cancelled by cancelPendingInto never fires` (:341); `releaseMomentary: a release before the beat cancels the queued trigger` (:457); `...press A, press B, release A leaves B queued` (:474); `Layer triggers return the exact transition pair` (:245), `LayerRuntimeCell pack...` (:66, :92, :177: the 4-bit snap override is packed in the word).
- tests/test_layer_runtime_race.cpp: `R1 message-thread triggers vs render clock / autopilot` (:127; uses `Beat` snap every 5th trigger :208) and `R-bf9b fenced box and stack edits...` (:393; `comp.fire(..., Beat)` :503): a queue/drain race stress.
- tests/test_show_model.cpp: `T5 a bar-snapped trigger queued on deck 0 survives a switch to deck 1 and fires on its bar (bf9b, K5)` (:474; `beatSnapMode = Bar`, `processPendingTrigger(2,0)` waits, `(0,1)` fires); `T6 Remove Deck ... a queue into it cancelled` (:503); `T6e ...` (:602); `T7 ...` (:804).
- tests/test_shared_field_types.cpp:19 (a lint over `pendingTriggerColumn`/`pendingTriggerSnapOverride` field types, Pitfall 63).
- tests/test_composition.cpp `Composition JSON roundtrip` (:151-205): `bpmMultiplier = 2`, `quantizeMode = NextBeat`, `clip.beatSnap = true` all round trip.
- tests/test_program_preamble.cpp `Program::compile builds the preamble from checkpoint0 in restore order` (:46-104): `comp/quantize` row with `v == 2`. tests/test_take.cpp `Player::firePreamble fires discrete then continuous exactly once per call` (:1076-1100, a `quantize` key). tests/test_recorder_host.cpp `RecorderHost::play restores checkpoint 0 ...` (:616, comment :738 "8 discrete entries (activeDeck, quantize, ...)" -- the count of 8 rests on the quantize row).
- tests/test_master_signal_link.cpp: `layout: the Signal fader sits to the right of Quantize with no overlap on Master` (:177), `layout at 1728 ...` (:292), `layout at the minimum window width (1280) ...` (:331), `bf9b S3.3: the TopBar has no deck Fade control` (:366-386) all read `bar.quantizeSelectorBoundsForTest()` (TopBar.h:62): removing the widget breaks these tests' compile and their reference point.
- Routine quantize (a different setting): tests/test_routine.cpp `Routine: composition round-trip, legacy file, unknown quantize, orphan bank entry` (:140); tests/test_routine_deck_view.cpp (pad menu rows `QuantizeOff..FourBar` :246-265, `settingsChangeFor` :301-309, fixture `quantize = "bar"` :26, :246) and tests/tool_routine_deck_snapshot.cpp:55; tests/test_routine_engine.cpp: 74 mentions, among them `RoutineEngine: waits for the next bar, restores, then plays...` (:268), `re-fire restarts on the next bar; global Quantize overrides; no beat starts at once` (:425), `quantize Off and Beat; 2 Bar and 4 Bar parity comes from barCount` (:675), the glide cases G2/G3 (Quantize Off spill, :957, :990), `the 2 Bar and 4 Bar boundary prediction` (:1660), display D1/D6/D7 (`startsOn`, an edit while waiting, :1713, :1914, :2034), and `a Beat-quantized start survives a tick gap that swallows the beat edge` (:2206). These stay red/green on the ROUTINE engine regardless of the clip/top-bar removal; they turn red only if routine quantize is removed too.
- tests/test_autopilot.cpp has no quantise case (`beatSnap(...)` there is a snapshot helper's name, :231). tests/ShowFixture.h:59 fires with `BeatSnapMode::Off`.
- No test targets the TopBar combo's behaviour itself (bf7 AM8's `test_topbar_quantize_sync.cpp` was never built).

Probes (.harmony/):
- probe-tsan.sh/.py scenario d: fixture sets one clip `beatSnapMode Beat`, one `Bar`; the VALIDITY line requires `render_pending_fired >= 1` AND `render_autopilot_advances >= 1` (probe-tsan.sh:22; probe-tsan.py:16, 21, 111-128, 147). Removing the queue makes this validity gate fail.
- probe-boxes.py K5 `k5_queue_link_off` (:1067-1097): a Bar-snapped trigger (`beatSnapMode=2`) queued at beatInBar 1, deck switch, fires ON the bar; `k5_queue_link_on` is BLOCKED (no Link build).
- probe-routine-display.sh d2/d13/d15 (:20-29, :196-240): routine pad `quantize` 4bar/bar `startsOn` and edits while waiting.
- probe-idle-paint.py:342 (routine `quantize: "off"` in a fixture); probe-vupload.py:489 and make-bf9b-check.py:52,61 write `quantizeMode 0`, `beatSnapMode 0`, `bpmMultiplier 1` into fixtures (a removed field would only need these keys dropped; INFERRED: the loader reads `quantizeMode` with no guard, Composition.h:880, so a removed key makes the load read 0 or garbage -- UNKNOWN-NEEDS-A-RUN, see section 4).

### Q5 THE ADOPTED RULINGS (table below, then one line per stage)

Convention: "lane" = TRANSPORT (RT, RD, R2, RA), NUDGE (RN), NUDGE-ROW (RR), BF7 (RB7). Files one-save, outputs, effect-looks, looks-answers: see the last block (no row).
Precedence (R2 header): R2 > plan-transport-delta2 > RD > RT > plan-transport; RA's rules are TB-1..TB-14 over R2. RB7 predates BF20 and the Quantize ruling and is not on main.

TABLE Q5 (file:line | lane | STAGE | row/amendment id | quoted phrase | what it assumes about Quantize)

| file:line | lane | stage | id | quote (<= 25 words) | assumes |
|---|---|---|---|---|---|
| RT:389-392 | transport | S2 | AM-1 | "a queued re-fire changes the pending slot and nothing else ... the restart happens on the release." | A re-fire of the ACTIVE clip is queued under Quantize / own Snap and restarts on the bar; needs the queue to exist and `triggerClip`'s `ref != activeRef` test removed (RT:386-388) |
| RT:831-834 | transport | S2 | TL-U3, TL-U3b, TL-U3c | "with a forced snap a re-fire of the active clip is queued: stamp, play state and playhead stay until the release" | unit cases for the queued re-fire and a momentary release while it is queued |
| RT:967-975 | transport | S2's rows live in S5 (RT) = S6 (R2) probe | TR6, TR6b | "A ramp whose own Snap is Bar ... Fire; inject the downbeat; ... fire again; then: `pendingClip` equals the active ref" | live rows that need a clip with its own Snap = Bar and the `pendingClip` REST field |
| RT:972 | transport | S2 | TR6 RED arm | "the old app seeks a Snap clip to the injected beat phase as a file position, MainComponent.cpp:4868-4889" | the per-clip beat-phase seek is to be deleted with Snap |
| RT:276, 767-768 | transport | S2 / bars lane | handler lines | "the beat-phase seek of a Snap clip" / "a re-fire is queued like any fire ... the per-clip beat-phase seek is gone" | the per-clip Snap exists as the way a fire waits |
| RT:461, 494-502 | transport | S1 | F13 / AM-8 "live" | "the layer's tuple (active clip, previous clip while its fade runs, queued clip)" | a QUEUED clip counts as live for Undo; K6 below |
| RT:818-819 | transport | S1 | TL-UC14, TL-UC15 | "Undo of Add Deck and of Load Deck with a trigger queued into the deck retires it and the trigger fires" | needs the queue |
| RT:1260 | transport | S1 | K6 | "The queue now counts as live for decks (AM-8): a deck Cmd+Z takes away can still start a clip on the next bar." | a pending slot keeps a deck alive |
| RT:126, 304 | transport | S0/S2 | R5, ST-9 | "Quantize is Off unless the tracker is LOCKED (MainComponent.cpp:36-43)" / "no lock, no Quantize" | Boris check 3 could not be done without a lock |
| RT:629-630 | transport | S2 | AM-23 | "with no steady beat the app cannot wait for a bar: a re-fire restarts at once" | the lock guard of the global Quantize; STANDS in R2 (R2:74) |
| RT:1122-1125 | transport | S5 -> S6 (Boris page) | check 3 | "Set Quantize to 'Next Downbeat'. Pick a column ... then they all start over together. With 'Next Beat' they start over on the next beat." | the top-bar Quantize combo and its two items are on screen for Boris's check |
| RD:312 | transport | S4c | RD-13 | "The lock reads `frameSnap_`, the snapshot the quantized release got in that frame" | the lock shares the snapshot of the queue's drain |
| RD:345-347 | transport | S4c | DA-1 | "A fire is never moved to fit the beat: the clip starts ... at once, or on the bar under Quantize." "Under Quantize the clip starts on the frame of the beat" | a Quantize landing is one of the two ways a BPM clip starts |
| RD:612, 1179 | transport | S4b(bars) | DA-8 note, NOT IN THIS LANE | "The per-clip Snap setting and the Quantize menu name bars today; they launch a clip, they do not set it; outside this lane." | names both gates, leaves them alone |
| RD:909 | transport | S4c / S5 | TR15 | "BPM-synced ramp ... Quantize off; fire at driver phase [0.20, 0.30]" | the row works with Quantize off (default) |
| RD:979 | transport | S4c | FM-4 | "fired on a downbeat with Quantize on; in the fifth minute 20 reads" | a measurement fixture that gets the clip onto the "1" with Quantize on |
| RD:998-1000, 1049 | transport | S5 (Boris page) | checks 3, 28 | "With Quantize off, fire that clip between two beats." | Boris checks use the combo |
| RD:1149-1162 | transport | S4c | KR-6, KR-10 | "After a drop, a Resync or an unquantized fire the loop's first beat can sit on beat 2, 3 or 4" / "the phase the quantized release uses" | named risks; the lock's phase is the release's |
| R2:212 | transport | S4c (V17) | fact | "ClipInspector.cpp:162-167: Snap Off, Beat, Bar, 2 Bar, 4 Bar. Only Bar and above land on the '1'." | per-clip Snap Bar/2/4 is how a fire gets on the "1" |
| R2:304 | transport | SM-b (M2, M3, M5 setup) | measurement | "A clip is fired 'on a 1' by its own Snap = Bar in the composition." | the production-mode runs fire the probe clip through the clip's own Snap; M1, M4, M6-M8 use an injected driver (R2:300-301) |
| R2:724-725 | transport | S4c / S6 | Boris check 5 | "Fire a BPM-synced clip in the middle of a bar with Snap off ... With Snap on Bar it starts on the '1' and nothing slides." | check text names the clip's Snap |
| R2:742-743 | transport | S4c | question 71 text | "with Snap on Bar a fire lands on the '1' and nothing slides" | question text told to Boris |
| R2:841 | transport | S4t | K-10 | "S4t changes the bar's beat after a Tap for EVERY reader (the wheel, Bar-quantised fires, phrases)" | the Bar-quantised queue is one of the readers of the bar position |
| RA:253-254 | transport | S4c | TB-2 cut table | "a fire that lands on the '1' (Snap Bar, 2 Bar, 4 Bar) | no" / "a fire off the '1' (Snap off or Beat) | if more than 0.10 beat off" | the table's two fire rows split on per-clip Snap; with every fire immediate only the second row exists |
| RA:618-622 | transport | S6 | TR23 | "Without S4c: fired on a '1' (Snap Bar), wait 4 beats." | the live row uses a clip with Snap Bar |
| RA:671-673 | transport | S6 (docs) / S4c | D-6 | "after every fire that is not on the '1' (with Snap on Bar a fire lands on the '1' and nothing cuts)" | docs sentence names Snap |
| RA:20-60 (cut table) others | transport | S4c | TB-1..TB-2 | "ONE cut on the next '1'" for drop, play, Tap, nudge | NOT Quantize: a seek onto the bar, not a wait for a fire (listed so it is not mistaken) |
| RN:31, 78, 90 | nudge | S1 | A1 / V3 | "Readers at a count edge take the bar position from the SAME snapshot: Autopilot.cpp:74 ... processPendingTrigger(snapshot.beatInBar, snapshot.barCount ...)" | the queue's drain is one of the readers the nudge arithmetic must keep consistent |
| RN:189 | nudge | S1 | BE-3 | "ACCEPTED: the Auto unit case with a Bar snap, live arms for Next Downbeat and Bar snap." | unit + live arms use the queue |
| RN:524-529 | nudge | S1 | T-N5b | "A BAR SNAP IN AUTO. ... with a pending Bar-snap trigger consumed as Autopilot does ... `Layer::processPendingTrigger(beatInBar, barCount, ...)`" | calls the real `processPendingTrigger` (or its two-line rule) |
| RN:639-655 | nudge | S2 (Harmony's live rows after S2) | L3a, L3b, L3c, L3d | "L3 A QUANTISED FIRE LANDS ON THE SHIFTED BEAT; A FIRE BY HAND DOES NOT WAIT." | 4 live rows: Quantize "Next Beat" (30 fires), "Off" (30), "Next Downbeat" + clip Snap Bar (12), Auto info; "The routes that set Quantize and read the playing clip are the ones probe-boxes.sh uses" |
| RN:700-701, 707-708 | nudge | S5 / VG (Boris page) | B1, B4 | "a quantised clip still fires on the old beat while the effects pulse on the new one" / "Set Quantize to Next Beat and fire -> the clip starts on the nudged beat." | Boris checks use the combo |
| RN:759-761, 780-783, 789-790, 824-826 | nudge | filed side findings | H-7, SF-1, SF-3, R4 | "In Auto a bar-quantised fire can land one beat AFTER the downbeat" | records main's bar-quantised lag; filed, not built |
| RN:738 | nudge | S3b | question 62 B | "the nudge then sits further right, after Quantize" | layout anchor |
| RR:18 | nudge-row | S3m / S3r | constraint | "Manual, Link and Quantize stay to its right (reading R69)" | Quantize is the right-hand end of the tempo cluster |
| RR:49-53 | nudge-row | S2r | rule 2 | "A QUANTISED CLIP FIRE is queued (V8) and drains only on a totalBeatCount edge (V9). Held timer: no edge: it WAITS." | the new Stop/Pause timer makes a queued fire wait; "Quantize Off: at once, as always" |
| RR:164-166 | nudge-row | S2r | V8, V9 | "a fire is queued unless Quantize is Off or the tracker is not LOCKED" | facts |
| RR:390-395 | nudge-row | S2r + VG | NA-10 | "A QUANTISED FIRE WAITS: A READING, NOT A QUESTION" ... "told as reading R81 and checked in B-R5" | the R81 reading; the visual gate R9b captures a waiting pad |
| RR:566-575 | nudge-row | S2r (LR2 b, c) | LR2 | "Manual 120, Quantize 'Next Downbeat' ... (b) A CLIP WAITS ... for 1.0 s the layer's playing clip is not the new one (reading R81)." | live row needs the combo and the queue |
| RR:424-428 | nudge-row | S1r/S2r | NA-16 | "A waiting CLIP lands on the play press. A waiting ROUTINE starts on the first bar line after play." | clip wait AND routine bar start |
| RR:91-96 | nudge-row | S3m, S3r | layout | "Quantize 161 -- to x = 1185 ... the Quantize combo ... never shed" | the row's width arithmetic includes a 161 px Quantize cluster; "Quantize:" caption is shed 2nd-4th |
| RR:460, 527-538 | nudge-row | S3m, S3r | T-RB1..T-RB5, T-RW1 | "the cluster from the routines stop to Quantize in `resized()`"; `fitTopBar`: "DSP, FPS, the caption, the state word" shed | layout code and unit bars are written against the Quantize combo |
| RR:693-696, 710 | nudge-row | S5 / VG (Boris page) | B-R5, B-R10 | "Stopped, with Quantize on, fire a clip -> it waits; press play -> it starts on that press." | Boris checks |
| RR:757-760, 785-786 | nudge-row | S2r / Boris questions | R81, R83, HR-9 | "R81 Stopped or paused with Quantize on: a clip you fire waits and starts when you press play" | the reading Boris VOIDED (BD:975-976: "tempo-row reading R81 is VOID") |
| RB7:20, 94 | bf7 | S4 | verdict | "The global Quantize gets only two changes: the relabel ('Next Bar') and the fix that makes the combo follow the model." | relabel + follow model |
| RB7:295-311 | bf7 | S4 | AM8 | "TopBar.cpp:178-180 items become 'Off', 'Next Beat', 'Next Bar' ... Add a PUBLIC TopBar::syncQuantizeFromComposition()" | the combo exists |
| RB7:380, 565-572 | bf7 | S4 | T7 | "T7: the Quantize combo follows the model" | unit case on the combo |
| RB7:421, 440 | bf7 | S4 (docs) | AM18, row 2 | "Quantize reads Off / Next Beat / Next Bar and follows the model." | docs text |
| RB7:586, 348-352 | bf7 | S2 | G3 lint (b') | "(b') 4 hits: '2 Bar' and '4 Bar' at ClipInspector.cpp:166-167 and RoutineDeckView.h:278-279." | the clip Snap list and the routine Quantize menu are relabelled in bars |
| RB7:596, 602, 644-649 | bf7 | G5/G7 gates after S1-S4 | visual gate | "TopBar with Quantize at 'Next Bar'" / "the snap combo" / "TopBar: the Quantize combo" | the gate renders the combo |
| RB7:606-609, 623 | bf7 | G5 | live check | "Load a composition fixture with quantizeMode 2 ... the Quantize combo reading 'Next Bar' straight from the file" ; BLOCK rule 5 | combo shown, follows the file |
| RB7:662-668 | bf7 | Boris Q2 | question | "Should the top-bar Quantize also offer 'Next 2 Bars' and 'Next 4 Bars'?" | wait up to 8 s; BF20 (1 bar, 1/2, 1/4, 1/8, 1/16) is Boris's later change (backlog line 158) and has NO ruling |
| RB7:710-715 | bf7 | findings H1, H2 | | "A queued clip trigger is invisible ... The global Quantize combo has never followed a loaded composition" | facts, agree with this sheet |

Rulings with NO row that names Quantize / per-clip Snap or relies on a fire waiting (grep'd for quantiz|snap|beat edge|bar line|queued|downbeat|next beat|waits for, then read hits): ruling-one-save.md (only unrelated "queued" / "waits for a save"; its live rows fire with the default Quantize Off: OS-L5 `midi {41,1,on}` ruling-one-save.md:690-692, OS-L11 `/api/trigger_clip` :712 -- they need a fire to land at once, which holds only while Quantize is Off and no clip has Snap; its SF-2 :887 notes `bpmMultiplier` is read by nothing); ruling-outputs.md (B-1 :779 "a clip that flashes on the beat" is a Delay check, not a fire wait); ruling-effect-looks.md (none); ruling-looks-answers.md (:141, :341 "Timeline" envelope on the beat clock is a signal source, not a fire).

PER-STAGE CLASSIFICATION

TRANSPORT lane (stage keys of R2 section 4; the base stage lists are RT:708-790 and RD:673-745; RA:467-506 changes some)
- S0 -- names it only in passing (RT:710-725 S0 lists "the readers of the pending slot (R8)" and "every probe that fires a clip twice and expects it to resume"; R2 S0 inherits that list; no Quantize behaviour built).
- SM-a -- does not touch it (a poller and offline law script, no clip fire).
- S4t -- names it only in passing (R2:841 K-10: Bar-quantised fires are one reader of the bar the Tap moves).
- SR -- does not touch it.
- SM-b -- rests on per-clip Snap for the setup of the production-mode runs M2, M3, M5 (R2:304); the law functions and M1/M4/M6-M8 (injected driver) do not.
- S1 -- RESTS on the live queue (RT:461, 494-502 AM-8/F13, TL-UC14, TL-UC15, K6): a queued clip counts as live for Undo; a trigger queued into a deck is cancelled/fired by Undo.
- S2 -- RESTS on the live queue: AM-1 (queued re-fire), TL-U3, TL-U3b, TL-U3c, TR6, TR6b, AM-23, the removal of the per-clip beat-phase seek; also touches the same MainComponent handlers.
- S3 -- does not touch it (INFERRED: no hit in any of the five files).
- S3h -- does not touch it.
- S4a -- does not touch it (bars engine; RD:612 "the Snap and Quantize menus ... outside this lane").
- S4d -- does not touch it.
- S4e -- does not touch it (BeatLoopr cuts on bar lines are seeks, not fire waits).
- S5a -- does not touch the behaviour; INFERRED it edits ClipInspector.cpp, the file that holds the Snap combo (ClipInspector.cpp:161-175, :751): a file-collision, not a dependency.
- S5b -- does not touch it.
- S4c -- RESTS on it in rows: RA:253-254 (cut table's two fire rows split on Snap Bar/2/4 vs Snap off/Beat), R2:212 V17, R2:724-725 check 5, R2:742 q71, RA:671-673 D-6, RD:312 RD-13 (lock reads the snapshot the quantized release got), RD:345-347, RD:979 FM-4. The lock/cut law itself does not need Quantize.
- S6 -- RESTS on it in probe rows TR6, TR6b (chain rows, `EXPECTED_ROWS` 18/30 pins) and TR23 (RA:618-622), and in the docs sentences (D-6) and Boris's page check 3 (RT:1122, RD:998).

NUDGE lane (stage keys of RN:469-496 and RR:446-480; the user's list is the union of both tables)
- S1 -- names it: T-N5b (RN:524-529) and BE-3 call the real `processPendingTrigger`; pure arithmetic otherwise.
- S1r -- names it only in passing (RR:424-428 NA-16; the timer's start hop counts a beat not a bar).
- S2 -- RESTS on it in Harmony's live rows L3a-L3d (RN:639-655) and the RED arm (build-mut-bypass).
- S2r -- RESTS on it: LR2 (b)(c) (RR:566-575), NA-10 (RR:390-395), R81/R83 (RR:757-760), rule 2 (RR:49-53).
- S3a -- does not touch it (hooks only; INFERRED the `ui_text` dump lists the top bar's widgets including the Quantize combo: RN:469-490 "every top-bar widget").
- S3m -- RESTS on the Quantize combo's width: fitTopBar/T-RB4 numbers (RR:91-96, :527-538).
- S3r -- RESTS on it: the row is laid "from the routines stop to Quantize in `resized()`" (RR:460), keeps the combo never shed (RR:95-96).
- S4 -- does not touch it (key and pad targets).
- S4r -- does not touch it.
- S5 -- names it in passing (docs: "a routine fired while stopped starts on the first bar line after play", RR:464; manual wording R81 voided).

BF7 bars lane (not merged): S0/S1/S3 do not touch it; S2 names it only in passing (G3 lint names the Snap list and routine menu labels, RB7:586); S4 RESTS on live Quantize (the whole stage).

### Q6 A RECORDED PRESS: what the take recorder stores about a fired clip's time

- VERIFIED the capture site: `handleClipTrigger` / `handleColumnTrigger` capture one `DiscretePoint` on key `layer/activeClip` with `v = column`, `retrigger = wasRetrigger`, `origin`, and for a column fire a shared `group` id (MainComponent.cpp:4986-5001; :5105-5120). The capture runs right after `composition_.fire/triggerColumn`, so a press that was only QUEUED is captured exactly like one that landed. There is no capture at the drain (the drain runs on the render thread, Autopilot.cpp:74-92, and `recorderHost_.capture` is message-thread only: RecorderHost.cpp:666-672).
- VERIFIED what a point stores (src/recording/Lane.h:10-30, 79-105; PerformanceRecorder.cpp:25-41): `s.seq` (global capture order), `s.t` seconds since recording start (wall clock), `s.sample` audio samples delivered, `beat` = the recorder's continuous beat time (totalBeatCount + beatPhase integrated, RecorderClock.h:25-35), `bpm`, `origin`, `v`, `action`, `retrigger`, `group`, `via`.
- VERIFIED the stamp is `clock_->now()`, i.e. the reading of the 120 Hz message-thread tick that came BEFORE the press, not an exact press moment (PerformanceRecorder.cpp:31; RecorderClock::now returns `current_`). So a press is stamped up to about one tick (about 8 ms) early (INFERRED size, from "120 Hz" in RoutineEngine.h:112 and RecorderClock.h:36-40).
- So: the press time is stored in seconds AND samples AND beats; the time the clip landed after snapping is NOT stored in any field. The pending state is not recorded either (only at checkpoint 0: `pendingTriggerColumn`, `pendingTriggerSnapOverride`, PerfState.cpp:86-87).
- VERIFIED replay uses the press time: `pickAt` returns `s.t` (Wall), `beat` (Beat; routines) or `s.sample` (Sample) (src/recording/Program.cpp:540-551, :591) and the event then fires through the SAME quantising handler (MainComponent.cpp:1960-1971), so with Quantize on the replayed fire is quantised again. A routine cut from the take keeps `beat` of each point relative to the cut (compileRoutine uses `DriveClock::Beat`, Program.cpp:712-713); no event time is rounded (RoutineSlice.cpp only rounds the routine's LENGTH up to whole bars, :291-294).
- VERIFIED the recorder does not record: the global Quantize value as a lane (dropped by name from routines, RoutineSlice.cpp:36; only the checkpoint stores it), a Momentary release (no capture in releaseMomentaryRefs), or a fire of Origin::Replay (RecorderHost.cpp:669).
- VERIFIED a clip's auto-play at the press is captured as its own `playing` point ("resume") at the press time, even for a queued fire (`captureAutoPlay`, MainComponent.cpp:5003-5004, :5014-5023); the actual auto-play lands later at the drain (Layer.h:549-556). INFERRED consequence: on replay with Quantize off the play point and the activeClip point are both at the press time.
- Boris's wish ("it will clip to the exact bar or beat in the recording", BD:967-971) therefore has what it needs in `s.t` / `beat` of every press (raw, un-snapped); NO landed time exists to compare against.

---------------------------------------------------------------------------------------------------

## 3 WHAT A PLANNER MUST NOT ASSUME

1. The top-bar "Quantize" is not the only gate. A clip with its own Snap (Beat/Bar/2 Bar/4 Bar) queues a fire with the global combo at Off. R87's "each clip's own beat-snap setting goes too" is a second removal (ClipInspector combo, `Clip::beatSnapMode`, `beatSnap`, the show-file keys, the `Layer.h` queue's OWN path). Removing only the combo leaves queued fires.
2. The queue is not only for clicks. Autopilot's `triggerClip` passes forced Off but still queues if the TARGET clip has Snap on (Layer.h:420-421). A routine's or a take's replayed clip fire goes through `handleClipTrigger(..., immediate=false)`, so it is quantised by the live settings at replay; only checkpoint-0 fires skip it (MainComponent.cpp:1941, :4834-4841).
3. Routine pads have their own Quantize (default Bar). "Every clip you fire starts at once" (R86) does not say what a routine pad does. A routine pad press still waits for a bar with global Quantize Off; the pad menu has a "Quantize" submenu; the 8-slot bank, `/api/routine/set`, OSC fire and the engine tests all use it. Boris's quote concerns the top bar and clip fires; whether the routine's own quantize stays is UNDECIDED in his words (listed in section 4).
4. "/4 /2 x1 x2 x4" are dead (Q1b). Do not read them as part of Quantize. They are also in the saved show (`bpmMultiplier`), which the nudge-row lane's S3r plans to drop (RR:460) and RN S3b plans to hide (the five multiplier buttons `setVisible(false)`, RN:480).
5. The top-bar combo does not follow the model (Q1a; RB7 H2). A loaded show with quantizeMode set quantises while the combo says Off. A removal must not assume the visible state equals the model state.
6. The routines' "restore-then-replay on the next bar" is the routine engine's own lead-in design (RR:51-58, NA-16: `startFromOne` counts a beat not a bar so the glide lands on the bar). A rule "nothing waits" must name whether it covers pads.
7. `docs/claude/architecture.md:94` says `beatSnap` = "Snap playhead to beat on trigger"; the code does BOTH: a legacy playhead-to-beatPhase seek (MainComponent.cpp:4866-4889) and, via `beatSnapMode`, the queue. The ClipInspector tooltip says only "Quantize clip trigger". `beatSnap` bool and `beatSnapMode` are two fields (Clip.h:139-140) kept in step by the UI and the loader.
8. Test hooks name the widget: `TopBar::quantizeSelectorBoundsForTest()` (TopBar.h:62) is used by four layout tests as the left neighbour of the Master Signal group (tests/test_master_signal_link.cpp:177, 292, 331, 366). Removing the widget is not only a UI edit.
9. `Composition::fromVar` reads `quantizeMode` and `bpmMultiplier` WITHOUT `hasProperty` (Composition.h:879-880), so an old show without the key reads 0 or a default; an old show WITH quantizeMode set and no reader is simply ignored only if the load line is kept or dropped on purpose. RR:460 says the nudge-row lane's S3r "reads and drops the key" for `bpmMultiplier`.
10. Pitfalls and rules that bind this area: Pitfall 63 (the Layer trigger tuple is ONE atomic word; the pending slot and its 4-bit snap override are inside it; `.harmony/probe-tsan-unit.sh` is REQUIRED for any change to it; test_shared_field_types lints it), Pitfall 38 (one Autopilot per frame: the drain lives inside `Autopilot::processFrame`), Pitfall 42 (a beat clock integrates totalBeatCount, never the beatPhase wrap; the drain uses the count delta), Pitfall 41 (a strip/inspector widget showing a model value must follow the model from the timer: the Quantize combo is a standing violation), Pitfall 59 (a model-driven widget's change test compares what it paints), Pitfall 5 (ResettableSlider; not relevant to combos), CLAUDE.md "Sacred Rules" (nothing here runs on the audio callback).
11. Names that mislead: `TopBar::onQuantizeChanged` and `onBpmMultiplierChanged` are never assigned; `Autopilot::processFrame` is the show-wide beat drain, not only "autopilot"; `RoutineSnap` is a copy of `Clip::BeatSnapMode` (static_assert RoutineEngine.cpp:12-17), so removing the clip enum breaks the routine engine's type unless it is re-declared; `Routine::quantize` is typed `Clip::BeatSnapMode` (Routine.h:21).
12. RB7's AM8 (relabel to "Next Bar", combo follows the model) is adopted on paper but NOT in main; BF20 (Boris's list 1 bar, 1/2, 1/4, 1/8, 1/16) is an unplanned later change. Boris's 2026-10-04 words replace BF20 (BD:976-977).
13. Stage lists differ by file: RT stage keys are S0, S1, S2, S2b, S3, S3h, S4a, S4b, S5; the user's list (SR, SM-a, SM-b, S4t, S4c, S4d, S4e, S5a, S5b, S6) is R2 section 4 (R2:542-570), changed by RA:467-506. The nudge list S1r/S2r/S3m/S3r/S4r is RR:446-480.
14. The "beat lock" and the "slide" are design only; the ONLY beat-phase code on main for a BPM clip is the legacy trigger-time seek in Q2 and a speed (rate) formula.

## 4 UNKNOWN (each with the cheapest test)

U1. Does a pending trigger left in a take's checkpoint 0 get restored (queued again) at replay, or ignored? -> UNKNOWN: I read PerfState capture/serialisation but not Program.cpp's preamble consumer for `pendingTriggerColumn`. Cheapest test: grep `pendingTriggerColumn` in src/recording/Program.cpp and RoutineSlice.cpp, then a ctest-only read (no run needed).
U2. How late does a Bar-quantised fire land in Auto with onsets trailing the wrap (RN SF-1: "INFERRED")? Cheapest test: nudge lane's FM-2 / L3d (a probe on the click file); not run by me.
U3. With no steady beat, what does a clip with its own Snap = Bar do (does it sit queued for ever)? INFERRED from Layer.h:421 and Autopilot.cpp:74-80 only. Cheapest test: test-mode app, no inject, set a clip's Snap to Bar, POST /api/trigger_clip, read `pendingClip` in GET /api/composition for 10 s.
U4. Whether an old show carrying `quantizeMode` / `beatSnapMode` / `bpmMultiplier` loads clean after a field is dropped (Composition.h:879-880 reads with no guard). Cheapest test: a unit case loading a hand-written JSON without the keys (test_composition.cpp shape); and with the keys against the new reader.
U5. Whether the transport lane's RD-13 ("the lock reads the snapshot the quantized release got") survived R2/RA, whose bars-based design replaced DA-1. Cheapest test: read R2's amendments RA-2/RA-3 and plan-transport-delta2's S4c for the string `frameSnap_`.
U6. Whether R2/RA keep RT's TR6 / TR6b in the 18/30 probe rows: R2:623 says "the chain's and the plan's rows stand", so INFERRED yes; Cheapest test: the `probe-transport.sh` row list after S6, or grep TR6 in plan-transport*.md.
U7. Whether Boris's "remove it from the top bar" also covers the routine pad's Quantize submenu and the routine's default Bar start, and the clip beat-snap field's REST/show-file form (R86/R87 say clip snap "goes too"; routines are not named). Only Boris can settle it (his words are BD:967-975; R86 and R87 in boris-clarify-135-143.md:43-45).

STATUS: DONE

## VERIFICATION (independent re-read)

Verifier: an adversarial seat, read-only. Pin check re-run: `git -C /Users/boriskarpman/projects/RealTimeAudio diff --stat 7bc6df7 -- src tests docs CMakeLists.txt` printed nothing -> main = 7bc6df7 for src/tests/docs. Nothing was built or run. Marks: CONFIRMED / CITATION-OFF (true, wrong pointer) / WRONG. Paths relative to the repo root; "RT/RD/R2/RA/RN/RR/RB7" as in the sheet header.

VERDICT: SOUND_WITH_CORRECTIONS. No claim of substance was found false. Five code citations and two table citations point at the wrong line, four lists the sheet presents as whole are not whole (cancels, tests, surfaces that count beats, Q5 rows), and one UNKNOWN (U1) can be closed by a grep.

### A. The 18 VERIFIED lines a planner leans on hardest (re-read at the cited file:line)

1. Q1a "a control named Quantize exists in the top bar, Off / Next Beat / Next Downbeat, default Off" -- CONFIRMED (TopBar.cpp:172-187; TopBar.h:117-119; layout 601-603).
2. Q1a "the combo does not follow the model" -- CONFIRMED. The only `setSelectedId` on `quantizeSelector_` is TopBar.cpp:181 (grep of every `quantizeSelector_` in src: .h:62,119; .cpp:177-187, 603). `onQuantizeChanged` is declared (TopBar.h:29), called (TopBar.cpp:186-187), assigned nowhere.
3. Q1a/Q3 "only three writers of the global Quantize: the combo, a loaded show, a replayed take's preamble" -- CONFIRMED. grep -i quantiz over src outside the routine files finds only TopBar.cpp:184, Composition.h:230/742/880, MainComponent.cpp:2012-2015, Program.cpp:292-295, PerfState*/PerfStateCapture.cpp:53. No REST/OSC/MIDI/binding/preset/undo writer.
4. Q1b "/4 /2 x1 x2 x4 write `bpmMultiplier` and nothing reads it" -- CONFIRMED. grep `bpmMult|bpm_mult|tempoMult|bpmScale` over src+tests: Composition.h:142/229/741/879, TopBar.cpp:403, tests/test_composition.cpp:157/194. No consumer in the tracker, renderer, REST, OSC, autopilot.
5. Q1c row 2 "clip Snap combo: Snap Off / Beat / Bar / 2 Bar / 4 Bar, always visible when a clip is inspected" -- CONFIRMED (ClipInspector.cpp:161-175; bounds 746-751 with no setVisible on it; refresh 1428).
6. Q1 completeness ("no Preferences control, menu-bar item, layer-strip, ClipCell, DeckView, binding-overlay, MIDI, OSC field quantises") -- CONFIRMED with patterns the author did not use. I grepped src/ui, binding, midi, osc for `snap`, `beatSnap`, `bar line|next bar|downbeat`, every `addItem("...Beat|Bar...")`, and listed all of src/ui. The only quantise/snap controls are the three the sheet names (top-bar combo, clip Snap combo, routine pad Quantize submenu, DeckView.cpp:921-938). Also checked: `Binding::Action` (Binding.h:24-43) has no quantise/snap action; OSC has onTriggerClip and onTriggerRoutine only (no column trigger, no quantise). NOT quantisers but beat-counting surfaces the sheet's Q1c table does not list: MilkDropBrowser.cpp:563-566 jukebox timing "4/8/16/32 beats"; LayerInspector.cpp:75-111 "End of Video / On Beat" + beat count (the sheet's row 5 covers it only as "the Layer's own default"); CompositionInspector.cpp:105-117 per-type "Opaque/Transparent/Effect Beats" sliders; SignalInspector.cpp:41-91 envelope beat durations.
7. Q2 "a re-fire of the active clip is never queued" -- CONFIRMED in behaviour; CITATION-OFF by one line: the test `snapEnabled && ref != r.activeRef()` is Layer.h:424, not :423 (RT:116 also says :424). `immediateNext` 361-373 is right.
8. Q2 "global Quantize maps NextBeat->Beat, NextDownbeat->Bar, Off when not LOCKED" -- CONFIRMED (MainComponent.cpp:36-43).
9. Q2 "per-clip snap has no tracker-lock guard; only the global path has one" -- CONFIRMED: Layer.h:420-431 reads `target->beatSnapMode`/`beatSnap` with no snapshot argument at all, so it cannot see the tracker. The "never drains" consequence stays INFERRED (U3).
10. Q2 "drain: Autopilot::processFrame, render thread, once per frame, on a totalBeatCount delta, then `processPendingTrigger(beatInBar, barCount)`" -- CONFIRMED (Autopilot.cpp:74-92; called Renderer.cpp:513). New detail the sheet does not state: the call sits inside `if (deckActive)` (Renderer.cpp:508), so with no deck/composition no queue drains at all.
11. Q2 "what decides the line (forced wins, else clip's own, else legacy bool -> Beat; Bar `beatInBar==0`; 2/4 Bar parity of barCount; pending Off = Beat)" -- CONFIRMED (Layer.h:420-421, 449-482; the `case Off`/`case Beat` pair is 468-470).
12. Q2 "VERIFIED cancels: clear/X, momentary release, Remove Deck; deck switch cancels nothing" -- CONFIRMED but NOT COMPLETE (see C-2): Clear Deck / Clear Layer row and the Undo of Add Deck / Insert Deck also cancel a queued trigger on main.
13. Q2 path table "autopilot advance: global NOT applied, but queues if the TARGET clip's own Snap is on" -- CONFIRMED (Autopilot.cpp:326/355/417 pass forced Off; Layer.h:420-421 still reads the target's own mode).
14. Q2/headline 4 "a take/routine replay of a clip fire goes through the same quantising handler (Origin::Replay, immediate=false); only Origin::Preamble is immediate" -- CONFIRMED (MainComponent.cpp:1941, 1960-1971, 4832-4850; `routineEngine_.dispatch.fire = recorderHost_.dispatch.fire` at :2051, so routine playback is the same path).
15. Q2 routine pad: `effectiveSnap(forced, own)`, default Bar, a fire with no beat starts at once, edits while pending re-time -- CONFIRMED (RoutineEngine.cpp:181, 222-244, 246-258, 489-523, 564-570, 683-704, 757-775; Routine.h:21, 102-109). Nuance: "no beat" is `trackerState == LOCKED && bpm > 0` (MainComponent.cpp:4281, 6256), not only "not locked". Also: the routine tick re-reads the global Quantize EVERY tick (MainComponent.cpp:4278-4281), so flipping Quantize while a pad waits re-times it (via resyncPending); the sheet shows only the fire-time read.
16. Q2 "the render thread tries at most 16 CASes, then waits for the next qualifying beat (Layer.h comment :330-340)" -- CITATION-OFF. True, but the comment is Layer.h:393-396 (and :323); :330-340 is `updateRuntime`'s body. `kRenderTriggerAttempts` Autopilot.cpp:11 is right.
17. Q2 "transitions are not beat-aligned: transitionSpeed is a duration in seconds, <=0 falls back to 0.5 s; <=0 in immediateNext = a cut" -- CONFIRMED (LayerClock.h:10-25; Layer.h:371-372).
18. Q1c row 6 "Random FX on Beat toggle + beat-count combo + Sync button at MainComponent.cpp:200-232" -- CITATION-OFF. Content right (combo 1/2/4/8/16/32 default 4; Sync resets `beatCounter_`; beat deltas at :4683-4700 and the 30 Hz call at :4363-4364 are right), but the widgets are built at MainComponent.cpp:227-262, not 200-232 (that range is the constructor's top). Also `resync` (REST/OSC action, :5783-5786) resets the same counter.

Other VERIFIED lines re-read and CONFIRMED: Q2 legacy beat-phase seek (MainComponent.cpp:4868-4889, runs on `t.after.activeRef()`); Q3 stored fields (Composition.h:742/880, Clip.cpp:72-73/229-233, Routine.h:124/159-160, PerfState.cpp:86-87/121-122/192/225, PerfStateCapture.cpp:53/70, AppSettings none); Q3 routes (ApiServer.cpp:455, 1555, TestServer.cpp:691, routine routes); Q6 capture site (MainComponent.cpp:4986-5001 unconditional on `changed`, 5105-5120), stamp = `clock_->now()` returning `current_` (RecorderClock.h:50), `s.t/s.sample/beat/bpm` only (PerformanceRecorder.cpp:25-41), Replay filter (RecorderHost.cpp:669), `pickAt` (Program.cpp:540-551), RoutineSlice.cpp:36 drops `quantize`; Q4 test names and lines in test_undo_commands/test_layer_runtime/test_show_model/test_master_signal_link (the four layout tests cite TEST_CASE lines 177/292/331/366; the actual `quantizeSelectorBoundsForTest()` calls are at :191, 302, 342, 386); Renderer.cpp:1657-1667 and 1718-1736 (rate only, no lock).

Small corrections inside the sheet's own text:
- Line 98 says a drained queued clip "auto-plays on every drained activation". Code (Layer.h:548-556, MainComponent.cpp:4866): auto-play at the drain happens only while `hasBeenTriggered` is false; the flag is set for the clip ACTIVE after a press, so a clip that was ever the active clip at some press (e.g. fired immediately once) no longer auto-plays at a later drain. Still INFERRED, not run.
- Line 29: RB7 AM8 "planned a fix NOT on main" -- true.

### B. TABLE Q5: 12+ rows re-opened at the cited line (about 60 rows read in all)

CONFIRMED (text at the cited lines, quote and id match): RT:389-392 (AM-1), RT:831-834, RT:967-975 and 972, RT:276 and 767-768, RT:461, RT:494-502, RT:818-819, RT:1260 (K6), RT:126 and 304, RT:629-630 (AM-23), RT:1122-1125; RD:312, RD:612 and 1179, RD:909, RD:979, RD:998-1000 and 1049, RD:1149-1162; R2:74 (AM-23 STANDS), R2:212, R2:304 (and 300-301), R2:724-725, R2:742-743, R2:841; RA:253-254, RA:618-622, RA:671-673; RN:189, 524-529, 639-655, 700-701, 707-708, 738, 759-761, 780-783, 789-790, 824-826; RR:18, 49-53, 91-96, 164-166, 390-395, 424-428, 460, 527-538, 566-575, 693-696, 710, 757-760, 785-786; RB7:20 and 94, 295-311, 380 and 565-572, 421 and 440, 586 and 348-352, 596/602/644-649, 606-609 and 623, 662-668, 710-715. BD:967-971 / 975 / 976 and boris-clarify-135-143.md:43-45 (R86, R87) also read and match.

CITATION-OFF:
- Row "RN:31, 78, 90 | A1 / V3": the quoted sentence ("Readers at a count edge take the bar position from the SAME snapshot ... Autopilot.cpp:74 ... processPendingTrigger(snapshot.beatInBar ...") is RN:77-78 (V3). RN:31 and RN:90 are about BE-1's bar fields, not the quote.
- Row "RD:345-347 | DA-1": the sentence starts at RD:344 and sits under "HOW IT LIVES WITH HIS OTHER RULINGS", not under DA-1. More important: that same RD text says "Fired BETWEEN beats with Quantize off, a clip ... slides onto the NEAREST beat" -- the slide that R2/RA then replaced with the one cut on the next "1" (RA:20-60). The sheet quotes only the part that survives and does not say the row is superseded (it does hint at it in U5).
- Row "RR:18": the sentence starts on RR:17-18 (mid-sentence at 18); harmless.

Table rows the sheet MISSED (grep of every ruling for bar line|next bar|waits|snap|quantis|quantiz|queued|queue, each hit read):
- R2:52 "AM-1 queue-only transitions never run the tail | STANDS", R2:59 (AM-8 STANDS), R2:72 (AM-21 STANDS), R2:69 (AM-18 "the inert combo ... his now"): R2's status rows for the three Quantize-dependent amendments. The table cites RT's AM-1 / AM-8 / AM-21 text but never says R2 ruled them STANDS; only AM-23 is marked STANDS.
- RT:319 + RT:614-619 + RT:744 (RE-6, AM-21, stage S2b, FM-1): the stale-frame fix is conditional and covers "queued fires"; the S2b stage touches Pitfall 56 -- a stage that rests on the queue.
- RT:247 (R32) and RD:116: `Autopilot::processFrame` "runs the quantized release (Autopilot.cpp:85)" is part of the scope seat's "must not be cut" list; the beat drain lives in a function other stages keep.
- RT:36, RT:116 (RE-1, R1), RT:914 and 927 (mutants MU-3 "queue test keeps `ref != activeRef`" -> TL-U3; MU-4 "tail runs for a queue-only transition"): the mutation rows that pin the queue.
- RD:465 ("the tuple: what it plays, what fades out, what is queued") -- the strip-field rule of the Undo lane counts a queued clip.
- RN:436-441 (A18) and RN:721-731 (B11, B12, B13): texts told to Boris -- "A clip FIRED quantised starts on the nudged beat", "things that START on a beat (a quantised clip, an autopilot step)", "a clip set to start on the bar ... starts on the 1". B13 and B12 are readings he is shown that rest on a bar-quantised fire existing.
- RN:512 (T-N4) and RN:545: bar-line arithmetic with `barCount`/`beatInBar` that the queue reads.
- RR:56-62 (V11/V12 text): "A ROUTINE ... waits for a BAR LINE (V12)" and "AUTOPILOT in beat mode ... it waits" while the timer is held; RR:219 (V22) "a waiting quantised fire would land"; RR:348, 749, 763 (a routine "waits" with a held timer; draft question 129 not asked); RR:576-577 are in the LR2 row already cited.
- RB7:75 and 712 ("The 8 s waits ... come from the per-clip 4 Bar snap, which bf7 leaves unchanged"); RB7:387-399 (AM16 "pre-registered Quantize probe": 4-bar / 2-bar arms) and RB7:591; RB7:485 (stage S4 "Quantize relabel + follows the model"); RB7:696-701 (R6 strongest counterargument). The sheet's RB7 rows cover AM8/T7/H1/H2/Q2 but not the pre-registered probe or R6.
- ruling-one-save / outputs / effect-looks / looks-answers: I re-grepped; I agree with the sheet's "no row" line.

### C. Corrections the sheet needs (not whole-line errors)

C-1. Q4 test list is incomplete. Also reference the quantise/snap model: tests/test_layer_strip_source_deck.cpp (aliases `Snap = Clip::BeatSnapMode`, :22); the four take fixtures tests/fixtures/take_v2_future.json, take_v2_legacy_audio.json, take_v3_audio.json, take_v3_start_bpm0.json carry `quantizeMode` in checkpoint0 (the last also `pendingTriggerColumn`) and are loaded by test_take.cpp / test_audio_store.cpp; `.harmony/probe-step3.sh:732` pins "preambleFired >= 8 (deck, quantize, 5 flags, activeClip)" (a probe that rests on the quantize preamble row, besides test_recorder_host's count of 8); `.harmony/probe-tsan-analyze.py:35-36, 68-70` lists `beatSnapMode|beatSnap|quantizeMode` in the race-analysis field list.
C-2. Q2 "VERIFIED cancels" is not the full list. A queued trigger into deck D is also cancelled by: Clear Deck and Clear Layer row (`clearTupleForRow`, core/DeckCommands.h:272-285, called at MainComponent.cpp:6791 and 6875); `clearedLayerClips` for their Undo/Redo (DeckCommands.h:247-262); the Undo of Add Deck and of an Insert Deck that added no layers (`cancelPendingInto`, DeckCommands.h:727 and 854; RT's AM-8 plans to REMOVE those two on S1, so on main they cancel); Remove Deck (:946) is the sheet's. Also a re-fire of the ACTIVE clip clears a pending trigger (Layer.h:361-364 `immediateNext` resets the pending slot; the comment at Layer.h:411-415 says so).
C-3. U1 can be answered now (grep, no run): `pendingTriggerColumn` / `pendingTriggerSnapOverride` outside Layer.h/core/ appear only in PerfState.h:60-61, PerfState.cpp:86-87/121-122 and PerfStateCapture.cpp:70-71. Program.cpp and RoutineSlice.cpp never read them. So the take stores the queue at checkpoint 0 and NOTHING restores it (the preamble row 2 restores only the global Quantize, Program.cpp:292-295). VERIFIED by grep of src.
C-4. docs that state the legacy seek or the combo: docs/claude/performance-controls.md:48 ("a beat-snapped clip seeks to the beat phase") and :73 (the granularity and the 16-attempt rule), architecture.md:94, recording.md (8 mentions), .harmony/APP-INVENTORY.md:56, 58, 186, 188. The sheet cites architecture.md:94 only.

### D. Attempts to REFUTE the three main conclusions

Main conclusions taken as: (i) a "Quantize" combo exists in the top bar and the five multiplier buttons are inert; (ii) two live gates on a fire (global, per-clip) plus a third for routine pads, with a replayed fire re-quantised; (iii) the recorder stores only the press time, never the landed time.

(i) NOT REFUTED. Looked for a second Quantize widget (grep over src/ui for the label and for every `ComboBox` item list that contains Beat/Bar), a reader of `bpmMultiplier` (tracker, ApiServer, TestServer, OSC, tests), a menu-bar item, a Preferences field, a setter that syncs the combo (none), a binding action (none). Found nothing the sheet did not read. Extra surfaces found are in A.6 and are not quantisers.
(ii) NOT REFUTED. Looked for a fire path that skips `handleClipTrigger`/`handleColumnTrigger` or reaches `Layer::triggerClip` without the forced snap: all `triggerClip(`, `triggerClipImmediate(`, `.fire(`, `triggerColumn(` call sites in src are Autopilot (forced Off), Composition.h:433-453, MainComponent's two handlers, and the routine/recorder `dispatch.fire` (the same lambda). OSC has no column trigger. REST trigger routes are the two handlers. Looked for a replay path that is `immediate` other than Origin::Preamble: none. Looked for a guard that makes a re-fire of the active clip queue: none (Layer.h:424).
(iii) NOT REFUTED. Capture runs at press time inside both handlers (not at the drain, which runs on the render thread; RecorderHost::capture is message-thread only). No lane, field or anchor stores a landed time. The recorder also has no capture for a Momentary release.
No conclusion defaulted to "refuted"; every path I could find that the sheet did not read was a path that agrees with it (list in A.6, A.12 and C-2).

### E. MISSING (a planner needs these and the sheet does not answer them)

1. The full list of what cancels a queued trigger (C-2), because a rule "nothing waits" removes the queue but the cancel code and its Undo snapshots (`clearedLayerClips`, `clearTupleForRow`, `cancelPendingInto`) stay named in Commands that tests (TL-UC14/15, K6) pin.
2. Whether the queue is read by anything outside Autopilot.cpp:74-92: answered here -- the only drains are `Autopilot::processFrame` and the routine engine's own tick; the only UI reader is none (grep `pendingRef|pendingTrigger|pendingClip` in src/ui and MainComponent.cpp returns nothing).
3. R2's status for each queue-dependent amendment (R2:52, 59, 72, 74): AM-1, AM-8, AM-21, AM-23 all STANDS. A planner reading only the table cannot tell which rows were already voided.
4. The superseding of RD DA-1's "slide onto the nearest beat" by RA's cut table (RA:20-60, 253-254), so that RD:345-347 is not read as current.
5. Which stored fields are dead the moment the queue goes: `Clip::beatSnapMode`, `Clip::beatSnap`, `Layer` pending slot (4-bit override inside the CAS word, Pitfall 63, probe-tsan-unit.sh REQUIRED), PerfState pending fields (no reader, C-3), `Routine::quantize` typed `Clip::BeatSnapMode` and `RoutineSnap` static_assert (the sheet has these in section 3 item 11 but not the PerfState pair).
6. What a pad's routine "starts on" with Quantize out of the top bar: `forced` would be Off always, so `effectiveSnap` returns the routine's own (default Bar) -- the routine tick passes `quantizeModeToForcedSnap(...)` at MainComponent.cpp:4280 and 6255; those two lines are the only places that couple routines to the top-bar value. Not in the sheet.
7. The beat-counting surfaces in A.6 (jukebox timing, layer On Beat, per-type cycle sliders, signal envelope beat durations) -- if the new rule says "nothing waits for a beat" a planner must know these still count beats.
8. A statement of what Boris could verify on screen today for a queued clip: nothing (confirmed: no UI shows it; only REST `pendingClip`). The sheet has it; keep.

STATUS: VERIFIED (independent re-read, appended).
