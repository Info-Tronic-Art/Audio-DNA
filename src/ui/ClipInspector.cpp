#include "ui/ClipInspector.h"

ClipInspector::ClipInspector()
{
    addAndMakeVisible(macroPanel_);

    // --- Transport mode ---
    transportModeSelector_.addItem("Timeline", 1);
    transportModeSelector_.addItem("BPM Sync", 2);
    transportModeSelector_.setSelectedId(1, juce::dontSendNotification);
    transportModeSelector_.onChange = [this] {
        if (!clip_) return;
        clip_->transportMode = transportModeSelector_.getSelectedId() == 2
            ? Clip::TransportMode::BPMSync : Clip::TransportMode::Timeline;
        resized();
        repaint();
    };
    addAndMakeVisible(transportModeSelector_);

    // Transport control buttons
    auto setupSmallBtn = [this](juce::TextButton& btn, const juce::String& text) {
        btn.setButtonText(text);
        btn.setColour(juce::TextButton::buttonColourId,
                      juce::Colour(AudioDNALookAndFeel::kSurface));
        btn.setColour(juce::TextButton::textColourOffId,
                      juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        addAndMakeVisible(btn);
    };

    setupSmallBtn(playBackBtn_, juce::String(juce::CharPointer_UTF8("\xe2\x97\x80")));
    setupSmallBtn(pauseBtn_, juce::String(juce::CharPointer_UTF8("\xe2\x8f\xb8")));
    setupSmallBtn(playBtn_, juce::String(juce::CharPointer_UTF8("\xe2\x96\xb6")));
    playBtn_.setColour(juce::TextButton::buttonColourId,
                       juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f));

    // Loop + Trigger dropdowns
    loopDropdown_.addItem("Loop", 1);
    loopDropdown_.addItem("Ping Pong", 2);
    loopDropdown_.addItem("One Shot", 3);
    loopDropdown_.setSelectedId(1, juce::dontSendNotification);
    loopDropdown_.onChange = [this] {
        if (!clip_) return;
        int sel = loopDropdown_.getSelectedId();
        if (sel == 1) clip_->loopMode = Clip::LoopMode::Loop;
        else if (sel == 2) clip_->loopMode = Clip::LoopMode::PingPong;
        else if (sel == 3) clip_->loopMode = Clip::LoopMode::OneShot;
    };
    addAndMakeVisible(loopDropdown_);

    triggerDropdown_.addItem("Restart", 1);
    triggerDropdown_.addItem("Continue", 2);
    triggerDropdown_.addItem("Relative", 3);
    triggerDropdown_.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(triggerDropdown_);

    // Speed slider
    speedSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    speedSlider_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 30, 20);
    speedSlider_.setRange(0.0, 4.0, 0.01);
    speedSlider_.setValue(1.0, juce::dontSendNotification);
    speedSlider_.setScrollWheelEnabled(false);
    speedSlider_.setColour(juce::Slider::thumbColourId,
                           juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    speedSlider_.onValueChange = [this] {
        if (clip_) clip_->speed = static_cast<float>(speedSlider_.getValue());
    };
    addAndMakeVisible(speedSlider_);

    // Duration with ½ and ×2
    durationSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    durationSlider_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
    durationSlider_.setRange(0.1, 300.0, 0.1);
    durationSlider_.setValue(8.0, juce::dontSendNotification);
    durationSlider_.setScrollWheelEnabled(false);
    durationSlider_.setColour(juce::Slider::thumbColourId,
                              juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    addAndMakeVisible(durationSlider_);

    setupSmallBtn(halfSpeedBtn_, juce::String(juce::CharPointer_UTF8("\xc3\xb7")) + "2");
    setupSmallBtn(doubleSpeedBtn_, juce::String(juce::CharPointer_UTF8("\xc3\x97")) + "2");
    setupSmallBtn(durHalfBtn_, "/2");
    setupSmallBtn(durDoubleBtn_, juce::String(juce::CharPointer_UTF8("\xc3\x97")) + "2");

    halfSpeedBtn_.onClick = [this] {
        if (!clip_) return;
        clip_->speed = std::max(0.01f, clip_->speed * 0.5f);
        speedSlider_.setValue(static_cast<double>(clip_->speed), juce::dontSendNotification);
    };
    doubleSpeedBtn_.onClick = [this] {
        if (!clip_) return;
        clip_->speed = std::min(4.0f, clip_->speed * 2.0f);
        speedSlider_.setValue(static_cast<double>(clip_->speed), juce::dontSendNotification);
    };

    // Reverse
    reverseBtn_.setColour(juce::TextButton::buttonColourId,
                          juce::Colour(AudioDNALookAndFeel::kSurface));
    reverseBtn_.onClick = [this] {
        if (!clip_) return;
        clip_->reverse = !clip_->reverse;
        reverseBtn_.setColour(juce::TextButton::buttonColourId,
            clip_->reverse ? juce::Colour(0xff4a4a2a) : juce::Colour(AudioDNALookAndFeel::kSurface));
    };
    addAndMakeVisible(reverseBtn_);

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

    // Image sequence FPS slider
    sequenceFpsLabel_.setText("Images/ Sec", juce::dontSendNotification);
    sequenceFpsLabel_.setColour(juce::Label::textColourId,
                                juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    sequenceFpsLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(sequenceFpsLabel_);

    sequenceFpsSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    sequenceFpsSlider_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 34, 20);
    sequenceFpsSlider_.setRange(0.0, 6.0, 0.1);
    sequenceFpsSlider_.setValue(2.5, juce::dontSendNotification);
    sequenceFpsSlider_.setScrollWheelEnabled(false);
    sequenceFpsSlider_.setColour(juce::Slider::thumbColourId,
                                  juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    sequenceFpsSlider_.setColour(juce::Slider::textBoxTextColourId,
                                  juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    sequenceFpsSlider_.setColour(juce::Slider::textBoxBackgroundColourId,
                                  juce::Colour(0x00000000));
    sequenceFpsSlider_.setColour(juce::Slider::textBoxOutlineColourId,
                                  juce::Colour(0x00000000));
    sequenceFpsSlider_.onValueChange = [this] {
        if (clip_) clip_->sequenceFps = static_cast<float>(sequenceFpsSlider_.getValue());
    };
    addAndMakeVisible(sequenceFpsSlider_);

    // Beat division dropdown (shown in BPM Sync mode for image sequences)
    beatDivisionLabel_.setText("Beats/ Cycle", juce::dontSendNotification);
    beatDivisionLabel_.setColour(juce::Label::textColourId,
                                  juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    beatDivisionLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(beatDivisionLabel_);

    //                              ID    display text        beatDivision value
    beatDivisionSelector_.addItem("1/4 Beat",   1);   // 0.25
    beatDivisionSelector_.addItem("1/2 Beat",   2);   // 0.5
    beatDivisionSelector_.addItem("1 Beat",     3);   // 1
    beatDivisionSelector_.addItem("2 Beats",    4);   // 2
    beatDivisionSelector_.addItem("4 Beats (1 Bar)", 5); // 4
    beatDivisionSelector_.addItem("8 Beats (2 Bars)", 6); // 8
    beatDivisionSelector_.addItem("16 Beats (4 Bars)", 7); // 16
    beatDivisionSelector_.setSelectedId(5, juce::dontSendNotification); // default: 4 beats
    beatDivisionSelector_.onChange = [this] {
        if (!clip_) return;
        int sel = beatDivisionSelector_.getSelectedId();
        switch (sel)
        {
            case 1: clip_->beatDivision = 0.25f; break;
            case 2: clip_->beatDivision = 0.5f; break;
            case 3: clip_->beatDivision = 1.0f; break;
            case 4: clip_->beatDivision = 2.0f; break;
            case 5: clip_->beatDivision = 4.0f; break;
            case 6: clip_->beatDivision = 8.0f; break;
            case 7: clip_->beatDivision = 16.0f; break;
            default: break;
        }
    };
    addAndMakeVisible(beatDivisionSelector_);

    // Content Beats slider (how many beats the source media contains)
    videoBeatsLabel_.setText("Content Beats", juce::dontSendNotification);
    videoBeatsLabel_.setColour(juce::Label::textColourId,
                               juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    videoBeatsLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(videoBeatsLabel_);

    videoBeatsSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    videoBeatsSlider_.setTextBoxStyle(juce::Slider::TextBoxLeft, true, 34, 20);
    videoBeatsSlider_.setTextBoxIsEditable(true);
    videoBeatsSlider_.setRange(1.0, 64.0, 1.0);
    videoBeatsSlider_.setValue(4.0, juce::dontSendNotification);
    videoBeatsSlider_.setScrollWheelEnabled(false);
    // Snap to musical values: 0, 1, 2, 4, 8, 16, 32, 64
    videoBeatsSlider_.setSkewFactor(0.5);  // bunch low values together
    videoBeatsSlider_.onValueChange = [this] {
        // Snap to nearest musical value
        double raw = videoBeatsSlider_.getValue();
        static const double snaps[] = {1, 2, 4, 8, 16, 32, 64};
        double best = 0;
        double bestDist = 999;
        for (double s : snaps)
        {
            double d = std::abs(raw - s);
            if (d < bestDist) { bestDist = d; best = s; }
        }
        if (std::abs(raw - best) > 0.01)
            videoBeatsSlider_.setValue(best, juce::dontSendNotification);
        if (clip_) clip_->videoBeats = static_cast<float>(best);
    };
    videoBeatsSlider_.setColour(juce::Slider::thumbColourId,
                                juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    videoBeatsSlider_.setColour(juce::Slider::textBoxTextColourId,
                                juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    videoBeatsSlider_.setColour(juce::Slider::textBoxBackgroundColourId,
                                juce::Colour(0x00000000));
    videoBeatsSlider_.setColour(juce::Slider::textBoxOutlineColourId,
                                juce::Colour(0x00000000));
    addAndMakeVisible(videoBeatsSlider_);

    // Cuepoints
    for (int i = 0; i < kNumCuepoints; ++i)
    {
        cuepointBtns_[static_cast<size_t>(i)] =
            std::make_unique<juce::TextButton>(juce::String(i + 1));
        cuepointBtns_[static_cast<size_t>(i)]->setColour(
            juce::TextButton::buttonColourId, juce::Colour(AudioDNALookAndFeel::kSurface));
        addAndMakeVisible(cuepointBtns_[static_cast<size_t>(i)].get());
    }

    // --- Video section ---
    clipOpacityControl_.setParamName("Opacity");
    clipOpacityControl_.setParamValue(1.0f);
    clipOpacityControl_.onValueChanged = [this](float val) { if (clip_) clip_->clipOpacity = val; };
    clipOpacityControl_.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
    addAndMakeVisible(clipOpacityControl_);

    auto setupIntSlider = [](juce::Slider& s, double min, double max, double val) {
        s.setSliderStyle(juce::Slider::IncDecButtons);
        s.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
        s.setRange(min, max, 1);
        s.setValue(val, juce::dontSendNotification);
        s.setScrollWheelEnabled(false);
    };
    setupIntSlider(clipWidthSlider_, 1, 7680, 1920);
    clipWidthSlider_.onValueChange = [this] { if (clip_) clip_->clipWidth = static_cast<int>(clipWidthSlider_.getValue()); };
    addAndMakeVisible(clipWidthSlider_);

    setupIntSlider(clipHeightSlider_, 1, 4320, 1080);
    clipHeightSlider_.onValueChange = [this] { if (clip_) clip_->clipHeight = static_cast<int>(clipHeightSlider_.getValue()); };
    addAndMakeVisible(clipHeightSlider_);

    clipBlendModeSelector_.addItem("Layer Determined", 1);
    clipBlendModeSelector_.addItem("Normal", 2);
    clipBlendModeSelector_.addItem("Additive", 3);
    clipBlendModeSelector_.addItem("Screen", 4);
    clipBlendModeSelector_.addItem("Multiply", 5);
    clipBlendModeSelector_.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(clipBlendModeSelector_);

    clipAlphaTypeSelector_.addItem("Premultiplied", 1);
    clipAlphaTypeSelector_.addItem("Straight", 2);
    clipAlphaTypeSelector_.setSelectedId(1, juce::dontSendNotification);
    clipAlphaTypeSelector_.onChange = [this] {
        if (clip_) clip_->alphaType = clipAlphaTypeSelector_.getSelectedId() == 2
            ? Clip::AlphaType::Straight : Clip::AlphaType::Premultiplied;
    };
    addAndMakeVisible(clipAlphaTypeSelector_);

    // RGBA toggles
    auto setupChannelToggle = [this](juce::ToggleButton& btn) {
        btn.setColour(juce::ToggleButton::textColourId,
                      juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        btn.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(btn);
    };
    setupChannelToggle(channelRBtn_);
    setupChannelToggle(channelGBtn_);
    setupChannelToggle(channelBBtn_);
    setupChannelToggle(channelABtn_);

    channelRBtn_.onStateChange = [this] { if (clip_) clip_->channelR = channelRBtn_.getToggleState(); };
    channelGBtn_.onStateChange = [this] { if (clip_) clip_->channelG = channelGBtn_.getToggleState(); };
    channelBBtn_.onStateChange = [this] { if (clip_) clip_->channelB = channelBBtn_.getToggleState(); };
    channelABtn_.onStateChange = [this] { if (clip_) clip_->channelA = channelABtn_.getToggleState(); };

    // --- Transform ---
    auto setupTransformParam = [this](UniversalParamControl& pc, const juce::String& name, float defVal) {
        pc.setParamName(name);
        pc.setParamValue(defVal);
        pc.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
        addAndMakeVisible(pc);
    };
    setupTransformParam(posXControl_, "Position X", 0.5f);
    setupTransformParam(posYControl_, "Position Y", 0.5f);
    setupTransformParam(scaleControl_, "Scale", 0.5f);
    setupTransformParam(rotationControl_, "Rotation", 0.5f);
    setupTransformParam(anchorControl_, "Anchor", 0.5f);

    posXControl_.onValueChanged = [this](float v) { if (clip_) clip_->positionX = (v - 0.5f) * 3840.0f; };
    posYControl_.onValueChanged = [this](float v) { if (clip_) clip_->positionY = (v - 0.5f) * 2160.0f; };
    scaleControl_.onValueChanged = [this](float v) { if (clip_) clip_->scale = v * 2.0f; };
    rotationControl_.onValueChanged = [this](float v) { if (clip_) clip_->rotation = (v - 0.5f) * 720.0f; };
    anchorControl_.onValueChanged = [this](float v) { if (clip_) clip_->anchorX = (v - 0.5f) * 3840.0f; };

    // --- Effects ---
    addAndMakeVisible(effectStackView_);
}

void ClipInspector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    if (!clip_)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.drawText("No clip selected", getLocalBounds(), juce::Justification::centred, false);
        return;
    }

    // Name bar
    auto nameBar = getLocalBounds().removeFromTop(kNameBarHeight);
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(nameBar);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(12.0f)).boldened());
    g.drawText(juce::String(clip_->name), nameBar.withTrimmedLeft(4).withTrimmedRight(40),
               juce::Justification::centredLeft, true);

    // Section headers
    int y = kNameBarHeight + MacroPanel::kPreferredHeight + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Transport");
    int transportExtraH = 0;
    bool showExtraRow = false;
    if (clip_->isPlayable())
    {
        bool bpmSync = (clip_->transportMode == Clip::TransportMode::BPMSync);
        bool isSeq = (clip_->mediaType == Clip::MediaType::ImageSequence);
        showExtraRow = bpmSync || isSeq;
        if (bpmSync)
            transportExtraH = kRowHeight * 2;  // Beats/Cycle + Content Beats
        else if (isSeq)
            transportExtraH = kRowHeight;       // Images/Sec only
    }
    if (showExtraRow)
    {

        // Draw signal-connect triangle
        int fpsRowY = y + kSectionHeaderHeight + kRowHeight * 4;
        float triCx = 4.0f + 7.0f;
        float triCy = static_cast<float>(fpsRowY) + static_cast<float>(kRowHeight) * 0.5f;
        float hs = 4.0f;
        juce::Path tri;
        tri.addTriangle(triCx - hs, triCy - hs, triCx - hs, triCy + hs, triCx + hs, triCy);
        bool connected = (clip_->transportMode == Clip::TransportMode::BPMSync);
        g.setColour(connected ? juce::Colour(AudioDNALookAndFeel::kAccentCyan)
                              : juce::Colour(0xff666666));
        g.fillPath(tri);
    }
    y += kSectionHeaderHeight + kRowHeight * 4 + transportExtraH + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Cuepoints");
    y += kSectionHeaderHeight + kRowHeight + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Autopilot");
    y += kSectionHeaderHeight + kRowHeight * 2 + kSectionGap;

    // Source params (conditional)
    if (clip_->mediaType == Clip::MediaType::Source && !sourceParamControls_.empty())
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Source");
        int srcH = 0;
        for (auto& pc : sourceParamControls_) srcH += pc->getPreferredHeight();
        y += kSectionHeaderHeight + srcH + kSectionGap;
    }

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Video");
    y += kSectionHeaderHeight + clipOpacityControl_.getPreferredHeight() + kRowHeight * 4 + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Transform", true);
    int transformH = posXControl_.getPreferredHeight() + posYControl_.getPreferredHeight()
                   + scaleControl_.getPreferredHeight() + rotationControl_.getPreferredHeight()
                   + anchorControl_.getPreferredHeight();
    y += kSectionHeaderHeight + transformH + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Effects");
}

void ClipInspector::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    int y = 0;

    // Name bar
    y += kNameBarHeight;

    // Dashboard
    macroPanel_.setBounds(area.getX(), y, area.getWidth(), MacroPanel::kPreferredHeight);
    y += MacroPanel::kPreferredHeight + kSectionGap;

    if (!clip_) return;

    // --- Transport ---
    y += kSectionHeaderHeight;

    // Mode dropdown + transport buttons
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        transportModeSelector_.setBounds(row.removeFromRight(100));
    }
    y += kRowHeight;

    // Transport buttons: ◀ ⏸ ▶ + loop dropdown + trigger dropdown
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        playBackBtn_.setBounds(row.removeFromLeft(26));
        row.removeFromLeft(2);
        pauseBtn_.setBounds(row.removeFromLeft(26));
        row.removeFromLeft(2);
        playBtn_.setBounds(row.removeFromLeft(26));
        row.removeFromLeft(8);
        loopDropdown_.setBounds(row.removeFromLeft(row.getWidth() / 2 - 2));
        row.removeFromLeft(4);
        triggerDropdown_.setBounds(row);
    }
    y += kRowHeight;

    // Speed row: label + ½ ×2 + slider
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        halfSpeedBtn_.setBounds(row.removeFromLeft(28));
        row.removeFromLeft(2);
        doubleSpeedBtn_.setBounds(row.removeFromLeft(28));
        row.removeFromLeft(4);
        reverseBtn_.setBounds(row.removeFromRight(55));
        row.removeFromRight(4);
        speedSlider_.setBounds(row);
    }
    y += kRowHeight;

    // Duration row
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        durHalfBtn_.setBounds(row.removeFromRight(24));
        row.removeFromRight(2);
        durDoubleBtn_.setBounds(row.removeFromRight(24));
        row.removeFromRight(4);
        durationSlider_.setBounds(row);
    }
    y += kRowHeight;

    // Playable clips: show Images/Sec (Timeline, image seq only) or Beats/Cycle (BPM Sync, all playable)
    if (clip_ && clip_->isPlayable())
    {
        bool bpmSync = (clip_->transportMode == Clip::TransportMode::BPMSync);
        bool isSeq = (clip_->mediaType == Clip::MediaType::ImageSequence);
        int triW = 14;
        int labelW = 76;

        if (bpmSync)
        {
            // BPM Sync: show beat division dropdown + content beats
            sequenceFpsLabel_.setVisible(false);
            sequenceFpsSlider_.setVisible(false);
            beatDivisionLabel_.setVisible(true);
            beatDivisionSelector_.setVisible(true);
            beatDivisionLabel_.setBounds(area.getX() + triW, y, labelW, kRowHeight);
            beatDivisionSelector_.setBounds(area.getX() + triW + labelW, y,
                                             area.getWidth() - triW - labelW, kRowHeight);
            y += kRowHeight;

            // Content Beats row
            videoBeatsLabel_.setVisible(true);
            videoBeatsSlider_.setVisible(true);
            int cbLabelW = 90;
            videoBeatsLabel_.setBounds(area.getX() + triW, y, cbLabelW, kRowHeight);
            videoBeatsSlider_.setBounds(area.getX() + triW + cbLabelW, y,
                                         area.getWidth() - triW - cbLabelW, kRowHeight);
            y += kRowHeight;
        }
        else if (isSeq)
        {
            // Timeline + Image Sequence: show Images/Sec slider
            beatDivisionLabel_.setVisible(false);
            beatDivisionSelector_.setVisible(false);
            videoBeatsLabel_.setVisible(false);
            videoBeatsSlider_.setVisible(false);
            sequenceFpsLabel_.setVisible(true);
            sequenceFpsSlider_.setVisible(true);
            sequenceFpsLabel_.setBounds(area.getX() + triW, y, labelW, kRowHeight);
            sequenceFpsSlider_.setBounds(area.getX() + triW + labelW, y,
                                          area.getWidth() - triW - labelW, kRowHeight);
            y += kRowHeight;
        }
        else
        {
            // Timeline + Video: no extra row needed
            sequenceFpsLabel_.setVisible(false);
            sequenceFpsSlider_.setVisible(false);
            beatDivisionLabel_.setVisible(false);
            beatDivisionSelector_.setVisible(false);
            videoBeatsLabel_.setVisible(false);
            videoBeatsSlider_.setVisible(false);
        }
    }
    else
    {
        sequenceFpsLabel_.setVisible(false);
        sequenceFpsSlider_.setVisible(false);
        beatDivisionLabel_.setVisible(false);
        beatDivisionSelector_.setVisible(false);
        videoBeatsLabel_.setVisible(false);
        videoBeatsSlider_.setVisible(false);
    }
    y += kSectionGap;

    // --- Cuepoints ---
    y += kSectionHeaderHeight;
    int cpBtnWidth = area.getWidth() / kNumCuepoints;
    for (int i = 0; i < kNumCuepoints; ++i)
        cuepointBtns_[static_cast<size_t>(i)]->setBounds(
            area.getX() + i * cpBtnWidth, y, cpBtnWidth - 2, kRowHeight);
    y += kRowHeight + kSectionGap;

    // --- Autopilot ---
    y += kSectionHeaderHeight;
    autopilotActionSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        autopilotDurationSelector_.setBounds(row.removeFromLeft(row.getWidth() / 2 - 2));
        row.removeFromLeft(4);
        beatSnapToggle_.setBounds(row);
    }
    y += kRowHeight + kSectionGap;

    // --- Source params ---
    if (clip_->mediaType == Clip::MediaType::Source && !sourceParamControls_.empty())
    {
        y += kSectionHeaderHeight;
        for (auto& pc : sourceParamControls_)
        {
            int pcH = pc->getPreferredHeight();
            pc->setBounds(area.getX(), y, area.getWidth(), pcH);
            y += pcH;
        }
        y += kSectionGap;
    }

    // --- Video ---
    y += kSectionHeaderHeight;
    clipOpacityControl_.setBounds(area.getX(), y, area.getWidth(), clipOpacityControl_.getPreferredHeight());
    y += clipOpacityControl_.getPreferredHeight();
    clipWidthSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    clipHeightSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    clipBlendModeSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        clipAlphaTypeSelector_.setBounds(row.removeFromLeft(row.getWidth() / 2 - 2));
        row.removeFromLeft(4);
        int toggleW = row.getWidth() / 4;
        channelRBtn_.setBounds(row.removeFromLeft(toggleW));
        channelGBtn_.setBounds(row.removeFromLeft(toggleW));
        channelBBtn_.setBounds(row.removeFromLeft(toggleW));
        channelABtn_.setBounds(row);
    }
    y += kRowHeight + kSectionGap;

    // --- Transform ---
    y += kSectionHeaderHeight;
    posXControl_.setBounds(area.getX(), y, area.getWidth(), posXControl_.getPreferredHeight()); y += posXControl_.getPreferredHeight();
    posYControl_.setBounds(area.getX(), y, area.getWidth(), posYControl_.getPreferredHeight()); y += posYControl_.getPreferredHeight();
    scaleControl_.setBounds(area.getX(), y, area.getWidth(), scaleControl_.getPreferredHeight()); y += scaleControl_.getPreferredHeight();
    rotationControl_.setBounds(area.getX(), y, area.getWidth(), rotationControl_.getPreferredHeight()); y += rotationControl_.getPreferredHeight();
    anchorControl_.setBounds(area.getX(), y, area.getWidth(), anchorControl_.getPreferredHeight()); y += anchorControl_.getPreferredHeight() + kSectionGap;

    // --- Effects ---
    y += kSectionHeaderHeight;
    int fxHeight = effectStackView_.getPreferredHeight();
    effectStackView_.setBounds(area.getX(), y, area.getWidth(), fxHeight);
}

