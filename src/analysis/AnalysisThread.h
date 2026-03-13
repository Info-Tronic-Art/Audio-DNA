#pragma once
#include <juce_core/juce_core.h>
#include <atomic>
#include "FeatureSnapshot.h"
#include "audio/RingBuffer.h"

// Dedicated thread that reads from ring buffer, computes RMS per block.
// Publishes results via atomic snapshot.
class AnalysisThread : public juce::Thread
{
public:
    static constexpr int kBlockSize = 2048;
    static constexpr int kHopSize = 512;

    explicit AnalysisThread(RingBuffer<float>& ringBuffer);
    ~AnalysisThread() override;

    void run() override;

    // Read by UI/render thread
    float getRMS() const { return currentRMS_.load(std::memory_order_relaxed); }
    float getPeak() const { return currentPeak_.load(std::memory_order_relaxed); }

    // Get raw waveform samples for display (lock-free snapshot)
    void getWaveformSamples(float* dest, int& count) const;

    static constexpr int kWaveformBufferSize = 2048;

private:
    RingBuffer<float>& ringBuffer_;

    // Analysis state
    std::array<float, kBlockSize> analysisBuffer_{};
    int samplesInBuffer_ = 0;

    std::atomic<float> currentRMS_{0.0f};
    std::atomic<float> currentPeak_{0.0f};

    // Waveform display buffer — written by analysis thread, read by UI
    alignas(64) std::array<float, kWaveformBufferSize> waveformBuffer_{};
    std::atomic<int> waveformSampleCount_{0};
};
