# s166 recon — tempo/timing plumbing to the video side

READ-ONLY. All line numbers re-derived fresh from source on 2026-09-05 (not
copied from prior `.harmony/` docs — those were read only for leads, per the
dispatch's hard rule, and every claim below was independently re-grepped).

---

## 1. BPM today — production, storage, consumers

**Production (three converging sources, all funnel into `BPMTracker`):**
- Default: real-time aubio-based tempo tracking. `BPMTracker::process()`
  (`src/analysis/BPMTracker.h:83`) runs a 5-stage stabilization pipeline
  (range gate → confidence gate → octave correction → median filter →
  hysteresis lock — `BPMTracker.h:7-14`) and exposes the result via
  `bpm()` (`BPMTracker.h:94`).
- Tap tempo: `TopBar`'s Tap button → `onTapTempo` callback →
  `tracker->setManualBPM(tappedBPM)` (`src/MainComponent.cpp:555-557`).
  Also reachable via OSC/API-triggered paths at `MainComponent.cpp:1774`,
  `:1824`, `:5781` (same call).
- Ableton Link: `MainComponent.cpp:3064-3076`, in the ~30Hz `timerCallback`
  block — when `linkSync_.isEnabled()`, `linkSync_.getBPM()` is pushed into
  `tracker->setManualMode(true); tracker->setManualBPM(...)`. Link is
  one-directional (feeds the tracker); nothing in the app calls
  `LinkSync::setBPM` — confirmed by two patterns: `grep -rn "setBPM(" src`
  → no non-`LinkSync` hits, and `grep -rn "linkSync_\." src/MainComponent.cpp`
  → only `isEnabled()`, `update()`, `getBPM()` at the cited lines.

**Storage:** `AnalysisThread::run()` (single function, `src/analysis/AnalysisThread.cpp:53-372`)
sets `snap->bpm = bpmTracker_->bpm()` at **line 173** ("stage 5" per the
file's own stage comments), inside its per-hop loop, on the analysis thread.
It is published to the `FeatureBus` at `AnalysisThread.cpp:310`
(`featureBusWriter_.publishWrite()`), i.e. *after* everything described
below.

**Every real consumer of `FeatureSnapshot::bpm` (or `MappingSource::BPM`),
found via `grep -rn "\.bpm\b\|->bpm\b\|MappingSource::BPM\b" src` and cross-checked with `grep -rn "u_bpm"`:**

| Consumer | Citation | What it does |
|---|---|---|
| GenreDetector | `src/analysis/GenreDetector.cpp:36,50,70,88,104,139` | Absolute-BPM-range genre scoring (see below) |
| Mapping system | `src/mapping/MappingEngine.cpp:84` | `case MappingSource::BPM: return snap.bpm;` — generic per-effect-parameter routing, message thread |
| SignalRegistry "Tempo" | `src/signal/SignalRegistry.cpp:26` | Registers an `AudioSignal` wrapping `MappingSource::BPM`, visible in the Audio submenu as "Tempo" |
| TopBar readout | `src/ui/TopBar.cpp:108,227,255-256,370,413` | Numeric BPM label + beat-flash gating (`hasBpm = displaySnap_.bpm > 0.0f && trackerState >= 1`) |
| AudioReadoutPanel | `src/ui/AudioReadoutPanel.cpp:35,125-126` | Numeric readout only |
| Renderer (Clip BPMSync) | `src/render/Renderer.cpp:1173-1181, 1237-1250` | Real playback-rate math (see §4) |
| CompositorEngine | `src/render/CompositorEngine.cpp:1445` | `u_bpm` uniform upload, GL thread |
| EffectChain | `src/effects/EffectChain.cpp:355` | `u_bpm` uniform upload, GL thread |
| ProceduralSource | `src/sources/ProceduralSource.cpp:197-198` | `u_bpm` uniform upload, GL thread |
| ApiServer | `src/api/ApiServer.cpp:248,542,587,623` | HTTP GET exposes `bpm`; PUT can inject a value into the snapshot |
| TestServer | `src/test/TestServer.cpp:425` | Test-only snapshot injection, not production |

**GenreDetector — why "ABSOLUTE-range-based" is accurate.**
`GenreDetector::computeScores()` (`src/analysis/GenreDetector.cpp:23-45`)
reads `float bpm = features.bpm;` (line 36) and gates all BPM-dependent
scoring on `bpmLocked = (features.trackerState == 2)` (line 40). It then
scores against **hard-coded absolute BPM windows**, e.g. House
115–135 (line 50), Techno 120–150 (line 70), DnB 155–185 (line 88),
Hip-Hop 75–105 (line 104) — a bell curve centered on a fixed number
(e.g. 125 for House, line 52). These are not relative to any reference
tempo or octave-normalized; they are literal BPM values. Critically,
this happens **inside the same function, same per-hop iteration, same
`snap` object, before publish**: `AnalysisThread.cpp:173` sets `snap->bpm`,
then `AnalysisThread.cpp:259-279` ("stage 13") builds
`GenreDetector::Features gf; gf.bpm = snap->bpm;` (line 260) and calls
`genreDetector_->process(gf)` (line 275) — both inside `void
AnalysisThread::run()` (`AnalysisThread.cpp:53`, the only enclosing
function in that line range, confirmed via
`grep -n "^void AnalysisThread::\|^AnalysisThread::" AnalysisThread.cpp`).
Any multiplier applied to `snap->bpm` upstream of or inside this stage
moves a track's true BPM out of its correct absolute bucket (e.g. a 125
BPM house track scaled ×2 reads as 250, outside every genre's window).

---

## 2. bpmMultiplier — write site, serialization, consumers, candidate seams

**Confirmed dead, fresh two-pattern check:**
```
grep -rn "bpmMultiplier" --include="*.cpp" --include="*.h" .   (excl. build/)
→ tests/test_composition.cpp:143,180 (write + equality assert only)
→ src/ui/TopBar.cpp:300                (only write site)
→ src/model/Composition.h:50,121,179,270 (decl / reset / serialize / deserialize)

grep -rn "\.bpmMultiplier\|->bpmMultiplier" --include="*.cpp" --include="*.h" .
→ identical set, same three sites
```
Zero read-consumers exist anywhere in `src/`. `int bpmMultiplier = 1;` is
declared `Composition.h:50` (comment: `-4=÷4, -2=÷2, 1=×1, 2=×2, 4=×4`),
reset to `1` in `initDefault()` (`:121`), and round-tripped through JSON
(`toVar()` `:179`, `fromVar()` `:270`). `TopBar::handleMultiplierButton(int)`
(`src/ui/TopBar.cpp:298-323`) writes `composition_.bpmMultiplier = multiplier;`
at line 300 — `composition_` is a live reference
(`Composition& composition_;`, `TopBar.h`), not a copy — then fires
`onBpmMultiplierChanged(multiplier)` at lines 322-323. That callback
(`std::function<void(int)> onBpmMultiplierChanged;`, `TopBar.h:27`) is
**never assigned**: `grep -rn "onBpmMultiplierChanged\s*="` and
`grep -rn "\.onBpmMultiplierChanged\|->onBpmMultiplierChanged"` (excluding
its own declaration/invocation) both return nothing. Second dead path,
not a working one. Buttons are real and wired
(`multDiv4Button_`…`multX4Button_`, `TopBar.h:79-83`,
`handleMultiplierButton` bound at construction) — clicking them changes
button highlight color and nothing else.

**Candidate seams for applying a multiplier so it scales the VIDEO, without
touching GenreDetector — three structurally different points, verified this
session:**

1. **`MappingEngine::extractSource()`, `src/mapping/MappingEngine.cpp:49-...`,
   the `case MappingSource::BPM: return snap.bpm;` at line 84.** This is the
   single extraction point for the generic per-effect-parameter routing
   system (the "connect any parameter to Tempo" path a user builds via
   the Mapping UI / `SignalRegistry`'s "Tempo" signal). `processFrame()`
   (`MappingEngine.cpp:144`) runs on the **message thread only**
   (documented at `MappingEngine.h:21-25` and asserted at `.cpp:150`) and
   is the sole caller (comment: "no other thread may run processFrame
   concurrently"). Scaling *only* at this line affects exactly the set of
   user-created Mappings that target BPM/Tempo as a source — it never
   touches `AnalysisThread`, `FeatureBus`, or `GenreDetector` (those already
   ran, upstream, on a different thread, before this read). `MappingEngine`
   currently holds no `Composition*`/multiplier reference — this would need
   to arrive as a new parameter to `processFrame` or a settable member; no
   new thread-crossing is required since the caller of `processFrame`
   (`MainComponent`'s `MappingTickTimer`) already runs on the message
   thread alongside `TopBar`'s `composition_`.
   Since BPM is a plain non-cyclic scalar (unlike `beatPhase`, a wrapped
   [0,1) ramp), a multiply here has no divide-direction phase-math issue.

2. **`Renderer.cpp`'s two `Clip::TransportMode::BPMSync` branches** —
   `src/render/Renderer.cpp:1173-1181` (video: `player->setSpeed(clip->videoBeats / clip->beatDivision)`,
   itself not yet BPM-scaled, only gated on `snap.bpm > 0.0f`) and
   `:1237-1250` (image sequence: `secondsPerCycle = clip->beatDivision * 60.0f / snap.bpm;`
   at line 1249). Runs on the **GL/render thread**, where `Renderer` already
   holds a raw `Composition*` and reads several other plain scalar
   `Composition` fields directly with no synchronization — confirmed fresh:
   `composition_->decks` (line 460), `composition_->crossfaderBlendMode`
   (552), `composition_->activeDeckIndex` (568),
   `composition_->globalTransitionSpeed` (587),
   `composition_->compPositionX/Y/compScale/compRotation/compAnchorX/Y`
   (1918-1923). Reading `composition_->bpmMultiplier` here would match
   this existing pattern, not introduce a new one.

3. **The one shader that reads `u_bpm` for real math** —
   `src/render/EmbeddedShaders.h:10733`
   (`p.x += u_time * (u_bpm > 0.0 ? u_bpm / 120.0 : 1.0) * 0.3;`, inside
   `sourceStructuralLandscape`). This is fed by three separate GL-thread
   upload call sites — `CompositorEngine.cpp:1445`, `EffectChain.cpp:355`,
   `ProceduralSource.cpp:197-198` — **none of which currently hold any
   `Composition*` reference** (confirmed: `grep -n "Composition\b"
   src/render/CompositorEngine.h src/render/CompositorEngine.cpp
   src/effects/EffectChain.h src/sources/ProceduralSource.h` → zero real
   hits; `EffectChain.h:161` has only an unrelated comment mentioning
   "Composition-mutation LAW pattern"). This is a **correction** to the
   idea that "GL thread ⇒ Composition already available" — that's true for
   `Renderer` specifically, not for these three classes, which would need
   new plumbing (an extra parameter or reference) to reach the multiplier
   at all.

None of the three seams touches `AnalysisThread`, `FeatureBus`, or
`GenreDetector`. Seam 1 is the most architecturally central (it's the
generic "any parameter, any signal" routing point the owner's framing
describes) and the cheapest to wire (message-thread-only, no new
cross-thread channel). Seams 2 and 3 are narrower (one feature / one
shader each) but seam 3 needs new plumbing into three classes that don't
have it today.

---

## 3. Beat phase vs BPM in the 243 embedded shaders

Total shader source strings in `src/render/EmbeddedShaders.h`:
`grep -c '^inline const char\* ' EmbeddedShaders.h` → **243** (each is a
`inline const char* name = R"(...)";` declaration).

Per-shader-block tally (Python, block-bounded by successive
`^inline const char\* ` lines, so `sourceDualPlaneDrift`'s internal
multi-pass repetition doesn't inflate the block count):

- **51 of 243** blocks declare `uniform float u_beatPhase;`
  (`grep -c "uniform float u_beatPhase;"` → 56 raw line-matches, but one
  block, `sourceDualPlaneDrift` (`EmbeddedShaders.h:10018-10984`, a large
  multi-pass shader), declares it **6 separate times** internally —
  51 unique shaders + 5 extra = 56, reconciled).
- **1 of 243** blocks (`sourceStructuralLandscape`) declares
  `uniform float u_bpm;`.
- Of the 51 that *declare* `u_beatPhase`, only **8 blocks actually
  reference it anywhere in their GLSL body** outside the declaration line
  itself (verified by scanning each block's lines for any non-declaration
  occurrence of the substring). The other **43 declare it and never use
  it**. Spot-checked `sourcePerlinNoise`
  (`EmbeddedShaders.h:2548-2557`): the uniform is declared as part of a
  documented boilerplate block — the comment two lines above it literally
  reads *"Common uniforms: u_time, u_resolution, u_rms, u_bass, u_mid,
  u_high, u_beatPhase, u_spectralCentroid, u_onsetStrength"*
  (`EmbeddedShaders.h:2545-2546`) — and is never read again in that
  shader's ~100+ remaining lines.
- The 1 shader that declares `u_bpm` (`sourceStructuralLandscape`,
  `EmbeddedShaders.h:10708-10733`) does use it, for real math, at line 10733.

**This is a nuance the "56 of 57 phase-driven" framing glosses over**: the
prior recon's ratio is correct as a *declaration* count (51 unique +
duplicates = 56, vs 1), but as an *actual-GLSL-math* count it's closer to
**8 shaders genuinely animate on beat phase, 1 on raw BPM, and 43 declare
a beat-phase uniform that their own shader body never reads** — which a
GLSL linker will typically optimize away (an unreferenced uniform commonly
gets stripped, meaning the CPU-side `glUniform1f(loc("u_beatPhase"), ...)`
calls in `CompositorEngine.cpp:1433` / `EffectChain.cpp:330` /
`ProceduralSource.cpp:169-170` would silently no-op via `loc() == -1`
for those 43 — this is a plausible-but-unverified inference about driver
behavior, not something this recon could execute/prove without building).
The remaining 243 − 51 − 1 = 191 shaders reference neither uniform at all
— they react to other audio uniforms directly (`u_rms`, `u_bass`,
`u_spectralFlux`, etc., all confirmed present in the same files), which is
already exactly the "connect to an audio signal" behavior the owner wants;
they are simply not tempo-reactive at all today, by neither phase nor BPM.

**Plain-English difference between phase and BPM as inputs:** `beatPhase`
is a **sawtooth ramp in [0,1)** that resets on every beat and free-runs at
a rate derived from the locked BPM (`BPMTracker.h:16-17`,
`BPMTracker::updatePhase`) — it's already "the beat" as a continuously
animatable value, ideal for anything a shader wants to pulse/rotate/loop
once per beat (`sin(u_beatPhase * 6.28318 + ...)`,
`EmbeddedShaders.h:8741`). Raw `bpm` is a **static number** (60-200) that
says how *fast* those beats are — useful for computing a *rate* (frames
per beat, cycle duration) but not itself a phase you can feed straight
into a periodic function without also tracking elapsed time (which is
exactly what `sourceStructuralLandscape` does: `u_time * (u_bpm/120.0)`,
scaling elapsed time by a tempo ratio rather than reading a phase). A
multiplier on `beatPhase` genuinely needs the wrap-aware treatment the
prior packet described (integer multiplies of a `frac()` ramp are clean;
divides need parity state from `beatInBar`/`barCount`) — that trap is real
and independent of the declaration-vs-usage finding above.

---

## 4. The dead parameter-source trio — BPMSync, ClipPosition, Timeline

All three are `enum class SourceMode : uint8_t` values on
**`UniversalParamControl`** (`src/ui/UniversalParamControl.h:61-70`), the
per-parameter source-picker widget used throughout the Inspector (Effect
stack rows, Source params). This is a **different** `BPMSync` than
`Clip::TransportMode::BPMSync` (§1/§2 above, which IS live) — same word,
two unrelated features; do not conflate them.

**UI offer (all three are real, clickable menu items):**
- `BPMSync`: a full "BPM Sync" submenu — 4 waveform shapes (Sine, Saw,
  Triangle, Square) × 6 beat divisions (1/4 Beat … 8 Beats), built in
  `buildSourcePickerMenu` (`UniversalParamControl.cpp:369-391`, ids 300-323).
- `ClipPosition`: a single top-level menu item "Clip Position", id 2
  (`UniversalParamControl.cpp:438`).
- `Timeline`: a single top-level menu item "Timeline", explicitly commented
  *"placeholder for future"* (`UniversalParamControl.cpp:440-441`), id 3.

**Selecting any of the three works cosmetically** — `handleSourcePickerResult`
(`UniversalParamControl.cpp:463-...`) sets `sourceMode_`, `sourceName_`,
and the button label for all three (ids 2, 3, and 300-323 branches,
`.cpp:472-521`).

**What computes a value for each — confirmed via the two live consumer
sites that drive real parameter values, `EffectStackView::tickModulation`
(`src/ui/EffectStackView.cpp:140-190`) and
`ClipInspector::tickModulation` (`src/ui/ClipInspector.cpp:822-875`):**
both contain the identical pattern —
```
if ((mode == Signal || mode == Oscillator || mode == Envelope) && signalRegistry_) { ... found = true; }
else if (mode == Macro && macroBank_) { ... found = true; }
```
(`EffectStackView.cpp:161-177`, `ClipInspector.cpp:844-860`). **`BPMSync`,
`ClipPosition`, and `Timeline` are not checked in either `if`/`else if`
chain** — they fall through with `found = false`, so the parameter's value
is simply never written by the modulation tick. Confirmed via a second,
differently-shaped pattern: `grep -rn "SourceMode::" src` outside
`UniversalParamControl.*` shows only `Signal`, `Oscillator`, `Envelope`,
`Macro` referenced by consumers (`EffectStackView.cpp:161-177`,
`ClipInspector.cpp:844-860`) — `BPMSync`/`ClipPosition`/`Timeline` appear
nowhere outside `UniversalParamControl`'s own file.

**What each would mean if implemented:**
- **BPMSync** (per-parameter): drive the parameter with a
  waveform-shaped, beat-division-scaled oscillation — i.e. exactly what
  `OscillatorSignal` (§5) already does, just picked per-parameter with an
  inline shape+division instead of pointing at a shared "Mod 1/Mod 2"
  slot. Would need `handleSourcePickerResult`'s stored shape+division
  (currently only encoded into the display string, e.g. `"Sine 1 Beat"`,
  not into a structured field) to be parsed back out and evaluated against
  `snapshot.beatPhase`/`beatInBar` in the tick functions above.
- **ClipPosition**: drive the parameter from the *active clip's* playhead
  position [0,1]. A dedicated class for exactly this already exists —
  **`ClipPositionSignal`** (`src/signal/ClipPositionSignal.h:8-45`),
  registered in `SignalRegistry::initDefaults()`
  (`SignalRegistry.cpp:76-80`, hidden by default, `Type::Audio`) — but it
  is itself dead: `updateFromClip()` and `setCurrentPosition()`
  (`ClipPositionSignal.h:20,26`) and `SignalRegistry::getClipPositionSignal()`
  (`SignalRegistry.cpp:183-190`, declared `SignalRegistry.h:45`) have
  **zero callers anywhere** —
  `grep -rn "getClipPositionSignal\|updateFromClip\|setCurrentPosition" src`
  returns only the declaration/definition sites themselves. So there are
  actually **two independent dead "Clip Position" paths**: the
  `SourceMode::ClipPosition` enum branch (never checked by a consumer, §
  above), and — even if a user instead picks "Clip Position" from the
  *Audio* submenu (which routes through the live `Signal` branch, since
  `ClipPositionSignal::getType() == Type::Audio`) — the signal itself is
  permanently stuck at its default `0.0f` because nothing ever calls
  `setCurrentPosition`/`updateFromClip` to feed it the real playhead value.
- **Timeline** (per-parameter): per-parameter keyframed automation curves
  (the comment says as much, `UniversalParamControl.cpp:440`) — no keyframe
  storage, editor, or evaluator exists anywhere in `src/`
  (`grep -rn "keyframe" src` → no hits outside this one comment). This is
  the least-built of the three: not even a placeholder data structure.

These three are exactly the "timed" sources the owner's framing describes
(BPM-synced, clip-position-synced, keyframe-timed) — none is a UI bug,
all are genuinely unfinished plumbing with a real, named destination
(the same `found = true` branch that `Signal`/`Oscillator`/`Envelope`/`Macro`
already use in both tick functions) still open.

---

## 5. Oscillators — do they exist today, and are they tempo-locked?

Yes. `Signal::Type` (`src/signal/Signal.h:12-17`) has exactly three
variants: `Audio`, `Oscillator`, `Envelope` — no independent free-running
Hz-based LFO type exists.

- **`OscillatorSignal`** (`src/signal/OscillatorSignal.h:8-77`): 5 wave
  shapes (Sine, SawUp, SawDown, Triangle, Square, line 13), a
  `beatDuration_` (cycle length in beats: 0.25…8, comment line 16-17), a
  `phaseOffset_`, and an `amplitude_`. Its `getValue()` (lines 23-57)
  derives phase from `snapshot.beatPhase + snapshot.beatInBar` (line 28) —
  **beat-phase-locked, not raw-BPM-driven** (it never reads
  `snapshot.bpm`).
- **`EnvelopeSignal`** (`src/signal/EnvelopeSignal.h:10-126`): custom
  control-point curve (Linear/Exponential/SCurve interpolation, lines 19,
  68-78), same `beatDuration_` + phase derivation pattern from
  `beatPhase + beatInBar` (line 36) — also beat-phase-locked.
- **Instantiation is fixed, not user-extensible today**: both classes are
  only ever constructed in `SignalRegistry::initDefaults()`
  (`SignalRegistry.cpp:65-73`) — exactly two default instances, "Mod 1"
  (`OscillatorSignal`, Sine, 1-beat) and "Mod 2" (`EnvelopeSignal`,
  4-beat). `grep -rn "\.addSignal(\|->addSignal(" src` (the only public
  method that could add more) returns **zero callers anywhere** — no UI
  path exists to create a third oscillator or envelope slot; users pick
  from exactly these two pre-built modulation sources via the
  Oscillator/Envelope submenus (`UniversalParamControl.cpp:395-431`).

So: real oscillators/envelopes exist, are genuinely usable today
(confirmed live consumers in §4's `found = true` branches), but (a) there
are only 2 fixed slots, not a general "any parameter gets its own LFO"
system, and (b) they are all beat-phase-locked already — none is a
free-running, tempo-independent Hz oscillator. This means a meaningful
chunk of "connect any parameter to an oscillator, all timed" already
exists structurally; what's missing is (i) more than 2 slots / per-parameter
inline oscillators (which is what the dead `SourceMode::BPMSync` in §4 was
apparently meant to become), and (ii) a raw-BPM-scaled variant for anyone
who wants oscillation speed to track a multiplier rather than the beat
grid directly.

---

## Cross-cutting note for the design decision

The clean way to make "audio controls the video, all timed, connect
anything to anything" cohere with a working BPM multiplier is: **the
multiplier belongs on the *consumption* side (Mapping/Oscillator
evaluation, seam #1 in §2, or a new structured field for the dead
per-parameter `BPMSync` mode in §4), not on the *production* side
(`AnalysisThread`/`FeatureBus`/`GenreDetector`)**. Every real analytical
consumer (GenreDetector's absolute windows, TopBar's/AudioReadoutPanel's
readouts, ApiServer's external exposure, Ableton Link's one-directional
feed) needs the *true* detected BPM to stay true. Every visual/timed
consumer that would benefit from a multiplier (Mapping's `BPM` source,
per-parameter `BPMSync` oscillation once built, the one raw-`u_bpm`
shader, Clip `BPMSync` transport) sits downstream of the snapshot and can
be scaled independently without corrupting anything upstream.
