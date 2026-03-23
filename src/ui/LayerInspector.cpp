#include "ui/LayerInspector.h"

LayerInspector::LayerInspector()
{
    // --- Name label (editable on click) ---
    nameLabel_.setFont(juce::Font(juce::FontOptions(12.0f)).boldened());
    nameLabel_.setColour(juce::Label::textColourId,
                         juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    nameLabel_.setColour(juce::Label::backgroundColourId, juce::Colour(0xff222222));
    nameLabel_.setColour(juce::Label::textWhenEditingColourId,
                         juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    nameLabel_.setColour(juce::Label::backgroundWhenEditingColourId,
                         juce::Colour(0xff2a2a2a));
    nameLabel_.setColour(juce::Label::outlineWhenEditingColourId,
                         juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    nameLabel_.setEditable(false, true, false); // editing triggered programmatically via mouseDown
    nameLabel_.setTooltip("Click to rename layer");
    nameLabel_.onTextChange = [this] {
        if (layer_)
        {
            layer_->name = nameLabel_.getText().toStdString();
            if (onLayerNameChanged)
                onLayerNameChanged();
        }
    };
    addAndMakeVisible(nameLabel_);

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

    apRewindBtn_.setTooltip("Autopilot: play previous clip on beat");
    apOffBtn_.setTooltip("Autopilot: disabled");
    apForwardBtn_.setTooltip("Autopilot: play next clip on beat");
    apRandomBtn_.setTooltip("Autopilot: play random clip on beat");

    apOffBtn_.onClick = [this] {
        if (!layer_) return;
        layer_->autopilotEnabled = false;
        updateAutopilotButtons();
    };
    apForwardBtn_.onClick = [this] {
        if (!layer_) return;
        layer_->autopilotEnabled = true;
        layer_->defaultAutopilotAction = Clip::AutopilotAction::PlayNext;
        updateAutopilotButtons();
    };
    apRewindBtn_.onClick = [this] {
        if (!layer_) return;
        layer_->autopilotEnabled = true;
        layer_->defaultAutopilotAction = Clip::AutopilotAction::PlayPrevious;
        updateAutopilotButtons();
    };
    apRandomBtn_.onClick = [this] {
        if (!layer_) return;
        layer_->autopilotEnabled = true;
        layer_->defaultAutopilotAction = Clip::AutopilotAction::PlayRandom;
        updateAutopilotButtons();
    };

    // Trigger mode: when to advance to the next clip
    apTriggerModeSelector_.addItem("End of Video", 1);
    apTriggerModeSelector_.addItem("On Beat", 2);
    apTriggerModeSelector_.setSelectedId(2, juce::dontSendNotification);
    apTriggerModeSelector_.setTooltip("When to advance to the next clip");
    apTriggerModeSelector_.onChange = [this] {
        if (!layer_) return;
        int sel = apTriggerModeSelector_.getSelectedId();
        if (sel == 1)
        {
            // End of Video mode: advance when playhead reaches outPoint
            layer_->autopilotEndOfVideo = true;
        }
        else
        {
            layer_->autopilotEndOfVideo = false;
            // On Beat mode: use the selected beat count
            int beatSel = apBeatCountSelector_.getSelectedId();
            static const Clip::AutopilotDuration durations[] = {
                Clip::AutopilotDuration::Beat1, Clip::AutopilotDuration::Beat2,
                Clip::AutopilotDuration::Beat4, Clip::AutopilotDuration::Beat8,
                Clip::AutopilotDuration::Beat16, Clip::AutopilotDuration::Beat32
            };
            if (beatSel >= 1 && beatSel <= 6)
                layer_->defaultAutopilotDuration = durations[beatSel - 1];
        }
        apBeatCountSelector_.setVisible(apTriggerModeSelector_.getSelectedId() == 2);
        resized();
    };
    addAndMakeVisible(apTriggerModeSelector_);

    // Beat count (visible when trigger mode is "On Beat")
    apBeatCountSelector_.addItem("1 Beat", 1);
    apBeatCountSelector_.addItem("2 Beats", 2);
    apBeatCountSelector_.addItem("4 Beats", 3);
    apBeatCountSelector_.addItem("8 Beats", 4);
    apBeatCountSelector_.addItem("16 Beats", 5);
    apBeatCountSelector_.addItem("32 Beats", 6);
    apBeatCountSelector_.setSelectedId(3, juce::dontSendNotification); // Default: 4 beats
    apBeatCountSelector_.setTooltip("Number of beats before advancing");
    apBeatCountSelector_.onChange = [this] {
        if (!layer_) return;
        static const Clip::AutopilotDuration durations[] = {
            Clip::AutopilotDuration::Beat1, Clip::AutopilotDuration::Beat2,
            Clip::AutopilotDuration::Beat4, Clip::AutopilotDuration::Beat8,
            Clip::AutopilotDuration::Beat16, Clip::AutopilotDuration::Beat32
        };
        int sel = apBeatCountSelector_.getSelectedId() - 1;
        if (sel >= 0 && sel < 6) layer_->defaultAutopilotDuration = durations[sel];
    };
    addAndMakeVisible(apBeatCountSelector_);

    // Loops: how many times to loop a clip before advancing
    apLoopsLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    apLoopsLabel_.setColour(juce::Label::textColourId,
                            juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(apLoopsLabel_);

    apLoopsSlider_.setSliderStyle(juce::Slider::IncDecButtons);
    apLoopsSlider_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 30, 20);
    apLoopsSlider_.setRange(1, 99, 1);
    apLoopsSlider_.setValue(1, juce::dontSendNotification);
    apLoopsSlider_.setDefaultValue(1.0);
    apLoopsSlider_.setTooltip("Number of loops before advancing to next clip");
    apLoopsSlider_.setScrollWheelEnabled(false);
    apLoopsSlider_.onValueChange = [this] {
        if (layer_)
            layer_->autopilotLoops = static_cast<int>(apLoopsSlider_.getValue());
    };
    addAndMakeVisible(apLoopsSlider_);

    // --- Layer Master ---
    masterControl_.setParamName("Master");
    masterControl_.setParamValue(1.0f);
    masterControl_.setDefaultValue(1.0f);
    masterControl_.onValueChanged = [this](float val) {
        if (layer_) layer_->opacity = val;
    };
    masterControl_.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
    addAndMakeVisible(masterControl_);

    // P21: Persistent layer toggle
    persistentToggle_.setColour(juce::ToggleButton::textColourId,
                                juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    persistentToggle_.setTooltip("Keep this layer rendering when switching to another deck");
    persistentToggle_.onClick = [this] {
        if (layer_) layer_->persistent = persistentToggle_.getToggleState();
    };
    addAndMakeVisible(persistentToggle_);

    // Ignore Column Trigger toggle
    ignoreColumnToggle_.setColour(juce::ToggleButton::textColourId,
                                  juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    ignoreColumnToggle_.setTooltip("This layer ignores column trigger buttons");
    ignoreColumnToggle_.onClick = [this] {
        if (layer_) layer_->ignoreColumnTrigger = ignoreColumnToggle_.getToggleState();
    };
    addAndMakeVisible(ignoreColumnToggle_);

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
    opacityControl_.setDefaultValue(1.0f);
    opacityControl_.onValueChanged = [this](float val) {
        if (layer_) layer_->opacity = val;
    };
    opacityControl_.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
    addAndMakeVisible(opacityControl_);

    // Video: Width/Height
    auto setupIntSlider = [](ResettableSlider& s, double min, double max, double val) {
        s.setSliderStyle(juce::Slider::IncDecButtons);
        s.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
        s.setRange(min, max, 1);
        s.setValue(val, juce::dontSendNotification);
        s.setDefaultValue(val);
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

    // --- Transition --- (same full list as LayerStrip's F dropdown)
    {
        using M = Layer::MixMode;

        transitionBlendSelector_.addSectionHeading("Compositing");
        transitionBlendSelector_.addItem("Alpha", 1 + static_cast<int>(M::Normal));
        transitionBlendSelector_.addItem("Add", 1 + static_cast<int>(M::Additive));
        transitionBlendSelector_.addItem("Screen", 1 + static_cast<int>(M::Screen));
        transitionBlendSelector_.addItem("Multiply", 1 + static_cast<int>(M::Multiply));
        transitionBlendSelector_.addItem("Overlay", 1 + static_cast<int>(M::Overlay));

        transitionBlendSelector_.addSectionHeading("Light");
        transitionBlendSelector_.addItem("Soft Light", 1 + static_cast<int>(M::SoftLight));
        transitionBlendSelector_.addItem("Hard Light", 1 + static_cast<int>(M::HardLight));
        transitionBlendSelector_.addItem("Vivid Light", 1 + static_cast<int>(M::VividLight));
        transitionBlendSelector_.addItem("Linear Light", 1 + static_cast<int>(M::LinearLight));
        transitionBlendSelector_.addItem("Pin Light", 1 + static_cast<int>(M::PinLight));
        transitionBlendSelector_.addItem("Hard Mix", 1 + static_cast<int>(M::HardMix));

        transitionBlendSelector_.addSectionHeading("Compare");
        transitionBlendSelector_.addItem("Darken", 1 + static_cast<int>(M::Darken));
        transitionBlendSelector_.addItem("Lighten", 1 + static_cast<int>(M::Lighten));
        transitionBlendSelector_.addItem("Darker Color", 1 + static_cast<int>(M::DarkerColor));
        transitionBlendSelector_.addItem("Lighter Color", 1 + static_cast<int>(M::LighterColor));

        transitionBlendSelector_.addSectionHeading("Dodge / Burn");
        transitionBlendSelector_.addItem("Color Dodge", 1 + static_cast<int>(M::ColorDodge));
        transitionBlendSelector_.addItem("Color Burn", 1 + static_cast<int>(M::ColorBurn));

        transitionBlendSelector_.addSectionHeading("Inversion");
        transitionBlendSelector_.addItem("Difference", 1 + static_cast<int>(M::Difference));
        transitionBlendSelector_.addItem("Exclusion", 1 + static_cast<int>(M::Exclusion));
        transitionBlendSelector_.addItem("Subtract", 1 + static_cast<int>(M::Subtract));

        transitionBlendSelector_.addSectionHeading("Component");
        transitionBlendSelector_.addItem("Hue", 1 + static_cast<int>(M::Hue));
        transitionBlendSelector_.addItem("Saturation", 1 + static_cast<int>(M::Saturation));
        transitionBlendSelector_.addItem("Color", 1 + static_cast<int>(M::Color));
        transitionBlendSelector_.addItem("Luminosity", 1 + static_cast<int>(M::Luminosity));

        transitionBlendSelector_.addSectionHeading("Special");
        transitionBlendSelector_.addItem("Dissolve", 1 + static_cast<int>(M::Dissolve));
        transitionBlendSelector_.addItem("Cut", 1 + static_cast<int>(M::Cut));

        transitionBlendSelector_.addSectionHeading("Wipe");
        transitionBlendSelector_.addItem("Wipe Left", 1 + static_cast<int>(M::WipeLeft));
        transitionBlendSelector_.addItem("Wipe Right", 1 + static_cast<int>(M::WipeRight));
        transitionBlendSelector_.addItem("Wipe Up", 1 + static_cast<int>(M::WipeUp));
        transitionBlendSelector_.addItem("Wipe Down", 1 + static_cast<int>(M::WipeDown));
        transitionBlendSelector_.addItem("Wipe Ellipse", 1 + static_cast<int>(M::WipeEllipse));
        transitionBlendSelector_.addItem("Wipe Diagonal", 1 + static_cast<int>(M::WipeDiagonal));

        transitionBlendSelector_.addSectionHeading("Push");
        transitionBlendSelector_.addItem("Push Left", 1 + static_cast<int>(M::PushLeft));
        transitionBlendSelector_.addItem("Push Right", 1 + static_cast<int>(M::PushRight));
        transitionBlendSelector_.addItem("Push Up", 1 + static_cast<int>(M::PushUp));
        transitionBlendSelector_.addItem("Push Down", 1 + static_cast<int>(M::PushDown));

        transitionBlendSelector_.addSectionHeading("Zoom");
        transitionBlendSelector_.addItem("Zoom In", 1 + static_cast<int>(M::ZoomIn));
        transitionBlendSelector_.addItem("Zoom Out", 1 + static_cast<int>(M::ZoomOut));

        transitionBlendSelector_.addSectionHeading("3D");
        transitionBlendSelector_.addItem("Rotate X", 1 + static_cast<int>(M::RotateX));
        transitionBlendSelector_.addItem("Rotate Y", 1 + static_cast<int>(M::RotateY));
        transitionBlendSelector_.addItem("Spin", 1 + static_cast<int>(M::Spin));
        transitionBlendSelector_.addItem("Cube", 1 + static_cast<int>(M::Cube));
        transitionBlendSelector_.addItem("Flip", 1 + static_cast<int>(M::Flip));
        transitionBlendSelector_.addItem("Fold", 1 + static_cast<int>(M::Fold));

        transitionBlendSelector_.addSectionHeading("Color Fade");
        transitionBlendSelector_.addItem("To Black", 1 + static_cast<int>(M::ToBlack));
        transitionBlendSelector_.addItem("To White", 1 + static_cast<int>(M::ToWhite));

        transitionBlendSelector_.addSectionHeading("Creative");
        transitionBlendSelector_.addItem("Pixelate", 1 + static_cast<int>(M::Pixelate));
        transitionBlendSelector_.addItem("Blur", 1 + static_cast<int>(M::Blur));
        transitionBlendSelector_.addItem("Noise", 1 + static_cast<int>(M::Noise));
        transitionBlendSelector_.addItem("RGB Split", 1 + static_cast<int>(M::RGBSplit));
        transitionBlendSelector_.addItem("Glitch Blocks", 1 + static_cast<int>(M::GlitchBlocks));
        transitionBlendSelector_.addItem("Strobe", 1 + static_cast<int>(M::Strobe));
        transitionBlendSelector_.addItem("Slide", 1 + static_cast<int>(M::Slide));
        transitionBlendSelector_.addItem("Stretch", 1 + static_cast<int>(M::Stretch));
        transitionBlendSelector_.addItem("Displace", 1 + static_cast<int>(M::Displace));

        transitionBlendSelector_.setSelectedId(1 + static_cast<int>(M::Dissolve), juce::dontSendNotification);
    }
    transitionBlendSelector_.onChange = [this] {
        if (!layer_) return;
        int sel = transitionBlendSelector_.getSelectedId();
        if (sel >= 1)
            layer_->transitionMode = static_cast<Layer::MixMode>(sel - 1);
    };
    addAndMakeVisible(transitionBlendSelector_);

    auto setupSlider = [](ResettableSlider& s, double min, double max, double val) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        s.setRange(min, max, 0.01);
        s.setValue(val, juce::dontSendNotification);
        s.setDefaultValue(val);
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
        pc.setDefaultValue(defVal);
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
    scaleControl_.onValueChanged = [this](float v) { if (layer_) layer_->layerScale = std::pow(2.0f, (v - 0.5f) * 2.0f); };
    rotationControl_.onValueChanged = [this](float v) { if (layer_) layer_->layerRotation = (v - 0.5f) * 720.0f; };
    anchorControl_.onValueChanged = [this](float v) { if (layer_) layer_->layerAnchorX = (v - 0.5f) * 3840.0f; };

    // --- Feedback ---
    feedbackEnableBtn_.setColour(juce::ToggleButton::textColourId,
                                  juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    feedbackEnableBtn_.setColour(juce::ToggleButton::tickColourId,
                                  juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    feedbackEnableBtn_.setTooltip("Enable feedback loop (Larsen effect)");
    feedbackEnableBtn_.onClick = [this] {
        if (layer_) layer_->feedback.enabled = feedbackEnableBtn_.getToggleState();
    };
    addAndMakeVisible(feedbackEnableBtn_);

    feedbackPresetSelector_.addItem("Custom", 1);
    {
        int idx = 2;
        for (const auto& p : FeedbackProcessor::getPresets())
            feedbackPresetSelector_.addItem(juce::String(p.name), idx++);
    }
    feedbackPresetSelector_.setSelectedId(1, juce::dontSendNotification);
    feedbackPresetSelector_.setTooltip("Feedback preset");
    feedbackPresetSelector_.onChange = [this] {
        if (!layer_) return;
        int sel = feedbackPresetSelector_.getSelectedId();
        if (sel > 1)
        {
            auto& presets = FeedbackProcessor::getPresets();
            int idx = sel - 2;
            if (idx >= 0 && idx < static_cast<int>(presets.size()))
            {
                layer_->feedback = presets[static_cast<size_t>(idx)].config;
                feedbackEnableBtn_.setToggleState(true, juce::dontSendNotification);
                syncFromLayer();
            }
        }
    };
    addAndMakeVisible(feedbackPresetSelector_);

    auto setupFbSlider = [this](ResettableSlider& s, double min, double max, double val,
                                 const juce::String& tooltip) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        s.setRange(min, max, 0.01);
        s.setValue(val, juce::dontSendNotification);
        s.setDefaultValue(val);
        s.setScrollWheelEnabled(false);
        s.setTooltip(tooltip);
        s.setColour(juce::Slider::thumbColourId,
                    juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        addAndMakeVisible(s);
    };

    setupFbSlider(feedbackAmountSlider_, 0.0, 1.0, 0.5, "Feedback amount (0 = none, 1 = full)");
    feedbackAmountSlider_.onValueChange = [this] {
        if (layer_) { layer_->feedback.amount = static_cast<float>(feedbackAmountSlider_.getValue()); layer_->feedback.presetName.clear(); feedbackPresetSelector_.setSelectedId(1, juce::dontSendNotification); }
    };

    setupFbSlider(feedbackScaleXSlider_, 0.5, 1.5, 0.98, "Feedback horizontal scale");
    feedbackScaleXSlider_.onValueChange = [this] {
        if (layer_) { layer_->feedback.scaleX = static_cast<float>(feedbackScaleXSlider_.getValue()); layer_->feedback.presetName.clear(); feedbackPresetSelector_.setSelectedId(1, juce::dontSendNotification); }
    };

    setupFbSlider(feedbackScaleYSlider_, 0.5, 1.5, 0.98, "Feedback vertical scale");
    feedbackScaleYSlider_.onValueChange = [this] {
        if (layer_) { layer_->feedback.scaleY = static_cast<float>(feedbackScaleYSlider_.getValue()); layer_->feedback.presetName.clear(); feedbackPresetSelector_.setSelectedId(1, juce::dontSendNotification); }
    };

    setupFbSlider(feedbackRotationSlider_, -15.0, 15.0, 0.0, "Feedback rotation per frame (degrees)");
    feedbackRotationSlider_.onValueChange = [this] {
        if (layer_) { layer_->feedback.rotation = static_cast<float>(feedbackRotationSlider_.getValue()); layer_->feedback.presetName.clear(); feedbackPresetSelector_.setSelectedId(1, juce::dontSendNotification); }
    };

    setupFbSlider(feedbackOffsetXSlider_, -0.1, 0.1, 0.0, "Feedback horizontal offset per frame");
    feedbackOffsetXSlider_.onValueChange = [this] {
        if (layer_) { layer_->feedback.offsetX = static_cast<float>(feedbackOffsetXSlider_.getValue()); layer_->feedback.presetName.clear(); feedbackPresetSelector_.setSelectedId(1, juce::dontSendNotification); }
    };

    setupFbSlider(feedbackOffsetYSlider_, -0.1, 0.1, 0.0, "Feedback vertical offset per frame");
    feedbackOffsetYSlider_.onValueChange = [this] {
        if (layer_) { layer_->feedback.offsetY = static_cast<float>(feedbackOffsetYSlider_.getValue()); layer_->feedback.presetName.clear(); feedbackPresetSelector_.setSelectedId(1, juce::dontSendNotification); }
    };

    setupFbSlider(feedbackLumaKeySlider_, 0.0, 1.0, 0.0, "Luma key: fade dark areas from feedback");
    feedbackLumaKeySlider_.onValueChange = [this] {
        if (layer_) { layer_->feedback.lumaKey = static_cast<float>(feedbackLumaKeySlider_.getValue()); layer_->feedback.presetName.clear(); feedbackPresetSelector_.setSelectedId(1, juce::dontSendNotification); }
    };

    // --- Layer Effects ---
    addAndMakeVisible(effectStackView_);

}

void LayerInspector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // FX drop highlight
    if (fxDropHighlight_)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.15f));
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.6f));
        g.drawRect(getLocalBounds(), 2);
    }

    if (!layer_)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.drawText("No layer selected", getLocalBounds(), juce::Justification::centred, false);
        return;
    }

    // Name bar background (label is a child component drawn on top)
    auto nameBar = getLocalBounds().removeFromTop(kNameBarHeight);
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(nameBar);

    // Calculate section header positions
    int y = kNameBarHeight + MacroPanel::kPreferredHeight + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Autopilot");
    {
        int apRows = 3; // direction + trigger mode + loops
        if (apTriggerModeSelector_.getSelectedId() == 2)
            apRows += 1; // beat count row
        y += kSectionHeaderHeight + kRowHeight * apRows + kSectionGap;
    }

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Layer");
    y += kSectionHeaderHeight + masterControl_.getPreferredHeight() + kRowHeight + kSectionGap;

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

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Feedback");
    // Enable + Preset row + 7 slider rows
    y += kSectionHeaderHeight + kRowHeight * 9 + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Layer Effects");
}

void LayerInspector::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    int y = 0;

    // Name label
    nameLabel_.setBounds(area.getX(), y, area.getWidth(), kNameBarHeight);
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

    apTriggerModeSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;

    // Show beat count only in "On Beat" mode
    bool onBeatMode = (apTriggerModeSelector_.getSelectedId() == 2);
    apBeatCountSelector_.setVisible(onBeatMode);
    if (onBeatMode)
    {
        apBeatCountSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        y += kRowHeight;
    }

    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        apLoopsLabel_.setBounds(row.removeFromLeft(40));
        apLoopsSlider_.setBounds(row);
    }
    y += kRowHeight + kSectionGap;

    // --- Layer (Master) ---
    y += kSectionHeaderHeight;
    masterControl_.setBounds(area.getX(), y, area.getWidth(), masterControl_.getPreferredHeight());
    y += masterControl_.getPreferredHeight();
    persistentToggle_.setBounds(area.getX(), y, area.getWidth() / 2, kRowHeight);
    ignoreColumnToggle_.setBounds(area.getX() + area.getWidth() / 2, y, area.getWidth() / 2, kRowHeight);
    y += kRowHeight + kSectionGap;

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

    // --- Feedback ---
    y += kSectionHeaderHeight;
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        feedbackEnableBtn_.setBounds(row.removeFromLeft(area.getWidth() / 3));
        feedbackPresetSelector_.setBounds(row);
    }
    y += kRowHeight;
    feedbackAmountSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
    feedbackScaleXSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
    feedbackScaleYSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
    feedbackRotationSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
    feedbackOffsetXSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
    feedbackOffsetYSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
    feedbackLumaKeySlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight + kSectionGap;

    // --- Layer Effects ---
    y += kSectionHeaderHeight;
    int fxHeight = effectStackView_.getPreferredHeight();
    effectStackView_.setBounds(area.getX(), y, area.getWidth(), fxHeight);
}

