#include "KeyEditor.h"
#include "ui/LookAndFeel.h"
#include <cmath>

// === ColorSwatch ===
void KeyEditor::ColorSwatch::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(colour);
    g.fillRoundedRectangle(b, 3.0f);
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.drawRoundedRectangle(b, 3.0f, 1.0f);
}

void KeyEditor::ColorSwatch::mouseDown(const juce::MouseEvent&)
{
    auto* cs = new juce::ColourSelector(
        juce::ColourSelector::showColourAtTop
        | juce::ColourSelector::showSliders
        | juce::ColourSelector::showColourspace);
    cs->setSize(300, 300);
    cs->setCurrentColour(colour);
    cs->addChangeListener(this);
    juce::CallOutBox::launchAsynchronously(
        std::unique_ptr<juce::Component>(cs), getScreenBounds(), nullptr);
}

void KeyEditor::ColorSwatch::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (auto* cs = dynamic_cast<juce::ColourSelector*>(source))
    {
        colour = cs->getCurrentColour();
        repaint();
        if (onColourChanged) onColourChanged(colour);
    }
}

// === PreviewComponent ===
void KeyEditor::PreviewComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    switch (bg)
    {
        case Black: g.setColour(juce::Colours::black); g.fillRect(bounds); break;
        case Grey:  g.setColour(juce::Colour(128, 128, 128)); g.fillRect(bounds); break;
        case White: g.setColour(juce::Colours::white); g.fillRect(bounds); break;
        case Checker:
            for (int y = 0; y < getHeight(); y += 8)
                for (int x = 0; x < getWidth(); x += 8)
                {
                    g.setColour(((x / 8 + y / 8) % 2 == 0)
                        ? juce::Colour(200, 200, 200) : juce::Colour(150, 150, 150));
                    g.fillRect(x, y, 8, 8);
                }
            break;
    }
    if (previewImage.isValid())
    {
        float imgA = static_cast<float>(previewImage.getWidth()) / static_cast<float>(previewImage.getHeight());
        float bA = bounds.getWidth() / bounds.getHeight();
        float dW, dH;
        if (imgA > bA) { dW = bounds.getWidth(); dH = dW / imgA; }
        else { dH = bounds.getHeight(); dW = dH * imgA; }
        float dX = bounds.getX() + (bounds.getWidth() - dW) * 0.5f;
        float dY = bounds.getY() + (bounds.getHeight() - dH) * 0.5f;
        g.drawImage(previewImage, dX, dY, dW, dH, 0, 0, previewImage.getWidth(), previewImage.getHeight());
    }
    else
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.3f));
        g.drawText("No image", bounds, juce::Justification::centred);
    }
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawRect(bounds, 1.0f);
}

