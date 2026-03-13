#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "analysis/AnalysisThread.h"

class AudioReadoutPanel : public juce::Component, private juce::Timer
{
public:
    explicit AudioReadoutPanel(const AnalysisThread& analysisThread);

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    const AnalysisThread& analysisThread_;
    float displayRMS_ = 0.0f;
    float displayPeak_ = 0.0f;

    void drawMeter(juce::Graphics& g, juce::Rectangle<float> bounds,
                   const juce::String& label, float value);
};
