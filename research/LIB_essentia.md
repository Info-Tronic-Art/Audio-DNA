# Essentia: Complete Technical Reference for Real-Time Audio Analysis

**Scope**: Exhaustive engineering reference for integrating the Essentia library into a real-time VJ / music visualization pipeline.
**Version coverage**: Essentia 2.1-beta6 and later (C++14, Python 3.x bindings).
**Cross-references**: [LIB_aubio.md](LIB_aubio.md) | [LIB_juce.md](LIB_juce.md) | [LIB_fft_comparison.md](LIB_fft_comparison.md) | [FEATURES_spectral.md](FEATURES_spectral.md) | [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) | [ARCH_pipeline.md](ARCH_pipeline.md)

---

## 1. Architecture Overview

Essentia (developed by the Music Technology Group at Universitat Pompeu Fabra, Barcelona) is a dual-mode C++ library for audio analysis and music information retrieval. Understanding its architecture is prerequisite to using it correctly in a real-time context.

### 1.1 Two Execution Modes

Essentia provides two completely separate APIs that share algorithm implementations but differ fundamentally in how data moves:

| Aspect | Standard Mode | Streaming Mode |
|---|---|---|
| Namespace | `essentia::standard` | `essentia::streaming` |
| Execution model | Pull: caller invokes `compute()` per algorithm | Push: scheduler drives data through a connected graph |
| Data passing | Explicit `Input<T>` / `Output<T>` set/get | Port connections, automatic buffering |
| Control flow | Procedural, caller manages order | Declarative, topology determines order |
| Real-time suitability | Good (explicit control) | Good (built-in dataflow) |
| Boilerplate | More verbose per-frame | More setup, less per-frame code |

**Standard mode** wraps each algorithm in a stateless `compute()` call. You allocate input/output vectors, set them on the algorithm, call `compute()`, and read outputs. This is the simpler model and maps naturally onto an audio callback where you already have a processing loop.

**Streaming mode** builds a dataflow graph. Algorithms are connected port-to-port, and a scheduler pushes data from sources through the graph. This is closer to how visual dataflow systems (Max/MSP, Pure Data) operate and is well-suited for complex analysis chains where you want Essentia to manage buffer sizes and processing order.

### 1.2 Algorithm Base Class Hierarchy

```
essentia::Algorithm (abstract base)
├── essentia::standard::Algorithm
│   ├── declareInput<T>(name)
│   ├── declareOutput<T>(name)
│   └── compute()              // pure virtual
└── essentia::streaming::Algorithm
    ├── AlgorithmComposite      // wraps standard algos for streaming
    ├── declareInputStream<T>()
    ├── declareOutputStream<T>()
    └── process()              // called by scheduler
```

Every algorithm self-describes its inputs and outputs via `declareParameters()`, `declareInput()`, `declareOutput()`. The `AlgorithmFactory` provides runtime creation by name string:

```cpp
Algorithm* algo = AlgorithmFactory::create("SpectralCentroid",
    "sampleRate", 44100.0);
```

### 1.3 Input/Output Port System

In standard mode, ports are typed containers:

```cpp
// Standard mode: explicit I/O binding
std::vector<Real> spectrum;
Real centroid;

Algorithm* sc = standard::AlgorithmFactory::create("SpectralCentroid",
    "sampleRate", 44100.0);
sc->input("array").set(spectrum);
sc->output("centroid").set(centroid);
sc->compute();
// centroid now holds the value
```

In streaming mode, ports are connected via the `>>` operator or `connect()`:

```cpp
// Streaming mode: port connections
streaming::Algorithm* frameCutter = streaming::AlgorithmFactory::create("FrameCutter", ...);
streaming::Algorithm* windowing = streaming::AlgorithmFactory::create("Windowing", ...);

frameCutter->output("frame") >> windowing->input("frame");
```

The `>>` operator creates a `Connector` that includes a FIFO buffer between producer and consumer. Buffer sizes are managed automatically but can be tuned.

### 1.4 Streaming Mode Dataflow Graph

Data flows through the streaming graph as follows:

1. **Source nodes** (e.g., `VectorInput`, `MonoLoader`, `AudioLoader`) produce tokens (samples or frames).
2. Each token passes through the output port's FIFO buffer to connected input ports.
3. The **scheduler** determines which algorithms are ready to fire (all required input tokens available).
4. When an algorithm fires, it consumes input tokens, runs `process()`, and produces output tokens.
5. **Sink nodes** (e.g., `Pool`, `VectorOutput`, `FileOutput`) collect final results.

The scheduler implements a **topological sort** of the graph and processes algorithms in dependency order. For a linear chain (the most common case in real-time analysis), this is simply left-to-right processing.

**Token model**: Streaming algorithms declare how many tokens they consume and produce per firing. `FrameCutter` consumes `hopSize` audio samples and produces one frame of `frameSize` samples. `Spectrum` consumes one frame and produces one spectrum vector. This token ratio system allows the scheduler to determine when each node is ready.

---

## 2. Streaming Mode for Real-Time Analysis

### 2.1 The Canonical Real-Time Analysis Chain

The fundamental pattern for real-time spectral analysis in Essentia mirrors the standard DSP pipeline:

```
Audio Input → FrameCutter → Windowing → Spectrum → [Descriptors]
```

In streaming mode, this becomes a connected graph:

