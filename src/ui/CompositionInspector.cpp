#include "ui/CompositionInspector.h"

CompositionInspector::CompositionInspector()
{
    addAndMakeVisible(macroPanel_);

    // --- Autopilot direction buttons ---
    auto setupApBtn = [this](juce::TextButton& btn, const juce::String& text) {
        btn.setButtonText(text);
        btn.setColour(juce::TextButton::buttonColourId,
                      juce::Colour(AudioDNALookAndFeel::kSurface));
        btn.setColour(juce::TextButton::textColourOffId,
                      juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        addAndMakeVisible(btn);
    };
    setupApBtn(apRewindBtn_, juce::String(juce::CharPointer_UTF8("\xe2\x97\x80\xe2\x97\x80")));
    setupApBtn(apOffBtn_, "OFF");
    setupApBtn(apForwardBtn_, juce::String(juce::CharPointer_UTF8("\xe2\x96\xb6\xe2\x96\xb6")));
    setupApBtn(apRandomBtn_, juce::String(juce::CharPointer_UTF8("\xf0\x9f\x94\x80")));

    apOffBtn_.setColour(juce::TextButton::buttonColourId,
                        juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f));

    apRewindBtn_.onClick = [this] {
        if (composition_) { composition_->autopilotDirection = Composition::AutopilotDirection::Rewind; syncFromComposition(); }
    };
    apOffBtn_.onClick = [this] {
        if (composition_) { composition_->autopilotDirection = Composition::AutopilotDirection::Off; syncFromComposition(); }
    };
    apForwardBtn_.onClick = [this] {
        if (composition_) { composition_->autopilotDirection = Composition::AutopilotDirection::Forward; syncFromComposition(); }
    };
    apRandomBtn_.onClick = [this] {
        if (composition_) { composition_->autopilotDirection = Composition::AutopilotDirection::Random; syncFromComposition(); }
    };

    // Duration selector
    apDurationSelector_.addItem("Longest Clip", 1);
    apDurationSelector_.addItem("Clip Transport", 2);
    apDurationSelector_.addItem("Custom", 3);
    apDurationSelector_.setSelectedId(1, juce::dontSendNotification);
    apDurationSelector_.onChange = [this] {
        if (!composition_) return;
        composition_->autopilotDurationMode =
            static_cast<Composition::AutopilotDurationMode>(apDurationSelector_.getSelectedId() - 1);
    };
    addAndMakeVisible(apDurationSelector_);

    // Clip Loops
    apClipLoopsSlider_.setSliderStyle(juce::Slider::IncDecButtons);
    apClipLoopsSlider_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 30, 20);
    apClipLoopsSlider_.setRange(1, 99, 1);
    apClipLoopsSlider_.setValue(1, juce::dontSendNotification);
    apClipLoopsSlider_.setScrollWheelEnabled(false);
    apClipLoopsSlider_.setColour(juce::Slider::textBoxTextColourId,
                                  juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    apClipLoopsSlider_.onValueChange = [this] {
        if (composition_)
            composition_->autopilotClipLoops = static_cast<int>(apClipLoopsSlider_.getValue());
    };
    addAndMakeVisible(apClipLoopsSlider_);

    // Loop toggle
    apLoopToggle_.setColour(juce::ToggleButton::textColourId,
                            juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    apLoopToggle_.onStateChange = [this] {
        if (composition_) composition_->autopilotLoop = apLoopToggle_.getToggleState();
    };
    addAndMakeVisible(apLoopToggle_);

    // Master Layer selector
    apMasterLayerSelector_.addItem("Off", 1);
    for (int i = 1; i <= 8; ++i)
        apMasterLayerSelector_.addItem("Layer " + juce::String(i), i + 1);
    apMasterLayerSelector_.setSelectedId(1, juce::dontSendNotification);
    apMasterLayerSelector_.onChange = [this] {
        if (composition_)
            composition_->autopilotMasterLayer = apMasterLayerSelector_.getSelectedId() - 2; // -1 = Off
    };
    addAndMakeVisible(apMasterLayerSelector_);

    // --- Composition Master + Speed (with signal triangles) ---
    masterControl_.setParamName("Master");
    masterControl_.setParamValue(1.0f);
    masterControl_.onValueChanged = [this](float val) {
        if (composition_) composition_->masterOpacity = val;
    };
    masterControl_.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
    addAndMakeVisible(masterControl_);

    speedControl_.setParamName("Speed");
    speedControl_.setParamValue(1.0f);
    speedControl_.onValueChanged = [this](float val) {
        if (composition_) composition_->masterSpeed = val * 4.0f; // [0,1] → [0,4]
    };
    speedControl_.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
    addAndMakeVisible(speedControl_);

    // --- Video Opacity ---
    opacityControl_.setParamName("Opacity");
    opacityControl_.setParamValue(1.0f);
    opacityControl_.onValueChanged = [this](float val) {
        if (composition_) composition_->compOpacity = val;
    };
    opacityControl_.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
    addAndMakeVisible(opacityControl_);

    // --- Transform ---
    auto setupTransformParam = [this](UniversalParamControl& pc, const juce::String& name, float defVal) {
        pc.setParamName(name);
        pc.setParamValue(defVal);
        pc.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
        addAndMakeVisible(pc);
    };
    setupTransformParam(posXControl_, "Position X", 0.5f);
    setupTransformParam(posYControl_, "Position Y", 0.5f);
    setupTransformParam(scaleControl_, "Scale", 0.5f);   // 0.5 = 100% in [0,200%] mapped
    setupTransformParam(rotationControl_, "Rotation", 0.5f);
    setupTransformParam(anchorControl_, "Anchor", 0.5f);

    posXControl_.onValueChanged = [this](float v) { if (composition_) composition_->compPositionX = (v - 0.5f) * 3840.0f; };
    posYControl_.onValueChanged = [this](float v) { if (composition_) composition_->compPositionY = (v - 0.5f) * 2160.0f; };
    scaleControl_.onValueChanged = [this](float v) { if (composition_) composition_->compScale = v * 2.0f; };
    rotationControl_.onValueChanged = [this](float v) { if (composition_) composition_->compRotation = (v - 0.5f) * 720.0f; };
    anchorControl_.onValueChanged = [this](float v) {
        if (composition_) { composition_->compAnchorX = (v - 0.5f) * 3840.0f; composition_->compAnchorY = 0.0f; }
    };

    // --- Global Effects ---
    addAndMakeVisible(effectStackView_);

    // --- Output Settings ---
    resolutionSelector_.addItem("1920x1080", 1);
    resolutionSelector_.addItem("1280x720", 2);
    resolutionSelector_.addItem("2560x1440", 3);
    resolutionSelector_.addItem("3840x2160", 4);
    resolutionSelector_.setSelectedId(1, juce::dontSendNotification);
    resolutionSelector_.onChange = [this] {
        if (!composition_) return;
        switch (resolutionSelector_.getSelectedId())
        {
            case 1: composition_->outputWidth = 1920; composition_->outputHeight = 1080; break;
            case 2: composition_->outputWidth = 1280; composition_->outputHeight = 720; break;
            case 3: composition_->outputWidth = 2560; composition_->outputHeight = 1440; break;
            case 4: composition_->outputWidth = 3840; composition_->outputHeight = 2160; break;
        }
    };
    addAndMakeVisible(resolutionSelector_);
}

void CompositionInspector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    if (!composition_) return;

    // Name bar
    auto nameBar = getLocalBounds().removeFromTop(kNameBarHeight);
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(nameBar);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(12.0f)).boldened());
    juce::String nameStr = juce::String(composition_->name) + " ("
        + juce::String(composition_->outputWidth) + " x "
        + juce::String(composition_->outputHeight) + ")";
    g.drawText(nameStr, nameBar.withTrimmedLeft(4).withTrimmedRight(40),
               juce::Justification::centredLeft, true);

    // Section headers at calculated positions
    int y = kNameBarHeight + MacroPanel::kPreferredHeight + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Autopilot");
    y += kSectionHeaderHeight + kRowHeight * 4 + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Composition");
    y += kSectionHeaderHeight + masterControl_.getPreferredHeight() + speedControl_.getPreferredHeight() + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Video");
    y += kSectionHeaderHeight + opacityControl_.getPreferredHeight() + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Transform", true);
    int transformH = posXControl_.getPreferredHeight() + posYControl_.getPreferredHeight()
                   + scaleControl_.getPreferredHeight() + rotationControl_.getPreferredHeight()
                   + anchorControl_.getPreferredHeight();
    y += kSectionHeaderHeight + transformH + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Global Effects");
    y += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Output Settings");
}

