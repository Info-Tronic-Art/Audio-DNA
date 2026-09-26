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
    // s-rta-0925 mastersignal Step 0: DISPLAY-ONLY. The model write used to
    // happen here (assigning signalValue straight into the slot's param
    // vector), reading through UniversalParamControl's own SourceMode/
    // getSourceName lookup and
    // bypassing ConnectionShaper (RANGE/INVERT/curve ignored) and every
    // source kind tickModulation() didn't special-case (BPM Sync, Clip
    // Position). The engine (ConnectionEngine::tick, via each row's bound
    // fx.paramConns[p]/fx.dryWetConn) is now the ONLY writer of
    // paramValues/dryWet's live twins; this just pushes fx.effParam(p)/
    // fx.effDryWet() to the display when a row is bound.
    if (!effects_) return;

    for (size_t i = 0; i < rows_.size(); ++i)
    {
        auto& row = *rows_[i];
        if (row.effectIndex >= static_cast<int>(effects_->size())) continue;

        auto& fx = (*effects_)[static_cast<size_t>(row.effectIndex)];

        if (row.dryWetControl && row.dryWetControl->isConnected())
            row.dryWetControl->setParamValue(fx.effDryWet());

        for (size_t p = 0; p < row.paramControls.size() && p < fx.paramValues.size(); ++p)
        {
            auto& pc = *row.paramControls[p];
            if (!pc.isConnected()) continue;

            const float v = fx.effParam(p);

            // Cost trap (L9): diff against the LAST VALUE ACTUALLY PUSHED to
            // the display (row.lastPushedValue[p]) so a slow modulator isn't
            // suppressed forever by a per-tick epsilon check. lastPushedValue
            // starts nullopt (set in rebuildRows()) so the first tick after a
            // rebuild always pushes, regardless of what value it computes.
            bool changed = p >= row.lastPushedValue.size()
                || !row.lastPushedValue[p].has_value()
                || std::abs(v - *row.lastPushedValue[p]) > kModulationChangeEpsilon;

            if (changed && pc.isVisible())
            {
                pc.setSourceValue(v);
                pc.setParamValue(v);
                if (p < row.lastPushedValue.size())
                    row.lastPushedValue[p] = v;
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

        // Update dry/wet control -- fx.effDryWet() (s-rta-0925 mastersignal
        // Step 0) so a connected dry/wet displays its engine-published value,
        // not the raw manual field.
        if (row.dryWetControl)
            row.dryWetControl->setParamValue(fx.effDryWet());

        // Always update the display from the current effective param value
        // (fx.effParam(p) -- includes whatever tickModulation() just applied
        // for connected params, and the manual value otherwise).
        for (size_t p = 0; p < row.paramControls.size() && p < fx.paramValues.size(); ++p)
            row.paramControls[p]->setParamValue(fx.effParam(p));
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

UniversalParamControl* EffectStackView::paramControlForTest(int effectIndex, int paramIndex) const
{
    for (auto& row : rows_)
        if (row->effectIndex == effectIndex)
            return (paramIndex >= 0 && paramIndex < static_cast<int>(row->paramControls.size()))
                ? row->paramControls[static_cast<size_t>(paramIndex)].get()
                : nullptr;
    return nullptr;
}

UniversalParamControl* EffectStackView::dryWetControlForTest(int effectIndex) const
{
    for (auto& row : rows_)
        if (row->effectIndex == effectIndex)
            return row->dryWetControl.get();
    return nullptr;
}

void EffectStackView::rebuildRows()
{
    // Remove all children first. forgetConnection() FIRST (s-rta-0925
    // mastersignal Step 0): the bound ParamConnection/LiveValue elements live
    // INSIDE the EffectSlot vector this rebuild is about to re-walk (and
    // whose backing effects_ may have just been erased/reallocated by the
    // caller, e.g. the delete button's erase()) -- bindConnection(nullptr,
    // nullptr) and ~UniversalParamControl both dereference the OLD conn_ to
    // release an active grip, which is a use-after-free once that element is
    // gone. forgetConnection() drops the pointer without touching it.
    for (auto& row : rows_)
    {
        if (row->dryWetControl)
            row->dryWetControl->forgetConnection();
        for (auto& pc : row->paramControls)
            pc->forgetConnection();

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
            // GL fence (2026-09-05): same crash-class exposure as the erase/add
            // fences below once EffectSlot gains a non-trivial (string/vector)
            // field — an unfenced toggle would then race the GL thread's
            // by-reference iteration of *effects_. Fence now, before that lands.
            runFenced([this, capturedIndex] {
                auto& s = (*effects_)[static_cast<size_t>(capturedIndex)];
                s.bypassed = !s.bypassed;
            });
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
            // s-rta-0925 mastersignal Step 0: bind to the slot's own
            // connection (element lives inside *effects_, sized 1:1 by
            // construction -- no resizeParams call needed here).
            dwc->bindConnection(&fx.dryWetConn, &fx.dryWetLive);

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
            // s-rta-0925 mastersignal Step 0: bind to this param's own
            // connection/live twin (parallel arrays sized 1:1 with
            // paramValues by construction via EffectSlot::addParam -- no
            // resizeParams call here, an unfenced message-thread reallocation
            // would race the GL thread's effParam() read).
            if (static_cast<size_t>(p) < fx.paramConns.size())
                pc->bindConnection(&fx.paramConns[static_cast<size_t>(p)],
                                   &fx.paramLive[static_cast<size_t>(p)]);

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
                slot.addParam(p.defaultValue);

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
