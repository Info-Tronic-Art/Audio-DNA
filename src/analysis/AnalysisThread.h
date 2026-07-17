#pragma once
#include <juce_core/juce_core.h>
#include <atomic>
#include <array>
#include <cstdint>
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
class GenreDetector;
class AdvancedAudioAnalyzer;

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
//  13. Genre detection
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

    // Access BPM tracker for resync/phrase configuration
    BPMTracker* getBpmTracker() { return bpmTracker_.get(); }

    static constexpr int kWaveformBufferSize = 2048;

    // Get recent PCM samples for external consumers (e.g., projectM).
    // Returns number of samples copied (up to maxSamples, max kPCMSnapshotSize).
    // Lock-free: reads from an atomic-swapped snapshot.
    static constexpr int kPCMSnapshotSize = 512;
    int getPCMSamples(float* dest, int maxSamples) const;

private:
    RingBuffer<float>& ringBuffer_;

    // Analysis state
    std::array<float, kBlockSize> analysisBuffer_{};
    int samplesInBuffer_ = 0;

    // Quick atomic accessors (backward-compatible)
    std::atomic<float> currentRMS_{0.0f};
    std::atomic<float> currentPeak_{0.0f};

    // Waveform display buffer (seqlock). The reader retries whenever the writer
    // bumps the version mid-read, so it never returns a torn/half-written buffer —
    // regardless of thread scheduling. atomic<float> elements make the concurrent
    // access race-free; the release/acquire fences order the data against the
    // version counter. (A plain double buffer — as the PCM path below uses — only
    // NARROWS the torn-read window: a reader preempted long enough for the single
    // writer to lap the two slots can still tear. The torn-read stress test caught
    // exactly that, so the waveform path uses a seqlock instead.)
    alignas(64) std::array<std::atomic<float>, kWaveformBufferSize> waveformBuffer_{};
    std::atomic<std::uint32_t> waveformSeq_{0};  // even = stable, odd = write in progress
    std::atomic<int> waveformSampleCount_{0};

    // PCM snapshot for external consumers (lock-free double buffer)
    alignas(64) std::array<float, kPCMSnapshotSize> pcmSnapshot_[2]{};
    std::atomic<int> pcmWriteIdx_{0};  // toggles 0/1
    std::atomic<int> pcmSampleCount_{0};

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
    std::unique_ptr<GenreDetector> genreDetector_;
    std::unique_ptr<AdvancedAudioAnalyzer> advancedAnalyzer_;

    // Transient density: sliding window onset counter
    static constexpr int kOnsetWindowSize = 256;  // ~2.7 seconds at 93.75 hops/sec
    std::array<bool, kOnsetWindowSize> onsetHistory_{};
    int onsetHistoryPos_ = 0;
    int onsetCount_ = 0;
    float hopsPerSecond_ = 0.0f;

    // Cached HCDF from previous hop (for downbeat scoring in stage 5,
    // since chroma HCDF is computed in stage 7)
    float prevHCDF_ = 0.0f;

    // Cached structural state from previous hop (for phrase reset in BPMTracker,
    // since structural detection is computed in stage 11)
    uint8_t prevStructuralState_ = 0;

    // Sample counter for timestamps
    uint64_t totalSamplesProcessed_ = 0;

    // CPU load tracking (percentage of hop period used for analysis)
    std::atomic<float> cpuLoad_{0.0f};

    // Per-stage profiling (logged periodically)
    static constexpr int kNumStages = 14;
    std::array<double, kNumStages> stageTimesUs_{};
    int profileFrameCount_ = 0;
    static constexpr int kProfileInterval = 500;  // Log every N hops (~5.3s)

public:
    float getCpuLoad() const { return cpuLoad_.load(std::memory_order_relaxed); }
};