void ClipInspector::setClip(Clip* clip)
{
    clip_ = clip;
    if (clip)
    {
        effectStackView_.setEffects(&clip->effects);
        buildSourceParamControls();
        syncFromClip();
    }
    else
    {
        effectStackView_.setEffects(nullptr);
        sourceParamControls_.clear();
    }
    resized();
    repaint();
}

void ClipInspector::buildSourceParamControls()
{
    for (auto& pc : sourceParamControls_) removeChildComponent(pc.get());
    sourceParamControls_.clear();

    if (!clip_ || clip_->mediaType != Clip::MediaType::Source || clip_->sourceParams.empty())
        return;

    for (size_t i = 0; i < clip_->sourceParams.size(); ++i)
    {
        auto& sp = clip_->sourceParams[i];
        auto pc = std::make_unique<UniversalParamControl>();
        pc->setParamName(juce::String(sp.name));
        pc->setParamValue(sp.value);
        pc->setSignalRegistry(signalRegistry_);

        auto idx = i;
        pc->onValueChanged = [this, idx](float val) {
            if (!clip_ || idx >= clip_->sourceParams.size()) return;
            clip_->sourceParams[idx].value = val;
            if (onSourceParamsChanged) onSourceParamsChanged(clip_);
        };
        pc->onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };

        addAndMakeVisible(pc.get());
        sourceParamControls_.push_back(std::move(pc));
    }
}

