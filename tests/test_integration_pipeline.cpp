#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "analysis/FFTProcessor.h"
#include "analysis/SpectralFeatures.h"
#include "analysis/OnsetDetector.h"
#include "analysis/BPMTracker.h"
#include "analysis/MFCCExtractor.h"
#include "analysis/ChromaExtractor.h"
#include "analysis/KeyDetector.h"
#include "analysis/LoudnessAnalyzer.h"
#include "analysis/StructuralDetector.h"
#include "analysis/PitchTracker.h"
#include "analysis/FeatureSnapshot.h"
#include "features/FeatureBus.h"
#include "audio/RingBuffer.h"
#include <array>
#include <cmath>
#include <vector>

using Catch::Approx;

// Pipeline constants (must match AnalysisThread)
static constexpr int kBlockSize  = 2048;
static constexpr int kHopSize    = 512;
static constexpr int kSampleRate = 48000;
static constexpr int kNumBins    = FFTProcessor::kNumBins;  // 1025
static constexpr float kPI       = 3.14159265358979323846f;

// Generate a mono sine wave at a given frequency and amplitude.
static std::vector<float> generateSine(float freqHz, float amplitude, int numSamples)
{
    std::vector<float> out(static_cast<size_t>(numSamples));
    for (int i = 0; i < numSamples; ++i)
    {
        out[static_cast<size_t>(i)] = amplitude * std::sin(
            2.0f * kPI * freqHz * static_cast<float>(i) / static_cast<float>(kSampleRate));
    }
    return out;
}

// Run the full analysis pipeline on a block of audio, filling a FeatureSnapshot.
// This mirrors AnalysisThread::run() but is single-threaded and deterministic.
struct PipelineRunner
{
    FFTProcessor        fft;
    SpectralFeatures    spectral{kNumBins, static_cast<float>(kSampleRate), FFTProcessor::kFFTSize};
    OnsetDetector       onset{kHopSize, 1024, kSampleRate};
    BPMTracker          bpm{kHopSize, 1024, kSampleRate};
    MFCCExtractor       mfcc{kNumBins, static_cast<float>(kSampleRate), FFTProcessor::kFFTSize};
    ChromaExtractor     chroma{kNumBins, static_cast<float>(kSampleRate), FFTProcessor::kFFTSize};
    KeyDetector         key;
    LoudnessAnalyzer    loudness{kSampleRate};
    StructuralDetector  structural{static_cast<float>(kSampleRate), kHopSize};
    PitchTracker        pitch{kHopSize, FFTProcessor::kFFTSize, kSampleRate};

    // Overlap buffer (mirrors AnalysisThread)
    std::array<float, kBlockSize> analysisBuffer{};
    int samplesInBuffer = 0;

    // Transient density tracking
    static constexpr int kOnsetWindowSize = 256;
    std::array<bool, kOnsetWindowSize> onsetHistory{};
    int onsetHistoryPos = 0;
    int onsetCount = 0;
    float hopsPerSecond = static_cast<float>(kSampleRate) / static_cast<float>(kHopSize);
    uint64_t totalSamples = 0;

    PipelineRunner()
    {
        onsetHistory.fill(false);
    }

