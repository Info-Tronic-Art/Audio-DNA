#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AudioDNALookAndFeel : public juce::LookAndFeel_V4
{
public:
    AudioDNALookAndFeel();

    // Color constants
    static constexpr juce::uint32 kBackground    = 0xff1a1a2e;
    static constexpr juce::uint32 kSurface       = 0xff252540;
    static constexpr juce::uint32 kSurfaceLight  = 0xff30305a;
    static constexpr juce::uint32 kAccentCyan    = 0xff00e5ff;
    static constexpr juce::uint32 kAccentMagenta = 0xffff00e5;
    static constexpr juce::uint32 kTextPrimary   = 0xffe0e0e0;
    static constexpr juce::uint32 kTextSecondary = 0xff808090;
    static constexpr juce::uint32 kMeterGreen    = 0xff00e676;
    static constexpr juce::uint32 kMeterYellow   = 0xffffea00;
    static constexpr juce::uint32 kMeterRed      = 0xffff1744;

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool isMouseOver, bool isButtonDown) override;

    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool isMouseOver, bool isButtonDown) override;
};
