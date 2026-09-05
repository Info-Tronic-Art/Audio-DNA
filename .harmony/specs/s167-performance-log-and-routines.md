# Performance Take v2 — per-control lanes, audio-in-the-take, routines (Audio-DNA / RealTimeAudio)

Session s167 · Architect (Fable) · read-only pass over `/Users/boriskarpman/projects/RealTimeAudio`
at HEAD `d749b88` (tree clean except `.harmony/` docs and `graphify-out/`). **Revision 2 — folds in
Boris's amendment (per-control timelines, Ableton-style; audio captured with the log; three
multiplicative opacities).** Revision 1's flat-event-stream model is replaced in place, not
appended to. Every claim about existing behaviour is cited `path:line` and tagged VERIFIED (read
this session), INFERRED (follows from verified facts) or ASSUMED (stated so a builder can check
it). Everything under a "PROPOSED" heading does not exist yet. `MainComponent.cpp` line numbers
drift constantly in this repo (`.harmony/binding-decisions.md`, "re-grep by anchor text"); every
MainComponent cite carries its function name as the anchor.

Owner rulings this spec obeys (`.harmony/binding-decisions.md`, "2026-09-05 (s167)"): #2 replay +
offline render + editable + ROUTINES; #4 tempo keeps running from last/tapped BPM in silence;
#5 a cue does not survive a stop; #7 hand-back glides; #10 macro banks at global/layer/clip;
standing: everything tempo-locked, current UI will be scrapped, dead UI gets built. Plus today's
amendment, verbatim: "All of the different sliders and buttons used in a performance will be
logged on its own timeline. These timelines can be changed similar to how Ableton Live works."
/ "[the audio] gets recorded along with the log and we will test it to make sure it stays in
time." / opacity: master × layer × clip, "it won't get past 50%". Connection architecture:
`.harmony/specs/s166-universal-connection-architecture.md` (its ADDENDUM wins over its body).

---

## 0. What this gives you (Boris — no engineering words)

Today the app can write down *which clip you hit and when*. Nothing else, and the Play button does
nothing. This turns that notebook into a real **recording of a performance**: a "take".

A take has two halves that are always saved together:

- **Your hands.** Every slider and every button you used gets **its own timeline**, like a track
  in Ableton: the opacity of layer 2 is one timeline, the clip playing on layer 1 is another, the
  master fader another, tap-tempo another. Each timeline is stamped three ways at once — by the
  clock on the wall, by the beat of the music, and by the exact spot in the audio — so it can be
  moved, trimmed, nudged and redrawn on a bar grid, and replayed on the beat at any tempo.
- **The sound the app was listening to** — microphone, sound card, whatever it was — recorded
  as an audio file next to the timelines, sharing the same clock. The take is self-contained:
  no need to find the DJ's original track later.

Because the timelines are tiny and the audio is ordinary audio, one take can be used four ways:

1. **Replay** — the app performs the set again, live. Built first.
2. **Render later** — turn the take into a video overnight, at full quality, with its own audio
   in it, without recording video during the show.
3. **Fix it up** — while it plays or stopped: grab a knob and the recording steps aside until
   you let go; with Record armed, your move replaces that stretch of the timeline. Or edit the
   timelines directly.
4. **Routines** — grab any stretch ("bars 33 to 40") of any timelines, name it, and fire it
   later like a clip: on the beat, looping if you want, several at once, on any set. A routine
   is "your hands for eight bars", and it behaves like hands: if you grab a knob a routine is
   moving, you win until you let go; then it glides back.

Two rules make this safe: a take records what **you** did, never what the **music** did to a
knob — the music does its part again live or in the render; and a knob's own hand-drawn curve
("Timeline") and a recorded timeline are the **same kind of curve**, so anything recorded can be
printed onto a knob permanently with one click, and anything drawn can be recorded into a take.

Only replay (and the audio capture with its timing test) gets built now. The rest is shaped
into the file today so nothing gets thrown away later.

---

## 1. Ground truth (VERIFIED)

| # | Fact | Cite |
|---|---|---|
| G1 | `SessionRecorder::Event` is one flat struct with 7 numeric `EventType`s; save writes `"version": 1` and the enum as an **integer** `"type"` | `src/recording/SessionRecorder.h:26-57`; `SessionRecorder.cpp:173,211-213,238` |
| G2 | Playback hands out **raw pointers into `events_`** (`std::vector<const Event*> advancePlayback`) and auto-stops inside the same call; any mutation while playing dangles them | `SessionRecorder.h:80,94-100`; `SessionRecorder.cpp:120-140` |
| G3 | Timestamps are wall-clock seconds since record start (`getMillisecondCounterHiRes`) | `SessionRecorder.cpp:3-6,26` |
| G4 | Exactly ONE capture call exists in production: `recordClipTrigger` inside `MainComponent::handleClipTrigger`; the other six `record*` methods and `advancePlayback` have zero callers | grep over `src/`: `MainComponent.cpp:3815` (anchor `sessionRecorder_.recordClipTrigger`); `RecordPanel.cpp:38` calls `startPlayback()` with nothing driving it |
| G5 | `handleClipTrigger` early-outs SILENTLY on a bad deck/layer; so does `handleDeckSwitch` on a bad index | `MainComponent.cpp:3783-3787` (anchor `void MainComponent::handleClipTrigger`), `:4518-4519` (anchor `void MainComponent::handleDeckSwitch`) |
| G6 | Clip ids are **re-minted on every load** from a file-static counter; layer ids restart at 0..2 per deck and continue at 100+; deck ids 0 then 100+ | `src/core/CompositionLoad.h:61-72`; `MainComponent.cpp:12` (`s_nextClipId = 1000`), `:2668,:2810`; `src/model/Deck.h:34,198`; `Composition.h:145,421` |
| G7 | The whole app addresses triggers by **coordinate** (deck, layer index, column): REST `trigger_clip {layer,column}`, OSC `/audiodna/clip/…`, MIDI `ByPosition`, and undo's `TriggerClipCmd` re-resolves by coordinate and never stores a pointer | `src/api/ApiServer.cpp:320-341`; `src/osc/OscHandler.cpp:58`; `src/binding/Binding.h:68-77`; `src/core/TriggerCommands.h` header; `src/core/ClipCommands.h:16` (`ClipLayerResolver`) |
| G8 | Effect params are addressed by **name** at the REST boundary and by **index** in the model; the library def carries `name` + `uniformName` per param | `ApiServer.cpp:365-440` (`set_param`, lookup `:423-436`); `src/effects/EffectLibrary.h:16-30`; `src/model/Clip.h:48-56` |
| G9 | The analysis clock counts samples **popped from the ring buffer**: `snap.timestamp = totalSamplesProcessed_`; the analysis pipeline contains no RNG | `src/analysis/AnalysisThread.cpp:96,305-306`; `FeatureSnapshot.h:8-9`; grep `rand|random|std::mt|srand` over `src/analysis/*.cpp` → 0 hits |
| G10 | Beat state per hop: `bpm` (0 if unknown), `beatPhase` [0,1), `trackerState`, `beatInBar`, `barPhase`, `barCount` (uint16, **resets on phrase reset**) | `FeatureSnapshot.h:39-50`; `AnalysisThread.cpp:173-180` |
| G11 | User tempo interventions mutate the tracker directly at ≥5 sites: tap, manual on/off, resync, Ableton Link (30 Hz), bindings | `MainComponent.cpp:568-595` (anchors `onTapTempo`, `onResync`), `:1814-1815`, `:1864-1865`, `:3112-3125` (anchor `linkSync_.isEnabled()`), `:5825-5859` (anchor `case Binding::Action::TapTempo`); `src/analysis/BPMTracker.h:125-137` |
| G12 | The only always-alive cadence is the **120 Hz message-thread tick** `tickFeaturePipeline`; the UI timer is 30 Hz | `MainComponent.h:272` (`kMappingTickHz = 120`); `MainComponent.cpp:309,313,3056-3079,3081` |
| G13 | Autopilot triggers clips **on the GL thread** and picks random columns with `std::rand()`; it hops to the message thread only for UI refresh | `src/render/Renderer.cpp:245-254`; `src/model/Autopilot.cpp:241` |
| G14 | Beat-crossing detection precedent: `snap.beatPhase < last - 0.5f` | `Renderer.cpp` anchor `beatCrossing =` (~:299); `src/model/Autopilot.h:54-56` |
| G15 | Renderer time: `u_time` has a deterministic override, but video/sequence advance uses a **hard-coded `dt = 1/60`** at four compositor sites; wall clock read at `:142`, `:183` (FPS only), `:384` (overridable), `:1870` (snapshot name) | `Renderer.h:310-312`; `Renderer.cpp:381-384,131-133,1148`; `src/render/CompositorEngine.cpp:728,747,831,895,1077-1090`; `src/media/VideoPlayer.h:85,151` |
| G16 | Deterministic single-frame capture exists (blocking promise, 5 s timeout, `glReadPixels`), used by test-mode `render_frame` with locked resolution | `Renderer.cpp:1744-1784,1786-1830`; `src/test/TestServer.cpp:519-570`; `ApiServer.cpp:844-862` |
| G17 | A real FFmpeg video recorder exists (GL-thread `submitFrame`, encoder thread, H264/ProRes/MJPEG, **no audio**), started from a menu at 1080p30 | `src/recording/VideoRecorder.h`; `MainComponent.cpp:480,5232-5251` |
| G18 | Audio path: `AudioEngine::CombinedCallback` runs the device callback; **file mode** renders the transport into `outputChannelData`, feeds it to analysis, then silences the output; **mic mode** feeds the input channels (scaled into the output buffers when gain ≠ 1). The analysis thread polls the ring buffer with `sleep(1)` | `src/audio/AudioEngine.h:17-21,39-41,66-125`; `AnalysisThread.cpp:62-69` |
| G19 | Test-mode feature injection writes a whole `FeatureSnapshot` through the bus writer | `ApiServer.cpp:612-675`; `TestServer.cpp:416` |
| G20 | The grip/touch API and `src/connect/` **do not exist yet** (connection arc L0, L1, L5a, L8 landed; L2 not): `touchScalar|gripHeld|gripTouch|ParamConnection|LiveValue|ConnectionEngine` → 0 hits in `src/`; `ls src/connect` → no such directory | `git log` (`f53a8f1` L0, `e49a6ad` L1, `baa8b7d` L5a, `84383e0` L8); conn spec §2.3, §5 L2 |
| G21 | Existing "runtime state" notion: `LayerRuntimeSnapshot {activeClipColumn, previousClipColumn, crossfadeProgress, pendingTriggerColumn, pendingTriggerSnapOverride}` | `src/core/DeckCommands.h:185-192` |
| G22 | Composition JSON has **no version key**; loaders tolerate absent keys with `hasProperty` guards | `Composition.h:172-260,277-340` |
| G23 | Library folder convention: `~/Library/Application Support/AudioDNA/{Presets,FX Saves,Decks}` | `src/ui/PresetManager.cpp:445-470` |
| G24 | Headless Catch2 test pattern linking only `juce_core` | `tests/CMakeLists.txt:235-245` (`test_composition`) |
| G25 | `RecordPanel` keeps a shadow `recording_` bool, `refresh()` has no caller, "Video (Future)" combo item | `src/ui/RecordPanel.cpp` (whole file), `RecordPanel.h:35` |
| G26 | The PCM ring buffer is **single-producer single-consumer** (16384 floats ≈ 341 ms at 48 kHz); the producer **ignores `push`'s return value** → on overflow, samples are dropped from the analysis stream silently and the analysis sample counter (G9) undercounts delivered audio | `src/audio/RingBuffer.h:8-11,29-42`; `MainComponent.h:222`; `src/audio/AudioCallback.cpp:47` |
| G27 | `AudioCallback` **mono-downmixes** the "output" channels it is handed and pushes mono; no resampling anywhere; the analysis hard-codes **48 000 Hz** (`kSampleRate`) while the device rate is whatever the device runs (`getCurrentSampleRate`) — INFERRED: at 44.1 kHz the analysis clock, `wallClockSeconds` and BPM are 8.8 % off | `AudioCallback.cpp:18-47`; `AnalysisThread.h:45-47`; `AudioEngine.h:37` |
| G28 | Nothing reads the callback's timing context (`AudioIODeviceCallbackContext` is an ignored parameter everywhere) → no xrun/gap detection exists | grep `hostTimeNs|AudioIODeviceCallbackContext` over `src/` → only the ignored parameter |
| G29 | `juce_audio_formats` and `juce_audio_utils` are linked → `WavAudioFormat`, `FlacAudioFormat`, `AudioFormatWriter::ThreadedWriter` available; no audio writer is used anywhere today | `CMakeLists.txt:500-501`; grep `ThreadedWriter|WavAudioFormat|FlacAudioFormat` → 0 hits |
| G30 | Two envelope shapes exist/are specified: registry `EnvelopeSignal::ControlPoint {position, value}` + `CurveType {Linear, Exponential, SCurve}` (beat-cycled, phase offset, one-shot); and the conn spec's per-connection `ConnSource::Envelope { vector<pair<float,float>> points; Clock {Beats, ClipPosition}; cycleBeats }` with `loop`/playback transforms and "Timeline" as its picker name (owner D6/D8: BUILD) | `src/signal/EnvelopeSignal.h:12-31,36-70`; conn spec §2.1 (`struct Envelope`), §2.4, §7.2 D6 |

