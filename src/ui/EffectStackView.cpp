#include "ui/EffectStackView.h"

EffectStackView::EffectStackView()
{
    setOpaque(false);
}

void EffectStackView::paint(juce::Graphics& g)
{
    // FX drop highlight
    if (fxDropHighlight_)
    {
        g.setColour(juce::Colour(0xff8866cc).withAlpha(0.15f));
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colour(0xff8866cc));
        g.drawRect(getLocalBounds(), 2);
    }

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
        row.deleteBtn.setBounds(area.getWidth() - 26, y + 3, 24, kHeaderHeight - 6);
        y += kHeaderHeight;

        // Param controls (if expanded)
        if (row.expanded)
        {
            // Dry/wet control first
            if (row.dryWetControl)
            {
                int pcHeight = row.dryWetControl->getPreferredHeight();
                row.dryWetControl->setBounds(kParamIndent, y, area.getWidth() - kParamIndent - 4, pcHeight);
                row.dryWetControl->setVisible(true);
                y += pcHeight;
            }
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
            if (row.dryWetControl)
                row.dryWetControl->setVisible(false);
            for (auto& pc : row.paramControls)
                pc->setVisible(false);
        }
    }
}

void EffectStackView::setEffects(std::vector<Clip::EffectSlot>* effects, EffectScope scope)
{
    effects_ = effects;
    scope_ = scope;
    rebuildRows();
}

void EffectStackView::tickModulation()
{
    if (!effects_) return;

    for (size_t i = 0; i < rows_.size(); ++i)
    {
        auto& row = *rows_[i];
        if (row.effectIndex >= static_cast<int>(effects_->size())) continue;

        auto& fx = (*effects_)[static_cast<size_t>(row.effectIndex)];

        // Apply signal-driven modulation to connected params
        for (size_t p = 0; p < row.paramControls.size() && p < fx.paramValues.size(); ++p)
        {
            auto& pc = *row.paramControls[p];

            // If a signal source is connected, drive the param value from it
            if (!pc.isConnected()) continue;

            auto mode = pc.getSourceMode();
            auto sourceName = pc.getSourceName();
            float signalValue = 0.0f;
            bool found = false;

            if ((mode == UniversalParamControl::SourceMode::Signal
                || mode == UniversalParamControl::SourceMode::Oscillator
                || mode == UniversalParamControl::SourceMode::Envelope)
                && signalRegistry_)
            {
                for (int s = 0; s < signalRegistry_->getNumSignals(); ++s)
                {
                    auto* sig = signalRegistry_->getSignalAt(s);
                    if (sig && juce::String(sig->getName()) == sourceName)
                    {
                        signalValue = signalRegistry_->getCachedValue(sig->getId());
                        found = true;
                        break;
                    }
                }
            }
            else if (mode == UniversalParamControl::SourceMode::Macro && macroBank_)
            {
                // Parse "Macro N" or "Link N" to get the index
                int macroIdx = -1;
                if (sourceName.startsWithIgnoreCase("Macro ") || sourceName.startsWithIgnoreCase("Link "))
                {
                    macroIdx = sourceName.getTrailingIntValue() - 1;
                }
                if (macroIdx >= 0 && macroIdx < MacroBank::kNumMacros)
                {
                    signalValue = macroBank_->getMacroValue(macroIdx);
                    found = true;
                }
            }

            if (found)
            {
                // Render-critical write: unconditional, every tick, regardless
                // of the display gate below — CompositorEngine reads this
                // every GL frame and must never see it suppressed.
                fx.paramValues[p] = signalValue;

                // Cost trap (L9), CORRECTED (fix-round, blocking review
                // finding): diff against the LAST VALUE ACTUALLY PUSHED to
                // the display (row.lastPushedValue[p]), not against
                // fx.paramValues[p] — that field was just overwritten above
                // on this same tick, so comparing against it measured
                // PER-TICK delta: a slow modulator (e.g. a ~60s-period LFO)
                // moves less than kModulationChangeEpsilon in any single
                // 120Hz tick, so the old guard suppressed the display update
                // forever. lastPushedValue starts as nullopt (set in
                // rebuildRows()) so the first tick after a rebuild always
                // pushes, regardless of what value it computes.
                bool changed = p >= row.lastPushedValue.size()
                    || !row.lastPushedValue[p].has_value()
                    || std::abs(signalValue - *row.lastPushedValue[p]) > kModulationChangeEpsilon;

                // setSourceValue() repaints unconditionally, so only pay for
                // it when the displayed value actually moved AND the row is
                // expanded (collapsed rows have their paramControls hidden
                // via setVisible(false), see resized()). A background
                // Inspector tab's own hidden viewport is a separate,
                // higher-up ancestor visibility that JUCE's repaint()/
                // internalRepaint() walk already short-circuits at for free
                // — no extra guard needed for that case here.
                if (changed && pc.isVisible())
                {
                    pc.setSourceValue(signalValue);
                    if (p < row.lastPushedValue.size())
                        row.lastPushedValue[p] = signalValue;
                }

                // Notify renderer
                if (onParamChanged)
                    onParamChanged(row.effectIndex, static_cast<int>(p), signalValue);
            }
        }
    }
}

