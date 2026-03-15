#include "KeyEditor.h"
#include "ui/LookAndFeel.h"

KeyEditor::KeyEditor(EffectLibrary& effectLibrary)
    : effectLibrary_(effectLibrary)
{
    // Title
    addAndMakeVisible(titleLabel_);
    titleLabel_.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId,
                          juce::Colour(AudioDNALookAndFeel::kAccentCyan));

    addAndMakeVisible(closeButton_);
    closeButton_.onClick = [this] { if (onClose) onClose(); };

    // Media
    addAndMakeVisible(openImageBtn_);
    addAndMakeVisible(clearMediaBtn_);
    addAndMakeVisible(mediaInfoLabel_);
    mediaInfoLabel_.setColour(juce::Label::textColourId,
                              juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    openImageBtn_.onClick = [this] {
        if (currentKey_ && onRequestImage)
            onRequestImage(*currentKey_);
    };
    clearMediaBtn_.onClick = [this] {
        if (currentKey_)
        {
            currentKey_->mediaType = KeySlot::MediaType::None;
            currentKey_->mediaFile = juce::File();
            refreshFromKey();
        }
    };

    // Transparency mode
    addAndMakeVisible(transparencyLabel_);
    transparencyLabel_.setFont(juce::Font(12.0f, juce::Font::bold));
    transparencyLabel_.setColour(juce::Label::textColourId,
                                  juce::Colour(AudioDNALookAndFeel::kAccentMagenta));

    addAndMakeVisible(transparencyModeSelector_);
    transparencyModeSelector_.addItem("Alpha", 1);
    transparencyModeSelector_.addItem("Luma Key", 2);
    transparencyModeSelector_.addItem("Chroma Key", 3);
    transparencyModeSelector_.addItem("Light", 4);
    transparencyModeSelector_.onChange = [this] {
        if (currentKey_)
        {
            currentKey_->transparencyMode =
                static_cast<KeySlot::TransparencyMode>(transparencyModeSelector_.getSelectedId() - 1);
            resized(); // Show/hide mode-specific controls
        }
    };

    // Opacity slider
    auto setupSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& name,
                              float min, float max, float def) {
        addAndMakeVisible(s);
        addAndMakeVisible(l);
        l.setText(name, juce::dontSendNotification);
        l.setColour(juce::Label::textColourId,
                     juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        l.setFont(10.0f);
        s.setRange(static_cast<double>(min), static_cast<double>(max), 0.01);
        s.setValue(static_cast<double>(def), juce::dontSendNotification);
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 16);
    };

    setupSlider(opacitySlider_, opacityLabel_, "Opacity", 0.0f, 1.0f, 1.0f);
    opacitySlider_.onValueChange = [this] {
        if (currentKey_) currentKey_->opacity = static_cast<float>(opacitySlider_.getValue());
    };

    setupSlider(lumaThresholdSlider_, lumaThresholdLabel_, "Threshold", 0.0f, 1.0f, 0.1f);
    lumaThresholdSlider_.onValueChange = [this] {
        if (currentKey_) currentKey_->lumaKeyThreshold = static_cast<float>(lumaThresholdSlider_.getValue());
    };

    setupSlider(lumaSoftnessSlider_, lumaSoftnessLabel_, "Softness", 0.0f, 0.5f, 0.05f);
    lumaSoftnessSlider_.onValueChange = [this] {
        if (currentKey_) currentKey_->lumaKeySoftness = static_cast<float>(lumaSoftnessSlider_.getValue());
    };

    setupSlider(chromaToleranceSlider_, chromaToleranceLabel_, "Tolerance", 0.0f, 1.0f, 0.2f);
    chromaToleranceSlider_.onValueChange = [this] {
        if (currentKey_) currentKey_->chromaKeyTolerance = static_cast<float>(chromaToleranceSlider_.getValue());
    };

    setupSlider(chromaSoftnessSlider_, chromaSoftnessLabel_, "Softness", 0.0f, 0.5f, 0.1f);
    chromaSoftnessSlider_.onValueChange = [this] {
        if (currentKey_) currentKey_->chromaKeySoftness = static_cast<float>(chromaSoftnessSlider_.getValue());
    };

    // Playback
    addAndMakeVisible(playbackLabel_);
    playbackLabel_.setFont(juce::Font(12.0f, juce::Font::bold));
    playbackLabel_.setColour(juce::Label::textColourId,
                              juce::Colour(AudioDNALookAndFeel::kAccentMagenta));

    addAndMakeVisible(latchToggle_);
    latchToggle_.onClick = [this] {
        if (currentKey_) currentKey_->latched = latchToggle_.getToggleState();
    };

    addAndMakeVisible(ignoreRandomToggle_);
    ignoreRandomToggle_.onClick = [this] {
        if (currentKey_) currentKey_->ignoreRandom = ignoreRandomToggle_.getToggleState();
    };

    addAndMakeVisible(latchBeatSelector_);
    addAndMakeVisible(latchBeatLabel_);
    latchBeatLabel_.setColour(juce::Label::textColourId,
                               juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    latchBeatLabel_.setFont(10.0f);
    latchBeatSelector_.addItem("Infinite", 1);
    for (int i = 1; i <= 32; ++i)
        latchBeatSelector_.addItem(juce::String(i), i + 1);
    latchBeatSelector_.onChange = [this] {
        if (currentKey_)
            currentKey_->latchBeatDuration = latchBeatSelector_.getSelectedId() - 1;
    };

    addAndMakeVisible(randomBeatSelector_);
    addAndMakeVisible(randomBeatLabel_);
    randomBeatLabel_.setColour(juce::Label::textColourId,
                                juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    randomBeatLabel_.setFont(10.0f);
    for (int i = 1; i <= 32; ++i)
        randomBeatSelector_.addItem(juce::String(i), i);
    randomBeatSelector_.onChange = [this] {
        if (currentKey_)
            currentKey_->randomBeatDuration = randomBeatSelector_.getSelectedId();
    };

    // Effects
    addAndMakeVisible(effectsLabel_);
    effectsLabel_.setFont(juce::Font(12.0f, juce::Font::bold));
    effectsLabel_.setColour(juce::Label::textColourId,
                             juce::Colour(AudioDNALookAndFeel::kAccentMagenta));

    addAndMakeVisible(addEffectSelector_);
    addEffectSelector_.addItem("Select effect...", 1);
    auto names = effectLibrary_.getEffectNames();
    for (int i = 0; i < names.size(); ++i)
        addEffectSelector_.addItem(names[i], i + 2);

    addAndMakeVisible(addEffectBtn_);
    addEffectBtn_.onClick = [this] {
        if (!currentKey_) return;
        int sel = addEffectSelector_.getSelectedId();
        if (sel <= 1) return;
        auto allNames = effectLibrary_.getEffectNames();
        int idx = sel - 2;
        if (idx >= allNames.size()) return;

        KeySlot::EffectSlot slot;
        slot.effectName = allNames[idx].toStdString();
        // Initialize params from library default count
        auto* def = effectLibrary_.getEffectDef(allNames[idx]);
        if (def)
            slot.params.resize(def->params.size(), 0.0f);
        currentKey_->effects.push_back(std::move(slot));
        addEffectSelector_.setSelectedId(1, juce::dontSendNotification);
        rebuildEffectRows();
        resized();
    };

    addAndMakeVisible(clearEffectsBtn_);
    clearEffectsBtn_.onClick = [this] {
        if (currentKey_)
        {
            currentKey_->effects.clear();
            rebuildEffectRows();
            resized();
        }
    };
}

void KeyEditor::setKey(KeySlot* key)
{
    currentKey_ = key;
    refreshFromKey();
    rebuildEffectRows();
    resized();
}

void KeyEditor::refreshFromKey()
{
    if (!currentKey_)
    {
        titleLabel_.setText("No Key Selected", juce::dontSendNotification);
        return;
    }

    titleLabel_.setText("Key: " + juce::String::charToString(currentKey_->keyChar),
                        juce::dontSendNotification);

    // Media info
    switch (currentKey_->mediaType)
    {
        case KeySlot::MediaType::None:
            mediaInfoLabel_.setText("No media", juce::dontSendNotification); break;
        case KeySlot::MediaType::Image:
            mediaInfoLabel_.setText("IMG: " + currentKey_->mediaFile.getFileNameWithoutExtension(),
                                    juce::dontSendNotification); break;
        case KeySlot::MediaType::VideoFile:
            mediaInfoLabel_.setText("VID: " + currentKey_->mediaFile.getFileNameWithoutExtension(),
                                    juce::dontSendNotification); break;
        case KeySlot::MediaType::Camera:
            mediaInfoLabel_.setText("Camera", juce::dontSendNotification); break;
    }

    // Transparency
    transparencyModeSelector_.setSelectedId(
        static_cast<int>(currentKey_->transparencyMode) + 1, juce::dontSendNotification);
    opacitySlider_.setValue(currentKey_->opacity, juce::dontSendNotification);
    lumaThresholdSlider_.setValue(currentKey_->lumaKeyThreshold, juce::dontSendNotification);
    lumaSoftnessSlider_.setValue(currentKey_->lumaKeySoftness, juce::dontSendNotification);
    chromaToleranceSlider_.setValue(currentKey_->chromaKeyTolerance, juce::dontSendNotification);
    chromaSoftnessSlider_.setValue(currentKey_->chromaKeySoftness, juce::dontSendNotification);

    // Playback
    latchToggle_.setToggleState(currentKey_->latched, juce::dontSendNotification);
    ignoreRandomToggle_.setToggleState(currentKey_->ignoreRandom, juce::dontSendNotification);
    latchBeatSelector_.setSelectedId(currentKey_->latchBeatDuration + 1, juce::dontSendNotification);
    randomBeatSelector_.setSelectedId(currentKey_->randomBeatDuration, juce::dontSendNotification);
}

void KeyEditor::rebuildEffectRows()
{
    for (auto& row : effectRows_)
    {
        removeChildComponent(row.enableBtn.get());
        removeChildComponent(row.nameLabel.get());
        removeChildComponent(row.removeBtn.get());
    }
    effectRows_.clear();

    if (!currentKey_) return;

    for (size_t i = 0; i < currentKey_->effects.size(); ++i)
    {
        EffectRow row;
        row.enableBtn = std::make_unique<juce::ToggleButton>();
        row.enableBtn->setToggleState(currentKey_->effects[i].enabled, juce::dontSendNotification);
        row.enableBtn->onClick = [this, i] {
            if (currentKey_ && i < currentKey_->effects.size())
                currentKey_->effects[i].enabled = effectRows_[i].enableBtn->getToggleState();
        };
        addAndMakeVisible(row.enableBtn.get());

        row.nameLabel = std::make_unique<juce::Label>("", juce::String(currentKey_->effects[i].effectName));
        row.nameLabel->setColour(juce::Label::textColourId,
                                  juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        row.nameLabel->setFont(11.0f);
        addAndMakeVisible(row.nameLabel.get());

        row.removeBtn = std::make_unique<juce::TextButton>("X");
        row.removeBtn->onClick = [this, i] {
            if (currentKey_ && i < currentKey_->effects.size())
            {
                currentKey_->effects.erase(currentKey_->effects.begin() + static_cast<long>(i));
                rebuildEffectRows();
                resized();
            }
        };
        addAndMakeVisible(row.removeBtn.get());

        effectRows_.push_back(std::move(row));
    }
}

void KeyEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1a1a2e));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
}

