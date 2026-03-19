#include "ui/LayerInspector.h"

LayerInspector::LayerInspector()
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

    apDurationSelector_.addItem("Clip Transport", 1);
    apDurationSelector_.addItem("Longest Clip", 2);
    apDurationSelector_.addItem("Custom", 3);
    apDurationSelector_.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(apDurationSelector_);

    apClipLoopsSlider_.setSliderStyle(juce::Slider::IncDecButtons);
    apClipLoopsSlider_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 30, 20);
    apClipLoopsSlider_.setRange(1, 99, 1);
    apClipLoopsSlider_.setValue(1, juce::dontSendNotification);
    addAndMakeVisible(apClipLoopsSlider_);

    apLoopToggle_.setColour(juce::ToggleButton::textColourId,
                            juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    addAndMakeVisible(apLoopToggle_);

    // --- Layer Master ---
    masterControl_.setParamName("Master");
    masterControl_.setParamValue(1.0f);
    masterControl_.onValueChanged = [this](float val) {
        if (layer_) layer_->opacity = val;
    };
    masterControl_.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
    addAndMakeVisible(masterControl_);

    // --- Video: Blend mode ---
    populateBlendModes();
    blendModeSelector_.onChange = [this] {
        if (!layer_) return;
        int sel = blendModeSelector_.getSelectedId();
        if (sel > 0) layer_->blendMode = static_cast<Layer::MixMode>(sel - 1);
    };
    addAndMakeVisible(blendModeSelector_);

    // Video: Opacity
    opacityControl_.setParamName("Opacity");
    opacityControl_.setParamValue(1.0f);
    opacityControl_.onValueChanged = [this](float val) {
        if (layer_) layer_->opacity = val;
    };
    opacityControl_.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
    addAndMakeVisible(opacityControl_);

    // Video: Width/Height
    auto setupIntSlider = [](juce::Slider& s, double min, double max, double val) {
        s.setSliderStyle(juce::Slider::IncDecButtons);
        s.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
        s.setRange(min, max, 1);
        s.setValue(val, juce::dontSendNotification);
        s.setScrollWheelEnabled(false);
    };
    setupIntSlider(widthSlider_, 1, 7680, 1920);
    widthSlider_.onValueChange = [this] {
        if (layer_) layer_->layerWidth = static_cast<int>(widthSlider_.getValue());
    };
    addAndMakeVisible(widthSlider_);

    setupIntSlider(heightSlider_, 1, 4320, 1080);
    heightSlider_.onValueChange = [this] {
        if (layer_) layer_->layerHeight = static_cast<int>(heightSlider_.getValue());
    };
    addAndMakeVisible(heightSlider_);

    // Video: Auto Size
    autoSizeSelector_.addItem("Off", 1);
    autoSizeSelector_.addItem("Fill", 2);
    autoSizeSelector_.addItem("Fit", 3);
    autoSizeSelector_.addItem("Stretch", 4);
    autoSizeSelector_.addItem("Original", 5);
    autoSizeSelector_.setSelectedId(1, juce::dontSendNotification);
    autoSizeSelector_.onChange = [this] {
        if (layer_) layer_->autoSize = static_cast<Layer::AutoSizeMode>(autoSizeSelector_.getSelectedId() - 1);
    };
    addAndMakeVisible(autoSizeSelector_);

    // --- Transition ---
    transitionBlendSelector_.addItem("Alpha", 1);
    transitionBlendSelector_.addItem("Add", 2);
    transitionBlendSelector_.addItem("Dissolve", 3);
    transitionBlendSelector_.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(transitionBlendSelector_);

    auto setupSlider = [](juce::Slider& s, double min, double max, double val) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        s.setRange(min, max, 0.01);
        s.setValue(val, juce::dontSendNotification);
        s.setScrollWheelEnabled(false);
        s.setColour(juce::Slider::thumbColourId,
                    juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    };

    setupSlider(transitionDurationSlider_, 0.0, 5.0, 0.0);
    transitionDurationSlider_.onValueChange = [this] {
        if (layer_) layer_->transitionSpeed = static_cast<float>(transitionDurationSlider_.getValue());
    };
    addAndMakeVisible(transitionDurationSlider_);

    // --- Keying ---
    populateKeyingModes();
    keyingModeSelector_.onChange = [this] {
        if (!layer_) return;
        int sel = keyingModeSelector_.getSelectedId();
        if (sel > 0) layer_->keyingMode = static_cast<Layer::KeyingMode>(sel - 1);
    };
    addAndMakeVisible(keyingModeSelector_);

    setupSlider(keyThresholdSlider_, 0.0, 1.0, 0.1);
    keyThresholdSlider_.onValueChange = [this] {
        if (layer_) layer_->keyThreshold = static_cast<float>(keyThresholdSlider_.getValue());
    };
    addAndMakeVisible(keyThresholdSlider_);

    setupSlider(keySoftnessSlider_, 0.0, 1.0, 0.1);
    keySoftnessSlider_.onValueChange = [this] {
        if (layer_) layer_->keySoftness = static_cast<float>(keySoftnessSlider_.getValue());
    };
    addAndMakeVisible(keySoftnessSlider_);

    // --- FX Only ---
    setupSlider(dryWetSlider_, 0.0, 1.0, 1.0);
    dryWetSlider_.onValueChange = [this] {
        if (layer_) layer_->dryWetMix = static_cast<float>(dryWetSlider_.getValue());
    };
    addAndMakeVisible(dryWetSlider_);

    // --- 3D Controls ---
    setupSlider(rotXSlider_, -180.0, 180.0, 0.0);
    rotXSlider_.onValueChange = [this] { if (layer_) layer_->rotationX = static_cast<float>(rotXSlider_.getValue()); };
    addAndMakeVisible(rotXSlider_);

    setupSlider(rotYSlider_, -180.0, 180.0, 0.0);
    rotYSlider_.onValueChange = [this] { if (layer_) layer_->rotationY = static_cast<float>(rotYSlider_.getValue()); };
    addAndMakeVisible(rotYSlider_);

    setupSlider(rotZSlider_, -180.0, 180.0, 0.0);
    rotZSlider_.onValueChange = [this] { if (layer_) layer_->rotationZ = static_cast<float>(rotZSlider_.getValue()); };
    addAndMakeVisible(rotZSlider_);

    setupSlider(rotSpeedSlider_, 0.0, 10.0, 0.0);
    rotSpeedSlider_.onValueChange = [this] { if (layer_) layer_->rotationSpeed = static_cast<float>(rotSpeedSlider_.getValue()); };
    addAndMakeVisible(rotSpeedSlider_);

    setupSlider(scale3DSlider_, 0.1, 5.0, 1.0);
    scale3DSlider_.onValueChange = [this] { if (layer_) layer_->scale3D = static_cast<float>(scale3DSlider_.getValue()); };
    addAndMakeVisible(scale3DSlider_);

    // --- Transform ---
    auto setupTransformParam = [this](UniversalParamControl& pc, const juce::String& name, float defVal) {
        pc.setParamName(name);
        pc.setParamValue(defVal);
        pc.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
        addAndMakeVisible(pc);
    };
    setupTransformParam(posXControl_, "Position X", 0.5f);
    setupTransformParam(posYControl_, "Position Y", 0.5f);
    setupTransformParam(scaleControl_, "Scale", 0.5f);
    setupTransformParam(rotationControl_, "Rotation", 0.5f);
    setupTransformParam(anchorControl_, "Anchor", 0.5f);

    posXControl_.onValueChanged = [this](float v) { if (layer_) layer_->positionX = (v - 0.5f) * 3840.0f; };
    posYControl_.onValueChanged = [this](float v) { if (layer_) layer_->positionY = (v - 0.5f) * 2160.0f; };
    scaleControl_.onValueChanged = [this](float v) { if (layer_) layer_->layerScale = v * 2.0f; };
    rotationControl_.onValueChanged = [this](float v) { if (layer_) layer_->layerRotation = (v - 0.5f) * 720.0f; };
    anchorControl_.onValueChanged = [this](float v) { if (layer_) layer_->layerAnchorX = (v - 0.5f) * 3840.0f; };

    // --- Layer Effects ---
    addAndMakeVisible(effectStackView_);

    // --- Autopilot Defaults ---
    defaultApActionSelector_.addItem("Play Next", 1);
    defaultApActionSelector_.addItem("Play Previous", 2);
    defaultApActionSelector_.addItem("Play Random", 3);
    defaultApActionSelector_.addItem("Play First", 4);
    defaultApActionSelector_.addItem("Play Last", 5);
    defaultApActionSelector_.addItem("Do Nothing", 6);
    defaultApActionSelector_.setSelectedId(1, juce::dontSendNotification);
    defaultApActionSelector_.onChange = [this] {
        if (!layer_) return;
        static const Clip::AutopilotAction actions[] = {
            Clip::AutopilotAction::PlayNext, Clip::AutopilotAction::PlayPrevious,
            Clip::AutopilotAction::PlayRandom, Clip::AutopilotAction::PlayFirst,
            Clip::AutopilotAction::PlayLast, Clip::AutopilotAction::DoNothing
        };
        int sel = defaultApActionSelector_.getSelectedId() - 1;
        if (sel >= 0 && sel < 6) layer_->defaultAutopilotAction = actions[sel];
    };
    addAndMakeVisible(defaultApActionSelector_);

    defaultApDurationSelector_.addItem("1 Beat", 1);
    defaultApDurationSelector_.addItem("2 Beats", 2);
    defaultApDurationSelector_.addItem("4 Beats", 3);
    defaultApDurationSelector_.addItem("8 Beats", 4);
    defaultApDurationSelector_.addItem("16 Beats", 5);
    defaultApDurationSelector_.addItem("32 Beats", 6);
    defaultApDurationSelector_.setSelectedId(3, juce::dontSendNotification);
    defaultApDurationSelector_.onChange = [this] {
        if (!layer_) return;
        static const Clip::AutopilotDuration durations[] = {
            Clip::AutopilotDuration::Beat1, Clip::AutopilotDuration::Beat2,
            Clip::AutopilotDuration::Beat4, Clip::AutopilotDuration::Beat8,
            Clip::AutopilotDuration::Beat16, Clip::AutopilotDuration::Beat32
        };
        int sel = defaultApDurationSelector_.getSelectedId() - 1;
        if (sel >= 0 && sel < 6) layer_->defaultAutopilotDuration = durations[sel];
    };
    addAndMakeVisible(defaultApDurationSelector_);
}

