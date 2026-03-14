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

    int contentWidth = area.getWidth() - 12;
    int knobsPerRow = juce::jmax(1, contentWidth / Knob::kPreferredWidth);
    int sectionSpacing = 8;

    // Calculate content height
    int contentHeight = 0;
    for (const auto& section : sections_)
    {
        contentHeight += 24; // Effect header row
        if (section->enableToggle->getToggleState())
        {
            int numParams = static_cast<int>(section->paramKnobs.size());
            int knobRows = (numParams + knobsPerRow - 1) / knobsPerRow;
            // Each knob row: knob height + map button height
            contentHeight += knobRows * (Knob::kPreferredHeight + 18);
        }
        contentHeight += sectionSpacing;
    }

    contentHeight = std::max(contentHeight, area.getHeight());
    contentComponent_.setSize(contentWidth, contentHeight);

    // Layout sections
    auto contentArea = contentComponent_.getLocalBounds().reduced(2, 0);
    for (const auto& section : sections_)
    {
        // Header row: toggle + name
        auto headerRow = contentArea.removeFromTop(24);
        section->enableToggle->setBounds(headerRow.removeFromLeft(24));
        section->nameLabel->setBounds(headerRow);

        // Parameter knobs in grid (only if enabled)
        if (section->enableToggle->getToggleState())
        {
            int numParams = static_cast<int>(section->paramKnobs.size());
            for (int i = 0; i < numParams; i += knobsPerRow)
            {
                auto knobRow = contentArea.removeFromTop(Knob::kPreferredHeight);
                auto mapBtnRow = contentArea.removeFromTop(18);

                int knobsThisRow = juce::jmin(knobsPerRow, numParams - i);
                int knobWidth = contentArea.getWidth() / knobsPerRow;

                for (int j = 0; j < knobsThisRow; ++j)
                {
                    auto& pk = section->paramKnobs[static_cast<size_t>(i + j)];
                    auto knobArea = knobRow.removeFromLeft(knobWidth);
                    pk->knob->setBounds(knobArea);

                    auto btnArea = mapBtnRow.removeFromLeft(knobWidth);
                    pk->mapButton->setBounds(btnArea.reduced(knobWidth / 4, 0));
                }
            }
        }

        contentArea.removeFromTop(sectionSpacing);
    }
}

void EffectsRackPanel::timerCallback()
{
    // Detect when effects become available (GL thread creates them async)
    int currentCount = effectChain_.getNumEffects();
    if (currentCount != lastKnownEffectCount_)
    {
        lastKnownEffectCount_ = currentCount;
        rebuildUI();
        return;
    }

    // Update knob values to reflect current effect parameter values
    // (which may have been changed by the MappingEngine)
    for (const auto& section : sections_)
    {
        auto* effect = effectChain_.getEffect(section->effectIndex);
        if (effect == nullptr)
            continue;

        for (const auto& pk : section->paramKnobs)
        {
            float currentValue = effect->getParam(pk->paramIndex).value;
            auto& slider = pk->knob->getSlider();
            // Only update if not being dragged
            if (!slider.isMouseButtonDown())
            {
                slider.setValue(currentValue, juce::dontSendNotification);
            }

            // Update mapping indicator
            int mappingIdx = findMappingForParam(pk->effectIndex, pk->paramIndex);
            if (mappingIdx >= 0)
            {
                const auto* m = mappingEngine_.getMapping(mappingIdx);
                pk->knob->setMappingIndicator(m ? "mapped" : juce::String());
                pk->mapButton->setButtonText("M");
            }
            else
            {
                pk->knob->setMappingIndicator({});
                pk->mapButton->setButtonText("+");
            }
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

void EffectsRackPanel::mappingEditorCloseRequested(MappingEditor* /*editor*/)
{
    closeMappingEditor();
}

void EffectsRackPanel::refreshFromChain()
{
    lastKnownEffectCount_ = effectChain_.getNumEffects();
    rebuildUI();
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

        // Parameter knobs
        for (int pi = 0; pi < effect->getNumParams(); ++pi)
        {
            const auto& param = effect->getParam(pi);

            auto pk = std::make_unique<ParamKnob>();
            pk->effectIndex = ei;
            pk->paramIndex = pi;

            // Knob
            pk->knob = std::make_unique<Knob>(param.name);
            pk->knob->getSlider().setValue(param.value, juce::dontSendNotification);
            contentComponent_.addAndMakeVisible(pk->knob.get());

            int capturedPi = pi;
            pk->knob->getSlider().onValueChange = [this, capturedEi, capturedPi]
            {
                auto* eff = effectChain_.getEffect(capturedEi);
                if (eff == nullptr)
                    return;
                auto& sec = sections_[static_cast<size_t>(capturedEi)];
                auto& p = sec->paramKnobs[static_cast<size_t>(capturedPi)];
                eff->setParamValue(capturedPi,
                                   static_cast<float>(p->knob->getSlider().getValue()));
            };

            // Map button
            pk->mapButton = std::make_unique<juce::TextButton>("+");
            pk->mapButton->setColour(juce::TextButton::buttonColourId,
                                     juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
            contentComponent_.addAndMakeVisible(pk->mapButton.get());

            pk->mapButton->onClick = [this, capturedEi, capturedPi]
            {
                openMappingEditor(capturedEi, capturedPi);
            };

            section->paramKnobs.push_back(std::move(pk));
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