    // Feed one hop of audio and run the full pipeline. Returns true if a
    // full block was available and the snapshot was filled.
    bool processHop(const float* hopData, FeatureSnapshot& snap)
    {
        // Shift analysis buffer left by hop, append new samples
        if (samplesInBuffer >= kBlockSize)
        {
            std::memmove(analysisBuffer.data(),
                         analysisBuffer.data() + kHopSize,
                         static_cast<size_t>(kBlockSize - kHopSize) * sizeof(float));
            samplesInBuffer = kBlockSize - kHopSize;
        }

        std::memcpy(analysisBuffer.data() + samplesInBuffer,
                     hopData,
                     static_cast<size_t>(kHopSize) * sizeof(float));
        samplesInBuffer += kHopSize;
        totalSamples += kHopSize;

        if (samplesInBuffer < kBlockSize)
            return false;

        snap.clear();

        // 1. Raw time-domain: RMS, peak
        float sumSq = 0.0f;
        float peak = 0.0f;
        for (int i = 0; i < kBlockSize; ++i)
        {
            float s = analysisBuffer[static_cast<size_t>(i)];
            sumSq += s * s;
            float absS = std::fabs(s);
            if (absS > peak)
                peak = absS;
        }
        float rms = std::sqrt(sumSq / static_cast<float>(kBlockSize));
        snap.rms = rms;
        snap.peak = peak;
        snap.rmsDB = (rms > 1e-10f) ? 20.0f * std::log10(rms) : -100.0f;

        // 2. FFT
        fft.process(analysisBuffer.data(), kBlockSize);
        const float* mag = fft.magnitudeSpectrum();

        // 3. Spectral features
        spectral.process(mag);
        snap.spectralCentroid = spectral.centroid();
        snap.spectralFlux     = spectral.flux();
        snap.spectralFlatness = spectral.flatness();
        snap.spectralRolloff  = spectral.rolloff();
        std::memcpy(snap.bandEnergies, spectral.bandEnergies(), sizeof(snap.bandEnergies));

        // 4. Onset detection
        onset.process(hopData);
        snap.onsetDetected = onset.onsetDetected();
        snap.onsetStrength = onset.onsetStrength();

        // 5. BPM tracking + beat phase
        bpm.process(hopData);
        snap.bpm       = bpm.bpm();
        snap.beatPhase = bpm.beatPhase();

        // 6. MFCC
        mfcc.process(mag);
        std::memcpy(snap.mfccs, mfcc.mfccs(), sizeof(snap.mfccs));

        // 7. Chroma + HCDF
        chroma.process(mag);
        std::memcpy(snap.chromagram, chroma.chromagram(), sizeof(snap.chromagram));
        snap.harmonicChangeDetection = chroma.hcdf();

        // 8. Key detection
        key.process(chroma.chromagram());
        snap.detectedKey = key.detectedKey();
        snap.keyIsMajor  = key.isMajor();

        // 9. Pitch detection
        pitch.process(hopData);
        snap.dominantPitch  = pitch.dominantPitch();
        snap.pitchConfidence = pitch.confidence();

        // 10. Loudness
        loudness.process(hopData, kHopSize);
        snap.lufs         = loudness.lufs();
        snap.dynamicRange = loudness.dynamicRange();

        // 11. Transient density
        if (onsetHistory[static_cast<size_t>(onsetHistoryPos)])
            --onsetCount;
        onsetHistory[static_cast<size_t>(onsetHistoryPos)] = snap.onsetDetected;
        if (snap.onsetDetected)
            ++onsetCount;
        onsetHistoryPos = (onsetHistoryPos + 1) % kOnsetWindowSize;
        float windowDuration = static_cast<float>(kOnsetWindowSize) / hopsPerSecond;
        snap.transientDensity = static_cast<float>(onsetCount) / windowDuration;

        // 12. Structural detection
        structural.process(rms, spectral.flux(), snap.transientDensity);
        snap.structuralState = structural.structuralState();

        // Timing
        snap.timestamp = totalSamples;
        snap.wallClockSeconds = static_cast<double>(totalSamples) / static_cast<double>(kSampleRate);

        return true;
    }
};

// Feed a full signal through the pipeline, returning the last valid snapshot.
// Runs enough hops for the pipeline to stabilize.
static FeatureSnapshot runPipeline(const std::vector<float>& signal, PipelineRunner& runner)
{
    FeatureSnapshot lastSnap;
    lastSnap.clear();

    size_t offset = 0;
    while (offset + kHopSize <= signal.size())
    {
        FeatureSnapshot snap;
        if (runner.processHop(signal.data() + offset, snap))
            lastSnap = snap;
        offset += kHopSize;
    }
    return lastSnap;
}


// =============================================================================
// Integration Tests: 440 Hz Sine Wave
// =============================================================================