// === KeyEditor Constructor ===
KeyEditor::KeyEditor(EffectLibrary& effectLibrary)
    : effectLibrary_(effectLibrary)
{
    setWantsKeyboardFocus(false);

    addAndMakeVisible(titleLabel_);
    titleLabel_.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kAccentCyan));

    addAndMakeVisible(closeButton_);
    closeButton_.onClick = [this] { if (onClose) onClose(); };

    // Preview
    addAndMakeVisible(preview_);
    addAndMakeVisible(bgBlackBtn_); addAndMakeVisible(bgGreyBtn_);
    addAndMakeVisible(bgWhiteBtn_); addAndMakeVisible(bgCheckerBtn_);
    bgBlackBtn_.onClick = [this] { preview_.bg = PreviewComponent::Black; preview_.repaint(); };
    bgGreyBtn_.onClick = [this] { preview_.bg = PreviewComponent::Grey; preview_.repaint(); };
    bgWhiteBtn_.onClick = [this] { preview_.bg = PreviewComponent::White; preview_.repaint(); };
    bgCheckerBtn_.onClick = [this] { preview_.bg = PreviewComponent::Checker; preview_.repaint(); };

    // Media
    addAndMakeVisible(openImageBtn_); addAndMakeVisible(clearMediaBtn_); addAndMakeVisible(mediaInfoLabel_);
    mediaInfoLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    openImageBtn_.onClick = [this] { if (currentKey_ && onRequestImage) onRequestImage(*currentKey_); };
    clearMediaBtn_.onClick = [this] {
        if (currentKey_) { currentKey_->mediaType = KeySlot::MediaType::None; currentKey_->mediaFile = juce::File(); refreshFromKey(); updatePreviewImage(); }
    };

    // Keying
    auto setupLabel = [](juce::Label& l, const juce::String& text, bool bold = false) {
        l.setText(text, juce::dontSendNotification);
        l.setFont(bold ? juce::Font(12.0f, juce::Font::bold) : juce::Font(10.0f));
        l.setColour(juce::Label::textColourId, bold
            ? juce::Colour(AudioDNALookAndFeel::kAccentMagenta)
            : juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    };

    addAndMakeVisible(keyingLabel_); setupLabel(keyingLabel_, "Keying", true);
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
        if (currentKey_) { currentKey_->keyingMode = static_cast<KeySlot::KeyingMode>(keyingModeSelector_.getSelectedId() - 1); updatePreviewImage(); resized(); }
    };

    addAndMakeVisible(blendLabel_); setupLabel(blendLabel_, "Blend", true);
    addAndMakeVisible(blendModeSelector_);
    blendModeSelector_.addItem("Normal", 1);
    blendModeSelector_.addSeparator(); blendModeSelector_.addSectionHeading("-- LIGHTEN --");
    blendModeSelector_.addItem("Additive", 2); blendModeSelector_.addItem("Screen", 3);
    blendModeSelector_.addItem("Lighten", 9); blendModeSelector_.addItem("Color Dodge", 10);
    blendModeSelector_.addSeparator(); blendModeSelector_.addSectionHeading("-- DARKEN --");
    blendModeSelector_.addItem("Multiply", 4); blendModeSelector_.addItem("Darken", 8);
    blendModeSelector_.addItem("Color Burn", 11);
    blendModeSelector_.addSeparator(); blendModeSelector_.addSectionHeading("-- CONTRAST --");
    blendModeSelector_.addItem("Overlay", 5); blendModeSelector_.addItem("Soft Light", 6);
    blendModeSelector_.addItem("Hard Light", 7); blendModeSelector_.addItem("Vivid Light", 15);
    blendModeSelector_.addItem("Linear Light", 16); blendModeSelector_.addItem("Pin Light", 17);
    blendModeSelector_.addItem("Hard Mix", 18);
    blendModeSelector_.addSeparator(); blendModeSelector_.addSectionHeading("-- INVERSION --");
    blendModeSelector_.addItem("Difference", 12); blendModeSelector_.addItem("Exclusion", 13);
    blendModeSelector_.addItem("Subtract", 14);
    blendModeSelector_.onChange = [this] { if (currentKey_) currentKey_->blendMode = static_cast<KeySlot::BlendMode>(blendModeSelector_.getSelectedId() - 1); };

    auto setupSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& name, float mn, float mx, float def) {
        addAndMakeVisible(s); addAndMakeVisible(l);
        l.setText(name, juce::dontSendNotification);
        l.setFont(10.0f); l.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        s.setRange(static_cast<double>(mn), static_cast<double>(mx), 0.01);
        s.setValue(static_cast<double>(def), juce::dontSendNotification);
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 16);
    };

    setupSlider(opacitySlider_, opacityLabel_, "Opacity", 0, 1, 1);
    opacitySlider_.onValueChange = [this] { if (currentKey_) { currentKey_->opacity = static_cast<float>(opacitySlider_.getValue()); updatePreviewImage(); } };

    setupSlider(thresholdSlider_, thresholdLabel_, "Threshold", 0, 1, 0.1f);
    thresholdSlider_.onValueChange = [this] { if (currentKey_) { currentKey_->keyThreshold = static_cast<float>(thresholdSlider_.getValue()); updatePreviewImage(); } };

    setupSlider(softnessSlider_, softnessLabel_, "Softness", 0, 0.5f, 0.1f);
    softnessSlider_.onValueChange = [this] { if (currentKey_) { currentKey_->keySoftness = static_cast<float>(softnessSlider_.getValue()); updatePreviewImage(); } };

    addAndMakeVisible(chromaColorSwatch_); addAndMakeVisible(chromaColorLabel_);
    chromaColorLabel_.setFont(10.0f); chromaColorLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    chromaColorSwatch_.onColourChanged = [this](juce::Colour col) {
        if (currentKey_) { currentKey_->chromaKeyR = col.getFloatRed(); currentKey_->chromaKeyG = col.getFloatGreen(); currentKey_->chromaKeyB = col.getFloatBlue(); updatePreviewImage(); }
    };

    setupSlider(chromaToleranceSlider_, chromaToleranceLabel_, "Tolerance", 0, 1, 0.2f);
    chromaToleranceSlider_.onValueChange = [this] { if (currentKey_) { currentKey_->chromaKeyTolerance = static_cast<float>(chromaToleranceSlider_.getValue()); updatePreviewImage(); } };

    setupSlider(chromaSoftnessSlider_, chromaSoftnessLabel_, "Softness", 0, 0.5f, 0.1f);
    chromaSoftnessSlider_.onValueChange = [this] { if (currentKey_) { currentKey_->chromaKeySoftness = static_cast<float>(chromaSoftnessSlider_.getValue()); updatePreviewImage(); } };

    // Playback
    addAndMakeVisible(playbackLabel_); setupLabel(playbackLabel_, "Playback", true);
    addAndMakeVisible(latchToggle_);
    latchToggle_.onClick = [this] { if (currentKey_) currentKey_->latched = latchToggle_.getToggleState(); };
    addAndMakeVisible(ignoreRandomToggle_);
    ignoreRandomToggle_.onClick = [this] { if (currentKey_) currentKey_->ignoreRandom = ignoreRandomToggle_.getToggleState(); };

    addAndMakeVisible(randomBeatLabel1_);
    randomBeatLabel1_.setFont(9.0f); randomBeatLabel1_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(randomBeatLabel2_);
    randomBeatLabel2_.setFont(9.0f); randomBeatLabel2_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(randomBeatSelector_);
    randomBeatSelector_.addItem("Use Global", 1);
    const int beatVals[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
    for (int i = 0; i < 8; ++i)
        randomBeatSelector_.addItem(juce::String(beatVals[i]), i + 2);
    randomBeatSelector_.onChange = [this] {
        if (!currentKey_) return;
        int sel = randomBeatSelector_.getSelectedId();
        if (sel == 1) currentKey_->randomBeatDuration = 0; // 0 = use global
        else { const int v[] = { 1,2,4,8,16,32,64,128 }; currentKey_->randomBeatDuration = v[sel - 2]; }
    };

    // Effects section
    addAndMakeVisible(effectsLabel_); setupLabel(effectsLabel_, "Effects", true);
    addAndMakeVisible(addEffectSelector_);
    addEffectSelector_.addItem("Add effect...", 1);
    {
        const juce::String cats[] = { "3d","warp","color","glitch","pattern","animation","blend","blur" };
        const juce::String labels[] = { "-- 3D/DEPTH --","-- WARP --","-- COLOR --","-- GLITCH --","-- PATTERN --","-- ANIMATION --","-- BLEND --","-- BLUR --" };
        int id = 2;
        for (int c = 0; c < 8; ++c) {
            auto names = effectLibrary_.getEffectsByCategory(cats[c]);
            if (names.isEmpty()) continue;
            addEffectSelector_.addSeparator(); addEffectSelector_.addSectionHeading(labels[c]);
            for (int i = 0; i < names.size(); ++i) addEffectSelector_.addItem(names[i], id++);
        }
    }
    addEffectSelector_.onChange = [this] {
        if (!currentKey_ || addEffectSelector_.getSelectedId() <= 1) return;
        juce::String fxName = addEffectSelector_.getText();
        if (fxName.isEmpty()) return;
        KeySlot::EffectSlot slot;
        slot.effectName = fxName.toStdString();
        auto* def = effectLibrary_.getEffectDef(fxName);
        if (def) {
            slot.params.resize(def->params.size(), 0.0f);
            slot.mappings.resize(def->params.size());
        }
        currentKey_->effects.push_back(std::move(slot));
        addEffectSelector_.setSelectedId(1, juce::dontSendNotification);
        rebuildEffectRows(); layoutEffectsContent();
    };

    addAndMakeVisible(clearEffectsBtn_);
    clearEffectsBtn_.onClick = [this] { if (currentKey_) { currentKey_->effects.clear(); rebuildEffectRows(); layoutEffectsContent(); } };

    // Scrollable viewport for effects
    effectsViewport_.setViewedComponent(&effectsContent_, false);
    effectsViewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(effectsViewport_);
}

