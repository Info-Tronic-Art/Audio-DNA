#include "ui/ClipInspector.h"

ClipInspector::ClipInspector()
{
    addAndMakeVisible(macroPanel_);

    // Transport mode
    transportModeSelector_.addItem("Timeline", 1);
    transportModeSelector_.addItem("BPM Sync", 2);
    transportModeSelector_.setSelectedId(1, juce::dontSendNotification);
    transportModeSelector_.onChange = [this] {
        if (!clip_) return;
        clip_->transportMode = transportModeSelector_.getSelectedId() == 2
            ? Clip::TransportMode::BPMSync : Clip::TransportMode::Timeline;
    };
    addAndMakeVisible(transportModeSelector_);

    // Loop mode
    loopModeSelector_.addItem("Loop", 1);
    loopModeSelector_.addItem("Ping Pong", 2);
    loopModeSelector_.addItem("One Shot", 3);
    loopModeSelector_.setSelectedId(1, juce::dontSendNotification);
    loopModeSelector_.onChange = [this] {
        if (!clip_) return;
        int sel = loopModeSelector_.getSelectedId();
        if (sel == 1) clip_->loopMode = Clip::LoopMode::Loop;
        else if (sel == 2) clip_->loopMode = Clip::LoopMode::PingPong;
        else if (sel == 3) clip_->loopMode = Clip::LoopMode::OneShot;
    };
    addAndMakeVisible(loopModeSelector_);

    // Speed slider
    speedSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    speedSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    speedSlider_.setRange(0.0, 4.0, 0.01);
    speedSlider_.setValue(1.0, juce::dontSendNotification);
    speedSlider_.setColour(juce::Slider::thumbColourId,
                           juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    speedSlider_.onValueChange = [this] {
        if (clip_) clip_->speed = static_cast<float>(speedSlider_.getValue());
    };
    addAndMakeVisible(speedSlider_);

    // Reverse button
    reverseBtn_.setColour(juce::TextButton::buttonColourId,
                          juce::Colour(AudioDNALookAndFeel::kSurface));
    reverseBtn_.onClick = [this] {
        if (!clip_) return;
        clip_->reverse = !clip_->reverse;
        reverseBtn_.setColour(juce::TextButton::buttonColourId,
            clip_->reverse ? juce::Colour(0xff4a4a2a) : juce::Colour(AudioDNALookAndFeel::kSurface));
    };
    addAndMakeVisible(reverseBtn_);

    // Speed multiplier buttons
    halfSpeedBtn_.setColour(juce::TextButton::buttonColourId,
                            juce::Colour(AudioDNALookAndFeel::kSurface));
    halfSpeedBtn_.onClick = [this] {
        if (!clip_) return;
        clip_->speed = std::max(0.01f, clip_->speed * 0.5f);
        speedSlider_.setValue(static_cast<double>(clip_->speed), juce::dontSendNotification);
    };
    addAndMakeVisible(halfSpeedBtn_);

    doubleSpeedBtn_.setColour(juce::TextButton::buttonColourId,
                              juce::Colour(AudioDNALookAndFeel::kSurface));
    doubleSpeedBtn_.onClick = [this] {
        if (!clip_) return;
        clip_->speed = std::min(4.0f, clip_->speed * 2.0f);
        speedSlider_.setValue(static_cast<double>(clip_->speed), juce::dontSendNotification);
    };
    addAndMakeVisible(doubleSpeedBtn_);

    // Autopilot
    autopilotActionSelector_.addItem("Layer Determined", 1);
    autopilotActionSelector_.addItem("Do Nothing", 2);
    autopilotActionSelector_.addItem("Play Next", 3);
    autopilotActionSelector_.addItem("Play Previous", 4);
    autopilotActionSelector_.addItem("Play Random", 5);
    autopilotActionSelector_.addItem("Play First", 6);
    autopilotActionSelector_.addItem("Play Last", 7);
    autopilotActionSelector_.addItem("Play Specific", 8);
    autopilotActionSelector_.setSelectedId(1, juce::dontSendNotification);
    autopilotActionSelector_.onChange = [this] {
        if (!clip_) return;
        int sel = autopilotActionSelector_.getSelectedId();
        if (sel >= 1 && sel <= 8)
            clip_->autopilotAction = static_cast<Clip::AutopilotAction>(sel - 1);
    };
    addAndMakeVisible(autopilotActionSelector_);

    autopilotDurationSelector_.addItem("Layer Determined", 1);
    autopilotDurationSelector_.addItem("1 Beat", 2);
    autopilotDurationSelector_.addItem("2 Beats", 3);
    autopilotDurationSelector_.addItem("4 Beats", 4);
    autopilotDurationSelector_.addItem("8 Beats", 5);
    autopilotDurationSelector_.addItem("16 Beats", 6);
    autopilotDurationSelector_.addItem("32 Beats", 7);
    autopilotDurationSelector_.setSelectedId(1, juce::dontSendNotification);
    autopilotDurationSelector_.onChange = [this] {
        if (!clip_) return;
        int sel = autopilotDurationSelector_.getSelectedId();
        if (sel >= 1 && sel <= 7)
            clip_->autopilotDuration = static_cast<Clip::AutopilotDuration>(sel - 1);
    };
    addAndMakeVisible(autopilotDurationSelector_);

    // Beat snap
    beatSnapToggle_.setColour(juce::ToggleButton::textColourId,
                              juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    beatSnapToggle_.onStateChange = [this] {
        if (clip_) clip_->beatSnap = beatSnapToggle_.getToggleState();
    };
    addAndMakeVisible(beatSnapToggle_);

    // Effect stack
    addAndMakeVisible(effectStackView_);

    // Cuepoint buttons
    for (int i = 0; i < kNumCuepoints; ++i)
    {
        cuepointBtns_[static_cast<size_t>(i)] =
            std::make_unique<juce::TextButton>(juce::String(i + 1));
        cuepointBtns_[static_cast<size_t>(i)]->setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(AudioDNALookAndFeel::kSurface));
        addAndMakeVisible(cuepointBtns_[static_cast<size_t>(i)].get());
    }
}

void ClipInspector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    auto area = getLocalBounds();

    if (!clip_)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.drawText("No clip selected", area, juce::Justification::centred, false);
        return;
    }

    // Section headers are painted at fixed positions
    int y = MacroPanel::kPreferredHeight + kSectionGap;
    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "TRANSPORT");

    y += kSectionHeaderHeight + kRowHeight * 3 + kSectionGap;
    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "AUTOPILOT");

    y += kSectionHeaderHeight + kRowHeight * 2 + kSectionGap;
    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "BEAT SNAP");

    y += kSectionHeaderHeight + kRowHeight + kSectionGap;
    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "EFFECTS");

    y += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap;
    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "CUEPOINTS");
}

