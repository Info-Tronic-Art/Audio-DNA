# Master Signal — design (no build) — s-rta-0925

Architect (Fable), 2026-09-25. Read-only pass over src/, .harmony/specs, tests/. Every code claim
is cited `path:line` against HEAD e501b00; confidence is marked VERIFIED (read it) / INFERRED
(derived from what was read) / ASSUMED (not checked; builder verifies).

QUESTION: Boris — "a slider top right next to the master opacity for master audio AND all
signals including oscillators, etc. This is taking the gain slider and adding the other signals."
Design ONE "Master Signal" control; settle whether the analysis input Gain stays separate.

---

## APPROACH (stated first)

**Keep the Gain slider as what it is (a pre-analysis microphone trim) and add a NEW, separate
post-analysis "Master Signal" fader at the top right, next to Master.** Master Signal is a
composition-level scalar (`CompScalar::Signal`, key `"signal"`, model field
`Composition::masterSignal`, default 1.0 = today's behaviour, 0 = every connected control sits
at its hand-set value and the picture stops reacting to audio) applied as a **depth on the
output of every signal-to-parameter path**, never on the analysis input:

```
effective = manual + depth * (signalDriven - manual)      // depth 1 -> bit-identical to today
```

It rides the existing universal-connection plumbing (`manualWrite` funnel, LiveValue twins,
recorder `scalar` lane, REST live block) so persistence, MIDI/OSC/REST bindability and the
recorder come with it instead of being re-derived. One prerequisite is exposed by this pass:
the effect-parameter and source-parameter connection twins are **render-dead today** (§2), so
the already-named follow-up (lane-3 plan §11 "effect/source-param vertical slice") must land
first, otherwise Master Signal would cover opacity/transform/speed/macros but not effect
sliders — which is most of what Boris will test it on.

Plain words for Boris (the gain question): *Gain is the volume knob for the beat detector's
ears. Master Signal is how much the music (and the oscillators) are allowed to move your
sliders. If we turned the Gain slider into Master Signal, pulling it down to calm the picture
would also make the app deaf — the beat wheel stops, the BPM drops out, hits vanish, and the
audio recorded into your take goes quiet too. Pushing it up to hype the picture would clip the
analysis. Two knobs, two jobs: Gain stays on the left (set once per venue), Master Signal is the
new performance fader on the right.*

---

## 1. What "Gain:" does today (VERIFIED)

| Fact | Where |
|---|---|
| TopBar "Gain:" is a `ResettableSlider`, range 0..4, default 1.0 | `src/ui/TopBar.h:64-65`, `src/ui/TopBar.cpp:21-25` |
| Its only consumer: `audioEngine_.setInputGain(v)` | `src/MainComponent.cpp:624-626` |
| `setInputGain` stores a relaxed atomic read **inside the real-time audio callback** | `src/audio/AudioEngine.h:51`, `src/audio/CombinedCallback.h:30,66` |
| Applied ONLY in mic/input mode (`useInputForAnalysis`): raw input samples × gain are written into the output scratch buffer and THAT is what analysis is fed | `CombinedCallback.h:54-90` (multiply at :76-79, analysis fed at :107-110) |
| The gained buffer is ALSO what `AudioTap` records into the take (`listened`) | `CombinedCallback.h:129` |
| In file mode the gain is **not applied at all** (the file path never reads `inputGain`) | `CombinedCallback.h:91-101` |
| Peak meter value is gain-scaled too | `CombinedCallback.h:66-70` |
| A second, hidden v1 copy of the same slider exists and is the one persisted in `.deck.json` (`deck.inputGain`); the TopBar slider is persisted nowhere and the two are not synced | `src/MainComponent.h:354`, `src/MainComponent.cpp:301-308, 2425-2426, 3605, 3700-3701`, `src/ui/PresetManager.h:86` |

So Gain is a **pre-analysis input trim**: it scales RMS/peak/band energies/onset ODF/spectral
flux before aubio onset + tempo, before the silence detector (`BPMTracker::feedSilenceDetection`,
RMS < 0.005 — CLAUDE.md "Smart BPM Recovery"), before genre/energy classification, and before the
recorder's audio file. It changes beat/onset detection (worse when turned down, clipping when
turned up). It has **zero** effect on oscillators, envelopes, macros, clip position — and today
it does nothing at all while an audio file is playing, which is INFERRED to be why it reads as
"free to repurpose".

**Verdict on the key question:** keep analysis input gain separate; the new control is
post-analysis depth. Repurposing the slider literally would couple "calm the visuals" to
"break the beat tracker and dim the take audio" (`CombinedCallback.h:107-110,129`).

---

## 2. Every signal-to-parameter path that renders today (VERIFIED unless marked)

The message-thread 120 Hz tick `MainComponent::tickFeaturePipeline` (`src/MainComponent.cpp:
3323-3381`) runs, in order: `signalRegistry_.evaluateAll(snap)` (:3332) → v1
`MappingEngine::processFrame` (:3334) → `globalMacroBank_.updateValues` (:3344) → recorder tick
(:3354-3361) → `connectionEngine_.tick` (:3376-3378) → `inspectorPanel_->tickModulation()`
(:3380). The GL threads read the bus themselves for shader uniforms.

