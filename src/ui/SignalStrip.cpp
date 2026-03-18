#include "SignalStrip.h"
#include <cmath>
#include <algorithm>

SignalStrip::SignalStrip(Signal& signal, const SignalRegistry& registry)
    : signal_(signal), registry_(registry)
{
}

void SignalStrip::updateValue(float newValue)
{
    // Smooth the display value
    constexpr float smoothAlpha = 0.3f;
    smoothedValue_ += smoothAlpha * (newValue - smoothedValue_);
    displayValue_ = smoothedValue_;

    // Peak hold
    if (displayValue_ > peakValue_)
    {
        peakValue_ = displayValue_;
        peakHoldTimer_ = kPeakHoldFrames;
    }
    else if (peakHoldTimer_ > 0)
    {
        --peakHoldTimer_;
    }
    else
    {
        peakValue_ *= kPeakDecay;
    }

    // Flash on trigger signals (onset/hit)
    if (signal_.getName() == "Hit" && newValue > 0.3f)
        flashAlpha_ = 1.0f;
    else
        flashAlpha_ *= 0.85f;
}

void SignalStrip::setDisplaySize(DisplaySize size)
{
    displaySize_ = size;
}

int SignalStrip::getPreferredWidth() const
{
    switch (displaySize_)
    {
        case DisplaySize::Minimized: return 36;
        case DisplaySize::Normal:    return 40;
        case DisplaySize::Expanded:  return 120;
    }
    return 40;
}

int SignalStrip::getPreferredHeight() const
{
    switch (displaySize_)
    {
        case DisplaySize::Minimized: return 22;
        case DisplaySize::Normal:    return 80;
        case DisplaySize::Expanded:  return 300;
    }
    return 80;
}

void SignalStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    switch (displaySize_)
    {
        case DisplaySize::Minimized: paintMinimized(g, bounds); break;
        case DisplaySize::Normal:    paintNormal(g, bounds); break;
        case DisplaySize::Expanded:  paintExpanded(g, bounds); break;
    }
}

void SignalStrip::mouseDown(const juce::MouseEvent& /*event*/)
{
    if (onSelected)
        onSelected(signal_);
}

//==============================================================================

void SignalStrip::paintMinimized(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto col = getSignalColour();

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(bounds, 2.0f);

    // Small square indicator fills based on value
    float indicatorSize = 10.0f;
    auto indicatorRect = juce::Rectangle<float>(
        bounds.getX() + 2.0f,
        bounds.getCentreY() - indicatorSize / 2.0f,
        indicatorSize, indicatorSize);

    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
    g.fillRect(indicatorRect);

    float fill = std::clamp(displayValue_, 0.0f, 1.0f);
    g.setColour(col.withAlpha(0.3f + fill * 0.7f));
    g.fillRect(indicatorRect.withHeight(indicatorRect.getHeight() * fill)
                   .withBottomY(indicatorRect.getBottom()));

    // Name (abbreviated)
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(8.0f)));
    auto nameStr = juce::String(signal_.getName());
    if (nameStr.length() > 4)
        nameStr = nameStr.substring(0, 3);
    g.drawText(nameStr,
               juce::Rectangle<float>(indicatorRect.getRight() + 2.0f, bounds.getY(),
                                      bounds.getWidth() - indicatorSize - 6.0f, bounds.getHeight()),
               juce::Justification::centredLeft);
}