void ClipInspector::setEffectLibrary(EffectLibrary* lib) { effectStackView_.setEffectLibrary(lib); }

void ClipInspector::setSignalRegistry(SignalRegistry* reg)
{
    signalRegistry_ = reg;
    macroPanel_.setSignalRegistry(reg);
    effectStackView_.setSignalRegistry(reg);
    clipOpacityControl_.setSignalRegistry(reg);
    posXControl_.setSignalRegistry(reg);
    posYControl_.setSignalRegistry(reg);
    scaleControl_.setSignalRegistry(reg);
    rotationControl_.setSignalRegistry(reg);
    anchorControl_.setSignalRegistry(reg);
    for (auto& pc : sourceParamControls_) pc->setSignalRegistry(reg);
}

void ClipInspector::setMacroBank(MacroBank* bank)
{
    macroBank_ = bank;
    macroPanel_.setMacroBank(bank);
    effectStackView_.setMacroBank(bank);
}

void ClipInspector::refresh()
{
    if (clip_)
    {
        syncFromClip();
        effectStackView_.refresh();
        macroPanel_.refresh();

        // Drive source params from connected signals and macros
        if (clip_->mediaType == Clip::MediaType::Source)
        {
            for (size_t i = 0; i < sourceParamControls_.size() && i < clip_->sourceParams.size(); ++i)
            {
                auto& pc = *sourceParamControls_[i];
                if (!pc.isConnected()) continue;

                auto mode = pc.getSourceMode();
                auto sourceName = pc.getSourceName();
                float val = 0.0f;
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
                            val = signalRegistry_->getCachedValue(sig->getId());
                            found = true;
                            break;
                        }
                    }
                }
                else if (mode == UniversalParamControl::SourceMode::Macro && macroBank_)
                {
                    int macroIdx = -1;
                    if (sourceName.startsWithIgnoreCase("Macro ") || sourceName.startsWithIgnoreCase("Link "))
                        macroIdx = sourceName.getTrailingIntValue() - 1;
                    if (macroIdx >= 0 && macroIdx < MacroBank::kNumMacros)
                    {
                        val = macroBank_->getMacroValue(macroIdx);
                        found = true;
                    }
                }

                if (found)
                {
                    clip_->sourceParams[i].value = val;
                    pc.setSourceValue(val);
                    pc.setParamValue(val);
                    if (onSourceParamsChanged)
                        onSourceParamsChanged(clip_);
                }
            }
        }
    }
    repaint();
}

