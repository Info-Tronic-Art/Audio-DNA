# Minimal Working Prototype: Real-Time Audio Feature Extraction

**Document ID:** IMPL_minimal_prototype
**Status:** Active
**Cross-references:** [ARCH_pipeline.md](ARCH_pipeline.md) | [ARCH_audio_io.md](ARCH_audio_io.md) | [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md) | [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md) | [LIB_fft_comparison.md](LIB_fft_comparison.md) | [FEATURES_spectral.md](FEATURES_spectral.md) | [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md) | [IMPL_project_setup.md](IMPL_project_setup.md)

---

## 1. Goal

Build the "hello world" of real-time audio analysis: a single C++ program that opens the default audio capture device, runs a 2048-point FFT on incoming audio, extracts a core feature set, and prints those features to the console at 30 frames per second. The feature set:

| Feature | Description |
|---------|-------------|
| RMS | Root-mean-square amplitude of the analysis block |
| Spectral Centroid | Brightness measure -- weighted average frequency |
| 7 Frequency Bands | Sub-bass through brilliance energy distribution |
| Onset Detection | Spectral-flux-based transient detector with adaptive threshold |

The prototype must compile and run on Windows, macOS, and Linux with no platform-specific code in the application layer. All platform abstraction is delegated to libraries.

This prototype directly implements the first two stages of the pipeline described in [ARCH_pipeline.md](ARCH_pipeline.md): the audio callback pushes samples into a lock-free SPSC ring buffer, and the analysis thread pulls blocks, windows them, computes the FFT, and extracts features. The third stage (render thread) is replaced by a simple console print loop. The constraints documented in [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md) -- no allocation, no locks, no I/O in the audio callback -- are followed rigorously.

---

## 2. Library Choices

### 2.1 miniaudio -- Audio I/O

**Repository:** https://github.com/mackron/miniaudio
**License:** Public domain / MIT-0
**Integration:** Single header file (`miniaudio.h`, ~90,000 lines)

miniaudio abstracts the platform audio APIs behind a unified C API. On macOS it uses CoreAudio, on Windows it uses WASAPI (with DirectSound and WinMM fallbacks), and on Linux it uses PulseAudio, ALSA, or JACK. The library handles device enumeration, format conversion, sample rate conversion, and channel mapping internally.

Why miniaudio over alternatives:

- **Zero external dependencies.** RtAudio requires ALSA/Pulse development headers on Linux. PortAudio requires a separate build step and shared library distribution. miniaudio compiles entirely from one header.
- **Callback-based API** that matches the real-time pipeline model. The callback signature gives us raw float samples in the format we need.
- **Capture device support** is first-class. Some lightweight audio libraries focus on playback only.
- **Public domain license** means no attribution requirements, no license compatibility concerns. This matters for a prototype that may evolve into a commercial product.
- **Active maintenance.** The library tracks OS audio API changes (e.g., AAudio on Android, WASAPI low-latency on Windows 10+).

The primary trade-off is that miniaudio is C, not C++. This is acceptable -- the callback interface is simple, and we wrap it in a thin C++ layer.

### 2.2 KissFFT -- Fast Fourier Transform

**Repository:** https://github.com/mborgerding/kissfft
**License:** BSD 3-Clause
**Integration:** Two files (`kiss_fft.h`, `kiss_fft.c`) plus the real-FFT extension (`kiss_fftr.h`, `kiss_fftr.c`)

KissFFT is a small, portable FFT library that handles real-input FFTs efficiently. For a 2048-point real FFT, it returns 1025 complex bins. Performance is adequate for real-time analysis: a 2048-point FFT completes in ~15 microseconds on modern hardware, well within our per-frame budget.

Why KissFFT over alternatives:

- **FFTW** is faster (3-5x for large transforms) but requires a complex build system, has a GPL license (or paid commercial license), and is overkill for a 2048-point transform that runs once per analysis frame.
- **pffft** is faster than KissFFT (~2x) but requires SSE/NEON and has less portable build requirements.
- **muFFT** requires Vulkan-style memory management semantics.
- **Accelerate (vDSP)** is macOS-only.

KissFFT is the right choice for a minimal prototype: it compiles everywhere, has no SIMD requirements, and the performance penalty versus FFTW is irrelevant at our transform size. When we move to production, we can swap in pffft or FFTW behind the same interface (see Section 10).

### 2.3 Custom Onset Detection -- Spectral Flux

Onset detection uses spectral flux: the sum of positive differences between consecutive magnitude spectra. This is a well-established algorithm (Bello et al., 2005) that requires no external library -- just the magnitude spectrum we already compute for band energies. An adaptive threshold (running median + constant offset) handles varying dynamics without manual tuning.

We avoid pulling in Aubio or Essentia at this stage because either library would multiply our dependency count by 10x and introduce build complexity that obscures the prototype's purpose. The spectral flux implementation is ~30 lines of C++.

---

## 3. Complete Source Code

### 3.1 File Structure

```
minimal_prototype/
  CMakeLists.txt
  main.cpp
  ring_buffer.h
  ext/
    miniaudio.h        (download from GitHub)
    kiss_fft.h         (download from GitHub)
    kiss_fft.c
    kiss_fftr.h
    kiss_fftr.c
```

### 3.2 ring_buffer.h -- Lock-Free SPSC Ring Buffer

This ring buffer is the critical communication path between the audio callback (producer) and the analysis thread (consumer). As documented in [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md), the audio callback must never block. This implementation is wait-free for both producer and consumer: `push` writes samples and advances the write index atomically; `pop` reads samples and advances the read index atomically. Neither operation can fail in a way that requires retry or waiting (assuming the buffer is not full/empty).

The key correctness properties:

- **Single producer, single consumer (SPSC).** Only the audio callback writes; only the analysis thread reads. This avoids ABA problems and the need for CAS loops.
- **Acquire/release memory ordering.** The write index is stored with `memory_order_release` after writing data, ensuring the consumer sees the data before it sees the updated index. The read index is loaded with `memory_order_acquire` before reading data.
- **Power-of-two capacity.** Index masking with `capacity_ - 1` replaces modulo, which would invoke integer division (expensive, and technically not wait-free on all architectures due to microcode).
- **No false sharing.** The read and write indices are separated by a cache-line padding to prevent the producer and consumer from bouncing the same cache line between cores. On x86, a cache line is 64 bytes. On Apple Silicon, it is 128 bytes. We use 128 to be safe on all platforms.

