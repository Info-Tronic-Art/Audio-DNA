#include "WaveformDisplay.h"
#include "ui/LookAndFeel.h"

WaveformDisplay::WaveformDisplay(const AnalysisThread& analysisThread)
    : analysisThread_(analysisThread)
{
    startTimerHz(30);
}

void WaveformDisplay::timerCallback()
{
    analysisThread_.getWaveformSamples(displayBuffer_.data(), displayCount_);
    repaint();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(bounds, 4.0f);

    if (displayCount_ < 2)
        return;

    const float midY = bounds.getCentreY();
    const float halfH = bounds.getHeight() * 0.45f;
    const float xStep = bounds.getWidth() / static_cast<float>(displayCount_ - 1);

    // Draw waveform
    juce::Path waveform;
    waveform.startNewSubPath(bounds.getX(),
                             midY - displayBuffer_[0] * halfH);

    for (int i = 1; i < displayCount_; ++i)
    {
        float x = bounds.getX() + static_cast<float>(i) * xStep;
        float y = midY - displayBuffer_[static_cast<size_t>(i)] * halfH;
        waveform.lineTo(x, y);
    }

    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.8f));
    g.strokePath(waveform, juce::PathStrokeType(1.5f));

    // Center line
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.3f));
    g.drawHorizontalLine(static_cast<int>(midY), bounds.getX(), bounds.getRight());
}
