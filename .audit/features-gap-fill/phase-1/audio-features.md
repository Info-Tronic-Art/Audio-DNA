# Audio-DNA Feature Audit: Audio & Analysis Domain

**Audited:** 2026-05-20 | **Builder:** 2 of 3 (Audio/Analysis domain)
**Source SHA:** verified against live code | **Source files:** MappingTypes.h, CurveTransforms.h, SignalRegistry.cpp, SourceRegistry.cpp, EffectLibrary.cpp, TimingWindow.cpp/h

---

## 1. Curve Types (MappingCurve enum + CurveTransforms.h)

**FEATURES.md says:** 5 curve types (Linear, Exponential, Logarithmic, S-Curve, Stepped)
**Actual code:** 24 curve types (5 original + 19 P24 easing functions)
**Source:** `src/mapping/MappingTypes.h:89-120`, `src/mapping/CurveTransforms.h:1-254`

### 1.1 Original Curves (5)

| # | Enum Name | Display | Formula | Behavior |
|---|-----------|---------|---------|----------|
| 0 | Linear | Linear | y = x | Direct 1:1 mapping, no transform |
| 1 | Exponential | Exponential | y = x^2 | Emphasizes peaks, compresses lows. Quiet signals produce little output |
| 2 | Logarithmic | Logarithmic | y = log(1+9x)/log(10) | Compresses peaks, lifts lows. Subtle audio changes produce visible output |
| 3 | SCurve | S-Curve | y = x^2(3-2x) | Smoothstep, de-emphasizes extremes. Concentrates action in midrange |
| 4 | Stepped | Stepped | y = floor(x*N)/N | Quantizes to N discrete steps (default N=4). Creates staircase response |

### 1.2 P24 Easing Functions (19)

Standard animation easing curves organized by family. Added in Phase 24.

**Circular family** (3 curves):

| # | Enum Name | Display | Behavior |
|---|-----------|---------|----------|
| 5 | CircularIn | Circular In | Slow start, accelerating via circular arc (1 - sqrt(1-x^2)) |
| 6 | CircularOut | Circular Out | Fast start, decelerating via circular arc |
| 7 | CircularInOut | Circular In/Out | Slow-fast-slow, symmetric circular arcs |

**Back family** (3 curves) -- overshoots then settles:

| # | Enum Name | Display | Behavior |
|---|-----------|---------|----------|
| 8 | BackIn | Back In | Pulls back slightly before accelerating forward (overshoot constant c=1.70158) |
| 9 | BackOut | Back Out | Overshoots target then settles back |
| 10 | BackInOut | Back In/Out | Pull-back start, overshoot end, symmetric |

**Elastic family** (3 curves) -- spring oscillation:

| # | Enum Name | Display | Behavior |
|---|-----------|---------|----------|
| 11 | ElasticIn | Elastic In | Oscillating ramp up with exponential amplitude growth |
| 12 | ElasticOut | Elastic Out | Overshoots and oscillates around target, amplitude decays exponentially |
| 13 | ElasticInOut | Elastic In/Out | Oscillating start and end, smooth center |

**Bounce family** (3 curves):

| # | Enum Name | Display | Behavior |
|---|-----------|---------|----------|
| 14 | BounceIn | Bounce In | Reversed bounce effect approaching start |
| 15 | BounceOut | Bounce Out | Ball-drop bounce: 4-stage parabolic arcs with decreasing amplitude (n1=7.5625) |
| 16 | BounceInOut | Bounce In/Out | Bounce at both ends |

**Cubic family** (3 curves):

| # | Enum Name | Display | Behavior |
|---|-----------|---------|----------|
| 17 | CubicIn | Cubic In | x^3, stronger acceleration than exponential (x^2) |
| 18 | CubicOut | Cubic Out | (x-1)^3 + 1, stronger deceleration than exponential |
| 19 | CubicInOut | Cubic In/Out | Symmetric cubic ease, sharper transitions than S-Curve |

**Sine family** (3 curves):

| # | Enum Name | Display | Behavior |
|---|-----------|---------|----------|
| 20 | SineIn | Sine In | 1 - cos(x * pi/2), gentle sinusoidal acceleration |
| 21 | SineOut | Sine Out | sin(x * pi/2), gentle sinusoidal deceleration |
| 22 | SineInOut | Sine In/Out | -(cos(pi*x) - 1)/2, smooth sinusoidal ease |