void ClipInspector::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    int y = 0;

    // Macros
    macroPanel_.setBounds(area.getX(), y, area.getWidth(), MacroPanel::kPreferredHeight);
    y += MacroPanel::kPreferredHeight + kSectionGap;

    // Transport header
    y += kSectionHeaderHeight;

    // Transport mode + loop mode row
    auto transportRow1 = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
    transportModeSelector_.setBounds(transportRow1.removeFromLeft(area.getWidth() / 2 - 2));
    transportRow1.removeFromLeft(4);
    loopModeSelector_.setBounds(transportRow1);
    y += kRowHeight;

    // Speed row
    auto speedRow = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
    halfSpeedBtn_.setBounds(speedRow.removeFromLeft(30));
    speedRow.removeFromLeft(2);
    doubleSpeedBtn_.setBounds(speedRow.removeFromLeft(30));
    speedRow.removeFromLeft(4);
    reverseBtn_.setBounds(speedRow.removeFromRight(60));
    speedRow.removeFromRight(4);
    speedSlider_.setBounds(speedRow);
    y += kRowHeight;

    // Extra transport row (placeholder for scrubber)
    y += kRowHeight;
    y += kSectionGap;

    // Autopilot header
    y += kSectionHeaderHeight;

    auto apRow1 = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
    autopilotActionSelector_.setBounds(apRow1);
    y += kRowHeight;

    auto apRow2 = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
    autopilotDurationSelector_.setBounds(apRow2);
    y += kRowHeight;
    y += kSectionGap;

    // Beat Snap header
    y += kSectionHeaderHeight;
    beatSnapToggle_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    y += kSectionGap;

    // Effects header
    y += kSectionHeaderHeight;
    int fxHeight = effectStackView_.getPreferredHeight();
    effectStackView_.setBounds(area.getX(), y, area.getWidth(), fxHeight);
    y += fxHeight;
    y += kSectionGap;

    // Cuepoints header
    y += kSectionHeaderHeight;
    int cpBtnWidth = area.getWidth() / kNumCuepoints;
    for (int i = 0; i < kNumCuepoints; ++i)
    {
        cuepointBtns_[static_cast<size_t>(i)]->setBounds(
            area.getX() + i * cpBtnWidth, y, cpBtnWidth - 2, kRowHeight);
    }
}

