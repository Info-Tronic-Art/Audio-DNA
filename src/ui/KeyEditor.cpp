#include "KeyEditor.h"
#include "ui/LookAndFeel.h"

// === PreviewComponent ===
void KeyEditor::PreviewComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Draw background
    switch (bg)
    {
        case Black:   g.setColour(juce::Colours::black); g.fillRect(bounds); break;
        case Grey:    g.setColour(juce::Colour(128, 128, 128)); g.fillRect(bounds); break;
        case White:   g.setColour(juce::Colours::white); g.fillRect(bounds); break;
        case Checker:
        {
            int checkSize = 8;
            for (int y = 0; y < getHeight(); y += checkSize)
                for (int x = 0; x < getWidth(); x += checkSize)
                {
                    bool light = ((x / checkSize + y / checkSize) % 2) == 0;
                    g.setColour(light ? juce::Colour(200, 200, 200) : juce::Colour(150, 150, 150));
                    g.fillRect(x, y, checkSize, checkSize);
                }
            break;
        }
    }

    // Draw image scaled to fit
    if (previewImage.isValid())
    {
        float imgAspect = static_cast<float>(previewImage.getWidth()) /
                          static_cast<float>(previewImage.getHeight());
        float boundsAspect = bounds.getWidth() / bounds.getHeight();

        float drawW, drawH;
        if (imgAspect > boundsAspect)
        {
            drawW = bounds.getWidth();
            drawH = drawW / imgAspect;
        }
        else
        {
            drawH = bounds.getHeight();
            drawW = drawH * imgAspect;
        }

        float drawX = bounds.getX() + (bounds.getWidth() - drawW) * 0.5f;
        float drawY = bounds.getY() + (bounds.getHeight() - drawH) * 0.5f;

        g.drawImage(previewImage,
                     drawX, drawY, drawW, drawH,
                     0, 0, previewImage.getWidth(), previewImage.getHeight());
    }
    else
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.3f));
        g.drawText("No image", bounds, juce::Justification::centred);
    }

    // Border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawRect(bounds, 1.0f);
}

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

    // Don't let editor steal keyboard focus from MainComponent
    setWantsKeyboardFocus(false);

    // Preview with background buttons
    addAndMakeVisible(preview_);
    addAndMakeVisible(bgBlackBtn_);
    addAndMakeVisible(bgGreyBtn_);
    addAndMakeVisible(bgWhiteBtn_);
    addAndMakeVisible(bgCheckerBtn_);
    auto setupBgBtn = [this](juce::TextButton& btn, PreviewComponent::Background bg) {
        btn.onClick = [this, bg] { preview_.bg = bg; preview_.repaint(); };
    };
    setupBgBtn(bgBlackBtn_, PreviewComponent::Black);
    setupBgBtn(bgGreyBtn_, PreviewComponent::Grey);
    setupBgBtn(bgWhiteBtn_, PreviewComponent::White);
    setupBgBtn(bgCheckerBtn_, PreviewComponent::Checker);

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
            updatePreviewImage();
        }
    };

    // Keying mode
    addAndMakeVisible(keyingLabel_);
    keyingLabel_.setFont(juce::Font(12.0f, juce::Font::bold));
    keyingLabel_.setColour(juce::Label::textColourId,
                            juce::Colour(AudioDNALookAndFeel::kAccentMagenta));

    addAndMakeVisible(keyingModeSelector_);
    keyingModeSelector_.addItem("Alpha", 1);
    keyingModeSelector_.addItem("Luma Key", 2);
    keyingModeSelector_.addItem("Inverted Luma Key", 3);
    keyingModeSelector_.addItem("Luma is Alpha", 4);
    keyingModeSelector_.addItem("Inverted Luma is Alpha", 5);
    keyingModeSelector_.addItem("Chroma Key", 6);
    keyingModeSelector_.addItem("Max RGB", 7);
    keyingModeSelector_.addItem("Saturation Key", 8);
    keyingModeSelector_.addItem("Edge Detection", 9);
    keyingModeSelector_.addItem("Threshold (50%)", 10);
    keyingModeSelector_.addItem("Channel: Red", 11);
    keyingModeSelector_.addItem("Channel: Green", 12);
    keyingModeSelector_.addItem("Channel: Blue", 13);
    keyingModeSelector_.addItem("Vignette (Spotlight)", 14);
    keyingModeSelector_.onChange = [this] {
        if (currentKey_)
        {
            currentKey_->keyingMode =
                static_cast<KeySlot::KeyingMode>(keyingModeSelector_.getSelectedId() - 1);
            updatePreviewImage();
            resized();
        }
    };

    // Blend mode
    addAndMakeVisible(blendLabel_);
    blendLabel_.setFont(juce::Font(12.0f, juce::Font::bold));
    blendLabel_.setColour(juce::Label::textColourId,
                           juce::Colour(AudioDNALookAndFeel::kAccentMagenta));

    addAndMakeVisible(blendModeSelector_);
    blendModeSelector_.addItem("Normal", 1);
    blendModeSelector_.addSeparator();
    blendModeSelector_.addSectionHeading("-- LIGHTEN --");
    blendModeSelector_.addItem("Additive", 2);
    blendModeSelector_.addItem("Screen", 3);
    blendModeSelector_.addItem("Lighten", 9);
    blendModeSelector_.addItem("Color Dodge", 10);
    blendModeSelector_.addSeparator();
    blendModeSelector_.addSectionHeading("-- DARKEN --");
    blendModeSelector_.addItem("Multiply", 4);
    blendModeSelector_.addItem("Darken", 8);
    blendModeSelector_.addItem("Color Burn", 11);
    blendModeSelector_.addSeparator();
    blendModeSelector_.addSectionHeading("-- CONTRAST --");
    blendModeSelector_.addItem("Overlay", 5);
    blendModeSelector_.addItem("Soft Light", 6);
    blendModeSelector_.addItem("Hard Light", 7);
    blendModeSelector_.addItem("Vivid Light", 15);
    blendModeSelector_.addItem("Linear Light", 16);
    blendModeSelector_.addItem("Pin Light", 17);
    blendModeSelector_.addItem("Hard Mix", 18);
    blendModeSelector_.addSeparator();
    blendModeSelector_.addSectionHeading("-- INVERSION --");
    blendModeSelector_.addItem("Difference", 12);
    blendModeSelector_.addItem("Exclusion", 13);
    blendModeSelector_.addItem("Subtract", 14);
    blendModeSelector_.onChange = [this] {
        if (currentKey_)
            currentKey_->blendMode =
                static_cast<KeySlot::BlendMode>(blendModeSelector_.getSelectedId() - 1);
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
        if (currentKey_) { currentKey_->opacity = static_cast<float>(opacitySlider_.getValue()); updatePreviewImage(); }
    };

    setupSlider(thresholdSlider_, thresholdLabel_, "Threshold", 0.0f, 1.0f, 0.1f);
    thresholdSlider_.onValueChange = [this] {
        if (currentKey_) { currentKey_->keyThreshold = static_cast<float>(thresholdSlider_.getValue()); updatePreviewImage(); }
    };

    setupSlider(softnessSlider_, softnessLabel_, "Softness", 0.0f, 0.5f, 0.1f);
    softnessSlider_.onValueChange = [this] {
        if (currentKey_) { currentKey_->keySoftness = static_cast<float>(softnessSlider_.getValue()); updatePreviewImage(); }
    };

    // Chroma key color swatch — click to open color picker popup
    addAndMakeVisible(chromaColorSwatch_);
    addAndMakeVisible(chromaColorLabel_);
    chromaColorLabel_.setColour(juce::Label::textColourId,
                                 juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    chromaColorLabel_.setFont(10.0f);
    chromaColorSwatch_.onColourChanged = [this](juce::Colour col) {
        if (!currentKey_) return;
        currentKey_->chromaKeyR = col.getFloatRed();
        currentKey_->chromaKeyG = col.getFloatGreen();
        currentKey_->chromaKeyB = col.getFloatBlue();
        updatePreviewImage();
    };

    setupSlider(chromaToleranceSlider_, chromaToleranceLabel_, "Tolerance", 0.0f, 1.0f, 0.2f);
    chromaToleranceSlider_.onValueChange = [this] {
        if (currentKey_) { currentKey_->chromaKeyTolerance = static_cast<float>(chromaToleranceSlider_.getValue()); updatePreviewImage(); }
    };

    setupSlider(chromaSoftnessSlider_, chromaSoftnessLabel_, "Softness", 0.0f, 0.5f, 0.1f);
    chromaSoftnessSlider_.onValueChange = [this] {
        if (currentKey_) { currentKey_->chromaKeySoftness = static_cast<float>(chromaSoftnessSlider_.getValue()); updatePreviewImage(); }
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

    // Latch is always infinite — no beat selector needed

    addAndMakeVisible(randomBeatSelector_);
    addAndMakeVisible(randomBeatLabel_);
    randomBeatLabel_.setColour(juce::Label::textColourId,
                                juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    randomBeatLabel_.setFont(10.0f);
    // Specific beat values: 1, 2, 4, 8, 16, 32, 64, 128
    const int beatValues[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
    for (int i = 0; i < 8; ++i)
        randomBeatSelector_.addItem(juce::String(beatValues[i]), i + 1);
    randomBeatSelector_.onChange = [this] {
        if (currentKey_)
        {
            const int vals[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
            int sel = randomBeatSelector_.getSelectedId();
            if (sel >= 1 && sel <= 8)
                currentKey_->randomBeatDuration = vals[sel - 1];
        }
    };

    // Effects
    addAndMakeVisible(effectsLabel_);
    effectsLabel_.setFont(juce::Font(12.0f, juce::Font::bold));
    effectsLabel_.setColour(juce::Label::textColourId,
                             juce::Colour(AudioDNALookAndFeel::kAccentMagenta));

    addAndMakeVisible(addEffectSelector_);
    addEffectSelector_.addItem("Select effect...", 1);
    {
        // Build categorized menu
        const juce::String categories[] = {
            "3d", "warp", "color", "glitch", "pattern", "animation", "blend", "blur"
        };
        const juce::String categoryLabels[] = {
            "-- 3D/DEPTH --", "-- WARP --", "-- COLOR --", "-- GLITCH --",
            "-- PATTERN --", "-- ANIMATION --", "-- BLEND --", "-- BLUR --"
        };
        int itemId = 2;
        for (int c = 0; c < 8; ++c)
        {
            auto catNames = effectLibrary_.getEffectsByCategory(categories[c]);
            if (catNames.isEmpty()) continue;
            // Add category separator (disabled item)
            addEffectSelector_.addSeparator();
            addEffectSelector_.addSectionHeading(categoryLabels[c]);
            for (int i = 0; i < catNames.size(); ++i)
                addEffectSelector_.addItem(catNames[i], itemId++);
        }
    }

    // Auto-add effect on selection (no separate button needed)
    addEffectSelector_.onChange = [this] {
        if (!currentKey_) return;
        int sel = addEffectSelector_.getSelectedId();
        if (sel <= 1) return;
        juce::String fxName = addEffectSelector_.getText();
        if (fxName.isEmpty()) return;

        KeySlot::EffectSlot slot;
        slot.effectName = fxName.toStdString();
        auto* def = effectLibrary_.getEffectDef(fxName);
        if (def)
            slot.params.resize(def->params.size(), 0.0f);
        currentKey_->effects.push_back(std::move(slot));
        addEffectSelector_.setSelectedId(1, juce::dontSendNotification);
        rebuildEffectRows();
        resized();
    };

    // Keep addEffectBtn_ for "Clear FX" only
    addEffectBtn_.setVisible(false);

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
    updatePreviewImage();
    rebuildEffectRows();
    resized();
}

void KeyEditor::updatePreviewImage()
{
    if (!currentKey_ || currentKey_->mediaType != KeySlot::MediaType::Image ||
        !currentKey_->mediaFile.existsAsFile())
    {
        preview_.previewImage = juce::Image();
        preview_.repaint();
        return;
    }

    auto src = juce::ImageFileFormat::loadFrom(currentKey_->mediaFile);
    if (!src.isValid())
    {
        preview_.previewImage = juce::Image();
        preview_.repaint();
        return;
    }

    // Apply keying mode in software to show transparency in preview
    auto img = src.convertedToFormat(juce::Image::ARGB);
    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readWrite);

    for (int y = 0; y < img.getHeight(); ++y)
    {
        for (int x = 0; x < img.getWidth(); ++x)
        {
            auto pixel = bmp.getPixelColour(x, y);
            float r = pixel.getFloatRed();
            float g = pixel.getFloatGreen();
            float b = pixel.getFloatBlue();
            float luma = 0.2126f * r + 0.7152f * g + 0.0722f * b;
            float alpha = 1.0f;

            switch (currentKey_->keyingMode)
            {
                case KeySlot::KeyingMode::Alpha:
                    alpha = pixel.getFloatAlpha();
                    break;
                case KeySlot::KeyingMode::LumaKey:
                {
                    float t = currentKey_->keyThreshold;
                    float s = currentKey_->keySoftness;
                    alpha = std::clamp((luma - (t - s)) / (2.0f * s + 0.001f), 0.0f, 1.0f);
                    break;
                }
                case KeySlot::KeyingMode::InvertedLumaKey:
                {
                    float t = currentKey_->keyThreshold;
                    float s = currentKey_->keySoftness;
                    alpha = 1.0f - std::clamp((luma - (t - s)) / (2.0f * s + 0.001f), 0.0f, 1.0f);
                    break;
                }
                case KeySlot::KeyingMode::LumaIsAlpha:
                    alpha = luma;
                    break;
                case KeySlot::KeyingMode::InvertedLumaIsAlpha:
                    alpha = 1.0f - luma;
                    break;
                case KeySlot::KeyingMode::ChromaKey:
                {
                    float dr = r - currentKey_->chromaKeyR;
                    float dg = g - currentKey_->chromaKeyG;
                    float db = b - currentKey_->chromaKeyB;
                    float dist = std::sqrt(dr * dr + dg * dg + db * db);
                    float t = currentKey_->chromaKeyTolerance;
                    float s = currentKey_->chromaKeySoftness;
                    alpha = std::clamp((dist - (t - s)) / (2.0f * s + 0.001f), 0.0f, 1.0f);
                    break;
                }
                case KeySlot::KeyingMode::MaxRGB:
                    alpha = std::max(r, std::max(g, b));
                    break;
                case KeySlot::KeyingMode::SaturationKey:
                {
                    float sat = std::max(r, std::max(g, b)) - std::min(r, std::min(g, b));
                    float t = currentKey_->keyThreshold;
                    float s = currentKey_->keySoftness;
                    alpha = std::clamp((sat - (t - s)) / (2.0f * s + 0.001f), 0.0f, 1.0f);
                    break;
                }
                case KeySlot::KeyingMode::ThresholdMask:
                    alpha = luma >= 0.5f ? 1.0f : 0.0f;
                    break;
                case KeySlot::KeyingMode::ChannelR:
                    alpha = r;
                    break;
                case KeySlot::KeyingMode::ChannelG:
                    alpha = g;
                    break;
                case KeySlot::KeyingMode::ChannelB:
                    alpha = b;
                    break;
                case KeySlot::KeyingMode::VignetteAlpha:
                {
                    float cx = static_cast<float>(x) / static_cast<float>(img.getWidth()) - 0.5f;
                    float cy = static_cast<float>(y) / static_cast<float>(img.getHeight()) - 0.5f;
                    float dist = std::sqrt(cx * cx + cy * cy);
                    alpha = 1.0f - std::clamp((dist - currentKey_->vignetteInner) /
                            (currentKey_->vignetteOuter - currentKey_->vignetteInner + 0.001f), 0.0f, 1.0f);
                    break;
                }
                default:
                    break;
            }

            alpha *= currentKey_->opacity;
            bmp.setPixelColour(x, y, pixel.withAlpha(alpha));
        }
    }

    preview_.previewImage = img;
    preview_.repaint();
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

    // Keying + Blend
    keyingModeSelector_.setSelectedId(
        static_cast<int>(currentKey_->keyingMode) + 1, juce::dontSendNotification);
    blendModeSelector_.setSelectedId(
        static_cast<int>(currentKey_->blendMode) + 1, juce::dontSendNotification);
    opacitySlider_.setValue(currentKey_->opacity, juce::dontSendNotification);
    thresholdSlider_.setValue(currentKey_->keyThreshold, juce::dontSendNotification);
    softnessSlider_.setValue(currentKey_->keySoftness, juce::dontSendNotification);
    chromaToleranceSlider_.setValue(currentKey_->chromaKeyTolerance, juce::dontSendNotification);
    chromaSoftnessSlider_.setValue(currentKey_->chromaKeySoftness, juce::dontSendNotification);

    // Set chroma key color swatch
    chromaColorSwatch_.colour = juce::Colour::fromFloatRGBA(
        currentKey_->chromaKeyR, currentKey_->chromaKeyG,
        currentKey_->chromaKeyB, 1.0f);
    chromaColorSwatch_.repaint();

    // Playback
    latchToggle_.setToggleState(currentKey_->latched, juce::dontSendNotification);
    ignoreRandomToggle_.setToggleState(currentKey_->ignoreRandom, juce::dontSendNotification);
    // Map randomBeatDuration to selector index
    const int vals[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
    int selIdx = 1; // default to 1 beat
    for (int i = 0; i < 8; ++i)
        if (vals[i] == currentKey_->randomBeatDuration) { selIdx = i + 1; break; }
    randomBeatSelector_.setSelectedId(selIdx, juce::dontSendNotification);
}

void KeyEditor::rebuildEffectRows()
{
    for (auto& row : effectRows_)
    {
        removeChildComponent(row.enableBtn.get());
        removeChildComponent(row.nameLabel.get());
        removeChildComponent(row.removeBtn.get());
        for (auto& pr : row.paramRows)
        {
            removeChildComponent(pr.label.get());
            removeChildComponent(pr.slider.get());
        }
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
                                  juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        row.nameLabel->setFont(juce::Font(11.0f, juce::Font::bold));
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

        // Add parameter sliders
        auto* def = effectLibrary_.getEffectDef(juce::String(currentKey_->effects[i].effectName));
        if (def)
        {
            for (size_t p = 0; p < def->params.size() && p < currentKey_->effects[i].params.size(); ++p)
            {
                EffectRow::ParamRow pr;
                pr.label = std::make_unique<juce::Label>("", juce::String(def->params[p].name));
                pr.label->setColour(juce::Label::textColourId,
                                     juce::Colour(AudioDNALookAndFeel::kTextSecondary));
                pr.label->setFont(9.0f);
                addAndMakeVisible(pr.label.get());

                pr.slider = std::make_unique<juce::Slider>();
                pr.slider->setRange(0.0, 1.0, 0.01);
                pr.slider->setValue(currentKey_->effects[i].params[p], juce::dontSendNotification);
                pr.slider->setSliderStyle(juce::Slider::LinearHorizontal);
                pr.slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 35, 14);
                pr.slider->onValueChange = [this, i, p] {
                    if (currentKey_ && i < currentKey_->effects.size() &&
                        p < currentKey_->effects[i].params.size())
                    {
                        currentKey_->effects[i].params[p] =
                            static_cast<float>(effectRows_[i].paramRows[p].slider->getValue());
                    }
                };
                addAndMakeVisible(pr.slider.get());

                row.paramRows.push_back(std::move(pr));
            }
        }

        effectRows_.push_back(std::move(row));
    }
}

bool KeyEditor::keyPressed(const juce::KeyPress& key)
{
    // Escape closes editor
    if (key.isKeyCode(juce::KeyPress::escapeKey))
    {
        if (onClose) onClose();
        return true;
    }
    // Let parent handle everything else (shift+key for latch, etc.)
    return false;
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

    // Preview panel on the right
    auto previewArea = area.removeFromRight(std::min(220, area.getWidth() / 3));
    previewArea.removeFromLeft(8);
    auto bgRow = previewArea.removeFromTop(20);
    bgBlackBtn_.setBounds(bgRow.removeFromLeft(bgRow.getWidth() / 4));
    bgGreyBtn_.setBounds(bgRow.removeFromLeft(bgRow.getWidth() / 3));
    bgWhiteBtn_.setBounds(bgRow.removeFromLeft(bgRow.getWidth() / 2));
    bgCheckerBtn_.setBounds(bgRow);
    previewArea.removeFromTop(2);
    preview_.setBounds(previewArea);

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

    // Keying + Blend section
    keyingLabel_.setBounds(area.removeFromTop(18));
    area.removeFromTop(2);
    auto keyRow1 = area.removeFromTop(22);
    keyingModeSelector_.setBounds(keyRow1.removeFromLeft(150));
    keyRow1.removeFromLeft(8);
    opacityLabel_.setBounds(keyRow1.removeFromLeft(40));
    opacitySlider_.setBounds(keyRow1.removeFromLeft(150));
    area.removeFromTop(2);

    auto blendRow = area.removeFromTop(22);
    blendLabel_.setBounds(blendRow.removeFromLeft(35));
    blendModeSelector_.setBounds(blendRow.removeFromLeft(130));
    area.removeFromTop(2);

    // Mode-specific keying controls
    auto kMode = currentKey_ ? currentKey_->keyingMode : KeySlot::KeyingMode::Alpha;
    bool showThreshold = (kMode == KeySlot::KeyingMode::LumaKey ||
                          kMode == KeySlot::KeyingMode::InvertedLumaKey ||
                          kMode == KeySlot::KeyingMode::SaturationKey ||
                          kMode == KeySlot::KeyingMode::EdgeDetection);
    bool showChroma = (kMode == KeySlot::KeyingMode::ChromaKey);

    thresholdLabel_.setVisible(showThreshold);
    thresholdSlider_.setVisible(showThreshold);
    softnessLabel_.setVisible(showThreshold);
    softnessSlider_.setVisible(showThreshold);
    chromaColorSwatch_.setVisible(showChroma);
    chromaColorLabel_.setVisible(showChroma);
    chromaToleranceLabel_.setVisible(showChroma);
    chromaToleranceSlider_.setVisible(showChroma);
    chromaSoftnessLabel_.setVisible(showChroma);
    chromaSoftnessSlider_.setVisible(showChroma);

    if (showThreshold)
    {
        auto thRow = area.removeFromTop(22);
        thresholdLabel_.setBounds(thRow.removeFromLeft(55));
        thresholdSlider_.setBounds(thRow.removeFromLeft(150));
        thRow.removeFromLeft(8);
        softnessLabel_.setBounds(thRow.removeFromLeft(45));
        softnessSlider_.setBounds(thRow.removeFromLeft(150));
        area.removeFromTop(2);
    }
    else if (showChroma)
    {
        auto chromaRow1 = area.removeFromTop(24);
        chromaColorLabel_.setBounds(chromaRow1.removeFromLeft(55));
        chromaColorSwatch_.setBounds(chromaRow1.removeFromLeft(40).reduced(0, 2));
        area.removeFromTop(2);
        auto chromaRow2 = area.removeFromTop(22);
        chromaToleranceLabel_.setBounds(chromaRow2.removeFromLeft(55));
        chromaToleranceSlider_.setBounds(chromaRow2.removeFromLeft(150));
        chromaRow2.removeFromLeft(8);
        chromaSoftnessLabel_.setBounds(chromaRow2.removeFromLeft(45));
        chromaSoftnessSlider_.setBounds(chromaRow2.removeFromLeft(150));
        area.removeFromTop(2);
    }

    area.removeFromTop(4);

    // Playback section
    playbackLabel_.setBounds(area.removeFromTop(18));
    area.removeFromTop(2);
    auto playRow1 = area.removeFromTop(22);
    latchToggle_.setBounds(playRow1.removeFromLeft(70));
    playRow1.removeFromLeft(12);
    ignoreRandomToggle_.setBounds(playRow1.removeFromLeft(130));
    playRow1.removeFromLeft(12);
    randomBeatLabel_.setBounds(playRow1.removeFromLeft(80));
    randomBeatSelector_.setBounds(playRow1.removeFromLeft(70));

    area.removeFromTop(6);

    // Effects section
    effectsLabel_.setBounds(area.removeFromTop(18));
    area.removeFromTop(2);
    auto addRow = area.removeFromTop(22);
    addEffectSelector_.setBounds(addRow.removeFromLeft(220));
    addRow.removeFromLeft(4);
    clearEffectsBtn_.setBounds(addRow.removeFromLeft(60));

    area.removeFromTop(4);

    // Effect rows with parameter sliders
    for (auto& row : effectRows_)
    {
        auto fxRow = area.removeFromTop(18);
        row.enableBtn->setBounds(fxRow.removeFromLeft(20));
        fxRow.removeFromLeft(2);
        row.removeBtn->setBounds(fxRow.removeFromRight(20));
        fxRow.removeFromRight(2);
        row.nameLabel->setBounds(fxRow);
        area.removeFromTop(1);

        // Parameter sliders
        for (auto& pr : row.paramRows)
        {
            auto paramRow = area.removeFromTop(16);
            paramRow.removeFromLeft(24); // indent
            pr.label->setBounds(paramRow.removeFromLeft(60));
            pr.slider->setBounds(paramRow);
            area.removeFromTop(1);
        }
        area.removeFromTop(2);
    }
}
