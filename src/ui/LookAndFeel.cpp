#include "LookAndFeel.h"

AudioDNALookAndFeel::AudioDNALookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(kBackground));
    setColour(juce::TextButton::buttonColourId, juce::Colour(kSurface));
    setColour(juce::TextButton::textColourOnId, juce::Colour(kTextPrimary));
    setColour(juce::TextButton::textColourOffId, juce::Colour(kTextPrimary));
    setColour(juce::Label::textColourId, juce::Colour(kTextPrimary));
    setColour(juce::Label::backgroundColourId, juce::Colour(0x00000000));

    setDefaultSansSerifTypeface(juce::Font(juce::FontOptions(14.0f)).getTypefacePtr());
}

void AudioDNALookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                 const juce::Colour&,
                                                 bool isMouseOver, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    auto baseColour = juce::Colour(kSurface);

    if (isButtonDown)
        baseColour = juce::Colour(kAccentCyan).withAlpha(0.3f);
    else if (isMouseOver)
        baseColour = juce::Colour(kSurfaceLight);

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(juce::Colour(kAccentCyan).withAlpha(0.4f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void AudioDNALookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                          bool, bool)
{
    g.setColour(juce::Colour(kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(14.0f)));
    g.drawText(button.getButtonText(), button.getLocalBounds(),
               juce::Justification::centred, false);
}