bool KeyEditor::keyPressed(const juce::KeyPress& key)
{
    if (key.isKeyCode(juce::KeyPress::escapeKey)) { if (onClose) onClose(); return true; }
    return false;
}

void KeyEditor::setKey(KeySlot* key)
{
    currentKey_ = key;
    refreshFromKey();
    updatePreviewImage();
    rebuildEffectRows();
    layoutEffectsContent();
    resized();
}

void KeyEditor::refreshFromKey()
{
    if (!currentKey_) { titleLabel_.setText("No Key", juce::dontSendNotification); return; }
    titleLabel_.setText("Key: " + juce::String::charToString(currentKey_->keyChar), juce::dontSendNotification);

    switch (currentKey_->mediaType) {
        case KeySlot::MediaType::None: mediaInfoLabel_.setText("No media", juce::dontSendNotification); break;
        case KeySlot::MediaType::Image: mediaInfoLabel_.setText(currentKey_->mediaFile.getFileNameWithoutExtension(), juce::dontSendNotification); break;
        case KeySlot::MediaType::VideoFile: mediaInfoLabel_.setText("Video: " + currentKey_->mediaFile.getFileNameWithoutExtension(), juce::dontSendNotification); break;
        case KeySlot::MediaType::Camera: mediaInfoLabel_.setText("Camera", juce::dontSendNotification); break;
    }

    keyingModeSelector_.setSelectedId(static_cast<int>(currentKey_->keyingMode) + 1, juce::dontSendNotification);
    blendModeSelector_.setSelectedId(static_cast<int>(currentKey_->blendMode) + 1, juce::dontSendNotification);
    opacitySlider_.setValue(currentKey_->opacity, juce::dontSendNotification);
    thresholdSlider_.setValue(currentKey_->keyThreshold, juce::dontSendNotification);
    softnessSlider_.setValue(currentKey_->keySoftness, juce::dontSendNotification);
    chromaToleranceSlider_.setValue(currentKey_->chromaKeyTolerance, juce::dontSendNotification);
    chromaSoftnessSlider_.setValue(currentKey_->chromaKeySoftness, juce::dontSendNotification);
    chromaColorSwatch_.colour = juce::Colour::fromFloatRGBA(currentKey_->chromaKeyR, currentKey_->chromaKeyG, currentKey_->chromaKeyB, 1.0f);
    chromaColorSwatch_.repaint();

    latchToggle_.setToggleState(currentKey_->latched, juce::dontSendNotification);
    ignoreRandomToggle_.setToggleState(currentKey_->ignoreRandom, juce::dontSendNotification);

    if (currentKey_->randomBeatDuration == 0)
        randomBeatSelector_.setSelectedId(1, juce::dontSendNotification); // "Use Global"
    else {
        const int v[] = {1,2,4,8,16,32,64,128};
        for (int i = 0; i < 8; ++i)
            if (v[i] == currentKey_->randomBeatDuration) { randomBeatSelector_.setSelectedId(i + 2, juce::dontSendNotification); break; }
    }
}

