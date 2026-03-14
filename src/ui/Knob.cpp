#include "Knob.h"
#include "ui/LookAndFeel.h"

Knob::Knob(const juce::String& name)
{
    // Rotary slider
    slider_.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    slider_.setRange(0.0, 1.0, 0.001);
    slider_.setColour(juce::Slider::rotarySliderFillColourId,
                      juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    slider_.setColour(juce::Slider::rotarySliderOutlineColourId,
                      juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
    addAndMakeVisible(slider_);

    // Name label
    nameLabel_.setText(name, juce::dontSendNotification);
    nameLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    nameLabel_.setColour(juce::Label::textColourId,
                         juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    nameLabel_.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(nameLabel_);

    // Value label (shown below the knob, above the name)
    valueLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    valueLabel_.setColour(juce::Label::textColourId,
                          juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    valueLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(valueLabel_);

    // Update value label when slider changes
    slider_.onValueChange = [this]
    {
        auto val = slider_.getValue();
        valueLabel_.setText(juce::String(val, 2), juce::dontSendNotification);
    };

    // Initial value display
    valueLabel_.setText("0.00", juce::dontSendNotification);
}

void Knob::paint(juce::Graphics& g)
{
    if (!mapped_)
        return;

    // Draw mapping indicator ring behind the slider
    auto sliderBounds = slider_.getBounds().toFloat().reduced(1.0f);
    auto radius = juce::jmin(sliderBounds.getWidth(), sliderBounds.getHeight()) / 2.0f;
    auto centreX = sliderBounds.getCentreX();
    auto centreY = sliderBounds.getCentreY();

    auto indicatorRadius = radius + 1.0f;
    auto arcWidth = 2.0f;

    // Full arc in magenta to show "this param is mapped"
    float startAngle = juce::MathConstants<float>::pi * 1.25f;
    float endAngle = juce::MathConstants<float>::pi * 2.75f;

    juce::Path arc;
    arc.addCentredArc(centreX, centreY,
                      indicatorRadius, indicatorRadius,
                      0.0f, startAngle, endAngle, true);

    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.6f));
    g.strokePath(arc, juce::PathStrokeType(arcWidth,
                 juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void Knob::resized()
{
    auto area = getLocalBounds();

    // Name label at the bottom
    nameLabel_.setBounds(area.removeFromBottom(14));

    // Value label just above the name
    valueLabel_.setBounds(area.removeFromBottom(12));

    // Slider takes the rest
    slider_.setBounds(area);
}

void Knob::setParamName(const juce::String& name)
{
    nameLabel_.setText(name, juce::dontSendNotification);
}

void Knob::setMappingIndicator(const juce::String& sourceName)
{
    mapped_ = sourceName.isNotEmpty();
    mappedSourceName_ = sourceName;
    repaint();
}
