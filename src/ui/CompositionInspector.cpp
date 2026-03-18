#include "ui/CompositionInspector.h"

CompositionInspector::CompositionInspector()
{
    addAndMakeVisible(macroPanel_);
    addAndMakeVisible(effectStackView_);

    // Master opacity
    masterOpacitySlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    masterOpacitySlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    masterOpacitySlider_.setRange(0.0, 1.0, 0.01);
    masterOpacitySlider_.setValue(1.0, juce::dontSendNotification);
    masterOpacitySlider_.setColour(juce::Slider::thumbColourId,
                                    juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    masterOpacitySlider_.onValueChange = [this] {
        if (composition_)
            composition_->masterOpacity = static_cast<float>(masterOpacitySlider_.getValue());
    };
    addAndMakeVisible(masterOpacitySlider_);

    // Transition speed
    transitionSpeedSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    transitionSpeedSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    transitionSpeedSlider_.setRange(0.0, 5.0, 0.01);
    transitionSpeedSlider_.setValue(0.3, juce::dontSendNotification);
    transitionSpeedSlider_.setColour(juce::Slider::thumbColourId,
                                      juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    transitionSpeedSlider_.onValueChange = [this] {
        if (composition_)
            composition_->globalTransitionSpeed =
                static_cast<float>(transitionSpeedSlider_.getValue());
    };
    addAndMakeVisible(transitionSpeedSlider_);

    // Resolution selector
    resolutionSelector_.addItem("1920x1080", 1);
    resolutionSelector_.addItem("1280x720", 2);
    resolutionSelector_.addItem("2560x1440", 3);
    resolutionSelector_.addItem("3840x2160", 4);
    resolutionSelector_.setSelectedId(1, juce::dontSendNotification);
    resolutionSelector_.onChange = [this] {
        if (!composition_) return;
        switch (resolutionSelector_.getSelectedId())
        {
            case 1: composition_->outputWidth = 1920; composition_->outputHeight = 1080; break;
            case 2: composition_->outputWidth = 1280; composition_->outputHeight = 720; break;
            case 3: composition_->outputWidth = 2560; composition_->outputHeight = 1440; break;
            case 4: composition_->outputWidth = 3840; composition_->outputHeight = 2160; break;
        }
    };
    addAndMakeVisible(resolutionSelector_);
}

void CompositionInspector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    int y = MacroPanel::kPreferredHeight + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "GLOBAL EFFECTS");
    y += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "MASTER OPACITY");
    y += kSectionHeaderHeight + kRowHeight + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "TRANSITION SPEED");
    y += kSectionHeaderHeight + kRowHeight + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "OUTPUT SETTINGS");
}

void CompositionInspector::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    int y = 0;

    // Macros
    macroPanel_.setBounds(area.getX(), y, area.getWidth(), MacroPanel::kPreferredHeight);
    y += MacroPanel::kPreferredHeight + kSectionGap;

    // Global effects
    y += kSectionHeaderHeight;
    int fxHeight = effectStackView_.getPreferredHeight();
    effectStackView_.setBounds(area.getX(), y, area.getWidth(), fxHeight);
    y += fxHeight + kSectionGap;

    // Master opacity
    y += kSectionHeaderHeight;
    masterOpacitySlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight + kSectionGap;

    // Transition speed
    y += kSectionHeaderHeight;
    transitionSpeedSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight + kSectionGap;

    // Output settings
    y += kSectionHeaderHeight;
    resolutionSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
}

void CompositionInspector::setComposition(Composition* comp)
{
    composition_ = comp;
    if (comp)
    {
        effectStackView_.setEffects(&comp->globalEffects);
        syncFromComposition();
    }
    else
    {
        effectStackView_.setEffects(nullptr);
    }
    resized();
    repaint();
}

void CompositionInspector::setEffectLibrary(EffectLibrary* lib)
{
    effectStackView_.setEffectLibrary(lib);
}

void CompositionInspector::setSignalRegistry(SignalRegistry* reg)
{
    macroPanel_.setSignalRegistry(reg);
    effectStackView_.setSignalRegistry(reg);
}

void CompositionInspector::setMacroBank(MacroBank* bank)
{
    macroPanel_.setMacroBank(bank);
}

void CompositionInspector::refresh()
{
    if (composition_) syncFromComposition();
    effectStackView_.refresh();
    macroPanel_.refresh();
    repaint();
}

int CompositionInspector::getPreferredHeight() const
{
    int h = MacroPanel::kPreferredHeight + kSectionGap;
    h += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap;
    h += kSectionHeaderHeight + kRowHeight + kSectionGap; // opacity
    h += kSectionHeaderHeight + kRowHeight + kSectionGap; // transition
    h += kSectionHeaderHeight + kRowHeight + 8; // output
    return h;
}

void CompositionInspector::paintSectionHeader(juce::Graphics& g,
                                               const juce::Rectangle<int>& bounds,
                                               const juce::String& title)
{
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(bounds);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)).boldened());
    g.drawText(title, bounds.withTrimmedLeft(4), juce::Justification::centredLeft, false);
}

void CompositionInspector::syncFromComposition()
{
    if (!composition_) return;

    masterOpacitySlider_.setValue(static_cast<double>(composition_->masterOpacity),
                                  juce::dontSendNotification);
    transitionSpeedSlider_.setValue(static_cast<double>(composition_->globalTransitionSpeed),
                                    juce::dontSendNotification);

    // Match resolution selector
    if (composition_->outputWidth == 1920) resolutionSelector_.setSelectedId(1, juce::dontSendNotification);
    else if (composition_->outputWidth == 1280) resolutionSelector_.setSelectedId(2, juce::dontSendNotification);
    else if (composition_->outputWidth == 2560) resolutionSelector_.setSelectedId(3, juce::dontSendNotification);
    else if (composition_->outputWidth == 3840) resolutionSelector_.setSelectedId(4, juce::dontSendNotification);
}