void LayerInspector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    if (!layer_)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.drawText("No layer selected", getLocalBounds(), juce::Justification::centred, false);
        return;
    }

    // Name bar
    auto nameBar = getLocalBounds().removeFromTop(kNameBarHeight);
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(nameBar);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(12.0f)).boldened());
    g.drawText(juce::String(layer_->name), nameBar.withTrimmedLeft(4).withTrimmedRight(40),
               juce::Justification::centredLeft, true);

    // Calculate section header positions
    int y = kNameBarHeight + MacroPanel::kPreferredHeight + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Autopilot");
    y += kSectionHeaderHeight + kRowHeight * 3 + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Layer");
    y += kSectionHeaderHeight + masterControl_.getPreferredHeight() + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Video");
    y += kSectionHeaderHeight + kRowHeight + opacityControl_.getPreferredHeight()
       + kRowHeight * 2 + kRowHeight + kSectionGap; // blend + opacity + w/h + autosize

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Transition");
    y += kSectionHeaderHeight + kRowHeight * 2 + kSectionGap;

    bool isTransparent = layer_->type == Layer::Type::Transparent;
    bool isFxOnly = layer_->type == Layer::Type::FXOnly;
    bool is3D = layer_->type == Layer::Type::ThreeD;

    if (isTransparent)
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Keying");
        y += kSectionHeaderHeight + kRowHeight * 3 + kSectionGap;
    }
    if (isFxOnly)
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Dry / Wet");
        y += kSectionHeaderHeight + kRowHeight + kSectionGap;
    }
    if (is3D)
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "3D Controls");
        y += kSectionHeaderHeight + kRowHeight * 5 + kSectionGap;
    }

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Transform", true);
    int transformH = posXControl_.getPreferredHeight() + posYControl_.getPreferredHeight()
                   + scaleControl_.getPreferredHeight() + rotationControl_.getPreferredHeight()
                   + anchorControl_.getPreferredHeight();
    y += kSectionHeaderHeight + transformH + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Layer Effects");
    y += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Autopilot Defaults");
}

