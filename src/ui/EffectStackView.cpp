#include "ui/EffectStackView.h"

EffectStackView::EffectStackView()
{
    setOpaque(false);
}

void EffectStackView::paint(juce::Graphics& g)
{
    if (!effects_) return;

    for (size_t i = 0; i < rows_.size(); ++i)
    {
        auto& row = *rows_[i];
        auto hdr = row.headerBounds.toFloat();

        // Header background
        bool isBypassed = false;
        if (row.effectIndex < static_cast<int>(effects_->size()))
            isBypassed = (*effects_)[static_cast<size_t>(row.effectIndex)].bypassed;

        g.setColour(isBypassed ? juce::Colour(0xff2a2020) : juce::Colour(0xff2a2a2a));
        g.fillRect(hdr);

        // Border
        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(hdr, 1.0f);

        if (row.effectIndex < static_cast<int>(effects_->size()))
        {
            auto& fx = (*effects_)[static_cast<size_t>(row.effectIndex)];

            // Effect name
            g.setColour(isBypassed ? juce::Colour(AudioDNALookAndFeel::kTextSecondary)
                                   : juce::Colour(AudioDNALookAndFeel::kTextPrimary));
            g.setFont(juce::Font(juce::FontOptions(12.0f)));
            g.drawText(juce::String(fx.effectName),
                       row.headerBounds.withTrimmedLeft(30),
                       juce::Justification::centredLeft, true);

            // Main value (first param) on the right when collapsed
            if (!row.expanded && !fx.paramValues.empty())
            {
                g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
                g.setFont(juce::Font(juce::FontOptions(11.0f)));
                g.drawText(juce::String(fx.paramValues[0], 2),
                           row.headerBounds.withTrimmedRight(6),
                           juce::Justification::centredRight, false);
            }

            // Expand indicator
            g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
            auto indicatorX = hdr.getRight() - 16.0f;
            auto indicatorY = hdr.getCentreY();
            if (row.expanded)
            {
                // Down arrow
                juce::Path arrow;
                arrow.addTriangle(indicatorX - 3.0f, indicatorY - 2.0f,
                                  indicatorX + 3.0f, indicatorY - 2.0f,
                                  indicatorX, indicatorY + 3.0f);
                g.fillPath(arrow);
            }
            else
            {
                // Right arrow
                juce::Path arrow;
                arrow.addTriangle(indicatorX - 2.0f, indicatorY - 3.0f,
                                  indicatorX - 2.0f, indicatorY + 3.0f,
                                  indicatorX + 3.0f, indicatorY);
                g.fillPath(arrow);
            }
        }
    }
}

void EffectStackView::resized()
{
    auto area = getLocalBounds();
    int y = 0;

    for (size_t i = 0; i < rows_.size(); ++i)
    {
        auto& row = *rows_[i];

        // Header
        row.headerBounds = juce::Rectangle<int>(0, y, area.getWidth(), kHeaderHeight);
        row.bypassBtn.setBounds(2, y + 3, 24, kHeaderHeight - 6);
        y += kHeaderHeight;

        // Param controls (if expanded)
        if (row.expanded)
        {
            for (auto& pc : row.paramControls)
            {
                int pcHeight = pc->getPreferredHeight();
                pc->setBounds(kParamIndent, y, area.getWidth() - kParamIndent - 4, pcHeight);
                pc->setVisible(true);
                y += pcHeight;
            }
        }
        else
        {
            for (auto& pc : row.paramControls)
                pc->setVisible(false);
        }
    }
}

void EffectStackView::setEffects(std::vector<Clip::EffectSlot>* effects)
{
    effects_ = effects;
    rebuildRows();
}

void EffectStackView::refresh()
{
    if (!effects_) return;

    for (size_t i = 0; i < rows_.size(); ++i)
    {
        auto& row = *rows_[i];
        if (row.effectIndex >= static_cast<int>(effects_->size())) continue;

        auto& fx = (*effects_)[static_cast<size_t>(row.effectIndex)];

        // Update bypass button color
        row.bypassBtn.setColour(juce::TextButton::buttonColourId,
            fx.bypassed ? juce::Colour(0xff6a3a3a) : juce::Colour(0xff333333));

        // Update param values
        for (size_t p = 0; p < row.paramControls.size() && p < fx.paramValues.size(); ++p)
            row.paramControls[p]->setParamValue(fx.paramValues[p]);
    }

    repaint();
}

