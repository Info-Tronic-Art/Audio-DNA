# s-rta-0923 — Spec STEP 3: MainComponent wiring of the recorder — PLAN

Author: Architect (Fable), 2026-09-23, secondary lane for RealTimeAudio. Read-only pass over
`/Users/boriskarpman/projects/RealTimeAudio` @ HEAD `d434e0e` (main). Working tree at read time carried
STAGED, UNCOMMITTED work from two in-flight lanes (`git diff --cached --stat`, VERIFIED): H1
(`src/recording/AudioTap.cpp` chunking fix + `tests/test_bt_device_shapes.cpp` + an append to
`tests/CMakeLists.txt`) and L5 (`src/ui/RecordPanel.{h,cpp}` disabled-with-tooltips). Nothing under
`.harmony/` or `src/` was written by this pass; this file is the dispatch-named scratch output.
Labels: VERIFIED = read on disk this session at the cited line; INFERRED = derived from cited code;
ASSUMED = stated so a builder can check it. `MainComponent.cpp` cites carry a function/lambda anchor
because its line numbers drift (`.harmony/binding-decisions.md:179-182`).

Inputs obeyed: `.harmony/HANDOFF.md:2371-2590` (s168 + addenda + s171); `.harmony/binding-decisions.md`
rulings 25-28 (`:407-466`); `.harmony/specs/s167-performance-log-and-routines.md` (D6, D8, D14, §3,
§5 row 3, §6 R5/R8/R9/R10, §9.1); `.harmony/specs/s-rta-0923-ruling28-audio-store.v2.md` (§5.1, §5.2,
§5.4, §5.6, §9, §12 R3/R8); `.harmony/specs/s166-universal-connection-architecture.md` (§2.3, §4.3,
§5 L3(e), ADDENDUM A1); `.harmony/scout-lane3-connect-surface.md` (§2 "no manualWrite funnel", §6
double-writers, §7 sequencing); `.harmony/specs/s-rta-0923-review-fixes-plan.md` (L5 in flight).

---

## QUESTION

Plan spec build-order row 3 (`s167 spec:895`): wire the recorder into the live app — `RecorderClock::tick`
+ `Player::advanceTo` in the 120 Hz tick, `Origin`/`deck` on the trigger/deck handlers, `manualWrite` at the
D6a sites, the AudioStore `beginAsset → tap.start → finalize → referencing → save` lifecycle per v2 §5.6
(provisional `take.json` at arm, periodic save, per-tick `popGap` drain, `isRunning()` before `stop()`,
asset-frame clock in overdub, `advanceTo` vs `seek`), the R14 `gapDetection` flag into a live take
(HANDOFF addendum 3). Decide exactly what step 3 needs from Lane 3 (`manualWrite`) and whether it can
proceed without it. Cut builder lanes with DISJOINT file ownership. Give a live-app verification recipe
(production mode, NO output window) and the list of what only Boris can check.

---

## APPROACH (verdict first)

**Step 3 proceeds NOW, without Lane 3, for everything except continuous lanes — and it does so by NOT
living in `MainComponent`.** A new message-thread class `RecorderHost` (`src/recording/RecorderHost.{h,cpp}`)
owns the clock, recorder, store, player and the whole arm→tick→stop lifecycle, and talks to the app only
through a `Dispatch` struct of `std::function`s; `MainComponent` shrinks to ~120 lines of wiring (one tick
call, `Origin` threading on three handlers, five new choke-point methods, the `Dispatch` lambdas). The
continuous half (D6a `manualWrite` sites, `Sink::touch/set/release`) is a **`std::function` seam the host
leaves EMPTY and refuses LOUDLY** (counted, shown in status) until the CONNECTION lane's `ManualWriter`
lands — R8 honoured to the letter: the connection lane DEFINES the funnel; the recorder SUBSCRIBES to its
`onWrite` observer. The minimal Lane 3 slice that unblocks the continuous half ("Lane S3-M" below) is
L3(e) alone — a ~½-day `src/connect/ManualWriter.{h,cpp}` + ten one-line site replacements — and it needs
NEITHER the renderer read-repoint nor the `ConnectionEngine::tick` wiring (both stay L3(a)-(d)). Four lanes,
disjoint files: **S3-A** recording-side (new files + two accessors on `PerformanceRecorder`), **S3-B**
`MainComponent.{h,cpp}` only, **S3-C** `ApiServer.{h,cpp}` only (REST `/api/perf/*`, pulled forward from
build-order row 5 because it is the ONLY production-mode trigger surface step 3 can be verified through —
RecordPanel is L5's file and step 4's scope), **S3-M** the connection lane's funnel. Live verification is
a REST-driven recipe on port 7070 in production mode with the analysis thread live, no output window,
graceful quit, screen verified by `screencapture`.

Answer to the dispatch's central question, stated plainly: **Lane 3 does not have to land first. Step 3's
discrete lanes, the audio lifecycle, the R14 flag, the provisional/periodic save, the REST surface and
replay of every button/trigger/tempo/deck event are buildable and verifiable today. Only the continuous
half waits, and it waits for a half-day slice of Lane 3 — not for Lane 3.** A stub of `manualWrite`
written BY step 3 is forbidden by R8 and is also unnecessary: the recorder's own seam (an empty
`std::function` that refuses and counts) is not a definition of `manualWrite`, it is the hook the spec
says the recorder owns.

---

## 0. GROUND TRUTH THIS PLAN STANDS ON (all VERIFIED unless marked)