void LayerInspector::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    int y = 0;

    // Name bar
    y += kNameBarHeight;

    // Dashboard
    macroPanel_.setBounds(area.getX(), y, area.getWidth(), MacroPanel::kPreferredHeight);
    y += MacroPanel::kPreferredHeight + kSectionGap;

    // Hide type-specific controls
    keyingModeSelector_.setVisible(false);
    keyThresholdSlider_.setVisible(false);
    keySoftnessSlider_.setVisible(false);
    dryWetSlider_.setVisible(false);
    rotXSlider_.setVisible(false); rotYSlider_.setVisible(false); rotZSlider_.setVisible(false);
    rotSpeedSlider_.setVisible(false); scale3DSlider_.setVisible(false);

    if (!layer_) return;

    bool isTransparent = layer_->type == Layer::Type::Transparent;
    bool isFxOnly = layer_->type == Layer::Type::FXOnly;
    bool is3D = layer_->type == Layer::Type::ThreeD;

    // --- Autopilot ---
    y += kSectionHeaderHeight;
    {
        auto dirRow = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        int btnW = dirRow.getWidth() / 4;
        apRewindBtn_.setBounds(dirRow.removeFromLeft(btnW));
        apOffBtn_.setBounds(dirRow.removeFromLeft(btnW));
        apForwardBtn_.setBounds(dirRow.removeFromLeft(btnW));
        apRandomBtn_.setBounds(dirRow);
    }
    y += kRowHeight;

    apDurationSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;

    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        apClipLoopsSlider_.setBounds(row.removeFromLeft(row.getWidth() / 2));
        apLoopToggle_.setBounds(row);
    }
    y += kRowHeight + kSectionGap;

    // --- Layer (Master) ---
    y += kSectionHeaderHeight;
    masterControl_.setBounds(area.getX(), y, area.getWidth(), masterControl_.getPreferredHeight());
    y += masterControl_.getPreferredHeight() + kSectionGap;

    // --- Video ---
    y += kSectionHeaderHeight;
    blendModeSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    opacityControl_.setBounds(area.getX(), y, area.getWidth(), opacityControl_.getPreferredHeight());
    y += opacityControl_.getPreferredHeight();
    widthSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    heightSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    autoSizeSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight + kSectionGap;

    // --- Transition ---
    y += kSectionHeaderHeight;
    transitionBlendSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    transitionDurationSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight + kSectionGap;

    // --- Keying (Transparent) ---
    if (isTransparent)
    {
        y += kSectionHeaderHeight;
        keyingModeSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        keyingModeSelector_.setVisible(true);
        y += kRowHeight;
        keyThresholdSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        keyThresholdSlider_.setVisible(true);
        y += kRowHeight;
        keySoftnessSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        keySoftnessSlider_.setVisible(true);
        y += kRowHeight + kSectionGap;
    }

    // --- FX Only ---
    if (isFxOnly)
    {
        y += kSectionHeaderHeight;
        dryWetSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        dryWetSlider_.setVisible(true);
        y += kRowHeight + kSectionGap;
    }

    // --- 3D Controls ---
    if (is3D)
    {
        y += kSectionHeaderHeight;
        rotXSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); rotXSlider_.setVisible(true); y += kRowHeight;
        rotYSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); rotYSlider_.setVisible(true); y += kRowHeight;
        rotZSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); rotZSlider_.setVisible(true); y += kRowHeight;
        rotSpeedSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); rotSpeedSlider_.setVisible(true); y += kRowHeight;
        scale3DSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); scale3DSlider_.setVisible(true); y += kRowHeight + kSectionGap;
    }

    // --- Transform ---
    y += kSectionHeaderHeight;
    posXControl_.setBounds(area.getX(), y, area.getWidth(), posXControl_.getPreferredHeight()); y += posXControl_.getPreferredHeight();
    posYControl_.setBounds(area.getX(), y, area.getWidth(), posYControl_.getPreferredHeight()); y += posYControl_.getPreferredHeight();
    scaleControl_.setBounds(area.getX(), y, area.getWidth(), scaleControl_.getPreferredHeight()); y += scaleControl_.getPreferredHeight();
    rotationControl_.setBounds(area.getX(), y, area.getWidth(), rotationControl_.getPreferredHeight()); y += rotationControl_.getPreferredHeight();
    anchorControl_.setBounds(area.getX(), y, area.getWidth(), anchorControl_.getPreferredHeight()); y += anchorControl_.getPreferredHeight() + kSectionGap;

    // --- Layer Effects ---
    y += kSectionHeaderHeight;
    int fxHeight = effectStackView_.getPreferredHeight();
    effectStackView_.setBounds(area.getX(), y, area.getWidth(), fxHeight);
    y += fxHeight + kSectionGap;

    // --- Autopilot Defaults ---
    y += kSectionHeaderHeight;
    defaultApActionSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    defaultApDurationSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
}

