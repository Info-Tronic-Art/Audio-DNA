# Architecture: the universal connection — "every parameter, same method, all timed"

Session s166 · Architect (Fable) · read-only pass over `/Users/boriskarpman/projects/RealTimeAudio`
at HEAD `6858ec2`. **Revision 2 — converged on the owner's two rulings (§R).** The working tree
was being edited by a teammate while I read it (`git status`: `M src/MainComponent.cpp`,
`M tests/CMakeLists.txt`); my own greps of `MainComponent.cpp` returned different line numbers
minutes apart. Every `MainComponent.cpp` citation is therefore an ANCHOR (function/lambda name),
not a line. All other files were clean; their line numbers are as of `6858ec2`.

Confidence tags: VERIFIED = read in source this session. INFERRED = follows from verified facts
but not itself observed. ASSUMED = stated so a builder can check it; not evidence.

---

## R. Owner rulings applied (DECIDED — not options)

**Ruling A — takeover semantics.** Verbatim: "If there is a signal going, and I start to slide it,
Latest input takes precedence, so my sliding will control it. The moment I let go of the slider,
the signal takes over." → A connected parameter is driven by its signal continuously; any manual
input takes ownership for as long as it is active; on release the signal resumes. There is no
"replace vs modulate-around-a-centre" mode: the signal REPLACES (sweeps the chosen sub-range of
the slider) and the human interrupts it. The earlier revision's `mode`/`depth` fields are deleted.
The takeover is a MODEL concept (§2.3, the "grip"), not a widget concept.

**Ruling B — what a connection is.** Verbatim: "any parameter at all which is anything that has a
slider can be connected to a signal. A signal is just generic for anything we could connect to it
whether it is one of the audio channels or it is a frequency we generate inside of the application
that is timed to the bpm. And specifically within the parameter once we connect to a specific
signal, we could have some controls on whether the signal, let's say it goes from 0 to 100, if that
signal controls the entire range of the slider or just a part of it, or it is [in]versed, or it
plays in a loop or plays ping-pong back-and-forth or if it plays backwards." →
(1) SCOPE: every slider (continuous value). Toggles and dropdowns are OUT of scope — not designed.
(2) SIGNAL = audio channel OR internally generated frequency TEMPO-LOCKED to BPM. "Timed" means
tempo-locked RATES. Attack/release/smoothing are NOT owner-ruled: the earlier `attack/release/
timeUnit` fields are deleted; one optional smoothing field remains as MY recommendation, default
OFF (§2.5, flagged).
(3) PER-CONNECTION CONTROLS (his words): RANGE (signal sweeps the whole slider travel or a
sub-range → target min/max), INVERT, PLAYBACK (forward / backward / loop / ping-pong).
(4) Curves: not mentioned by him; kept as MY call, default Linear (§2.5, flagged).

Also folded in (team-lead verification): the four render-dead fields are written by the UI AND by
live MIDI bindings AND (`masterOpacity`) published over HTTP while no renderer reads them — three
surfaces advertise them as working. Their lane (§5 L4b) is a RENDERER feature, not a modulation
feature, and is stated as such.

---

## 0. Corrections to the recon (build on its evidence; these are the deltas)

C1. **Connections do not exist outside a widget instance.** Two greps of different shape:
    (a) `grep -rn "setSourceMode\|onSourceChanged\|setSourceName" src/` minus the widget's own
    files → only `AudioEngine::setSourceMode` (unrelated); (b) `grep -rn "sourceMode\|SourceMode"
    src/model src/core src/ui/PresetManager.cpp` → nothing. A connection made with the triangle
    picker lives ONLY in `UniversalParamControl::sourceMode_/sourceName_`
    (`UniversalParamControl.h:116-117`). `EffectStackView::rebuildRows()` destroys every row and
    constructs fresh controls (`EffectStackView.cpp:296`, `:394`) and runs on every `setEffects()`
    (`:130-134`) — which `ClipInspector::setClip` calls on every clip selection
    (`ClipInspector.cpp:751`); `buildSourceParamControls()` does the same for source params
    (`:766-767`). Nothing serializes a connection (`Clip::toVar` writes `paramValues` only,
    `Clip.cpp:73-88`). **Consequence: today's "working" mechanism modulates only the
    currently-inspected clip/layer, and every connection is silently destroyed by clicking
    another clip, adding/removing an effect, or undo/redo** (`EffectCommands.h:69-71` documents
    the re-point/rebuild). VERIFIED.

C2. **`tickFeaturePipeline()` never evaluates signals itself.** Body (anchor
    `void MainComponent::tickFeaturePipeline`): reads the bus, runs `MappingEngine::processFrame`,
    `globalMacroBank_.updateValues(signalRegistry_)`, `inspectorPanel_->tickModulation()`.
    `updateValues` and both tick loops only READ `cachedValues_`. The two evaluators are the GL
    thread (`Renderer.cpp:231`) and `SignalBar::timerCallback` (`SignalBar.cpp:117`). Removing both
    without adding evaluation to the tick freezes every macro and connection. VERIFIED.
    (Corollary: preview GL detached AND signal bar hidden ⇒ signals already freeze today.)

C3. **System 4 is the live master-bus effects pass.** `Renderer::renderOpenGL` composites the deck
    into `sourceTexture` (`Renderer.cpp:449-455`) then falls through to `effectChain_.render(
    sourceTexture, …)` (`:504`) before `applyCompTransform` (`:512`). The v1 chain (all library
    effects loaded once, all disabled, `:1693-1718`) is applied to the composited output every
    frame; `MappingEngine` drives its params from the 120 Hz tick. recon-timing's "no global
    effects render" is true of `Composition::globalEffects` only. VERIFIED.

C4. **Dead-4 writers, complete list** (two grep shapes: field-name assignment; `opacity` callers):
    `masterOpacity` ← CompositionInspector, OSC `onSetMaster`, MIDI `Binding::Action::MasterOpacity`
    (both anchors in `MainComponent.cpp`), and is REPORTED by REST (`ApiServer.cpp:266`).
    `clipOpacity` ← ClipInspector + two MIDI sites (velocity-normalized, anchor
    `clip->clipOpacity = value`). `compOpacity`, `masterSpeed` ← CompositionInspector only.
    Render consumers: none for any of the four (grep outside inspector/model → writers and
    serialization only). VERIFIED.

C5. **`RoutingEngine` and `MappingEngine` never compete in production.** Both target
    `effectChain_` (`Renderer.cpp:238-241`; `MappingEngine::processFrame`), but routes are always
    empty in the shipped app (`addRoute` sole caller `TestServer.cpp:828`; second shape:
    `routingEngine_` in `ApiServer.cpp` is ctor/member only, `:35`, `ApiServer.h:112`).

C6. **Clip ids are advisory and re-minted on every load** (`CompositionLoad.h:61-72`,
    `compload::remintClipIds`). Layer ids restart at 0..2 per deck (`Deck.h: initDefault`) and
    `Route` carries no deck id (`Route.h:19-24`). A global route table keyed by ids cannot survive
    save/load here. This decides the data-model shape (§2.2). VERIFIED.