void LayerInspector::mouseDown(const juce::MouseEvent& event)
{
    // Programmatically trigger name label editing when clicking in the name bar area
    if (layer_ && nameLabel_.getBounds().contains(event.getPosition()))
    {
        nameLabel_.showEditor();
        return;
    }
    Component::mouseDown(event);
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
    {
        int apRows = 3;
        if (apTriggerModeSelector_.getSelectedId() == 2) apRows += 1;
        h += kSectionHeaderHeight + kRowHeight * apRows + kSectionGap; // Autopilot
    }
    h += kSectionHeaderHeight + masterControl_.getPreferredHeight() + kRowHeight + kSectionGap; // Layer + toggles
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
    h += kSectionHeaderHeight + kRowHeight * 9 + kSectionGap; // Feedback: enable+preset + 7 sliders
    h += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap;
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

void LayerInspector::updateAutopilotButtons()
{
    auto defaultCol = juce::Colour(AudioDNALookAndFeel::kSurface);
    auto activeCol = juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f);

    bool enabled = layer_ && layer_->autopilotEnabled;
    auto action = layer_ ? layer_->defaultAutopilotAction : Clip::AutopilotAction::PlayNext;

    apOffBtn_.setColour(juce::TextButton::buttonColourId,
        !enabled ? activeCol : defaultCol);
    apForwardBtn_.setColour(juce::TextButton::buttonColourId,
        (enabled && action == Clip::AutopilotAction::PlayNext) ? activeCol : defaultCol);
    apRewindBtn_.setColour(juce::TextButton::buttonColourId,
        (enabled && action == Clip::AutopilotAction::PlayPrevious) ? activeCol : defaultCol);
    apRandomBtn_.setColour(juce::TextButton::buttonColourId,
        (enabled && action == Clip::AutopilotAction::PlayRandom) ? activeCol : defaultCol);

    repaint();
}