```cpp
#include <essentia/essentia.h>
#include <essentia/algorithmfactory.h>
#include <essentia/streaming/algorithms/vectorinput.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/scheduler/network.h>
#include <essentia/pool.h>

using namespace essentia;
using namespace essentia::streaming;
using namespace essentia::scheduler;

// Global state for real-time feeding
static std::vector<Real> audioBuffer;
static VectorInput<Real>* vectorInput = nullptr;
static Network* network = nullptr;
static Pool pool;

void buildAnalysisNetwork(int sampleRate, int frameSize, int hopSize) {
    essentia::init();

    // Source: VectorInput wraps an external buffer
    vectorInput = new VectorInput<Real>(&audioBuffer);

    // Frame cutting
    Algorithm* frameCutter = AlgorithmFactory::create("FrameCutter",
        "frameSize", frameSize,
        "hopSize", hopSize,
        "startFromZero", true,
        "silentFrames", "keep");

    // Windowing
    Algorithm* windowing = AlgorithmFactory::create("Windowing",
        "type", "hann",
        "zeroPadding", 0,
        "normalized", false);

    // FFT-based spectrum (magnitude)
    Algorithm* spectrum = AlgorithmFactory::create("Spectrum",
        "size", frameSize);

    // Descriptors
    Algorithm* centroid = AlgorithmFactory::create("SpectralCentroid",
        "sampleRate", (Real)sampleRate);

    Algorithm* mfcc = AlgorithmFactory::create("MFCC",
        "inputSize", frameSize / 2 + 1,
        "sampleRate", (Real)sampleRate,
        "numberCoefficients", 13,
        "numberBands", 40);

    Algorithm* rms = AlgorithmFactory::create("RMS");

    Algorithm* flux = AlgorithmFactory::create("Flux");

    Algorithm* rolloff = AlgorithmFactory::create("RollOff",
        "sampleRate", (Real)sampleRate,
        "cutoff", 0.85);

    // === Connect the graph ===
    vectorInput->output("data")       >> frameCutter->input("signal");
    frameCutter->output("frame")      >> windowing->input("frame");
    windowing->output("frame")        >> spectrum->input("frame");

    // Spectral descriptors branch from spectrum output
    spectrum->output("spectrum")      >> centroid->input("array");
    spectrum->output("spectrum")      >> mfcc->input("spectrum");
    spectrum->output("spectrum")      >> flux->input("spectrum");
    spectrum->output("spectrum")      >> rolloff->input("spectrum");

    // RMS branches from the windowed frame directly
    windowing->output("frame")        >> rms->input("array");

    // Sink all outputs to Pool
    centroid->output("centroid")      >> PC(pool, "spectral.centroid");
    mfcc->output("mfcc")             >> PC(pool, "spectral.mfcc");
    mfcc->output("bands")            >> PC(pool, "spectral.melbands");
    flux->output("flux")             >> PC(pool, "spectral.flux");
    rolloff->output("rollOff")       >> PC(pool, "spectral.rolloff");
    rms->output("rms")               >> PC(pool, "loudness.rms");

    // Build and prepare the network
    network = new Network(vectorInput);
}
```

### 2.2 VectorInput for External Audio Callbacks

`VectorInput<Real>` is the critical adapter for real-time use. It wraps a `std::vector<Real>` and acts as a streaming source. The workflow:

1. Audio callback writes samples into the vector.
2. Call `network->run()` to process them through the graph.
3. Read results from the `Pool`.
4. Clear the pool for the next block.

```cpp
void processAudioBlock(const float* input, int numSamples) {
    // Copy audio callback data into Essentia's buffer
    audioBuffer.assign(input, input + numSamples);

    // Reset the VectorInput to re-read from the beginning
    vectorInput->reset();

    // Clear previous results
    pool.clear();

    // Run the network — processes all frames that fit in this block
    network->run();

    // Read results (pool contains one value per frame that was analyzed)
    if (pool.contains<std::vector<Real>>("spectral.centroid")) {
        const auto& centroids = pool.value<std::vector<Real>>("spectral.centroid");
        // Use last value as "current" centroid, or average the vector
        if (!centroids.empty()) {
            float currentCentroid = centroids.back();
            // Feed to visualization...
        }
    }
}
```

**Critical caveat**: `network->run()` is not lock-free. It allocates internally and is not safe to call directly from an audio thread on systems with strict real-time constraints (CoreAudio, ASIO). The recommended pattern is:

1. Audio callback copies samples into a lock-free ring buffer.
2. A separate analysis thread reads from the ring buffer, runs the Essentia network, and writes results to an atomic/lock-free output structure.
3. The render thread reads the latest results.

See [ARCH_pipeline.md](ARCH_pipeline.md) for the full lock-free architecture.

### 2.3 Network Topology Patterns

**Linear chain** (simplest):
```
Source → FrameCutter → Windowing → Spectrum → Centroid → Pool
```

**Fan-out** (one source, multiple descriptors — the VJ workhorse):
```
                                    ┌→ Centroid → Pool
Source → FrameCutter → Windowing → Spectrum ─┤→ Flux     → Pool
                               │              ├→ MFCC     → Pool
                               │              └→ RollOff  → Pool
                               └→ RMS → Pool
```

**Multi-resolution** (different frame sizes for different features):
```
Source ──┬→ FrameCutter(2048) → Windowing → Spectrum → [spectral descriptors]
         └→ FrameCutter(1024) → Windowing → Spectrum → [onset descriptors]
```

**Parallel chains** (separate networks for different concerns):
```
Network 1 (per-frame):  Source → FrameCutter → Spectrum → fast descriptors
Network 2 (periodic):   Source → FrameCutter → BeatTracker (run every N seconds)
```

### 2.4 Scheduler and Processing Order

Essentia's streaming scheduler operates in two modes:

- **`Network::run()`**: Processes all available data in the source. For `VectorInput`, this means processing the entire vector.
- **`Network::runPrepare()` / `Network::runStep()`**: `runStep()` advances by one scheduler cycle (one frame through the chain). Useful for frame-by-frame control, but less efficient.

Processing order is determined by the network topology. The scheduler performs a breadth-first traversal from sources. Algorithms fire when all input ports have sufficient tokens.

### 2.5 Standard Mode Alternative for Real-Time

For maximum control, standard mode avoids the scheduler overhead entirely:

```cpp
// Pre-create algorithms once
auto* frameCutterAlgo = standard::AlgorithmFactory::create("FrameCutter",
    "frameSize", 2048, "hopSize", 512, "startFromZero", true);
auto* windowAlgo = standard::AlgorithmFactory::create("Windowing",
    "type", "hann");
auto* spectrumAlgo = standard::AlgorithmFactory::create("Spectrum",
    "size", 2048);
auto* centroidAlgo = standard::AlgorithmFactory::create("SpectralCentroid",
    "sampleRate", 44100.0);

// Persistent buffers
std::vector<Real> frame, windowedFrame, spectrumVec;
Real centroidValue;

// Bind once
frameCutterAlgo->input("signal").set(audioBuffer);
frameCutterAlgo->output("frame").set(frame);
windowAlgo->input("frame").set(frame);
windowAlgo->output("frame").set(windowedFrame);
spectrumAlgo->input("frame").set(windowedFrame);
spectrumAlgo->output("spectrum").set(spectrumVec);
centroidAlgo->input("array").set(spectrumVec);
centroidAlgo->output("centroid").set(centroidValue);

// Per-block processing
void processBlock(const float* input, int numSamples) {
    audioBuffer.assign(input, input + numSamples);

    // Manual frame iteration
    frameCutterAlgo->reset();  // reset internal position

    while (true) {
        frameCutterAlgo->compute();
        if (frame.empty()) break;  // FrameCutter signals end with empty frame

        windowAlgo->compute();
        spectrumAlgo->compute();
        centroidAlgo->compute();

        // centroidValue is now valid for this frame
        latestCentroid.store(centroidValue);  // atomic for render thread
    }
}
```

