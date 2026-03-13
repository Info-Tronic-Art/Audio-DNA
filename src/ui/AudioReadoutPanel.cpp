#include "AudioReadoutPanel.h"
#include "ui/LookAndFeel.h"
#include <cmath>

AudioReadoutPanel::AudioReadoutPanel(const AnalysisThread& analysisThread)
    : analysisThread_(analysisThread)
{
    startTimerHz(30);
}

void AudioReadoutPanel::timerCallback()
{
    // Smooth toward target with simple EMA
    constexpr float alpha = 0.3f;
    float targetRMS = analysisThread_.getRMS();
    float targetPeak = analysisThread_.getPeak();
    displayRMS_ += alpha * (targetRMS - displayRMS_);
    displayPeak_ += alpha * (targetPeak - displayPeak_);
    repaint();
}

void AudioReadoutPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(bounds, 4.0f);

    auto area = bounds.reduced(8.0f);

    // Title
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold)));
    g.drawText("Audio", area.removeFromTop(24.0f), juce::Justification::centredLeft);

    area.removeFromTop(8.0f);

    // RMS meter
    drawMeter(g, area.removeFromTop(28.0f), "RMS", displayRMS_);
    area.removeFromTop(6.0f);

    // Peak meter
    drawMeter(g, area.removeFromTop(28.0f), "Peak", displayPeak_);

    // dB readouts
    area.removeFromTop(12.0f);
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    float rmsDB = (displayRMS_ > 0.0f)
                      ? 20.0f * std::log10(displayRMS_)
                      : -100.0f;
    float peakDB = (displayPeak_ > 0.0f)
                       ? 20.0f * std::log10(displayPeak_)
                       : -100.0f;

    g.drawText(juce::String("RMS:  ") + juce::String(rmsDB, 1) + " dB",
               area.removeFromTop(18.0f), juce::Justification::centredLeft);
    g.drawText(juce::String("Peak: ") + juce::String(peakDB, 1) + " dB",
               area.removeFromTop(18.0f), juce::Justification::centredLeft);
}

void AudioReadoutPanel::drawMeter(juce::Graphics& g, juce::Rectangle<float> bounds,
                                   const juce::String& label, float value)
{
    auto labelArea = bounds.removeFromLeft(40.0f);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    g.drawText(label, labelArea, juce::Justification::centredLeft);

    auto meterBounds = bounds.reduced(0.0f, 4.0f);

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
    g.fillRoundedRectangle(meterBounds, 2.0f);

    // Fill
    float clampedVal = std::clamp(value, 0.0f, 1.0f);
    auto fillBounds = meterBounds.withWidth(meterBounds.getWidth() * clampedVal);

    juce::Colour meterColour;
    if (clampedVal < 0.6f)
        meterColour = juce::Colour(AudioDNALookAndFeel::kMeterGreen);
    else if (clampedVal < 0.85f)
        meterColour = juce::Colour(AudioDNALookAndFeel::kMeterYellow);
    else
        meterColour = juce::Colour(AudioDNALookAndFeel::kMeterRed);

    g.setColour(meterColour);
    g.fillRoundedRectangle(fillBounds, 2.0f);
}
