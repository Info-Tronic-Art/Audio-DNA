# Audio-DNA — Master Feature Inventory

**Purpose:** Authoritative source of truth for the design overhaul.
**Date:** 2026-05-19
**Method:** 3 parallel audit agents + 3 parallel inventory writers + verification agent
**Sources:** OurAppFeatures_1.md, FEATURES.md, 58K LOC codebase, 4231-node knowledge graph, runtime API verification

## Reading Guide
- **[GHOST — NO UI]** = Feature exists in code but has no user interface
- **[GHOST — DEFERRED]** = Feature exists in code, deferred to future build
- **[HIDDEN-V1]** = Component exists but hidden in v2 layout
- **[PLACEHOLDER]** = UI shell exists but content not implemented
- **[ALWAYS-VISIBLE]** = Must be accessible during live performance
- **[MINIMAL-IN-PRESENTATION]** = Needs compact representation during performance
- **[PRESENTATION-HIDDEN]** = Can be fully hidden during performance
- **[PROGRAMMING-ONLY]** = Only needed during setup/editing

## Ghost Feature Decisions (Boris, 2026-05-19)
| Feature | Decision | Notes |
|---------|----------|-------|
| MappingSuggester | INCLUDE | AI mapping suggestions — needs UI |
| Per-scope MacroBanks | INCLUDE | Wire Clip + Layer scopes (currently Global only) |
| Crossfader | EXCLUDE | Boris decided not to have this |
| LUT Loader | INCLUDE | Expose as global post-process |
| Camera Input | INCLUDE | Live camera as clip source |
| Syphon Input | DEFERRED | Future build |
| Chained Signals | INCLUDE as Macro Conditions | Gate + ScaleRange in MacroPanel, not separate UI |

## Parameter Census
- Effects: 135 (333 parameters)
- Sources: 108 (parameter count needs reconciliation — see notes)
- Total adjustable parameters: 1,253+
- Parameters without dedicated UI: ~1,050

> **Note:** Source parameter totals pending reconciliation against source code. Category-level counts are verified; individual parameter counts may have minor arithmetic discrepancies.

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
- Camera Input [GHOST — NO UI] — `Clip::MediaType::Camera` defined in model with `cameraDeviceIndex` field. Not wired to audio I/O but exists as media type. INCLUDE per Boris decision (Camera Input provides live camera as clip source; Syphon Input is a separate feature, DEFERRED).

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

#### Chained Signal (signal modulating signal) [GHOST — NO UI]
- ChainedSignal [PROGRAMMING-ONLY] — One signal modulates another. 4 chain modes: Multiply, Add, Gate, ScaleRange. Parameters: gain (1.0), modulationDepth (1.0), gateThreshold (0.1). Can be created via code/API but NO UI exists for creating chained signals. Existing chained signals are evaluated via `evaluateAll()`.

> **Boris decision:** Chained Signals will be exposed as MACRO CONDITIONS (Gate + ScaleRange) integrated into the MacroPanel source picker, NOT as a separate routing UI. See design/MACRO_CONDITIONS_PLAN.md.

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
- Clip Macro Bank (8 knobs) [GHOST — NO UI] — Per-clip dashboard knobs exist in Inspector UI but per-clip scope macro routing is NOT YET WIRED. Knobs display and accept manual values, but clip-scope macro distribution to linked parameters is not implemented.
- Layer Macro Bank (8 knobs) [GHOST — NO UI] — Per-layer dashboard knobs exist in Inspector UI but per-layer scope macro routing is NOT YET WIRED. Same situation as clip macros.
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
**Current UI:** [GHOST — NO UI]
**Sub-features:**

- Genre-Aware Suggestions [GHOST — NO UI] — `MappingSuggester::suggestMappings()` analyzes current audio snapshot and returns scored source-to-param recommendations (max 8). Each `Suggestion` contains: sourceName, targetCategory, targetEffect, targetParam, curveType, reason, relevance [0,1]. Sorted by relevance.
- Genre-Specific Suggestions [GHOST — NO UI] — `MappingSuggester::suggestGenreMappings()` returns genre-specific recommendations (max 6). Takes detected genre ID (0-7).
- Feature Activity Scoring [GHOST — NO UI] — `featureActivity()` scores a feature's relevance based on current value vs low/high thresholds.
- Universal Suggestions [GHOST — NO UI] — `addUniversalSuggestions()` generates mappings that work for any genre.

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
- Phase Reset [PROGRAMMING-ONLY] — `requestBeatAtTime()` forces phase alignment to downbeat. [GHOST — NO UI] per GAP_REPORT.
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

## Cross-Domain Summary (Domains 1, 5, 6)

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

### Ghost Features in Domains 1, 5, 6

| Feature | Location | Status |
|---------|----------|--------|
| MappingSuggester | src/mapping/MappingSuggester.h | [GHOST — NO UI] Code complete, ~200 LOC, no UI |
| Clip Macro Bank | Clip Inspector Dashboard | [GHOST — NO UI] Knobs visible, routing NOT WIRED |
| Layer Macro Bank | Layer Inspector Dashboard | [GHOST — NO UI] Knobs visible, routing NOT WIRED |
| Chained Signal Creation | src/signal/ChainedSignal.h | [GHOST — NO UI] Class complete, no creation UI |
| LinkSync requestBeatAtTime | src/sync/LinkSync.h | [GHOST — NO UI] Method exists, no trigger |
| Route TargetScope Clip/Layer | src/routing/Route.h | [GHOST — NO UI] Enum values defined, wiring incomplete |
| `/api/set_bpm` | src/api/ApiServer.cpp | NO-OP STUB — returns OK but does nothing |

### Known Data Races / Bugs in Domains 1, 5, 6

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

---

# Domain 2: Visual Content

## 2.1 Procedural Sources

**Level:** PRIMARY
**Current UI:** Sources Browser (Browser Panel tab "Sources") + Clip Inspector section 8.6 (Source Parameters)
**Sub-features:**

108 procedural sources across 17 categories, 628 total parameters. Each source is a GLSL 410 fragment shader rendered on a fullscreen quad. All source params normalized [0,1]. All sources receive 42 audio uniforms (u_rms, u_bass, u_beatPhase, etc.) automatically.

### Category: Noise (3 sources, 12 params)

- Perlin Noise [PRESENTATION-HIDDEN] — Noise. Params: Scale, Speed, Octaves, Color Shift (4)
- Plasma [PRESENTATION-HIDDEN] — Noise. Params: Speed, Complexity, Color Cycle, Intensity (4)
- Voronoi [PRESENTATION-HIDDEN] — Noise. Params: Scale, Speed, Edge Width, Color Mode (4)

### Category: Fractal — 2D (7 sources, 52 params)