This standard-mode approach gives you explicit control over every step and avoids any hidden allocations from the streaming scheduler.

---

## 3. Algorithm Catalog for VJ Use

### 3.1 Spectral Algorithms

| Algorithm | Key Parameters | Output Type | CPU Cost | Real-Time? | VJ Use Case |
|---|---|---|---|---|---|
| **Spectrum** | `size` (FFT size, power of 2) | `vector<Real>` magnitude spectrum, size N/2+1 | Medium | Yes | Foundation for all spectral features |
| **SpectralCentroid** | `sampleRate` | `Real` (Hz) | Light | Yes | "Brightness" — map to color temperature, particle speed |
| **SpectralFlux** | `halfRectify` (bool) | `Real` (non-negative) | Light | Yes | Spectral change detection — flash/pulse triggers |
| **SpectralRolloff** | `sampleRate`, `cutoff` (0.0-1.0, default 0.85) | `Real` (Hz) | Light | Yes | High-frequency energy boundary — filter effects |
| **SpectralFlatness** | (none) | `Real` (0.0-1.0) | Light | Yes | Noise vs tone — 0=pure tone, 1=white noise; morph textures |
| **SpectralContrast** | `sampleRate`, `numberBands` (default 6), `frameSize` | `vector<Real>` spectral valleys, `vector<Real>` spectral peaks | Light-Medium | Yes | Per-band peak/valley contrast — multi-band visual EQ |
| **SpectralComplexity** | `sampleRate`, `magnitudeThreshold` | `Real` (number of spectral peaks) | Medium | Yes | Harmonic complexity — scene complexity, particle count |

**Spectrum** internally uses Essentia's FFT implementation (which can delegate to FFTW3 or Accelerate on macOS). For a 2048-point FFT at 44.1 kHz, a single `compute()` call takes approximately 15-40 microseconds depending on platform and SIMD support.

**SpectralCentroid** computes the weighted mean of frequencies:

```
centroid = sum(f[i] * mag[i]) / sum(mag[i])
```

Cost is O(N/2) — negligible after the FFT is already done.

**SpectralFlux** computes the L2 norm of the difference between consecutive magnitude spectra. When `halfRectify=true`, only positive differences contribute (onset-sensitive). Requires the algorithm to maintain internal state (previous spectrum).

### 3.2 Rhythm Algorithms

| Algorithm | Key Parameters | Output Type | CPU Cost | Real-Time? | VJ Use Case |
|---|---|---|---|---|---|
| **OnsetDetection** | `method` ("hfc", "complex", "flux", "melflux"), `sampleRate` | `Real` onset detection function value | Medium | Yes (per-frame) | Beat-reactive triggers |
| **BeatTrackerMultiFeature** | `minTempo`, `maxTempo` | `vector<Real>` tick positions (seconds) | **Heavy** | No (offline) | Pre-analyze tracks for tempo map |
| **TempoTapDegara** | `sampleRate`, `maxTempo`, `minTempo` | `vector<Real>` ticks, `Real` tempo | **Heavy** | No (offline) | Accurate tempo extraction |
| **RhythmExtractor2013** | `method` ("multifeature", "degara") | `Real` bpm, `vector<Real>` ticks, `Real` confidence, `vector<Real>` estimates | **Heavy** | No (offline) | Full rhythm analysis |

**OnsetDetection** is the only rhythm algorithm suitable for per-frame real-time use. It outputs a continuous onset detection function (ODF) value. Thresholding this value gives onset events. The `method` parameter selects the detection function:

- `"hfc"` (High Frequency Content): fast, good for percussive onsets. Weights high frequencies.
- `"complex"`: uses spectral phase deviation. Better for pitched onsets but ~2x cost of HFC.
- `"flux"`: spectral flux variant. Good all-rounder.
- `"melflux"`: flux on mel bands. Good for music with dense spectra.

```cpp
// OnsetDetection requires both spectrum AND phase
Algorithm* fft = AlgorithmFactory::create("FFT", "size", 2048);
Algorithm* onset = AlgorithmFactory::create("OnsetDetection",
    "method", "hfc",
    "sampleRate", 44100.0);

// FFT outputs complex spectrum; OnsetDetection takes both magnitude and phase
fft->output("fft")          >> onset->input("spectrum");  // complex
spectrum->output("spectrum") >> onset->input("spectrum");  // magnitude
// Note: OnsetDetection actually takes magnitude spectrum + phase spectrum
```

**Beat tracking algorithms** (`BeatTrackerMultiFeature`, `TempoTapDegara`, `RhythmExtractor2013`) require multiple seconds of audio context and are far too expensive for per-frame processing. Use them for offline pre-analysis or run them periodically (every 5-10 seconds) on a dedicated thread with a large audio buffer. See [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) for the recommended hybrid approach.

### 3.3 Tonal Algorithms

| Algorithm | Key Parameters | Output Type | CPU Cost | Real-Time? | VJ Use Case |
|---|---|---|---|---|---|
| **PitchYin** | `frameSize`, `sampleRate`, `tolerance` (0.0-1.0) | `Real` pitch (Hz), `Real` confidence | Medium | Yes | Dominant pitch — map to hue, note position |
| **PitchYinProbabilistic** | `frameSize`, `sampleRate`, `hopSize`, `lowRMSThreshold` | `vector<Real>` pitches, `vector<Real>` probs | Medium-Heavy | Marginal | Multi-pitch — polyphonic visualization |
| **Key** | `profileType` ("temperley", "krumhansl", "edma") | `string` key, `string` scale, `Real` strength | Heavy | No (needs long context) | Color palette selection per song section |
| **HPCP** | `sampleRate`, `size` (bins, typically 12 or 36), `harmonics` | `vector<Real>` (12 or 36 chroma values) | Medium | Yes (with care) | Chromagram — 12 pitch classes mapped to colors |
| **ChordsDetection** | `hopSize`, `sampleRate`, `windowSize` | `vector<string>` chords, `vector<Real>` strengths | Heavy | No (needs HPCP sequence) | Chord-reactive color schemes |
| **Dissonance** | (none) | `Real` (0.0-1.0) | Medium | Yes | Tension/consonance — visual distortion intensity |