The July spec (`.harmony/specs/session-recorder-spec.md`) still describes the code exactly (its
§0). Its flood rule (§2.1), coalescing rule (§2.3), undo exclusion (§6) and risk register (§9)
carry forward where still true; its widget capture-site list (§2.2) and 30 Hz drive (§4.1) do not.

---

## 2. Decisions

### D1. TIMEBASE — three clocks on every point, one tempo map per take, ONE drive clock per use; the audio's sample counter is the origin

**Decision.** Every lane point carries all three stamps from one `RecorderClock` (PROPOSED,
message thread, fed by the 120 Hz tick — G12):

| field | type | source | drives |
|---|---|---|---|
| `t` | double, seconds since record start | `juce::Time::getMillisecondCounterHiRes()` (G3, kept) | **Replay** (1) |
| `beat` | double, continuous beats since record start | integrated beat counter (below) | **Routines** (4), the editor's bar grid (3) |
| `sample` | uint64, **delivered-audio sample counter** | the audio callback's producin counter (D10) — NOT `snap.timestamp` (G9/G26 undercount) | **Audio alignment** and **offline render** (2) |
| `bpm` | float | `snap.bpm` at capture | retime math, lint |

Plus a take-level **tempo map**: anchors `{t, beat, sample, bpm, why}` appended when the locked
BPM changes by > 0.05, on every tempo command (G11), on every audio-transport change and detected
audio gap (the sample clock stalls or jumps — D10), on tracker phase resets, and at least every
8 bars. Between anchors `beat(t)` is linear at the anchor's bpm; `sample(t)` is linear at the
device rate, constant across a stalled segment.

**Beat integration rule** (`RecorderClock::tick(snap)`): `phase = snap.beatPhase`; if
`phase < last − 0.5` → `wholeBeats += 1` (G14 precedent). A smaller backward jump (resync, tap,
relock — `BPMTracker.h:127-137` reset phase) is absorbed into an offset so `beat` stays monotonic
(`offset += last − phase`) and an anchor `why:"reset"` is written. If `snap.bpm == 0` (no lock,
nothing tapped) the beat clock does not advance and the anchor marks the segment **unmetered**.
Ruling #4 lives in the tracker; the clock follows the tracker.

