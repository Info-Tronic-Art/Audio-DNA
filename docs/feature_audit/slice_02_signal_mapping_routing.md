# Slice 02: Signal System + Mapping + Routing

Audit of the audio-reactivity engine — every signal, curve, mapping field, route field, macro, and suggestion rule with file:line citations.

---

## Section 2: Signal System

### 2.1 Signal sources — complete `MappingSource` enum listing

Enum declared in `src/mapping/MappingTypes.h:6-87`. Feature extraction (enum→FeatureSnapshot field) in `src/mapping/MappingEngine.cpp:43-128`.

| # | Enum value | File:line | Snapshot field | Category group |
|---|------------|-----------|----------------|----------------|
| 0  | `RMS`              | `MappingTypes.h:9`  | `snap.rms` (`MappingEngine.cpp:48`) | Amplitude |
| 1  | `Peak`             | `MappingTypes.h:10` | `snap.peak` (`MappingEngine.cpp:49`) | Amplitude |
| 2  | `RmsDB`            | `MappingTypes.h:11` | `snap.rmsDB` (`MappingEngine.cpp:50`) | Amplitude |
| 3  | `LUFS`             | `MappingTypes.h:14` | `snap.lufs` (`MappingEngine.cpp:53`) | Loudness |
| 4  | `DynamicRange`     | `MappingTypes.h:15` | `snap.dynamicRange` (`MappingEngine.cpp:54`) | Loudness |
| 5  | `TransientDensity` | `MappingTypes.h:16` | `snap.transientDensity` (`MappingEngine.cpp:55`) | Loudness |
| 6  | `SpectralCentroid` | `MappingTypes.h:19` | `snap.spectralCentroid` (`MappingEngine.cpp:58`) | Spectral |
| 7  | `SpectralFlux`     | `MappingTypes.h:20` | `snap.spectralFlux` (`MappingEngine.cpp:59`) | Spectral |
| 8  | `SpectralFlatness` | `MappingTypes.h:21` | `snap.spectralFlatness` (`MappingEngine.cpp:60`) | Spectral |
| 9  | `SpectralRolloff`  | `MappingTypes.h:22` | `snap.spectralRolloff` (`MappingEngine.cpp:61`) | Spectral |
| 10 | `BandSub`          | `MappingTypes.h:25` | `snap.bandEnergies[0]` (`MappingEngine.cpp:64`) | 7-Band energies |
| 11 | `BandBass`         | `MappingTypes.h:26` | `snap.bandEnergies[1]` (`MappingEngine.cpp:65`) | 7-Band energies |
| 12 | `BandLowMid`       | `MappingTypes.h:27` | `snap.bandEnergies[2]` (`MappingEngine.cpp:66`) | 7-Band energies |
| 13 | `BandMid`          | `MappingTypes.h:28` | `snap.bandEnergies[3]` (`MappingEngine.cpp:67`) | 7-Band energies |
| 14 | `BandHighMid`      | `MappingTypes.h:29` | `snap.bandEnergies[4]` (`MappingEngine.cpp:68`) | 7-Band energies |
| 15 | `BandPresence`     | `MappingTypes.h:30` | `snap.bandEnergies[5]` (`MappingEngine.cpp:69`) | 7-Band energies |
| 16 | `BandBrilliance`   | `MappingTypes.h:31` | `snap.bandEnergies[6]` (`MappingEngine.cpp:70`) | 7-Band energies |
| 17 | `OnsetStrength`    | `MappingTypes.h:34` | `snap.onsetStrength` (`MappingEngine.cpp:73`) | Onset/rhythm |
| 18 | `BeatPhase`        | `MappingTypes.h:35` | `snap.beatPhase` (`MappingEngine.cpp:74`) | Onset/rhythm |
| 19 | `BPM`              | `MappingTypes.h:36` | `snap.bpm` (`MappingEngine.cpp:75`) | Onset/rhythm |
| 20 | `BarPhase`         | `MappingTypes.h:37` | `snap.barPhase` (`MappingEngine.cpp:76`) | Onset/rhythm |
| 21 | `PhrasePhase`      | `MappingTypes.h:38` | `snap.phrasePhase` (`MappingEngine.cpp:77`) | Onset/rhythm |
| 22 | `BarCount`         | `MappingTypes.h:39` | `snap.barCount` (`MappingEngine.cpp:78`) | Onset/rhythm |
| 23 | `StructuralState`  | `MappingTypes.h:42` | `snap.structuralState` (`MappingEngine.cpp:81`) | Structural |
| 24 | `DominantPitch`    | `MappingTypes.h:45` | `snap.dominantPitch` (`MappingEngine.cpp:84`) | Pitch/harmony |
| 25 | `PitchConfidence`  | `MappingTypes.h:46` | `snap.pitchConfidence` (`MappingEngine.cpp:85`) | Pitch/harmony |
| 26 | `DetectedKey`      | `MappingTypes.h:47` | `snap.detectedKey` (`MappingEngine.cpp:86`) | Pitch/harmony |
| 27 | `HarmonicChange`   | `MappingTypes.h:48` | `snap.harmonicChangeDetection` (`MappingEngine.cpp:87`) | Pitch/harmony |
| 28 | `MFCC0`            | `MappingTypes.h:51` | `snap.mfccs[0]` (`MappingEngine.cpp:90`) | Timbral MFCC |
| 29 | `MFCC1`            | `MappingTypes.h:52` | `snap.mfccs[1]` (`MappingEngine.cpp:91`) | Timbral MFCC |
| 30 | `MFCC2`            | `MappingTypes.h:53` | `snap.mfccs[2]` (`MappingEngine.cpp:92`) | Timbral MFCC |
| 31 | `MFCC3`            | `MappingTypes.h:54` | `snap.mfccs[3]` (`MappingEngine.cpp:93`) | Timbral MFCC |
| 32 | `MFCC4`            | `MappingTypes.h:55` | `snap.mfccs[4]` (`MappingEngine.cpp:94`) | Timbral MFCC |
| 33 | `MFCC5`            | `MappingTypes.h:56` | `snap.mfccs[5]` (`MappingEngine.cpp:95`) | Timbral MFCC |
| 34 | `MFCC6`            | `MappingTypes.h:57` | `snap.mfccs[6]` (`MappingEngine.cpp:96`) | Timbral MFCC |
| 35 | `MFCC7`            | `MappingTypes.h:58` | `snap.mfccs[7]` (`MappingEngine.cpp:97`) | Timbral MFCC |
| 36 | `MFCC8`            | `MappingTypes.h:59` | `snap.mfccs[8]` (`MappingEngine.cpp:98`) | Timbral MFCC |
| 37 | `MFCC9`            | `MappingTypes.h:60` | `snap.mfccs[9]` (`MappingEngine.cpp:99`) | Timbral MFCC |
| 38 | `MFCC10`           | `MappingTypes.h:61` | `snap.mfccs[10]` (`MappingEngine.cpp:100`) | Timbral MFCC |
| 39 | `MFCC11`           | `MappingTypes.h:62` | `snap.mfccs[11]` (`MappingEngine.cpp:101`) | Timbral MFCC |
| 40 | `MFCC12`           | `MappingTypes.h:63` | `snap.mfccs[12]` (`MappingEngine.cpp:102`) | Timbral MFCC |
| 41 | `ChromaC`          | `MappingTypes.h:66` | `snap.chromagram[0]` (`MappingEngine.cpp:105`) | Chroma |
| 42 | `ChromaCs`         | `MappingTypes.h:67` | `snap.chromagram[1]` (`MappingEngine.cpp:106`) | Chroma |
| 43 | `ChromaD`          | `MappingTypes.h:68` | `snap.chromagram[2]` (`MappingEngine.cpp:107`) | Chroma |
| 44 | `ChromaDs`         | `MappingTypes.h:69` | `snap.chromagram[3]` (`MappingEngine.cpp:108`) | Chroma |
| 45 | `ChromaE`          | `MappingTypes.h:70` | `snap.chromagram[4]` (`MappingEngine.cpp:109`) | Chroma |
| 46 | `ChromaF`          | `MappingTypes.h:71` | `snap.chromagram[5]` (`MappingEngine.cpp:110`) | Chroma |
| 47 | `ChromaFs`         | `MappingTypes.h:72` | `snap.chromagram[6]` (`MappingEngine.cpp:111`) | Chroma |
| 48 | `ChromaG`          | `MappingTypes.h:73` | `snap.chromagram[7]` (`MappingEngine.cpp:112`) | Chroma |
| 49 | `ChromaGs`         | `MappingTypes.h:74` | `snap.chromagram[8]` (`MappingEngine.cpp:113`) | Chroma |
| 50 | `ChromaA`          | `MappingTypes.h:75` | `snap.chromagram[9]` (`MappingEngine.cpp:114`) | Chroma |
| 51 | `ChromaAs`         | `MappingTypes.h:76` | `snap.chromagram[10]` (`MappingEngine.cpp:115`) | Chroma |
| 52 | `ChromaB`          | `MappingTypes.h:77` | `snap.chromagram[11]` (`MappingEngine.cpp:116`) | Chroma |
| 53 | `SidechainPump`    | `MappingTypes.h:80` | `snap.sidechainPump` (`MappingEngine.cpp:119`) | Advanced P25 |
| 54 | `SwingRatio`       | `MappingTypes.h:81` | `snap.swingRatio` (`MappingEngine.cpp:120`) | Advanced P25 |
| 55 | `FormantPresence`  | `MappingTypes.h:82` | `snap.formantPresence` (`MappingEngine.cpp:121`) | Advanced P25 |
| 56 | `ResonancePeak`    | `MappingTypes.h:83` | `snap.resonancePeak` (`MappingEngine.cpp:122`) | Advanced P25 |
| 57 | `ReeseBass`        | `MappingTypes.h:84` | `snap.reeseBass` (`MappingEngine.cpp:123`) | Advanced P25 |
| 58 | `Count`            | `MappingTypes.h:86` | sentinel (not a feature, `MappingEngine.cpp:125`) | — |