**PitchYin** implements the YIN algorithm (de Cheveigne & Kawahara, 2002). It operates on the time-domain frame directly (no FFT required), using autocorrelation with cumulative mean normalization. Cost is O(N * tau_max) where tau_max corresponds to the lowest detectable frequency. For `frameSize=2048` at 44100 Hz, this detects down to ~43 Hz (sufficient for bass fundamentals). Takes approximately 50-100 microseconds per frame.

**HPCP** (Harmonic Pitch Class Profile) produces a 12-dimensional (or 36-dimensional for finer resolution) chroma vector. Each dimension represents the energy in one pitch class (C, C#, D, ..., B). This is excellent for VJ mapping: assign each pitch class a hue, and the HPCP vector drives a 12-color palette in real time. Requires a magnitude spectrum as input. Cost is moderate — the harmonic summation over spectral peaks is O(P * H) where P is the number of peaks and H is the number of harmonics considered.

### 3.4 Loudness Algorithms

| Algorithm | Key Parameters | Output Type | CPU Cost | Real-Time? | VJ Use Case |
|---|---|---|---|---|---|
| **RMS** | (none) | `Real` | Light | Yes | Global brightness, master intensity |
| **Loudness** | (none — Zwicker model) | `Real` (sone) | Medium | Yes | Perceptual loudness — more natural scaling |
| **LoudnessEBUR128** | `hopSize`, `sampleRate`, `startAtZero` | `Real` momentary, `Real` shortTerm, `Real` integrated | Medium | Yes | Broadcast-standard loudness — smooth VJ master |
| **DynamicComplexity** | `sampleRate`, `frameSize` | `Real` complexity, `Real` loudnessMean | Heavy | No (needs long context) | Classify calm vs dynamic sections |

**RMS** is the simplest and fastest loudness measure. It computes the root-mean-square of the input vector in O(N) with no allocations. Use this as your primary amplitude envelope for driving VJ intensity.

**LoudnessEBUR128** implements the EBU R128 standard with three time scales:
- **Momentary** (400ms window): fast-reacting, good for beat-reactive visuals.
- **Short-term** (3s window): smooth, good for scene transitions.
- **Integrated** (entire program): useful for normalization.

### 3.5 Cepstral Coefficients

| Algorithm | Key Parameters | Output Type | CPU Cost | Real-Time? | VJ Use Case |
|---|---|---|---|---|---|
| **MFCC** | `inputSize`, `sampleRate`, `numberCoefficients` (13), `numberBands` (40) | `vector<Real>` MFCCs, `vector<Real>` mel bands | Medium | Yes | Timbre fingerprint — cluster/classify sounds |
| **GFCC** | `inputSize`, `sampleRate`, `numberCoefficients`, `numberBands` | `vector<Real>` GFCCs, `vector<Real>` gammatone bands | Medium | Yes | Alternative timbre (auditory-model based) |
| **BFCC** | `inputSize`, `sampleRate`, `numberCoefficients`, `numberBands` | `vector<Real>` BFCCs, `vector<Real>` bark bands | Medium | Yes | Bark-scale timbre — perceptual spacing |

**MFCC** is the standard timbre descriptor. The 40 mel bands are themselves valuable for VJ use — they provide a perceptually-spaced frequency decomposition that maps well to a visual EQ or a multi-band color driver. The 13 MFCC coefficients are more useful for machine learning (timbre classification) than for direct visual mapping, though coefficient 1 (overall spectral slope) and coefficient 2 (spectral balance) can drive visual parameters.

The mel filter bank computation dominates MFCC cost: it multiplies a (40 x N/2+1) matrix by the spectrum vector. For N=2048, this is 40 * 1025 multiply-accumulates plus the DCT of 40 values. Total: approximately 30-80 microseconds per frame.

### 3.6 Envelope and Temporal Algorithms

| Algorithm | Key Parameters | Output Type | CPU Cost | Real-Time? | VJ Use Case |
|---|---|---|---|---|---|
| **Envelope** | `sampleRate`, `attackTime`, `releaseTime` | `vector<Real>` envelope signal | Light | Yes | Amplitude follower — smooth visual intensity |
| **LogAttackTime** | `sampleRate`, `startAttackThreshold`, `stopAttackThreshold` | `Real` LAT, `Real` attackStart, `Real` attackStop | Medium | Marginal | Classify transient vs sustained — effect selection |
| **EffectiveDuration** | `sampleRate`, `thresholdRatio` | `Real` (seconds) | Light | Yes | Note duration — animation timing |

**Envelope** implements a simple attack/release follower with configurable time constants. This is the same circuit as an analog envelope follower and is extremely cheap to compute. Prefer this over raw RMS for driving visual parameters, as it provides smooth, configurable tracking.

---

## 4. Performance Classification

### 4.1 CPU Cost Tiers

**Light (< 10 us per frame, safe for every audio callback)**:
- RMS
- ZeroCrossingRate
- SpectralCentroid (post-FFT)
- SpectralFlux (post-FFT)
- SpectralRolloff (post-FFT)
- SpectralFlatness (post-FFT)
- Envelope

**Medium (10-100 us per frame, fine at typical buffer sizes)**:
- Spectrum (FFT): 15-40 us for 2048 points
- MFCC: 30-80 us
- HPCP: 40-100 us
- PitchYin: 50-100 us
- OnsetDetection: 20-60 us
- Dissonance: 30-50 us
- SpectralContrast: 20-40 us
- SpectralComplexity: 20-40 us
- LoudnessEBUR128: 20-50 us

**Heavy (> 1 ms, or requires multi-second context -- avoid in hot path)**:
- BeatTrackerMultiFeature: 50-500 ms per run
- TempoTapDegara: 100-800 ms per run
- RhythmExtractor2013: 200 ms - 2 s per run
- Key detection: 10-50 ms (needs accumulated HPCP)
- ChordsDetection: 20-100 ms per run
- DynamicComplexity: needs full track

Timing estimates are for a single core of Apple M1 or Intel i7-10th gen at 44100 Hz sample rate.

### 4.2 Budget Calculation

At 44100 Hz with a 512-sample buffer, the audio callback fires every **11.6 ms**. A real-time analysis thread (separate from the audio callback) can safely use up to ~8 ms per block without falling behind, assuming it runs at audio-block rate.

Typical VJ analysis budget per block:
```
FFT (2048-pt):          ~30 us
SpectralCentroid:        ~3 us
SpectralFlux:            ~3 us
SpectralRolloff:         ~3 us
SpectralFlatness:        ~2 us
RMS:                     ~2 us
MFCC (13 coeffs):       ~50 us
OnsetDetection (HFC):   ~30 us
PitchYin:               ~70 us
────────────────────────────────
Total:                  ~193 us   (< 2% of 11.6 ms budget)
```

This leaves enormous headroom. You can comfortably run all "Light" and "Medium" algorithms every frame. The bottleneck is never per-frame features; it is the heavy algorithms that need architectural solutions.

### 4.3 Scheduling Heavy Algorithms

For beat tracking, key detection, and chord analysis, use a dedicated background thread:

```cpp
#include <thread>
#include <atomic>
#include <mutex>

class HeavyAnalysisThread {
    std::thread worker;
    std::atomic<bool> running{true};

    // Lock-free ring buffer for accumulated audio
    LockFreeRingBuffer<float> audioRing{44100 * 10};  // 10 seconds

    // Results (protected by atomic or lock-free mechanism)
    std::atomic<float> currentBPM{120.0f};
    std::atomic<int> currentKey{0};  // 0-11 for C-B

    // Essentia algorithms (owned by worker thread)
    standard::Algorithm* beatTracker = nullptr;
    standard::Algorithm* keyDetector = nullptr;

public:
    void start() {
        worker = std::thread([this]() {
            // Create heavy algorithms on worker thread
            beatTracker = standard::AlgorithmFactory::create(
                "RhythmExtractor2013",
                "method", "multifeature");
            keyDetector = standard::AlgorithmFactory::create("Key",
                "profileType", "temperley");

            std::vector<Real> analysisBuffer(44100 * 8);  // 8 seconds

            while (running.load()) {
                // Wait until we have enough audio
                if (audioRing.available() >= analysisBuffer.size()) {
                    audioRing.read(analysisBuffer.data(), analysisBuffer.size());

                    // Run beat tracker
                    Real bpm;
                    std::vector<Real> ticks;
                    Real confidence;
                    std::vector<Real> estimates;
                    std::string rhythmType;

                    beatTracker->input("signal").set(analysisBuffer);
                    beatTracker->output("bpm").set(bpm);
                    beatTracker->output("ticks").set(ticks);
                    beatTracker->output("confidence").set(confidence);
                    beatTracker->output("estimates").set(estimates);
                    beatTracker->compute();

                    if (confidence > 0.5f) {
                        currentBPM.store(bpm);
                    }

                    beatTracker->reset();
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }

            delete beatTracker;
            delete keyDetector;
        });
    }

    void feedAudio(const float* data, int numSamples) {
        audioRing.write(data, numSamples);
    }

    float getBPM() const { return currentBPM.load(); }
    int getKey() const { return currentKey.load(); }

    void stop() {
        running.store(false);
        if (worker.joinable()) worker.join();
    }
};
```

---

## 5. Building from Source Cross-Platform

### 5.1 Dependencies

| Dependency | Purpose | Required? | Version |
|---|---|---|---|
| **FFTW3** | Fast FFT (fallback: built-in KissFFT) | Optional but recommended | 3.3.x |
| **libsamplerate** | Sample rate conversion | Optional | 0.1.9+ |
| **TagLib** | Audio file metadata | Optional (not needed for real-time) | 1.11+ |
| **yaml-cpp** | Configuration file parsing | Optional | 0.6+ |
| **libavcodec/libavformat** (FFmpeg) | Audio file decoding | Optional (not needed for real-time) | 4.x/5.x |
| **Chromaprint** | Audio fingerprinting | Optional (not needed for VJ) | 1.5+ |
| **Eigen3** | Linear algebra (some ML algorithms) | Optional | 3.3+ |
| **TensorFlow Lite** | Neural network models | Optional | 2.x |

For a real-time VJ system that receives audio from a live input (not files), the minimal dependency set is: **FFTW3 only** (or even no external deps if KissFFT suffices).

### 5.2 Build Steps

**macOS (Homebrew)**:
```bash
# Install dependencies
brew install fftw libsamplerate libyaml eigen

# Clone
git clone https://github.com/MTG/essentia.git
cd essentia

# Configure — minimal real-time build
python3 waf configure \
    --mode=release \
    --with-static-examples \
    --build-static \
    --no-avcodec \
    --no-taglib \
    --no-chromaprint \
    --fft=FFTW

# Build
python3 waf build -j$(sysctl -n hw.ncpu)

# Install
sudo python3 waf install
```

**Linux (Ubuntu/Debian)**:
```bash
sudo apt-get install libfftw3-dev libsamplerate0-dev libeigen3-dev

git clone https://github.com/MTG/essentia.git
cd essentia
python3 waf configure --mode=release --build-static --fft=FFTW
python3 waf build -j$(nproc)
sudo python3 waf install
```

**Windows (MSVC + vcpkg)**:
```powershell
vcpkg install fftw3:x64-windows eigen3:x64-windows

git clone https://github.com/MTG/essentia.git
cd essentia

# Essentia uses waf, not CMake natively. For MSVC integration:
# Option 1: Use waf with MSVC
python waf configure --mode=release --build-static --fft=FFTW --prefix=C:\essentia

# Option 2: Use the community CMakeLists.txt
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
cmake --build . --config Release
```

Note: Essentia's primary build system is **waf** (Python-based), not CMake. Community-maintained CMake files exist but may lag behind. The waf build is authoritative.

### 5.3 Linking into a JUCE or Standalone C++ Project

**Static linking** (recommended for deployment):
```cmake
# CMakeLists.txt for a JUCE plugin or standalone app
find_library(ESSENTIA_LIB essentia PATHS /usr/local/lib)
find_path(ESSENTIA_INCLUDE essentia/essentia.h PATHS /usr/local/include)

target_include_directories(MyApp PRIVATE ${ESSENTIA_INCLUDE})
target_link_libraries(MyApp PRIVATE ${ESSENTIA_LIB})

# Also link FFTW if Essentia was built with it
find_library(FFTW3_LIB fftw3f PATHS /usr/local/lib)
target_link_libraries(MyApp PRIVATE ${FFTW3_LIB})
```

**For JUCE specifically**, add to your `CMakeLists.txt`:
```cmake
target_link_libraries(MyJucePlugin
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_processors
        ${ESSENTIA_LIB}
        ${FFTW3_LIB}
)

target_compile_definitions(MyJucePlugin
    PRIVATE
        JUCE_VST3_CAN_REPLACE_VST2=0
)
```

**Static vs dynamic linking**: Static linking bundles Essentia into your binary (larger binary, no runtime dependency). Dynamic linking requires `libessentia.so` / `libessentia.dylib` at runtime. For a VJ application distributed as a standalone binary, static is preferred. For development iteration, dynamic allows faster relinking.

### 5.4 macOS Framework Considerations

On macOS, if building with `--fft=ACCELERATE` instead of FFTW, Essentia uses Apple's Accelerate framework (vDSP FFT). This eliminates the FFTW dependency entirely and leverages Apple's SIMD-optimized FFT. Link with:

```cmake
target_link_libraries(MyApp PRIVATE "-framework Accelerate")
```

This is the recommended configuration for macOS-only VJ applications, as Accelerate's vDSP FFT matches or exceeds FFTW performance on Apple Silicon.

---

## 6. C++ Integration Patterns

### 6.1 Complete Real-Time Analysis Engine

Below is a self-contained class suitable for integration into a VJ application. It uses standard mode for maximum control and separates the analysis thread from the audio callback.

```cpp
#pragma once
#include <essentia/essentia.h>
#include <essentia/algorithmfactory.h>
#include <vector>
#include <atomic>
#include <array>
#include <cstring>

using namespace essentia;
using namespace essentia::standard;

// Lock-free result structure (pod-like, atomically swappable via pointer)
struct AnalysisResult {
    float rms = 0.0f;
    float spectralCentroid = 0.0f;
    float spectralFlux = 0.0f;
    float spectralRolloff = 0.0f;
    float spectralFlatness = 0.0f;
    float pitch = 0.0f;
    float pitchConfidence = 0.0f;
    float onsetValue = 0.0f;
    std::array<float, 40> melBands = {};
    std::array<float, 13> mfcc = {};
    std::array<float, 12> hpcp = {};
};

class EssentiaAnalyzer {
public:
    static constexpr int FRAME_SIZE = 2048;
    static constexpr int HOP_SIZE = 512;

    EssentiaAnalyzer(int sampleRate = 44100) : sampleRate_(sampleRate) {
        essentia::init();
        createAlgorithms();
        allocateBuffers();
        bindPorts();
    }

    ~EssentiaAnalyzer() {
        destroyAlgorithms();
        essentia::shutdown();
    }

    // Call from analysis thread (NOT audio callback)
    AnalysisResult analyze(const float* audio, int numSamples) {
        AnalysisResult result;

        audioVec_.assign(audio, audio + numSamples);

        frameCutter_->reset();
        frameCutter_->input("signal").set(audioVec_);

        while (true) {
            frameCutter_->compute();
            if (frame_.empty()) break;

            // Windowing
            windowing_->compute();

            // Spectrum
            spectrum_->compute();

            // === Light descriptors (all run every frame) ===
            rmsAlgo_->input("array").set(frame_);
            rmsAlgo_->compute();
            result.rms = rmsValue_;

            centroid_->compute();
            result.spectralCentroid = centroidValue_;

            flux_->compute();
            result.spectralFlux = fluxValue_;

            rolloff_->compute();
            result.spectralRolloff = rolloffValue_;

            flatness_->compute();
            result.spectralFlatness = flatnessValue_;

            // === Medium descriptors ===
            mfccAlgo_->compute();
            for (int i = 0; i < 13 && i < (int)mfccCoeffs_.size(); ++i)
                result.mfcc[i] = mfccCoeffs_[i];
            for (int i = 0; i < 40 && i < (int)melBandsVec_.size(); ++i)
                result.melBands[i] = melBandsVec_[i];

            pitchYin_->input("signal").set(frame_);
            pitchYin_->compute();
            result.pitch = pitchValue_;
            result.pitchConfidence = pitchConf_;

            hpcpAlgo_->compute();
            for (int i = 0; i < 12 && i < (int)hpcpVec_.size(); ++i)
                result.hpcp[i] = hpcpVec_[i];
        }

        return result;
    }

private:
    int sampleRate_;

    // Algorithms
    Algorithm* frameCutter_ = nullptr;
    Algorithm* windowing_ = nullptr;
    Algorithm* spectrum_ = nullptr;
    Algorithm* rmsAlgo_ = nullptr;
    Algorithm* centroid_ = nullptr;
    Algorithm* flux_ = nullptr;
    Algorithm* rolloff_ = nullptr;
    Algorithm* flatness_ = nullptr;
    Algorithm* mfccAlgo_ = nullptr;
    Algorithm* pitchYin_ = nullptr;
    Algorithm* hpcpAlgo_ = nullptr;

    // Buffers
    std::vector<Real> audioVec_;
    std::vector<Real> frame_;
    std::vector<Real> windowedFrame_;
    std::vector<Real> spectrumVec_;
    std::vector<Real> mfccCoeffs_;
    std::vector<Real> melBandsVec_;
    std::vector<Real> hpcpVec_;
    Real rmsValue_, centroidValue_, fluxValue_, rolloffValue_, flatnessValue_;
    Real pitchValue_, pitchConf_;

    void createAlgorithms() {
        auto& F = AlgorithmFactory::instance();
        frameCutter_ = F.create("FrameCutter",
            "frameSize", FRAME_SIZE, "hopSize", HOP_SIZE, "startFromZero", true);
        windowing_ = F.create("Windowing", "type", "hann");
        spectrum_ = F.create("Spectrum", "size", FRAME_SIZE);
        rmsAlgo_ = F.create("RMS");
        centroid_ = F.create("SpectralCentroid", "sampleRate", (Real)sampleRate_);
        flux_ = F.create("Flux");
        rolloff_ = F.create("RollOff", "sampleRate", (Real)sampleRate_);
        flatness_ = F.create("FlatnessDB");
        mfccAlgo_ = F.create("MFCC",
            "inputSize", FRAME_SIZE / 2 + 1,
            "sampleRate", (Real)sampleRate_,
            "numberCoefficients", 13,
            "numberBands", 40);
        pitchYin_ = F.create("PitchYin",
            "frameSize", FRAME_SIZE,
            "sampleRate", (Real)sampleRate_);
        hpcpAlgo_ = F.create("HPCP",
            "sampleRate", (Real)sampleRate_,
            "size", 12);
    }

    void allocateBuffers() {
        frame_.resize(FRAME_SIZE);
        windowedFrame_.resize(FRAME_SIZE);
        spectrumVec_.resize(FRAME_SIZE / 2 + 1);
        mfccCoeffs_.resize(13);
        melBandsVec_.resize(40);
        hpcpVec_.resize(12);
    }

    void bindPorts() {
        frameCutter_->output("frame").set(frame_);
        windowing_->input("frame").set(frame_);
        windowing_->output("frame").set(windowedFrame_);
        spectrum_->input("frame").set(windowedFrame_);
        spectrum_->output("spectrum").set(spectrumVec_);

        centroid_->input("array").set(spectrumVec_);
        centroid_->output("centroid").set(centroidValue_);
        flux_->input("spectrum").set(spectrumVec_);
        flux_->output("flux").set(fluxValue_);
        rolloff_->input("spectrum").set(spectrumVec_);
        rolloff_->output("rollOff").set(rolloffValue_);
        flatness_->input("array").set(spectrumVec_);
        flatness_->output("flatnessDB").set(flatnessValue_);
        mfccAlgo_->input("spectrum").set(spectrumVec_);
        mfccAlgo_->output("mfcc").set(mfccCoeffs_);
        mfccAlgo_->output("bands").set(melBandsVec_);
        pitchYin_->output("pitch").set(pitchValue_);
        pitchYin_->output("pitchConfidence").set(pitchConf_);

        // HPCP requires spectral peaks — simplified: feed spectrum directly
        // In practice, SpectralPeaks should precede HPCP
        hpcpAlgo_->input("frequencies").set(spectrumVec_);  // simplified
        hpcpAlgo_->input("magnitudes").set(spectrumVec_);
        hpcpAlgo_->output("hpcp").set(hpcpVec_);
    }

    void destroyAlgorithms() {
        delete frameCutter_;
        delete windowing_;
        delete spectrum_;
        delete rmsAlgo_;
        delete centroid_;
        delete flux_;
        delete rolloff_;
        delete flatness_;
        delete mfccAlgo_;
        delete pitchYin_;
        delete hpcpAlgo_;
    }
};
```

### 6.2 Integration with Audio Callback (Lock-Free Pattern)

```cpp
#include <atomic>
#include <thread>

class RealTimeAnalysisBridge {
    // Triple buffer for lock-free audio → analysis thread transfer
    static constexpr int BLOCK_SIZE = 2048;
    std::array<std::array<float, BLOCK_SIZE>, 3> buffers_;
    std::atomic<int> writeIndex_{0};
    std::atomic<int> readIndex_{1};
    std::atomic<int> freeIndex_{2};
    std::atomic<bool> newDataReady_{false};

    // Triple buffer for analysis → render thread transfer
    std::array<AnalysisResult, 3> results_;
    std::atomic<int> resultWriteIdx_{0};
    std::atomic<int> resultReadIdx_{1};
    std::atomic<int> resultFreeIdx_{2};
    std::atomic<bool> newResultReady_{false};

    EssentiaAnalyzer analyzer_;
    std::thread analysisThread_;
    std::atomic<bool> running_{true};

public:
    // Called from audio callback (lock-free, O(1))
    void pushAudio(const float* data, int numSamples) {
        int idx = freeIndex_.load();
        std::memcpy(buffers_[idx].data(), data,
                     std::min(numSamples, BLOCK_SIZE) * sizeof(float));
        freeIndex_.store(writeIndex_.exchange(idx));
        newDataReady_.store(true);
    }

    // Called from render thread (lock-free, O(1))
    AnalysisResult getLatestResult() {
        if (newResultReady_.exchange(false)) {
            resultFreeIdx_.store(resultReadIdx_.exchange(resultWriteIdx_.load()));
        }
        return results_[resultReadIdx_.load()];
    }

    void startAnalysisThread() {
        analysisThread_ = std::thread([this]() {
            while (running_.load()) {
                if (newDataReady_.exchange(false)) {
                    int idx = readIndex_.exchange(writeIndex_.load());
                    // Run analysis (not lock-free internally, but on its own thread)
                    AnalysisResult result = analyzer_.analyze(
                        buffers_[idx].data(), BLOCK_SIZE);

                    // Publish result
                    int rIdx = resultFreeIdx_.load();
                    results_[rIdx] = result;
                    resultFreeIdx_.store(resultWriteIdx_.exchange(rIdx));
                    newResultReady_.store(true);
                } else {
                    std::this_thread::sleep_for(std::chrono::microseconds(500));
                }
            }
        });
    }

    void stop() {
        running_.store(false);
        if (analysisThread_.joinable()) analysisThread_.join();
    }
};
```

### 6.3 JUCE AudioProcessor Integration Sketch

```cpp
class MyVJProcessor : public juce::AudioProcessor {
    RealTimeAnalysisBridge analysisBridge;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        analysisBridge.startAnalysisThread();
    }

    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer&) override {
        // Mix to mono and push to analysis
        const float* left = buffer.getReadPointer(0);
        analysisBridge.pushAudio(left, buffer.getNumSamples());
    }

    void releaseResources() override {
        analysisBridge.stop();
    }
};

// In the render/UI thread:
void renderFrame() {
    AnalysisResult r = analysisBridge.getLatestResult();
    // Use r.rms, r.spectralCentroid, r.melBands, etc. to drive visuals
}
```

---

## 7. Python Bindings

Essentia's Python bindings are excellent for prototyping analysis chains before porting to C++. They mirror both standard and streaming modes.

### 7.1 Installation

```bash
pip install essentia        # PyPI (pre-built wheels for Linux/macOS)
# or
pip install essentia-tensorflow  # includes TF model support
```

### 7.2 Standard Mode in Python

```python
import essentia
import essentia.standard as es
import numpy as np

# Load audio (or receive from sounddevice callback)
audio = es.MonoLoader(filename='track.wav', sampleRate=44100)()

# Create algorithms
frame_size = 2048
hop_size = 512
w = es.Windowing(type='hann')
spectrum = es.Spectrum(size=frame_size)
centroid = es.SpectralCentroid(sampleRate=44100)
mfcc = es.MFCC(inputSize=frame_size // 2 + 1, sampleRate=44100,
                numberCoefficients=13, numberBands=40)
pitch = es.PitchYin(frameSize=frame_size, sampleRate=44100)
onset = es.OnsetDetection(method='hfc', sampleRate=44100)
rms_algo = es.RMS()

# Frame-by-frame analysis
results = []
for start in range(0, len(audio) - frame_size, hop_size):
    frame = audio[start:start + frame_size]
    windowed = w(frame)
    spec = spectrum(windowed)

    c = centroid(spec)
    mfcc_bands, mfcc_coeffs = mfcc(spec)
    p, p_conf = pitch(frame)
    o = onset(spec, spec)  # magnitude, phase (simplified)
    r = rms_algo(frame)

    results.append({
        'centroid': c,
        'mfcc': mfcc_coeffs,
        'mel_bands': mfcc_bands,
        'pitch': p,
        'pitch_confidence': p_conf,
        'onset': o,
        'rms': r,
    })
```

### 7.3 Real-Time Python Prototype with sounddevice

```python
import essentia.standard as es
import sounddevice as sd
import numpy as np
from collections import deque

SAMPLE_RATE = 44100
FRAME_SIZE = 2048
HOP_SIZE = 512
BLOCK_SIZE = 512

# Pre-create algorithms
w = es.Windowing(type='hann')
spec = es.Spectrum(size=FRAME_SIZE)
centroid = es.SpectralCentroid(sampleRate=SAMPLE_RATE)
rms_algo = es.RMS()
mfcc_algo = es.MFCC(inputSize=FRAME_SIZE // 2 + 1,
                     sampleRate=SAMPLE_RATE)

# Ring buffer for accumulating frames
ring = deque(maxlen=FRAME_SIZE)
ring.extend([0.0] * FRAME_SIZE)

latest = {'rms': 0, 'centroid': 0, 'mel_bands': np.zeros(40)}

def audio_callback(indata, frames, time_info, status):
    global latest
    mono = indata[:, 0].astype(np.float32)
    ring.extend(mono)

    frame = np.array(ring, dtype=np.float32)
    windowed = w(frame)
    s = spec(windowed)

    latest['rms'] = float(rms_algo(frame))
    latest['centroid'] = float(centroid(s))
    bands, coeffs = mfcc_algo(s)
    latest['mel_bands'] = bands

with sd.InputStream(samplerate=SAMPLE_RATE, channels=1,
                     blocksize=BLOCK_SIZE, callback=audio_callback):
    print("Analyzing... Press Ctrl+C to stop")
    import time
    while True:
        print(f"RMS={latest['rms']:.4f}  Centroid={latest['centroid']:.0f} Hz")
        time.sleep(0.05)
```

### 7.4 Streaming Mode in Python

```python
import essentia
import essentia.streaming as ess
from essentia import Pool

# Create algorithms
loader = ess.MonoLoader(filename='track.wav', sampleRate=44100)
fc = ess.FrameCutter(frameSize=2048, hopSize=512)
w = ess.Windowing(type='hann')
spec = ess.Spectrum(size=2048)
centroid = ess.SpectralCentroid(sampleRate=44100)

pool = Pool()

# Connect
loader.audio >> fc.signal
fc.frame >> w.frame
w.frame >> spec.frame
spec.spectrum >> centroid.array
centroid.centroid >> (pool, 'centroid')

# Run entire network
essentia.run(loader)

# Results
print(f"Mean centroid: {np.mean(pool['centroid']):.1f} Hz")
print(f"Frames analyzed: {len(pool['centroid'])}")
```

### 7.5 Validation Workflow

The recommended workflow for VJ development:

1. **Prototype in Python**: Test algorithm combinations, tune parameters, visualize outputs with matplotlib.
2. **Validate against reference**: Compare Python Essentia output against known-correct values or aubio (see [LIB_aubio.md](LIB_aubio.md)).
3. **Port to C++**: Translate the algorithm chain to C++ using the patterns in Section 6.
4. **Cross-validate**: Run the same audio through both Python and C++ implementations, assert outputs match within floating-point tolerance.

```python
# Validation: compare Python vs C++ output
import json
import numpy as np

py_results = pool['centroid']
cpp_results = np.array(json.load(open('cpp_centroids.json')))

max_diff = np.max(np.abs(py_results - cpp_results))
assert max_diff < 0.01, f"Mismatch: max diff = {max_diff}"
print(f"Validated: max difference = {max_diff:.6f}")
```

---

## Appendix A: Quick Reference — Algorithm Selection by VJ Task

| Visual Effect | Primary Feature | Essentia Algorithm | Typical Mapping |
|---|---|---|---|
| Global brightness/intensity | Loudness | RMS, LoudnessEBUR128 | Linear or log scale to opacity/brightness |
| Beat pulse / flash | Onset strength | OnsetDetection (HFC) | Threshold → trigger |
| Color temperature | Brightness | SpectralCentroid | Low Hz → warm, high Hz → cool |
| Texture roughness | Noisiness | SpectralFlatness | 0 (tone) → smooth, 1 (noise) → rough |
| Multi-band EQ bars | Frequency decomposition | MFCC (mel bands output) | 40 bands → 40 bar heights |
| Chromatic color wheel | Pitch class distribution | HPCP | 12 values → 12 hues |
| Scene complexity | Harmonic density | SpectralComplexity | More peaks → more particles / layers |
| Distortion / glitch | Dissonance | Dissonance | High dissonance → visual distortion |
| Animation speed | Tempo | BeatTrackerMultiFeature (offline) | BPM → base animation rate |
| Transition triggers | Spectral change | SpectralFlux | Large flux → scene change |

## Appendix B: Essentia vs Aubio Feature Comparison

| Feature | Essentia | Aubio |
|---|---|---|
| Algorithm count | ~200 | ~20 |
| Spectral descriptors | Comprehensive (15+) | Basic (centroid, flux) |
| Beat tracking | Multiple methods, high accuracy | Good, lower latency |
| Pitch detection | YIN, probabilistic, melodia | YIN, YINfast, spectral |
| Tonal (key, chords) | Full support | Not available |
| MFCCs | Yes (MFCC, GFCC, BFCC) | Yes (MFCC only) |
| Real-time streaming | Streaming mode with scheduler | Native real-time design |
| Build complexity | Medium-high (waf, many deps) | Low (simple autotools/cmake) |
| Binary size (static) | ~5-15 MB | ~200 KB |
| Python bindings | Full coverage | Full coverage |
| License | AGPL-3.0 (or commercial) | GPL-3.0 |

See [LIB_aubio.md](LIB_aubio.md) for detailed aubio coverage and [LIB_fft_comparison.md](LIB_fft_comparison.md) for FFT backend performance comparisons.

## Appendix C: Licensing

Essentia is dual-licensed:
- **AGPL-3.0**: Free for open-source projects. Any software using Essentia must also be AGPL.
- **Commercial license**: Available from MTG/UPF for proprietary applications.

For a commercial VJ application, you will need either the commercial license or to ensure your application complies with AGPL (which requires distributing source code). This is a significant consideration compared to aubio (GPL-3.0, also copyleft but with clearer linking exceptions in some interpretations) or writing your own feature extractors. See [ARCH_pipeline.md](ARCH_pipeline.md) for discussion of build-vs-buy tradeoffs.
