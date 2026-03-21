#include "ui/UniversalParamControl.h"

UniversalParamControl::UniversalParamControl()
{
    // Value slider — horizontal, thin
    valueSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    valueSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    valueSlider_.setRange(0.0, 1.0, 0.001);
    valueSlider_.setValue(0.5, juce::dontSendNotification);
    valueSlider_.setScrollWheelEnabled(false); // Scroll should scroll the inspector, not change value
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
        s.setScrollWheelEnabled(false);
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

void UniversalParamControl::drawSignalTriangle(juce::Graphics& g,
                                                juce::Rectangle<float> area,
                                                bool connected)
{
    // Draw a small right-pointing triangle (play button shape)
    // Grey when unconnected, cyan when a signal is routed
    juce::Path triangle;
    float cx = area.getCentreX();
    float cy = area.getCentreY();
    float halfSize = 4.0f;

    // Right-pointing triangle
    triangle.addTriangle(cx - halfSize, cy - halfSize,
                         cx - halfSize, cy + halfSize,
                         cx + halfSize, cy);

    if (connected)
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    else
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.5f));

    g.fillPath(triangle);
}

void UniversalParamControl::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Collapsed row: triangle + name + value text + source indicator
    auto row = bounds.removeFromTop(static_cast<float>(kCollapsedHeight));

    // Signal connect triangle (left side, always visible)
    auto triangleArea = row.removeFromLeft(static_cast<float>(kTriangleSize));
    drawSignalTriangle(g, triangleArea, isConnected());

    // Parameter name
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText(paramName_, row.removeFromLeft(72.0f).toNearestInt(),
               juce::Justification::centredLeft, true);

    // Value text
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText(juce::String(currentValue_, 2),
               juce::Rectangle<int>(kTriangleSize + 72, 0, 36, kCollapsedHeight),
               juce::Justification::centredRight, false);

    // Source-driven mini meter (below collapsed row, if source active and expanded)
    if (sourceMode_ != SourceMode::Manual && expanded_)
    {
        auto meterRow = bounds.removeFromTop(14.0f);
        meterRow = meterRow.withTrimmedLeft(static_cast<float>(kTriangleSize) + 4.0f).withTrimmedRight(4.0f);

        // Source name with arrow
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

    // When connected but collapsed, show a subtle source name hint
    if (isConnected() && !expanded_)
    {
        // Draw tiny source label above the slider area
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.5f));
        g.setFont(juce::Font(juce::FontOptions(8.0f)));
        auto hintBounds = getLocalBounds().toFloat();
        hintBounds = hintBounds.removeFromTop(10.0f);
        hintBounds.removeFromLeft(static_cast<float>(kTriangleSize) + 72.0f + 36.0f + 46.0f);
        g.drawText(sourceName_, hintBounds.toNearestInt(),
                   juce::Justification::centredLeft, true);
    }
}