```cpp
// ring_buffer.h -- Lock-free SPSC ring buffer for real-time audio
//
// Producer (audio callback): push() is wait-free.
// Consumer (analysis thread): pop() is wait-free.
// Capacity MUST be a power of two.

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <atomic>
#include <cstddef>
#include <cstring>

// Cache line size. 128 bytes covers both x86 (64B) and Apple Silicon (128B).
static constexpr size_t CACHE_LINE = 128;

template <typename T>
class RingBuffer {
public:
    // capacity must be a power of two. If it is not, it is rounded up.
    explicit RingBuffer(size_t capacity) {
        // Round up to next power of two
        size_t cap = 1;
        while (cap < capacity) cap <<= 1;
        capacity_ = cap;
        mask_ = cap - 1;
        buffer_ = new T[cap];
    }

    ~RingBuffer() {
        delete[] buffer_;
    }

    // Non-copyable, non-movable (contains raw pointer and atomics)
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    // Push a contiguous block of samples. Returns the number of samples
    // actually written. If the buffer is full, excess samples are dropped
    // (the audio callback cannot wait).
    size_t push(const T* data, size_t count) {
        const size_t writePos = writeIndex_.load(std::memory_order_relaxed);
        const size_t readPos  = readIndex_.load(std::memory_order_acquire);

        const size_t available = capacity_ - (writePos - readPos);
        const size_t toWrite   = (count < available) ? count : available;

        // Write in up to two segments (wrap-around)
        const size_t writeOffset = writePos & mask_;
        const size_t firstChunk  = capacity_ - writeOffset;

        if (toWrite <= firstChunk) {
            std::memcpy(buffer_ + writeOffset, data, toWrite * sizeof(T));
        } else {
            std::memcpy(buffer_ + writeOffset, data, firstChunk * sizeof(T));
            std::memcpy(buffer_, data + firstChunk,
                        (toWrite - firstChunk) * sizeof(T));
        }

        // Release: ensure data is visible before index advances
        writeIndex_.store(writePos + toWrite, std::memory_order_release);
        return toWrite;
    }

    // Pop a contiguous block of samples into the destination buffer.
    // Returns the number of samples actually read.
    size_t pop(T* dest, size_t count) {
        const size_t readPos  = readIndex_.load(std::memory_order_relaxed);
        const size_t writePos = writeIndex_.load(std::memory_order_acquire);

        const size_t available = writePos - readPos;
        const size_t toRead    = (count < available) ? count : available;

        const size_t readOffset = readPos & mask_;
        const size_t firstChunk = capacity_ - readOffset;

        if (toRead <= firstChunk) {
            std::memcpy(dest, buffer_ + readOffset, toRead * sizeof(T));
        } else {
            std::memcpy(dest, buffer_ + readOffset, firstChunk * sizeof(T));
            std::memcpy(dest + firstChunk, buffer_,
                        (toRead - firstChunk) * sizeof(T));
        }

        // Release: ensure reads complete before index advances
        readIndex_.store(readPos + toRead, std::memory_order_release);
        return toRead;
    }

    // Number of samples available for reading.
    size_t availableRead() const {
        return writeIndex_.load(std::memory_order_acquire)
             - readIndex_.load(std::memory_order_relaxed);
    }

private:
    T*     buffer_   = nullptr;
    size_t capacity_ = 0;
    size_t mask_     = 0;

    // Pad to separate cache lines. Each atomic is on its own cache line
    // to prevent false sharing between producer and consumer cores.
    alignas(CACHE_LINE) std::atomic<size_t> writeIndex_{0};
    alignas(CACHE_LINE) std::atomic<size_t> readIndex_{0};
};

#endif // RING_BUFFER_H
```

**Design notes:**

The indices are `size_t` and monotonically increasing. They never wrap -- they are masked only when used as array offsets. This eliminates the "is the buffer full or empty?" ambiguity that plagues ring buffers with wrapping indices. The difference `writeIndex_ - readIndex_` always gives the exact number of samples in the buffer, even after the indices have overflowed (because unsigned overflow is well-defined in C++ and the subtraction still produces the correct delta, provided the buffer capacity is less than `SIZE_MAX / 2`).

For a 48 kHz mono stream, `size_t` overflow would take ~12 million years. We will not worry about it.

---

### 3.3 main.cpp -- Complete Application