void ClipInspector::setClip(Clip* clip)
{
    clip_ = clip;
    if (clip)
    {
        effectStackView_.setEffects(&clip->effects);
        syncFromClip();
    }
    else
    {
        effectStackView_.setEffects(nullptr);
    }
    resized();
    repaint();
}

void ClipInspector::setEffectLibrary(EffectLibrary* lib)
{
    effectStackView_.setEffectLibrary(lib);
}

void ClipInspector::setSignalRegistry(SignalRegistry* reg)
{
    macroPanel_.setSignalRegistry(reg);
    effectStackView_.setSignalRegistry(reg);
}

void ClipInspector::setMacroBank(MacroBank* bank)
{
    macroPanel_.setMacroBank(bank);
}

void ClipInspector::refresh()
{
    if (clip_)
    {
        syncFromClip();
        effectStackView_.refresh();
        macroPanel_.refresh();
    }
    repaint();
}

int ClipInspector::getPreferredHeight() const
{
    if (!clip_) return 100;

    int h = MacroPanel::kPreferredHeight + kSectionGap;
    h += kSectionHeaderHeight + kRowHeight * 3 + kSectionGap; // transport
    h += kSectionHeaderHeight + kRowHeight * 2 + kSectionGap; // autopilot
    h += kSectionHeaderHeight + kRowHeight + kSectionGap; // beat snap
    h += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + kSectionGap; // effects
    h += kSectionHeaderHeight + kRowHeight + 8; // cuepoints
    return h;
}

void ClipInspector::paintSectionHeader(juce::Graphics& g,
                                        const juce::Rectangle<int>& bounds,
                                        const juce::String& title)
{
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(bounds);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)).boldened());
    g.drawText(title, bounds.withTrimmedLeft(4), juce::Justification::centredLeft, false);
}

void ClipInspector::syncFromClip()
{
    if (!clip_) return;

    transportModeSelector_.setSelectedId(
        clip_->transportMode == Clip::TransportMode::BPMSync ? 2 : 1,
        juce::dontSendNotification);

    switch (clip_->loopMode)
    {
        case Clip::LoopMode::Loop:     loopModeSelector_.setSelectedId(1, juce::dontSendNotification); break;
        case Clip::LoopMode::PingPong: loopModeSelector_.setSelectedId(2, juce::dontSendNotification); break;
        case Clip::LoopMode::OneShot:  loopModeSelector_.setSelectedId(3, juce::dontSendNotification); break;
    }

    speedSlider_.setValue(static_cast<double>(clip_->speed), juce::dontSendNotification);
    reverseBtn_.setColour(juce::TextButton::buttonColourId,
        clip_->reverse ? juce::Colour(0xff4a4a2a) : juce::Colour(AudioDNALookAndFeel::kSurface));
    beatSnapToggle_.setToggleState(clip_->beatSnap, juce::dontSendNotification);

    autopilotActionSelector_.setSelectedId(
        static_cast<int>(clip_->autopilotAction) + 1, juce::dontSendNotification);
    autopilotDurationSelector_.setSelectedId(
        static_cast<int>(clip_->autopilotDuration) + 1, juce::dontSendNotification);

    // Update cuepoint button colors
    for (int i = 0; i < kNumCuepoints; ++i)
    {
        bool hasCue = i < clip_->numCuepoints;
        cuepointBtns_[static_cast<size_t>(i)]->setColour(
            juce::TextButton::buttonColourId,
            hasCue ? juce::Colour(0xff3a4a3a) : juce::Colour(AudioDNALookAndFeel::kSurface));
    }
}