**Total MappingSource values: 58 real sources + 1 sentinel = 59 enum entries.**

### 2.1.1 Signal base class (`src/signal/Signal.h`)

Base class for all value streams used in routing (`Signal.h:9`).

`Signal::Type` enum (`Signal.h:12-17`):
- `Audio` — wraps a FeatureSnapshot field
- `Oscillator` — BPM-locked waveform
- `Envelope` — custom curve

`Signal::Category` enum (`Signal.h:19-22`): `Amplitude, Bands, Rhythm, Pitch, Chroma, Timbre, Structure, Modulation` (8 categories).

Fields: `name_` (string, `Signal.h:49`), `type_` (`Signal.h:50`), `category_` (`Signal.h:51`), `id_ = 0` (uint32_t, `Signal.h:52`), `visible_ = true` (bool, `Signal.h:53`).

Signal derived types:
- `AudioSignal` (`src/signal/AudioSignal.h:8`) — wraps a `MappingSource`, calls `MappingEngine::extractSource` (`AudioSignal.h:17`).
- `OscillatorSignal` (`src/signal/OscillatorSignal.h:8`).
- `EnvelopeSignal` (`src/signal/EnvelopeSignal.h:10`).
- `ClipPositionSignal` (`src/signal/ClipPositionSignal.h:8`) — exposes `Clip::playheadPosition` normalized to `[inPoint, outPoint]` (`ClipPositionSignal.h:35-40`). Atomic `std::atomic<float> currentPosition_{0.0f}` (`ClipPositionSignal.h:44`).
- `ChainedSignal` (`src/signal/ChainedSignal.h:10`) — one signal modulates another.