void SignalStrip::paintNormal(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto col = getSignalColour();

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(bounds, 3.0f);

    // Border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawRoundedRectangle(bounds, 3.0f, 0.5f);

    // Flash overlay for trigger signals
    if (flashAlpha_ > 0.05f)
    {
        g.setColour(col.withAlpha(flashAlpha_ * 0.15f));
        g.fillRoundedRectangle(bounds, 3.0f);
    }

    float padding = 2.0f;
    float nameHeight = 12.0f;
    float valueHeight = 11.0f;

    // Value readout at top
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    g.drawText(getFormattedValue(),
               juce::Rectangle<float>(bounds.getX() + padding, bounds.getY() + padding,
                                      bounds.getWidth() - padding * 2.0f, valueHeight),
               juce::Justification::centred);

    // Vertical meter bar (the main visual)
    float meterTop = bounds.getY() + padding + valueHeight + 2.0f;
    float meterBottom = bounds.getBottom() - nameHeight - 4.0f;
    float meterHeight = meterBottom - meterTop;
    float meterWidth = std::min(bounds.getWidth() - padding * 2.0f - 2.0f, 20.0f);
    float meterX = bounds.getCentreX() - meterWidth / 2.0f;

    auto meterBg = juce::Rectangle<float>(meterX, meterTop, meterWidth, meterHeight);

    // Meter background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
    g.fillRoundedRectangle(meterBg, 2.0f);

    // Meter fill (bottom to top)
    float fill = std::clamp(displayValue_, 0.0f, 1.0f);
    float fillHeight = meterHeight * fill;
    auto fillRect = juce::Rectangle<float>(
        meterX, meterTop + meterHeight - fillHeight,
        meterWidth, fillHeight);
    g.setColour(col);
    g.fillRoundedRectangle(fillRect, 2.0f);

    // Peak hold line
    if (peakValue_ > 0.01f)
    {
        float peakY = meterTop + meterHeight * (1.0f - std::clamp(peakValue_, 0.0f, 1.0f));
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary).withAlpha(0.6f));
        g.fillRect(meterX, peakY, meterWidth, 1.0f);
    }

    // Name at bottom
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(8.0f)));
    auto nameStr = juce::String(signal_.getName());
    // Abbreviate long names for the strip
    if (nameStr == "Beat Position") nameStr = "Beat";
    else if (nameStr == "Sub Bass") nameStr = "Sub";
    else if (nameStr == "Hits Per Second") nameStr = "HPS";
    else if (nameStr == "Beat In Bar") nameStr = "Bar";
    else if (nameStr == "Hit Strength") nameStr = "HStr";
    else if (nameStr == "Note Confidence") nameStr = "NtCf";
    else if (nameStr == "Chord Change") nameStr = "Chrd";

    g.drawText(nameStr,
               juce::Rectangle<float>(bounds.getX() + padding,
                                      bounds.getBottom() - nameHeight - 1.0f,
                                      bounds.getWidth() - padding * 2.0f, nameHeight),
               juce::Justification::centred);
}

void SignalStrip::paintExpanded(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto col = getSignalColour();

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawRoundedRectangle(bounds, 3.0f, 0.5f);

    float padding = 4.0f;

    // Signal name at top
    g.setColour(col);
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText(juce::String(signal_.getName()),
               juce::Rectangle<float>(bounds.getX() + padding, bounds.getY() + padding,
                                      bounds.getWidth() - padding * 2.0f, 16.0f),
               juce::Justification::centred);

    // Value readout
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    g.drawText(getFormattedValue(),
               juce::Rectangle<float>(bounds.getX() + padding, bounds.getY() + 20.0f,
                                      bounds.getWidth() - padding * 2.0f, 18.0f),
               juce::Justification::centred);

    // Large vertical meter
    float meterTop = bounds.getY() + 42.0f;
    float meterBottom = bounds.getBottom() - 24.0f;
    float meterHeight = meterBottom - meterTop;
    float meterWidth = std::min(bounds.getWidth() - padding * 2.0f - 4.0f, 30.0f);
    float meterX = bounds.getCentreX() - meterWidth / 2.0f;

    auto meterBg = juce::Rectangle<float>(meterX, meterTop, meterWidth, meterHeight);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
    g.fillRoundedRectangle(meterBg, 2.0f);

    float fill = std::clamp(displayValue_, 0.0f, 1.0f);
    float fillHeight = meterHeight * fill;
    g.setColour(col);
    g.fillRoundedRectangle(
        juce::Rectangle<float>(meterX, meterTop + meterHeight - fillHeight,
                               meterWidth, fillHeight), 2.0f);

    // Peak hold
    if (peakValue_ > 0.01f)
    {
        float peakY = meterTop + meterHeight * (1.0f - std::clamp(peakValue_, 0.0f, 1.0f));
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary).withAlpha(0.6f));
        g.fillRect(meterX, peakY, meterWidth, 1.5f);
    }

    // Category label at bottom
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(9.0f)));
    juce::String catName;
    switch (signal_.getCategory())
    {
        case Signal::Category::Amplitude:  catName = "Amplitude";  break;
        case Signal::Category::Bands:      catName = "Bands";      break;
        case Signal::Category::Rhythm:     catName = "Rhythm";     break;
        case Signal::Category::Pitch:      catName = "Pitch";      break;
        case Signal::Category::Chroma:     catName = "Chroma";     break;
        case Signal::Category::Timbre:     catName = "Timbre";     break;
        case Signal::Category::Structure:  catName = "Structure";  break;
        case Signal::Category::Modulation: catName = "Modulation"; break;
    }
    g.drawText(catName,
               juce::Rectangle<float>(bounds.getX() + padding, bounds.getBottom() - 20.0f,
                                      bounds.getWidth() - padding * 2.0f, 16.0f),
               juce::Justification::centred);
}

