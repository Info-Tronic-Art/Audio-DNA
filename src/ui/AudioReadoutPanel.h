#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "analysis/AnalysisThread.h"
#include "features/FeatureBus.h"
#include "analysis/FeatureSnapshot.h"

// Left panel showing all audio feature values updating in real-time.
// Reads from the FeatureBus to display comprehensive audio analysis.
class AudioReadoutPanel : public juce::Component, private juce::Timer
{
public:
    AudioReadoutPanel(const AnalysisThread& analysisThread, FeatureBus& featureBus);

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    FeatureBus& featureBus_;

    // Smoothed display snapshot
    FeatureSnapshot displaySnap_{};
    bool hasData_ = false;

    // Onset flash animation
    float onsetFlash_ = 0.0f;

    // Helper drawing methods
    float drawSection(juce::Graphics& g, float y, float width, const juce::String& title);
    float drawMeter(juce::Graphics& g, float y, float width,
                    const juce::String& label, float value, juce::Colour color);
    float drawLabel(juce::Graphics& g, float y, float width,
                    const juce::String& label, const juce::String& value);

    // Key name helper
    static juce::String keyName(int key, bool isMajor);
    static juce::String structStateName(uint8_t state);
};