int ClipInspector::getPreferredHeight() const
{
    if (!clip_) return 100;

    int h = kNameBarHeight + MacroPanel::kPreferredHeight + kSectionGap;
    int transportH = kRowHeight * 4;
    if (clip_->isPlayable())
    {
        bool bpmSync = (clip_->transportMode == Clip::TransportMode::BPMSync);
        bool isSeq = (clip_->mediaType == Clip::MediaType::ImageSequence);
        if (bpmSync)
            transportH += kRowHeight * 2;  // Beats/Cycle + Content Beats
        else if (isSeq)
            transportH += kRowHeight;       // Images/Sec
    }
    h += kSectionHeaderHeight + transportH + kSectionGap; // Transport
    h += kSectionHeaderHeight + kRowHeight + kSectionGap; // Cuepoints
    h += kSectionHeaderHeight + kRowHeight * 2 + kSectionGap; // Autopilot

    if (clip_->mediaType == Clip::MediaType::Source && !sourceParamControls_.empty())
    {
        int srcH = 0;
        for (auto& pc : sourceParamControls_) srcH += pc->getPreferredHeight();
        h += kSectionHeaderHeight + srcH + kSectionGap;
    }

    h += kSectionHeaderHeight + clipOpacityControl_.getPreferredHeight() + kRowHeight * 4 + kSectionGap; // Video
    h += kSectionHeaderHeight + posXControl_.getPreferredHeight() + posYControl_.getPreferredHeight()
       + scaleControl_.getPreferredHeight() + rotationControl_.getPreferredHeight()
       + anchorControl_.getPreferredHeight() + kSectionGap; // Transform
    h += kSectionHeaderHeight + effectStackView_.getPreferredHeight() + 8; // Effects
    return h;
}

