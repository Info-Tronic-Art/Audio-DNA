# s-rta-0926 -- L-R ROUTINES, SLICE 1: build plan (Architect, Fable)

Author: Architect (Fable), 2026-09-26. Read-only pass at HEAD `51a34a1` (main); no source edited, nothing built,
app not launched. Inputs: spec `.harmony/specs/s167-performance-log-and-routines.md` (0, 1, D2, D4, D5, D8, D9, 3, 4,
5, 6), `.harmony/binding-decisions.md` rulings 17/20/21/22/26/27 (+2, 5, 24, "hold, don't stop"), 
`.harmony/.reports/s-rta-0925/plan-roadmap.md` section 3 (the built preamble), and the sources cited per claim.
Labels: VERIFIED = read on disk this pass at the cited line; INFERRED = derived from cited code; ASSUMED = stated so
the Builder checks it. Line numbers in `MainComponent.cpp` drift -- every cite there carries its function name as
the anchor (binding-decisions.md:179-182). Rig facts carried: no Xcode (CLT build), no Bluetooth audio, live gates on
production port 7070 via `open -g`, render evidence pixel-decoded, NEVER lldb/gdb on any binary.

---

## QUESTION

What is the smallest slice 1 of L-R Routines that lets Boris take a piece of a recorded take, save it as a named
routine in the composition, fire it from its own bank, have it restore its recorded state by default and then play its
lanes starting on the next bar -- buildable and live-gatable in one session by 1-2 builder lanes, without painting
slices 2+ into a corner.

## APPROACH (verdict first)

**Slice 1 = "one routine, one bank, fired as a hand."** Five lines:
1. `Routine` = a composition-owned, beat-native lane set + an explicit PREAMBLE table (the state at the slice start,
   derived from the take's lanes, falling back to checkpoint 0, then to defaults) + settings {loop, restoreState,
   quantize}; saved inside the composition JSON under two new keys (`routines`, `routineBank`); old files load
   with no routines.
2. `sliceRoutine(take, fromBeat, toBeat)` cuts each lane, rebases x to the routine's beat 0, synthesizes begin/end
   breakpoints for straddling gestures, drops wall/sample stamps, refuses an unmetered range, drops transport-class
   lanes by name (counted).
3. `RoutineEngine` (message thread, 120 Hz) runs each fired routine as its OWN `Player` on a shared beat clock
   (a `RecorderClock` that ticks always), starts it on the next bar (per-routine quantize; global Quantize overrides),
   fires the preamble through the SAME `Player::firePreamble` -> `dispatch.fire`/`manualWrite` path the replay
   restore already uses, loops or holds at the end, releases every grip on stop.
4. Surfaces: REST `/api/routine/{save,fire,stop,set,remove,status}`, OSC `/audiodna/routine/{slot}`,
   `Binding::Action::TriggerRoutine` (keyboard + MIDI, learnable from 8 whole-word overlay targets "Routine 1..8").
5. Gates: ~14 new ctest cases (model, slice, compile, scheduler) + `.harmony/probe-routines.sh` (RED on today's
   build: `/api/routine/*` is 404) proving restore-then-grid-replay over REST with a pixel-decoded frame.

**Explicitly DEFERRED to slice 2+ (and why the deferral is safe):** the `routine` lane in takes (trigger capture +
`via`-tagged children, D6) -- nothing is written into takes by slice 1, so no take format is redefined later
(`Scope::Routine`/"routine" control are already reserved, `src/model/ControlPath.h:20,190`); the lane editor /
range-select UI (ruling 23, needs the UI rewrite); a per-slot binding/re-target table (D9 `bindings[]`) -- compile
reports unresolved/rebound today and the table is additive; import/export files (`audiodna-routine`, D12:699-702) --
the JSON shape below is the same object, so export is a serializer call later; deck switches / tempo / audio
transport INSIDE a routine -- dropped by name at slice time, additive to allow later; routine-vs-set-replay
`missingRoutines` compile logic; per-layer/per-clip macro scopes (ruling 10). None of these change a field's
meaning; every one adds a key, a control string or a lane (D12 "ADD, never REDEFINE", spec:720-727).

---

## 1. VERDICT -- scope in five lines (above) and the cut

What Boris sees after slice 1 (whole words, no jargon): record a take; type a name, "bars 1 to 4", press Save
Routine (REST or the optional pad strip); hit the routine's key/pad; on the next bar the layers and knobs it uses
snap back to how they were at the start of those bars, then his recorded moves replay on the beat grid; it either
plays once and holds the last look, or loops until he stops it; if he grabs a knob the routine is moving, he wins
until he lets go. Restore is the default; "Start from now" is a per-routine switch.

Deferred (see APPROACH). Slice 1 deliberately does NOT record anything routine-related into a take.

---

## 2. STATUS OF EACH PRIMITIVE (VERIFIED on disk this pass)

| Primitive | Status | Evidence |
|---|---|---|
| Preamble (checkpoint 0 -> `Program::preamble`/`preambleContinuous`, fired at Play) | BUILT, live-gated | `src/recording/Program.h:56-65,95-105`; `Program.cpp:229-413` (`buildPreamble`), `:422-426` (unconditional in `compile`); `Player.h:68-76`, `Player.cpp:84-103` (`firePreamble`: discrete via `sink.fire`, continuous touch("held")->set->release, returns refusals); `RecorderHost.cpp:713-719` (fired before `playing_ = true`); `MainComponent.cpp:1945-2003` (`dispatch.fire`: `immediate = origin == Preamble`, control names activeClip/activeDeck/tempo/audio/visible/solo/mute/bypass/autopilot/playing/quantize); `MainComponent.cpp:4162-4170` (`handleClipTrigger` immediate branch); gate `.harmony/probe-step3.sh:584-700` (92/1 at close, HANDOFF.md:2880) |
| Program / Player (immutable schedule + one player per running thing) | BUILT | `Program.h:99-109,131-132`; `Player.h:29-118` (start/advanceTo/seek/firePreamble/stop/swap); `Player.cpp:105-160` (advanceTo: discrete <= pos, gestures touch/set/release, TOUCH displacement) |
| `slice()` | NOT BUILT | `Program.h:21-24`: `Range` is a clip-to-[from,to) filter "with no preamble synthesis; full slice() semantics (D4) land with routines"; `Program.cpp:481,517` apply it; no `slice`/`Routine` symbol anywhere (`grep -rn "routine\|Routine" src/` -> only `ControlPath.h:20,190,200` and the `handFor` comment `MainComponent.cpp:169`) |
| Take lanes (beat-stamped, gestures as `AutomationCurve` + parallel stamps) | BUILT | `src/recording/Lane.h:127-138` (Gesture: curve x = beat offset, `stamps[i]` parallel), `:79-90` (DiscretePoint has `beat`), `Take.h:84-127`; compile picks `beat` for `DriveClock::Beat` (`Program.cpp:428-437`, `:511-512`) and reports a stampless gesture as `invalid` for every clock (`:503-504,527-529`) -- must NOT for a beat-native routine (section 3.4) |
| Composition serialization (hasProperty-guarded, additive) | BUILT | `src/model/Composition.h:220-335` (`toVar`), `:337-510` (`fromVar`; `decks.clear()` at `:446`, guarded reads throughout); no `routines` key (grep 0) |
| Binding actions | BUILT, additive | `src/binding/Binding.h:24-46` (enum ends `MasterSignal`), `:83-87` targets; `BindingManager.cpp:228` stores `action` as INT (append-only is back-compatible), `:235-240,278-283` per-target fields; overlays mint bindings from `BindableTarget` at `src/ui/BindingOverlay.cpp:185-198`, `MidiLearnOverlay.cpp:223-234,269-280`; targets list `MainComponent.cpp:6557-6590` (`buildBindableTargets`, global row "Tap Tempo".."Master Signal"); action switch `MainComponent.cpp:6704+` (`handleBindingAction`, GlobalStop at `:6857-6860`) |
| Bar-quantized clip trigger path | BUILT, GL-thread drained | `src/model/Layer.h:210-238` (`triggerClip` queues `pendingTriggerColumn`), `:279-315` (`processPendingTrigger`: Bar = `beatInBar == 0`, TwoBar/FourBar use `barCount % N`), drained on the GL thread by `src/model/Autopilot.cpp:43-58` at a beat crossing; global Quantize -> forced snap `MainComponent.cpp:22-31` (`quantizeModeToForcedSnap`: Off unless `trackerState == STATE_LOCKED`); cancelled on deck switch / global Stop via `src/core/DeckCommands.h:240-252` (`cancelPendingTriggers`), `MainComponent.cpp:4979-4984` (`handleDeckSwitch`) |
| Beat clock on the message thread | PARTIAL | `src/recording/RecorderClock.cpp:11-98` (monotonic beat from `snap.beatPhase` wraps, resync absorbed, frozen while `bpm == 0`) -- but it ticks ONLY while recording (`RecorderHost.cpp:425-429`, fix-plan F1). `FeatureSnapshot::totalBarCount` is the monotonic bar counter never rewound (`src/analysis/FeatureSnapshot.h:57-62`); manual BPM = `trackerState_ = STATE_LOCKED` + predicted phase regime (`src/analysis/BPMTracker.cpp:77-82,545-554`) |
| Grip chain (a routine is a hand at `Hand::Lane`) | BUILT | `src/connect/ManualWrite.h:24` (`None < Lane < HumanDecaying < HumanHeld`), `:61-71` (equal rank: latest writer wins; a lane cannot release a human); `MainComponent.cpp:164-169` (`handFor`: every non-Human origin -> `Hand::Lane`), `:3323-3346` (`manualWrite/Release/Touch` wrappers, message thread jassert) |
| REST marshal pattern + status posture | BUILT | `src/api/ApiServer.cpp:268-274` routes, `:1281-1374` handlers (503 when unwired, `callAsync`), `:1410` status synchronous from a mutex-guarded copy; `ApiServer.h:98-124` |
| Dispatch seams the engine can reuse | BUILT | `RecorderHost.h:49-82` (`Dispatch{fire, continuous{touch,set,release}, capturePerfState, notify, replayFinished}`), wired `MainComponent.cpp:1945-2032`; HostSink counting rule `RecorderHost.cpp:55-109` |
| Composition readback for a probe | PARTIAL | `ApiServer.cpp:324-391` (`handleComposition`): layer opacity/flags/activeClipColumn and per-clip id/name/playing/live block -- NO clip effect params (`:367-379`) -> section 5.1 adds them |

