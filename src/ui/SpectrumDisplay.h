#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "features/FeatureBus.h"
#include <array>

// Displays 7-band energy bars with smooth animation, peak hold markers,
// and gradient fills. Reads from the FeatureBus at 30fps.
class SpectrumDisplay : public juce::Component, private juce::Timer
{
public:
    explicit SpectrumDisplay(FeatureBus& featureBus);

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    FeatureBus& featureBus_;

    // Smoothed display values (fast attack, slow release)
    std::array<float, 7> displayBands_{};

    // Peak hold per band
    std::array<float, 7> peakHold_{};
    std::array<int, 7> peakTimer_{};
    static constexpr int kPeakHoldFrames = 20;   // ~0.67s at 30fps
    static constexpr float kPeakDecay = 0.95f;

    // Attack/release smoothing
    static constexpr float kAttackAlpha = 0.6f;   // Fast rise
    static constexpr float kReleaseAlpha = 0.08f;  // Slow fall

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
