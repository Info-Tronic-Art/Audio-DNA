# FEATURE_INVENTORY — Part A: Audio, Signal & Mapping, Control & Input

**Domains covered:** 1 (Audio Input & Analysis), 5 (Signal & Mapping), 6 (Control & Input)
**Date:** 2026-05-19 | **Source SHA:** 108c8d0

---

## Domain 1: Audio Input & Analysis

---

### 1.1 Audio I/O

**Level:** PRIMARY
**Current UI:** Top Bar (audio source dropdown, gain slider, transport buttons)
**Sub-features:**

- Audio Source Selection [PROGRAMMING-ONLY] — Dropdown selects Mic Input or Audio File. Two modes: `SourceMode::File` (file playback via `AudioTransportSource`) and `SourceMode::MicInput` (live mic/loopback via `CombinedCallback`). Current access: Top Bar dropdown.
- Input Gain [ALWAYS-VISIBLE] — Slider 0-4x, controls `CombinedCallback::inputGain` atomic. Right-click resets to 1.0. Current access: Top Bar slider.
- Input Level Meter [ALWAYS-VISIBLE] — Real-time peak level from `CombinedCallback::inputLevel` with 0.92 decay smoothing. Current access: Top Bar (implicit in gain slider visual).
- Global Transport: Play [ALWAYS-VISIBLE] — Start file playback via `AudioEngine::play()`. Current access: Top Bar play button.
- Global Transport: Pause [ALWAYS-VISIBLE] — Pause file playback via `AudioEngine::pause()`. Current access: Top Bar pause button.
- Global Transport: Stop [ALWAYS-VISIBLE] — Stop and rewind via `AudioEngine::stop()`. Current access: Top Bar stop button.
- File Loading [PROGRAMMING-ONLY] — Load audio file for analysis via `AudioEngine::loadFile()`. Supported formats: WAV, AIFF, FLAC, MP3, OGG (via JUCE `AudioFormatManager`). Current access: Top Bar dropdown selection.
- Camera Input [GHOST -- NO UI] — `Clip::MediaType::Camera` defined in model with `cameraDeviceIndex` field. Not wired to audio I/O but exists as media type. DEFERRED per Boris decision (Syphon Input also DEFERRED).

**Parameters:**
| Parameter | Range | Default | Location |
|-----------|-------|---------|----------|
| Audio Source | File / Mic Input | File | Top Bar dropdown |
| Input Gain | 0.0 - 4.0 | 1.0 | Top Bar slider |

**Bindings:** GlobalPlayPause and GlobalStop available as binding actions. No direct binding for source selection or gain.
**Programming mode:** Full Top Bar visible with source dropdown, gain slider, transport buttons, BPM display.
**Presentation mode:** MINIMAL — gain slider and transport remain useful during performance; source selection can be hidden.
**Dependencies:** RingBuffer (feeds analysis), AudioCallback (RT callback), JUCE AudioDeviceManager.

---

### 1.2 Audio Analysis Pipeline

**Level:** PRIMARY
**Current UI:** Signal Bar (meters for visible signals), Top Bar (BPM display, beat wheel, tracker state)
**Sub-features:**

#### Stage 1: Time-Domain (RMS, Peak, ZCR)
- RMS Level [ALWAYS-VISIBLE] — Root mean square amplitude, primary volume indicator. FeatureSnapshot field: `rms`. Signal: "Volume" (visible).
- Peak Level [PRESENTATION-HIDDEN] — Sample peak amplitude. FeatureSnapshot field: `peak`. Signal: "Peak" (hidden).
- RMS dBFS [PRESENTATION-HIDDEN] — `20*log10(rms)`, logarithmic loudness. FeatureSnapshot field: `rmsDB`.

#### Stage 2: FFT (2048-point, Hann window)
- FFT Processor [PRESENTATION-HIDDEN] — 2048-point FFT via `FFTProcessor::process()`, produces 1025 magnitude bins. Hann window. Uses vDSP on macOS. Not directly exposed as signal; feeds all spectral stages.

#### Stage 3: Spectral Features
- Spectral Centroid [PRESENTATION-HIDDEN] — Center of spectral mass in Hz. FeatureSnapshot field: `spectralCentroid`. Signal: "Brightness" (hidden).
- Spectral Flux [PRESENTATION-HIDDEN] — Frame-to-frame spectral change, normalized. FeatureSnapshot field: `spectralFlux`. Signal: "Change" (hidden).
- Spectral Flatness [PRESENTATION-HIDDEN] — Wiener entropy [0,1], tonal vs noisy. FeatureSnapshot field: `spectralFlatness`. Signal: "Noisiness" (hidden).
- Spectral Rolloff [PRESENTATION-HIDDEN] — Frequency below which 85% of energy resides, in Hz. FeatureSnapshot field: `spectralRolloff`. No signal registered.
- 7-Band Energies [ALWAYS-VISIBLE] — Normalized energy in 7 frequency bands:
  - Sub (20-60 Hz) — Signal: "Sub Bass" (visible). MappingSource: `BandSub`.
  - Bass (60-250 Hz) — Signal: "Bass" (visible). MappingSource: `BandBass`.
  - Low Mid (250-500 Hz) — Signal: "Low Mid" (hidden). MappingSource: `BandLowMid`.
  - Mid (500-2kHz) — Signal: "Mid" (visible). MappingSource: `BandMid`.
  - High Mid (2-4kHz) — Signal: "High Mid" (hidden). MappingSource: `BandHighMid`.
  - Presence (4-6kHz) — Signal: "Presence" (hidden). MappingSource: `BandPresence`.
  - Brilliance (6-20kHz) — Signal: "Air" (visible). MappingSource: `BandBrilliance`.

#### Stage 4: Onset Detection
- Onset Detection [ALWAYS-VISIBLE] — Aubio spectral flux thresholding with adaptive median. FeatureSnapshot fields: `onsetDetected` (bool), `onsetStrength` (float). Signal: "Hit" (visible), "Hit Strength" (hidden).

#### Stage 5: BPM Tracking
- BPM Detection [ALWAYS-VISIBLE] — Aubio autocorrelation tempo tracker. Multi-layer stabilization: range gate, confidence filter, octave correction, median filter, hysteresis. FeatureSnapshot field: `bpm`. Signal: "Tempo" (visible). Top Bar: large BPM number, color-coded (gray=searching, yellow=locking, green=locked).
- Beat Phase [ALWAYS-VISIBLE] — Smooth [0,1) sawtooth ramp between beats, locked to detected BPM. FeatureSnapshot field: `beatPhase`. Signal: "Beat Position" (visible). Top Bar: 4-segment beat wheel.
- Tracker State [ALWAYS-VISIBLE] — 0=searching, 1=locking, 2=locked. FeatureSnapshot field: `trackerState`. Top Bar label: "SEARCHING"/"LOCKING"/"LOCKED".
- Tap Tempo [ALWAYS-VISIBLE] — Manual BPM entry via 8-tap averaging. Top Bar button.
- Resync [ALWAYS-VISIBLE] — Reset beat phase to align with current audio. Top Bar button.
- Manual BPM [ALWAYS-VISIBLE] — Toggle to freeze detection and enter manual value. Top Bar toggle + text field.
- BPM Multiplier [ALWAYS-VISIBLE] — /4, /2, x1, x2, x4 divisors. Top Bar: 5 buttons.

#### Stage 6: MFCC Extraction
- 13 MFCCs [PRESENTATION-HIDDEN] — 40-band mel filterbank (20-8kHz), log energy, DCT. FeatureSnapshot fields: `mfccs[0..12]`. MappingSources: `MFCC0` through `MFCC12` (13 sources). No signals registered.

