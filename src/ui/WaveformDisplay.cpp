#include "WaveformDisplay.h"
#include "ui/LookAndFeel.h"
#include <cmath>
#include <algorithm>

WaveformDisplay::WaveformDisplay(const AnalysisThread& analysisThread)
    : analysisThread_(analysisThread)
{
    startTimerHz(30);
}

void WaveformDisplay::timerCallback()
{
    analysisThread_.getWaveformSamples(rawBuffer_.data(), rawCount_);

    if (rawCount_ < 2)
        return;

    // Compute one column from the current raw buffer
    Column col;
    col.minVal = 0.0f;
    col.maxVal = 0.0f;
    float sumSq = 0.0f;

    for (int i = 0; i < rawCount_; ++i)
    {
        float s = rawBuffer_[static_cast<size_t>(i)];
        col.minVal = std::min(col.minVal, s);
        col.maxVal = std::max(col.maxVal, s);
        sumSq += s * s;
    }
    col.rms = std::sqrt(sumSq / static_cast<float>(rawCount_));

    // Push into circular buffer
    columns_[static_cast<size_t>(writePos_)] = col;
    writePos_ = (writePos_ + 1) % kMaxColumns;
    if (columnCount_ < kMaxColumns)
        ++columnCount_;

    // Update peak hold
    float peakAbs = std::max(std::abs(col.minVal), col.maxVal);
    if (peakAbs >= peakHoldPos_)
    {
        peakHoldPos_ = peakAbs;
        peakHoldNeg_ = -peakAbs;
        peakHoldTimer_ = kPeakHoldFrames;
    }
    else if (peakHoldTimer_ > 0)
    {
        --peakHoldTimer_;
    }
    else
    {
        peakHoldPos_ *= kPeakDecayRate;
        peakHoldNeg_ *= kPeakDecayRate;
    }

    repaint();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Panel border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    if (columnCount_ < 2)
        return;

    auto drawArea = bounds.reduced(4.0f, 4.0f);
    float midY = drawArea.getCentreY();
    float halfH = drawArea.getHeight() * 0.48f;

    // Center line
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.2f));
    g.drawHorizontalLine(static_cast<int>(midY), drawArea.getX(), drawArea.getRight());

    // How many columns fit in the draw area
    int numCols = std::min(columnCount_, static_cast<int>(drawArea.getWidth()));
    float colWidth = drawArea.getWidth() / static_cast<float>(numCols);

    // Read columns from oldest to newest (right-to-left scrolling)
    // Newest column is at writePos_ - 1, oldest at writePos_ - numCols
    int startIdx = (writePos_ - numCols + kMaxColumns) % kMaxColumns;

    // === RMS fill (underneath) ===
    {
        juce::Path rmsTop, rmsBottom;
        bool started = false;

        for (int i = 0; i < numCols; ++i)
        {
            int idx = (startIdx + i) % kMaxColumns;
            float x = drawArea.getX() + static_cast<float>(i) * colWidth;
            float rmsH = columns_[static_cast<size_t>(idx)].rms * halfH;

            if (!started)
            {
                rmsTop.startNewSubPath(x, midY - rmsH);
                rmsBottom.startNewSubPath(x, midY + rmsH);
                started = true;
            }
            else
            {
                rmsTop.lineTo(x, midY - rmsH);
                rmsBottom.lineTo(x, midY + rmsH);
            }
        }

        // Close the RMS shape
        juce::Path rmsFill;
        rmsFill.addPath(rmsTop);
        // Traverse bottom in reverse
        for (int i = numCols - 1; i >= 0; --i)
        {
            int idx = (startIdx + i) % kMaxColumns;
            float x = drawArea.getX() + static_cast<float>(i) * colWidth;
            float rmsH = columns_[static_cast<size_t>(idx)].rms * halfH;
            rmsFill.lineTo(x, midY + rmsH);
        }
        rmsFill.closeSubPath();

        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.15f));
        g.fillPath(rmsFill);
    }

    // === Peak envelope (min/max lines) ===
    {
        juce::Path peakTop, peakBot;

        for (int i = 0; i < numCols; ++i)
        {
            int idx = (startIdx + i) % kMaxColumns;
            float x = drawArea.getX() + static_cast<float>(i) * colWidth;
            float yMax = midY - columns_[static_cast<size_t>(idx)].maxVal * halfH;
            float yMin = midY - columns_[static_cast<size_t>(idx)].minVal * halfH;

            if (i == 0)
            {
                peakTop.startNewSubPath(x, yMax);
                peakBot.startNewSubPath(x, yMin);
            }
            else
            {
                peakTop.lineTo(x, yMax);
                peakBot.lineTo(x, yMin);
            }
        }

        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.7f));
        g.strokePath(peakTop, juce::PathStrokeType(1.0f));
        g.strokePath(peakBot, juce::PathStrokeType(1.0f));
    }

    // === Peak hold lines ===
    if (peakHoldPos_ > 0.001f)
    {
        float holdYTop = midY - peakHoldPos_ * halfH;
        float holdYBot = midY + peakHoldPos_ * halfH;

        g.setColour(juce::Colour(AudioDNALookAndFeel::kMeterYellow).withAlpha(0.6f));
        g.drawHorizontalLine(static_cast<int>(holdYTop), drawArea.getX(), drawArea.getRight());
        g.drawHorizontalLine(static_cast<int>(holdYBot), drawArea.getX(), drawArea.getRight());
    }
}
