# s-rta-0925 -- Recorder roadmap: what is ACTUALLY next after step 4 (Architect, Fable)

Author: Architect, 2026-09-25. Read-only pass (no source edited, app not launched). Inputs: spec
`.harmony/specs/s167-performance-log-and-routines.md` (sections 0, 1, D4, D5, D8, D9, D10, D11, D12, D14,
5, 6, 7), `.harmony/HANDOFF.md` sessions s-rta-0923 / s-rta-0924 / s-rta-0924b (:2648-2841),
`.harmony/binding-decisions.md` (:318-485, incl. the two rulings dated TODAY), `git log` (main, HEAD e501b00),
and the sources cited per row. Labels: VERIFIED = read on disk this pass at the cited line; INFERRED = derived
from cited code; ASSUMED = stated so the Builder checks it. BINDING carried: no Bluetooth audio, ever.

---

## QUESTION

Which of spec section 5's steps 5-7 are done in fact, what is the state of the LATER lanes (L-E, L-R, L-P,
L-D), and what is the concrete next unbuilt step -- with a buildable plan.

## APPROACH (verdict first)

Steps 5 and 6 are DONE in fact; step 7 (docs) is NOT done. The next BUILD step is not on section 5 at all:
Boris ruled this morning that **replaying a whole take must first restore the look at the moment Record was
pressed** (`binding-decisions.md:478-481`), and that is VERIFIED unbuilt -- `Program::preamble` exists as an
empty field, nothing fills or fires it, and the live gate itself documents "Replay does NOT apply
checkpoint0". It is spec D4's PREAMBLE mechanism, which is also the first primitive routines (L-R, ruling 26
"a routine restores the state it was recorded in") need. So the order is:

1. **Step 7 docs** -- S, no build loop, exact rows in section 4 below (do it first or fold it into item 2's
   docs commit; it needs no critic).
2. **Replay restore (checkpoint 0 -> Program preamble -> fired at Play)** -- M-small, architect -> critic ->
   build -> review -> live gate. Full plan in section 3.
3. Then **L-R Routines slice 1** (Routine model + slice() + bank + trigger), all of whose product questions
   Boris has already answered (rulings 17, 20, 22, 26, 27). Not planned here.

---

## 1. STATUS TABLE -- section 5 steps 5-7 and the LATER lanes, with evidence

| # | Step (spec section 5) | Status | Evidence |
|---|---|---|---|
| 5 | REST `/api/perf/*` | **DONE** | Routes: `src/api/ApiServer.cpp:259-265` (POST record/stop/load/play/stop_play/repair, GET status); handlers `:1210-1356`, callAsync-marshalled, 503 "Recorder unavailable" when unwired; wired `src/MainComponent.cpp:2039-2048` into the perf* funnel `:5074-5239` that the Record panel shares (`:1604-1611`). Spec listed a `save` endpoint: not needed -- Stop saves (`RecorderHost.h:117-121`, `.cpp` disarm) and a periodic save runs every `kCheckpointSeconds = 60` (`RecorderHost.h:243`, `.cpp:472-487`); `repair` is extra (Ruling 28 crash recovery). The spec's verify text says "over 8080": wrong rig fact -- test mode never starts the analysis thread (`HANDOFF.md:2409-2412`); the gate runs on 7070 in production mode: `.harmony/probe-step3.sh` sections 5-11, **69 PASS / 0 FAIL** (`HANDOFF.md:2798`). VERIFIED |
| 6 | T2 on real hardware (mean offset reported; p95 jitter <= 15 ms; 10-min drift <= 1 ms) | **DONE on the rig's built-in CoreAudio device; the wired-interface variant is only-Boris** | Rows: `probe-step3.sh:552` (drift <= 1 ms + 2 sigma), `:557` (p95 <= 15 ms), `:560` (mean offset in [0,60] ms), `:563` (>= 90 % of clicks matched). Short take: `mean_offset_ms=33.22 drift_ms=-0.14 drift_stderr_ms=2.26 p95_jitter_ms=12.11` (`.harmony/.reports/s-rta-0924b/review-drift.md:18`, re-derived by the reviewer). Long take (`STEP3_LONG=1`, bffa2d9): 10 min 75/0; **20 min 82/0, slope drift +0.28 ms (stderr 0.46)** (`HANDOFF.md:2801`, `.harmony/s-rta-0924-work.md:132`). Not run: an external wired interface ("none on the rig", `HANDOFF.md:2808`; "T2 hardware sync run on his real interface" = only-Boris, `HANDOFF.md:2699`). Bluetooth is banned (`binding-decisions.md:469-476`), so no other device variant exists. T2 as specified (file-mode click track, D10.3 `:606-612`) is what ran; the clock under test is the device callback's delivered-sample counter in both modes. VERIFIED |
| 7 | Docs: APP-INVENTORY rows; the spec's `features` list; the D7 note cross-referenced in the connection spec | **NOT DONE** | `.harmony/APP-INVENTORY.md:85` still says Record panel "Play fires nothing -- playback dead"; `:135-165` REST table lists 22 endpoints, none of `/api/perf/*`; `:21-32` counts row says "22 REST endpoints", "188 unit tests" (ctest is 445, `HANDOFF.md:2841`); `:223-226` and `:267` describe `SessionRecorder` (deleted in 3736f02, s168) as PARTIAL/DEAD; no row anywhere for AudioTap / AudioStore / RecorderHost / Take / Player / the real Record panel / the three live gates. `CLAUDE.md:256` source tree still lists `SessionRecorder.h/cpp` and none of the 22 files in `src/recording/`; `CLAUDE.md:14` key-capabilities line has no performance recorder (only the "Step 3 LIVE" paragraph at `:1145` exists). Features list: code writes `lanes, tempoMap, checkpoint0` + `audio` (if mode set) + `markers` (if any) and knows `wallOnly` (`src/recording/Take.cpp:145,159-165`), `kFormatVersion = 3` (`Take.h:95`); spec D12 (`:672-676`) still shows v2 without `wallOnly`. D7 cross-ref: `grep 'AutomationCurve\|s167' .harmony/specs/s166-universal-connection-architecture.md` -> 0 hits, although the CODE side of D7 is done (`src/connect/AutomationCurve.h`; `src/connect/ParamConnection.h:52-63` Envelope holds an `AutomationCurve` with the "architect ruling, s167-l2" comment). `.harmony/VALIDATION.md:52` has only the ctest row -- no rows for probe-step3 / probe-onset-render / probe-finalize-loop / STEP3_LONG. VERIFIED |