### 2.1.2 Default signal registration

`SignalRegistry::initDefaults()` registers these at startup (`src/signal/SignalRegistry.cpp:4-84`):

**Visible audio signals (8)** — `SignalRegistry.cpp:20-27`:

| Signal name | MappingSource | Category |
|-------------|---------------|----------|
| `"Volume"`        | `RMS`            | `Amplitude` (`:20`) |
| `"Sub Bass"`      | `BandSub`        | `Bands` (`:21`) |
| `"Bass"`          | `BandBass`       | `Bands` (`:22`) |
| `"Mid"`           | `BandMid`        | `Bands` (`:23`) |
| `"Air"`           | `BandBrilliance` | `Bands` (`:24`) |
| `"Tempo"`         | `BPM`            | `Rhythm` (`:25`) |
| `"Beat Position"` | `BeatPhase`      | `Rhythm` (`:26`) |
| `"Hit"`           | `OnsetStrength`  | `Rhythm` (`:27`) |

**Hidden audio signals (21)** — `SignalRegistry.cpp:38-60`:

| Signal name | MappingSource | Category |
|-------------|---------------|----------|
| `"Peak"`            | `Peak`             | `Amplitude` (`:38`) |
| `"Punch"`           | `DynamicRange`     | `Amplitude` (`:39`) |
| `"Hits Per Second"` | `TransientDensity` | `Amplitude` (`:40`) |
| `"Low Mid"`         | `BandLowMid`       | `Bands` (`:41`) |
| `"High Mid"`        | `BandHighMid`      | `Bands` (`:42`) |
| `"Presence"`        | `BandPresence`     | `Bands` (`:43`) |
| `"Hit Strength"`    | `OnsetStrength`    | `Rhythm` (`:44`) *(duplicate source of "Hit")* |
| `"Bar Position"`    | `BarPhase`         | `Rhythm` (`:45`) |
| `"Phrase Position"` | `PhrasePhase`      | `Rhythm` (`:46`) |
| `"Bar Count"`       | `BarCount`         | `Rhythm` (`:47`) |
| `"Brightness"`      | `SpectralCentroid` | `Amplitude` (`:48`) *(category note: grouped under Amplitude)* |
| `"Change"`          | `SpectralFlux`     | `Amplitude` (`:49`) |
| `"Noisiness"`       | `SpectralFlatness` | `Amplitude` (`:50`) |
| `"Note"`            | `DominantPitch`    | `Pitch` (`:51`) |
| `"Note Confidence"` | `PitchConfidence`  | `Pitch` (`:52`) |
| `"Chord Change"`    | `HarmonicChange`   | `Pitch` (`:53`) |
| `"Sidechain Pump"`  | `SidechainPump`    | `Amplitude` (`:56`) |
| `"Swing"`           | `SwingRatio`       | `Rhythm` (`:57`) |
| `"Vocal Presence"`  | `FormantPresence`  | `Amplitude` (`:58`) |
| `"Resonance"`       | `ResonancePeak`    | `Amplitude` (`:59`) |
| `"Reese Bass"`      | `ReeseBass`        | `Bands` (`:60`) |

**Default modulation signals (2 visible)**:
- `"Mod 1"` — `OscillatorSignal`, `WaveShape::Sine`, beatDuration=`1.0f` (`SignalRegistry.cpp:64`).
- `"Mod 2"` — `EnvelopeSignal`, beatDuration=`4.0f` (`SignalRegistry.cpp:70`).

**Hidden clip signal (1)**:
- `"Clip Position"` — `ClipPositionSignal`, visible=`false` (`SignalRegistry.cpp:78-82`).

**Total default signals registered: 8 visible audio + 21 hidden audio + 2 modulation + 1 hidden clip = 32 signals.** Note: MappingSource exposes 58 features, so 27 sources (LUFS, RmsDB, SpectralRolloff, StructuralState, DetectedKey, all 13 MFCCs, all 12 Chroma pitch classes — minus any overlap) are not exposed as default signals but are still routable via `MappingSource`.

### 2.2 Oscillators (`src/signal/OscillatorSignal.h`)

`WaveShape` enum (`OscillatorSignal.h:11-14`): **5 shapes**:
1. `Sine` — `0.5 + 0.5*sin(phase*2π)` (`OscillatorSignal.h:40`)
2. `SawUp` — `cyclePhase` (`OscillatorSignal.h:43`)
3. `SawDown` — `1 - cyclePhase` (`OscillatorSignal.h:46`)
4. `Triangle` — `cyclePhase<0.5 ? 2*cyclePhase : 2-2*cyclePhase` (`OscillatorSignal.h:49`)
5. `Square` — `cyclePhase<0.5 ? 1 : 0` (`OscillatorSignal.h:52`)

Parameters:

| Field | Type | Default | Range | File:line |
|-------|------|---------|-------|-----------|
| `shape_`        | `WaveShape` | `Sine`  | 5 values | `OscillatorSignal.h:73` |
| `beatDuration_` | `float`     | `1.0f`  | beats/cycle (e.g., 0.25, 0.5, 1, 2, 4, 8 per comment `:16`) | `OscillatorSignal.h:74` |
| `amplitude_`    | `float`     | `1.0f`  | multiplier on output | `OscillatorSignal.h:75` |
| `phaseOffset_`  | `float`     | `0.0f`  | `[0, 1]` phase wrap offset | `OscillatorSignal.h:76` |

Phase derivation: `totalBeatPhase = snapshot.beatPhase + beatInBar`, `cyclePhase = fmod(totalBeatPhase/beatDuration_, 1.0) + phaseOffset_ wrap` (`OscillatorSignal.h:28-34`).