void EffectStackView::refresh()
{
    if (!effects_) return;

    tickModulation();

    for (size_t i = 0; i < rows_.size(); ++i)
    {
        auto& row = *rows_[i];
        if (row.effectIndex >= static_cast<int>(effects_->size())) continue;

        auto& fx = (*effects_)[static_cast<size_t>(row.effectIndex)];

        // Update bypass button color
        row.bypassBtn.setColour(juce::TextButton::buttonColourId,
            fx.bypassed ? juce::Colour(0xff6a3a3a) : juce::Colour(0xff333333));

        // Update dry/wet control
        if (row.dryWetControl)
            row.dryWetControl->setParamValue(fx.dryWet);

        // Always update the display from the current param value (including
        // whatever tickModulation() just applied)
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
            if (row->dryWetControl)
                h += row->dryWetControl->getPreferredHeight();
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
        removeChildComponent(&row->deleteBtn);
        if (row->dryWetControl)
            removeChildComponent(row->dryWetControl.get());
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
            // #29: capture the whole chain before the toggle so the edit is one
            // undo unit. The live toggle + refresh below are exactly as before.
            std::vector<Clip::EffectSlot> before = *effects_;
            auto& slot = (*effects_)[static_cast<size_t>(capturedIndex)];
            slot.bypassed = !slot.bypassed;
            juce::String fxName = juce::String(slot.effectName);
            bool nowBypassed = slot.bypassed;
            if (onBypassChanged) onBypassChanged(capturedIndex, slot.bypassed);
            refresh();
            if (onPerformEdit)
                onPerformEdit(scope_, std::move(before), *effects_,
                              (nowBypassed ? "Bypass Effect '" : "Enable Effect '")
                                  + fxName + "'");
        };
        addAndMakeVisible(row->bypassBtn);

        // Delete button
        row->deleteBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff333333));
        row->deleteBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcc5555));
        row->deleteBtn.onClick = [this, capturedIndex] {
            if (!effects_ || capturedIndex >= static_cast<int>(effects_->size())) return;
            // #28: capture the whole chain before the erase for one undo unit; the
            // live erase + rebuild/repaint below are exactly as before.
            std::vector<Clip::EffectSlot> before = *effects_;
            juce::String fxName =
                juce::String((*effects_)[static_cast<size_t>(capturedIndex)].effectName);
            // GL fence (2026-07-28): erase reallocates *effects_ — same
            // crash-class exposure as the structural deck/clip commands.
            runFenced([this, capturedIndex] { effects_->erase(effects_->begin() + capturedIndex); });
            if (onEffectRemoved) onEffectRemoved(capturedIndex);
            rebuildRows();
            resized();
            if (auto* parent = getParentComponent()) parent->resized();
            repaint();
            if (onPerformEdit)
                onPerformEdit(scope_, std::move(before), *effects_,
                              "Remove Effect '" + fxName + "'");
        };
        addAndMakeVisible(row->deleteBtn);

        // Create dry/wet control (first param when expanded)
        {
            auto dwc = std::make_unique<UniversalParamControl>();
            dwc->setParamName("Dry/Wet");
            dwc->setParamValue(fx.dryWet);
            dwc->setDefaultValue(1.0f);
            dwc->setSignalRegistry(signalRegistry_);

            int capturedFxIdx = i;
            dwc->onValueChanged = [this, capturedFxIdx](float val) {
                if (!effects_ || capturedFxIdx >= static_cast<int>(effects_->size())) return;
                (*effects_)[static_cast<size_t>(capturedFxIdx)].dryWet = val;
                if (onDryWetChanged) onDryWetChanged(capturedFxIdx, val);
            };
            dwc->onExpandToggled = [this] {
                resized();
                if (auto* parent = getParentComponent())
                    parent->resized();
            };

            addChildComponent(dwc.get());
            row->dryWetControl = std::move(dwc);
        }

        // Create param controls from effect library definition
        const EffectLibrary::EffectDef* def = nullptr;
        if (effectLibrary_)
            def = effectLibrary_->getEffectDef(juce::String(fx.effectName));

        int numParams = static_cast<int>(fx.paramValues.size());

        // L9 fix-round: parallel to paramControls, one nullopt slot per
        // param — "never pushed yet" so tickModulation()'s first tick after
        // this rebuild always pushes regardless of the value it computes.
        row->lastPushedValue.assign(static_cast<size_t>(numParams), std::nullopt);

        for (int p = 0; p < numParams; ++p)
        {
            auto pc = std::make_unique<UniversalParamControl>();

            // Set param name and default value from effect definition if available
            if (def && p < static_cast<int>(def->params.size()))
            {
                pc->setParamName(juce::String(def->params[static_cast<size_t>(p)].name));
                pc->setDefaultValue(def->params[static_cast<size_t>(p)].defaultValue);
            }
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

// === DragAndDropTarget (FX drops from browser) ===

bool EffectStackView::isInterestedInDragSource(const SourceDetails& details)
{
    return details.description.toString().startsWith("fx:");
}

void EffectStackView::itemDragEnter(const SourceDetails&)
{
    fxDropHighlight_ = true;
    repaint();
}

void EffectStackView::itemDragExit(const SourceDetails&)
{
    fxDropHighlight_ = false;
    repaint();
}

void EffectStackView::itemDropped(const SourceDetails& details)
{
    fxDropHighlight_ = false;
    repaint();

    auto desc = details.description.toString();
    if (!desc.startsWith("fx:") || effects_ == nullptr || effectLibrary_ == nullptr)
        return;

    // #27: capture the whole chain before appending, for one undo unit (covers the
    // append-to-chain branch — a drop always push_back()s onto the current chain).
    std::vector<Clip::EffectSlot> before = *effects_;

    // Support comma-separated multi-select drops: "fx:Echo,Ripple,Freeze"
    auto namesList = desc.substring(3);
    auto names = juce::StringArray::fromTokens(namesList, ",", "");

    bool anyAdded = false;
    int addedCount = 0;
    juce::String lastAddedName;
    // GL fence (2026-07-28): push_back below reallocates *effects_ — same
    // crash-class exposure as the structural deck/clip commands. ONE fence for
    // the whole drop gesture (a multi-select drop adds several effects in this
    // one loop), not per effect.
    runFenced([&]
    {
        for (const auto& effectName : names)
        {
            auto trimmed = effectName.trim();
            const auto* def = effectLibrary_->getEffectDef(trimmed);
            if (def == nullptr)
                continue;

            Clip::EffectSlot slot;
            slot.effectName = trimmed.toStdString();
            for (const auto& p : def->params)
                slot.paramValues.push_back(p.defaultValue);

            effects_->push_back(slot);
            anyAdded = true;
            ++addedCount;
            lastAddedName = trimmed;

            if (onEffectAdded)
                onEffectAdded(trimmed);
        }
    });

    if (anyAdded)
    {
        rebuildRows();
        resized();

        if (auto* parent = getParentComponent())
            parent->resized();

        if (onPerformEdit)
        {
            juce::String d = (addedCount == 1)
                ? "Add Effect '" + lastAddedName + "'"
                : "Add " + juce::String(addedCount) + " Effects";
            onPerformEdit(scope_, std::move(before), *effects_, d);
        }
    }
}
