#include "ui/UniversalParamControl.h"

UniversalParamControl::UniversalParamControl()
{
    // Value slider — horizontal, thin
    valueSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    valueSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    valueSlider_.setRange(0.0, 1.0, 0.001);
    valueSlider_.setValue(0.5, juce::dontSendNotification);
    valueSlider_.setColour(juce::Slider::thumbColourId,
                           juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    valueSlider_.setColour(juce::Slider::trackColourId,
                           juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
    valueSlider_.onValueChange = [this] {
        currentValue_ = static_cast<float>(valueSlider_.getValue());
        updateValueDisplay();
        if (onValueChanged) onValueChanged(currentValue_);
    };
    addAndMakeVisible(valueSlider_);

    // Decrement / increment buttons
    decrementBtn_.setColour(juce::TextButton::buttonColourId,
                            juce::Colour(AudioDNALookAndFeel::kSurface));
    decrementBtn_.setColour(juce::TextButton::textColourOffId,
                            juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    decrementBtn_.onClick = [this] {
        float newVal = juce::jlimit(0.0f, 1.0f, currentValue_ - 0.01f);
        setParamValue(newVal);
        if (onValueChanged) onValueChanged(currentValue_);
    };
    addAndMakeVisible(decrementBtn_);

    incrementBtn_.setColour(juce::TextButton::buttonColourId,
                            juce::Colour(AudioDNALookAndFeel::kSurface));
    incrementBtn_.setColour(juce::TextButton::textColourOffId,
                            juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    incrementBtn_.onClick = [this] {
        float newVal = juce::jlimit(0.0f, 1.0f, currentValue_ + 0.01f);
        setParamValue(newVal);
        if (onValueChanged) onValueChanged(currentValue_);
    };
    addAndMakeVisible(incrementBtn_);

    // Source button (expanded only)
    sourceBtn_.setColour(juce::TextButton::buttonColourId,
                         juce::Colour(AudioDNALookAndFeel::kSurface));
    sourceBtn_.setColour(juce::TextButton::textColourOffId,
                         juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    sourceBtn_.onClick = [this] { showSourcePicker(); };
    addChildComponent(sourceBtn_);

    // Invert toggle (expanded only)
    invertToggle_.setColour(juce::ToggleButton::textColourId,
                            juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    invertToggle_.onStateChange = [this] {
        inverted_ = invertToggle_.getToggleState();
        if (onInvertChanged) onInvertChanged(inverted_);
    };
    addChildComponent(invertToggle_);

    // Range sliders (expanded only)
    auto setupRangeSlider = [](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 35, 18);
        s.setRange(0.0, 1.0, 0.01);
        s.setColour(juce::Slider::thumbColourId,
                    juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        s.setColour(juce::Slider::trackColourId,
                    juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
    };
    setupRangeSlider(rangeMinSlider_);
    setupRangeSlider(rangeMaxSlider_);
    rangeMinSlider_.setValue(0.0, juce::dontSendNotification);
    rangeMaxSlider_.setValue(1.0, juce::dontSendNotification);

    rangeMinSlider_.onValueChange = [this] {
        outputMin_ = static_cast<float>(rangeMinSlider_.getValue());
        if (onRangeChanged) onRangeChanged(outputMin_, outputMax_);
    };
    rangeMaxSlider_.onValueChange = [this] {
        outputMax_ = static_cast<float>(rangeMaxSlider_.getValue());
        if (onRangeChanged) onRangeChanged(outputMin_, outputMax_);
    };
    addChildComponent(rangeMinSlider_);
    addChildComponent(rangeMaxSlider_);

    rangeLabel_.setText("Range", juce::dontSendNotification);
    rangeLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    rangeLabel_.setColour(juce::Label::textColourId,
                          juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addChildComponent(rangeLabel_);
}

void UniversalParamControl::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Collapsed row: name + value text + source indicator
    auto row = bounds.removeFromTop(static_cast<float>(kCollapsedHeight));

    // Parameter name
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText(paramName_, row.removeFromLeft(80.0f).toNearestInt(),
               juce::Justification::centredLeft, true);

    // Value text
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText(juce::String(currentValue_, 2),
               juce::Rectangle<int>(80, 0, 36, kCollapsedHeight),
               juce::Justification::centredRight, false);

    // Source-driven mini meter (below collapsed row, if source active)
    if (sourceMode_ != SourceMode::Manual && expanded_)
    {
        auto meterRow = bounds.removeFromTop(14.0f);
        meterRow = meterRow.withTrimmedLeft(4.0f).withTrimmedRight(4.0f);

        // Source name
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(juce::String(juce::CharPointer_UTF8("\xe2\x86\x90 ")) + sourceName_,
                   meterRow.removeFromLeft(80.0f).toNearestInt(),
                   juce::Justification::centredLeft, true);

        // Mini meter bar
        auto meterBounds = meterRow.reduced(2.0f, 2.0f);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
        g.fillRect(meterBounds);

        float fillWidth = meterBounds.getWidth() * sourceValue_;
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.7f));
        g.fillRect(meterBounds.withWidth(fillWidth));
    }
}

void UniversalParamControl::resized()
{
    auto area = getLocalBounds();

    // Collapsed row
    auto row = area.removeFromTop(kCollapsedHeight);

    // Name (80px) + value text (36px) painted in paint()
    row.removeFromLeft(80 + 36);

    // -/+ buttons
    row.removeFromLeft(2);
    decrementBtn_.setBounds(row.removeFromLeft(20));
    row.removeFromLeft(1);
    incrementBtn_.setBounds(row.removeFromLeft(20));
    row.removeFromLeft(4);

    // Slider takes the rest
    valueSlider_.setBounds(row);

    // Expanded controls
    if (expanded_)
    {
        area.removeFromTop(2);

        // Source picker row
        if (sourceMode_ != SourceMode::Manual)
            area.removeFromTop(14); // space for source meter (painted)

        auto sourceRow = area.removeFromTop(22);
        sourceBtn_.setBounds(sourceRow.removeFromLeft(120));
        sourceRow.removeFromLeft(8);
        invertToggle_.setBounds(sourceRow.removeFromLeft(70));

        sourceBtn_.setVisible(true);
        invertToggle_.setVisible(true);

        area.removeFromTop(2);

        // Range row
        auto rangeRow = area.removeFromTop(20);
        rangeLabel_.setBounds(rangeRow.removeFromLeft(40));
        rangeLabel_.setVisible(true);

        int halfWidth = rangeRow.getWidth() / 2;
        rangeMinSlider_.setBounds(rangeRow.removeFromLeft(halfWidth));
        rangeMaxSlider_.setBounds(rangeRow);
        rangeMinSlider_.setVisible(true);
        rangeMaxSlider_.setVisible(true);
    }
    else
    {
        sourceBtn_.setVisible(false);
        invertToggle_.setVisible(false);
        rangeMinSlider_.setVisible(false);
        rangeMaxSlider_.setVisible(false);
        rangeLabel_.setVisible(false);
    }
}

void UniversalParamControl::mouseDown(const juce::MouseEvent& event)
{
    // Click on the name/value area toggles expanded
    if (event.position.x < 116.0f && event.position.y < static_cast<float>(kCollapsedHeight))
    {
        setExpanded(!expanded_);
        return;
    }
    Component::mouseDown(event);
}

void UniversalParamControl::setParamName(const juce::String& name)
{
    paramName_ = name;
    repaint();
}

void UniversalParamControl::setParamValue(float value)
{
    currentValue_ = juce::jlimit(0.0f, 1.0f, value);
    valueSlider_.setValue(static_cast<double>(currentValue_), juce::dontSendNotification);
    updateValueDisplay();
}

void UniversalParamControl::setExpanded(bool expanded)
{
    if (expanded_ == expanded) return;
    expanded_ = expanded;
    resized();
    if (onExpandToggled) onExpandToggled();

    if (auto* parent = getParentComponent())
        parent->resized();
}

int UniversalParamControl::getPreferredHeight() const
{
    if (!expanded_) return kCollapsedHeight;

    int h = kCollapsedHeight;
    if (sourceMode_ != SourceMode::Manual)
        h += 14; // source meter
    h += 2 + 22; // source picker row
    h += 2 + 20; // range row
    return h;
}

void UniversalParamControl::showSourcePicker()
{
    juce::PopupMenu menu;

    // Manual option
    menu.addItem(1, "Manual", true, sourceMode_ == SourceMode::Manual);

    // Signals submenu
    if (signalRegistry_)
    {
        juce::PopupMenu signalMenu;
        int itemId = 100;
        for (int i = 0; i < signalRegistry_->getNumSignals(); ++i)
        {
            auto* sig = signalRegistry_->getSignalAt(i);
            if (sig)
            {
                signalMenu.addItem(itemId + i,
                    juce::String(sig->getName()),
                    true,
                    sourceMode_ == SourceMode::Signal &&
                    sourceName_ == juce::String(sig->getName()));
            }
        }
        menu.addSubMenu("Signals", signalMenu);
    }

    // Macros submenu (placeholder — actual macro linking comes from Inspector)
    {
        juce::PopupMenu macroMenu;
        for (int i = 0; i < MacroBank::kNumMacros; ++i)
        {
            macroMenu.addItem(200 + i,
                "Macro " + juce::String(i + 1),
                true,
                sourceMode_ == SourceMode::Macro &&
                sourceName_ == "Macro " + juce::String(i + 1));
        }
        menu.addSubMenu("Macros", macroMenu);
    }

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&sourceBtn_),
        [this](int result) {
            if (result == 0) return; // dismissed

            if (result == 1)
            {
                sourceMode_ = SourceMode::Manual;
                sourceName_ = {};
                sourceBtn_.setButtonText("Manual");
            }
            else if (result >= 100 && result < 200)
            {
                int sigIdx = result - 100;
                if (signalRegistry_)
                {
                    if (auto* sig = signalRegistry_->getSignalAt(sigIdx))
                    {
                        sourceMode_ = SourceMode::Signal;
                        sourceName_ = juce::String(sig->getName());
                        sourceBtn_.setButtonText(sourceName_);
                    }
                }
            }
            else if (result >= 200 && result < 206)
            {
                int macroIdx = result - 200;
                sourceMode_ = SourceMode::Macro;
                sourceName_ = "Macro " + juce::String(macroIdx + 1);
                sourceBtn_.setButtonText(sourceName_);
            }

            if (onSourceChanged) onSourceChanged(sourceMode_, sourceName_);
            resized();
            repaint();
        });
}

void UniversalParamControl::updateValueDisplay()
{
    repaint();
}
