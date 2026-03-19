#include "ui/MacroPanel.h"

MacroPanel::MacroPanel()
{
    for (int i = 0; i < MacroBank::kNumMacros; ++i)
    {
        auto& slot = slots_[static_cast<size_t>(i)];

        slot.knob = std::make_unique<Knob>("Link " + juce::String(i + 1));
        slot.knob->getSlider().setValue(0.5, juce::dontSendNotification);

        int capturedIdx = i;
        slot.knob->getSlider().onValueChange = [this, capturedIdx] {
            float val = static_cast<float>(slots_[static_cast<size_t>(capturedIdx)]
                                               .knob->getSlider().getValue());
            if (macroBank_)
                macroBank_->getMacro(capturedIdx).manualValue = val;
            if (onMacroValueChanged) onMacroValueChanged(capturedIdx, val);
        };
        addAndMakeVisible(slot.knob.get());

        slot.sourceBtn = std::make_unique<juce::TextButton>("Manual");
        slot.sourceBtn->setColour(juce::TextButton::buttonColourId,
                                  juce::Colour(AudioDNALookAndFeel::kSurface));
        slot.sourceBtn->setColour(juce::TextButton::textColourOffId,
                                  juce::Colour(AudioDNALookAndFeel::kTextSecondary));

        slot.sourceBtn->onClick = [this, capturedIdx] {
            showSourcePicker(capturedIdx);
        };
        addAndMakeVisible(slot.sourceBtn.get());
    }
}

void MacroPanel::paint(juce::Graphics& g)
{
    // Section header
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText("DASHBOARD", getLocalBounds().removeFromTop(14),
               juce::Justification::centredLeft, false);
}

void MacroPanel::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(14); // header

    int knobWidth = area.getWidth() / MacroBank::kNumMacros;
    int srcBtnHeight = 16;

    for (int i = 0; i < MacroBank::kNumMacros; ++i)
    {
        auto& slot = slots_[static_cast<size_t>(i)];
        auto col = area.removeFromLeft(knobWidth);

        auto btnArea = col.removeFromBottom(srcBtnHeight);
        slot.sourceBtn->setBounds(btnArea.reduced(2, 0));
        slot.knob->setBounds(col.reduced(2, 0));
    }
}

void MacroPanel::setMacroBank(MacroBank* bank)
{
    macroBank_ = bank;
    refresh();
}

void MacroPanel::refresh()
{
    if (!macroBank_) return;

    // Update macro values from signal sources
    if (signalRegistry_)
        macroBank_->updateValues(*signalRegistry_);

    for (int i = 0; i < MacroBank::kNumMacros; ++i)
    {
        auto& macro = macroBank_->getMacro(i);
        auto& slot = slots_[static_cast<size_t>(i)];

        slot.knob->setParamName(juce::String(macro.name));

        if (macro.isManual())
        {
            slot.knob->getSlider().setValue(static_cast<double>(macro.manualValue),
                                             juce::dontSendNotification);
            slot.sourceBtn->setButtonText("Manual");
            slot.knob->setMappingIndicator({});
        }
        else
        {
            // Show the computed value (driven by signal), not the manual value
            slot.knob->getSlider().setValue(static_cast<double>(macro.currentValue),
                                             juce::dontSendNotification);
            juce::String sigName = "Signal";
            if (signalRegistry_)
            {
                if (auto* sig = signalRegistry_->getSignal(macro.sourceSignalId))
                    sigName = juce::String(sig->getName());
            }
            slot.sourceBtn->setButtonText(sigName);
            slot.knob->setMappingIndicator(sigName);
        }
    }
}

void MacroPanel::showSourcePicker(int macroIndex)
{
    juce::PopupMenu menu;
    menu.addItem(1, "Manual");

    if (signalRegistry_)
    {
        juce::PopupMenu signalMenu;
        for (int i = 0; i < signalRegistry_->getNumSignals(); ++i)
        {
            auto* sig = signalRegistry_->getSignalAt(i);
            if (sig)
                signalMenu.addItem(100 + i, juce::String(sig->getName()));
        }
        menu.addSubMenu("Signals", signalMenu);
    }

    auto& btn = slots_[static_cast<size_t>(macroIndex)].sourceBtn;
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(btn.get()),
        [this, macroIndex](int result) {
            if (result == 0) return;

            if (!macroBank_) return;
            auto& macro = macroBank_->getMacro(macroIndex);

            if (result == 1)
            {
                macro.sourceSignalId = 0;
                if (onMacroSourceChanged) onMacroSourceChanged(macroIndex, 0);
            }
            else if (result >= 100 && signalRegistry_)
            {
                int sigIdx = result - 100;
                if (auto* sig = signalRegistry_->getSignalAt(sigIdx))
                {
                    macro.sourceSignalId = sig->getId();
                    if (onMacroSourceChanged)
                        onMacroSourceChanged(macroIndex, sig->getId());
                }
            }

            refresh();
        });
}
