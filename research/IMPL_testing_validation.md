# Testing and Validation Strategies for Real-Time Audio Analysis

**Document ID:** IMPL_testing_validation
**Status:** Active
**Cross-references:** [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md), [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md), [REF_latency_numbers.md](REF_latency_numbers.md), [LIB_aubio.md](LIB_aubio.md), [LIB_essentia.md](LIB_essentia.md)

---

## 1. Test Signal Generation

Real-time audio analysis code cannot be validated against live input alone. You need deterministic, mathematically defined signals whose spectral and temporal properties are known analytically. Every test signal below has a closed-form expected output for FFT magnitude, RMS, spectral centroid, or onset position. If your implementation disagrees with the analytical result, the implementation is wrong -- not the test.

### 1.1 Pure Sine Waves at Known Frequencies

A pure sine at frequency `f` sampled at rate `fs` should produce energy in exactly one FFT bin (assuming `f` aligns with the bin grid, i.e., `f = k * fs / N` for integer `k`). When `f` does not align, energy leaks into adjacent bins according to the window's sidelobe pattern. Testing both aligned and misaligned frequencies validates windowing correctness.

**Expected FFT output for a 440 Hz sine at 48 kHz, N=4096:**
- Bin index: `k = 440 * 4096 / 48000 = 37.5467` (non-integer -- spectral leakage expected)
- With a Hann window, the main lobe spans 4 bins. Peak energy at bins 37 and 38.
- For an aligned test: use `f = 37 * 48000 / 4096 = 433.59375 Hz`. This should produce a single non-zero bin at k=37 with a Hann window (to within sidelobe attenuation of ~-31 dB).

**Expected RMS:** For `x(n) = A * sin(2*pi*f*n/fs)`, RMS = `A / sqrt(2)`.

```cpp
#include <vector>
#include <cmath>
#include <numbers>

// Generate a pure sine tone.
// @param frequency  Frequency in Hz
// @param sampleRate Sample rate in Hz
// @param numSamples Total number of samples to generate
// @param amplitude  Peak amplitude (default 1.0)
// @return Vector of float samples
std::vector<float> generateSine(double frequency, double sampleRate,
                                 size_t numSamples, double amplitude = 1.0) {
    std::vector<float> buffer(numSamples);
    const double phaseIncrement = 2.0 * std::numbers::pi * frequency / sampleRate;
    for (size_t i = 0; i < numSamples; ++i) {
        buffer[i] = static_cast<float>(amplitude * std::sin(phaseIncrement * i));
    }
    return buffer;
}

// Generate a bin-aligned sine for zero-leakage FFT testing.
// @param binIndex   Target FFT bin (integer)
// @param fftSize    FFT size N
// @param sampleRate Sample rate in Hz
// @param numSamples Number of samples (should be >= fftSize)
std::vector<float> generateBinAlignedSine(int binIndex, int fftSize,
                                           double sampleRate, size_t numSamples,
                                           double amplitude = 1.0) {
    double frequency = static_cast<double>(binIndex) * sampleRate /
                       static_cast<double>(fftSize);
    return generateSine(frequency, sampleRate, numSamples, amplitude);
}
```

### 1.2 White Noise

White noise has a flat power spectral density. The expected magnitude spectrum is constant across all bins (within statistical variance). For `M` averaged spectra, the standard deviation of each bin's power estimate decreases as `1/sqrt(M)`. With 100 averaged frames, expect +/-1 dB uniformity across the spectrum.

**Validation criterion:** Compute the power spectrum averaged over at least 100 frames. The standard deviation across bins (excluding DC and Nyquist) should be within 1 dB of the mean power.

```cpp
#include <random>

// Generate white noise with known statistical properties.
// Uses a Mersenne Twister seeded deterministically for reproducible tests.
// @param numSamples Number of samples
// @param seed       RNG seed for reproducibility
// @param amplitude  RMS amplitude (sigma of the Gaussian)
std::vector<float> generateWhiteNoise(size_t numSamples, uint32_t seed = 42,
                                       double amplitude = 0.1) {
    std::vector<float> buffer(numSamples);
    std::mt19937 gen(seed);
    std::normal_distribution<float> dist(0.0f, static_cast<float>(amplitude));
    for (size_t i = 0; i < numSamples; ++i) {
        buffer[i] = dist(gen);
    }
    return buffer;
}
```

### 1.3 Pink Noise (-3 dB/octave)

Pink noise has a power spectral density proportional to `1/f`. When plotted on a log-frequency axis, the magnitude spectrum decreases at -3 dB per octave (equivalently, -10 dB per decade). The standard generation method is the Voss-McCartney algorithm or filtering white noise with a -3 dB/octave IIR filter.

**Validation criterion:** Compute averaged power spectrum (100+ frames). For each octave band, the mean power should decrease by 3 dB +/-1 dB relative to the previous octave.

```cpp
// Generate pink noise using the Voss-McCartney algorithm.
// Uses 16 rows of white noise generators with staggered update rates.
// @param numSamples Number of samples
// @param seed       RNG seed
std::vector<float> generatePinkNoise(size_t numSamples, uint32_t seed = 42) {
    constexpr int kNumRows = 16;
    std::vector<float> buffer(numSamples);
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float rows[kNumRows] = {};
    float runningSum = 0.0f;

    // Initialize rows
    for (int r = 0; r < kNumRows; ++r) {
        rows[r] = dist(gen);
        runningSum += rows[r];
    }

    for (size_t i = 0; i < numSamples; ++i) {
        // Determine which row to update using trailing zero count
        // (Voss-McCartney trick: row r is updated every 2^r samples)
        int rowToUpdate = 0;
        size_t n = i;
        if (n > 0) {
            // Count trailing zeros in binary representation
            while ((n & 1) == 0 && rowToUpdate < kNumRows - 1) {
                n >>= 1;
                ++rowToUpdate;
            }
        }

        runningSum -= rows[rowToUpdate];
        rows[rowToUpdate] = dist(gen);
        runningSum += rows[rowToUpdate];

        // Normalize by number of rows
        buffer[i] = runningSum / static_cast<float>(kNumRows);
    }
    return buffer;
}
```

### 1.4 Click Trains at Known BPM

A click train is a sequence of impulses (single-sample spikes) separated by a fixed inter-onset interval (IOI). For BPM `b` at sample rate `fs`, IOI = `60 * fs / b` samples. The click train validates onset detection, beat tracking, and BPM estimation simultaneously.

**Expected outputs:**
- Onset detector should report onsets at sample positions `0, IOI, 2*IOI, ...` with tolerance <= 1 hop size.
- BPM estimator should converge to the ground-truth BPM within +/-0.5 BPM after sufficient context (typically 4-8 beats).

| BPM | IOI at 44100 Hz | IOI at 48000 Hz | Notes                    |
|-----|-----------------|-----------------|--------------------------|
| 120 | 22050           | 24000           | Common reference tempo    |
| 140 | 18900           | 20571.4         | Non-integer at 48 kHz     |
| 174 | 15206.9         | 16551.7         | Drum & bass standard      |
| 60  | 44100           | 48000           | One beat per second        |
| 200 | 13230           | 14400           | Fast tempo stress test     |

```cpp
// Generate a click train at a known BPM.
// Each click is a single-sample impulse of the specified amplitude.
// @param bpm         Tempo in beats per minute
// @param sampleRate  Sample rate in Hz
// @param durationSec Duration of the signal in seconds
// @param amplitude   Click amplitude (default 1.0)
// @return Pair of {audio buffer, onset sample positions}
std::pair<std::vector<float>, std::vector<size_t>>
generateClickTrain(double bpm, double sampleRate, double durationSec,
                   double amplitude = 1.0) {
    size_t numSamples = static_cast<size_t>(durationSec * sampleRate);
    double ioi = 60.0 * sampleRate / bpm;  // inter-onset interval in samples

    std::vector<float> buffer(numSamples, 0.0f);
    std::vector<size_t> onsetPositions;

    double position = 0.0;
    while (static_cast<size_t>(position) < numSamples) {
        size_t idx = static_cast<size_t>(std::round(position));
        if (idx < numSamples) {
            buffer[idx] = static_cast<float>(amplitude);
            onsetPositions.push_back(idx);
        }
        position += ioi;
    }
    return {buffer, onsetPositions};
}
```

### 1.5 Frequency Sweeps (Chirps)

A linear chirp sweeps from frequency `f0` to `f1` over duration `T`. The instantaneous frequency at time `t` is `f(t) = f0 + (f1 - f0) * t / T`. The spectrogram of a chirp should show a straight diagonal line from `f0` to `f1`. This validates that the FFT analysis correctly tracks frequency changes across time.

**Validation criterion:** At each analysis frame with center time `t`, the spectral centroid or peak bin should match `f(t)` within one FFT bin width (`fs / N`).

```cpp
// Generate a linear frequency sweep (chirp).
// @param f0          Start frequency in Hz
// @param f1          End frequency in Hz
// @param sampleRate  Sample rate in Hz
// @param durationSec Duration in seconds
// @param amplitude   Peak amplitude
std::vector<float> generateChirp(double f0, double f1, double sampleRate,
                                  double durationSec, double amplitude = 1.0) {
    size_t numSamples = static_cast<size_t>(durationSec * sampleRate);
    std::vector<float> buffer(numSamples);
    const double sweepRate = (f1 - f0) / durationSec;

    for (size_t i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double instantFreq = f0 + sweepRate * t;
        double phase = 2.0 * std::numbers::pi *
                       (f0 * t + 0.5 * sweepRate * t * t);
        buffer[i] = static_cast<float>(amplitude * std::sin(phase));
    }
    return buffer;
}
```

### 1.6 Multi-Tone Signals

A signal containing multiple simultaneous sinusoids validates frequency resolution. Two tones separated by less than one bin width (`fs / N`) will merge into a single peak. Tones separated by more than the main lobe width of the window function should resolve as distinct peaks.

**Resolution limit:** With a Hann window, the main lobe is 4 bins wide (`4 * fs / N` Hz). Two tones must be separated by at least `2 * fs / N` Hz to produce two distinct peaks.

