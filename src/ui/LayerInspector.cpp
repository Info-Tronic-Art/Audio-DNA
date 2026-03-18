#include "ui/LayerInspector.h"

LayerInspector::LayerInspector()
{
    addAndMakeVisible(macroPanel_);

    // Blend mode selector
    populateBlendModes();
    blendModeSelector_.onChange = [this] {
        if (!layer_) return;
        int sel = blendModeSelector_.getSelectedId();
        if (sel > 0)
            layer_->blendMode = static_cast<Layer::MixMode>(sel - 1);
    };
    addAndMakeVisible(blendModeSelector_);

    // Keying mode
    populateKeyingModes();
    keyingModeSelector_.onChange = [this] {
        if (!layer_) return;
        int sel = keyingModeSelector_.getSelectedId();
        if (sel > 0)
            layer_->keyingMode = static_cast<Layer::KeyingMode>(sel - 1);
    };
    addAndMakeVisible(keyingModeSelector_);

    // Keying sliders
    auto setupSlider = [](juce::Slider& s, double min, double max, double val) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        s.setRange(min, max, 0.01);
        s.setValue(val, juce::dontSendNotification);
        s.setColour(juce::Slider::thumbColourId,
                    juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    };

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

    // Dry/wet
    setupSlider(dryWetSlider_, 0.0, 1.0, 1.0);
    dryWetSlider_.onValueChange = [this] {
        if (layer_) layer_->dryWetMix = static_cast<float>(dryWetSlider_.getValue());
    };
    addAndMakeVisible(dryWetSlider_);

    // 3D controls
    setupSlider(rotXSlider_, -180.0, 180.0, 0.0);
    rotXSlider_.onValueChange = [this] {
        if (layer_) layer_->rotationX = static_cast<float>(rotXSlider_.getValue());
    };
    addAndMakeVisible(rotXSlider_);

    setupSlider(rotYSlider_, -180.0, 180.0, 0.0);
    rotYSlider_.onValueChange = [this] {
        if (layer_) layer_->rotationY = static_cast<float>(rotYSlider_.getValue());
    };
    addAndMakeVisible(rotYSlider_);

    setupSlider(rotZSlider_, -180.0, 180.0, 0.0);
    rotZSlider_.onValueChange = [this] {
        if (layer_) layer_->rotationZ = static_cast<float>(rotZSlider_.getValue());
    };
    addAndMakeVisible(rotZSlider_);

    setupSlider(rotSpeedSlider_, 0.0, 10.0, 0.0);
    rotSpeedSlider_.onValueChange = [this] {
        if (layer_) layer_->rotationSpeed = static_cast<float>(rotSpeedSlider_.getValue());
    };
    addAndMakeVisible(rotSpeedSlider_);

    setupSlider(scale3DSlider_, 0.1, 5.0, 1.0);
    scale3DSlider_.onValueChange = [this] {
        if (layer_) layer_->scale3D = static_cast<float>(scale3DSlider_.getValue());
    };
    addAndMakeVisible(scale3DSlider_);

    // Effect stack
    addAndMakeVisible(effectStackView_);

    // Autopilot defaults
    defaultApActionSelector_.addItem("Play Next", 1);
    defaultApActionSelector_.addItem("Play Previous", 2);
    defaultApActionSelector_.addItem("Play Random", 3);
    defaultApActionSelector_.addItem("Play First", 4);
    defaultApActionSelector_.addItem("Play Last", 5);
    defaultApActionSelector_.addItem("Do Nothing", 6);
    defaultApActionSelector_.setSelectedId(1, juce::dontSendNotification);
    defaultApActionSelector_.onChange = [this] {
        if (!layer_) return;
        // Map dropdown IDs to enum values
        static const Clip::AutopilotAction actions[] = {
            Clip::AutopilotAction::PlayNext, Clip::AutopilotAction::PlayPrevious,
            Clip::AutopilotAction::PlayRandom, Clip::AutopilotAction::PlayFirst,
            Clip::AutopilotAction::PlayLast, Clip::AutopilotAction::DoNothing
        };
        int sel = defaultApActionSelector_.getSelectedId() - 1;
        if (sel >= 0 && sel < 6)
            layer_->defaultAutopilotAction = actions[sel];
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
        if (sel >= 0 && sel < 6)
            layer_->defaultAutopilotDuration = durations[sel];
    };
    addAndMakeVisible(defaultApDurationSelector_);

    // Transition speed
    setupSlider(transitionSpeedSlider_, 0.0, 5.0, 0.3);
    transitionSpeedSlider_.onValueChange = [this] {
        if (layer_)
            layer_->transitionSpeed = static_cast<float>(transitionSpeedSlider_.getValue());
    };
    addAndMakeVisible(transitionSpeedSlider_);
}