| # | Path | Computes | Rendered from | Status |
|---|---|---|---|---|
| A | `ConnectionEngine::tick` → scalar twins (clip/layer/comp opacity, transform, speed) | `ConnectionEngine.cpp:198-218, 301-302, 309-310, 323-324` | `Composition::eff()` at `Renderer.cpp:432, 726`; `clip.eff()/layer.eff()` at `CompositorEngine.cpp:447-453, 562-567, 626` | LIVE |
| B | `ConnectionEngine::tick` → macro `conn` | `ConnectionEngine.cpp:287-299` | `Macro::currentValue` read by path D-ui and by `Kind::Macro` sources (`ConnectionEngine.cpp:106-113`) | LIVE |
| C | `ConnectionEngine::tick` → effect-param twins `fx.paramLive` / `dryWetLive` | `ConnectionEngine.cpp:220-259` | **nothing** — `Clip.h:90 effParam()` has zero callers (grep src/); compositor uploads raw `slot.paramValues[p]` at `CompositorEngine.cpp:372-376` (also :278-279, :1475-1478) and raw `slot.dryWet` at :384, :409 | **RENDER-DEAD** |
| D-ui | `EffectStackView::tickModulation` writes the RAW registry/macro value into `fx.paramValues[p]` (no ConnShape: RANGE/INVERT/curve are ignored) | `src/ui/EffectStackView.cpp:137-236` (write at :196) via `InspectorPanel.cpp:170-178` | `CompositorEngine.cpp:372-376` | LIVE (this is how effect sliders move today) |
| E | `ConnectionEngine::tick` → source-param twins `sp.live` | `ConnectionEngine.cpp:327-340` | **nothing** — `Renderer.cpp:1101` uploads `cp.value`; `onSourceParamsPublished` is never assigned (grep: only `ConnectionEngine.h:75`/`.cpp:339-340`) | **RENDER-DEAD** |
| E-ui | `ClipInspector::tickModulation` writes RAW value into `clip_->sourceParams[i].value` | `src/ui/ClipInspector.cpp:838-893` (write at :893) | `CompositorEngine.cpp:827-828` → `Renderer.cpp:1094-1101` | LIVE |
| F | Legacy macro path `MacroBank::updateValues` from `sourceSignalId` | `src/routing/MacroBank.h:74-87`, `MainComponent.cpp:3344`; MacroPanel still writes `sourceSignalId` (`src/ui/MacroPanel.cpp:136,144`) | `Macro::currentValue` | LIVE |
| G | v1 `MappingEngine::processFrame` → Renderer's v1 `effectChain_` params | `src/mapping/MappingEngine.cpp:144-245` (store :242-243), `MainComponent.cpp:3334` | `Renderer.cpp:564` (v1 chain on the final output) | LIVE but only fed by v1 preset slots / `randomizeAllEffects` (`MainComponent.cpp:3898`, fired by hidden `beatRandomToggle_` whose state loads from `.deck.json`) — INFERRED reachable only through legacy files |
| H | Shader-direct audio uniforms from a snapshot copy | main GL: `Renderer.cpp:229-233` (`frameSnap_`) → `compositor_.setLatestSnapshot(snap)` :495, `effectChain_.render(...snap...)` :564, `renderSource` → `ProceduralSource::uploadUniforms`; output GL: `src/ui/OutputWindow.cpp:103-104` reads the bus itself | every shader declaring `u_rms` etc. — `EmbeddedShaders.h` mentions: u_rms 191, u_beatPhase 68, u_bass 44, u_onsetStrength 40, u_spectralCentroid 17, u_bandEnergies 13, u_chromagram 12, u_mfccs 11, u_onsetDetected 8 (`> 0.5` gate at :12302/:12422, multiplicative at :12226) | LIVE, bypasses routing entirely |
| I | `RoutingEngine` (`src/routing/RoutingEngine.cpp`) | — | dead: no live consumer (`Renderer.cpp:247-254` comment; `addRoute` only from TestServer) | DEAD — ignore |
| J | Autopilot / smart random / genre + structural callbacks | `src/model/Autopilot.cpp:34-56, 326-338`; `Renderer.cpp:266-300` | reads beatPhase/beatInBar/barCount/structuralState/energyState/genre | clock + label consumer, not a level (see §3.3) |

Consequences that shape the design:
- Paths C and E are computed and discarded; the *rendered* modulation for effect and source
  sliders is D-ui/E-ui, which overwrite the model's manual field with the raw signal every tick.
  There is therefore **no manual value to fall back to** for effect/source params today, and
  RANGE/INVERT set on an effect param do nothing on screen (pre-existing bug, same class as the
  render-dead masterOpacity that `Renderer.cpp:708-716` documents).