void CompositionInspector::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    int y = 0;

    // Name bar
    y += kNameBarHeight;

    // Dashboard
    macroPanel_.setBounds(area.getX(), y, area.getWidth(), MacroPanel::kPreferredHeight);
    y += MacroPanel::kPreferredHeight + kSectionGap;

    // Autopilot
    y += kSectionHeaderHeight;

    // Direction row: label + 4 buttons
    {
        auto dirRow = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        auto labelArea = dirRow.removeFromLeft(80);
        // "Direction" painted in paint() — but we don't want to paint inside resized()
        // so we just layout the 4 buttons
        int btnW = (dirRow.getWidth()) / 4;
        apRewindBtn_.setBounds(dirRow.removeFromLeft(btnW));
        apOffBtn_.setBounds(dirRow.removeFromLeft(btnW));
        apForwardBtn_.setBounds(dirRow.removeFromLeft(btnW));
        apRandomBtn_.setBounds(dirRow);
    }
    y += kRowHeight;

    // Duration row
    {
        auto durRow = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        apDurationSelector_.setBounds(durRow);
    }
    y += kRowHeight;

    // Clip Loops row
    {
        auto loopsRow = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        apClipLoopsSlider_.setBounds(loopsRow);
    }
    y += kRowHeight;

    // Loop + Master Layer row
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        apLoopToggle_.setBounds(row.removeFromLeft(row.getWidth() / 2));
        apMasterLayerSelector_.setBounds(row);
    }
    y += kRowHeight + kSectionGap;

    // Composition section
    y += kSectionHeaderHeight;
    masterControl_.setBounds(area.getX(), y, area.getWidth(), masterControl_.getPreferredHeight());
    y += masterControl_.getPreferredHeight();
    speedControl_.setBounds(area.getX(), y, area.getWidth(), speedControl_.getPreferredHeight());
    y += speedControl_.getPreferredHeight() + kSectionGap;

    // Video section
    y += kSectionHeaderHeight;
    opacityControl_.setBounds(area.getX(), y, area.getWidth(), opacityControl_.getPreferredHeight());
    y += opacityControl_.getPreferredHeight() + kSectionGap;

    // Transform section
    y += kSectionHeaderHeight;
    posXControl_.setBounds(area.getX(), y, area.getWidth(), posXControl_.getPreferredHeight());
    y += posXControl_.getPreferredHeight();
    posYControl_.setBounds(area.getX(), y, area.getWidth(), posYControl_.getPreferredHeight());
    y += posYControl_.getPreferredHeight();
    scaleControl_.setBounds(area.getX(), y, area.getWidth(), scaleControl_.getPreferredHeight());
    y += scaleControl_.getPreferredHeight();
    rotationControl_.setBounds(area.getX(), y, area.getWidth(), rotationControl_.getPreferredHeight());
    y += rotationControl_.getPreferredHeight();
    anchorControl_.setBounds(area.getX(), y, area.getWidth(), anchorControl_.getPreferredHeight());
    y += anchorControl_.getPreferredHeight() + kSectionGap;

    // Global Effects
    y += kSectionHeaderHeight;
    int fxHeight = effectStackView_.getPreferredHeight();
    effectStackView_.setBounds(area.getX(), y, area.getWidth(), fxHeight);
    y += fxHeight + kSectionGap;

    // Output Settings
    y += kSectionHeaderHeight;
    resolutionSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
}