TEST_CASE("Integration: 440Hz sine — spectral centroid near 440Hz", "[integration]")
{
    // ~1 second of 440Hz sine at full amplitude
    auto signal = generateSine(440.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // Centroid should be close to 440Hz. Allow ±50Hz for windowing/leakage.
    REQUIRE(snap.spectralCentroid > 390.0f);
    REQUIRE(snap.spectralCentroid < 490.0f);
}

TEST_CASE("Integration: 440Hz sine — amplitude features", "[integration]")
{
    auto signal = generateSine(440.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // RMS of a sine with amplitude A is A/sqrt(2) ≈ 0.636
    REQUIRE(snap.rms > 0.5f);
    REQUIRE(snap.rms < 0.75f);

    // Peak should be near 0.9
    REQUIRE(snap.peak > 0.85f);
    REQUIRE(snap.peak <= 0.91f);

    // RMS in dB should be roughly -3 to -5 dBFS
    REQUIRE(snap.rmsDB > -6.0f);
    REQUIRE(snap.rmsDB < 0.0f);
}

TEST_CASE("Integration: 440Hz sine — spectral flatness is low (tonal)", "[integration]")
{
    auto signal = generateSine(440.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // A pure sine is highly tonal → flatness should be low
    REQUIRE(snap.spectralFlatness < 0.15f);
}

TEST_CASE("Integration: 440Hz sine — rolloff near 440Hz", "[integration]")
{
    auto signal = generateSine(440.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // All energy at 440Hz → rolloff should be at or near 440Hz
    // Allow a wider margin since windowing causes some spectral leakage
    REQUIRE(snap.spectralRolloff > 300.0f);
    REQUIRE(snap.spectralRolloff < 600.0f);
}

TEST_CASE("Integration: 440Hz sine — bass band has energy, not brilliance", "[integration]")
{
    auto signal = generateSine(440.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // 440Hz is in LowMid (250-500Hz) band, index 2
    // At minimum, LowMid should have more energy than Brilliance (6k-20k)
    float lowMid     = snap.bandEnergies[2];
    float brilliance = snap.bandEnergies[6];

    REQUIRE(lowMid > 0.0f);
    REQUIRE(lowMid > brilliance);
}

TEST_CASE("Integration: 440Hz sine — pitch detection near 440Hz", "[integration]")
{
    // Use a longer signal for pitch to stabilize
    auto signal = generateSine(440.0f, 0.9f, kSampleRate * 2);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // Aubio yinfft should detect pitch near 440Hz
    // Allow generous margin — pitch detection on pure sine can vary
    if (snap.pitchConfidence > 0.5f)
    {
        REQUIRE(snap.dominantPitch > 400.0f);
        REQUIRE(snap.dominantPitch < 480.0f);
    }
}

TEST_CASE("Integration: 440Hz sine — chroma peaks on A", "[integration]")
{
    // A4 = 440Hz, chroma index 9 (A)
    auto signal = generateSine(440.0f, 0.9f, kSampleRate * 2);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // Find the dominant chroma bin
    int maxBin = 0;
    float maxVal = snap.chromagram[0];
    for (int i = 1; i < 12; ++i)
    {
        if (snap.chromagram[i] > maxVal)
        {
            maxVal = snap.chromagram[i];
            maxBin = i;
        }
    }

    // A = chroma index 9
    REQUIRE(maxBin == 9);
}

TEST_CASE("Integration: 440Hz sine — LUFS is reasonable", "[integration]")
{
    auto signal = generateSine(440.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // A loud sine wave should produce LUFS somewhere around -4 to -1
    // K-weighting slightly boosts mid frequencies
    REQUIRE(snap.lufs > -10.0f);
    REQUIRE(snap.lufs < 0.0f);
}

TEST_CASE("Integration: 440Hz sine — MFCCs are computed", "[integration]")
{
    auto signal = generateSine(440.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // MFCC[0] is the log energy — should be nonzero for a loud signal
    // Just verify the pipeline ran and produced values
    bool anyNonZero = false;
    for (int i = 0; i < 13; ++i)
    {
        if (snap.mfccs[i] != 0.0f)
        {
            anyNonZero = true;
            break;
        }
    }
    REQUIRE(anyNonZero);
}

TEST_CASE("Integration: 440Hz sine — steady tone has no onsets", "[integration]")
{
    // A continuous sine with no attacks should produce very few onsets
    // after the initial onset. Allow the first ~0.5s to settle, then check.
    auto signal = generateSine(440.0f, 0.9f, kSampleRate * 2);
    PipelineRunner runner;

    // Run first second to let detectors stabilize
    FeatureSnapshot warmup;
    size_t offset = 0;
    int warmupHops = kSampleRate / kHopSize;  // ~1 second of hops
    for (int h = 0; h < warmupHops && offset + kHopSize <= signal.size(); ++h)
    {
        runner.processHop(signal.data() + offset, warmup);
        offset += kHopSize;
    }

    // Count onsets in remaining signal
    int onsetCount = 0;
    int hopCount = 0;
    while (offset + kHopSize <= signal.size())
    {
        FeatureSnapshot snap;
        if (runner.processHop(signal.data() + offset, snap))
        {
            if (snap.onsetDetected)
                ++onsetCount;
            ++hopCount;
        }
        offset += kHopSize;
    }

    // Steady tone: should have very few or zero onsets after warmup
    // Allow up to 3 spurious onsets (Aubio may trigger on windowing artifacts)
    REQUIRE(onsetCount <= 3);
}

TEST_CASE("Integration: 440Hz sine — structural state is stable", "[integration]")
{
    // A constant-level sine should settle to a stable state (not "drop" = 2).
    // The structural detector may classify a constant loud signal as normal (0)
    // or buildup (1) depending on envelope dynamics, but it should not be "drop".
    auto signal = generateSine(440.0f, 0.9f, kSampleRate * 2);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // Should not be "drop" (2) since there's no sudden energy spike after buildup
    REQUIRE(snap.structuralState != 2);
}

// =============================================================================
// Integration Tests: Silence
// =============================================================================

TEST_CASE("Integration: silence — all features near zero/default", "[integration]")
{
    std::vector<float> silence(static_cast<size_t>(kSampleRate), 0.0f);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(silence, runner);

    REQUIRE(snap.rms == Approx(0.0f).margin(1e-6f));
    REQUIRE(snap.peak == Approx(0.0f).margin(1e-6f));
    REQUIRE(snap.rmsDB < -90.0f);
    REQUIRE(snap.spectralCentroid == Approx(0.0f).margin(1.0f));
    REQUIRE(snap.spectralFlux == Approx(0.0f).margin(0.01f));
    REQUIRE(snap.dominantPitch == Approx(0.0f).margin(1.0f));
    REQUIRE(snap.structuralState == 0);
}

// =============================================================================
// Integration Tests: 10kHz sine (high frequency)
// =============================================================================

TEST_CASE("Integration: 10kHz sine — centroid is high", "[integration]")
{
    auto signal = generateSine(10000.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // Centroid should be near 10kHz
    REQUIRE(snap.spectralCentroid > 9000.0f);
    REQUIRE(snap.spectralCentroid < 11000.0f);
}

TEST_CASE("Integration: 10kHz sine — brilliance band dominant", "[integration]")
{
    auto signal = generateSine(10000.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureSnapshot snap = runPipeline(signal, runner);

    // 10kHz falls in Brilliance band (6k-20k), index 6
    float brilliance = snap.bandEnergies[6];
    REQUIRE(brilliance > 0.0f);

    // Should dominate over bass
    REQUIRE(brilliance > snap.bandEnergies[1]);
}

// =============================================================================
// Integration Tests: Ring Buffer → Pipeline (end-to-end data flow)
// =============================================================================

TEST_CASE("Integration: ring buffer feeds pipeline correctly", "[integration]")
{
    // Verify that pushing through a ring buffer and popping produces
    // the same results as direct pipeline feeding.
    // Push in chunks since ring buffer capacity (16384) < total signal size.
    auto signal = generateSine(440.0f, 0.9f, kSampleRate);

    RingBuffer<float> rb(16384);
    PipelineRunner runner;

    FeatureSnapshot lastSnap;
    lastSnap.clear();
    std::array<float, kHopSize> hopBuf{};

    size_t pushOffset = 0;
    constexpr size_t kPushChunk = 4096;

    while (pushOffset < signal.size())
    {
        // Push a chunk
        size_t toWrite = std::min(kPushChunk, signal.size() - pushOffset);
        size_t written = rb.push(signal.data() + pushOffset, toWrite);
        pushOffset += written;

        // Pop and process available hops
        while (rb.availableToRead() >= static_cast<size_t>(kHopSize))
        {
            rb.pop(hopBuf.data(), kHopSize);
            FeatureSnapshot snap;
            if (runner.processHop(hopBuf.data(), snap))
                lastSnap = snap;
        }
    }

    // Should get same centroid result as direct feeding
    REQUIRE(lastSnap.spectralCentroid > 390.0f);
    REQUIRE(lastSnap.spectralCentroid < 490.0f);
    REQUIRE(lastSnap.rms > 0.5f);
}

// =============================================================================
// Integration Tests: Feature Bus publishes correctly
// =============================================================================

TEST_CASE("Integration: pipeline publishes to FeatureBus", "[integration]")
{
    auto signal = generateSine(440.0f, 0.9f, kSampleRate);
    PipelineRunner runner;
    FeatureBus bus;

    size_t offset = 0;
    int publishCount = 0;

    while (offset + kHopSize <= signal.size())
    {
        FeatureSnapshot snap;
        if (runner.processHop(signal.data() + offset, snap))
        {
            // Publish to FeatureBus as AnalysisThread would
            FeatureSnapshot* ws = bus.acquireWrite();
            *ws = snap;
            bus.publishWrite();
            ++publishCount;
        }
        offset += kHopSize;
    }

    REQUIRE(publishCount > 0);

    // Read back from FeatureBus
    const FeatureSnapshot* rs = bus.acquireRead();
    REQUIRE(rs != nullptr);
    REQUIRE(rs->spectralCentroid > 390.0f);
    REQUIRE(rs->spectralCentroid < 490.0f);
    REQUIRE(rs->rms > 0.5f);
}