Getters/setters: `getShape/setShape` (`:60-61`), `getBeatDuration/setBeatDuration` (`:63-64`), `getAmplitude/setAmplitude` (`:66-67`), `getPhaseOffset/setPhaseOffset` (`:69-70`).

### 2.3 Envelopes (`src/signal/EnvelopeSignal.h`)

`EnvelopeSignal::ControlPoint` struct (`EnvelopeSignal.h:13-17`):
- `position: float` default `0.0f`, `[0, 1]` along the cycle
- `value: float` default `0.0f`, `[0, 1]` output value

`EnvelopeSignal::CurveType` enum (`EnvelopeSignal.h:19`): **3 types**:
1. `Linear` — passthrough (`EnvelopeSignal.h:77` default branch)
2. `Exponential` — `t = t*t` (`EnvelopeSignal.h:71`)
3. `SCurve` — smoothstep `t*t*(3-2t)` (`EnvelopeSignal.h:74`)

Default envelope shape (`EnvelopeSignal.h:26-28`): three control points forming linear ramp up then down: `(0, 0) → (0.5, 1) → (1, 0)`.

Fields and defaults:

| Field | Type | Default | File:line |
|-------|------|---------|-----------|
| `points_`        | `vector<ControlPoint>` | 3-point ramp | `EnvelopeSignal.h:119`, init `:26-28` |
| `beatDuration_`  | `float`     | `4.0f` (constructor default `:21`) | `EnvelopeSignal.h:120` |
| `amplitude_`     | `float`     | `1.0f` | `EnvelopeSignal.h:121` |
| `phaseOffset_`   | `float`     | `0.0f` | `EnvelopeSignal.h:122` |
| `curveType_`     | `CurveType` | `Linear` | `EnvelopeSignal.h:123` |
| `oneShot_`       | `bool`      | `false` | `EnvelopeSignal.h:124` |
| `looping_`       | `bool`      | `true`  | `EnvelopeSignal.h:125` |

Methods:
- `getPoints() / setPoints(pts)` with sort (`EnvelopeSignal.h:85-93`).
- `addPoint(pos, val)` with sort (`EnvelopeSignal.h:95-102`).
- Getters/setters for all fields (`EnvelopeSignal.h:105-116`).

One-shot logic (`EnvelopeSignal.h:43-46`): when `oneShot_ && !looping_`, clamp `cyclePhase = min(cyclePhase, 1.0)`.

**No ADSR-style attack/decay/sustain/release fields** — envelope is a free-form control point curve, not a traditional AHDSR.

**No explicit trigger source** — envelope always runs BPM-locked off `beatPhase + beatInBar` (`EnvelopeSignal.h:36`). There is no external event trigger.

### 2.4 Macros (`src/routing/MacroBank.h`)

`MacroBank::Scope` enum (`MacroBank.h:18`): `Clip, Layer, Global`.

`MacroBank::kNumMacros = 8` per scope (`MacroBank.h:16`). Three scopes × 8 = **24 macros total** when all scopes are instantiated. Default name pattern: `"Link " + (i+1)` (`MacroBank.h:51`).

`MacroBank::MacroLink` struct (`MacroBank.h:20-26`):
- `target: RouteTarget` — which parameter this macro drives
- `outputMin: float` = `0.0f`
- `outputMax: float` = `1.0f`
- `inverted: bool` = `false`

`MacroBank::Macro` struct (`MacroBank.h:28-44`):
- `name: string` — user-editable (`:30`)
- `manualValue: float` = `0.5f` (`:31`)
- `currentValue: float` = `0.5f` (`:32`)
- `sourceSignalId: uint32_t` = `0` → **0 = Manual, any nonzero = Signal ID** (`:35`, confirmed by `isManual()` at `:43`)
- `links: vector<MacroLink>` — dynamic number of linked parameters (`:38`)
- `id: uint32_t` = `0` — unique for use as route source (`:41`)

Update rule (`MacroBank.h:64-77`): per frame, each macro's `currentValue` is either `manualValue` (when manual) or `signals.getCachedValue(sourceSignalId)` (when signal-driven).

### 2.5 Curve types — all options (`src/mapping/MappingTypes.h:90-120` + `src/mapping/CurveTransforms.h`)

`MappingCurve` enum (`MappingTypes.h:90-120`). Dispatcher in `CurveTransforms::applyCurve` (`CurveTransforms.h:222-252`). **24 enum values total (including `Count` sentinel) — 23 usable curves**:

| # | Enum value | Formula | File:line (enum) | File:line (impl) |
|---|------------|---------|------------------|------------------|
| 0  | `Linear`        | `x` clamped                                   | `MappingTypes.h:92`  | `CurveTransforms.h:13-16` |
| 1  | `Exponential`   | `x*x` — emphasizes peaks                      | `MappingTypes.h:93`  | `CurveTransforms.h:19-23` |
| 2  | `Logarithmic`   | `log(1+9x)/log(10)` — compresses peaks        | `MappingTypes.h:94`  | `CurveTransforms.h:26-30` |
| 3  | `SCurve`        | `x*x*(3-2x)` smoothstep                       | `MappingTypes.h:95`  | `CurveTransforms.h:33-37` |
| 4  | `Stepped`       | `floor(x*N)/N`, default `N=4`                 | `MappingTypes.h:96`  | `CurveTransforms.h:40-45` |
| 5  | `CircularIn`    | `1 - sqrt(1 - x²)`                            | `MappingTypes.h:99`  | `CurveTransforms.h:53-57` |
| 6  | `CircularOut`   | `sqrt(1 - (x-1)²)`                            | `MappingTypes.h:100` | `CurveTransforms.h:59-64` |
| 7  | `CircularInOut` | piecewise circ                                | `MappingTypes.h:101` | `CurveTransforms.h:66-73` |
| 8  | `BackIn`        | `C3*x³ - C1*x²`, overshoots then settles      | `MappingTypes.h:102` | `CurveTransforms.h:80-84` |
| 9  | `BackOut`       | `1 + C3*t³ + C1*t²` with `t = x-1`            | `MappingTypes.h:103` | `CurveTransforms.h:86-91` |
| 10 | `BackInOut`     | piecewise cubic with overshoot constant C2    | `MappingTypes.h:104` | `CurveTransforms.h:93-103` |
| 11 | `ElasticIn`     | `-2^(10x-10) * sin((10x-10.75)·C4)` spring    | `MappingTypes.h:105` | `CurveTransforms.h:109-115` |
| 12 | `ElasticOut`    | `2^(-10x) * sin((10x-0.75)·C4) + 1`           | `MappingTypes.h:106` | `CurveTransforms.h:117-123` |
| 13 | `ElasticInOut`  | piecewise spring                              | `MappingTypes.h:107` | `CurveTransforms.h:125-133` |
| 14 | `BounceIn`      | `1 - bounceOut(1-x)`                          | `MappingTypes.h:108` | `CurveTransforms.h:157-161` |
| 15 | `BounceOut`     | 4-segment bounce (`n1=7.5625`, `d1=2.75`)     | `MappingTypes.h:109` | `CurveTransforms.h:136-155` |
| 16 | `BounceInOut`   | piecewise bounce                              | `MappingTypes.h:110` | `CurveTransforms.h:163-169` |
| 17 | `CubicIn`       | `x³`                                          | `MappingTypes.h:111` | `CurveTransforms.h:172-176` |
| 18 | `CubicOut`      | `(x-1)³ + 1`                                  | `MappingTypes.h:112` | `CurveTransforms.h:178-183` |
| 19 | `CubicInOut`    | piecewise cubic                               | `MappingTypes.h:113` | `CurveTransforms.h:185-192` |
| 20 | `SineIn`        | `1 - cos(x·π/2)`                              | `MappingTypes.h:114` | `CurveTransforms.h:195-199` |
| 21 | `SineOut`       | `sin(x·π/2)`                                  | `MappingTypes.h:115` | `CurveTransforms.h:201-205` |
| 22 | `SineInOut`     | `-(cos(πx) - 1)/2`                            | `MappingTypes.h:116` | `CurveTransforms.h:207-211` |
| 23 | `Hold`          | step: `0` until `x>=1`, then `1`              | `MappingTypes.h:117` | `CurveTransforms.h:214-218` |
| 24 | `Count`         | sentinel                                      | `MappingTypes.h:119` | — |

Back-easing constants: `kBackC1 = 1.70158`, `kBackC2 = 1.70158*1.525`, `kBackC3 = 2.70158` (`CurveTransforms.h:76-78`).
Elastic constants: `kElasticC4 = 2π/3`, `kElasticC5 = 2π/4.5` (`CurveTransforms.h:106-107`).

**Total curve types: 23 usable + 1 sentinel.**

### 2.6 Mapping fields — `Mapping` struct (`src/mapping/MappingTypes.h:123-140`)

| Field | Type | Default | Purpose | File:line |
|-------|------|---------|---------|-----------|
| `source`            | `MappingSource` | `RMS` | Audio feature | `:125` |
| `targetEffectId`    | `uint32_t`      | `0`   | Index into EffectChain | `:127` |
| `targetParamIndex`  | `uint32_t`      | `0`   | Index into Effect's param list | `:128` |
| `curve`             | `MappingCurve`  | `Linear` | Curve transform | `:130` |
| `inputMin`          | `float`         | `0.0f` | Source normalization range | `:132` |
| `inputMax`          | `float`         | `1.0f` | | `:133` |
| `outputMin`         | `float`         | `0.0f` | Target output range | `:134` |
| `outputMax`         | `float`         | `1.0f` | | `:135` |
| `smoothing`         | `float`         | `0.15f` | EMA alpha (higher = less smoothing) | `:137` |
| `enabled`           | `bool`          | `true` | Active flag | `:139` |

Mapping pipeline (`MappingEngine.cpp:135-216`): Extract (`:179`) → Normalize via `(raw - inputMin)/range`, clamped (`:182-185`) → Curve (`:188`) → Scale (`:191`) → Smooth (EMA, `:194-197`) → **Accumulate** into target param (not assign) (`:200`) → Clamp all targets to `[0,1]` (`:213-214`). Smoother alpha is hot-swapped if `smoothing` field changes (`:195-196`).

### 2.7 Route fields — `Route` struct (`src/routing/Route.h:8-41`)

| Field | Type | Default | Purpose | File:line |
|-------|------|---------|---------|-----------|
| `id`                | `uint32_t`           | `0`              | Unique per-route ID (assigned on add) | `:10` |
| `sourceType`        | `SourceType` enum    | `Signal`         | `Signal` or `Macro` | `:14-15` |
| `sourceId`          | `uint32_t`           | `0`              | Signal ID or Macro ID | `:16` |
| `targetScope`       | `TargetScope` enum   | `Clip`           | `Clip` / `Layer` / `Global` | `:19-20` |
| `targetLayerId`     | `uint32_t`           | `0`              | Which layer | `:21` |
| `targetClipId`      | `uint32_t`           | `0`              | Which clip | `:22` |
| `targetEffectIndex` | `int`                | `0`              | Effect in chain | `:23` |
| `targetParamIndex`  | `int`                | `0`              | Param in that effect | `:24` |
| `outputMin`         | `float`              | `0.0f`           | Output range lower | `:27` |
| `outputMax`         | `float`              | `1.0f`           | Output range upper | `:28` |
| `inverted`          | `bool`               | `false`          | Flip output (`1-x`) | `:29` |
| `dialRangeMin`      | `float`              | `0.0f`           | Input sensitivity lower | `:32` |
| `dialRangeMax`      | `float`              | `1.0f`           | Input sensitivity upper | `:33` |
| `threshold`         | `float`              | `0.0f`           | Source must exceed to activate | `:36` |
| `gain`              | `float`              | `1.0f`           | Post-threshold multiplier | `:37` |
| `falloff`           | `float`              | `0.1f`           | Decay rate below threshold | `:38` |
| `enabled`           | `bool`               | `true`           | Active flag | `:40` |