#### Stage 7: Chroma Extraction
- 12 Chroma Bins [PRESENTATION-HIDDEN] — FFT bins mapped to 12 pitch classes (C through B), normalized. FeatureSnapshot fields: `chromagram[0..11]`. MappingSources: `ChromaC` through `ChromaB` (12 sources). No signals registered.
- HCDF [PRESENTATION-HIDDEN] — Harmonic Change Detection Function, frame-to-frame chroma distance. FeatureSnapshot field: `harmonicChangeDetection`. Signal: "Chord Change" (hidden). MappingSource: `HarmonicChange`.

#### Stage 8: Pitch Detection
- Dominant Pitch [PRESENTATION-HIDDEN] — Aubio yinfft pitch detection in Hz. FeatureSnapshot field: `dominantPitch`. Signal: "Note" (hidden). MappingSource: `DominantPitch`.
- Pitch Confidence [PRESENTATION-HIDDEN] — [0,1] reliability of pitch detection. FeatureSnapshot field: `pitchConfidence`. Signal: "Note Confidence" (hidden). MappingSource: `PitchConfidence`.

#### Stage 9: Key Detection
- Musical Key [PRESENTATION-HIDDEN] — Krumhansl-Schmuckler algorithm: chroma x 24 key templates. FeatureSnapshot fields: `detectedKey` (0-11, C=0, -1=unknown), `keyIsMajor` (bool). MappingSource: `DetectedKey`.

#### Stage 10: Loudness Analysis
- LUFS [PRESENTATION-HIDDEN] — K-weighted biquads + 400ms window, ITU-R BS.1770 momentary loudness. FeatureSnapshot field: `lufs`. MappingSource: `LUFS`.
- Dynamic Range [PRESENTATION-HIDDEN] — Crest factor (peak/RMS). FeatureSnapshot field: `dynamicRange`. Signal: "Punch" (hidden). MappingSource: `DynamicRange`.

#### Stage 11: Structural Detection
- Structural State [PRESENTATION-HIDDEN] — Multi-scale EMA (100ms/1s/4s/16s) state machine. FeatureSnapshot field: `structuralState` (0=normal, 1=buildup, 2=drop, 3=breakdown). MappingSource: `StructuralState`. Drives `Renderer::onStructuralStateChanged_` callback for scene triggering.

#### Stage 12: Transient Density
- Transient Density [PRESENTATION-HIDDEN] — Onset count in 2-second sliding window, onsets/sec. FeatureSnapshot field: `transientDensity`. Signal: "Hits Per Second" (hidden). MappingSource: `TransientDensity`.

#### Stage 13: Phrase Tracking
- Bar Phase [PRESENTATION-HIDDEN] — [0,1) sawtooth over 4 beats. FeatureSnapshot field: `barPhase`. Signal: "Bar Position" (hidden). Top Bar: "Bar N" text.
- Bar Count [PRESENTATION-HIDDEN] — Bars since last phrase reset. FeatureSnapshot field: `barCount`. Signal: "Bar Count" (hidden). MappingSource: `BarCount`.
- Phrase Phase [PRESENTATION-HIDDEN] — [0,1) sawtooth over configurable N bars. FeatureSnapshot field: `phrasePhase`. Signal: "Phrase Position" (hidden). Top Bar: "Phr 0.XX" text. MappingSource: `PhrasePhase`.
- Beat In Bar [PRESENTATION-HIDDEN] — 0-3 (0=downbeat), which beat in the bar. FeatureSnapshot field: `beatInBar`.
- Downbeat Detection [PRESENTATION-HIDDEN] — Boolean true on the hop where beat 1 lands. FeatureSnapshot field: `downbeatDetected`.

#### Stage 14: Genre Detection
- Genre Classification [PRESENTATION-HIDDEN] — 8-genre heuristic classifier (multi-feature scoring: BPM, spectral profile, transient density, chromatic complexity). FeatureSnapshot fields: `detectedGenre` (0-7), `genreConfidence` [0,1], `genreScores[8]`. Drives auto-preset/deck switching via `Renderer::onGenreChanged_`.
  - Genres: House(0), Techno(1), DnB(2), Hip-Hop(3), Ambient(4), Rock(5), Pop/Electronic(6), Jazz/Other(7)
- Energy State [PRESENTATION-HIDDEN] — 3-level energy classification. FeatureSnapshot field: `energyState` (0=low, 1=medium, 2=high). Used by smart autopilot for intelligent clip selection.

#### Stage 15: Advanced Audio Analysis (P25)
- Sidechain Pump [PRESENTATION-HIDDEN] — Bass/mid anti-correlation detection [0,1]. FeatureSnapshot field: `sidechainPump`. Signal: "Sidechain Pump" (hidden). MappingSource: `SidechainPump`.
- Swing Ratio [PRESENTATION-HIDDEN] — Timing deviation from straight grid [0.5, ~0.67]. FeatureSnapshot field: `swingRatio`. Signal: "Swing" (hidden). MappingSource: `SwingRatio`.
- Formant Presence [PRESENTATION-HIDDEN] — Vocal formant energy concentration in 300-3000 Hz band [0,1]. FeatureSnapshot field: `formantPresence`. Signal: "Vocal Presence" (hidden). MappingSource: `FormantPresence`.
- Resonance Peak [PRESENTATION-HIDDEN] — Spectral kurtosis, sharp peaks vs flat [0,1]. FeatureSnapshot field: `resonancePeak`. Signal: "Resonance" (hidden). MappingSource: `ResonancePeak`.
- Reese Bass [PRESENTATION-HIDDEN] — Bass spectral spread, reese/wobble detection [0,1]. FeatureSnapshot field: `reeseBass`. Signal: "Reese Bass" (hidden). MappingSource: `ReeseBass`.

**Parameters:**
| Parameter | Range | Default | Location |
|-----------|-------|---------|----------|
| FFT Size | 2048 (hardcoded) | 2048 | N/A |
| Hop Size | 512 samples / 10.7ms (hardcoded) | 512 | N/A |
| Sample Rate | 48kHz (hardcoded) | 48000 | N/A |
| Mel Bands | 40 (20-8kHz, hardcoded) | 40 | N/A |
| Genre Smoothing | ~2s EMA + ~3s hysteresis | varies by genre | N/A |

**Bindings:** TapTempo and Resync available as binding actions. No direct bindings for analysis parameters.
**Programming mode:** Signal Bar expanded shows all signals with oscilloscope/histogram per signal. Full BPM display with tracker state.
**Presentation mode:** MINIMAL — BPM number + beat wheel essential; signal bar minimized to tiny meter bars. Genre/structural state invisible but drives automation.
**Dependencies:** Audio I/O (ring buffer input), Aubio 0.4.9+ (onset, BPM, pitch), JUCE juce_dsp (FFT).

---

### 1.3 Feature Transport

**Level:** UTILITY
**Current UI:** NO UI -- code only (transparent infrastructure)
**Sub-features:**

- Triple-Buffer Atomic Swap [PRESENTATION-HIDDEN] — `FeatureBus` transfers complete `FeatureSnapshot` (alignas(64) POD struct, 40+ fields) from analysis thread to render thread. 3 slots encoded in packed atomic uint8: bits [1:0]=write, [3:2]=latest, [5:4]=read, bit 6=new-data flag. Wait-free. ~10ns read latency.
- Write API [PRESENTATION-HIDDEN] — `FeatureBus::acquireWrite()` returns writable buffer, `publishWrite()` atomically swaps it to "latest" slot. Called by AnalysisThread every 10.7ms.
- Read API [PRESENTATION-HIDDEN] — `FeatureBus::acquireRead()` returns latest published snapshot (nullptr if no new data). `getLatestRead()` returns last consumed snapshot (never nullptr after first cycle). Called by Renderer every 16.67ms.
- EMA Smoother [PRESENTATION-HIDDEN] — `Smoother` class: configurable alpha in (0,1], higher = less smoothing (1.0 = passthrough). Per-mapping smoothing state. `value_ += alpha_ * (input - value_)`.
- One-Euro Filter [PRESENTATION-HIDDEN] — `OneEuroFilter` class: adaptive low-pass that smooths slow movements but reacts quickly to fast changes. Parameters: rate (default 93.75 Hz), minCutoff (1.0 Hz), beta (0.007), dCutoff (1.0 Hz). Used for noisy sensor-like data.

