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
    static constexpr juce::uint32 kPanelBorder   = 0xff3a3a5c;

    // --- Buttons ---
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool isMouseOver, bool isButtonDown) override;

    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool isMouseOver, bool isButtonDown) override;

    // --- Rotary Slider (Knob) ---
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    // --- Linear Slider ---
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;

    // --- Toggle Button ---
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    // --- ComboBox ---
    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox&) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override;

    // --- PopupMenu ---
    void drawPopupMenuBackground(juce::Graphics&, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    // --- Label ---
    void drawLabel(juce::Graphics&, juce::Label&) override;

    // --- ScrollBar ---
    void drawScrollbar(juce::Graphics&, juce::ScrollBar&, int x, int y,
                       int width, int height, bool isScrollbarVertical,
                       int thumbStartPosition, int thumbSize,
                       bool isMouseOver, bool isMouseDown) override;
};