void KeyEditor::updatePreviewImage()
{
    if (!currentKey_ || currentKey_->mediaType != KeySlot::MediaType::Image || !currentKey_->mediaFile.existsAsFile())
    { preview_.previewImage = juce::Image(); preview_.repaint(); return; }

    auto src = juce::ImageFileFormat::loadFrom(currentKey_->mediaFile);
    if (!src.isValid()) { preview_.previewImage = juce::Image(); preview_.repaint(); return; }

    auto img = src.convertedToFormat(juce::Image::ARGB);
    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readWrite);

    for (int y = 0; y < img.getHeight(); ++y)
        for (int x = 0; x < img.getWidth(); ++x)
        {
            auto px = bmp.getPixelColour(x, y);
            float r = px.getFloatRed(), g = px.getFloatGreen(), b = px.getFloatBlue();
            float luma = 0.2126f * r + 0.7152f * g + 0.0722f * b;
            float alpha = 1.0f;

            switch (currentKey_->keyingMode) {
                case KeySlot::KeyingMode::Alpha: alpha = px.getFloatAlpha(); break;
                case KeySlot::KeyingMode::LumaKey: { float t=currentKey_->keyThreshold, s=currentKey_->keySoftness; alpha = std::clamp((luma-(t-s))/(2*s+0.001f),0.f,1.f); break; }
                case KeySlot::KeyingMode::InvertedLumaKey: { float t=currentKey_->keyThreshold, s=currentKey_->keySoftness; alpha = 1.f-std::clamp((luma-(t-s))/(2*s+0.001f),0.f,1.f); break; }
                case KeySlot::KeyingMode::LumaIsAlpha: alpha = luma; break;
                case KeySlot::KeyingMode::InvertedLumaIsAlpha: alpha = 1.f - luma; break;
                case KeySlot::KeyingMode::ChromaKey: { float d=std::sqrt((r-currentKey_->chromaKeyR)*(r-currentKey_->chromaKeyR)+(g-currentKey_->chromaKeyG)*(g-currentKey_->chromaKeyG)+(b-currentKey_->chromaKeyB)*(b-currentKey_->chromaKeyB)); float t=currentKey_->chromaKeyTolerance, s=currentKey_->chromaKeySoftness; alpha=std::clamp((d-(t-s))/(2*s+0.001f),0.f,1.f); break; }
                case KeySlot::KeyingMode::MaxRGB: alpha = std::max(r, std::max(g, b)); break;
                case KeySlot::KeyingMode::SaturationKey: { float sat=std::max(r,std::max(g,b))-std::min(r,std::min(g,b)); float t=currentKey_->keyThreshold, s=currentKey_->keySoftness; alpha=std::clamp((sat-(t-s))/(2*s+0.001f),0.f,1.f); break; }
                case KeySlot::KeyingMode::ThresholdMask: alpha = luma >= 0.5f ? 1.f : 0.f; break;
                case KeySlot::KeyingMode::ChannelR: alpha = r; break;
                case KeySlot::KeyingMode::ChannelG: alpha = g; break;
                case KeySlot::KeyingMode::ChannelB: alpha = b; break;
                case KeySlot::KeyingMode::VignetteAlpha: { float cx=static_cast<float>(x)/img.getWidth()-0.5f, cy=static_cast<float>(y)/img.getHeight()-0.5f; float d=std::sqrt(cx*cx+cy*cy); alpha=1.f-std::clamp((d-currentKey_->vignetteInner)/(currentKey_->vignetteOuter-currentKey_->vignetteInner+0.001f),0.f,1.f); break; }
                default: break;
            }
            alpha *= currentKey_->opacity;
            bmp.setPixelColour(x, y, px.withAlpha(alpha));
        }

    preview_.previewImage = img;
    preview_.repaint();
}

