#include "SpectrumDisplay.h"
#include "ui/LookAndFeel.h"
#include <cmath>
#include <algorithm>

SpectrumDisplay::SpectrumDisplay(FeatureBus& featureBus)
    : featureBus_(featureBus)
{
    displayBands_.fill(0.0f);
    peakHold_.fill(0.0f);
    peakTimer_.fill(0);
    startTimerHz(30);
}

void SpectrumDisplay::timerCallback()
{
    const FeatureSnapshot* snap = featureBus_.getLatestRead();
    if (snap == nullptr)
        return;

    for (int i = 0; i < 7; ++i)
    {
        auto idx = static_cast<size_t>(i);
        float target = snap->bandEnergies[idx];

        // Fast attack, slow release
        float alpha = (target > displayBands_[idx]) ? kAttackAlpha : kReleaseAlpha;
        displayBands_[idx] += alpha * (target - displayBands_[idx]);

        // Peak hold
        if (displayBands_[idx] >= peakHold_[idx])
        {
            peakHold_[idx] = displayBands_[idx];
            peakTimer_[idx] = kPeakHoldFrames;
        }
        else if (peakTimer_[idx] > 0)
        {
            --peakTimer_[idx];
        }
        else
        {
            peakHold_[idx] *= kPeakDecay;
        }
    }

    repaint();
}

void SpectrumDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

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
        g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground).withAlpha(0.6f));
        g.fillRoundedRectangle(barBounds, 2.0f);

        // Filled portion (bottom-up) with gradient
        float val = std::clamp(displayBands_[idx], 0.0f, 1.0f);
        float fillHeight = barBounds.getHeight() * val;
        auto fillBounds = barBounds.withTop(barBounds.getBottom() - fillHeight);

        if (fillHeight > 1.0f)
        {
            auto bandCol = juce::Colour(kBandColors[idx]);
            // Gradient: dimmer at bottom, full brightness at top
            g.setGradientFill(juce::ColourGradient(
                bandCol, fillBounds.getX(), fillBounds.getY(),
                bandCol.withAlpha(0.4f), fillBounds.getX(), fillBounds.getBottom(),
                false));
            g.fillRoundedRectangle(fillBounds, 2.0f);

            // Bright cap at top of bar
            auto capBounds = fillBounds.withHeight(std::min(3.0f, fillHeight));
            g.setColour(bandCol.brighter(0.3f));
            g.fillRoundedRectangle(capBounds, 1.0f);
        }

        // Peak hold marker
        float peakVal = std::clamp(peakHold_[idx], 0.0f, 1.0f);
        if (peakVal > 0.01f)
        {
            float peakY = barBounds.getBottom() - barBounds.getHeight() * peakVal;
            g.setColour(juce::Colour(kBandColors[idx]).withAlpha(0.8f));
            g.fillRect(barBounds.getX(), peakY, barBounds.getWidth(), 2.0f);
        }

        // Label
        auto lbl = juce::Rectangle<float>(x, labelArea.getY(), barWidth, labelArea.getHeight());
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText(kBandLabels[idx], lbl, juce::Justification::centred);
    }
}