| # | Fact | Cite |
|---|---|---|
| T1 | `manualWrite`/`writeManual`/`touchScalar` exist NOWHERE in `src/` or `tests/` | grep, 0 hits |
| T2 | `MainComponent.{h,cpp}` reference NO recorder class: no `RecorderClock`, `PerformanceRecorder`, `AudioTap`, `AudioStore`, `Player`, `ConnectionEngine` | grep over both files, 0 hits |
| T3 | `tickFeaturePipeline` body = read snap → `signalRegistry_.evaluateAll(snap)` → `MappingEngine::processFrame` → `globalMacroBank_.updateValues` → `inspectorPanel_->tickModulation()`. `ConnectionEngine::tick` is NOT called (L3(b) unlanded) | `MainComponent.cpp` anchor `void MainComponent::tickFeaturePipeline` |
| T4 | Analysis thread starts ONLY in production: `if (!testMode_) … analysisThread_.startThread(...)`; `--test-mode` never starts it | `MainComponent.cpp:1762-1765` (anchor `startThread(juce::Thread::Priority::high)`) |
| T5 | Production boots in mic mode (`setSourceMode(MicInput)` at ctor) and REST 7070 binds regardless of test mode; `/api/inject_features` is registered only in test mode | `MainComponent.cpp:153`; `ApiServer.cpp:117-224` (`inject_features` at `:177` inside a test-mode branch) |
| T6 | REST 7070 production endpoints usable as a live driver: `trigger_clip`, `trigger_column`, `switch_deck`, `set_bpm`, `set_layer_opacity`, `set_param`, `load_composition`, `bpm`, `features`, `composition`, `status`, `health` | `ApiServer.cpp:117-224` |
| T7 | REST `set_param` (clip branch) and `set_layer_opacity` write the MODEL directly inside ApiServer via `callAsync` — they never pass through MainComponent | `ApiServer.cpp` anchors `void ApiServer::handleSetParam` (clip branch `fx.paramValues[pi] = value`) and `void ApiServer::handleSetLayerOpacity` (`lay->opacity = opacity`) |
| T8 | The D6a manual-writer sites, complete list (10): OSC `onSetMaster` (`composition_.masterOpacity = level`), OSC `onSetLayerOpacity` (`layer->opacity = opacity`), OSC `onSetMacro` (`manualValue = value`), binding `TriggerClip` velocity→`clip->clipOpacity`, binding `TriggerColumn` velocity→`clip->clipOpacity`, binding `MasterOpacity`, binding `AdjustLayerOpacity`, binding `AdjustMacro`, REST `set_layer_opacity`, `LayerStrip.cpp:423` (`opacitySlider_.onValueChange`) — plus REST `set_param`'s clip branch (T7) = 11 | grep `clipOpacity = \|masterOpacity = \|->opacity = \|manualValue = ` over `src/MainComponent.cpp src/api/ApiServer.cpp src/ui/LayerStrip.cpp` |
| T9 | `LayerStrip` opacity slider is a `ResettableSlider` with `onValueChange` only — no `onDragStart`/`onDragEnd` → today it can only be a Decaying grip unless those two JUCE callbacks are added | `LayerStrip.cpp:414-425` |
| T10 | Tempo writers (G11) today: TopBar `onTapTempo` → `setManualBPM`; `onManualBpmChanged` → `setManualMode`+`setManualBPM`; `onResync` → `beatCounter_=0; lastBeatPhase_=0; resetBeatPhase(); resetPhrase()`; REST `onSetBpm` and OSC `onSetBpm` → `setManualMode(true)`+`setManualBPM`; Link (30 Hz `timerCallback`) → same pair; binding `TapTempo` (own 8-tap averager) → `setManualBPM`; binding `Resync` → same as TopBar | `MainComponent.cpp` anchors `topBar_->onTapTempo`, `onManualBpmChanged`, `onResync`, `apiServer_->onSetBpm`, `oscHandler_.onSetBpm`, `linkSync_.isEnabled()`, `case Binding::Action::TapTempo`, `case Binding::Action::Resync` |
| T11 | Audio transport writers: `audioEngine_.play()` at 4 file-load sites (drop/open/deck-file) + binding `GlobalPlayPause` (toggles play/**stop**, not pause) + `GlobalStop`; the ONLY `setPosition` writer in the tree is `AudioEngine::stop()` → `setPosition(0.0)` — NO forward-scrub surface exists | `MainComponent.cpp:176-187, 512-523, 3034-3040, 3408-3413, 5860-5874`; `AudioEngine.cpp:67`; grep `setPosition(` over `src/` |
| T12 | TopBar Play/Pause/Stop are CLIP transport for every layer's active clip on the active deck (`clip->playing`, `clip->playheadPosition = inPoint`), NOT the audio engine | `MainComponent.cpp` anchors `topBar_->onPlay/onPause/onStop` |
| T13 | Layer-flag writers: OSC `onSetLayerBypass/Solo/Mute` (absolute) and binding `ToggleLayerBypass/Solo/Mute/Autopilot/Visible` (flip); effect bypass: binding `ToggleEffectBypass` on the ACTIVE clip's `effects[targetEffectIndex].bypassed`; per-clip play: binding `LayerTransport` flips `clip->playing`; LayerStrip/ClipInspector buttons write `playing` directly (UI, scrapped) | `MainComponent.cpp` anchors `oscHandler_.onSetLayerBypass`, `case Binding::Action::ToggleLayerBypass`, `case Binding::Action::ToggleEffectBypass`, `case Binding::Action::LayerTransport`; `LayerStrip.cpp:359-377`; `ClipInspector.cpp:39-50` |
| T14 | `handleClipTrigger(layer, col)` reads ONLY the active deck, pushes `TriggerClipCmd` when runtime changed; `handleColumnTrigger(col)` → `deck->triggerColumn` + one child cmd per non-ignoring layer; `handleDeckSwitch(idx)` pushes NO command (the deck-tab wrapper `deckView_->onDeckSwitched` does) — every REST/OSC/MIDI/genre path calls these three directly | `MainComponent.cpp` anchors `void MainComponent::handleClipTrigger`, `handleColumnTrigger`, `handleDeckSwitch`, `deckView_->onDeckSwitched`; callers at `:676, :693, :696, :1403, :1800-1802, :1830-1833, :5719, :5755, :5817` |
| T15 | `quantizeMode` has NO writer in `MainComponent.cpp` and `topBar_->onQuantizeChanged` is never assigned; `onBpmMultiplierChanged` likewise (the /4../x4 buttons are dead — Boris ruling 4 says build, not this step) | grep `onQuantizeChanged\|quantizeMode = \|multiplier` in `MainComponent.cpp` → 0 assignment hits; `TopBar.h:27-28` |
| T16 | `PerformanceRecorder` has NO accessor to its in-progress `Take` and NO `checkpoint0` setter; `start()` resets `take_ = Take{}`; `stop()` returns it by value. Without an accessor the v2 §5.6 provisional/periodic save cannot be written | `PerformanceRecorder.h:28-61`; `.cpp:12-23, 147-165` |
| T17 | `PerfState` has NO capture-from-Composition function ("step 3's job"); `PerfState.cpp` is linked by targets that do NOT link the model (`test_audio_store`, `test_audio_tap_sync`) → a capture function MUST live in a separate TU | `PerfState.h:16-19`; `tests/CMakeLists.txt` `test_audio_store` block (`AudioStore.cpp AudioTap.cpp Take.cpp TempoMap.cpp PerfState.cpp`, no `Clip.cpp`) |
| T18 | `AudioEngine` exposes `getDeliveredSamples()`, `getAudioTap()`, `getTransportSource()`, `getCurrentSampleRate()`, `loadFile/play/pause/stop`, `setSourceMode`; `setSourceMode` re-opens the device (`setAudioDeviceSetup(setup, true)`) → `audioDeviceAboutToStart` → `tap.prepare(rate, activeOutputChannels, block)`; `prepare()` with a DIFFERENT rate/channels while a take runs SELF-STOPS the tap | `AudioEngine.h:19-53`; `AudioEngine.cpp` `setSourceMode`; `CombinedCallback.h:142-160`; `AudioTap.h:49-64` |
| T19 | `AudioStore` API landed exactly as v2 §4.1 (beginAsset/activeAssetId/abandonAsset/CaptureFacts/finalize/find/resolve/referencing/fingerprint/scanTakes); `defaultRoot()` = `~/Documents/Audio-DNA/Audio`; `beginAsset` creates the root | `AudioStore.h` (whole file); `AudioStore.cpp:88-125` |
| T20 | `Take::save(folder)` creates the folder and writes `take.json` via `replaceWithText` (atomic, `TemporaryFile`+rename); `Take v3/3`; `AudioRef::Segment{id,fingerprint,file,firstSample,frames,rate,channels}`; `Meta{recordedAt,app,duration,durationBeats}`; `Take::markers` exists | `Take.cpp:284-289`; `Take.h:20-56, 91, 95-96` |
| T21 | `Player` has `start/advanceTo/seek/stop/swap/setOverride([[nodiscard]] bool)/reenable/position/running/report`; `advanceTo` treats any decrease as a SEEK; a deliberate FORWARD jump needs an explicit `seek()` first | `Player.h:40-92` |
| T22 | `Sink{fire,touch,set,release}` — `bool` = D8 "accepted"; `Fired{at,seq,key,target(ResolvedTarget{deck,layer,col,fx,param,macro}),p}` by value | `Player.h:16-23`; `Program.h:29-46` |
| T23 | `Program::compile` resolves Comp scope as always-addressable; Clip/Layer scopes by `deck.i`/`deckRelative` + `deckName`, `layer.i` + `layerName`, `col.i` + `clipName`, `fx.i` + `fxName`; Macro/Routine scopes report unresolved in row 1 | `Program.cpp:22-130` |
| T24 | `DiscretePoint.v` is `int`; `action` is a string; `bpm` = the clock's bpm at capture (D1 semantics) — a tapped BPM cannot be stored losslessly in `v` without a unit rule | `Lane.h:79-90`; `s167 spec:112-117` |
| T25 | `Origin{Human,Replay,Routine,Engine,Preamble}` lives in `recording/Lane.h`; `ControlPath.control` is free-form on purpose ("only the dispatcher needs to recognise it") | `Lane.h:50`; `ControlPath.h:41-45` |
| T26 | `ParamConnection` grip API: `gripHeld()`, `gripTouch(now)` (refused while Held), `release(now)`; Decaying expiry is done ONLY inside `ConnectionEngine::evaluate` (`now − lastTouch ≥ gripHoldMs/1000`) — which is never called in the app today (T3) → a Decaying grip set today never expires | `ParamConnection.h:159-167`; `ConnectionEngine.cpp:75-80` |
| T27 | Hand-back GLIDE on release is implemented ONLY in `ConnectionEngine::evaluate` (`handBackFrom`/`handBackStart`) → no glide exists in the app until L3(b) ticks the engine | `ConnectionEngine.cpp:155-173` |
| T28 | `RecordPanel` is L5's file (staged: five buttons disabled with "coming" tooltips; callbacks `onStartRecording/onStopRecording/onPlayRecording` declared, never assigned); `BrowserPanel::getRecordPanel()` exposes it | `git show :src/ui/RecordPanel.h`; `BrowserPanel.h:36,59` |
| T29 | Builders here run in git worktrees (`.claude/worktrees/wf_*`), so same-file edits in different lanes are merged at integration; the HANDOFF rig fact still requires each lane to configure its own `-B` dir | `git worktree list` (two worktrees present); `HANDOFF.md:2493` |
| T30 | `tests/CMakeLists.txt` currently ends with H1's STAGED `test_bt_device_shapes` block (uncommitted); any further append must land AFTER that commit | `git diff --cached -- tests/CMakeLists.txt` |
| T31 | App version string is `"0.1.0"` at `Main.cpp:8` (`getApplicationVersion`) and duplicated literally in `ApiServer.cpp:234` | both files |
| T32 | Live production launch + `/api/bpm` on 7070 is a PROVEN, screen-safe recipe (row 4 of `.harmony/VALIDATION.md`): `totalBarCount` 2→8 over 10 s on the built-in mic; `/api/health` answered on `127.0.0.1:7070` in the crash-diagnosis runs | `.harmony/VALIDATION.md` row 4; `.harmony/.reports/s-rta-0923-startup-crash-diagnosis.md` §1 |
| T33 | The startup crashes of 21:17-21:19 were two signatures (JUCE combiner SIGSEGV on a BT device event; malloc `free_list_checksum_botch`), triggered only with the `soundcore P31i` 16 kHz HFP headset as default device; H1 found and fixed (staged) a real heap-overflow READ in `AudioTap::writeFrames` (unchunked silence padding) | crash-diagnosis report §2-3; `git diff --cached -- src/recording/AudioTap.cpp`; `H1.review.md` |

---

## 1. THE `manualWrite` QUESTION — precise answer

**What step 3 needs from a manual-write funnel (the recorder's four asks):**
1. ONE choke point every manual write to a continuous control passes through, carrying `(ControlPath key,
   float normalised value, GripKind, Origin)`;
2. a `bool` return = the D8 verdict ("refused: a more deliberate hand holds it");
3. an OBSERVER the recorder can subscribe to (`onWrite(key, v, grip, origin, accepted)`) so capture is a
   subscription, not an edit inside the funnel;
4. grip semantics that do not freeze replay while the ConnectionEngine is un-ticked (T26): a Decaying grip
   must be treated as expired after `gripHoldMs` INSIDE the funnel's own verdict, or a single MIDI touch
   would refuse every replayed `set()` on that control forever.

**Who may build it:** R8 as ruled in s168 (`HANDOFF.md:2459-2460`): the CONNECTION lane. So step 3 may not
define it. But nothing in step 3's DISCRETE half touches it: `activeClip`, `activeDeck`, `tempo`, `audio`,
layer flags, effect bypass, clip play go through their OWN choke points (D6b — `applyTempoCommand`,
`applyAudioTransport`, `applyLayerFlag`, `applyEffectBypass`, `applyClipPlaying`, and the `Origin`-threaded
trigger/deck handlers), all of which step 3 owns outright.

**Decision:**
- **Step 3 half A (discrete + clock + audio lifecycle + store + provisional/periodic save + R14 + REST +
  discrete replay): proceeds now, depends on nothing in Lane 3.**
- **Step 3 half B (continuous capture; `Sink::touch/set/release`): BLOCKED on Lane S3-M = L3(e) only**
  (`ManualWriter` + site instrumentation + the two wiring lines). Not on L3(a)-(d). The host ships half B's
  SEAM now: `RecorderHost::Dispatch::continuous` (three `std::function`s) left empty → every replayed
  continuous gesture returns `false` (Player marks the lane displaced — its existing, tested TOUCH-refusal
  path, `Player.h:99`), the host increments `continuousUnavailable`, and `/api/perf/status` shows it. Loud,
  additive, no stub of `manualWrite` anywhere.
- **Minimal stub ruled OUT** (would be step 3 defining the funnel — R8). **Full Lane 3 ruled OUT as a
  prerequisite** (L3(a)-(d) are renderer/UI/TestServer work with no bearing on capture).
- Interim behaviour once S3-M lands but L3(b) has not: hand-back on release is a SNAP to the lane's
  next `set()` (no glide, T27) — exactly D14's "with L2 absent" interim (`s167 spec:748-750`); the glide
  arrives with L3(b), no recorder change needed.

Strongest counterargument: "just let the step-3 builder write `manualWrite` in MainComponent as D6a
literally says (`s167 spec:322-326`) and let Lane 3 add the grip later." Loses on the s168 ruling (R8 was
re-decided AFTER the spec was written, `HANDOFF.md:2459`) and on mechanics: the ten writer sites are the
connection arc's grip sites, and a second owner editing the same ten lines a week apart is the exact
merge-collision class R8 exists to prevent.

---

## 2. TRADEOFFS CONSIDERED

- **`RecorderHost` class vs. everything in `MainComponent` (the spec's literal placement).** Host chosen:
  (a) `MainComponent.cpp` is ~6000 lines and is the file BOTH S3-M and step 4 must also edit — disjoint
  ownership is impossible if step 3's logic lives there; (b) a host with a `Dispatch` seam is headless-
  testable under ctest (the spec's row-3 acceptance is app-only — the weakest evidence class this repo
  accepts; s93 method); (c) it survives the UI rewrite (ruling 5: mechanism over UI). Cost: one extra
  indirection per dispatched event (~10/s) — nil.
- **REST `/api/perf/*` now (row 5) vs. after RecordPanel (row 4).** Now: it is the ONLY production-mode
  arm/stop/play surface that does not touch L5's file; without it step 3 cannot be verified in the app at
  all. Row 4 (RecordPanel real) then becomes UI over the same host API — nothing is thrown away.
- **Status read for `GET /api/perf/status`: mutex-guarded copy vs. `callAsync`+wait.** Mutex (message
  thread writes once per tick, HTTP thread reads): both threads are off the hot path (sacred rule 2 forbids
  new mutexes ON hot paths, not between two non-RT threads); a blocking round-trip would stall the HTTP
  worker during modal dialogs (R10). Same posture as `VideoRecorder`'s encoder-side locking.
- **Tempo point payload: centi-BPM in `v` vs. a new float field on `DiscretePoint`.** Centi-BPM
  (`v = lround(bpm*100)`, documented on the `tempo` control) chosen: `Lane.h` stays untouched (ADD-never-
  REDEFINE needs no new field), the dispatcher and capture are in ONE lane so the unit cannot drift, and
  L1's bridge precedent already fixed `audio` points at `v = 0`. Rejected: reuse the point's `bpm` field —
  D1 defines it as the clock's bpm at capture; redefining it is what D12 forbids.
- **`audio` (transport) points during replay-with-audio: fire vs. skip.** SKIP (counted as `skipped`,
  reason shown): in file mode the app's transport IS the show's audio source, so the captured WAV already
  embodies every play/pause/stop; replaying `stop` would halt the very transport the Player is clocked
  from (§5.4) and freeze replay. On wall-clock replay they fire (whatever file is loaded). Boris Q4.
- **Overdub scrub policy (v2 R3):** no scrub surface exists (T11); the one backward jump (`stop()` →
  position 0) is REFUSED while an overdub is armed (loud message), so `sample` stamps stay monotonic.
  Rejected: "treat as record-over" — that is the editor's mode (ruling 18) and needs the editor.
- **Onset markers for T2 (`origin:"engine"` points, D14 LATER) as an opt-in arm flag vs. deferring T2
  entirely.** Opt-in flag (`onsetMarkers:true` → `take.markers`, never a lane): ~15 lines, makes T2
  runnable over REST without a human, no format change (`Take::markers` exists, T20).
- **`handleClipTrigger` gains `deck` as a 4th defaulted parameter vs. a new overload.** Defaulted param
  (`Origin = Human, deck = -1 → active`): all nine existing callers compile unchanged; reviewer grep is
  one pattern.

---

## 3. DECISION / SPEC

### 3.1 New class — `src/recording/RecorderHost.{h,cpp}` (Lane S3-A) — the contract

Message-thread only (same `jassert` idiom as `PerformanceRecorder.cpp:8-10`). Includes only
`recording/*` + `juce_core` + `<mutex>`; NEVER includes `MainComponent.h`, anything under `src/ui/`,
`src/connect/`, or `src/render/`. Signatures are the contract; Lane S3-B codes against this header.

```cpp
#pragma once
#include "recording/PerformanceRecorder.h"
#include "recording/RecorderClock.h"
#include "recording/AudioStore.h"
#include "recording/AudioTap.h"
#include "recording/Player.h"
#include "recording/Program.h"
#include <juce_core/juce_core.h>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

struct Composition;
struct FeatureSnapshot;

// Mirrors ParamConnection::Grip::Kind's two ACTIVE values without including connect/ (kept decoupled;
// Lane S3-M maps between them in MainComponent).
enum class GripKind : uint8_t { Held, Decaying };

class RecorderHost
{
public:
    // ---- what the app plugs in (MainComponent, Lane S3-B); every function runs on the message thread ----
    struct Dispatch
    {
        // Discrete replay. The host only ever dispatches Origin::Replay; MainComponent's handler
        // re-resolves f.target by coordinate (G7) and calls the SAME handler the human path uses.
        // Returns accepted (false = refused/unresolvable at fire time -> counted as `skipped`).
        std::function<bool(const Fired& f)> fire;

        // Continuous replay -- LEFT EMPTY until Lane S3-M lands (R8: the connection lane owns
        // manualWrite). Empty => touch/set return false, release is a no-op, and
        // Status::continuousUnavailable counts every refusal. Never a stub of manualWrite.
        struct Continuous
        {
            std::function<bool(const ControlPath&, GripKind)> touch;
            std::function<bool(const ControlPath&, float v)> set;      // v normalised [0,1]
            std::function<void(const ControlPath&)> release;
        } continuous;

        // checkpoint0 / checkpointEnd (D4) from the live model -- implemented by MainComponent via
        // PerfStateCapture (3.2). Called at arm and at stop.
        std::function<PerfState()> capturePerfState;

        // One-line human-readable notices (status line / log). Never a modal.
        std::function<void(const std::string&)> notify;
    };
    Dispatch dispatch;

    explicit RecorderHost(AudioStore store);   // MainComponent passes AudioStore(AudioStore::defaultRoot())

    // ---- recording lifecycle (v2 spec 5.1 / 5.2 / 5.5 / 5.6) ----
    struct ArmOptions
    {
        juce::File takeFolder;                 // ~/Documents/Audio-DNA/Takes/<name>.adna-take (MainComponent picks)
        bool audio = true;                     // ruling 19's switch; default ON
        std::string audioMode;                 // "input" | "file" -- AudioEngine::getSourceMode() at arm
        double deviceRate = 0.0;               // AudioEngine::getCurrentSampleRate() at arm (CaptureFacts.rate)
        int deviceChannels = 0;                // active output channels at arm (CaptureFacts.channels)
        std::string appVersion;                // "0.1.0" (Main.cpp:8)
        bool onsetMarkers = false;             // T2 enabler: take.markers point per snap.onsetDetected (origin Engine)
        std::optional<std::string> overdubAssetId;   // 5.2: record a FRESH take against a stored asset; NO tap; clock = asset frame
    };
    struct ArmResult { bool ok = false; std::string error; std::string assetId; juce::File takeFolder; };
    // Sequence (5.1): beginAsset -> tap.start(store.wavFile(id)) [refuse + abandonAsset on failure] ->
    // recorder.start -> checkpoint0 = dispatch.capturePerfState() -> PROVISIONAL take.json (5.6 #1) with
    // audio = referencing(stub{id, fingerprint "", frames 0, rate, ch, mode, gapDetection = tap.gapDetectionSupported()}, 0).
    // With audio == false: 5.5 (no store, no tap, take.audio default). With overdubAssetId: 5.2
    // (store.find -> refuse with reason if absent; take.audio = referencing(*asset, 0); no tap).
    ArmResult arm(const Composition& comp, AudioTap& tap, const ArmOptions& opts);

    struct StopResult
    {
        bool ok = false; std::string error;            // error = finalize/save problems, NEVER blanks the reference (D-A11)
        juce::File takeFolder; std::string assetId; uint64_t frames = 0; bool gapDetection = false;
        int gaps = 0; int lanes = 0; int points = 0; double duration = 0.0;
    };
    // Sequence (5.1): endedEarly = tapWasStarted && !tap.isRunning(); tap.stop(); facts{...} incl. the
    // per-tick-drained gaps; if endedEarly: unreliableFrom = min(existing, firstSample+framesWritten);
    // fin = store.finalize(id, facts); take = recorder.stop(comp); take.checkpointEnd =
    // dispatch.capturePerfState(); take.audio = referencing(fin.asset, tap.firstSample()) [EVERY branch];
    // meta{recordedAt, app, duration, durationBeats}; take.save(folder); surface fin.error via notify.
    StopResult disarm(const Composition& comp, AudioTap& tap);
    bool isRecording() const;

    // ---- the 120 Hz tick (FIRST thing in tickFeaturePipeline -- 3.3) ----
    // deliveredSamples: AudioEngine::getDeliveredSamples(). transportFrames: AudioTransportSource::
    // getNextReadPosition() when the audio transport is the clock (play-with-audio, overdub), else
    // nullopt. deviceRate: for the 5.2 asset-frame conversion. Does, in order: (1) clock.tick(snap,
    // wallNow, sampleForClock) where sampleForClock = deliveredSamples normally, or the asset frame
    // (llround(transportFrames * asset.rate / deviceRate)) while overdubbing (5.2 formula, VERIFIED
    // AudioEngine.cpp:48-50 + juce_AudioTransportSource.cpp:189-198 per the v2 spec); (2) while
    // recording: drain tap.popGap into facts (5.6 #2), onset marker if armed and snap.onsetDetected,
    // periodic save every kCheckpointSeconds of clock t (5.6 #1); (3) while playing: pos = wall or
    // (assetFrame + firstSample) per DriveClock; player.advanceTo(pos, sink) -- backward pos is
    // Player's own seek (T21); (4) publish Status under the mutex.
    void tick(const FeatureSnapshot& snap, double wallNow, uint64_t deliveredSamples,
              AudioTap& tap, std::optional<int64_t> transportFrames, double deviceRate);

    // ---- capture (called by MainComponent's choke points; message thread) ----
    // Discrete: the caller fills v/action/retrigger/origin/group; stamps are minted here from the
    // clock (PerformanceRecorder::discrete). Ignored unless recording AND origin != Replay (D6 origin
    // rule; R5 overdub feedback is closed by construction, not by the caller remembering).
    void capture(const ControlPath& key, DiscretePoint&& p);
    uint64_t nextGroupId();                              // shared `group` for a column trigger's points
    // Continuous: Lane S3-M's ManualWriter::onWrite subscriber calls these (origin Human only).
    void onHumanTouch(const ControlPath& key, GripKind g);
    void onHumanSet(const ControlPath& key, float v);
    void onHumanRelease(const ControlPath& key);
    void marker(const std::string& action);              // take.markers, origin Engine (T2)

    // ---- playback (5.4) ----
    struct LoadResult { bool ok = false; std::string error; LoadStats stats; AudioStore::Resolution audio; };
    LoadResult load(const juce::File& takeFolder);       // Take::load + store.resolve; refusal reasons verbatim (spec 6)
    enum class PlayMode { WallClock, WithAudio };
    struct PlayResult
    {
        bool ok = false; std::string error;
        // WithAudio: MainComponent must loadFile(wav) in File mode and play() BEFORE the first tick with
        // transportFrames; the host compiles DriveClock::Sample and expects assetFrame from tick().
        juce::File wav; double assetRate = 0.0; uint64_t firstSample = 0;
        CompileReport report;
    };
    PlayResult play(PlayMode mode, const Composition& comp);   // compile + Player::start(0)
    void stopPlay();                                           // Player::stop(sink) -> every touch released (R9)
    bool isPlaying() const;
    // Repair (5.4 Incomplete): store.finalize(id, *CaptureFacts::fromAudioRef(loaded.audio, app))
    std::string repairLoadedAudio(const std::string& appVersion);

    // ---- status (ANY thread; mutex-guarded copy published by tick()) ----
    struct Status
    {
        bool recording = false, playing = false, overdub = false;
        std::string takeFolder, assetId, audioMode, playMode, lastError;
        double t = 0.0, beat = 0.0; uint64_t sample = 0; float bpm = 0.0f;
        int lanes = 0, points = 0, gestures = 0, markers = 0;
        bool gapDetection = false; uint64_t framesWritten = 0; int gaps = 0;
        // playback
        double position = 0.0, length = 0.0;
        int unresolved = 0, reboundByPosition = 0, reboundByName = 0, invalid = 0;
        int skipped = 0, continuousUnavailable = 0, refusedByHand = 0;
        std::string audioStatus;   // AudioStore::Status name + reason
    };
    Status status() const;

    // MainComponent's destructor calls this FIRST (before the audio device closes): disarm if
    // recording (flush + finalize + save), stopPlay if playing.
    void shutdown(const Composition& comp, AudioTap& tap);

    static constexpr double kCheckpointSeconds = 60.0;   // 5.6 #1 periodic Take::save

private:
    struct HostSink;                                     // implements Sink; forwards to `dispatch`; skips `audio`
                                                         // points while PlayMode::WithAudio (counted as skipped)
    // ...clock_, recorder_, store_, tap facts (mode, rateAtStart, channelsAtStart, gaps, onsetMarkers),
    // take folder, asset id, lastCheckpointT_, overdub asset, loaded Take, program_, player_, playMode_,
    // playStartWall_, sink_, counters, mutable std::mutex statusMutex_, Status published_
};
```

Rules the implementation MUST follow (each is a test in 3.6):
- **R-A1 Arm order is fixed:** `beginAsset` → `tap.start(store.wavFile(id))` → on failure
  `store.abandonAsset(id)` + refuse (nothing else touched) → `recorder.start` → `setCheckpoint0` →
  provisional save. If the provisional save fails, arming still succeeds (audio is running) but
  `notify` says so and `Status.lastError` carries it.
- **R-A2 The reference is never blank** (D-A11): every save (provisional, periodic, final) writes
  `take.audio` via `AudioStore::referencing(...)`; the periodic save refreshes the stub with
  `frames = tap.framesWritten()`, `firstSample = tap.firstSample()`, the gaps drained so far and
  `gapDetection = tap.gapDetectionSupported()` — so a crash at any second leaves a take from which
  `CaptureFacts::fromAudioRef` repairs the sidecar (D-A3) and the R14 flag is on disk from the first
  periodic save (HANDOFF addendum 3).
- **R-A3 Periodic snapshot = `recorder_.current()` copied, plus audio/meta/checkpoint0.** Open gestures
  live in `openGestures_` and are NOT in the snapshot (≤ one in-progress gesture is lost on a crash —
  documented; finished gestures and every discrete point are).
- **R-A4 `popGap` drained EVERY tick** while recording (T30-class capacity: 63 markers, v2 §5.1).
- **R-A5 `isRunning()` read BEFORE `stop()`**; `endedEarly` → `unreliableFromInTakeClock = min(...)`.
- **R-A6 Overdub:** no tap; `firstSample = 0`; clock sample = asset frame (5.2 formula); `arm` refuses if
  `store.find(id)` fails or the resolution is not Resolved/ResolvedUnverified.
- **R-A7 Sink policy:** `Origin::Replay` on every dispatch; `audio`-control Fired skipped while
  `WithAudio`; `Macro`/`Routine` scopes never reach the sink (compile reports them); unresolvable at fire
  time → `dispatch.fire` returns false → `skipped++` (never silent, D2 policy 3).
- **R-A8 `Player::seek`:** the host never calls it in row 3 (no forward scrub exists, T11); the ONLY
  re-position is `AudioEngine::stop()`, which is REFUSED while overdubbing (3.3 `applyAudioTransport`) and
  is otherwise a backward jump that `advanceTo` already handles (T21). Documented on `tick()`.
- **R-A9 No allocation-per-tick claims:** `tick` may allocate (message thread); the audio thread is
  never touched by this class (only `AudioTap::popGap`'s consumer side, `AudioTap.h:32-35`).

### 3.2 Two small additions in existing recording files (Lane S3-A)

- `src/recording/PerformanceRecorder.{h,cpp}` — ADD two methods, nothing else changes (T16):
  ```cpp
  void setCheckpoint0(PerfState s);           // recording only; copies into take_.checkpoint0
  const Take& current() const { return take_; } // in-progress document (finished gestures + points); host copies it for periodic saves
  ```
  The `folder` argument of `start()` stays (unused today — the host owns the folder; not this lane's cleanup).
- NEW `src/recording/PerfStateCapture.{h,cpp}` — `PerfState capturePerfState(const Composition& comp, float bpm,
  const std::string& audioAction);` fills `activeDeckIndex`, `quantizeMode = int(comp.quantizeMode)`, `bpm`,
  `audioAction`, and per deck/layer: `name`, `activeClipColumn`, `previousClipColumn`, `crossfadeProgress`,
  `pendingTriggerColumn`, `pendingTriggerSnapOverride`, `opacity`, `visible/bypassed/solo/muted/autopilotEnabled`,
  layer-effect manual `paramValues` that differ from the library default (needs `EffectLibrary` — same
  link set as `test_take`, `tests/CMakeLists.txt` `test_take` block), and per clip with non-default state:
  `name`, `playing`, `playheadPosition`, non-default effect params, scalars (`clipScalarDefs()` keys →
  `toNorm(model)`). Separate TU because of T17. Field names are the ones in `Layer.h:34-55,165-172`,
  `Clip.h:58-90,142,203` (VERIFIED).
- CMake: root `CMakeLists.txt` +4 sources after line 247 (`AudioStore.cpp`); `tests/CMakeLists.txt`
  APPEND ONE block `test_recorder_host` at EOF **after H1's block is committed** (T30): sources
  `test_recorder_host.cpp` + the `test_take` source list + `AudioTap.cpp` + `AudioStore.cpp` +
  `AudioCallback.cpp` + `RecorderHost.cpp` + `PerfStateCapture.cpp`; links `test_audio_store`'s module
  set + `juce_graphics` (Clip.cpp needs it, as `test_take` does); definitions as `test_take` +
  `TEST_FIXTURES_DIR`; `apply_sanitizers`; `catch_discover_tests`. Never edit an existing block.

### 3.3 `MainComponent` wiring (Lane S3-B) — every hunk, by anchor

Members (`MainComponent.h`, declared AFTER `syphonOutput_` so it is destroyed FIRST — before
`audioEngine_` (`:222`) and `composition_` (`:334`)):
```cpp
#include "recording/RecorderHost.h"
RecorderHost recorderHost_{ AudioStore(AudioStore::defaultRoot()) };
```
`~MainComponent()`: FIRST statement `recorderHost_.shutdown(composition_, audioEngine_.getAudioTap());`.

**B1 — the tick** (anchor `void MainComponent::tickFeaturePipeline`), inserted immediately after
`const FeatureSnapshot snap = ...read();` and BEFORE `signalRegistry_.evaluateAll(snap)`:
```cpp
// s167 §9.1 ordering: the recorder/player run BEFORE any connection evaluation so a replayed
// gesture's grip is current when the engine (L3(b), when it lands -- insert it AFTER this block)
// decides whether to publish.
{
    const double wallNow = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    std::optional<int64_t> transportFrames;
    if (recorderHost_.isPlaying() || recorderHost_.status().overdub)   // cheap: status() copies under a mutex; or expose host.needsTransport()
        transportFrames = audioEngine_.getTransportSource().getNextReadPosition();
    recorderHost_.tick(snap, wallNow, audioEngine_.getDeliveredSamples(),
                       audioEngine_.getAudioTap(), transportFrames, audioEngine_.getCurrentSampleRate());
}
```
(Builder: add `bool RecorderHost::needsTransportFrames() const` rather than calling `status()` per tick.)

**B2 — Origin + deck on the three handlers** (`MainComponent.h:393-416`):
```cpp
void handleClipTrigger(int layerIndex, int column, Origin origin = Origin::Human, int deckIndex = -1);
void handleColumnTrigger(int column, Origin origin = Origin::Human, int deckIndex = -1);
void handleDeckSwitch(int deckIndex, Origin origin = Origin::Human);
```
Bodies: `deck = deckIndex < 0 ? composition_.getActiveDeck() : &composition_.decks[deckIndex]`
(bounds-checked, else return AND — new — `notify`/log the refusal; G5's silent early-out stays silent only
for the Human path, where the UI already refused). Preview/deck-view refresh only when
`deckIndex == activeDeckIndex`. **Undo push gated:** `if (origin == Origin::Human) pushCommands(...)`
(R5; `s167 spec:868-870`). **Capture:** after the model mutation, `if (origin == Origin::Human)`
(the host also filters — belt and braces) build the key with `makeLayerKey(deckIdx, layerIdx)` =
`{Scope::Layer, deck = idx, deckName = deck.name, layer = l, layerId = layer.id, layerName = layer.name,
control = "activeClip"}` and `recorderHost_.capture(key, {v = column, retrigger = wasRetrigger})`;
`handleColumnTrigger`: one point per non-ignoring layer, all with `group = recorderHost_.nextGroupId()`.
`handleDeckSwitch`: key `{Scope::Comp, control = "activeDeck"}`, `v = deckIndex`, captured only on an
actual change (the same-deck no-op switch records nothing). All nine callers (T14) compile unchanged.
Momentary release (`case TriggerClip/TriggerColumn`, value == 0 branches) → captured as `activeClip v = -1`
via a new tiny helper `applyClearActiveClip(layerIdx, origin)` that wraps the existing inline body.

**B3 — five choke points (D6b), each `Origin`-tagged, each capturing when Human:**
- `void applyTempoCommand(const TempoCmd& c, Origin)` with `TempoCmd{action ∈ {tap, manual, auto, resync,
  link}, float bpm}`. Replaces the direct tracker writes at ALL sites in T10 (TopBar tap/manual/resync,
  REST `onSetBpm`, OSC `onSetBpm`, Link tick, binding TapTempo/Resync). Dispatch: `tap|manual|link` →
  `setManualMode(true)` (tap keeps today's behaviour: `setManualBPM` only — builder preserves exactly
  what each site does today, the choke point is a MOVE not a change) ; `auto` → `setManualMode(false)`;
  `resync` → `beatCounter_ = 0; lastBeatPhase_ = 0; resetBeatPhase(); resetPhrase()`. Capture key
  `{Comp, "tempo"}`, `action` as above, **`v = lround(bpm*100)` (centi-BPM; 0 for auto/resync)** —
  the unit is documented at the capture site AND the dispatch site. Link (R3): capture only when
  `|bpm − lastLinkCapturedBpm| ≥ 0.01`.
- `void applyAudioTransport(const std::string& action /*play|pause|stop*/, Origin)` → the audio ENGINE
  (`audioEngine_.play/pause/stop`). Sites: binding `GlobalPlayPause` (keeps its play/stop toggle),
  `GlobalStop`, and the four file-load auto-plays (record `play`). Key `{Comp, "audio"}`, `v = 0`
  (matches L1's bridge). **While an overdub is armed, `stop` is REFUSED with a notice** (R-A8/v2 R3).
- `void applyLayerFlag(int layerIdx, const std::string& flag /*visible|solo|mute|bypass|autopilot*/,
  bool value, Origin)` — sites: OSC bypass/solo/mute (absolute), binding Toggle* (flip = `!current`).
  Key `{Layer, control = flag}`, `v = value`.
- `void applyEffectBypass(int layerIdx, int col, int fx, bool value, Origin)` — site: binding
  `ToggleEffectBypass` (col = `layer->activeClipColumn` at capture, so the key is exact). Key
  `{Clip, deck/layer/col/fx + fxName = slot.effectName, control = "bypass"}`, `v`.
- `void applyClipPlaying(int layerIdx, int col, const std::string& action /*play|pause|stop|reverse*/,
  Origin)` — sites: binding `LayerTransport` (flip), TopBar `onPlay/onPause/onStop` (one point per layer,
  shared `group`). Key `{Clip, ..., control = "playing"}` (ADD to the D2 vocabulary — allowed by
  D12 "ADD, never REDEFINE"; documented in the dispatcher's vocabulary comment), `v = playing ? 1 : 0`,
  `action` as given. LayerStrip/ClipInspector transport buttons are NOT hooked (D14: widgets are scrapped).
- `quantize` (D14 NOW list): T15 shows NO writer reachable from MainComponent → captured in
  `checkpoint0.quantizeMode` only; the dispatcher still ACCEPTS a `quantize` Fired (`v` →
  `composition_.quantizeMode`) so an edited take can set it. Disclosed, not silent.

**B4 — the `Dispatch` lambdas** (in the ctor, near `apiServer_->onTriggerClip`):
`fire` switches on `f.key.control`: `activeClip` → `f.p.v < 0 ? applyClearActiveClip : handleClipTrigger(
f.target.layer, f.p.v, Replay, f.target.deck)`; `activeDeck` → `handleDeckSwitch(f.p.v, Replay)`; `tempo`
→ `applyTempoCommand({f.p.action, f.p.v / 100.0f}, Replay)`; `audio` → `applyAudioTransport(f.p.action,
Replay)`; `visible|solo|mute|bypass|autopilot` (Layer scope) → `applyLayerFlag`; `bypass` (Clip scope with
`fx ≥ 0`) → `applyEffectBypass`; `playing` → `applyClipPlaying`; `quantize` → set; anything else →
`return false` (counted). `capturePerfState` → `capturePerfState(composition_, snap.bpm, lastAudioAction_)`.
`notify` → log + `statusLabel`-equivalent (the TopBar status text if one exists; else `std::cerr` — UI is
disposable). `continuous` left EMPTY in this lane (Lane S3-M fills it).

**B5 — REST callbacks** (Lane S3-C's new `std::function`s, wired here):
`onPerfRecord(opts) → ArmOptions{takeFolder = ~/Documents/Audio-DNA/Takes/<name or yyyy-MM-dd_HHmmss>.adna-take,
audio, audioMode = getSourceMode()=="file"?"file":"input", deviceRate, channels =
deviceManager.getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits() (the same reading
CombinedCallback.h:154-155 uses), appVersion = JUCEApplication::getInstance()->getApplicationVersion(),
onsetMarkers, overdubAssetId}`; **if `opts.audioFile` is given: `loadFile` → `setSourceMode(File)` →
(device restart, tap re-prepared) → THEN `recorderHost_.arm(...)` → `play()`** — the arm MUST follow the
mode switch (T18: a `prepare()` with changed channels after `start()` self-stops the tap).
`onPerfStop → disarm`; `onPerfLoad → load`; `onPerfPlay(withAudio) → play(); if WithAudio: loadFile(r.wav)
+ setSourceMode(File) + play()`; `onPerfStopPlay → stopPlay() + audioEngine_.stop()`; `onPerfStatus →
recorderHost_.status()` serialised to `juce::var`; `onPerfRepair → repairLoadedAudio`.

Reviewer greps for S3-B (each MUST hold): (i) `setManualBPM\|setManualMode\|resetPhrase\|resetBeatPhase`
outside `applyTempoCommand` → 0 in `MainComponent.cpp`; (ii) `audioEngine_\.\(play\|pause\|stop\)()`
outside `applyAudioTransport` → 0; (iii) every `handleClipTrigger(\|handleColumnTrigger(\|handleDeckSwitch(`
call from a replay/dispatch context passes `Origin::Replay`; (iv) `pushCommands(` inside the three
handlers is guarded by `origin == Origin::Human`; (v) `recorderHost_.capture(` is never reached with
`Origin::Replay` (the host filters too — a test in 3.6 pins it).

### 3.4 REST surface — `src/api/ApiServer.{h,cpp}` ONLY (Lane S3-C)

Pattern: exactly like `trigger_clip` (`ApiServer.cpp:132`, callbacks `ApiServer.h:58-70`): parse JSON,
validate shape synchronously, `callAsync` the callback, respond `ok` — EXCEPT `status`, which reads a
thread-safe copy synchronously. New callbacks on `ApiServer`:
```cpp
struct PerfRecordOpts { juce::String name; bool audio = true; juce::String audioFile; bool onsetMarkers = false; juce::String overdubAssetId; };
std::function<void(const PerfRecordOpts&)> onPerfRecord;
std::function<void()> onPerfStop;
std::function<void(juce::File takeFolder)> onPerfLoad;
std::function<void(bool withAudio)> onPerfPlay;
std::function<void()> onPerfStopPlay;
std::function<void()> onPerfRepair;
std::function<juce::var()> onPerfStatus;        // synchronous; MUST be thread-safe (RecorderHost::status() is)
```
Endpoints: `POST /api/perf/record {name?, audio?, audioFile?, onsetMarkers?, overdubAssetId?}`,
`POST /api/perf/stop`, `POST /api/perf/load {folder}`, `POST /api/perf/play {withAudio?}`,
`POST /api/perf/stop_play`, `POST /api/perf/repair`, `GET /api/perf/status`. Model-state validation moves
to the message thread (the documented sibling convention, `handleSetParam`'s comment block); results are
read back through `status` (`lastError`, `audioStatus`, counts). No `RecorderHost` include in ApiServer.
ApiServer's `this`-capture invariant (ONE lifecycle) is unchanged — note it in the new handlers' comment.

### 3.5 The connection lane's slice — `ManualWriter` (Lane S3-M; CONNECTION lane owns it)

Commit M1 (parallel-safe, new files): `src/connect/ManualWriter.{h,cpp}` —
```cpp
enum class GripKind : uint8_t { Held, Decaying };   // or reuse RecorderHost's -- S3-M picks ONE and maps
class ManualWriter {
public:
    ManualWriter(Composition& comp, MacroBank& globalBank);
    // Resolves `key` by coordinate (deck/layer/col/fx/param/scalar/macro -- same policy as Program.cpp:22-130,
    // names never trusted), converts the NORMALISED value to the model unit (ScalarDef::toModel for scalars;
    // 1:1 for effect/source params, dryWet, macros), applies the grip (gripHeld / gripTouch(now) /
    // release), writes the manual field, returns the D8 verdict. Refuses (false, nothing written) when
    // (a) a Held grip exists and this write is Decaying or Replay, (b) an unexpired Decaying grip exists
    // and this write is Replay, (c) the key does not resolve. Expiry is evaluated HERE
    // (now - grip.lastTouch >= gripHoldMs/1000, the ConnectionEngine.cpp:77-80 formula) so an un-ticked
    // engine can never leave a stale grip refusing replay forever.
    bool write(const ControlPath& key, float normalised, GripKind grip, Origin origin, double now);
    bool touch(const ControlPath& key, GripKind grip, Origin origin, double now);   // grip without a value
    void release(const ControlPath& key, double now);
    std::function<void(const ControlPath&, float normalised, GripKind, Origin, bool accepted)> onWrite;
    std::function<void(const ControlPath&, GripKind, Origin)> onTouch;
    std::function<void(const ControlPath&, Origin)> onRelease;
};
```
Values at the ten sites are already in [0,1] with identity `toModel` (opacities, macro, params) — VERIFIED
against `ScalarParams.h` formulas (`Opacity v`) — so the sites pass the model value as the normalised one.
Tests: append `[manualwriter]` cases to `tests/test_connection.cpp` (existing target, links the model):
resolve+write for comp/layer/clip opacity, a clip effect param, a macro; Held refuses Decaying and Replay;
Decaying older than `gripHoldMs` does NOT refuse Replay; unresolved key → false; `onWrite` fires with
`accepted` both ways. Root `CMakeLists.txt` +2 sources — after S3-A's CMake commit (serialize the two
one-line edits; never concurrent).

Commit M2 (SERIAL, after S3-B and S3-C have landed — it edits their files): replace the eleven writer
lines (T8 + T7's clip branch): MainComponent ×7 → `manualWriter_.write(key, v, Decaying, Origin::Human,
now)` (velocity→clipOpacity: Decaying; MIDI CC: Decaying), `ApiServer.cpp` ×2 (set_layer_opacity,
set_param clip branch — ApiServer gains a `std::function<bool(const ControlPath&, float)> onManualWrite`
so it keeps no `ManualWriter` include), `LayerStrip.cpp:423` → `write(..., Decaying)` PLUS
`opacitySlider_.onDragStart → touch(Held)`, `onDragEnd → release` (T9; JUCE `Slider::onDragStart/onDragEnd`
exist — builder verifies in `juce_Slider.h`). Then the two wiring lines in MainComponent:
`recorderHost_.dispatch.continuous = { touch → manualWriter_.touch(k, g, Replay, now), set →
manualWriter_.write(k, v, Held, Replay, now), release → manualWriter_.release(k, now) }` and
`manualWriter_.onWrite = [&](k, v, g, origin, accepted){ if (origin == Human && accepted) recorderHost_.onHumanSet(k, v); }`
(+ `onTouch → onHumanTouch`, `onRelease → onHumanRelease`). Decaying gestures with no release event: the
recorder synthesises `end` after 250 ms of silence on that lane (`s167 spec:347-350`) — implement in
`RecorderHost::tick` (S3-A: `kDecayingGestureEndMs = 250`), gated on grip == Decaying.

Reviewer grep for M2: `clipOpacity = \|masterOpacity = \|->opacity = \|manualValue = \|paramValues\[.*\] = `
over `src/MainComponent.cpp src/api/ApiServer.cpp src/ui/LayerStrip.cpp` → 0 outside `ManualWriter.cpp`
(inspector widgets under `src/ui/*Inspector*.cpp` are exempt by D14 and stay).

### 3.6 Tests (fail-first where the pre-change code exists; contract tests otherwise)

`tests/test_recorder_host.cpp` (Lane S3-A) — headless, temp dirs under `juce::File::tempDirectory`,
a `FakeDispatch` recording every call, scripted `FeatureSnapshot`s (`makeSnap` idiom, `test_take.cpp:35-41`),
the tap driven through a real `CombinedCallback` + `FakeAudioIODevice` (same shape as
`test_audio_tap_sync.cpp:299-341` / `test_bt_device_shapes.cpp` — copy the helper, it is in an anonymous
namespace and not shared), an `AudioStore` on a temp root:
1. `[host][arm] provisional take.json exists before the first tick and carries the asset id` — after
   `arm()`, `Take::load(folder)` → `audio.segments[0].id == assetId`, `fingerprint == ""`, `mode == "input"`;
   `store.resolve` → `Incomplete` (wav exists, no sidecar) — the repairable state D-A3 promises.
2. `[host][periodic] a save lands every 60 s of clock time and refreshes frames/firstSample/gaps/gapDetection`
   — tick 125 s of scripted wall with blocks pushed; the file's `frames` grows; `firstSample ==
   tap.firstSample()`; `gapDetection` present.
3. `[host][r14] gapDetection false reaches the PERIODIC and the FINAL take` — null `hostTimeNs` device
   (as `test_audio_tap_sync.cpp:299-341`) → both files say `false` (HANDOFF addendum 3 — the production
   glue that did not exist).
4. `[host][gaps] 80 gaps survive the per-tick drain` — mirrors R28 test 16 through the host.
5. `[host][stop] disarm finalizes, references, saves; take folder has no audio.wav; resolve == Resolved;
   asset.frames == framesWritten; checkpointEnd captured`.
6. `[host][arm] tap.start failure refuses arm and abandons the asset` — unwritable/free-space-failing root
   (use a root path inside a read-only temp dir) → `!ok`, folder gone, no take.json.
7. `[host][capture] discrete points get clock stamps; Origin::Replay is never recorded` — `capture` with
   `origin = Replay` → lane count unchanged (R5 by construction).
8. `[host][replay] the same discrete sequence fires through Dispatch in (at,seq) order; audio points are
   skipped in WithAudio and fired in WallClock; skipped count is surfaced`.
9. `[host][overdub] arm against a stored asset uses firstSample 0 and the asset-frame clock` — 44 100 Hz
   asset on a 48 000 device: clock `sample == llround(frames * 44100/48000)` ± 1.
10. `[host][continuous] empty Dispatch::continuous refuses loudly` — Player with one gesture →
    `continuousUnavailable > 0`, no crash, lane displaced (Player's existing TOUCH path).
11. `[host][selfstop] a mid-take rate change ends the audio early` — `prepare(44100,…)` mid-take →
    final `unreliableFrom == firstSample + framesWritten` (v2 §5.1 endedEarly).
12. `[host][status][concurrency] status() from a second thread while tick() runs` — TSan
    (`-DADNA_SANITIZE=thread`, `cmake/Sanitizers.cmake`) zero diagnostics.
13. `[perfstate] capturePerfState from a Composition` — `initDefault()` + one triggered clip + opacity
    0.4 + a bypassed layer → the PerfState round-trips through `toVar/fromVar` with those values.
14. `[host][decaying] a Decaying gesture with no release gets an exact end after 250 ms of silence`.

Acceptance S3-A: `test_recorder_host` green in `build-s3a`; TSan on case 12; count delta MEASURED.
S3-B: app target builds in `build-s3b`; reviewer greps (3.3) hold; the live recipe (§4) is Harmony's gate.
S3-C: builds; the recipe's curl script exercises every endpoint. S3-M: `test_connection` +N green; M2's
grep holds; the continuous rows of §4 pass.

### 3.7 Lane table — DISJOINT file ownership

| Lane | Owns (nothing else) | Depends on | Size |
|---|---|---|---|
| **S3-A** recording side | NEW `src/recording/RecorderHost.{h,cpp}`, NEW `src/recording/PerfStateCapture.{h,cpp}`, `src/recording/PerformanceRecorder.{h,cpp}` (+2 methods), NEW `tests/test_recorder_host.cpp`, `tests/CMakeLists.txt` (append one block, after H1's), root `CMakeLists.txt` (+4 lines) | H1 committed (T30) | M (1-2 d) — commit **A1** (headers + CMake, tree configures) within the first hour, then **A2** (impl + tests) |
| **S3-B** app wiring | `src/MainComponent.h`, `src/MainComponent.cpp` ONLY | A1 (headers), C (callback names) | M (1-1.5 d) |
| **S3-C** REST | `src/api/ApiServer.h`, `src/api/ApiServer.cpp` ONLY | nothing | S (½ d) |
| **S3-M** funnel (CONNECTION lane) | M1: NEW `src/connect/ManualWriter.{h,cpp}`, `tests/test_connection.cpp` (append cases), root `CMakeLists.txt` (+2 lines, after A1) · M2: the eleven site lines in `src/MainComponent.cpp`, `src/api/ApiServer.{h,cpp}`, `src/ui/LayerStrip.cpp` + 2 wiring lines in `MainComponent.cpp` | M1: A1 (CMake serialisation only) · M2: B and C landed | M1 S-M (½-1 d) · M2 S (½ d) |
| **S3-D** docs | `CLAUDE.md` (recording section + source tree), `.harmony/APP-INVENTORY.md` rows, `.harmony/VALIDATION.md` rows | everything | S |

NOT touched by any lane: `src/ui/RecordPanel.*` (L5 in flight / step 4), `src/ui/TopBar.*`, any
`*Inspector*`, `src/render/*`, `src/connect/ConnectionEngine.*`, `Take.*`, `Program.*`, `Player.*`,
`RecorderClock.*`, `AudioTap.*`, `AudioStore.*`, `Lane.h`, `ControlPath.h`, `PerfState.{h,cpp}`.

Order: **Wave 0** Harmony commits (or reverts) the staged H1 + L5 work so every lane branches from a clean
tree. **Wave 1 (parallel)** A1, C, M1-files. **Wave 2 (parallel)** A2, B, M1-CMake+tests. **Wave 3
(serial)** M2. **Wave 4** Harmony: ONE clean forced rebuild in ONE directory, `ctest` (RUN it), then §4.
Conventions per lane: own `-B build-<lane>`; `git add -f` under `.harmony/`; `git show --stat HEAD` per
commit; shared-tree rule (create a file BEFORE the CMake line that references it).

---

## 4. LIVE-APP VERIFICATION RECIPE — production mode, NO output window (Harmony runs this; law-compliant)

Preconditions: Wave 4 build in `build-gate/`; `pgrep -f "MacOS/Audio-DNA"` empty; built-in mic/speakers
are the default device (the BT headset is NOT connected — T33); a click-track WAV at
`/tmp/click_48k.wav` (generate: `.venv/bin/python -c` writing 120 s of 48 kHz stereo silence with a
1-sample 1.0 impulse every 24 000 frames — D10.3's T1 signal, 120 BPM equivalent).

```bash
cd /Users/boriskarpman/projects/RealTimeAudio
open --stdout /tmp/adna-out.log --stderr /tmp/adna-err.log build-gate/AudioDNA_artefacts/Release/Audio-DNA.app   # NO --test-mode (T4)
sleep 6; curl -s -m3 http://127.0.0.1:7070/api/health                                # {"ok":true,...} (T32; try [::1] if empty)
curl -s http://127.0.0.1:7070/api/bpm; sleep 10; curl -s http://127.0.0.1:7070/api/bpm  # totalBarCount increasing => analysis live
# 1. arm with deterministic audio (file mode) + onset markers (T2-shaped)
curl -s -X POST http://127.0.0.1:7070/api/perf/record -d '{"name":"gate1","audio":true,"audioFile":"/tmp/click_48k.wav","onsetMarkers":true}'
sleep 2; curl -s http://127.0.0.1:7070/api/perf/status      # recording:true, assetId 32-hex, gapDetection present, framesWritten>0
ls ~/Documents/Audio-DNA/Takes/gate1.adna-take/take.json    # PROVISIONAL file exists (5.6 #1)
# 2. perform over REST (each is a Human-origin capture)
for c in 1 2 3; do curl -s -X POST http://127.0.0.1:7070/api/trigger_clip -d "{\"layer\":0,\"column\":$c}"; sleep 4; done
curl -s -X POST http://127.0.0.1:7070/api/switch_deck -d '{"deck":1}'; sleep 3
curl -s -X POST http://127.0.0.1:7070/api/switch_deck -d '{"deck":0}'; sleep 3
curl -s -X POST http://127.0.0.1:7070/api/set_bpm -d '{"bpm":128}'; sleep 3
curl -s -X POST http://127.0.0.1:7070/api/trigger_column -d '{"column":2}'; sleep 3
curl -s -X POST http://127.0.0.1:7070/api/set_layer_opacity -d '{"layer":0,"opacity":0.4}'   # continuous: captured ONLY after S3-M
sleep 45   # cross the 60 s periodic-save boundary
stat -f %m ~/Documents/Audio-DNA/Takes/gate1.adna-take/take.json   # mtime advanced past the arm-time write
# 3. stop
curl -s -X POST http://127.0.0.1:7070/api/perf/stop; sleep 3; curl -s http://127.0.0.1:7070/api/perf/status   # recording:false, lastError ""
```
Disk checks (script them; every one is pass/fail):
- `~/Documents/Audio-DNA/Audio/<id>.adna-audio/` has `audio.wav` AND `audio.json`; the take folder has
  ONLY `take.json` (ruling 28).
- `take.json`: `version 3`; `audio.segments[0].{id,fingerprint "fp1:…",frames>0,rate 48000,channels 2}`;
  `audio.gapDetection` is a boolean (R14 glue — true expected on CoreAudio); `audio.mode == "file"`.
- Lanes: `layer/activeClip` (deck 0, layer 0) with ≥ 4 points each carrying `t`, `beat`, `sample`, `bpm`,
  `origin "human"`; the column trigger's points share one `group`; `comp/activeDeck` 2 points (v 1, 0);
  `comp/tempo` 1 point `action "manual"`, `v 12800`; `comp/audio` 1 point `action "play"` (the arm-time
  file play). `tempoMap` has `start` + at least one `periodic` anchor (> 32 beats elapsed at ~120 BPM in
  ~75 s); `checkpoint0.activeDeckIndex == 0`; `checkpointEnd` present; `meta.duration ≈ 75`.
- Alignment (short T2): for each `markers[]` point, `assetFrame = sample − firstSample`; nearest impulse
  in `audio.wav` (read with `.venv/bin/python` + `wave`/`numpy`); report mean offset (expect ≈ 20-30 ms:
  hop + tick + ring depth, D10.3) and drift = offset(last) − offset(first) (expect ≤ 1 ms over 75 s);
  p95 jitter ≤ 15 ms. Numbers go in the gate report; a drift > 1 ms is a FAIL for step 3.
- Load + replay: `POST /api/perf/load {"folder": ".../gate1.adna-take"}` → `status.audioStatus ==
  "Resolved"`, `unresolved 0`; `POST /api/perf/play {"withAudio":true}` → poll `/api/composition` every
  second: layer 0 `activeClipColumn` becomes 1, 2, 3 at ≈ 2 s, 6 s, 10 s after start; `/api/bpm` → 128 after
  the tempo point; `status.position` increases; `status.skipped == 1` (the `audio play` point, WithAudio
  policy); `POST /api/perf/stop_play` → `playing:false`. Then `play {"withAudio":false}` → same
  sequence on the wall clock, `skipped == 0`.
- Overdub safety (R5): `POST /api/perf/record {"name":"gate2","overdubAssetId":"<id>"}` while
  `play {"withAudio":true}` runs; wait 20 s; `stop`; `gate2/take.json` has NO `activeClip` points from the
  replay (only what REST sent during it — send one `trigger_clip` to prove capture still works);
  `audio.segments[0].firstSample == 0` and `id == <id>` (5.2); `Audio/` still has exactly one asset.
- Continuous rows (only after S3-M): during `record`, `set_layer_opacity` 0.4 → 0.7 → 0.2 over 3 s →
  `layer/scalar:opacity` lane has one gesture, `grip "decaying"`, ≥ 3 breakpoints with parallel stamps;
  on replay `/api/composition` layer 0 opacity follows 0.4→0.7→0.2 smoothly; a `set_layer_opacity` sent
  DURING replay wins for `gripHoldMs` then the lane's next `set` resumes (snap, no glide until L3(b)).
- Crash-readability (optional, destructive, run LAST before teardown): arm `gate3` with audio; after
  30 s `kill -9 $(pgrep -f MacOS/Audio-DNA)`; relaunch; `perf/load gate3` → `audioStatus == "Incomplete"`
  with the repair reason; `POST /api/perf/repair` → `Resolved`; the WAV header counts ≥ 12 s of audio
  (D-A10 bound). This is the only step that kills the app; no window is open, so the law's "close the
  output window first" clause does not apply, and the screen check below still runs.

Teardown (SCREEN-SAFETY LAW): `osascript -e 'quit app "Audio-DNA"'` (graceful — `~MainComponent`
calls `recorderHost_.shutdown` first); wait; `pgrep -f "MacOS/Audio-DNA"` empty; `screencapture -x
/tmp/step3-eos.png` and READ the image (no black overlay, no dialog); state the screen state in the handoff.
Never open the Output window in this recipe; never `pkill` while a window could be open.

Secondary, deterministic beat path (optional): `--test-mode` + `/api/inject_features` with a scripted
`beatPhase`/`bpm` ramp DOES drive `RecorderClock` (the clock integrates whatever snapshot it is fed) while
the wall and sample clocks run for real — useful for a CI-style discrete-replay check, but it never
exercises the analysis thread (T4) and so proves nothing about the live beat grid.

---

## 5. WHAT ONLY BORIS CAN CHECK (and the questions to put to him)

Only-Boris checks:
1. **T2 on his real interface** — his words ("we will test it to make sure it stays in time"): the §4
   alignment numbers on the built-in mic are a rehearsal; the deliverable is the same run through his
   interface at his sample rate, and only he can plug it in. Also the R13 latent: a 44.1 kHz interface
   makes the analysis grid 8.8 % off — the take's `sample` clock is unaffected, the beat lane is not.
2. **The Bluetooth-headset crash re-test** (T33): reconnect the `soundcore P31i` as default device and
   relaunch the H1-fixed build three times; only he has the headset. Until then step 3's live gate runs
   on the built-in mic by rule.
3. **MIDI grip feel** (after S3-M): turn a mapped knob while a take replays that knob — knob wins, lane
   resumes after a quarter second (snap until L3(b), glide after). Needs his controller.
4. **Does the replay LOOK like the show** on his monitors — the visual half of "playback with audio"
   (D14) is owner-attended by the screen-safety law.
5. **The Record tab** (step 4) — whether five buttons + status line are the surface he wants before the
   UI rewrite, or REST + a menu item is enough until then.

Questions (plain English; default already built in brackets):
- Q1. **Where should recordings live and how should they be named?** [`~/Documents/Audio-DNA/Takes/
  <date-time>.adna-take`; a name can be given when you press Record. Audio in `…/Audio/`, v2 Q2/Q3 stand.]
- Q2. **When you press Stop on the audio while replaying a recording WITH its audio, the recording's own
  "play/pause/stop" presses are skipped** (the audio file already contains their effect, and firing "stop"
  would stop the very audio the replay runs on). Replaying WITHOUT audio fires them. OK? [Skip with audio;
  fire without.]
- Q3. **A manual Resync by hand — should it also re-align the oscillator shapes to the new downbeat, or
  keep them flowing?** (HANDOFF addendum 1, still unasked. Step 3 records and replays your Resync either
  way; this decides what the oscillators do.) [Keep flowing = today's build; say the word to flip it.]
- Q4. **Recording over a stored audio (the re-do): pressing Stop on the audio while you are recording over
  it is refused** (it would rewind the clock under the recording). Pause is allowed. OK, or should Stop
  end the recording instead? [Refuse, with a message.]
- Q5. **Crash safety: after a crash you lose at most ~12-18 s of audio and at most one knob movement in
  progress; everything else is on disk every 60 s.** Tighter (every 10 s) costs a little disk activity.
  [60 s.]
- Q6. **Should a recording arm automatically when you play a set (Record on Play), or only when you press
  Record?** [Only on Record — nothing arms itself.]
- Q7. (Carried, v2 §13) Deleting stored audio; store on an external SSD; the 2 GB free-space floor below
  one night's 2.8 GB. [Never delete; fixed path; 2 GB — all unchanged.]

---

## 6. RISKS (each with the file it lives in) and the strongest counterargument

- **R-1 Overdub feedback (spec R5) — `Origin` must reach EVERY handler.** `MainComponent.cpp`. Closed
  twice: the host refuses to record `Origin::Replay` (test 7) AND the handlers gate capture+undo on
  Human. Reviewer runs grep (iii)-(v) in 3.3.
- **R-2 Arm-after-mode-switch ordering.** `MainComponent.cpp` B5. `setSourceMode(File)` restarts the
  device → `tap.prepare()`; a prepare with changed channels AFTER `tap.start()` self-stops the tap (T18,
  `AudioTap.h:55-58`). The arm MUST follow the mode switch; test 11 pins the self-stop bookkeeping, the
  recipe's `audioFile` row pins the order live. Mic→file keeps 2 output channels either way (INFERRED from
  `CombinedCallback.h:154-155` reading OUTPUT channels; builder verifies with a log line at prepare).
- **R-3 Same-file merges at integration.** `MainComponent.cpp` is edited by S3-B then S3-M2; `ApiServer.cpp`
  by S3-C then S3-M2; `tests/CMakeLists.txt`/root `CMakeLists.txt` by H1 → S3-A → S3-M1. All serialized in
  §3.7; worktrees (T29) make the parallel parts safe; Harmony integrates serially (s93).
- **R-4 Message-thread stalls (spec R10).** A modal dialog pauses the tick; on resume `advanceTo` fires the
  due burst (exactly-once semantics) — correct, but visible. Play-with-audio timing is immune (clocked from
  the transport). Not new; disclosed.
- **R-5 The periodic snapshot copies the whole `Take` every 60 s.** `RecorderHost.cpp`. A 4-hour take is
  ~1-3 MB of JSON; a `std::map` copy + JSON serialise on the message thread costs tens of ms once a
  minute — acceptable; if a profile shows a hitch, move the serialise to a `TimeSliceThread` (the copy stays
  on the message thread). Measure, do not assume: log the save duration.
- **R-6 `kMinFreeBytes` 2 GB < one night** (v2 R6/Q2). `AudioTap.h:132`. Unchanged; Boris Q7.
- **R-7 `quantize` and the BPM multiplier have no capture site** (T15). Disclosed: quantize is checkpoint-
  only; the multiplier is ruling-4 work outside this step. Not silent — the CLAUDE.md note names both.
- **R-8 `DiscretePoint.v` centi-BPM unit.** `MainComponent.cpp` (capture + dispatch in one lane). A future
  reader that assumes `v` is BPM reads 12800; mitigated by the vocabulary comment and by `bpm` on the same
  point still carrying the clock's bpm. If Boris ever wants sub-centi precision, ADD a field then.
- **R-9 H1's staged AudioTap fix must be in the base.** `src/recording/AudioTap.cpp`. S3-A's tap-driving
  tests push variable block sizes only through `FakeAudioIODevice` at a fixed block; the heap-overflow read
  is fixed in the staged hunk — Wave 0 commits it before any lane branches.
- **R-10 Status mutex.** `RecorderHost.cpp`. Message thread + HTTP thread only; never the audio or GL
  thread. Reviewer greps `statusMutex_` uses → only `tick()` (publish) and `status()` (read).
- **R-11 Two recorders in the UI (spec R16).** `Binding::Action::ToggleRecording` still toggles the VIDEO
  recorder (`case Binding::Action::ToggleRecording`). Untouched here; step 4/the rewrite unify them.
- **R-12 L1's `audio` bridge assumption holds:** step 3 gives `audio` points `v = 0` and an action string —
  exactly what `Take.cpp`'s v1 bridge now emits (fixes-plan risk 3 discharged).

**Strongest counterargument to the whole plan, and why it loses:** "Wait for Lane 3 proper; wiring the
recorder before the grip engine ticks means the first thing Boris sees is a replay that snaps instead of
glides and refuses knob lanes." It loses on the owner's own ranking: ruling 24 ("the event logger is
crucial… events are more important") and ruling 28 (record the show FIRST, then rehearse against it) both
put the discrete log + audio store ahead of knob feel; every deferred piece here is additive (a
`std::function` filled in later, a snap that becomes a glide when L3(b) lands) and nothing recorded under
this plan has to be re-recorded when Lane 3 completes — which is exactly the property the v2 spec was
written to protect.

---

## 7. SUMMARY FOR HARMONY (15 lines)

1. Step 3 is entirely unbuilt: no recorder class is referenced by `MainComponent`, `manualWrite` exists nowhere, `ConnectionEngine::tick` is never called.
2. Verdict on Lane 3: NOT a prerequisite. Discrete lanes, the audio-store lifecycle, provisional/periodic save, the R14 flag, REST and discrete replay proceed now; only continuous lanes wait — for a half-day L3(e) slice (`ManualWriter`), not for Lane 3.
3. A step-3-written `manualWrite` stub is ruled out (R8 as re-decided in s168); the recorder's seam is an EMPTY `std::function` that refuses loudly and counts — a hook, not a definition.
4. New `RecorderHost` (`src/recording/`) owns clock/recorder/store/player + the whole lifecycle behind a `Dispatch` struct; `MainComponent` gets ~120 lines of wiring; headless-testable (14 cases listed).
5. Two unblocking additions the spec did not foresee: `PerformanceRecorder::{setCheckpoint0, current()}` (no accessor exists — periodic save was impossible) and a separate `PerfStateCapture` TU (`PerfState.cpp` is linked by model-free test targets).
6. Choke points: `Origin`+`deck` on the three trigger/deck handlers (defaulted params, nine callers unchanged), plus `applyTempoCommand`, `applyAudioTransport`, `applyLayerFlag`, `applyEffectBypass`, `applyClipPlaying`; undo push and capture gated on `Origin::Human`.
7. Tempo points store centi-BPM in `v` (documented unit); `audio` transport points are skipped during play-with-audio (they would stop the replay's own clock) and fired on wall-clock replay.
8. Arm order is load-bearing: mode switch → `beginAsset` → `tap.start` → recorder → checkpoint0 → provisional `take.json`; every save goes through `AudioStore::referencing` so the reference is never blank and the R14 flag is on disk from the first periodic save.
9. Overdub (re-do against stored audio): no tap, `firstSample 0`, clock = asset frame (5.2 formula); audio Stop is refused while overdubbing (the only seek writer in the tree is `AudioEngine::stop()`).
10. REST `/api/perf/{record,stop,load,play,stop_play,repair,status}` is pulled forward from row 5: it is the only production-mode trigger surface that does not touch L5's in-flight `RecordPanel` files.
11. Four lanes, disjoint files: S3-A recording side, S3-B `MainComponent` only, S3-C `ApiServer` only, S3-M (connection lane) `ManualWriter` + eleven site lines; S3-M2 and the CMake one-liners are the only serialized edits.
12. Wave 0: commit the STAGED H1 (AudioTap heap-read fix + BT test) and L5 (RecordPanel) work first — every lane must branch from that.
13. Live gate: production launch (never `--test-mode` — it starts no analysis), REST on 7070, a click WAV via `record {audioFile}`, disk assertions on store + take, load/replay/overdub rows, short-form T2 numbers (mean offset ≈ 20-30 ms, drift ≤ 1 ms), graceful `osascript` quit, `screencapture` read.
14. Only Boris: T2 on his interface, the BT-headset crash re-test, MIDI grip feel, "does the replay look like the show", the Record tab.
15. Boris questions: take location/naming; skip-audio-points-with-audio policy; Resync-vs-oscillators (addendum 1, still unasked); Stop refused while overdubbing; 60 s crash-loss window; arm-on-play or Record only; the carried v2 §13 three.

REPORT_FILE: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/7e364303-6335-4445-955e-49321e60ded4/scratchpad/s-rta-0923-step3-plan.md
STATUS: COMPLETE