---

## 3. MODEL

### 3.1 `src/model/Routine.h` (NEW, header-only like `Composition.h`/`Deck.h`; includes `recording/Lane.h`,
`model/ControlPath.h`, `model/Clip.h` for `BeatSnapMode`)

```cpp
// Routine -- s167 D9: a composition-owned, beat-native lane set + the state it needs first (D4 step 2).
struct Routine
{
    std::string uuid;                 // juce::Uuid().toString(); identity; the bank references it
    std::string name;                 // whole words, user-typed; default "Routine N"
    double lengthBeats = 16.0;        // whole bars by default (D4 step 5); the Player's end (Program::length)
    Clip::BeatSnapMode quantize = Clip::BeatSnapMode::Bar;   // ruling 27; global Quantize overrides when on
    bool loop = false;                // ruling 22: once (default) / loop
    bool restoreState = true;         // ruling 26: restore (default) / "start from now"
    bool deckRelative = true;         // D2/D9: keys resolve on the ACTIVE deck at fire time; layer = recorded (ruling 17)

    // Where it came from (display + re-slice later; never identity).
    struct Source { std::string takeFolder; double fromBeat = 0.0, toBeat = 0.0; };
    Source source;

    // D4 step 2 PREAMBLE, explicit: "make the world look the way it did for the things I am about to touch".
    // Discrete entries carry v or action (the same vocabulary dispatch.fire accepts); continuous carry a
    // NORMALISED value (what Sink::set takes). ORDER IS LOAD-BEARING (R4): the slicer emits layer flags,
    // then activeClip, then clip play/pause; Player::firePreamble fires all discrete before all continuous.
    struct PreambleEntry
    {
        ControlPath key;              // deckRelative = true, positional layer/col/fx/param + names
        bool continuous = false;
        int v = 0;                    // discrete: state value (activeClip column, 0/1 flag)
        std::string action;           // discrete, action-valued controls ("resume" | "pause")
        float norm = 0.0f;            // continuous: [0,1]
        juce::var toVar() const; static PreambleEntry fromVar(const juce::var&);
    };
    std::vector<PreambleEntry> preamble;

    std::map<ControlPath, Lane> lanes;   // x / beat in ROUTINE beats (0 = the fire boundary); Gesture::stamps EMPTY

    juce::var toVar() const; static Routine fromVar(const juce::var& v);
    static const char* quantizeToString(Clip::BeatSnapMode);           // "off"|"beat"|"bar"|"2bar"|"4bar"
    static Clip::BeatSnapMode quantizeFromString(const juce::String&); // unknown -> Bar (never silently Off)
};

struct RoutineSlot { int slot = -1; std::string uuid; };
```

`Composition.h` gains (additive, next to `globalEffects`, `:36-37`): `std::vector<Routine> routines;
std::vector<RoutineSlot> routineBank;` and `static constexpr int kRoutineBankSize = 8;` plus helpers
`const Routine* routineInSlot(int slot) const`, `int firstFreeRoutineSlot() const`, `bool assignRoutineSlot(int slot,
const std::string& uuid)` (replaces an occupant: the old uuid is erased from `routines` unless another slot still
references it), `bool removeRoutineSlot(int slot)`. Bank size 8 matches the global macro convention (CLAUDE.md
"MacroBank ... 8 live") and the 8 pad targets in section 5.3.

### 3.2 Serialization inside the composition JSON (D9 "inside the thing it belongs to")

```jsonc
"routines": [ { "uuid": "...", "name": "Drop 1", "lengthBeats": 16, "quantize": "bar", "loop": false,
                "restoreState": true, "deckRelative": true,
                "source": { "takeFolder": "/Users/.../Takes/Friday.adna-take", "fromBeat": 128, "toBeat": 144 },
                "preamble": [ { "key": {...ControlPath...}, "v": 2 },
                              { "key": {...}, "action": "pause" },
                              { "key": {...}, "norm": 0.5 } ],
                "lanes": [ /* Lane::toVar(), x in routine beats, "stamps": [] */ ] } ],
"routineBank": [ { "slot": 0, "uuid": "..." } ]
```
Rules: enums are strings (D12); `toVar` writes both keys ALWAYS (an empty array is fine); `fromVar` clears both
vectors then reads them guarded by `hasProperty` exactly like every other block at `Composition.h:337-510`, so an
old file (no keys) loads with empty vectors -- test 1 pins this. `ControlPath::toVar/fromVar` and `Lane::toVar/
fromVar` are reused unchanged (`ControlPath.h:73-180`, `Lane.h:202-266`). A bank entry whose uuid matches no
routine is dropped at load and counted into a `LoadNote` string the app shows once (never silent). No `validate`
change: `compload::validateComposition` (`src/core/CompositionLoad.h:49-59`) checks decks only; routines carry no
GL resources, so `swapCompositionModel` (`MainComponent.cpp:2951`) needs no new fence -- but the engine must
`stopAll()` before the swap (section 4.5).

### 3.3 `sliceRoutine` (NEW `src/recording/RoutineSlice.{h,cpp}`; links EffectLibrary like `PerfStateCapture.cpp`)

```cpp
struct SliceRequest { double fromBeat = 0.0, toBeat = 0.0; std::string name; bool wholeBars = true; };
struct SliceResult
{
    std::optional<Routine> routine; std::string error;          // error non-empty => refused, routine empty
    std::vector<std::string> droppedLanes;                        // transport-class lanes, by ControlPath text
    int preambleFromLanes = 0, preambleFromCheckpoint = 0, preambleFromDefaults = 0, preambleUnknown = 0;
};
SliceResult sliceRoutine(const Take& take, const SliceRequest& req);
// Bar helpers (section 3.6): take beat of the start of 1-based bar n, given Meta::startBeatInBar.
double takeBeatOfBar(const Take& take, int bar);
```

Semantics, in order:
1. Refuse (`error`) when `toBeat <= fromBeat`; when `take.tempo.a` is empty; when the anchor bracketing
   `fromBeat` or any anchor with `beat` in [from, to] has `bpm <= 0` -- "this stretch has no beat; the tempo was
   unknown while it was recorded" (D4 step 4; `TempoMap::beatAt` freezes there, `TempoMap.cpp:66-73`).
2. Lane filter: skip `Lane::Kind::Opaque`; DROP (named in `droppedLanes`) every `Scope::Comp` lane whose control is
   `tempo`, `audio`, `activeDeck` or `quantize`, and every `Scope::Macro`/`Scope::Routine` lane. Rationale: a
   routine is hands on layers/clips/comp scalars, not the transport (WithAudio replay already skips `audio`,
   `RecorderHost.cpp:63-67`; a deck switch inside a deck-relative routine is self-contradictory; macro capture is
   LATER, `Program.cpp:103-104`). Additive later.