**Hold** (1 curve):

| # | Enum Name | Display | Behavior |
|---|-----------|---------|----------|
| 23 | Hold | Hold | Step function: outputs 0 for all inputs < 1.0, then jumps to 1.0. Gate/trigger behavior |

### 1.3 Dispatch

`CurveTransforms::applyCurve(int curveIndex, float x, int steppedN)` dispatches by enum index via switch statement. `steppedN` parameter only applies to curve index 4 (Stepped); ignored for all others.

---

## 2. Signals (SignalRegistry)

**GAP_REPORT says:** 36 signals (8 visible + 25 hidden + 3 modulation)
**Actual code:** 32 signals (8 visible + 21 hidden audio + 2 modulation + 1 clip position)
**Source:** `src/signal/SignalRegistry.cpp:4-84`

**Correction:** GAP_REPORT overcounts by 4. The hidden count is 21 (not 25), and modulation count is 2 (not 3). Clip Position is a separate signal type, not a modulation signal.

### 2.1 Visible Audio Signals (8)

Displayed in the Signal Bar by default.

| Signal Name | MappingSource | Category | What it measures |
|-------------|---------------|----------|------------------|
| Volume | RMS | Amplitude | Root mean square of audio samples -- overall loudness |
| Sub Bass | BandSub | Bands | Energy in sub-bass frequency band (bandEnergies[0], ~20-60Hz) |
| Bass | BandBass | Bands | Energy in bass frequency band (bandEnergies[1], ~60-250Hz) |
| Mid | BandMid | Bands | Energy in midrange frequency band (bandEnergies[3], ~500-2kHz) |
| Air | BandBrilliance | Bands | Energy in highest frequency band (bandEnergies[6], ~6-20kHz) |
| Tempo | BPM | Rhythm | Detected beats per minute via Aubio autocorrelation |
| Beat Position | BeatPhase | Rhythm | Phase within current beat [0, 1), resets at each beat |
| Hit | OnsetStrength | Rhythm | Onset/transient detection strength via Aubio spectral flux |

### 2.2 Hidden Audio Signals (21)

Registered at startup but `setVisible(false)`. Available for signal routing and mapping but not shown in default Signal Bar.

**Amplitude group (3):**

| Signal Name | MappingSource | Source File | What it measures |
|-------------|---------------|-------------|------------------|
| Peak | Peak | SignalRegistry.cpp:38 | Peak sample amplitude in analysis window |
| Punch | DynamicRange | SignalRegistry.cpp:39 | Dynamic range (difference between peak and RMS) |
| Hits Per Second | TransientDensity | SignalRegistry.cpp:40 | Number of detected onsets per second in 2s sliding window |

**Bands group (3):**

| Signal Name | MappingSource | Source File | What it measures |
|-------------|---------------|-------------|------------------|
| Low Mid | BandLowMid | SignalRegistry.cpp:41 | Energy in low-mid band (bandEnergies[2], ~250-500Hz) |
| High Mid | BandHighMid | SignalRegistry.cpp:42 | Energy in high-mid band (bandEnergies[4], ~2-4kHz) |
| Presence | BandPresence | SignalRegistry.cpp:43 | Energy in presence band (bandEnergies[5], ~4-6kHz) |

**Rhythm group (4):**

| Signal Name | MappingSource | Source File | What it measures |
|-------------|---------------|-------------|------------------|
| Hit Strength | OnsetStrength | SignalRegistry.cpp:44 | Same source as "Hit" (duplicate signal, different name for routing) |
| Bar Position | BarPhase | SignalRegistry.cpp:45 | Phase within current bar [0, 1) based on detected downbeats |
| Phrase Position | PhrasePhase | SignalRegistry.cpp:46 | Phase within current phrase (N bars) |
| Bar Count | BarCount | SignalRegistry.cpp:47 | Cumulative bar count since audio start |

**Spectral/Timbral group (3):**

| Signal Name | MappingSource | Source File | What it measures |
|-------------|---------------|-------------|------------------|
| Brightness | SpectralCentroid | SignalRegistry.cpp:48 | Weighted mean frequency of spectrum -- how "bright" the sound is |
| Change | SpectralFlux | SignalRegistry.cpp:49 | Frame-to-frame spectral difference -- how much the sound changes |
| Noisiness | SpectralFlatness | SignalRegistry.cpp:50 | Ratio of geometric to arithmetic mean of spectrum (0=tonal, 1=noise) |