```cpp
// main.cpp -- Minimal real-time audio feature extraction prototype
//
// Opens the default audio capture device, runs FFT on incoming audio,
// extracts spectral features, and prints them to the console at 30fps.
//
// Dependencies: miniaudio.h, kiss_fft.h/c, kiss_fftr.h/c
// Build: see CMakeLists.txt or single-file instructions below.

// ---------------------------------------------------------------------------
// miniaudio configuration: we only need capture, not playback.
// Define MINIAUDIO_IMPLEMENTATION exactly once, in exactly one .cpp file.
// ---------------------------------------------------------------------------
#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_ENCODING       // We don't encode audio to files
#define MA_NO_GENERATION     // We don't need waveform generators
#include "ext/miniaudio.h"

// KissFFT
#include "ext/kiss_fft.h"
#include "ext/kiss_fftr.h"

// Ring buffer
#include "ring_buffer.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <thread>
#include <vector>
#include <algorithm>
#include <numeric>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

// Audio capture parameters
static constexpr int    SAMPLE_RATE     = 48000;
static constexpr int    CHANNELS        = 1;  // Mono capture simplifies everything
static constexpr ma_format SAMPLE_FORMAT = ma_format_f32;

// FFT parameters
static constexpr int    FFT_SIZE        = 2048;  // ~42.7ms window at 48kHz
static constexpr int    FFT_BINS        = FFT_SIZE / 2 + 1;  // 1025 bins
static constexpr float  BIN_WIDTH_HZ    = static_cast<float>(SAMPLE_RATE)
                                          / FFT_SIZE;  // ~23.4 Hz per bin

// Analysis hop. At 48kHz with a 2048-sample FFT, hopping every 1600 samples
// gives us 30 analysis frames per second (48000 / 1600 = 30).
static constexpr int    HOP_SIZE        = 1600;

// Ring buffer capacity: 4x FFT_SIZE gives ~170ms of buffering.
// This absorbs scheduling jitter on the analysis thread without
// dropping audio data from the callback.
static constexpr int    RING_CAPACITY   = FFT_SIZE * 4;  // 8192 samples

// Console output rate
static constexpr double OUTPUT_INTERVAL_MS = 1000.0 / 30.0;  // ~33.3ms

// Onset detection parameters
static constexpr int    ONSET_MEDIAN_WINDOW = 10;  // frames for adaptive threshold
static constexpr float  ONSET_THRESHOLD_MULT = 1.5f;
static constexpr float  ONSET_THRESHOLD_ADD  = 0.002f;

// Frequency band edges (Hz). 7 bands from sub-bass to brilliance.
// These are standard divisions used in audio analysis and equalization.
//
//   Band 0: Sub-bass     20 --  60 Hz   (kick drum fundamentals, bass rumble)
//   Band 1: Bass         60 -- 250 Hz   (bass guitar, low vocals)
//   Band 2: Low-mid     250 -- 500 Hz   (vocal fundamental, warmth)
//   Band 3: Mid         500 --  2k Hz   (vocal presence, snare body)
//   Band 4: Upper-mid     2 --  4k Hz   (vocal clarity, guitar bite)
//   Band 5: Presence      4 --  6k Hz   (sibilance, attack transients)
//   Band 6: Brilliance    6 -- 20k Hz   (air, cymbal shimmer)
//
static constexpr float BAND_EDGES[] = {
    20.0f, 60.0f, 250.0f, 500.0f, 2000.0f, 4000.0f, 6000.0f, 20000.0f
};
static constexpr int NUM_BANDS = 7;

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------

// Shared ring buffer. The audio callback pushes; the analysis thread pops.
static RingBuffer<float> g_ringBuffer(RING_CAPACITY);

// Graceful shutdown flag.
static std::atomic<bool> g_running{true};

// Signal handler for Ctrl+C
static void signalHandler(int) {
    g_running.store(false, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// Feature data structure
//
// This is the "feature bus" concept from ARCH_pipeline.md, simplified to
// a single struct that the analysis thread writes and the main thread reads.
// In a full application, this would be a double-buffered atomic-swap
// structure. For the console prototype, a simple atomic flag suffices
// because the consumer (printf) is fast relative to the producer (FFT).
// ---------------------------------------------------------------------------

struct AudioFeatures {
    float rms;                      // Root mean square amplitude [0, 1]
    float spectralCentroid;         // In Hz
    float bands[NUM_BANDS];         // Energy per frequency band
    bool  onset;                    // True if onset detected this frame
    float spectralFlux;             // Raw spectral flux value
};

// Double buffer for feature data: analysis writes to one, main reads the other.
static AudioFeatures g_features[2];
static std::atomic<int> g_featureReadIndex{0};

static void publishFeatures(const AudioFeatures& f) {
    int writeIdx = 1 - g_featureReadIndex.load(std::memory_order_relaxed);
    g_features[writeIdx] = f;
    g_featureReadIndex.store(writeIdx, std::memory_order_release);
}

static AudioFeatures readFeatures() {
    int readIdx = g_featureReadIndex.load(std::memory_order_acquire);
    return g_features[readIdx];
}

// ---------------------------------------------------------------------------
// Audio callback
//
// Called by miniaudio on a high-priority OS audio thread. This function
// MUST be real-time safe: no allocation, no locks, no I/O, no exceptions.
// See ARCH_realtime_constraints.md for the full rule set.
//
// We simply copy the incoming samples into the ring buffer. All analysis
// happens on a separate thread.
// ---------------------------------------------------------------------------

static void audioCallback(ma_device* device, void* output,
                           const void* input, ma_uint32 frameCount) {
    (void)device;
    (void)output;  // We are capture-only; output is unused.

    const float* samples = static_cast<const float*>(input);

    // push() is wait-free. If the ring buffer is full (analysis thread
    // fell behind), samples are silently dropped. This is the correct
    // behavior: the audio callback must never stall.
    g_ringBuffer.push(samples, frameCount);
}

// ---------------------------------------------------------------------------
// Hann window
//
// Applied before FFT to reduce spectral leakage. The Hann window has good
// frequency resolution (main lobe width 4 bins) and reasonable sidelobe
// suppression (-31 dB first sidelobe). For music analysis, it is the
// standard choice -- better frequency resolution than Hamming, better
// sidelobe suppression than rectangular.
//
// We precompute the window coefficients once at startup.
// ---------------------------------------------------------------------------

static float g_hannWindow[FFT_SIZE];

static void initHannWindow() {
    for (int i = 0; i < FFT_SIZE; ++i) {
        // The "periodic" Hann window (not "symmetric"). For FFT analysis,
        // the periodic variant is correct because it makes the window
        // exactly periodic over the FFT length, which means the DFT of
        // the window is purely real and non-negative.
        g_hannWindow[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / FFT_SIZE));
    }
}

// ---------------------------------------------------------------------------
// Analysis thread
//
// Runs continuously, pulling FFT_SIZE samples from the ring buffer,
// computing the FFT, extracting features, and publishing them.
// ---------------------------------------------------------------------------

static void analysisThread() {
    // Allocate all working memory up front (no allocations in the loop).

    // Input buffer: accumulates samples from the ring buffer.
    // We maintain a sliding window: keep the last (FFT_SIZE - HOP_SIZE)
    // samples and read HOP_SIZE new ones each frame.
    std::vector<float> inputBuffer(FFT_SIZE, 0.0f);

    // Windowed samples, ready for FFT
    std::vector<float> windowedBuffer(FFT_SIZE);

    // FFT output: FFT_BINS complex values
    std::vector<kiss_fft_cpx> fftOutput(FFT_BINS);

    // Magnitude spectrum (current and previous frame, for spectral flux)
    std::vector<float> magnitude(FFT_BINS, 0.0f);
    std::vector<float> prevMagnitude(FFT_BINS, 0.0f);

    // Onset detection history (circular buffer of recent spectral flux values)
    std::vector<float> fluxHistory(ONSET_MEDIAN_WINDOW, 0.0f);
    int fluxHistoryIdx = 0;

    // Temporary buffer for reading from ring buffer
    std::vector<float> hopBuffer(HOP_SIZE);

    // Create KissFFT plan for real-valued input.
    // The plan is allocated once and reused for every FFT call.
    // 0 = forward transform (time -> frequency).
    kiss_fftr_cfg fftCfg = kiss_fftr_alloc(FFT_SIZE, 0, nullptr, nullptr);
    if (!fftCfg) {
        fprintf(stderr, "ERROR: Failed to create KissFFT plan\n");
        return;
    }

    // Wait until we have at least one full FFT window of data
    // before starting analysis. This avoids analyzing a buffer
    // that is mostly zeros.
    while (g_running.load(std::memory_order_relaxed)) {
        if (g_ringBuffer.availableRead() >= static_cast<size_t>(FFT_SIZE)) {
            // Read the initial full window
            g_ringBuffer.pop(inputBuffer.data(), FFT_SIZE);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Main analysis loop
    while (g_running.load(std::memory_order_relaxed)) {

        // Wait until HOP_SIZE new samples are available.
        // sleep_for(1ms) is acceptable here -- this is NOT the audio callback.
        // The analysis thread runs at near-real-time priority but is allowed
        // to sleep briefly. At 48kHz, HOP_SIZE=1600 samples arrive every
        // 33.3ms, so we check frequently but don't spin.
        if (g_ringBuffer.availableRead() < static_cast<size_t>(HOP_SIZE)) {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            continue;
        }

        // Slide the input buffer: move the tail forward, read new samples.
        // This implements the overlap: we keep (FFT_SIZE - HOP_SIZE) = 448
        // samples from the previous frame and append HOP_SIZE = 1600 new ones.
        std::memmove(inputBuffer.data(),
                     inputBuffer.data() + HOP_SIZE,
                     (FFT_SIZE - HOP_SIZE) * sizeof(float));

        g_ringBuffer.pop(inputBuffer.data() + (FFT_SIZE - HOP_SIZE), HOP_SIZE);

        // ----- Step 1: Compute RMS on the raw (unwindowed) input -----
        //
        // RMS is computed on the time-domain signal, not the frequency domain.
        // Windowing would attenuate samples near the edges and bias the RMS
        // estimate downward.

        float sumSquares = 0.0f;
        for (int i = 0; i < FFT_SIZE; ++i) {
            sumSquares += inputBuffer[i] * inputBuffer[i];
        }
        float rms = sqrtf(sumSquares / FFT_SIZE);

        // ----- Step 2: Apply Hann window -----

        for (int i = 0; i < FFT_SIZE; ++i) {
            windowedBuffer[i] = inputBuffer[i] * g_hannWindow[i];
        }

        // ----- Step 3: Compute FFT -----
        //
        // kiss_fftr() computes a real-input FFT. The input is FFT_SIZE real
        // values; the output is FFT_SIZE/2 + 1 complex values representing
        // the non-negative frequencies (the negative frequencies are
        // conjugate-symmetric for real input and are not stored).

        kiss_fftr(fftCfg, windowedBuffer.data(), fftOutput.data());

        // ----- Step 4: Compute magnitude spectrum -----
        //
        // |X[k]| = sqrt(re^2 + im^2)
        //
        // We normalize by FFT_SIZE to get amplitude values that are
        // independent of the transform length. This way, a pure sine wave
        // at amplitude 1.0 produces a peak of ~0.5 (due to the Hann window's
        // coherent gain of 0.5).

        for (int i = 0; i < FFT_BINS; ++i) {
            float re = fftOutput[i].r;
            float im = fftOutput[i].i;
            magnitude[i] = sqrtf(re * re + im * im) / FFT_SIZE;
        }

        // ----- Step 5: Spectral centroid -----
        //
        // centroid = sum(f[k] * |X[k]|) / sum(|X[k]|)
        //
        // This gives the "center of mass" of the spectrum in Hz.
        // Bright sounds (cymbals, hi-hats) have high centroids (3-8 kHz).
        // Dark sounds (bass, kick) have low centroids (100-500 Hz).
        // Silence produces centroid = 0 (we guard against division by zero).

        float weightedSum = 0.0f;
        float magnitudeSum = 0.0f;
        for (int i = 0; i < FFT_BINS; ++i) {
            float freq = i * BIN_WIDTH_HZ;
            weightedSum  += freq * magnitude[i];
            magnitudeSum += magnitude[i];
        }
        float centroid = (magnitudeSum > 1e-10f)
                       ? (weightedSum / magnitudeSum)
                       : 0.0f;

        // ----- Step 6: Frequency band energies -----
        //
        // For each band, we sum the squared magnitudes of all bins whose
        // center frequency falls within the band. The result is the
        // band's energy (proportional to power in that band).
        //
        // We take the square root to get an amplitude-like quantity that
        // is easier to visualize and map to graphics parameters.

        float bands[NUM_BANDS];
        for (int b = 0; b < NUM_BANDS; ++b) {
            float loHz = BAND_EDGES[b];
            float hiHz = BAND_EDGES[b + 1];

            // Convert Hz to bin indices
            int loBin = static_cast<int>(ceilf(loHz / BIN_WIDTH_HZ));
            int hiBin = static_cast<int>(floorf(hiHz / BIN_WIDTH_HZ));

            // Clamp to valid range
            if (loBin < 0)        loBin = 0;
            if (hiBin >= FFT_BINS) hiBin = FFT_BINS - 1;

            float energy = 0.0f;
            for (int i = loBin; i <= hiBin; ++i) {
                energy += magnitude[i] * magnitude[i];
            }

            // Normalize by number of bins to make bands comparable
            // despite having different widths.
            int binCount = hiBin - loBin + 1;
            if (binCount > 0) {
                energy /= binCount;
            }

            bands[b] = sqrtf(energy);
        }

        // ----- Step 7: Onset detection via spectral flux -----
        //
        // Spectral flux measures how much the spectrum changed since the
        // last frame. We use the "half-wave rectified" variant: only
        // increases in magnitude count. This makes the detector insensitive
        // to energy decay (which is not an onset) and responsive to
        // energy attacks (which are).
        //
        // The adaptive threshold is the median of the last N flux values
        // multiplied by a constant, plus a fixed floor. This handles
        // quiet passages (where the floor prevents false positives) and
        // loud passages (where the multiplicative factor scales with the
        // music's dynamics).
        //
        // Reference: Bello, J.P., Daudet, L., et al. (2005).
        //   "A Tutorial on Onset Detection in Music Signals."
        //   IEEE Trans. Speech and Audio Processing.

        float flux = 0.0f;
        for (int i = 0; i < FFT_BINS; ++i) {
            float diff = magnitude[i] - prevMagnitude[i];
            if (diff > 0.0f) {
                flux += diff;
            }
        }

        // Store in circular history for adaptive threshold
        fluxHistory[fluxHistoryIdx] = flux;
        fluxHistoryIdx = (fluxHistoryIdx + 1) % ONSET_MEDIAN_WINDOW;

        // Compute median of flux history
        std::vector<float> sortedFlux(fluxHistory.begin(), fluxHistory.end());
        std::nth_element(sortedFlux.begin(),
                         sortedFlux.begin() + ONSET_MEDIAN_WINDOW / 2,
                         sortedFlux.end());
        float medianFlux = sortedFlux[ONSET_MEDIAN_WINDOW / 2];

        float threshold = medianFlux * ONSET_THRESHOLD_MULT + ONSET_THRESHOLD_ADD;
        bool onset = flux > threshold;

        // Save current magnitude for next frame's spectral flux
        std::swap(magnitude, prevMagnitude);

        // ----- Publish features -----

        AudioFeatures features;
        features.rms              = rms;
        features.spectralCentroid = centroid;
        features.onset            = onset;
        features.spectralFlux     = flux;
        for (int b = 0; b < NUM_BANDS; ++b) {
            features.bands[b] = bands[b];
        }

        publishFeatures(features);
    }

    // Cleanup
    kiss_fft_free(fftCfg);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main() {
    // Register signal handler for clean shutdown on Ctrl+C
    signal(SIGINT, signalHandler);

    printf("=== Real-Time Audio Feature Extraction Prototype ===\n");
    printf("Sample rate: %d Hz | FFT size: %d | Hop: %d | FPS: %.1f\n\n",
           SAMPLE_RATE, FFT_SIZE, HOP_SIZE,
           static_cast<float>(SAMPLE_RATE) / HOP_SIZE);

    // Initialize Hann window lookup table
    initHannWindow();

    // Initialize feature buffers to zero
    std::memset(g_features, 0, sizeof(g_features));

    // ----- Configure and start audio capture device -----

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.format   = SAMPLE_FORMAT;
    deviceConfig.capture.channels = CHANNELS;
    deviceConfig.sampleRate       = SAMPLE_RATE;
    deviceConfig.dataCallback     = audioCallback;

    // Set a small buffer size for low latency. miniaudio will choose
    // the closest supported size. 512 samples = ~10.7ms at 48kHz.
    deviceConfig.periodSizeInFrames = 512;
    deviceConfig.periods            = 2;  // Double-buffered

    ma_device device;
    ma_result result = ma_device_init(nullptr, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize audio device (error %d)\n",
                result);
        return 1;
    }

    printf("Audio device: %s\n", device.capture.name);
    printf("Actual sample rate: %u Hz\n", device.sampleRate);
    printf("Actual period size: %u frames\n\n", device.capture.internalPeriodSizeInFrames);

    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to start audio device (error %d)\n",
                result);
        ma_device_uninit(&device);
        return 1;
    }

    // ----- Start analysis thread -----

    std::thread analysisWorker(analysisThread);

    // ----- Main loop: print features at 30fps -----

    printf("Listening... Press Ctrl+C to stop.\n\n");
    printf("%-6s %-8s %-7s %-7s %-7s %-7s %-7s %-7s %-7s %-5s\n",
           "RMS", "Centroid", "SubBas", "Bass", "LoMid", "Mid",
           "HiMid", "Pres", "Brill", "Onset");
    printf("--------------------------------------------------------------"
           "--------------------\n");

    auto nextPrintTime = std::chrono::steady_clock::now();

    while (g_running.load(std::memory_order_relaxed)) {
        auto now = std::chrono::steady_clock::now();
        if (now < nextPrintTime) {
            // Sleep until next print time. This is the main thread --
            // it is not real-time critical.
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1));
            continue;
        }
        nextPrintTime += std::chrono::microseconds(
            static_cast<int64_t>(OUTPUT_INTERVAL_MS * 1000.0));

        AudioFeatures f = readFeatures();

        // \r returns the cursor to the start of the line, overwriting
        // the previous output. This gives the illusion of an updating
        // display without scrolling.
        printf("\r%-6.4f %-8.1f %-7.4f %-7.4f %-7.4f %-7.4f %-7.4f %-7.4f %-7.4f %s",
               f.rms,
               f.spectralCentroid,
               f.bands[0], f.bands[1], f.bands[2], f.bands[3],
               f.bands[4], f.bands[5], f.bands[6],
               f.onset ? "***" : "   ");
        fflush(stdout);
    }

    // ----- Shutdown -----

    printf("\n\nShutting down...\n");

    // Stop audio device first (stops the callback)
    ma_device_stop(&device);
    ma_device_uninit(&device);

    // Wait for analysis thread to exit
    analysisWorker.join();

    printf("Done.\n");
    return 0;
}
```