//==============================================================================

juce::String SignalStrip::getFormattedValue() const
{
    auto name = signal_.getName();

    // Special formatting per signal type
    if (name == "Tempo")
    {
        if (displayValue_ > 0.0f)
            return juce::String(static_cast<int>(displayValue_ + 0.5f));
        return "---";
    }
    if (name == "Beat In Bar")
    {
        int beat = static_cast<int>(displayValue_) + 1;
        return juce::String(std::clamp(beat, 1, 4));
    }
    if (name == "Hit")
    {
        return displayValue_ > 0.3f ? juce::CharPointer_UTF8("\xe2\x97\x8f")
                                    : juce::CharPointer_UTF8("\xe2\x97\x8b");
    }
    if (name == "Beat Position" || name == "Bar Position")
    {
        return juce::String(displayValue_, 2);
    }

    // Default: two decimal places for [0,1] range, integer for larger values
    if (displayValue_ >= 10.0f)
        return juce::String(static_cast<int>(displayValue_ + 0.5f));
    return juce::String(displayValue_, 2);
}

juce::Colour SignalStrip::getSignalColour() const
{
    switch (signal_.getCategory())
    {
        case Signal::Category::Amplitude:
            return juce::Colour(AudioDNALookAndFeel::kMeterGreen);
        case Signal::Category::Bands:
        {
            // Color-code by band
            auto name = signal_.getName();
            if (name == "Sub Bass")  return juce::Colour(0xffff1744u);  // red
            if (name == "Bass")      return juce::Colour(0xffff6d00u);  // orange
            if (name == "Low Mid")   return juce::Colour(0xffffab00u);  // amber
            if (name == "Mid")       return juce::Colour(0xff00e676u);  // green
            if (name == "High Mid")  return juce::Colour(0xff00bcd4u);  // teal
            if (name == "Presence")  return juce::Colour(0xff2979ffu);  // blue
            if (name == "Air")       return juce::Colour(0xff7c4dffu);  // purple
            return juce::Colour(AudioDNALookAndFeel::kAccentCyan);
        }
        case Signal::Category::Rhythm:
            return juce::Colour(AudioDNALookAndFeel::kAccentMagenta);
        case Signal::Category::Pitch:
            return juce::Colour(0xff64ffdau);  // mint
        case Signal::Category::Chroma:
            return juce::Colour(0xffffd740u);  // gold
        case Signal::Category::Timbre:
            return juce::Colour(0xffb388ffu);  // lavender
        case Signal::Category::Structure:
            return juce::Colour(AudioDNALookAndFeel::kMeterYellow);
        case Signal::Category::Modulation:
            return juce::Colour(AudioDNALookAndFeel::kAccentCyan);
    }
    return juce::Colour(AudioDNALookAndFeel::kAccentCyan);
}
