#include "ProgrammingMode.h"

ProgrammingMode::ProgrammingMode(SignalBar& signalBar)
    : signalBar_(signalBar)
{
    addAndMakeVisible(headerLabel_);
    headerLabel_.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    headerLabel_.setColour(juce::Label::textColourId,
                           juce::Colour(AudioDNALookAndFeel::kAccentCyan));

    addAndMakeVisible(closeButton_);
    closeButton_.onClick = [this] { setActive(false); };
}

void ProgrammingMode::setActive(bool active)
{
    if (active_ == active)
        return;

    active_ = active;

    if (active_)
    {
        signalBar_.setDisplaySize(SignalStrip::DisplaySize::Expanded);
    }
    else
    {
        signalBar_.setDisplaySize(SignalStrip::DisplaySize::Normal);
    }

    setVisible(active_);

    // Trigger parent layout
    if (auto* parent = getParentComponent())
        parent->resized();
}

void ProgrammingMode::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Dark overlay background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground).withAlpha(0.95f));
    g.fillRect(bounds);

    // Border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f));
    g.drawRect(bounds, 1.0f);
}

void ProgrammingMode::resized()
{
    auto area = getLocalBounds().reduced(8);

    // Header row
    auto headerRow = area.removeFromTop(24);
    headerLabel_.setBounds(headerRow.removeFromLeft(200));
    closeButton_.setBounds(headerRow.removeFromRight(60));

    area.removeFromTop(4);

    // The signal bar occupies the rest (it's managed by parent,
    // this component just provides the overlay and controls)
}