3. Every kept lane key gets `deckRelative = true` (positional deck index kept for display; D2:162-163, D9).
4. Discrete lane: keep points with `fromBeat <= beat < toBeat`; `beat -= fromBeat`; `s.t = 0; s.sample = 0`
   (seq kept: it is the (at, seq) tie-break, `Program.cpp:533-537`). PREAMBLE entry = last point with
   `beat < fromBeat` -> its `v`/`action` (`preambleFromLanes++`); else checkpoint 0 (`preambleFromCheckpoint++`):
   `activeClip` -> `LayerRuntime::activeClipColumn`; `visible/bypass/solo/mute/autopilot` -> the flags
   (`PerfState.h:54-73`); `playing` -> `ClipRuntime::playing` if present else "pause" (absent = default =
   paused, `PerfStateCapture.cpp:108-113`); clip-scope `bypass` (effect slot) -> not in PerfState ->
   `preambleUnknown++`, no entry. For every `activeClip` lane whose restored column is >= 0, ALSO emit a `playing`
   entry for (layer, that column) by the same rule -- the trigger auto-plays a never-triggered clip
   (`Layer.h:271-274`, plan-roadmap R4), and the entry MUST follow the `activeClip` entry (3.1 order).
5. Continuous lane: keep gestures with `x1 > fromBeat && x0 < toBeat`; drop breakpoints outside [from, to); a
   gesture straddling the start gets a synthesized FIRST breakpoint `{x: fromBeat, y: curve.eval(fromBeat)}`, one
   straddling the end gets a synthesized LAST `{x: toBeat, y: curve.eval(toBeat)}` (D4 step 3); then `x -= fromBeat`;
   `stamps` cleared; `grip` kept. PREAMBLE: if a gesture COVERS `fromBeat` -> no entry (its synthesized begin IS the
   state); else last gesture with `x1 <= fromBeat` -> `curve.pts.back().y` (`preambleFromLanes++`); else
   checkpoint 0: layer `scalar:opacity` -> `layerScalarDefs()[Opacity].toNorm(LayerRuntime::opacity)`
   (`src/connect/ScalarParams.h:112-125`); layer `param` -> `LayerRuntime::effectParams[PerfState::fxParamKey(fx,
   param)]`; clip `scalar:<key>` -> `ClipRuntime::scalars[key]` (already normalised, `PerfStateCapture.cpp:99-105`);
   clip `param` -> `ClipRuntime::effectParams[...]`; absent in checkpoint = it was at its DEFAULT at Record:
   params -> `EffectLibrary::getEffectDef(key.fxName)->params[key.param].defaultValue` (`key.fxName` is the
   display name the capture path writes, `MainComponent.cpp:5513-5514`), scalars -> `ScalarDef::defaultNorm`
   (`preambleFromDefaults++`); comp scalars (master opacity / signal) are NOT in `PerfState` (`PerfState.h:84-88`
   VERIFIED) -> `preambleUnknown++`, no entry; an effect def not found -> `preambleUnknown++`.
6. `lengthBeats = toBeat - fromBeat`, rounded UP to a whole bar (`ceil(len / 4) * 4`) when `wholeBars` (D4 step 5).
7. `uuid = juce::Uuid().toString()`; `source = {folder, from, to}`; preamble order per 3.1.

**Ruling on "restore the state it was recorded in" for a slice starting mid-take (the open question):** D4's
"checkpoint 0 only" (`spec:248-261`) says which checkpoints the TAKE stores (none periodic); it does not say a routine
restores the take's start. D4 step 2 (`spec:265-270`) defines the preamble as "the state at `beatFrom` of every
selected lane's control (last value before the cut, or checkpoint 0)". So "the state it was recorded in" (ruling 26,
`binding-decisions.md:427-430`) = the state at the slice's own START, DERIVED lane-by-lane, falling back to
checkpoint 0 for controls never touched before the cut, and to library/scalar defaults for controls absent from
checkpoint 0 (absence means "was default", `PerfStateCapture.cpp:46-57`). Only the controls the routine touches are
restored -- never a full-scene snapshot (D4 rejected list, `spec:276-280`). RULED; tests 2-3 pin it.

### 3.4 Compile for a routine (`src/recording/Program.{h,cpp}`, additive)

```cpp
// Program.h
std::shared_ptr<const Program> compileRoutine(const Routine& routine, const Composition& comp);
```
- Refactor `compile()`'s lane loop (`Program.cpp:459-541`) into a file-local `compileLanes(const std::map<ControlPath,
  Lane>&, const TempoMap&, const Composition&, DriveClock, std::optional<Range>, Program&, double& maxAt)`; `compile()`
  keeps its signature and behaviour (every existing test unchanged).
- Stamp rule fix (load-bearing): `const bool haveStamps = g.stamps.size() == g.curve.pts.size(); converted.x =
  haveStamps ? pickAt(...) : convertBeatX(...); if (!haveStamps && clock != DriveClock::Beat) ++stampMismatches;` --
  for `DriveClock::Beat` the curve's own x IS the datum (`convertBeatX` returns `beatX`, `Program.cpp:450`), so a
  stampless routine gesture is exact, not `invalid`. Test 6 pins `report.invalid.empty()`.
- `compileRoutine`: `program->clock = Beat; loop = routine.loop; length = routine.lengthBeats` (NOT `maxAt`);
  preamble from `routine.preamble` via `resolveKey` (`Program.cpp:96-151`; deck-relative keys resolve on
  `comp.activeDeckIndex`, `:27-32`): ExactMatch -> emit; PositionOnly/NameOnly -> emit + `addPreambleRebind`
  (`:166-172`, reason "preamble: routine ..."); Missing -> `report.preambleUnresolved.push_back({key, reason})`.
  Discrete -> `Fired{0, 0, key, target, DiscretePoint{v, action, origin = Preamble}}`; continuous -> `PreambleSet{key,
  target, norm}`; `report.preambleCount` = emitted. When `routine.restoreState == false` the preamble is still
  compiled (so the report is honest) but the engine never fires it (4.2).
- Lanes via `compileLanes(routine.lanes, TempoMap{}, comp, Beat, nullopt, ...)`.

### 3.5 Relative lanes, loop/once, restore vs start-from-now, next-bar start -- one line each
- Relative: x = beats since the fire boundary; `pos = clock.beat - startBeat` (4.2). No tempo map in a routine.
- Loop: at `pos >= lengthBeats` -> `startBeat += lengthBeats` (exact, no drift), `player.stop(sink)` (releases),
  `player.start(0)`, re-fire the preamble if `restoreState` (D9 "loop restarts re-fire the preamble", `spec:492-493`),
  continue in the same tick with the residual. Once: `player.stop(sink)` -> the routine leaves `running_`; the look
  HOLDS (manual values stay; a connected param glides back to its signal -- D8 hand-back, ruling 7).
- Restore (default): `firePreamble` at the start boundary; "Start from now" (`restoreState = false`): skipped.
- Next bar (default `quantize = Bar`, ruling 27): the routine is PENDING until the engine sees a `totalBarCount`
  edge; global Quantize (`Composition::quantizeMode`, `Composition.h:99-100`) overrides via the same
  `quantizeModeToForcedSnap` rule (`MainComponent.cpp:22-31`); tracker not locked / `bpm == 0` -> start NOW with a
  notice (mirrors that rule's honesty: a queued trigger that may never drain is worse than an immediate one).

### 3.6 Take meta addition (S, additive): `Meta::startBeatInBar`
"Bars 33 to 40" needs the take's bar grid; today a take knows beats since Record but not where its bars fall.
Add `double startBeatInBar = -1.0` (unknown) to `Meta` (`Take.h:47-56`; `Take.cpp:109,121` toVar/fromVar, absent ->
-1); `RecorderHost::ArmOptions` gains `double startBeatInBar = -1.0` (`RecorderHost.h:93-109`), stored at arm and
written into `meta` at the three save sites (arm's provisional save, `tick`'s periodic save `RecorderHost.cpp:476-487`,
`disarm`); `MainComponent::perfRecord` fills it from `snap.beatInBar + snap.beatPhase` when `snap.trackerState ==
BPMTracker::STATE_LOCKED` (`FeatureSnapshot.h:40-45`). `takeBeatOfBar(take, n) = (n - 1) * 4 + fmod(4 -
startBeatInBar, 4)` (bar 1 = the first FULL bar after Record; the partial bar before it is bar 0); unknown (-1) ->
treat as 0 and say so in the save notice ("bars counted from the start of the take"). Old takes: unchanged.

---

## 4. RUNTIME -- `src/recording/RoutineEngine.{h,cpp}` (NEW; message thread only; never includes MainComponent.h,
src/ui/, src/render/ -- the same rule as `RecorderHost.h:21-26`)

### 4.1 Public shape
```cpp
class RoutineEngine
{
public:
    struct Dispatch   // the SAME four lambdas RecorderHost::Dispatch carries; MainComponent copies them over
    {
        std::function<bool(const Fired&)> fire;
        std::function<bool(const ControlPath&, const std::string& grip)> touch;
        std::function<bool(const ControlPath&, float v)> set;
        std::function<void(const ControlPath&)> release;
        std::function<void(const std::string&)> notify;
    };
    Dispatch dispatch;

