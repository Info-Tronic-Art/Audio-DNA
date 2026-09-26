# s-rta-0926 -- L-R ROUTINES, SLICE 1: FINAL BUILD PLAN (Architect, Fable, after three blind critiques)

Author: Architect (Fable), 2026-09-26. Read-only pass at HEAD `51a34a1` (main; `git status` shows no change under
`src/`, `tests/`, `CMakeLists.txt`), no source edited, nothing built, app not launched, no lldb/gdb. This file
REPLACES `plan-routines-s1.md`: builders read ONLY this file. Every claim about the codebase is cited `path:line`
and labelled VERIFIED (read on disk this pass), INFERRED (derived from cited code) or ASSUMED (stated so the
Builder checks it). `MainComponent.cpp` cites drift; each carries its function name as the anchor. Rig facts:
no Xcode (CLT build), no Bluetooth audio, live gates on production port 7070 via `open -g`, render evidence
pixel-decoded, NEVER lldb/debugserver/gdb on any binary, never a full-screen capture.

Inputs ruled on: spec `.harmony/specs/s167-performance-log-and-routines.md` (0, 1, D4 `:248-280`, D5 `:282-313`,
D8 `:435-464`, D9 `:466-508`, 3 `:781-880`, 4 `:882-894`, 5 `:897-924`, 6 `:927-978`), `.harmony/binding-decisions.md`
rulings 5 `:243-245`, 7 `:254-256`, 17 `:335`, 20 `:357-361`, 21 `:363-365`, 22 `:369-387`, 26 `:427-430`,
27 `:432-434` (all VERIFIED this pass), `plan-roadmap.md` section 3 (the built preamble), and the three critiques
`critic-plan-routines-s1-{rulings,runtime,gate}.md`.

---

## 0. FINDINGS DISPOSITION (every finding from the three blind critiques)