void ClipInspector::paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                        const juce::String& title, bool hasPButton)
{
    if (title == "Transform")
        g.setColour(juce::Colour(0xff1a3a3a));
    else
        g.setColour(juce::Colour(0xff222222));
    g.fillRect(bounds);

    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)).boldened());

    auto textBounds = bounds.withTrimmedLeft(4);
    juce::Path tri;
    float tx = textBounds.getX() + 2.0f;
    float ty = static_cast<float>(textBounds.getCentreY());
    tri.addTriangle(tx, ty - 3.0f, tx, ty + 3.0f, tx + 4.0f, ty);
    g.fillPath(tri);

    g.drawText(title, textBounds.withTrimmedLeft(10), juce::Justification::centredLeft, false);

    if (hasPButton)
    {
        auto pBounds = juce::Rectangle<int>(bounds.getRight() - 20, bounds.getY(), 20, bounds.getHeight());
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText("P.", pBounds, juce::Justification::centred, false);
    }
}

void ClipInspector::syncFromClip()
{
    if (!clip_) return;

    transportModeSelector_.setSelectedId(
        clip_->transportMode == Clip::TransportMode::BPMSync ? 2 : 1, juce::dontSendNotification);

    switch (clip_->loopMode)
    {
        case Clip::LoopMode::Loop:     loopDropdown_.setSelectedId(1, juce::dontSendNotification); break;
        case Clip::LoopMode::PingPong: loopDropdown_.setSelectedId(2, juce::dontSendNotification); break;
        case Clip::LoopMode::OneShot:  loopDropdown_.setSelectedId(3, juce::dontSendNotification); break;
    }

    speedSlider_.setValue(static_cast<double>(clip_->speed), juce::dontSendNotification);
    reverseBtn_.setColour(juce::TextButton::buttonColourId,
        clip_->reverse ? juce::Colour(0xff4a4a2a) : juce::Colour(AudioDNALookAndFeel::kSurface));
    beatSnapToggle_.setToggleState(clip_->beatSnap, juce::dontSendNotification);

    // Image sequence FPS / beat division
    if (clip_->isPlayable())
    {
        if (clip_->mediaType == Clip::MediaType::ImageSequence)
            sequenceFpsSlider_.setValue(static_cast<double>(clip_->sequenceFps), juce::dontSendNotification);

        // Map beatDivision value to dropdown ID
        int bdId = 5; // default: 4 beats
        if (clip_->beatDivision <= 0.25f) bdId = 1;
        else if (clip_->beatDivision <= 0.5f) bdId = 2;
        else if (clip_->beatDivision <= 1.0f) bdId = 3;
        else if (clip_->beatDivision <= 2.0f) bdId = 4;
        else if (clip_->beatDivision <= 4.0f) bdId = 5;
        else if (clip_->beatDivision <= 8.0f) bdId = 6;
        else bdId = 7;
        beatDivisionSelector_.setSelectedId(bdId, juce::dontSendNotification);
        videoBeatsSlider_.setValue(static_cast<double>(clip_->videoBeats), juce::dontSendNotification);
    }

    autopilotActionSelector_.setSelectedId(
        static_cast<int>(clip_->autopilotAction) + 1, juce::dontSendNotification);
    autopilotDurationSelector_.setSelectedId(
        static_cast<int>(clip_->autopilotDuration) + 1, juce::dontSendNotification);

    // Cuepoint colors
    for (int i = 0; i < kNumCuepoints; ++i)
    {
        bool hasCue = i < clip_->numCuepoints;
        cuepointBtns_[static_cast<size_t>(i)]->setColour(
            juce::TextButton::buttonColourId,
            hasCue ? juce::Colour(0xff3a4a3a) : juce::Colour(AudioDNALookAndFeel::kSurface));
    }

    // Video
    clipOpacityControl_.setParamValue(clip_->clipOpacity);
    clipWidthSlider_.setValue(clip_->clipWidth, juce::dontSendNotification);
    clipHeightSlider_.setValue(clip_->clipHeight, juce::dontSendNotification);
    channelRBtn_.setToggleState(clip_->channelR, juce::dontSendNotification);
    channelGBtn_.setToggleState(clip_->channelG, juce::dontSendNotification);
    channelBBtn_.setToggleState(clip_->channelB, juce::dontSendNotification);
    channelABtn_.setToggleState(clip_->channelA, juce::dontSendNotification);

    // Transform
    posXControl_.setParamValue(clip_->positionX / 3840.0f + 0.5f);
    posYControl_.setParamValue(clip_->positionY / 2160.0f + 0.5f);
    scaleControl_.setParamValue(clip_->scale / 2.0f);
    rotationControl_.setParamValue(clip_->rotation / 720.0f + 0.5f);
    anchorControl_.setParamValue(clip_->anchorX / 3840.0f + 0.5f);
}
