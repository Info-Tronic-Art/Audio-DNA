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
    float displayBands_[7] = {};
    bool hasData_ = false;

    // Onset flash animation
    float onsetFlash_ = 0.0f;

    // Helper drawing methods
    float drawSection(juce::Graphics& g, float y, float width, const juce::String& title);
    float drawMeter(juce::Graphics& g, float y, float width,
                    const juce::String& label, float value, juce::Colour color);
    float drawDbMeter(juce::Graphics& g, float y, float width,
                      const juce::String& label, float dbValue, float minDb, float maxDb);
    float drawLabel(juce::Graphics& g, float y, float width,
                    const juce::String& label, const juce::String& value);
    float drawBandMeters(juce::Graphics& g, float y, float width);
    float drawBeatPhase(juce::Graphics& g, float y, float width);
    float drawOnsetIndicator(juce::Graphics& g, float y, float width);
    float drawStructuralState(juce::Graphics& g, float y, float width);

    // Key name helper
    static juce::String keyName(int key, bool isMajor);
    static juce::String structStateName(uint8_t state);
    static juce::Colour structStateColour(uint8_t state);

    // Band colors
    static constexpr juce::uint32 kBandColors[7] = {
        0xffff1744,  // Sub - red
        0xffff6d00,  // Bass - orange
        0xffffab00,  // Low Mid - amber
        0xff00e676,  // Mid - green
        0xff00bcd4,  // High Mid - teal
        0xff2979ff,  // Presence - blue
        0xff7c4dff,  // Brilliance - purple
    };

    static constexpr const char* kBandNames[7] = {
        "Sub", "Bass", "LMid", "Mid", "HMid", "Pres", "Bril"
    };
};
