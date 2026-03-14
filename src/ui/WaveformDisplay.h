#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "analysis/AnalysisThread.h"
#include <array>

// Scrolling waveform display with peak envelope and RMS fill.
//
// Accumulates waveform data into time slices. Each slice stores:
//   - min/max sample values (peak envelope)
//   - RMS value (filled area)
//
// Scrolls right-to-left as new audio arrives. Peak hold lines
// show recent maximum amplitude with slow decay.
class WaveformDisplay : public juce::Component, private juce::Timer
{
public:
    explicit WaveformDisplay(const AnalysisThread& analysisThread);

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    const AnalysisThread& analysisThread_;

    // Per-column data for the scrolling display
    struct Column
    {
        float minVal = 0.0f;    // Min sample in this time slice
        float maxVal = 0.0f;    // Max sample in this time slice
        float rms = 0.0f;       // RMS of this time slice
    };

    static constexpr int kMaxColumns = 512;
    std::array<Column, kMaxColumns> columns_{};
    int writePos_ = 0;          // Circular write position
    int columnCount_ = 0;       // Total columns written (up to kMaxColumns)

    // Peak hold
    float peakHoldPos_ = 0.0f;  // Current peak hold value (positive)
    float peakHoldNeg_ = 0.0f;  // Current peak hold value (negative)
    int peakHoldTimer_ = 0;     // Frames until decay starts
    static constexpr int kPeakHoldFrames = 30; // ~1 second at 30fps
    static constexpr float kPeakDecayRate = 0.97f;

    // Raw sample buffer for computing columns
    std::array<float, AnalysisThread::kWaveformBufferSize> rawBuffer_{};
    int rawCount_ = 0;
};
