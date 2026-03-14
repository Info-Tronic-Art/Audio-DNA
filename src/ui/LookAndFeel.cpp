#include "LookAndFeel.h"

AudioDNALookAndFeel::AudioDNALookAndFeel()
{
    // Window & general backgrounds
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(kBackground));

    // Buttons
    setColour(juce::TextButton::buttonColourId, juce::Colour(kSurface));
    setColour(juce::TextButton::textColourOnId, juce::Colour(kTextPrimary));
    setColour(juce::TextButton::textColourOffId, juce::Colour(kTextPrimary));

    // Labels
    setColour(juce::Label::textColourId, juce::Colour(kTextPrimary));
    setColour(juce::Label::backgroundColourId, juce::Colour(0x00000000));

    // Sliders
    setColour(juce::Slider::thumbColourId, juce::Colour(kAccentCyan));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(kAccentCyan));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(kSurfaceLight));
    setColour(juce::Slider::trackColourId, juce::Colour(kSurfaceLight));
    setColour(juce::Slider::backgroundColourId, juce::Colour(kSurface));
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(kTextPrimary));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(kSurface));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(kPanelBorder));

    // ComboBox
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(kSurface));
    setColour(juce::ComboBox::textColourId, juce::Colour(kTextPrimary));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(kPanelBorder));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(kAccentCyan));

    // PopupMenu
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(kSurface));
    setColour(juce::PopupMenu::textColourId, juce::Colour(kTextPrimary));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(kAccentCyan).withAlpha(0.2f));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(kAccentCyan));

    // ScrollBar
    setColour(juce::ScrollBar::thumbColourId, juce::Colour(kSurfaceLight));
    setColour(juce::ScrollBar::trackColourId, juce::Colour(kBackground));

    // TextEditor (used by slider text boxes)
    setColour(juce::TextEditor::backgroundColourId, juce::Colour(kSurface));
    setColour(juce::TextEditor::textColourId, juce::Colour(kTextPrimary));
    setColour(juce::TextEditor::outlineColourId, juce::Colour(kPanelBorder));
    setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(kAccentCyan));
    setColour(juce::TextEditor::highlightColourId, juce::Colour(kAccentCyan).withAlpha(0.3f));

    setDefaultSansSerifTypeface(juce::Font(juce::FontOptions(14.0f)).getTypefacePtr());
}

//==============================================================================
// Buttons
//==============================================================================

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

//==============================================================================
// Rotary Slider (Knob)
//==============================================================================

void AudioDNALookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y,
                                            int width, int height,
                                            float sliderPos,
                                            float rotaryStartAngle,
                                            float rotaryEndAngle,
                                            juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto diameter = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Outer track (background arc)
    auto trackWidth = 3.0f;
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f,
                                 radius - trackWidth * 0.5f,
                                 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(kSurfaceLight));
    g.strokePath(backgroundArc, juce::PathStrokeType(trackWidth,
                 juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Filled arc (value)
    if (sliderPos > 0.0f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius - trackWidth * 0.5f,
                               radius - trackWidth * 0.5f,
                               0.0f, rotaryStartAngle, angle, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, juce::PathStrokeType(trackWidth,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Inner knob circle
    auto knobRadius = radius * 0.6f;
    g.setColour(juce::Colour(kSurface));
    g.fillEllipse(centreX - knobRadius, centreY - knobRadius,
                  knobRadius * 2.0f, knobRadius * 2.0f);

    g.setColour(juce::Colour(kPanelBorder));
    g.drawEllipse(centreX - knobRadius, centreY - knobRadius,
                  knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

    // Pointer line
    juce::Path pointer;
    auto pointerLength = knobRadius * 0.7f;
    auto pointerThickness = 2.5f;
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -knobRadius + 2.0f,
                                 pointerThickness, pointerLength, 1.0f);
    pointer.applyTransform(juce::AffineTransform::rotation(angle)
                           .translated(centreX, centreY));
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.fillPath(pointer);
}

//==============================================================================
// Linear Slider
//==============================================================================

void AudioDNALookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y,
                                            int width, int height,
                                            float sliderPos, float minSliderPos,
                                            float maxSliderPos,
                                            juce::Slider::SliderStyle style,
                                            juce::Slider& slider)
{
    auto isHorizontal = (style == juce::Slider::LinearHorizontal ||
                         style == juce::Slider::LinearBar);

    auto trackThickness = 4.0f;

    if (isHorizontal)
    {
        auto trackY = static_cast<float>(y) + static_cast<float>(height) * 0.5f - trackThickness * 0.5f;
        auto trackLeft = static_cast<float>(x);
        auto trackRight = static_cast<float>(x + width);

        // Track background
        g.setColour(juce::Colour(kSurfaceLight));
        g.fillRoundedRectangle(trackLeft, trackY, trackRight - trackLeft,
                               trackThickness, trackThickness * 0.5f);

        // Filled portion
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillRoundedRectangle(trackLeft, trackY, sliderPos - trackLeft,
                               trackThickness, trackThickness * 0.5f);

        // Thumb
        auto thumbSize = 14.0f;
        auto thumbY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillEllipse(sliderPos - thumbSize * 0.5f, thumbY - thumbSize * 0.5f,
                      thumbSize, thumbSize);

        // Thumb glow on hover
        if (slider.isMouseOverOrDragging())
        {
            g.setColour(slider.findColour(juce::Slider::thumbColourId).withAlpha(0.15f));
            g.fillEllipse(sliderPos - thumbSize, thumbY - thumbSize,
                          thumbSize * 2.0f, thumbSize * 2.0f);
        }
    }
    else
    {
        // Vertical slider
        auto trackX = static_cast<float>(x) + static_cast<float>(width) * 0.5f - trackThickness * 0.5f;
        auto trackTop = static_cast<float>(y);
        auto trackBottom = static_cast<float>(y + height);

        // Track background
        g.setColour(juce::Colour(kSurfaceLight));
        g.fillRoundedRectangle(trackX, trackTop, trackThickness,
                               trackBottom - trackTop, trackThickness * 0.5f);

        // Filled portion (bottom-up)
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillRoundedRectangle(trackX, sliderPos, trackThickness,
                               trackBottom - sliderPos, trackThickness * 0.5f);

        // Thumb
        auto thumbSize = 14.0f;
        auto thumbX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumbX - thumbSize * 0.5f, sliderPos - thumbSize * 0.5f,
                      thumbSize, thumbSize);
    }
}

//==============================================================================
// Toggle Button
//==============================================================================

void AudioDNALookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto toggleSize = 16.0f;
    auto toggleX = bounds.getX() + 4.0f;
    auto toggleY = bounds.getCentreY() - toggleSize * 0.5f;
    auto toggleBounds = juce::Rectangle<float>(toggleX, toggleY, toggleSize, toggleSize);

    // Background
    g.setColour(button.getToggleState() ? juce::Colour(kAccentCyan).withAlpha(0.2f)
                                        : juce::Colour(kSurface));
    g.fillRoundedRectangle(toggleBounds, 3.0f);

    // Border
    g.setColour(button.getToggleState() ? juce::Colour(kAccentCyan)
                                        : juce::Colour(kPanelBorder));
    g.drawRoundedRectangle(toggleBounds, 3.0f, 1.5f);

    // Check mark
    if (button.getToggleState())
    {
        auto checkBounds = toggleBounds.reduced(3.5f);
        juce::Path check;
        check.startNewSubPath(checkBounds.getX(), checkBounds.getCentreY());
        check.lineTo(checkBounds.getX() + checkBounds.getWidth() * 0.35f,
                     checkBounds.getBottom());
        check.lineTo(checkBounds.getRight(), checkBounds.getY());
        g.setColour(juce::Colour(kAccentCyan));
        g.strokePath(check, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // Highlight on hover
    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(juce::Colour(kAccentCyan).withAlpha(0.08f));
        g.fillRoundedRectangle(bounds, 4.0f);
    }

    // Text
    auto textBounds = bounds.withLeft(toggleX + toggleSize + 6.0f);
    g.setColour(button.findColour(juce::ToggleButton::textColourId, true)
                    .isTransparent() ? juce::Colour(kTextPrimary)
                                     : button.findColour(juce::ToggleButton::textColourId, true));
    g.setFont(juce::Font(juce::FontOptions(13.0f)));
    g.drawText(button.getButtonText(), textBounds.toNearestInt(),
               juce::Justification::centredLeft, true);
}

//==============================================================================
// ComboBox
//==============================================================================

void AudioDNALookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                        bool isButtonDown,
                                        int buttonX, int buttonY,
                                        int buttonW, int buttonH,
                                        juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                          static_cast<float>(height));

    // Background
    g.setColour(juce::Colour(kSurface));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border
    g.setColour(isButtonDown ? juce::Colour(kAccentCyan)
                             : juce::Colour(kPanelBorder));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    // Arrow
    auto arrowZone = juce::Rectangle<float>(static_cast<float>(width) - 20.0f, 0.0f,
                                             16.0f, static_cast<float>(height));
    juce::Path arrow;
    auto arrowCentre = arrowZone.getCentre();
    arrow.addTriangle(arrowCentre.x - 4.0f, arrowCentre.y - 2.0f,
                      arrowCentre.x + 4.0f, arrowCentre.y - 2.0f,
                      arrowCentre.x, arrowCentre.y + 3.0f);
    g.setColour(juce::Colour(kAccentCyan));
    g.fillPath(arrow);
}

juce::Font AudioDNALookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font(juce::FontOptions(13.0f));
}

//==============================================================================
// PopupMenu
//==============================================================================

void AudioDNALookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.setColour(juce::Colour(kSurface));
    g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width),
                           static_cast<float>(height), 4.0f);

    g.setColour(juce::Colour(kPanelBorder));
    g.drawRoundedRectangle(0.5f, 0.5f, static_cast<float>(width) - 1.0f,
                           static_cast<float>(height) - 1.0f, 4.0f, 1.0f);
}