void CompositionInspector::setComposition(Composition* comp)
{
    composition_ = comp;
    if (comp)
    {
        effectStackView_.setEffects(&comp->globalEffects);
        syncFromComposition();
    }
    else
    {
        effectStackView_.setEffects(nullptr);
    }
    resized();
    repaint();
}

void CompositionInspector::setEffectLibrary(EffectLibrary* lib)
{
    effectStackView_.setEffectLibrary(lib);
}

void CompositionInspector::setSignalRegistry(SignalRegistry* reg)
{
    signalRegistry_ = reg;
    macroPanel_.setSignalRegistry(reg);
    effectStackView_.setSignalRegistry(reg);
    masterControl_.setSignalRegistry(reg);
    speedControl_.setSignalRegistry(reg);
    opacityControl_.setSignalRegistry(reg);
    posXControl_.setSignalRegistry(reg);
    posYControl_.setSignalRegistry(reg);
    scaleControl_.setSignalRegistry(reg);
    rotationControl_.setSignalRegistry(reg);
    anchorControl_.setSignalRegistry(reg);
}

void CompositionInspector::setMacroBank(MacroBank* bank)
{
    macroPanel_.setMacroBank(bank);
    effectStackView_.setMacroBank(bank);
}

void CompositionInspector::refresh()
{
    if (composition_) syncFromComposition();
    effectStackView_.refresh();
    macroPanel_.refresh();
    repaint();
}