**Pitch group (3):**

| Signal Name | MappingSource | Source File | What it measures |
|-------------|---------------|-------------|------------------|
| Note | DominantPitch | SignalRegistry.cpp:51 | Detected fundamental pitch via Aubio yinfft algorithm |
| Note Confidence | PitchConfidence | SignalRegistry.cpp:52 | Confidence of pitch detection [0, 1] |
| Chord Change | HarmonicChange | SignalRegistry.cpp:53 | HCDF: chroma frame difference function -- harmonic transition detection |

**P25 Advanced group (5):**

| Signal Name | MappingSource | Source File | What it measures |
|-------------|---------------|-------------|------------------|
| Sidechain Pump | SidechainPump | SignalRegistry.cpp:57 | Bass/mid anti-correlation detecting sidechain compression (EDM pump) |
| Swing | SwingRatio | SignalRegistry.cpp:58 | Timing deviation from straight grid -- groove/swing detection |
| Vocal Presence | FormantPresence | SignalRegistry.cpp:59 | Vocal formant energy concentration in 300-3kHz region |
| Resonance | ResonancePeak | SignalRegistry.cpp:60 | Spectral kurtosis measuring sharp resonant peaks |
| Reese Bass | ReeseBass | SignalRegistry.cpp:61 | Bass spectral spread detecting reese/wobble bass synthesis |

### 2.3 Modulation Signals (2, visible by default)

| Signal Name | Type | Source File | What it does |
|-------------|------|-------------|--------------|
| Mod 1 | OscillatorSignal | SignalRegistry.cpp:64 | Sine wave oscillator at 1.0 Hz, generates cyclic modulation |
| Mod 2 | EnvelopeSignal | SignalRegistry.cpp:69 | Envelope follower at 4.0 Hz, generates attack/decay shape |

### 2.4 Clip Position Signal (1, hidden)

| Signal Name | Type | Source File | What it does |
|-------------|------|-------------|--------------|
| Clip Position | ClipPositionSignal | SignalRegistry.cpp:76 | Tracks playback position [0, 1] of active clip. Updated from render thread. Hidden by default (P24 addition) |

---

## 3. Mapping Sources (MappingSource enum)

**FEATURES.md says:** "40+" mapping sources
**GAP_REPORT says:** 57 total, only 40 documented
**Actual code:** 58 entries in MappingSource enum (excluding Count sentinel)
**Source:** `src/mapping/MappingTypes.h:6-86`

### 3.1 Full Mapping Source Inventory

Organized by the enum's comment groups. Every value corresponds to a FeatureSnapshot field.

**Amplitude (3):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 0 | RMS | rms | Root mean square of audio samples |
| 1 | Peak | peak | Peak sample amplitude in analysis window |
| 2 | RmsDB | rmsDB | RMS converted to decibel scale |

**Loudness (3):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 3 | LUFS | lufs | Loudness Units Full Scale (K-weighted, 400ms window) |
| 4 | DynamicRange | dynamicRange | Difference between peak and RMS |
| 5 | TransientDensity | transientDensity | Onset count in 2s sliding window |

**Spectral (4):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 6 | SpectralCentroid | spectralCentroid | Weighted mean frequency of spectrum |
| 7 | SpectralFlux | spectralFlux | Frame-to-frame spectral difference |
| 8 | SpectralFlatness | spectralFlatness | Geometric/arithmetic mean ratio (tonality measure) |
| 9 | SpectralRolloff | spectralRolloff | Frequency below which 85% of energy lies |

**7-Band Energies (7):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 10 | BandSub | bandEnergies[0] | Sub-bass energy (~20-60Hz) |
| 11 | BandBass | bandEnergies[1] | Bass energy (~60-250Hz) |
| 12 | BandLowMid | bandEnergies[2] | Low-mid energy (~250-500Hz) |
| 13 | BandMid | bandEnergies[3] | Mid energy (~500-2kHz) |
| 14 | BandHighMid | bandEnergies[4] | High-mid energy (~2-4kHz) |
| 15 | BandPresence | bandEnergies[5] | Presence energy (~4-6kHz) |
| 16 | BandBrilliance | bandEnergies[6] | Brilliance energy (~6-20kHz) |