void LayerInspector::setLayer(Layer* layer)
{
    layer_ = layer;
    if (layer)
    {
        effectStackView_.setEffects(&layer->layerEffects);
        syncFromLayer();
    }
    else
        effectStackView_.setEffects(nullptr);
    resized();
    repaint();
}

void LayerInspector::setEffectLibrary(EffectLibrary* lib) { effectStackView_.setEffectLibrary(lib); }

void LayerInspector::setSignalRegistry(SignalRegistry* reg)
{
    signalRegistry_ = reg;
    macroPanel_.setSignalRegistry(reg);
    effectStackView_.setSignalRegistry(reg);
    masterControl_.setSignalRegistry(reg);
    opacityControl_.setSignalRegistry(reg);
    posXControl_.setSignalRegistry(reg);
    posYControl_.setSignalRegistry(reg);
    scaleControl_.setSignalRegistry(reg);
    rotationControl_.setSignalRegistry(reg);
    anchorControl_.setSignalRegistry(reg);
}

void LayerInspector::setMacroBank(MacroBank* bank)
{
    macroPanel_.setMacroBank(bank);
    effectStackView_.setMacroBank(bank);
}

void LayerInspector::refresh()
{
    if (layer_) { syncFromLayer(); effectStackView_.refresh(); macroPanel_.refresh(); }
    repaint();
}

