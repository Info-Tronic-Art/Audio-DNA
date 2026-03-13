#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "analysis/AnalysisThread.h"

class WaveformDisplay : public juce::Component, private juce::Timer
{
public:
    explicit WaveformDisplay(const AnalysisThread& analysisThread);

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    const AnalysisThread& analysisThread_;
    std::array<float, AnalysisThread::kWaveformBufferSize> displayBuffer_{};
    int displayCount_ = 0;
};
