#include "EffectsRackPanel.h"
#include "ui/LookAndFeel.h"

EffectsRackPanel::EffectsRackPanel(MappingEngine& mappingEngine,
                                   EffectChain& effectChain,
                                   EffectLibrary& effectLibrary)
    : mappingEngine_(mappingEngine),
      effectChain_(effectChain),
      effectLibrary_(effectLibrary)
{
    viewport_.setViewedComponent(&contentComponent_, false);
    viewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport_);

    rebuildUI();
    startTimerHz(10); // Update parameter displays at 10 fps
}

EffectsRackPanel::~EffectsRackPanel()
{
    stopTimer();
    closeMappingEditor();
}

void EffectsRackPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(AudioDNALookAndFeel::kBackground));

    // Title
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("Effects Rack", getLocalBounds().removeFromTop(24).reduced(6, 0),
               juce::Justification::centredLeft);
}

void EffectsRackPanel::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(26); // Title area
    viewport_.setBounds(area);

    // Calculate content height
    int contentHeight = 0;
    int sectionSpacing = 6;

    for (const auto& section : sections_)
    {
        contentHeight += 24; // Effect header row
        if (section->enableToggle->getToggleState())
        {
            contentHeight += static_cast<int>(section->paramRows.size()) * 26;
        }
        contentHeight += sectionSpacing;
    }

    contentHeight = std::max(contentHeight, area.getHeight());
    contentComponent_.setSize(area.getWidth() - 12, contentHeight);

    // Layout sections
    auto contentArea = contentComponent_.getLocalBounds().reduced(2, 0);
    for (const auto& section : sections_)
    {
        // Header row: toggle + name
        auto headerRow = contentArea.removeFromTop(24);
        section->enableToggle->setBounds(headerRow.removeFromLeft(24));
        section->nameLabel->setBounds(headerRow);

        // Parameter rows (only if enabled)
        if (section->enableToggle->getToggleState())
        {
            for (const auto& paramRow : section->paramRows)
            {
                auto row = contentArea.removeFromTop(26);
                auto labelArea = row.removeFromLeft(60);
                auto mapBtnArea = row.removeFromRight(30);
                paramRow->nameLabel->setBounds(labelArea);
                paramRow->mapButton->setBounds(mapBtnArea.reduced(0, 2));
                paramRow->slider->setBounds(row);
            }
        }

        contentArea.removeFromTop(sectionSpacing);
    }
}

void EffectsRackPanel::timerCallback()
{
    // Update slider values to reflect current effect parameter values
    // (which may have been changed by the MappingEngine)
    for (const auto& section : sections_)
    {
        auto* effect = effectChain_.getEffect(section->effectIndex);
        if (effect == nullptr)
            continue;

        for (const auto& paramRow : section->paramRows)
        {
            float currentValue = effect->getParam(paramRow->paramIndex).value;
            // Only update if not being dragged
            if (!paramRow->slider->isMouseButtonDown())
            {
                paramRow->slider->setValue(currentValue, juce::dontSendNotification);
            }

            // Update map button text to show if a mapping exists
            int mappingIdx = findMappingForParam(paramRow->effectIndex, paramRow->paramIndex);
            if (mappingIdx >= 0)
                paramRow->mapButton->setButtonText("M");
            else
                paramRow->mapButton->setButtonText("+");
        }
    }
}

void EffectsRackPanel::mappingEditorChanged(MappingEditor* editor)
{
    int mappingIdx = editor->getMappingIndex();
    if (mappingIdx < 0)
        return;

    auto* mapping = mappingEngine_.getMapping(mappingIdx);
    if (mapping == nullptr)
        return;

    Mapping updated = editor->getMapping();
    // Preserve target info
    updated.targetEffectId = mapping->targetEffectId;
    updated.targetParamIndex = mapping->targetParamIndex;
    *mapping = updated;
}

void EffectsRackPanel::mappingEditorDeleteRequested(MappingEditor* editor)
{
    int mappingIdx = editor->getMappingIndex();
    if (mappingIdx >= 0)
    {
        mappingEngine_.removeMapping(mappingIdx);
    }
    closeMappingEditor();
}