```cpp
// Generate a multi-tone signal (sum of sinusoids).
// @param frequencies  Vector of frequencies in Hz
// @param amplitudes   Vector of amplitudes (same size as frequencies)
// @param sampleRate   Sample rate in Hz
// @param numSamples   Number of samples
std::vector<float> generateMultiTone(const std::vector<double>& frequencies,
                                      const std::vector<double>& amplitudes,
                                      double sampleRate, size_t numSamples) {
    assert(frequencies.size() == amplitudes.size());
    std::vector<float> buffer(numSamples, 0.0f);

    for (size_t f = 0; f < frequencies.size(); ++f) {
        const double phaseInc = 2.0 * std::numbers::pi *
                                frequencies[f] / sampleRate;
        for (size_t i = 0; i < numSamples; ++i) {
            buffer[i] += static_cast<float>(
                amplitudes[f] * std::sin(phaseInc * i));
        }
    }
    return buffer;
}
```

---

## 2. Unit Testing Audio Algorithms

Use Catch2 (header-only, no dependencies, natural expression syntax) or Google Test. Below all examples use Catch2 v3. The test project should link against your analysis library and the test signal generators above.

### 2.1 Project Setup (CMake + Catch2)

```cmake
# test/CMakeLists.txt
find_package(Catch2 3 REQUIRED)

add_executable(audio_tests
    test_fft.cpp
    test_rms.cpp
    test_spectral_centroid.cpp
    test_onset_detection.cpp
    test_bpm.cpp
    test_signal_generators.cpp
)

target_link_libraries(audio_tests PRIVATE
    audio_analysis_lib   # your analysis library target
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(audio_tests)
```

### 2.2 Testing FFT Output Against Known Analytical Solutions

For a bin-aligned sine at bin `k` with amplitude `A`, after applying a Hann window and computing the DFT, the expected magnitude at bin `k` is `A * N / 4` (for a Hann window whose sum is `N/2`), and all other bins should be below the Hann sidelobe floor of approximately `-31.5 dB` relative to the peak.

```cpp
// test_fft.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "test_signal_generators.h"
#include "fft_analyzer.h"  // your FFT wrapper

using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

TEST_CASE("FFT of bin-aligned sine produces single peak", "[fft]") {
    constexpr int kFFTSize = 4096;
    constexpr double kSampleRate = 48000.0;
    constexpr int kBin = 100;  // f = 100 * 48000 / 4096 = 1171.875 Hz
    constexpr double kAmplitude = 1.0;

    auto signal = generateBinAlignedSine(kBin, kFFTSize, kSampleRate,
                                          kFFTSize, kAmplitude);

    FFTAnalyzer fft(kFFTSize);
    fft.applyHannWindow(signal.data(), kFFTSize);
    auto magnitudes = fft.computeMagnitudeSpectrum(signal.data(), kFFTSize);

    // Find peak bin
    int peakBin = 0;
    float peakValue = 0.0f;
    for (int i = 1; i < kFFTSize / 2; ++i) {
        if (magnitudes[i] > peakValue) {
            peakValue = magnitudes[i];
            peakBin = i;
        }
    }

    REQUIRE(peakBin == kBin);

    // Verify sidelobe attenuation: all other bins should be at least
    // 30 dB below the peak (Hann window theoretical: -31.5 dB)
    float peakDB = 20.0f * std::log10(peakValue);
    for (int i = 1; i < kFFTSize / 2; ++i) {
        if (std::abs(i - kBin) > 2) {  // skip immediate neighbors
            float binDB = 20.0f * std::log10(magnitudes[i] + 1e-30f);
            REQUIRE(binDB < peakDB - 28.0f);  // 28 dB margin (2 dB slack)
        }
    }
}

TEST_CASE("FFT magnitude scales linearly with amplitude", "[fft]") {
    constexpr int kFFTSize = 2048;
    constexpr double kSampleRate = 44100.0;
    constexpr int kBin = 50;

    FFTAnalyzer fft(kFFTSize);

    for (double amp : {0.1, 0.5, 1.0, 2.0}) {
        auto signal = generateBinAlignedSine(kBin, kFFTSize, kSampleRate,
                                              kFFTSize, amp);
        fft.applyHannWindow(signal.data(), kFFTSize);
        auto mags = fft.computeMagnitudeSpectrum(signal.data(), kFFTSize);
        float peak = *std::max_element(mags.begin() + 1,
                                        mags.begin() + kFFTSize / 2);
        // Peak should be proportional to amplitude
        // Using 5% relative tolerance
        REQUIRE_THAT(peak / mags[kBin], WithinRel(1.0f, 0.05f));
    }
}

TEST_CASE("FFT of white noise has flat spectrum", "[fft][statistical]") {
    constexpr int kFFTSize = 4096;
    constexpr double kSampleRate = 48000.0;
    constexpr int kNumFrames = 200;

    FFTAnalyzer fft(kFFTSize);
    std::vector<double> avgPower(kFFTSize / 2, 0.0);

    for (int frame = 0; frame < kNumFrames; ++frame) {
        auto noise = generateWhiteNoise(kFFTSize, 42 + frame);
        fft.applyHannWindow(noise.data(), kFFTSize);
        auto mags = fft.computeMagnitudeSpectrum(noise.data(), kFFTSize);
        for (int i = 0; i < kFFTSize / 2; ++i) {
            avgPower[i] += static_cast<double>(mags[i] * mags[i]);
        }
    }

    // Average and convert to dB
    for (auto& p : avgPower) p = 10.0 * std::log10(p / kNumFrames + 1e-30);

    // Compute mean and stddev of power across bins (skip DC and last 5 bins)
    double sum = 0.0, sumSq = 0.0;
    int count = 0;
    for (int i = 5; i < kFFTSize / 2 - 5; ++i) {
        sum += avgPower[i];
        sumSq += avgPower[i] * avgPower[i];
        ++count;
    }
    double mean = sum / count;
    double stddev = std::sqrt(sumSq / count - mean * mean);

    // Flat spectrum: stddev should be less than 1.5 dB
    REQUIRE(stddev < 1.5);
}
```

### 2.3 Testing RMS Against Known Amplitudes

RMS of a sine wave with peak amplitude `A` is exactly `A / sqrt(2)`. RMS of DC signal with value `A` is `A`. RMS of silence is 0.

```cpp
// test_rms.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "rms_calculator.h"
#include "test_signal_generators.h"

TEST_CASE("RMS of sine wave equals A/sqrt(2)", "[rms]") {
    constexpr double kSampleRate = 48000.0;
    constexpr size_t kNumSamples = 48000;  // 1 second

    for (double amp : {0.1, 0.5, 1.0}) {
        auto signal = generateSine(440.0, kSampleRate, kNumSamples, amp);
        float rms = computeRMS(signal.data(), kNumSamples);
        float expected = static_cast<float>(amp / std::sqrt(2.0));
        REQUIRE_THAT(rms, WithinRel(expected, 0.001));  // 0.1% tolerance
    }
}

TEST_CASE("RMS of silence is zero", "[rms]") {
    std::vector<float> silence(4096, 0.0f);
    float rms = computeRMS(silence.data(), silence.size());
    REQUIRE_THAT(rms, WithinAbs(0.0f, 1e-7f));
}

TEST_CASE("RMS of DC signal equals the DC value", "[rms]") {
    std::vector<float> dc(4096, 0.75f);
    float rms = computeRMS(dc.data(), dc.size());
    REQUIRE_THAT(rms, WithinRel(0.75f, 0.001f));
}

TEST_CASE("RMS scales linearly with amplitude", "[rms]") {
    constexpr size_t kN = 48000;
    auto sig1 = generateSine(1000.0, 48000.0, kN, 0.5);
    auto sig2 = generateSine(1000.0, 48000.0, kN, 1.0);
    float rms1 = computeRMS(sig1.data(), kN);
    float rms2 = computeRMS(sig2.data(), kN);
    REQUIRE_THAT(rms2 / rms1, WithinRel(2.0f, 0.01f));
}
```

### 2.4 Testing Spectral Centroid Against Known Spectra

The spectral centroid of a pure sine at frequency `f` should equal `f` (all energy is at one frequency). For a two-tone signal with equal amplitudes at `f1` and `f2`, the centroid should be `(f1 + f2) / 2`. For unequal amplitudes, the centroid is the amplitude-weighted mean frequency.

```cpp
// test_spectral_centroid.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "spectral_features.h"
#include "test_signal_generators.h"

TEST_CASE("Spectral centroid of pure sine equals tone frequency", "[centroid]") {
    constexpr int kFFTSize = 4096;
    constexpr double kSampleRate = 48000.0;

    // Use bin-aligned frequency to avoid leakage artifacts
    for (int bin : {20, 50, 100, 200, 500}) {
        double freq = static_cast<double>(bin) * kSampleRate / kFFTSize;
        auto signal = generateBinAlignedSine(bin, kFFTSize, kSampleRate,
                                              kFFTSize);
        SpectralAnalyzer analyzer(kFFTSize, kSampleRate);
        float centroid = analyzer.computeSpectralCentroid(signal.data());

        // Tolerance: within 1 bin width
        double binWidth = kSampleRate / kFFTSize;
        REQUIRE_THAT(static_cast<double>(centroid),
                     WithinAbs(freq, binWidth));
    }
}

TEST_CASE("Spectral centroid of equal-amplitude two-tone is midpoint",
          "[centroid]") {
    constexpr int kFFTSize = 4096;
    constexpr double kSampleRate = 48000.0;
    constexpr int kBin1 = 50;   // ~585.9 Hz
    constexpr int kBin2 = 150;  // ~1757.8 Hz

    double f1 = kBin1 * kSampleRate / kFFTSize;
    double f2 = kBin2 * kSampleRate / kFFTSize;

    auto signal = generateMultiTone({f1, f2}, {1.0, 1.0},
                                     kSampleRate, kFFTSize);
    SpectralAnalyzer analyzer(kFFTSize, kSampleRate);
    float centroid = analyzer.computeSpectralCentroid(signal.data());

    double expectedCentroid = (f1 + f2) / 2.0;
    double tolerance = 2.0 * kSampleRate / kFFTSize;  // 2 bin widths
    REQUIRE_THAT(static_cast<double>(centroid),
                 WithinAbs(expectedCentroid, tolerance));
}
```

