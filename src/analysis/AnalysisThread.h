#pragma once
#include <juce_core/juce_core.h>
#include <atomic>
#include <array>
#include <memory>
#include "FeatureSnapshot.h"
#include "audio/RingBuffer.h"
#include "features/FeatureBus.h"

// Forward declarations — avoid including heavy headers
class FFTProcessor;
class SpectralFeatures;
class OnsetDetector;
class BPMTracker;
class MFCCExtractor;
class ChromaExtractor;
class KeyDetector;
class LoudnessAnalyzer;
class StructuralDetector;
class PitchTracker;

// Dedicated thread that reads from ring buffer, runs the full audio analysis
// pipeline, and publishes FeatureSnapshots to the FeatureBus.
//
// Pipeline order (each step depends on prior results):
//   1. Raw time-domain: RMS, peak
//   2. FFT → magnitude spectrum
//   3. Spectral features: centroid, flux, flatness, rolloff, band energies
//   4. Onset detection
//   5. BPM tracking + beat phase
//   6. MFCC
//   7. Chroma + HCDF
//   8. Key detection
//   9. Pitch detection
//  10. Loudness (LUFS, dynamic range)
//  11. Structural detection
//  12. Transient density
class AnalysisThread : public juce::Thread
{
public:
    static constexpr int kBlockSize = 2048;
    static constexpr int kHopSize = 512;
    static constexpr int kSampleRate = 48000;

    explicit AnalysisThread(RingBuffer<float>& ringBuffer);
    ~AnalysisThread() override;

    void run() override;

    // --- Accessors for UI (backward-compatible) ---
    float getRMS() const { return currentRMS_.load(std::memory_order_relaxed); }
    float getPeak() const { return currentPeak_.load(std::memory_order_relaxed); }

    // Get the FeatureBus for reading snapshots
    FeatureBus& getFeatureBus() { return featureBus_; }
    const FeatureBus& getFeatureBus() const { return featureBus_; }

    // Get raw waveform samples for display (lock-free snapshot)
    void getWaveformSamples(float* dest, int& count) const;

    static constexpr int kWaveformBufferSize = 2048;

private:
    RingBuffer<float>& ringBuffer_;

    // Analysis state
    std::array<float, kBlockSize> analysisBuffer_{};
    int samplesInBuffer_ = 0;

    // Quick atomic accessors (backward-compatible)
    std::atomic<float> currentRMS_{0.0f};
    std::atomic<float> currentPeak_{0.0f};

    // Waveform display buffer
    alignas(64) std::array<float, kWaveformBufferSize> waveformBuffer_{};
    std::atomic<int> waveformSampleCount_{0};

    // Feature publishing
    FeatureBus featureBus_;

    // Analysis modules (owned, created at construction)
    std::unique_ptr<FFTProcessor> fftProcessor_;
    std::unique_ptr<SpectralFeatures> spectralFeatures_;
    std::unique_ptr<OnsetDetector> onsetDetector_;
    std::unique_ptr<BPMTracker> bpmTracker_;
    std::unique_ptr<MFCCExtractor> mfccExtractor_;
    std::unique_ptr<ChromaExtractor> chromaExtractor_;
    std::unique_ptr<KeyDetector> keyDetector_;
    std::unique_ptr<LoudnessAnalyzer> loudnessAnalyzer_;
    std::unique_ptr<StructuralDetector> structuralDetector_;
    std::unique_ptr<PitchTracker> pitchTracker_;

    // Transient density: sliding window onset counter
    static constexpr int kOnsetWindowSize = 256;  // ~2.7 seconds at 93.75 hops/sec
    std::array<bool, kOnsetWindowSize> onsetHistory_{};
    int onsetHistoryPos_ = 0;
    int onsetCount_ = 0;
    float hopsPerSecond_ = 0.0f;

    // Sample counter for timestamps
    uint64_t totalSamplesProcessed_ = 0;

    // CPU load tracking (percentage of hop period used for analysis)
    std::atomic<float> cpuLoad_{0.0f};

    // Per-stage profiling (logged periodically)
    static constexpr int kNumStages = 13;
    std::array<double, kNumStages> stageTimesUs_{};
    int profileFrameCount_ = 0;
    static constexpr int kProfileInterval = 500;  // Log every N hops (~5.3s)

public:
    float getCpuLoad() const { return cpuLoad_.load(std::memory_order_relaxed); }
};