**Why all three, not one canonical + derivation.** The relations are not invertible without the
tempo map, and the tempo map is the thing an editor can get wrong. Storing all three makes every
take self-describing and lets the loader LINT (`|beat − map.beatAt(t)| > 0.05` → counted and
reported, never silently "fixed"). Cost: 16 bytes per point. The tempo map is what EDIT
operations use to re-derive the other two after a move in one domain (D5). The editor's default
domain is `beat` (Ableton's arrangement grid is bars/beats); an unmetered segment falls back to
seconds — the editor says so.

**Rejected.** *Wall-only* (July §1): useless for "all will be timed" — a routine re-run at a
different tempo lands off-beat; beat-snap (`handleClipTrigger`, `MainComponent.cpp:3823-3844`)
rescues triggers, not knob gestures. *Musical only*: stalls while `bpm == 0`, jumps on resync,
and offline render needs sample alignment. *Samples only*: stalls while the transport is stopped.
*`barCount`*: resets on phrase reset (G10). *`snap.timestamp` as the sample clock*: it counts
popped samples and undercounts on ring overflow (G26) and lags by ring depth; the delivered
counter in the callback is the truth the audio file is written from.

**Replay drive clock.** Default `wall`; a `beat`-driven "musical replay" of a whole set is a
compile option once the field exists (LATER, free).

### D2. IDENTITY / ADDRESSING — `ControlPath` is the lane key: positional path + names, resolved at compile, failures reported

Now load-bearing for editing (a lane IS its control), not only replay.

**Decision.** Every lane is keyed by a `ControlPath` (PROPOSED, `src/model/ControlPath.h` —
shared by recorder, REST and, later, the connection test endpoints): a **positional path**
(what the app resolves by, G7/G8) **plus the names that were there** (what a human recognises
and what re-binding matches on), plus the **control** on that object:

```jsonc
"key": {
  "scope": "clip",                          // clip | layer | comp | macro | routine
  "deck":  { "i": 0, "name": "Deck 1" },     // "i": -1, "rel": true = active deck at fire time (routines default)
  "layer": { "i": 1, "id": 1, "name": "Layer 2" },
  "col":   { "i": 3, "clip": "loop_a.mov" },
  "fx":    { "i": 0, "name": "ripple" },     // slot in the owner's stack (duplicate names possible → index)
  "control": "param",                        // what on that object — see vocabulary
  "param":  { "i": 1, "key": "amplitude" }   // control=param: key = ParamDef::name (G8)
  //  control=scalar → "scalar": "opacity"  (ScalarDef::key, conn spec §2.2)
  //  control=macro  → "macro": { "scope": "global", "i": 2 }   (ruling #10: layer/clip scopes later, additive)
  //  control=dryWet | enable | bypass | activeClip | visible | solo | mute | autopilot | speed | cue
  //           | activeDeck | tempo | audio | quantize | routine | marker
}
```
Never keyed on `Clip::id` (G6). `layer.id` is written as a tie-breaker (stable within a saved
deck, `Deck.h:189-193`), never as the primary key.

**Control vocabulary** (which lane kind each is — D3): continuous: `param`, `dryWet`, `scalar`
(opacity, positionX/Y, scale, rotation, anchorX/Y per clip/layer/comp; D13), `macro`, `speed`.
Discrete: `activeClip` (per layer; value = column, −1 = cleared; `retrigger` flag), `activeDeck`
(comp), `enable`/`bypass` (per effect slot), `visible`/`solo`/`mute`/`autopilot`/`bypass` (per
layer), `cue` (per clip; value = cuepoint index), `tempo` (comp; action-valued: tap/manual/
resync/link), `audio` (comp; play/pause/stop/seek), `quantize` (comp), `routine` (per slot;
trigger/stop), `marker` (comp), `conn` (reserved: connection set/clear as a discrete lane).

**Resolution policy** (`Program::compile`, once per load/play, produces a report):
1. Positional path resolves AND names match → resolved.
2. Positional resolves, names differ → **set replay**: cell-follows-position (Resolume
   semantics, July risk #6), listed as `rebound-by-position`; **routine**: its binding table
   decides (D9), default position, listed.
3. Positional does NOT resolve → search by name within the enclosing scope (clip name within the
   deck; effect name within the stack; param key within the effect def via
   `EffectLibrary::getEffectDef`, G8). Found → `rebound-by-name`, listed. Not found → the **lane is
   UNRESOLVED**: compiled out, counted, listed with reason; status line and REST show
   `unresolved: N`; the future editor shows it on the lane header with a re-target control. The
   fire path re-resolves by coordinate at fire time (TriggerClipCmd pattern, G7); if the structure
   changed *since compile* the runtime `skipped` counter increments and is surfaced. **There is no
   path on which a lane silently does nothing** — G5 shows the raw handlers would.

**Rejected.** *Clip id* — G6. *Names only* — not unique, rename freely. *A new stable UUID per
clip* — right long-term, but requires the composition format and ≥5 clip-creating sites
(`MainComponent.cpp:1128,1257,1316,1518,4345`) to mint it; out of scope, and `col` gains a
`"uuid"` beside `"clip"` when it exists (additive).

### D3. LANES ARE THE MODEL AND THE ON-DISK FORM — continuous and discrete lanes, one structure, two evaluators

**Decision.** A take is `map<ControlPath, Lane>`, not a chronological list. A `Lane` is a
sorted vector of `Point`s; every point carries the three stamps, a global capture sequence
number `seq` (monotonic across the whole take, minted at capture; unique; the point's id), an
`origin`, and an optional `group`/`via`.

**Two kinds, one structure, two evaluators:**

| | Continuous lane (a slider) | Discrete lane (a button / trigger / command) |
|---|---|---|
| point payload | `v` float (normalised slider value), `g` gesture marker `begin|mid|end`, `grip` `held|decaying` on `begin` | `v` state value (int/bool) or `action` + `v` (tempo/audio/cue), flags (`retrigger`) |
| meaning of a run of points | a **gesture**: `begin` … `end`; between gestures the lane is **silent** — the control belongs to its normal owner (its signal, or rest) | each point is an **instantaneous command** that leaves the control in state `v` (state-valued, so the editor draws blocks: "clip 3 playing from bar 8 to 24") |
| evaluator | **sampled**: the player evaluates the curve every tick inside a gesture (`interp` = linear by default → a 20 pt/s recorded drag plays back smooth at 120 Hz, better than step-replaying the raw points) | **fired**: k-way merged with every other discrete lane by `(at, seq)` and dispatched once |
| editing | move/add/delete points, redraw a span, split/merge gestures, change `interp` | move/delete points, change value, insert |
| capture coalescing | ≤ 1 point per 50 ms per lane (July §2.3), `begin` and `end` values exact | never |

A **column trigger** is captured as one `activeClip` point per non-ignoring layer with a shared
`group` id (Deck::triggerColumn already skips `ignoreColumnTrigger` layers, `Deck.h:109-117`);
the editor moves a group together; replay fires the group within one tick through
`handleClipTrigger` per layer — same model outcome as `handleColumnTrigger`
(`MainComponent.cpp:3942-3993`), minus that handler's deck-view highlight (UI, being scrapped).
A **routine trigger** expands to points in the routine's lanes tagged `via` (D9). Autopilot's
choices (G13) land, LATER, as `origin:"engine"` points in the SAME `activeClip` lane — replay
follows them instead of re-rolling (D6).

**Why lanes on disk, not a flat log with a lane index.**
1. Every edit operation is lane-local (move a point, delete a span, redraw, retime a lane): with
   lanes as storage an edit touches one vector; with a flat log every edit invalidates the index
   and re-sorts the global list.
2. The presentation Boris asked for IS lanes; the on-disk form should be the mental model.
3. Cross-lane order — the only thing a flat log gives — is needed at PLAYBACK only, and the
   Program compile produces it by a k-way merge on `(at, seq)` (O(n log k), trivial at 10⁴
   points). Capture order is preserved exactly by `seq`; simultaneity by `group`.
4. Slicing is per-lane trivially (cut each lane at the range, D4).
5. The chronological "history" list remains available as a derived VIEW
   (`Take::chronological()`) for a list UI and debugging — never authoritative.
Cost: none in bytes; one k-way merge per compile.

**Rejected.** *Flat log + index* (rev 1): above. *Two lane classes*: the stamps, keys, editing
ops, slicing and serialization are identical; only the payload and evaluator differ — one
struct with a `kind` and a variant payload, two evaluators.

### D4. SLICING — per-lane cut; a routine = PREAMBLE + relative lanes; checkpoint 0 only

**State at any time is derivable from the lanes**: a discrete lane's state at `x` is its last
point ≤ `x`; a continuous lane's resting value at `x` is its last `end` ≤ `x` (or, inside a
gesture, the curve). What no lane can tell you is the value of a control the performance **never
touched** — so the take stores **checkpoint 0**: the full performance state at record start
(PROPOSED `PerfState`, G21 generalised — activeDeck, quantize, tempo, audio transport; per layer
runtime + flags + opacity + layer-effect manual values; per clip with non-default state: effect
manual values, scalars, playing/playhead; macro manual values (three scopes when ruling #10
lands, additive)). `params[]` are the MANUAL values (today `EffectSlot::paramValues`, Clip.h:51;
after connection L2 the manual field — the signal-driven twin regenerates itself, conn spec §3.1).
A second checkpoint at stop is written for lint only. Periodic checkpoints (rev 1) are dropped:
per-lane last-value lookup is O(log n) and needs no engine. Video playheads and crossfade state
are engine state and are not restored on a scrub — the same approximation every DAW makes.

**Slice** (LATER; format reserved now): `slice(take, beatFrom, beatTo, laneFilter)`:
1. For each selected lane, cut at the range; `x -= beatFrom`; `t`/`sample` are **dropped** — a
   routine is beat-native and carries no tempo map.
2. **Preamble** = the state at `beatFrom` of every selected lane's control (last value before the
   cut, or checkpoint 0), plus the `activeClip` state of every layer any selected `activeClip`
   lane touches, emitted as points at `x = 0` with `origin:"preamble"` — "make the world look the
   way it did for the things I am about to touch".
3. A continuous gesture straddling the cut gets a synthesised `begin` at the first in-range
   point; one straddling the end gets a synthesised `end` at `lengthBeats`.
4. A region whose tempo-map segment is unmetered cannot become a beat-native routine: the
   slicer refuses with a message.
5. `lengthBeats` = ceil to whole bars by default (owner call, §7).

**Rejected.** *Purely relative* (no preamble): a routine whose first move is "opacity 0.7 →
0.2" would jump from wherever the layer is now — sometimes wanted, so the preamble is a
per-routine switch (D9), default ON. *Full snapshot on trigger*: a scene, not a routine; trashes
layers the routine never touches. *Periodic checkpoints*: unnecessary once lanes are
state-valued.

### D5. EDITABILITY — TAKE (document) vs PROGRAM (immutable); playback never sees the vector being edited

**Decision.** Three objects (PROPOSED, `src/recording/`):
- `Take` — the **document**: lanes (D3), `tempoMap`, `checkpoint0`, `audio` (D10), `meta`,
  `nextSeq`. A value type; edit-undo is a before/after snapshot command in the existing shape
  (`EffectStackCmd`, conn spec §4.1 row 8).
- `Program` — the **compiled, immutable schedule** for ONE drive clock: `discrete` (k-way merged
  `Fired{at, seq, key, payload, ResolvedTarget}`), `continuous` (per lane: resolved target,
  `gestures[] {xBegin, xEnd, curve}`), `preamble`, `length`, `loop`, `CompileReport` (D2). Held
  as `std::shared_ptr<const Program>`.
- `Player` — owns a `shared_ptr<const Program>`, a position, one cursor per discrete stream and
  per continuous lane; `advanceTo(pos, Sink&)` fires due discrete points and, for every
  continuous lane, emits `touch` on entering a gesture, `set(curve(pos))` every tick inside it,
  `release` on leaving — all **synchronously while the program is pinned**; nothing outlives the
  call. Each running routine is its own `Player` (polyphony, D9). `stop()` releases every touch
  the player opened (a stopped routine lets go of its hands — conn spec §8 risk).
- Edit while playing = compile a new program and `Player::swap(program)` at the next tick: every
  cursor is re-seated at `pos`; a lane that was mid-gesture in the old program and has no gesture
  at `pos` in the new one is released immediately.

This retires G2 by construction: `advancePlayback` and its raw pointers are deleted. Edit
operations on the document (LATER except `deleteLane`/`deletePoints`): `movePoints(seqs, Δ,
domain)`, `retimeLane`, `setValue`, `deleteSpan(lane, x0, x1)`, `redrawSpan(lane, curve)`,
`splitGesture`/`mergeGestures`, `replaceSpan` (the overwrite primitive, D8), `slice`,
`insertMarker`. A move in one domain re-derives the other two via the tempo map (D1). Ties break
by `seq`. **Overdub** is native: replayed points carry `origin:"replay"` and are not re-recorded
(D6); with Record armed the human's gesture becomes a `replaceSpan` on that lane (D8).

**Rejected.** *Guarding mutation with state flags + UI gating* (July §5): fragile forever.
*Editing the live vectors under a lock*: playback observes half-applied edits. *Per-lane
recompile on edit*: an optimisation with no need at this size; full recompile + swap is simpler
and already correct.

### D6. PARAMETER MOVES — hook the manual-WRITE funnel once; record the hand, never the signal

**Verdict on the original premise: CONFIRMED, with two precisions.**

(a) The hook sits in the **manual-write funnel**, not in `gripHeld/gripTouch/release` alone: a
grip call carries no value (conn spec §2.3) and the recorder needs `(key, value, gripKind)`. The
connection spec already centralises this — "one `touchScalar(Clip&, ClipScalar, float model,
GripKind)` per struct, so a writer cannot forget the grip" (§2.3) — and lists ≥9 writer sites.
G20: that helper does not exist yet; the NOW build introduces `manualWrite(const ControlPath&,
float value, GripKind, Origin) → bool` (PROPOSED, `MainComponent`) at the sites the connection
arc's L3(e) must touch anyway (REST `set_param` `ApiServer.cpp:433`, `set_layer_opacity` `:499`,
OSC `onSetMaster`/`onSetLayerOpacity`, MIDI `MasterOpacity`/`AdjustLayerOpacity`/`AdjustMacro`,
the two velocity→`clipOpacity` writes, `LayerStrip.cpp:423`); L2's builder puts the grip call
inside it. Inspector widget sites are NOT hooked — they are being scrapped; the rewritten UI
calls `manualWrite` by construction. The return value (`false` = refused because a more
deliberate hand holds the control) is what D8 builds on.

(b) The grip API covers **sliders only** (Ruling B.1). Triggers, toggles, transport, cuepoints,
deck switches and tempo commands go through their own (mostly existing) choke points:
`handleClipTrigger`/`handleColumnTrigger`/`handleDeckSwitch` gain an `Origin` (and `deck`)
parameter; tempo (G11) and audio transport are funnelled through NEW `applyTempoCommand(TempoCmd,
Origin)` and `applyAudioTransport(cmd, Origin)`; toggles through `applyToggle(ControlPath, bool,
Origin)` at the binding and REST paths now, widget paths at the rewrite.

**The flood rule (load-bearing; July §2.1 restated for the new engine):**

> A continuous lane receives only writes that pass through `manualWrite`. The connection engine
> publishes to `LiveValue` twins (conn spec §3.1) and never calls `manualWrite`; therefore a
> signal-owned parameter is **never** recorded as automation — by construction, not convention.

What IS recorded for a connected parameter: the *hand*, as a gesture — `begin{grip}` → points
(coalesced) → `end`. On replay the same gesture re-enters `manualWrite` with `origin:"replay"`:
begin → `gripHeld()`, points → manual field, end → hand-back **glide** (ruling #7; conn spec A1,
120 ms). A `Decaying` grip (MIDI/OSC/HTTP, no release event) gets an explicit `end` when the
engine expires it (`gripHoldMs`, default 250 ms) so gestures always have boundaries; until L2
exists the recorder synthesises that `end` itself after 250 ms of silence on that lane.
Connecting/disconnecting a signal *during* a performance is itself a move: the `conn` discrete
lane (reserved now, captured when L3 lands) — what lets a routine say "at bar 3, connect bass to
scale".

**Non-human writers that DO get recorded** (LATER, `origin:"engine"`): autopilot's random column
choice (G13), `randomizeAllEffects`, `RandomBag` playlist picks — into the same lanes, recorded
from the message-thread hop the renderer already makes (`Renderer.cpp:249-252`), never from the
GL thread. Needed for D11's determinism.

**Origin rule.** `Origin {human, replay, routine, engine, preamble}`. The recorder records
`human` and `engine`. `replay` is never recorded. `routine`: the `routine` lane records the
trigger AND the expanded child points are recorded tagged `via`; at compile, if the routine
resolves the children are dropped and the routine scheduled; if missing, the children play and
the report says `routine X missing — inlined points used`.

**Rejected.** *Hook `Effect::setParamValue` / `onParamChanged` / `EffectStackView::refresh`* —
multiplex automated writes at frame/refresh rate (July §2.1; true until L3). *Record the twin's
published value at a low rate* — reproduces the signal badly and doubles as a flood.

### D7. IS A RECORDED LANE THE SAME OBJECT AS THE KNOB'S "TIMELINE" CONNECTION SOURCE? — YES for the CURVE, NO for the SOURCE. What Lane 2 must change now.

**Verdict.** The unification **holds at the data level and must land in Lane 2**: the
hand-drawable "Timeline" attachable to any knob (conn spec §2.1 `ConnSource::Envelope`, owner D6
"build") and a recorded continuous lane are **the same curve type** — one drawn, one captured —
and must share one struct, one editor and one evaluator. It **does not hold at the ownership
level**: a recorded lane is not a third `ConnSource::Kind`, and a playing take does not become the
parameter's connection. Four concrete reasons, each a property the connection type has that a
lane cannot have:

1. **Exclusivity.** "Exactly one `ParamConnection` per parameter — a field, not a list; enforced by
   the type" (conn spec §2.6). A parameter can be driven by the set replay AND two routines AND a
   hand in the same minute; two of them may target it at once (D9). Making the lane the
   connection would force a routine trigger to *replace* the knob's existing bass→scale
   connection and restore it after — "the routine unplugs the bass while it runs" — which is the
   opposite of what was recorded (a hand on top of the bass).
2. **Gaps.** A lane is silent between gestures — the control belongs to its normal owner. An
   envelope is a total function over its cycle (`EnvelopeSignal.h:36-70`; conn spec §2.4). "No
   value here" is a HAND concept (release → hand-back glide), not a source concept.
3. **Clock and trigger.** An envelope cycles on the beat grid or the owning clip's playhead and
   restarts on **clip** trigger (`Clock {Beats, ClipPosition}`, `loop=false` "set on clip trigger",
   §2.4). A routine lane runs on the **routine's** timeline from the **routine** trigger, quantized
   as a routine, stopped by Stop, and spans many parameters that must start and stop together —
   that identity has to live outside any single parameter anyway (D9).
4. **Half the lanes are not connectable at all.** Buttons, triggers, deck switches, tempo commands
   are discrete lanes, and toggles/dropdowns are out of connection scope by Ruling B.1. A model
   that makes "lane = connection source" covers sliders only and needs a second model for the
   rest; the lane model covers everything with one structure.

Ableton has ONE owner concept (automation owns the parameter; a touch overrides; re-enable
returns). This app already has TWO — a **permanent owner** (the connection: signal, LFO, or the
drawn Timeline) and a **transient owner** (a hand, via the grip). A recorded lane is a recorded
hand. The Ableton "automation owns it" feeling is reached by **printing** a lane onto the knob's
Timeline connection — one click, same curve, now permanent and saved with the clip.

**What Lane 2 must do NOW (small, additive, before `ConnSource::Envelope` is written in stone):**
- L2-a. Introduce `src/model/AutomationCurve.h` (PROPOSED; no JUCE UI, no renderer includes; usable
  from `src/connect/` and `src/recording/`):
  ```cpp
  struct Breakpoint { double x; float y; enum class Interp : uint8_t { Linear, Hold, Smooth } interp = Interp::Linear; };
  struct AutomationCurve { std::vector<Breakpoint> pts;   // sorted by x, x unique
                           float eval(double x) const;    // Hold = step; Smooth = the existing SCurve math (EnvelopeSignal.h CurveType)
                           double xMin() const; double xMax() const; };
  ```
  `ConnSource::Envelope::points` becomes `AutomationCurve curve` with `x ∈ [0,1]` (its cycle
  domain — semantics, `playbackXform`, `loop`, `Clock` all unchanged). Cost to L2: a type rename
  and one enum; zero behavioural change; the `Interp::Hold` gives the drawn Timeline a step mode
  it needs for strobe-like curves anyway.
- L2-b. Do NOT add a `Kind::Lane` / `Kind::Performance` to `ConnSource`, and do NOT add a
  performance clock to `Envelope::Clock` — that would couple composition state to a transient
  player.
- L2-c. Keep `Envelope` a value type serialized inside the clip (already the design) so the two
  conversions below are plain curve copies.
- The registry `EnvelopeSignal` (G30) is unaffected now; when the conn arc retires it into
  `ConnSource::Envelope` (its stated direction), it converts to the same curve.

**The two conversions (LATER, cheap once the type is shared):** `printLane(lane, gesture) →
Envelope{curve = gesture.curve rescaled to [0,1], cycleBeats = gesture length, loop = false or true,
Clock::Beats}` — "make this permanent on that knob"; and `bakeEnvelope(conn, beats) → gesture` —
"record what the Timeline would do into the take". The editor for both is the same component.

**What this changes in this spec:** a continuous lane's gesture stores an `AutomationCurve`
(`x` = beat offset within the take, `y` = normalised value, `interp` per point) rather than a
bespoke point list — D3's "run of points" IS that curve, with the gesture markers as its bounds.

### D8. OVERRIDE SEMANTICS DURING PLAYBACK — one ownership chain; Touch by default; Overwrite only when Record is armed

"You can change it while it's playing." A lane is playing and the human grabs that control.

**The chain** (extends the connection spec's grip rule with one more transient owner, in the
same order of deliberateness): **human Held grip > human Decaying grip (MIDI/OSC/HTTP) > playing
lane gesture (replay or routine) > permanent owner (connection) > rest (manual field)**. A
`manualWrite` from a less deliberate source than the current holder returns `false` (D6a); a
more deliberate one displaces the holder for the rest of *that* gesture.

**Behaviours that exist:**

| mode | when | while you hold | when you let go |
|---|---|---|---|
| **TOUCH** (default, not armed) | plain playback | you own it (grip); the lane's writes are refused; the lane keeps advancing silently | ownership returns to whatever is active: the lane's gesture if still running → **glide** to its current value over `handBackGlideMs` (ruling #7); else the connection (glide); else rest at your value. Exactly the signal rule with "lane gesture" inserted in the chain |
| **LATCH** (option, not armed) | plain playback | same | your value **holds**; the lane is suspended on that control until its **next gesture begins** (the natural re-enable point) or you press "re-enable" (global / per-lane). On a connected control the connection resumes after your release regardless — Latch only defers the lane |
| **OVERWRITE** (Record armed) | overdub | your gesture is **recorded into the lane** and the lane's data in `[touch, release]` is replaced (`replaceSpan`, D5) | the old lane resumes at its next point; the recorder inserts a breakpoint at your release value so the join is a ramp, not a jump; recompile + swap |
| **NEW TAKE** (option, armed) | overdub | as Overwrite, but into a new **take version** of that lane (`lane.takes[]`, format reserved; LATER) | pick the active take per lane in the editor |

Two humans (slider + MIDI) on one control: the existing rule (conn spec §2.3 iii) — a Held grip
is not displaced by a Decaying one; a Decaying one is displaced by Held. Two lanes (two routines,
or routine + set replay) on one control: the later `begin` wins for the rest of that gesture
(D9). All of it is one comparison on a small enum in the grip holder; no per-tick flicker by
construction.

**Recommended defaults:** TOUCH when not armed; OVERWRITE (touch-scoped) when armed.
**Genuinely Boris's feel calls** (§7): (i) after letting go, does the recording's move come back
right away (Touch) or hold until it next moves that control (Latch)? (ii) when recording over a
playing take, replace only while touching, or from the touch until Stop? Neither changes the
format or the engine — both are a mode flag on the Player.

### D9. ROUTINES AS CONTENT — a composition-owned bank of beat-native lane sets, triggered like clips, run as independent hands

**Model (PROPOSED, `src/model/Routine.h`, saved inside the composition — "inside the thing it
belongs to", the same choice the connection arc made; the library folder `…/AudioDNA/Routines/`
(G23 convention) is import/export only):**

```cpp
struct Routine {
    std::string uuid, name;
    double lengthBeats = 16.0;                                // whole bars by default
    Clip::BeatSnapMode quantize = Clip::BeatSnapMode::Bar;    // reuse Clip.h:72-79; global Quantize forces (quantizeModeToForcedSnap, MainComponent.cpp:22)
    bool loop = false, restoreState = true, deckRelative = true;
    std::map<ControlPath, Lane> lanes;                         // beat-native points (D3/D4); preamble points at x = 0
    struct Binding { ControlPath recorded, current; std::string status; };   // re-target table (D2 policy 2/3)
    std::vector<Binding> bindings;
    std::string colour; std::vector<std::string> tags;
};
// Composition.h gains: std::vector<Routine> routines;  std::vector<RoutineSlot{int slot; std::string uuid;}> routineBank;
```

**Trigger.** A dedicated **routine bank** (N slots), not grid cells. Fired through the same input
plumbing as clips: `Binding::Action::TriggerRoutine` (Binding.h:24-44, additive), OSC
`/audiodna/routine/<slot>`, REST `POST /api/trigger_routine {slot}`, keyboard. **Quantized like a
clip trigger**: start aligns to the next boundary per `quantize`, overridden by the global Quantize
when on; pending state mirrors `Layer::pendingTriggerColumn` (Layer.h:157-163) and is cancelled by
global Stop (ruling #5). `x = 0` = that boundary on the RecorderClock. Re-trigger while running =
restart at the next boundary (clip retrigger semantics, Layer.h:236-246). Loop restarts re-fire the
preamble.

**Polyphony and conflicts.** Several routines run at once (one `Player` each, all stepped by the
120 Hz tick). A routine is a hand, so conflicts use D8's chain: two routines on one **control**:
the later `begin` wins for the rest of that gesture; a **human** touching it: D8. Two routines on
one **layer's clip**: last trigger wins (one active clip per layer). Routine vs the **set
replay**: identical rules; the replay is one more player.

**What a future UI needs (not designed):** a bank of slots (name, bars, quantize, loop,
restore-state, running indicator with position); range-select on the take's lane view to slice;
a binding table with unresolved/rebound targets and re-target; "arm a slot and record a routine
directly" (slicing with the whole range); per-slot colour.

**Rejected.** *Routine as a clip `MediaType` in a grid cell*: spans layers; a cell is one layer's
exclusive content (Layer.h:154) — it would evict the clip there and could not run two at once.
*Routines as separate files only*: the missing-file failure class.

### D10. AUDIO IS PART OF THE TAKE — the tap, the sync contract, the test, the cost

**The recording unit is `{lanes + audio}`.** A take is a folder `Name.adna-take/` containing
`take.json` and `audio.wav`, the JSON referencing the audio by relative path + `firstSample` +
hash. Self-contained: offline render needs no external track file.

#### D10.1 The tap — a second consumer, not a reuse (PROPOSED `src/recording/AudioTap.{h,cpp}`)

G26: the ring buffer is SPSC and owned by the analysis thread; it cannot be read twice. The tap is a
**second fan-out inside the callback**, fed the SAME buffers the analysis is fed, at the SAME
point:

- **Hook site:** `AudioEngine::CombinedCallback::audioDeviceIOCallbackWithContext`
  (`AudioEngine.h:66-125`). Restructure the two branches so that "what the app listens to"
  (`const float* const* listened; int listenedChans`) is computed once — mic mode: the input
  channels after gain (`:93-107`); file mode: `outputChannelData` after the transport renders it
  and before it is silenced (`:113-122`) — and then feed **both** `analysisCallback_` and
  `audioTap_.push(listened, listenedChans, numSamples, context)`. One tap call per block.
- **The delivered-sample counter** (the take's origin, D1): `std::atomic<uint64_t>
  deliveredSamples_` in `CombinedCallback`, incremented by `numSamples` at the top of every
  callback in both modes (plus any inserted gap, below), exposed as
  `AudioEngine::getDeliveredSamples()`. `RecorderClock` reads it for `sample`. It is independent
  of the ring buffer, so G26's silent overflow cannot desynchronise the take.
- **Writer:** `juce::WavAudioFormat` 16-bit at the **device rate** (`getCurrentSampleRate`,
  `AudioEngine.h:37`), `listenedChans` channels, wrapped in `juce::AudioFormatWriter::ThreadedWriter`
  (G29 — JUCE's realtime-safe SPSC FIFO + `TimeSliceThread` flush; the canonical
  AudioRecordingDemo pattern) with an 8-second FIFO. `write()` is a memcpy on the audio thread; no
  allocation, no lock.
- **Sample-exact start:** `start(file)` on the message thread creates the writer and sets an
  atomic `armed`; the **callback** consumes `armed` at the top of its next block, records
  `firstSample = deliveredSamples_ (before this block)`, and writes from this block on. The take's
  `audio.firstSample` is that value: WAV frame `k` ⇔ delivered sample `firstSample + k`, exactly.
  `stop()` clears the flag; the `ThreadedWriter` destructor flushes (message thread, blocking OK).
- **Gap detection (G28 — new):** the callback compares `context.hostTimeNs` (ASSUMED non-null on
  macOS/CoreAudio in JUCE 7+; if null, gap detection is disabled and `audio.gapDetection:false`
  is written) between consecutive callbacks: `expected = Δhost · rate`; if `expected − numSamples
  > blockSize/2`, the driver dropped `expected − numSamples` samples: the tap **inserts that many
  zero frames**, `deliveredSamples_` advances by the same amount, and an `audio.gap {sample, n}`
  marker is queued (lock-free) for the recorder. The audio file therefore always has one frame
  per delivered-or-lost sample, and wall↔sample alignment survives the xrun.
- **FIFO overrun** (disk stall > 8 s): `ThreadedWriter::write` returns false → the tap counts
  `droppedFrames_`, inserts silence to keep the frame count honest, and the take is flagged
  `audio.unreliableFrom = sample` in `meta` — surfaced, never silent.
- **Device stop/restart** (`audioDeviceStopped`/`AboutToStart`, `AudioEngine.h:110-119`): the
  counter continues; if the rate or channel count changed the tap closes the current file and
  opens `audio-2.wav` with its own `firstSample`; the take lists segments. A tempo-map anchor
  `why:"audio.restart"` is written.

#### D10.2 The sync contract

1. **One origin:** the delivered-sample counter. Every lane point's `sample` and every audio
   frame index are readings of it. Alignment is a subtraction, not a conversion.
2. **Lane stamps are exact to the reading.** A point stamped on the message thread carries
   *input latency* (mouse → handler, MIDI → `callAsync`, a few ms) — that is *when the app acted*,
   which is also when the visuals changed, so it is the right thing to align a video to. It carries
   no audio-relative error: the counter is the audio.
3. **Wall clock is never used for audio alignment.** `t` drifts against the audio crystal by
   tens of ppm (up to ~30 ms per 10 min); the tempo map's anchors every 8 bars keep `t ↔ sample`
   conversions under 1 ms for editing, and playback of a take *with its audio* drives from
   `sample` (the audio player's position is the clock). Replay *without* audio drives from `t`.
4. **Dropouts/xruns:** D10.1 inserts silence and advances the counter → later points still align.
   Ring-buffer overflow (G26) affects only the analysis stream, never the take; LATER an
   `audio.overrun` marker is logged from the analysis side so a render can warn that the live
   beat grid may have slipped there.
5. **Sample-rate mismatch:** the WAV carries the device rate. G27 (analysis hard-codes 48 kHz) is
   a pre-existing defect outside this lane; consequences here: (a) the offline render pump (D11)
   must resample the WAV to 48 kHz before feeding analysis (`juce::ResamplingAudioSource`);
   (b) recommend the app request 48 kHz from the device — separate ticket.

#### D10.3 THE TEST — Boris's "we will test it to make sure it stays in time" (a deliverable)

**T1 — headless alignment proof** (`tests/test_audio_tap_sync.cpp`, Catch2, links `juce_core` +
`juce_audio_formats` + `juce_audio_basics`; no device; runs in ctest):
1. A `FakeDevice` calls `CombinedCallback::audioDeviceIOCallbackWithContext` (extracted so it is
   constructible without a device — builder: `CombinedCallback` is a private nested class,
   `AudioEngine.h:58`; lift it to `src/audio/CombinedCallback.h` or friend the test) with
   512-sample blocks at 48 kHz, stereo, and a `hostTimeNs` that advances `512/48000` s per block.
   Signal = **click train**: a single-sample impulse (1.0) at absolute indices `c_k = 24000·k`
   (every 0.5 s), zero elsewhere, identical on both channels.
2. Arm the tap at block 7. Read back `firstSample`; assert it equals `7·512`.
3. For each click `c_k ≥ firstSample`, the test acts as the recorder and appends two discrete
   points: **exact** (`sample = c_k`, simulating a stamp read at the instant) and **late**
   (`sample = deliveredSamples after the block containing c_k`, simulating a message-thread stamp
   one block late).
4. Fault injection: between blocks 40 and 41 advance `hostTimeNs` by 3 blocks while delivering 1
   (a 1024-sample driver drop); at block 80 call `audioDeviceStopped` then `AboutToStart` with a
   256-sample block size; at block 120 make `ThreadedWriter::write` return false for 3 blocks (a
   disk stall shorter than the FIFO — must NOT lose frames).
5. Stop; read `audio.wav` with `WavAudioFormat`; locate impulses (`|x| > 0.5`) → indices `w_k`.
   **PASS =** for every exact point `w_k + firstSample == sample_k` (**0 samples** tolerance);
   for every late point `0 ≤ sample_k − (w_k + firstSample) ≤ 512` (**≤ 10.7 ms** at 512/48k, i.e.
   at most one device block); both hold **before and after** the drop and the restart; WAV frame
   count == `deliveredSamples − firstSample` including the 1024 inserted zeros; `droppedFrames_ ==
   0`; exactly one `audio.gap` marker with `n == 1024`.
   Same suite at 44.1 kHz / 128-sample blocks (tolerance scales to one block = 2.9 ms).

**T2 — live device check** (manual, Boris present or scripted over REST): play a 10-minute
click-track WAV in file mode while recording a take; the recorder logs `origin:"engine"` marker
points on every `snap.onsetDetected` (message-thread stamp). Compare each marker's `sample` with
the nearest click in `audio.wav`: report **mean offset** (the constant analysis + tick lag,
expected ≈ 20-30 ms: hop 10.7 ms + ≤ 8.3 ms tick + ring depth), **PASS = p95 jitter ≤ 15 ms and
drift (offset at minute 10 − offset at minute 0) ≤ 1 ms**. Drift is the number that proves
"stays in time"; jitter is the number that bounds how tight a beat-snapped replay can be.

**T3 — end-to-end in the video** (with the render lane, LATER): a take whose only lane fires a
full-white flash on each click; offline-render with audio mux; `ffprobe`/`ffmpeg` scene-cut
timestamps vs audio peaks: **PASS = |Δ| ≤ half a frame (16.7 ms at 30 fps)** for every click.

#### D10.4 Format and cost

| item | choice | per hour |
|---|---|---|
| audio (NOW) | WAV PCM 16-bit, device rate, listened channels | stereo 48 kHz: **691 MB**; mono (a 1-ch mic): **346 MB**; 24-bit stereo: 1.04 GB |
| audio (option, v1.1) | FLAC via `FlacAudioFormat` + the same `ThreadedWriter` (≈ 55 % of WAV; ~1-2 % of a core to encode) | stereo ≈ **380 MB** |
| lanes (`take.json`) | JSON, ≤ 20 pts/s per dragged control, three stamps each | **1-3 MB** |
| video (for comparison, what the take avoids) | 1080p30 H.264 CRF 23 (G17 config) | 2-4 GB |

WAV now because it is zero-risk, the JUCE pattern is proven, and FFmpeg muxes it straight into
the render (D11). In file mode the captured audio duplicates the DJ's file; Boris asked for what
the app heard, and self-containment is the point — a "reference the original instead" dedupe is
a LATER option.

### D11. OFFLINE RENDER — what must be true, and the honest limits

**Reproduction contract.** A render = `take (lanes + audio) + composition + media`. The lanes
reproduce the **hands** (and, once `origin:"engine"` points exist, the engine's random decisions);
the **sound-driven** motion is regenerated by re-analysing the take's own audio — it is not in the
lanes and must not be (D6). Therefore:

1. **Audio always present (D10)** — no external file dependency. "Render without audio" is an
   explicit option (tempo from the tempo map, ruling #4 semantics; audio-channel connections
   flat) offered with a warning, never a silent fallback.
2. **Sample-aligned, faster-than-realtime analysis.** The analysis is sample-driven with no RNG
   (G9): fed the take's WAV (resampled to 48 kHz if needed, G27) from `firstSample` in the same
   hop sequence, it publishes the same snapshots (ASSUMED for aubio internals — builder verifies
   with a two-run hash of the snapshot stream). Needs an offline pump: `AnalysisThread` stepped
   synchronously from a file reader instead of the `sleep(1)` poll (G18), advancing until
   `snap.timestamp ≥ frameSample − firstSample`. **Tempo interventions are lane points** (G11:
   `tempo` lane) applied to the tracker at their `sample` during re-analysis; without them the
   offline beat grid diverges from the show.
3. **A fixed-step frame clock.** Frame `f` at `fps`: `sample = firstSample + f·sr/fps`, `u_time =
   f/fps` (override exists, G15), video/sequence `dt = 1/fps` — the four hard-coded `1/60` sites
   (G15) become a compositor `frameDt_` set per frame. (Side finding, ASSUMED: if
   `VideoPlayer::advanceFrame(dt)` uses `dt` literally, video already runs at half speed whenever
   the preview renders at 30 fps — builder checks `VideoPlayer.cpp` before touching it.) The 120 Hz
   tick (G12) becomes steppable: `tickFeaturePipeline(dt)` called `round(120/fps)` times per frame
   with timers stopped; then `Player::advanceTo(frameSample)`; then one `renderOpenGL` via
   `executeOnGLThread(…, true)` with `setLockedResolution` (G16) and `VideoRecorder::submitFrame`
   (G17). Per-frame `captureFrame` (G16) is the wrong primitive (blocking promise, PNG per frame).
4. **Audio in the output** (ruling #3, approved, Harmony triages): FFmpeg is linked (G17); add an
   audio stream from `audio.wav` offset by `firstSample`. T3 proves the mux.
5. **Non-deterministic content is named, not hidden**: projectM/MilkDrop sources (own clock),
   camera clips, image slideshow, and — until `origin:"engine"` logging lands — autopilot random
   choices (G13). The render report lists which clips fell in that class.

**Bit-identical reproduction: NO, and not promised.** Promised: every human and logged-engine
decision lands on the exact frame it did live (event-accurate, T1/T3 tolerances), the beat grid
matches the show, and rendering is as deterministic as the GPU is — same machine/build/driver,
shader-only content is typically pixel-identical across runs; across machines it is not.

### D12. VERSIONING + MIGRATION — `version` + `minReader` + capability flags + opaque round-trip; ADD, never REDEFINE

**Take envelope (v2):**
```jsonc
{ "format": "audiodna-take", "version": 2, "minReader": 2,
  "features": ["lanes", "tempoMap", "checkpoint0", "audio", "markers"],
  "meta": { "recordedAt": "2026-09-05T21:14:02Z", "app": "0.1.0", "duration": 3612.4, "durationBeats": 7702.1 },
  "clock": { "startWall": "…", "unmeteredSegments": 1 },
  "audio": { "segments": [ { "file": "audio.wav", "firstSample": 3584, "frames": 173145600, "rate": 48000, "channels": 2, "sha1Head": "…" } ],
             "mode": "file" | "input", "gapDetection": true, "gaps": [ { "sample": 9830400, "n": 1024 } ], "unreliableFrom": null },
  "composition": { "name": "Friday", "file": "Friday.audiodna", "decks": 2 },
  "tempoMap": [ { "t": 0, "beat": 0, "sample": 3584, "bpm": 0, "why": "start" }, { "t": 3.2, "beat": 0, "sample": 157184, "bpm": 128.0, "why": "lock" } ],
  "checkpoint0": { "t": 0, "beat": 0, "sample": 3584, "state": { …PerfState… } },
  "checkpointEnd": { … },
  "markers": [ { "seq": 7, "t": 61.2, "beat": 128, "sample": 2941184, "name": "drop" } ],
  "lanes": [
    { "key": { "scope": "layer", "deck": {"i":0,"name":"Deck 1"}, "layer": {"i":0,"id":0,"name":"Layer 1"}, "control": "activeClip" },
      "kind": "discrete",
      "points": [ { "seq": 2, "t": 4.01, "beat": 1.7, "sample": 196064, "bpm": 128, "origin": "human", "v": 2, "group": 11 },
                  { "seq": 9, "t": 12.0, "beat": 18.9, "sample": 579584, "bpm": 128, "origin": "human", "v": 2, "retrigger": true } ] },
    { "key": { "scope": "layer", "deck": {"i":0,"name":"Deck 1"}, "layer": {"i":1,"id":1,"name":"Layer 2"}, "control": "scalar", "scalar": "opacity" },
      "kind": "continuous",
      "gestures": [ { "grip": "held", "curve": [ { "x": 12.80, "y": 0.60 }, { "x": 12.93, "y": 0.62 }, { "x": 14.10, "y": 0.85, "interp": "linear" } ],
                      "stamps": [ { "seq": 3, "t": 9.30, "sample": 449984 }, { "seq": 4, "t": 9.35, "sample": 452384 }, { "seq": 5, "t": 9.90, "sample": 478784 } ],
                      "origin": "human" } ] },
    { "key": { "scope": "comp", "control": "tempo" }, "kind": "discrete",
      "points": [ { "seq": 6, "t": 30.1, "beat": 57.2, "sample": 1448384, "bpm": 128, "origin": "human", "action": "tap", "v": 129.5 } ] }
  ] }