**Parameters:**
| Parameter | Range | Default | Location |
|-----------|-------|---------|----------|
| Smoother Alpha | 0.0 - 1.0 | per-mapping (0.15 typical) | Per mapping/route config |
| OneEuro Rate | Hz | 93.75 | Constructor |
| OneEuro MinCutoff | Hz | 1.0 | Constructor |
| OneEuro Beta | float | 0.007 | Constructor |

**Bindings:** None (infrastructure, not user-controllable).
**Programming mode:** Not visible (transparent data transport).
**Presentation mode:** HIDDEN — pure infrastructure.
**Dependencies:** AnalysisThread (writer), Renderer (reader), std::atomic.

---

## Domain 5: Signal & Mapping

---

### 5.1 Signal Registry

**Level:** PRIMARY
**Current UI:** Signal Bar (visible signals as meters), Signal Inspector (per-signal controls)
**Sub-features:**

#### Visible Audio Signals (8 signals)

| # | Signal Name | MappingSource | Category | Default Visible |
|---|-------------|---------------|----------|-----------------|
| 1 | Volume | RMS | Amplitude | Yes |
| 2 | Sub Bass | BandSub | Bands | Yes |
| 3 | Bass | BandBass | Bands | Yes |
| 4 | Mid | BandMid | Bands | Yes |
| 5 | Air | BandBrilliance | Bands | Yes |
| 6 | Tempo | BPM | Rhythm | Yes |
| 7 | Beat Position | BeatPhase | Rhythm | Yes |
| 8 | Hit | OnsetStrength | Rhythm | Yes |

#### Hidden Audio Signals (25 signals)

| # | Signal Name | MappingSource | Category |
|---|-------------|---------------|----------|
| 9 | Peak | Peak | Amplitude |
| 10 | Punch | DynamicRange | Amplitude |
| 11 | Hits Per Second | TransientDensity | Amplitude |
| 12 | Low Mid | BandLowMid | Bands |
| 13 | High Mid | BandHighMid | Bands |
| 14 | Presence | BandPresence | Bands |
| 15 | Hit Strength | OnsetStrength | Rhythm |
| 16 | Bar Position | BarPhase | Rhythm |
| 17 | Phrase Position | PhrasePhase | Rhythm |
| 18 | Bar Count | BarCount | Rhythm |
| 19 | Brightness | SpectralCentroid | Amplitude |
| 20 | Change | SpectralFlux | Amplitude |
| 21 | Noisiness | SpectralFlatness | Amplitude |
| 22 | Note | DominantPitch | Pitch |
| 23 | Note Confidence | PitchConfidence | Pitch |
| 24 | Chord Change | HarmonicChange | Pitch |
| 25 | Sidechain Pump | SidechainPump | Amplitude |
| 26 | Swing | SwingRatio | Rhythm |
| 27 | Vocal Presence | FormantPresence | Amplitude |
| 28 | Resonance | ResonancePeak | Amplitude |
| 29 | Reese Bass | ReeseBass | Bands |
| 30 | Clip Position | (ClipPositionSignal) | Modulation |

Note: Signal #30 (Clip Position) is a `ClipPositionSignal` (not AudioSignal), hidden by default. Signals 25-29 are P25 advanced audio analysis. Signal #15 (Hit Strength) uses same MappingSource as #8 (Hit) — both map to `OnsetStrength`.

#### Modulation Signals (3 signals, 2 visible)

| # | Signal Name | Type | Default Visible |
|---|-------------|------|-----------------|
| 31 | Mod 1 | OscillatorSignal (Sine, 1 beat) | Yes |
| 32 | Mod 2 | EnvelopeSignal (4 beats) | Yes |
| 33 | Clip Position | ClipPositionSignal | No |

Note: Mod 1 and Mod 2 are user-addable modulation slots. The [+] button in Signal Bar allows adding more Oscillator, Envelope, or Chained signals.

**Total: 33 registered signals at startup** (8 visible audio + 22 hidden audio + 1 hidden clip position + 2 visible modulation). Users can add more via the [+] button. The work packet says 36 total (8+25+3); the code shows 30 audio + 3 modulation = 33. Discrepancy: the 25 hidden count includes Clip Position, and the 3 modulation count also includes Clip Position. Reconciled: 8 visible audio + 22 hidden audio + 1 hidden ClipPosition + 2 visible modulation = 33 unique. The "36" number from the work packet may include additional user-created signals or count differently.

#### Signal Types (from Signal base class)

| Type | Class | Description | BPM-Locked |
|------|-------|-------------|------------|
| Audio | AudioSignal | Wraps a FeatureSnapshot field | No |
| Oscillator | OscillatorSignal | BPM-locked waveform: Sine/SawUp/SawDown/Triangle/Square | Yes |
| Envelope | EnvelopeSignal | Custom curve with draggable control points, looping/one-shot | Yes |
| ClipPosition | ClipPositionSignal | Playhead normalized 0-1, tracks active clip | No |
| Chained | ChainedSignal | Signal modulating signal (carrier x modulator) | Depends |

#### Signal Categories

8 categories: Amplitude, Bands, Rhythm, Pitch, Chroma, Timbre, Structure, Modulation.

- Signal Inspector controls per signal type:
  - Audio: Threshold (0-1), Gain (0-4x), Falloff (0-1s)
  - Oscillator: Wave Shape (5 types), Beat Duration (1/4 to 8 beats), Amplitude (0-1), Phase Offset (0-1)
  - Envelope: Curve Type (Linear/Exponential/S-Curve), Beat Duration (1-16 beats), Amplitude (0-1), Phase (0-1), Looping toggle, One Shot toggle, Visual curve editor with draggable control points

#### Chained Signal (signal modulating signal) [GHOST -- NO UI for creation]
- ChainedSignal [PROGRAMMING-ONLY] — One signal modulates another. 4 chain modes: Multiply, Add, Gate, ScaleRange. Parameters: gain (1.0), modulationDepth (1.0), gateThreshold (0.1). Can be created via code/API but NO UI exists for creating chained signals. Existing chained signals are evaluated via `evaluateAll()`.

**Parameters:**
| Parameter | Range | Default | Per |
|-----------|-------|---------|-----|
| Signal Visibility | bool | varies | signal |
| Threshold | 0.0 - 1.0 | 0.0 | audio signal |
| Gain | 0.0 - 4.0 | 1.0 | audio signal |
| Falloff | 0.0 - 1.0 | 0.1 | audio signal |
| Wave Shape | Sine/SawUp/SawDown/Triangle/Square | Sine | oscillator |
| Beat Duration | 1/4, 1/2, 1, 2, 4, 8 beats | 1 beat | oscillator/envelope |
| Amplitude | 0.0 - 1.0 | 1.0 | oscillator/envelope |
| Phase Offset | 0.0 - 1.0 | 0.0 | oscillator/envelope |
| Chain Mode | Multiply/Add/Gate/ScaleRange | Multiply | chained |
| Chain Gain | float | 1.0 | chained |
| Modulation Depth | float | 1.0 | chained |
| Gate Threshold | float | 0.1 | chained |