void LayerInspector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    if (!layer_)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.drawText("No layer selected", getLocalBounds(),
                   juce::Justification::centred, false);
        return;
    }

    // Paint section headers at calculated positions
    int y = MacroPanel::kPreferredHeight + kSectionGap;

    bool isTransparent = layer_->type == Layer::Type::Transparent;
    bool isFxOnly = layer_->type == Layer::Type::FXOnly;
    bool is3D = layer_->type == Layer::Type::ThreeD;

    if (isTransparent)
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "BLEND MODE");
        y += kSectionHeaderHeight + kRowHeight + kSectionGap;

        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "KEYING");
        y += kSectionHeaderHeight + kRowHeight * 3 + kSectionGap;
    }

    if (isFxOnly)
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "DRY / WET");
        y += kSectionHeaderHeight + kRowHeight + kSectionGap;
    }

    if (is3D)
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "3D CONTROLS");
        y += kSectionHeaderHeight + kRowHeight * 5 + kSectionGap;
    }

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "LAYER EFFECTS");
    y += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "AUTOPILOT DEFAULTS");
    y += kSectionHeaderHeight + kRowHeight * 2 + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "TRANSITION SPEED");
}

void LayerInspector::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    int y = 0;

    // Macros
    macroPanel_.setBounds(area.getX(), y, area.getWidth(), MacroPanel::kPreferredHeight);
    y += MacroPanel::kPreferredHeight + kSectionGap;

    // Hide type-specific controls by default
    blendModeSelector_.setVisible(false);
    keyingModeSelector_.setVisible(false);
    keyThresholdSlider_.setVisible(false);
    keySoftnessSlider_.setVisible(false);
    dryWetSlider_.setVisible(false);
    rotXSlider_.setVisible(false);
    rotYSlider_.setVisible(false);
    rotZSlider_.setVisible(false);
    rotSpeedSlider_.setVisible(false);
    scale3DSlider_.setVisible(false);

    if (!layer_) return;

    bool isTransparent = layer_->type == Layer::Type::Transparent;
    bool isFxOnly = layer_->type == Layer::Type::FXOnly;
    bool is3D = layer_->type == Layer::Type::ThreeD;

    if (isTransparent)
    {
        y += kSectionHeaderHeight;
        blendModeSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        blendModeSelector_.setVisible(true);
        y += kRowHeight + kSectionGap;

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

    if (isFxOnly)
    {
        y += kSectionHeaderHeight;
        dryWetSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        dryWetSlider_.setVisible(true);
        y += kRowHeight + kSectionGap;
    }

    if (is3D)
    {
        y += kSectionHeaderHeight;
        rotXSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        rotXSlider_.setVisible(true);
        y += kRowHeight;
        rotYSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        rotYSlider_.setVisible(true);
        y += kRowHeight;
        rotZSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        rotZSlider_.setVisible(true);
        y += kRowHeight;
        rotSpeedSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        rotSpeedSlider_.setVisible(true);
        y += kRowHeight;
        scale3DSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
        scale3DSlider_.setVisible(true);
        y += kRowHeight + kSectionGap;
    }

    // Layer effects
    y += kSectionHeaderHeight;
    int fxHeight = effectStackView_.getPreferredHeight();
    effectStackView_.setBounds(area.getX(), y, area.getWidth(), fxHeight);
    y += fxHeight + kSectionGap;

    // Autopilot defaults
    y += kSectionHeaderHeight;
    defaultApActionSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    defaultApDurationSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight + kSectionGap;

    // Transition speed
    y += kSectionHeaderHeight;
    transitionSpeedSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
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
    {
        effectStackView_.setEffects(nullptr);
    }
    resized();
    repaint();
}

