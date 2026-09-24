# s-rta-0924b — Spec STEP 4: the Record panel UI — PLAN

Author: Architect (Fable), 2026-09-24, secondary lane for RealTimeAudio. Read-only pass over
`/Users/boriskarpman/projects/RealTimeAudio` @ main (tree: `.harmony/` docs modified, `AGENTS.md` untracked;
no `src/` changes). Labels: VERIFIED = read on disk this session at the cited line; INFERRED = derived from
cited code; ASSUMED = stated so a builder can check it. `MainComponent.cpp` cites carry a function/lambda
anchor because its line numbers drift (`.harmony/binding-decisions.md`, "re-grep by anchor text").

Inputs obeyed: `.harmony/specs/s167-performance-log-and-routines.md` (§5 row 4, §7, D14, R10, R15, R16);
`.harmony/specs/s-rta-0923-step3-plan.md` (what step 3 shipped; §5 only-Boris #5; Q1-Q7) as amended by
`.harmony/.reports/s-rta-0924/step3-critic.md`; `.harmony/specs/s-rta-0923-ruling28-audio-store.v2.md`
(§5.4, §6, §13); `.harmony/binding-decisions.md` rulings 5, 19, 24, 28; `.harmony/HANDOFF.md` s-rta-0924
section (STILL OPEN, SCREEN-SAFETY LAW); `.harmony/gotchas.md` 2026-09-24 live-app rules; CLAUDE.md UI rules
(whole words, `ResettableSlider`, `showMenuAsync`); Harmony VISUAL WORK GATE + Interaction-Logic Critic
standing rule (`~/Harmony_Main/.claude/agents/harmony.md:305-323`,
`~/Harmony_Main/.claude/agents/harmony-references/execution-protocol.md:911-925`).

---

## QUESTION

Plan spec build-order row 4: make `RecordPanel` real against `RecorderHost` (shipped in step 3) — what
the spec asks, what exists, the controls and states (idle / armed / recording / playing / overdub, errors,
human-refused), how the panel drives `RecorderHost` on the message thread through the SAME funnel REST
uses, which calls are Boris's (with a default each so nothing blocks), the visual-review plan (critic panel
incl. an Interaction-Logic critic because the panel is stateful), tests, and a lane split with one owner
per file. Flag what should wait.

---

## APPROACH (verdict first)

**Build the panel as a THIN VIEW over a GUI-free, ctest-pinned state model, driven by ONE MainComponent
funnel that REST and the panel share.** Three layers, three owners:

1. `RecorderHost::Status` gains the handful of fields the panel needs (loaded-take facts, playback position
   in SECONDS, the loaded asset id) — additive, published by the existing `publishStatus()` (S4-A).
2. A NEW header `src/ui/RecordPanelModel.h`: a pure function `Status + panel inputs → RecordPanelView`
   (every button's text/enabled/tooltip/tone, the status/warning/notice lines). No `juce_gui_basics`; the
   click-through matrix (§4.3) is a Catch2 table test linking `juce_core` only (S4-M). This is the piece the
   Interaction-Logic critic reviews and the piece that survives the UI rewrite (ruling 5).
3. `RecordPanel` becomes widgets + `refresh(status, now)` that calls the model and applies it; every action
   is a `std::function` returning the refusal text (S4-C). `MainComponent` extracts the six REST lambda
   bodies (`MainComponent.cpp:2012-2104`, anchor `apiServer_->onPerfRecord`) into member functions
   `perfRecord/perfStop/perfLoad/perfPlay/perfStopPlay/perfRepair`; REST lambdas become one-liners; the
   panel's callbacks call the same functions (S4-B). The panel NEVER touches `recorderHost_` directly.

Two deliberate behaviour changes ride S4-B because the panel cannot be honest without them: (a) a
programmatic source-mode switch also syncs the TopBar's "Audio" selector (today `setSourceMode(File)` at
the play-with-audio path leaves the selector saying "Mic Input" — VERIFIED `MainComponent.cpp:2082-2089`
anchor `apiServer_->onPerfPlay`, no selector write); (b) Stop Playback restores the source mode that was
active before a play-with-audio (today the app stays in File mode with the take's WAV loaded — VERIFIED
`:2093-2096` anchor `apiServer_->onPerfStopPlay`). Both are gated by `probe-step3.sh` 63/0.