```
A continuous gesture stores its curve as the shared `AutomationCurve` (D7; `x` in beats) plus a
parallel `stamps` array (one per breakpoint: `seq`, `t`, `sample`) so the three-clock rule holds
per point without bloating the curve type. Routine files: `"format": "audiodna-routine"`, lanes
only, `x` in beats, no `stamps`/tempo map/audio.

**Loader rules.**
1. `minReader > reader` → refuse with a message naming the version; never half-play.
2. Load; every `features` entry the reader does not know is listed in `LoadStats` (the conn spec's
   reporting shape, §2.7).
3. Unknown `control`, `kind`, lane field or top-level section → kept **opaque** (raw `juce::var`
   on the object), skipped at compile, counted by name, **re-saved verbatim**.
4. Missing required fields → the lane/point is marked invalid, counted, listed; never silently
   defaulted. Missing optional → documented defaults.
5. **v1 → v2 at load** (G1): `ClipTrigger(layer, col)` → a point in the `activeClip` lane of
   `(deck rel, layer)`; `ColumnTrigger(col)` → one point per layer present in the current
   composition, one `group`; `ParameterChange/MacroChange/TransportChange/EffectToggle/CuepointJump`
   → best-effort lanes with `rebound-by-name` attempts, else unresolved (the July spec noted no
   real v1 files exist beyond clip-trigger-only sessions); `t` → `t`; `beat`/`sample` absent →
   `features` gains `"wallOnly"`, no `audio`; the compiler refuses beat/sample drive with a clear
   message. `seq` minted sequentially. **Never rewritten on load**; save writes v2.

**The rule that keeps capabilities 3 and 4 from breaking the format:** *ADD, never REDEFINE.* A
capability may add controls, lane fields, features or top-level sections; it may not change the
meaning, unit or type of an existing field; if a meaning must change, it gets a new name and the
old one stays readable. Enums are strings. Every reader round-trips what it does not understand.
Enforced by fixtures (§5 step 1): a v1 file, a v2 file, and a "from the future" file with an
unknown control, an unknown section and an unknown feature must all load, compile with the right
report counts, and re-save with the unknowns byte-preserved. Reserved now for LATER: `lane.takes[]`
(D8 New Take), `control: "conn"`, `origin: "engine"`, `audio.segments[n>1]`, `routine` lanes.

### D13. OPACITY — three levels, multiplicative; the composition's second field merges into master (model note only)

Per Boris: `final = master × layer × clip`; a clip pinned at 0.5 never exceeds 0.5 whatever
master and layer do. In this spec: three continuous lanes exist — `scalar: opacity` at
`comp` (master), `layer`, `clip` — and `PerfState` carries the same three. `Composition::
compOpacity` (`Composition.h:29`) is **not** a lane and not in `PerfState`: it merges into
`masterOpacity` (`:25`); the conn spec's `CompScalar` table loses its `Opacity` entry (its D7 "two"
recommendation is superseded by the owner). The renderer change is the lead's lane, not this one.

### D14. WHAT TO BUILD NOW vs LATER — the clean cut

**NOW (capability 1 + the audio half of the take + its timing proof):**
- `ControlPath`, `AutomationCurve` (shared with Lane 2 — D7), `Take`/`Lane`/`Program`/`Player`/
  `PerformanceRecorder`/`RecorderClock`, `PerfState` capture (checkpoint 0 + end), v1 loader,
  fixtures.
- `AudioTap` + delivered-sample counter + gap detection + **T1** in ctest; the `.adna-take` folder.
- Capture at choke points that exist or are stable: `activeClip` (clip + column, grouped),
  `activeDeck`, `tempo`, `audio`, `quantize`; continuous lanes at the REST/MIDI/OSC `manualWrite`
  sites (D6a); `enable`/`bypass`/layer flags at the binding and REST paths.
- Player driven from the 120 Hz tick with continuous lanes sampled (smooth) and `Origin` threaded
  into the handlers (overdub-safe); TOUCH override mode (D8) — with L2 absent, "refused while a
  human holds" is implemented by the recorder's own held-set until the grip engine takes it over.
- Playback with audio: when a take has audio, the player drives from the audio player's sample
  position (D10.2 #3) — reuse `AudioEngine::loadFile/play` on `audio.wav` (G18) with a sample-
  offset so the analysis re-hears the same sound (this is what makes a replay *look* like the
  show, not just fire the same buttons).
- `RecordPanel` made real (dead UI gets built): Record/Stop/Play/Save/Load against the new
  classes, 4 Hz refresh, compile report on the status line; the "Video (Future)" item becomes
  "Render… (coming)" disabled with a tooltip, not hidden.
- REST `/api/perf/{status,record,stop,play,stop_play,save,load}`.

**Deliberately NOT built now, and what it costs:**
- Inspector-widget parameter capture → knob moves made with the mouse on the current inspector
  are not in the take until the UI rewrite / connection L3 wires `manualWrite`. MIDI/OSC/REST
  moves are.
- `conn` lane capture → connecting a signal mid-set is not logged until L3.
- The lane editor, edit ops beyond delete, LATCH/OVERWRITE/NEW TAKE modes, slicing, routines,
  `origin:"engine"` logging, print/bake conversions, offline pump/fixed-step clock/render driver/
  audio mux, FLAC, T2/T3 → capabilities 2-4 unavailable, but every field, control and section they
  need is in the format.
- "Musical replay" of a whole set by beat → a compile option, LATER.

---

## 3. Model sketches (PROPOSED — signatures a builder can hold to)

```cpp
// src/model/ControlPath.h — the lane key; shared by recorder, REST, later the connection test endpoints
struct ControlPath {
    enum class Scope : uint8_t { Clip, Layer, Comp, Macro, Routine } scope;
    int deck = -1; bool deckRelative = false; std::string deckName;
    int layer = -1; uint32_t layerId = 0; std::string layerName;
    int col = -1; std::string clipName;
    int fx = -1; std::string fxName;
    std::string control;                    // D2 vocabulary
    int param = -1; std::string paramKey;   // control == "param"
    std::string scalar;                     // control == "scalar"
    int macroScope = 0, macro = -1;         // control == "macro"
    bool isContinuous() const;              // param|dryWet|scalar|macro|speed
    bool operator<(const ControlPath&) const;   // map key: positional fields first, names never compared
    juce::var toVar() const; static ControlPath fromVar(const juce::var&);
};

