#include "SpectrumDisplay.h"
#include "ui/LookAndFeel.h"
#include <cmath>
#include <algorithm>

SpectrumDisplay::SpectrumDisplay(FeatureBus& featureBus)
    : featureBus_(featureBus)
{
    displayBands_.fill(0.0f);
    startTimerHz(30);
}

void SpectrumDisplay::timerCallback()
{
    // AudioReadoutPanel calls acquireRead() — we just read the latest
    const FeatureSnapshot* snap = featureBus_.getLatestRead();
    if (snap == nullptr)
        return;

    // Smooth toward target with EMA
    constexpr float alpha = 0.3f;
    for (int i = 0; i < 7; ++i)
    {
        auto idx = static_cast<size_t>(i);
        displayBands_[idx] += alpha * (snap->bandEnergies[idx] - displayBands_[idx]);
    }

    repaint();
}

void SpectrumDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(bounds, 4.0f);

    auto area = bounds.reduced(8.0f);

    // Title
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    g.drawText("Spectrum", area.removeFromTop(20.0f), juce::Justification::centredLeft);

    area.removeFromTop(4.0f);

    // Reserve space for labels at bottom
    auto labelArea = area.removeFromBottom(16.0f);

    // Draw 7 vertical bars
    float barWidth = area.getWidth() / 7.0f;
    float gap = 2.0f;

    for (int i = 0; i < 7; ++i)
    {
        auto idx = static_cast<size_t>(i);
        float x = area.getX() + static_cast<float>(i) * barWidth;
        auto barBounds = juce::Rectangle<float>(x + gap, area.getY(),
                                                 barWidth - gap * 2.0f, area.getHeight());

        // Background slot
        g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
        g.fillRoundedRectangle(barBounds, 2.0f);

        // Filled portion (bottom-up)
        float val = std::clamp(displayBands_[idx], 0.0f, 1.0f);
        float fillHeight = barBounds.getHeight() * val;
        auto fillBounds = barBounds.withTop(barBounds.getBottom() - fillHeight);

        g.setColour(juce::Colour(kBandColors[idx]));
        g.fillRoundedRectangle(fillBounds, 2.0f);

        // Label
        auto lbl = juce::Rectangle<float>(x, labelArea.getY(), barWidth, labelArea.getHeight());
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText(kBandLabels[idx], lbl, juce::Justification::centred);
    }
}