int EffectStackView::getPreferredHeight() const
{
    int h = 0;
    for (auto& row : rows_)
    {
        h += kHeaderHeight;
        if (row->expanded)
        {
            for (auto& pc : row->paramControls)
                h += pc->getPreferredHeight();
        }
    }
    return std::max(h, 20);
}

void EffectStackView::rebuildRows()
{
    // Remove all children first
    for (auto& row : rows_)
    {
        removeChildComponent(&row->bypassBtn);
        for (auto& pc : row->paramControls)
            removeChildComponent(pc.get());
    }
    rows_.clear();

    if (!effects_) return;

    for (int i = 0; i < static_cast<int>(effects_->size()); ++i)
    {
        auto& fx = (*effects_)[static_cast<size_t>(i)];
        auto row = std::make_unique<EffectRow>();
        row->effectIndex = i;
        row->expanded = false;

        // Bypass button
        row->bypassBtn.setColour(juce::TextButton::buttonColourId,
            fx.bypassed ? juce::Colour(0xff6a3a3a) : juce::Colour(0xff333333));
        row->bypassBtn.setColour(juce::TextButton::textColourOffId,
            juce::Colour(AudioDNALookAndFeel::kTextPrimary));

        int capturedIndex = i;
        row->bypassBtn.onClick = [this, capturedIndex] {
            if (!effects_ || capturedIndex >= static_cast<int>(effects_->size())) return;
            auto& slot = (*effects_)[static_cast<size_t>(capturedIndex)];
            slot.bypassed = !slot.bypassed;
            if (onBypassChanged) onBypassChanged(capturedIndex, slot.bypassed);
            refresh();
        };
        addAndMakeVisible(row->bypassBtn);

        // Create param controls from effect library definition
        const EffectLibrary::EffectDef* def = nullptr;
        if (effectLibrary_)
            def = effectLibrary_->getEffectDef(juce::String(fx.effectName));

        int numParams = static_cast<int>(fx.paramValues.size());
        for (int p = 0; p < numParams; ++p)
        {
            auto pc = std::make_unique<UniversalParamControl>();

            // Set param name from effect definition if available
            if (def && p < static_cast<int>(def->params.size()))
                pc->setParamName(juce::String(def->params[static_cast<size_t>(p)].name));
            else
                pc->setParamName("Param " + juce::String(p + 1));

            pc->setParamValue(fx.paramValues[static_cast<size_t>(p)]);
            pc->setSignalRegistry(signalRegistry_);

            int capturedFx = i;
            int capturedParam = p;
            pc->onValueChanged = [this, capturedFx, capturedParam](float val) {
                if (!effects_ || capturedFx >= static_cast<int>(effects_->size())) return;
                auto& slot = (*effects_)[static_cast<size_t>(capturedFx)];
                if (capturedParam < static_cast<int>(slot.paramValues.size()))
                    slot.paramValues[static_cast<size_t>(capturedParam)] = val;
                if (onParamChanged) onParamChanged(capturedFx, capturedParam, val);
            };

            pc->onExpandToggled = [this] {
                resized();
                if (auto* parent = getParentComponent())
                    parent->resized();
            };

            addChildComponent(pc.get());
            row->paramControls.push_back(std::move(pc));
        }

        rows_.push_back(std::move(row));
    }

    if (getWidth() > 0)
        resized();
}

void EffectStackView::toggleExpand(int rowIndex)
{
    if (rowIndex < 0 || rowIndex >= static_cast<int>(rows_.size())) return;
    rows_[static_cast<size_t>(rowIndex)]->expanded =
        !rows_[static_cast<size_t>(rowIndex)]->expanded;
    resized();
    if (auto* parent = getParentComponent())
        parent->resized();
    repaint();
}

void EffectStackView::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.position.toInt();

    // Check if click is on a header (but not on bypass button)
    for (size_t i = 0; i < rows_.size(); ++i)
    {
        if (rows_[i]->headerBounds.contains(pos) && pos.x > 28)
        {
            toggleExpand(static_cast<int>(i));
            return;
        }
    }

    Component::mouseDown(event);
}