void AudioDNALookAndFeel::drawPopupMenuItem(juce::Graphics& g,
                                             const juce::Rectangle<int>& area,
                                             bool isSeparator, bool isActive,
                                             bool isHighlighted, bool isTicked,
                                             bool hasSubMenu,
                                             const juce::String& text,
                                             const juce::String& shortcutKeyText,
                                             const juce::Drawable* icon,
                                             const juce::Colour* textColour)
{
    if (isSeparator)
    {
        auto sepArea = area.reduced(6, 0);
        g.setColour(juce::Colour(kPanelBorder));
        g.fillRect(sepArea.getX(), sepArea.getCentreY(), sepArea.getWidth(), 1);
        return;
    }

    auto areaF = area.toFloat();

    if (isHighlighted && isActive)
    {
        g.setColour(juce::Colour(kAccentCyan).withAlpha(0.15f));
        g.fillRect(areaF);
    }

    auto colour = isHighlighted && isActive ? juce::Colour(kAccentCyan)
                                             : juce::Colour(kTextPrimary);
    if (!isActive)
        colour = juce::Colour(kTextSecondary);

    g.setColour(colour);
    g.setFont(juce::Font(juce::FontOptions(13.0f)));

    auto textArea = area.reduced(8, 0);

    if (isTicked)
    {
        auto tickArea = textArea.removeFromLeft(18);
        juce::Path tick;
        auto tb = tickArea.toFloat().reduced(4.0f);
        tick.startNewSubPath(tb.getX(), tb.getCentreY());
        tick.lineTo(tb.getX() + tb.getWidth() * 0.35f, tb.getBottom());
        tick.lineTo(tb.getRight(), tb.getY());
        g.strokePath(tick, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    g.drawFittedText(text, textArea, juce::Justification::centredLeft, 1);

    if (shortcutKeyText.isNotEmpty())
    {
        g.setColour(juce::Colour(kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawText(shortcutKeyText, textArea, juce::Justification::centredRight, false);
    }

    if (hasSubMenu)
    {
        auto arrowX = static_cast<float>(area.getRight()) - 12.0f;
        auto arrowY = static_cast<float>(area.getCentreY());
        juce::Path arrow;
        arrow.addTriangle(arrowX - 3.0f, arrowY - 4.0f,
                          arrowX - 3.0f, arrowY + 4.0f,
                          arrowX + 3.0f, arrowY);
        g.setColour(juce::Colour(kTextSecondary));
        g.fillPath(arrow);
    }
}

//==============================================================================
// Label
//==============================================================================

void AudioDNALookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    auto bounds = label.getLocalBounds().toFloat();

    auto bgColour = label.findColour(juce::Label::backgroundColourId);
    if (!bgColour.isTransparent())
    {
        g.setColour(bgColour);
        g.fillRoundedRectangle(bounds, 2.0f);
    }

    if (!label.isBeingEdited())
    {
        auto textColour = label.findColour(juce::Label::textColourId);
        g.setColour(textColour.isTransparent() ? juce::Colour(kTextPrimary) : textColour);
        g.setFont(label.getFont());
        g.drawFittedText(label.getText(), label.getBorderSize().subtractedFrom(label.getLocalBounds()),
                         label.getJustificationType(),
                         juce::jmax(1, static_cast<int>(bounds.getHeight() / label.getFont().getHeight())),
                         label.getMinimumHorizontalScale());
    }
}

//==============================================================================
// ScrollBar
//==============================================================================

void AudioDNALookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar,
                                         int x, int y, int width, int height,
                                         bool isScrollbarVertical,
                                         int thumbStartPosition, int thumbSize,
                                         bool isMouseOver, bool isMouseDown)
{
    // Track
    g.setColour(juce::Colour(kBackground));
    g.fillRect(x, y, width, height);

    // Thumb
    auto thumbColour = juce::Colour(kSurfaceLight);
    if (isMouseDown)
        thumbColour = juce::Colour(kAccentCyan).withAlpha(0.5f);
    else if (isMouseOver)
        thumbColour = juce::Colour(kAccentCyan).withAlpha(0.3f);

    g.setColour(thumbColour);

    if (isScrollbarVertical)
    {
        auto thumbW = static_cast<float>(width) - 4.0f;
        g.fillRoundedRectangle(static_cast<float>(x) + 2.0f,
                               static_cast<float>(thumbStartPosition),
                               thumbW,
                               static_cast<float>(thumbSize),
                               thumbW * 0.5f);
    }
    else
    {
        auto thumbH = static_cast<float>(height) - 4.0f;
        g.fillRoundedRectangle(static_cast<float>(thumbStartPosition),
                               static_cast<float>(y) + 2.0f,
                               static_cast<float>(thumbSize),
                               thumbH,
                               thumbH * 0.5f);
    }
}