// src/model/AutomationCurve.h — shared with ConnSource::Envelope (D7)
struct Breakpoint { double x; float y; enum class Interp : uint8_t { Linear, Hold, Smooth } interp = Interp::Linear; };
struct AutomationCurve { std::vector<Breakpoint> pts; float eval(double x) const; double xMin() const; double xMax() const; };

// src/recording/Lane.h
struct Stamp { uint64_t seq; double t; uint64_t sample; };
enum class Origin : uint8_t { Human, Replay, Routine, Engine, Preamble };
struct DiscretePoint { Stamp s; double beat; float bpm; Origin origin; int v = 0; std::string action; bool retrigger = false; uint64_t group = 0, via = 0; juce::var raw; };
struct Gesture { std::string grip; AutomationCurve curve; std::vector<Stamp> stamps; float bpmAtBegin; Origin origin; uint64_t via = 0; };
struct Lane { ControlPath key; enum class Kind : uint8_t { Continuous, Discrete } kind; std::vector<DiscretePoint> points; std::vector<Gesture> gestures; juce::var raw; };

// src/recording/Take.h — the DOCUMENT (value type)
struct TempoAnchor { double t, beat; uint64_t sample; float bpm; std::string why; };
struct TempoMap { std::vector<TempoAnchor> a; double beatAt(double t) const; double tAt(double beat) const; uint64_t sampleAt(double t) const; };
struct AudioRef { struct Segment { std::string file; uint64_t firstSample, frames; double rate; int channels; std::string sha1Head; }; std::vector<Segment> segments; std::string mode; bool gapDetection; std::vector<std::pair<uint64_t,uint32_t>> gaps; std::optional<uint64_t> unreliableFrom; };
struct Take {
    std::map<ControlPath, Lane> lanes; TempoMap tempo; PerfState checkpoint0, checkpointEnd; AudioRef audio; std::vector<DiscretePoint> markers; Meta meta; uint64_t nextSeq = 1;
    bool save(const juce::File& folder) const; static std::optional<Take> load(const juce::File& folder, LoadStats&);
    std::vector<std::pair<double, const void*>> chronological() const;   // derived view (D3)
    void deletePoints(std::span<const uint64_t> seqs); void deleteLane(const ControlPath&);   // NOW; the rest LATER (D5)
};