void EffectsRackPanel::rebuildUI()
{
    // Clear existing sections
    closeMappingEditor();
    sections_.clear();
    contentComponent_.removeAllChildren();

    int numEffects = effectChain_.getNumEffects();
    for (int ei = 0; ei < numEffects; ++ei)
    {
        auto* effect = effectChain_.getEffect(ei);
        if (effect == nullptr)
            continue;

        auto section = std::make_unique<EffectSection>();
        section->effectIndex = ei;

        // Effect name label
        section->nameLabel = std::make_unique<juce::Label>();
        section->nameLabel->setText(effect->getName(), juce::dontSendNotification);
        section->nameLabel->setFont(juce::Font(13.0f, juce::Font::bold));
        section->nameLabel->setColour(juce::Label::textColourId,
                                      juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        contentComponent_.addAndMakeVisible(section->nameLabel.get());

        // Enable toggle
        section->enableToggle = std::make_unique<juce::ToggleButton>();
        section->enableToggle->setToggleState(effect->isEnabled(), juce::dontSendNotification);
        contentComponent_.addAndMakeVisible(section->enableToggle.get());

        int capturedEi = ei;
        section->enableToggle->onClick = [this, capturedEi]
        {
            auto* eff = effectChain_.getEffect(capturedEi);
            if (eff != nullptr && capturedEi < static_cast<int>(sections_.size()))
            {
                eff->setEnabled(sections_[static_cast<size_t>(capturedEi)]
                                    ->enableToggle->getToggleState());
                resized(); // Relayout to show/hide params
            }
        };

        // Parameter rows
        for (int pi = 0; pi < effect->getNumParams(); ++pi)
        {
            const auto& param = effect->getParam(pi);

            auto paramRow = std::make_unique<ParamRow>();
            paramRow->effectIndex = ei;
            paramRow->paramIndex = pi;

            // Name label
            paramRow->nameLabel = std::make_unique<juce::Label>();
            paramRow->nameLabel->setText(param.name, juce::dontSendNotification);
            paramRow->nameLabel->setFont(juce::Font(11.0f));
            paramRow->nameLabel->setColour(juce::Label::textColourId,
                                           juce::Colour(AudioDNALookAndFeel::kTextSecondary));
            contentComponent_.addAndMakeVisible(paramRow->nameLabel.get());

            // Slider
            paramRow->slider = std::make_unique<juce::Slider>();
            paramRow->slider->setRange(0.0, 1.0, 0.001);
            paramRow->slider->setValue(param.value, juce::dontSendNotification);
            paramRow->slider->setSliderStyle(juce::Slider::LinearHorizontal);
            paramRow->slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
            paramRow->slider->setColour(juce::Slider::thumbColourId,
                                        juce::Colour(AudioDNALookAndFeel::kAccentCyan));
            contentComponent_.addAndMakeVisible(paramRow->slider.get());

            int capturedPi = pi;
            paramRow->slider->onValueChange = [this, capturedEi, capturedPi]
            {
                auto* eff = effectChain_.getEffect(capturedEi);
                if (eff == nullptr)
                    return;
                auto& sec = sections_[static_cast<size_t>(capturedEi)];
                auto& pr = sec->paramRows[static_cast<size_t>(capturedPi)];
                eff->setParamValue(capturedPi,
                                   static_cast<float>(pr->slider->getValue()));
            };

            // Map button
            paramRow->mapButton = std::make_unique<juce::TextButton>("+");
            paramRow->mapButton->setColour(juce::TextButton::buttonColourId,
                                           juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
            contentComponent_.addAndMakeVisible(paramRow->mapButton.get());

            paramRow->mapButton->onClick = [this, capturedEi, capturedPi]
            {
                openMappingEditor(capturedEi, capturedPi);
            };

            section->paramRows.push_back(std::move(paramRow));
        }

        sections_.push_back(std::move(section));
    }

    resized();
}

void EffectsRackPanel::openMappingEditor(int effectIndex, int paramIndex)
{
    closeMappingEditor();

    editingEffectIndex_ = effectIndex;
    editingParamIndex_ = paramIndex;

    activeMappingEditor_ = std::make_unique<MappingEditor>();
    activeMappingEditor_->addListener(this);

    // Check if a mapping already exists for this param
    int existingIdx = findMappingForParam(effectIndex, paramIndex);
    if (existingIdx >= 0)
    {
        // Edit existing mapping
        const auto* mapping = mappingEngine_.getMapping(existingIdx);
        activeMappingEditor_->setMapping(*mapping, existingIdx);
    }
    else
    {
        // Create a new mapping
        Mapping newMapping;
        newMapping.source = MappingSource::RMS;
        newMapping.targetEffectId = static_cast<uint32_t>(effectIndex);
        newMapping.targetParamIndex = static_cast<uint32_t>(paramIndex);
        newMapping.curve = MappingCurve::Linear;
        newMapping.smoothing = 0.15f;
        int idx = mappingEngine_.addMapping(newMapping);
        activeMappingEditor_->setMapping(newMapping, idx);
    }

    // Show as child component overlaid on the panel
    addAndMakeVisible(activeMappingEditor_.get());
    activeMappingEditor_->setBounds(getLocalBounds().reduced(4));
    activeMappingEditor_->toFront(true);
}

void EffectsRackPanel::closeMappingEditor()
{
    if (activeMappingEditor_)
    {
        activeMappingEditor_->removeListener(this);
        removeChildComponent(activeMappingEditor_.get());
        activeMappingEditor_.reset();
    }
    editingEffectIndex_ = -1;
    editingParamIndex_ = -1;
}

int EffectsRackPanel::findMappingForParam(int effectIndex, int paramIndex) const
{
    for (int i = 0; i < mappingEngine_.getNumMappings(); ++i)
    {
        const auto* m = mappingEngine_.getMapping(i);
        if (m != nullptr
            && static_cast<int>(m->targetEffectId) == effectIndex
            && static_cast<int>(m->targetParamIndex) == paramIndex
            && m->enabled)
        {
            return i;
        }
    }
    return -1;
}