void LayerInspector::setEffectLibrary(EffectLibrary* lib)
{
    effectStackView_.setEffectLibrary(lib);
}

void LayerInspector::setSignalRegistry(SignalRegistry* reg)
{
    macroPanel_.setSignalRegistry(reg);
    effectStackView_.setSignalRegistry(reg);
}

void LayerInspector::setMacroBank(MacroBank* bank)
{
    macroPanel_.setMacroBank(bank);
}

void LayerInspector::refresh()
{
    if (layer_)
    {
        syncFromLayer();
        effectStackView_.refresh();
        macroPanel_.refresh();
    }
    repaint();
}

int LayerInspector::getPreferredHeight() const
{
    if (!layer_) return 100;

    int h = MacroPanel::kPreferredHeight + kSectionGap;

    if (layer_->type == Layer::Type::Transparent)
        h += kSectionHeaderHeight + kRowHeight + kSectionGap
           + kSectionHeaderHeight + kRowHeight * 3 + kSectionGap;

    if (layer_->type == Layer::Type::FXOnly)
        h += kSectionHeaderHeight + kRowHeight + kSectionGap;

    if (layer_->type == Layer::Type::ThreeD)
        h += kSectionHeaderHeight + kRowHeight * 5 + kSectionGap;

    h += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap;
    h += kSectionHeaderHeight + kRowHeight * 2 + kSectionGap;
    h += kSectionHeaderHeight + kRowHeight + 8;
    return h;
}

void LayerInspector::paintSectionHeader(juce::Graphics& g,
                                         const juce::Rectangle<int>& bounds,
                                         const juce::String& title)
{
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(bounds);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)).boldened());
    g.drawText(title, bounds.withTrimmedLeft(4), juce::Justification::centredLeft, false);
}

void LayerInspector::syncFromLayer()
{
    if (!layer_) return;

    blendModeSelector_.setSelectedId(static_cast<int>(layer_->blendMode) + 1,
                                      juce::dontSendNotification);
    keyingModeSelector_.setSelectedId(static_cast<int>(layer_->keyingMode) + 1,
                                       juce::dontSendNotification);
    keyThresholdSlider_.setValue(static_cast<double>(layer_->keyThreshold),
                                 juce::dontSendNotification);
    keySoftnessSlider_.setValue(static_cast<double>(layer_->keySoftness),
                                juce::dontSendNotification);
    dryWetSlider_.setValue(static_cast<double>(layer_->dryWetMix),
                           juce::dontSendNotification);
    rotXSlider_.setValue(static_cast<double>(layer_->rotationX), juce::dontSendNotification);
    rotYSlider_.setValue(static_cast<double>(layer_->rotationY), juce::dontSendNotification);
    rotZSlider_.setValue(static_cast<double>(layer_->rotationZ), juce::dontSendNotification);
    rotSpeedSlider_.setValue(static_cast<double>(layer_->rotationSpeed),
                              juce::dontSendNotification);
    scale3DSlider_.setValue(static_cast<double>(layer_->scale3D), juce::dontSendNotification);

    float transSpeed = layer_->transitionSpeed < 0.0f ? 0.3f : layer_->transitionSpeed;
    transitionSpeedSlider_.setValue(static_cast<double>(transSpeed), juce::dontSendNotification);
}

void LayerInspector::populateBlendModes()
{
    blendModeSelector_.clear();
    // Standard compositing modes
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
    blendModeSelector_.setSelectedId(2, juce::dontSendNotification); // Additive default
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
