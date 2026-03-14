#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "features/FeatureBus.h"
#include <array>

// Displays 7-band energy bars in real-time, painted at 30fps.
// Reads from the FeatureBus to get the latest band energies.
class SpectrumDisplay : public juce::Component, private juce::Timer
{
public:
    explicit SpectrumDisplay(FeatureBus& featureBus);

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    FeatureBus& featureBus_;

    // Smoothed display values
    std::array<float, 7> displayBands_{};

    // Band labels
    static constexpr const char* kBandLabels[7] = {
        "Sub", "Bass", "LMid", "Mid", "HMid", "Pres", "Bril"
    };

    // Band colors (warm to cool gradient)
    static constexpr juce::uint32 kBandColors[7] = {
        0xffff1744,  // Sub - red
        0xffff6d00,  // Bass - orange
        0xffffea00,  // LowMid - yellow
        0xff00e676,  // Mid - green
        0xff00e5ff,  // HighMid - cyan
        0xff2979ff,  // Presence - blue
        0xffaa00ff   // Brilliance - purple
    };
};