int LayerInspector::getPreferredHeight() const
{
    if (!layer_) return 100;

    int h = kNameBarHeight + MacroPanel::kPreferredHeight + kSectionGap;
    h += kSectionHeaderHeight + kRowHeight * 3 + kSectionGap; // Autopilot
    h += kSectionHeaderHeight + masterControl_.getPreferredHeight() + kSectionGap; // Layer
    h += kSectionHeaderHeight + kRowHeight + opacityControl_.getPreferredHeight() + kRowHeight * 3 + kSectionGap; // Video
    h += kSectionHeaderHeight + kRowHeight * 2 + kSectionGap; // Transition

    if (layer_->type == Layer::Type::Transparent)
        h += kSectionHeaderHeight + kRowHeight * 3 + kSectionGap;
    if (layer_->type == Layer::Type::FXOnly)
        h += kSectionHeaderHeight + kRowHeight + kSectionGap;
    if (layer_->type == Layer::Type::ThreeD)
        h += kSectionHeaderHeight + kRowHeight * 5 + kSectionGap;

    h += kSectionHeaderHeight + posXControl_.getPreferredHeight() + posYControl_.getPreferredHeight()
       + scaleControl_.getPreferredHeight() + rotationControl_.getPreferredHeight()
       + anchorControl_.getPreferredHeight() + kSectionGap; // Transform
    h += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap;
    h += kSectionHeaderHeight + kRowHeight * 2 + 8; // Autopilot Defaults
    return h;
}