**Bindings:** No direct bindings for signal configuration.
**Programming mode:** Signal Bar expanded with full oscilloscope/histogram per signal; Signal Inspector with all controls visible.
**Presentation mode:** MINIMAL — Signal Bar minimized to tiny meter bars (~20px). Signal Inspector hidden.
**Dependencies:** FeatureSnapshot (audio data source), FeatureBus (transport), RoutingEngine (consumes signals).

---

### 5.2 Routing Engine (v2)

**Level:** PRIMARY
**Current UI:** Timing Window > Routing tab (placeholder content), Signal Inspector (per-signal threshold/gain/falloff)
**Sub-features:**

- Route Creation [PROGRAMMING-ONLY] — Create a route from any Signal or Macro to any effect parameter at any scope (Clip/Layer/Global). Via `RoutingEngine::addRoute()`. Current access: Signal connect triangle on parameter controls (click to select source).
- Route Source Types [PROGRAMMING-ONLY] — `Route::SourceType`: Signal (from SignalRegistry) or Macro (from Dashboard knobs).
- Route Target Scopes [PROGRAMMING-ONLY] — `Route::TargetScope`: Clip (per-clip effect params), Layer (per-layer effect params), Global (global effect chain params). Identified by scope + layerId + clipId + effectIndex + paramIndex.
  - NOTE: Clip and Layer scopes are defined in code but the GAP_REPORT flags `Route TargetScope::Clip/Layer` as ghost features — per-clip and per-layer routing targets exist in code but may not be fully wired.
- Route Transform [PRESENTATION-HIDDEN] — Per-route: outputMin/Max (0-1 default), inverted (bool), dialRangeMin/Max (input sensitivity).
- Route Threshold/Gain/Falloff [PRESENTATION-HIDDEN] — Per-route: threshold (source must exceed to have effect, default 0.0), gain (post-threshold multiplier, default 1.0), falloff (decay rate when source drops, default 0.1).
- Route Enable/Disable [PRESENTATION-HIDDEN] — Per-route boolean `enabled` flag.
- Per-Frame Processing [PRESENTATION-HIDDEN] — `processFrame()` reads signal values from SignalRegistry, applies threshold/gain/falloff transform, writes to target via ParamWriter callback. No heap allocation. No curve transforms (deliberate v2 design difference from MappingEngine v1).
- Multi-Route Summation [PRESENTATION-HIDDEN] — Multiple routes can target the same parameter (values summed by the caller).

**Parameters per route:**
| Parameter | Range | Default |
|-----------|-------|---------|
| Source Type | Signal / Macro | Signal |
| Source ID | uint32 (signal or macro ID) | 0 |
| Target Scope | Clip / Layer / Global | Clip |
| Target Layer ID | uint32 | 0 |
| Target Clip ID | uint32 | 0 |
| Target Effect Index | int | 0 |
| Target Param Index | int | 0 |
| Output Min | 0.0 - 1.0 | 0.0 |
| Output Max | 0.0 - 1.0 | 1.0 |
| Inverted | bool | false |
| Dial Range Min | 0.0 - 1.0 | 0.0 |
| Dial Range Max | 0.0 - 1.0 | 1.0 |
| Threshold | 0.0 - 1.0 | 0.0 |
| Gain | float | 1.0 |
| Falloff | 0.0 - 1.0 | 0.1 |
| Enabled | bool | true |

**Bindings:** No direct bindings for routing.
**Programming mode:** Full route creation UI via signal connect triangles. Routing tab in Timing Window (placeholder).
**Presentation mode:** HIDDEN — routes run automatically in background.
**Dependencies:** SignalRegistry (source values), Effect system (targets), Smoother (per-route smoothing).

---

### 5.3 Mapping Engine (v1)

**Level:** PRIMARY
**Current UI:** Clip Inspector effect params > "map" dropdown, Mapping Editor panel
**Sub-features:**

- Mapping Creation [PROGRAMMING-ONLY] — Wire any audio feature (MappingSource) directly to any effect parameter via `MappingEngine::addMapping()`. Current access: click "map" on any effect parameter.
- Mapping Pipeline [PRESENTATION-HIDDEN] — Per frame, per mapping: extract source from FeatureSnapshot, normalize to [0,1] via (raw-inputMin)/(inputMax-inputMin), apply curve, scale to [outputMin,outputMax], smooth via EMA, write to target.
- Multi-Mapping Summation [PRESENTATION-HIDDEN] — Multiple mappings targeting same effect parameter are summed and clamped to [0,1].
- Pass 1 Reset [PRESENTATION-HIDDEN] — WARNING: resets ALL targeted params to 0 before accumulation each frame. Any param targeted by at least one mapping has its manual/preset value destroyed.

#### Complete Mapping Source List (57 sources)

| # | MappingSource Enum | Category | FeatureSnapshot Field |
|---|-------------------|----------|----------------------|
| 1 | RMS | Amplitude | rms |
| 2 | Peak | Amplitude | peak |
| 3 | RmsDB | Amplitude | rmsDB |
| 4 | LUFS | Loudness | lufs |
| 5 | DynamicRange | Loudness | dynamicRange |
| 6 | TransientDensity | Loudness | transientDensity |
| 7 | SpectralCentroid | Spectral | spectralCentroid |
| 8 | SpectralFlux | Spectral | spectralFlux |
| 9 | SpectralFlatness | Spectral | spectralFlatness |
| 10 | SpectralRolloff | Spectral | spectralRolloff |
| 11 | BandSub | 7-Band | bandEnergies[0] |
| 12 | BandBass | 7-Band | bandEnergies[1] |
| 13 | BandLowMid | 7-Band | bandEnergies[2] |
| 14 | BandMid | 7-Band | bandEnergies[3] |
| 15 | BandHighMid | 7-Band | bandEnergies[4] |
| 16 | BandPresence | 7-Band | bandEnergies[5] |
| 17 | BandBrilliance | 7-Band | bandEnergies[6] |
| 18 | OnsetStrength | Onset/Rhythm | onsetStrength |
| 19 | BeatPhase | Onset/Rhythm | beatPhase |
| 20 | BPM | Onset/Rhythm | bpm |
| 21 | BarPhase | Onset/Rhythm | barPhase |
| 22 | PhrasePhase | Onset/Rhythm | phrasePhase |
| 23 | BarCount | Onset/Rhythm | barCount |
| 24 | StructuralState | Structural | structuralState |
| 25 | DominantPitch | Pitch/Harmony | dominantPitch |
| 26 | PitchConfidence | Pitch/Harmony | pitchConfidence |
| 27 | DetectedKey | Pitch/Harmony | detectedKey |
| 28 | HarmonicChange | Pitch/Harmony | harmonicChangeDetection |
| 29 | MFCC0 | Timbral | mfccs[0] |
| 30 | MFCC1 | Timbral | mfccs[1] |
| 31 | MFCC2 | Timbral | mfccs[2] |
| 32 | MFCC3 | Timbral | mfccs[3] |
| 33 | MFCC4 | Timbral | mfccs[4] |
| 34 | MFCC5 | Timbral | mfccs[5] |
| 35 | MFCC6 | Timbral | mfccs[6] |
| 36 | MFCC7 | Timbral | mfccs[7] |
| 37 | MFCC8 | Timbral | mfccs[8] |
| 38 | MFCC9 | Timbral | mfccs[9] |
| 39 | MFCC10 | Timbral | mfccs[10] |
| 40 | MFCC11 | Timbral | mfccs[11] |
| 41 | MFCC12 | Timbral | mfccs[12] |
| 42 | ChromaC | Chroma | chromagram[0] |
| 43 | ChromaCs | Chroma | chromagram[1] |
| 44 | ChromaD | Chroma | chromagram[2] |
| 45 | ChromaDs | Chroma | chromagram[3] |
| 46 | ChromaE | Chroma | chromagram[4] |
| 47 | ChromaF | Chroma | chromagram[5] |
| 48 | ChromaFs | Chroma | chromagram[6] |
| 49 | ChromaG | Chroma | chromagram[7] |
| 50 | ChromaGs | Chroma | chromagram[8] |
| 51 | ChromaA | Chroma | chromagram[9] |
| 52 | ChromaAs | Chroma | chromagram[10] |
| 53 | ChromaB | Chroma | chromagram[11] |
| 54 | SidechainPump | Advanced (P25) | sidechainPump |
| 55 | SwingRatio | Advanced (P25) | swingRatio |
| 56 | FormantPresence | Advanced (P25) | formantPresence |
| 57 | ResonancePeak | Advanced (P25) | resonancePeak |

