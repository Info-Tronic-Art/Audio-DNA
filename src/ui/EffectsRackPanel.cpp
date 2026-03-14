#include "EffectsRackPanel.h"
#include "ui/LookAndFeel.h"
#include <map>

EffectsRackPanel::EffectsRackPanel(MappingEngine& mappingEngine,
                                   EffectChain& effectChain,
                                   EffectLibrary& /*effectLibrary*/)
    : mappingEngine_(mappingEngine),
      effectChain_(effectChain)
{
    viewport_.setViewedComponent(&contentComponent_, false);
    viewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport_);

    addAndMakeVisible(randomizeButton_);
    randomizeButton_.onClick = [this]
    {
        juce::Random rng;
        int numEffects = effectChain_.getNumEffects();
        int numToEnable = rng.nextInt({3, 8});

        // Disable all, clear mappings
        for (int i = 0; i < numEffects; ++i)
        {
            auto* fx = effectChain_.getEffect(i);
            if (fx) fx->setEnabled(false);
        }
        mappingEngine_.clearAll();

        static const MappingSource sources[] = {
            MappingSource::RMS, MappingSource::Peak, MappingSource::SpectralCentroid,
            MappingSource::SpectralFlux, MappingSource::BandSub, MappingSource::BandBass,
            MappingSource::BandMid, MappingSource::OnsetStrength, MappingSource::BeatPhase,
            MappingSource::TransientDensity, MappingSource::HarmonicChange
        };
        static const MappingCurve curves[] = {
            MappingCurve::Linear, MappingCurve::Exponential,
            MappingCurve::Logarithmic, MappingCurve::SCurve
        };

        for (int enabled = 0; enabled < numToEnable && enabled < numEffects; )
        {
            int idx = rng.nextInt(numEffects);
            auto* fx = effectChain_.getEffect(idx);
            if (fx && !fx->isEnabled())
            {
                fx->setEnabled(true);
                for (int p = 0; p < fx->getNumParams(); ++p)
                    fx->setParamValue(p, rng.nextFloat());

                // Random mappings
                int paramsToMap = rng.nextInt({1, std::min(3, fx->getNumParams() + 1)});
                for (int p = 0; p < paramsToMap; ++p)
                {
                    Mapping m;
                    m.source = sources[rng.nextInt(11)];
                    m.targetEffectId = static_cast<uint32_t>(idx);
                    m.targetParamIndex = static_cast<uint32_t>(p);
                    m.curve = curves[rng.nextInt(4)];
                    m.outputMin = rng.nextFloat() * 0.3f;
                    m.outputMax = 0.5f + rng.nextFloat() * 0.5f;
                    m.smoothing = rng.nextFloat() * 0.4f;
                    m.enabled = true;
                    mappingEngine_.addMapping(m);
                }
                ++enabled;
            }
        }

        refreshFromChain();
    };

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
    g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
    g.drawText("Effects Rack", getLocalBounds().removeFromTop(24).reduced(6, 0),
               juce::Justification::centredLeft);
}