---

## 4. Build Instructions

### 4.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)
project(AudioFeaturePrototype LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# KissFFT source files (compiled as C)
add_library(kissfft STATIC
    ext/kiss_fft.c
    ext/kiss_fftr.c
)
target_include_directories(kissfft PUBLIC ext/)

# Main application
add_executable(audio_features main.cpp)
target_include_directories(audio_features PRIVATE ext/ .)
target_link_libraries(audio_features PRIVATE kissfft)

# Platform-specific audio backend libraries.
# miniaudio links to these internally via #pragma comment(lib, ...)
# on Windows, but on Unix we must link them explicitly.
if(APPLE)
    # CoreAudio and AudioToolbox are the macOS audio backends.
    # CoreFoundation is needed for CFString operations in device enumeration.
    find_library(COREAUDIO_LIB CoreAudio REQUIRED)
    find_library(AUDIOTOOLBOX_LIB AudioToolbox REQUIRED)
    find_library(COREFOUNDATION_LIB CoreFoundation REQUIRED)
    target_link_libraries(audio_features PRIVATE
        ${COREAUDIO_LIB} ${AUDIOTOOLBOX_LIB} ${COREFOUNDATION_LIB})
elseif(UNIX)
    # On Linux, miniaudio can use ALSA, PulseAudio, or JACK.
    # Link pthread (required for miniaudio's internal threading),
    # libm (math), and libdl (dynamic loading of audio backends).
    target_link_libraries(audio_features PRIVATE pthread m dl)
endif()
# Windows: no explicit linking needed; miniaudio uses
# #pragma comment(lib, ...) to link ole32, user32, etc.
```

### 4.2 Build Commands

**macOS:**
```bash
# Download dependencies
mkdir -p ext
curl -sL https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h \
     -o ext/miniaudio.h
curl -sL https://raw.githubusercontent.com/mborgerding/kissfft/master/kiss_fft.h \
     -o ext/kiss_fft.h
curl -sL https://raw.githubusercontent.com/mborgerding/kissfft/master/kiss_fft.c \
     -o ext/kiss_fft.c
curl -sL https://raw.githubusercontent.com/mborgerding/kissfft/master/tools/kiss_fftr.h \
     -o ext/kiss_fftr.h
curl -sL https://raw.githubusercontent.com/mborgerding/kissfft/master/tools/kiss_fftr.c \
     -o ext/kiss_fftr.c

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run
./audio_features
```

**Linux (Ubuntu/Debian):**
```bash
# Install ALSA development headers (needed by miniaudio)
sudo apt-get install libasound2-dev

# Same download steps as above, then:
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./audio_features
```

**Windows (Visual Studio 2019+):**
```cmd
REM Same download steps (use PowerShell Invoke-WebRequest or manual download)
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
Release\audio_features.exe
```

### 4.3 Single-File Alternative

For maximum simplicity, the entire prototype can be compiled as a single `.cpp` file. Inline the KissFFT source and ring buffer header directly:

```bash
# Single-file compile on macOS:
c++ -std=c++17 -O2 -DMINIAUDIO_IMPLEMENTATION \
    -framework CoreAudio -framework AudioToolbox -framework CoreFoundation \
    -I ext/ \
    -x c ext/kiss_fft.c ext/kiss_fftr.c \
    -x c++ main.cpp \
    -o audio_features

# Single-file compile on Linux:
g++ -std=c++17 -O2 -DMINIAUDIO_IMPLEMENTATION \
    -I ext/ \
    ext/kiss_fft.c ext/kiss_fftr.c \
    main.cpp \
    -lpthread -lm -ldl \
    -o audio_features
```

If you truly want one file, copy the contents of `ring_buffer.h` into `main.cpp` (before the `#include "ring_buffer.h"` line, replacing it) and compile with the KissFFT `.c` files linked in as shown above. KissFFT cannot be easily inlined into a C++ file because it is C code; you would need `extern "C"` wrappers or compile it separately.

---

## 5. Ring Buffer Implementation Details

The ring buffer (Section 3.2) is the most critical piece of infrastructure in the prototype. It is the boundary between the real-time audio callback and the non-real-time analysis thread. Everything about its design is driven by the constraints in [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md).

### 5.1 Why Not std::queue / std::deque / std::vector?

All standard containers allocate memory dynamically. `std::queue` backed by `std::deque` allocates chunks on the heap as it grows. Even `std::vector` with `reserve()` is unsafe in the audio callback because `push_back()` may trigger reallocation if the size exceeds capacity, and the capacity check itself is not atomic with the push.

### 5.2 Why Not boost::lockfree::spsc_queue?

It would work correctly. We avoid it to keep the dependency count at zero. The SPSC ring buffer is small enough (~80 lines) that carrying a Boost dependency is not justified.

### 5.3 Memory Ordering Rationale

The ring buffer uses only `memory_order_relaxed`, `memory_order_acquire`, and `memory_order_release`. It never uses `memory_order_seq_cst` (the default for `std::atomic` operations), which is unnecessarily strong and generates full memory barriers on ARM.

- **Producer stores writeIndex with `release`:** This guarantees that all the `memcpy` writes to the buffer are visible to any thread that subsequently loads `writeIndex` with `acquire`. On ARM, this emits a `stlr` (store-release) instruction. On x86, `release` stores are free because x86 has a total store order.

- **Consumer loads writeIndex with `acquire`:** This guarantees that the consumer sees the data written by the producer. On ARM, this emits a `ldar` (load-acquire) instruction.

- **The reverse pair (consumer stores readIndex with `release`, producer loads readIndex with `acquire`)** ensures the producer does not overwrite data that the consumer is still reading.

### 5.4 Capacity Sizing

The ring buffer capacity is `FFT_SIZE * 4 = 8192` samples. At 48 kHz, this is ~170 ms of audio. The analysis thread processes one hop (1600 samples, ~33ms) per analysis frame. If the analysis thread stalls for 4 consecutive frames (~133ms), the ring buffer still has room. The 5th stall would cause sample drops. In practice, the analysis thread completes an FFT and feature extraction in under 100 microseconds, so stalls of this magnitude only happen if the OS deschedules the thread entirely (which requires extraordinary system load).

---

## 6. FFT Processing Deep Dive

### 6.1 Window Function Application

The Hann window is applied element-wise before the FFT:

```
windowed[i] = input[i] * 0.5 * (1 - cos(2*pi*i/N))
```

This is a multiplication of two length-N vectors -- trivially vectorizable by the compiler with `-O2`. The window is precomputed in a lookup table (`g_hannWindow`) to avoid recomputing cosines every frame.

The Hann window has a **coherent gain** of 0.5: a pure DC signal of amplitude 1.0 will appear as amplitude 0.5 in the windowed spectrum. For music analysis, this is acceptable because we care about relative magnitudes across bands, not absolute calibration.

The Hann window's **noise bandwidth** is 1.5 bins (versus 1.0 for rectangular). This means each spectral bin "leaks" into adjacent bins slightly. For our frequency band aggregation, this is a non-issue because we sum over many bins per band.

### 6.2 KissFFT Real FFT

The `kiss_fftr()` function exploits the conjugate symmetry of real-valued input to halve the computation. Internally, it packs the N-point real input into an N/2-point complex input, runs a complex FFT, and unpacks the result into N/2+1 complex bins.

The output bins represent frequencies from 0 Hz (bin 0, DC) to Nyquist (bin N/2, 24000 Hz at 48 kHz). Bin `k` corresponds to frequency `k * sampleRate / N`. With N=2048 and sampleRate=48000, each bin spans 23.4375 Hz.

### 6.3 Magnitude Computation

We compute `sqrt(re*re + im*im) / N` for each bin. The division by N normalizes the spectrum so that magnitudes are independent of FFT size. Some implementations divide by N/2 or omit the normalization entirely; the choice affects absolute magnitude values but not relative comparisons between bands.

An alternative is to compute power (`re*re + im*im`) without the square root, which is slightly cheaper. We use magnitude (with sqrt) because it compresses the dynamic range, making the values easier to work with when mapping to visual parameters. In a production system targeting maximum performance, you would skip the sqrt and work in power domain.

### 6.4 Frequency Band Aggregation

The 7 bands are defined by frequency edges. For each band, we identify the FFT bins whose center frequencies fall within the band, sum their squared magnitudes (energy), normalize by bin count, and take the square root.

Normalizing by bin count is important: the brilliance band (6-20 kHz) spans ~600 bins, while the sub-bass band (20-60 Hz) spans only ~2 bins. Without normalization, high-frequency bands would dominate purely due to having more bins, even if the actual energy density is lower.

The resulting band values are roughly comparable in scale: a pure sine wave at any frequency within a band will produce roughly the same band value (modulo windowing effects).

---

## 7. Feature Extraction Code Details

### 7.1 RMS Computation

RMS is computed on the raw (unwindowed) time-domain signal:

```
RMS = sqrt(sum(x[i]^2) / N)
```

This gives a single-number measure of the signal's amplitude. For 16-bit audio normalized to [-1, 1], typical RMS values are:

- Silence: < 0.001
- Quiet speech: 0.01 -- 0.05
- Loud music: 0.1 -- 0.5
- Clipping: approaching 1.0

RMS is computed before windowing because the Hann window attenuates samples at the edges, which would bias the RMS estimate downward by a factor of ~0.58 (the RMS of the Hann window itself).

### 7.2 Spectral Centroid

The spectral centroid is the weighted average of frequencies, using magnitude as the weight:

```
centroid = sum(freq[k] * mag[k]) / sum(mag[k])
```

For music, typical centroid values range from 200 Hz (bass-heavy content) to 8000 Hz (bright/harsh content). The centroid correlates well with perceived brightness and is one of the most useful features for audio-reactive visuals -- mapping it to color temperature or particle size produces intuitive results.

The division-by-zero guard (`magnitudeSum > 1e-10f`) handles silence. Without it, the centroid would be NaN during silent passages, which propagates to infinity in downstream math.

### 7.3 Onset Detection: Spectral Flux with Adaptive Threshold

The onset detector uses a three-step process:

**Step 1: Spectral flux.** For each FFT bin, compute the difference between the current magnitude and the previous frame's magnitude. Sum only the positive differences (half-wave rectification). This produces a single number that is large when new energy appears in the spectrum (a note onset, drum hit, etc.) and small during sustained sounds or decay.

```
flux = sum(max(0, mag[k] - prev_mag[k]))   for k = 0..N/2
```

**Step 2: Adaptive threshold.** Maintain a circular buffer of recent flux values. The threshold is the median of the last 10 flux values multiplied by 1.5, plus a fixed additive term (0.002). The median tracks the "typical" flux level and adapts to the music's dynamics. The multiplicative factor (1.5) requires the current flux to exceed the typical level by 50% to trigger. The additive term (0.002) prevents false triggers during near-silence where median is close to zero.

We use `std::nth_element` to compute the median efficiently -- it is O(N) average case, and with N=10 the overhead is negligible (~20 nanoseconds).

**Step 3: Decision.** If `flux > threshold`, report an onset.

This algorithm detects most percussive onsets (kick, snare, hi-hat, staccato notes) with few false positives. It is less reliable for soft onsets (legato strings, slow pad swells) because the spectral change is gradual. For a prototype, this is acceptable. Production systems add a peak-picking stage and a minimum inter-onset interval to improve accuracy (see [FEATURES_spectral.md](FEATURES_spectral.md) for advanced onset detection).

---

## 8. Output Format

When running, the prototype produces console output like this:

```
=== Real-Time Audio Feature Extraction Prototype ===
Sample rate: 48000 Hz | FFT size: 2048 | Hop: 1600 | FPS: 30.0

Audio device: MacBook Pro Microphone
Actual sample rate: 48000 Hz
Actual period size: 512 frames

Listening... Press Ctrl+C to stop.

RMS    Centroid SubBas  Bass    LoMid   Mid     HiMid   Pres    Brill   Onset
--------------------------------------------------------------------------------
0.0342 1847.3   0.0012  0.0089  0.0134  0.0098  0.0067  0.0043  0.0021  ***
```

The last line updates in-place (via `\r` carriage return). During a music playback test, you will see:

- **RMS** rising and falling with the music's volume
- **Centroid** shifting higher when hi-hats/cymbals are prominent, lower during bass-heavy passages
- **SubBas/Bass** bands lighting up on kick drums and bass notes
- **Mid/HiMid** bands responding to vocals and guitars
- **Onset** showing `***` on beat transients (kick, snare, note attacks)

For a more detailed view during development, you can modify the printf to print on new lines instead of overwriting:

```cpp
// Replace the \r printf with:
printf("%.4f %7.1f  %.4f %.4f %.4f %.4f %.4f %.4f %.4f  %s\n",
       f.rms, f.spectralCentroid,
       f.bands[0], f.bands[1], f.bands[2], f.bands[3],
       f.bands[4], f.bands[5], f.bands[6],
       f.onset ? "ONSET" : "     ");
```

This scrolls the terminal, which is useful for examining the temporal pattern of onsets or watching how band energies evolve during a song.

---

## 9. Testing and Verification

### 9.1 Basic Smoke Test

1. Build and run the prototype.
2. Speak into the microphone. You should see:
   - RMS values between 0.01 and 0.2
   - Centroid between 500 and 4000 Hz (human voice fundamental + harmonics)
   - Bass and Low-mid bands higher than Sub-bass and Brilliance
   - Onset triggers on plosive consonants (p, t, k)

### 9.2 Music Playback Test

Play music through speakers near the microphone (or use system audio loopback if available). Verify:

- **Kick drum**: Sub-bass and Bass bands spike. Onset triggers.
- **Snare**: Mid and Upper-mid bands spike. Onset triggers.
- **Hi-hat**: Presence and Brilliance bands spike. Onset may trigger (depends on the hi-hat's attack sharpness).
- **Bass line**: Bass band follows the bass notes. Centroid stays low.
- **Vocal section**: Low-mid and Mid bands dominate. Centroid sits at 1000-3000 Hz.
- **Drop/breakdown**: RMS jumps noticeably. Multiple bands spike simultaneously.

### 9.3 Sine Wave Test

Use an online tone generator (e.g., https://www.szynalski.com/tone-generator/) to play pure sine waves:

- **100 Hz sine**: Only the Bass band should show significant energy. Centroid should read ~100 Hz. All other bands near zero.
- **1000 Hz sine**: Only the Mid band should show energy. Centroid ~1000 Hz.
- **10000 Hz sine**: Only the Brilliance band should show energy. Centroid ~10000 Hz.
- **Sweep from 20 to 20000 Hz**: Watch each band light up in sequence from Sub-bass to Brilliance. Centroid should track the sweep frequency smoothly.

### 9.4 Onset Accuracy Test

Use a metronome app or drum loop. Count the onset triggers over 30 seconds. For a 120 BPM metronome (2 beats per second), you should see approximately 60 onset triggers in 30 seconds. Some beats may be missed; some double-triggers may occur. An accuracy above 80% indicates the detector is working correctly. Below 60% suggests a calibration issue with the threshold parameters.

### 9.5 Latency Estimation

The end-to-end latency from audio event to console display is:

| Component | Latency |
|-----------|---------|
| Audio device buffer (512 samples @ 48kHz) | ~10.7 ms |
| Ring buffer transit (variable) | 0--33 ms |
| FFT window (2048 samples, centered) | ~21.3 ms |
| Analysis computation | ~0.1 ms |
| Console print loop (up to 33ms wait) | 0--33 ms |
| **Total worst case** | **~98 ms** |
| **Total typical case** | **~45 ms** |

For a console prototype, sub-100ms latency is acceptable. The primary optimization targets for reducing latency are: (1) reduce HOP_SIZE to increase analysis rate, (2) use a tighter print loop or event-driven display, and (3) reduce the audio device buffer size.

---

## 10. Next Steps: From Prototype to Full Application

### 10.1 Add OpenGL Visualization

Replace the console print loop with an OpenGL render loop. The `readFeatures()` function already provides the feature data in a thread-safe manner. Map features to shader uniforms:

```cpp
// In the render loop:
AudioFeatures f = readFeatures();
glUniform1f(rmsLoc, f.rms);
glUniform1f(centroidLoc, f.spectralCentroid / 20000.0f);  // normalize to [0,1]
glUniform1fv(bandsLoc, NUM_BANDS, f.bands);
glUniform1i(onsetLoc, f.onset ? 1 : 0);
```

Use GLFW or SDL2 for window creation and input handling. Both are cross-platform and lightweight. See [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md) for the full integration guide.

### 10.2 Add More Features

The analysis function is the natural extension point. After the FFT, you have the full magnitude and phase spectrum. Additional features to compute:

- **Spectral rolloff**: Frequency below which 85% of spectral energy is concentrated. Useful for distinguishing voiced from unvoiced sounds.
- **Spectral flatness**: Ratio of geometric mean to arithmetic mean of the power spectrum. Measures "noisiness" -- high for noise, low for tonal content.
- **Chromagram**: 12-bin energy distribution mapping to musical pitch classes (C, C#, D, ..., B). Requires logarithmic frequency mapping.
- **BPM estimation**: Autocorrelation of the onset detection function over a 4-8 second window. See [FEATURES_spectral.md](FEATURES_spectral.md).
- **Mel-frequency cepstral coefficients (MFCCs)**: Standard in speech and music information retrieval. Requires a Mel filterbank and DCT.

### 10.3 Integrate Essentia or Aubio

For production-grade feature extraction, consider adding Essentia (C++, AGPL) or Aubio (C, GPL). Both provide battle-tested implementations of BPM tracking, pitch detection, and dozens of spectral features. The integration point is the analysis thread: replace the hand-rolled FFT and feature extraction with library calls, feeding them the same windowed audio buffer.

Essentia's streaming mode is particularly well-suited to real-time analysis: you configure an algorithm network (audio source -> windowing -> FFT -> spectral centroid -> output) and push samples through it. See [LIB_fft_comparison.md](LIB_fft_comparison.md) for performance comparisons.

### 10.4 System Audio Loopback

The prototype captures from the microphone by default. For analyzing system audio output (e.g., music playing in Spotify), you need platform-specific loopback:

- **macOS**: Use BlackHole or Soundflower virtual audio device, or the ScreenCaptureKit API (macOS 13+).
- **Windows**: WASAPI loopback mode (`AUDCLNT_STREAMFLAGS_LOOPBACK`). miniaudio supports this via `ma_device_type_loopback`.
- **Linux**: PulseAudio monitor source. Set the device name to the monitor source of the output device.

### 10.5 Performance Optimization

When moving to production with a full render pipeline:

- **Replace KissFFT with pffft or FFTW.** For 2048-point transforms the difference is small (~15us vs ~5us), but if you add multiple parallel FFTs (e.g., multi-channel analysis, chromagram with multiple window sizes), the 3x speedup matters.
- **SIMD-optimize the magnitude computation.** The `sqrt(re*re + im*im)` loop is a hot loop that benefits from SSE/NEON intrinsics. Alternatively, use `vDSP_zvabs` on macOS or IPP on Intel.
- **Remove the `std::vector` sorting in onset detection.** Replace the median computation with a running median (order-statistic tree or double-heap), which is O(log N) per insertion instead of O(N) per frame.
- **Use a condition variable or semaphore** instead of `sleep_for(500us)` in the analysis thread. This reduces latency jitter from 0-500us to near-zero, though it violates the "no blocking primitives" rule. The analysis thread is not the audio callback, so blocking primitives are acceptable here.

### 10.6 Configuration and Calibration

The hardcoded constants (FFT size, hop size, band edges, onset thresholds) should become configurable. A simple approach is a JSON or INI configuration file loaded at startup. More advanced: expose them as runtime-adjustable parameters via a GUI or OSC (Open Sound Control) interface, allowing live tuning while music plays.

---

## Appendix A: Complete Dependency Download Script

```bash
#!/bin/bash
# download_deps.sh -- Fetch all external dependencies for the prototype.
# Run from the project root directory.

set -euo pipefail

MINIAUDIO_VERSION="0.11.21"
KISSFFT_VERSION="131.1.0"

mkdir -p ext

echo "Downloading miniaudio ${MINIAUDIO_VERSION}..."
curl -sL "https://raw.githubusercontent.com/mackron/miniaudio/${MINIAUDIO_VERSION}/miniaudio.h" \
     -o ext/miniaudio.h

echo "Downloading KissFFT ${KISSFFT_VERSION}..."
KISSFFT_BASE="https://raw.githubusercontent.com/mborgerding/kissfft/${KISSFFT_VERSION}"
curl -sL "${KISSFFT_BASE}/kiss_fft.h"       -o ext/kiss_fft.h
curl -sL "${KISSFFT_BASE}/kiss_fft.c"       -o ext/kiss_fft.c
curl -sL "${KISSFFT_BASE}/tools/kiss_fftr.h" -o ext/kiss_fftr.h
curl -sL "${KISSFFT_BASE}/tools/kiss_fftr.c" -o ext/kiss_fftr.c

echo "Dependencies downloaded to ext/"
ls -la ext/
```

## Appendix B: Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| "Failed to initialize audio device" | No capture device found, or permission denied | Check microphone permissions in OS settings. On macOS, grant Terminal access in System Preferences > Privacy > Microphone. |
| All band values are zero | Audio device is not receiving signal | Speak into the mic; verify the correct device is selected; check system input volume is not zero. |
| RMS is nonzero but bands are all near-zero | Sample rate mismatch | The device may be running at a different rate than 48000 Hz. Check the "Actual sample rate" printout and adjust `SAMPLE_RATE` in the code. |
| Onset triggers constantly | Threshold too low for the environment | Increase `ONSET_THRESHOLD_ADD` (e.g., from 0.002 to 0.01) or increase `ONSET_THRESHOLD_MULT`. |
| Onset never triggers | Threshold too high, or microphone gain too low | Decrease threshold parameters, or increase system microphone gain. |
| Audio glitches / xruns reported by OS | Analysis thread is stealing CPU from audio callback | This should not happen in the prototype (analysis is lightweight). If it does, increase the audio device period size to 1024 or 2048 frames. |
| Build fails: "kiss_fft.h not found" | Missing dependency files | Run the download script (Appendix A) or manually download the files into `ext/`. |
| Build fails on Linux: "asoundlib.h not found" | Missing ALSA development package | `sudo apt-get install libasound2-dev` |
| Compile error: "M_PI undeclared" | MSVC does not define M_PI by default | Add `#define _USE_MATH_DEFINES` before `#include <cmath>`, or replace `M_PI` with `3.14159265358979323846`. |

---

*This document provides a complete, self-contained starting point. Once the prototype compiles and you can see frequency bands responding to music, you have a verified audio analysis pipeline. Everything after this -- OpenGL, more features, better onset detection -- is incremental extension of working code.*