### 2.5 Testing Onset Detection Against Known Click Positions

```cpp
// test_onset_detection.cpp
#include <catch2/catch_test_macros.hpp>
#include "onset_detector.h"
#include "test_signal_generators.h"

TEST_CASE("Onset detection finds all clicks in a click train", "[onset]") {
    constexpr double kBPM = 120.0;
    constexpr double kSampleRate = 44100.0;
    constexpr double kDuration = 5.0;  // 5 seconds = 10 beats
    constexpr int kHopSize = 512;

    auto [signal, expectedOnsets] = generateClickTrain(kBPM, kSampleRate,
                                                        kDuration);

    OnsetDetector detector(kSampleRate, 2048, kHopSize);
    std::vector<size_t> detectedOnsets;

    // Process signal in hop-sized chunks (simulating real-time)
    for (size_t pos = 0; pos + kHopSize <= signal.size(); pos += kHopSize) {
        if (detector.processFrame(signal.data() + pos, kHopSize)) {
            detectedOnsets.push_back(pos);
        }
    }

    // Match detected onsets to expected onsets with tolerance
    size_t toleranceSamples = kHopSize;  // 1 hop tolerance
    size_t matched = 0;

    for (size_t expected : expectedOnsets) {
        for (size_t detected : detectedOnsets) {
            if (detected >= expected - toleranceSamples &&
                detected <= expected + toleranceSamples) {
                ++matched;
                break;
            }
        }
    }

    // Precision: fraction of detected onsets that are correct
    float precision = static_cast<float>(matched) / detectedOnsets.size();
    // Recall: fraction of true onsets that were detected
    float recall = static_cast<float>(matched) / expectedOnsets.size();

    REQUIRE(precision > 0.95f);
    REQUIRE(recall > 0.95f);
}

TEST_CASE("Onset detection does not fire on silence", "[onset]") {
    constexpr double kSampleRate = 44100.0;
    constexpr int kHopSize = 512;
    std::vector<float> silence(44100 * 2, 0.0f);  // 2 seconds

    OnsetDetector detector(kSampleRate, 2048, kHopSize);
    int falseOnsets = 0;

    for (size_t pos = 0; pos + kHopSize <= silence.size(); pos += kHopSize) {
        if (detector.processFrame(silence.data() + pos, kHopSize)) {
            ++falseOnsets;
        }
    }
    REQUIRE(falseOnsets == 0);
}
```

### 2.6 Testing BPM Against Known Tempos

```cpp
// test_bpm.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "bpm_tracker.h"
#include "test_signal_generators.h"

TEST_CASE("BPM tracker converges on click train tempo", "[bpm]") {
    constexpr double kSampleRate = 44100.0;
    constexpr int kHopSize = 512;
    constexpr double kDuration = 15.0;  // 15 seconds for convergence

    for (double targetBPM : {60.0, 90.0, 120.0, 140.0, 174.0, 200.0}) {
        auto [signal, onsets] = generateClickTrain(targetBPM, kSampleRate,
                                                    kDuration);

        BPMTracker tracker(kSampleRate, 2048, kHopSize);
        float estimatedBPM = 0.0f;

        for (size_t pos = 0; pos + kHopSize <= signal.size();
             pos += kHopSize) {
            estimatedBPM = tracker.processFrame(signal.data() + pos,
                                                 kHopSize);
        }

        INFO("Target BPM: " << targetBPM
             << ", Estimated: " << estimatedBPM);
        // Allow BPM to be within 1.0 BPM or half/double
        // (octave error is common and acceptable for many applications)
        bool withinTolerance =
            std::abs(estimatedBPM - targetBPM) < 1.0 ||
            std::abs(estimatedBPM - targetBPM * 2.0) < 1.0 ||
            std::abs(estimatedBPM - targetBPM / 2.0) < 1.0;

        REQUIRE(withinTolerance);
    }
}
```

---

## 3. Offline vs. Online Validation

### 3.1 Offline Validation: WAV File Processing

Offline validation processes a complete audio file from disk, compares the output with ground-truth annotations, and produces accuracy metrics. This is the gold standard for algorithm correctness because it eliminates all real-time variables (buffer underruns, scheduling jitter, clock drift).

```cpp
// offline_validator.cpp
#include <sndfile.h>   // libsndfile for WAV I/O
#include <fstream>
#include <iomanip>
#include "analysis_pipeline.h"

struct GroundTruth {
    std::vector<double> onsetTimesSeconds;
    double bpm;
    std::string key;
};

// Load ground-truth onset annotations (one time per line, in seconds)
GroundTruth loadGroundTruth(const std::string& path) {
    GroundTruth gt;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        if (line.starts_with("bpm:")) {
            gt.bpm = std::stod(line.substr(4));
        } else if (line.starts_with("key:")) {
            gt.key = line.substr(4);
        } else {
            gt.onsetTimesSeconds.push_back(std::stod(line));
        }
    }
    return gt;
}

struct ValidationResult {
    float onsetPrecision;
    float onsetRecall;
    float onsetFMeasure;
    float bpmError;
    bool bpmOctaveCorrect;  // within factor of 2
};

ValidationResult validateOffline(const std::string& audioPath,
                                  const std::string& annotationPath,
                                  int fftSize = 2048, int hopSize = 512) {
    // Open audio file
    SF_INFO sfInfo;
    SNDFILE* file = sf_open(audioPath.c_str(), SFM_READ, &sfInfo);
    if (!file) throw std::runtime_error("Cannot open " + audioPath);

    double sampleRate = sfInfo.samplerate;
    std::vector<float> audio(sfInfo.frames * sfInfo.channels);
    sf_readf_float(file, audio.data(), sfInfo.frames);
    sf_close(file);

    // Mix to mono if stereo
    std::vector<float> mono(sfInfo.frames);
    if (sfInfo.channels == 2) {
        for (int i = 0; i < sfInfo.frames; ++i)
            mono[i] = (audio[2*i] + audio[2*i+1]) * 0.5f;
    } else {
        mono = audio;
    }

    // Run analysis pipeline
    AnalysisPipeline pipeline(sampleRate, fftSize, hopSize);
    std::vector<double> detectedOnsets;
    float estimatedBPM = 0.0f;

    for (size_t pos = 0; pos + hopSize <= mono.size(); pos += hopSize) {
        auto result = pipeline.processFrame(mono.data() + pos, hopSize);
        if (result.onsetDetected) {
            detectedOnsets.push_back(
                static_cast<double>(pos) / sampleRate);
        }
        estimatedBPM = result.currentBPM;
    }

    // Compare with ground truth
    auto gt = loadGroundTruth(annotationPath);

    // Onset evaluation with 50ms tolerance window
    constexpr double kToleranceSec = 0.050;
    int truePositives = 0;
    std::vector<bool> gtMatched(gt.onsetTimesSeconds.size(), false);

    for (double detected : detectedOnsets) {
        for (size_t g = 0; g < gt.onsetTimesSeconds.size(); ++g) {
            if (!gtMatched[g] &&
                std::abs(detected - gt.onsetTimesSeconds[g]) <
                    kToleranceSec) {
                ++truePositives;
                gtMatched[g] = true;
                break;
            }
        }
    }

    ValidationResult result;
    result.onsetPrecision = detectedOnsets.empty() ? 0.0f :
        static_cast<float>(truePositives) / detectedOnsets.size();
    result.onsetRecall = gt.onsetTimesSeconds.empty() ? 0.0f :
        static_cast<float>(truePositives) / gt.onsetTimesSeconds.size();
    result.onsetFMeasure = (result.onsetPrecision + result.onsetRecall > 0) ?
        2.0f * result.onsetPrecision * result.onsetRecall /
            (result.onsetPrecision + result.onsetRecall)
        : 0.0f;
    result.bpmError = std::abs(estimatedBPM - static_cast<float>(gt.bpm));
    result.bpmOctaveCorrect =
        result.bpmError < 1.0f ||
        std::abs(estimatedBPM - static_cast<float>(gt.bpm * 2.0)) < 1.0f ||
        std::abs(estimatedBPM - static_cast<float>(gt.bpm / 2.0)) < 1.0f;

    return result;
}
```

### 3.2 Online Validation: Real-Time Analysis of Test Signals

Online validation feeds test signals through the actual real-time pipeline (audio callback, ring buffer, analysis thread) and records the output. This catches bugs that only manifest under real-time scheduling pressure: race conditions, buffer overflows, timing-dependent state corruption.

```cpp
// online_validator.h
// Records all analysis output frames with timestamps for later comparison.

#include <atomic>
#include <vector>
#include <chrono>

struct TimestampedFeatures {
    std::chrono::steady_clock::time_point timestamp;
    float rms;
    float spectralCentroid;
    float bpm;
    bool onsetDetected;
    size_t frameIndex;
};

class OnlineValidator {
    std::vector<TimestampedFeatures> recorded_;
    std::atomic<bool> recording_{false};
    size_t frameCounter_ = 0;

public:
    void startRecording() {
        recorded_.clear();
        recorded_.reserve(10000);  // pre-allocate to avoid RT allocation
        frameCounter_ = 0;
        recording_.store(true, std::memory_order_release);
    }

    void stopRecording() {
        recording_.store(false, std::memory_order_release);
    }

    // Called from analysis thread -- must not allocate
    void recordFrame(float rms, float centroid, float bpm,
                     bool onset) {
        if (!recording_.load(std::memory_order_acquire)) return;
        if (frameCounter_ < recorded_.capacity()) {
            recorded_.push_back({
                std::chrono::steady_clock::now(),
                rms, centroid, bpm, onset, frameCounter_
            });
        }
        ++frameCounter_;
    }

    // Called from main thread after recording stops
    void saveToCSV(const std::string& path) const {
        std::ofstream out(path);
        out << "frame,timestamp_us,rms,centroid,bpm,onset\n";
        auto t0 = recorded_.empty() ?
            std::chrono::steady_clock::now() : recorded_[0].timestamp;
        for (const auto& f : recorded_) {
            auto us = std::chrono::duration_cast<
                std::chrono::microseconds>(f.timestamp - t0).count();
            out << f.frameIndex << "," << us << ","
                << f.rms << "," << f.spectralCentroid << ","
                << f.bpm << "," << f.onsetDetected << "\n";
        }
    }

    const std::vector<TimestampedFeatures>& getRecorded() const {
        return recorded_;
    }
};
```