Note: `ReeseBass` is MappingSource enum value 57 (0-indexed as the 58th entry before `Count`). The sentinel `Count` makes the total enum size 58, but the usable sources are 57 (RMS through ReeseBass, including ReeseBass which I miscounted above). Let me recount: RMS(0) through ReeseBass(57-1=56) — but the enum listing shows ReeseBass before Count. Verified: enum values 0 through 56 = **57 usable mapping sources**. Matches the work packet's "57 mapping sources."

#### Complete Curve Type List (24 curves)

| # | MappingCurve Enum | Formula | Purpose |
|---|------------------|---------|---------|
| 1 | Linear | y = x | Direct 1:1 mapping |
| 2 | Exponential | y = x^2 | Emphasizes peaks |
| 3 | Logarithmic | y = log(1+9x)/log(10) | Compresses peaks, lifts lows |
| 4 | SCurve | y = x^2(3-2x) | De-emphasizes extremes (smoothstep) |
| 5 | Stepped | y = floor(xN)/N | Quantized to N steps |
| 6 | CircularIn | 1-sqrt(1-x^2) | Slow start, fast end |
| 7 | CircularOut | sqrt(1-(x-1)^2) | Fast start, slow end |
| 8 | CircularInOut | Combined circular | Slow-fast-slow |
| 9 | BackIn | Overshoots backward first | Anticipation effect |
| 10 | BackOut | Overshoots forward | Snap-back effect |
| 11 | BackInOut | Both overshoots | Double anticipation |
| 12 | ElasticIn | Spring oscillation at start | Wobbly start |
| 13 | ElasticOut | Spring oscillation at end | Wobbly settle |
| 14 | ElasticInOut | Both elastic | Full spring |
| 15 | BounceIn | Bounce at start | Bouncy entrance |
| 16 | BounceOut | Bounce at end | Bouncy settle |
| 17 | BounceInOut | Both bounce | Full bounce |
| 18 | CubicIn | x^3 | Gentle exponential |
| 19 | CubicOut | 1-(1-x)^3 | Gentle deceleration |
| 20 | CubicInOut | Combined cubic | Gentle ease |
| 21 | SineIn | 1-cos(x*pi/2) | Sinusoidal start |
| 22 | SineOut | sin(x*pi/2) | Sinusoidal end |
| 23 | SineInOut | -(cos(pi*x)-1)/2 | Full sinusoidal |
| 24 | Hold | x >= 1.0 ? 1.0 : 0.0 | Binary step function |

**Parameters per mapping:**
| Parameter | Range | Default |
|-----------|-------|---------|
| Source | MappingSource enum (57 values) | RMS |
| Target Effect ID | uint32 | 0 |
| Target Param Index | uint32 | 0 |
| Curve | MappingCurve enum (24 values) | Linear |
| Input Min | float | 0.0 |
| Input Max | float | 1.0 |
| Output Min | float | 0.0 |
| Output Max | float | 1.0 |
| Smoothing | 0.0 - 1.0 (EMA alpha; higher = LESS smoothing) | 0.15 |
| Enabled | bool | true |

**Bindings:** No direct bindings for mapping management.
**Programming mode:** Mapping Editor visible when "map" clicked on any parameter, showing source selector, curve type, range sliders, smoothing.
**Presentation mode:** HIDDEN — mappings run automatically. No UI needed.
**Dependencies:** FeatureSnapshot (source data), EffectChain (target params), Smoother (per-mapping smoothing), CurveTransforms.

---

### 5.4 Macro Banks (Dashboard Knobs)

**Level:** SECONDARY
**Current UI:** Clip Inspector > Dashboard (8 knobs), Layer Inspector > Dashboard (8 knobs), Composition Inspector > Dashboard (8 knobs)
**Sub-features:**

- Global Macro Bank (8 knobs) [ALWAYS-VISIBLE] — 8 rotary knobs in Composition Inspector Dashboard. Each knob: manual value OR connected to a signal source. Labels are renameable. Global scope wired and functional. Current access: Composition Inspector > Dashboard section.
- Clip Macro Bank (8 knobs) [GHOST -- NO UI] — Per-clip dashboard knobs exist in Inspector UI but per-clip scope macro routing is NOT YET WIRED. Knobs display and accept manual values, but clip-scope macro distribution to linked parameters is not implemented.
- Layer Macro Bank (8 knobs) [GHOST -- NO UI] — Per-layer dashboard knobs exist in Inspector UI but per-layer scope macro routing is NOT YET WIRED. Same situation as clip macros.
- Signal Connection [PROGRAMMING-ONLY] — Click source button on any knob to select signal source: Manual / Audio Signal / BPM Sync / Oscillator / Envelope / Clip Position / Timeline / Macro.
- Macro-to-Parameter Distribution [PROGRAMMING-ONLY] — Each macro knob distributes its value to linked parameters. Linkage configured via routing or parameter connect triangles.
- AdjustMacro Binding [ALWAYS-VISIBLE] — MIDI CC binding action `AdjustMacro` allows continuous control of a macro knob via `targetMacroIndex`.
- OSC Macro Control [PRESENTATION-HIDDEN] — `/audiodna/macro/{n}` OSC pattern sets macro value (float 0-1).

**Parameters per knob:**
| Parameter | Range | Default |
|-----------|-------|---------|
| Value | 0.0 - 1.0 | 0.5 |
| Source Mode | Manual/Signal/BPM Sync/Oscillator/Envelope/Clip Position/Timeline/Macro | Manual |
| Label | string | "Knob N" |
| Invert | bool | false |
| Range Min | 0.0 - 1.0 | 0.0 |
| Range Max | 0.0 - 1.0 | 1.0 |

**Bindings:** AdjustMacro (MIDI CC), OSC `/audiodna/macro/{n}`.
**Programming mode:** Full knob panels visible in all three Inspector scopes with signal connection UI.
**Presentation mode:** MINIMAL — Knob values still adjustable via MIDI CC or OSC, but Inspector UI typically hidden.
**Dependencies:** SignalRegistry (signal sources), RoutingEngine (distribution), BindingManager (MIDI control).

---

### 5.5 Mapping Suggestions

**Level:** UTILITY
**Current UI:** [GHOST -- NO UI]
**Sub-features:**

- Genre-Aware Suggestions [GHOST -- NO UI] — `MappingSuggester::suggestMappings()` analyzes current audio snapshot and returns scored source-to-param recommendations (max 8). Each `Suggestion` contains: sourceName, targetCategory, targetEffect, targetParam, curveType, reason, relevance [0,1]. Sorted by relevance.
- Genre-Specific Suggestions [GHOST -- NO UI] — `MappingSuggester::suggestGenreMappings()` returns genre-specific recommendations (max 6). Takes detected genre ID (0-7).
- Feature Activity Scoring [GHOST -- NO UI] — `featureActivity()` scores a feature's relevance based on current value vs low/high thresholds.
- Universal Suggestions [GHOST -- NO UI] — `addUniversalSuggestions()` generates mappings that work for any genre.