void KeyEditor::rebuildEffectRows()
{
    for (auto& row : effectRows_)
    {
        effectsContent_.removeChildComponent(row.enableBtn.get());
        effectsContent_.removeChildComponent(row.nameLabel.get());
        effectsContent_.removeChildComponent(row.removeBtn.get());
        for (auto& pr : row.paramRows) {
            effectsContent_.removeChildComponent(pr.label.get());
            effectsContent_.removeChildComponent(pr.slider.get());
            if (pr.mapBtn) effectsContent_.removeChildComponent(pr.mapBtn.get());
            if (pr.sourceCombo) effectsContent_.removeChildComponent(pr.sourceCombo.get());
            if (pr.curveCombo) effectsContent_.removeChildComponent(pr.curveCombo.get());
        }
    }
    effectRows_.clear();
    if (!currentKey_) return;

    for (size_t i = 0; i < currentKey_->effects.size(); ++i)
    {
        EffectRow row;
        row.enableBtn = std::make_unique<juce::ToggleButton>();
        row.enableBtn->setToggleState(currentKey_->effects[i].enabled, juce::dontSendNotification);
        row.enableBtn->onClick = [this, i] { if (currentKey_ && i < currentKey_->effects.size()) currentKey_->effects[i].enabled = effectRows_[i].enableBtn->getToggleState(); };
        effectsContent_.addAndMakeVisible(row.enableBtn.get());

        row.nameLabel = std::make_unique<juce::Label>("", juce::String(currentKey_->effects[i].effectName));
        row.nameLabel->setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        row.nameLabel->setFont(juce::Font(11.0f, juce::Font::bold));
        effectsContent_.addAndMakeVisible(row.nameLabel.get());

        row.removeBtn = std::make_unique<juce::TextButton>("X");
        row.removeBtn->onClick = [this, i] {
            if (currentKey_ && i < currentKey_->effects.size()) {
                currentKey_->effects.erase(currentKey_->effects.begin() + static_cast<long>(i));
                rebuildEffectRows(); layoutEffectsContent();
            }
        };
        effectsContent_.addAndMakeVisible(row.removeBtn.get());

        auto* def = effectLibrary_.getEffectDef(juce::String(currentKey_->effects[i].effectName));
        // Ensure mappings vector is sized to match params
        while (currentKey_->effects[i].mappings.size() < currentKey_->effects[i].params.size())
            currentKey_->effects[i].mappings.push_back(KeySlot::ParamMapping{});

        if (def)
        {
            for (size_t p = 0; p < def->params.size() && p < currentKey_->effects[i].params.size(); ++p)
            {
                EffectRow::ParamRow pr;
                pr.label = std::make_unique<juce::Label>("", juce::String(def->params[p].name));
                pr.label->setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));
                pr.label->setFont(9.0f);
                effectsContent_.addAndMakeVisible(pr.label.get());

                pr.slider = std::make_unique<juce::Slider>();
                pr.slider->setRange(0.0, 1.0, 0.01);
                pr.slider->setValue(currentKey_->effects[i].params[p], juce::dontSendNotification);
                pr.slider->setSliderStyle(juce::Slider::LinearHorizontal);
                pr.slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 35, 14);
                pr.slider->onValueChange = [this, i, p] {
                    if (currentKey_ && i < currentKey_->effects.size() && p < currentKey_->effects[i].params.size())
                        currentKey_->effects[i].params[p] = static_cast<float>(effectRows_[i].paramRows[p].slider->getValue());
                };
                effectsContent_.addAndMakeVisible(pr.slider.get());

                // Mapping button — toggles mapping controls
                bool hasMapping = (p < currentKey_->effects[i].mappings.size() &&
                                   currentKey_->effects[i].mappings[p].active);
                pr.mapBtn = std::make_unique<juce::TextButton>("M");
                if (hasMapping)
                    pr.mapBtn->setColour(juce::TextButton::buttonColourId,
                                          juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.5f));
                pr.mapBtn->onClick = [this, i, p] {
                    if (i < effectRows_.size() && p < effectRows_[i].paramRows.size())
                    {
                        effectRows_[i].paramRows[p].mappingExpanded = !effectRows_[i].paramRows[p].mappingExpanded;
                        layoutEffectsContent();
                    }
                };
                effectsContent_.addAndMakeVisible(pr.mapBtn.get());

                // Source combo
                pr.sourceCombo = std::make_unique<juce::ComboBox>();
                pr.sourceCombo->addItem("None", 1);
                pr.sourceCombo->addSectionHeading("-- Amplitude --");
                pr.sourceCombo->addItem("RMS", 2);
                pr.sourceCombo->addItem("Peak", 3);
                pr.sourceCombo->addItem("Spectral Centroid", 9);
                pr.sourceCombo->addItem("Spectral Flux", 10);
                pr.sourceCombo->addSectionHeading("-- Bands --");
                pr.sourceCombo->addItem("Sub", 13);
                pr.sourceCombo->addItem("Bass", 14);
                pr.sourceCombo->addItem("Low Mid", 15);
                pr.sourceCombo->addItem("Mid", 16);
                pr.sourceCombo->addItem("High Mid", 17);
                pr.sourceCombo->addItem("Presence", 18);
                pr.sourceCombo->addItem("Brilliance", 19);
                pr.sourceCombo->addSectionHeading("-- Rhythm --");
                pr.sourceCombo->addItem("Onset Strength", 20);
                pr.sourceCombo->addItem("Beat Phase", 21);
                pr.sourceCombo->addItem("Transient Density", 8);
                pr.sourceCombo->addSectionHeading("-- Other --");
                pr.sourceCombo->addItem("Harmonic Change", 27);
                pr.sourceCombo->addItem("Dynamic Range", 7);

                // Set current value
                if (p < currentKey_->effects[i].mappings.size() && currentKey_->effects[i].mappings[p].active)
                {
                    int srcId = static_cast<int>(currentKey_->effects[i].mappings[p].source) + 2;
                    pr.sourceCombo->setSelectedId(srcId, juce::dontSendNotification);
                }
                else
                    pr.sourceCombo->setSelectedId(1, juce::dontSendNotification);

                pr.sourceCombo->onChange = [this, i, p] {
                    if (!currentKey_ || i >= currentKey_->effects.size() || p >= currentKey_->effects[i].mappings.size())
                        return;
                    int sel = effectRows_[i].paramRows[p].sourceCombo->getSelectedId();
                    if (sel <= 1) {
                        currentKey_->effects[i].mappings[p].active = false;
                        effectRows_[i].paramRows[p].mapBtn->removeColour(juce::TextButton::buttonColourId);
                    } else {
                        currentKey_->effects[i].mappings[p].active = true;
                        currentKey_->effects[i].mappings[p].source = static_cast<MappingSource>(sel - 2);
                        effectRows_[i].paramRows[p].mapBtn->setColour(juce::TextButton::buttonColourId,
                            juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.5f));
                    }
                };
                effectsContent_.addAndMakeVisible(pr.sourceCombo.get());

                // Curve combo
                pr.curveCombo = std::make_unique<juce::ComboBox>();
                pr.curveCombo->addItem("Linear", 1);
                pr.curveCombo->addItem("Exponential", 2);
                pr.curveCombo->addItem("Logarithmic", 3);
                pr.curveCombo->addItem("S-Curve", 4);
                pr.curveCombo->addItem("Stepped", 5);
                if (p < currentKey_->effects[i].mappings.size())
                    pr.curveCombo->setSelectedId(static_cast<int>(currentKey_->effects[i].mappings[p].curve) + 1, juce::dontSendNotification);
                else
                    pr.curveCombo->setSelectedId(1, juce::dontSendNotification);

                pr.curveCombo->onChange = [this, i, p] {
                    if (currentKey_ && i < currentKey_->effects.size() && p < currentKey_->effects[i].mappings.size()) {
                        int sel = effectRows_[i].paramRows[p].curveCombo->getSelectedId();
                        currentKey_->effects[i].mappings[p].curve = static_cast<MappingCurve>(sel - 1);
                    }
                };
                effectsContent_.addAndMakeVisible(pr.curveCombo.get());

                // Start hidden unless mapping is active
                pr.mappingExpanded = hasMapping;
                pr.sourceCombo->setVisible(pr.mappingExpanded);
                pr.curveCombo->setVisible(pr.mappingExpanded);

                row.paramRows.push_back(std::move(pr));
            }
        }
        effectRows_.push_back(std::move(row));
    }
}