Answer to "should this wait for the UI rewrite" (step-3 plan §5 only-Boris #5): **no.** D14 lists the
panel in NOW (`s167:755-757`); ruling 19 requires a REAL, visible "record audio" switch (REST-only is a
hidden setting — `binding-decisions.md:353-355`); ruling 24 makes the log the product, and the only human
who can judge it cannot run curl. The disposable part (view code) is ~300 lines; the model, the funnel and
the Status fields outlive the rewrite.

---

## 0. GROUND TRUTH (VERIFIED unless marked)

| # | Fact | Cite |
|---|---|---|
| G1 | Spec row 4 text: "`RecordPanel` real: shadow bool removed (G25), state from recorder/player, 4 Hz refresh, `Playing 12.3 / 45.6 s · unresolved: N`, Save/Load gated to Idle, "Render… (coming)" disabled item \| S \| click-through matrix; a take loaded against a smaller composition shows `unresolved: N`, not silence" | `s167:896` |
| G2 | D14 NOW: "`RecordPanel` made real (dead UI gets built): Record/Stop/Play/Save/Load against the new classes, 4 Hz refresh, compile report on the status line; the "Video (Future)" item becomes "Render… (coming)" disabled with a tooltip, not hidden." | `s167:755-757` |
| G3 | R10: modal dialogs pause the 120 Hz tick; "Save/Load gated to Idle". R15: "`RecordPanel` should refuse to start on a volume with < 2 GB free". R16: event-log tab and the menu's video recorder "must become one Record concept in the rewrite" | `s167:940-942, 954-959` |
| G4 | Today's panel: five `TextButton`s (Record/Stop/Play/Save/Load), `browseOutputBtn_`, `formatSelector_` ("JSON Events", "Video (coming later)"), `statusLabel_`, `outputDirLabel_`; all disabled at alpha 0.4 with whole-word tooltips; callbacks `onStartRecording/onStopRecording/onPlayRecording` declared; `refresh()` empty; NO `recording_` shadow bool remains (G25 already discharged by L5) | `src/ui/RecordPanel.h:19-42`; `RecordPanel.cpp:3-94` |
| G5 | `BrowserPanel::getRecordPanel()` has ZERO callers outside `BrowserPanel.h`; the panel is tab 4 of 6, tab button text "Record" | grep `getRecordPanel` over `src/` → only `BrowserPanel.h:36`; `BrowserPanel.h:39,51` |
| G6 | `RecorderHost` public surface: `arm/disarm/isRecording/tick/capture/onHuman*/load/play(PlayMode)/stopPlay/isPlaying/repairLoadedAudio/status()/shutdown`; `Status` has recording/playing/overdub, takeFolder, assetId, audioMode, playMode ("withAudio"/"wallClock"/""), lastError, t/beat/sample/bpm, lanes/points/gestures/markers, gapDetection/framesWritten/gaps, position/length, unresolved/reboundByPosition/reboundByName/invalid, skipped/continuousUnavailable/refusedByHand, audioStatus, deviceRate, rateChangedSinceArm, humanRefused. NO loaded-take facts, NO seconds conversion, NO loaded asset id | `src/recording/RecorderHost.h:85-212` |
| G7 | `status()` is a mutex-guarded copy, callable from any thread; `publishStatus()` runs at play/stopPlay/disarm/every tick; `load()` does NOT publish | `RecorderHost.cpp:646,660,350,483-484,747-751`; `load()` body `:576-596` |
| G8 | `Status.position/length` are in the Program's drive-clock domain: WithAudio → `pos = playFirstSample_ + transportFrames` (samples), Wall → seconds since play start; a compiled point's `at` under `DriveClock::Sample` is the ABSOLUTE `s.sample` stamp; `length = maxAt` | `RecorderHost.cpp:473-481, 721-722`; `Program.cpp:162-164, 273` |
| G9 | `ClockStamp.t` is seconds since the first tick after arm (`startWall_`); the clock's `sample` is the absolute delivered counter (anchor "start" at `deliveredSamples`) | `RecorderClock.cpp:16,22,27,90` |
| G10 | Every `RecorderHost` public method asserts the message thread; `dispatch.notify` is only ever invoked from those methods (or from MainComponent on the message thread) | `RecorderHost.cpp:109,145,260,357,578,600,652,665,755`; `MainComponent.cpp:1992-1994` anchor `recorderHost_.dispatch.notify =` |
| G11 | `disarm()` leaves `takeFolder_` set (only recording/overdub/tap/gaps/markers/baseline fields reset), so `status().takeFolder` names the LAST take after Stop | `RecorderHost.cpp:341-348` |
| G12 | `arm()` refusals: "already recording", "overdub asset not found: …", "could not begin audio asset (store root unwritable?)", "AudioTap failed to start (disk / free-space?)"; `play()` refusals: "no take loaded", "already playing", "audio not resolved: <reason>"; `load()` refusal = `stats.refusalReason` verbatim | `RecorderHost.cpp:149,188,202,208,604,609,616,586` |
| G13 | `tick()` sets `lastError_` (and notifies) on a mid-take device-rate change and on a tap self-stop; `lastError_` persists until the next arm | `RecorderHost.cpp:372-381, 407-415, 171` |
| G14 | The REST callbacks (`onPerfRecord/Stop/Load/Play/StopPlay/Repair/Status`) are lambdas in the ctor that call `recorderHost_` directly and report failures ONLY via `dispatch.notify` → `std::cerr` | `MainComponent.cpp:2011-2154` anchor `apiServer_->onPerfRecord`; `:1992-1994` |
| G15 | `onPerfRecord`: name defaults to `%Y-%m-%d_%H%M%S`; folder = `~/Documents/Audio-DNA/Takes/<name>.adna-take` (computed inline, no helper); `audioMode` from `getSourceMode()`; `onsetMarkers`/`overdubAssetId` pass-through; auto-play only when `opts.audioFile` was given | `MainComponent.cpp:2032-2061` |
| G16 | `onPerfPlay(withAudio)`: `play()` then `loadFile(wav)` + `setSourceMode(File)` + `applyAudioTransport("play")`; `onPerfStopPlay`: `stopPlay()` + `applyAudioTransport("stop")` (refused with a notice while an overdub is armed) — source mode is never restored | `MainComponent.cpp:2073-2096`; `applyAudioTransport` body (anchor `void MainComponent::applyAudioTransport`, "audio stop refused: overdub in progress (R-A8)") |
| G17 | Two audio-source selectors write the engine mode: `audioSourceSelector_` (ctor, items "Mic Input"/"Audio File") and `topBar_->getAudioSourceSelector()`; neither is updated by the programmatic `setSourceMode(File)` calls at the perf paths | `MainComponent.cpp:255-272, 583-597, 2029, 2088`; `TopBar.h:37,63` |
| G18 | UI refresh cadence: `timerCallback` at 30 Hz with a `uiUpdateCounter_ >= 8` slot (~4 Hz) for FPS/DSP labels; inspector refresh at ~10 Hz | `MainComponent.cpp:417, 3488-3510` |
| G19 | `ApiServer.h` includes `<httplib.h>` → no UI file may include it; `ApiServer::PerfRecordOpts {name, audio, audioFile, onsetMarkers, overdubAssetId}` | `src/api/ApiServer.h:3, 95-102` |
| G20 | LookAndFeel draws buttons purely from `buttonColourId`/`textColourOffId` (no toggle/enabled awareness); state colours precedent: `kMeterRed`/`kMeterYellow`/`kMeterGreen` for SEARCHING/LOCKING/LOCKED | `src/ui/LookAndFeel.cpp` `drawButtonBackground`/`drawButtonText`; `TopBar.cpp:262-285`; `LookAndFeel.h:10-20` |
| G21 | `juce::FileChooser::launchAsync` on macOS uses `beginWithCompletionHandler` (non-blocking; the message loop and timers keep running); only `runModally` calls `runModal` | `build/_deps/juce-src/modules/juce_gui_basics/native/juce_FileChooser_mac.mm:198, 210-221` |
| G22 | No ctest target links `juce_gui_basics` today; the GUI-headless spike (`ScopedJuceInitialiser_GUI` under ctest) was specified but NEVER run — feasibility is ASSUMED | `tests/CMakeLists.txt` (grep `juce_gui_basics` → only the "no juce_gui_basics" comment at `:491-492`); `.harmony/specs/s-rta-0923-review-fixes-plan.md:125-136`; `.harmony/.reports/s-rta-0923/L5.review.md` finding 3 |
| G23 | `AudioStore::Status {NoAudio, Resolved, ResolvedUnverified, Missing, Incomplete, Mismatch, Legacy, MultiSegment}`; `Resolution{status, reason, wav, asset}`; spec §6: never degrade to wall clock silently; Incomplete → offer Repair | `src/recording/AudioStore.h:111-117`; ruling-28 v2 spec `:533-545, 509-518` |
| G24 | `Take::Meta{recordedAt, app, duration, durationBeats}`; `Take::load(folder, LoadStats&)`; `LoadStats{refused, refusalReason, unknownFeatures, …}` | `src/recording/Take.h:47-55, 61-69, 110` |
| G25 | `Binding::Action::ToggleRecording` and Output › "Start Recording"/"Stop Recording" drive the VIDEO recorder, not the take recorder | `MainComponent.cpp:6648-6666` anchor `case Binding::Action::ToggleRecording`; `src/ui/MenuBarModel.cpp:148-149` |
| G26 | `ResettableSlider` (`UniversalParamControl.h:23-44`) is mandatory for any slider; `TopBar`/`LayerStrip` use it. The panel has NO slider and NO `PopupMenu` (the ComboBox's own popup is JUCE-internal) | `UniversalParamControl.h:23-44`; `TopBar.h:65,91,95` |
| G27 | Live-app rules: production launch via `open`; `pgrep -x Audio-DNA` (never the literal path in a watcher); `tests/visual/ax_press.py "<AXTitle>"` is the accepted way to press in-window controls, `osascript` for menus only; NO debugger, NO synthetic keystrokes/coordinate clicks, STOP on any unexpected dialog; graceful `osascript` quit; `screencapture -x` and LOOK before closing | `.harmony/gotchas.md:438-448, 222-228`; `.harmony/VALIDATION.md:31-39, 68-72`; `HANDOFF.md:134` |
| G28 | Prior RecordPanel critic panel (L5): 4 seats (logic, UX, visual, graphic), verdicts from source + `screencapture` full/crop/hover PNGs, all MUST-FIXes folded | `.harmony/.reports/s-rta-0923/l5-visual/critic-panel.md`; `HANDOFF.md:2641` |
| G29 | Tooltip window: `tooltipWindow_ = std::make_unique<juce::TooltipWindow>(this, 600)` | `MainComponent.cpp:566` |
| G30 | Free-space floor is enforced inside `AudioTap::start` (arm refuses "AudioTap failed to start (disk / free-space?)") — INFERRED from step-3 plan R-6 (`AudioTap.h:132` `kMinFreeBytes` 2 GB, not re-read here) | `s-rta-0923-step3-plan.md:788`; `RecorderHost.cpp:205-209` |

---

## 1. WHAT STEP 4 IS, PER THE SPEC (quoted)

- Row 4 (`s167:896`): *"`RecordPanel` real: shadow bool removed (G25), state from recorder/player, 4 Hz
  refresh, `Playing 12.3 / 45.6 s · unresolved: N`, Save/Load gated to Idle, "Render… (coming)" disabled
  item — Size S — Verify: click-through matrix; a take loaded against a smaller composition shows
  `unresolved: N`, not silence."*
- D14 NOW (`s167:755-757`): *"`RecordPanel` made real (dead UI gets built): Record/Stop/Play/Save/Load
  against the new classes, 4 Hz refresh, compile report on the status line; the "Video (Future)" item
  becomes "Render… (coming)" disabled with a tooltip, not hidden."*
- §7 NEEDS BORIS (`s167:973-991`) contains NO panel question; the panel questions live in the step-3 plan
  (`:742-743` only-Boris #5; Q1 location/naming, Q6 arm-on-play) and ruling-28 §13 (deleting audio, store
  location, naming). Carried STILL-OPEN items 4-8 (`HANDOFF.md:2733-2734`) are not step-4 blockers.
- Rulings that shape it: **5** (UI disposable → mechanism first, `binding-decisions.md:125-129`);
  **19** (audio capture is a REAL switch, default on, `:353-355`); **24** (the log is the product, `:400+`);
  **28** (record the show, then re-do the knob work over the same audio → overdub is the core workflow).

What the spec's row 4 wording no longer matches, and how this plan resolves it (each is a §5 product
call with a default): "Save" has no backend — `RecorderHost` has no `save()`; a take is saved at arm
(provisional), every 60 s and at Stop (`RecorderHost.h:214`, `RecorderHost.cpp:239-253, 322`), and REST
shipped without `/api/perf/save` (`ApiServer.cpp:258-264`). "Stop" is ambiguous once playback and
recording can overlap (overdub). "Output Folder…" has no consumer (the takes root is fixed, G15).

---

## 2. WHAT ALREADY EXISTS IN `RecordPanel` (G4)

Reusable as-is: the component skeleton, `kControlHeight/kLabelHeight/kRowSpacing`, `kDisabledAlpha`
comment (the LookAndFeel does not dim disabled controls — `RecordPanel.h:48-55`), the whole-word tooltip
idiom, `setComponentID` ids, the `FileChooser::launchAsync` idiom (`RecordPanel.cpp:70-81`).
Removed by this step: `stopBtn_`, `saveBtn_`, `browseOutputBtn_`, `outputDirLabel_`, `outputDir_`,
`getOutputDir()`, the three unassigned callbacks, the empty `refresh()`. Kept and renamed: the format
combo (D14 "Render... (coming)" item).

---

## 3. TRADEOFFS CONSIDERED

- **Panel logic inside `RecordPanel` (widgets decide) vs a GUI-free model.** Model chosen: the state
  matrix is the risk (the flow critic's domain), and it is only testable under ctest if it does not need a
  display (G22: GUI-headless feasibility unproven). Cost: one header + one table test. Rejected: relying on
  a GUI test spike that may never run.
- **Panel drives `recorderHost_` directly vs through MainComponent's funnel.** Funnel: arm order is
  load-bearing (mode switch → arm; `s-rta-0923-step3-plan.md:504-506`), `ArmOptions` needs device facts
  only MainComponent has (G15), and REST/OSC/UI must stay one path (spec D6/D8 posture). Rejected: a
  second `ArmOptions` builder in the panel.
- **One shared "Stop" vs per-lifecycle toggling buttons.** Two toggling buttons ("Record Take"/"Stop
  Recording", "Play Take"/"Stop Playback"): in overdub both lifecycles run at once and a single Stop would
  have to guess — the exact "navigation trap" class the Interaction-Logic rule exists for. Rejected: a
  Stop that "stops whichever is running".
- **Poll `status()` at 4 Hz from the existing 30 Hz timer vs a panel-owned `juce::Timer`.** Existing slot
  (G18): one clock, the precedent for the inspector, and the spec says 4 Hz. Cost: `status()` copies under a
  mutex 4×/s — nil.
- **Loaded-take facts: publish in `Status` vs MainComponent remembering `LoadResult`.** `Status`: REST
  `/api/perf/status` gets them for free, one source of truth, and `publishStatus()` already runs every tick.
- **Load via a folder chooser vs a takes list in the tab.** Chooser now (10 lines, `launchAsync` is
  non-blocking G21); a list is rewrite-era UI (ruling 5). `AudioStore::scanTakes` exists if the rewrite
  wants it (`AudioStore.h:138`).
- **Position readout in seconds: convert in the host vs in the panel.** Host: only it knows
  `playAssetRate_`/`playFirstSample_` (G8, private); converting in the panel would need two more published
  fields and duplicate the formula.
- **"Record Over" (overdub) in the panel now vs REST-only.** Include: zero new mechanism (REST path exists,
  probe-tested), and it is ruling 28's core loop. Separable (S4-C step 6) if the lane must shrink.

---

## 4. DECISION / SPEC

### 4.1 Architecture

```
RecordPanel (view, S4-C) --refresh(status, now)--> RecordPanelModel::derive() (pure, S4-M) --> widgets
      | onRecord/onStop/onPlay/onStopPlay/onLoad/onRepair (std::function → std::string refusal)
      v
MainComponent::perf{Record,Stop,Load,Play,StopPlay,Repair} (S4-B)  <-- ALSO called by apiServer_->onPerf* lambdas
      v
RecorderHost (S4-A: Status additions only)
```
Reviewer greps that pin it: `recorderHost_` in `src/ui/` → 0; `ApiServer\|MainComponent` in
`src/ui/RecordPanel*` → 0; `AlertWindow\|runModal\|enterModalState\|NativeMessageBox` in
`src/ui/RecordPanel*` → 0 (R10); `juce::Slider` in `src/ui/RecordPanel*` → 0 (no slider is planned; if one
is ever added it must be `ResettableSlider` + `setDefaultValue`, CLAUDE.md UI Patterns); `PopupMenu` in
`src/ui/RecordPanel*` → 0 (if ever added: `showMenuAsync().withParentComponent(getTopLevelComponent())`).

### 4.2 Lane S4-A — `RecorderHost::Status` additions (`src/recording/RecorderHost.{h,cpp}`, `tests/test_recorder_host.cpp`)

Add to `Status` (after `humanRefused`, additive, D12 "ADD, never REDEFINE"):
```cpp
// s-rta-0924b step 4 (Lane S4-A): facts the Record panel / REST status need that the host alone knows.
std::string loadedTakeFolder;   // "" when no take is loaded
std::string loadedRecordedAt;   // Take::Meta::recordedAt of the loaded take
double      loadedDuration = 0.0;   // Take::Meta::duration (seconds)
int         loadedLanes = 0;
std::string loadedAssetId;      // AudioStore::Resolution::asset.id when Resolved/ResolvedUnverified, else ""
std::string audioReason;        // AudioStore::Resolution::reason ("" when Resolved/NoAudio)
// Playback position in SECONDS regardless of DriveClock (Wall: as-is; Sample: relative to the asset's
// firstSample, divided by the asset rate) -- Status::position/length stay in the drive-clock domain.
double positionSeconds = 0.0, lengthSeconds = 0.0;
```
`publishStatus()`: under the existing `if (loadedTake_.has_value())` fill the six loaded fields
(`loadedTakeFolder` needs a new member `juce::File loadedTakeFolder_` set in `load()` on success);
under `if (playing_ && player_)`: Wall → `positionSeconds = position; lengthSeconds = length`; Sample →
`positionSeconds = max(0, position − playFirstSample_) / playAssetRate_`, `lengthSeconds = max(0, length −
playFirstSample_) / playAssetRate_` (guard `playAssetRate_ > 0`, else 0). `load()`: call `publishStatus()`
before returning on BOTH branches (today it never publishes, G7) — the panel and REST see a load
immediately, and the headless test needs no tick. Nothing else changes; no CMake edit.

Tests (append to `tests/test_recorder_host.cpp`, `FakeDispatch` idiom at `:106`):
- `[host][status][loaded] load() publishes the loaded facts without a tick` — arm/tick/disarm a 2-point take,
  `load(folder)` → `status().loadedTakeFolder == folder`, `loadedLanes == 1`, `loadedDuration ≈ meta.duration`,
  `audioStatus == "Resolved"`, `loadedAssetId == assetId`; before `load()` all are empty/0.
- `[host][status][seconds] positionSeconds/lengthSeconds in both clocks` — WithAudio with a 48 000 Hz asset
  whose `firstSample = 96 000` and last point at sample 144 000: after `tick(transportFrames = 24 000)` →
  `positionSeconds ≈ 0.5`, `lengthSeconds ≈ 1.0`; WallClock: after a 0.5 s scripted wall → `positionSeconds ≈ 0.5`.
  Fail-first: both cases fail on the pre-change header (fields absent → compile error is acceptable as the
  red state for a contract test; the S3-A idiom).

### 4.3 Lane S4-M — `src/ui/RecordPanelModel.h` (NEW, header-only) + `tests/test_record_panel_model.cpp` (NEW) + `tests/CMakeLists.txt` (append)

```cpp
#pragma once
#include "recording/RecorderHost.h"   // juce_core-only header (RecorderHost.h:18-19)
#include <juce_core/juce_core.h>

struct RecordPanelInputs
{
    double nowSeconds = 0.0;          // wall clock (Time::getMillisecondCounterHiRes()/1000)
    bool   recordAudio = true;        // the ruling-19 switch, user-owned
    bool   playWithAudio = true;      // user-owned; forced off by the model when audio is not ready
    juce::String notice;              // last refusal/notify text
    double noticeAtSeconds = -1.0;    // when it was set; expires after kNoticeSeconds
};

struct RecordPanelView
{
    enum class Tone { Neutral, Recording, Playing, Warning };
    struct Button { juce::String text; bool enabled = false; juce::String tooltip; Tone tone = Tone::Neutral; };
    Button record, play, load, reveal, repair;
    bool recordAudioEnabled = true, playWithAudioEnabled = false, playWithAudioValue = false, nameEnabled = true;
    bool recordSendsOverdub = false;  // what pressing Record would request right now
    juce::String statusText; Tone statusTone = Tone::Neutral;
    juce::String warningText;         // "" when none (kMeterYellow line)
    juce::String noticeText;          // "" when none/expired
    juce::String revealFolder;        // "" => reveal disabled
};

static constexpr double kNoticeSeconds = 10.0;
static constexpr double kArmedWarnSeconds = 2.0;
juce::String formatClock(double seconds);   // "m:ss", "h:mm:ss" past one hour; negative/NaN -> "0:00"
RecordPanelView deriveRecordPanelView(const RecorderHost::Status& s, const RecordPanelInputs& in);
```
Derived predicates (all from `Status`, never from panel memory — the G25 class is structurally impossible):
`recording = s.recording`, `overdub = s.overdub`, `playing = s.playing`, `withAudio = s.playMode ==
"withAudio"`, `loaded = !s.loadedTakeFolder.empty()`, `audioRequested = !s.assetId.empty() && !overdub`,
`armed = recording && audioRequested && s.framesWritten == 0`, `audioReady = s.audioStatus ∈ {Resolved,
ResolvedUnverified}`, `audioIncomplete = s.audioStatus == "Incomplete"`.

**The click-through matrix (the S4-M table test AND the Interaction-Logic critic's brief):**

| # | State | Record | Play | Load Take... | Show in Finder | Repair Audio | Record audio | Play with audio | Name |
|---|---|---|---|---|---|---|---|---|---|
| 1 | Idle, nothing loaded, nothing recorded | "Record Take" ON | "Play Take" OFF (tip "Load a take first.") | ON | OFF | OFF | ON | OFF | ON |
| 2 | Idle, last take recorded (`takeFolder` set, G11), nothing loaded | ON | OFF | ON | ON → last take | OFF | ON | OFF | ON |
| 3 | Idle, loaded, audio ready | ON | ON | ON | ON → loaded | OFF | ON | ON (user value) | ON |
| 4 | Idle, loaded, audio Incomplete | ON | ON (wall clock only) | ON | ON | ON | ON | OFF, value forced off | ON |
| 5 | Idle, loaded, audio Missing/Mismatch/Legacy/MultiSegment/NoAudio | ON | ON (wall clock) | ON | ON | OFF | ON | OFF, forced off | ON |
| 6 | Armed (recording, audio requested, 0 frames yet) | "Stop Recording" ON, Recording tone | OFF ("Stop the recording first.") | OFF | ON (provisional folder exists) | OFF | OFF | OFF | OFF |
| 7 | Recording (plain) | "Stop Recording" ON, Recording tone | OFF | OFF | ON | OFF | OFF | OFF | OFF |
| 8 | Playing, wall clock | "Record Take" ON (plain take) | "Stop Playback" ON, Playing tone | OFF | ON | OFF | ON | OFF (locked while playing) | ON |
| 9 | Playing with audio | "Record Over" ON (`recordSendsOverdub = true`) | "Stop Playback" ON, Playing tone | OFF | ON | OFF | OFF (tip "Recording over a take always uses that take's audio.") | OFF | ON |
| 10 | Overdub recording + playing | "Stop Recording" ON, Recording tone | "Stop Playback" OFF ("Stop the recording first.") | OFF | ON | OFF | OFF | OFF | OFF |
| 11 | Overdub recording, playback already stopped (REST-only) | "Stop Recording" ON | OFF | OFF | ON | OFF | OFF | OFF | OFF |
| 12 | Plain recording while playing (REST-only) | "Stop Recording" ON | "Stop Playback" ON | OFF | ON | OFF | OFF | OFF | OFF |

Status line (`statusText`/`statusTone`), one per state:
1 "Ready. No take loaded." · 2 "Ready. Last take: <folderName>" · 3/4/5 "Loaded: <folderName> — <clock
loadedDuration> · <n> lanes · Audio: <word>" with word = ready / ready (unverified) / incomplete, repair
available / missing / does not match / old format, record again / unsupported / none ·
6 "Armed, waiting for audio..." · 7 "Recording <clock s.t> from <live input|audio file> · <lanes> lanes ·
<points+gestures> moves" (+ " · <gaps> audio gaps" if > 0) · 8/9 "Playing <clock positionSeconds> / <clock
lengthSeconds> · unresolved: <unresolved>" (+ " · moved: <rebound sum>" if > 0, + " · skipped: <skipped>" if > 0)
· 10/11 "Recording over <folderName> <clock s.t> · …" (10 also shows the Playing readout after " — ") ·
12 as 7 + " — playing <clock>". Tones: 6/7/10/11 Recording, 8/9 Playing, else Neutral.

Warning line (`warningText`, first match wins, `Tone::Warning`): `s.lastError` non-empty while recording →
verbatim; `armed && s.t > kArmedWarnSeconds` → "No audio is arriving. Check the input source."; playing
with audio → "Replaying with the take's audio. Live input is paused until Stop Playback."; `s.humanRefused
> 0` while recording → "<n> moves were refused because a control was already held."; `s.continuousUnavailable
> 0` while playing → "<n> knob moves could not be replayed."; `s.rateChangedSinceArm` → covered by lastError
(G13). Notice line: `in.notice` unless `nowSeconds − noticeAtSeconds > kNoticeSeconds`.

Tooltips (whole words, sentences): Record Take "Starts a new take. Audio is recorded too when Record audio
is on." · Stop Recording "Stops this take. It is saved automatically." · Record Over "Records a new take over
this take's audio, from the current playback position." · Play Take "Replays the loaded take." · Stop Playback
"Stops the replay." · Load Take... "Choose a take folder (.adna-take)." · Show in Finder "Shows the take folder
in the Finder." · Repair Audio "Rebuilds the audio record of a take that was cut off, for example by a crash."
· Record audio "Records the sound the app is listening to, alongside the timelines." · Play with audio "Replays
the take's own audio instead of the live input."

Test target (`tests/CMakeLists.txt`, append at EOF after `test_analysis_resampler`, `test_thumbnail_cache`
shape `:491-500`): sources `test_record_panel_model.cpp` only; links `Catch2::Catch2WithMain juce::juce_core
juce::juce_events juce::juce_graphics` (whatever `RecorderHost.h`'s transitive includes need — builder trims
to the minimum that configures; `juce_gui_basics` is NOT allowed). Cases: one `TEST_CASE` per matrix row
(tags `[recordpanel][model]`), plus: `formatClock` (0 → "0:00", 83.4 → "1:23", 3661 → "1:01:01"); notice
expiry at 10 s; "forced off" of `playWithAudio` when audio not ready; `recordSendsOverdub` only in row 9;
warning precedence (lastError beats the replay note). **Fail-first**: commit the header with
`deriveRecordPanelView` returning `RecordPanelView{}` and the tests → red (row 1 "Record enabled" fails);
implement → green.

### 4.4 Lane S4-C — `src/ui/RecordPanel.{h,cpp}` (view only)

Public surface:
```cpp
struct RecordRequest { juce::String name; bool audio = true; bool overdub = false; };
std::function<std::string(const RecordRequest&)> onRecord;
std::function<std::string()> onStop;
std::function<std::string(bool withAudio)> onPlay;
std::function<std::string()> onStopPlay;
std::function<std::string(const juce::File& takeFolder)> onLoad;
std::function<std::string()> onRepair;
void setTakesRoot(const juce::File& root);              // chooser's initial directory + "Takes: <path>" caption
void setNotice(const std::string& text);                // stores text + now; applied at the next refresh (coalesces 120 Hz notify)
void refresh(const RecorderHost::Status& s, double nowSeconds);   // model → widgets; 4 Hz from MainComponent (G18)
```
Widgets: `recordBtn_`, `playBtn_`, `loadBtn_` ("Load Take..."), `revealBtn_` ("Show in Finder"), `repairBtn_`
("Repair Audio"); `juce::ToggleButton recordAudioToggle_` ("Record audio", default ON), `playWithAudioToggle_`
("Play with audio", default ON); `juce::TextEditor nameEditor_` with `setTextToShowWhenEmpty("Name (blank =
date and time)")` (precedent `TopBar.h:109` `bpmEditField_`); `formatSelector_`: item 1 "Take (timelines and
audio)" selected, item 2 "Render... (coming)" via `addItem(..., 2)` then `setItemEnabled(2, false)`, combo
tooltip "Rendering a take to video is coming in a later build." (ASCII dots — pitfall 6, Unicode at small
sizes); `statusLabel_` (11 pt, kTextPrimary), `warningLabel_` (10 pt, kMeterYellow), `noticeLabel_` (10 pt,
kAccentCyan), `takesRootLabel_` (10 pt, kTextSecondary, "Takes: ~/Documents/Audio-DNA/Takes").

Layout (top→bottom, `reduced(4)`, `kRowSpacing` between): row A `recordBtn_` (120) `playBtn_` (120);
row B `loadBtn_` (100) `revealBtn_` (110) `repairBtn_` (100); row C `nameEditor_` (stretch, min 160)
`recordAudioToggle_` (110) `playWithAudioToggle_` (130); `statusLabel_`; `warningLabel_`; `noticeLabel_`;
row D `formatSelector_` (200) `takesRootLabel_` (rest). Two button rows on purpose — "Stop Recording" at
14 pt needs ~110 px and the browser area is narrow.

`refresh()`: `view = deriveRecordPanelView(s, inputs)`; for each button `setButtonText/setEnabled/
setTooltip`, `setAlpha(enabled ? 1 : kDisabledAlpha)` (G20: the LookAndFeel does not dim), and
`setColour(buttonColourId, tone == Recording ? kMeterRed : tone == Playing ? kMeterGreen : kSurfaceLight)`;
toggles `setEnabled` + (`playWithAudio`) `setToggleState(view.playWithAudioValue, dontSendNotification)`;
labels `setText`. Cache the last `Status` and re-apply in `visibilityChanged()` so a tab switch shows current
state without waiting 250 ms. Button handlers: Record → `onRecord({nameEditor_.getText().trim(),
recordAudioToggle_.getToggleState(), lastView_.recordSendsOverdub})` when `!lastStatus_.recording`, else
`onStop()`; Play → `onPlay(playWithAudioToggle_.getToggleState())` when `!lastStatus_.playing`, else
`onStopPlay()`; every handler stores the returned string via `setNotice` (empty string clears). Load →
`FileChooser("Choose a take folder", takesRoot_)` with `openMode | canSelectDirectories`, `launchAsync`
(G21, non-blocking) → `onLoad(result)` if `result.isDirectory()`. Show in Finder →
`juce::File(view.revealFolder).revealToUser()`. All handlers run on the message thread (JUCE button
callbacks); no `callAsync` needed. No modal dialog anywhere in the file (R10).

Component ids for the AX gate: "recordTake", "playTake", "loadTake", "revealTake", "repairAudio",
"recordAudio", "playWithAudio", "takeName". Button texts double as AX titles: "Record Take"/"Play Take" are
deliberately NOT "Record"/"Play" so `ax_press.py "Record"` matches the TAB button (G5, G27) uniquely.

### 4.5 Lane S4-B — `src/MainComponent.{h,cpp}` only (every hunk by anchor)

**B1 — the funnel.** New private members (`MainComponent.h`, next to the `apply*` choke points `:466-479`):
```cpp
// s-rta-0924b step 4: ONE funnel for REST (/api/perf/*) and the Record panel. Each returns "" on success,
// else the refusal/failure text -- the same text is also sent through recorderHost_.dispatch.notify.
std::string perfRecord(const ApiServer::PerfRecordOpts& opts);
std::string perfStop();
std::string perfLoad(const juce::File& takeFolder);
std::string perfPlay(bool withAudio);
std::string perfStopPlay();
std::string perfRepair();
juce::var   perfStatusVar() const;                 // the existing onPerfStatus body, moved verbatim
static juce::File takesRoot();                     // ~/Documents/Audio-DNA/Takes (was inline at onPerfRecord)
void setAudioSourceModeSynced(AudioEngine::SourceMode mode);   // engine + BOTH selectors (dontSendNotification)
std::optional<AudioEngine::SourceMode> sourceModeBeforeReplay_; // set by perfPlay(withAudio), consumed by perfStopPlay
```
Bodies = the six lambda bodies at `MainComponent.cpp:2012-2104` MOVED verbatim (anchor `apiServer_->
onPerfRecord`), with exactly these changes: (1) each failure branch `return`s the text it notifies;
(2) `takesRoot()` replaces the inline path; (3) the two `setSourceMode(File)` calls (`:2029`, `:2088`) become
`setAudioSourceModeSynced(File)`; (4) `perfPlay(withAudio=true)`: before switching, `if
(!sourceModeBeforeReplay_) sourceModeBeforeReplay_ = audioEngine_.getSourceMode();`; (5) `perfStopPlay()`:
after the existing two calls, `if (sourceModeBeforeReplay_ && !recorderHost_.isRecording()) {
setAudioSourceModeSynced(*sourceModeBeforeReplay_); } sourceModeBeforeReplay_.reset();` — never restore
while a take is armed (a device restart re-prepares the tap; T18/R-2 of the step-3 plan); (6) `perfStop()`
on success also notifies "Saved: <takeFolder>" (the panel shows it as the 10 s notice). The seven REST
lambdas become one-liners: `apiServer_->onPerfRecord = [this](const ApiServer::PerfRecordOpts& o){
perfRecord(o); };` etc.; `onPerfStatus = [this]{ return perfStatusVar(); };`. `setAudioSourceModeSynced`:
`audioEngine_.setSourceMode(mode); const int id = mode == File ? 2 : 1; audioSourceSelector_.setSelectedId(id,
dontSendNotification); if (topBar_) topBar_->getAudioSourceSelector().setSelectedId(id, dontSendNotification);`
(G17). Also `fileLabel_` text as the selector handlers do (`:266-268`).

**B2 — notify fan-out** (anchor `recorderHost_.dispatch.notify =`, `:1992-1994`): keep the `std::cerr` line
and add `if (browserPanel_) browserPanel_->getRecordPanel().setNotice(msg);` (message thread only, G10;
`browserPanel_` is constructed at `:1564`, before this assignment — VERIFIED line order inside the ctor).

**B3 — panel wiring** (after `browserPanel_->getFilesBrowser().onFileActivated`, `:1586`):
```cpp
auto& rp = browserPanel_->getRecordPanel();
rp.setTakesRoot(takesRoot());
rp.onRecord = [this](const RecordPanel::RecordRequest& r) {
    ApiServer::PerfRecordOpts o; o.name = r.name; o.audio = r.audio;
    if (r.overdub) o.overdubAssetId = juce::String(recorderHost_.status().loadedAssetId);
    return perfRecord(o); };
rp.onStop     = [this] { return perfStop(); };
rp.onPlay     = [this](bool withAudio) { return perfPlay(withAudio); };
rp.onStopPlay = [this] { return perfStopPlay(); };
rp.onLoad     = [this](const juce::File& f) { return perfLoad(f); };
rp.onRepair   = [this] { return perfRepair(); };
```
**B4 — 4 Hz refresh** (anchor `if (++uiUpdateCounter_ >= 8)`, `:3491`): inside that block add
`if (browserPanel_) browserPanel_->getRecordPanel().refresh(recorderHost_.status(), juce::Time::
getMillisecondCounterHiRes() / 1000.0);`.

Reviewer greps for S4-B: (i) `recorderHost_\.\(arm\|disarm\|load\|play\|stopPlay\|repairLoadedAudio\)(` in
`MainComponent.cpp` → only inside the six `perf*` bodies; (ii) `apiServer_->onPerf` lambdas contain no
`recorderHost_` call; (iii) `setSourceMode(` outside `setAudioSourceModeSynced` → only the two selector
`onChange` handlers (`:266,272,592,596`) and the file-load sites (`:3381, :3813`, untouched); (iv)
`browserPanel_->getRecordPanel()` appears in B2, B3, B4 only.

### 4.6 Tests (summary)

| Lane | Test | Independent pass/fail |
|---|---|---|
| S4-A | `test_recorder_host` +2 cases (§4.2) | loaded facts published by `load()`; seconds conversion in both clocks (1.0 / 0.5) |
| S4-M | NEW `test_record_panel_model` (§4.3) | 12 matrix rows + formatClock + notice expiry + forced-off + overdub flag + warning precedence; red on the stub, green after |
| S4-B | `probe-step3.sh` 63/0 from the MAIN checkout (`.venv` required) | REST behaviour preserved through the extraction; WithAudio→WallClock replay rows still pass with the source-mode restore |
| S4-B | new probe rows (Harmony appends to `probe-step3.sh` or a sibling `probe-step4.sh`) | after `play {withAudio:true}` → `stop_play`: `/api/perf/status.playing == false` AND a new `GET /api/status`-visible fact… (none exists for source mode) → verify via stderr `[Recorder]` lines + the TopBar selector crop in §6 |
| S4-C | no ctest (view); optional `test_record_panel` ONLY if the 20-min GUI spike (`review-fixes-plan.md:125-136`) passes; never a gate | `findChildWithID("recordTake")` enabled/text per matrix row after `refresh()` with a scripted Status |
| all | reviewer greps §4.1 and §4.5; whole-word grep over button/tooltip strings in `RecordPanel.cpp` (no "Rec", "Cfg", "Fx" etc.) | 0 hits |

### 4.7 Lane table — DISJOINT file ownership

| Lane | Owns (nothing else) | Depends on | Size |
|---|---|---|---|
| **S4-A** host status | `src/recording/RecorderHost.{h,cpp}`, `tests/test_recorder_host.cpp` | — | S (½ d) — commit **A1** (header fields) within the first hour |
| **S4-M** model | NEW `src/ui/RecordPanelModel.h`, NEW `tests/test_record_panel_model.cpp`, `tests/CMakeLists.txt` (append one block at EOF) | A1 | S-M (½-1 d) |
| **S4-C** view | `src/ui/RecordPanel.h`, `src/ui/RecordPanel.cpp` | A1, S4-M header | M (1 d) |
| **S4-B** app wiring | `src/MainComponent.h`, `src/MainComponent.cpp` | A1, S4-C header (callback names) | S-M (½-1 d) |
| **S4-D** docs | `CLAUDE.md` (Audio Store § "Step 4: LIVE" + source tree `RecordPanel` line + Common Pitfalls: "ToggleRecording is the VIDEO recorder"), `.harmony/APP-INVENTORY.md` rows 85/267, `.harmony/VALIDATION.md` (new gate row) | everything | S |
| **Harmony** | gate: `build-s4gate/`, ctest, probe-step3, §6 screenshots + critic panel, fold-ins re-dispatched to C/B | all | ½-1 d |

NOT touched by any lane: `src/ui/BrowserPanel.*` (tab + accessor already exist), `src/api/ApiServer.*`,
`src/ui/TopBar.*`, `Take.*`, `Program.*`, `Player.*`, `AudioStore.*`, `AudioTap.*`, `Binding.h`.
Order: **Wave 1** A1 (header) → **Wave 2 (parallel)** A2, S4-M, S4-C → **Wave 3** S4-B (serial: it codes
against C's header) → **Wave 4** Harmony: ONE clean forced rebuild in `build-s4gate/`, `ctest` (RUN it; expect
408 + 2 + N), `probe-step3.sh` from the main checkout, §6 → fold-ins → **Wave 5** S4-D. Conventions: own
`-B build-<lane>` (never `./build`), worktree builds have no `.venv` (probe runs from main), `git add -f`
under `.harmony/`, `git show --stat HEAD` per commit.

---

## 5. WHAT IS GENUINELY BORIS'S CALL (plain words; default in brackets so nothing blocks)

1. **Where takes go and what they are called.** [Fixed folder `~/Documents/Audio-DNA/Takes`; a name box on
   the panel, blank = date and time. No folder picker. Audio stays in `…/Audio` (ruling 28 Q2).]
2. **No Save button.** A take saves itself when you press Stop, and every minute while recording. [Remove
   Save; add "Show in Finder" so you can see where it went.]
3. **Two buttons that flip** ("Record Take" ↔ "Stop Recording", "Play Take" ↔ "Stop Playback") instead of one
   shared Stop. [Two buttons.]
4. **"Record audio" switch** (ruling 19): on at every launch, not remembered between launches. [On, not
   remembered — like the Syphon toggle.]
5. **"Play with audio"**: on by default when the take has its audio; while replaying, the take's audio
   replaces the live input; when playback stops, the input goes back to what it was. [On + go back.]
6. **"Record Over"** (re-do your knob work over a take's audio, ruling 28) from the panel while a take plays
   with its audio. [Include; it is the same path REST already proved.]
7. **Load** = choose the take folder in a file dialog; a list of takes inside the tab comes with the new UI.
   [Dialog now.]
8. **Onset markers** (an engineering test aid) stay REST-only. [Not on the panel.]
9. **"Repair Audio"** always visible, greyed unless a take's audio was cut off. [Visible, greyed.]
10. **Times** shown as minutes:seconds (hours:minutes:seconds past an hour), no tenths. [That.]
11. **The video recorder** stays under Output › Start/Stop Recording until the rewrite (R16); the panel's
    format menu shows "Render... (coming)" greyed. [Leave it.]
12. **Free space**: today the app refuses to start a take below 2 GB free and says so; a live "free space"
    readout can come later. [Refuse-with-message only.]
Carried, unchanged, not step-4 blockers: replay does not restore the starting look (HANDOFF s-rta-0924
#1); deleting/locating/naming stored audio (ruling-28 §13); Resync vs oscillators.

---

## 6. VISUAL-REVIEW PLAN (VISUAL WORK GATE — before Boris sees it)

Not micro (layout + many controls + stateful) → the FULL panel, five seats, critics as subagents, each
verdict written to `.harmony/.reports/s-rta-0924b/step4-visual/critic-panel.md`:
1. **Interaction-Logic (flow/state) critic — MANDATORY** (standing rule, `execution-protocol.md:911-925`):
   input = §4.3 matrix + `RecordPanelModel.h` + the state screenshots; looks for dead-ends (a state with no
   way out), a button whose meaning changes without its label changing, an action that is enabled but will
   be refused by the funnel (e.g. Play with audio on an Incomplete take), REST-only states the panel
   misreports (rows 11/12), tab-switch/relaunch showing stale state, the overdub double-stop trap.
2. **Logic critic (source)**: greps §4.1/§4.5; the funnel is a MOVE not a change; no modal; message-thread
   only; the `notify` fan-out cannot re-enter the host.
3. **UX critic**: is every refusal visible without hovering; does a non-technical performer know a take was
   saved and where; is "Record Over" understandable.
4. **Visual-design critic** and 5. **Graphic-design critic**: tones (red recording / green playing) legible
   at the app's size; disabled = dimmed uniformly (the L5 finding: one button must not dim differently);
   tooltip never covers the status line (L5 MUST-FIX — keep the status BELOW the button rows, `RecordPanel.
   cpp:105-106`); whole words everywhere.

Harmony drives the BEHAVIOURAL part herself, screen-safety law in force (G27):
```bash
cd /Users/boriskarpman/projects/RealTimeAudio            # MAIN checkout (.venv lives here)
pgrep -x Audio-DNA && { echo "instance running -- not ours to quit"; exit 1; }
cmake -B build-s4gate -DCMAKE_BUILD_TYPE=Release && cmake --build build-s4gate --config Release -j
(cd build-s4gate && ctest)                                # RUN it; record the count
bash .harmony/probe-step3.sh                              # 63/0 is the S4-B regression bar
open --stdout /tmp/s4-out.log --stderr /tmp/s4-err.log build-s4gate/AudioDNA_artefacts/Release/Audio-DNA.app   # NO --test-mode
sleep 6; curl -s -m3 http://127.0.0.1:7070/api/health
.venv/bin/python tests/visual/ax_press.py "Record"        # the TAB (unique title; the buttons are "Record Take"/"Play Take")
S(){ sleep 1; screencapture -x "/tmp/s4-$1.png"; }        # Harmony crops the browser area afterwards
S idle-nothing-loaded
curl -s -X POST http://127.0.0.1:7070/api/perf/play -d '{"withAudio":false}'; S notice-no-take-loaded     # "no take loaded" notice
curl -s -X POST http://127.0.0.1:7070/api/perf/record -d '{"name":"s4gate","audio":true}'; S armed-or-recording
sleep 4; S recording                                      # red "Stop Recording", "Recording 0:04 from live input · …"
curl -s -X POST http://127.0.0.1:7070/api/perf/stop; S idle-after-stop                                     # "Saved: …" notice, Show in Finder on
curl -s -X POST http://127.0.0.1:7070/api/perf/load -d "{\"folder\":\"$HOME/Documents/Audio-DNA/Takes/s4gate.adna-take\"}"; S loaded-audio-ready
curl -s -X POST http://127.0.0.1:7070/api/perf/play -d '{"withAudio":true}'; sleep 2; S playing-with-audio   # green "Stop Playback", "Playing 0:02 / 0:0x · unresolved: 0", live-input warning
ID=$(curl -s http://127.0.0.1:7070/api/perf/status | .venv/bin/python -c 'import json,sys;print(json.load(sys.stdin)["loadedAssetId"])')
curl -s -X POST http://127.0.0.1:7070/api/perf/record -d "{\"name\":\"s4over\",\"overdubAssetId\":\"$ID\"}"; S overdub   # both red/green; Stop Playback greyed
curl -s -X POST http://127.0.0.1:7070/api/perf/stop; curl -s -X POST http://127.0.0.1:7070/api/perf/stop_play; S idle-restored   # TopBar "Audio" selector back to Mic Input (crop the top bar too)
# unresolved: copy s4gate/take.json, add an activeClip lane on layer 9 (deck 0), load it, play wall clock → "unresolved: 1"
.venv/bin/python tests/visual/ax_inspector.py --output /tmp/s4-ax.json     # tooltips via AXHelp (INFERRED JUCE mapping; if absent, Boris hovers -- Tier 4)
osascript -e 'quit app "Audio-DNA"'; sleep 3; pgrep -x Audio-DNA; screencapture -x /tmp/s4-eos.png   # LOOK at it
```
Never open the Output window; no coordinate clicks; if any dialog appears, stop and report. The crops +
critic verdicts + fold-in commits are the deliverable Boris then checks at Tier 4 (launch, Browser › Record).

---

## 7. WHAT SHOULD WAIT (named so nobody builds it by accident)

- A takes LIST in the tab, per-take metadata browsing, deleting takes/audio (ruling-28 §7, §13) — rewrite.
- Unifying the video recorder with the take recorder (R16); renaming Output › "Start Recording" — rewrite.
- A keyboard/MIDI binding for the take recorder: needs a NEW `Binding::Action` (never reuse
  `ToggleRecording`, which is the video recorder, G25) — separate lane after step 4.
- Compile-report preview at LOAD time ("unresolved: N" before pressing Play): needs `RecorderHost::load` to
  dry-compile against the Composition (new API) — LATER; step 4 shows it while playing (G1 met on Play).
- Persisting the panel's toggles/name; a "free space" readout (R15 pre-check); LATCH/OVERWRITE modes,
  editing, routines — D14 LATER.

---

## 8. RISKS (each with the file it lives in) and the strongest counterargument

- **R-1 Funnel extraction regresses REST.** `MainComponent.cpp`. It is a MOVE; the two intended changes
  (selector sync, source-mode restore) are listed in §4.5; `probe-step3.sh` 63/0 before merge is the bar.
- **R-2 Source-mode restore re-prepares the tap.** `MainComponent.cpp` `perfStopPlay`. Guarded: never
  restore while `isRecording()`; the model never offers Stop Playback during overdub (row 10) — REST can still
  reach that state and keeps today's behaviour.
- **R-3 A 120 Hz `notify` burst repaints the panel.** `RecordPanel.cpp`. `setNotice` only stores; the label
  is applied at 4 Hz.
- **R-4 Playback readout domain.** `RecorderHost.cpp` `publishStatus`. G8 is VERIFIED but the firstSample
  subtraction is INFERRED from `Program.cpp:164` + `RecorderClock.cpp:22`; the S4-A seconds test (1.0 / 0.5)
  is the check — if it reads 2.0 the assumption is wrong and the builder stops.
- **R-5 "Armed, waiting for audio" is a transient the recorder itself does not name.** Derived from
  `framesWritten == 0` — honest and cheap, but on a device that never delivers (no mic permission) it stays
  "Armed" with a warning after 2 s rather than "Recording"; that is the truth.
- **R-6 GUI-headless test feasibility is unknown** (G22). The model test needs no GUI; the view test is
  optional and never a gate.
- **R-7 `ax_press.py "Record"` title collision.** Solved by naming the buttons "Record Take"/"Play Take";
  if the AX tree still finds the tab ambiguous, Harmony falls back to Boris switching the tab (Tier 4) — the
  REST-driven states do not depend on the tab being visible except for the screenshots.
- **R-8 `FileChooser` initial directory for a folder pick** (macOS `canSelectDirectories`): VERIFIED
  non-blocking (G21); whether the panel-less Finder sheet lets a user descend INTO `.adna-take` folders
  (they are plain directories, no bundle bit) is ASSUMED — the builder checks once by hand.

**Strongest counterargument to this plan, and why it loses:** *"Ruling 5 says the current UI will be
scrapped — do not build a panel; REST plus a menu item is enough until the rewrite."* It loses on three
rulings Boris made AFTER ruling 5: 19 (a REAL audio switch, not a hidden setting), 24 (the log is the
product — and the product owner cannot exercise it over curl), and 28 (record the show, then re-do the knob
work over the same audio — a loop that needs Record Over and Play with audio at hand, live). The plan
already concedes the counterargument's valid half: the disposable view is kept thin and cosmetic work is
zero; the model, the funnel and the Status fields are the mechanism that outlives the rewrite.

---

## 9. SUMMARY FOR HARMONY (12 lines)

1. Step 4 = spec row 4 / D14: make the Record tab real over `RecorderHost`; G25's shadow bool is already gone (L5), the rest is unbuilt (`getRecordPanel()` has zero callers).
2. Shape: GUI-free `RecordPanelModel.h` (Status → view, ctest table of 12 states) + thin `RecordPanel` view + ONE MainComponent funnel `perf{Record,Stop,Load,Play,StopPlay,Repair}` shared by REST and the panel; the panel never touches the host.
3. Host additions (additive Status fields): loaded-take facts, loaded asset id, `positionSeconds/lengthSeconds`; `load()` now publishes.
4. Controls: "Record Take"↔"Stop Recording", "Play Take"↔"Stop Playback", "Load Take...", "Show in Finder", "Repair Audio", "Record audio" (ruling 19, default on), "Play with audio", a name box, "Render... (coming)" greyed. Save/Stop/Output Folder removed (no backend / ambiguous / no consumer).
5. States: idle / armed (0 frames yet) / recording / playing (wall or with audio) / overdub (+2 REST-only combos); errors = funnel return text (10 s notice) + `lastError` warning; human-refused and continuous-unavailable counts on the warning line; `unresolved: N` on the Playing readout.
6. Two intended behaviour changes ride S4-B: programmatic source-mode switches sync the "Audio" selectors; Stop Playback restores the pre-replay input (never while a take is armed). probe-step3 63/0 is the bar.
7. Boris's calls (12, defaults in §5): takes folder/naming, no Save, two flipping buttons, audio switch not remembered, play-with-audio restores input, Record Over in the panel, folder dialog for Load, onset markers REST-only, Repair always visible, m:ss times, video recorder stays put, free-space refuse-with-message.
8. Lanes (disjoint files): S4-A host, S4-M model+test+CMake, S4-C view, S4-B MainComponent, S4-D docs; A1 header first, then M/C parallel, then B, then the gate.
9. Tests: 2 host cases, a 12-row model table (fail-first on a stub), probe-step3 regression, reviewer greps; GUI test optional, never a gate.
10. Visual review: full 5-seat panel INCLUDING the Interaction-Logic critic (stateful UI is never "micro"); Harmony drives states over REST, switches the tab with `ax_press.py "Record"`, screencaptures each state, graceful quit, screen looked at.
11. Wait: takes list, video/take unification (R16), a new Binding action, compile report at load, persistence, editing.
12. Risks named: the funnel move (probe gate), tap re-prepare on restore (guarded), readout domain (test 1.0/0.5), GUI-test feasibility (sidestepped), AX title collision (solved by naming).

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0924b/plan-step4-record-panel.md
STATUS: COMPLETE