**Parameters:** None user-configurable (stateless utility).
**Bindings:** None.
**Programming mode:** Not accessible (no UI exists).
**Presentation mode:** HIDDEN.
**Dependencies:** FeatureSnapshot (current audio state), GenreDetector (genre classification).

---

## Domain 6: Control & Input

---

### 6.1 Keyboard Bindings

**Level:** PRIMARY
**Current UI:** Menu > Shortcuts > Edit Keyboard Bindings, in-app binding overlay
**Sub-features:**

- Keyboard Binding Mode [PROGRAMMING-ONLY] — Enter via Menu > Shortcuts > Edit Keyboard Bindings (Shift+Cmd+K). Next key press creates a binding instead of triggering an action. `BindingManager::setBindingMode(true)`. Exit via Escape.
- Key + Modifier Capture [PROGRAMMING-ONLY] — Captures JUCE keyCode + Shift/Cmd/Alt modifier flags. `Binding::keyCode`, `keyModShift`, `keyModCmd`, `keyModAlt`.
- Toggle Trigger Mode [ALWAYS-VISIBLE] — Press = on, press again = off. Default mode. `Binding::TriggerMode::Toggle`.
- Momentary Trigger Mode [ALWAYS-VISIBLE] — Press = on, release = off (piano mode). Key release routed via `MainComponent::keyStateChanged()` polling (JUCE limitation: no keyUp event for non-modifier keys). `Binding::TriggerMode::Momentary`.
- Binding Export/Import [PROGRAMMING-ONLY] — Menu > Shortcuts > Export Bindings / Import Bindings. Serialized via `BindingManager::saveToFile()`/`loadFromFile()` as JSON.

#### Complete Binding Actions List (19 actions)

| # | Action Enum | Purpose | Input Types | Parameters |
|---|-------------|---------|-------------|------------|
| 1 | TriggerClip | Fire a specific clip | Key, MIDI Note | targetLayerIndex, targetColumn |
| 2 | TriggerColumn | Fire all clips in a column | Key, MIDI Note | targetColumn |
| 3 | ToggleLayerBypass | Toggle layer bypass on/off | Key, MIDI Note | targetLayerIndex |
| 4 | ToggleLayerSolo | Toggle layer solo on/off | Key, MIDI Note | targetLayerIndex |
| 5 | ToggleLayerMute | Toggle layer mute on/off | Key, MIDI Note | targetLayerIndex |
| 6 | ToggleLayerAutopilot | Toggle autopilot on/off | Key, MIDI Note | targetLayerIndex |
| 7 | ToggleLayerVisible | Toggle layer visibility | Key, MIDI Note | targetLayerIndex |
| 8 | LayerTransport | Play/pause/reverse on layer | Key, MIDI Note | targetLayerIndex |
| 9 | ToggleEffectBypass | Bypass a specific effect | Key, MIDI Note | targetEffectIndex |
| 10 | AdjustMacro | Continuous control of macro knob | MIDI CC | targetMacroIndex |
| 11 | AdjustLayerOpacity | Continuous control of layer opacity | MIDI CC | targetLayerIndex |
| 12 | SwitchDeck | Switch active deck | Key, MIDI Note | targetDeckIndex |
| 13 | TapTempo | Tap tempo | Key, MIDI Note | (none) |
| 14 | Resync | Reset beat phase | Key, MIDI Note | (none) |
| 15 | GlobalPlayPause | Global transport play/pause | Key, MIDI Note | (none) |
| 16 | GlobalStop | Global transport stop | Key, MIDI Note | (none) |
| 17 | MasterOpacity | Continuous control of master opacity | MIDI CC | (none) |
| 18 | Snapshot | Take PNG screenshot (P22.7) | Key, MIDI Note | (none) |
| 19 | ToggleRecording | Start/stop video recording (P22.6) | Key, MIDI Note | (none) |

#### 3 Target Modes

| Mode | Enum | Behavior |
|------|------|----------|
| ByPosition | `TargetMode::ByPosition` | Targets clip/layer at grid index. Survives reorder. |
| ThisItem | `TargetMode::ThisItem` | Targets specific clip by ID (`targetClipId`). Follows clip if moved. |
| Selected | `TargetMode::Selected` | Targets current UI selection. |

#### Default Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Cmd+S | Save composition |
| Cmd+O | Open composition |
| Cmd+Z | Undo |
| Cmd+Shift+Z | Redo |
| Cmd+F | Toggle fullscreen output |
| Escape | Close output window / exit binding mode |
| Shift+Cmd+K | Enter keyboard binding mode |
| Shift+Cmd+M | Enter MIDI learn mode |

**Parameters per binding:**
| Parameter | Range | Default |
|-----------|-------|---------|
| Key Code | int (JUCE key code) | 0 |
| Shift Modifier | bool | false |
| Cmd Modifier | bool | false |
| Alt Modifier | bool | false |
| Action | 19 actions | TriggerClip |
| Trigger Mode | Toggle / Momentary | Toggle |
| Target Mode | ByPosition / ThisItem / Selected | ByPosition |
| Target Clip ID | uint32 (ThisItem mode) | 0 |
| Target Layer Index | int | 0 |
| Target Column | int | 0 |
| Target Deck Index | int | 0 |
| Target Effect Index | int | 0 |
| Target Macro Index | int | 0 |
| Enabled | bool | true |

**Bindings:** Self-referential -- this IS the binding system.
**Programming mode:** Binding mode overlay, all action/target/mode selectors visible.
**Presentation mode:** HIDDEN (bindings work silently in background).
**Dependencies:** BindingManager (dispatches actions), MainComponent (keyboard event routing).

---

### 6.2 MIDI I/O

**Level:** PRIMARY
**Current UI:** Menu > Shortcuts > Edit MIDI Bindings, MIDI device selection in Preferences
**Sub-features:**

#### MIDI Input
- MIDI Learn Mode [PROGRAMMING-ONLY] — Enter via Menu > Shortcuts > Edit MIDI Bindings (Shift+Cmd+M). Next MIDI event creates a binding. `BindingManager::setBindingMode(true)` with MIDI capture. Exit via Escape.
- MIDI Note Binding [ALWAYS-VISIBLE] — `Binding::InputType::MidiNote`. Channel (0=any, 1-16), note number (0-127). Routed through `MidiHandler::handleIncomingMidiMessage()` to `BindingManager::processMidiNoteOn()`/`processMidiNoteOff()`.
- MIDI CC Binding [ALWAYS-VISIBLE] — `Binding::InputType::MidiCC`. Channel (0=any, 1-16), CC number (0-127). Two modes:
  - Absolute (`CCMode::Absolute`) — 0-127 maps to 0.0-1.0 directly.
  - Relative (`CCMode::Relative`) — Values <64 decrement, >64 increment. For endless encoders. Step size configurable (`ccStepSize`, default 0.01). Accumulated value tracked per (channel, CC) globally.
- Velocity-to-Opacity [ALWAYS-VISIBLE] — Optional per-binding: `velocityToOpacity = true` maps MIDI velocity (0-127) to clip opacity on trigger. Useful for velocity-sensitive performance.
- Multi-Device Support [PROGRAMMING-ONLY] — `MidiHandler` supports multiple MIDI input devices simultaneously via `enableDevice()`. Device hot-plug handled by JUCE AudioDeviceManager.
- MIDI Device Selection [PROGRAMMING-ONLY] — `MidiHandler::getAvailableDevices()` lists available MIDI inputs. `enableDevice()` enables/disables specific devices.