**Onset / Rhythm (6):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 17 | OnsetStrength | onsetStrength | Onset detection strength (Aubio spectral flux) |
| 18 | BeatPhase | beatPhase | Phase within current beat [0, 1) |
| 19 | BPM | bpm | Detected tempo (Aubio autocorrelation) |
| 20 | BarPhase | barPhase | Phase within current bar [0, 1) |
| 21 | PhrasePhase | phrasePhase | Phase within current phrase |
| 22 | BarCount | barCount | Cumulative bar count |

**Structural (1):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 23 | StructuralState | structuralState | Multi-scale EMA state machine output (intro/verse/chorus/drop/outro) |

**Pitch / Harmony (4):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 24 | DominantPitch | dominantPitch | Fundamental pitch via Aubio yinfft |
| 25 | PitchConfidence | pitchConfidence | Pitch detection confidence [0, 1] |
| 26 | DetectedKey | detectedKey | Musical key via Krumhansl-Schmuckler (0-23: 12 major + 12 minor) |
| 27 | HarmonicChange | harmonicChange | HCDF: chroma frame difference function |

**Timbral / MFCCs (13):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 28 | MFCC0 | mfccs[0] | Mel-frequency cepstral coefficient 0 (overall energy/loudness) |
| 29 | MFCC1 | mfccs[1] | MFCC 1 (spectral slope -- brightness vs warmth) |
| 30 | MFCC2 | mfccs[2] | MFCC 2 (spectral shape) |
| 31 | MFCC3 | mfccs[3] | MFCC 3 |
| 32 | MFCC4 | mfccs[4] | MFCC 4 |
| 33 | MFCC5 | mfccs[5] | MFCC 5 |
| 34 | MFCC6 | mfccs[6] | MFCC 6 |
| 35 | MFCC7 | mfccs[7] | MFCC 7 |
| 36 | MFCC8 | mfccs[8] | MFCC 8 |
| 37 | MFCC9 | mfccs[9] | MFCC 9 |
| 38 | MFCC10 | mfccs[10] | MFCC 10 |
| 39 | MFCC11 | mfccs[11] | MFCC 11 |
| 40 | MFCC12 | mfccs[12] | MFCC 12 (highest detail coefficient) |

**Chroma (12):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 41 | ChromaC | chroma[0] | Energy in pitch class C |
| 42 | ChromaCs | chroma[1] | Energy in pitch class C#/Db |
| 43 | ChromaD | chroma[2] | Energy in pitch class D |
| 44 | ChromaDs | chroma[3] | Energy in pitch class D#/Eb |
| 45 | ChromaE | chroma[4] | Energy in pitch class E |
| 46 | ChromaF | chroma[5] | Energy in pitch class F |
| 47 | ChromaFs | chroma[6] | Energy in pitch class F#/Gb |
| 48 | ChromaG | chroma[7] | Energy in pitch class G |
| 49 | ChromaGs | chroma[8] | Energy in pitch class G#/Ab |
| 50 | ChromaA | chroma[9] | Energy in pitch class A |
| 51 | ChromaAs | chroma[10] | Energy in pitch class A#/Bb |
| 52 | ChromaB | chroma[11] | Energy in pitch class B |

**Advanced Audio / P25 (5):**

| # | Enum Name | FeatureSnapshot Field | Description |
|---|-----------|----------------------|-------------|
| 53 | SidechainPump | sidechainPump | Bass/mid anti-correlation (sidechain compression detection) |
| 54 | SwingRatio | swingRatio | Timing deviation from straight grid |
| 55 | FormantPresence | formantPresence | Vocal formant energy concentration |
| 56 | ResonancePeak | resonancePeak | Spectral kurtosis (sharp resonant peaks) |
| 57 | ReeseBass | reeseBass | Bass spectral spread (reese/wobble bass) |

**Summary:** 58 mapping sources across 10 groups. FEATURES.md documents approximately 40 (lists "40+"). The 18 undocumented sources are: RmsDB, LUFS, DynamicRange, TransientDensity, SpectralRolloff, BarCount, StructuralState, PitchConfidence, DetectedKey, HarmonicChange, MFCC0-12 (listed as group), all 12 Chroma entries, and all 5 P25 advanced sources.