**Route source enum**: `Route::SourceType = { Signal, Macro }` (2 values, `Route.h:13`).
**Route scope enum**: `Route::TargetScope = { Clip, Layer, Global }` (3 values, `Route.h:19`).

`RouteTarget` helper struct (`Route.h:45-61`): `{scope, layerId, clipId, effectIndex, paramIndex}` with equality operator for fast lookup.

**Total Route fields: 17** (1 ID + 2 source + 5 target + 6 transform + 2 gating + 1 enabled).

### 2.8 Signal connect popup / triangle controls — sources exposed

What's available at any "signal connect triangle" popup, based on code surface:

**Source mode options** (from `Route::SourceType` and signal types):
1. **Manual** — implicit via `Macro::isManual()` (sourceSignalId=0) (`MacroBank.h:43`). Macro-mode route with `manualValue`.
2. **Audio (Signal)** — any `AudioSignal` registered in the registry, wrapping one of 58 `MappingSource` values.
3. **Oscillator (Signal)** — `OscillatorSignal` with 5 wave shapes + beatDuration/amplitude/phaseOffset.
4. **Envelope (Signal)** — `EnvelopeSignal` with custom control points, 3 curve types, one-shot/looping, beatDuration/amplitude/phaseOffset.
5. **Clip Position (Signal)** — `ClipPositionSignal` (hidden by default, registered in `initDefaults`).
6. **Macro (Route source)** — `Route::SourceType::Macro` points to a `MacroBank::Macro` id (up to 8 per scope × 3 scopes = 24).
7. **Chained Signal** — `ChainedSignal` with 4 `ChainMode` values (`Multiply`, `Add`, `Gate`, `ScaleRange`) combining two other signals (`ChainedSignal.h:23`).

**Target modes**: 3 scopes (`Clip`, `Layer`, `Global`) via `Route::TargetScope`, pinpointed by `{layerId, clipId, effectIndex, paramIndex}` (`Route.h:19-24`).

**ChainedSignal params** (`ChainedSignal.h:23-42`):
- `carrierSignalId: uint32_t`
- `modulatorSignalId: uint32_t`
- `chainMode: ChainMode` = `Multiply` (default)
- `gain: float` = `1.0f`
- `modulationDepth: float` = `1.0f`
- `gateThreshold: float` = `0.1f`

ChainMode semantics (`ChainedSignal.cpp:12-30`):
- `Multiply`: `carrier * modulator * gain`
- `Add`: `clamp(carrier + modulator*depth, 0, 1) * gain`
- `Gate`: `modulator > gateThreshold ? carrier*gain : 0`
- `ScaleRange`: `(lo + carrier*(hi-lo)) * gain` where `lo = depth*(1-mod)`, `hi = lo+mod`

### 2.9 Routing engine behavior (`src/routing/RoutingEngine.cpp`)

Processing pipeline per route per frame (`RoutingEngine.cpp:77-126`):

1. **Skip if `!route.enabled`** (`:82`)
2. **Fetch source value**: `signals.getCachedValue(route.sourceId)` for `Signal` sources (`:89`). Macros are handled by `MacroBank` before routing (comment at `:91`).
3. **Dial range normalization**: `clamp((raw - dialRangeMin)/(dialRangeMax - dialRangeMin), 0, 1)` (`:94-97`).
4. **Threshold + falloff**: if below threshold, apply EMA-like falloff toward 0 using `lastValues_[i]` (`:100-105`). If above, `(normalized - threshold) * gain` clamped `[0, 1]` (`:108-111`).
5. **Invert**: `1 - x` if `route.inverted` (`:114-115`).
6. **Scale to output range**: `outputMin + x*(outputMax - outputMin)` (`:118`).
7. **Smooth**: per-route `Smoother` initialized at alpha=`0.15f` (`:10`), processes the scaled value (`:121`).
8. **Write**: user-provided `ParamWriter` callback fires with `(route, smoothed_value)` (`:124`).

Per-route persistent state: one `Smoother` and one `lastValues_` float per route, maintained in parallel `vector`s (`RoutingEngine.h:51-54`).

**RoutingEngine public methods**:
- `addRoute(Route) → uint32_t` (`RoutingEngine.h:20`, impl `:5-13`)
- `removeRoute(uint32_t) → bool` (`:23`, impl `:15-28`)
- `getRoute(uint32_t)`, `getRoute(uint32_t) const` (`:26-27`)
- `getNumRoutes() → int` (`:30`)
- `getRouteAt(int) → Route*` (`:31`)
- `getRoutesForTarget(RouteTarget) → vector<Route*>` (`:34`, impl `:51-66`)
- `getRoutesForSource(uint32_t signalId) → vector<Route*>` (`:37`, filters where `sourceType == Signal`, impl `:68-75`)
- `processFrame(SignalRegistry&, ParamWriter&)` (`:45`)
- `clearAll()` (`:48`, impl `:128-133`)