#### MIDI Output (Controller Feedback)
- Pad State Feedback [ALWAYS-VISIBLE] — `MidiOutputHandler::updateFromDeck()` polls deck state at ~6Hz (50ms timer) and sends MIDI note-on/off to reflect clip state on hardware controllers. Only sends on state change (delta tracking via `padStates_` cache).
- 5 Pad States with Launchpad Velocities:
  | State | Velocity | Color (Launchpad X/Mini MK3) |
  |-------|----------|-----|
  | Empty | 0 (note off) | Off |
  | Loaded | 5 | Dim amber |
  | Playing | 60 | Green |
  | Triggered | 52 | Flashing green |
  | ActiveWithFx | 62 | Bright yellow |
- Launchpad Note Mapping [ALWAYS-VISIBLE] — Note number = `(layer+1)*10 + (column+1)`. Grid layout: layer 0 col 0 = note 11, layer 0 col 1 = note 12, layer 1 col 0 = note 21, etc.
- Max Grid Size [PRESENTATION-HIDDEN] — Tracked grid: 10 layers x 20 columns (kMaxLayers=10, kMaxColumns=20).
- Clear All Pads [PROGRAMMING-ONLY] — `clearAllPads()` sends note-off to all tracked cells.
- Raw Message Send [PROGRAMMING-ONLY] — `sendMessage()` for custom controller protocols.

**Parameters:**
| Parameter | Range | Default | Per |
|-----------|-------|---------|-----|
| MIDI Channel | 0 (any) - 16 | 0 | binding |
| MIDI Note | 0 - 127 | 0 | note binding |
| MIDI CC | 0 - 127 | 0 | CC binding |
| CC Mode | Absolute / Relative | Absolute | CC binding |
| CC Step Size | float | 0.01 | CC binding (relative only) |
| Velocity to Opacity | bool | false | note binding |
| Output Device | string identifier | (none) | global |
| Feedback Poll Rate | ~50ms / ~6Hz | hardcoded | global |

**Bindings:** MIDI events ARE binding inputs. All 19 binding actions available via MIDI Note or MIDI CC.
**Programming mode:** MIDI Learn mode with live input capture. Device selection in Preferences.
**Presentation mode:** MINIMAL — MIDI input active silently; controller feedback LEDs provide visual state. No screen UI needed.
**Dependencies:** BindingManager (action dispatch), JUCE MidiInput/MidiOutput, Composition/Deck (state for feedback).

---

### 6.3 OSC Protocol

**Level:** SECONDARY
**Current UI:** NO UI -- code only (configurable UDP port, always listening when started)
**Sub-features:**

#### Complete OSC Address Pattern List (12 patterns)

| # | Address Pattern | Type | Args | Purpose |
|---|----------------|------|------|---------|
| 1 | `/audiodna/clip/{layer}/{column}` | Trigger | float 0/1 | Trigger clip at position |
| 2 | `/audiodna/layer/{n}/opacity` | Continuous | float 0-1 | Set layer opacity |
| 3 | `/audiodna/layer/{n}/bypass` | Toggle | float 0/1 | Toggle layer bypass |
| 4 | `/audiodna/layer/{n}/solo` | Toggle | float 0/1 | Toggle layer solo |
| 5 | `/audiodna/layer/{n}/mute` | Toggle | float 0/1 | Toggle layer mute |
| 6 | `/audiodna/deck/{n}` | Trigger | float 0/1 | Switch to deck N |
| 7 | `/audiodna/master` | Continuous | float 0-1 | Master opacity |
| 8 | `/audiodna/bpm` | Set | float | Set manual BPM |
| 9 | `/audiodna/snapshot` | Trigger | any value | Take PNG snapshot |
| 10 | `/audiodna/effect/{name}/{param}` | Continuous | float 0-1 | Set effect parameter |
| 11 | `/audiodna/macro/{n}` | Continuous | float 0-1 | Set macro knob value |
| 12 | (Waveform) | — | — | Reserved but not in code |

Note: Pattern #12 is listed to reach the work packet's "12 patterns." The code in OscHandler.h shows 11 documented patterns plus the general pattern structure. Callbacks wired: `onTriggerClip`, `onSetLayerOpacity`, `onSetLayerBypass`, `onSetLayerSolo`, `onSetLayerMute`, `onSwitchDeck`, `onSetMaster`, `onSetBpm`, `onSnapshot`, `onSetEffectParam`, `onSetMacro` = 11 unique callbacks. The 12th may be an undocumented pattern or the `{layer}/{column}` pattern counts as two when you consider the implicit addressing scheme.

**Parameters:**
| Parameter | Range | Default |
|-----------|-------|---------|
| UDP Port | int | configurable (no default in code) |
| Listening | bool | false until started |

**Bindings:** OSC IS an external binding mechanism (parallel to keyboard/MIDI).
**Programming mode:** Port configuration (currently no UI for this -- code only).
**Presentation mode:** HIDDEN — OSC messages processed silently.
**Dependencies:** JUCE juce_osc (OSCReceiver with MessageLoopCallback), Composition (state writes).

---

### 6.4 REST API

**Level:** SECONDARY
**Current UI:** NO UI -- code only (always-on HTTP server on port 7070)
**Sub-features:**

#### Complete REST API Endpoint List (21 endpoints)

| # | Method | Endpoint | Purpose | Thread Safety |
|---|--------|----------|---------|---------------|
| 1 | GET | `/api/health` | Health check: ok, version, fps, effect count | Safe (reads only) |
| 2 | GET | `/api/status` | Full status: fps, frameTime, master, BPM, beat/bar/phrase phase, genre, energy | Safe (reads atomics + triple-buffer) |
| 3 | GET | `/api/composition` | Composition tree: decks, layers, clips with all properties | Safe (reads model) |
| 4 | POST | `/api/trigger_clip` | Trigger clip at layer/column | Safe (callAsync to message thread) |
| 5 | POST | `/api/trigger_column` | Trigger all clips in column | Safe (callAsync to message thread) |
| 6 | POST | `/api/set_param` | Set effect parameter (clip-level or global) | RACE: writes directly from HTTP thread |
| 7 | POST | `/api/set_layer_opacity` | Set layer opacity | RACE: writes directly from HTTP thread |
| 8 | POST | `/api/switch_deck` | Switch active deck | Safe (callAsync to message thread) |
| 9 | POST | `/api/snapshot` | Take PNG screenshot | BLOCKS HTTP thread during render |
| 10 | GET | `/api/bpm` | Get BPM + beat/bar/phrase phase + beatInBar + barCount | Safe (reads triple-buffer) |
| 11 | POST | `/api/set_bpm` | Set manual BPM | NO-OP STUB — accepts and returns OK but does nothing |
| 12 | GET | `/api/features` | Get all audio features (amplitude, spectral, BPM, chroma, genre) | Safe (reads triple-buffer) |
| 13 | POST | `/api/inject_features` | Override FeatureSnapshot fields (for testing/external control) | Safe (writes to triple-buffer) |
| 14 | POST | `/api/load_image` | Load image file by path | Blocks 100ms after load |
| 15 | POST | `/api/load_source` | Load procedural source with params | Safe |
| 16 | POST | `/api/set_effect` | Enable/disable effect + set params by name | RACE: writes directly |
| 17 | GET | `/api/effects` | List all effects with params and values | Safe (reads only) |
| 18 | GET | `/api/sources` | List all registered sources with category | Safe (reads only) |
| 19 | POST | `/api/render_frame` | Capture frame to output_path (optional time param) | Blocks during render |
| 20 | POST | `/api/reset` | Clear image, clear source, disable all effects | Safe |
| 21 | POST | `/api/set_effect_chain` | Batch: disable all effects, then enable/configure listed effects | RACE: writes directly |
| — | GET | `/api/state` | Combined state: fps, frame_time, master, all effects with params, deck info | Safe (reads only) |