---

## 4. Effect Parameters

**GAP_REPORT says:** 333 total across 135 effects
**Actual code:** 333 parameters across 135 effects (CONFIRMED)
**Source:** `src/effects/EffectLibrary.cpp:1-812`

### 4.1 Breakdown by Category

| Category | Effects | Parameters | Avg Params/Effect |
|----------|---------|------------|-------------------|
| warp | 27 | 67 | 2.5 |
| color | 31 | 64 | 2.1 |
| pattern | 19 | 50 | 2.6 |
| glitch | 15 | 40 | 2.7 |
| animation | 6 | 23 | 3.8 |
| blur | 10 | 21 | 2.1 |
| 3d | 9 | 20 | 2.2 |
| time | 6 | 14 | 2.3 |
| composite | 3 | 13 | 4.3 |
| audio | 4 | 12 | 3.0 |
| blend | 5 | 9 | 1.8 |
| **Total** | **135** | **333** | **2.5** |

### 4.2 Effects by Category

**Warp (27 effects, 67 params):**
Ripple (3), Bulge (3), Wave (3), Liquid (2), Kaleidoscope (2), Fisheye (1), Swirl (2), Polar Coords (1), Twirl (2), Shear (2), Elastic Bounce (2), Ripple Pond (2), Diamond Distort (2), Barrel Distort (1), Sine Grid (2), Glitch Displace (2), Quad Mirror (2), Flip (2), Warp Field (3), Slide Wrap (2), Tile Grid (4), Spot Zoom (6), Bendoscope (3), UV Remap (3), Liquid Morph (3), Zoom Warp (4), Density Wave (3)

**Color (31 effects, 64 params):**
Hue Shift (1), Saturation (1), Brightness (1), Duotone (7), Chromatic Aberration (2), Invert (1), Posterize (1), Color Shift (3), Thermal (1), Contrast (2), Sepia (1), Cross Process (1), Split Tone (3), Color Halftone (2), Dither (2), Heat Map (1), Selective Color (2), Film Grain (2), Gamma Levels (3), Solarize (2), Greyscale (2), Threshold (2), Exposure (1), Vibrance (1), Auto Mask (3), Chroma Key (4), Palette Remap (3), Color Grade (1), Pitch Chromatic Shift (3), Key Palette (3), Chroma Dissolve (2)

**Pattern (19 effects, 50 params):**
CRT (2), VHS (2), ASCII Art (2), Dot Matrix (2), Crosshatch (2), Emboss (2), Oil Paint (1), Pencil Sketch (2), Voronoi Glass (2), Cross Stitch (2), Night Vision (1), Triangulate (2), Neon Edge (4), Cartoon Ink (4), Pop Raster (4), Brush Strokes (4), Bump Light (4), Monitor Wall (4), Topographic Lines (4)

**Glitch (15 effects, 40 params):**
Pixel Scatter (2), RGB Split (2), Block Glitch (2), Scanlines (2), Digital Rain (2), Noise (2), Mirror (2), Pixelate (1), Pixel Explosion (4), Color Flash (5), Fragment Burst (4), Signal Destroy (3), Rhythm Slice (3), Data Corruption (3), Glitch Sort (3)

**Animation (6 effects, 23 params):**
Strobe (2), Pulse (2), Slit Scan (2), Point Zoom (8), Directional Feedback (6), Transient Flash (3)

**Blur (10 effects, 21 params):**
Gaussian Blur (1), Zoom Blur (3), Shake (2), Vignette (2), Motion Blur (2), Glow (2), Edge Detect (1), Sharpen (2), Edge Blur (2), Drop Shadow (4)

**3D (9 effects, 20 params):**
Perspective Tilt (2), Cylinder Wrap (2), Sphere Wrap (1), Tunnel (2), Page Curl (2), Parallax Layers (2), Dot Field (3), Luminance Terrain (3), Voxel Matrix (3)

**Time (6 effects, 14 params):**
Echo (2, temporal), Posterize Time (2, temporal), Freeze (1, temporal), Screen Split (4), Frame Stutter (2), Channel Delay (3, temporal)

**Composite (3 effects, 13 params):**
Line Cloner (5), Radial Cloner (4), Cube Scatter (4)