void KeyEditor::layoutEffectsContent()
{
    int w = effectsViewport_.getWidth() - 12; // account for scrollbar
    if (w < 100) w = 300;
    int y = 0;

    for (auto& row : effectRows_)
    {
        row.enableBtn->setBounds(0, y, 20, 18);
        row.nameLabel->setBounds(24, y, w - 50, 18);
        row.removeBtn->setBounds(w - 22, y, 22, 18);
        y += 20;

        for (auto& pr : row.paramRows)
        {
            pr.label->setBounds(24, y, 60, 16);
            int sliderRight = w - 30;
            pr.slider->setBounds(85, y, sliderRight - 85, 16);
            pr.mapBtn->setBounds(sliderRight + 2, y, 24, 16);
            y += 18;

            // Mapping controls row (shown when M is clicked)
            if (pr.sourceCombo && pr.curveCombo)
            {
                pr.sourceCombo->setVisible(pr.mappingExpanded);
                pr.curveCombo->setVisible(pr.mappingExpanded);
                if (pr.mappingExpanded)
                {
                    pr.sourceCombo->setBounds(40, y, 140, 18);
                    pr.curveCombo->setBounds(185, y, 100, 18);
                    y += 20;
                }
            }
        }
        y += 4;
    }

    effectsContent_.requiredHeight_ = y;
    effectsContent_.setSize(w, std::max(y, 10));
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
    auto header = area.removeFromTop(22);
    titleLabel_.setBounds(header.removeFromLeft(200));
    closeButton_.setBounds(header.removeFromRight(24));
    area.removeFromTop(4);

    // === Right column: Preview ===
    auto rightCol = area.removeFromRight(std::min(200, area.getWidth() / 4));
    rightCol.removeFromLeft(8);
    auto bgRow = rightCol.removeFromTop(18);
    bgBlackBtn_.setBounds(bgRow.removeFromLeft(bgRow.getWidth() / 4));
    bgGreyBtn_.setBounds(bgRow.removeFromLeft(bgRow.getWidth() / 3));
    bgWhiteBtn_.setBounds(bgRow.removeFromLeft(bgRow.getWidth() / 2));
    bgCheckerBtn_.setBounds(bgRow);
    rightCol.removeFromTop(2);
    preview_.setBounds(rightCol);

    // === Left column: Media + Keying + Playback ===
    int leftW = std::min(280, area.getWidth() / 3);
    auto leftCol = area.removeFromLeft(leftW);

    auto mediaRow = leftCol.removeFromTop(22);
    openImageBtn_.setBounds(mediaRow.removeFromLeft(75));
    mediaRow.removeFromLeft(3);
    clearMediaBtn_.setBounds(mediaRow.removeFromLeft(45));
    mediaRow.removeFromLeft(4);
    mediaInfoLabel_.setBounds(mediaRow);
    leftCol.removeFromTop(6);

    keyingLabel_.setBounds(leftCol.removeFromTop(16));
    leftCol.removeFromTop(2);
    keyingModeSelector_.setBounds(leftCol.removeFromTop(20).removeFromLeft(leftW));
    leftCol.removeFromTop(2);

    auto opRow = leftCol.removeFromTop(20);
    opacityLabel_.setBounds(opRow.removeFromLeft(45));
    opacitySlider_.setBounds(opRow);
    leftCol.removeFromTop(2);

    // Mode-specific
    auto km = currentKey_ ? currentKey_->keyingMode : KeySlot::KeyingMode::Alpha;
    bool showTh = (km == KeySlot::KeyingMode::LumaKey || km == KeySlot::KeyingMode::InvertedLumaKey || km == KeySlot::KeyingMode::SaturationKey || km == KeySlot::KeyingMode::EdgeDetection);
    bool showCh = (km == KeySlot::KeyingMode::ChromaKey);

    thresholdLabel_.setVisible(showTh); thresholdSlider_.setVisible(showTh);
    softnessLabel_.setVisible(showTh); softnessSlider_.setVisible(showTh);
    chromaColorSwatch_.setVisible(showCh); chromaColorLabel_.setVisible(showCh);
    chromaToleranceLabel_.setVisible(showCh); chromaToleranceSlider_.setVisible(showCh);
    chromaSoftnessLabel_.setVisible(showCh); chromaSoftnessSlider_.setVisible(showCh);

    if (showTh) {
        auto r1 = leftCol.removeFromTop(18); thresholdLabel_.setBounds(r1.removeFromLeft(55)); thresholdSlider_.setBounds(r1);
        leftCol.removeFromTop(1);
        auto r2 = leftCol.removeFromTop(18); softnessLabel_.setBounds(r2.removeFromLeft(55)); softnessSlider_.setBounds(r2);
        leftCol.removeFromTop(2);
    } else if (showCh) {
        auto r0 = leftCol.removeFromTop(20); chromaColorLabel_.setBounds(r0.removeFromLeft(55)); chromaColorSwatch_.setBounds(r0.removeFromLeft(30).reduced(0,2));
        leftCol.removeFromTop(1);
        auto r1 = leftCol.removeFromTop(18); chromaToleranceLabel_.setBounds(r1.removeFromLeft(55)); chromaToleranceSlider_.setBounds(r1);
        leftCol.removeFromTop(1);
        auto r2 = leftCol.removeFromTop(18); chromaSoftnessLabel_.setBounds(r2.removeFromLeft(55)); chromaSoftnessSlider_.setBounds(r2);
        leftCol.removeFromTop(2);
    }

    blendLabel_.setBounds(leftCol.removeFromTop(16));
    leftCol.removeFromTop(2);
    blendModeSelector_.setBounds(leftCol.removeFromTop(20).removeFromLeft(leftW));
    leftCol.removeFromTop(6);

    playbackLabel_.setBounds(leftCol.removeFromTop(16));
    leftCol.removeFromTop(2);
    auto playRow = leftCol.removeFromTop(20);
    latchToggle_.setBounds(playRow.removeFromLeft(65));
    playRow.removeFromLeft(8);
    ignoreRandomToggle_.setBounds(playRow.removeFromLeft(130));
    leftCol.removeFromTop(4);

    auto randRow1 = leftCol.removeFromTop(14);
    randomBeatLabel1_.setBounds(randRow1);
    auto randRow2 = leftCol.removeFromTop(20);
    randomBeatLabel2_.setBounds(randRow2.removeFromLeft(80));
    randomBeatSelector_.setBounds(randRow2.removeFromLeft(80));

    // === Center column: Effects (scrollable) ===
    area.removeFromLeft(8);
    effectsLabel_.setBounds(area.removeFromTop(16));
    area.removeFromTop(2);
    auto addRow = area.removeFromTop(22);
    addEffectSelector_.setBounds(addRow.removeFromLeft(200));
    addRow.removeFromLeft(4);
    clearEffectsBtn_.setBounds(addRow.removeFromLeft(100));
    area.removeFromTop(4);

    effectsViewport_.setBounds(area);
    layoutEffectsContent();
}