// src/recording/Program.h — IMMUTABLE compiled schedule
enum class DriveClock : uint8_t { Wall, Beat, Sample };
struct ResolvedTarget { /* validated coordinates only; no pointers */ };
struct Fired { double at; uint64_t seq; ControlPath key; ResolvedTarget target; DiscretePoint p; };
struct ContLane { ControlPath key; ResolvedTarget target; struct G { double x0, x1; AutomationCurve curve; std::string grip; }; std::vector<G> gestures; };
struct CompileReport { std::vector<Issue> unresolved, reboundByPosition, reboundByName, invalid; std::map<std::string,int> unknown; std::vector<std::string> missingRoutines; };
struct Program { DriveClock clock; double length; bool loop; std::vector<Fired> preamble, discrete; std::vector<ContLane> continuous; CompileReport report; };
std::shared_ptr<const Program> compile(const Take&, const Composition&, DriveClock, std::optional<Range> = {});

// src/recording/Player.h — one per running thing (set replay, each routine)
struct Sink { virtual bool fire(const Fired&) = 0; virtual bool touch(const ControlPath&, const std::string& grip) = 0;
              virtual bool set(const ControlPath&, float v) = 0; virtual void release(const ControlPath&) = 0; };   // bool = accepted (D8 chain)
class Player {
public:
    enum class Override : uint8_t { Touch, Latch };
    explicit Player(std::shared_ptr<const Program>);
    void start(double at = 0); void advanceTo(double pos, Sink&); void stop(Sink&);   // stop releases all open touches
    void swap(std::shared_ptr<const Program>, Sink&);                                   // re-seat cursors; release orphaned gestures
    void setOverride(Override); void reenable(const ControlPath&); void reenableAll();
    double position() const; bool running() const; const CompileReport& report() const;
private:
    std::shared_ptr<const Program> prog_; size_t nextDiscrete_ = 0; double pos_ = 0;
    struct LaneCursor { size_t gesture = 0; bool inGesture = false; bool displaced = false; }; std::vector<LaneCursor> cursors_;
};

// src/recording/PerformanceRecorder.h — capture only, message thread only (jassert)
class PerformanceRecorder {
public:
    bool start(const Composition&, RecorderClock&, AudioTap&, const juce::File& takeFolder);   // checkpoint 0 + arms the tap
    void discrete(const ControlPath&, DiscretePoint&&);                                         // stamps, seq, group
    void touch(const ControlPath&, std::string grip); void set(const ControlPath&, float v); void release(const ControlPath&);   // gesture capture + 50 ms coalesce
    Take stop(const Composition&);                                                              // checkpointEnd + closes the tap
};

// src/recording/RecorderClock.h — fed by tickFeaturePipeline (G12)
struct ClockStamp { double t, beat; uint64_t sample; float bpm; };
class RecorderClock { public: void tick(const FeatureSnapshot&, double wallNow, uint64_t deliveredSamples); ClockStamp now() const; const TempoMap& tempo() const; };