void LayerInspector::syncFromLayer()
{
    if (!layer_) return;

    // Sync name label — skip if user is actively editing to avoid closing the editor
    if (!nameLabel_.isBeingEdited())
        nameLabel_.setText(juce::String(layer_->name), juce::dontSendNotification);

    updateAutopilotButtons();

    // Sync loops slider
    apLoopsSlider_.setValue(static_cast<double>(layer_->autopilotLoops), juce::dontSendNotification);

    // Sync autopilot trigger mode
    if (layer_->autopilotEndOfVideo)
    {
        apTriggerModeSelector_.setSelectedId(1, juce::dontSendNotification); // End of Video
    }
    else
    {
        apTriggerModeSelector_.setSelectedId(2, juce::dontSendNotification); // On Beat
        // Map duration enum to selector ID
        auto dur = layer_->defaultAutopilotDuration;
        static const int durToSel[] = { 3, 1, 2, 3, 4, 5, 6, 3 }; // LayerDet=4beats, Beat1-32=1-6, Custom=4beats
        int idx = static_cast<int>(dur);
        if (idx >= 0 && idx < 8)
            apBeatCountSelector_.setSelectedId(durToSel[idx], juce::dontSendNotification);
    }

    masterControl_.setParamValue(layer_->opacity);
    persistentToggle_.setToggleState(layer_->persistent, juce::dontSendNotification);
    ignoreColumnToggle_.setToggleState(layer_->ignoreColumnTrigger, juce::dontSendNotification);
    opacityControl_.setParamValue(layer_->opacity);
    blendModeSelector_.setSelectedId(static_cast<int>(layer_->blendMode) + 1, juce::dontSendNotification);
    widthSlider_.setValue(layer_->layerWidth, juce::dontSendNotification);
    heightSlider_.setValue(layer_->layerHeight, juce::dontSendNotification);
    autoSizeSelector_.setSelectedId(static_cast<int>(layer_->autoSize) + 1, juce::dontSendNotification);

    transitionBlendSelector_.setSelectedId(1 + static_cast<int>(layer_->transitionMode), juce::dontSendNotification);
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
    scaleControl_.setParamValue(std::log2(std::max(0.01f, layer_->layerScale)) / 2.0f + 0.5f);
    rotationControl_.setParamValue(layer_->layerRotation / 720.0f + 0.5f);
    anchorControl_.setParamValue(layer_->layerAnchorX / 3840.0f + 0.5f);

    // --- Feedback ---
    feedbackEnableBtn_.setToggleState(layer_->feedback.enabled, juce::dontSendNotification);
    feedbackAmountSlider_.setValue(static_cast<double>(layer_->feedback.amount), juce::dontSendNotification);
    feedbackScaleXSlider_.setValue(static_cast<double>(layer_->feedback.scaleX), juce::dontSendNotification);
    feedbackScaleYSlider_.setValue(static_cast<double>(layer_->feedback.scaleY), juce::dontSendNotification);
    feedbackRotationSlider_.setValue(static_cast<double>(layer_->feedback.rotation), juce::dontSendNotification);
    feedbackOffsetXSlider_.setValue(static_cast<double>(layer_->feedback.offsetX), juce::dontSendNotification);
    feedbackOffsetYSlider_.setValue(static_cast<double>(layer_->feedback.offsetY), juce::dontSendNotification);
    feedbackLumaKeySlider_.setValue(static_cast<double>(layer_->feedback.lumaKey), juce::dontSendNotification);

    // Match preset selector
    if (!layer_->feedback.presetName.empty())
    {
        const auto& presets = FeedbackProcessor::getPresets();
        for (int i = 0; i < static_cast<int>(presets.size()); ++i)
        {
            if (presets[static_cast<size_t>(i)].name == layer_->feedback.presetName)
            {
                feedbackPresetSelector_.setSelectedId(i + 2, juce::dontSendNotification);
                break;
            }
        }
    }
    else
    {
        feedbackPresetSelector_.setSelectedId(1, juce::dontSendNotification);
    }
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

bool LayerInspector::isInterestedInDragSource(const SourceDetails& details)
{
    return layer_ != nullptr && details.description.toString().startsWith("fx:");
}

void LayerInspector::itemDragEnter(const SourceDetails&)
{
    fxDropHighlight_ = true;
    repaint();
}

void LayerInspector::itemDragExit(const SourceDetails&)
{
    fxDropHighlight_ = false;
    repaint();
}

void LayerInspector::itemDropped(const SourceDetails& details)
{
    fxDropHighlight_ = false;
    if (!layer_) return;
    effectStackView_.itemDropped(details);
    repaint();
}