    // 120 Hz, right after recorderHost_.tick and BEFORE connectionEngine_.tick (ordering fact, section 4.6).
    // `comp` is read for the bank listing published in Status; `forcedSnap` = quantizeModeToForcedSnap(...)
    // computed by MainComponent every tick (Off when Quantize is off or the tracker is not locked).
    void tick(const FeatureSnapshot& snap, double wallNow, const Composition& comp, Clip::BeatSnapMode forcedSnap);

    // Compiles NOW against `comp` (targets resolved once, D2), then queues per quantize. Returns "" or a refusal.
    // beatAvailable == false (tracker not locked or bpm == 0) => starts immediately + notice (3.5).
    std::string fire(const Composition& comp, int slot, Clip::BeatSnapMode forcedSnap, bool beatAvailable);
    void stop(int slot);          // releases every grip (Player::stop), idle at once
    void stopAll();               // global Stop, composition load, shutdown
    Status status() const;        // mutex-guarded copy (HTTP thread reads it)
    static constexpr int kBankSize = Composition::kRoutineBankSize;
private:
    struct SlotSink;              // Sink -> dispatch; counts `skipped` (non-preamble refusals), like HostSink
    struct Running { int slot; std::string uuid, name; std::shared_ptr<const Program> program;
                     std::unique_ptr<Player> player; std::unique_ptr<SlotSink> sink;
                     bool pending = true; Clip::BeatSnapMode snap; bool restore, loop; double lengthBeats;
                     double startBeat = 0.0; uint32_t startedTotalBar = 0; int cycle = 0, restarts = 0;
                     bool restartRequested = false; int preambleFired = 0, preambleRefused = 0; };
    std::vector<Running> running_;   // at most one per slot
    RecorderClock clock_;             // the routine beat clock: ticks EVERY tick from app start
    bool haveBar_ = false; uint32_t lastTotalBar_ = 0; double lastWholeBeat_ = -1.0;
    void startNow(Running&, const FeatureSnapshot&);   // startBeat = clock.beat; start(0); preamble; counters
    void publishStatus(const Composition&);
    mutable std::mutex statusMutex_; Status published_;
};
```
`Status` (published every tick and on every transition): `double clockBeat; bool beatAvailable; int fires;
std::string lastError; struct Slot { int slot; std::string uuid, name; double lengthBeats; bool loop, restoreState;
std::string quantize; int lanes, preambleEntries; std::string state; /* "empty"|"idle"|"pending"|"running" */
double position; int cycle, restarts; uint32_t startedTotalBar; int unresolved, reboundByPosition, reboundByName,
preambleUnresolved, preambleCount, preambleFired, preambleRefused, skipped; } slots[kBankSize]; struct LastSaved {
int slot = -1; std::string uuid, name; int lanes, preambleEntries, preambleUnknown; std::vector<std::string>
dropped; } lastSaved;` (`lastSaved` is written by MainComponent's save funnel through a setter).

### 4.2 Scheduling (tick), exactly
1. `clock_.tick(snap, wallNow, 0)` -- the `sample` argument is unused here (`RecorderClock.h:37-45`); the TempoMap it
   grows is bounded (one anchor per 32 beats + bpm changes, `RecorderClock.cpp:87-89`; ~1000 anchors per 4-hour set,
   INFERRED) and never read. Reusing the proven class beats a second beat-integrator (test_take `[recorderclock]`
   already pins monotonicity across resync/unmetered).
2. Edges: `barEdge = haveBar_ && snap.totalBarCount != lastTotalBar_` (the rising-edge counter the codebase
   recommends, CLAUDE.md pitfall 32; bars are >= 1.2 s at 200 BPM, never missed at 120 Hz); `beatEdge =
   floor(clock.beat) != lastWholeBeat_`. Update both trackers after use.
3. For each `Running` r: if `r.pending`: due when `snap == Off` || (`Beat` && beatEdge) || (`Bar` && barEdge) ||
   (`TwoBar` && barEdge && `totalBarCount % 2 == 0`) || (`FourBar` && barEdge && `% 4 == 0`) -- the same arithmetic as
   `Layer::processPendingTrigger` (`Layer.h:295-311`) but on `totalBarCount` (never rewound). Due -> `startNow`.
   Else if running: `pos = clock.beat - r.startBeat`; if `r.restartRequested` and the edge rule for `r.snap` is met
   -> `player->stop(*sink)`, `restarts++`, `startNow` (retrigger = restart at the next boundary, D9 `spec:491-492`);
   else if `pos >= lengthBeats` -> loop: `player->stop; startBeat += lengthBeats; cycle++; player->start(0); if
   restore: preamble; player->advanceTo(pos - lengthBeats)`; once: `player->stop(*sink)` -> erase r (state idle;
   the look holds); else `player->advanceTo(pos, *sink)`.
4. `startNow`: `startBeat = clock.beat; startedTotalBar = snap.totalBarCount; player->start(0.0); if (restore)
   preambleRefused = player->firePreamble(*sink); preambleFired = report.preambleCount - preambleRefused; pending =
   false;` one notice: `"Routine <name> started on bar <n>"` (+ counts when non-zero, whole words: "N settings could
   not be restored (a layer or clip no longer exists)", "N controls you are holding were left alone").
5. `publishStatus(comp)`.
Unmetered (`bpm == 0`): `clock_` freezes (`RecorderClock.cpp:31-39`) so a running routine holds its position and a
pending one waits; `fire()` with `beatAvailable == false` starts immediately with the notice "no beat yet -- the
routine starts now". Disclosed.

### 4.3 Stacking, re-fire, stop, deck switch, missing targets
- Two routines on ONE control: both write at `Hand::Lane`; `manualWriteCore` "equal rank: latest writer wins"
  (`ManualWrite.h:61-67`) and `running_` is advanced in fire order, so the LATER-fired routine writes last each tick
  -- D9's "the later begin wins" (`spec:495-499`) with no new mechanism; test 10 pins the order. A human Held/Decaying
  grip refuses both (D8); each Player marks itself displaced for that gesture only (`Player.cpp:136-138,151-156`).
- Two routines on one layer's clip: last trigger wins (one active clip per layer) -- inherent.
- Re-fire while running: `restartRequested = true` (restart at the next boundary); while pending: no-op.
- `stop(slot)`: `player->stop(*sink)` (every grip released, R9) -> idle, published at once. `stopAll()`: the same
  for all. Called from: REST `stop`, `Binding::Action::GlobalStop` and the TopBar Stop lambda (`MainComponent.cpp:
  691-700`, `:6857-6860`) -- ruling 5 "Stop means stop" extended to running routines (feel check, section 8);
  `swapCompositionModel` before the swap (`MainComponent.cpp:2951`); `~MainComponent` before `recorderHost_.shutdown`.
- Deck switch mid-routine: targets were resolved at fire time to the deck that was active THEN
  (`ResolvedTarget.deck`), and every handler takes that deck explicitly (`dispatch.fire` passes `f.target.deck`,
  `MainComponent.cpp:1960,1984,1990,1996`; UI refresh gated to the active deck, `:4181-4185`) -- so the routine keeps
  playing on the deck it started on, exactly as a replay does. Not stopped (persistent layers exist, `Layer.h:56`).
  A routine fired on the NEW deck resolves there. Disclosed.
- A lane whose layer/clip/effect no longer exists: compiled OUT and counted (`report.unresolved`, D2 policy 3,
  `Program.cpp:464-474`); preamble entries -> `preambleUnresolved`; fire-time refusals (`dispatch.fire` false) ->
  `skipped` (SlotSink, same rule as `RecorderHost.cpp:70-85` minus the WithAudio audio-skip); a human grip at the
  preamble -> `preambleRefused`. All four per slot in `/api/routine/status` and in the start notice. No silent path.

### 4.4 What a routine does NOT do in slice 1 (disclosed): it is not recorded into a take (no `routine` lane, no
`via`); its writes enter the handlers as `Origin::Replay` (so capture gates `origin != Replay` at
`MainComponent.cpp:5488,5515,5546`, `RecorderHost.cpp:569` and undo gates `origin == Human` hold with NO new gate --
same construction as the preamble, `:1945-1953`). If a take is being recorded while a routine plays, the take gets
nothing from the routine (slice 2 adds the trigger + `via`). Video playheads / crossfade / pending clip triggers are
engine state and are not restored (D4 `spec:259-261`; plan-roadmap 3.1).

### 4.5 Threading contract (against the Sacred Rules)
| State | Owner | Readers | Mechanism |
|---|---|---|---|
| `RoutineEngine` (clock, players, running_) | message thread (120 Hz `tickFeaturePipeline`; REST via `callAsync`; bindings/OSC already on the message thread) | -- | confinement + the `RECORDER_HOST_ASSERT_MESSAGE_THREAD` idiom copied (`RecorderHost.cpp:44-47`) |
| `Program` per running routine | immutable after `compileRoutine` | its `Player` | `shared_ptr<const>` |
| `Status` | written by `publishStatus` on the message thread | HTTP thread (`/api/routine/status`), the 4 Hz panel | mutex-guarded copy -- the same posture as `RecorderHost::status()` (`RecorderHost.cpp:891-895`); not a hot path (audio/analysis/GL untouched) |
| `Composition::routines/routineBank` | message thread | message thread only | REST handlers marshal (`callAsync`), status reads the engine's copy, never the vector |
| Model writes | the SAME handlers a click uses (`handleClipTrigger`, `applyLayerFlag`, `applyClipPlaying`, `manualWrite`) | GL thread reads the model as it does after any click | no new mutex; no allocation on the GL/audio/analysis threads; the GL thread may render one frame mid-restore (same class as a column trigger) |
Ordering: engine tick AFTER `recorderHost_.tick` (`MainComponent.cpp:3392-3400`) and BEFORE `connectionEngine_.tick`
(`:3418`) so a routine gesture's grip is current when the engine decides what to publish (ConnectionEngine.cpp's
ORDERING FACT cited at `:3369-3376`); replay first, routines second -> a routine fired over a replay writes last
(D9 "the replay is one more player").

---

## 5. SURFACES (slice 1)

### 5.1 REST (`src/api/ApiServer.{h,cpp}`, same 503-when-unwired + `callAsync` shape as `/api/perf/*`,
`ApiServer.cpp:1281-1374`; status synchronous from the engine's copy like `:1410`)
| Route | Body | Effect / answer |
|---|---|---|
| `POST /api/routine/save` | `{"name":"Drop 1", "fromBeat":128, "toBeat":144}` or `{"fromBar":33, "toBar":36}` (1-based, inclusive: `toBeat = takeBeatOfBar(toBar + 1)`); optional `slot` (0-7; default first free; occupied -> replaced, said in the notice; none free -> refused "the routine bank is full"), `loop`, `restoreState`, `quantize` ("off"/"beat"/"bar"/"2bar"/"4bar"), `wholeBars`, `takeFolder` (default: the LOADED take via a new `const Take* RecorderHost::loadedTake() const`; none -> "No take is loaded. Use Load Take... first.") | marshalled -> `MainComponent::perfRoutineSave`; result in `status.lastSaved` / `status.lastError` + one notice "Saved routine <name> to pad N: M timelines, K restores" (+ ", dropped: tempo, audio" when any) |
| `POST /api/routine/fire` | `{"slot":0}` | `routineEngine_.fire(...)`; refusal text -> `lastError` |
| `POST /api/routine/stop` | `{"slot":0}` or `{"all":true}` | stop / stopAll |
| `POST /api/routine/set` | `{"slot":0, "loop":true, "restoreState":false, "quantize":"beat", "name":"..."}` (any subset) | edits the composition's routine; a RUNNING routine picks up `loop` at its next end, `quantize` at its next (re)start; `name` at once |
| `POST /api/routine/remove` | `{"slot":0}` | stops it if running, frees the pad, erases the routine if no other pad references it |
| `GET /api/routine/status` | -- | `{ok, clockBeat, beatAvailable, fires, lastError, lastSaved{slot,uuid,name,lanes,preambleEntries,preambleUnknown,dropped[]}, bank:[8 x {slot,uuid,name,lengthBeats,loop,restoreState,quantize,lanes,preambleEntries,state,position,cycle,restarts,startedTotalBar,unresolved,reboundByPosition,reboundByName,preambleUnresolved,preambleCount,preambleFired,preambleRefused,skipped}]}` |
Also (additive, needed by the probe and by Boris's own REST tooling): `GET /api/composition` clips gain
`"effects": [ { "name": ..., "bypassed": ..., "params": [ {"name": <ParamDef::name>, "value": <effParam(i)>} ] } ]`
(`ApiServer.cpp:367-379`; names via `renderer_.getEffectLibrary().getEffectDef(fx.effectName)` as `:496-500` does).

### 5.2 OSC (`src/osc/OscHandler.{h,cpp}`): `/audiodna/routine/{slot}` (value > 0 -> fire; 0 ignored), one branch in
the `startsWith` chain (`OscHandler.cpp:57-70` shape), callback `std::function<void(int slot)> onTriggerRoutine`,
wired in `MainComponent.cpp` next to `oscHandler_.onTriggerClip` (`:2083`). Header pattern list `OscHandler.h:14-26`
+1 (doc count 13 -> 14 goes to the docs step).

### 5.3 Binding (`src/binding/Binding.h`, `BindingManager.cpp`, overlays): append `TriggerRoutine` AFTER
`MasterSignal` (`Binding.h:45`; int-serialized, append-only); `int targetRoutineSlot = 0;` in the targets block
(`:83-87`) + one `setProperty`/`getProperty` pair (`BindingManager.cpp:235-240,278-283`); `BindableTarget` gains
`int routineSlot = 0` (`src/ui/BindingOverlay.h:33-43`) copied at the three minting sites (`BindingOverlay.cpp:185-198`,
`MidiLearnOverlay.cpp:223-234,269-280`); `buildBindableTargets` adds 8 targets in a new row under the global row,
labels "Routine 1".."Routine 8" (whole words; `MainComponent.cpp:6574-6590` idiom); `handleBindingAction`: `case
TriggerRoutine: if (value > 0) fire(slot) else if (triggerMode == Momentary) stop(slot)` (hold-to-run for free;
Toggle = press to fire/restart). The overlays' `switch (target.action)` blocks already `default:` (`BindingOverlay.cpp:
289-291`, `MidiLearnOverlay.cpp:336-338`); the engine's `-Wall -Wextra` (`cmake/CompilerWarnings.cmake:13`, no
`-Werror`) means an unhandled enumerator warns, never fails -- the Builder still adds the case.

### 5.4 UI ruling: slice 1 needs NO new panel to be Boris-usable for FIRING (a bound key or MIDI pad, learnable in
bind mode from "Routine N"), and REST for authoring. A minimal bank strip is LANE 3, optional, cut-able (section 7):
in the Browser's Record tab (`src/ui/RecordPanel.{h,cpp}`, `RecordPanel.cpp:255-297` row layout), a "Routines" row of
8 pads (`"1: Drop 1"`, `"1: Empty"`; tone Playing while running, Warning while pending, text "(next bar)"/"(bar 2)")
+ a "Save Routine" row: `From bar` / `To bar` editors + name + button, driven by a pure `src/ui/RoutineBankModel.h`
(`deriveRoutineBankView(const RoutineEngine::Status&)`, ctest-pinned like `RecordPanelModel.h:1-30`). Whole words
only (CLAUDE.md UI Text Rules). Why here and not the deck: ruling 5 (current UI is disposable; spend on mechanism),
ruling 20 (own bank, not grid cells), and the Record tab already has the perf* funnel plumbing (`MainComponent.cpp:
1587-1604`). Why not zero UI: ruling 2 (dead UI gets built; a bank that exists only over REST is a pad Boris cannot
press) -- but the engine + REST + binding are gate-complete without it, so it is the first thing to cut.

---

## 6. TESTS

### 6.1 ctest -- new target `test_routine` (`tests/CMakeLists.txt`, copy the `test_recorder_host` block minus
`AudioTap.cpp`/`AudioStore.cpp`, plus `${SRC_DIR}/recording/RoutineSlice.cpp`, `RoutineEngine.cpp`,
`${SRC_DIR}/binding/BindingManager.cpp`; links juce_core/events/graphics; `TEST_FIXTURES_DIR` as there). Idioms:
`FakeSink` (`tests/test_take.cpp:385-445`), `FakeDispatch` (`test_recorder_host.cpp:107-134`), `makeComposition()`
= `comp.initDefault()` (Deck 1 / Layer 1..3 / 12 columns, `src/model/Deck.h:28-40`), `makeSnap(bpm, phase)`
(`test_recorder_host.cpp:53-60`), `EffectLibrary::registerDefaults()` (`test_program_preamble.cpp:24-32`). Every
case is RED before the code exists (compile failure counts as RED; the Builder commits the RED tests first).

| # | Tag | Case | Pins |
|---|---|---|---|
| 1 | `[routine][model]` | Composition round-trip with one routine (2 lanes, 3 preamble entries, `loop`, `quantize "2bar"`, bank slot 3); a var WITHOUT the keys -> both vectors empty; unknown quantize string -> Bar; a bank uuid with no routine -> dropped, `LoadNote` non-empty | 3.1-3.2 |
| 2 | `[routine][slice]` | Take: activeClip points at beats 1/5/9; layer-0 opacity gestures [0,1]->0.6 and [3,7] 0.2->0.8; layer-1 opacity gesture [0,1]->0.6; slice [4,12): activeClip at x=1,5; layer-0 gesture begins at x=0 y=eval(4)=0.35, ends x=3 y=0.8, `stamps.empty()`; NO layer-0 preamble (covered); layer-1 preamble norm 0.6; activeClip preamble v=1 and a following `playing` entry; `lengthBeats == 8` | 3.3 steps 4-7 |
| 3 | `[routine][slice]` | preamble fallbacks: a layer opacity untouched before the cut -> checkpoint 0.4 (`preambleFromCheckpoint`); a clip fx param absent from checkpoint -> EffectLibrary default (`preambleFromDefaults`); a comp/opacity lane -> `preambleUnknown == 1`, no entry | 3.3 step 5 |
| 4 | `[routine][slice]` | comp/tempo + comp/audio + comp/activeDeck lanes -> `droppedLanes.size() == 3` by name; an anchor with bpm 0 inside the range -> `error` non-empty, no routine; `wholeBars` rounds 6 beats to 8 | 3.3 steps 1-2, 6 |
| 5 | `[routine][slice]` | `takeBeatOfBar`: `startBeatInBar 2.5` -> bar 1 at 1.5, bar 2 at 5.5; unknown (-1) -> bar 1 at 0 | 3.6 |
| 6 | `[routine][compile]` | routine keyed to layer 5 (missing) + layer 0: `unresolved 1`, `preambleUnresolved 1`, `resolvedCount 1`, `length == lengthBeats`, `clock == Beat`, `invalid.empty()` despite stampless gestures; `compile()` on a Wall take with a stampless gesture STILL reports `invalid` (regression guard) | 3.4 |
| 7 | `[routine][engine]` | fire(Bar) then ticks at bpm 120 with `totalBarCount 10` (phase ramping): nothing fired, `state pending`; a tick with `totalBarCount 11` -> preamble fired (discrete then continuous), `state running`, `startedTotalBar 11`; feed beatPhase wraps -> the point at x=1.0 fires exactly once when `clockBeat - startBeat >= 1.0`; gesture [2,3] -> touch/sets/release | 4.2 |
| 8 | `[routine][engine]` | loop: at pos >= 4 (length 4) the preamble fires again (count 2), `cycle 2`, the x=1 point fires again in cycle 2; `stop()` -> releases logged, `state idle`; once: after length -> `state idle`, releases logged, no further fire | 3.5, 4.3 |
| 9 | `[routine][engine]` | re-fire while running -> `restarts 1` at the next bar edge + preamble again; `forcedSnap Beat` overrides a routine's Bar (starts at the next beat edge); `beatAvailable false` -> `pending false` on the same call + a notice | 4.2-4.3 |
| 10 | `[routine][engine]` | two routines on one opacity key, fired A then B: each tick's `sets` order is [A, B] (the later-fired writes last) | 4.3 |
| 11 | `[routine][engine]` | quantize Off starts on the same tick; `Beat` on the next whole beat; `TwoBar` waits for an even `totalBarCount` | 4.2 |
| 12 | `[routine][binding]` | `Binding{action TriggerRoutine, targetRoutineSlot 5}` round-trips through `BindingManager::toVar/fromVar`; a v1 bindings JSON without the key loads slot 0 | 5.3 |
| 13 | `[take][meta]` (in `tests/test_take.cpp`, one SECTION on the existing round-trip at `:78`) | `Meta::startBeatInBar 2.5` round-trips; absent -> -1 | 3.6 |
| 14 | `[routine][bank]` (lane 3 only, `tests/test_routine_bank_model.cpp`, juce_core-only like `test_record_panel_model`) | pad text/tone per state: empty -> "1: Empty" disabled; idle -> "1: Drop 1"; pending -> Warning "(next bar)"; running -> Playing "(bar 2)" | 5.4 |
Expected: ctest 539 -> 539 + 13 (+1 with lane 3). The Builder REPORTS the number `ctest` prints; nobody inherits it.

### 6.2 LIVE PROBE `.harmony/probe-routines.sh` (+ fixture `.harmony/probe-routines.json`) -- Harmony runs it; RED
first on today's build (every `/api/routine/*` row 404s), then GREEN. Mechanics copied verbatim from
`probe-step3.sh:146-180,206-222,1093-1135` (helpers, `open -g` launch, graceful `osascript` quit, Quartz 0-Output-
window witness, `'MacOS/Audio-DN[A]'`) and `probe-mastersignal.sh:107-175` (pixel oracle: `assert_nonblank`,
`mean_abs_diff`, `MAD_THRESHOLD 0.5`, `.venv` PIL+numpy). Production mode, port 7070, NO `--test-mode`. No audio
device dependency: `POST /api/set_bpm {"bpm":120}` puts the tracker in manual LOCKED mode with a predicted phase
(`BPMTracker.cpp:77-82,545-554`; the same trick `probe-mastersignal.sh:436` uses), so a bar = exactly 2.0 s and
the take needs no audio (`"audio": false`, the 5.5 no-store branch).

Fixture: one deck "A", two layers; L1 col 0 = image clip `tests/fixtures/test_card.png` (mediaType 1, `mediaFile`,
the shape `probe-mastersignal.sh:405-428` uses) with effect "Brightness" params [0.5]; L1 col 1 = source
`solid_color` (`SourceRegistry.cpp:137`) with effect "Brightness" [0.5]; L2 col 0 = source `checkerboard`
(`:160`). Static content on both cells -> pixel comparisons are meaningful (no `u_time` drift; gravity_well-class
sources are black in a single frame, CLAUDE.md pitfall 22 -- avoided).

Rows (each `ok/no`; timing at 120 BPM manual: beat 0.5 s, bar 2 s):
1. Preconditions, launch, health, `totalBarCount` readable; `set_bpm 120`; `load_composition` fixture;
   `trigger_clip L0 C0`; sleep 1; frame `ref.png` -> non-blank.
2. Record: `/api/perf/record {"name":"probe-routines","audio":false}`; at +0.6 s `set_layer_opacity L0 0.5`; +1.6 s
   `set_layer_opacity L1 0.3`; +2.6 s `trigger_clip L0 C1`; +4.6 s `set_param {layer 0, column 1, effect
   "Brightness", param "amount", value 0.9}`; +6.6 s `set_layer_opacity L0 0.9`; +8.6 s `/api/perf/stop`. Rows:
   `recording false`, take.json has >= 4 lanes with `beat` stamps, `meta.startBeatInBar` present and in [0,4)
   (RED today: absent).
3. Save: `/api/perf/load {folder}`; `/api/routine/save {"name":"Probe Routine","fromBeat":0,"toBeat":16,"slot":0}`;
   poll `/api/routine/status` <= 1 s: `lastSaved.slot == 0`, `bank[0].name == "Probe Routine"`, `lanes >= 4`,
   `preambleEntries >= 5`, `lengthBeats == 16`, `dropped == []`, `state idle`. (RED today: 404.)
4. Perturb: `trigger_clip L0 C1`; `set_layer_opacity L0 0.1`; `set_layer_opacity L1 0.6`; `set_param C1 Brightness
   0.2`; sleep 1 (> `gripHoldMs` 250 ms so the Decaying grips expire -- else the lane-rank restore is CORRECTLY
   refused); comp reads the perturbed values; frame `pert.png` non-blank; `mad(ref, pert) > 0.5`.
5. Fire on the bar: read `/api/bpm` -> `B0 = totalBarCount`, `T0 = now`; `/api/routine/fire {"slot":0}`; poll status
   every 50 ms until `bank[0].state == "running"` (<= 2.5 s): `T1`; rows: `startedTotalBar >= B0 + 1` (a LATER bar,
   not immediately), `T1 - T0 <= 2.2 s`, `preambleUnresolved == 0`, `preambleRefused == 0`, `preambleFired >= 5`,
   `unresolved == 0`.
6. Restore (within 300 ms of running): comp `L0 activeClipColumn == 0`, `L0 opacity` within 0.05 of 1.0, `L1 opacity`
   ~1.0, `C1 Brightness amount` ~0.5 (via the new `effects` block); frame `rest.png` non-blank; `mad(pert, rest) >
   0.5`; `mad(ref, rest) <= 2.0` (the restored look IS the original look, pixel-decoded; the Builder tunes the bound
   from a real run and states it -- never md5).
7. Grid replay: sample comp every 100 ms for 9 s from T1; first sample where `L0 opacity ~0.5` at `T1 + 0.6 +/- 0.35`;
   `activeClipColumn == 1` at `T1 + 2.6 +/- 0.35`; `amount ~0.9` at `T1 + 4.6 +/- 0.35`; `opacity ~0.9` at `T1 + 6.6
   +/- 0.35`; ORDER as listed. At `T1 + 8.3`: `state idle`, comp still `opacity 0.9`, `activeClipColumn 1` (once =
   hold the last look).
8. Loop: `/api/routine/set {"slot":0,"loop":true}`; fire; wait running (`T2`); within `T2 + 8.0 .. 8.6` observe `L0
   opacity` return to ~1.0 (the preamble re-fired) then ~0.5 by `T2 + 8.9`; status `cycle >= 2`, `preambleFired`
   grew; `/api/routine/stop {"slot":0}` -> `state idle` within 200 ms; `set_layer_opacity L0 0.42` -> comp reads 0.42
   within 300 ms (no stale grip after stop).
9. Re-fire: fire; wait running; fire again; within 2.3 s `restarts == 1` and `L0 opacity` snapped back to ~1.0 at
   the restart.
10. Start from now: `set {"restoreState":false}`; perturb `L0 opacity 0.1`; fire; wait running; `preambleFired == 0`
    and `L0 opacity` stays 0.1 until the routine's own first move (~T + 0.6 s -> 0.5).
11. Teardown per `probe-step3.sh:1093-1135` (graceful quit; 0 Output windows; no full-screen capture).
Artifacts under `/tmp/audiodna-routines/`; PASS/FAIL count printed; exit 1 on any FAIL. Harmony LOOKS at `rest.png`
before accepting (rig rule).

---

## 7. BUILD PLAN

Order: wave 1 (parity A / step3-row B / docs C / polish D, `.harmony/s-rta-0926-work.md`) MERGES FIRST -- lane 1
below touches none of their files, but MainComponent.cpp is shared with nothing in wave 1 (VERIFIED: wave 1 fences
are `src/render/CompositorEngine.*` + `.harmony/probe-effects-parity.*` (A), `.harmony/probe-step3.sh` (B),
`CLAUDE.md`/`.harmony/APP-INVENTORY.md`/spec docs (C), `src/ui/CompDecksBrowser.*`/`src/ui/ClipInspector.*`/TopBar
shots (D)). Disk: `df -h /System/Volumes/Data` showed 380 GB free at plan time -- still run it before each lane
(8 GB/lane + 20 GB headroom; max 3 build lanes). After lane 1 merges (it edits the ROOT `CMakeLists.txt` source list
at `:261` region and `tests/CMakeLists.txt`): `cmake -S . -B build` before building.

**LANE 1 -- core + wiring + probe (one builder, worktree, opus tier; M-large, ~900 source lines + ~600 test lines
+ ~350 probe lines).** Steps, each a commit, RED tests first:
1. `tests/test_routine.cpp` cases 1-13 RED (compile failures allowed as RED); `tests/CMakeLists.txt` target.
2. `src/model/Routine.h` + `Composition.h` fields/serialization/helpers (3.1-3.2). Case 1 green.
3. `Take.{h,cpp}` `Meta::startBeatInBar`; `RecorderHost.{h,cpp}` `ArmOptions::startBeatInBar` -> meta at the three
   save sites + `loadedTake()` accessor; case 13 green.
4. `src/recording/RoutineSlice.{h,cpp}` (3.3, 3.6). Cases 2-5 green.
5. `Program.{h,cpp}`: `compileLanes` refactor, stamp rule, `compileRoutine` (3.4). Case 6 green; ALL existing
   `test_program_*`/`test_take`/`test_recorder_host` cases still green (the refactor must be behaviour-neutral).
6. `src/recording/RoutineEngine.{h,cpp}` (4.1-4.3). Cases 7-11 green.
7. `Binding.h`, `BindingManager.cpp` (5.3 model half). Case 12 green.
8. App wiring in `src/MainComponent.{h,cpp}`: member `RoutineEngine routineEngine_;` (declared AFTER `recorderHost_`,
   `MainComponent.h:561`, so it is destroyed first); `dispatch` = copies of the four recorder lambdas + notify
   (right after `:2032`); tick call after the `recorderHost_.tick` block (`:3392-3400`) with `forcedSnap =
   quantizeModeToForcedSnap(composition_.quantizeMode, snap)`; `perfRoutineSave/Fire/Stop/Set/Remove` funnel next to
   the perf* funnel (`:5142-5356`, returning "" or the refusal text; notices through `dispatch.notify`);
   `routineStatusVar()` (reads ONLY `routineEngine_.status()`); `handleBindingAction` case; `buildBindableTargets`
   8 targets + `BindableTarget::routineSlot` + the three overlay minting one-liners; TopBar `onStop` and
   `GlobalStop` -> `stopAll()`; `swapCompositionModel` -> `stopAll()` before the swap; `~MainComponent` ->
   `stopAll()` before `recorderHost_.shutdown`; `perfRecord` fills `startBeatInBar`.
9. `ApiServer.{h,cpp}`: six routes + callbacks (5.1) + composition `effects` readback; `OscHandler.{h,cpp}` (5.2).
10. Root `CMakeLists.txt`: add `src/recording/RoutineSlice.cpp`, `src/recording/RoutineEngine.cpp` next to `:261`.
11. `.harmony/probe-routines.sh` + `.harmony/probe-routines.json`: written, `bash -n`-checked, NOT run by the
    Builder ("the party that builds never verifies"); `git add -f` under `.harmony/` (force-tracked, use `;` not
    `&&`).
12. Builder report: real ctest count, the exact `mad` numbers it could NOT measure (Harmony measures), deviations.
Lane 1 MUST NOT touch: `src/recording/Player.{h,cpp}` (if a Player change seems needed, stop and report -- the
scheduler owns loop/restart), `Lane.h`, `ControlPath.h`, `PerfState*` (format-frozen), `src/render/*`,
`src/ui/*` except the 3 overlay one-liners and `BindingOverlay.h`, `CompDecksBrowser.*`, `ClipInspector.*`,
`RecordPanel.*`, `RecordPanelModel.h`, `.harmony/probe-step3.sh`, `CLAUDE.md`, `.harmony/APP-INVENTORY.md`, any spec.
It must not "fix" the open effects-parity blank-frame bug, the step3 `inputSource` row, or the Deck Load clip --
other lanes own them today.

**LANE 2 -- review (independent reviewer) + Harmony's gates:** rebuild `build/` (`cmake -S . -B build` first), ctest,
`probe-routines.sh` RED on the pre-merge binary then GREEN, `probe-step3.sh` unchanged count, `probe-mastersignal.sh`
(the tick ordering change sits next to its consumers), look at `rest.png`. Reviewer greps: `grep -rn "Origin::Routine"
src/` -> only enum/vocabulary (no capture path), `grep -n "routines\|routineBank" src/model/Composition.h` -> both
cleared in `fromVar`, every `firePreamble` call in `RoutineEngine.cpp` gated on `restore`, no `std::mutex` outside
`publishStatus/status`, `Player.cpp` diff empty.

**LANE 3 -- optional bank strip (one builder, worktree, sonnet tier; S-M; AFTER lane 1 merges, never parallel with it
because both touch `MainComponent.cpp`):** `src/ui/RoutineBankModel.h` (new), `src/ui/RecordPanel.{h,cpp}` (rows 5.4;
`std::function` hooks `onFireRoutine(int)`, `onStopRoutine(int)`, `onSaveRoutine(name, fromBar, toBar)`, `onRoutineStatus()`),
`tests/test_routine_bank_model.cpp` + CMake block, and ONLY the RecordPanel wiring block of `MainComponent.cpp`
(`:1587-1604`). Gate: window-only shot of the Record tab (Quartz window id, never full-screen) + case 14 + the
critic panel. Cut without loss if the session runs short: everything Boris-visible in section 6.2 holds without it.

Sequence: lane 1 -> reviewer + Harmony gates -> merge, remove the worktree the same turn -> lane 3 (optional) ->
docs (section 7 rows below) AFTER wave-1 lane C has merged its doc rows (it owns those files today): APP-INVENTORY
REST count +6 and a Routines row, `CLAUDE.md` one paragraph under "Audio Store (Ruling 28)"/Step 3 + the source-tree
entries for `Routine.h`, `RoutineSlice`, `RoutineEngine`, OSC pattern count, `binding-decisions.md` "BUILT <commit>"
notes under rulings 17/20/22/26/27.

---

## 8. RISKS (file each lives in) and what ONLY Boris can check

- **R1 (strongest counterargument to the recommendation):** "drive routines through the existing per-layer
  `pendingTriggerColumn` + `Program::Range` and skip a new engine." It loses: `Range` synthesizes no preamble
  (`Program.h:21-24`), pending triggers are per-layer/one-column and drained on the GL thread
  (`Autopilot.cpp:43-58`) where a Player must never run (spec section 4, `RecorderHost.cpp:44-47` asserts the message
  thread), and a routine spans many controls that must start/stop as one (D9). The engine is ~250 lines around a
  Player that already exists.
- **R2 Two beat clocks disagree** (`RoutineEngine.cpp` vs the GL-thread `Autopilot` beat crossing): a routine's clip
  hit at x = 0 fires at the bar edge seen at 120 Hz on the message thread; a human quantized trigger fires at the
  beat crossing seen on the GL thread -- up to ~16 ms apart. Same class as replay today; disclose, do not "fix".
- **R3 `totalBarCount` under a manual Resync** (`BPMTracker.cpp:534-541`): the counter is never rewound, `beatInBar`
  re-aligns; a pending routine may start one short bar after a Resync. Accepted (the same thing a queued clip does).
- **R4 Restore refused by a human grip** (`ManualWrite.cpp:160-176`): counted in `preambleRefused`, said in the
  notice; the probe's `sleep 1` after REST perturbations is load-bearing (`gripHoldMs` 250 ms, `Composition.h:92`).
- **R5 Preamble order** (`RoutineSlice.cpp`): `playing` before `activeClip` auto-plays a paused clip (`Layer.h:271-274`).
  Test 2 pins the order; test 5 in the preamble suite already pins the same rule for takes.
- **R6 The `compileLanes` refactor** (`Program.cpp:459-541`): a behaviour drift there breaks every replay. Guard: all
  existing program/host tests must stay green with NO edits, plus test 6's Wall-clock regression clause.
- **R7 `swapCompositionModel` while a routine runs** (`MainComponent.cpp:2951`): a Player holding coordinates into a
  replaced model would write into the new one. `stopAll()` before the swap; reviewer greps for it.
- **R8 Status copy cost** (`RoutineEngine::publishStatus`): 8 slots x ~20 fields copied under a mutex at 120 Hz on the
  message thread -- trivial, but keep the bank NAME strings out of the per-tick copy when nothing changed (copy names
  on transitions only) if the reviewer measures otherwise. Not a hot path by the Sacred Rules' definition.
- **R9 Static image clip loads on restore** (`handleClipTrigger` `:4191-4233` loads media synchronously): a routine
  restoring N layers' clips does N loads once at its start -- same as a column trigger. Acceptable.
- **R10 `Binding::Action` append** (`Binding.h:24-46`): appending after `MasterSignal` keeps saved int values valid;
  INSERTING anywhere else would silently retarget every saved binding. Reviewer checks the enum diff is a pure append.
- **R11 Unknown-bar takes** (`Meta::startBeatInBar == -1`, every take recorded before this lands): "bars" in the save
  form count from the take's start, said in the notice. Old takes are never rewritten.
- **R12 The probe's pixel bound** (`probe-routines.sh` row 6, `mad(ref, rest) <= 2.0`): ASSUMED; a static image +
  solid_color should reproduce near-exactly, but the Builder cannot run the app -- Harmony reads the real number on the
  first GREEN run and pins it with a margin, never widens it to pass.

**Only Boris can check (Tier 4, feel):** (1) the restore is a hard cut for opacity/knobs and a normal clip
transition for clips (`Layer.h:265` runs the layer's own transition) -- right, or should it fade? (2) Global Stop
also stops running routines -- does "Stop means stop" extend to routines, or should they ride through? (3) two
routines fired on one knob: the later one wins per tick; does that feel like "hands"? (4) loop restart re-snaps the
look every cycle -- wanted, or should loops skip the restore after cycle 1? (5) a bound MIDI pad: does the next-bar
wait read clearly enough on the pad (lane 3's "(next bar)")? (6) whole-bar rounding of a slice.

---

## 9. PRODUCT QUESTIONS NOT ANSWERED BY RULINGS 17/20/21/22/26/27 -- decided here, Boris may overrule

1. Running routines on global Stop: STOP them (ruling 5 "Stop means stop" reaches beyond cues; a knob still moving
   after Stop is the surprising outcome). Pending ones are cancelled either way.
2. Whole-bar rounding of a slice by default (D4 step 5 called it an owner call; nobody asked it): YES, `wholeBars`
   default true; REST can pass false. A routine that is "3.7 bars" long does not loop cleanly.
3. Deck: a routine fires on the ACTIVE deck's recorded layer (D2/D9 `deckRelative` default; ruling 17 answers the
   LAYER, not the deck). Kept as a per-routine flag; slice 1 exposes it only in JSON.
4. Re-fire while running = restart at the next boundary (spec D9, clip retrigger semantics, `Layer.h:250-260`);
   fire while pending = no-op.
5. Saving onto an occupied pad replaces it (said in the notice); the old routine is erased unless another pad still
   references it. A VJ re-slicing a pad expects replacement, not a refusal.
6. Slice 1 records nothing about routines into a take (section 4.4). Slice 2 owns the `routine` lane + `via`.
None of these blocks the build; each is one flag or one line to flip.

---

## TRADEOFFS CONSIDERED

- **Preamble as a filtered `PerfState` reusing `buildPreamble`** -- rejected: `LayerRuntime` carries all five flags +
  opacity + activeClip and `buildPreamble` emits all of them plus activeDeck/quantize (`Program.cpp:238-329`) -- a
  routine that only moves one knob would restore a whole layer (D4 rejected "full snapshot on trigger",
  `spec:277-279`). An explicit entry list restores exactly the touched controls and reuses `firePreamble` unchanged.
- **Preamble as x=0 points inside the lanes** (D9 sketch "preamble points at x = 0", `spec:478`) -- rejected: a
  zero-length gesture never calls `set()` (`Player.cpp:141-157`: release fires when `pos >= x1` before any set), so a
  continuous restore would need a fake length; `Program::preamble/preambleContinuous` already exist for this.
- **Routines as a clip `MediaType`** -- rejected by D9 (`spec:506-508`) and ruling 20.
- **A bespoke `BeatClock` instead of `RecorderClock`** -- rejected: the integration rule (wrap detection, resync
  absorption, unmetered freeze) is already implemented and tested; the unused TempoMap growth is bounded.
- **Record routine children with `Origin::Routine` now** -- rejected: without `via` (needs a parameter through 5
  capture sites + the continuous hook) slice 2 would double-fire takes recorded in slice 1; recording nothing is the
  only choice that is safely additive.
- **Parallel lanes (core || wiring) via a header contract** -- rejected for this session: both end in
  `MainComponent.cpp`, and three reviews PASSed live-broken work last session; one serial lane + one live gate has
  fewer moving parts. Lane 3 is the only parallel-capable piece and it is optional.
- **Zero UI vs a full bank panel** -- middle path (5.4): REST + bindable pads now; a thin, disposable strip only if
  time allows.

STATUS: COMPLETE -- slice 1 scoped (model + slice + compileRoutine + RoutineEngine + REST/OSC/binding + 13 ctest cases +
a RED-first live probe), deferrals shown additive, threading on the message thread with no new hot-path mutex, one
serial builder lane plus an optional UI strip lane, fences named against wave 1, six decided product calls listed for
Boris to overrule.