**Audio (4 effects, 12 params):**
Harmonic Displacement (3), Timbral Mosaic (3), Structural Morph (3), Beat Ripple (3)

**Blend (5 effects, 9 params):**
Double Exposure (2), Frosted Glass (2), Prism (2), Rain on Glass (2), Hexagonalize (1)

---

## 5. Source Parameters

**GAP_REPORT says:** 628 total across 101 sources
**Actual code:** 754 parameters across 108 sources
**Source:** `src/sources/SourceRegistry.cpp:1-1301`

**Correction:** GAP_REPORT's "628" is the raw `grep` count of `s->addParam()` calls in the file. It does not account for:
- `addTorusControls()` helper function called on 7 torus sources, adding 12 params each (only counted once in grep, adds 72 runtime params)
- `registerWireframe()` helper called 7 times with 9 params each (only counted once in grep, adds 54 runtime params)
- Runtime total: 628 - 12 (torus body) - 9 (wireframe body) + (12x7) + (9x7) = **754 parameters**

GAP_REPORT's "101 sources" is also incorrect -- the FEATURES.md correctly states 108.

### 5.1 Breakdown by Category (18 categories)

| Category | Sources | Parameters | Avg Params/Source |
|----------|---------|------------|-------------------|
| 3D | 24 | 298 | 12.4 |
| Wireframe | 7 | 63 | 9.0 |
| Lines | 11 | 65 | 5.9 |
| Geometric | 11 | 60 | 5.5 |
| Fractal | 7 | 54 | 7.7 |
| Math | 8 | 45 | 5.6 |
| Audio-Visual | 9 | 42 | 4.7 |
| Pattern | 8 | 40 | 5.0 |
| Nature | 6 | 31 | 5.2 |
| Simulation | 3 | 18 | 6.0 |
| Noise | 3 | 12 | 4.0 |
| Text | 2 | 12 | 6.0 |
| Particle | 3 | 12 | 4.0 |
| Utility | 2 | 11 | 5.5 |
| Lighting | 1 | 6 | 6.0 |
| Organic | 1 | 5 | 5.0 |
| Routing | 1 | 1 | 1.0 |
| MilkDrop | 1 | 0 | 0.0 |
| **Total** | **108** | **754** | **7.0** |

### 5.2 Sources by Category

**3D (24 sources, 298 params):**
Includes 8 ray-marched fractals, 8 torus/tunnel variants, 3 3D P17 sources, and 5 P17/P19 3D sources.

- Mandelbulb (16), Menger Sponge (14), Kaleidoscopic IFS (16), Julia Set 3D (17), Burning Ship 3D (14), Newton 3D (14), Sierpinski Tetrahedron (13), Apollonian 3D (14)
- Striped Torus (14), Spiral Vortex (14), Checker Torus (14), Ribbed Vortex (14), Wormhole Tunnel (13), Twisted Torus (14), Wormhole (14), Torus Hole (19)
- Spiral Tunnel (5), Crystal Cavern (6), Infinite Corridor (6), Orbit Chamber (7)
- Scroll Plane (5), Rotating Cube Map (5), Dual Plane Drift (5), DNA Helix (4)

**Audio-Visual (9 sources, 42 params):**
Audio Waveform (4), Spectrum Landscape (5), Chromatic Ring (4), Band Tower (6), Timbral Nebula (6), Structural Landscape (6), Cymatics (3), Spectral Waterfall (3), Spectral Ring (5)

**Fractal (7 sources, 54 params):**
Kaleidoscopic Fractal (6), Mandelbrot / Julia (11), Julia Set (9), Burning Ship (9), Newton Fractal (6), Sierpinski (7), Apollonian Gasket (6)

**Geometric (11 sources, 60 params):**
Geometric Tunnel (4), Color Gradient (4), Shape Generator (5), Infinite Zoom (4), Moire Interference (7), Astral Grid (6), Radial Burst (7), Hex Grid (6), Sacred Geometry (7), Radar Sweep (4), Dot Matrix Wave (6)

**Lines (11 sources, 65 params):**
Zigzag Lines (5), Star Burst (5), Polygon Lines (6), Waveform Lines (7), Lissajous (6), Spirograph (5), Angular Grid (6), Fractal Tree (6), Laser Scan (5), Moire Lines (5), Line Generator (9)