// src/recording/AudioTap.h — D10.1
class AudioTap {
public:
    void prepare(double deviceRate, int channels, int maxBlock);          // audioDeviceAboutToStart
    bool start(const juce::File& wav);                                     // message thread; sets armed
    void push(const float* const* ch, int n, uint64_t deliveredBefore, const juce::AudioIODeviceCallbackContext&);   // audio thread, RT-safe
    void stop();                                                           // message thread; flushes
    uint64_t firstSample() const; uint64_t framesWritten() const; uint64_t droppedFrames() const;
    bool popGap(std::pair<uint64_t,uint32_t>&);                            // lock-free queue drained by the recorder
};
```

Dispatch (`MainComponent`'s `Sink` implementation, PROPOSED): re-resolves the target by coordinate
(G7 pattern), then calls the SAME handler the human path uses with `Origin::Replay|Routine` —
`handleClipTrigger(deck, layer, col, origin)` (gains `deck`; today it reads only the active deck,
`MainComponent.cpp:3783`), `handleDeckSwitch`, `applyTempoCommand`, `applyAudioTransport`,
`applyToggle`, `manualWrite`. Undo: replayed points push no commands (July §6 stands;
`handleClipTrigger` pushes `TriggerClipCmd` today at `MainComponent.cpp:3932-3939` → gate on
`origin == Human`).

---

## 4. Threading contract

| State | Owner | Readers | Mechanism |
|---|---|---|---|
| `deliveredSamples_` | audio thread (increment) | message thread (`RecorderClock`), tap | relaxed atomic; monotonic |
| `AudioTap` FIFO | audio thread writes | `TimeSliceThread` flushes to disk | `ThreadedWriter` (JUCE SPSC); `armed` atomic consumed on the audio thread |
| gap markers | audio thread pushes | message thread drains per tick | small lock-free queue (`juce::AbstractFifo` + array) |
| `RecorderClock`, `PerformanceRecorder`, `Player`s | message thread (120 Hz tick, G12) | — | confinement + `jassert(MessageManager::existsAndIsCurrentThread())` |
| `Take` (document) | message thread | save on message thread (JSON ≤ a few MB) | value type |
| `Program` | immutable after compile | any `Player` | `shared_ptr<const>` |
| GL-originated points (autopilot, G13) | recorded from the existing `callAsync` hop (`Renderer.cpp:249-252`) | — | never from the GL thread; `SessionRecorder`'s `CriticalSection` is deleted with it |
| REST/OSC/MIDI captures | already marshalled to the message thread (`ApiServer.cpp:408`, conn spec §2.3) | — | unchanged |

---

## 5. Build order — NOW

| # | Step | Size | Verify (independent pass/fail) |
|---|---|---|---|
| 1 | `src/model/{ControlPath.h, AutomationCurve.h}`; `src/recording/{Lane.h, Take.{h,cpp}, TempoMap, PerfState.{h,cpp}, Program.{h,cpp}, Player.{h,cpp}, PerformanceRecorder.{h,cpp}, RecorderClock.{h,cpp}}`; delete `SessionRecorder.{h,cpp}`; ctest `test_take` per G24 | M | Catch2: (a) JSON round-trip lane-by-lane, seqs stable; (b) v1 fixture → lanes, `wallOnly`, counts; (c) "future" fixture → unknown control/section/feature byte-preserved on re-save, reported; (d) `RecorderClock`: scripted `beatPhase` incl. a resync drop → monotonic `beat`, anchors, unmetered segment at `bpm=0`; (e) coalescing: 100 `set` in 50 ms → bounded, begin/end exact; `AutomationCurve::eval` linear/hold; (f) compile: unresolved / rebound-by-position / rebound-by-name each produce the right report entry; (g) `Player`: scripted dt (8.3 ms fixed, seeded jitter, one 400 ms stall) fires the identical discrete sequence each ≥ its `at`; a continuous gesture emits touch/set(interp)/release with values on the curve; `swap` mid-gesture re-seats and releases orphans; `stop` releases all; TOUCH refusal (`Sink::set` returns false) marks the lane displaced for that gesture only |
| 2 | `AudioTap` + `deliveredSamples_` + gap detection in `CombinedCallback` (lift it to `src/audio/CombinedCallback.h` so T1 can construct it); `.adna-take` folder save/load; **T1** (`tests/test_audio_tap_sync.cpp`) | M | **T1 passes at the tolerances in D10.3** (0 samples exact; ≤ 1 block late; across drop + restart + FIFO stall); a 60 s app recording produces a WAV whose frame count equals `deliveredSamples − firstSample` |
| 3 | `MainComponent` wiring: `RecorderClock::tick` + `Player::advanceTo` in `tickFeaturePipeline`; `Origin`/`deck` on the trigger/deck handlers; `applyTempoCommand` replacing the ≥5 direct tracker writes (G11); `applyAudioTransport`; `applyToggle` at binding + REST; `manualWrite` at the D6a sites (REST `set_param` becomes a marshalled callback like its siblings); `Sink` impl; undo push gated on `Origin::Human`; playback-with-audio driving from the audio position | M | app: record → 3 clip hits, deck switch, tap tempo, MIDI opacity drag, effect toggle via binding → stop → `take.json` shows the lanes with three stamps, `audio.wav` beside it, tempo map with `lock`/`tap`; play → same visible sequence, the drag replays smooth; grab the opacity mid-replay → it follows the hand, let go → glides back to the lane; record during replay → only the new human moves appear |
| 4 | `RecordPanel` real: shadow bool removed (G25), state from recorder/player, 4 Hz refresh, `Playing 12.3 / 45.6 s · unresolved: N`, Save/Load gated to Idle, "Render… (coming)" disabled item | S | click-through matrix; a take loaded against a smaller composition shows `unresolved: N`, not silence |
| 5 | REST `/api/perf/*` (pattern `ApiServer.cpp:118-221`, marshalled like `switch_deck`) | S | scripted smoke over 8080: record → REST triggers → stop → save → load → play → status reports position + report counts |
| 6 | **T2** on real hardware (Harmony runs the gate; Boris optional) | S | mean offset reported; p95 jitter ≤ 15 ms; 10-min drift ≤ 1 ms |
| 7 | Docs: APP-INVENTORY rows; this spec's `features` list; the Lane 2 note (D7) cross-referenced in the connection spec | S | rows match app |

Steps 1-3 are the demoable core. Dependencies: after connection **L0** (landed); **step 1's
`AutomationCurve` must be agreed with Lane 2's builder before either lands** (D7, R8).

**LATER lanes, in order** (each additive to the format):
- **L-E Editing**: the lane editor (bar grid, per-lane view), edit ops + undo commands, hot-swap
  in the app, LATCH/OVERWRITE/NEW TAKE, markers, print/bake (D7).
- **L-R Routines**: `Routine` model + serialization, `slice()`, bank, `TriggerRoutine`
  binding/OSC/REST, quantized start + pending state, polyphony, `routine` lane + `via`, binding
  table + report.
- **L-P Parameter/toggle capture completion**: rides connection L3(e) (`manualWrite` gains the
  grip; the recorder's held-set retires) and the UI rewrite; `conn` lane.
- **L-D Determinism + render**: `origin:"engine"` logging → fixed-step frame clock (compositor
  `frameDt_`, the four `1/60` sites) → offline analysis pump (+ resampler, G27) → steppable
  `tickFeaturePipeline(dt)` → render driver over `VideoRecorder` → audio mux → **T3**; FLAC option;
  `audio.overrun` markers.

---

## 6. Risks (each with the file it lives in)

- **R1 Pointer hand-out survives the rewrite** — `SessionRecorder.cpp:120-140` is deleted; reviewer
  greps `const Event*|const DiscretePoint*` returned from any `advance` in `src/recording/` → 0.
- **R2 Silent early-outs reached by replay** — `MainComponent.cpp:3783-3787, 4518-4519`.
  Compile-time resolution before any dispatch; fire-time re-resolution failures counted. Test (f).
- **R3 Beat clock on tracker resets + Link flood** — `BPMTracker.h:127-137`; Link at 30 Hz
  (`MainComponent.cpp:3112-3125`) must record `tempo{link}` only on a ≥ 0.01 BPM change. Test (d).
- **R4 Clip-id keying** — `CompositionLoad.h:65`; reviewer greps `clip.id|->id` in `src/recording/`
  and `ControlPath.h` → 0.
- **R5 Overdub feedback** — replayed points re-entering `handleClipTrigger` with the recorder
  running (`MainComponent.cpp:3815`). `Origin` must reach every handler; step-3 gate.
- **R6 Flood through non-funnel writers** — `EffectStackView::refresh()`, inspector
  `tickModulation` (conn spec C1/C2) until L3. Do not hook them.
- **R7 GL-thread capture** — `Renderer.cpp:245-254`, `Autopilot.cpp:241`. Message-thread hop only;
  `PerformanceRecorder` asserts the thread.
- **R8 Collision with connection L2/L3** — TWO shared points now: `manualWrite` at the ≥9 sites
  (L3(e)) and `AutomationCurve` inside `ConnSource::Envelope` (L2, D7). Name one owner per item
  in both packets; L2 must not freeze `Envelope::points` as a bare pair-vector.
- **R9 Held grips never released by a stopped routine/replay** — `Player::stop/swap` release all;
  test (g). Until L2, the recorder's own held-set must be cleared on stop and on preset load.
- **R10 Message-thread stalls** — modal dialogs pause the tick; events burst on resume (July risk
  #2). Playback-with-audio is immune for timing (drives from the audio position) but the burst
  still happens; Save/Load gated to Idle.
- **R11 Hard-coded `dt = 1/60`** — `CompositorEngine.cpp:728,747,831,895`; possible visible
  half-speed at 30 fps if `VideoPlayer::advanceFrame` (`VideoPlayer.h:85`) uses `dt` literally.
  Check before L-D; file separately if so.
- **R12 Ring-buffer overflow is silent** — `AudioCallback.cpp:47` ignores `push`'s return; the
  analysis stream (and the live beat grid) can lose samples under a stall with no trace. Not the
  take's problem (D10) but a render's: LATER `audio.overrun` marker; pre-existing, file it.
- **R13 Analysis rate hard-coded at 48 kHz** — `AnalysisThread.h:47` vs device rate; BPM and
  `wallClockSeconds` wrong at 44.1 kHz (INFERRED). Pre-existing; the pump resamples; recommend
  forcing 48 kHz at the device. File it.
- **R14 `hostTimeNs` availability** — ASSUMED non-null on CoreAudio; if null, gap detection is
  off and the take says so (`audio.gapDetection:false`). T1 covers both branches.
- **R15 Disk throughput** — 691 MB/h stereo WAV is 192 KB/s: trivial for any SSD; a network
  volume or a nearly-full disk trips the 8 s FIFO → `unreliableFrom` flagged, never silent.
  `RecordPanel` should refuse to start on a volume with < 2 GB free.
- **R16 Two recorders in the UI** (`.harmony/HANDOFF.md:2192-2197`): the event-log tab and the
  menu's video recorder must become one "Record" concept in the rewrite — a take is the primary
  artefact; video is a render of it. UI need, noted.

**Strongest counterargument to this design, and why it loses:** *lanes + three stamps + a tempo
map + an audio tap + a sync test is a lot of format for capability 1; ship wall-clock v2 with a
flat log as the July spec said and add the rest when 2-4 are built.* It loses on what the owner
said today: the timelines are per-control and editable like Ableton, and the audio is part of the
recording. A flat wall-only log recorded tonight cannot be turned into lanes with beats later
(the tracker state is gone), cannot be aligned to audio that was never captured, and cannot be
sliced into a routine. The extra cost now is one clock class, one curve type shared with a lane
already being built, one tap in a callback that already fans out once, and a test — against a
v3 that could not read v2 for the purpose it was recorded for.

---

## 7. NEEDS BORIS (feel/product calls only; recommendation in brackets)

1. When a routine fires, should it first put the layers and knobs it uses back the way they were
   when you recorded it, or start from wherever things are right now? [Restore, with a per-routine
   "start from now" switch.]
2. Should a routine start on the next beat or the next bar? [Bar; per-routine choice; the global
   Quantize overrides when it is on.]
3. Play once, or loop, by default? [Once; loop is a per-slot switch.]
4. A routine's clip hits: on the layer they were recorded on, or on the layer you are working on
   now? [Recorded layer, with a re-target list when it no longer exists.]
5. While a take plays you grab a knob it is moving, then let go: should the take's move come back
   right away (gliding, like letting go of a signal), or should your value hold until the take
   next moves that knob? [Come back right away.]
6. With Record armed over a playing take, should your move replace the take's move only while
   you are touching, or from the moment you touch until you press Stop? [Only while touching.]
7. Routines live in their own bank of pads, not in the clip grid. Any objection? [Bank.]

(The audio question from revision 1 is withdrawn — Boris decided: the audio is always recorded
with the take.)

---

## 8. Summary for Harmony (10 lines)

1. The take = `{per-control lanes + audio}`; lanes are the on-disk form (edits are lane-local; cross-lane order is a compile-time k-way merge on `(at, seq)`); continuous lanes (sliders: gestures as curves, sampled smooth at 120 Hz) and discrete lanes (buttons/triggers/tempo/transport: state-valued points, fired) share one structure and two evaluators.
2. Timebase unchanged in shape — `t` + `beat` + `sample` + `bpm` per point, tempo map per take — but `sample` is now the **delivered-audio counter** in the callback (the ring-buffer counter undercounts on overflow), which is also the audio file's origin.
3. Addressing: `ControlPath` (positional path + names + control) is the lane key; never clip id; resolved at compile with a report; unresolved lanes are counted and shown, never silent.
4. **D7 — the answer you needed:** the drawn "Timeline" and a recorded lane are the SAME CURVE (Lane 2 must adopt a shared `AutomationCurve{x,y,interp}` for `ConnSource::Envelope::points` now — additive, no behaviour change) but NOT the same OWNER: a lane is a recorded hand (stackable, gappy, routine-clocked, covers non-connectable buttons too); a connection is the exclusive permanent owner. Print-to-Timeline / bake-to-lane are one-click curve copies. Do not add a `Kind::Lane` to `ConnSource`.
5. Override: one ownership chain — human Held > human Decaying > playing lane gesture > connection > rest; TOUCH default (let go → glide back to the lane, same as a signal), LATCH option (hold until the lane's next gesture / re-enable), OVERWRITE when armed (touch-scoped `replaceSpan`), NEW TAKE reserved.
6. Audio: `AudioTap` = second fan-out inside `CombinedCallback` (ring buffer is SPSC, cannot be reused), `ThreadedWriter` WAV 16-bit at device rate, sample-exact arm, host-time xrun detection inserting silence, restart segments; one origin = the delivered counter; wall clock never used for alignment. Cost: 691 MB/h stereo (346 mono; FLAC ≈ 380 later) vs 1-3 MB/h lanes.
7. The timing TEST is specified as a deliverable: T1 headless click-train through the real callback with a fake device, fault-injected drop/restart/FIFO stall — pass = 0 samples for exact stamps, ≤ 1 block (10.7 ms) for message-thread stamps, before and after faults; T2 live: p95 jitter ≤ 15 ms, 10-min drift ≤ 1 ms; T3 in the rendered video: ≤ half a frame.
8. Routines unchanged in shape (composition-owned, bank-triggered, quantized, polyphonic, preamble + relative lanes); checkpoints reduced to checkpoint 0 + end because state-valued lanes make periodic ones unnecessary.
9. Opacity: three multiplicative lanes (master/layer/clip); `compOpacity` merges into master and leaves `PerfState` and the scalar table.
10. NOW = format + classes + `AudioTap` + T1 + choke-point capture + 120 Hz sampled player with TOUCH + playback-with-audio + real RecordPanel + REST + T2; NOT NOW = inspector-widget capture, editor/edit modes, slicing, routines, render. Two coordination points with the connection arc: `manualWrite` (L3e) and `AutomationCurve` (L2).

---

## 9. ADDENDUM — timelines and connection sources (answer to the team-lead's four questions; §D7 has the long form)

### 9.1 HOLDS or BREAKS

**BREAKS at the owner level; HOLDS at the curve level.** Your reading — "a recorded lane plugs into
`ConnSource` where `Signal` and `Oscillator` do" — is wrong, and the reconciliation is exactly
the one D6/D8 imply: **the `Player` is a WRITER through `manualWrite`, one more hand in the grip
chain; a lane is not a connection source.** Plainly: two features do not collapse into one
`ConnSource` kind. They collapse into one **curve type** (9.2), which is the part worth taking
into Lane 2 today.

The evaluation shape is *not* what separates them — a continuous lane's gesture IS a function
evaluated per tick (the `Player` samples its `AutomationCurve` at 120 Hz, D3/D5), the same
shape as `ConnectionEngine::evaluate`. What separates them is ownership, and on four properties
a connection has and a lane cannot (D7, one line each):

1. **Exclusivity.** One `ParamConnection` per parameter, enforced by the type (conn spec §2.6).
   A knob can be under the set replay AND two routines AND a hand in the same minute; two of
   them at once is legal (D9). A lane-as-owner would make a routine trigger *unplug* the knob's
   bass connection and plug it back after — the opposite of what was recorded (a hand on top of
   the bass).
2. **Gaps.** A lane is silent between gestures; the control belongs to its owner, and the
   gesture's end is a hand-back glide. An envelope is a total function over its cycle.
3. **Clock and trigger.** An envelope cycles on the beat grid or the clip playhead and restarts
   on *clip* trigger (`Clock {Beats, ClipPosition}`, §2.4). A lane runs from the *take/routine*
   trigger, quantized, stopped and looped as a unit with every other lane in that routine — an
   identity that must live outside any one parameter regardless.
4. **Coverage.** Half the lanes (clip hits, deck switches, tap tempo, enable/bypass) are buttons
   — not connectable by Ruling B.1. "Lane = source" would cover sliders only and need a second
   model for the rest.

**Do `Program`/`Player` and a per-parameter owner coexist, or fight?** Coexist, by the rule that
already exists. They never write the same field: the `ConnectionEngine` publishes the owner's
value into the `LiveValue` twin (§3.1); the `Player` writes the *manual* field through
`manualWrite`, which takes a `Held` grip, so the engine stores `NaN` in the twin and the
renderer reads the manual field (§2.3) — the lane's value. On the gesture's `end` the player
releases, and the engine glides from the last manual value to the owner's value
(`handBackGlideMs`). A human grabbing the knob outranks the player (D8 chain), and the player's
next gesture re-takes. **One ordering requirement for the builder (new, belongs in §4):** within
the 120 Hz tick, `Player::advanceTo` runs BEFORE `ConnectionEngine::tick`, so grip state is
current when the engine decides whether to publish; otherwise a gesture's first tick renders one
frame late.

### 9.2 What HOLDS, and the minimal Lane 2 change

The hand-drawn "Timeline" (`ConnSource::Kind::Envelope`, owner D6/D8 "build") and a recorded
continuous lane are the **same curve** — one drawn, one captured — and must share one struct,
one editor, one evaluator:

- `src/model/AutomationCurve.h` (PROPOSED, D7): `Breakpoint { double x; float y; Interp
  {Linear, Hold, Smooth} }` + `AutomationCurve { pts; eval(x); xMin(); xMax() }`.
- `ConnSource::Envelope::points` (a bare `vector<pair<float,float>>` today, §2.1) becomes
  `AutomationCurve curve` with `x ∈ [0,1]`. Semantics — `playbackXform`, `loop`, `Clock`,
  `cycleBeats` — unchanged. Cost: a type rename and one enum. `Interp::Hold` is the step mode a
  drawn Timeline needs for strobe-like shapes anyway.
- **Reserve nothing else.** Do NOT add `Kind::Lane`/`Kind::Performance`, and do NOT add a
  performance clock to `Envelope::Clock` (that couples composition state to a transient player).
  The seam you want already exists: `ConnSource` kinds and `Clock` values are serialized as
  **strings** and the loader maps unknown kinds to `None` and reports them (§2.7) — so a new
  kind or clock value later is *additive*, not a format break. The only thing that WOULD be a
  break later is the shape of `points` — which is why the curve type goes in now.
- The two bridges (LATER, plain curve copies once the type is shared): **Print to knob** — a
  lane gesture → `Envelope { curve rescaled to [0,1], cycleBeats = gesture length, loop, Clock::
  Beats }` on that knob; **Record into take** — a knob's Timeline → a lane gesture. Same editor
  component in both places.

### 9.3 How the hand-drawn curve gets built, and what Boris will SEE that differs

Built **as a connection source** — `ConnSource::Kind::Envelope`, exactly as the connection spec
already specifies for Lane 2/L5 — with recorded lanes as writers. He experiences both as "a
shape over time driving a knob"; the difference that will land on him is *where the shape lives*
and *when it plays*, which is a distinction he already uses (a clip's settings vs a performance):

| | Timeline on a knob (connection source) | Recorded lane (writer, in a take or routine) |
|---|---|---|
| lives in | the knob — saved and copied with the clip; shows in the knob's source picker next to Bass and LFO | the take or routine — shows in the take's lane view, never in the knob's picker |
| plays when | whenever the clip/beat runs; loops per its own setting; restarts on clip trigger; stays until he disconnects it | only while that take/routine is playing; starts when he fires it, ends when it ends; several can stack |
| between/after | it IS the owner — there is no "between" | the knob's own owner (Bass, LFO, or its Timeline) resumes between gestures and after the take |
| grab the knob | he wins; let go → glides back to the curve (Ruling A) | he wins; let go → glides back to the lane's gesture (D8 TOUCH) — same feel |
| range / invert / direction | apply (it is a source shaped by `ConnShape`) | do NOT apply — a lane replays the hand's literal positions; shaping it would replay a different performance than the one recorded. Want it shaped? Print it to the knob and shape the Timeline |
| bridge | "Record into take" | "Print to knob" |

Same curve editor in both places, so drawing and editing feel identical; only the header says
"Ripple › Amplitude › Timeline" versus "Friday take › Layer 2 › Opacity".

### 9.4 The per-control lane model — structural, already done, not a view

Revision 2 already made lanes the **structural on-disk form** (D3: `Take = map<ControlPath,
Lane>`), not an index over an event stream — because Boris's sentence is an *editing*
requirement ("can be changed similar to Ableton"), not a presentation one. A chronological
stream with a per-control index gives the *picture* for free but not the *editing*: every move,
retime or span-delete on one control re-sorts the global list and invalidates the index, and
gestures (begin/points/end on one control) are only implicit. With lanes as storage an edit
touches one vector; a gesture is one object holding one `AutomationCurve` (9.2); cross-lane
ordering — the one thing a stream gives — is needed only at playback and the `Program` compile
produces it by a k-way merge on `(at, seq)` with capture order preserved by the global `seq`
and simultaneity by `group`. The chronological list survives as a derived view
(`Take::chronological()`) for a history list and debugging, never as the authority. D4/D5's
document-vs-program split is about safety; D3's lanes are about editing; they compose.


REPORT_FILE: /Users/boriskarpman/Harmony_Main/memory/.reports/s167-rta-performance-log-and-routines.md
STATUS: COMPLETE