### 3.3 Offline-Online Comparison

After recording online output, compare it against offline output from the same signal. Differences indicate real-time-specific bugs. The comparison should account for alignment offsets (the online pipeline may start analysis at a different sample position due to initial buffering).

```cpp
// Align two onset sequences by finding the offset that maximizes matches
int findBestAlignment(const std::vector<double>& offlineOnsets,
                       const std::vector<double>& onlineOnsets,
                       double toleranceSec = 0.050) {
    int bestOffset = 0;
    int bestMatches = 0;

    // Try offsets in range [-0.5s, +0.5s] at 1ms resolution
    for (int offsetMs = -500; offsetMs <= 500; ++offsetMs) {
        double offsetSec = offsetMs * 0.001;
        int matches = 0;
        for (double online : onlineOnsets) {
            for (double offline : offlineOnsets) {
                if (std::abs((online + offsetSec) - offline) <
                    toleranceSec) {
                    ++matches;
                    break;
                }
            }
        }
        if (matches > bestMatches) {
            bestMatches = matches;
            bestOffset = offsetMs;
        }
    }
    return bestOffset;
}
```

---

## 4. Reference Datasets

Validating against synthetic signals proves algorithmic correctness, but validating against real music proves practical utility. Several standard datasets exist with human-annotated ground truth.

### 4.1 GTZAN Genre Dataset

- **Contents:** 1000 audio excerpts, 30 seconds each, 10 genres (100 per genre)
- **Format:** 22050 Hz, 16-bit WAV mono
- **Annotations available:** Genre labels (original), BPM annotations (Marchand & Peeters 2015), beat positions (various)
- **Download:** <http://marsyas.info/downloads/datasets.html>
- **Use case:** BPM estimation validation, genre-dependent accuracy analysis

```cpp
// Example: batch-validate BPM on GTZAN
void validateGTZAN(const std::string& datasetDir,
                    const std::string& annotationDir) {
    // Annotation format: one file per track, containing BPM as float
    namespace fs = std::filesystem;
    int correct = 0, total = 0;

    for (auto& entry : fs::recursive_directory_iterator(datasetDir)) {
        if (entry.path().extension() != ".wav") continue;

        std::string annotPath = annotationDir + "/" +
            entry.path().stem().string() + ".bpm";
        if (!fs::exists(annotPath)) continue;

        std::ifstream annot(annotPath);
        double trueBPM;
        annot >> trueBPM;

        auto result = validateOffline(entry.path().string(), annotPath);

        // MIREX-style BPM evaluation: correct if within 4% of true BPM
        // or within 4% of 2x/0.5x/3x/0.33x
        bool mirexCorrect = false;
        for (double factor : {1.0, 2.0, 0.5, 3.0, 1.0/3.0}) {
            if (std::abs(result.bpmError) < 0.04 * trueBPM * factor) {
                mirexCorrect = true;
                break;
            }
        }

        if (mirexCorrect) ++correct;
        ++total;
    }

    std::cout << "GTZAN BPM accuracy: " << correct << "/" << total
              << " (" << 100.0 * correct / total << "%)" << std::endl;
}
```

### 4.2 RWC Music Database

- **Contents:** 315 pieces across pop, classical, jazz, and other genres
- **Annotations:** Beat positions, chord labels, melody, structure
- **Access:** <https://staff.aist.go.jp/m.goto/RWC-MDB/>
- **Use case:** Beat tracking, chord estimation, structural segmentation
- **Note:** Requires institutional license for some subsets

### 4.3 MIREX Onset Detection Evaluation Data

The MIREX (Music Information Retrieval Evaluation eXchange) onset detection task uses several datasets with millisecond-accurate onset annotations:

- **ODB (Onset Detection Benchmark):** Mixed instruments, annotated onsets
- **Bello dataset:** Drum patterns, pitched instruments, complex mixtures
- **Access:** Typically provided through MIREX submission (<https://www.music-ir.org/mirex/>)

**Evaluation protocol:** Standard MIREX onset detection uses a 50ms tolerance window. A detected onset is a true positive if it falls within 50ms of a ground-truth onset, with each ground-truth onset matched at most once.

### 4.4 Practical Validation Workflow

```
Step 1: Synthetic tests pass                → Algorithmic correctness
Step 2: GTZAN/RWC offline results match     → Practical accuracy
        published benchmarks within 5%
Step 3: Online results match offline        → Real-time correctness
        results within 1 hop of alignment
Step 4: Stress tests pass at target         → Production readiness
        buffer size for 24+ hours
```

---

## 5. Latency Measurement

Latency in a real-time audio analysis system has multiple components. Total end-to-end latency from audio event to feature availability is the sum of all stages.

### 5.1 Latency Budget Breakdown

| Component                | Typical Range      | Notes                                    |
|--------------------------|--------------------|------------------------------------------|
| Audio hardware buffer    | 1.33-10.67 ms     | 64-512 samples @ 48 kHz                   |
| OS audio callback jitter | 0-2 ms             | Scheduling variance                       |
| Ring buffer transfer     | ~0 ms              | Lock-free, sub-microsecond                |
| Analysis accumulation    | 0-N/fs ms          | Waiting for FFT-size samples              |
| FFT computation          | 0.01-0.1 ms       | For N=2048-8192                           |
| Feature extraction       | 0.01-0.5 ms       | Depends on algorithm complexity           |
| Feature bus transfer     | ~0 ms              | Atomic pointer swap                       |
| **Total**                | **2-25 ms**        | Dominated by buffer sizes, not compute    |

### 5.2 Software Latency Measurement

The most accurate software measurement uses high-resolution timestamps at the point of audio input and at the point of feature output, then computes the difference.

```cpp
#include <chrono>
#include <atomic>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>

class LatencyMeasurer {
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    std::atomic<TimePoint> clickInjectionTime_{};
    std::atomic<bool> clickPending_{false};
    std::vector<double> latencyMeasurementsUs_;

public:
    // Call from audio callback when injecting/detecting a click
    void markClickInput() {
        clickInjectionTime_.store(Clock::now(), std::memory_order_release);
        clickPending_.store(true, std::memory_order_release);
    }

    // Call from analysis thread when onset is detected
    void markOnsetDetected() {
        if (!clickPending_.load(std::memory_order_acquire)) return;

        auto now = Clock::now();
        auto injectionTime = clickInjectionTime_.load(
            std::memory_order_acquire);
        auto latencyUs = std::chrono::duration_cast<
            std::chrono::microseconds>(now - injectionTime).count();

        latencyMeasurementsUs_.push_back(static_cast<double>(latencyUs));
        clickPending_.store(false, std::memory_order_release);
    }

    // Call from main thread after measurement session
    void printStatistics() const {
        if (latencyMeasurementsUs_.empty()) {
            std::cout << "No latency measurements recorded.\n";
            return;
        }

        auto sorted = latencyMeasurementsUs_;
        std::sort(sorted.begin(), sorted.end());

        double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
        double mean = sum / sorted.size();
        double median = sorted[sorted.size() / 2];
        double p99 = sorted[static_cast<size_t>(sorted.size() * 0.99)];
        double maxVal = sorted.back();

        // Compute standard deviation
        double sqSum = 0.0;
        for (double v : sorted) sqSum += (v - mean) * (v - mean);
        double stddev = std::sqrt(sqSum / sorted.size());

        std::cout << "Latency measurements (n=" << sorted.size() << "):\n"
                  << "  Mean:   " << mean / 1000.0 << " ms\n"
                  << "  Median: " << median / 1000.0 << " ms\n"
                  << "  StdDev: " << stddev / 1000.0 << " ms\n"
                  << "  P99:    " << p99 / 1000.0 << " ms\n"
                  << "  Max:    " << maxVal / 1000.0 << " ms\n";
    }
};
```

### 5.3 Hardware Measurement with Loopback

The gold standard for end-to-end latency measurement uses a hardware loopback (connecting audio output to audio input) and an oscilloscope or logic analyzer:

1. Generate a click on the audio output.
2. The click passes through the DAC, travels through the loopback cable, enters the ADC.
3. The analysis pipeline detects the click and toggles a GPIO pin or sends a MIDI note.
4. An oscilloscope measures the time between the output click and the GPIO toggle.

This captures the full chain including DAC latency, ADC latency, and OS scheduling -- quantities invisible to software timestamps.

```
Oscilloscope setup:
  Channel 1: Audio output (line-level, before DAC if digital out available)
  Channel 2: GPIO pin toggled on onset detection
  Trigger: Channel 1 rising edge
  Measurement: Time difference Ch1→Ch2
  Expected: 3-30 ms depending on buffer configuration
```

For setups without GPIO access, use a secondary audio output channel: output silence on channel 2 normally, output a click on channel 2 when the analysis detects an onset on channel 1 input. The oscilloscope measures the roundtrip, and you subtract one buffer period (known) to get the analysis-only latency.

### 5.4 Automated Latency Test

```cpp
// Automated latency test using loopback or test signal injection
TEST_CASE("End-to-end latency is within budget", "[latency][integration]") {
    constexpr double kSampleRate = 48000.0;
    constexpr int kBufferSize = 128;
    constexpr int kFFTSize = 2048;
    constexpr int kHopSize = 512;
    constexpr int kNumClicks = 50;  // statistical sample
    constexpr double kClickIntervalSec = 0.5;

    // Maximum acceptable latency:
    // buffer (2.67ms) + accumulation (worst case: FFT size = 42.67ms)
    // + processing (1ms margin) = ~46 ms
    constexpr double kMaxLatencyMs =
        (static_cast<double>(kBufferSize + kFFTSize) / kSampleRate) * 1000.0
        + 5.0;  // 5ms margin for scheduling

    LatencyMeasurer measurer;

    // Generate click train
    auto [signal, onsets] = generateClickTrain(
        60.0 / kClickIntervalSec, kSampleRate,
        kNumClicks * kClickIntervalSec + 1.0);

    // Run through pipeline with timing
    AnalysisPipeline pipeline(kSampleRate, kFFTSize, kHopSize);

    size_t clickIdx = 0;
    for (size_t pos = 0; pos + kBufferSize <= signal.size();
         pos += kBufferSize) {
        // Check if this buffer contains a click
        if (clickIdx < onsets.size() &&
            onsets[clickIdx] >= pos &&
            onsets[clickIdx] < pos + kBufferSize) {
            measurer.markClickInput();
            ++clickIdx;
        }

        // Process through pipeline
        // (in real system, this goes through ring buffer + analysis thread)
        for (size_t hop = 0; hop + kHopSize <= kBufferSize;
             hop += kHopSize) {
            auto result = pipeline.processFrame(
                signal.data() + pos + hop, kHopSize);
            if (result.onsetDetected) {
                measurer.markOnsetDetected();
            }
        }
    }

    measurer.printStatistics();
    // The actual latency assertion would go here
    // (requires real threading to be meaningful)
}
```

---

## 6. Profiling the Analysis Thread

The analysis thread must complete all processing within one hop period. At 48 kHz with a 512-sample hop, that is 10.67 ms. Profiling identifies bottlenecks and ensures headroom.

### 6.1 Application-Level Profiling with std::chrono

The most portable approach: wrap each algorithm stage with high-resolution timing. This runs in production builds (with minimal overhead) and produces per-algorithm timing breakdowns.

```cpp
#include <chrono>
#include <array>
#include <string>
#include <iostream>
#include <iomanip>
#include <atomic>
#include <algorithm>

class AnalysisProfiler {
public:
    enum Stage {
        kWindowing,
        kFFT,
        kMagnitude,
        kRMS,
        kSpectralCentroid,
        kOnsetDetection,
        kBPMTracking,
        kChromagram,
        kTotal,
        kNumStages
    };

    static constexpr const char* kStageNames[] = {
        "Windowing", "FFT", "Magnitude", "RMS", "SpectralCentroid",
        "OnsetDetection", "BPMTracking", "Chromagram", "Total"
    };

private:
    using Clock = std::chrono::steady_clock;

    struct StageStats {
        double totalUs = 0.0;
        double maxUs = 0.0;
        double minUs = 1e9;
        uint64_t count = 0;

        void record(double us) {
            totalUs += us;
            maxUs = std::max(maxUs, us);
            minUs = std::min(minUs, us);
            ++count;
        }

        double meanUs() const {
            return count > 0 ? totalUs / count : 0.0;
        }
    };

    std::array<StageStats, kNumStages> stats_{};
    Clock::time_point stageStart_;
    Clock::time_point frameStart_;

public:
    void beginFrame() {
        frameStart_ = Clock::now();
    }

    void beginStage() {
        stageStart_ = Clock::now();
    }

    void endStage(Stage stage) {
        auto elapsed = std::chrono::duration_cast<
            std::chrono::nanoseconds>(Clock::now() - stageStart_).count();
        stats_[stage].record(elapsed / 1000.0);
    }

    void endFrame() {
        auto elapsed = std::chrono::duration_cast<
            std::chrono::nanoseconds>(Clock::now() - frameStart_).count();
        stats_[kTotal].record(elapsed / 1000.0);
    }

    void printReport(double hopDurationUs) const {
        std::cout << "\n=== Analysis Thread Profile ===\n"
                  << std::setw(20) << "Stage"
                  << std::setw(12) << "Mean(us)"
                  << std::setw(12) << "Max(us)"
                  << std::setw(12) << "Min(us)"
                  << std::setw(10) << "% Budget"
                  << "\n" << std::string(66, '-') << "\n";

        for (int i = 0; i < kNumStages; ++i) {
            auto& s = stats_[i];
            if (s.count == 0) continue;
            double pct = 100.0 * s.meanUs() / hopDurationUs;
            std::cout << std::setw(20) << kStageNames[i]
                      << std::setw(12) << std::fixed << std::setprecision(1)
                      << s.meanUs()
                      << std::setw(12) << s.maxUs
                      << std::setw(12) << s.minUs
                      << std::setw(9) << std::setprecision(1) << pct << "%"
                      << "\n";
        }

        auto& total = stats_[kTotal];
        std::cout << "\nBudget: " << hopDurationUs / 1000.0 << " ms/hop | "
                  << "Used: " << total.meanUs() / 1000.0 << " ms mean, "
                  << total.maxUs / 1000.0 << " ms max | "
                  << "Headroom: "
                  << (hopDurationUs - total.maxUs) / 1000.0 << " ms\n";

        if (total.maxUs > hopDurationUs) {
            std::cout << "WARNING: Max frame time exceeds hop budget by "
                      << (total.maxUs - hopDurationUs) / 1000.0
                      << " ms\n";
        }
    }
};

// Usage inside analysis loop:
//   profiler.beginFrame();
//   profiler.beginStage();
//   applyWindow(data, N);
//   profiler.endStage(AnalysisProfiler::kWindowing);
//   profiler.beginStage();
//   computeFFT(data, N);
//   profiler.endStage(AnalysisProfiler::kFFT);
//   ...
//   profiler.endFrame();
```

### 6.2 macOS: Instruments

macOS Instruments provides the most detailed profiling for CoreAudio applications:

- **Time Profiler:** Sampling-based CPU profiler. Set the thread filter to the analysis thread. Look for unexpectedly hot functions (memory allocation, mutex contention, page faults).
- **System Trace:** Shows thread scheduling, context switches, and interrupt handling. Invaluable for diagnosing latency spikes caused by OS scheduling.
- **Audio System Trace (AU):** CoreAudio-specific template showing audio thread activity and timing.

```bash
# Record 30 seconds of analysis thread activity
xcrun xctrace record --template 'Time Profiler' \
    --time-limit 30s \
    --launch ./audio_analyzer

# Record system trace (requires SIP partial disable on some versions)
xcrun xctrace record --template 'System Trace' \
    --time-limit 10s \
    --attach $(pgrep audio_analyzer)
```

**Key things to look for in Instruments:**
1. Any frame where the analysis thread was preempted for >1ms (System Trace: look at thread state timeline)
2. Any malloc/free calls on the analysis thread (Time Profiler: search for `malloc` in call tree)
3. Any lock acquisition on the analysis thread (search for `pthread_mutex_lock`)
4. Page faults on the analysis thread (System Trace: VM Fault instrument)

### 6.3 Linux: perf and Flamegraphs

```bash
# Record perf data for 30 seconds
perf record -g -p $(pgrep audio_analyzer) -- sleep 30

# Generate flamegraph (requires Brendan Gregg's FlameGraph scripts)
perf script | stackcollapse-perf.pl | flamegraph.pl > analysis_flame.svg

# Per-thread CPU statistics
perf stat -t <analysis_thread_tid> -e \
    cycles,instructions,cache-misses,page-faults,context-switches \
    -- sleep 10
```

**Interpreting perf stat for audio threads:**
- `context-switches`: Should be very low (ideally 0 involuntary). High numbers mean the thread is being preempted.
- `page-faults`: Must be 0 during steady-state operation. Any page fault can stall for milliseconds.
- `cache-misses`: High L1/L2 cache miss rate indicates poor data locality in FFT or feature extraction.
- `instructions/cycle`: Below 1.0 suggests memory-bound code; above 2.0 is efficient compute.

### 6.4 Windows: VTune and ETW

```
Intel VTune:
  - Hotspots analysis: identifies slow functions
  - Threading analysis: shows lock contention, thread imbalance
  - Microarchitecture Exploration: cache misses, branch mispredictions

Windows Performance Analyzer (WPA) with ETW:
  - xperf -on PROC_THREAD+LOADER+CSWITCH+DISPATCHER -stackwalk CSwitch
  - Captures context switches with call stacks
  - Shows exactly when and why the audio/analysis thread was preempted
```

### 6.5 CPU Usage Monitoring Per Thread

```cpp
#include <thread>
#ifdef __linux__
#include <sys/resource.h>
#endif
#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/thread_info.h>
#endif

// Get CPU time used by the current thread (in microseconds)
double getThreadCPUTimeUs() {
#ifdef __APPLE__
    mach_port_t thread = mach_thread_self();
    thread_basic_info_data_t info;
    mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
    kern_return_t kr = thread_info(thread, THREAD_BASIC_INFO,
                                    reinterpret_cast<thread_info_t>(&info),
                                    &count);
    mach_port_deallocate(mach_task_self(), thread);
    if (kr != KERN_SUCCESS) return -1.0;
    return (info.user_time.seconds + info.system_time.seconds) * 1e6 +
           info.user_time.microseconds + info.system_time.microseconds;
#elif defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1000.0;
#else
    return -1.0;  // Not implemented for this platform
#endif
}

// Monitor CPU usage of the analysis thread over a measurement window.
// Call periodically (e.g., once per second) from a monitoring thread.
class ThreadCPUMonitor {
    double lastCPUTimeUs_ = 0.0;
    std::chrono::steady_clock::time_point lastWallTime_;
    bool initialized_ = false;

public:
    // Returns CPU usage as a fraction [0.0, 1.0]
    // (1.0 = thread used 100% of one core)
    double update() {
        double cpuNow = getThreadCPUTimeUs();
        auto wallNow = std::chrono::steady_clock::now();

        if (!initialized_) {
            lastCPUTimeUs_ = cpuNow;
            lastWallTime_ = wallNow;
            initialized_ = true;
            return 0.0;
        }

        double cpuDeltaUs = cpuNow - lastCPUTimeUs_;
        double wallDeltaUs = std::chrono::duration_cast<
            std::chrono::microseconds>(wallNow - lastWallTime_).count();

        lastCPUTimeUs_ = cpuNow;
        lastWallTime_ = wallNow;

        return wallDeltaUs > 0 ? cpuDeltaUs / wallDeltaUs : 0.0;
    }
};
```

---

## 7. Audio Dropout Detection

An audio dropout (xrun) occurs when the audio callback does not complete before the hardware needs the next buffer. On ALSA, this is called an "underrun" (output) or "overrun" (input). On JACK, it is simply an xrun. On CoreAudio, it manifests as an I/O cycle being missed. Detecting and logging xruns is essential for validating real-time performance.

### 7.1 Programmatic Xrun Detection via Callback Timing

The most portable approach: measure the time between consecutive audio callbacks. If the interval exceeds `1.5 * expectedInterval`, a dropout likely occurred (the OS queued up two callbacks worth of work).

```cpp
#include <chrono>
#include <vector>
#include <fstream>
#include <cmath>

struct XrunEvent {
    std::chrono::steady_clock::time_point timestamp;
    double expectedIntervalMs;
    double actualIntervalMs;
    uint64_t callbackIndex;
};

class DropoutDetector {
    using Clock = std::chrono::steady_clock;
    Clock::time_point lastCallbackTime_;
    bool firstCallback_ = true;
    double expectedIntervalMs_;
    uint64_t callbackIndex_ = 0;

    // Pre-allocated ring buffer for xrun events
    // (no allocation in audio callback)
    static constexpr size_t kMaxXruns = 1024;
    std::array<XrunEvent, kMaxXruns> xrunLog_;
    std::atomic<size_t> xrunCount_{0};

public:
    // @param bufferSize  Audio buffer size in samples
    // @param sampleRate  Sample rate in Hz
    DropoutDetector(int bufferSize, double sampleRate)
        : expectedIntervalMs_(
              1000.0 * bufferSize / sampleRate) {}

    // Call at the START of every audio callback.
    // Returns true if an xrun was detected.
    bool checkCallback() {
        auto now = Clock::now();
        ++callbackIndex_;

        if (firstCallback_) {
            firstCallback_ = false;
            lastCallbackTime_ = now;
            return false;
        }

        double intervalMs = std::chrono::duration<double, std::milli>(
            now - lastCallbackTime_).count();
        lastCallbackTime_ = now;

        // Threshold: 1.5x expected interval indicates a missed callback
        if (intervalMs > expectedIntervalMs_ * 1.5) {
            size_t idx = xrunCount_.load(std::memory_order_relaxed);
            if (idx < kMaxXruns) {
                xrunLog_[idx] = {now, expectedIntervalMs_, intervalMs,
                                  callbackIndex_};
                xrunCount_.store(idx + 1, std::memory_order_release);
            }
            return true;
        }
        return false;
    }

    size_t getXrunCount() const {
        return xrunCount_.load(std::memory_order_acquire);
    }

    // Call from main thread -- not real-time safe
    void saveLog(const std::string& path) const {
        std::ofstream out(path);
        out << "index,timestamp_us,expected_ms,actual_ms,ratio\n";
        size_t count = xrunCount_.load(std::memory_order_acquire);
        auto t0 = count > 0 ? xrunLog_[0].timestamp : Clock::now();

        for (size_t i = 0; i < count; ++i) {
            auto& x = xrunLog_[i];
            auto us = std::chrono::duration_cast<
                std::chrono::microseconds>(x.timestamp - t0).count();
            out << x.callbackIndex << "," << us << ","
                << x.expectedIntervalMs << ","
                << x.actualIntervalMs << ","
                << (x.actualIntervalMs / x.expectedIntervalMs) << "\n";
        }
    }
};
```

### 7.2 JACK Xrun Callback

JACK provides a direct xrun notification callback:

```cpp
#include <jack/jack.h>

static std::atomic<int> jackXrunCount{0};

int jackXrunCallback(void* /*arg*/) {
    // This is called by JACK when an xrun occurs.
    // Incrementing an atomic is real-time safe.
    jackXrunCount.fetch_add(1, std::memory_order_relaxed);
    return 0;
}

// During setup:
// jack_set_xrun_callback(client, jackXrunCallback, nullptr);
```

### 7.3 ALSA Xrun Recovery

ALSA signals xruns through error return codes from `snd_pcm_readi`/`snd_pcm_writei` (`-EPIPE`). The standard recovery procedure:

```cpp
#include <alsa/asoundlib.h>

int handleAlsaXrun(snd_pcm_t* handle, int err) {
    if (err == -EPIPE) {
        // Underrun (output) or overrun (input)
        err = snd_pcm_prepare(handle);
        if (err < 0) {
            fprintf(stderr, "ALSA: cannot recover from xrun: %s\n",
                    snd_strerror(err));
        }
        return err;
    } else if (err == -ESTRPIPE) {
        // Suspended -- wait until resume
        while ((err = snd_pcm_resume(handle)) == -EAGAIN) {
            usleep(1000);  // wait 1ms and retry
        }
        if (err < 0) {
            err = snd_pcm_prepare(handle);
        }
        return err;
    }
    return err;
}
```

### 7.4 CoreAudio Overload Notification (macOS)

CoreAudio does not provide a direct xrun callback, but you can detect overloads via the `kAudioDeviceProcessorOverload` property listener:

```cpp
#include <CoreAudio/CoreAudio.h>

static std::atomic<int> coreAudioOverloads{0};

OSStatus overloadListener(AudioObjectID device, UInt32 numAddresses,
                           const AudioObjectPropertyAddress addresses[],
                           void* clientData) {
    for (UInt32 i = 0; i < numAddresses; ++i) {
        if (addresses[i].mSelector ==
            kAudioDeviceProcessorOverload) {
            coreAudioOverloads.fetch_add(1, std::memory_order_relaxed);
        }
    }
    return noErr;
}

// During setup:
void registerOverloadListener(AudioDeviceID deviceID) {
    AudioObjectPropertyAddress addr = {
        kAudioDeviceProcessorOverload,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    AudioObjectAddPropertyListener(deviceID, &addr,
                                    overloadListener, nullptr);
}
```

---

## 8. Stress Testing

Stress tests push the system to its limits to find failure modes that do not appear under normal conditions.

### 8.1 Minimum Buffer Size Testing

Test at progressively smaller buffer sizes until xruns appear. This determines the minimum viable buffer size for the target hardware.

```cpp
// Stress test: run analysis at various buffer sizes for 60 seconds each.
// Record xrun count at each buffer size.
struct BufferSizeTestResult {
    int bufferSize;
    int xrunCount;
    double cpuUsagePercent;
    double maxFrameTimeMs;
};

std::vector<BufferSizeTestResult> stressTestBufferSizes(
    const std::vector<int>& bufferSizes, double testDurationSec = 60.0) {

    std::vector<BufferSizeTestResult> results;

    for (int bufSize : bufferSizes) {
        // Reconfigure audio engine with new buffer size
        AudioEngine engine;
        engine.setBufferSize(bufSize);
        engine.setSampleRate(48000.0);

        DropoutDetector detector(bufSize, 48000.0);
        AnalysisProfiler profiler;

        engine.setCallback([&](float* input, float* output, int frames) {
            detector.checkCallback();
            profiler.beginFrame();
            // ... run analysis ...
            profiler.endFrame();
        });

        engine.start();
        std::this_thread::sleep_for(
            std::chrono::duration<double>(testDurationSec));
        engine.stop();

        results.push_back({
            bufSize,
            static_cast<int>(detector.getXrunCount()),
            0.0,  // fill from profiler
            0.0   // fill from profiler
        });

        std::cout << "Buffer " << bufSize << " samples: "
                  << detector.getXrunCount() << " xruns\n";
    }
    return results;
}

// Test matrix for buffer size stress test:
// stressTestBufferSizes({32, 64, 128, 256, 512, 1024});
```

### 8.2 CPU Load Simulation

Inject artificial CPU load into the analysis thread to determine how much headroom exists. This simulates the effect of adding more analysis algorithms.

```cpp
// Artificial CPU burn: spin for a given number of microseconds
// Uses volatile to prevent compiler optimization
void burnCPU(double microseconds) {
    auto start = std::chrono::steady_clock::now();
    volatile double x = 1.0;
    while (true) {
        auto elapsed = std::chrono::duration_cast<
            std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start).count();
        if (elapsed >= microseconds) break;
        // Prevent optimization
        x = std::sin(x) + std::cos(x);
    }
}

// Stress test: progressively increase load until xruns appear
void stressTestCPULoad(int bufferSize, double sampleRate) {
    double hopDurationUs = 1e6 * bufferSize / sampleRate;

    for (double loadPercent = 10.0; loadPercent <= 100.0;
         loadPercent += 10.0) {
        double burnUs = hopDurationUs * loadPercent / 100.0;

        // Run for 30 seconds at this load level
        DropoutDetector detector(bufferSize, sampleRate);
        // ... configure audio engine with callback that calls
        //     burnCPU(burnUs) after analysis ...

        std::cout << "Load " << loadPercent << "% ("
                  << burnUs / 1000.0 << " ms): "
                  << detector.getXrunCount() << " xruns\n";

        if (detector.getXrunCount() > 0) {
            std::cout << "Maximum safe load: "
                      << (loadPercent - 10.0) << "%\n";
            break;
        }
    }
}
```

### 8.3 Long-Running Stability Test

Run the full pipeline for hours to detect memory leaks, accumulating drift, numerical instability, or gradual CPU usage increase.

```cpp
// Long-running stability test harness
void longRunningStabilityTest(double durationHours = 24.0) {
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::duration<double>(
        durationHours * 3600.0);

    DropoutDetector detector(128, 48000.0);
    size_t sampleIntervalSec = 60;  // Log stats every minute
    auto nextSample = startTime +
        std::chrono::seconds(sampleIntervalSec);

    struct StabilitySnapshot {
        double elapsedMinutes;
        size_t residentMemoryKB;
        double cpuUsage;
        size_t xrunCount;
    };
    std::vector<StabilitySnapshot> snapshots;
    snapshots.reserve(
        static_cast<size_t>(durationHours * 60) + 1);

    // ... start audio engine ...

    while (std::chrono::steady_clock::now() < endTime) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (std::chrono::steady_clock::now() >= nextSample) {
            double elapsed = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - startTime).count() / 60.0;

            // Get resident memory (platform-specific)
            size_t rssKB = 0;
#ifdef __APPLE__
            struct mach_task_basic_info info;
            mach_msg_type_number_t size = MACH_TASK_BASIC_INFO_COUNT;
            if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                          (task_info_t)&info, &size) == KERN_SUCCESS) {
                rssKB = info.resident_size / 1024;
            }
#elif defined(__linux__)
            std::ifstream statm("/proc/self/statm");
            size_t pages;
            statm >> pages >> pages;  // second field is RSS
            rssKB = pages * (sysconf(_SC_PAGESIZE) / 1024);
#endif

            snapshots.push_back({elapsed, rssKB, 0.0,
                                  detector.getXrunCount()});

            // Check for memory growth (potential leak)
            if (snapshots.size() > 10) {
                size_t firstMem = snapshots[5].residentMemoryKB;
                size_t lastMem = snapshots.back().residentMemoryKB;
                double growthPercent =
                    100.0 * (lastMem - firstMem) /
                    static_cast<double>(firstMem);
                if (growthPercent > 10.0) {
                    std::cerr << "WARNING: Memory grew by "
                              << growthPercent << "% ("
                              << firstMem << " -> " << lastMem
                              << " KB)\n";
                }
            }

            nextSample += std::chrono::seconds(sampleIntervalSec);
        }
    }

    // Final report
    std::cout << "\n=== Stability Test Complete ===\n"
              << "Duration: " << durationHours << " hours\n"
              << "Xruns: " << detector.getXrunCount() << "\n"
              << "Memory start: " << snapshots.front().residentMemoryKB
              << " KB\n"
              << "Memory end: " << snapshots.back().residentMemoryKB
              << " KB\n";
}
```

### 8.4 Test Matrix

| Test                        | Buffer Size | Duration | Pass Criteria                              |
|-----------------------------|-------------|----------|--------------------------------------------|
| Min buffer stress           | 32          | 60s      | Document xrun count (informational)         |
| Min buffer stress           | 64          | 60s      | <5 xruns                                    |
| Target buffer stress        | 128         | 60s      | 0 xruns                                     |
| Target buffer stress        | 256         | 60s      | 0 xruns                                     |
| CPU load 50%                | 128         | 60s      | 0 xruns                                     |
| CPU load 70%                | 128         | 60s      | 0 xruns                                     |
| CPU load 90%                | 128         | 60s      | Document xrun behavior                      |
| Long-running stability      | 128         | 1h       | 0 xruns, <5% memory growth                  |
| Long-running stability      | 128         | 24h      | <10 xruns, <10% memory growth               |
| Multi-device (2 inputs)     | 128         | 300s     | 0 xruns, correct per-device features        |

---

## 9. Accuracy Metrics

### 9.1 BPM Accuracy Metrics

**Absolute Error:** `|estimated - true|`. Simple but does not handle octave errors.

**P-Score (McKinney & Moelants, 2004):** Measures the similarity between two pulse trains. A predicted BPM generates a pulse train, which is cross-correlated with the ground-truth pulse train. The P-Score is the maximum cross-correlation value, normalized to [0, 1]. A P-Score of 1.0 means perfect phase-locked tempo estimation.

**Continuity-based Metrics (CMLT, CMST, AMLt, AMLc):**
- **CMLt (Correct Metrical Level, total):** Fraction of beats that match ground truth within a tolerance (typically 17.5% of the beat period), at the correct metrical level.
- **CMLc (Correct Metrical Level, continuity):** Longest continuous run of correct beats as a fraction of total.
- **AMLt:** Same as CMLt but allows octave errors (half-time, double-time).
- **AMLc:** Same as CMLc but allows octave errors.

```cpp
// BPM evaluation metrics
struct BPMEvaluation {
    double absoluteError;     // |estimated - true|
    bool exactCorrect;        // within 4% of true BPM
    bool octaveCorrect;       // within 4% of true BPM * {0.5, 1, 2, 3, 1/3}
    double pScore;            // pulse train correlation
};

BPMEvaluation evaluateBPM(double estimatedBPM, double trueBPM) {
    BPMEvaluation eval;
    eval.absoluteError = std::abs(estimatedBPM - trueBPM);
    eval.exactCorrect = eval.absoluteError < 0.04 * trueBPM;

    eval.octaveCorrect = false;
    for (double factor : {1.0, 2.0, 0.5, 3.0, 1.0/3.0}) {
        if (std::abs(estimatedBPM - trueBPM * factor) <
            0.04 * trueBPM * factor) {
            eval.octaveCorrect = true;
            break;
        }
    }

    // Simplified P-Score: cross-correlation of pulse trains
    // Full implementation requires beat phase estimation
    eval.pScore = 0.0;  // placeholder -- full impl below

    return eval;
}
```

### 9.2 Onset Detection: Precision, Recall, F-Measure

The standard onset detection evaluation from MIREX uses a tolerance window `w` (typically 50ms). Each ground-truth onset is matched to at most one detected onset.

```cpp
struct OnsetEvaluation {
    float precision;   // TP / (TP + FP) -- how many detections are correct
    float recall;      // TP / (TP + FN) -- how many true onsets are found
    float fMeasure;    // harmonic mean of precision and recall
    int truePositives;
    int falsePositives;
    int falseNegatives;
};

OnsetEvaluation evaluateOnsets(
    const std::vector<double>& detectedSec,
    const std::vector<double>& groundTruthSec,
    double toleranceSec = 0.050) {

    OnsetEvaluation eval{};
    std::vector<bool> gtMatched(groundTruthSec.size(), false);
    std::vector<bool> detMatched(detectedSec.size(), false);

    // Greedy matching: for each detected onset, find the closest
    // unmatched ground-truth onset within tolerance
    for (size_t d = 0; d < detectedSec.size(); ++d) {
        double bestDist = toleranceSec;
        int bestGT = -1;
        for (size_t g = 0; g < groundTruthSec.size(); ++g) {
            if (gtMatched[g]) continue;
            double dist = std::abs(detectedSec[d] - groundTruthSec[g]);
            if (dist < bestDist) {
                bestDist = dist;
                bestGT = static_cast<int>(g);
            }
        }
        if (bestGT >= 0) {
            gtMatched[bestGT] = true;
            detMatched[d] = true;
            ++eval.truePositives;
        }
    }

    eval.falsePositives = static_cast<int>(detectedSec.size()) -
                          eval.truePositives;
    eval.falseNegatives = static_cast<int>(groundTruthSec.size()) -
                          eval.truePositives;

    eval.precision = detectedSec.empty() ? 0.0f :
        static_cast<float>(eval.truePositives) / detectedSec.size();
    eval.recall = groundTruthSec.empty() ? 0.0f :
        static_cast<float>(eval.truePositives) / groundTruthSec.size();
    eval.fMeasure = (eval.precision + eval.recall > 0.0f) ?
        2.0f * eval.precision * eval.recall /
            (eval.precision + eval.recall)
        : 0.0f;

    return eval;
}
```

### 9.3 Pitch Detection Accuracy

**Raw Pitch Accuracy (RPA):** Fraction of voiced frames where the estimated pitch is within 50 cents (half semitone) of the ground truth. The comparison is done in log-frequency space: `|1200 * log2(estimated/true)| < 50`.

**Raw Chroma Accuracy (RCA):** Same as RPA but ignoring octave errors (pitch class only).

**Gross Error Rate (GER):** Fraction of voiced frames with pitch error > 50 cents. GER = 1 - RPA.

```cpp
struct PitchEvaluation {
    float rawPitchAccuracy;   // within 50 cents
    float rawChromaAccuracy;  // within 50 cents ignoring octave
    float grossErrorRate;     // > 50 cents error

    // Voicing detection metrics
    float voicingPrecision;
    float voicingRecall;
};

PitchEvaluation evaluatePitch(
    const std::vector<float>& estimatedHz,
    const std::vector<float>& groundTruthHz,
    const std::vector<bool>& groundTruthVoiced) {

    assert(estimatedHz.size() == groundTruthHz.size());
    assert(estimatedHz.size() == groundTruthVoiced.size());

    int voicedFrames = 0, correctPitch = 0, correctChroma = 0;

    for (size_t i = 0; i < estimatedHz.size(); ++i) {
        if (!groundTruthVoiced[i]) continue;
        if (estimatedHz[i] <= 0.0f || groundTruthHz[i] <= 0.0f) continue;
        ++voicedFrames;

        // Pitch difference in cents
        double cents = 1200.0 * std::log2(
            estimatedHz[i] / groundTruthHz[i]);

        if (std::abs(cents) < 50.0) {
            ++correctPitch;
        }

        // Chroma accuracy: reduce to within one octave
        double chromaCents = std::fmod(std::abs(cents), 1200.0);
        if (chromaCents > 600.0) chromaCents = 1200.0 - chromaCents;
        if (chromaCents < 50.0) {
            ++correctChroma;
        }
    }

    PitchEvaluation eval;
    eval.rawPitchAccuracy = voicedFrames > 0 ?
        static_cast<float>(correctPitch) / voicedFrames : 0.0f;
    eval.rawChromaAccuracy = voicedFrames > 0 ?
        static_cast<float>(correctChroma) / voicedFrames : 0.0f;
    eval.grossErrorRate = 1.0f - eval.rawPitchAccuracy;

    return eval;
}
```

### 9.4 Key Detection: MIREX Scoring

MIREX key detection scoring awards partial credit for musically related keys:

| Relationship                     | Score | Example (if true = C major)    |
|----------------------------------|-------|-------------------------------|
| Correct                         | 1.0   | C major                       |
| Perfect fifth                   | 0.5   | G major                       |
| Relative major/minor            | 0.3   | A minor                       |
| Parallel major/minor            | 0.2   | C minor                       |
| Other                           | 0.0   | anything else                  |

```cpp
enum class KeyQuality { Major, Minor };

struct Key {
    int pitchClass;      // 0=C, 1=C#, ..., 11=B
    KeyQuality quality;
};

float mirexKeyScore(Key estimated, Key groundTruth) {
    if (estimated.pitchClass == groundTruth.pitchClass &&
        estimated.quality == groundTruth.quality) {
        return 1.0f;  // Correct
    }

    // Perfect fifth: 7 semitones up, same quality
    int fifth = (groundTruth.pitchClass + 7) % 12;
    if (estimated.pitchClass == fifth &&
        estimated.quality == groundTruth.quality) {
        return 0.5f;
    }

    // Relative major/minor
    // Relative minor of C major is A minor (pitchClass - 3)
    // Relative major of A minor is C major (pitchClass + 3)
    int relativePitch;
    KeyQuality relativeQuality;
    if (groundTruth.quality == KeyQuality::Major) {
        relativePitch = (groundTruth.pitchClass + 9) % 12;  // -3 mod 12
        relativeQuality = KeyQuality::Minor;
    } else {
        relativePitch = (groundTruth.pitchClass + 3) % 12;
        relativeQuality = KeyQuality::Major;
    }
    if (estimated.pitchClass == relativePitch &&
        estimated.quality == relativeQuality) {
        return 0.3f;
    }

    // Parallel major/minor: same pitch class, different quality
    if (estimated.pitchClass == groundTruth.pitchClass &&
        estimated.quality != groundTruth.quality) {
        return 0.2f;
    }

    return 0.0f;
}
```

### 9.5 Accuracy Target Matrix

| Algorithm           | Metric                | Target (synthetic) | Target (real music) |
|---------------------|-----------------------|--------------------|---------------------|
| FFT magnitude       | Peak bin accuracy      | Exact (0 error)    | N/A                 |
| RMS                 | Relative error         | <0.1%              | N/A                 |
| Spectral centroid   | Absolute error (Hz)    | <1 bin width       | <2 bin widths       |
| Onset detection     | F-measure (50ms tol)   | >0.99              | >0.80               |
| BPM estimation      | Accuracy (4% tol)      | 100%               | >85% (MIREX level)  |
| BPM estimation      | Octave-correct         | 100%               | >95%                |
| Pitch (if used)     | Raw Pitch Accuracy     | >0.99              | >0.85               |
| Key (if used)       | MIREX weighted score   | N/A                | >0.70               |

---

## 10. Continuous Integration for Audio

Audio algorithm tests are pure compute -- they do not require audio hardware. This makes them ideal for CI pipelines. The test signal generators produce deterministic output, and all validation is numerical comparison.

### 10.1 CI Pipeline Structure

```yaml
# .github/workflows/audio-tests.yml
name: Audio Analysis Tests

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  unit-tests:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        build-type: [Release, Debug]
        compiler: [gcc-13, clang-17]

    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            libfftw3-dev \
            libsndfile1-dev \
            libasound2-dev

      - name: Configure
        run: |
          cmake -B build \
            -DCMAKE_BUILD_TYPE=${{ matrix.build-type }} \
            -DCMAKE_CXX_COMPILER=${{ matrix.compiler }} \
            -DBUILD_TESTS=ON

      - name: Build
        run: cmake --build build -j$(nproc)

      - name: Run unit tests
        run: |
          cd build
          ctest --output-on-failure --timeout 120

      - name: Run accuracy benchmarks
        run: |
          ./build/audio_accuracy_benchmark \
            --reporter=junit \
            --out=accuracy_results.xml

      - name: Upload test results
        uses: actions/upload-artifact@v4
        with:
          name: test-results-${{ matrix.compiler }}-${{ matrix.build-type }}
          path: accuracy_results.xml

  performance-regression:
    runs-on: ubuntu-latest
    needs: unit-tests

    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y libfftw3-dev libsndfile1-dev

      - name: Build release
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
          cmake --build build -j$(nproc)

      - name: Run performance benchmark
        run: |
          ./build/audio_perf_benchmark \
            --benchmark-samples=100 \
            --output=perf_results.json

      - name: Check for regressions
        run: |
          python3 scripts/check_perf_regression.py \
            --baseline=perf_baseline.json \
            --current=perf_results.json \
            --threshold=10  # fail if >10% slower

  sanitizer-checks:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Build with ASan + UBSan
        run: |
          cmake -B build \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
            -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" \
            -DBUILD_TESTS=ON
          cmake --build build -j$(nproc)

      - name: Run tests under sanitizers
        run: |
          cd build
          ASAN_OPTIONS=detect_leaks=1 \
          UBSAN_OPTIONS=print_stacktrace=1 \
          ctest --output-on-failure --timeout 300

  thread-safety:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Build with TSan
        run: |
          cmake -B build \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_CXX_COMPILER=clang++-17 \
            -DCMAKE_CXX_FLAGS="-fsanitize=thread" \
            -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread" \
            -DBUILD_TESTS=ON
          cmake --build build -j$(nproc)

      - name: Run threading stress test
        run: |
          cd build
          ctest -R "threading|concurrent|ringbuffer" \
            --output-on-failure --timeout 300
```

### 10.2 Performance Regression Detection

Track per-algorithm execution time across commits. Flag any commit that increases processing time by more than 10%.

```cpp
// audio_perf_benchmark.cpp
// Uses Catch2 BENCHMARK macro for microbenchmarking

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "fft_analyzer.h"
#include "spectral_features.h"
#include "onset_detector.h"
#include "test_signal_generators.h"

TEST_CASE("FFT performance", "[benchmark]") {
    constexpr int kFFTSize = 4096;
    auto signal = generateWhiteNoise(kFFTSize);
    FFTAnalyzer fft(kFFTSize);

    BENCHMARK("FFT-4096") {
        fft.applyHannWindow(signal.data(), kFFTSize);
        return fft.computeMagnitudeSpectrum(signal.data(), kFFTSize);
    };
}

TEST_CASE("Onset detection performance", "[benchmark]") {
    constexpr int kHopSize = 512;
    auto signal = generateWhiteNoise(kHopSize);
    OnsetDetector detector(48000.0, 2048, kHopSize);

    BENCHMARK("onset-detect-hop") {
        return detector.processFrame(signal.data(), kHopSize);
    };
}

TEST_CASE("Full analysis frame performance", "[benchmark]") {
    constexpr int kFFTSize = 2048;
    constexpr int kHopSize = 512;
    auto signal = generateWhiteNoise(kHopSize);
    AnalysisPipeline pipeline(48000.0, kFFTSize, kHopSize);

    BENCHMARK("full-analysis-frame") {
        return pipeline.processFrame(signal.data(), kHopSize);
    };
}
```

### 10.3 Regression Check Script

```python
#!/usr/bin/env python3
# scripts/check_perf_regression.py
"""Compare current benchmark results against baseline.
Exit with error code 1 if any benchmark regressed by more than threshold%."""

import json
import sys
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', required=True)
    parser.add_argument('--current', required=True)
    parser.add_argument('--threshold', type=float, default=10.0,
                        help='Max allowed regression in percent')
    args = parser.parse_args()

    with open(args.baseline) as f:
        baseline = json.load(f)
    with open(args.current) as f:
        current = json.load(f)

    regressions = []
    for name, base_ns in baseline.items():
        if name not in current:
            continue
        curr_ns = current[name]
        change_pct = 100.0 * (curr_ns - base_ns) / base_ns
        status = "REGRESSION" if change_pct > args.threshold else "OK"
        print(f"  {name}: {base_ns:.0f} -> {curr_ns:.0f} ns "
              f"({change_pct:+.1f}%) [{status}]")
        if change_pct > args.threshold:
            regressions.append((name, change_pct))

    if regressions:
        print(f"\nFAILED: {len(regressions)} benchmark(s) regressed "
              f"by >{args.threshold}%")
        sys.exit(1)
    else:
        print(f"\nPASSED: No regressions above {args.threshold}% threshold")

if __name__ == '__main__':
    main()
```

### 10.4 CI Test Categories and Execution Time Targets

| Test Category          | CI Stage          | Timeout  | Required for Merge |
|------------------------|-------------------|----------|--------------------|
| Unit tests (synthetic) | unit-tests        | 120s     | Yes                |
| FFT accuracy           | unit-tests        | 30s      | Yes                |
| Onset F-measure        | unit-tests        | 60s      | Yes                |
| BPM accuracy           | unit-tests        | 60s      | Yes                |
| Performance benchmarks | perf-regression   | 300s     | Yes (no regression)|
| ASan + UBSan           | sanitizer-checks  | 300s     | Yes                |
| TSan (threading)       | thread-safety     | 300s     | Yes                |
| Dataset validation     | nightly           | 3600s    | No (informational) |
| Long-running stability | weekly            | 86400s   | No (informational) |

### 10.5 Sanitizer Integration Notes

**AddressSanitizer (ASan):** Detects buffer overflows, use-after-free, double-free. Critical for audio code where off-by-one errors in buffer indexing produce silent corruption rather than crashes.

**UndefinedBehaviorSanitizer (UBSan):** Catches signed integer overflow, null dereference, misaligned access. Integer overflow in sample index calculations is a common bug in long-running audio sessions.

**ThreadSanitizer (TSan):** Detects data races between the audio callback thread and the analysis thread. Cannot run simultaneously with ASan (they conflict). Essential for validating that lock-free data structures are correctly implemented.

**MemorySanitizer (MSan):** Detects reads of uninitialized memory. Catches bugs where an FFT output buffer is read before the FFT is computed (e.g., on the first frame before enough samples have accumulated).

---

## Summary: Test Execution Checklist

Before declaring the analysis pipeline production-ready, all of the following must pass:

1. All unit tests pass for FFT, RMS, spectral centroid, onset detection, and BPM on synthetic signals
2. Onset detection F-measure > 0.80 on at least one reference dataset (ODB or MIREX)
3. BPM accuracy > 85% (octave-correct) on GTZAN or equivalent
4. Online validation matches offline validation within 1 hop alignment
5. Zero xruns at target buffer size (128 or 256 samples) for 1 hour
6. Analysis thread CPU usage < 70% of hop budget at target buffer size
7. No memory growth > 5% over 1 hour run
8. ASan, UBSan, and TSan report zero issues
9. Performance benchmarks show no regression > 10% from baseline
10. Latency (audio event to feature output) < 25 ms at target configuration