C7. **Mechanism count, bounded.** Criterion: "evaluates an audio/tempo/macro source every tick and
    stores into a parameter". Three grep shapes (`getCachedValue(` callers; `extractSource(`
    callers; files calling `featureBus.read()`): **exactly 4** parameter-modulation mechanisms (v1
    `MappingEngine`; v2 `RoutingEngine`; the two `tickModulation` loops as one mechanism in two
    copies; `MacroBank` is a source multiplexer inside #3). **Plus one audio→video path that is not
    a connection:** shaders read audio directly as uniforms (`CompositorEngine::uploadAudioUniforms`,
    `.cpp:1418-1433`: `u_rms`, `u_bass`, `u_beatPhase`; `EffectChain.cpp:322-330`). Some effects
    react to audio with no connection; that stays. External control writers (MIDI/OSC/REST/
    keyboard) write MANUAL values — under Ruling A they are grips (§2.3), not modulation.

C8. Line drift only: `tickFeaturePipeline` 3016→3056; `EffectsRackPanel` ctor 451→~465; MIDI
    clipOpacity 5655/5691→5711/5747 (moved again during this session).

---

## 1. THE VERDICT

**One new, model-resident connection type; one engine; the proven math from v1 kept as pure
functions; the four runtime paths retired in a fixed order. Compute on the message thread (a
dedicated control thread later is permitted by the design); publish through per-parameter
atomics; the GL threads only read.**

In owner terms: the app has four half-finished ways to make audio move a control. None remembers
what you connected once you click somewhere else, none can reach the 21 position/scale/opacity
controls, and the only one with the response curves is hidden. We make ONE way, store the
connection **inside the thing it controls** (saved with the clip, copied with the clip, undone
with the clip), and make every slider — effect knobs, source knobs, macro knobs, clip/layer/
composition transform and opacity — use exactly that one way. While you touch a slider you own
it; let go and the signal takes over again.

### 1.1 Why not adopt `RoutingEngine` (v2)

- Global-table addressing keyed by ids this codebase deliberately keeps unstable (C6).
- GL-thread compute. This repo already moved v1 OFF the GL thread on purpose (W5:
  `Renderer.cpp:247-252`, `MappingEngine.h:21-26`) so params keep moving while the preview GL
  context is detached; there are two GL contexts (`OutputRenderer`, `OutputWindow.h:17`), neither
  guaranteed alive. The only always-alive cadence is the 120 Hz message timer
  (`MainComponent.h: kMappingTickHz`). `ARCHITECTURE_V2.md:832,837` says "render thread applies
  routes"; this design overrides that line for the W5 reason.
- Per-route state in parallel vectors (`RoutingEngine.h:51-53`) — breaks under copy/undo.
- No curves; its extras (threshold/gain/falloff) are not owner-ruled and are DROPPED (§2.5).

### 1.2 Why not keep `MappingEngine` (v1)

Targets `Effect` objects in `Renderer::effectChain_` by chain index, not the model
(`MappingEngine.cpp:176`); knows nothing of clips/layers/scalars; separate preset persistence.
Its VALUE — `CurveTransforms`, `extractSource` (also used by `AudioSignal.h:17`), the D1 key-based
preset resolution (`PresetManager.cpp:311-406`) — is reused as-is.

### 1.3 Why not extend `tickModulation`

C1. Widget state. Extending to 21 more widgets triples the code and keeps every defect.

### 1.4 Disposition table

| Thing | Disposition | When |
|---|---|---|
| `EffectStackView::tickModulation`, `ClipInspector::tickModulation` (+ pass-throughs) | RETIRED → `ConnectionEngine::tick` | Lane 3 |
| `UniversalParamControl` source/range/invert STATE | RETIRED as state; widget becomes a view/editor of a `ParamConnection*` and a grip caller | Lane 3/4 |
| `RoutingEngine`, `Route`, `RouteTarget`, `Renderer::routingEngine_` + `processFrame` call | RETIRED; `TestServer` route endpoints re-pointed to the model | Lane 3 (dead call removed in Lane 1) |
| `MacroBank::MacroLink` (uses `RouteTarget`), `Macro::sourceSignalId` | RETIRED if `.links` has zero writers (builder: two greps); macro feed becomes `Macro::conn` (§2.2) | Lane 3 |
| `SignalRegistry::evaluateAll` calls at `Renderer.cpp:231`, `SignalBar.cpp:117` | RETIRED; single evaluator in the tick | Lane 1 |
| `MappingEngine::processFrame` + `mappings_`, `EffectsRackPanel`, `MappingEditor` | RETIRED; v1 presets CONVERTED on load (§1.5) | Lane 7 |
| `MappingEngine::extractSource`, `applyCurve`, `CurveTransforms.h`, `Smoother.h` | KEPT (pure) | — |
| `Renderer::effectChain_` (v1 chain) | KEPT for now, driven by the ONE engine via the master-chain family (§3.4); deletion belongs to the OutputWindow arc | Lane 7 (drive) |
| `Composition::globalEffects` (UI+undo+JSON, no renderer) | BUILT: rendered post-composite where `effectChain_.render` runs | Lane 7 |
| `SignalRegistry`, `OscillatorSignal`, `EnvelopeSignal`, `MacroBank` | KEPT; message-thread-confined | Lane 1 |
| `ClipPositionSignal` (zero callers) | RETIRED; ClipPosition becomes a source kind reading the owning clip | Lane 5 |

### 1.5 Presets already on disk — no silent loss

(1) **Composition files** (`Composition::toVar`, no version key, `Composition.h:175-182`): contain
NO connections today (C1) — nothing to migrate; new keys are additive and older builds ignore
unknown properties (`PresetManager.cpp:182-185` states the policy). (2) **v1 presets / deck files**
(`…/AudioDNA/Presets`, `FX Saves`, `Decks` — `PresetManager.cpp:445-470`) carry `"mappings"` with
source, curve, inputMin/Max, outputMin/Max, smoothing, and D1 keys (`:130-138`). Lane 7 keeps
`loadPreset` parsing unchanged and CONVERTS each resolved `Mapping` to a `ParamConnection`:
source→`Signal(name of the AudioSignal wrapping that MappingSource)`, curve preserved,
inputMin/Max→`inMin/inMax`, outputMin/Max→`outMin/outMax`, smoothing alpha→`smoothingMs` (§2.5
parity table). **Multiple v1 mappings on one param** (v1 summed them, `MappingEngine.h:18-19`)
→ the first converts, the rest are reported through the existing `LoadStats.droppedDescriptions`
path (anchor `Preset-retarget-fix W5`); the model has exactly one connection per parameter by
construction (§2.6). Files are never rewritten on load; save writes the new form alongside the
legacy keys for one release.

---

## 2. THE CONNECTION AS A DATA MODEL

New directory `src/connect/` (matches `src/mapping/`, `src/routing/`). Model-level, no JUCE UI,
no renderer includes — links into headless tests like `EffectScope.h` (`EffectScope.h:14-16`).

### 2.1 The struct (converged)

```cpp
// src/connect/ParamConnection.h
struct ConnSource {
    enum class Kind : uint8_t { None, Signal, Macro, Lfo, Envelope, ClipPosition };
    Kind kind = Kind::None;
    std::string signalName;       // Kind::Signal — PERSISTENT key (registry ids are minted per
                                  // process, SignalRegistry.cpp:14,33,65,70,77 — never saved)
    int macroIndex = -1;          // Kind::Macro (0..MacroBank::kNumMacros-1)
    struct Lfo {                  // Kind::Lfo — "a frequency we generate … timed to the bpm"
        enum class Shape : uint8_t { Sine, SawUp, Triangle, Square, SampleHold };   // SawDown = SawUp + Backward
        Shape shape = Shape::Sine;
        float cycleBeats = 1.0f;  // 0.25 … 16; tempo-locked by definition (Ruling B)
        float phaseOffset = 0.0f; // [0,1)
        float pulseWidth = 0.5f;  // Square
    } lfo;
    struct Envelope {             // Kind::Envelope — inline keyframes; the picker's "Timeline" (owner D6)
        std::vector<std::pair<float,float>> points;   // (position 0..1, value 0..1), sorted
        enum class Clock : uint8_t { Beats, ClipPosition } clock = Clock::Beats;
        float cycleBeats = 4.0f;
    } env;
};

struct ConnShape {
    // ---- OWNER-RULED (Ruling B.3) ----
    float outMin = 0.0f, outMax = 1.0f;   // RANGE: which part of the slider's travel the signal sweeps
    bool  inverted = false;               // INVERT
    enum class Playback : uint8_t { Forward, Backward, PingPong } playback = Playback::Forward;
    bool  loop = true;                    // false = play once per trigger (§2.4)
    // ---- ARCHITECT'S CALL, not owner-ruled; neutral defaults = owner's literal behaviour ----
    uint8_t curve = 0;                    // MappingCurve (Linear). Reason: repo's only complete shaping
                                          // math; needed for lossless v1 preset conversion; zero cost at Linear.
    float inMin = 0.0f, inMax = 1.0f;     // input window (v1 normalization). Needed for v1 conversion parity.
    float smoothingMs = 0.0f;             // 0 = OFF (owner's literal ruling). Recommendation: expose
                                          // as "Smooth" for audio channels only; see §2.5.
};

struct ParamConnection {
    ConnSource source;
    ConnShape  shape;
    bool enabled = true;
    bool isConnected() const { return source.kind != ConnSource::Kind::None; }

    // ---- The GRIP (Ruling A): model-level "latest input wins, automatic hand-back" ----
    struct Grip {
        enum class Kind : uint8_t { None, Held, Decaying };  // Held = has an explicit release
                                                             // (a dragged slider); Decaying = no
                                                             // release event exists (MIDI CC, OSC,
                                                             // HTTP, note velocity) — expires after
                                                             // gripHoldMs without a refresh
        Kind kind = Kind::None;
        double lastTouch = 0.0;                              // engine time, seconds
    } grip;
    void gripHeld();                      // slider mouse-down / touch-begin
    void gripTouch(double now);           // every manual write from a release-less input
    void release(double now);             // slider mouse-up (also used by the model on disconnect)

    // Runtime state; INSIDE the struct so it copies/undoes with the connection (never a
    // parallel vector — the v2 smoothers_ shape). Not serialized.
    struct State {
        float smooth = NAN;               // smoother memory (only if smoothingMs > 0)
        float handBackFrom = NAN; double handBackStart = 0.0;   // glide on release (§2.3)
        float shValue = 0.0f; int shCycle = -1;                // Sample&Hold
        double onceStartBeats = -1.0;     // loop==false: cycle start (beats) since last trigger
        uint32_t cachedSignalId = 0;      // name→id cache, validated each tick
    };
    mutable State state;
};
```

Picker item → model (UI-independent, survives the UI rewrite): `Manual→None`;
`Audio/Oscillator/Envelope (registry)→Signal(name)`; `BPM Sync(shape,division)→Lfo`;
`Macro N→Macro(N-1)`; `Clip Position→ClipPosition`; `Timeline→Envelope`.

### 2.2 Where connections live (co-located with the target — decided by C6)

| Target family (every slider) | Manual value (existing, message-thread owned, serialized) | Connection | Live twin (GL reads) |
|---|---|---|---|
| Effect param | `Clip::EffectSlot::paramValues[p]` (`Clip.h:51`) | `EffectSlot::paramConns[p]` (parallel, same size) | `EffectSlot::paramLive[p]` |
| Effect dry/wet | `EffectSlot::dryWet` | `EffectSlot::dryWetConn` | `EffectSlot::dryWetLive` |
| Source param | `Clip::SourceParam::value` (`Clip.h:42`) | `SourceParam::conn` | `SourceParam::live` |
| Clip scalar (7) | `clipOpacity, positionX, positionY, scale, rotation, anchorX, anchorY` | `Clip::scalarConns[ClipScalar]` | `Clip::scalarLive[ClipScalar]` |
| Layer scalar (7) | `opacity, positionX, positionY, layerScale, layerRotation, layerAnchorX, layerAnchorY` | `Layer::scalarConns[LayerScalar]` | `Layer::scalarLive[LayerScalar]` |
| Composition scalar (9) | `masterOpacity, masterSpeed, compOpacity, compPositionX/Y, compScale, compRotation, compAnchorX/Y` | `Composition::scalarConns[CompScalar]` | `Composition::scalarLive[CompScalar]` |
| Macro knob (8) | `MacroBank::Macro::manualValue` (`MacroBank.h:31`) | `Macro::conn` (replaces `sourceSignalId`, `:35`; `MacroPanel.cpp:136,144` set it today) | `Macro::currentValue` (message-thread only — macros are consumed by the engine, not GL) |
| Master-chain param (system 4, until the OutputWindow arc) | `EffectParam::value` in `Renderer::effectChain_` | `Composition::masterChainConns[] = {shaderName, uniformName, ParamConnection}` | writes `EffectParam::value` (existing A2 crossing) |

The macro knob is itself a slider that can be connected (ARCHITECTURE_V2.md:354 already says so);
making it the same struct removes a special case and gives macros the grip for free (MIDI
`AdjustMacro` writes `manualValue`, anchor `globalMacroBank_.getMacro(...).manualValue = value`).
Macros tick BEFORE parameters (they are sources for parameters).

Layer has two widgets ("Master" and "Opacity") writing ONE field (`LayerInspector.cpp:149,186`);
the table has one entry. Composition "Master" vs "Opacity" — owner D7.

Scalars use a descriptor table so engine, serializer and ANY future UI share one definition
(formulas VERIFIED at the cited lines):

```cpp
// src/connect/ScalarParams.h
enum class ClipScalar : uint8_t { Opacity, PosX, PosY, Scale, Rotation, AnchorX, AnchorY, Count };
struct ScalarDef { const char* key; float (*toModel)(float norm); float (*toNorm)(float model); float defaultNorm; };
// Clip (ClipInspector.cpp:336,404-408; 1242-1255):
//  Opacity v | PosX (v-0.5)*3840 | PosY (v-0.5)*2160 | Scale 2^((v-0.5)*2) | Rotation (v-0.5)*720
//  AnchorX (v-0.5)*3840 | AnchorY (v-0.5)*2160  <- NEW: no UI ever wrote anchorY (recon §4)
// Layer (LayerInspector.cpp:149,399-403): same formulas on its seven fields
// Composition (CompositionInspector.cpp:138,146,155,173-178):
//  Master v | Speed v*4 | Opacity v | PosX (v-0.5)*3840 | PosY: builder verifies (ASSUMED (v-0.5)*2160)
//  Scale v*2 (linear — unlike clip/layer's 2^; pre-existing, kept, flagged) | Rotation (v-0.5)*720
//  AnchorX (v-0.5)*3840 | AnchorY: UI FORCES 0 (`:178`) — table gives it a real formula
float& manualRef(Clip&, ClipScalar);   // switch — the only place that names the fields
```

### 2.3 The grip — Ruling A at the model layer

Definitions (all on the message thread; every manual writer already lands there — MIDI via
`callAsync` `MidiHandler.cpp:80-97`, OSC via `MessageLoopCallback` `OscHandler.h:26-27`, HTTP via
`callAsync` `ApiServer.cpp:408`, sliders natively):

- **Touch** = any manual write to a connected parameter's manual field. Two flavours, chosen by
  the WRITER, not the widget: `gripHeld()` when the input has a release event (a dragged slider,
  a touch surface); `gripTouch(now)` when it does not (MIDI CC absolute/relative, note velocity,
  OSC, HTTP `set_param`/`set_layer_opacity`). A `Decaying` grip is refreshed by every write and
  expires `gripHoldMs` after the last one (global setting, default 250 ms — owner D15).
- **Release** = `release(now)` (slider mouse-up) or expiry of a `Decaying` grip. `Held` grips never
  expire (a user holding a slider still for ten seconds must keep it). Safety: the model
  mutator that disconnects a parameter releases; a future UI must pair begin/end (debug `jassert`
  if a component dies holding a grip). Undo/redo and preset load clear all grips.
- **While gripped**: the engine does not publish for that parameter and stores `NAN` into its live
  twin once at grip start → the renderer falls through to the manual field, which the slider is
  writing directly (zero added latency, no engine in the loop). The manual field therefore ends
  up holding the human's last value — that is what the parameter returns to when disconnected.
- **On release — glide, not snap (RECOMMENDED, open, non-blocking; owner D14)**: the engine
  resumes publishing from the gripped value and glides to the signal's current value over
  `handBackGlideMs` (global setting, default 120 ms; 0 = snap). Reason: snapping is the literal
  reading of the ruling and is fully predictable, but a discontinuity at mouse-up is visible on
  the output; 120 ms is imperceptible as lag. A per-connection override is NOT provided (keeps
  the struct to the owner's list); flip the global to 0 if he prefers the snap.
- **Two non-human writers on one parameter**: (i) two SIGNALS — impossible by construction, one
  `ParamConnection` per parameter (§2.6); combining signals is a job for a mixer SIGNAL (or a
  macro fed by one signal driving several parameters), not for two connections; (ii) a signal
  and a MIDI/OSC/HTTP stream — the stream is a `Decaying` grip: latest input wins while it keeps
  writing, the signal resumes `gripHoldMs` after it stops. Same rule, one level down. (iii) two
  manual streams (slider + MIDI) — whoever wrote last owns it; a `Held` slider grip is not
  displaced by a decaying one (the hand on the slider is the more deliberate input), a decaying
  grip IS displaced by a `Held` one. Non-manual model writers (autopilot, renderer) never write
  connectable fields (VERIFIED: they write `playing`, `playheadPosition`, `activeClipColumn`).

Grip sites the builder must instrument (each already writes the field; add one `gripTouch`):
`oscHandler_.onSetMaster`, `oscHandler_.onSetLayerOpacity`, MIDI `MasterOpacity`,
`AdjustLayerOpacity`, `AdjustMacro`, the two velocity→`clipOpacity` writes (all anchors in
`MainComponent.cpp`), `LayerStrip.cpp:423` (layer strip opacity slider — `gripHeld`/`release`),
`ApiServer.cpp:433` (`set_param`) and `:499` (`set_layer_opacity`). **At least 9**; the builder
re-greps `clipOpacity = |masterOpacity = |->opacity = |paramValues\[.*\] = ` before claiming the
set is complete. Centralize: one `touchScalar(Clip&, ClipScalar, float model, GripKind)` per
struct, so a writer cannot forget the grip.

### 2.4 Semantics of the pipeline (one function, unit-tested; the lanes' oracle)

```
-- source value, by kind --
Signal(name)   raw = registry cached value (audio channel, or shared Mod 1/Mod 2 value)
Macro(i)       raw = macros[i].currentValue (already ticked this frame)
Lfo            phase = frac((beatsNow / cycleBeats) + phaseOffset)          beatsNow = beatPhase + beatInBar + 4*barCount
               phase = playbackXform(phase)                                   (below)
               raw   = shape(phase)   Sine 0.5+0.5sin | SawUp phase | Triangle fold | Square (phase<pw) | S&H per cycle
Envelope       pos   = clock==Beats ? playbackXform(frac(beatsNow / cycleBeats)) : playbackXform(clipPos)
               raw   = piecewise-linear(points, pos)
ClipPosition   raw   = playbackXform(owning clip's playhead 0..1 over in/out)
-- playback (Ruling B.3 iii) --
playbackXform(p): Forward p | Backward 1-p | PingPong (frac(p2)<0.5 ? 2*frac(p2) : 2-2*frac(p2)) with p2 = p_cycles/2
                  loop==false: cycle runs once from onceStartBeats (set on clip trigger for clip-scope
                  targets, on enable/connect otherwise) and then HOLDS its end value
   APPLIES TO: Lfo, Envelope, ClipPosition (phase-based sources).
   IGNORED FOR: Signal (audio channels have no direction; shared Mod 1/Mod 2 keep their own
   direction in OscillatorSignal — edited in one place) and Macro. The field is stored and
   serialized regardless (one struct); a UI greys it for those kinds. Stated honestly: this is the
   one control that is not uniform across kinds.
-- shaping (owner-ruled + neutral-default extras) --
w   = clamp((raw - inMin) / (inMax - inMin))            (v1, MappingEngine.cpp:222-225)   [neutral: 0..1]
c   = CurveTransforms::applyCurve(curve, w)             (v1, :139-142)                    [neutral: Linear]
c   = inverted ? 1 - c : c                              (INVERT)
y   = outMin + c * (outMax - outMin)                    (RANGE — sub-range of the slider's travel)
y   = smoothingMs > 0 ? ema(y, tau = smoothingMs) : y   [neutral: off]  (parity: alpha/tick = 1-exp(-dt/tau))
-- takeover (Ruling A) --
if grip.active: publish nothing (twin holds NAN → renderer reads the manual field)
elif handing back: y = lerp(handBackFrom, y, (now - handBackStart)/glide), then clear when done
live = toModel(y) for scalars | y for effect/source/macro params
```

`beatsNow` includes `4*barCount` so cycles longer than a bar do not fold; the registry oscillator
folds at one bar today (`OscillatorSignal.h:28-30`, INFERRED bug for `beatDuration > 4`) — fix it
in Lane 1 (two lines). Beat phase advances only while the audio device delivers hops
(`AnalysisThread.cpp:63-73`, `BPMTracker.cpp:185`); with tap/manual BPM the phase runs from the
locked BPM without stabilization (`BPMTracker.cpp:74-79,459-469`). Owner D3c.

### 2.5 The fields that are mine, not his (flagged)

- `curve` — keep. The repo's only complete shaping math (`CurveTransforms.h`, 24 curves), needed
  so v1 presets convert losslessly (D9), free at Linear. If the owner never wants it, the field
  stays hidden at Linear; nothing else changes.
- `inMin/inMax` — keep at 0..1; exists only for v1 conversion parity. Hidden.
- `smoothingMs` — keep, default 0. Recommendation: expose ONE "Smooth" control for audio-channel
  sources only. Reason: the analysis hop is 10.67 ms (`FeatureBus.h:119`) and RMS/band energies
  jitter frame to frame; today's tick loops are raw passthrough and the owner has watched them
  without asking for smoothing, so the default is off — but scale/position driven by raw bass
  visibly shakes, and both v1 and v2 shipped with smoothing for that reason. Not owner-ruled.
- DROPPED from the earlier revision: `mode/depth` (Ruling A), `attack/release/timeUnit`
  (Ruling B.2), `threshold/gain/falloff/dialMin/dialMax` (v2 extras with zero production users
  and no owner mention; their only tests are in the dead `RoutingEngine` suite).

### 2.6 Invariants

- Exactly one `ParamConnection` per parameter (a field, not a list). Enforced by the type.
- Toggles/dropdowns (`EffectSlot::enabled/bypassed`, `Layer::visible/solo`, blend/keying modes,
  transport modes) are NOT connectable — out of scope per Ruling B.1.
- Signal names are the persistent key; `initDefaults` names must be unique (they are today:
  "Hit"/"Hit Strength" are distinct names — builder adds a `jassert` on duplicates).

### 2.7 Serialization (additive, sparse, forward-tolerant)

```json
"effects":[{"name":"ripple","enabled":true,"bypassed":false,"dryWet":1.0,"params":[0.5,0.3],
  "conns":[{"p":1,"src":{"kind":"lfo","shape":"sine","cycleBeats":1.0,"phase":0.0},
            "shape":{"min":0,"max":1,"invert":false,"playback":"forward","loop":true,
                     "curve":"Linear","inMin":0,"inMax":1,"smoothMs":0},"enabled":true}],
  "dryWetConn":{...}}],
"sourceParams":[{"name":"…","uniformName":"…","value":0.5,"defaultValue":0.5,"conn":{...}}],
"conns":{"opacity":{...},"positionX":{...}},          // Clip / Layer / Composition scalars by ScalarDef::key
"macros":[{"name":"Link 1","manual":0.5,"conn":{...}}],
"connect":{"gripHoldMs":250,"handBackGlideMs":120}    // Composition-level settings
```
Only connected entries are written. Curve names reuse `PresetManager`'s `kCurveNames`
(`PresetManager.cpp:22-31`) moved to `src/connect/ConnSerialization.{h,cpp}`. Loader tolerates
absent keys and unknown kinds (→ `None`, counted and reported like `LoadStats`). A file SAVED by an
older build drops the keys — acceptable, release-note it.

---

## 3. THE TARGET SIDE — how a computed value reaches the renderer

### 3.1 Generic setter = "publish to the live twin"; renderer reads `effective()`

```cpp
// src/connect/LiveValue.h — the SignalRegistry::cachedValues_ template (SignalRegistry.h:50,
// static_assert at SignalRegistry.cpp:3-4) made COPYABLE, because EffectSlot/Clip/Layer are value
// types snapshotted by EffectStackCmd (EffectCommands.h:110), copied by Clip::replaceContent
// (Clip.h:199,217) and held by value in Layer::clips (Layer.h:151).
struct LiveValue {
    std::atomic<float> v{NAN};                 // NAN = "not driven": renderer uses the manual field
    LiveValue(const LiveValue& o) : v(o.v.load(std::memory_order_relaxed)) {}
    LiveValue& operator=(const LiveValue& o) { v.store(o.v.load(std::memory_order_relaxed), std::memory_order_relaxed); return *this; }
    float effective(float manual) const { float x = v.load(std::memory_order_relaxed); return std::isnan(x) ? manual : x; }
};
```
Stored in MODEL units (through `toModel`) so the renderer does no math. Read sites (VERIFIED):

| Today | Becomes |
|---|---|
| `CompositorEngine.cpp:372-376` (`glUniform1f(loc, slot.paramValues[p])`), `:278-279`, `:1338-1341` | `slot.effParam(p)` |
| `:384,409` (`slot.dryWet`) | `slot.effDryWet()` |
| `SourceParam::value` consumers (`:722,828,892,1072` pass the vector to `sourceRenderFn_`; the procedural source's uniform upload in `src/sources/` — ASSUMED one site, builder locates) | `sp.live.effective(sp.value)` |
| Standalone-source path `Renderer.cpp:196-207` (mutex copy fed by `updateActiveSourceParams`) | unchanged; engine fires the same hook after publishing (§4.3) |
| `applyClipTransform` `:442-474` | `clip.eff(ClipScalar::…)` |
| `applyLayerTransform` `:489-519` | `layer.eff(LayerScalar::…)` |
| Layer opacity `:542,573,577,786,805,964` | `layer.eff(LayerScalar::Opacity)` |
| `Renderer::applyCompTransform` `Renderer.cpp:1918-1923` | `composition_->eff(CompScalar::…)` |

### 3.2 The four render-dead fields — a RENDERER lane, not a modulation lane

Three surfaces already advertise these as working (inspector sliders; live MIDI bindings —
velocity→`clipOpacity` ×2, controller→`masterOpacity`; `masterOpacity` published by REST
`ApiServer.cpp:266`) and NO renderer path reads any of them — the renderer reads only
`layer.opacity`. Connecting a signal to them cannot show anything until the renderer consumes
them. Lane L4b is therefore renderer work with its own manual-slider proof per field:

| Field | Consumer site | Spec |
|---|---|---|
| `Clip::clipOpacity` | `CompositorEngine::blendLayerOntoAccumulator` `u_opacity` (`:964`), FX-only constant-alpha blend (`:573-577`) | `u_opacity = layer.eff(Opacity) * clip.eff(Opacity)`. Crossfade (`applyTransition`, `:748`): builder checks whether the transition shader has per-input opacity; if not, bake clip opacity into the clip texture in `applyClipTransform`'s pass (one multiply) so it precedes the transition. |
| `Composition::compOpacity` | after `applyCompTransform` (`Renderer.cpp:512`), before Syphon/capture | final full-viewport pass through `passthrough` with `u_opacity` onto black; fold into `comp_transform` when that pass runs (drop the `isDefault` early-return `:1925-1927` when opacity < 0.999). |
| `Composition::masterOpacity` | same final pass, applied LAST | `u_opacity = compOpacity * masterOpacity` (if D7 keeps both); Master last so it also fades Syphon output and recording (builder confirms `publishSyphonFrame`/`processPendingCapture` run after the new pass — `:497,512` order VERIFIED). |
| `Composition::masterSpeed` | `Renderer.cpp:1186` (`setSpeed(clip->speed)`), `:1222` (image sequence), procedural `time` `:391-393` | Timeline-mode clips: `speed * masterSpeed`. BPM-sync clips (`:1173-1183`): NOT multiplied (D8). Procedural sources: `scaledTime_ += dt * masterSpeed` on the GL thread so a speed change never jumps. Only `Renderer` changes — `OutputRenderer` does not run the compositor (§6). |

Also: `Clip::anchorY`, `Layer::layerAnchorY` are rendered (`:470,515`) but no UI writes them; the
descriptor table makes them real parameters (owner D13).

### 3.3 Effect-param sizing

`paramConns`/`paramLive` are sized with `paramValues` at every creation site: three
`slot.paramValues.push_back(p.defaultValue)` sites in `MainComponent.cpp` (anchor
`existingClip->effects.push_back(slot)` and siblings), `Clip::fromVar` (`Clip.cpp:234-236`),
`Layer::fromVar`, `Composition` `globalEffects` load — ONE helper `EffectSlot::resizeParams(n)`;
builder greps `paramValues.push_back` (at least 4 hits) and every slot-building loader.

### 3.4 Master-chain family (system 4 bridge)

`Composition::masterChainConns` entries resolve `(shaderName, uniformName)` → `Effect*`/param
index via `EffectChain::getEffect` / `Effect::getShaderName` / `EffectParam::uniformName` — the D1
keys `PresetManager.cpp:130-137` writes — cache the index in `state`, and write
`Effect::setParamValue` (`Effect.cpp:16-20`): the existing A2 plain-float crossing read by both GL
contexts (`EffectChain.cpp:389-392`, `OutputWindow.cpp:162`), unchanged, one store per param per
tick (MappingEngine's W4/A3 rule kept). Deleted with `effectChain_` in the OutputWindow arc.

---

## 4. THE THREADING CONTRACT (mandatory)

### 4.1 Ownership

| State | Owner thread | Readers | Mechanism |
|---|---|---|---|
| `FeatureSnapshot` | analysis thread | any | seqlock `FeatureBus::read()` (`FeatureBus.h:8-37,106`) — unchanged |
| `SignalRegistry` (signals, settings, `cachedValues_`) | **message thread only** after Lane 1 — evaluation, `SignalInspector` setters (`SignalInspector.cpp:36-115`), add/remove | `cachedValues_` atomics from any thread (TestServer `:768`); GL threads no longer touch the registry | confinement + `jassert(isThisTheMessageThread())` in `evaluateAll` (A6 precedent, `MappingEngine.cpp:11,150`) |
| `MacroBank` | message thread | engine (message thread) | unchanged |
| `ParamConnection` (source/shape/grip/state) | message thread | **never read by GL** | confinement; every grip writer is already on the message thread (§2.3) |
| `LiveValue` twins | engine (message thread) | GL threads, HTTP (reporting) | relaxed atomic store/load; NAN sentinel |
| Manual fields | message thread | GL | pre-existing plain-float crossing (A2). Not widened; hardening = `LiveValue`-typed fields later |
| Model STRUCTURE (vectors) | message thread, under the fence | GL iterates | `UndoService::withDeckDetached` via `DeckFenceHook` (anchor `MainComponent::makeDeckFence`), as `EffectStackCmd` already does (`EffectCommands.h:99-100`) |

### 4.2 Single owner for `evaluateAll`

Evaluated ONCE per tick, on the message thread, inside `tickFeaturePipeline`, BEFORE macros and
BEFORE `ConnectionEngine::tick`. `Renderer.cpp:231` and `SignalBar.cpp:117` are deleted; `SignalBar`
keeps its repaint timer and reads the cache. Reasons the message thread owns it: (a) W5 detach;
(b) two GL contexts; (c) `Signal` settings are mutated by the UI and read in `getValue` —
GL-thread evaluation is a data race TODAY (`OscillatorSignal.h:73-76`, `EnvelopeSignal.h:119-125`
plain fields; `setPoints` reallocates a vector the GL thread iterates); (d) the consumers are on
the message thread (C2). `Signal.h:8` comment updated.

### 4.3 The engine tick

```cpp
class ConnectionEngine {                          // src/connect/ConnectionEngine.{h,cpp}
public:
    struct Context { const SignalRegistry& signals; MacroBank& macros; const FeatureSnapshot& snap;
                     float dt; double now; float gripHoldMs; float handBackGlideMs; };
    void tick(Composition& comp, const Context& ctx);          // message thread only (jassert)
    static float evaluate(ParamConnection& c, float manualNorm, const Context& ctx, const ClipClock* clock); // pure, tested
    std::function<void(const Clip*)> onSourceParamsPublished;  // → Renderer::updateActiveSourceParams
};
```
Walk order: macros → `comp.scalarConns` → `comp.globalEffects` → `comp.masterChainConns` → per
deck: layer scalars + `layerEffects` → per clip cell with a value: scalars, `effects[].paramConns`,
`sourceParams[].conn`. O(all parameters) at 120 Hz — a few thousand `isConnected()` checks; only
connected ones evaluate; expired `Decaying` grips are released here. No allocation in `tick`
(`MappingEngine.h:26` rule). Only the message thread mutates structure (under the fence, same
thread as the timer), so the walk cannot interleave with a reallocation. `dt` = measured delta,
clamped to [0, 50 ms]. Disconnect = the model mutator stores NAN into the twin and releases the
grip; the engine skips `!enabled || kind == None`.

Strongest counterargument to my own recommendation: **a blocked message thread (file dialog,
heavy repaint, a synchronous `executeOnGLThread` round-trip like `Renderer.cpp:820,896,931`)
freezes every connection.** `tickModulation` has that property today, so it is not a regression,
and the design's inputs (snapshot seqlock, registry atomics, macro values) and outputs (twins)
are thread-agnostic: the tick can move to a dedicated high-priority control thread later with ONE
change (edits and grips become queued messages). GL-thread compute can never be fixed the same
way — bound to a context that can detach and to a frame rate. "Message thread now, control
thread later, GL thread never."

### 4.4 The hazard that fixes the lane ORDER (Lane 0)

`CompositorEngine.cpp:761-763` builds a temporary `Clip` and does `layerFxClip.effects =
layer.layerEffects` **on the GL thread every frame** — a whole-vector copy of `EffectSlot`s the
message thread edits (bypass toggle `EffectStackView.cpp:320` is NOT fenced; only erase is,
`:344`). Today the payload is `std::string effectName + vector<float>`: already a race, "works"
because a torn float is harmless. Once `EffectSlot` carries a `ParamConnection` (`std::string
signalName`, envelope `vector`), that copy can observe a string mid-reassignment → heap
corruption. **Lane 0 removes the per-frame copy before any connection state enters `EffectSlot`**:
`applyClipEffects` takes `const std::vector<Clip::EffectSlot>&` (it only reads `clip.effects` —
ASSUMED from `:232-244`, builder verifies) and receives `layer.layerEffects` by reference.
`EffectStackCmd::apply`'s `*vec = snapshot` (`EffectCommands.h:110`) is fenced and stays.

### 4.5 Cross-parameter coherence

Not guaranteed, not needed (≤ 8.3 ms skew between params). Upgrade path if ever wanted: a
per-clip seqlock snapshot of its live block — additive.

---

## 5. SEQUENCED LANES (ORDER IS A CLAIM — justified per step; UNSAFE orderings named)

Size S/M/L ≈ <1 / 1–3 / 3+ builder-days; risk L/M/H.

**L0 — GL copy removal (S, L).** §4.4. Proves: ctest + TSan app run (build-tsan recipe, gotchas
2026-08-03) while spamming bypass toggles. Unblocks: any state inside `EffectSlot`. MUST precede L2.

**L1 — Single evaluator (S, L).** In `tickFeaturePipeline`: `snap` → `signalRegistry_.evaluateAll(snap)`
→ `updateValues` → rest. THEN delete `Renderer.cpp:229-231`, `SignalBar.cpp:116-117`; add the
message-thread `jassert`; update `Signal.h:8`; fix the one-bar fold in `OscillatorSignal.h:28-30`
and `EnvelopeSignal.h:36-37` (add `4*barCount`). Delete the dead `routingEngine_.processFrame` call
(`Renderer.cpp:233-245`) after grepping `add_route` in `tests/` (if an e2e depends on it, the
deletion moves to L3). Proves: unit test "cached value changes only after an explicit tick"; app
gate: signal-bar meters move with the preview hidden. UNSAFE reversed inside L1 (C2). Independent
of L0.

**L2 — Model + engine core, headless (M, M).** `src/connect/{ParamConnection.h, LiveValue.h,
ScalarParams.h, ConnectionShaper.{h,cpp}, ConnectionEngine.{h,cpp}, ConnSerialization.{h,cpp}}`;
`EffectSlot::paramConns/paramLive/dryWetConn/dryWetLive` + `resizeParams`; `SourceParam::conn/live`;
scalar arrays + `eff()`/`manualRef()` on Clip/Layer/Composition; `Macro::conn`; grip API;
Composition `connect` settings; JSON. New ctest target `test_connection` following
`tests/CMakeLists.txt:243-276`. Tests: (a) shaper parity — v1 curve cases
(`test_mapping_engine.cpp:463-525`) reproduced bit-for-bit (Linear/inMin/inMax/outMin/outMax/
smoothing alpha→ms table); (b) LFO phase at bar 0/1/2 for 1, 4, 8, 16 beats; each `Playback`
transform; `loop=false` holds its end value; (c) INVERT and RANGE sub-range; (d) GRIP: held grip
suspends publishing (twin reads NAN), decaying grip expires at `gripHoldMs`, held displaces
decaying and not vice-versa, release glides over `handBackGlideMs` and snaps at 0; (e)
`EffectStackCmd` undo round-trip preserves connections and clears grips (extend
`test_undo_commands.cpp`; gotcha 2026-09-05: sweep `tests/` for ctor changes); (f) JSON: old file
loads; unknown kind → None and reported; (g) engine tick over a bare `Composition` publishes
into twins, macros tick first, NAN on disconnect. Requires L0.

**L3 — VERTICAL SLICE: effect params on any clip, tempo-locked, visible (M–L, M-H).**
(a) Renderer reads `effParam/effDryWet` and the source-param upload site. (b) `tickFeaturePipeline`
calls `ConnectionEngine::tick` after `updateValues`; wire `onSourceParamsPublished` →
`Renderer::updateActiveSourceParams`. (c) Retire both `tickModulation` bodies + pass-throughs;
`UniversalParamControl` gains `bind(ParamConnection*, const LiveValue*)`, its picker mutates the
bound connection via the host's `onPerformEdit` → `EffectStackCmd` (Cmd-Z removes a connection
for free), its slider calls `gripHeld()` on mouse-down and `release()` on mouse-up, it paints the
twin's value on the host's existing refresh. `SourceMode`/`sourceName_` deleted. (d) `TestServer`:
replace `add_route/remove_route/list_routes` (`TestServer.cpp:~790-870`) with
`add_connection/remove_connection/list_connections` addressed by `(layer, column, effectName,
paramName)` like `set_param` (`ApiServer.cpp:408-440`), plus `grip/release` test endpoints; delete
`RoutingEngine`/`Route.h`/`MacroLink` (after the `.links` writer grep) and `routingEngine_` in
`Renderer`, `ApiServer`, `TestServer`; `MacroPanel` sets `macro.conn` instead of `sourceSignalId`.
(e) Instrument the ≥9 grip sites (§2.3).
Proves: L2 units; e2e on 8080 (`--test-mode`, `localhost`, gotchas): `load_source` → add `ripple`
→ `add_connection {lfo sine 1 beat → amplitude, min 0.2 max 0.9}` → inject `beatPhase` 0.0 / 0.5
→ `render_frame` ×2 → PSNR finite; `{signal "Bass"}` with `bandEnergies[1]` 0 vs 1 → same;
`grip` → `set_param 0.3` → frames identical at both phases → `release` → frames differ again.
Manual gate (Boris present, TCC prompt): connect Bass → Ripple.amplitude on clip A, click B,
click A — still connected; drag the slider — it follows the hand, let go — the bass takes over;
save/quit/load — still connected. UNSAFE inside L3: (c) before (a)+(b) freezes every connected
param (the L9 class); (d)'s deletion before the endpoint re-point breaks the test-server build.

**L4 — Scalar targets: the 21 controls + macro knobs (M, M).** Renderer read sites §3.1 rows 5–8;
inspectors bind their 21 controls to `scalarConns/scalarLive`; `MacroPanel` binds knobs to
`Macro::conn`; `ScalarConnCmd` (before/after value of one `ParamConnection`, coordinate-resolved
like `EffectStackCmd`; no fence — no reallocation, GL never reads the struct). First demos:
**Layer Opacity ← Square 1 beat** (strobe), **Clip Scale ← Bass, range 0.4–0.6**. Proves: e2e +
manual gate incl. the MIDI grip (turn a mapped controller while the signal runs: knob wins, then
hands back). Requires L2; parallel with L3.

**L4b — The four dead render fields (S each, L–M). RENDERER lane.** Four commits, each proven by
the MANUAL slider (and the existing MIDI binding) changing `render_frame` PSNR before any
connection is attached: clipOpacity; compOpacity + masterOpacity (final pass, Syphon-after
check); masterSpeed (speed multiply + `scaledTime_`). Independent of L2–L4 — parallel with L2.
UNSAFE to demo "connect X to Clip Opacity" before this lands (nothing visible → false
"engine is broken" diagnosis).

**L5 — Timed sources (M, L–M).** `ClipPosition` per scope: clip = own `playheadPosition`
(GL-written `mutable double`, `Clip.h:157`, `Renderer.cpp:1190`; read via `std::atomic_ref<double>`
on BOTH sides, or convert the field — builder picks after counting writers); layer = its active
clip; composition = disabled until "master layer" exists (D5). `Envelope` inline keyframes with
`Clock::Beats|ClipPosition`; `loop=false` restart on clip trigger. Retire `ClipPositionSignal`.
Proves: units (phase math, one-shot restart); e2e with injected `beatPhase`. Requires L2.

**L6 — Picker hygiene (S).** After L3/L5, BPM Sync / Clip Position / Timeline are REAL; delete any
residual do-nothing branch so the dead-mode count is explicitly zero. (Folded into L3/L5.)

**L7 — Consolidate system 4 (L, H).** (a) Render `composition_->globalEffects` post-composite at
`Renderer.cpp:504` via a `CompositorEngine` pass shaped like `applyClipEffects` (`:232-244`),
keeping the per-context GL-state rule (`EffectChain.h:14-33`). (b) Engine gains the master-chain
family (§3.4); delete `MappingEngine::processFrame/mappings_/smoothers_` and its tick call;
`MappingEngine` shrinks to the two static utilities. (c) `PresetManager::loadPreset` converts
`"mappings"` → `masterChainConns` (D1 resolution reused verbatim; first-mapping-wins on
duplicates, rest reported; parity test extending `test_preset_manager.cpp`: a v1 preset with an
exponential curve + smoothing yields the same param trajectory through the new engine);
`savePreset` writes `"conns"` beside `"mappings"` for one release. (d) Delete `EffectsRackPanel`,
`MappingEditor`, `effectsRackPanel_` + its two `setVisible(false)`; re-point `TestServer`
`add_mapping/remove_mapping` (`TestServer.cpp:956-980`). (e) Leave `effectChain_`/`OutputRenderer`
untouched (OutputWindow arc). UNSAFE reversed: (d) before (a) removes the only curve-capable path
while `globalEffects` still renders nothing. Requires L2, L3.

Parallelism: {L0, L1, L4b} independent of each other and of L2; L2 needs L0; L3 ‖ L4 after L2;
L5 after L2; L7 last.

---

## 6. FINDINGS OUTSIDE THIS DESIGN'S SCOPE (flagged, not designed)

- **The Output window does not render the composition.** `OutputRenderer::renderOpenGL`
  (`OutputWindow.cpp:66-168`) draws only its own loaded image/camera through `effectChain_`;
  `:132` says the deck/composition surface was excluded from that arc. On a projector driven by
  the output window, "audio controls the video" reaches only the v1 chain over a still image; the
  composition is visible in the preview and via Syphon (`publishSyphonFrame`). Surface to the
  owner separately; it is why L7 cannot delete `effectChain_`.
- The registry oscillator/envelope fold phase at one bar (`OscillatorSignal.h:28-30`,
  `EnvelopeSignal.h:36-37`): "8 Beats" cycles twice per intended period. INFERRED; fixed in L1.
- `Composition::compScale` is linear (`v*2`) while clip/layer scale is exponential — pre-existing;
  preserved; owner may align them in the rewrite.

---

## 7. OWNER DECISIONS

### 7.1 Decided (recorded; do not re-ask)
- D1 (takeover): latest input wins; automatic hand-back on release. Signal replaces the slider's
  value within the chosen range; no centre/amount mode.
- D2 (controls): Range (min/max of the slider's travel), Invert, Playback (forward/backward/
  loop/ping-pong). Nothing else is owner-required.
- D3 (timed): tempo-locked rates (beats per cycle). No attack/release requirement.
- Scope: every slider; not toggles or dropdowns.

### 7.2 Still his (one sentence each; recommendation in brackets)
D3c. When no beat is detected (silence, no input), should beat-locked things freeze or keep running
     from the last/tapped BPM? [Keep running from the tapped/last BPM; manual BPM mode already exists.]
D4.  Should "BPM Sync" stay a private per-control oscillator (pick shape + beats in the menu, nothing
     to create first), with Mod 1/Mod 2 remaining shared oscillators edited in one place? [Yes to both.]
D5.  "Clip Position" on a LAYER control = that layer's playing clip; on a COMPOSITION control = the
     master layer's clip, or disabled until a master layer exists? [Layer = its clip; Composition = disabled for now.]
D6.  "Timeline" = a drawable per-control curve that runs over N beats or over the clip's playback
     (looping or once per trigger), or removed from the menu until later? [Build it (L5).]
D7.  Composition "Master" and "Opacity" are both dead today — two things (Master = final output fader
     that also dims Syphon/recording; Opacity = video opacity before global effects) or one? [Two.]
D8.  Should Composition "Speed" also speed up beat-locked clips (breaking their lock)? [No.]
D9.  The hidden v1 rack's preset files will be converted on load and never rewritten — any you want
     preserved bit-for-bit instead? [Convert; report what did not convert.]
D10. Cmd-Z should remove a connection? [Yes — free for effect controls.]
D11. Should the Composition inspector's "Global Effects" stack become THE master effects, applied
     after all layers? [Yes — L7.]
D12. Macros: one global bank of 8 (today) or per-clip/per-layer banks? [Later; the model supports it.]
D13. Anchor Y has no slider and Layer has two sliders on one opacity — fix now or in the UI rewrite? [Rewrite.]
D14. When you let go of a slider, should the value SNAP to where the signal is or GLIDE there over
     about a tenth of a second? [Glide, 120 ms, adjustable; 0 = snap. Open, non-blocking.]
D15. A MIDI knob or OSC/HTTP stream has no "let go" — how long after it stops moving should the
     signal take back over? [A quarter second, adjustable.]
D16. Should audio-channel connections offer an optional "Smooth" control (off by default) so raw
     bass does not visibly shake scale/position? [Yes, off by default — Architect's recommendation,
     not something you asked for.]
D17. Playback direction (backward/ping-pong) applies to generated signals; on an audio channel it
     does nothing — should the control be hidden there, or shown greyed? [Hidden; UI rewrite's call.]

---

## 8. RISKS (and what to verify while building)

- **Widest blast radius is L7** — the only lane that deletes a working path; last, gated on a parity
  test against the owner's real `Presets/` files (ask for a copy).
- **The per-frame GL copy (§4.4)** is the one ordering that can crash the app in the owner's hands;
  L0 is tiny and first. Builder verifies `applyClipEffects` uses nothing but `clip.effects`.
- **Grip completeness**: a manual writer that forgets to grip will fight the signal (value flickers
  between the two every tick). Mitigation: the per-struct `touchScalar` helper and the `set_param`
  path are the only sanctioned writers; builder re-greps all field assignments (§2.3, ≥9).
- **Held grips that never release** (UI bug) freeze the signal on that parameter forever. Mitigation:
  debug `jassert` on destruction with a live grip; "release all grips" on preset load/undo.
- **Message-thread stalls freeze modulation** (unchanged; §4.3 gives the exit).
- **`playheadPosition` is a GL-written double** — L5 names the two acceptable reads.
- **Line drift**: `MainComponent.cpp` is under concurrent edit; packets re-grep anchors.
- **Two GL readers** of `EffectParam::value` (master-chain family) keep the existing A2 crossing.
- Counts to re-verify before "done": ≥4 `paramValues.push_back` sizing sites; 21 inspector
  controls + 8 macro knobs; 18+4 renderer read sites (§3.1/3.2); ≥9 grip sites; zero `add_route`
  users in `tests/`; `.links` writers = 0 before deleting `MacroLink`.


---

## ADDENDUM (s166, after commit `694f8f3` and the owner's D1 answer) — supersedes the named lines above; the body is left as written

### A1. D1 — already converged in rev 2; Modulate is DROPPED, not kept

Rev 2 (§R, §2.1, §2.3) already made the owner's model the only model: the connection OWNS the
value (`y = outMin + c·(outMax−outMin)` into the chosen sub-range of the slider's travel, invert,
`Playback {Forward, Backward, PingPong}` + `loop`), and a human touch takes over transiently via
the model-level GRIP. The rev-1 recommendation (centre + Amount) is withdrawn and `mode/depth` do
not exist in the struct. **Why Modulate does not earn a per-connection slot:** under
latest-input-wins the manual field is, by definition, "wherever the human last left it" — a
centre+amount mode would turn that leftover into an invisible offset added to every future
signal value, so the same connection would render differently depending on the last place a
finger happened to release; it is a second mental model on a UI being scrapped; and every
legitimate use ("wobble a little around 0.6") is expressible as a narrower RANGE (0.5–0.7) with
no extra concept. Touch/release at the model layer, and the writers that must call them, are in
§2.3 (`gripHeld()` for inputs with a release event; `gripTouch(now)` for MIDI/OSC/HTTP, expiring
after `gripHoldMs`); GL never reads grip state (twin holds NaN while gripped → renderer reads the
manual field the hand is writing). Hand-back: **GLIDE**, 120 ms, global `handBackGlideMs`,
0 = snap (D14, open, non-blocking). Direction/loop honesty (§2.4): phase-based sources
(Lfo, Envelope, ClipPosition) honour Backward/PingPong/once; an audio channel or macro has no
phase — the field is stored and serialized (one struct) and the engine ignores it there; D17
asks whether the future UI hides or greys it.

### A2. `Composition::globalEffects` — VERDICT on the shipped placement (`694f8f3`): ACCEPTABLE, build on it

Shipped: `CompositorEngine::applyGlobalEffects()` (`CompositorEngine.cpp:923-940`) called at
`Renderer.cpp:481-488` — after `compositeDeck` (`:453`), after the persistent-layer loop (`:460-466`),
after `updateFeedbackBuffer` (`:470`), before `effectChain_.render` (`:520`) and before
`applyCompTransform` (`:528`); guarded on `deckActive && composition_ && sourceTexture != 0`.
VERIFIED. Pipeline analysis, stage by stage:
- After both composites — CORRECT (global effects must cover persistent layers from other decks).
- Before `applyCompTransform` — CORRECT (effects run in composition pixel space; the transform is
  output placement; Resolume order).
- Before the legacy `effectChain_` — INDIFFERENT (two master passes; `effectChain_` is all-disabled
  in production; moot when it is retired).
- After `updateFeedbackBuffer` — ACCEPTABLE WITH ONE NAMED, VISIBLE CONSEQUENCE. `updateFeedbackBuffer`
  copies `accumulatorTex_` (the composite WITHOUT global effects) into `feedbackTex_`
  (`CompositorEngine.cpp:1192-1213`), which every non-temporal effect binds as `u_feedbackTex`
  (`:346-358`; three shaders sample it, `EmbeddedShaders.h:5007,5083,5599`). So a feedback-style
  effect placed in the GLOBAL stack sees last frame's composite without its own output: it shows
  one frame of decayed trail and never accumulates. Temporal (`u_prev_frame`) global effects are
  unaffected — their buffer is keyed by the sentinel id and saves the pass's own output
  (`:425-429`), so they DO self-accumulate. This is the conservative choice: `feedbackTex_` stays the
  "pure composite" that every per-clip/per-layer feedback effect depends on today; moving the
  global pass above `updateFeedbackBuffer` would make global trails accumulate but would also feed
  the globally-effected image back into every clip-level feedback effect next frame. Decision:
  KEEP the shipped placement; record the consequence; revisit only if the owner reports "trails
  don't build on a global effect" (then it is a one-line move plus that trade-off, his call).
- Not applied when no deck is active (standalone image/source path) — consistent with "composition"
  semantics; note for the owner.
Supersedes: §1.4 row "`Composition::globalEffects` … BUILT … Lane 7" → DONE (`694f8f3`); §5 L7(a)
→ DELETED. L7 is now (b)–(e) only: master-chain family for the v1 chain, v1 preset → connection
conversion, delete `MappingEngine::processFrame`/`EffectsRackPanel`/`MappingEditor`, re-point the
`add_mapping` test endpoints. L7's UNSAFE-reversed rule ("(d) before (a)") is discharged.

### A3. L0 re-checked against the new code — same edit, now at TWO sites, and load-bearing for thread safety (not identity)

(a) **Same edit twice, not harder.** `applyClipEffects` reads only `clip.effects` (`:237,242,252` are
the only `clip.` uses in its body; `clip.id` is never read) — VERIFIED. Changing its signature to
`const std::vector<Clip::EffectSlot>&` makes BOTH temp-Clip sites (`:761-763` layer, `:936-938`
global) collapse to passing the vector; `applyGlobalEffects` already receives that vector, so its
temp Clip is pure overhead that vanishes. Builder greps `applyClipEffects(`: at least 6 call sites
(`:536`, `:744`, `:763`, `:905`, `:938`, `:1130`) — all become `x.effects` / `layer.layerEffects` /
`globalEffects`.
(b) **Cost and caching.** Per frame, per site, on the GL thread: a default `Clip` (strings, `juce::File`,
vectors, `juce::Image` — cheap but non-trivial) plus a deep copy of the effects vector (1 + up to 2
allocations per slot). Cost only. No identity hazard: nothing keys on the temp Clip (id 0) — the
caches key on the `layerId` argument. **What IS load-bearing is thread safety (§4.4), now doubled:**
both sites copy, on the GL thread, a vector the message thread mutates — `layer.layerEffects` and
`composition_->globalEffects` — and the bypass toggle (`EffectStackView.cpp:320`) is unfenced (only
erase/add/undo go through the deck fence, which does cover the global pass because it sits inside
`if (deckActive)`). Today a torn float/bool is harmless; once `EffectSlot` carries a
`ParamConnection` (`std::string signalName`, envelope `vector`) a GL-thread copy mid-assignment is
heap corruption. **L0 must remove both copies before L2, and must be verified with TSan while
toggling bypass on a layer AND on the global stack.**
(c) **Sentinel `kGlobalEffectsLayerId = 0xFFFFFFFF` (`CompositorEngine.h:168`) — bounded, and no
interaction with this design.** The layer-keyed GL resources are `layerTemporalBuffers_` (`:184`),
`layerRingBuffers_` (`:206`), `feedbackProcessors_` (`:171`), `layerOutput{Textures,FBOs,TexStorage}_`
(`:280-284`). The sentinel is ONE constant key → at most one temporal buffer (w×h, only if a
temporal effect is in the global stack) and one ring buffer (only for Screen Split / Frame
Stutter), each re-created in place on size change (`:1227-1248`, `:1269-1307`), never one per
frame. `applyGlobalEffects` never calls `getOrCreateFeedbackProcessor` or `saveLayerOutput`, so the
sentinel never appears as a Layer-Router-routable layer (`Renderer.cpp:984`). Real layer ids are
0..2 and 100+ (`Deck.h:198`) — no collision. Pre-existing and unrelated: none of these maps are ever
erased (zero `.erase` hits), so add/remove-layer churn grows them — not caused by the sentinel.
My design keys nothing by layer id (per-parameter twins inside the structs; the engine walks
structure), so there is no coupling to fix.

### A4. §6 output-window claim — VERIFIED by three grep shapes of different kind

1. Call-graph shape: every `x.y(` / `x->y(` token in the body of `OutputRenderer::renderOpenGL`
   (`OutputWindow.cpp:66-168`) resolves to `texMgr_.*`, `effectChain_.render`, `featureBus_.read`,
   `glContext_.*`, `OpenGLHelpers::clear` — the only other two tokens (`mappingEngine_.processFrame(`,
   `MainComponent::tickFeaturePipeline(`) occur inside the comment at `:109-119` that says
   processFrame is intentionally NOT called there.
2. Symbol shape: `compositor|Compositor|compositeDeck|activeDeck|Deck|Composition|setActiveDeck|
   setComposition` → 0 hits in `OutputWindow.cpp`, 0 in `OutputWindow.h`.
3. Producer shape: every `outputWindow_->` call in `MainComponent.cpp` is `loadImage(<file>)` (the
   triggered clip's `mediaFile`, a dropped file, a deck image, slideshow images), `getRenderer().
   queueCameraFrame`, `getRenderer().detach`, `goFullscreenOnDisplay`, `setVisible`; the class's public
   API (`OutputWindow.h:77-95`) has no deck or composition setter at all.
Plain statement for the owner: the projector window shows the triggered clip's still image (or the
camera) through the hidden v1 effect rack — no layers, no per-clip effects or transforms, no
procedural sources, no global effects. "Audio controls the video" reaches that screen only through
(i) v1 mappings loaded from a preset file and (ii) the audio uniforms hard-wired into some shaders.
The composition reaches an audience only via the preview panel or Syphon. VERIFIED.

REPORT_FILE: memory/.reports/s166/arch-universal-connection.md
STATUS: COMPLETE