void KeyEditor::resized()
{
    auto area = getLocalBounds().reduced(8);

    // Header
    auto header = area.removeFromTop(24);
    titleLabel_.setBounds(header.removeFromLeft(200));
    closeButton_.setBounds(header.removeFromRight(24));
    area.removeFromTop(6);

    // Media
    auto mediaRow = area.removeFromTop(24);
    openImageBtn_.setBounds(mediaRow.removeFromLeft(80));
    mediaRow.removeFromLeft(4);
    clearMediaBtn_.setBounds(mediaRow.removeFromLeft(50));
    mediaRow.removeFromLeft(8);
    mediaInfoLabel_.setBounds(mediaRow);
    area.removeFromTop(6);

    // Transparency section
    transparencyLabel_.setBounds(area.removeFromTop(18));
    area.removeFromTop(2);
    auto transRow1 = area.removeFromTop(22);
    transparencyModeSelector_.setBounds(transRow1.removeFromLeft(120));
    transRow1.removeFromLeft(8);
    opacityLabel_.setBounds(transRow1.removeFromLeft(40));
    opacitySlider_.setBounds(transRow1.removeFromLeft(150));

    area.removeFromTop(2);

    // Mode-specific controls
    auto mode = currentKey_ ? currentKey_->transparencyMode : KeySlot::TransparencyMode::Alpha;
    bool showLuma = (mode == KeySlot::TransparencyMode::LumaKey);
    bool showChroma = (mode == KeySlot::TransparencyMode::ChromaKey);

    lumaThresholdLabel_.setVisible(showLuma);
    lumaThresholdSlider_.setVisible(showLuma);
    lumaSoftnessLabel_.setVisible(showLuma);
    lumaSoftnessSlider_.setVisible(showLuma);
    chromaToleranceLabel_.setVisible(showChroma);
    chromaToleranceSlider_.setVisible(showChroma);
    chromaSoftnessLabel_.setVisible(showChroma);
    chromaSoftnessSlider_.setVisible(showChroma);

    if (showLuma)
    {
        auto lumaRow = area.removeFromTop(22);
        lumaThresholdLabel_.setBounds(lumaRow.removeFromLeft(55));
        lumaThresholdSlider_.setBounds(lumaRow.removeFromLeft(150));
        lumaRow.removeFromLeft(8);
        lumaSoftnessLabel_.setBounds(lumaRow.removeFromLeft(45));
        lumaSoftnessSlider_.setBounds(lumaRow.removeFromLeft(150));
        area.removeFromTop(2);
    }
    else if (showChroma)
    {
        auto chromaRow = area.removeFromTop(22);
        chromaToleranceLabel_.setBounds(chromaRow.removeFromLeft(55));
        chromaToleranceSlider_.setBounds(chromaRow.removeFromLeft(150));
        chromaRow.removeFromLeft(8);
        chromaSoftnessLabel_.setBounds(chromaRow.removeFromLeft(45));
        chromaSoftnessSlider_.setBounds(chromaRow.removeFromLeft(150));
        area.removeFromTop(2);
    }

    area.removeFromTop(4);

    // Playback section
    playbackLabel_.setBounds(area.removeFromTop(18));
    area.removeFromTop(2);
    auto playRow1 = area.removeFromTop(22);
    latchToggle_.setBounds(playRow1.removeFromLeft(70));
    playRow1.removeFromLeft(4);
    latchBeatLabel_.setBounds(playRow1.removeFromLeft(60));
    latchBeatSelector_.setBounds(playRow1.removeFromLeft(70));
    playRow1.removeFromLeft(12);
    ignoreRandomToggle_.setBounds(playRow1.removeFromLeft(100));
    playRow1.removeFromLeft(4);
    randomBeatLabel_.setBounds(playRow1.removeFromLeft(70));
    randomBeatSelector_.setBounds(playRow1.removeFromLeft(60));

    area.removeFromTop(6);

    // Effects section
    effectsLabel_.setBounds(area.removeFromTop(18));
    area.removeFromTop(2);
    auto addRow = area.removeFromTop(22);
    addEffectSelector_.setBounds(addRow.removeFromLeft(160));
    addRow.removeFromLeft(4);
    addEffectBtn_.setBounds(addRow.removeFromLeft(50));
    addRow.removeFromLeft(4);
    clearEffectsBtn_.setBounds(addRow.removeFromLeft(60));

    area.removeFromTop(4);

    // Effect rows
    for (auto& row : effectRows_)
    {
        auto fxRow = area.removeFromTop(20);
        row.enableBtn->setBounds(fxRow.removeFromLeft(22));
        fxRow.removeFromLeft(2);
        row.removeBtn->setBounds(fxRow.removeFromRight(22));
        fxRow.removeFromRight(2);
        row.nameLabel->setBounds(fxRow);
        area.removeFromTop(2);
    }
}