void EffectsRackPanel::resized()
{
    auto area = getLocalBounds();
    auto titleArea = area.removeFromTop(26);
    randomizeButton_.setBounds(titleArea.removeFromRight(42).reduced(2));
    viewport_.setBounds(area);

    int contentWidth = area.getWidth() - 12;
    int knobsPerRow = juce::jmax(1, contentWidth / Knob::kPreferredWidth);
    int sectionSpacing = 8;

    // Calculate content height
    int contentHeight = 0;
    juce::String lastCat;
    for (const auto& section : sections_)
    {
        auto* effect = effectChain_.getEffect(section->effectIndex);
        if (effect && effect->getCategory() != lastCat)
        {
            lastCat = effect->getCategory();
            contentHeight += 20; // Category header
        }

        contentHeight += 22; // Effect header row
        if (section->enableToggle->getToggleState())
        {
            int numParams = static_cast<int>(section->paramKnobs.size());
            int knobRows = (numParams + knobsPerRow - 1) / knobsPerRow;
            contentHeight += knobRows * (Knob::kPreferredHeight + 18);
        }
        contentHeight += sectionSpacing;
    }

    contentHeight = std::max(contentHeight, area.getHeight());
    contentComponent_.setSize(contentWidth, contentHeight);

    // Layout sections with category headers
    auto contentArea = contentComponent_.getLocalBounds().reduced(2, 0);
    lastCat = {};
    int headerIdx = 0;
    for (const auto& section : sections_)
    {
        auto* effect = effectChain_.getEffect(section->effectIndex);
        if (effect && effect->getCategory() != lastCat)
        {
            lastCat = effect->getCategory();
            if (headerIdx < static_cast<int>(categoryHeaders_.size()))
            {
                auto headerRow = contentArea.removeFromTop(20);
                categoryHeaders_[static_cast<size_t>(headerIdx)]->label->setBounds(headerRow.reduced(4, 0));
                ++headerIdx;
            }
        }

        // Header row: toggle + name + rand button
        auto headerRow = contentArea.removeFromTop(22);
        section->enableToggle->setBounds(headerRow.removeFromLeft(22));
        section->randButton->setBounds(headerRow.removeFromRight(22));
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
                slider.setValue(static_cast<double>(currentValue), juce::dontSendNotification);
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
    categoryHeaders_.clear();
    contentComponent_.removeAllChildren();

    // Category colors
    static const std::map<juce::String, juce::uint32> categoryColors = {
        {"3d",        0xffb088f9},  // Purple
        {"warp",      0xffff6b9d},  // Pink
        {"color",     0xff4ecdc4},  // Teal
        {"glitch",    0xffffe66d},  // Yellow
        {"pattern",   0xffff9f43},  // Orange
        {"animation", 0xff78e08f},  // Green
        {"blend",     0xff82ccdd},  // Sky blue
        {"blur",      0xff95e1d3},  // Mint
    };

    static const std::map<juce::String, juce::String> categoryLabels = {
        {"3d",        "3D / DEPTH"},
        {"warp",      "WARP"},
        {"color",     "COLOR"},
        {"glitch",    "GLITCH"},
        {"pattern",   "PATTERN / STYLE"},
        {"animation", "ANIMATION"},
        {"blend",     "BLEND / COMPOSITE"},
        {"blur",      "BLUR / POST"},
    };

    juce::String lastCategory;
    int numEffects = effectChain_.getNumEffects();
    for (int ei = 0; ei < numEffects; ++ei)
    {
        auto* effect = effectChain_.getEffect(ei);
        if (effect == nullptr)
            continue;

        // Insert category header when category changes
        if (effect->getCategory() != lastCategory)
        {
            lastCategory = effect->getCategory();
            auto header = std::make_unique<CategoryHeader>();
            header->category = lastCategory;
            header->label = std::make_unique<juce::Label>();

            auto labelIt = categoryLabels.find(lastCategory);
            header->label->setText(labelIt != categoryLabels.end() ? labelIt->second : lastCategory.toUpperCase(),
                                   juce::dontSendNotification);
            header->label->setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));

            auto colorIt = categoryColors.find(lastCategory);
            juce::uint32 col = (colorIt != categoryColors.end()) ? colorIt->second : AudioDNALookAndFeel::kAccentCyan;
            header->label->setColour(juce::Label::textColourId, juce::Colour(col));

            contentComponent_.addAndMakeVisible(header->label.get());
            categoryHeaders_.push_back(std::move(header));
        }

        auto section = std::make_unique<EffectSection>();
        section->effectIndex = ei;

        // Effect name label
        section->nameLabel = std::make_unique<juce::Label>();
        section->nameLabel->setText(effect->getName(), juce::dontSendNotification);
        section->nameLabel->setFont(juce::Font(juce::FontOptions(13.0f).withStyle("Bold")));
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

        // Per-effect randomize button
        section->randButton = std::make_unique<juce::TextButton>("R");
        section->randButton->setColour(juce::TextButton::buttonColourId,
                                        juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
        contentComponent_.addAndMakeVisible(section->randButton.get());

        section->randButton->onClick = [this, capturedEi]
        {
            auto* eff = effectChain_.getEffect(capturedEi);
            if (!eff) return;

            juce::Random rng;

            // Randomize all params
            for (int p = 0; p < eff->getNumParams(); ++p)
                eff->setParamValue(p, rng.nextFloat());

            // Randomize mappings for this effect
            static const MappingSource sources[] = {
                MappingSource::RMS, MappingSource::Peak, MappingSource::SpectralCentroid,
                MappingSource::SpectralFlux, MappingSource::BandSub, MappingSource::BandBass,
                MappingSource::BandMid, MappingSource::BandHighMid, MappingSource::OnsetStrength,
                MappingSource::BeatPhase, MappingSource::TransientDensity, MappingSource::HarmonicChange
            };
            static const MappingCurve curves[] = {
                MappingCurve::Linear, MappingCurve::Exponential,
                MappingCurve::Logarithmic, MappingCurve::SCurve
            };

            // Remove existing mappings for this effect
            for (int i = mappingEngine_.getNumMappings() - 1; i >= 0; --i)
            {
                auto* m = mappingEngine_.getMapping(i);
                if (m && static_cast<int>(m->targetEffectId) == capturedEi)
                    mappingEngine_.removeMapping(i);
            }

            // Create random mappings for 1-2 params
            int paramsToMap = rng.nextInt({1, std::min(3, eff->getNumParams() + 1)});
            for (int p = 0; p < paramsToMap; ++p)
            {
                Mapping m;
                m.source = sources[rng.nextInt(12)];
                m.targetEffectId = static_cast<uint32_t>(capturedEi);
                m.targetParamIndex = static_cast<uint32_t>(p);
                m.curve = curves[rng.nextInt(4)];
                m.outputMin = rng.nextFloat() * 0.3f;
                m.outputMax = 0.5f + rng.nextFloat() * 0.5f;
                m.smoothing = rng.nextFloat() * 0.4f;
                m.enabled = true;
                mappingEngine_.addMapping(m);
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
            pk->knob->getSlider().setValue(static_cast<double>(param.value), juce::dontSendNotification);
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