- The lane-3 plan already names the fix as follow-up #1 (`.harmony/specs/s-rta-0923-lane3-plan.md:
  701-707`): repoint `CompositorEngine.cpp:278-279,376,384,409` to `effParam/effDryWet`, source upload
  to `sp.live.effective(sp.value)`, wire `onSourceParamsPublished`, delete `tickModulation`.
  That follow-up is **Step 0 of this design**.

---

## 3. DECISION — semantics

### 3.1 The control
- Name: **Master Signal**. TopBar label `"Signal:"` (whole word). Tooltip: *"Master Signal: how
  strongly audio, oscillators and every other signal move the controls they are connected to.
  100% = full; 0% = everything sits at its hand-set value."*
- Model: `float Composition::masterSignal = 1.0f` (`src/model/Composition.h`, beside
  `masterOpacity` :40). Scalar: `CompScalar::Signal` **appended before `Count`**
  (`src/connect/ScalarParams.h:35`), def `{ "signal", identity, identity, 1.0f }` appended to
  `compScalarDefs()` (:107-119), `manualRef` case (`Composition.h:516-532`). Key convention
  matches `"opacity"→masterOpacity`, `"speed"→masterSpeed`.
- Range 0..1 (0%..100%), step 0.01, default and right-click reset 1.0. No boost above 100%
  (a depth > 1 only overshoots into the [0,1] clamps; "more" is the per-connection RANGE's job).

### 3.2 What the value means, per path (d = depth)

| Path | Rule | d = 1 | d = 0 |
|---|---|---|---|
| Every `ParamConnection` (audio Signal, Lfo, Envelope, ClipPosition, Macro) — paths A, B, C, E after Step 0 | in `ConnectionEngine::evaluate` after smoothing and the hand-back glide, before `return y` (`ConnectionEngine.cpp:176-177`): `y = applyDepth(manualNorm, y, d)` | untouched (bit-identical, see 3.4) | returns `manualNorm` exactly → twin = manual → control sits where the hand left it |
| Gripped connection | unchanged: publishes NaN regardless of d (`ConnectionEngine.cpp:83-92`) | | |
| Oscillators / envelopes / clip position | the LFO keeps running on the beat clock underneath; only its *reach* is scaled. Fader back up → it comes in at the correct phase, no restart | | slider still, LFO silently continues |
| Signal-driven macro knob (B, F) | F: `MacroBank::updateValues(signals, d)`: `currentValue = applyDepth(manualValue, signal, d)` (`MacroBank.h:74-87`); B: via evaluate | | knob reads its manual value |
| Master Signal's own connection (it is a CompScalar, so a Macro/MIDI fader can drive it) | evaluated with d forced to 1 (no self-feedback) | | |
| v1 `MappingEngine` (G) | `processFrame(snap, chain, d)`: final store `clamp(applyDepth(param.defaultValue, sum, d))` (`MappingEngine.cpp:242-243`; `defaultValue` at `src/effects/Effect.h:13`) — v1 mappings have no manual value, so 0 = the effect's default | | |
| Shader-direct uniforms (H) | `scaleSignalLevels(FeatureSnapshot&, d)` on the GL thread's snapshot copy: **levels scale, clocks and labels do not** — scaled: `rms, peak, bandEnergies[7], onsetStrength, spectralFlux, transientDensity, harmonicChangeDetection, sidechainPump, formantPresence, resonancePeak, reeseBass`; pulses `onsetDetected`, `downbeatDetected` forced false only when d == 0; untouched: `bpm, beatPhase, barPhase, phrasePhase, beatInBar, barCount, totalBarCount, trackerState, structuralState, energyState, detectedGenre, genreConfidence, genreScores, detectedKey, keyIsMajor, dominantPitch, pitchConfidence, spectralCentroid, spectralRolloff, spectralFlatness, chromagram, mfccs, swingRatio, dynamicRange, rmsDB, lufs, timestamp, wallClockSeconds, sourceSampleRate, bandValidMask, onsetCount` | skipped entirely (bit-identical) | shaders see silence; beat-clock-driven motion continues (Q2) |
| `SignalRegistry` cached values / SignalBar meters (`src/ui/SignalBar.cpp:119`) | **not scaled** — meters show what the audio is doing; the fader shows how much of it reaches the controls | | meters keep dancing |
| Autopilot, smart random, genre/structural scene triggers (J) | out of scope: clip sequencing driven by the beat clock and labels, not a level; scaling `beatPhase` would change the tempo, not the intensity. They read only untouched fields (`Autopilot.cpp:34,44,56,326,338`; `Renderer.cpp:279-300`), so applying `scaleSignalLevels` to `frameSnap_` in place is safe | | |
| Analysis input gain, onset/BPM detection, recorded take audio | untouched by design (§1) | | |

### 3.3 Why "levels, not clocks"
At d = 0 the picture is not frozen: clips keep playing, `u_time` keeps running, autopilot keeps
sequencing. "0" means *no audio reactivity and no modulation*, not *no motion*. Beat-locked LFOs
on sliders DO stop moving sliders (they are connections). A shader that reads `u_beatPhase`
directly is reading the clock, like `u_time`; freezing it would be the one place a clock stops.
This is Q2 for Boris; the recommendation is "clocks keep running".

### 3.4 The one shared helper (`src/features/SignalDepth.h`, header-only, no JUCE)
```cpp
inline float applyDepth(float manual, float driven, float depth) {
    if (depth >= 1.0f) return driven;                    // bit-identical to today
    if (depth <= 0.0f) return manual;                    // exact hand value
    return manual + depth * (driven - manual);
}
void scaleSignalLevels(FeatureSnapshot& s, float depth); // no-op when depth >= 1
```
The `>= 1` / `<= 0` guards are load-bearing: `m + 1*(y-m)` is not guaranteed to equal `y` in
IEEE float, and the "1.0 = today's behaviour" promise must be bit-for-bit (tests pin it).

---

## 4. TRADEOFFS CONSIDERED

- **Repurpose the Gain slider (Boris's literal phrasing)** — rejected: pre-analysis, mic-mode only,
  couples "calm visuals" to degraded onset/BPM detection and quieter take audio
  (`CombinedCallback.h:76-79,107-110,129`); inert in file mode. Kept as a separate trim.
- **Scale the sources (registry cached values ×d, LFO amplitude ×d)** — rejected: an audio
  signal at ×0 lands the control at its RANGE bottom (`ConnectionShaper::shapeValue`
  `outMin`, `ConnectionShaper.cpp:80-91`), not at the hand-set value; an inverted connection would
  go to its RANGE top; LFOs would swing "from 0" instead of "around the hand". Output-side depth
  is the only formulation where 0 == "static at the manual value" for every kind.
- **Treat d = 0 as "gripped" (publish NaN)** — rejected: the source-param site skips the store on
  NaN (`ConnectionEngine.cpp:332-337`) and would freeze at the last signal value (the critic-#1
  frozen-twin class); returning a real number is uniform and exact.
- **Leave shader-direct uniforms out of scope ("they bypass routing")** — rejected: `u_rms` has 191
  mentions, `u_bass` 44, `u_onsetStrength` 40 across `EmbeddedShaders.h`; the audio-native
  effects and most audio-visual sources would keep reacting at 0 and Boris would report the
  fader "does nothing". The pure snapshot transform costs one function at two read sites.
- **Scale the snapshot for everything (one site, no engine change)** — rejected: LFOs, envelopes,
  clip position and macros do not come from the snapshot; and shaped audio connections would
  again converge to RANGE bottom, not manual.
- **Skip the Step-0 repoint and special-case `tickModulation` instead** — rejected: those ticks
  destroy the manual value every tick (`EffectStackView.cpp:196`, `ClipInspector.cpp:893`), so
  there is nothing to blend toward; the repoint is already the named follow-up and also fixes
  render-dead RANGE/INVERT on effect params.
- **Plain `Composition` float instead of a `CompScalar`** — rejected: `manualWrite` needs a
  `ParamConnection*` to grip (`ManualWrite.h:28-35`), the recorder lane is keyed by ControlPath
  `control:"scalar"` (`s167 spec :178-180`), and the REST live block iterates `compScalarDefs()`
  (`ApiServer.cpp:318`). A CompScalar gets all three for free; the only cost is self-feedback,
  handled by forcing d = 1 for its own connection.
- **Range above 100% (boost)** — rejected (3.1).
- **Persist vs boot at 100%** — persist recommended (Q3), mirroring `masterOpacity`.

---

## 5. SPEC — sequence, files, interfaces, "done"

### Step 0 — prerequisite: make effect/source-param connection twins render (lane-3 plan §11 #1)
Files: `src/render/CompositorEngine.cpp`, `src/render/Renderer.cpp`, `src/MainComponent.cpp`,
`src/ui/EffectStackView.cpp`, `src/ui/ClipInspector.cpp`, `src/connect/ConnectionEngine.cpp`.
1. `CompositorEngine.cpp:372-376` → `glUniform1f(loc, slot.effParam(p))`; :278-279 and :1475-1478
   (Screen Split / Frame Stutter intercepts) → `slot.effParam(0..3)`; :384 and :409 → `slot.effDryWet()`.
   `resizeParams` is already self-healed in the engine tick (`ConnectionEngine.cpp:230`); the
   GL thread must not resize — read `effParam` only when `p < slot.paramLive.size()`, else fall
   back to `paramValues[p]` (a slot created and rendered before its first engine tick).
2. `Renderer.cpp:1101` → `source->setParamValue(i, cp.live.effective(cp.value))`.
3. `ConnectionEngine.cpp:332-337`: always store (NaN on not-driven), same rule as :208-216.
4. `MainComponent.cpp` (near :3376): `connectionEngine_.onSourceParamsPublished = [this](const Clip* c){
   previewPanel_.getRenderer().updateActiveSourceParams(c->sourceParams); }` — needed because the
   standalone-source path copies the vector (`Renderer.cpp:80-101`, callers `MainComponent.cpp:811,
   1657, 4202`); VERIFIED production-reachable. Fire only for the clip that is the active
   standalone source (builder: compare `sourceType`/clip identity; the deck path reads the model
   vector live at `CompositorEngine.cpp:827-828` and needs nothing).
5. Delete the model writes `EffectStackView.cpp:196` and `ClipInspector.cpp:893`; keep their display
   code but feed it `fx.effParam(p)` / `sp.live.effective(sp.value)`. `onParamChanged`
   (`EffectStackView.cpp:224-225`): grep found no assignment site (INFERRED no-op today) — builder
   verifies; if it feeds a renderer-side copy, pass `effParam(p)`.
6. Behaviour change to state in the commit: effect-param RANGE/INVERT/curve now render (they were
   ignored); a connected effect param's saved `paramValues[p]` is again the hand value, not the
   last signal sample.
Gate: an effect param connected to "Bass" with RANGE 0.2..0.4 renders inside that range
(`/api/composition` live block + a `render_frame` pair); `grep -n "slot\.paramValues\[" src/render/
CompositorEngine.cpp` returns only the fallback branch.

### Step 1 — model + persistence
Files: `src/connect/ScalarParams.h`, `src/model/Composition.h`, `tests/test_composition.cpp`.
- `CompScalar::Signal` appended before `Count` (:35); def appended (:107-119).
- `Composition::masterSignal = 1.0f` (:40); `manualRef` case (:516-532); `initDefault` resets it (:144-160).
- `toVar`: `obj->setProperty("masterSignal", …)` beside :214. `fromVar`: **guarded**
  `if (obj->hasProperty("masterSignal"))` beside :331 (do NOT copy `masterOpacity`'s unguarded read —
  an absent key would load 0.0 and silence every show). Old files load as 1.0 = today.
- The sparse `"conns"` map already covers a connection on it (`ConnSerialization.h:43-73`).

### Step 2 — engine, macros, v1 mappings
Files: `src/features/SignalDepth.h` (NEW), `src/connect/ConnectionEngine.h/.cpp`,
`src/routing/MacroBank.h`, `src/mapping/MappingEngine.h/.cpp`, `src/MainComponent.cpp`.
- `ConnectionEngine::Context` gains `float signalDepth = 1.0f;` (default keeps every existing
  test bit-identical). `evaluate()`: exactly ONE read, `y = applyDepth(manualNorm, y,
  ctx.signalDepth);` inserted after the glide block (:158-175), before `return y` (:177).
- `tick()`: `tickScalars<Composition,CompScalar>` evaluates `CompScalar::Signal` with a local
  `Context full = ctx; full.signalDepth = 1.0f;` — the one exemption.
- `MainComponent::tickFeaturePipeline` (:3374-3378): `ctx.signalDepth = composition_.eff(CompScalar::Signal)`
  (previous tick's twin when Signal is itself connected — 8 ms lag, acceptable);
  `globalMacroBank_.updateValues(signalRegistry_, depth)` (:3344);
  `mappingEngine.processFrame(snap, chain, depth)` (:3334).
- Order of reads inside the tick is unchanged; the depth is read once per tick and passed down.

### Step 3 — shader-direct snapshot scaling
Files: `src/features/SignalDepth.h`, `src/render/Renderer.cpp`, `src/ui/OutputWindow.h/.cpp`,
`src/MainComponent.cpp`.
- `Renderer.cpp:233` (after the onset pulse derivation :230-232, before `autopilot_`/uploaders):
  `scaleSignalLevels(frameSnap_, composition_ ? composition_->eff(CompScalar::Signal) : 1.0f);`
  `composition_` may be null (`Renderer.h:419`) → 1.0.
- `OutputWindow.cpp:104`: the output renderer holds no `Composition*` (VERIFIED grep of
  `OutputWindow.h`) → add `std::atomic<float> signalDepth_{1.0f}` + `setSignalDepth()`, written
  from `tickFeaturePipeline` right after the engine tick; read relaxed on its GL thread and apply
  `scaleSignalLevels(snap, …)` after its own pulse derivation.
- `scaleSignalLevels` is a no-op at depth >= 1 (zero cost, bit-identical) and forces the two
  pulses false only at depth == 0.

### Step 4 — TopBar fader
Files: `src/ui/TopBar.h/.cpp`, `src/MainComponent.cpp`.
- `juce::Label masterSignalLabel_{"", "Signal:"}; ResettableSlider masterSignalSlider_;` range
  0..1 step 0.01, value/default 1.0, `LinearHorizontal`, `NoTextBox`, tooltip from 3.1. Getter
  `getMasterSignalSlider()` (pattern `TopBar.h:39`).
- Layout (`TopBar.cpp:520-532`): after `masterLabel_.setBounds(rightSection.removeFromRight(42))`
  add `rightSection.removeFromRight(4); masterSignalSlider_.setBounds(rightSection.removeFromRight(70));
  masterSignalLabel_.setBounds(rightSection.removeFromRight(42));` — i.e. `… | Signal: [==] | Master: [==] | Output …`.
  Gain stays at :446-447. Width: +112 px; INFERRED there is slack (the tempo block reserves 64 px
  unused at :478); builder verifies at the app's minimum window width and, if needed, shrinks
  `fadeSlider_` 100→80 and `quantizeSelector_` 100→90.
- Wiring in `MainComponent` (beside :628-632), through the funnel so it is one more "hand":
  ```
  auto& s = topBar_->getMasterSignalSlider();
  s.onDragStart  = [this]{ signalDragging_ = true;  manualTouch(compScalarPath("signal"), GripKind::Held, Origin::Human); };
  s.onDragEnd    = [this]{ signalDragging_ = false; manualRelease(compScalarPath("signal"), Origin::Human); };
  s.onValueChange= [this]{ manualWrite(compScalarPath("signal"), (float)s.getValue(),
                                      signalDragging_ ? GripKind::Held : GripKind::Decaying, Origin::Human); };
  ```
  (right-click reset fires `onValueChange` without a drag → Decaying, no stuck Held grip.)
- Sync back (MIDI/OSC/replay/connection moves the value): in `TopBar::timerCallback` (15 Hz,
  `TopBar.cpp:224-240`) `if (!masterSignalSlider_.isMouseButtonDown()) masterSignalSlider_.setValue(
  composition_.eff(CompScalar::Signal), juce::dontSendNotification);` — `TopBar` already holds
  `composition_` (`TopBar.h:50`). Use the identical wiring for the Master-opacity link (§10.A).

### Step 5 — bindability
| Surface | Change | Anchor |
|---|---|---|
| Keyboard / MIDI note / CC (incl. relative encoders) | `Binding::Action::MasterSignal` **appended at the END** of the enum — `BindingManager` serialises the action as an int (`BindingManager.cpp:228, 272`), so inserting mid-enum would silently re-map every saved binding | `src/binding/Binding.h:24-45` |
| Bind-mode / MIDI-learn target | `"Master Signal"` target beside `"Master Opacity"` | `MainComponent.cpp:6454-6455` |
| Handler | `case MasterSignal: manualWrite(compScalarPath("signal"), value, GripKind::Decaying, Origin::Human);` | beside `MainComponent.cpp:6727-6730` |
| OSC | `/audiodna/signal` (float 0-1) → `onSetMasterSignal` → same `manualWrite` | `src/osc/OscHandler.cpp:137-141` pattern; header list `OscHandler.h:13-24` |
| REST read | automatic in `GET /api/composition` live block (`live.signal`) since it iterates `compScalarDefs()`; also add `"masterSignal"` beside `"masterOpacity"` | `src/api/ApiServer.cpp:314, 318` |
| REST write | `POST /api/set_master_signal {"value":0.5}` → `onSetMasterSignal` callback → `manualWrite` Decaying (mirror `onSetLayerOpacity`) | `src/api/ApiServer.h:82`, `ApiServer.cpp:171-177` |
| Recorder (s167 D3 continuous `scalar` lane) | automatic: `manualWrite` → `onManualWrite` hook → `RecorderHost::onHumanWrite` keys the lane by the ControlPath verbatim; `Program::resolveKey` treats every Comp-scope key as `ExactMatch`; replay re-enters the funnel and `resolveScalar` finds `"signal"` by key | `MainComponent.cpp:2023-2027`, `RecorderHost.cpp:547-560`, `Program.cpp:98-99`, `ManualWrite.cpp:91-98`, spec `.harmony/specs/s167-performance-log-and-routines.md:178-180, 205-220` |
| Recorder checkpoint0 | GAP shared with `masterOpacity`/`masterSpeed`: `capturePerfState` records clip scalars + layer opacity only (`PerfStateCapture.cpp:53-110`), so "replay snaps back first" will not restore comp scalars. Recommend the replay lane add comp scalars in the clip-scalar style (:96-102) — not this lane | |
| Connectable | yes (a Macro → Signal lets an APC fader drive it); evaluated at full depth | Step 2 |

### Step 6 — tests (fail-first where marked)
| Test | File / target | Pins |
|---|---|---|
| `applyDepth`: d=1 returns `driven` bit-identically; d=0 returns `manual` exactly (`==`, not Approx); d=0.5 midpoint | new `tests/test_signal_depth.cpp` (header-only dep; add target after `test_record_panel_model`, `tests/CMakeLists.txt:1143`) | 3.4 |
| `scaleSignalLevels`: d=1 → `memcmp` identical POD; d=0 → listed level fields 0, pulses false, every clock/label field bit-identical; d=0.5 halves | same | 3.2 row H |
| `evaluate` depth for each `ConnSource::Kind` (Signal/Lfo/Envelope/ClipPosition/Macro) with an inverted sub-RANGE shape: d=0 == `manualNorm`; d=1 == today's value; gripped → NaN at any d; glide then depth | `tests/test_connection.cpp` (target `tests/CMakeLists.txt:421-462`) | Step 2 |
| `tick()`: `CompScalar::Signal` connected to a signal is published unscaled while `Opacity` (also connected) scales; `Composition::eff(Signal)` feeds `ctx.signalDepth` next tick | `tests/test_connection.cpp` | self-exemption |
| Effect-param twins render value after Step 0: `slot.effParam(p) == applyDepth(manual, shaped, d)`; source-param twin stores NaN when gripped (fail-first against today's skip at `ConnectionEngine.cpp:332-337`) | `tests/test_connection.cpp` | Step 0 |
| `MacroBank::updateValues(signals, d)` blend; legacy `sourceSignalId` path | `tests/test_routing_engine.cpp` (already links MacroBank per `tests/CMakeLists.txt:463`) or test_connection | path F |
| `MappingEngine::processFrame(snap, chain, d)`: d=0 → `defaultValue`; d=1 bit-identical to the existing single-store cases | `tests/test_mapping_engine.cpp` | path G |
| `masterSignal` round-trips; absent key loads 1.0 | `tests/test_composition.cpp:369, 474` | Step 1 |
| `compScalarPath("signal")` resolves; a funnel write lands in `masterSignal`; a Held grip refuses a Lane write | `tests/test_manual_write.cpp:88, 206` | Step 4/5 |
| Live gate (Harmony, production build, `.harmony/probe-*.sh` style): load a comp with Bass→clip Scale connected and a Beat Ripple clip; `POST /api/set_master_signal 0` → `/api/composition` `live.scale` == manual and two `render_frame`s 1 s apart are PSNR-identical; `1.0` → they differ; OSC `/audiodna/signal 0.3` and a MIDI CC binding move the TopBar fader (sync-back) | | end-to-end |

### "Done" looks like
Build + `ctest` green; Step-0 gate; the live gate above; `grep -n "signalDepth" src/connect/
ConnectionEngine.cpp` shows exactly one read in `evaluate` plus the exemption in `tick`; CLAUDE.md
"Audio Uniform System" gains one sentence (levels scale with Master Signal, clocks/labels do not)
and the P22 REST/OSC lists gain the two new addresses.

---

## 6. RT-safety / threading (VERIFIED against the existing crossings)
- Nothing in the audio callback or the analysis thread changes; `CombinedCallback::inputGain`
  stays as is. No new allocation in any tick (2 flops per connection; `scaleSignalLevels` is a
  fixed field list on a POD).
- Writers: message thread only — `manualWrite` asserts it (`MainComponent.cpp:3298`).
- Message-thread readers (engine, macro, mapping ticks): same thread as writers.
- GL-thread readers: `Composition::eff(CompScalar::Signal)` = `LiveValue` relaxed atomic + plain
  float fallback, the same "A2 known-deferred crossing" `masterOpacity` uses today
  (`Renderer.cpp:726`, lane-3 plan §4.4); OutputRenderer gets an explicit `std::atomic<float>`.
  No new mutex; the standalone-source `activeSourceMutex_` is pre-existing (`Renderer.cpp:80-101`).

---

## RISKS — and the strongest counterargument

1. **Strongest counterargument: "Boris said *one* slider — 'taking the gain slider'."** Merging
   is cheaper to explain but wrong in the room: the first time he drags it down to calm a
   breakdown, the beat wheel stops and the take goes quiet (`CombinedCallback.h:76-79,129`). It
   loses because the failure is silent (he will not connect "BPM lost" to a visuals fader).
   Mitigation if he insists on one visible slider: Gain leaves the top bar for Preferences → Audio
   (it already has a hidden twin, `MainComponent.h:354`); the mechanism above is unchanged.
2. **Step 0 changes rendering for existing connected effect params** (RANGE/INVERT now honoured;
   `paramValues` no longer overwritten). Verify with a saved comp that has connected effect
   params before/after; state it in the commit. Also `paramLive` may be shorter than
   `paramValues` for a slot rendered before its first engine tick — fall back, never resize on GL.
3. **Depth on the wrong side of the glide/smoothing** would make the fader "breathe" the EMA.
   Spec places it after both; test pins the order via the glide case.
4. **Self-connection loop** if the exemption is forgotten: a Macro→Signal connection would decay
   toward the manual value tick by tick. Pinned by the `tick()` test.
5. **Float round-trip at d = 0 for scalars**: `toModel(toNorm(manual))` is exact for
   opacity/params/speed/position/rotation; `normExpScale` floors at 0.01 (`ScalarParams.h:47`) —
   a clip/layer scale below 0.01x would read 0.01x at d=0. Cosmetic; note in the test.
6. **Persisted 0** (Q3): a show saved at 0% boots silent-looking; the fader shows it. If Boris
   prefers boot-at-100%, skip the `toVar` line — everything else stands.
7. **INFERRED items for the builder to verify**: `onParamChanged` consumers (Step 0.5); TopBar
   width at minimum window size (Step 4); v1 `MappingEngine` reachability (path G) — if it is
   truly unreachable in v2, the `processFrame(depth)` change is still correct and cheap.
8. What this does NOT do: SignalBar meters stay raw (decided, 3.2); `RoutingEngine` stays dead;
   checkpoint0 does not restore comp scalars (shared gap, Step 5).

---

## QUESTIONS FOR BORIS (taste / product only — max 3)

1. **Gain slider: keep it visible in the top bar (left, next to Audio), or move it into
   Preferences → Audio so the bar shows only the two right-hand faders?**
   Recommendation: keep it visible — it is the "mic is too quiet tonight" knob and you want it
   reachable; it does a different job (see plain words above).
2. **At 0%, should effects that read the beat clock directly keep pulsing with the beat, or
   should the beat also freeze?** Recommendation: keep pulsing — clips keep playing and time keeps
   running anyway, so 0% means "the music no longer moves my sliders", not "freeze the picture".
   Beat-locked oscillators on sliders DO stop moving them at 0%.
3. **Save the fader's position with the composition (like Master), or always start a show at
   100%?** Recommendation: save it, like Master — it is show state and the fader shows where it is.

---

## 10. Adjacent findings (Boris's other two asks; facts only, for the sibling lanes)

**A. Top-right "Master:" is not the Composition "Master" today (VERIFIED).** TopBar
`masterLevelSlider_` → `Renderer::setMasterLevel` (`MainComponent.cpp:629-632`) → a first dim
at `Renderer.cpp:676-705` (`masterLevel_`); CompositionInspector "Master" → `masterOpacity` → a
second, independent dim at `Renderer.cpp:708-748`. Two multipliers in series; neither follows the
other; the TopBar one is not persisted, not recorded, not in the grip chain; REST
`/api/status.masterLevel` (`ApiServer.cpp:287`) reports only the TopBar one. Fix shape = Step 4's
wiring with `compScalarPath("opacity")`: drag → Held grip through `manualWrite`; sync back in
`TopBar::timerCallback` from `composition_.eff(CompScalar::Opacity)`; delete the `masterLevel_`
block (keep `getMasterLevel()` returning `eff(Opacity)` for REST compat). Build both faders the
same way in one lane. The "Video Opacity" twin to delete is `CompositionInspector.cpp:151-163,
414, 550`.

**B. Right-click does not reset (VERIFIED mechanism, three causes).**
(1) `ResettableSlider::mouseDown` swallows the right-click and does nothing when no default was
set (`src/ui/UniversalParamControl.h:30-39`); 34 `ResettableSlider`s never call
`setDefaultValue` — ClipInspector `clipWidthSlider_/clipHeightSlider_`; CompositionInspector
`opaqueCycleSlider_/transparentCycleSlider_/effectCycleSlider_`; LayerInspector
`width/height/transitionDuration/keyThreshold/keySoftness/dryWet/rotSpeed/feedback*` (13);
MappingEditor (5); SignalInspector (7); UPC `valueSlider_/rangeMin/rangeMax`.
(2) A `UniversalParamControl` seeds its inner slider's default only when its owner calls
`UPC::setDefaultValue` (`UniversalParamControl.h:60`); the scalar-bound controls never do
(`CompositionInspector.cpp:134-141, 407-418` and the Layer/Clip equivalents), so right-click on
the track is a no-op and right-click on the label resets to the header default 0.5
(`UniversalParamControl.h:122`) — wrong for opacity (1.0) — although the correct default already
exists as `ScalarDef::defaultNorm` (`ScalarParams.h:23, 110-118`). (3) For a CONNECTED control the
track-path reset writes the manual field without a `gripTouch` (contrast `UniversalParamControl.
cpp:291-300`), so the engine's next tick overrides it within 8 ms — Ruling A by design, reads as
"doesn't reset". Fix plan (~30 lines): `bindScalarControls` sites call
`setDefaultValue(defs[i].defaultNorm)`; `ResettableSlider` gains an `onRightClickReset` callback
that UPC wires to `gripTouch` + `onValueChanged`; a `ResettableSlider` without a default forwards
the event to its parent instead of swallowing it; sweep the 34 sliders (`setDefaultValue(getValue())`
right after each ctor's `setValue`). Verify by right-clicking one slider of each class in the app.

---

## 11. Summary for Harmony (10 lines)
1. Gain = pre-analysis mic trim inside the RT callback (`CombinedCallback.h:66-79`), mic-mode only, also scales recorded take audio (:129), inert in file mode (:91-101). Keep it; do not repurpose.
2. New control: `CompScalar::Signal` / `Composition::masterSignal`, default 1.0, 0..1, TopBar "Signal:" left of "Master:" (`TopBar.cpp:520-532`), wired through `manualWrite(compScalarPath("signal"))` exactly like OSC/MIDI master opacity (`MainComponent.cpp:2073, 6729`).
3. Semantics: output-side depth `manual + d*(driven - manual)` in `ConnectionEngine::evaluate` after the glide (`ConnectionEngine.cpp:176-177`), one read of `ctx.signalDepth`; own connection exempt.
4. Shader-direct uniforms: pure `scaleSignalLevels(snapshot, d)` at `Renderer.cpp:233` and `OutputWindow.cpp:104` — levels scale, clocks/labels untouched, pulses off only at 0.
5. Legacy paths covered: `MacroBank::updateValues` (`MacroBank.h:74-87`), v1 `MappingEngine` (default-anchored, `MappingEngine.cpp:242-243`).
6. PREREQUISITE (Step 0): effect/source-param twins are render-dead — `Clip.h:90 effParam()` has zero callers; `CompositorEngine.cpp:372-376` and `Renderer.cpp:1101` upload raw fields; live modulation is `EffectStackView.cpp:196` / `ClipInspector.cpp:893` overwriting manual values. Repoint per lane-3 plan §11 first (also fixes render-dead RANGE/INVERT on effect params).
7. Persistence: `"masterSignal"` in `Composition::toVar/fromVar` (guarded; absent → 1.0). Q3 to Boris.
8. Bindability: `Binding::Action::MasterSignal` appended at enum END (int-serialised, `BindingManager.cpp:228`), OSC `/audiodna/signal`, REST `/api/set_master_signal` + automatic `live.signal`; recorder `scalar` lane automatic via the `onManualWrite` hook (`MainComponent.cpp:2023-2027`, `Program.cpp:98-99`).
9. RT-safe: no audio/analysis-thread change; GL reads via the same LiveValue crossing as `masterOpacity` (`Renderer.cpp:726`); OutputRenderer gets one `std::atomic<float>`.
10. Adjacent: top-right Master and Composition Master are two independent dims (`Renderer.cpp:676-705` vs `708-748`); right-click reset fails because 34 sliders have no default and scalar UPCs never receive `ScalarDef::defaultNorm` (`UniversalParamControl.h:30-39, 60, 122`).

STATUS: DONE