void UniversalParamControl::resized()
{
    auto area = getLocalBounds();

    // Collapsed row
    auto row = area.removeFromTop(kCollapsedHeight);

    // Triangle area (painted in paint(), not a component)
    row.removeFromLeft(kTriangleSize);

    // Name (72px) + value text (36px) painted in paint()
    row.removeFromLeft(72 + 36);

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
        sourceRow.removeFromLeft(kTriangleSize); // align with content
        sourceBtn_.setBounds(sourceRow.removeFromLeft(120));
        sourceRow.removeFromLeft(8);
        invertToggle_.setBounds(sourceRow.removeFromLeft(70));

        sourceBtn_.setVisible(true);
        invertToggle_.setVisible(true);

        area.removeFromTop(2);

        // Range row
        auto rangeRow = area.removeFromTop(20);
        rangeRow.removeFromLeft(kTriangleSize); // align with content
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
    // Right-click anywhere → reset to default value
    if (event.mods.isRightButtonDown())
    {
        setParamValue(defaultValue_);
        valueSlider_.setValue(static_cast<double>(defaultValue_), juce::sendNotificationSync);
        if (onValueChanged) onValueChanged(defaultValue_);
        return;
    }

    // Click on the triangle area → show source picker popup
    if (event.position.x < static_cast<float>(kTriangleSize) &&
        event.position.y < static_cast<float>(kCollapsedHeight))
    {
        showSourcePickerAtTriangle();
        return;
    }

    // Click on the name/value area toggles expanded
    if (event.position.x < static_cast<float>(kTriangleSize + 72 + 36) &&
        event.position.y < static_cast<float>(kCollapsedHeight))
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

void UniversalParamControl::showSourcePickerAtTriangle()
{
    // Show the popup anchored to the triangle area
    juce::PopupMenu menu;
    buildSourcePickerMenu(menu);

    auto triangleBounds = getLocalBounds().removeFromLeft(kTriangleSize)
                                          .removeFromTop(kCollapsedHeight);
    auto screenPos = localAreaToGlobal(triangleBounds);

    auto options = juce::PopupMenu::Options()
                       .withTargetScreenArea(screenPos);
    if (auto* topLevel = getTopLevelComponent())
        options = options.withParentComponent(topLevel);
    menu.showMenuAsync(options,
        [this](int result) { handleSourcePickerResult(result); });
}

void UniversalParamControl::showSourcePicker()
{
    juce::PopupMenu menu;
    buildSourcePickerMenu(menu);

    auto options = juce::PopupMenu::Options().withTargetComponent(&sourceBtn_);
    if (auto* topLevel = getTopLevelComponent())
        options = options.withParentComponent(topLevel);
    menu.showMenuAsync(options,
        [this](int result) { handleSourcePickerResult(result); });
}

void UniversalParamControl::buildSourcePickerMenu(juce::PopupMenu& menu)
{
    // === Manual (Basic) ===
    menu.addItem(1, "Manual", true, sourceMode_ == SourceMode::Manual);

    menu.addSeparator();

    // === Audio Signals ===
    if (signalRegistry_)
    {
        juce::PopupMenu audioMenu;
        int itemId = 100;
        for (int i = 0; i < signalRegistry_->getNumSignals(); ++i)
        {
            auto* sig = signalRegistry_->getSignalAt(i);
            if (sig && sig->getType() == Signal::Type::Audio)
            {
                audioMenu.addItem(itemId + i,
                    juce::String(sig->getName()),
                    true,
                    sourceMode_ == SourceMode::Signal &&
                    sourceName_ == juce::String(sig->getName()));
            }
        }
        if (audioMenu.getNumItems() > 0)
            menu.addSubMenu("Audio", audioMenu);
    }

    // === BPM Sync (per-parameter beat-synced oscillation) ===
    {
        juce::PopupMenu bpmMenu;

        // Waveform shapes × beat divisions
        static const char* const shapes[] = { "Sine", "Saw", "Triangle", "Square" };
        static const char* const divisions[] = {
            "1/4 Beat", "1/2 Beat", "1 Beat", "2 Beats", "4 Beats", "8 Beats"
        };

        int bpmId = 300;
        for (int s = 0; s < 4; ++s)
        {
            juce::PopupMenu shapeMenu;
            for (int d = 0; d < 6; ++d)
            {
                juce::String itemName = juce::String(divisions[d]);
                juce::String fullName = juce::String(shapes[s]) + " " + itemName;
                shapeMenu.addItem(bpmId + s * 6 + d, itemName, true,
                    sourceMode_ == SourceMode::BPMSync && sourceName_ == fullName);
            }
            bpmMenu.addSubMenu(shapes[s], shapeMenu);
        }
        menu.addSubMenu("BPM Sync", bpmMenu);
    }

    // === Oscillators (from SignalRegistry) ===
    if (signalRegistry_)
    {
        juce::PopupMenu oscMenu;
        int oscId = 400;
        for (int i = 0; i < signalRegistry_->getNumSignals(); ++i)
        {
            auto* sig = signalRegistry_->getSignalAt(i);
            if (sig && sig->getType() == Signal::Type::Oscillator)
            {
                oscMenu.addItem(oscId + i,
                    juce::String(sig->getName()),
                    true,
                    sourceMode_ == SourceMode::Oscillator &&
                    sourceName_ == juce::String(sig->getName()));
            }
        }
        if (oscMenu.getNumItems() > 0)
            menu.addSubMenu("Oscillator", oscMenu);
    }

    // === Envelopes (from SignalRegistry) ===
    if (signalRegistry_)
    {
        juce::PopupMenu envMenu;
        int envId = 500;
        for (int i = 0; i < signalRegistry_->getNumSignals(); ++i)
        {
            auto* sig = signalRegistry_->getSignalAt(i);
            if (sig && sig->getType() == Signal::Type::Envelope)
            {
                envMenu.addItem(envId + i,
                    juce::String(sig->getName()),
                    true,
                    sourceMode_ == SourceMode::Envelope &&
                    sourceName_ == juce::String(sig->getName()));
            }
        }
        if (envMenu.getNumItems() > 0)
            menu.addSubMenu("Envelope", envMenu);
    }

    // === Clip Position ===
    menu.addItem(2, "Clip Position", true, sourceMode_ == SourceMode::ClipPosition);

    // === Timeline (per-parameter keyframes — placeholder for future) ===
    menu.addItem(3, "Timeline", true, sourceMode_ == SourceMode::Timeline);

    menu.addSeparator();

    // === Macros ===
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
        menu.addSubMenu("Macro", macroMenu);
    }
}