**SignalRegistry public methods** (`src/signal/SignalRegistry.h:17-49`):
- `initDefaults()` — registers 32 default signals
- `addSignal(unique_ptr<Signal>)` — auto-assigns `id = nextId_++`; auto-wires ChainedSignal back to registry for cached lookup (`SignalRegistry.cpp:86-94`)
- `removeSignal(uint32_t id) → bool`
- `getSignal(id)`, `getSignalByName(name)`, `getSignalAt(index)`
- `getNumSignals() → int`
- `getSignalsByCategory(Category) → vector<Signal*>` (`SignalRegistry.cpp:145-152`)
- `evaluateAll(FeatureSnapshot&)` — updates all cached values in order (`:154-160`)
- `getCachedValue(uint32_t signalId) → float` — `O(N)` linear search (`:162-170`)
- `getClipPositionSignal() → ClipPositionSignal*` — dynamic_cast through all signals (`:172-180`)

Note: No serialize/deserialize methods on `SignalRegistry` or `RoutingEngine` in these files.

---

## Section 11 (partial): AI Mapping Suggestions

### Genres supported in `MappingSuggester` (`src/mapping/MappingSuggester.cpp:138-265`)

Genre IDs come from `GenreDetector` (`src/analysis/GenreDetector.h:26-33`). `MappingSuggester` matches on all **8 genres**:

| Genre | ID | `GenreDetector` const | `MappingSuggester` case |
|-------|----|-----------------------|-------------------------|
| House          | 0 | `kHouse` (`GenreDetector.h:26`)       | `MappingSuggester.cpp:143-156` |
| Techno         | 1 | `kTechno` (`GenreDetector.h:27`)      | `:158-171` |
| DnB            | 2 | `kDnB` (`GenreDetector.h:28`)         | `:173-186` |
| HipHop         | 3 | `kHipHop` (`GenreDetector.h:29`)      | `:188-201` |
| Ambient        | 4 | `kAmbient` (`GenreDetector.h:30`)     | `:203-216` |
| Rock           | 5 | `kRock` (`GenreDetector.h:31`)        | `:218-231` |
| Pop/Electronic | 6 | `kPopElectronic` (`GenreDetector.h:32`) | `:233-246` |
| Jazz/Other     | 7 | `kJazzOther` (`GenreDetector.h:33`)   | `:248-261` |

### Suggestion struct (`MappingSuggester.h:18-27`)

Fields:
- `sourceName: string` — e.g. `"Bass"`, `"Beat Phase"`, `"Spectral Flux"`
- `targetCategory: string` — e.g. `"warp"`, `"color"`, `"glitch"`
- `targetEffect: string` — e.g. `"Ripple"`, `"Hue Shift"`
- `targetParam: string` — e.g. `"intensity"`, `"amount"`
- `curveType: string` — e.g. `"Exponential"`, `"Linear"`, `"S-Curve"`
- `reason: string` — human-readable explanation
- `relevance: float` — `[0, 1]` sort key

### API methods
- `suggestMappings(FeatureSnapshot&, maxSuggestions=8)` — universal + genre-specific, sorted by relevance descending, trimmed (`MappingSuggester.h:34-35`, impl `:12-32`).
- `suggestGenreMappings(genre, maxSuggestions=6)` — genre-only (`:39-40`, impl `:34-51`).

### Universal rules (`MappingSuggester.cpp:53-136`) — fire conditionally on feature activity

Activity helper: `featureActivity(value, lowThresh, highThresh)` → `[0, 1]` (`:5-10`).

| Condition | Suggestion(s) | File:line |
|-----------|---------------|-----------|
| `bassActivity > 0.2` | `Bass → Ripple/intensity (Exponential, 0.8×bass)`; `Bass → Bulge/intensity (Exponential, 0.7×bass)` | `:66-74` |
| always | `Beat Phase → Pulse/intensity (Linear, 0.85×beatRelevance)`; `Bar Phase → Hue Shift/amount (Linear, 0.6×beatRelevance)` | `:77-82` |
| `onsetActivity > 0.3` | `Spectral Flux → Block Glitch/intensity (S-Curve, 0.75)`; `Onset Strength → RGB Split/amount (Exponential, 0.7)` | `:86-93` |
| `centroidActivity > 0.1` | `Spectral Centroid → Brightness/amount (Logarithmic, 0.6)` | `:97-102` |
| `rmsActivity > 0.2` | `RMS → Zoom Blur/intensity (Exponential, 0.65)`; `RMS → Glow/intensity (Linear, 0.55)` | `:106-113` |
| `midActivity > 0.3` | `Mid → Wave/intensity (Linear, 0.6)` | `:117-122` |
| `highActivity > 0.3` | `Brilliance → Chromatic Aberration/amount (Logarithmic, 0.55)` | `:125-130` |
| always | `Phrase Phase → Color Shift/amount (Linear, 0.5×beatRelevance)` | `:133-135` |

Key internal thresholds:
- `beatRelevance = 0.9f` if `trackerState == 2` else `0.3f` (`:62`)
- `bassActivity` sums `bandEnergies[0]+[1]`, thresholds `0.1 / 0.6` (`:56-57`)
- `midActivity` sums `bandEnergies[2]+[3]`, `0.1 / 0.6` (`:58-59`)
- `highActivity` sums `bandEnergies[4]+[5]+[6]`, `0.05 / 0.4` (`:60-61`)
- `onsetActivity = featureActivity(transientDensity, 1.0, 8.0)` (`:63`)
- `centroidActivity = featureActivity(spectralCentroid, 1000, 8000)` (`:96`)
- `rmsActivity = featureActivity(rms, 0.05, 0.5)` (`:105`)

### Example genre rules (all rules per genre)

**House (4 rules, `:143-156`)**:
- `Bass → Ripple/intensity (Exponential, 0.9)` — "House: 4-on-floor kick drives rhythmic ripples"
- `Beat Phase → Strobe/intensity (Stepped, 0.75)` — "House: strobe on every kick for club feel"
- `Phrase Phase → Hue Shift/amount (Linear, 0.7)` — "House: color evolves over 8-bar phrases"
- `Mid → Kaleidoscope/rotation (Linear, 0.65)` — "House: synth stabs rotate kaleidoscope pattern"