void LayerInspector::paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                         const juce::String& title, bool hasPButton)
{
    if (title == "Transform")
        g.setColour(juce::Colour(0xff1a3a3a));
    else
        g.setColour(juce::Colour(0xff222222));
    g.fillRect(bounds);

    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)).boldened());

    auto textBounds = bounds.withTrimmedLeft(4);
    juce::Path tri;
    float tx = textBounds.getX() + 2.0f;
    float ty = static_cast<float>(textBounds.getCentreY());
    tri.addTriangle(tx, ty - 3.0f, tx, ty + 3.0f, tx + 4.0f, ty);
    g.fillPath(tri);

    g.drawText(title, textBounds.withTrimmedLeft(10), juce::Justification::centredLeft, false);

    if (hasPButton)
    {
        auto pBounds = juce::Rectangle<int>(bounds.getRight() - 20, bounds.getY(), 20, bounds.getHeight());
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText("P.", pBounds, juce::Justification::centred, false);
    }
}

void LayerInspector::syncFromLayer()
{
    if (!layer_) return;

    masterControl_.setParamValue(layer_->opacity);
    opacityControl_.setParamValue(layer_->opacity);
    blendModeSelector_.setSelectedId(static_cast<int>(layer_->blendMode) + 1, juce::dontSendNotification);
    widthSlider_.setValue(layer_->layerWidth, juce::dontSendNotification);
    heightSlider_.setValue(layer_->layerHeight, juce::dontSendNotification);
    autoSizeSelector_.setSelectedId(static_cast<int>(layer_->autoSize) + 1, juce::dontSendNotification);

    float transSpeed = layer_->transitionSpeed < 0.0f ? 0.0f : layer_->transitionSpeed;
    transitionDurationSlider_.setValue(static_cast<double>(transSpeed), juce::dontSendNotification);

    keyingModeSelector_.setSelectedId(static_cast<int>(layer_->keyingMode) + 1, juce::dontSendNotification);
    keyThresholdSlider_.setValue(static_cast<double>(layer_->keyThreshold), juce::dontSendNotification);
    keySoftnessSlider_.setValue(static_cast<double>(layer_->keySoftness), juce::dontSendNotification);
    dryWetSlider_.setValue(static_cast<double>(layer_->dryWetMix), juce::dontSendNotification);

    rotXSlider_.setValue(static_cast<double>(layer_->rotationX), juce::dontSendNotification);
    rotYSlider_.setValue(static_cast<double>(layer_->rotationY), juce::dontSendNotification);
    rotZSlider_.setValue(static_cast<double>(layer_->rotationZ), juce::dontSendNotification);
    rotSpeedSlider_.setValue(static_cast<double>(layer_->rotationSpeed), juce::dontSendNotification);
    scale3DSlider_.setValue(static_cast<double>(layer_->scale3D), juce::dontSendNotification);

    posXControl_.setParamValue(layer_->positionX / 3840.0f + 0.5f);
    posYControl_.setParamValue(layer_->positionY / 2160.0f + 0.5f);
    scaleControl_.setParamValue(layer_->layerScale / 2.0f);
    rotationControl_.setParamValue(layer_->layerRotation / 720.0f + 0.5f);
    anchorControl_.setParamValue(layer_->layerAnchorX / 3840.0f + 0.5f);
}

void LayerInspector::populateBlendModes()
{
    blendModeSelector_.clear();
    static const char* const names[] = {
        "Normal", "Additive", "Screen", "Multiply", "Overlay",
        "Soft Light", "Hard Light", "Vivid Light", "Linear Light", "Pin Light", "Hard Mix",
        "Darken", "Lighten", "Darker Color", "Lighter Color",
        "Color Dodge", "Color Burn",
        "Difference", "Exclusion", "Subtract",
        "Hue", "Saturation", "Color", "Luminosity",
        "Dissolve"
    };
    for (int i = 0; i < 25; ++i)
        blendModeSelector_.addItem(names[i], i + 1);
    blendModeSelector_.setSelectedId(2, juce::dontSendNotification);
}

void LayerInspector::populateKeyingModes()
{
    keyingModeSelector_.clear();
    static const char* const names[] = {
        "Alpha", "Luma Key", "Inverted Luma Key", "Luma is Alpha",
        "Inverted Luma is Alpha", "Chroma Key", "Max RGB",
        "Saturation Key", "Edge Detection", "Threshold Mask",
        "Channel Red", "Channel Green", "Channel Blue"
    };
    for (int i = 0; i < 13; ++i)
        keyingModeSelector_.addItem(names[i], i + 1);
    keyingModeSelector_.setSelectedId(1, juce::dontSendNotification);
}