**Boris ruling 2026-09-25 (open call 1) -- "replay snaps back first": VERIFIED UNBUILT.**
`src/recording/Program.h:82` declares `std::vector<Fired> preamble;` but `compile()` (`Program.cpp:152-275`)
never writes it; `Player` (`Player.cpp`, whole file) has no preamble path; `RecorderHost::play()` is compile +
`player_->start(0.0)` (`RecorderHost.cpp:640-662`); `checkpoint0` is only ever CAPTURED (`RecorderHost.cpp:242-243`,
`:330`), `PerfState` has `toVar/fromVar` only (`PerfState.h`), no apply; the live gate says so in its own
comment: "Replay does NOT apply checkpoint0 (spec s167 D4/D14), so the first poll after play always shows the
pre-play leftover state" (`.harmony/probe-step3.sh:568-571`). Open call 2 ("Record Over") needs no change
(`src/ui/RecordPanel.cpp`, `RecordPanelModel.h:123-124`).

**LATER lanes (spec section 5 "in order"):**

| Lane | State | Evidence |
|---|---|---|
| L-E Editing | unbuilt except the two NOW edit ops | `Take::deletePoints/deleteLane` exist (`src/recording/Take.h:125-126`); `Player::swap` exists (`Player.cpp:154-200`, unit-tested) but has NO caller in `RecorderHost.cpp`/`MainComponent.cpp` (grep 0 hits) -> no hot-swap in the app; `Player::setOverride(Latch)` refuses (`Player.cpp:202-211`); no `replaceSpan/movePoints/...`; no lane editor (ruling 23: draw-don't-drag; ruling 16: per-knob openable lane). VERIFIED. Blocked on the UI rewrite for the editor; the edit OPS + hot-swap are engine work and could go first. |
| L-R Routines | unbuilt; product calls ALL answered | no `src/model/Routine.h` (ls: no such file); `Composition.h` has no `routines` (grep 0); `Program.h:21-24` `Range` is a filter with "no preamble synthesis"; `resolveKey` reports `Scope::Routine` unresolved (`Program.cpp:101-102`). Rulings: 17 (recorded layer), 20 (own bank), 22 (loop/once as controls), 26 (restore = default), 27 (next bar). VERIFIED. **This plan's preamble is L-R's first primitive** (D4 step 2, D9 "loop restarts re-fire the preamble"). |
| L-P Capture completion | still open; the dependency (connection L3) has landed but widgets do not reach `manualWrite` | Lane 3 shipped `src/connect/ManualWrite.{h,cpp}` and 11 OSC/MIDI/REST writer sites (`HANDOFF.md:2663`); the 21 inspector widgets grip the model DIRECTLY via `UniversalParamControl::bindConnection(ParamConnection*, LiveValue*)` and LayerStrip's opacity slider via `gripHeld()` "no ControlPath" (`.harmony/specs/s-rta-0923-lane3-plan.md:48,320`; `src/ui/LayerStrip.cpp:430`); `grep manualWrite src/ui` -> only that comment. So mouse moves on the inspector/LayerStrip are NOT in a take; MIDI/OSC/REST are (D14 as written). No `"conn"` lane anywhere (grep 0). VERIFIED |
| L-D Determinism + render | unbuilt; one prerequisite fell out of an earlier fix | `Origin::Engine` is captured only for the genre auto deck-switch (`MainComponent.cpp:765-767`); the four hard-coded `1/60` compositor sites are GONE (`grep '1.0f / 60' src/render/CompositorEngine.cpp` -> 0; the fps decoupling landed in 02b89a1, `HANDOFF.md:2385-2387`) but no steppable `frameDt_`/offline pump/render driver exists (grep 0); no FLAC (`grep FlacAudioFormat src/` -> 0). Ruling 24: the log is the product, video stays LATER. VERIFIED (the autopilot-advance capture status was not re-checked this pass -- ASSUMED still GL-thread-only per G13). |

---

## 2. TRADEOFFS CONSIDERED (which step is next)

- **Step 7 docs first, snap-back deferred to L-R** -- rejected as the ONLY next step: Boris ruled today and
  the binding-decisions entry says "if not built, it is a build item"; it is M-small; and it is the L-R
  primitive anyway, so building it now is on-roadmap, not a detour. Docs (S) run alongside.
- **Implement snap-back as a `PerfState::applyTo(Composition&)` that writes model fields directly from
  `perfPlay`** -- rejected: it bypasses every choke point (no renderer re-point / preview load / deckView
  refresh, `MainComponent.cpp:4131-4246`), bypasses the D8 grip chain (a knob a human is holding would be
  overwritten -- `ManualWrite.cpp:160-176`), creates a second dispatcher to keep in step with
  `dispatch.fire`, and routines could not reuse it. The preamble-through-`compile`-and-`Player` shape is what
  D4/D5/D9 specify (`spec:263-274, 288-291, 486-493`).
- **Fire the preamble on the first tick instead of inside `play()`** -- rejected: `play()` is synchronous on
  the message thread and testable through `FakeDispatch` without a tick; firing at Play also guarantees the
  restore precedes the audio transport start that `perfPlay` issues right after (`MainComponent.cpp:5177-5188`).
- **Restore tempo and audio transport from checkpoint 0** -- rejected (disclosed below): `PerfState` carries
  `bpm` but not whether the tracker was in manual mode, so a faithful restore is impossible; WithAudio replay
  re-hears the audio and re-locks; `audio` points are already skipped WithAudio (`RecorderHost.cpp:61-67`).
- **L-R slice 1 as the next build instead** -- rejected for ordering only: it needs the preamble first.

---

## 3. DECISION / SPEC -- "Replay restore": checkpoint 0 becomes the Program preamble, fired at Play

### 3.1 Behaviour (what Boris sees)

Pressing Play (panel or `/api/perf/play`, either mode) first puts the composition back the way it was when
Record was pressed -- active deck, quantize setting, every layer's flags, opacity, layer-effect manual
values, active clip, and each captured clip's effect values, scalars and play/pause -- then plays the moves.
Same look every time (ruling text `binding-decisions.md:479-480`). A control a human is holding at that
instant is NOT overwritten (D8 chain); such refusals and any layer/deck that no longer exists are COUNTED in
status and said in one plain notice line, never silent.

Not restored, by design (D4 `spec:259-261`): video playheads, crossfade progress, pending quantized triggers
(engine state); tempo and audio transport (see section 2); per-slot effect bypass and layer transform
scalars (not in `PerfState` today -- `PerfState.h:41-57`; additive fields later, D12 "ADD, never REDEFINE").

### 3.2 Model / compile (`src/recording/Program.h`, `Program.cpp`, `PerfState.h`, `PerfStateCapture.cpp`)

`Program.h` (additive):
```cpp
// Continuous restore entry (D4 preamble for a continuous control): fired as touch("held") -> set(v) ->
// release at Player::firePreamble, never by advanceTo. v is NORMALISED [0,1] (what Sink::set takes).
struct PreambleSet { ControlPath key; ResolvedTarget target; float v = 0.0f; };

struct CompileReport { /* existing */ std::vector<Issue> preambleUnresolved;  int preambleCount = 0; };
struct Program       { /* existing preamble */ std::vector<PreambleSet> preambleContinuous; };
```
`PerfState.h` (additive, header-only): move the private key encoding out of `PerfStateCapture.cpp:33-50`
into ONE source of truth used by capture AND compile:
```cpp
static constexpr int kFxParamKeyStride = 100;
static int fxParamKey(int slot, int param) { return slot * kFxParamKeyStride + param; }
static int fxParamSlot(int key)            { return key / kFxParamKeyStride; }
static int fxParamIndex(int key)           { return key % kFxParamKeyStride; }
```
(`PerfStateCapture.cpp:46` then calls `PerfState::fxParamKey(slotIdx, p)`; behaviour identical.)

`Program.cpp`: a new anonymous-namespace `buildPreamble(const PerfState& cp0, const Composition& comp,
Program& out)` called at the top of `compile()` (before the lane loop, after `program->clock` is set; a
`range` does not suppress it -- slicing-derived preambles are L-R, D4 step 2). It reuses `resolveDeck /
resolveLayer / resolveCol` (`Program.cpp:23-73`) by building a synthetic `ControlPath` per level (index from
the `PerfState` map key, name from `DeckRuntime::deck` / `LayerRuntime::layer` / `ClipRuntime::clip`), so the
D2 tri-state policy applies to the preamble exactly as to lanes: ExactMatch/PositionOnly/NameOnly -> emitted
(rebinds pushed to the existing `reboundByPosition/reboundByName` buckets with reason prefixed
`"preamble: "`), Missing -> one `Issue` in `preambleUnresolved` per deck/layer/clip, entries for it dropped
but counted. Every emitted `Fired` has `at = 0`, `seq = 0`, `p.s = {0,0,0}`, `p.origin = Origin::Preamble`
(`Lane.h:50`, already in the vocabulary), `p.v`/`p.action` per the table. `preambleCount` = discrete +
continuous entries emitted. Emission ORDER (load-bearing -- the trigger must precede the clip's
play/pause because `Layer::triggerClipImmediate` auto-plays a never-triggered clip, `Layer.h:271-274`):

| order | key | Fired / PreambleSet | source field |
|---|---|---|---|
| 1 | `comp / activeDeck` | Fired v = `activeDeckIndex` (skip + `preambleUnresolved` "deck index out of range" if not `0 <= i < decks.size()`; skip silently only when `cp0.decks` is empty AND `activeDeckIndex < 0`, i.e. a v1 take with no checkpoint) | `PerfState::activeDeckIndex` |
| 2 | `comp / quantize` | Fired v = `quantizeMode` | `PerfState::quantizeMode` |
| 3 | per deck (index order), per layer (index order): `layer / visible, bypass, solo, mute, autopilot` | 5 Fired, v = 0/1 (`bypassed`->"bypass", `muted`->"mute", `autopilotEnabled`->"autopilot" -- the control names `dispatch.fire` already accepts, `MainComponent.cpp:1971-1978`) | `LayerRuntime` flags |
| 4 | `layer / scalar:opacity` | PreambleSet v = `layerScalarDefs()[LayerScalar::Opacity].toNorm(opacity)` (`src/connect/ScalarParams.h:93-96`, identity today -- go through the def anyway) | `LayerRuntime::opacity` |
| 5 | `layer / fx:i / param:j` | PreambleSet per `(fxParamSlot(k), fxParamIndex(k)) -> v` after `resolveFx` bounds check against `layer.layerEffects` (no name in PerfState: index resolution only; out of range -> `preambleUnresolved`) | `LayerRuntime::effectParams` |
| 6 | `layer / activeClip` | Fired v = `activeClipColumn` (-1 = clear; `>= clips.size()` -> `preambleUnresolved`; an empty cell at that column is left to the handler, which clears -- `Layer.h:215-221`) | `LayerRuntime::activeClipColumn` |
| 7 | per captured clip (column order): `clip / fx:i / param:j`, then `clip / scalar:<key>` | PreambleSet v = stored value (clip scalars are ALREADY normalised, `PerfStateCapture.cpp:99-105`) | `ClipRuntime::effectParams`, `::scalars` |
| 8 | `clip / playing` for the ACTIVE column only | Fired action = `"resume"` if its `ClipRuntime.playing`, else `"pause"`; **if the active column has NO `ClipRuntime` at all, emit `"pause"`** -- a clip that was playing is always non-default (`PerfStateCapture.cpp:110-113`), so absence means it was paused, and without this entry step 6's trigger would auto-play it | `ClipRuntime::playing` |

Not emitted: `tempo`, `audio`, `previousClipColumn`, `crossfadeProgress`, `pendingTrigger*`,
`playheadPosition`, non-active clips' `playing`.

### 3.3 Player (`src/recording/Player.h`, `Player.cpp`)

```cpp
// D4/D9: fires every preamble entry exactly once per call, in Program order -- discrete via sink.fire,
// continuous via touch("held") -> set(v) -> release (release only when both were accepted; a refused
// touch/set leaves the human's grip alone, mirroring advanceTo's `displaced` rule). Never called by
// advanceTo/seek/swap; the owner calls it right after start(0). Returns the number of refused entries.
int firePreamble(Sink& sink);
```
`start()`/`advanceTo()`/`seek()`/`swap()`/`stop()` unchanged (the preamble is not in `discrete`, so
exactly-once and seek semantics are untouched; `Program::length` excludes it -- `Program.cpp:187,273`).

### 3.4 RecorderHost (`src/recording/RecorderHost.h`, `.cpp`)

- `play()` (`RecorderHost.cpp:661-662`): after `player_->start(0.0);` -> `preambleRefused_ =
  player_->firePreamble(*sink_); preambleFired_ = program_->report.preambleCount - preambleRefused_;`
  BEFORE `playing_ = true` (a preamble entry that re-enters the host must not look like a running replay).
  Reset both counters at the top of `play()` beside `skippedCount_` (`:649`).
- `HostSink::fire` (`:61-77`): a `Fired` with `p.origin == Origin::Preamble` that is refused must NOT
  increment `skippedCount_` (the probe pins `skipped == 1` for the arm-time audio point,
  `probe-step3.sh:~640`); it returns false and the Player counts it. The WithAudio `audio` skip cannot
  trigger (the preamble never emits `audio`).
- `Status` (additive, D12): `int preambleCount = 0, preambleFired = 0, preambleRefused = 0,
  preambleUnresolved = 0;` published in `publishStatus()` inside the `playing_ && player_` block
  (`:758-769`) from `program_->report` and the two counters; zero when not playing.
- `stopPlay()`: nothing new (every preamble grip was released at fire time).

### 3.5 MainComponent (`src/MainComponent.h`, `.cpp`)

1. `dispatch.fire` (`:1945-1997`): derive `const bool immediate = (f.p.origin == Origin::Preamble);` and
   keep passing `Origin::Replay` to every handler (so the existing capture gates `origin != Origin::Replay`
   at `:4267`, `:4969`, `:5359`, `:5381`, `:5415` and the undo gate `origin == Origin::Human` at `:4256`
   stay exactly as they are -- nothing from a preamble is ever recorded or undoable, R5 closed by
   construction). Pass `f.target.deck` to the four handlers that ignore it today (`:1976`, `:1982`,
   `:1988`, and `applyClearActiveClip` at `:1951`) -- this also fixes a latent replay bug: a lane point
   resolved to a NON-active deck is currently applied to whatever deck is active (VERIFIED: those four
   calls carry no deck while `handleClipTrigger` does).
2. `handleClipTrigger(int layerIndex, int column, Origin origin = Origin::Human, int deckIndex = -1,
   bool immediate = false)`: at `:4123-4125`, `if (immediate) { if (layer->getClipAt(column))
   layer->triggerClipImmediate(column); else layer->clearActiveClip(); } else { <existing forcedSnap path> }`
   -- the empty-cell branch mirrors `Layer::triggerClip` (`Layer.h:215-221`), because
   `triggerClipImmediate` alone would set `activeClipColumn` to an empty cell (`Layer.h:242-275`).
   Everything after (preview load, deckView refresh, undo/capture gates) is unchanged.
3. `applyClearActiveClip / applyLayerFlag / applyEffectBypass / applyClipPlaying` gain a trailing
   `int deckIndex = -1` (same B2 pattern as `handleClipTrigger`, `:4084-4101`): a private helper
   `Deck* MainComponent::deckForDispatch(int deckIndex, const char* who, Origin origin)` returns the active
   deck for `< 0`, the indexed deck otherwise, and issues the existing "deck unresolved" notice for a
   non-Human origin; each handler's capture key uses the RESOLVED deck index instead of
   `composition_.activeDeckIndex`; UI refreshes stay gated to the active deck as `handleClipTrigger` does
   (`:4135`). Existing callers pass nothing -> identical behaviour.
4. `perfPlay()` (`:5164-5191`): on success, one plain notice through `dispatch.notify` (F3 rule --
   whole words, no jargon): `"Playing <take>: the look from when Record was pressed is restored first."`;
   append `", but N settings could not be restored (a layer, deck or clip no longer exists)"` when
   `status().preambleUnresolved > 0`, and `", and N controls you are holding were left alone"` when
   `preambleRefused > 0`. `RecordPanel::setNotice` already shows the last line (`RecordPanel.h:45`).
5. `perfStatusVar()` (`:5241-5300`): add the four keys `preambleCount / preambleFired / preambleRefused /
   preambleUnresolved` (the probe reads them).

No change to `RecordPanelModel.h`, `ApiServer`, the take format, or the connection spec.

### 3.6 Threading (against the sacred rules)

Everything runs on the MESSAGE thread inside one call chain: `perfPlay` (callAsync from REST
`ApiServer.cpp:1298`, or the panel button) -> `RecorderHost::play` (asserts the message thread,
`RecorderHost.cpp:45-47,620`) -> `Player::firePreamble` -> `HostSink` -> `dispatch.fire` /
`dispatch.continuous.*` -> the same handlers a human click uses. Continuous restores go through
`manualTouch/manualWrite/manualRelease` (`MainComponent.cpp:3298-3321`, `jassert`ed message thread) at
`Hand::Lane` rank, so a human Held/Decaying grip refuses them (`ManualWrite.cpp:160-176`) and the recorder's
capture hooks ignore them (`:2023-2033`, `Origin::Replay`). No audio-thread code, no GL-thread code, no new
mutex, no allocation on any hot path; `Status` is published under the existing mutex by the existing
`publishStatus()`. The GL thread may render one frame mid-restore -- the same class of partial frame a human
column trigger produces today. Rules 1-4 (`CLAUDE.md` "Sacred Rules") are untouched.

### 3.7 Tests -- RED first, then green (Catch2, headless; idioms cited)

New target `tests/test_program_preamble.cpp` -- copy `test_program_stamps`' link set
(`tests/CMakeLists.txt`, the `add_executable(test_program_stamps ...)` block) plus
`${SRC_DIR}/recording/PerfStateCapture.cpp` (EffectLibrary is already linked there). Fixture: `Composition
comp; comp.initDefault();` ("Deck 1", layers "Layer 1..3", `tests/test_take.cpp:338`); the Builder checks
`Deck::initDefault` (`src/model/Deck.h:29-40`) for the column count and resizes `layer.clips` if needed; a
clip is `layer.clips[2] = Clip{}` with `name = "C"` and one `EffectSlot{effectName = "Ripple"}` sized from
`EffectLibrary::getEffectDef("Ripple")->params.size()` (display name -- `PerfStateCapture.cpp:39` looks the
def up by `slot.effectName`).

1. `[program][preamble]` "compile builds the preamble from checkpoint0 in restore order": mutate the live
   model (activeDeck 0, `quantizeMode = NextDownbeat`, layer 0: `activeClipColumn = 2`, `opacity = 0.5`,
   `visible = false`, `solo = true`; clip (0,2): `playing = true`, scalar opacity 0.25 via the model field
   + `clipScalarDefs()`, effect param 1 = default + 0.4 clamped), `take.checkpoint0 =
   capturePerfState(comp, 120.f, "")`, `auto p = compile(take, comp, DriveClock::Wall)`. REQUIRE the
   discrete order `[activeDeck 0, quantize 2, L0 visible 0, L0 bypass 0, L0 solo 1, L0 mute 0, L0 autopilot
   0, L0 activeClip 2, (0,2) playing "resume"]` (layers 1-2 emit their five flags + activeClip -1 in
   between -- assert the exact vector), every `Fired.p.origin == Origin::Preamble`, `at == 0`;
   `preambleContinuous == [L0 opacity 0.5, (0,2) fx0 param1 <v>, (0,2) scalar "opacity" 0.25]`;
   `report.preambleUnresolved.empty()`, `report.preambleCount == discrete + continuous`. **RED today:
   `p->preamble.empty()`.**
2. "an active clip with no ClipRuntime restores paused": layer 1 `activeClipColumn = 0` on a paused
   default clip -> a `(1,0) playing "pause"` entry exists. RED.
3. "a deck or layer that no longer exists is counted, never silently dropped": hand-build `checkpoint0`
   with deck index 3 named "Deck 9" and layer index 7 -> `preambleUnresolved.size() == 2`, both
   `Issue.reason` name the missing level; deck 0's entries still present. RED.
4. "rebind by name": layer index 5 named "Layer 2" -> entries carry `target.layer == 1`,
   `report.reboundByName` has one issue whose reason starts with `"preamble: "`. RED.
5. "the preamble never enters the discrete schedule": with one lane point at t = 0.5, `p->discrete.size()
   == 1` and `p->length == 0.5`. (Guards against folding the preamble into `maxAt`.) RED-by-construction
   only if the Builder gets it wrong; keep it.

`tests/test_take.cpp` (existing `[player]` cases, `:400-720` idiom -- a hand-built `Program` + a recording
`Sink`): 6. "Player::firePreamble fires discrete then continuous exactly once per call": preamble of 2
Fired + 1 PreambleSet -> sink log `[fire, fire, touch(held), set(v), release]`; `advanceTo(10.0)` fires
nothing more; a second `start(0)` + `firePreamble` re-fires (routine loop semantics, D9); a `Sink` whose
`set` returns false -> return value 1 and NO release for that key. RED (method does not exist).

`tests/test_recorder_host.cpp` (helpers `makeComposition / FakeDispatch / TempDir / layerKey`, `:35-140`;
replay idiom `:548-596`): 7. `[host][preamble]` "Play restores checkpoint 0 before the first lane point,
in both modes, and refusals are counted apart from skipped": take with `checkpoint0` = layer 0
`activeClipColumn 1`, `opacity 0.5`, plus one `activeClip` lane point v = 2 at t = 0.5 (Wall) / sample
24000 (Sample); `host.load`; `host.play(WallClock)` -> BEFORE any tick, `fake.fired` holds only entries with
`p.origin == Origin::Preamble` (last of them `activeClip v = 1`), `fake.touches/sets/releases` hold the
opacity 0.5 sequence, `status().preambleFired == preambleCount`, `preambleRefused == 0`, `skipped == 0`;
`tick(w0 + 0.6)` -> `fake.fired.back().p.v == 2`. SECTION WithAudio: same before the first
`transportFrames` tick. SECTION `fake.refuseFire = true`: before any tick `preambleRefused == <n
discrete>` and `skipped == 0`. RED.

Expected ctest: 445 -> 445 + 7 (Builder reports the real number; the count is run, never inherited).

### 3.8 Live gate (Harmony's file `.harmony/probe-step3.sh`, section 10, `:568-700`) -- fail-first pin

Before EACH `/api/perf/play` (WithAudio at `:604`, WallClock at `:667`), perturb the look, then require the
snap-back. `checkpoint0.decks` is an ARRAY of `{i, deck, layers:[{i, layer, activeClipColumn, opacity, ...}]}`
(`src/recording/PerfState.cpp:82-93,150,190`); layer 0's `activeClipColumn` at arm is -1 in this recipe
(nothing is triggered before section 5 and the field is runtime-only -- `Layer.h:168`, no loader writes it)
and its opacity is 1.0 (`.harmony/probe-step3.json`); read both from the take anyway, never hardcode:
```bash
CHK_COL="$(take_field "$TAKE_FOLDER" "next(l['activeClipColumn'] for dk in d['checkpoint0']['decks'] if dk['i']==0 for l in dk['layers'] if l['i']==0)")"; CHK_COL="$(normnum "$CHK_COL")"
CHK_OP="$(take_field "$TAKE_FOLDER" "next(l['opacity'] for dk in d['checkpoint0']['decks'] if dk['i']==0 for l in dk['layers'] if l['i']==0)")"
PERTURB_COL=3
curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d "{\"layer\":0,\"column\":$PERTURB_COL}" >/dev/null
curl -s --max-time 6 -X POST "$A/api/set_layer_opacity" -H 'Content-Type: application/json' -d '{"layer":0,"opacity":0.9}' >/dev/null
sleep 1   # > gripHoldMs: the REST write's Decaying grip must have expired, or the lane-rank restore is (correctly) refused
[ "$(comp_active_col)" = "$PERTURB_COL" ] && ok "snap-back: pre-play perturbation landed (col $PERTURB_COL, opacity 0.9)" || no "snap-back: perturbation did not land"
curl ... /api/perf/play ...
RESTORED=""; for i in $(seq 1 15); do CO="$(comp_col_op)"; c="${CO%%|*}"; o="${CO#*|}"
  if [ "$c" = "$CHK_COL" ] && awk -v x="$o" -v y="$CHK_OP" 'BEGIN{exit !(x-y<=0.05 && y-x<=0.05)}'; then RESTORED="$i"; break; fi; sleep 0.1; done
[ -n "$RESTORED" ] && ok "snap-back: layer 0 restored to checkpoint0 (col=$CHK_COL, opacity=$CHK_OP) within ${RESTORED}00 ms of Play" || no "snap-back: layer 0 NOT restored within 1.5 s (col=$c opacity=$o; expected $CHK_COL/$CHK_OP)"
PU="$(perf_field "d.get('preambleUnresolved','NA')")"; [ "$(normnum "$PU")" = "0" ] && ok "snap-back: preambleUnresolved == 0" || no "snap-back: preambleUnresolved == $PU"
PR="$(perf_field "d.get('preambleRefused','NA')")";   [ "$(normnum "$PR")" = "0" ] && ok "snap-back: preambleRefused == 0"   || no "snap-back: preambleRefused == $PR"
PF="$(perf_field "d.get('preambleFired','NA')")"; awk -v x="$PF" 'BEGIN{exit !(x+0>=8)}' && ok "snap-back: preambleFired >= 8 ($PF)" || no "snap-back: preambleFired == $PF (expected >= 8: deck, quantize, 5 flags, activeClip)"
```
Sequence rule change (`seq_matches_expected`, `:582-593`): the raw sequence may now start with the
perturbation and/or the restored value (`[3, -1, 0,1,2,3,2]` or `[-1, 0,1,2,3,2]`, depending on whether the
first poll beat the callAsync). Drop LEADING elements while `== PERTURB_COL` or (`== CHK_COL` and `CHK_COL !=
EXPECTED_SEQ[0]`); then compare to `0 1 2 3 2` as today. The F1 timing pin (`:614-618`, `:676-680`) keys on
the first change AFTER those dropped leaders. Today's build: the perturbation row passes, the restore row
FAILS (col stays 3 until the lane's first point) -- that is the fail-first evidence; Harmony runs the gate
once on the pre-fix build to see it RED. Expected after: 69 -> 69 + 10 rows PASS / 0 FAIL, `skipped == 1`
row unchanged, opacity rows (0.4/0.7/0.2) unchanged.

Gate mechanics unchanged and binding: scratch build dir / lane worktree with `-DFETCHCONTENT_SOURCE_DIR_*`,
never `./build`; run from the MAIN checkout (`.venv` numpy); `open -g`; window-only capture; `'MacOS/Audio-DN[A]'`
in every wait loop; graceful `osascript` quit; 0 Output windows; NO Bluetooth device anywhere in the chain
(built-in or wired only).

### 3.9 Docs for this step (same commit)

`CLAUDE.md` "Audio Store (Ruling 28)" step-3 paragraph (`:1145`): one sentence -- Play restores checkpoint 0
first (Boris 2026-09-25), what is and is not restored, the four status keys. `.harmony/APP-INVENTORY.md`
Record row + REST `/api/perf/status` row (section 4 below lands the rest). `binding-decisions.md:481`
"Implementation status: TO VERIFY" -> "BUILT <commit>".

### 3.10 Size and sequence

M-small: ~150 source lines (`Program.h/.cpp` ~90, `Player` ~25, `RecorderHost` ~30, `MainComponent` ~45
incl. the deck-aware handlers), ~260 test lines, ~45 probe lines. One builder lane in a worktree (RED
commits first: tests 1-7 failing to compile/assert), one independent review, one gate run (~4 min) plus one
fail-first run on the pre-fix build. Harmony: critic on this plan first (the deck-aware handler change is the
one item a critic may split into its own micro-lane -- if split, non-active decks restore `activeClip` only
and the rest is counted in `preambleUnresolved` with reason "non-active deck: not restorable yet").

### 3.11 What needs Boris

Nothing blocks: the ruling is given. Tier-4 feel checks once live: (1) press Play ten minutes into a set --
the snap is a hard cut for opacity/flags but a normal clip transition for clips (`Layer.h:265` sets
`crossfadeProgress = 0` -> the layer's own transition runs); is that right, or should the restore fade?
(2) Play now changes the look with no undo entry (replay points never push undo, `MainComponent.cpp:4253-4256`)
-- acceptable? Still open from s-rta-0924b: call 3 (stop by itself at the end and give the live input
back -- the natural NEXT micro-step after this one: `RecorderHost::tick` when `pos >= length` and the
transport has ended -> `stopPlay` + the `perfStopPlay` source-mode restore), call 4, call 5.

---

## 4. STEP 7 DOCS -- exact rows (S; no build loop; verify = grep the rows, run the counts)

1. `.harmony/APP-INVENTORY.md`
   - `:85` Record row -> "Record (`RecordPanel.cpp`, model `RecordPanelModel.h`) | Browser -> Record |
     Record / Record Over / Stop Recording / Load Take... / Play (with audio) / Stop Playback / Repair;
     name field; record-audio switch; notice line; 4 Hz refresh over `RecorderHost::Status` | yes
     (live-verified s-rta-0924b, 10 states over REST)".
   - `:135-165` REST table: "22 endpoints" -> "29"; add rows 23-29: POST `/api/perf/record` (arm: name,
     audio, audioFile, onsetMarkers, overdubAssetId), `/api/perf/stop`, `/api/perf/load` (folder),
     `/api/perf/play` (withAudio), `/api/perf/stop_play`, `/api/perf/repair`, GET `/api/perf/status`
     (fields per `MainComponent.cpp:5241-5300`); note the 503 when unwired and the callAsync marshal
     (`ApiServer.cpp:1197-1208`).
   - `:21-32` counts row: "22 REST endpoints" -> 29; "188 unit tests" -> the number `ctest` prints at
     close (445 at s-rta-0924b), "40 test targets" (`tests/CMakeLists.txt`, 40 `add_executable`).
   - `:223-226` Recording family: replace the `SessionRecorder` sentence with: performance take recorder
     (`src/recording/`: `AudioTap` second fan-out in `CombinedCallback`, `AudioStore`
     `~/Documents/Audio-DNA/Audio/<id>.adna-audio`, `RecorderClock`, `PerformanceRecorder`, `Take` v3 /
     `Lane` / `TempoMap` / `PerfState`, `Program` + `Player`, `RecorderHost`; takes at
     `~/Documents/Audio-DNA/Takes/<name>.adna-take/take.json`), gates: probe-step3 69/0, probe-onset-render
     13/0, probe-finalize-loop 40/0, STEP3_LONG 20 min 82/0 (drift +0.28 ms).
   - `:267` "Session playback DEAD" row -> "Performance recorder | LIVE (s168 core 3736f02; step 3 wiring
     s-rta-0924; Record panel s-rta-0924b) | src/recording/*, MainComponent.cpp:1945-2048, 5074-5300".
     `SessionRecorder` -> section 8 (removed s168, 3736f02).
2. `CLAUDE.md`
   - `:256` source tree: replace the `SessionRecorder.h/cpp` line with the `src/recording/` files
     (AudioTap, AudioStore, RecorderClock, PerformanceRecorder, Take/Lane/TempoMap/PerfState(+Capture),
     Program, Player, RecorderHost, VideoRecorder) and `src/model/ControlPath.h`,
     `src/connect/AutomationCurve.h` (shared with the connection Envelope, D7).
   - `:14` key-capabilities line: add "performance take recorder (per-control lanes + the audio the app
     heard, shared audio store, replay with audio, T2-proven timing)".
   - tests list (`:300-320` region): add the recorder test files (`test_take`, `test_audio_tap_sync`,
     `test_audio_store`, `test_recorder_host`, `test_program_stamps`, `test_take_v1_transport`,
     `test_recorder_double_touch`, `test_record_panel_model`, `test_onset_pulse`, `test_bt_device_shapes`,
     `test_httplib_bodyless_post`) and "40 targets / 445 tests".
3. `.harmony/specs/s167-performance-log-and-routines.md` D12 (`:672`): one note -- shipped envelope is
   `version 3 / minReader 3` (Ruling 28, `Take.h:95`); `features` written = `lanes, tempoMap, checkpoint0`
   (+ `audio`, + `markers` when present); `wallOnly` is a reader-side flag for v1 files (`Take.cpp:145,323`).
4. `.harmony/specs/s166-universal-connection-architecture.md`: addendum A5 after `:835` -- "D7 (s167):
   `ConnSource::Envelope::curve` IS `AutomationCurve` (`src/connect/AutomationCurve.h`,
   `ParamConnection.h:52-63`), the same struct/evaluator a recorded lane gesture stores (`Lane.h:131-135`);
   a lane is a WRITER through `manualWrite` (Hand::Lane), never a `ConnSource::Kind` -- see s167 D7/section 9."
5. `.harmony/VALIDATION.md` after `:52`: four gate rows (probe-step3 69/0, probe-onset-render 13/0,
   probe-finalize-loop 40/0, STEP3_LONG=1 STEP3_LONG_MINUTES=20 82/0) with their exact commands and the rig
   rules (main checkout `.venv`, scratch build dir).

Verify: `grep -c '/api/perf/' .harmony/APP-INVENTORY.md` >= 7; `grep -c SessionRecorder CLAUDE.md
.harmony/APP-INVENTORY.md` == only the "removed" mentions; `cd build && ctest` count matches the row.

---

## 5. RISKS

- **R1 The strongest counterargument to the recommendation**: "the preamble is capability-4 (routines)
  machinery; for whole-take replay a 40-line `applyPerfState` in `MainComponent` would do." It loses on
  three verified facts: direct writes skip the renderer re-point / preview load / deckView refresh that
  ONLY `handleClipTrigger` does (`:4131-4246`), so a restored clip would not show; they skip the D8 grip
  chain (`ManualWrite.cpp:160-176`) and would overwrite a knob under a hand; and ruling 26 needs the same
  restore fired by a `Player` for routines -- two dispatchers would drift.
- **R2 Human grip at Play** refuses continuous restores (by design) -- counted in `preambleRefused` and
  said in the notice; the probe's `sleep 1` after its REST perturbation is load-bearing (Decaying grip =
  `gripHoldMs` 250 ms, `Composition.h:82`).
- **R3 Deck-aware handlers** touch four capture sites; a wrong `resolvedDeckIndex` in a capture key would
  mis-file a lane. Reviewer greps every `layerScalarPath(composition_, ` / `clipScalarPath(composition_, `
  call in those four handlers for the resolved index, not `activeDeckIndex`.
- **R4 Restore order vs auto-play**: if the Builder emits `playing` before `activeClip`, a paused clip
  auto-plays (`Layer.h:271-274`). Test 1 pins the order; test 2 pins the "pause when absent" rule.
- **R5 Synchronous media loads at Play**: `handleClipTrigger` loads image/video/sequence content on the
  message thread per restored layer (`:4191-4233`) -- same cost as a column trigger; N layers -> N loads
  once. Acceptable; the notice arrives after.
- **R6 `quantize` restore** writes `composition_.quantizeMode` directly (`:1991-1994`, pre-existing path)
  with no TopBar refresh -- pre-existing for lane points; note, do not fix here.
- **R7 Determinism claim**: "looks the same every time" holds for model state; engine state (video
  playheads restart from `inPoint` on trigger -- deterministic; MilkDrop/camera/slideshow sources are not,
  D11 #5) is disclosed, not promised.
- **R8 Non-active decks**: without change 3.5(3) their flags/params cannot be restored; the plan restores
  them; if the critic splits that lane, the counted fallback applies (3.10).
- **R9 v1 / checkpoint-less takes**: `activeDeckIndex` default -1 and empty `decks` -> no entries, no
  issues -> behaves exactly as today (test 5's fixture covers the empty case if the Builder adds one
  SECTION -- recommended).
- **R10 Two-run race in the gate**: the first poll after `/api/perf/play` may land before the callAsync ran;
  the leading-drop rule in 3.8 handles both orders and the perturbation column (3) is chosen so the
  restored value (-1) is distinguishable; if `CHK_COL` ever equals the first lane value (0) the rule
  still holds because it never drops a leader equal to `EXPECTED_SEQ[0]`.

STATUS: COMPLETE -- steps 5 and 6 verified DONE (6 on built-in CoreAudio; wired-interface run only-Boris),
step 7 NOT done (exact rows above), next build = replay restore (checkpoint 0 -> Program preamble), M-small,
fully specified with RED-first tests and a fail-first live-gate pin; no Bluetooth anywhere.