**Techno (4 rules, `:158-171`)**:
- `Spectral Flux → Block Glitch/intensity (Exponential, 0.9)`
- `Bass → Bulge/intensity (Exponential, 0.85)`
- `Transient Density → Scanlines/intensity (Linear, 0.7)`
- `Onset Strength → Invert/amount (Stepped, 0.65)`

**DnB (4 rules, `:173-186`)**:
- `Bass → Liquid/intensity (Exponential, 0.9)`
- `Onset Strength → RGB Split/amount (Exponential, 0.85)`
- `Beat Phase → Shake/intensity (Exponential, 0.75)`
- `Spectral Centroid → Duotone/mix (Logarithmic, 0.6)`

**HipHop (4 rules, `:188-201`)**:
- `Bass → Bulge/intensity (Exponential, 0.85)`
- `Beat Phase → Pulse/intensity (S-Curve, 0.8)`
- `RMS → Contrast/amount (Linear, 0.7)`
- `Mid → Motion Blur/intensity (Linear, 0.6)`

**Ambient (4 rules, `:203-216`)**:
- `Spectral Centroid → Hue Shift/amount (Logarithmic, 0.85)`
- `RMS → Gaussian Blur/intensity (Linear, 0.8)`
- `Spectral Flatness → Saturation/amount (Linear, 0.7)`
- `Phrase Phase → Ripple/intensity (S-Curve, 0.6)`

**Rock (4 rules, `:218-231`)**:
- `RMS → Shake/intensity (Exponential, 0.85)`
- `Bass → Zoom Blur/intensity (Exponential, 0.8)`
- `Onset Strength → Pixel Scatter/intensity (Exponential, 0.75)`
- `Mid → Contrast/amount (Linear, 0.65)`

**Pop/Electronic (4 rules, `:233-246`)**:
- `Beat Phase → Pulse/intensity (S-Curve, 0.85)`
- `Bass → Ripple/intensity (Exponential, 0.75)`
- `Spectral Centroid → Chromatic Aberration/amount (Logarithmic, 0.7)`
- `Bar Phase → Color Shift/amount (Linear, 0.65)`

**Jazz/Other (4 rules, `:248-261`)**:
- `Harmonic Change → Hue Shift/amount (Linear, 0.85)`
- `Spectral Centroid → Brightness/amount (Logarithmic, 0.75)`
- `RMS → Gaussian Blur/intensity (S-Curve, 0.7)`
- `Dominant Pitch → Wave/frequency (Linear, 0.6)`

**Suggestion output format** (used by caller): each `Suggestion` is a literal-braces-initialized aggregate — sorted desc by `relevance`, truncated to `maxSuggestions`.

---

## Section 20 (partial): Cross-references

| Capability | File:line |
|------------|-----------|
| MappingSource enum | `src/mapping/MappingTypes.h:6-87` |
| MappingCurve enum (23 curves + Count) | `src/mapping/MappingTypes.h:90-120` |
| Mapping struct (10 fields) | `src/mapping/MappingTypes.h:123-140` |
| Curve dispatcher | `src/mapping/CurveTransforms.h:222-252` |
| MappingEngine::extractSource | `src/mapping/MappingEngine.cpp:43-128` |
| MappingEngine::processFrame (pipeline) | `src/mapping/MappingEngine.cpp:135-216` |
| MappingEngine::applyCurve | `src/mapping/MappingEngine.cpp:130-133` |
| Signal base | `src/signal/Signal.h:9-54` |
| Signal::Type (3) | `src/signal/Signal.h:12-17` |
| Signal::Category (8) | `src/signal/Signal.h:19-22` |
| AudioSignal | `src/signal/AudioSignal.h:8-24` |
| OscillatorSignal (5 waves) | `src/signal/OscillatorSignal.h:8-77` |
| EnvelopeSignal (3 curves, control points, one-shot, loop) | `src/signal/EnvelopeSignal.h:10-126` |
| ClipPositionSignal | `src/signal/ClipPositionSignal.h:8-45` |
| ChainedSignal (4 chain modes) | `src/signal/ChainedSignal.h:10-54`, `src/signal/ChainedSignal.cpp:5-31` |
| SignalRegistry::initDefaults (32 signals) | `src/signal/SignalRegistry.cpp:4-84` |
| SignalRegistry::evaluateAll | `src/signal/SignalRegistry.cpp:154-160` |
| SignalRegistry::getCachedValue | `src/signal/SignalRegistry.cpp:162-170` |
| Route struct (17 fields) | `src/routing/Route.h:8-41` |
| RouteTarget struct | `src/routing/Route.h:45-61` |
| Route::SourceType (Signal/Macro) | `src/routing/Route.h:13` |
| Route::TargetScope (Clip/Layer/Global) | `src/routing/Route.h:19` |
| RoutingEngine::processFrame (pipeline) | `src/routing/RoutingEngine.cpp:77-126` |
| RoutingEngine::addRoute / removeRoute / getters | `src/routing/RoutingEngine.cpp:5-75` |
| MacroBank (8 macros × 3 scopes) | `src/routing/MacroBank.h:13-97` |
| MacroBank::Scope enum (3) | `src/routing/MacroBank.h:18` |
| Macro::MacroLink struct | `src/routing/MacroBank.h:20-26` |
| Macro::sourceSignalId (0 = Manual) | `src/routing/MacroBank.h:35, :43` |
| MappingSuggester::Suggestion | `src/mapping/MappingSuggester.h:18-27` |
| MappingSuggester universal rules | `src/mapping/MappingSuggester.cpp:53-136` |
| MappingSuggester genre rules (8 genres × 4 rules) | `src/mapping/MappingSuggester.cpp:138-265` |
| Genre IDs | `src/analysis/GenreDetector.h:26-33` |