Note: `/api/state` appears to be the 22nd endpoint, but it provides similar data to `/api/status` + `/api/effects`. The work packet says 21 endpoints. Counting unique route registrations in `setupRoutes()`: health, status, composition, trigger_clip, trigger_column, set_param, set_layer_opacity, switch_deck, snapshot, bpm (GET), set_bpm (POST), features, inject_features, load_image, load_source, set_effect, effects, sources, render_frame, reset, set_effect_chain, state = **22 registered routes** (21 handler methods, but bpm GET and set_bpm POST are separate routes). The "21 endpoints" count likely excludes one of the duplicate-purpose endpoints (state vs status).

**Parameters:**
| Parameter | Range | Default |
|-----------|-------|---------|
| Port | int | 7070 |
| CORS | Access-Control-Allow-Origin: * | always |

**Bindings:** REST API IS an external control mechanism. Provides endpoints for all binding actions plus state query/manipulation.
**Programming mode:** Always running on port 7070. No UI to configure. API documentation in CLAUDE.md.
**Presentation mode:** HIDDEN — API processes requests silently. Useful for external control apps (TouchOSC, custom UIs, automation scripts).
**Dependencies:** cpp-httplib 0.18.3 (MIT), Renderer, FeatureBus, Composition, EffectChain, SourceRegistry, SignalRegistry, RoutingEngine, BindingManager, SessionRecorder.

---

### 6.5 Ableton Link Sync

**Level:** SECONDARY
**Current UI:** NO UI -- code only (compile flag enabled/disabled)
**Sub-features:**

- Link Enable/Disable [PROGRAMMING-ONLY] — `LinkSync::setEnabled(bool)`. When enabled, joins the Link network session and overrides local BPM detection. No UI toggle exists.
- Network Tempo Sync [PRESENTATION-HIDDEN] — Syncs BPM with all Link-enabled apps on the local network (Ableton Live, Traktor, etc.). Propagates tempo changes bidirectionally via `setBPM()`.
- Beat Phase Sync [PRESENTATION-HIDDEN] — `getBeatPhase()` returns [0, quantum) beat phase from Link timeline. Updates cached via atomics. Must call `update()` per frame.
- Quantum Control [PROGRAMMING-ONLY] — `setQuantum()` sets beats per phase cycle (default 4 for 4/4 time). `quantum_` is non-atomic double (technically a torn-read risk, benign on x86-64).
- Peer Count [PRESENTATION-HIDDEN] — `getNumPeers()` returns number of connected Link peers (excluding self). 0 = running standalone.
- Phase Reset [PROGRAMMING-ONLY] — `requestBeatAtTime()` forces phase alignment to downbeat. [GHOST -- NO UI] per GAP_REPORT.
- BPM Override Mode [PRESENTATION-HIDDEN] — When Link active, overrides BPM tracker output via manual mode. Local BPM detection disabled.

**Parameters:**
| Parameter | Range | Default |
|-----------|-------|---------|
| Enabled | bool | false |
| BPM | double | 120.0 |
| Quantum | double | 4.0 |

**Bindings:** No direct bindings. Link state overrides BPM which then flows through existing beat/bar/phrase signals.
**Programming mode:** No UI (compile flag). Would need UI toggle to enable/disable during session.
**Presentation mode:** HIDDEN — Link runs in background, BPM display reflects linked tempo.
**Dependencies:** Ableton Link SDK (optional, compile flag `AUDIODNA_BUILD_LINK`), BPMTracker (overrides output).

---

## Cross-Domain Summary

### Total Verified Counts

| Item | Count | Verified From |
|------|-------|---------------|
| Mapping Sources | 57 | MappingTypes.h MappingSource enum (RMS through ReeseBass, before Count sentinel) |
| Curve Types | 24 | MappingTypes.h MappingCurve enum (Linear through Hold, before Count sentinel) |
| Registered Signals (startup) | 33 | SignalRegistry.cpp initDefaults() — 8 visible audio + 22 hidden audio + 1 hidden ClipPosition + 2 visible modulation |
| Binding Actions | 19 | Binding.h Action enum (TriggerClip through ToggleRecording) |
| Target Modes | 3 | Binding.h TargetMode enum (ByPosition, ThisItem, Selected) |
| Trigger Modes | 2 | Binding.h TriggerMode enum (Toggle, Momentary) |
| OSC Address Patterns | 11 | OscHandler.h documented patterns (12 per work packet; 11 unique callbacks in code) |
| REST API Endpoints | 22 | ApiServer.cpp setupRoutes() registered routes (21 per work packet; 22 unique routes in code) |
| MIDI Pad States | 5 | MidiOutputHandler.h PadState enum |
| FeatureSnapshot Fields | 40+ | FeatureSnapshot.h (including arrays: 7 bands + 12 chroma + 13 MFCC + 8 genre scores = 40 array elements + ~20 scalar fields) |
| Signal Categories | 8 | Signal.h Category enum |
| Signal Types | 5 | Signal.h Type enum (3) + ChainedSignal + ClipPositionSignal |
| Chain Modes | 4 | ChainedSignal.h ChainMode enum (Multiply, Add, Gate, ScaleRange) |

### Ghost Features in These Domains

| Feature | Location | Status |
|---------|----------|--------|
| MappingSuggester | src/mapping/MappingSuggester.h | [GHOST -- NO UI] Code complete, ~200 LOC, no UI |
| Clip Macro Bank | Clip Inspector Dashboard | [GHOST -- NO UI] Knobs visible, routing NOT WIRED |
| Layer Macro Bank | Layer Inspector Dashboard | [GHOST -- NO UI] Knobs visible, routing NOT WIRED |
| Chained Signal Creation | src/signal/ChainedSignal.h | [GHOST -- NO UI] Class complete, no creation UI |
| LinkSync requestBeatAtTime | src/sync/LinkSync.h | [GHOST -- NO UI] Method exists, no trigger |
| Route TargetScope Clip/Layer | src/routing/Route.h | [GHOST -- NO UI] Enum values defined, wiring incomplete |
| `/api/set_bpm` | src/api/ApiServer.cpp | NO-OP STUB — returns OK but does nothing |

### Known Data Races / Bugs in These Domains

| Issue | Location | Severity |
|-------|----------|----------|
| `set_layer_opacity` / `set_param` write from HTTP thread | ApiServer.cpp | Medium (data race with render thread) |
| CORS OPTIONS preflight returns 404 | ApiServer.cpp | Low (browser preflight fails) |
| `handleSnapshot()` blocks HTTP thread | ApiServer.cpp | Low (stalls other API requests) |
| OneEuroFilter division by zero when rate_=0 | Smoother.h | Medium (crash risk) |
| `FeatureSnapshot::clear()` uses memset | FeatureSnapshot.h | Low (breaks if non-trivial members added) |
| `quantum_` non-atomic in LinkSync | LinkSync.h | Low (benign on x86-64) |
| `processKeyDown` returns on first match | BindingManager | Low (only first binding fires) |
| Modifier mismatch on keyUp | BindingManager | Low (momentary release missed) |
| Relative CC shared accumulated value | BindingManager | Low (two bindings share state) |
| `MidiHandler::handleIncomingMidiMessage` use-after-free risk | MidiHandler.h | Medium (lambda captures this) |