- Kaleidoscopic Fractal [PRESENTATION-HIDDEN] — Fractal. Params: Iterations, Fold Angle, Zoom, Rotation, Color Shift, Palette (6)
- Mandelbrot / Julia [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Location, Zoom, Center X, Center Y, Julia Mix, Max Iterations, Power, Color Speed, Color Shift, Palette (11)
- Julia Set [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Location, C Real, C Imaginary, Zoom, Iterations, Color Speed, Color Shift, Palette (9)
- Burning Ship [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Location, Center X, Center Y, Zoom, Iterations, Color Speed, Color Shift, Palette (9)
- Newton Fractal [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Power, Zoom, Damping, Color Shift, Palette (6)
- Sierpinski [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Mode, Zoom, Iterations, Rotation, Color Shift, Palette (7)
- Apollonian Gasket [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Zoom, Iterations, Rotation, Color Shift, Palette (6)

### Category: 3D — Ray-Marched Fractals (8 sources, 109 params)

- Mandelbulb [PRESENTATION-HIDDEN] — 3D. Params: Power, Iterations, Angle X, Angle Y, Zoom, Speed, Detail, Color Shift, Cross Section, Slice Count, Glow, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (16)
- Menger Sponge [PRESENTATION-HIDDEN] — 3D. Params: Iterations, Angle X, Angle Y, Zoom, Speed, Twist, Color Shift, Cross Section, Slice Count, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (14)
- Kaleidoscopic IFS [PRESENTATION-HIDDEN] — 3D. Params: Scale, Iterations, Fold Type, Angle X, Angle Y, Zoom, Speed, Offset, Color Shift, Cross Section, Slice Count, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (16)
- Julia Set 3D [PRESENTATION-HIDDEN] — 3D. Params: Location, C Real, C Imaginary, Angle X, Angle Y, Zoom, Speed, Iterations, Color Shift, Cross Section, Slice Count, Glow, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (17)
- Burning Ship 3D [PRESENTATION-HIDDEN] — 3D. Params: Power, Angle X, Angle Y, Zoom, Speed, Color Shift, Cross Section, Slice Count, Glow, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (14)
- Newton 3D [PRESENTATION-HIDDEN] — 3D. Params: Power, Angle X, Angle Y, Zoom, Speed, Damping, Height, Color Shift, Trail Distance, Trail Fade, Slice Count, Slice Distance, Feedback, Palette (14)
- Sierpinski Tetrahedron [PRESENTATION-HIDDEN] — 3D. Params: Iterations, Angle X, Angle Y, Zoom, Speed, Color Shift, Cross Section, Slice Count, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (13)
- Apollonian 3D [PRESENTATION-HIDDEN] — 3D. Params: Scale, Iterations, Angle X, Angle Y, Zoom, Speed, Color Shift, Cross Section, Slice Count, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (14)

### Category: 3D — Torus / Tunnel (8 sources, 110 params)

Shared torus controls (12 params each): Orbit, Tilt, Speed, Zoom, Lens Shape, Lens Rotate, Depth Fade, Tube Radius, Pinch, Heart, Shading, Color Shift. Plus per-source unique params.

- Striped Torus [PRESENTATION-HIDDEN] — 3D. Unique: Stripe Count, Twist + 12 shared (14)
- Spiral Vortex [PRESENTATION-HIDDEN] — 3D. Unique: Twist, Stripe Count + 12 shared (14)
- Checker Torus [PRESENTATION-HIDDEN] — 3D. Unique: Grid U, Grid V + 12 shared (14)
- Ribbed Vortex [PRESENTATION-HIDDEN] — 3D. Unique: Ridge Count, Color Mix + 12 shared (14)
- Wormhole Tunnel [PRESENTATION-HIDDEN] — 3D. Unique: Warp + 12 shared (13)
- Twisted Torus [PRESENTATION-HIDDEN] — 3D. Unique: Twist, Stripe Count + 12 shared (14)
- Wormhole [PRESENTATION-HIDDEN] — 3D. Unique: Warp, Glow + 12 shared (14)
- Torus Hole [PRESENTATION-HIDDEN] — 3D. Unique: Stripe Count, Twist, Stripe Angle, Stripe Scale, Stripe Width, Phi Offset, Theta Offset + Camera(4) + Lens(3) + Tube Radius + Pinch + Heart + Shading + Color Shift (20)

### Category: 3D — Other (8 sources, 42 params)

- Spiral Tunnel [PRESENTATION-HIDDEN] — 3D. Params: Speed, Arms, Depth, Twist, Color Shift (5)
- Crystal Cavern [PRESENTATION-HIDDEN] — 3D. Params: Speed, Crystal Size, Reflectivity, Light Color, Fog Density, Complexity (6)
- Infinite Corridor [PRESENTATION-HIDDEN] — 3D. Params: Speed, Width, Wall Pattern, Light Spacing, Light Intensity, Color (6)
- Orbit Chamber [PRESENTATION-HIDDEN] — 3D. Params: Object Count, Object Type, Orbit Speed, Orbit Radius, Material, Light Orbit, Color Shift (7)
- Scroll Plane [PRESENTATION-HIDDEN] — 3D. Params: Speed X, Speed Y, Scale, Warp, Color Shift (5)
- Rotating Cube Map [PRESENTATION-HIDDEN] — 3D. Params: Rotation X, Rotation Y, Scale, Light, Color Shift (5)
- Dual Plane Drift [PRESENTATION-HIDDEN] — 3D. Params: Speed, Rotation, Distance, Scale, Color Shift (5)
- DNA Helix [PRESENTATION-HIDDEN] — 3D. Params: Speed, Zoom, Glow, Color Shift (4)

### Category: Geometric (11 sources, 60 params)

- Geometric Tunnel [PRESENTATION-HIDDEN] — Geometric. Params: Speed, Segments, Twist, Color Shift (4)
- Color Gradient [PRESENTATION-HIDDEN] — Geometric. Params: Angle, Speed, Color 1, Color 2 (4)
- Shape Generator [PRESENTATION-HIDDEN] — Geometric. Params: Shape, Size, Rotation, Outline, Color Shift (5)
- Infinite Zoom [PRESENTATION-HIDDEN] — Geometric. Params: Speed, Layers, Rotation, Color Shift (4)
- Moire Interference [PRESENTATION-HIDDEN] — Geometric. Params: Pattern, Frequency, Offset X, Offset Y, Rotation, Zoom, Color Shift (7)
- Astral Grid [PRESENTATION-HIDDEN] — Geometric. Params: Grid Size, Scroll Speed, Tilt, Warp, Glow, Horizon Color (6)
- Radial Burst [PRESENTATION-HIDDEN] — Geometric. Params: Ray Count, Length, Rotation, Width, Taper, Glow, Color Shift (7)
- Hex Grid [PRESENTATION-HIDDEN] — Geometric. Params: Cell Size, Pattern, Fill, Edge Width, Rotation, Color Mode (6)
- Sacred Geometry [PRESENTATION-HIDDEN] — Geometric. Params: Pattern, Rotation, Breathe, Line Width, Glow, Reveal, Color Shift (7)
- Radar Sweep [PRESENTATION-HIDDEN] — Geometric. Params: Speed, Decay, Grid, Color Shift (4)
- Dot Matrix Wave [PRESENTATION-HIDDEN] — Geometric. Params: Density, Speed, Damping, Dot Size, Color, Sources (6)

### Category: Pattern (8 sources, 38 params)

- Checkerboard [PRESENTATION-HIDDEN] — Pattern. Params: Columns, Rows, Color 1 Hue, Color 2 Hue (4)
- Line Pattern [PRESENTATION-HIDDEN] — Pattern. Params: Count, Width, Rotation, Speed, Color Shift (5)
- Concentric Rings [PRESENTATION-HIDDEN] — Pattern. Params: Count, Spacing, Width, Rotation, Color Shift (5)
- Sine Oscillator [PRESENTATION-HIDDEN] — Pattern. Params: Waves, Frequency, Amplitude, Modulation, Thickness, Color Shift (6)
- Spiral Pattern [PRESENTATION-HIDDEN] — Pattern. Params: Arms, Zoom, Speed, Distortion, Color Shift (5)
- Terrain Lines [PRESENTATION-HIDDEN] — Pattern. Params: Height, Lines, Jagginess, Speed, Tilt, Color Shift (6)
- Bump Light [PRESENTATION-HIDDEN] — Pattern. Params: Light X, Light Y, Intensity, Bumps, Color Shift (5)
- Glitch Grid [PRESENTATION-HIDDEN] — Pattern. Params: Grid Size, Chaos, Flicker, Color Shift (4)

### Category: Lines (11 sources, 62 params)

- Zigzag Lines [PRESENTATION-HIDDEN] — Lines. Params: Count, Amplitude, Speed, Thickness, Color Shift (5)
- Star Burst [PRESENTATION-HIDDEN] — Lines. Params: Rays, Thickness, Speed, Taper, Color Shift (5)
- Polygon Lines [PRESENTATION-HIDDEN] — Lines. Params: Sides, Size, Thickness, Layers, Speed, Color Shift (6)
- Waveform Lines [PRESENTATION-HIDDEN] — Lines. Params: Waveform, Frequency, Amplitude, Count, Thickness, Speed, Color Shift (7)
- Lissajous [PRESENTATION-HIDDEN] — Lines. Params: Ratio X, Ratio Y, Phase, Thickness, Speed, Color Shift (6)
- Spirograph [PRESENTATION-HIDDEN] — Lines. Params: Inner Radius, Offset, Thickness, Speed, Color Shift (5)
- Angular Grid [PRESENTATION-HIDDEN] — Lines. Params: Angle, Count, Thickness, Symmetry, Speed, Color Shift (6)
- Fractal Tree [PRESENTATION-HIDDEN] — Lines. Params: Branches, Angle, Depth, Thickness, Speed, Color Shift (6)
- Laser Scan [PRESENTATION-HIDDEN] — Lines. Params: Beams, Speed, Thickness, Spread, Color Shift (5)
- Moire Lines [PRESENTATION-HIDDEN] — Lines. Params: Density, Angle Offset, Speed, Layers, Color Shift (5)
- Line Generator [PRESENTATION-HIDDEN] — Lines. Params: Pattern, Count, Thickness, Speed, Feedback, Feedback Zoom, Feedback Rotation, Feedback Decay, Color Shift (9)

### Category: Wireframe (7 sources, 63 params)

All share 9 params: Shape, Density, Rotation X, Rotation Y, Rotation Z, Thickness, Perspective, Glow, Color Shift.

- Wireframe Sphere [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Torus [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Cube [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Cylinder [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Cone [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Icosahedron [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Wolf [PRESENTATION-HIDDEN] — Wireframe. Params: (9)

### Category: Audio-Visual (9 sources, 42 params)

- Audio Waveform [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Style, Thickness, Glow, Color Shift (4)
- Spectrum Landscape [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Height Scale, Camera Angle, Color Mode, Glow, Smoothing (5)
- Chromatic Ring [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Ring Width, Glow, Rotation, Ripple (4)
- Band Tower [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Shape, Spacing, Reflection, Color Mode, Smoothing, 3D Rotation (6)
- Timbral Nebula [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Particle Count, Spread, Trail Length, Color Source, Glow, Sensitivity (6)
- Structural Landscape [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Terrain Scale, History Length, Drama, Color Palette, Fog, Camera Height (6)
- Cymatics [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Resonance, Damping, Color Shift (3)
- Spectral Waterfall [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Scroll Speed, Color Mode, Log Scale (3)
- Spectral Ring [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Radius, Thickness, Glow, Rotation, Color Shift (5)

### Category: Nature (6 sources, 26 params)

- Reaction-Diffusion [PRESENTATION-HIDDEN] — Nature. Stateful (ping-pong FBOs). Params: Feed Rate, Kill Rate, Diffusion A, Diffusion B (4)
- Cellular Automata [PRESENTATION-HIDDEN] — Nature. Stateful (ping-pong FBOs). Params: Mode, Birth Low, Birth High, Survival Low, Color Shift (5)
- Fire Wall [PRESENTATION-HIDDEN] — Nature. Params: Height, Turbulence, Speed, Temperature, Density, Wind (6)
- Water Caustics [PRESENTATION-HIDDEN] — Nature. Params: Complexity, Speed, Brightness, Color, Distortion, Scale (6)
- Electric Arc [PRESENTATION-HIDDEN] — Nature. Params: Arc Count, Chaos, Thickness, Branches, Glow, Color (6)
- Fire [PRESENTATION-HIDDEN] — Nature. Params: Height, Turbulence, Speed, Color Shift (4)

### Category: Organic (1 source, 5 params)

- Metaballs [PRESENTATION-HIDDEN] — Organic. Params: Count, Size, Speed, Blend, Color Shift (5)

### Category: Utility (2 sources, 11 params)

- Solid Color [PROGRAMMING-ONLY] — Utility. Params: Red, Green, Blue (3)
- Strobe Light [MINIMAL-IN-PRESENTATION] — Utility. Params: Frequency, Fade, Color 1 Red, Color 1 Green, Color 1 Blue, Color 2 Red, Color 2 Green, Color 2 Blue (8)

### Category: Text (2 sources, 12 params)

- Scrolling Text Wall [PRESENTATION-HIDDEN] — Text. Params: Speed, Density, Size, Color Shift (4)
- Text Animator [PRESENTATION-HIDDEN] — Text. Params: Font Size, Text Red, Text Green, Text Blue, Animation, Speed, Columns, Spacing (8)

### Category: Math (8 sources, 46 params)

- Lissajous Weaver [PRESENTATION-HIDDEN] — Math. Params: Frequency X, Frequency Y, Phase, Decay, Harmonics, Thickness, Color Shift (7)
- Fermat Spiral Garden [PRESENTATION-HIDDEN] — Math. Params: Count, Divergence Angle, Shape, Growth, Pulse, Color Spread (6)
- Hyperbolic Tiling [PRESENTATION-HIDDEN] — Math. Params: Polygon Sides, Vertex Order, Rotation, Zoom, Color Scheme, Line Width (6)
- Penrose Pulse [PRESENTATION-HIDDEN] — Math. Params: Generation, Ripple Speed, Color Mode, Edge Glow, Morph (5)
- Superformula [PRESENTATION-HIDDEN] — Math. Params: Symmetry, Roundness, Concavity, Blobbiness, Size, Color Shift (6)
- Truchet Labyrinth [PRESENTATION-HIDDEN] — Math. Params: Density, Style, Thickness, Glow, Color Shift (5)
- Rose Curves [PRESENTATION-HIDDEN] — Math. Params: Petals, Thickness, Layers, Glow, Color Shift (5)
- Fibonacci Spiral [PRESENTATION-HIDDEN] — Math. Params: Elements, Size, Spread, Glow, Color Shift (5)

### Category: Simulation (3 sources, 18 params)

All stateful (ping-pong FBOs).

- Strange Attractor [PRESENTATION-HIDDEN] — Simulation. Params: Attractor Type, Speed, Trail Length, Rotation, Glow, Color Mode (6)
- Gravity Well [PRESENTATION-HIDDEN] — Simulation. Params: Particle Density, Gravity, Scatter, Trail Length, Wells, Color Mode (6)
- Fluid Dynamics [PRESENTATION-HIDDEN] — Simulation. Params: Viscosity, Diffusion, Injection Radius, Color Mode, Curl, Decay (6)

### Category: Particle (3 sources, 12 params)

- Lightning Storm [PRESENTATION-HIDDEN] — Particle. Params: Intensity, Branches, Glow, Color Shift (4)
- Starfield [PRESENTATION-HIDDEN] — Particle. Params: Speed, Density, Streak, Color Shift (4)
- Particle Nebula [PRESENTATION-HIDDEN] — Particle. Params: Density, Scale, Speed, Color Shift (4)

### Category: Lighting (1 source, 6 params)

- Laser Scanner [PRESENTATION-HIDDEN] — Lighting. Params: Pattern, Beam Count, Color, Speed, Spread, Flicker (6)

### Category: Routing (1 source, 1 param)

- Layer Router [PROGRAMMING-ONLY] — Routing. Routes another layer's output as source content. Params: Source Layer (1)

**Total procedural sources: 108** (101 direct registerSource + 7 via registerWireframe helper)
**Total procedural source parameters: 628**

**Parameters:** All 628 source parameters listed above
**Bindings:** All source parameters are signal-connectable via UniversalParamControl (can bind to audio features, oscillators, envelopes, macros, BPM sync, clip position)
**Programming mode:** Sources Browser shows all 109 sources in 17 collapsible categories with search. Clip Inspector section 8.6 shows per-source parameter sliders.
**Presentation mode:** HIDDEN — source selection and parameter tuning are programming activities. During performance, sources are pre-configured and triggered via clip cells or autopilot.
**Dependencies:** OpenGL 4.1 Core, GLSL 410, FeatureSnapshot (audio uniforms), ShaderManager

---

## 2.2 MilkDrop / projectM Visualizer

**Level:** PRIMARY
**Current UI:** MilkDrop Browser (Browser Panel tab "MilkDrop") + VJ Clip mode in Deck Grid
**Sub-features:**
  - Preset Library [PRESENTATION-HIDDEN] — ~9,800 MilkDrop presets via libprojectM-4. 4 sub-tabs: Curated (30), Favorites, Recent (last 20), All (searchable)
  - Jukebox Mode [ALWAYS-VISIBLE] — Auto-advance through presets with configurable timing (4/8/16/32 beats or 10/30/60 seconds) and blend crossfade. Pool selection: All / Curated / Favorites. Play/Stop, navigation (</>/?/Lock)
  - VJ Clip Mode [ALWAYS-VISIBLE] — Click to preview, drag single preset to deck cell as a clip
  - Playlist Mode [PRESENTATION-HIDDEN] — Multi-select presets, drag group to create cycling playlist clip
  - Audio Feed [ALWAYS-VISIBLE] — PCM audio buffer (2048 samples) fed to projectM each frame for audio-reactive visualization
  - Preset Selector [PRESENTATION-HIDDEN] — Audio-driven preset selection system
  - GL State Save/Restore [ALWAYS-VISIBLE] — projectM manages its own shaders and feedback loops; Audio-DNA saves/restores full GL state around each render call
**Parameters:** Jukebox timing, Jukebox blend, Jukebox pool, preset lock
**Bindings:** Next/Prev/Random preset via keyboard/MIDI bindings. Lock toggle via binding.
**Programming mode:** Full MilkDrop Browser with all 4 sub-tabs, search, drag-to-deck
**Presentation mode:** MINIMAL — Jukebox auto-play or pre-loaded VJ clips. Next/Prev/Random/Lock via bindings.
**Dependencies:** libprojectM-4 (optional, compile flag `AUDIODNA_HAS_PROJECTM`), MilkDrop preset directory

---

## 2.3 Video Playback

**Level:** PRIMARY
**Current UI:** Clip Inspector section 8.3 (Transport) + section 8.7 (Video)
**Sub-features:**
  - FFmpeg Decode Pipeline [ALWAYS-VISIBLE] — Opens video via avformat_open_input, finds video stream, decodes via avcodec_send_packet/receive_frame, converts via sws_scale, uploads to GL texture
  - Codec Support [ALWAYS-VISIBLE] — H.264, H.265, ProRes, HAP (including HAP Alpha), MJPEG, and any codec supported by FFmpeg
  - Container Support [ALWAYS-VISIBLE] — MP4, MOV, AVI, MKV, WebM
  - Transport Modes [ALWAYS-VISIBLE] — Timeline (time-based) or BPM Sync (beat-locked). In BPM Sync, beatDivision x BPM determines playback speed
  - Loop Modes [ALWAYS-VISIBLE] — Loop, Ping-Pong, One Shot
  - In/Out Points [MINIMAL-IN-PRESENTATION] — Draggable on timeline [0,1]
  - Speed Control [MINIMAL-IN-PRESENTATION] — 0-4x playback rate, divide/multiply buttons, reverse toggle
  - Content Beats [PROGRAMMING-ONLY] — BPM Sync only: how many beats the video content represents (for exact timing of authored content)
  - Beats/Cycle [PROGRAMMING-ONLY] — BPM Sync only: 1/4 to 16 beats per playback cycle
  - Cuepoints [ALWAYS-VISIBLE] — 8 named cuepoints per clip. Set at current playhead, trigger to jump. Clear via right-click.
  - Trigger Modes [PROGRAMMING-ONLY] — Restart / Continue / Relative (how clip responds when re-triggered)
**Parameters:** Speed (0-4x), Duration (0.1-300s timeline / 1-64 beats BPM sync), In Point, Out Point, Content Beats, Beats/Cycle, Opacity, Width, Height, Blend Mode, Alpha Type, R/G/B/A channel toggles
**Bindings:** Play/Pause/Stop, cuepoint triggers, speed via MIDI CC
**Programming mode:** Full transport controls, in/out editing, BPM sync configuration, content beats
**Presentation mode:** MINIMAL — transport runs automatically via autopilot or bindings. Speed and cuepoints accessible via bindings.
**Dependencies:** FFmpeg 8.0 (libavformat, libavcodec, libavutil, libswscale)

---

## 2.4 Image Sequences

**Level:** SECONDARY
**Current UI:** Clip Inspector section 8.3 (Transport) — Images/Sec slider appears for ImageSequence type
**Sub-features:**
  - Multi-Image Loading [PROGRAMMING-ONLY] — Load folder of images as a clip
  - Configurable FPS [MINIMAL-IN-PRESENTATION] — Images/Sec slider controls playback rate
  - BPM Sync Playback [ALWAYS-VISIBLE] — Same BPM sync transport as video: beat division controls cycle speed
  - Loop Modes [ALWAYS-VISIBLE] — Loop, Ping-Pong, One Shot (shared with video transport)
**Parameters:** Images/Sec (FPS), same transport parameters as video (speed, duration, in/out, loop mode, BPM sync)
**Bindings:** Same transport bindings as video playback
**Programming mode:** Load image folder, configure FPS and BPM sync
**Presentation mode:** HIDDEN — pre-configured during setup, runs automatically
**Dependencies:** JUCE image loading (PNG, JPEG, BMP, GIF, TIFF)

---

## 2.5 Media Management

**Level:** UTILITY
**Current UI:** Composition menu (Menu Bar)
**Sub-features:**
  - Collect Media [PROGRAMMING-ONLY] — Menu > Composition > Collect Media. Packages composition + all referenced media files into one folder for portability.
  - Relocate Files [PROGRAMMING-ONLY] — Menu > Composition > Relocate Files. Finds and relinks missing media files when a composition is opened on a different machine or after moving files.
**Parameters:** None (menu actions)
**Bindings:** None
**Programming mode:** Available via menu
**Presentation mode:** HIDDEN
**Dependencies:** Filesystem access, JUCE File utilities

---

## 2.6 Camera Input [GHOST — NO UI]

**Level:** SECONDARY
**Current UI:** NO UI — code only. `Clip::MediaType::Camera` enum value exists. `cameraDeviceIndex` field in Clip model (-1 = none). Not wired to UI.
**Sub-features:**
  - Camera Device Selection [GHOST — NO UI] — `cameraDeviceIndex` field defined but no device enumeration UI
  - Live Feed as Clip Source [GHOST — NO UI] — MediaType::Camera defined in enum but no rendering path implemented
**Parameters:** cameraDeviceIndex (integer, model only)
**Bindings:** None
**Programming mode:** Not accessible
**Presentation mode:** Not accessible
**Dependencies:** Would require JUCE CameraDevice or platform-specific capture API
**Note:** Boris decided INCLUDE in design — needs UI and rendering implementation.

---

# Domain 3: Effects & Processing

## 3.1 Visual Effects Library

**Level:** PRIMARY
**Current UI:** FX Browser (Browser Panel tab "FX") + Clip Inspector section 8.9 (Effects Stack) + Layer Inspector section 9.12 + Composition Inspector section 10.8
**Sub-features:**

135 effects across 11 categories, 333 total parameters. Each effect is a GLSL 410 fragment shader. All effect params normalized [0,1] and signal-connectable. Effects applied via ping-pong FBO chain.

### Category: Warp (27 effects, 67 params)

- Ripple [PRESENTATION-HIDDEN] — Warp. Params: intensity, freq, speed (3)
- Bulge [PRESENTATION-HIDDEN] — Warp. Params: amount, center_x, center_y (3)
- Wave [PRESENTATION-HIDDEN] — Warp. Params: amplitude, frequency, direction (3)
- Liquid [PRESENTATION-HIDDEN] — Warp. Params: viscosity, turbulence (2)
- Kaleidoscope [PRESENTATION-HIDDEN] — Warp. Params: segments, rotation (2)
- Fisheye [PRESENTATION-HIDDEN] — Warp. Params: amount (1)
- Swirl [PRESENTATION-HIDDEN] — Warp. Params: amount, radius (2)
- Polar Coords [PRESENTATION-HIDDEN] — Warp. Params: amount (1)
- Twirl [PRESENTATION-HIDDEN] — Warp. Params: amount, radius (2)
- Shear [PRESENTATION-HIDDEN] — Warp. Params: x, y (2)
- Elastic Bounce [PRESENTATION-HIDDEN] — Warp. Params: amount, freq (2)
- Ripple Pond [PRESENTATION-HIDDEN] — Warp. Params: intensity, freq (2)
- Diamond Distort [PRESENTATION-HIDDEN] — Warp. Params: size, amount (2)
- Barrel Distort [PRESENTATION-HIDDEN] — Warp. Params: amount (1)
- Sine Grid [PRESENTATION-HIDDEN] — Warp. Params: freq, amount (2)
- Glitch Displace [PRESENTATION-HIDDEN] — Warp. Params: amount, speed (2)
- Quad Mirror [PRESENTATION-HIDDEN] — Warp. Params: center x, center y (2)
- Flip [PRESENTATION-HIDDEN] — Warp. Params: horizontal, vertical (2)
- Warp Field [PRESENTATION-HIDDEN] — Warp. Params: amount, frequency, speed (3)
- Slide Wrap [PRESENTATION-HIDDEN] — Warp. Params: x, y (2)
- Tile Grid [PRESENTATION-HIDDEN] — Warp. Params: columns, rows, offset, zoom (4)
- Spot Zoom [PRESENTATION-HIDDEN] — Warp. Params: center x, center y, size, zoom, shape, background (6)
- Bendoscope [PRESENTATION-HIDDEN] — Warp. Params: divisions, bend, rotation (3)
- UV Remap [PRESENTATION-HIDDEN] — Warp. Params: amount, scale, speed (3)
- Liquid Morph [PRESENTATION-HIDDEN] — Warp. Params: viscosity, amount, scale (3)
- Zoom Warp [PRESENTATION-HIDDEN] — Warp. Params: speed, rotation, center x, center y (4)
- Density Wave [PRESENTATION-HIDDEN] — Warp (audio). Params: amount, direction, wavelength (3)

### Category: Color (31 effects, 64 params)

- Hue Shift [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Saturation [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Brightness [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Duotone [PRESENTATION-HIDDEN] — Color. Params: color1_r, color1_g, color1_b, color2_r, color2_g, color2_b, mix (7)
- Chromatic Aberration [PRESENTATION-HIDDEN] — Color. Params: amount, angle (2)
- Invert [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Posterize [PRESENTATION-HIDDEN] — Color. Params: levels (1)
- Color Shift [PRESENTATION-HIDDEN] — Color. Params: red, green, blue (3)
- Thermal [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Contrast [PRESENTATION-HIDDEN] — Color. Params: contrast, balance (2)
- Sepia [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Cross Process [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Split Tone [PRESENTATION-HIDDEN] — Color. Params: shadow_hue, highlight_hue, amount (3)
- Color Halftone [PRESENTATION-HIDDEN] — Color. Params: scale, amount (2)
- Dither [PRESENTATION-HIDDEN] — Color. Params: levels, amount (2)
- Heat Map [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Selective Color [PRESENTATION-HIDDEN] — Color. Params: hue, range (2)
- Film Grain [PRESENTATION-HIDDEN] — Color. Params: amount, size (2)
- Gamma Levels [PRESENTATION-HIDDEN] — Color. Params: black, white, gamma (3)
- Solarize [PRESENTATION-HIDDEN] — Color. Params: threshold, amount (2)
- Greyscale [PRESENTATION-HIDDEN] — Color. Params: method, amount (2)
- Threshold [PRESENTATION-HIDDEN] — Color. Params: level, amount (2)
- Exposure [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Vibrance [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Auto Mask [PRESENTATION-HIDDEN] — Color. Params: threshold, softness, invert (3)
- Chroma Key [PRESENTATION-HIDDEN] — Color. Params: hue, tolerance, softness, amount (4)
- Palette Remap [PRESENTATION-HIDDEN] — Color. Params: palette, cycle, amount (3)
- Color Grade [PRESENTATION-HIDDEN] — Color. Uses LUT texture (LUTLoader). Params: amount (1)
- Pitch Chromatic Shift [PRESENTATION-HIDDEN] — Color (audio). Params: amount, mode, saturation (3)
- Key Palette [PRESENTATION-HIDDEN] — Color (audio). Params: amount, brightness, saturation (3)
- Chroma Dissolve [PRESENTATION-HIDDEN] — Color (audio). Params: amount, softness (2)

### Category: Glitch (15 effects, 40 params)

- Pixel Scatter [PRESENTATION-HIDDEN] — Glitch. Params: amount, seed (2)
- RGB Split [PRESENTATION-HIDDEN] — Glitch. Params: amount, angle (2)
- Block Glitch [PRESENTATION-HIDDEN] — Glitch. Params: intensity, block_size (2)
- Scanlines [PRESENTATION-HIDDEN] — Glitch. Params: intensity, frequency (2)
- Digital Rain [PRESENTATION-HIDDEN] — Glitch. Params: intensity, speed (2)
- Noise [PRESENTATION-HIDDEN] — Glitch. Params: amount, speed (2)
- Mirror [PRESENTATION-HIDDEN] — Glitch. Params: horizontal, vertical (2)
- Pixelate [PRESENTATION-HIDDEN] — Glitch. Params: size (1)
- Pixel Explosion [PRESENTATION-HIDDEN] — Glitch. Params: force, decay, center x, center y (4)
- Color Flash [PRESENTATION-HIDDEN] — Glitch. Params: intensity, red, green, blue, decay (5)
- Fragment Burst [PRESENTATION-HIDDEN] — Glitch. Params: copies, spread, rotation, scale (4)
- Signal Destroy [PRESENTATION-HIDDEN] — Glitch. Params: amount, speed, mode (3)
- Rhythm Slice [PRESENTATION-HIDDEN] — Glitch (audio). Params: amount, slices, sync (3)
- Data Corruption [PRESENTATION-HIDDEN] — Glitch. Params: amount, block size, color damage (3)
- Glitch Sort [PRESENTATION-HIDDEN] — Glitch. Params: amount, threshold, direction (3)

### Category: Blur / Post (10 effects, 21 params)

- Gaussian Blur [PRESENTATION-HIDDEN] — Blur. Params: radius (1)
- Zoom Blur [PRESENTATION-HIDDEN] — Blur. Params: amount, center_x, center_y (3)
- Shake [PRESENTATION-HIDDEN] — Blur. Params: amount_x, amount_y (2)
- Vignette [PRESENTATION-HIDDEN] — Blur. Params: intensity, softness (2)
- Motion Blur [PRESENTATION-HIDDEN] — Blur. Params: amount, angle (2)
- Glow [PRESENTATION-HIDDEN] — Blur. Params: amount, threshold (2)
- Edge Detect [PRESENTATION-HIDDEN] — Blur. Params: amount (1)
- Sharpen [PRESENTATION-HIDDEN] — Blur. Params: amount, radius (2)
- Edge Blur [PRESENTATION-HIDDEN] — Blur. Params: threshold, amount (2)
- Drop Shadow [PRESENTATION-HIDDEN] — Blur. Params: offset x, offset y, blur, opacity (4)

### Category: 3D / Depth (9 effects, 20 params)

- Perspective Tilt [PRESENTATION-HIDDEN] — 3D. Params: tilt_x, tilt_y (2)
- Cylinder Wrap [PRESENTATION-HIDDEN] — 3D. Params: amount, axis (2)
- Sphere Wrap [PRESENTATION-HIDDEN] — 3D. Params: amount (1)
- Tunnel [PRESENTATION-HIDDEN] — 3D. Params: speed, radius (2)
- Page Curl [PRESENTATION-HIDDEN] — 3D. Params: amount, radius (2)
- Parallax Layers [PRESENTATION-HIDDEN] — 3D. Params: amount, direction (2)
- Dot Field [PRESENTATION-HIDDEN] — 3D. Params: size, spacing, depth (3)
- Luminance Terrain [PRESENTATION-HIDDEN] — 3D. Params: height, segments, angle (3)
- Voxel Matrix [PRESENTATION-HIDDEN] — 3D. Params: size, height, rotation (3)

### Category: Pattern / Stylization (19 effects, 50 params)

- CRT [PRESENTATION-HIDDEN] — Pattern. Params: curvature, scanline (2)
- VHS [PRESENTATION-HIDDEN] — Pattern. Params: amount, tracking (2)
- ASCII Art [PRESENTATION-HIDDEN] — Pattern. Params: scale, color (2)
- Dot Matrix [PRESENTATION-HIDDEN] — Pattern. Params: scale, amount (2)
- Crosshatch [PRESENTATION-HIDDEN] — Pattern. Params: density, amount (2)
- Emboss [PRESENTATION-HIDDEN] — Pattern. Params: amount, angle (2)
- Oil Paint [PRESENTATION-HIDDEN] — Pattern. Params: radius (1)
- Pencil Sketch [PRESENTATION-HIDDEN] — Pattern. Params: amount, density (2)
- Voronoi Glass [PRESENTATION-HIDDEN] — Pattern. Params: scale, edge (2)
- Cross Stitch [PRESENTATION-HIDDEN] — Pattern. Params: scale, amount (2)
- Night Vision [PRESENTATION-HIDDEN] — Pattern. Params: amount (1)
- Triangulate [PRESENTATION-HIDDEN] — Pattern. Params: size, amount (2)
- Neon Edge [PRESENTATION-HIDDEN] — Pattern. Params: edge, glow, hue, original (4)
- Cartoon Ink [PRESENTATION-HIDDEN] — Pattern. Params: edge width, color steps, ink strength, saturation (4)
- Pop Raster [PRESENTATION-HIDDEN] — Pattern. Params: palette, bands, pattern size, mix (4)
- Brush Strokes [PRESENTATION-HIDDEN] — Pattern. Params: size, angle, flow, amount (4)
- Bump Light [PRESENTATION-HIDDEN] — Pattern. Params: light x, light y, intensity, height (4)
- Monitor Wall [PRESENTATION-HIDDEN] — Pattern. Params: columns, rows, border, glow (4)
- Topographic Lines [PRESENTATION-HIDDEN] — Pattern. Params: amount, levels, thickness, color mode (4)

### Category: Animation (6 effects, 23 params)

- Strobe [ALWAYS-VISIBLE] — Animation. Params: rate, intensity (2)
- Pulse [ALWAYS-VISIBLE] — Animation. Params: amount, speed (2)
- Slit Scan [PRESENTATION-HIDDEN] — Animation. Params: amount, direction (2)
- Transient Flash [ALWAYS-VISIBLE] — Animation (audio). Params: style, intensity, decay (3)
- Point Zoom [ALWAYS-VISIBLE] — Animation (feedback). Params: amount, zoom, rotation, x offset, y offset, decay, hue shift, saturation (8)
- Directional Feedback [ALWAYS-VISIBLE] — Animation (feedback). Params: amount, speed, direction, spread, decay, hue shift (6)

### Category: Time / Temporal (6 effects, 14 params)

- Echo [ALWAYS-VISIBLE] — Time. Temporal (uses u_prev_frame). Params: decay, operator (2)
- Posterize Time [PRESENTATION-HIDDEN] — Time. Temporal (uses u_prev_frame). Params: frame rate, amount (2)
- Freeze [ALWAYS-VISIBLE] — Time. Temporal (uses u_prev_frame). Params: amount (1)
- Screen Split [ALWAYS-VISIBLE] — Time. Uses frame ring buffer (480 frames at 1/4 res). Params: columns, rows, frames per cell, direction (4)
- Frame Stutter [ALWAYS-VISIBLE] — Time. Uses frame ring buffer. Params: depth, stutter (2)
- Channel Delay [PRESENTATION-HIDDEN] — Time. Temporal. Params: red delay, green delay, blue delay (3)

### Category: Audio-Reactive (4 effects, 12 params)

These effects use extended audio uniforms and are Audio-DNA's differentiator.

- Harmonic Displacement [PRESENTATION-HIDDEN] — Audio. Params: amount, smoothing, color mode (3)
- Timbral Mosaic [PRESENTATION-HIDDEN] — Audio. Params: amount, base size, complexity (3)
- Structural Morph [PRESENTATION-HIDDEN] — Audio. Params: intensity, normal style, drop style (3)
- Beat Ripple [PRESENTATION-HIDDEN] — Audio. Params: intensity, decay, count (3)

### Category: Blend / Composite (5 effects, 9 params)

- Double Exposure [PRESENTATION-HIDDEN] — Blend. Params: offset, blend (2)
- Frosted Glass [PRESENTATION-HIDDEN] — Blend. Params: amount, scale (2)
- Prism [PRESENTATION-HIDDEN] — Blend. Params: amount, angle (2)
- Rain on Glass [PRESENTATION-HIDDEN] — Blend. Params: amount, speed (2)
- Hexagonalize [PRESENTATION-HIDDEN] — Blend. Params: scale (1)

### Category: Composite / Cloner (3 effects, 13 params)

- Line Cloner [PRESENTATION-HIDDEN] — Composite. Params: copies, offset x, offset y, scale, rotation (5)
- Radial Cloner [PRESENTATION-HIDDEN] — Composite. Params: copies, radius, rotation, scale (4)
- Cube Scatter [PRESENTATION-HIDDEN] — Composite. Params: grid x, grid y, explode, rotation (4)

**VERIFIED TOTALS: 135 effects, 333 parameters**

Note: The category counts in the FX Browser (per Boris's doc) list slightly different groupings because some effects serve dual purposes. The authoritative grouping is from EffectLibrary.cpp `category` field.

**Parameters:** All 333 effect parameters listed above, plus Dry/Wet per effect, plus Bypass per effect
**Bindings:** All effect parameters are signal-connectable via UniversalParamControl. Bypass can be bound to keyboard/MIDI.
**Programming mode:** FX Browser shows all 135 effects in collapsible category sections with search. Effects Stack in inspector shows all params as signal-connectable sliders.
**Presentation mode:** HIDDEN for effect selection/tuning. Pre-configured effects run automatically. Individual effects can be toggled via bypass bindings.
**Dependencies:** OpenGL 4.1 Core, GLSL 410, ShaderManager, EffectChain (ping-pong FBOs), UniformBridge

---

## 3.2 Effect Chain Architecture

**Level:** PRIMARY
**Current UI:** Clip Inspector section 8.9, Layer Inspector section 9.12, Composition Inspector section 10.8
**Sub-features:**
  - Per-Clip Effect Chain [PRESENTATION-HIDDEN] — Effects applied to individual clip output before keying and layer blend. Each clip can have unlimited effects in ordered chain.
  - Per-Layer Effect Chain [PRESENTATION-HIDDEN] — Effects applied after all clips in a layer are composited, before accumulator blend. Accessed via Layer Inspector.
  - Global Effect Chain [MINIMAL-IN-PRESENTATION] — Effects applied after all layers are composited. Accessed via Composition Inspector. Often the "performance" effects.
  - Ping-Pong FBO Rendering [ALWAYS-VISIBLE] — EffectChain renders by alternating between FBO A and FBO B. Input texture -> Effect 1 -> FBO B -> Effect 2 -> FBO A -> ...
  - Effect Ordering [PROGRAMMING-ONLY] — Drag-reorder effects in the stack. Order determines processing order.
  - Per-Effect Bypass [ALWAYS-VISIBLE] — Toggle [B] button per effect to skip it without removing
  - Per-Effect Dry/Wet [PRESENTATION-HIDDEN] — Mix slider (0-1) blending between original and effected output
  - ISF Shader Import [PROGRAMMING-ONLY] — Menu > Audio-DNA > Import ISF. Parses ISF (Interactive Shader Format) JSON metadata, wraps with compatibility defines, converts to GLSL 410.
**Parameters:** Per effect: Dry/Wet, Bypass. Chain level: order.
**Bindings:** Bypass toggle per effect via keyboard/MIDI
**Programming mode:** Full effect chain editing — add, remove, reorder, configure params, ISF import
**Presentation mode:** MINIMAL — effects run automatically. Bypass toggles via bindings for live control.
**Dependencies:** EffectChain, Effect, UniformBridge, ISFShaderLoader, ping-pong FBOs (effectFBO_A_, effectFBO_B_)

---

## 3.3 Temporal Effects

**Level:** PRIMARY
**Current UI:** FX Browser (Time category) + Effects Stack in inspector
**Sub-features:**
  - Echo / Ghost Trails [ALWAYS-VISIBLE] — Blends current frame with previous frame. Temporal flag = true, uses u_prev_frame texture from per-layer temporal buffer. Params: decay (trail length), operator (fade mode). (2 params)
  - Freeze [ALWAYS-VISIBLE] — Holds current frame, ignoring new input. Temporal. Params: amount. (1 param)
  - Posterize Time / Frame Hold [PRESENTATION-HIDDEN] — Reduces effective frame rate by holding frames. Temporal. Params: frame rate, amount. (2 params)
  - Frame Stutter [ALWAYS-VISIBLE] — Plays back frames from the 480-frame ring buffer with configurable delay. Non-temporal (reads ring buffer). Params: depth, stutter. (2 params)
  - Screen Split [ALWAYS-VISIBLE] — Divides output into grid cells, each showing a different delayed frame from ring buffer. Non-temporal. Params: columns, rows, frames per cell, direction. (4 params)
  - Channel Delay [PRESENTATION-HIDDEN] — Delays individual R/G/B channels by different frame counts. Temporal. Params: red delay, green delay, blue delay. (3 params)
**Parameters:** 14 total (listed above per effect)
**Bindings:** All params signal-connectable. Bypass per effect.
**Programming mode:** Configure temporal effects in effect stack, tune decay/depth params
**Presentation mode:** ALWAYS-VISIBLE for Echo, Freeze, Frame Stutter, Screen Split (core performance tools). Frame Hold and Channel Delay PRESENTATION-HIDDEN.
**Dependencies:** Per-layer temporal buffers (layerTemporalBuffers_ in CompositorEngine), Frame ring buffer (480 frames at 1/4 resolution, ~240MB VRAM at 1080p). Temporal effects require BOTH EffectChain and CompositorEngine render paths.

---

## 3.4 Feedback System (Larsen Loop)

See Domain 4: Composition & Performance > Layer Controls > Layer Feedback for full documentation. Feedback is architecturally a per-layer temporal effect using ping-pong FBOs.

---

## 3.5 LUT Color Grading [GHOST — NO UI]

**Level:** SECONDARY
**Current UI:** NO UI — code only. LUTLoader (src/render/LUTLoader.h, 111 LOC) loads .cube LUT files into GL_TEXTURE_3D. Used by the "Color Grade" effect in the EffectLibrary.
**Sub-features:**
  - LUT File Loading [GHOST — NO UI] — Parses .cube format files (LUT_3D_SIZE N, N^3 RGB entries), uploads to GL_TEXTURE_3D
  - Color Grade Effect [ALWAYS-VISIBLE] — Registered in EffectLibrary as "Color Grade" (category: color, shader: lut_grade). Params: amount (1). The effect shader IS registered and can be applied, but there is no UI to browse/select/load LUT files.
  - LUT Texture Release [GHOST — NO UI] — Static method to release GL textures
**Parameters:** Color Grade effect: amount (1). LUT file selection: no UI.
**Bindings:** Color Grade amount is signal-connectable (standard effect param)
**Programming mode:** Color Grade effect can be dragged from FX Browser and its amount adjusted. But no way to load custom .cube LUT files — only the default/embedded LUT applies.
**Presentation mode:** Same as programming (effect functions, LUT selection does not)
**Dependencies:** OpenGL 4.1 (GL_TEXTURE_3D), .cube file format parser
**Note:** Boris decided INCLUDE in design — needs LUT browser/loader UI.

---

# Domain 7: Output & Recording

## 7.1 Display Output

**Level:** PRIMARY
**Current UI:** Output menu (Menu Bar) + Top Bar Output dropdown
**Sub-features:**
  - Fullscreen Output [ALWAYS-VISIBLE] — Menu > Output > Fullscreen on [display]. Creates borderless fullscreen window on selected display via JUCE DocumentWindow. One entry per connected display. Does NOT use macOS native fullscreen (avoids Space creation).
  - Windowed Output [ALWAYS-VISIBLE] — Menu > Output > Windowed. Floating resizable output window.
  - Output Disabled [PROGRAMMING-ONLY] — Menu > Output > Disabled. No external output window.
  - Test Card [PROGRAMMING-ONLY] — Menu > Output > Test Card. Shows calibration pattern for display alignment.
  - Identify Displays [PROGRAMMING-ONLY] — Menu > Output > Identify Displays. Flashes display names on each connected monitor.
  - Output Resolution [PROGRAMMING-ONLY] — Composition Inspector section 10.9. Dropdown: 1920x1080 / 1280x720 / 2560x1440 / 3840x2160.
  - Escape to Close [ALWAYS-VISIBLE] — Pressing Escape closes the output window.
**Parameters:** Output mode (disabled/fullscreen/windowed), target display index, output resolution
**Bindings:** Output mode selectable via keyboard/MIDI binding
**Programming mode:** Full output configuration — display selection, resolution, test card, identify
**Presentation mode:** ALWAYS-VISIBLE — output window is the primary performance output. Selection via top bar dropdown.
**Dependencies:** JUCE DocumentWindow, OpenGL context sharing (renderer output texture shared with output window)

---

## 7.2 Syphon Output (macOS)

**Level:** SECONDARY
**Current UI:** Enabled via compile flag. When active, publishes GL texture automatically.
**Sub-features:**
  - Syphon Server [ALWAYS-VISIBLE] — Publishes rendered composition as a Syphon server named "Audio-DNA". Zero-copy GPU texture sharing via IOSurface.
  - Server Name [PROGRAMMING-ONLY] — Configurable server name (default: "Audio-DNA"). Visible to Syphon clients.
  - Enable/Disable [PROGRAMMING-ONLY] — Toggle publishing without destroying the server. Atomic flag.
  - Client Discovery [ALWAYS-VISIBLE] — Other applications (MadMapper, VDMX, OBS, Resolume) can discover and receive the texture
**Parameters:** Server name, enabled flag
**Bindings:** Enable/disable toggle could be bound (not currently wired)
**Programming mode:** Configure server name, enable/disable
**Presentation mode:** ALWAYS-VISIBLE — runs automatically when enabled, zero user interaction needed
**Dependencies:** Syphon.framework (macOS only, BSD license, must be in /Library/Frameworks/). Compile flag: `-DAUDIODNA_BUILD_SYPHON=ON`. Uses `__has_include(<Syphon/Syphon.h>)` for compile-time detection; compiles as no-op stub if missing. Obj-C++ (.mm files) — macOS only.

---

## 7.3 Syphon Input [GHOST — DEFERRED]

**Level:** SECONDARY
**Current UI:** NO UI — code exists. SyphonInput class (src/output/SyphonInput.h) wraps SyphonClient. Can list servers, connect to a source, receive textures via IOSurface.
**Sub-features:**
  - Server Discovery [GHOST — DEFERRED] — `listServers()` returns available Syphon servers on the system
  - Connect to Source [GHOST — DEFERRED] — `connectToServer(appName, serverName)` connects to a specific server
  - Texture Receive [GHOST — DEFERRED] — `getLatestTexture()` returns GL texture ID from connected server
  - Disconnect [GHOST — DEFERRED] — Clean disconnection from source
**Parameters:** Source app name, server name
**Bindings:** None
**Programming mode:** Not accessible (no UI)
**Presentation mode:** Not accessible
**Dependencies:** Syphon.framework (macOS only)
**Note:** Boris decided DEFER to future. Code exists but will not be included in current design phase.

---

## 7.4 NDI Output [STUB]

**Level:** UTILITY
**Current UI:** NO UI — stub code only. NdiOutput class (src/output/NdiOutput.h) has interface but all methods are no-ops unless `AUDIODNA_HAS_NDI` is defined.
**Sub-features:**
  - NDI Sender [STUB] — Interface defined: init(sourceName), sendFrame(rgba, width, height), shutdown(). All return false / no-op.
  - Network Discovery [STUB] — Would allow NDI-compatible receivers to find Audio-DNA on the network
**Parameters:** Source name (string)
**Bindings:** None
**Programming mode:** Not accessible
**Presentation mode:** Not accessible
**Dependencies:** NDI SDK (not included, must be installed separately from https://ndi.video/download-ndi-sdk/)
**Note:** Not implemented. Stub only.

---

## 7.5 NDI Input [STUB]

**Level:** UTILITY
**Current UI:** NO UI — stub code only. NdiInput class (src/output/NdiInput.h).
**Sub-features:**
  - Source Discovery [STUB] — `listSources()` returns empty vector
  - Connect to Source [STUB] — `connectToSource()` returns false
  - Frame Receive [STUB] — `getLatestFrame()` returns nullptr
**Parameters:** None functional
**Bindings:** None
**Programming mode:** Not accessible
**Presentation mode:** Not accessible
**Dependencies:** NDI SDK (not included)
**Note:** Not implemented. Stub only.

---

## 7.6 Spout Output [STUB]

**Level:** UTILITY
**Current UI:** NO UI — stub code only. SpoutOutput class (src/output/SpoutOutput.h). Windows equivalent of Syphon. All methods are no-ops.
**Sub-features:**
  - Spout Sender [STUB] — Interface defined but all methods are no-ops. `isInitialized()` always returns false.
**Parameters:** Server name (string)
**Bindings:** None
**Programming mode:** Not accessible
**Presentation mode:** Not accessible
**Dependencies:** Spout2 SDK (Windows, not included). On macOS, this class is a no-op.
**Note:** Not implemented. Stub only.

---

## 7.7 Video Recording

**Level:** PRIMARY
**Current UI:** Output menu (Start/Stop Recording) + Record Panel (Browser tab "Record")
**Sub-features:**
  - Real-Time GL Capture [ALWAYS-VISIBLE] — `VideoRecorder::submitFrame()` does glReadPixels from GL framebuffer into rotating CPU-side triple buffer. Zero mutex on GL thread.
  - Triple-Buffered Readback [ALWAYS-VISIBLE] — 3 CPU pixel buffers with atomic index rotation. GL thread writes to one buffer while encoder thread reads another. Frame drops if encoder can't keep up (counted, not prevented).
  - Codec Selection [PROGRAMMING-ONLY] — H.264 (libx264, default, best compatibility), ProRes (prores_ks, best quality for editing), MJPEG (fast encoding, large files)
  - Recording Configuration [PROGRAMMING-ONLY] — Width, height, FPS (default 30), quality (CRF 0-51 for H264, 0-5 for ProRes), container (mp4/mov, auto-selected by codec)
  - Start/Stop Controls [ALWAYS-VISIBLE] — Menu > Output > Start/Stop Recording. Also available in Record Panel.
  - Frame Drop Counting [ALWAYS-VISIBLE] — Tracks dropped frames when encoder can't keep up with render rate. Displayed in Record Panel status.
  - Video Only [ALWAYS-VISIBLE] — Records video only, no audio track. Audio capture is handled separately by Session Recording.
**Parameters:** Codec (H264/ProRes/MJPEG), width, height, FPS, quality/CRF, output directory
**Bindings:** Start/Stop recording via keyboard/MIDI binding
**Programming mode:** Configure codec, resolution, quality before recording. Browse output folder.
**Presentation mode:** MINIMAL — Start/Stop via binding or menu. Status visible in Record Panel.
**Dependencies:** FFmpeg 8.0 (libavformat, libavcodec, libavutil, libswscale). Encoder thread (separate from GL and analysis threads).
**Storage:** ~/Documents/Audio-DNA/Recordings/

---

## 7.8 Session Recording

**Level:** SECONDARY
**Current UI:** Record Panel (Browser tab "Record")
**Sub-features:**
  - Event Capture [ALWAYS-VISIBLE] — Records timestamped events during performance: ParameterChange, ClipTrigger, ColumnTrigger, MacroChange, TransportChange, EffectToggle, CuepointJump. Thread-safe via mutex.
  - Session Playback [PROGRAMMING-ONLY] — Replays recorded events at their timestamps to recreate the entire performance. `advancePlayback(dt)` returns events to fire each frame.
  - Start/Stop Recording [ALWAYS-VISIBLE] — Record button in Record Panel. Atomic flag for state.
  - Save/Load JSON [PROGRAMMING-ONLY] — Export/import session as JSON file. Human-readable format.
  - Event Count Display [ALWAYS-VISIBLE] — Status showing recording state and event count
  - Duration Tracking [ALWAYS-VISIBLE] — Tracks recording duration from start time
**Parameters:** None (records all events automatically when active)
**Bindings:** Start/Stop session recording via keyboard/MIDI binding
**Programming mode:** Full Record Panel: start, stop, playback, save, load, browse output folder
**Presentation mode:** MINIMAL — Start/Stop via binding. Event count visible.
**Dependencies:** JUCE CriticalSection (thread safety), JUCE File (JSON save/load)
**Storage:** JSON files in user-chosen directory

---

## 7.9 Snapshots

**Level:** UTILITY
**Current UI:** Output menu (Snapshot) + keyboard shortcut
**Sub-features:**
  - PNG Capture [ALWAYS-VISIBLE] — One-click capture of current rendered output to PNG file. `Renderer::takeSnapshot()` reads GL framebuffer and saves to configured directory.
  - Configurable Directory [PROGRAMMING-ONLY] — `setSnapshotDir()` sets save location
  - Capture Callback [ALWAYS-VISIBLE] — `onSnapshotTaken` callback fires with file path after save
**Parameters:** Snapshot directory path
**Bindings:** Snapshot trigger via keyboard/MIDI binding
**Programming mode:** Configure snapshot directory
**Presentation mode:** ALWAYS-VISIBLE — one-press capture via binding
**Dependencies:** OpenGL (glReadPixels), JUCE Image/PNG writer
**Storage:** ~/Documents/Audio-DNA/Snapshots/ (default)

---

# Totals Summary (Domains 2, 3, 7)

| Domain | Items | Parameters |
|--------|-------|------------|
| Procedural Sources | 108 sources | 628 params |
| MilkDrop/projectM | ~9,800 presets, 3 modes | ~8 config params |
| Video Playback | 1 system | ~15 transport params |
| Image Sequences | 1 system | ~12 transport params |
| Media Management | 2 menu actions | 0 |
| Camera Input [GHOST — NO UI] | 1 system | 1 (model only) |
| Visual Effects | 135 effects | 333 params |
| Effect Chain Architecture | 3 levels | per-effect: bypass + dry/wet |
| Temporal Effects | 6 effects (subset of 135) | 14 params |
| Feedback System | 6 presets | 7 params per layer |
| LUT Color Grading [GHOST — NO UI] | 1 loader + 1 effect | 1 param (effect) |
| Display Output | 5 output modes | ~4 config params |
| Syphon Output | 1 server | 2 params |
| Syphon Input [GHOST — DEFERRED] | 1 client | 2 params |
| NDI Output [STUB] | 1 stub | 1 param |
| NDI Input [STUB] | 1 stub | 0 |
| Spout Output [STUB] | 1 stub | 1 param |
| Video Recording | 3 codecs | ~6 config params |
| Session Recording | 7 event types | 0 (auto-captures) |
| Snapshots | 1 capture system | 1 param (directory) |

---

## Domain 4: Composition & Performance

---

### 4.1 Deck/Layer/Clip Hierarchy

**Level:** PRIMARY
**Current UI:** Deck Grid (center area), Deck Tabs (bottom of grid), Layer Strips (left 250px), Clip Cells (90x96px grid)
**Sub-features:**
  - Composition container [ALWAYS-VISIBLE] — top-level entity owning all decks, global effects, global settings, genre-deck assignments, composition transform
  - Deck management (N decks) [ALWAYS-VISIBLE] — New / Insert Before / Insert After / Duplicate / Rename / Close / Clear Clips / Remove via Deck menu; tabs along bottom of grid for switching
  - Layer management (M layers per deck) [ALWAYS-VISIBLE] — New / Insert Above / Insert Below / Duplicate / Rename / Copy Effects / Paste Effects / Clear Clips / Remove / Ignore Column Trigger / Lock Content / Fold / Move Up / Move Down via Layer menu
  - Column management (K columns per deck) [ALWAYS-VISIBLE] — New / Insert Before / Insert After / Duplicate / Clear Clips / Remove / Remove All Before / Remove All After via Column menu
  - Clip management [ALWAYS-VISIBLE] — Select All / Cut / Copy / Paste / Copy Effects / Paste Effects / Rename / Clear / Show in Finder / New Source / New Effect / Replace Content / Lock Content via Clip menu
  - Column trigger buttons [ALWAYS-VISIBLE] — numbered 1..K across top of grid, click fires all clips in column simultaneously (respects per-layer Ignore Column Trigger flag)
  - Deck tabs [ALWAYS-VISIBLE] — one tab per deck, click to switch active deck; active deck highlighted
  - Active deck rendering [ALWAYS-VISIBLE] — only active deck renders; persistent layers from non-active decks also composite
  - Default grid: 3 layers x 12 columns per deck
**Parameters:**
  - Composition.activeDeckIndex (int, 0..N-1)
  - Deck.numColumns (int, default 12)
  - Deck.layers.size() (int, default 3)
**Bindings:** Switch Deck (keyboard/MIDI), Trigger Column (keyboard/MIDI)
**Programming mode:** Full grid visible with all controls
**Presentation mode:** VESTIGIAL — current ProgrammingMode only toggles signal bar expansion, hides all panels. No graduated presentation mode exists.
**Dependencies:** Render Pipeline (compositing order), Autopilot (clip advancement), Undo/Redo (all structural changes)

---

### 4.2 Layer Types

**Level:** PRIMARY
**Current UI:** Layer Inspector > Video section (implicit via layer type) and Layer Strip
**Sub-features:**
  - Opaque [ALWAYS-VISIBLE] — one clip at a time, replaces everything below. Default for first layer in deck.
  - Transparent [ALWAYS-VISIBLE] — composited over layers below with blend mode + keying. Default for layers 2+ in deck.
  - FXOnly [ALWAYS-VISIBLE] — no media; applies effects to the composited accumulator. Has dedicated Dry/Wet control.
  - ThreeD [ALWAYS-VISIBLE] — 3D model/surface rendering with rotation X/Y/Z, rotation speed, and scale controls.
  - Mask [ALWAYS-VISIBLE] — content used as luminance alpha mask for layers below.
**Parameters:**
  - Layer.type (enum: Opaque=0, Transparent=1, FXOnly=2, ThreeD=3, Mask=4)
  - FXOnly: Layer.dryWetMix (float, 0-1, default 1.0)
  - ThreeD: Layer.rotationX/Y/Z (float, degrees), Layer.rotationSpeed (float), Layer.scale3D (float, default 1.0)
**Bindings:** None directly (type set via inspector)
**Programming mode:** Full type selection and type-specific controls visible
**Presentation mode:** HIDDEN — type is set during programming, not changed during performance
**Dependencies:** Render Pipeline (compositing behavior differs by type), Keying (Transparent only), Effect Chain Architecture (FXOnly applies to accumulator)

---

### 4.3 Layer Controls

**Level:** PRIMARY
**Current UI:** Layer Strip (250px, left side of Deck Grid) + Layer Inspector

#### 4.3.1 Blend Modes (25)

**Level:** PRIMARY
**Current UI:** Layer Strip V dropdown, Layer Inspector > Video > Blend Mode dropdown
**Sub-features:**
  - Normal [ALWAYS-VISIBLE] — standard alpha compositing
  - Additive [ALWAYS-VISIBLE] — pixel values summed (default blend mode)
  - Screen [ALWAYS-VISIBLE] — inverted multiply
  - Multiply [ALWAYS-VISIBLE] — darkening blend
  - Overlay [ALWAYS-VISIBLE] — combines Multiply and Screen
  - SoftLight [ALWAYS-VISIBLE] — gentle contrast adjustment
  - HardLight [ALWAYS-VISIBLE] — strong contrast adjustment
  - VividLight [ALWAYS-VISIBLE] — extreme burn/dodge combo
  - LinearLight [ALWAYS-VISIBLE] — linear burn/dodge combo
  - PinLight [ALWAYS-VISIBLE] — conditional replacement
  - HardMix [ALWAYS-VISIBLE] — threshold posterization
  - Darken [ALWAYS-VISIBLE] — keep darker pixel
  - Lighten [ALWAYS-VISIBLE] — keep lighter pixel
  - DarkerColor [ALWAYS-VISIBLE] — keep darker color (luminance-based)
  - LighterColor [ALWAYS-VISIBLE] — keep lighter color (luminance-based)
  - ColorDodge [ALWAYS-VISIBLE] — brighten by dividing
  - ColorBurn [ALWAYS-VISIBLE] — darken by dividing
  - Difference [ALWAYS-VISIBLE] — absolute pixel difference
  - Exclusion [ALWAYS-VISIBLE] — lower-contrast version of Difference
  - Subtract [ALWAYS-VISIBLE] — subtract pixel values
  - Hue [ALWAYS-VISIBLE] — hue from layer, saturation+luminosity from below
  - Saturation [ALWAYS-VISIBLE] — saturation from layer, hue+luminosity from below
  - Color [ALWAYS-VISIBLE] — hue+saturation from layer, luminosity from below
  - Luminosity [ALWAYS-VISIBLE] — luminosity from layer, hue+saturation from below
  - Dissolve [ALWAYS-VISIBLE] — random pixel selection (dither pattern)
**Parameters:**
  - Layer.blendMode (MixMode enum, 25 blend values: Normal through Dissolve)
**Bindings:** Not directly bindable (dropdown selection)
**Programming mode:** Full dropdown with organized sections (Basic, Light, Dark/Light Compare, Dodge/Burn, Inversion, Component/HSL, Special)
**Presentation mode:** HIDDEN — set during programming
**Dependencies:** Render Pipeline (CompositorEngine blend shader)

#### 4.3.2 Transition Modes (30)

**Level:** PRIMARY
**Current UI:** Layer Strip F dropdown, Layer Inspector > Transition > Blend Mode dropdown
**Sub-features:**
  - Cut [ALWAYS-VISIBLE] — instant switch, no transition
  - WipeLeft [ALWAYS-VISIBLE] — horizontal wipe from right to left
  - WipeRight [ALWAYS-VISIBLE] — horizontal wipe from left to right
  - WipeUp [ALWAYS-VISIBLE] — vertical wipe upward
  - WipeDown [ALWAYS-VISIBLE] — vertical wipe downward
  - WipeEllipse [ALWAYS-VISIBLE] — elliptical/iris wipe
  - WipeDiagonal [ALWAYS-VISIBLE] — diagonal wipe
  - PushLeft [ALWAYS-VISIBLE] — content slides left, new enters from right
  - PushRight [ALWAYS-VISIBLE] — content slides right, new enters from left
  - PushUp [ALWAYS-VISIBLE] — content slides up, new enters from bottom
  - PushDown [ALWAYS-VISIBLE] — content slides down, new enters from top
  - ZoomIn [ALWAYS-VISIBLE] — new clip zooms in from center
  - ZoomOut [ALWAYS-VISIBLE] — new clip zooms out
  - RotateX [ALWAYS-VISIBLE] — 3D rotation around X axis
  - RotateY [ALWAYS-VISIBLE] — 3D rotation around Y axis
  - Spin [ALWAYS-VISIBLE] — 2D spin transition
  - Cube [ALWAYS-VISIBLE] — 3D cube rotation
  - Flip [ALWAYS-VISIBLE] — 3D flip card
  - Fold [ALWAYS-VISIBLE] — 3D folding paper
  - ToBlack [ALWAYS-VISIBLE] — fade through black
  - ToWhite [ALWAYS-VISIBLE] — fade through white
  - Pixelate [ALWAYS-VISIBLE] — pixelation transition
  - Blur [ALWAYS-VISIBLE] — blur transition
  - Noise [ALWAYS-VISIBLE] — noise dissolve
  - RGBSplit [ALWAYS-VISIBLE] — RGB channel offset transition
  - GlitchBlocks [ALWAYS-VISIBLE] — block glitch effect
  - Strobe [ALWAYS-VISIBLE] — strobe flash transition
  - Slide [ALWAYS-VISIBLE] — slide transition
  - Stretch [ALWAYS-VISIBLE] — stretch transition
  - Displace [ALWAYS-VISIBLE] — displacement map transition
**Parameters:**
  - Layer.transitionMode (MixMode enum, 30 transition values: Cut through Displace)
  - Layer.transitionBlendMode (MixMode, default Normal)
  - Layer.transitionSpeed (float, -1 = use global default, range 0.1-10s)
**Bindings:** Not directly bindable
**Programming mode:** Full dropdown with organized sections (Instant, Directional Wipes, Push, Zoom, 3D Rotation, Fade Through Color, Creative/VJ)
**Presentation mode:** HIDDEN — transitions are pre-configured, fire automatically on clip change
**Dependencies:** Render Pipeline (transitionFBO, crossfadeProgress per-layer)

#### 4.3.3 Keying Modes (13)

**Level:** PRIMARY
**Current UI:** Layer Inspector > Keying section (visible only for Transparent layer type)
**Sub-features:**
  - Alpha [ALWAYS-VISIBLE] — standard alpha channel transparency (default)
  - LumaKey [ALWAYS-VISIBLE] — dark areas become transparent based on luminance threshold
  - InvertedLumaKey [ALWAYS-VISIBLE] — light areas become transparent
  - LumaIsAlpha [ALWAYS-VISIBLE] — luminance value directly becomes alpha
  - InvertedLumaIsAlpha [ALWAYS-VISIBLE] — inverted luminance becomes alpha
  - ChromaKey [ALWAYS-VISIBLE] — remove specific color (green screen). Has dedicated color picker (R/G/B) and tolerance.
  - MaxRGB [ALWAYS-VISIBLE] — maximum of R/G/B channels used as key
  - SaturationKey [ALWAYS-VISIBLE] — low-saturation areas become transparent
  - EdgeDetection [ALWAYS-VISIBLE] — edge pixels visible, fill transparent
  - ThresholdMask [ALWAYS-VISIBLE] — hard binary threshold
  - ChannelR [ALWAYS-VISIBLE] — red channel as transparency
  - ChannelG [ALWAYS-VISIBLE] — green channel as transparency
  - ChannelB [ALWAYS-VISIBLE] — blue channel as transparency
**Parameters:**
  - Layer.keyingMode (KeyingMode enum, 13 values)
  - Layer.keyThreshold (float, 0-1, default 0.1)
  - Layer.keySoftness (float, 0-1, default 0.1)
  - Layer.chromaKeyR/G/B (float, 0-1, default green: 0,1,0)
  - Layer.chromaKeyTolerance (float, 0-1, default 0.2)
**Bindings:** keyThreshold and keySoftness are signal-connectable via Universal Parameter Controls
**Programming mode:** Full keying controls visible in inspector
**Presentation mode:** HIDDEN — keying is configured during setup
**Dependencies:** Render Pipeline (scratchFBO for keying shader), Layer Types (only applies to Transparent type)

#### 4.3.4 Layer Transform

**Level:** SECONDARY
**Current UI:** Layer Inspector > Transform section
**Sub-features:**
  - Position X/Y [PRESENTATION-HIDDEN] — pixel offset from center, signal-connectable
  - Scale [PRESENTATION-HIDDEN] — size multiplier (1.0 = 100%), signal-connectable
  - Rotation [PRESENTATION-HIDDEN] — degrees, signal-connectable
  - Anchor X/Y [PROGRAMMING-ONLY] — pivot point offset from center
**Parameters:**
  - Layer.positionX/Y (float, pixels)
  - Layer.layerScale (float, default 1.0)
  - Layer.layerRotation (float, degrees)
  - Layer.layerAnchorX/Y (float)
**Bindings:** All transform params are signal-connectable via Universal Parameter Controls
**Programming mode:** Full sliders with signal connect triangles
**Presentation mode:** HIDDEN — configured during programming, driven by audio mappings in performance
**Dependencies:** Render Pipeline (applied after clip compositing, before accumulator blend)

#### 4.3.5 Layer Feedback (Larsen Loop)

**Level:** SECONDARY
**Current UI:** Layer Inspector > Feedback section
**Sub-features:**
  - Enable toggle [PRESENTATION-HIDDEN] — turn feedback on/off
  - Preset dropdown [PROGRAMMING-ONLY] — Zoom In / Spiral / Drift / Kaleidoscope / Echo / Stretch (6 presets)
    - Zoom In — amount: 0.85, scaleX: 0.97, scaleY: 0.97, rotation: 0, offsetX: 0, offsetY: 0, lumaKey: 0
    - Spiral — amount: 0.80, scaleX: 0.98, scaleY: 0.98, rotation: 2.0, offsetX: 0, offsetY: 0, lumaKey: 0
    - Drift — amount: 0.75, scaleX: 1.0, scaleY: 1.0, rotation: 0, offsetX: 0.01, offsetY: 0, lumaKey: 0
    - Kaleidoscope — amount: 0.90, scaleX: 0.95, scaleY: 0.95, rotation: 5.0, offsetX: 0, offsetY: 0, lumaKey: 0
    - Echo — amount: 0.60, scaleX: 1.0, scaleY: 1.0, rotation: 0, offsetX: 0, offsetY: 0, lumaKey: 0
    - Stretch — amount: 0.85, scaleX: 1.02, scaleY: 0.98, rotation: 0, offsetX: 0, offsetY: 0, lumaKey: 0
  - Amount slider [PRESENTATION-HIDDEN] — how much previous frame bleeds through (0-1, default 0.5)
  - Scale X/Y [PRESENTATION-HIDDEN] — per-frame zoom (default 0.98 each)
  - Rotation [PRESENTATION-HIDDEN] — per-frame rotation in degrees
  - Offset X/Y [PRESENTATION-HIDDEN] — per-frame horizontal/vertical drift (-0.5 to 0.5)
  - Luma Key [PRESENTATION-HIDDEN] — fade dark areas from feedback to prevent muddiness (0-1)
**Parameters:**
  - FeedbackConfig.enabled (bool)
  - FeedbackConfig.presetName (string)
  - FeedbackConfig.amount (float, 0-1)
  - FeedbackConfig.scaleX/Y (float, default 0.98)
  - FeedbackConfig.rotation (float, degrees)
  - FeedbackConfig.offsetX/Y (float, -0.5 to 0.5)
  - FeedbackConfig.lumaKey (float, 0-1)
**Bindings:** All feedback params signal-connectable
**Programming mode:** Full feedback controls visible
**Presentation mode:** HIDDEN — configured in programming, feedback runs automatically
**Dependencies:** Render Pipeline (FeedbackProcessor, per-layer feedback FBOs)

#### 4.3.6 Layer State Controls

**Level:** PRIMARY
**Current UI:** Layer Strip (action buttons row 1)
**Sub-features:**
  - Opacity (V slider) [ALWAYS-VISIBLE] — layer visibility (0-1, default 1.0), signal-connectable
  - Visible toggle [ALWAYS-VISIBLE] — show/hide layer
  - Bypass (B button) [ALWAYS-VISIBLE] — skip layer during rendering (dark red when active)
  - Solo (S button) [ALWAYS-VISIBLE] — render only this layer (olive when active)
  - Mute [PRESENTATION-HIDDEN] — audio mute for layer
  - Clear (X button) [ALWAYS-VISIBLE] — clear/stop active clip on this layer
  - Persistent toggle [PROGRAMMING-ONLY] — keep rendering when deck is not active
  - Ignore Column Trigger [PROGRAMMING-ONLY] — layer won't respond to column triggers
  - Fold [MINIMAL-IN-PRESENTATION] — collapse layer row to save space
  - Content Lock [PROGRAMMING-ONLY] — prevent media replacement
**Parameters:**
  - Layer.opacity (float, 0-1)
  - Layer.visible (bool)
  - Layer.bypassed (bool)
  - Layer.solo (bool)
  - Layer.muted (bool)
  - Layer.persistent (bool)
  - Layer.ignoreColumnTrigger (bool)
  - Layer.folded (bool)
**Bindings:** Toggle Layer Bypass/Solo/Mute/Visible (keyboard/MIDI), Adjust Layer Opacity (MIDI CC)
**Programming mode:** All controls visible on layer strip
**Presentation mode:** MINIMAL — Bypass/Solo/Clear buttons visible; Opacity via MIDI/signal
**Dependencies:** Render Pipeline (bypassed/solo affect compositing), Deck (column trigger respects ignoreColumnTrigger)

#### 4.3.7 Layer Transport

**Level:** PRIMARY
**Current UI:** Layer Strip (row 1 transport buttons)
**Sub-features:**
  - Play backward (<) [ALWAYS-VISIBLE] — set reverse playback
  - Pause (||) [ALWAYS-VISIBLE] — pause playback
  - Play forward (>) [ALWAYS-VISIBLE] — play forward
  - Fast forward (>|) [ALWAYS-VISIBLE] — 2x speed
  - Speed slider (S, 0-4x) [ALWAYS-VISIBLE] — playback speed multiplier (cyan colored)
  - Fade slider (F, 0-4s) [ALWAYS-VISIBLE] — transition speed between clips (blue colored)
**Parameters:**
  - Layer strip speed (float, 0-4x, default 1x)
  - Layer strip fade (float, 0-4s, default 0.3s)
**Bindings:** Layer Transport Play/Pause/Reverse (keyboard/MIDI)
**Programming mode:** Full transport buttons and sliders visible
**Presentation mode:** MINIMAL — buttons visible on layer strip
**Dependencies:** Clip Transport (per-clip transport inherits/overrides), BPM Tracker (speed synced)

#### 4.3.8 Layer Effects

**Level:** PRIMARY
**Current UI:** Layer Inspector > Layer Effects section
**Sub-features:**
  - Layer effect stack [PRESENTATION-HIDDEN] — same UI pattern as clip effects (vertical list with bypass, expand, dry/wet per effect)
  - Applied after clip compositing, before accumulator blend
**Parameters:** Per-effect: EffectSlot.effectName, EffectSlot.paramValues[], EffectSlot.dryWet, EffectSlot.enabled, EffectSlot.bypassed
**Bindings:** Toggle Effect Bypass (keyboard/MIDI), all effect params signal-connectable
**Programming mode:** Full effects stack with all controls
**Presentation mode:** HIDDEN — effects run automatically; controlled via mappings/macros
**Dependencies:** Effect Chain Architecture (per-layer scope), Render Pipeline (applied in layer compositing)

---

### 4.4 Clip Controls

**Level:** PRIMARY
**Current UI:** Clip Cell (90x96px) + Clip Inspector

#### 4.4.1 Clip Transport

**Level:** PRIMARY
**Current UI:** Clip Inspector > Transport section
**Sub-features:**
  - Transport mode dropdown [PROGRAMMING-ONLY] — Timeline or BPM Sync
  - Timeline bar [PROGRAMMING-ONLY] — draggable in/out points + playhead; beat markers in BPM Sync mode
  - Play backward / Pause / Play forward [ALWAYS-VISIBLE] — playback direction
  - Loop mode [PROGRAMMING-ONLY] — Loop / Ping Pong / One Shot
  - Trigger mode [PROGRAMMING-ONLY] — Restart / Continue / Relative
  - Speed slider (0-4x) [ALWAYS-VISIBLE] — playback rate with /2 and x2 buttons and reverse toggle
  - Duration slider [PROGRAMMING-ONLY] — playback length (0.1-300s in Timeline, 1-64 beats in BPM Sync)
  - Beats/Cycle dropdown [PROGRAMMING-ONLY] — BPM Sync only: 1/4 to 16 beats per playback cycle
  - Content Beats slider [PROGRAMMING-ONLY] — BPM Sync only: how many beats the content contains
  - Images/Sec slider [PROGRAMMING-ONLY] — image sequence only: playback FPS
  - In/Out points [PROGRAMMING-ONLY] — normalized [0,1] start/end positions, draggable on timeline
**Parameters:**
  - Clip.transportMode (enum: Timeline=0, BPMSync=1)
  - Clip.loopMode (enum: Loop=0, PingPong=1, OneShot=2)
  - Clip.speed (float, 0-4x, default 1.0)
  - Clip.reverse (bool)
  - Clip.inPoint/outPoint (float, 0-1)
  - Clip.beatDivision (float, default 4.0)
  - Clip.videoBeats (float, default 4.0)
  - Clip.sequenceFps (float, default 2.5)
**Bindings:** Trigger Clip (keyboard/MIDI with optional velocity-to-opacity)
**Programming mode:** Full transport controls visible
**Presentation mode:** HIDDEN — transport pre-configured; clips triggered via grid/bindings
**Dependencies:** BPM Tracker (beat-synced transport), Layer Transport (inherits speed/direction)

#### 4.4.2 Clip BPM Sync

**Level:** SECONDARY
**Current UI:** Clip Inspector > Transport section (when mode = BPM Sync)
**Sub-features:**
  - Beat Division [PROGRAMMING-ONLY] — how many beats to play content over (1/4 to 16)
  - Content Beats [PROGRAMMING-ONLY] — how many beats the source content contains
  - Beat markers on timeline [PROGRAMMING-ONLY] — visual beat grid overlay
**Parameters:**
  - Clip.beatDivision (float, default 4.0)
  - Clip.videoBeats (float, default 4.0)
**Bindings:** Not directly bindable
**Programming mode:** Visible when Transport Mode = BPM Sync
**Presentation mode:** HIDDEN
**Dependencies:** BPM Tracker (provides BPM for sync calculation)

#### 4.4.3 Clip Beat Snap

**Level:** SECONDARY
**Current UI:** Clip Inspector > Autopilot section > Beat Snap dropdown
**Sub-features:**
  - Snap Off [ALWAYS-VISIBLE] — trigger immediately on click
  - Snap to Beat [ALWAYS-VISIBLE] — queue trigger until next beat
  - Snap to Bar [ALWAYS-VISIBLE] — queue trigger until next bar (4 beats)
  - Snap to 2 Bar [ALWAYS-VISIBLE] — queue trigger until 2-bar boundary (8 beats)
  - Snap to 4 Bar [ALWAYS-VISIBLE] — queue trigger until 4-bar boundary (16 beats)
**Parameters:**
  - Clip.beatSnapMode (enum: Off=0, Beat=1, Bar=2, TwoBar=3, FourBar=4)
**Bindings:** Not directly bindable (set per-clip in inspector)
**Programming mode:** Dropdown visible in Autopilot section
**Presentation mode:** HIDDEN — snap behavior runs automatically when clips are triggered
**Dependencies:** BPM Tracker (beat/bar phase for snap timing), Layer.processPendingTrigger()

#### 4.4.4 Cuepoints

**Level:** SECONDARY
**Current UI:** Clip Inspector > Cuepoints section
**Sub-features:**
  - 8 trigger buttons [ALWAYS-VISIBLE] — numbered 1-8, click to jump to cuepoint position
  - 8 set buttons [PROGRAMMING-ONLY] — click to set cuepoint at current playhead
  - Right-click/Ctrl-click to clear [PROGRAMMING-ONLY]
**Parameters:**
  - Clip.cuepoints[8] (float array, normalized [0,1] positions)
  - Clip.numCuepoints (int, 0-8)
**Bindings:** Not directly bindable (cuepoint jump could be mapped via REST API)
**Programming mode:** Full cuepoint grid with set/clear/trigger
**Presentation mode:** MINIMAL — trigger buttons accessible, set/clear hidden
**Dependencies:** Clip Transport (playhead jumps to cuepoint position)

#### 4.4.5 Clip Transform

**Level:** SECONDARY
**Current UI:** Clip Inspector > Transform section
**Sub-features:**
  - Position X/Y [PRESENTATION-HIDDEN] — pixel offset, signal-connectable
  - Scale [PRESENTATION-HIDDEN] — size multiplier (1.0 = 100%), signal-connectable
  - Rotation [PRESENTATION-HIDDEN] — degrees, signal-connectable
  - Anchor X/Y [PROGRAMMING-ONLY] — pivot point offset
**Parameters:**
  - Clip.positionX/Y (float, pixels)
  - Clip.scale (float, default 1.0)
  - Clip.rotation (float, degrees)
  - Clip.anchorX/Y (float)
**Bindings:** All signal-connectable via Universal Parameter Controls
**Programming mode:** Full sliders visible
**Presentation mode:** HIDDEN — driven by audio mappings
**Dependencies:** Render Pipeline (applied before layer compositing)

#### 4.4.6 Clip Video Properties

**Level:** SECONDARY
**Current UI:** Clip Inspector > Video section
**Sub-features:**
  - Opacity slider [ALWAYS-VISIBLE] — per-clip transparency (0-1), signal-connectable
  - Width / Height [PROGRAMMING-ONLY] — pixel dimensions with inc/dec buttons
  - Blend Mode Override dropdown [PROGRAMMING-ONLY] — Layer Determined or Override (Normal/Additive/Screen/Multiply)
  - Alpha Type dropdown [PROGRAMMING-ONLY] — Premultiplied / Straight
  - R G B A toggles [PROGRAMMING-ONLY] — channel visibility toggles
**Parameters:**
  - Clip.clipOpacity (float, 0-1, default 1.0)
  - Clip.clipWidth/clipHeight (int, default 1920x1080)
  - Clip.blendOverride (enum: LayerDetermined=0, Override=1)
  - Clip.alphaType (enum: Premultiplied=0, Straight=1)
  - Clip.channelR/G/B/A (bool, all default true)
**Bindings:** Opacity signal-connectable, MIDI velocity maps to opacity on trigger
**Programming mode:** Full controls visible
**Presentation mode:** HIDDEN except opacity
**Dependencies:** Layer blend mode (overridden when blendOverride = Override)

#### 4.4.7 Clip Effects Stack

**Level:** PRIMARY
**Current UI:** Clip Inspector > Effects Stack section
**Sub-features:**
  - Effect list [PRESENTATION-HIDDEN] — vertical list of applied effects
  - Per-effect bypass toggle (B) [PRESENTATION-HIDDEN] — enable/disable individual effect
  - Per-effect dry/wet slider [PRESENTATION-HIDDEN] — blend original with effected (0-1)
  - Per-effect parameters [PRESENTATION-HIDDEN] — signal-connectable sliders, right-click to reset
  - Drag-drop from FX Browser [PROGRAMMING-ONLY] — add effect to chain
  - Effect ordering [PROGRAMMING-ONLY] — drag to reorder within stack
**Parameters:** Per-effect: EffectSlot.effectName, EffectSlot.paramValues[], EffectSlot.dryWet, EffectSlot.enabled, EffectSlot.bypassed
**Bindings:** Toggle Effect Bypass (keyboard/MIDI), all params signal-connectable
**Programming mode:** Full stack with all controls, add/remove/reorder
**Presentation mode:** HIDDEN — effects run automatically; controlled via mappings/macros
**Dependencies:** Effect Chain Architecture (per-clip scope, applied before keying/blend), FX Browser (source for adding effects)

---

### 4.5 Autopilot System

**Level:** PRIMARY
**Current UI:** Clip Inspector > Autopilot, Layer Inspector > Autopilot, Composition Inspector > Autopilot + Per-Type Autopilot

#### 4.5.1 Per-Clip Autopilot

**Level:** PRIMARY
**Current UI:** Clip Inspector > Autopilot section
**Sub-features:**
  - Action dropdown [PROGRAMMING-ONLY] — determines what happens when duration expires
    - LayerDetermined (default) — inherit from layer settings
    - DoNothing — no auto-advance
    - PlayNext — advance to next clip in layer
    - PlayPrevious — go to previous clip
    - PlayRandom — random clip selection
    - PlayFirst — jump to first clip
    - PlayLast — jump to last clip
    - PlaySpecific — jump to a specific column
  - Duration dropdown [PROGRAMMING-ONLY] — how long before advancing
    - LayerDetermined (default) — inherit from layer
    - 1/4, 1/2, 1, 2, 4, 8, 16, 32 beats
    - Custom (integer beat count)
  - Beat Snap dropdown [PROGRAMMING-ONLY] — timing quantization for clip launch (Off / Beat / Bar / 2 Bar / 4 Bar)
**Parameters:**
  - Clip.autopilotAction (enum, 8 values)
  - Clip.autopilotSpecificCol (int, for PlaySpecific)
  - Clip.autopilotDuration (enum, 9 values including LayerDetermined and Custom)
  - Clip.autopilotCustomBeats (int, default 4)
  - Clip.beatSnapMode (enum, 5 values)
**Bindings:** Not directly bindable
**Programming mode:** Full controls visible in Clip Inspector
**Presentation mode:** HIDDEN — runs automatically
**Dependencies:** BPM Tracker (beat timing), Layer Autopilot (LayerDetermined fallback)

#### 4.5.2 Per-Layer Autopilot

**Level:** PRIMARY
**Current UI:** Layer Inspector > Autopilot section
**Sub-features:**
  - Direction buttons [PROGRAMMING-ONLY] — Rewind / Off / Forward / Random
  - Trigger mode [PROGRAMMING-ONLY] — End of Video or On Beat
  - Beat count [PROGRAMMING-ONLY] — 1/2/4/8/16/32 beats (On Beat mode only)
  - Loops slider (1-99) [PROGRAMMING-ONLY] — number of clip loops before advancing
  - Autopilot enable toggle [ALWAYS-VISIBLE] — master on/off per layer
**Parameters:**
  - Layer.autopilotEnabled (bool)
  - Layer.defaultAutopilotAction (AutopilotAction enum)
  - Layer.defaultAutopilotDuration (AutopilotDuration enum)
  - Layer.defaultAutopilotCustomBeats (int, default 4)
  - Layer.autopilotLoops (int, default 1)
  - Layer.autopilotEndOfVideo (bool)
**Bindings:** Toggle Layer Autopilot (keyboard/MIDI)
**Programming mode:** Full autopilot controls visible
**Presentation mode:** HIDDEN — autopilot state controlled via binding toggle
**Dependencies:** BPM Tracker, Clip Autopilot (per-clip overrides layer defaults)

#### 4.5.3 Per-Composition Autopilot

**Level:** SECONDARY
**Current UI:** Composition Inspector > Autopilot section
**Sub-features:**
  - Direction buttons [PROGRAMMING-ONLY] — Rewind / Off / Forward / Random
  - Duration mode [PROGRAMMING-ONLY] — Longest Clip / Clip Transport / Custom
  - Clip Loops slider (1-99) [PROGRAMMING-ONLY] — loops before advancing
  - Loop toggle [PROGRAMMING-ONLY] — restart sequence when complete
  - Master Layer dropdown [PROGRAMMING-ONLY] — which layer drives autopilot timing
**Parameters:**
  - Composition.autopilotDirection (enum: Rewind=0, Off=1, Forward=2, Random=3)
  - Composition.autopilotDurationMode (enum: LongestClip=0, ClipTransport=1, Custom=2)
  - Composition.autopilotClipLoops (int, default 1)
  - Composition.autopilotLoop (bool)
  - Composition.autopilotMasterLayer (int, -1 = Off)
**Bindings:** Not directly bindable
**Programming mode:** Full controls in Composition Inspector
**Presentation mode:** HIDDEN
**Dependencies:** Layer Autopilot (composition-level overrides), BPM Tracker

#### 4.5.4 Per-Type Autopilot

**Level:** SECONDARY
**Current UI:** Composition Inspector > Per-Type Autopilot section
**Sub-features:**
  - Enable toggle [PROGRAMMING-ONLY] — activate per-type cycle timers (when off, uses per-layer autopilot)
  - Opaque Beats (1-64) [PROGRAMMING-ONLY] — beat count for opaque layer cycling
  - Opaque Play Until End toggle [PROGRAMMING-ONLY] — wait for video end before advancing
  - Transparent Beats (1-64) [PROGRAMMING-ONLY] — beat count for transparent layer cycling
  - Transparent Max Layers [PROGRAMMING-ONLY] — max simultaneous transparent layers
  - Transparent Randomize toggle [PROGRAMMING-ONLY] — random vs sequential clip advance
  - Effect Beats (1-64) [PROGRAMMING-ONLY] — beat count for FX layer cycling
  - Effect Max Layers [PROGRAMMING-ONLY] — max simultaneous FX layers
  - Effect Randomize toggle [PROGRAMMING-ONLY] — random vs sequential
  - Global Randomize [PROGRAMMING-ONLY] — override all types to random
  - Loop Autopilot [PROGRAMMING-ONLY] — restart when complete
**Parameters:**
  - PerTypeAutopilotConfig.perTypeEnabled (bool, default false)
  - PerTypeAutopilotConfig.opaqueCycleBeats (int, default 16)
  - PerTypeAutopilotConfig.opaquePlayUntilEnd (bool)
  - PerTypeAutopilotConfig.transparentCycleBeats (int, default 8)
  - PerTypeAutopilotConfig.transparentMaxLayers (int, default 2)
  - PerTypeAutopilotConfig.transparentRandomize (bool, default true)
  - PerTypeAutopilotConfig.effectCycleBeats (int, default 4)
  - PerTypeAutopilotConfig.effectMaxLayers (int, default 2)
  - PerTypeAutopilotConfig.effectRandomize (bool, default true)
  - PerTypeAutopilotConfig.globalRandomize (bool)
  - PerTypeAutopilotConfig.loopAutopilot (bool, default true)
**Bindings:** Not directly bindable
**Programming mode:** Full controls in Composition Inspector
**Presentation mode:** HIDDEN — runs automatically once enabled
**Dependencies:** Layer Types (different beat counts per type), BPM Tracker

---

### 4.6 Genre Detection & Smart Features

**Level:** SECONDARY
**Current UI:** Minimal — data exposed in FeatureSnapshot, genre/energy state accessible via Signal Bar; automation config in Composition Inspector
**Sub-features:**
  - 8-genre classifier [PRESENTATION-HIDDEN] — real-time genre detection from multi-feature scoring
    - House (0) — steady 4-on-floor kick, 120-130 BPM, warm bass
    - Techno (1) — driving, 125-145 BPM, high transient density, spectral flux
    - DnB (2) — fast breakbeats, 160-180 BPM, heavy bass, syncopation
    - Hip-Hop (3) — slower groove, 80-100 BPM, strong bass + mids
    - Ambient (4) — sparse, low transient density, spectral flatness, sustained tones
    - Rock (5) — full spectrum, high peak levels, guitar frequency presence
    - Pop/Electronic (6) — varied, mid-range BPM, bright, high spectral centroid (default)
    - Jazz/Other (7) — complex harmony, variable BPM, chromatic complexity
  - 3 energy states [PRESENTATION-HIDDEN] — Low(0), Medium(1), High(2); EMA-smoothed
  - Structural detection (4 states) [PRESENTATION-HIDDEN] — multi-scale EMA (100ms/1s/4s/16s) state machine
    - Normal (0) — standard playback
    - Buildup (1) — rising energy/tension
    - Drop (2) — high-energy peak
    - Breakdown (3) — low-energy respite
  - Auto-preset on genre change [PROGRAMMING-ONLY] — auto-switch decks when genre changes
  - Genre-deck assignment [PROGRAMMING-ONLY] — map each genre to a specific deck index (-1 = no switch)
  - Genre effect presets [PROGRAMMING-ONLY] — named FX preset to load per genre
  - Smart random autopilot [PROGRAMMING-ONLY] — energy-aware clip selection (lower column = calmer, higher = intense)
  - Structural scene triggering [PROGRAMMING-ONLY] — auto-switch decks on structural transitions (drop, breakdown)
  - AI mapping suggestions [NO UI — code only] — MappingSuggester provides genre-aware source-to-param recommendations with scored confidence (ghost feature)
**Parameters:**
  - Composition.autoPresetOnGenre (bool, default false)
  - Composition.smartAutopilotEnabled (bool, default false)
  - Composition.structuralSceneEnabled (bool, default false)
  - Composition.genreDeckAssignment[8] (int array, default all -1)
  - Composition.genrePresetNames[8] (string array)
  - GenreDetector smoothing: ~2s EMA + ~3s hysteresis
  - Per-genre EMA alpha: Techno/DnB = 0.50-0.55 (fast attack), Ambient = 0.12 (slow)
**Bindings:** Not directly bindable (genre/energy exposed as signals for routing)
**Programming mode:** Genre automation config visible in Composition Inspector
**Presentation mode:** HIDDEN — genre detection and smart features run automatically
**Dependencies:** Audio Analysis Pipeline (provides features to GenreDetector), Autopilot (smart random uses energy state), Structural Detector (provides 4 structural states)

---

### 4.7 Crossfader

**EXCLUDED** — Boris decided NOT to have this feature in the design overhaul.

Note: Crossfader code EXISTS in the Composition model (crossfaderPhase, crossfaderBlendMode, crossfaderBehaviour, crossfaderCurve fields) but has no UI and is not wired to rendering. It is listed in GAP_REPORT.md as a ghost feature. Do NOT include in the design.

---

## Domain 8: UI Framework

---

### 8.1 Top Bar

**Level:** PRIMARY
**Current UI:** Top Bar — 34px height, full width, always visible
**Sub-features:**
  - Audio source dropdown [PROGRAMMING-ONLY] — Mic Input or Audio File selection
  - Gain slider (0-4x) [ALWAYS-VISIBLE] — input level amplification; right-click resets to 1.0
  - Play button [ALWAYS-VISIBLE] — start global playback
  - Pause button [ALWAYS-VISIBLE] — pause global playback
  - Stop button [ALWAYS-VISIBLE] — halt all playback
  - Beat wheel [ALWAYS-VISIBLE] — 4-segment circle, visual beat position indicator (beats 1-4)
  - Bar/Phrase display [ALWAYS-VISIBLE] — text showing "Bar N" and "Phr 0.XX"
  - BPM display (18px) [ALWAYS-VISIBLE] — current tempo, color-coded: gray=searching, yellow=locking, green=locked
  - Tracker state label [ALWAYS-VISIBLE] — "SEARCHING", "LOCKING", or "LOCKED"
  - Tap button [ALWAYS-VISIBLE] — tap tempo (accumulates 8 taps, computes BPM)
  - Resync button [ALWAYS-VISIBLE] — reset beat phase to align with current audio
  - Manual toggle [ALWAYS-VISIBLE] — freeze BPM detection, enter manual value
  - BPM edit field [MINIMAL-IN-PRESENTATION] — manual BPM entry (visible only in manual mode)
  - BPM multiplier buttons (/4, /2, x1, x2, x4) [ALWAYS-VISIBLE] — 5 buttons for tempo scaling
  - Quantize dropdown [ALWAYS-VISIBLE] — Off / Next Beat / Next Downbeat for clip launch timing
  - Fade slider (0-5s) [ALWAYS-VISIBLE] — global transition speed between clips
  - Master slider (0-1) [ALWAYS-VISIBLE] — master output brightness/opacity
  - Output dropdown [ALWAYS-VISIBLE] — select output display (Off / Fullscreen / Windowed)
  - FPS label [MINIMAL-IN-PRESENTATION] — real-time frame rate counter
  - DSP label [MINIMAL-IN-PRESENTATION] — CPU load percentage
**Parameters:**
  - Audio source mode (int: Mic=1, File=2)
  - Input gain (float, 0-4x, default 1.0)
  - Global transport state (play/pause/stop)
  - BPM (float, from tracker or manual)
  - BPM multiplier (int: -4, -2, 1, 2, 4)
  - Manual BPM mode (bool)
  - Quantize mode (enum: Off=0, NextBeat=1, NextDownbeat=2)
  - Global transition speed (float, 0-5s, default 0.3s)
  - Master opacity (float, 0-1, default 1.0)
  - Output display selection (int, -1=off, 0+=display index)
**Bindings:** Tap Tempo / Resync (keyboard/MIDI), Global Play/Pause/Stop (keyboard/MIDI), Master Opacity (MIDI CC)
**Programming mode:** All 19 controls visible
**Presentation mode:** MINIMAL — BPM display, beat wheel, master slider, output dropdown always needed; audio source, gain, quantize could be hidden
**Dependencies:** Audio I/O (source selection, gain), BPM Tracker (tempo display/control), Render Pipeline (master opacity, FPS/DSP stats), Output (display selection)

---

### 8.2 Signal Bar

**Level:** PRIMARY
**Current UI:** Signal Bar — horizontal strip below Top Bar, collapsible
**Sub-features:**
  - Minimized mode (~20px) [ALWAYS-VISIBLE] — tiny meter bars only
  - Normal mode (~80px) [ALWAYS-VISIBLE] — label + meter bar + peak hold + numeric value per signal
  - Expanded mode (fill) [ALWAYS-VISIBLE] — full oscilloscope / histogram per signal; used by ProgrammingMode toggle
  - Add signal button [+] [PROGRAMMING-ONLY] — add new signal (oscillator, envelope, etc.)
  - Shrink/Grow buttons [-] / [+] [ALWAYS-VISIBLE] — resize all strips
  - Click strip to open Signal Inspector [PROGRAMMING-ONLY]
  - Default signals (12 visible): RMS, Peak, Bass, Mid, High, Beat Phase, Bar Phase, BPM, Onset Strength, Dominant Pitch, Musical Key, Structural State
  - Hidden signals (25): advanced audio features (P25 — sidechain pump, swing ratio, formant, resonance, reese, etc.) registered but not visible in default signal bar
  - Modulation signals (3): oscillators, envelopes, clip position
**Parameters:**
  - Display size (enum: Minimized=0, Normal=1, Expanded=2)
  - Per-signal: threshold (0-1), gain (0-4x), falloff (0-1s) — for audio signals
  - Per-oscillator: wave shape, beat duration, amplitude, phase offset
  - Per-envelope: curve type, beat duration, amplitude, phase, looping, one-shot
**Bindings:** Not directly bindable (signals themselves are binding sources)
**Programming mode:** Normal or Expanded; click strips to configure in Signal Inspector
**Presentation mode:** MINIMIZED (26px) or HIDDEN — meters provide visual feedback but take screen space
**Dependencies:** Audio Analysis Pipeline (all audio signals), Signal Routing Engine (signal evaluation), BPM Tracker (beat/bar phase)

---

### 8.3 Deck Grid

**Level:** PRIMARY
**Current UI:** Center area — the main performance workspace
**Sub-features:**
  - Layer strips (left 250px) [ALWAYS-VISIBLE] — dense Resolume-style per-layer headers with action buttons, transport, sliders, dropdowns
  - Clip cells (90x96px each) [ALWAYS-VISIBLE] — thumbnail area (76px) + name bar (20px); visual states: empty, loaded, active (cyan border), selected (white border), missing (red border + "!"), locked (orange "L"), has-effects (FX count badge)
  - Column trigger buttons (22px, top) [ALWAYS-VISIBLE] — numbered 1..K, click fires all clips in column; active column highlighted cyan
  - Deck tabs (24px, bottom) [ALWAYS-VISIBLE] — one per deck, click to switch active deck
  - Horizontal scrolling [ALWAYS-VISIBLE] — for columns exceeding visible area
  - Vertical scrolling [ALWAYS-VISIBLE] — for layers exceeding visible area
  - Layer display order [ALWAYS-VISIBLE] — highest layer at top (like Photoshop/Resolume)
  - Clip cell interactions [ALWAYS-VISIBLE]:
    - Click thumbnail: trigger/retrigger clip
    - Click name bar: select for inspector editing (Cmd/Shift for multi-select)
    - Drag name bar: move clip to another cell
    - Drag file from OS: load image, video, or image sequence
    - Drag from FX Browser: add effect to clip
    - Drag from Sources Browser: create procedural source clip
    - Drag from MilkDrop Browser: create MilkDrop preset clip
  - Default grid: 3 layers x 12 columns
**Parameters:**
  - Deck.numColumns (int, default 12)
  - Deck.layers.size() (int, default 3)
  - Scroll position (runtime, not persisted)
**Bindings:** Trigger Clip at position (keyboard/MIDI), Trigger Column (keyboard/MIDI)
**Programming mode:** Full grid visible with all interactions
**Presentation mode:** VESTIGIAL — current ProgrammingMode hides entire grid when signal bar is expanded. No graduated visibility exists.
**Dependencies:** Layer Controls (layer strip content), Clip Controls (cell content), Autopilot (active clip highlighting), Drag-Drop system (media/effects loading)

---

### 8.4 Inspector Panel

**Level:** PRIMARY
**Current UI:** Bottom panels area (center-right, ~25%), 4 tabs
**Sub-features:**
  - Clip tab [PROGRAMMING-ONLY] — 9 sections: Name, Dashboard (8 Link Knobs), Transport, Cuepoints, Autopilot, Source Parameters, Video, Transform, Effects Stack
  - Layer tab [PROGRAMMING-ONLY] — 12 sections: Name, Dashboard (8 Link Knobs), Autopilot, Layer Master, Video, Transition, Keying, Dry/Wet, 3D Controls, Transform, Feedback, Layer Effects
  - Composition tab [PROGRAMMING-ONLY] — 9 sections: Name+Resolution, Dashboard (8 Link Knobs), Autopilot, Per-Type Autopilot, Composition Master, Video, Transform, Global Effects, Output Settings
  - Signal tab [PROGRAMMING-ONLY] — shows Signal Inspector for selected signal (Audio/Oscillator/Envelope controls)
  - Pin button [ALWAYS-VISIBLE] — prevent auto-tab-switching during performance
  - Auto-tab-switching [ALWAYS-VISIBLE] — click clip = Clip tab, click layer = Layer tab, click signal = Signal tab
**Parameters:**
  - Active tab (int, 0-3)
  - Pinned state (bool)
**Bindings:** Not directly bindable
**Programming mode:** Full inspector with all tabs and sections
**Presentation mode:** HIDDEN — inspector is for configuration, not performance. Pin button allows selective access.
**Dependencies:** All Composition/Layer/Clip controls (inspector surfaces their parameters), Signal Routing (Signal tab), Universal Parameter Control (all sliders use UPC widget)

---

### 8.5 Browser Panel

**Level:** PRIMARY
**Current UI:** Bottom panels area (right, ~25%), 6 tabs
**Sub-features:**
  - Files tab [PROGRAMMING-ONLY] — grid/list view with 64px thumbnails, path bar, search, favorites, drag-to-deck
  - FX tab [PROGRAMMING-ONLY] — 135 effects across 11 categories (Warp 28, Color 31, Glitch 15, Blur/Post 10, 3D/Depth 10, Pattern 15, Animation 4, Time 6, Audio 7, Blend 5, Composite 3); search field, multi-select, drag to cells/inspectors/stacks
  - Sources tab [PROGRAMMING-ONLY] — 101 procedural sources across 15+ categories; search, multi-select, drag to deck cells
  - Comp-Decks tab [PROGRAMMING-ONLY] — two sections (Compositions and Decks) with Save/Load/Delete for each
  - Record tab [PROGRAMMING-ONLY] — Record/Stop buttons, Play button, Save/Load as JSON, Browse Output Folder, Status/Event count display
  - MilkDrop tab [PROGRAMMING-ONLY] — ~9,800 presets via projectM; 4 sub-tabs (Curated 30 / Favorites / Recent 20 / All searchable); 3 play modes (Jukebox, VJ Clip, Playlist); Jukebox controls: Play/Stop, Pool dropdown, Timing (4/8/16/32 beats or 10/30/60 seconds), Blend slider; Navigation: < / > / ? (random) / Lock buttons
**Parameters:**
  - Active tab (int, 0-5)
  - Files: current path, view mode (grid/list), search query
  - FX: search query, expanded categories
  - Sources: search query, expanded categories
  - MilkDrop: active sub-tab, Jukebox mode/timing/pool/blend, lock state
**Bindings:** Not directly bindable (content accessed via drag-drop)
**Programming mode:** Full browser with all tabs
**Presentation mode:** HIDDEN — browser is for content loading during programming
**Dependencies:** Effect Library (FX tab content), Source Registry (Sources tab content), PresetManager (Comp-Decks save/load), SessionRecorder (Record tab), MilkDrop/projectM (MilkDrop tab)

---

### 8.6 Timing Window

**Level:** UTILITY
**Current UI:** Bottom panels area (center-left, ~28%), 3 tabs
**Sub-features:**
  - BPM tab [PLACEHOLDER] — intended for BPM visualization, content is placeholder
  - Routing tab [PLACEHOLDER] — intended for signal routing display, content is placeholder
  - Oscillators tab [PLACEHOLDER] — intended for oscillator waveform display, content is placeholder
**Parameters:** None functional
**Bindings:** None
**Programming mode:** Tabs visible but empty
**Presentation mode:** HIDDEN — no functional content
**Dependencies:** BPM Tracker (BPM tab would display), Signal Routing (Routing tab would display), Oscillator signals (Oscillators tab would display)

---

### 8.7 Programming Mode

**Level:** UTILITY
**Current UI:** View menu > Programming Mode toggle
**Sub-features:**
  - Toggle signal bar expansion [ALWAYS-VISIBLE] — switches SignalBar between Normal (84px) and Expanded (fills everything)
  - Panel hiding [ALWAYS-VISIBLE] — when expanded, all other panels (deck, inspector, browser, preview, timing) are setVisible(false)
**Parameters:**
  - Programming mode active (bool)
**Bindings:** Not directly bindable (menu toggle only)
**Programming mode:** Toggle via View menu
**Presentation mode:** VESTIGIAL — the ProgrammingMode component itself is always setVisible(false) in MainComponent::resized(). The dual-mode system that would provide graduated control visibility DOES NOT EXIST YET. Current implementation is binary: see everything OR see only expanded signal bar.
**Dependencies:** Signal Bar (expansion target), MainComponent (panel visibility management)

---

### 8.8 Preferences

**Level:** UTILITY
**Current UI:** Audio-DNA menu > Preferences (dialog window), 8 tabs
**Sub-features:**
  - General tab [PROGRAMMING-ONLY] — Confirm on quit, Show tooltips
  - Audio tab [PROGRAMMING-ONLY] — Sample rate, Buffer size, BPM detection range
  - Video tab [PROGRAMMING-ONLY] — FPS target, Render resolution, MilkDrop preset directory
  - MIDI tab [PLACEHOLDER] — placeholder, no functional content
  - Recording tab [PLACEHOLDER] — placeholder, no functional content
  - Defaults tab [PLACEHOLDER] — placeholder, no functional content
  - Feedback tab [PLACEHOLDER] — placeholder, no functional content
  - About tab [PROGRAMMING-ONLY] — Version, credits
**Parameters:**
  - General: confirmOnQuit (bool), showTooltips (bool)
  - Audio: sampleRate (int), bufferSize (int), bpmDetectionRange (range)
  - Video: fpsTarget (int), renderResolution (WxH), milkDropDir (path)
**Bindings:** None
**Programming mode:** Full preferences dialog
**Presentation mode:** HIDDEN — settings only changed outside performance
**Dependencies:** Audio I/O (audio settings), Render Pipeline (video settings), MilkDrop (preset directory)

---

### 8.9 Hidden v1 Components

**Level:** UTILITY
**Current UI:** NO UI — code exists but setVisible(false) in v2 layout

#### AudioReadoutPanel [HIDDEN-V1]
- Full audio feature readout panel from v1 layout
- Shows all audio analysis features as text/meters
- Hidden in v2 layout, superseded by Signal Bar

#### SpectrumDisplay [HIDDEN-V1]
- 7-band energy visualization from v1 layout
- Bar graph of sub/bass/lowmid/mid/highmid/presence/brilliance
- Hidden in v2 layout, partially replaced by Signal Bar band meters

#### EffectsRackPanel [HIDDEN-V1]
- v1 effects panel with knobs and mapping UI
- Superseded by EffectStackView in v2 inspector
- Hidden in v2 layout

#### v1 Controls in MainComponent [HIDDEN-V1]
- audioSourceSelector_, inputGainSlider_, masterLevelSlider_, displaySelector_
- resolutionSelector_, randomLabel_, beatRandomToggle_, beatCountSelector_
- syncButton_, fpsLabel_, cpuLabel_
- openImageButton_, fileLabel_, imageSequenceButton_
- v1 preset slots (10 buttons + dropdowns)
- v1 file-loading controls
- All setVisible(false) in v2 layout but code remains

**Parameters:** Various (all from v1 architecture)
**Bindings:** None in v2
**Programming mode:** Invisible
**Presentation mode:** Invisible
**Dependencies:** None in v2 (orphaned code)

---

## Domain 9: Data & Persistence

---

### 9.1 Preset Management

**Level:** PRIMARY
**Current UI:** Browser Panel > Comp-Decks tab (Save/Load/Delete for compositions and decks) + Composition menu (New/Open/Save/Save As)
**Sub-features:**
  - Save composition [ALWAYS-VISIBLE] — Cmd+S, full hierarchy serialized to JSON: all decks, layers, clips, effects, mappings, global settings, genre automation, transforms
  - Open composition [ALWAYS-VISIBLE] — Cmd+O, deserializes JSON back to full app state
  - Save As [PROGRAMMING-ONLY] — save to new file path
  - New composition [PROGRAMMING-ONLY] — reset to default state (1 deck, 3 layers, 12 columns)
  - FX preset save/load [PROGRAMMING-ONLY] — save/load effect chain + mappings as separate JSON (PresetManager)
  - Deck template save/load [PROGRAMMING-ONLY] — save/load deck state including audio path, image path, FX, mappings, slot assignments, keyboard layout (PresetManager.DeckState)
  - Collect Media [PROGRAMMING-ONLY] — package composition + all referenced media files into one folder
  - Relocate Files [PROGRAMMING-ONLY] — find and relink missing media file references
**Parameters:**
  - Composition.filePath (juce::File — current save location)
  - Composition.name (string)
**Bindings:** Not directly bindable (menu actions)
**Programming mode:** Full file management via menu and Comp-Decks browser tab
**Presentation mode:** HIDDEN — save operations happen between performances. Cmd+S always available.
**Dependencies:** Composition Serialization (JSON format), Effect Chain (FX preset content), Mapping Engine (mapping preset content)

---

### 9.2 Composition Serialization

**Level:** PRIMARY
**Current UI:** NO UI — code only (Composition::toVar/fromVar, Deck::toVar/fromVar, Layer::toVar/fromVar, Clip::toVar/fromVar)
**Sub-features:**
  - Composition to JSON [ALWAYS-VISIBLE] — Composition.toVar() serializes name, activeDeckIndex, masterOpacity, globalTransitionSpeed, bpmMultiplier, quantizeMode, outputWidth/Height/Display, all decks, global effects
  - Deck to JSON [ALWAYS-VISIBLE] — Deck.toVar() serializes name, id, numColumns, all layers
  - Layer to JSON [ALWAYS-VISIBLE] — Layer.toVar() serializes name, id, type, all layer fields (opacity, blend mode, keying, transition, transform, feedback, autopilot, effects, clips)
  - Clip to JSON [ALWAYS-VISIBLE] — Clip.toVar() serializes name, id, mediaType, mediaFile, sourceType, sourceParams, effects, transport, in/out points, beat snap, cuepoints, autopilot, video properties, transform, MilkDrop playlist, content lock
  - JSON parse/write [ALWAYS-VISIBLE] — via JUCE var/DynamicObject + JSON::toString/parse
  - File I/O [ALWAYS-VISIBLE] — Composition.saveToFile / loadFromFile (replaceWithText / loadFileAsString)
**Parameters:** All composition state serialized (see Data Hierarchy in source doc section 21)
**Bindings:** None (serialization is infrastructure)
**Programming mode:** Transparent — happens on save/load
**Presentation mode:** Transparent
**Dependencies:** JUCE (var, DynamicObject, JSON, File), all model types (Composition, Deck, Layer, Clip, EffectSlot)

**Known gaps:**
  - Composition.toVar() does NOT serialize: masterSpeed, compOpacity, crossfader fields, composition transform, autopilot config, per-type autopilot, genre automation config, genreDeckAssignment, genrePresetNames. These fields exist in the model but are missing from the serialization code.
  - PresetManager kSourceNames[] missing P25 advanced sources (SidechainPump, SwingRatio, etc.) — presets saved with P25 mappings fail to round-trip.

---

### 9.3 Layout Management

**Level:** UTILITY
**Current UI:** View menu > Save Layout / Load Layout / Reset Layout
**Sub-features:**
  - Save Layout [PROGRAMMING-ONLY] — save current workspace panel proportions and visibility to persistent storage
  - Load Layout [PROGRAMMING-ONLY] — restore saved panel arrangement
  - Reset Layout [PROGRAMMING-ONLY] — return to default 4-panel proportions (22% preview / 28% timing / 25% inspector / 25% browser)
  - Panel show/hide toggles [PROGRAMMING-ONLY] — View menu: Signal Bar / Deck / Preview / Inspector / Browser / Timing Window — toggle individual panel visibility
  - FPS Stats toggle [PROGRAMMING-ONLY] — View menu: toggle performance overlay
**Parameters:**
  - Panel visibility states (bool per panel: signal bar, deck, preview, inspector, browser, timing window)
  - Panel proportions (float, 4 panels with 3 dividers)
  - FPS stats overlay (bool)
**Bindings:** None
**Programming mode:** Full layout management via View menu
**Presentation mode:** HIDDEN — layout is configured during setup
**Dependencies:** MainComponent (panel visibility), Bottom panels (divider proportions)

---

### 9.4 Undo/Redo

**Level:** PRIMARY
**Current UI:** Composition menu > Undo (Cmd+Z) / Redo (Cmd+Shift+Z)
**Sub-features:**
  - Undo [ALWAYS-VISIBLE] — Cmd+Z, reverses most recent command
  - Redo [ALWAYS-VISIBLE] — Cmd+Shift+Z, re-applies most recently undone command
  - History stack [PRESENTATION-HIDDEN] — 500-deep linear history using Command pattern
  - Undo/Redo descriptions [PRESENTATION-HIDDEN] — human-readable descriptions for each command
  - History change callback [PRESENTATION-HIDDEN] — fires onHistoryChanged for menu state updates (enable/disable Undo/Redo items)
  - Clear history [PROGRAMMING-ONLY] — wipe all history (e.g., on new composition)
  - Oldest commands discarded [PRESENTATION-HIDDEN] — when stack exceeds 500, oldest commands are removed
**Parameters:**
  - UndoManager.history_ (vector of Command, max 500)
  - UndoManager.currentIndex_ (int, points to next write slot)
**Bindings:** Cmd+Z / Cmd+Shift+Z (hardcoded keyboard shortcuts)
**Programming mode:** Undo/Redo always available via menu and shortcuts
**Presentation mode:** MINIMAL — keyboard shortcuts still work, menu items may be hidden
**Dependencies:** Command pattern (all state-changing operations must create Command objects), Composition model (state to undo/redo)

---

### 9.5 Binding Import/Export

**Level:** UTILITY
**Current UI:** Shortcuts menu > Export Bindings / Import Bindings
**Sub-features:**
  - Export Bindings [PROGRAMMING-ONLY] — save all keyboard and MIDI bindings to JSON file
  - Import Bindings [PROGRAMMING-ONLY] — load bindings from JSON file, replacing current bindings
  - Copy Effects / Paste Effects [PROGRAMMING-ONLY] — via Composition menu (clip-to-clip) and Layer menu (layer-to-layer effect chain transfer)
**Parameters:**
  - Binding data: per-binding keyCode or MIDI note/CC, action, targetMode, triggerMode, modifiers
**Bindings:** None (this IS the binding management)
**Programming mode:** Menu actions for import/export
**Presentation mode:** HIDDEN — bindings configured before performance
**Dependencies:** BindingManager (owns all binding data), MidiHandler (MIDI binding source), JSON serialization

---

### 9.6 Session Recording & Playback

See Domain 7: Output & Recording > Session Recording for full documentation.
