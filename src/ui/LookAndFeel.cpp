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

    // AlertWindow (dialogs)
    setColour(juce::AlertWindow::backgroundColourId, juce::Colour(kBackground));
    setColour(juce::AlertWindow::textColourId, juce::Colour(kTextPrimary));
    setColour(juce::AlertWindow::outlineColourId, juce::Colour(kPanelBorder));

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
// App-wide default
//==============================================================================

void AudioDNALookAndFeel::installAsDefault()
{
    juce::LookAndFeel::setDefaultLookAndFeel(this);
}

void AudioDNALookAndFeel::uninstallAsDefault()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

//==============================================================================
// Buttons
//==============================================================================

void AudioDNALookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                 const juce::Colour&,
                                                 bool isMouseOver, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat();

    // Use the button's own buttonColourId (respects per-button setColour calls)
    auto baseColour = button.findColour(juce::TextButton::buttonColourId);

    if (isButtonDown)
        baseColour = baseColour.brighter(0.3f).withAlpha(1.0f);
    else if (isMouseOver)
        baseColour = baseColour.brighter(0.15f);

    g.setColour(baseColour);
    g.fillRect(bounds);

    g.setColour(juce::Colour(kPanelBorder));
    g.drawRect(bounds, 1.0f);
}

void AudioDNALookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                          bool, bool)
{
    // Use the button's own textColourOffId (respects per-button setColour calls)
    g.setColour(button.findColour(juce::TextButton::textColourOffId));
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
                                            float sliderPos, float /*minSliderPos*/,
                                            float /*maxSliderPos*/,
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
                                            bool /*shouldDrawButtonAsDown*/)
{
    // Disabled toggles previously rendered pixel-identical to enabled ones --
    // this scales every colour's alpha the same way the owners' own
    // Component::setAlpha(kDisabledAlpha) workaround did, so the toggle now
    // reads as disabled everywhere without each owner dimming it itself.
    const float alphaMul = button.isEnabled() ? 1.0f : kDisabledAlpha;

    auto bounds = button.getLocalBounds().toFloat();
    auto toggleSize = 16.0f;
    auto toggleX = bounds.getX() + 4.0f;
    auto toggleY = bounds.getCentreY() - toggleSize * 0.5f;
    auto toggleBounds = juce::Rectangle<float>(toggleX, toggleY, toggleSize, toggleSize);

    // Background
    g.setColour((button.getToggleState() ? juce::Colour(kAccentCyan).withAlpha(0.2f)
                                          : juce::Colour(kSurface)).withMultipliedAlpha(alphaMul));
    g.fillRoundedRectangle(toggleBounds, 3.0f);

    // Border
    g.setColour((button.getToggleState() ? juce::Colour(kAccentCyan)
                                          : juce::Colour(kPanelBorder)).withMultipliedAlpha(alphaMul));
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
        g.setColour(juce::Colour(kAccentCyan).withMultipliedAlpha(alphaMul));
        g.strokePath(check, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // Highlight on hover
    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(juce::Colour(kAccentCyan).withAlpha(0.08f).withMultipliedAlpha(alphaMul));
        g.fillRoundedRectangle(bounds, 4.0f);
    }

    // Text
    auto textBounds = bounds.withLeft(toggleX + toggleSize + 6.0f);
    auto textColour = button.findColour(juce::ToggleButton::textColourId, true)
                    .isTransparent() ? juce::Colour(kTextPrimary)
                                     : button.findColour(juce::ToggleButton::textColourId, true);
    g.setColour(textColour.withMultipliedAlpha(alphaMul));
    g.setFont(juce::Font(juce::FontOptions(13.0f)));
    g.drawText(button.getButtonText(), textBounds.toNearestInt(),
               juce::Justification::centredLeft, true);
}

//==============================================================================
// ComboBox
//==============================================================================

void AudioDNALookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                        bool isButtonDown,
                                        int /*buttonX*/, int /*buttonY*/,
                                        int /*buttonW*/, int /*buttonH*/,
                                        juce::ComboBox& box)
{
    // s-rta-0926b plan-fitmode: a disabled combo is drawn disabled app-wide (the drawToggleButton
    // treatment); its text is the combo's child Label, dimmed by drawLabel below.
    const float alphaMul = box.isEnabled() ? 1.0f : kDisabledAlpha;

    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                          static_cast<float>(height));

    // Background
    g.setColour(juce::Colour(kSurface).withMultipliedAlpha(alphaMul));
    g.fillRect(bounds);

    // Border
    g.setColour((isButtonDown ? juce::Colour(kAccentCyan)
                              : juce::Colour(kPanelBorder)).withMultipliedAlpha(alphaMul));
    g.drawRect(bounds, 1.0f);

    // Arrow
    auto arrowZone = juce::Rectangle<float>(static_cast<float>(width) - 20.0f, 0.0f,
                                             16.0f, static_cast<float>(height));
    juce::Path arrow;
    auto arrowCentre = arrowZone.getCentre();
    arrow.addTriangle(arrowCentre.x - 4.0f, arrowCentre.y - 2.0f,
                      arrowCentre.x + 4.0f, arrowCentre.y - 2.0f,
                      arrowCentre.x, arrowCentre.y + 3.0f);
    g.setColour(juce::Colour(kAccentCyan).withMultipliedAlpha(alphaMul));
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
    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                          static_cast<float>(height));

    g.setColour(juce::Colour(kSurface));
    g.fillRect(bounds);

    g.setColour(juce::Colour(kPanelBorder));
    g.drawRect(bounds, 1.0f);
}

void AudioDNALookAndFeel::drawPopupMenuItem(juce::Graphics& g,
                                             const juce::Rectangle<int>& area,
                                             bool isSeparator, bool isActive,
                                             bool isHighlighted, bool isTicked,
                                             bool hasSubMenu,
                                             const juce::String& text,
                                             const juce::String& shortcutKeyText,
                                             const juce::Drawable* /*icon*/,
                                             const juce::Colour* /*textColour*/)
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
// AlertWindow (dialogs)
//==============================================================================

juce::AlertWindow* AudioDNALookAndFeel::createAlertWindow(const juce::String& title,
                                                          const juce::String& message,
                                                          const juce::String& button1,
                                                          const juce::String& button2,
                                                          const juce::String& button3,
                                                          juce::MessageBoxIconType iconType,
                                                          int numButtons,
                                                          juce::Component* associatedComponent)
{
    // The plain window + buttons (LookAndFeel_V2), then this LookAndFeel -- which re-lays it out --
    // then LookAndFeel_V4's 50-px margin on top (the same three lines as
    // LookAndFeel_V4::createAlertWindow, applied after the re-layout so they are not undone by it).
    auto* aw = juce::LookAndFeel_V2::createAlertWindow(title, message, button1, button2, button3,
                                                       iconType, numButtons, associatedComponent);
    aw->setLookAndFeel(this);

    aw->setBounds(aw->getBounds().withSizeKeepingCentre(aw->getWidth() + 50, aw->getHeight() + 50));
    for (auto* child : aw->getChildren())
        if (auto* button = dynamic_cast<juce::TextButton*>(child))
            button->setBounds(button->getBounds() + juce::Point<int>(25, 40));

    return aw;
}

void AudioDNALookAndFeel::drawAlertBox(juce::Graphics& g, juce::AlertWindow& alert,
                                        const juce::Rectangle<int>& /*textArea*/,
                                        juce::TextLayout& textLayout)
{
    auto bounds = alert.getLocalBounds().toFloat();

    // Square panel + 1-px border, like every other panel. No icon: the app's dialogs are text only.
    g.setColour(alert.findColour(juce::AlertWindow::backgroundColourId));
    g.fillRect(bounds);

    g.setColour(alert.findColour(juce::AlertWindow::outlineColourId));
    g.drawRect(bounds, 1.0f);

    // The text where LookAndFeel_V4 puts it (30 px down, above the buttons).
    textLayout.draw(g, juce::Rectangle<float>(0.0f, 30.0f, bounds.getWidth(),
                                              bounds.getHeight()
                                                  - static_cast<float>(getAlertWindowButtonHeight()) - 20.0f));
}

//==============================================================================
// Fonts
//==============================================================================

juce::Typeface::Ptr AudioDNALookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    if (font.isBold() || font.isItalic())
        return juce::Font::getDefaultTypefaceForFont(font);

    return juce::LookAndFeel_V4::getTypefaceForFont(font);
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
        g.fillRect(bounds);
    }

    // A transparent text colour means "draw no text" -- e.g. AlertWindow's hidden accessibility label,
    // which holds a second copy of the dialog's message for screen readers.
    auto textColour = label.findColour(juce::Label::textColourId);
    if (!label.isBeingEdited() && !textColour.isTransparent())
    {
        // s-rta-0926b plan-fitmode: a disabled label (or one inside a disabled combo) is drawn disabled,
        // as JUCE's own LookAndFeel_V2::drawLabel does -- this override used to drop that.
        const float alphaMul = label.isEnabled() ? 1.0f : kDisabledAlpha;
        g.setColour(textColour.withMultipliedAlpha(alphaMul));
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

void AudioDNALookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar& /*scrollbar*/,
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