| # | Critic | Sev | Finding (short) | Disposition | Where it lands in this plan |
|---|---|---|---|---|---|
| F1 | rulings | MUST | Fire-order stacking is not D9's "the later `begin` wins for the rest of that gesture"; test 10 cannot tell them apart | **ACCEPT -- implement.** VERIFIED: `ManualWrite.cpp:168-177` refuses only a STRICTLY higher active rank, so two `Hand::Lane` writers both land and the last one in tick order wins every tick; `Player.cpp:151-156` already gives the per-gesture "displaced" semantics the rule needs. The engine's own sink now tracks which routine's gesture on a control began most recently and refuses the other routine's `set()` for the rest of ITS gesture -- ~30 lines, no change to `ManualWrite`/`Player`. Replay-vs-routine stays tick order (disclosed deviation, R13). Test 10 rewritten to discriminate; test 15 added | 4.3, 4.1 (`laneOwner_`), 6.1 tests 10/15, R13, section 9 item 8, Boris feel check (3) |
| F2 | rulings | SHOULD | "Restore = the slice's own start" stated as RULED, not as an inference from D4 | **ACCEPT.** Re-labelled as the architect's reading of ruling 26 via D4 step 2 (`spec:266-269`); moved to section 9 (item 7) and to the open Boris questions (one sentence, no design change) | 3.3, 9.7, 10.1 |
| F3 | rulings | SHOULD | `direction` (ruling 22's resolution text) silently absent from the deferral list | **ACCEPT.** Named in DEFERRED with the reason: `Player` has no reverse clock -- a decreasing `pos` is a SEEK (`Player.h:42-52` VERIFIED); a `direction` key is additive later | APPROACH / DEFERRED |
| F4 | runtime | MUST | TwoBar/FourBar parity on `totalBarCount` is NOT "the same arithmetic as `Layer::processPendingTrigger`" (which uses `barCount`) | **ACCEPT -- fix to `barCount`.** VERIFIED: `Layer.h:305-309` uses `barCount % 2/4`; `Autopilot.cpp:56-57` passes `snapshot.barCount`; `FeatureSnapshot.h:56-62` two counters; `BPMTracker.cpp:486-487` both advance on a new bar, `:500-506` `barCount_` alone resets on entering a drop / leaving a breakdown, `:539` on a manual Resync. Ruling 27 says "consistent with how a quantized clip trigger already behaves" -> the EDGE stays on `totalBarCount` (never rewound, pitfall 32), the PARITY is `snap.barCount % N` at that edge, exactly the clip's counter. Test 11 fixture now gives the two counters different parities to pin which one is consulted | 4.2 step 3, 6.1 test 11, R3 |
| F5 | runtime | SHOULD | `erase` inside the `for each Running` loop | **ACCEPT.** Mark-then-compact: `r.done = true` in the loop, `std::erase_if(running_, ...)` after it (C++20); test 15 finishes two once-mode routines in one tick under ASan | 4.2 step 3, 6.1 test 15 |
| F6 | gate | SHOULD | The `compileLanes` refactor (the function every replay depends on) has no live gate until the end of a ~1900-line pass | **ACCEPT.** Lane 1 split: **1a** (steps 1-5: model, take meta, slicer, `compileLanes` refactor + `compileRoutine`) -> Harmony live gate (rebuild, ctest, `probe-step3.sh` unchanged 92/1, `probe-mastersignal.sh`) -> **1b** (engine, wiring, REST/OSC/Binding, probe) | 7 |
| F7 | gate | SHOULD | "One session" has no defined fallback landing | **ACCEPT.** Lane 1a IS the fallback: it is behaviour-neutral, adds no surface and no dead UI (tests only), and merges on its own; if 1b does not land, 1b becomes "slice 1b" next session with this file as its plan unchanged | 7 |
| F8 | gate | NICE | One-line confirmation of the ruling-26 reading to Boris | **ACCEPT** (same as F2) | 10.1 |
| F9 | gate | NICE | The live probe is well-defended against a vacuous pass | No action; kept as designed | 6.2 |
| -- | gate | (unchecked) | Whether wave-1 lane D touches `MainComponent.cpp` | VERIFIED `.harmony/s-rta-0926-work.md:6-13`: lane D = "Deck Loa clip + No-clip-selected overlap" in a worktree, no file fence written, so it cannot be proven from disk -- MOOT BY ORDER: "Build waits for wave-1 merge" (`:13`); lanes 1a/1b branch from the post-merge HEAD and re-anchor every `MainComponent.cpp` cite by function name | 7 |

Corrections found in my own re-read (not raised by a critic, fixed here): `Clip::BeatSnapMode` is at
`src/model/Clip.h:129-136` (not `:72-79` as the spec sketch says); `AutomationCurve.h` lives in `src/connect/`
(`src/connect/AutomationCurve.h:16-25,65-92`), reached through `recording/Lane.h`; `Meta` is at `Take.h:48-57`,
its (de)serializer at `Take.cpp:107-125`; `FakeSink` is `tests/test_take.cpp:47`; `makeSnap` is
`tests/test_recorder_host.cpp:55-62`; `RecorderHost` has `loadedTake_` (`RecorderHost.h:363`,
`std::optional<Take>`) but no accessor -- the accessor below is new.

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
   restore already uses, loops or holds at the end, releases every grip on stop, and arbitrates two routines on one
   control by which gesture BEGAN later (D9), inside its own sink.
4. Surfaces: REST `/api/routine/{save,fire,stop,set,remove,status}`, OSC `/audiodna/routine/{slot}`,
   `Binding::Action::TriggerRoutine` (keyboard + MIDI, learnable from 8 whole-word overlay targets "Routine 1..8").
5. Gates: 15 new ctest cases (model, slice, compile, scheduler, stacking) + `.harmony/probe-routines.sh` (RED on
   today's build: `/api/routine/*` is 404) proving restore-then-grid-replay over REST with a pixel-decoded frame.

**Explicitly DEFERRED to slice 2+ (and why each deferral is safe -- every one ADDS a key, a control string or a
lane; none changes a field's meaning, D12 "ADD, never REDEFINE", `spec:720-727`):**
- the `routine` lane in takes (trigger capture + `via`-tagged children, D6) -- nothing is written into takes by
  slice 1, so no take format is redefined later (`Scope::Routine`/"routine" are already reserved,
  `src/model/ControlPath.h:20,190,200` VERIFIED);
- the lane editor / range-select UI (ruling 23, needs the UI rewrite);
- a per-slot binding/re-target table (D9 `bindings[]`) -- compile reports unresolved/rebound today; the table is
  additive;
- import/export files (`audiodna-routine`, D12) -- the JSON shape below is the same object; export is a serializer
  call later;
- deck switches / tempo / audio transport INSIDE a routine -- dropped by name at slice time, additive to allow later;
- routine-vs-set-replay `missingRoutines` compile logic;
- per-layer/per-clip macro scopes (ruling 10);
- **`direction` (reverse playback; ruling 22's resolution names "loop / once / direction / quantize")** -- `Player`
  has no reverse clock: a decreasing `pos` is defined as a SEEK (`Player.h:42-52` VERIFIED), so reverse needs a
  Player change this slice must not make (fence, section 7). A `direction` key is additive to the Routine JSON;
- **routine-vs-set-replay gesture arbitration** -- the replay's sink lives in `RecorderHost` (`RecorderHost.cpp:55-109`),
  the routines' in `RoutineEngine`; slice 1 arbitrates ROUTINE-vs-ROUTINE by gesture begin (D9) and leaves
  replay-vs-routine to tick order (replay ticks first, so a routine fired over a replay writes last each tick) --
  disclosed as R13; a shared owner table is the slice-2 fix.

---

## 1. VERDICT -- what Boris sees after slice 1 (whole words)

Record a take; type a name, "bars 1 to 4", press Save Routine (REST or the optional pad strip); hit the routine's
key/pad; on the next bar the layers and knobs it uses snap back to how they were at the start of those bars, then his
recorded moves replay on the beat grid; it either plays once and holds the last look, or loops until he stops it; if
he grabs a knob the routine is moving, he wins until he lets go; if two routines reach for one knob, the one that
grabbed it later keeps it until its own move is over. Restore is the default; "Start from now" is a per-routine switch.

Slice 1 deliberately does NOT record anything routine-related into a take.

---

## 2. STATUS OF EACH PRIMITIVE (VERIFIED on disk this pass)

| Primitive | Status | Evidence |
|---|---|---|
| Preamble (checkpoint 0 -> `Program::preamble`/`preambleContinuous`, fired at Play) | BUILT, live-gated | `src/recording/Program.h:56-65,95-108`; `Program.cpp:229-413` (`buildPreamble`), `:422-426` (unconditional in `compile`); `Player.h:68-76`, `Player.cpp:84-103` (`firePreamble`: all discrete via `sink.fire`, then continuous touch("held")->set->release, release only when both accepted; returns refusals); `RecorderHost.cpp:713-719` (fired before `playing_ = true`); `MainComponent.cpp:1945-2003` (`recorderHost_.dispatch.fire`: `immediate = origin == Preamble`; controls activeClip/activeDeck/tempo/audio/visible/solo/mute/bypass/autopilot/playing/quantize; unknown control -> `false`); `:4160-4168` (`handleClipTrigger` immediate branch); gate `.harmony/probe-step3.sh:584-700` |
| Program / Player (immutable schedule + one player per running thing) | BUILT | `Program.h:99-109,131-132`; `Player.h:29-118`; `Player.cpp:12-34` (`start` re-seats everything), `:105-160` (`advanceTo`: discrete `<= pos`, gestures touch/set/release, TOUCH displacement per gesture at `:136-138,151-156`), `:162-173` (`stop` releases every held gesture) |
| `slice()` | NOT BUILT | `Program.h:21-24`: `Range` is a clip-to-[from,to) filter "with no preamble synthesis; full slice() semantics (D4) land with routines"; `Program.cpp:481,517` apply it; no `slice`/`Routine` symbol in `src/` except `ControlPath.h:20,190,200` and the `handFor` comment `MainComponent.cpp:166-170` |
| Take lanes (beat-stamped, gestures as `AutomationCurve` + parallel stamps) | BUILT | `src/recording/Lane.h:127-139` (Gesture: curve x = beat offset, `stamps[i]` parallel), `:80-93` (`DiscretePoint` has `beat`), `:183` (`Kind {Continuous, Discrete, Opaque}`), `Take.h:84-90`; compile picks `beat` for `DriveClock::Beat` (`Program.cpp:428-437`, `:511-512`) and reports a stampless gesture as `invalid` for EVERY clock (`:503-504,527-529`) -- must not for the Beat clock (3.4) |
| Composition serialization (hasProperty-guarded, additive) | BUILT | `src/model/Composition.h:220-335` (`toVar`), `:337-510` (`fromVar`; `decks.clear()` at `:446`); no `routines` key (grep 0) |
| Binding actions | BUILT, additive | `src/binding/Binding.h:24-46` (enum ends `MasterSignal` at `:45`), `:83-87` targets; `BindingManager.cpp:228` stores `action` as INT (append-only is back-compatible), `:235-240,278-283` per-target fields; overlays mint bindings from `BindableTarget` at `src/ui/BindingOverlay.cpp:185-198`, `MidiLearnOverlay.cpp:223-234,269-280`; targets list `MainComponent.cpp:6557-6590` (`buildBindableTargets`, global row "Tap Tempo".."Master Signal"); action switch `MainComponent.cpp:6704` (`handleBindingAction`; `GlobalStop` at `:6857-6860`) |
| Bar-quantized clip trigger path | BUILT, GL-thread drained | `src/model/Layer.h:210-238` (`triggerClip` queues `pendingTriggerColumn`), `:279-315` (`processPendingTrigger`: Bar = `beatInBar == 0`, TwoBar/FourBar = `barCount % N` at `:305-309`), drained on the GL thread by `src/model/Autopilot.cpp:43-60` at a beat crossing with `snapshot.barCount` (`:56-57`); global Quantize -> forced snap `MainComponent.cpp:24-31` (`quantizeModeToForcedSnap`: Off unless `trackerState == STATE_LOCKED`); cancelled on deck switch / global Stop via `src/core/DeckCommands.h:240-252` (`cancelPendingTriggers`), `MainComponent.cpp:4980-4984` (`handleDeckSwitch`) |
| Beat / bar counters | BUILT | `FeatureSnapshot.h:56-62`: `barCount` (uint16, bars since last phrase reset) and `totalBarCount` (uint32, monotonic); `BPMTracker.cpp:486-487` both advance on the downbeat rising edge, `:500-506` `barCount_ = 0` on entering a drop / leaving a breakdown (suppressed in the predicted-beat regime), `:539-541` `barCount_ = 0` on a manual Resync, `totalBarCount_` never rewound |
| Beat clock on the message thread | PARTIAL | `src/recording/RecorderClock.cpp:11-98` (monotonic beat from `snap.beatPhase` wraps, resync absorbed, frozen while `bpm == 0` at `:31-39`; periodic anchor every 32 beats `:85-86`, `RecorderClock.h:70-74`) -- ticks ONLY while recording (`RecorderHost.cpp:423-426`); nothing in `tick()` needs an armed take (`RecorderClock.cpp:11-25` self-initialises on the first call). Manual BPM = `trackerState_ = STATE_LOCKED` + predicted phase regime (`BPMTracker.cpp:76-82,545-554`) |
| Grip chain (a routine is a hand at `Hand::Lane`) | BUILT | `src/connect/ManualWrite.h:24` (`None < Lane < HumanDecaying < HumanHeld`); `ManualWrite.cpp:160-177` (`manualTouchCore`: refused only if a STRICTLY higher rank actively holds; equal rank refreshes), `:180-189` (`manualWriteCore`), `:191-204` (`manualReleaseCore`: a lane cannot release a human); `MainComponent.cpp:166-170` (`handFor`: every non-Human origin -> `Hand::Lane`), `:3323-3346` (`manualWrite/Release/Touch`, message-thread jassert) |
| REST marshal pattern + status posture | BUILT | `src/api/ApiServer.cpp:268-274` perf routes, `:1281-1374` handlers (503 when unwired, `callAsync`), `:1410` status synchronous from a mutex-guarded copy; `ApiServer.h:98-124` callbacks |
| Dispatch seams the engine reuses | BUILT | `RecorderHost.h:50-83` (`Dispatch{fire, continuous{touch,set,release}, capturePerfState, notify, replayFinished}`), wired `MainComponent.cpp:1945-2032`; HostSink counting rule `RecorderHost.cpp:55-109` |
| Composition readback for a probe | PARTIAL | `ApiServer.cpp:324-391` (`handleComposition`): layer opacity/flags/activeClipColumn and per-clip id/name/playing/live block -- NO clip effect params (`:365-379`) -> 5.1 adds them; `/api/bpm` publishes `totalBarCount` (`:658`) |

---

## 3. MODEL

### 3.1 `src/model/Routine.h` (NEW, header-only like `Composition.h`/`Deck.h`; includes `recording/Lane.h`,
`model/ControlPath.h`, `model/Clip.h` for `BeatSnapMode` (`Clip.h:129-136`))

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
    // NORMALISED value (what Sink::set takes). ORDER IS LOAD-BEARING (R5): the slicer emits layer flags,
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

`Composition.h` gains (additive, next to `globalEffects`, `Composition.h:36-37`): `std::vector<Routine> routines;
std::vector<RoutineSlot> routineBank;` and `static constexpr int kRoutineBankSize = 8;` plus helpers
`const Routine* routineInSlot(int slot) const`, `Routine* routineInSlot(int slot)`, `int firstFreeRoutineSlot() const`,
`bool assignRoutineSlot(int slot, const std::string& uuid)` (replaces an occupant: the old uuid is erased from
`routines` unless another slot still references it), `bool removeRoutineSlot(int slot)`. Bank size 8 matches the
global macro convention (`src/routing/MacroBank.h:18` `kNumMacros = 8` VERIFIED) and the 8 pad targets in 5.3.

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
old file (no keys) loads with empty vectors -- test 1 pins this. `ControlPath::toVar/fromVar` (`ControlPath.h:73-180`;
`deckRelative` serialises as `"rel"`, `:82,144`) and `Lane::toVar/fromVar` (`Lane.h:202-266`) are reused unchanged. A
bank entry whose uuid matches no routine is dropped at load and counted into a `LoadNote` string the app shows once
(never silent). No `validate` change: `compload::validateComposition` (`src/core/CompositionLoad.h:49-58`) checks
decks only; routines carry no GL resources, so `swapCompositionModel` (`MainComponent.cpp:2951`) needs no new fence --
but the engine must `stopAll()` before the swap (4.3).

### 3.3 `sliceRoutine` (NEW `src/recording/RoutineSlice.{h,cpp}`; links EffectLibrary like `PerfStateCapture.cpp`)

```cpp
struct SliceRequest { double fromBeat = 0.0, toBeat = 0.0; std::string name; bool wholeBars = true; };
struct SliceResult
{
    std::optional<Routine> routine; std::string error;          // error non-empty => refused, routine empty
    std::vector<std::string> droppedLanes;                        // transport-class lanes, by ControlPath text
    int preambleFromLanes = 0, preambleFromCheckpoint = 0, preambleFromDefaults = 0, preambleUnknown = 0;
};
SliceResult sliceRoutine(const Take& take, const SliceRequest& req, const EffectLibrary& library);
// Bar helper (3.6): take beat of the start of 1-based bar n, given Meta::startBeatInBar.
double takeBeatOfBar(const Take& take, int bar);
```

Semantics, in order:
1. Refuse (`error`) when `toBeat <= fromBeat`; when `take.tempo.a` is empty; when the anchor bracketing
   `fromBeat` or any anchor with `beat` in [from, to] has `bpm <= 0` -- "this stretch has no beat; the tempo was
   unknown while it was recorded" (D4 step 4; `TempoMap::beatAt` freezes there, `TempoMap.cpp:69`).
2. Lane filter: skip `Lane::Kind::Opaque`; DROP (named in `droppedLanes`) every `Scope::Comp` lane whose control is
   `tempo`, `audio`, `activeDeck` or `quantize`, and every `Scope::Macro`/`Scope::Routine` lane. Rationale: a
   routine is hands on layers/clips/comp scalars, not the transport (WithAudio replay already skips `audio`,
   `RecorderHost.cpp:63-67`; a deck switch inside a deck-relative routine is self-contradictory; macro capture is
   LATER, `Program.cpp:103-104`). Additive later.
3. Every kept lane key gets `deckRelative = true` (positional deck index kept for display; D2, D9).
4. Discrete lane: keep points with `fromBeat <= beat < toBeat`; `beat -= fromBeat`; `s.t = 0; s.sample = 0`
   (seq kept: it is the (at, seq) tie-break, `Program.cpp:533-537`). PREAMBLE entry = last point with
   `beat < fromBeat` -> its `v`/`action` (`preambleFromLanes++`); else checkpoint 0 (`preambleFromCheckpoint++`),
   looked up POSITIONALLY by the lane key's recorded deck/layer/col (`cp0.decks[key.deck].layers[key.layer]`,
   `PerfState.h:54-73,84-88`; checkpoint 0 was captured against the same composition the lane was recorded on):
   `activeClip` -> `LayerRuntime::activeClipColumn`; `visible/bypass/solo/mute/autopilot` -> the flags;
   `playing` -> `ClipRuntime::playing` if present else "pause" (absent = default = paused, `PerfStateCapture.cpp:104-110`);
   clip-scope `bypass` (effect slot) -> not in PerfState -> `preambleUnknown++`, no entry. For every `activeClip` lane
   whose restored column is >= 0, ALSO emit a `playing` entry for (layer, that column) by the same rule -- the trigger
   auto-plays a never-triggered clip (`Layer.h:271-274`), and the entry MUST follow the `activeClip` entry (3.1 order).
5. Continuous lane: keep gestures with `x1 > fromBeat && x0 < toBeat`; drop breakpoints outside [from, to); a
   gesture straddling the start gets a synthesized FIRST breakpoint `{x: fromBeat, y: curve.eval(fromBeat)}`, one
   straddling the end gets a synthesized LAST `{x: toBeat, y: curve.eval(toBeat)}` (D4 step 3; `AutomationCurve::eval`
   clamps outside [xMin, xMax], `src/connect/AutomationCurve.h:90-92`); then `x -= fromBeat`; `stamps` cleared;
   `grip` kept. PREAMBLE: if a gesture COVERS `fromBeat` -> no entry (its synthesized begin IS the state); else last
   gesture with `x1 <= fromBeat` -> `curve.pts.back().y` (`preambleFromLanes++`); else checkpoint 0: layer
   `scalar:opacity` -> `layerScalarDefs()[Opacity].toNorm(LayerRuntime::opacity)` (`src/connect/ScalarParams.h:27,96-98`);
   layer `param` -> `LayerRuntime::effectParams[PerfState::fxParamKey(fx, param)]` (`PerfState.h:31`); clip
   `scalar:<key>` -> `ClipRuntime::scalars[key]` (already normalised, `PerfStateCapture.cpp:97-101`); clip `param` ->
   `ClipRuntime::effectParams[...]`; absent in checkpoint = it was at its DEFAULT at Record: params ->
   `library.getEffectDef(key.fxName)->params[key.param].defaultValue` (`src/effects/EffectLibrary.h:16-20,48`;
   `key.fxName` is the display name the capture path writes, `MainComponent.cpp:5513-5515`), scalars ->
   `ScalarDef::defaultNorm` (`preambleFromDefaults++`); comp scalars (master opacity / signal) are NOT in `PerfState`
   (`PerfState.h:84-88` VERIFIED) -> `preambleUnknown++`, no entry; an effect def not found -> `preambleUnknown++`.
6. `lengthBeats = toBeat - fromBeat`, rounded UP to a whole bar (`ceil(len / 4) * 4`) when `wholeBars` (D4 step 5).
7. `uuid = juce::Uuid().toString()`; `source = {folder, from, to}`; preamble order per 3.1.

**Reading of "restore the state it was recorded in" for a slice starting mid-take (ARCHITECT'S INFERENCE, section 9
item 7, Boris question 10.1):** ruling 26 (`binding-decisions.md:427-430`) says "back the way they were at record time
... per the recommendation he accepted"; the recommendation is D4 step 2 (`spec:266-269`): "Preamble = the state at
`beatFrom` of every selected lane's control (last value before the cut, or checkpoint 0)". So this plan reads "at
record time" as the state at the slice's own START, derived lane-by-lane, falling back to checkpoint 0 for controls
never touched before the cut, and to library/scalar defaults for controls absent from checkpoint 0 (absence means
"was default", `PerfStateCapture.cpp:104-110`). Only the controls the routine touches are restored -- never a
full-scene snapshot (D4 rejected list, `spec:276-279`). The other reading (restore the take's checkpoint 0 in full)
is one flag away: `sliceRoutine` would copy `take.checkpoint0` through `buildPreamble` instead -- not built.
Tests 2-3 pin the reading built here.

### 3.4 Compile for a routine (`src/recording/Program.{h,cpp}`, additive)

```cpp
// Program.h
std::shared_ptr<const Program> compileRoutine(const Routine& routine, const Composition& comp);
```
- Refactor `compile()`'s lane loop (`Program.cpp:459-531`) into a file-local `compileLanes(const std::map<ControlPath,
  Lane>&, const TempoMap&, const Composition&, DriveClock, std::optional<Range>, Program&, double& maxAt)`; `compile()`
  keeps its signature and behaviour byte-for-byte (every existing test unchanged; the `pickAt`/`convertBeatX` lambdas
  `:428-455` move with the loop).
- Stamp rule fix (reporting only -- gate critic VERIFIED that for `DriveClock::Beat` the fallback path is already
  bit-identical: `convertBeatX` returns `beatX`, `Program.cpp:450`): `const bool haveStamps = g.stamps.size() ==
  g.curve.pts.size(); converted.x = haveStamps ? pickAt(...) : convertBeatX(...); if (!haveStamps && clock !=
  DriveClock::Beat) ++stampMismatches;` -- a stampless routine gesture is exact, not `invalid`. Test 6 pins
  `report.invalid.empty()` for Beat and the Wall regression clause.
- `compileRoutine`: `program->clock = Beat; loop = routine.loop; length = routine.lengthBeats` (NOT `maxAt`);
  preamble from `routine.preamble` via `resolveKey` (`Program.cpp:96-153`; deck-relative keys resolve on
  `comp.activeDeckIndex`, `:27-32`): ExactMatch -> emit; PositionOnly/NameOnly -> emit + `addPreambleRebind`
  (`:166-172`, reason "preamble: routine ..."); Missing -> `report.preambleUnresolved.push_back({key, reason})`.
  Discrete -> `Fired{0, 0, key, target, DiscretePoint{v, action, origin = Preamble}}`; continuous -> `PreambleSet{key,
  target, norm}`; `report.preambleCount` = emitted. When `routine.restoreState == false` the preamble is still
  compiled (so the report is honest) but the engine never fires it (4.2).
- Lanes via `compileLanes(routine.lanes, TempoMap{}, comp, Beat, nullopt, ...)` (the empty map is never read on the
  Beat clock -- `pickAt` uses `p.beat`, `convertBeatX` returns `beatX`).

### 3.5 Relative lanes, loop/once, restore vs start-from-now, next-bar start -- one line each
- Relative: x = beats since the fire boundary; `pos = clock.beat - startBeat` (4.2). No tempo map in a routine.
- Loop: at `pos >= lengthBeats` -> `startBeat += lengthBeats` (exact, no drift), `player.stop(sink)` (releases),
  `player.start(0)`, re-fire the preamble if `restoreState` (D9 "loop restarts re-fire the preamble", `spec:492-493`),
  continue in the same tick with the residual. Once: `player.stop(sink)` -> the routine leaves `running_`; the look
  HOLDS (manual values stay; a connected param glides back to its signal -- D8 hand-back, ruling 7).
- Restore (default): `firePreamble` at the start boundary; "Start from now" (`restoreState = false`): skipped.
- Next bar (default `quantize = Bar`, ruling 27): the routine is PENDING until the engine sees a `totalBarCount`
  rising edge; TwoBar/FourBar additionally need `snap.barCount % N == 0` at that edge (the clip's own counter, F4);
  global Quantize (`Composition::quantizeMode`, `Composition.h:99-100`) overrides via the same
  `quantizeModeToForcedSnap` rule (`MainComponent.cpp:24-31`); tracker not locked / `bpm == 0` -> start NOW with a
  notice (mirrors that rule's honesty: a queued trigger that may never drain is worse than an immediate one).

### 3.6 Take meta addition (S, additive): `Meta::startBeatInBar`
"Bars 33 to 40" needs the take's bar grid; today a take knows beats since Record but not where its bars fall.
Add `double startBeatInBar = -1.0` (unknown) to `Meta` (`Take.h:48-57`; `Take.cpp:107-125` toVar/fromVar, absent ->
-1); `RecorderHost::ArmOptions` gains `double startBeatInBar = -1.0` (`RecorderHost.h:94-106`), stored at arm and
written into `meta` at the three save sites (arm's provisional save, `tick`'s periodic save `RecorderHost.cpp:483-489`,
`disarm`); `MainComponent::perfRecord` (`MainComponent.cpp:5142`, lane 1b) fills it from `snap.beatInBar +
snap.beatPhase` when `snap.trackerState == BPMTracker::STATE_LOCKED` (`FeatureSnapshot.h:40-45`).
`takeBeatOfBar(take, n) = (n - 1) * 4 + fmod(4 - startBeatInBar, 4)` (bar 1 = the first FULL bar after Record; the
partial bar before it is bar 0); unknown (-1) -> treat as 0 and say so in the save notice ("bars counted from the start
of the take"). Old takes: unchanged. New accessor `const Take* RecorderHost::loadedTake() const` (returns
`loadedTake_ ? &*loadedTake_ : nullptr`, `RecorderHost.h:363`).

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

    // 120 Hz, right after recorderHost_.tick and BEFORE connectionEngine_.tick (ordering fact, 4.5).
    // `comp` is read for the bank listing published in Status; `forcedSnap` = quantizeModeToForcedSnap(...)
    // computed by MainComponent every tick (Off when Quantize is off or the tracker is not locked).
    void tick(const FeatureSnapshot& snap, double wallNow, const Composition& comp, Clip::BeatSnapMode forcedSnap);

    // Compiles NOW against `comp` (targets resolved once, D2), then queues per quantize. Returns "" or a refusal.
    // beatAvailable == false (tracker not locked or bpm == 0) => starts immediately + notice (3.5).
    std::string fire(const Composition& comp, int slot, Clip::BeatSnapMode forcedSnap, bool beatAvailable);
    void stop(int slot);          // releases every grip (Player::stop), idle at once
    void stopAll();               // global Stop, composition load, shutdown
    Status status() const;        // mutex-guarded copy (HTTP thread reads it)
    void setLastSaved(const Status::LastSaved&);   // MainComponent's save funnel reports through this
    static constexpr int kBankSize = Composition::kRoutineBankSize;
private:
    struct SlotSink;              // Sink -> dispatch; counts `skipped`; arbitrates routine-vs-routine (4.3)
    struct Running { int slot; std::string uuid, name; std::shared_ptr<const Program> program;
                     std::unique_ptr<Player> player; std::unique_ptr<SlotSink> sink;
                     bool pending = true; Clip::BeatSnapMode snap; bool restore, loop; double lengthBeats;
                     double startBeat = 0.0; uint32_t startedTotalBar = 0; int cycle = 0, restarts = 0;
                     bool restartRequested = false; int preambleFired = 0, preambleRefused = 0, yielded = 0;
                     bool done = false; };
    std::vector<Running> running_;   // at most one per slot; compacted AFTER each tick's loop (F5)
    struct LaneOwner { int slot; };  // F1: the routine whose ACCEPTED touch on this control is the most recent
    std::map<ControlPath, LaneOwner> laneOwner_;
    RecorderClock clock_;             // the routine beat clock: ticks EVERY tick from app start
    bool haveBar_ = false; uint32_t lastTotalBar_ = 0; double lastWholeBeat_ = -1.0;
    void startNow(Running&, const FeatureSnapshot&);   // startBeat = clock.beat; start(0); preamble; counters
    bool dueNow(Clip::BeatSnapMode, bool beatEdge, bool barEdge, const FeatureSnapshot&) const;
    void publishStatus(const Composition&);
    mutable std::mutex statusMutex_; Status published_;
};
```
`Status` (published every tick and on every transition): `double clockBeat; bool beatAvailable; int fires;
std::string lastError; struct Slot { int slot; std::string uuid, name; double lengthBeats; bool loop, restoreState;
std::string quantize; int lanes, preambleEntries; std::string state; /* "empty"|"idle"|"pending"|"running" */
double position; int cycle, restarts; uint32_t startedTotalBar; int unresolved, reboundByPosition, reboundByName,
preambleUnresolved, preambleCount, preambleFired, preambleRefused, skipped, yielded; } slots[kBankSize]; struct
LastSaved { int slot = -1; std::string uuid, name; int lanes, preambleEntries, preambleUnknown; std::vector<std::string>
dropped; } lastSaved;`. `yielded` = how many of this routine's gestures were displaced by ANOTHER routine's later
begin (4.3) -- never silent.

### 4.2 Scheduling (tick), exactly
1. `clock_.tick(snap, wallNow, 0)` -- the `sample` argument is unused here (`RecorderClock.h:45`); the TempoMap it
   grows is bounded (one anchor per 32 beats + bpm changes >= 0.05 BPM, `RecorderClock.cpp:85-86`, `RecorderClock.h:70-74`;
   the runtime critic traced `BPMTracker::bpm()` to the hysteresis-gated `lockedBPM_`, so periodic anchors dominate:
   ~1000 anchors per 4-hour set, INFERRED) and never read. Reusing the proven class beats a second beat-integrator
   (`tests/test_take.cpp:227,564,605` `[recorderclock]` pin monotonicity across resync/unmetered).
2. Edges: `barEdge = haveBar_ && snap.totalBarCount != lastTotalBar_` (the rising-edge counter the codebase
   recommends, CLAUDE.md pitfall 32; bars are >= 1.2 s at 200 BPM, never missed at 120 Hz); `beatEdge =
   floor(clock_.now().beat) != lastWholeBeat_`. Update both trackers after use.
3. `dueNow(mode, beatEdge, barEdge, snap)`: `Off` -> true; `Beat` -> beatEdge; `Bar` -> barEdge; `TwoBar` -> barEdge
   && `snap.barCount % 2 == 0`; `FourBar` -> barEdge && `snap.barCount % 4 == 0` -- the EDGE is `totalBarCount`'s
   (never rewound), the PARITY is `barCount`'s, exactly the counter `Layer::processPendingTrigger` consults
   (`Layer.h:305-309` via `Autopilot.cpp:56-57`), so a routine set to "2 Bar" and a clip set to "2 Bar" fire on the
   SAME bar, and both re-phrase together after a structural reset or a manual Resync (`BPMTracker.cpp:500-506,539`).
   For each `Running& r` (index loop over `running_`, NO erase inside): if `r.pending`: due -> `startNow(r, snap)`.
   Else: `pos = clock.beat - r.startBeat`; if `r.restartRequested && dueNow(r.snap, ...)` -> `player->stop(*sink)`,
   `restarts++`, `startNow` (retrigger = restart at the next boundary, D9 `spec:491-492`); else if `pos >=
   lengthBeats` -> loop: `player->stop(*sink); startBeat += lengthBeats; cycle++; player->start(0); if (restore)
   preamble counters += firePreamble; player->advanceTo(pos - lengthBeats, *sink)`; once: `player->stop(*sink);
   r.done = true` (the look holds); else `player->advanceTo(pos, *sink)`.
   After the loop: `std::erase_if(running_, [](const Running& r) { return r.done; })` (C++20). A `Running` is never
   erased while any reference into `running_` is live (F5); test 15 finishes two once-mode routines in one tick.
4. `startNow`: `startBeat = clock.beat; startedTotalBar = snap.totalBarCount; player->start(0.0); if (restore)
   preambleRefused = player->firePreamble(*sink); preambleFired = report.preambleCount - preambleRefused; pending =
   false;` one notice: `"Routine <name> started on bar <n>"` (+ counts when non-zero, whole words: "N settings could
   not be restored (a layer or clip no longer exists)", "N controls you are holding were left alone").
5. `publishStatus(comp)`.
Unmetered (`bpm == 0`): `clock_` freezes (`RecorderClock.cpp:31-39`) so a running routine holds its position and a
pending one waits; `fire()` with `beatAvailable == false` starts immediately with the notice "no beat yet -- the
routine starts now". Disclosed.
Re-entrancy: `fire()`/`stop()` are never called from inside `tick()` (REST arrives via `callAsync`, bindings/OSC as
their own messages); the reviewer greps that no lambda assigned to `routineEngine_.dispatch.*` names `routineEngine_`.

### 4.3 Stacking (F1), re-fire, stop, deck switch, missing targets
- **Two routines on ONE continuous control -- D9 `spec:495-497` "the later `begin` wins for the rest of that gesture",
  implemented in `SlotSink` (the engine sees every routine's touch/set/release; `ManualWrite`/`Player` untouched):**
  - `SlotSink::touch(key, grip)`: `ok = dispatch.touch(key, grip)`; if `ok`: `laneOwner_[key] = {mySlot}` (an
    ACCEPTED touch is a gesture begin; a touch refused by a human grip records nothing -- the human displaced us,
    D8). Return `ok`.
  - `SlotSink::set(key, v)`: if `laneOwner_` has `key` with `slot != mySlot` -> `++yielded` (once per gesture: the
    Player marks the cursor displaced on the first refusal, `Player.cpp:154-155`, and never calls `set` again for
    that gesture), return `false` WITHOUT calling `dispatch.set`. Else return `dispatch.set(key, v)`.
  - `SlotSink::release(key)`: if `laneOwner_[key].slot == mySlot` -> erase + `dispatch.release(key)`; else no-op (a
    routine that was overtaken never closes the grip the later routine now refreshes; the Player already skips
    `release` for a displaced cursor, `Player.cpp:143-144`, so this branch only catches a gesture that ended before
    its first refused `set`).
  - `stop(slot)`/`stopAll()`: after `player->stop(*sink)` also erase every `laneOwner_` entry with that slot.
  - READING of "for the rest of that gesture" (section 9 item 8): the EARLIER gesture is displaced for ITS remainder
    -- the same per-gesture `displaced` rule the built Player applies when a human refuses a lane (`Player.cpp:136-138,
    151-156`); the earlier routine does not come back mid-gesture when the later one lets go, it comes back at its
    own next gesture. Two routines touching one control in the SAME tick: tick order (fire order) is the tie-break,
    the later-fired one wins -- stated, not hidden.
  - The preamble participates: a restore (`firePreamble`: touch -> set -> release in one call, `Player.cpp:92-100`)
    that begins later displaces a running routine's gesture on that control for its remainder -- "a hand that grabs
    later wins", consistent by construction.
  - The underlying grip: both routines write at `Hand::Lane` through `manualWriteCore` (equal rank refreshes,
    `ManualWrite.cpp:168-177`); a human Held/Decaying grip refuses both (D8); each Player marks itself displaced for
    that gesture only. Test 10 discriminates gesture-begin order from fire order; test 15 covers compaction.
- Two routines on one layer's clip: last trigger wins (one active clip per layer) -- inherent.
- Routine vs the SET REPLAY: tick order only (replay first, `MainComponent.cpp:3392-3400`, routines second) -- a
  routine fired over a replay writes last each tick. NOT D9's "identical rules" (the replay's `HostSink` is in
  `RecorderHost.cpp:55-109`); disclosed R13, DEFERRED.
- Re-fire while running: `restartRequested = true` (restart at the next boundary); while pending: no-op.
- `stop(slot)`: `player->stop(*sink)` (every grip released, R9) -> idle, published at once. `stopAll()`: the same
  for all. Called from: REST `stop`, `Binding::Action::GlobalStop` and the TopBar Stop lambda (`MainComponent.cpp:
  691-700` `topBar_->onStop`, `:6857-6860`) -- ruling 5 "Stop means stop" extended to running routines (feel check,
  section 8); `swapCompositionModel` before the swap (`MainComponent.cpp:2951`); `~MainComponent` before
  `recorderHost_.shutdown`.
- Deck switch mid-routine: targets were resolved at fire time to the deck that was active THEN
  (`ResolvedTarget.deck`), and every handler takes that deck explicitly (`recorderHost_.dispatch.fire` passes
  `f.target.deck`, `MainComponent.cpp:1960,1984,1990,1996`; UI refresh gated to the active deck, `:4181-4187`) -- so
  the routine keeps playing on the deck it started on, exactly as a replay does. Not stopped (persistent layers
  exist, `Layer.h:56`). A routine fired on the NEW deck resolves there. Disclosed.
- A lane whose layer/clip/effect no longer exists: compiled OUT and counted (`report.unresolved`, D2 policy 3,
  `Program.cpp:464-474`); preamble entries -> `preambleUnresolved`; fire-time refusals (`dispatch.fire` false) ->
  `skipped` (SlotSink, same rule as `RecorderHost.cpp:70-85` minus the WithAudio audio-skip; a Preamble-origin
  refusal is counted by `preambleRefused`, never double-counted as `skipped`); a human grip at the preamble ->
  `preambleRefused`. All in `/api/routine/status` and in the start notice. No silent path.

### 4.4 What a routine does NOT do in slice 1 (disclosed)
It is not recorded into a take (no `routine` lane, no `via`); its writes enter the handlers as `Origin::Replay`
(so capture gates `origin != Replay` at `MainComponent.cpp:5488,5513,5546`, `RecorderHost.cpp:569` and undo gates
`origin == Human` hold with NO new gate -- same construction as the preamble, `:1945-1953`). If a take is being
recorded while a routine plays, the take gets nothing from the routine (slice 2 adds the trigger + `via`). Video
playheads / crossfade / pending clip triggers are engine state and are not restored (D4 `spec:259-261`).

### 4.5 Threading contract (against the Sacred Rules)
| State | Owner | Readers | Mechanism |
|---|---|---|---|
| `RoutineEngine` (clock, players, running_, laneOwner_) | message thread (120 Hz `tickFeaturePipeline`; REST via `callAsync`; bindings/OSC already on the message thread) | -- | confinement + the `RECORDER_HOST_ASSERT_MESSAGE_THREAD` idiom copied (`RecorderHost.cpp:45-47`) |
| `Program` per running routine | immutable after `compileRoutine` | its `Player` | `shared_ptr<const>` |
| `Status` | written by `publishStatus` on the message thread | HTTP thread (`/api/routine/status`), the 4 Hz panel | mutex-guarded copy -- the same posture as `RecorderHost::status()` (`RecorderHost.cpp:891-895`); not a hot path (audio/analysis/GL untouched) |
| `Composition::routines/routineBank` | message thread | message thread only | REST handlers marshal (`callAsync`), status reads the engine's copy, never the vector |
| Model writes | the SAME handlers a click uses (`handleClipTrigger`, `applyLayerFlag`, `applyClipPlaying`, `manualWrite`) | GL thread reads the model as it does after any click | no new mutex; no allocation on the GL/audio/analysis threads; the GL thread may render one frame mid-restore (same class as a column trigger) |
Ordering: engine tick AFTER `recorderHost_.tick` (`MainComponent.cpp:3392-3400`) and BEFORE `connectionEngine_.tick`
(`:3418`) so a routine gesture's grip is current when the engine decides what to publish (ConnectionEngine.cpp's
ORDERING FACT cited at `:3381-3387`); replay first, routines second (R13).

---

## 5. SURFACES (slice 1)

### 5.1 REST (`src/api/ApiServer.{h,cpp}`, same 503-when-unwired + `callAsync` shape as `/api/perf/*`,
`ApiServer.cpp:1281-1374`; status synchronous from the engine's copy like `:1410`)
| Route | Body | Effect / answer |
|---|---|---|
| `POST /api/routine/save` | `{"name":"Drop 1", "fromBeat":128, "toBeat":144}` or `{"fromBar":33, "toBar":36}` (1-based, inclusive: `toBeat = takeBeatOfBar(toBar + 1)`); optional `slot` (0-7; default first free; occupied -> replaced, said in the notice; none free -> refused "the routine bank is full"), `loop`, `restoreState`, `quantize` ("off"/"beat"/"bar"/"2bar"/"4bar"), `wholeBars`, `takeFolder` (default: the LOADED take via `RecorderHost::loadedTake()`; none -> "No take is loaded. Use Load Take... first.") | marshalled -> `MainComponent::perfRoutineSave`; result in `status.lastSaved` / `status.lastError` + one notice "Saved routine <name> to pad N: M timelines, K restores" (+ ", dropped: tempo, audio" when any) |
| `POST /api/routine/fire` | `{"slot":0}` | `routineEngine_.fire(...)`; refusal text -> `lastError` |
| `POST /api/routine/stop` | `{"slot":0}` or `{"all":true}` | stop / stopAll |
| `POST /api/routine/set` | `{"slot":0, "loop":true, "restoreState":false, "quantize":"beat", "name":"..."}` (any subset) | edits the composition's routine; a RUNNING routine picks up `loop` at its next end, `quantize` at its next (re)start; `name` at once |
| `POST /api/routine/remove` | `{"slot":0}` | stops it if running, frees the pad, erases the routine if no other pad references it |
| `GET /api/routine/status` | -- | `{ok, clockBeat, beatAvailable, fires, lastError, lastSaved{slot,uuid,name,lanes,preambleEntries,preambleUnknown,dropped[]}, bank:[8 x {slot,uuid,name,lengthBeats,loop,restoreState,quantize,lanes,preambleEntries,state,position,cycle,restarts,startedTotalBar,unresolved,reboundByPosition,reboundByName,preambleUnresolved,preambleCount,preambleFired,preambleRefused,skipped,yielded}]}` |
Also (additive, needed by the probe and by Boris's own REST tooling): `GET /api/composition` clips gain
`"effects": [ { "name": ..., "bypassed": ..., "params": [ {"name": <ParamDef::name>, "value": <effParam(i)>} ] } ]`
(`ApiServer.cpp:365-379`; names via `renderer_.getEffectLibrary().getEffectDef(fx.effectName)` as `:498-502` does).

### 5.2 OSC (`src/osc/OscHandler.{h,cpp}`): `/audiodna/routine/{slot}` (value > 0 -> fire; 0 ignored), one branch in
the `startsWith` chain (`OscHandler.cpp:57-70` shape), callback `std::function<void(int slot)> onTriggerRoutine`
(`OscHandler.h:45-57` list), wired in `MainComponent.cpp` next to `oscHandler_.onTriggerClip` (`:2083`). Header
pattern list `OscHandler.h:14-26` +1 (doc count 13 -> 14 goes to the docs step).

### 5.3 Binding (`src/binding/Binding.h`, `BindingManager.cpp`, overlays): append `TriggerRoutine` AFTER
`MasterSignal` (`Binding.h:45`; int-serialized, append-only); `int targetRoutineSlot = 0;` in the targets block
(`:83-87`) + one `setProperty`/`getProperty` pair (`BindingManager.cpp:235-240,278-283`); `BindableTarget` gains
`int routineSlot = 0` (`src/ui/BindingOverlay.h:33-43`) copied at the three minting sites (`BindingOverlay.cpp:185-198`,
`MidiLearnOverlay.cpp:223-234,269-280`); `buildBindableTargets` adds 8 targets in a new row under the global row,
labels "Routine 1".."Routine 8" (whole words; `MainComponent.cpp:6574-6590` idiom); `handleBindingAction`: `case
TriggerRoutine: if (value > 0) fire(slot) else if (triggerMode == Momentary) stop(slot)` (hold-to-run for free;
Toggle = press to fire/restart). The overlays' `switch (target.action)` blocks already `default:` (`BindingOverlay.cpp:
289-291`, `MidiLearnOverlay.cpp:336-338`); `-Wall -Wextra` without `-Werror` (`cmake/CompilerWarnings.cmake:13`) means
an unhandled enumerator warns, never fails -- the Builder still adds the case.

### 5.4 UI ruling: slice 1 needs NO new panel to be Boris-usable for FIRING (a bound key or MIDI pad, learnable in
bind mode from "Routine N"), and REST for authoring. A minimal bank strip is LANE 3, optional, cut-able (section 7):
in the Browser's Record tab (`src/ui/RecordPanel.{h,cpp}`, `RecordPanel.cpp:255-297` row layout, hooks pattern
`RecordPanel.h:29-43`), a "Routines" row of 8 pads (`"1: Drop 1"`, `"1: Empty"`; tone Playing while running, Warning
while pending, text "(next bar)"/"(bar 2)") + a "Save Routine" row: `From bar` / `To bar` editors + name + button,
driven by a pure `src/ui/RoutineBankModel.h` (`deriveRoutineBankView(const RoutineEngine::Status&)`, ctest-pinned like
`RecordPanelModel.h:1-30`). Whole words only (CLAUDE.md UI Text Rules). Why here and not the deck: ruling 5 (current
UI is disposable; spend on mechanism), ruling 20 (own bank, not grid cells), and the Record tab already has the perf*
funnel plumbing (`MainComponent.cpp:1587-1604`). Why not zero UI: ruling 2 (dead UI gets built; a bank that exists only
over REST is a pad Boris cannot press) -- but the engine + REST + binding are gate-complete without it, so it is the
first thing to cut.

---

## 6. TESTS

### 6.1 ctest -- TWO new targets so the two lanes never edit each other's CMake block
- `test_routine` (lane 1a): copy the `test_program_preamble` block (`tests/CMakeLists.txt:772-808`: Clip/Layer/
  ConnSerialization/EffectLibrary/Effect/TempoMap/PerfState/PerfStateCapture/Take/Program + juce_core/events/graphics)
  plus `${SRC_DIR}/recording/RoutineSlice.cpp`; cases 1-6 and 13.
- `test_routine_engine` (lane 1b): the same list plus `${SRC_DIR}/recording/RoutineEngine.cpp`,
  `${SRC_DIR}/recording/Player.cpp`, `${SRC_DIR}/recording/RecorderClock.cpp`, `${SRC_DIR}/binding/BindingManager.cpp`;
  cases 7-12 and 15. (`test_routine_bank_model`, lane 3 only: case 14, juce_core-only like `test_record_panel_model`.)
Idioms: `FakeSink` (`tests/test_take.cpp:47`), `FakeDispatch` (`test_recorder_host.cpp:107-133` -- for the engine, a
struct with the five lambdas of 4.1), `makeComposition()` = `comp.initDefault()` (Deck 1 / Layer 1..3 / 12 columns,
`src/model/Deck.h:28-40`), `makeSnap(bpm, phase)` (`test_recorder_host.cpp:55-62`; the engine tests also set
`barCount`/`totalBarCount`), `EffectLibrary::registerDefaults()` (`test_program_preamble.cpp:24-32`). Every case is RED
before the code exists (compile failure counts as RED; the Builder commits the RED tests first).

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
| 10 | `[routine][engine][stacking]` | **F1 discriminator.** A (constant 0.2 gesture on L0 opacity over [0,10], then [12,14]) and B (constant 0.8 gesture on the same key over [2,5]); FakeDispatch accepts every touch/set. Run 1: fire A, then B, same bar. Expect: ticks in [0,2) -> sets carry 0.2; from B's begin, every set on that key carries 0.8 and NO 0.2 set appears (A yielded: `slots[A].yielded == 1`); in [5,10) NO set at all on that key (A stays displaced for its gesture; B released once, owner erased); at [12,14) sets carry 0.2 again. Run 2: fire B first, then A -- IDENTICAL expectations (fire order does not decide). Run 3: both gestures begin at x=0 in the same tick -> the later-fired one's value stands from the second tick on (tie-break stated in 4.3) | 4.3 |
| 11 | `[routine][engine][quantize]` | **F4 pin.** quantize Off starts on the same tick; `Beat` on the next whole beat; `TwoBar`: a tick with `totalBarCount 11 -> 12` and `barCount 0 -> 1` (odd) does NOT start; the next edge `totalBarCount 13`, `barCount 2` DOES -- proving `barCount` (not `totalBarCount`) is the parity source; `FourBar` likewise with `barCount 4` | 4.2 |
| 12 | `[routine][binding]` | `Binding{action TriggerRoutine, targetRoutineSlot 5}` round-trips through `BindingManager::toVar/fromVar`; a v1 bindings JSON without the key loads slot 0 | 5.3 |
| 13 | `[take][meta]` (in `tests/test_take.cpp`, one SECTION on the existing round-trip at `:78`) | `Meta::startBeatInBar 2.5` round-trips; absent -> -1 | 3.6 |
| 14 | `[routine][bank]` (lane 3 only, `tests/test_routine_bank_model.cpp`) | pad text/tone per state: empty -> "1: Empty" disabled; idle -> "1: Drop 1"; pending -> Warning "(next bar)"; running -> Playing "(bar 2)" | 5.4 |
| 15 | `[routine][engine][compaction]` | **F5 pin.** Two once-mode routines (length 4, each with one held gesture) fired on the same bar; one tick jumps pos past 4 for both -> both `state idle`, exactly two releases logged, no skipped/duplicated entry (ASan/UBSan on via `apply_sanitizers`, `tests/CMakeLists.txt:1169` idiom) | 4.2 step 3 |
Expected: ctest 539 -> 539 + 14 (+1 with lane 3). The Builder REPORTS the number `ctest` prints; nobody inherits it.

### 6.2 LIVE PROBE `.harmony/probe-routines.sh` (+ fixture `.harmony/probe-routines.json`) -- Harmony runs it; RED
first on today's build (every `/api/routine/*` row 404s), then GREEN. Mechanics copied verbatim from
`probe-step3.sh:146-180,206-222,1093-1135` (helpers, `open -g` launch, graceful `osascript` quit, Quartz 0-Output-
window witness, `'MacOS/Audio-DN[A]'`) and `probe-mastersignal.sh:123,145-175` (pixel oracle: `MAD_THRESHOLD 0.5`,
`assert_nonblank`, `mean_abs_diff`, `.venv` PIL+numpy). Production mode, port 7070, NO `--test-mode`. No audio
device dependency: `POST /api/set_bpm {"bpm":120}` puts the tracker in manual LOCKED mode with a predicted phase
(`BPMTracker.cpp:76-82,545-554`; the same trick `probe-mastersignal.sh:289,441` uses), so a bar = exactly 2.0 s and
the take needs no audio (`"audio": false`). In that regime structural resets are suppressed (`BPMTracker.cpp:503-505`),
so `barCount` and `totalBarCount` advance in lockstep -- the probe uses `Bar` quantize; the `barCount` parity is
pinned by test 11, not by the probe.

Fixture: one deck "A", two layers; L1 col 0 = image clip `tests/fixtures/test_card.png` (mediaType 1, `mediaFile`,
the shape `probe-mastersignal.sh:405-428` uses) with effect "Brightness" params [0.5]; L1 col 1 = source
`solid_color` (`SourceRegistry.cpp:137`) with effect "Brightness" [0.5]; L2 col 0 = source `checkerboard`
(`:160`). Static content on both cells -> pixel comparisons are meaningful (no `u_time` drift; gravity_well-class
sources are black in a single frame, CLAUDE.md pitfall 22 -- avoided).

Rows (each `ok/no`; timing at 120 BPM manual: beat 0.5 s, bar 2 s):
1. Preconditions, launch, health, `/api/bpm` `totalBarCount` readable; `set_bpm 120`; `load_composition` fixture
   (`ApiServer.cpp:230`); `trigger_clip L0 C0`; sleep 1; frame `ref.png` -> non-blank.
2. Record: `/api/perf/record {"name":"probe-routines","audio":false}`; at +0.6 s `set_layer_opacity L0 0.5`; +1.6 s
   `set_layer_opacity L1 0.3`; +2.6 s `trigger_clip L0 C1`; +4.6 s `set_param {layer 0, column 1, effect
   "Brightness", param "amount", value 0.9}`; +6.6 s `set_layer_opacity L0 0.9`; +8.6 s `/api/perf/stop`. Rows:
   `recording false`, take.json has >= 4 lanes with `beat` stamps, `meta.startBeatInBar` present and in [0,4)
   (RED today: absent).
3. Save: `/api/perf/load {folder}`; `/api/routine/save {"name":"Probe Routine","fromBeat":0,"toBeat":16,"slot":0}`;
   poll `/api/routine/status` <= 1 s: `lastSaved.slot == 0`, `bank[0].name == "Probe Routine"`, `lanes >= 4`,
   `preambleEntries >= 5`, `lengthBeats == 16`, `dropped == []`, `state idle`. (RED today: 404.)
4. Perturb: `trigger_clip L0 C1`; `set_layer_opacity L0 0.1`; `set_layer_opacity L1 0.6`; `set_param C1 Brightness
   0.2`; sleep 1 (> `gripHoldMs` 250 ms, `Composition.h:92`, so the Decaying grips expire -- else the lane-rank
   restore is CORRECTLY refused); comp reads the perturbed values; frame `pert.png` non-blank; `mad(ref, pert) > 0.5`.
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
11. Stacking (F1, live): save a second routine to slot 1 from the same take with `fromBeat 4, toBeat 12` (its first
    move on `L0 opacity` lands at ~+2.6 s); fire slot 0, then fire slot 1 on the next bar; from slot 1's opacity move
    onward, `L0 opacity` follows slot 1's values (~0.9 by its +2.6 s) and `bank[0].yielded >= 1`; after slot 1 ends,
    `L0 opacity` does not jump back to slot 0's curve until slot 0's next gesture. Each expectation derived from the
    two saved lane sets, stated in the script as numbers.
12. Teardown per `probe-step3.sh:1093-1135` (graceful quit; 0 Output windows; no full-screen capture).
Artifacts under `/tmp/audiodna-routines/`; PASS/FAIL count printed; exit 1 on any FAIL. Harmony LOOKS at `rest.png`
before accepting (rig rule).

---

## 7. BUILD PLAN (F6/F7: two serial builder lanes with a live gate between them; 1a is the fallback landing)

Order: wave 1 (parity A / step3-row B / docs C / polish D, `.harmony/s-rta-0926-work.md:6-13`) MERGES FIRST --
"Build waits for wave-1 merge" (`:13`). Lane D's file fence is not written there, so a `MainComponent.cpp` collision
cannot be ruled out from disk; it is moot by order: lanes 1a/1b branch from the post-merge HEAD and re-anchor every
`MainComponent.cpp` cite by its function name. Disk: run `df -h /System/Volumes/Data` before each lane (8 GB/lane +
20 GB headroom; max 3 build lanes; 255 GB free at boot, `s-rta-0926-work.md:3`). After any merge that touches the
ROOT `CMakeLists.txt` (`:255-266` recording list) or `tests/CMakeLists.txt`: `cmake -S . -B build` before building.
Builders never run lldb/gdb, never launch the app except via the probe Harmony runs, delete their scratch build,
and the worktree is removed the turn it merges.

**LANE 1a -- format + slice + compile (one builder, worktree, opus tier; M; ~450 source + ~350 test lines).
Behaviour-neutral: adds no surface, no UI, no engine; the app runs exactly as before. Steps, each a commit, RED tests
first:**
1. `tests/test_routine.cpp` cases 1-6 RED + case 13 SECTION in `tests/test_take.cpp:78`; `tests/CMakeLists.txt`
   `test_routine` block (6.1).
2. `src/model/Routine.h` + `Composition.h` fields/serialization/helpers (3.1-3.2). Case 1 green.
3. `Take.{h,cpp}` `Meta::startBeatInBar`; `RecorderHost.{h,cpp}` `ArmOptions::startBeatInBar` -> meta at the three
   save sites + `loadedTake()` accessor (3.6). Case 13 green. (`perfRecord` is NOT touched here -- 1b fills the value;
   until then every new take carries -1 = unknown, exactly today's behaviour.)
4. `src/recording/RoutineSlice.{h,cpp}` (3.3, 3.6). Cases 2-5 green.
5. `Program.{h,cpp}`: `compileLanes` refactor, stamp rule, `compileRoutine` (3.4). Case 6 green; ALL existing
   `test_program_*`/`test_take`/`test_recorder_host` cases still green with NO edits (the refactor must be
   behaviour-neutral).
6. Root `CMakeLists.txt`: add `src/recording/RoutineSlice.h/.cpp` next to `:261`.
7. Builder report: real ctest count, the grep output of every call site of `compile(` (unchanged), deviations.
Lane 1a MUST NOT touch: `src/recording/Player.{h,cpp}`, `Lane.h`, `ControlPath.h`, `PerfState*` (format-frozen),
`RecorderClock.*`, `src/MainComponent.*`, `src/api/*`, `src/osc/*`, `src/binding/*`, `src/ui/*`, `src/render/*`,
`.harmony/probe-*.sh`, `CLAUDE.md`, `.harmony/APP-INVENTORY.md`, any spec. It must not "fix" the effects-parity blank
frame, the step3 `inputSource` row, or the Deck Load clip.

**GATE 1a (Harmony, before 1b starts):** rebuild `build/` (`cmake -S . -B build` first), ctest (539 -> 539 + 7),
`probe-step3.sh` UNCHANGED count (92/1 at close -- the `compileLanes` refactor is the one shared-code touch, R6),
`probe-mastersignal.sh` unchanged. Reviewer: `Player.cpp` diff empty; `compile()` signature unchanged; `git diff
--stat` shows only the 1a files. If this gate fails, the cost of unwinding is one lane, not two. If the session
ends here, 1a MERGES ON ITS OWN (F7): it is tests + format + a compile function with no caller outside tests --
the same class as s167 build-order step 1 -- and 1b becomes "slice 1b" next session with this file as its plan.

**LANE 1b -- engine + wiring + surfaces + probe (one builder, worktree, opus tier; M-large; ~500 source + ~350 test
+ ~400 probe lines).** Steps, each a commit, RED tests first:
1. `tests/test_routine_engine.cpp` cases 7-12, 15 RED; `tests/CMakeLists.txt` `test_routine_engine` block (6.1).
2. `src/recording/RoutineEngine.{h,cpp}` (4.1-4.3, incl. `laneOwner_` and the mark-then-compact loop). Cases 7-11,
   15 green.
3. `Binding.h`, `BindingManager.cpp` (5.3 model half). Case 12 green.
4. App wiring in `src/MainComponent.{h,cpp}`: member `RoutineEngine routineEngine_;` (declared AFTER `recorderHost_`,
   `MainComponent.h:561`, so it is destroyed first); `dispatch` = copies of the four recorder lambdas + notify
   (right after `:2032`); tick call after the `recorderHost_.tick` block (`:3392-3400`) with `forcedSnap =
   quantizeModeToForcedSnap(composition_.quantizeMode, snap)`; `perfRoutineSave/Fire/Stop/Set/Remove` funnel next to
   the perf* funnel (`:5142-5356`, returning "" or the refusal text; notices through `dispatch.notify`; save reports
   through `setLastSaved`); `routineStatusVar()` (reads ONLY `routineEngine_.status()`); `handleBindingAction` case;
   `buildBindableTargets` 8 targets + `BindableTarget::routineSlot` + the three overlay minting one-liners; TopBar
   `onStop` and `GlobalStop` -> `stopAll()`; `swapCompositionModel` -> `stopAll()` before the swap; `~MainComponent`
   -> `stopAll()` before `recorderHost_.shutdown`; `perfRecord` fills `ArmOptions::startBeatInBar` (3.6).
5. `ApiServer.{h,cpp}`: six routes + callbacks (5.1) + composition `effects` readback; `OscHandler.{h,cpp}` (5.2).
6. Root `CMakeLists.txt`: add `src/recording/RoutineEngine.h/.cpp` next to `:261`.
7. `.harmony/probe-routines.sh` + `.harmony/probe-routines.json`: written, `bash -n`-checked, NOT run by the
   Builder ("the party that builds never verifies"); `git add -f` under `.harmony/` (force-tracked, use `;` not
   `&&`).
8. Builder report: real ctest count, the exact `mad` numbers it could NOT measure (Harmony measures), the grep
   output for the re-entrancy rule (4.2), deviations.
Lane 1b MUST NOT touch: `src/recording/Player.{h,cpp}` (if a Player change seems needed, stop and report -- the
scheduler owns loop/restart and the sink owns arbitration), `src/connect/ManualWrite.*` (F1 is solved in the engine's
sink, never in the grip), `Lane.h`, `ControlPath.h`, `PerfState*`, `Program.*`/`RoutineSlice.*`/`Routine.h`/`Take.*`
(1a's, merged), `src/render/*`, `src/ui/*` except the 3 overlay one-liners and `BindingOverlay.h`,
`CompDecksBrowser.*`, `ClipInspector.*`, `RecordPanel.*`, `RecordPanelModel.h`, `.harmony/probe-step3.sh`,
`CLAUDE.md`, `.harmony/APP-INVENTORY.md`, any spec. Same three "do not fix" items as 1a.

**LANE 2 -- review (independent reviewer) + Harmony's gates after 1b:** rebuild `build/` (`cmake -S . -B build`
first), ctest, `probe-routines.sh` RED on the pre-1b binary then GREEN, `probe-step3.sh` unchanged count,
`probe-mastersignal.sh` (the tick ordering change sits next to its consumers), look at `rest.png`. Reviewer greps:
`grep -rn "Origin::Routine" src/` -> only enum/vocabulary (no capture path); `grep -n "routines\|routineBank"
src/model/Composition.h` -> both cleared in `fromVar`; every `firePreamble` call in `RoutineEngine.cpp` gated on
`restore`; no `std::mutex` outside `publishStatus/status`; `Player.cpp` and `ManualWrite.cpp` diffs empty; no
`routineEngine_` inside any `routineEngine_.dispatch.*` lambda; `Binding::Action` diff is a pure append after
`MasterSignal`; no `running_.erase` inside the tick loop (only the post-loop `erase_if`).

**LANE 3 -- optional bank strip (one builder, worktree, sonnet tier; S-M; AFTER 1b merges, never parallel with it
because both touch `MainComponent.cpp`):** `src/ui/RoutineBankModel.h` (new), `src/ui/RecordPanel.{h,cpp}` (rows 5.4;
`std::function` hooks `onFireRoutine(int)`, `onStopRoutine(int)`, `onSaveRoutine(name, fromBar, toBar)`,
`onRoutineStatus()`, the `RecordPanel.h:29-43` idiom), `tests/test_routine_bank_model.cpp` + CMake block, and ONLY
the RecordPanel wiring block of `MainComponent.cpp` (`:1587-1604`). Gate: window-only shot of the Record tab (Quartz
window id, never full-screen) + case 14 + the critic panel. Cut without loss if the session runs short: everything
Boris-visible in 6.2 holds without it.

Sequence: lane 1a -> gate 1a -> lane 1b -> reviewer + Harmony gates -> merge, remove the worktree the same turn ->
lane 3 (optional) -> docs (AFTER wave-1 lane C has merged its doc rows; it owns those files today): APP-INVENTORY
REST count +6 and a Routines row, `CLAUDE.md` one paragraph under "Audio Store (Ruling 28)"/Step 3 + the source-tree
entries for `Routine.h`, `RoutineSlice`, `RoutineEngine`, OSC pattern count 13 -> 14, `binding-decisions.md` "BUILT
<commit>" notes under rulings 17/20/22/26/27.

---

## 8. RISKS (file each lives in) and what ONLY Boris can check

- **R1 (strongest counterargument to the recommendation):** "drive routines through the existing per-layer
  `pendingTriggerColumn` + `Program::Range` and skip a new engine." It loses: `Range` synthesizes no preamble
  (`Program.h:21-24`), pending triggers are per-layer/one-column and drained on the GL thread
  (`Autopilot.cpp:43-60`) where a Player must never run (spec section 4, `RecorderHost.cpp:45-47` asserts the message
  thread), and a routine spans many controls that must start/stop as one (D9). The engine is ~280 lines around a
  Player that already exists.
- **R2 Two beat clocks disagree** (`RoutineEngine.cpp` vs the GL-thread `Autopilot` beat crossing): a routine's clip
  hit at x = 0 fires at the bar edge seen at 120 Hz on the message thread; a human quantized trigger fires at the
  beat crossing seen on the GL thread -- up to ~16 ms apart. Same class as replay today; disclose, do not "fix".
- **R3 Bar parity after a phrase reset or manual Resync** (`BPMTracker.cpp:500-506,539`): `barCount` restarts at 0,
  so a pending "2 Bar"/"4 Bar" routine re-phrases to the new phrase grid -- the SAME thing a queued "2 Bar" clip does
  (`Layer.h:305-309`). Accepted by design (F4); the bar EDGE itself is still `totalBarCount`'s and is never missed.
- **R4 Restore refused by a human grip** (`ManualWrite.cpp:168-172`): counted in `preambleRefused`, said in the
  notice; the probe's `sleep 1` after REST perturbations is load-bearing (`gripHoldMs` 250 ms, `Composition.h:92`).
- **R5 Preamble order** (`RoutineSlice.cpp`): `playing` before `activeClip` auto-plays a paused clip (`Layer.h:271-274`).
  Test 2 pins the order; the preamble suite already pins the same rule for takes.
- **R6 The `compileLanes` refactor** (`Program.cpp:459-531`): a behaviour drift there breaks every replay. Guard: all
  existing program/host tests green with NO edits, test 6's Wall-clock regression clause, AND gate 1a's live
  `probe-step3.sh` run BEFORE anything is built on top (F6). The stamp-rule change is reporting-only (gate critic
  VERIFIED: `convertBeatX` is the identity on the Beat clock, `Program.cpp:450`).
- **R7 `swapCompositionModel` while a routine runs** (`MainComponent.cpp:2951`): a Player holding coordinates into a
  replaced model would write into the new one. `stopAll()` before the swap; reviewer greps for it.
- **R8 Status copy cost** (`RoutineEngine::publishStatus`): 8 slots x ~21 fields copied under a mutex at 120 Hz on the
  message thread -- trivial, but keep the bank NAME strings out of the per-tick copy when nothing changed (copy names
  on transitions only) if the reviewer measures otherwise. Not a hot path by the Sacred Rules' definition.
- **R9 Static image clip loads on restore** (`handleClipTrigger` `:4189-4233` loads media synchronously): a routine
  restoring N layers' clips does N loads once at its start -- same as a column trigger. Acceptable.
- **R10 `Binding::Action` append** (`Binding.h:24-46`): appending after `MasterSignal` keeps saved int values valid;
  INSERTING anywhere else would silently retarget every saved binding. Reviewer checks the enum diff is a pure append.
- **R11 Unknown-bar takes** (`Meta::startBeatInBar == -1`, every take recorded before this lands): "bars" in the save
  form count from the take's start, said in the notice. Old takes are never rewritten.
- **R12 The probe's pixel bound** (`probe-routines.sh` row 6, `mad(ref, rest) <= 2.0`): ASSUMED; a static image +
  solid_color should reproduce near-exactly, but the Builder cannot run the app -- Harmony reads the real number on the
  first GREEN run and pins it with a margin, never widens it to pass.
- **R13 Replay-vs-routine arbitration is tick order, not D9's "identical rules"** (`RecorderHost.cpp:55-109` vs
  `RoutineEngine::SlotSink`): a set replay and a routine on one control both write at `Hand::Lane`; the routine ticks
  second (`MainComponent.cpp:3392-3400` then the new tick) so it writes last every tick regardless of which gesture
  began later. Disclosed in DEFERRED; slice 2 moves `laneOwner_` into a shared table both sinks consult.
- **R14 `laneOwner_` and grips can disagree for one tick** (`RoutineEngine.cpp`): routine A's `set` is refused by the
  engine (owner = B) while the underlying grip is still the Lane-rank grip both refreshed -- harmless: B refreshes it
  on its next `set` (`ManualWrite.cpp:168-177`), and if B is itself refused by a human the human already owns the
  control. No connection glide can slip in because the grip is never released by the loser (4.3 release rule).

**Only Boris can check (Tier 4, feel):** (1) the restore is a hard cut for opacity/knobs and a normal clip
transition for clips (`Layer.h:265` runs the layer's own transition) -- right, or should it fade? (2) Global Stop
also stops running routines -- does "Stop means stop" extend to routines, or should they ride through? (3) two
routines reaching for one knob: the one that grabbed it LATER keeps it until its own move ends, and the earlier one
stays quiet until its NEXT move (it does not jump back in when the later one lets go) -- does that feel like hands?
(4) loop restart re-snaps the look every cycle -- wanted, or should loops skip the restore after cycle 1? (5) a bound
MIDI pad: does the next-bar wait read clearly enough on the pad (lane 3's "(next bar)")? (6) whole-bar rounding of a
slice. (7) "2 Bar" / "4 Bar" for a routine counts bars from the last phrase reset, like a clip -- right?

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
6. Slice 1 records nothing about routines into a take (4.4). Slice 2 owns the `routine` lane + `via`.
7. **(F2) "Restore the state it was recorded in" = the state at the SLICE'S OWN START for the controls the routine
   touches** (3.3), read through D4 step 2 -- not the take's checkpoint 0 in full. The other reading is one flag away.
8. **(F1) "The later begin wins for the rest of that gesture" = the EARLIER routine's gesture is displaced for its
   remainder** (the Player's built per-gesture displacement, `Player.cpp:151-156`); it resumes at its next gesture,
   not when the later routine lets go. Same-tick ties go to the later-fired routine.
9. "2 Bar" / "4 Bar" quantize for a routine counts from the last phrase reset (`barCount`), exactly like a clip (F4,
   ruling 27's "consistent with a quantized clip trigger").
None of these blocks the build; each is one flag or one line to flip.

## 10. OPEN BORIS QUESTIONS (one sentence each, next status update; no design change needed to answer)

1. (F2/F8) "When a routine restores before playing, it puts back only the knobs and clips it is about to move, the
   way they were at the START of the bars you cut -- not the way the whole show looked when you pressed Record. Right?"
2. (F1) "If two routines reach for the same knob, the one that grabbed it later keeps it until its own move is over,
   and the first one stays quiet until its next move. Right?"

---

## TRADEOFFS CONSIDERED

- **Preamble as a filtered `PerfState` reusing `buildPreamble`** -- rejected: `LayerRuntime` carries all five flags +
  opacity + activeClip and `buildPreamble` emits all of them plus activeDeck/quantize (`Program.cpp:229-413`) -- a
  routine that only moves one knob would restore a whole layer (D4 rejected "full snapshot on trigger",
  `spec:277-279`). An explicit entry list restores exactly the touched controls and reuses `firePreamble` unchanged.
- **Preamble as x=0 points inside the lanes** (D9 sketch "preamble points at x = 0", `spec:478`) -- rejected: a
  zero-length gesture never calls `set()` (`Player.cpp:141-157`: release fires when `pos >= x1` before any set), so a
  continuous restore would need a fake length; `Program::preamble/preambleContinuous` already exist for this.
- **Routines as a clip `MediaType`** -- rejected by D9 (`spec:506-508`) and ruling 20.
- **A bespoke `BeatClock` instead of `RecorderClock`** -- rejected: the integration rule (wrap detection, resync
  absorption, unmetered freeze) is already implemented and tested; the unused TempoMap growth is bounded.
- **(F1) Gesture arbitration inside the grip (`ParamConnection::Grip` gains a holder token; `ManualWrite.cpp`
  compares it)** -- rejected for slice 1: it touches the connection lane's funnel every writer shares (R8 of the spec)
  and the replay's sink for a rule only routines exercise today; the engine-level `laneOwner_` gives D9's behaviour
  for routine-vs-routine with zero shared-code change, and moving it into a shared table later is additive (R13).
- **(F1) Disclosing fire-order stacking as a slice-1 simplification instead of implementing** -- rejected: the fix
  is ~30 lines in a new file, and shipping the wrong rule would give Boris a behaviour that later FLIPS on him (who
  wins would change with the fix) -- worse than building it once.
- **(F4) Keeping `totalBarCount` parity as a routine-specific meaning of "2 Bar"** -- rejected: ruling 27 ties routine
  quantize to the clip trigger's behaviour, the clip uses `barCount`, and two identical labels with two counters is a
  quiet failure class.
- **Record routine children with `Origin::Routine` now** -- rejected: without `via` (needs a parameter through 5
  capture sites + the continuous hook) slice 2 would double-fire takes recorded in slice 1; recording nothing is the
  only choice that is safely additive.
- **Parallel lanes (core || wiring) via a header contract** -- rejected: both end in `MainComponent.cpp`, and three
  reviews PASSed live-broken work last session. Two SERIAL lanes with a live gate between them (F6) keep one moving
  part per gate and give a fallback landing (F7).
- **Zero UI vs a full bank panel** -- middle path (5.4): REST + bindable pads now; a thin, disposable strip only if
  time allows.

STATUS: COMPLETE -- all nine critique findings dispositioned at the top (7 ACCEPT-with-fold, 2 NICE kept, 0 REJECT,
0 ESCALATE beyond two one-sentence Boris confirmations); final plan is self-contained: model + slice + compileRoutine
(lane 1a, fallback landing) -> live gate -> RoutineEngine with D9 gesture-begin arbitration, barCount parity, erase-safe
tick, REST/OSC/binding + 14 ctest cases + a RED-first live probe with a stacking row (lane 1b) -> optional strip;
fences and do-not-touch lists per lane; risks R1-R14; nine decided product calls and two open Boris questions.