int CompositionInspector::getPreferredHeight() const
{
    int h = kNameBarHeight;
    h += MacroPanel::kPreferredHeight + kSectionGap;
    h += kSectionHeaderHeight + kRowHeight * 4 + kSectionGap; // Autopilot
    h += kSectionHeaderHeight + masterControl_.getPreferredHeight() + speedControl_.getPreferredHeight() + kSectionGap; // Composition
    h += kSectionHeaderHeight + opacityControl_.getPreferredHeight() + kSectionGap; // Video
    h += kSectionHeaderHeight; // Transform header
    h += posXControl_.getPreferredHeight() + posYControl_.getPreferredHeight()
       + scaleControl_.getPreferredHeight() + rotationControl_.getPreferredHeight()
       + anchorControl_.getPreferredHeight() + kSectionGap;
    h += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap; // Global Effects
    h += kSectionHeaderHeight + kRowHeight + 8; // Output Settings
    return h;
}

void CompositionInspector::paintSectionHeader(juce::Graphics& g,
                                               const juce::Rectangle<int>& bounds,
                                               const juce::String& title,
                                               bool hasPButton)
{
    // Teal/green tinted header for Transform (like Resolume)
    if (title == "Transform")
        g.setColour(juce::Colour(0xff1a3a3a));
    else
        g.setColour(juce::Colour(0xff222222));

    g.fillRect(bounds);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)).boldened());

    // Collapse triangle (decorative — always expanded for now)
    auto textBounds = bounds.withTrimmedLeft(4);
    juce::Path tri;
    float tx = textBounds.getX() + 2.0f;
    float ty = textBounds.getCentreY();
    tri.addTriangle(tx, ty - 3.0f, tx, ty + 3.0f, tx + 4.0f, ty);
    g.fillPath(tri);

    g.drawText(title, textBounds.withTrimmedLeft(10),
               juce::Justification::centredLeft, false);

    // "P." button on the right (Resolume-style, for Transform section)
    if (hasPButton)
    {
        auto pBounds = juce::Rectangle<int>(bounds.getRight() - 20, bounds.getY(), 20, bounds.getHeight());
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText("P.", pBounds, juce::Justification::centred, false);
    }
}

void CompositionInspector::syncFromComposition()
{
    if (!composition_) return;

    masterControl_.setParamValue(composition_->masterOpacity);
    speedControl_.setParamValue(composition_->masterSpeed / 4.0f); // [0,4] → [0,1]
    opacityControl_.setParamValue(composition_->compOpacity);

    // Autopilot direction buttons
    auto dir = composition_->autopilotDirection;
    auto setDirBtnColor = [](juce::TextButton& btn, bool active) {
        btn.setColour(juce::TextButton::buttonColourId,
            active ? juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f)
                   : juce::Colour(AudioDNALookAndFeel::kSurface));
    };
    setDirBtnColor(apRewindBtn_, dir == Composition::AutopilotDirection::Rewind);
    setDirBtnColor(apOffBtn_, dir == Composition::AutopilotDirection::Off);
    setDirBtnColor(apForwardBtn_, dir == Composition::AutopilotDirection::Forward);
    setDirBtnColor(apRandomBtn_, dir == Composition::AutopilotDirection::Random);

    apDurationSelector_.setSelectedId(
        static_cast<int>(composition_->autopilotDurationMode) + 1, juce::dontSendNotification);
    apClipLoopsSlider_.setValue(composition_->autopilotClipLoops, juce::dontSendNotification);
    apLoopToggle_.setToggleState(composition_->autopilotLoop, juce::dontSendNotification);
    apMasterLayerSelector_.setSelectedId(composition_->autopilotMasterLayer + 2, juce::dontSendNotification);

    // Transform
    posXControl_.setParamValue(composition_->compPositionX / 3840.0f + 0.5f);
    posYControl_.setParamValue(composition_->compPositionY / 2160.0f + 0.5f);
    scaleControl_.setParamValue(composition_->compScale / 2.0f);
    rotationControl_.setParamValue(composition_->compRotation / 720.0f + 0.5f);
    anchorControl_.setParamValue(composition_->compAnchorX / 3840.0f + 0.5f);

    // Resolution
    if (composition_->outputWidth == 1920) resolutionSelector_.setSelectedId(1, juce::dontSendNotification);
    else if (composition_->outputWidth == 1280) resolutionSelector_.setSelectedId(2, juce::dontSendNotification);
    else if (composition_->outputWidth == 2560) resolutionSelector_.setSelectedId(3, juce::dontSendNotification);
    else if (composition_->outputWidth == 3840) resolutionSelector_.setSelectedId(4, juce::dontSendNotification);
}