void UniversalParamControl::handleSourcePickerResult(int result)
{
    if (result == 0) return; // dismissed

    if (result == 1)
    {
        sourceMode_ = SourceMode::Manual;
        sourceName_ = {};
        sourceBtn_.setButtonText("Manual");
    }
    else if (result == 2)
    {
        sourceMode_ = SourceMode::ClipPosition;
        sourceName_ = "Clip Position";
        sourceBtn_.setButtonText("Clip Position");
    }
    else if (result == 3)
    {
        sourceMode_ = SourceMode::Timeline;
        sourceName_ = "Timeline";
        sourceBtn_.setButtonText("Timeline");
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
    else if (result >= 300 && result < 324)
    {
        // BPM Sync: shapes[0-3] × divisions[0-5]
        int bpmIdx = result - 300;
        int shapeIdx = bpmIdx / 6;
        int divIdx = bpmIdx % 6;

        static const char* const shapes[] = { "Sine", "Saw", "Triangle", "Square" };
        static const char* const divisions[] = {
            "1/4 Beat", "1/2 Beat", "1 Beat", "2 Beats", "4 Beats", "8 Beats"
        };

        sourceMode_ = SourceMode::BPMSync;
        sourceName_ = juce::String(shapes[shapeIdx]) + " " + juce::String(divisions[divIdx]);
        sourceBtn_.setButtonText(sourceName_);
    }
    else if (result >= 400 && result < 500)
    {
        int sigIdx = result - 400;
        if (signalRegistry_)
        {
            if (auto* sig = signalRegistry_->getSignalAt(sigIdx))
            {
                sourceMode_ = SourceMode::Oscillator;
                sourceName_ = juce::String(sig->getName());
                sourceBtn_.setButtonText(sourceName_);
            }
        }
    }
    else if (result >= 500 && result < 600)
    {
        int sigIdx = result - 500;
        if (signalRegistry_)
        {
            if (auto* sig = signalRegistry_->getSignalAt(sigIdx))
            {
                sourceMode_ = SourceMode::Envelope;
                sourceName_ = juce::String(sig->getName());
                sourceBtn_.setButtonText(sourceName_);
            }
        }
    }

    if (onSourceChanged) onSourceChanged(sourceMode_, sourceName_);
    resized();
    repaint();
}

void UniversalParamControl::updateValueDisplay()
{
    repaint();
}