**Math (8 sources, 45 params):**
Lissajous Weaver (7), Fermat Spiral Garden (6), Hyperbolic Tiling (6), Penrose Pulse (5), Superformula (6), Truchet Labyrinth (5), Rose Curves (5), Fibonacci Spiral (5)

**Nature (6 sources, 31 params):**
Reaction-Diffusion (4, stateful), Cellular Automata (5, stateful), Fire Wall (6), Water Caustics (6), Electric Arc (6), Fire (4)

**Noise (3 sources, 12 params):**
Perlin Noise (4), Plasma (4), Voronoi (4)

**Organic (1 source, 5 params):**
Metaballs (5)

**Particle (3 sources, 12 params):**
Lightning Storm (4), Starfield (4), Particle Nebula (4)

**Pattern (8 sources, 40 params):**
Checkerboard (4), Line Pattern (5), Concentric Rings (5), Sine Oscillator (6), Spiral Pattern (5), Terrain Lines (6), Bump Light (5), Glitch Grid (4)

**Routing (1 source, 1 param):**
Layer Router (1) -- routes another layer's output as source content

**Simulation (3 sources, 18 params):**
Strange Attractor (6, stateful), Gravity Well (6, stateful), Fluid Dynamics (6, stateful)

**Text (2 sources, 12 params):**
Scrolling Text Wall (4), Text Animator (8)

**Utility (2 sources, 11 params):**
Solid Color (3), Strobe Light (8)

**Wireframe (7 sources, 63 params):**
Wireframe Sphere (9), Wireframe Torus (9), Wireframe Cube (9), Wireframe Cylinder (9), Wireframe Cone (9), Wireframe Icosahedron (9), Wireframe Wolf (9)

**MilkDrop (1 source, 0 standard params):**
MilkDrop Visualizer -- implemented via ProjectMSource (separate class inheriting ProceduralSource), not standard addParam. Uses libprojectM-4 with its own preset system (~9800 presets).

**Lighting (1 source, 6 params):**
Laser Scanner (6)

---

## 6. TimingWindow

**GAP_REPORT says:** 3 tabs (BPM/Routing/Oscillators) are placeholder
**Actual code:** CONFIRMED -- pure placeholder, content is a centered text label showing the tab name
**Source:** `src/ui/TimingWindow.h:1-32`, `src/ui/TimingWindow.cpp:1-99`

### 6.1 What Exists

- Component class with 3 tab buttons: "BPM", "Routing", "Oscillators"
- Tab switching via `setActiveTab()` -- updates button colors and accent line
- Active tab indicator: 3px cyan accent line under active tab
- Tab bar height: 26px

### 6.2 What Is Stub

- **All 3 tabs render only a placeholder text label** (the tab name in secondary text at 40% opacity, centered in content area). Source: `TimingWindow.cpp:64-65`.
- No child components for any tab content
- No connection to BPMTracker, RoutingEngine, or OscillatorSignal
- No parameter controls, no routing UI, no oscillator configuration
- The component is purely structural (tab switching works) but functionally empty

### 6.3 Intended Purpose (from architecture docs)

- **BPM tab:** Should display detected BPM, beat phase visualization, manual BPM override, tap tempo, Link sync status
- **Routing tab:** Should display signal routing configuration, ChainedSignal editor, route creation/deletion
- **Oscillators tab:** Should display modulation signal configuration (OscillatorSignal wave shape, frequency, phase; EnvelopeSignal attack/decay)

---

## Corrections to GAP_REPORT

| Item | GAP_REPORT Value | Verified Value | Notes |
|------|-----------------|----------------|-------|
| Curve types | 24 | 24 | Correct |
| Hidden signals | 25 | 21 | GAP_REPORT overcounts by 4 |
| Total signals | 36 | 32 | 8 visible + 21 hidden + 2 modulation + 1 clip position |
| Modulation signals | 3 | 2 + 1 | 2 modulation (Mod 1, Mod 2) + 1 clip position (separate type) |
| Mapping sources | 57 | 58 | GAP_REPORT undercounts by 1 |
| Source count | 101 | 108 | GAP_REPORT used Boris's doc count, not code count |
| Source parameters | 628 | 754 | GAP_REPORT counted raw grep, not runtime (missed helper functions) |
| Effect parameters | 333 | 333 | Correct |
| Effect count | 135 | 135 | Correct |
