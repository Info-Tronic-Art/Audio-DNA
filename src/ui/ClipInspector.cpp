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

    playBackBtn_.onClick = [this] {
        if (!clip_) return;
        clip_->reverse = true;
        clip_->playing = true;
        updateTransportHighlights();
    };
    pauseBtn_.onClick = [this] {
        if (!clip_) return;
        clip_->playing = false;
        updateTransportHighlights();
    };
    playBtn_.onClick = [this] {
        if (!clip_) return;
        clip_->reverse = false;
        clip_->playing = true;
        updateTransportHighlights();
    };

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
    speedSlider_.setDefaultValue(1.0);
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
    durationSlider_.setDefaultValue(8.0);
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

    // Beat snap mode (P21: granularity)
    beatSnapSelector_.setTextWhenNothingSelected("Snap Off");
    beatSnapSelector_.addItem("Snap Off", 1);
    beatSnapSelector_.addItem("Beat", 2);
    beatSnapSelector_.addItem("Bar", 3);
    beatSnapSelector_.addItem("2 Bar", 4);
    beatSnapSelector_.addItem("4 Bar", 5);
    beatSnapSelector_.setTooltip("Quantize clip trigger to next beat/bar boundary");
    beatSnapSelector_.onChange = [this] {
        if (!clip_) return;
        int sel = beatSnapSelector_.getSelectedId();
        clip_->beatSnapMode = static_cast<Clip::BeatSnapMode>(sel - 1);
        clip_->beatSnap = (sel > 1);
    };
    addAndMakeVisible(beatSnapSelector_);

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
    sequenceFpsSlider_.setDefaultValue(2.5);
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
    videoBeatsSlider_.setDefaultValue(4.0);
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

    // Cuepoints — two rows: trigger buttons (top) + set buttons (bottom)
    for (int i = 0; i < kNumCuepoints; ++i)
    {
        // Trigger button: numbered, jumps to cuepoint. Ctrl+click clears.
        cuepointBtns_[static_cast<size_t>(i)] =
            std::make_unique<juce::TextButton>(juce::String(i + 1));
        auto& trigBtn = *cuepointBtns_[static_cast<size_t>(i)];
        trigBtn.setColour(juce::TextButton::buttonColourId,
                          juce::Colour(AudioDNALookAndFeel::kSurface));
        trigBtn.onClick = [this, i] {
            if (!clip_ || i >= clip_->numCuepoints) return;

            bool ctrlHeld = juce::ModifierKeys::currentModifiers.isCtrlDown()
                         || juce::ModifierKeys::currentModifiers.isCommandDown();
            if (ctrlHeld)
            {
                // Clear this cuepoint
                for (int j = i; j < clip_->numCuepoints - 1; ++j)
                    clip_->cuepoints[j] = clip_->cuepoints[j + 1];
                clip_->cuepoints[clip_->numCuepoints - 1] = 0.0f;
                --clip_->numCuepoints;
                syncFromClip();
                repaint();
                return;
            }

            // Jump to cuepoint
            double pos = static_cast<double>(clip_->cuepoints[i]);
            clip_->playheadPosition = pos;
            if (onCuepointJump)
                onCuepointJump(clip_, pos);
        };
        addAndMakeVisible(&trigBtn);

        // Set button: small "Set" button, saves current playhead position
        cuepointSetBtns_[static_cast<size_t>(i)] =
            std::make_unique<juce::TextButton>("Set");
        auto& setBtn = *cuepointSetBtns_[static_cast<size_t>(i)];
        setBtn.setColour(juce::TextButton::buttonColourId,
                         juce::Colour(0xff2a2a2a));
        setBtn.setColour(juce::TextButton::textColourOffId,
                         juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        setBtn.onClick = [this, i] {
            if (!clip_) return;
            // Set or overwrite this cuepoint at current playhead
            clip_->cuepoints[i] = static_cast<float>(clip_->playheadPosition);
            if (i >= clip_->numCuepoints)
                clip_->numCuepoints = i + 1;
            if (onCuepointSet)
                onCuepointSet(clip_, i);
            syncFromClip();
            repaint();
        };
        addAndMakeVisible(&setBtn);
    }

    // --- Video section ---
    clipOpacityControl_.setParamName("Opacity");
    clipOpacityControl_.setParamValue(1.0f);
    clipOpacityControl_.setDefaultValue(1.0f);
    clipOpacityControl_.onValueChanged = [this](float val) { if (clip_) clip_->clipOpacity = val; };
    clipOpacityControl_.onExpandToggled = [this] { resized(); if (auto* p = getParentComponent()) p->resized(); };
    addAndMakeVisible(clipOpacityControl_);

    auto setupIntSlider = [](ResettableSlider& s, double min, double max, double val) {
        s.setSliderStyle(juce::Slider::IncDecButtons);
        s.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
        s.setRange(min, max, 1);
        s.setValue(val, juce::dontSendNotification);
        s.setDefaultValue(val);
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
        pc.setDefaultValue(defVal);
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
    scaleControl_.onValueChanged = [this](float v) { if (clip_) clip_->scale = std::pow(2.0f, (v - 0.5f) * 2.0f); };
    rotationControl_.onValueChanged = [this](float v) { if (clip_) clip_->rotation = (v - 0.5f) * 720.0f; };
    anchorControl_.onValueChanged = [this](float v) { if (clip_) clip_->anchorX = (v - 0.5f) * 3840.0f; };

    // --- Effects ---
    addAndMakeVisible(effectStackView_);
}

void ClipInspector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // FX drop highlight
    if (fxDropHighlight_)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.15f));
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.6f));
        g.drawRect(getLocalBounds(), 2);
    }

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

    // Transport header — mode selector is placed inside the header row by resized()
    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Transport");
    int transportExtraH = 0;
    bool showExtraRow = false;
    if (clip_->isPlayable())
    {
        bool bpmSync = (clip_->transportMode == Clip::TransportMode::BPMSync);
        bool isSeq = (clip_->mediaType == Clip::MediaType::ImageSequence);
        showExtraRow = bpmSync || isSeq;
        if (bpmSync)
            transportExtraH = kRowHeight * 2;
        else if (isSeq)
            transportExtraH = kRowHeight;
    }

    // Track Y incrementally to stay in sync with resized()
    {
        auto area = getLocalBounds().reduced(kInset, 0);
        int ty = y; // local transport Y tracker

        // Position readout in header
        float pos = static_cast<float>(clip_->playheadPosition);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        int readoutX = getWidth() - kInset - 100 - 42;
        g.drawText(juce::String(pos, 2),
                   juce::Rectangle<int>(readoutX, ty, 38, kSectionHeaderHeight),
                   juce::Justification::centredRight, false);
        ty += kSectionHeaderHeight;

        // Timeline
        paintTimeline(g, juce::Rectangle<int>(area.getX(), ty, area.getWidth(), kTimelineHeight));
        ty += kTimelineHeight + 2;

        // Transport buttons row
        ty += kRowHeight + 2;

        // Speed row — paint label
        bool bpmSyncMode = (clip_->transportMode == Clip::TransportMode::BPMSync);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText("Speed", area.getX(), ty, 52, kRowHeight, juce::Justification::centredLeft, false);
        ty += kRowHeight;

        // Duration/Beats row — paint label
        g.drawText(bpmSyncMode ? "Beats" : "Duration",
                   area.getX(), ty, 52, kRowHeight, juce::Justification::centredLeft, false);
        ty += kRowHeight;

        // Extra rows (signal-connect triangle)
        if (showExtraRow)
        {
            float triCx = static_cast<float>(area.getX()) + 7.0f;
            float triCy = static_cast<float>(ty) + static_cast<float>(kRowHeight) * 0.5f;
            float hs = 4.0f;
            juce::Path tri;
            tri.addTriangle(triCx - hs, triCy - hs, triCx - hs, triCy + hs, triCx + hs, triCy);
            g.setColour(bpmSyncMode ? juce::Colour(AudioDNALookAndFeel::kAccentCyan)
                                    : juce::Colour(0xff666666));
            g.fillPath(tri);
        }
    }
    // Advance y past entire transport section
    y += kSectionHeaderHeight + kTimelineHeight + 2 + kRowHeight + 2 + kRowHeight + kRowHeight + transportExtraH + kSectionGap;

    paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "Cuepoints");
    y += kSectionHeaderHeight + kRowHeight + 14 + kSectionGap;

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
    auto area = getLocalBounds().reduced(kInset, 0);
    int y = 0;

    // Name bar
    y += kNameBarHeight;

    // Dashboard
    macroPanel_.setBounds(area.getX(), y, area.getWidth(), MacroPanel::kPreferredHeight);
    y += MacroPanel::kPreferredHeight + kSectionGap;

    if (!clip_) return;

    // --- Transport ---
    // Mode selector sits inside the section header
    transportModeSelector_.setBounds(area.getRight() - 100, y, 100, kSectionHeaderHeight);
    y += kSectionHeaderHeight;

    // Timeline bar (painted in paint(), interactive via mouse handlers)
    timelineBounds_ = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kTimelineHeight);
    y += kTimelineHeight + 2;  // 2px breathing room

    // Transport buttons: ◀ ⏸ ▶ + loop/trigger dropdowns
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        int btnW = 28;
        playBackBtn_.setBounds(row.removeFromLeft(btnW));
        row.removeFromLeft(2);
        pauseBtn_.setBounds(row.removeFromLeft(btnW));
        row.removeFromLeft(2);
        playBtn_.setBounds(row.removeFromLeft(btnW));
        row.removeFromLeft(10);
        // Loop and Trigger dropdowns share remaining space
        int remaining = row.getWidth();
        loopDropdown_.setBounds(row.removeFromLeft(remaining / 2 - 2));
        row.removeFromLeft(4);
        triggerDropdown_.setBounds(row);
    }
    y += kRowHeight + 2;

    // Speed row: "Speed" label (painted) + slider (has text box) + ÷2 ×2 + Reverse
    {
        int labelW = 42;
        auto row = juce::Rectangle<int>(area.getX() + labelW, y, area.getWidth() - labelW, kRowHeight);
        reverseBtn_.setBounds(row.removeFromRight(50));
        row.removeFromRight(2);
        doubleSpeedBtn_.setBounds(row.removeFromRight(22));
        row.removeFromRight(1);
        halfSpeedBtn_.setBounds(row.removeFromRight(22));
        row.removeFromRight(4);
        speedSlider_.setBounds(row);
    }
    y += kRowHeight;

    // Duration/Beats row: label (painted) + slider (has text box) + /2 ×2
    {
        int labelW = 52;
        auto row = juce::Rectangle<int>(area.getX() + labelW, y, area.getWidth() - labelW, kRowHeight);
        durDoubleBtn_.setBounds(row.removeFromRight(22));
        row.removeFromRight(1);
        durHalfBtn_.setBounds(row.removeFromRight(22));
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

    // --- Cuepoints (2 rows: trigger + set) ---
    y += kSectionHeaderHeight;
    int cpBtnWidth = area.getWidth() / kNumCuepoints;
    // Row 1: Trigger buttons (numbered)
    for (int i = 0; i < kNumCuepoints; ++i)
        cuepointBtns_[static_cast<size_t>(i)]->setBounds(
            area.getX() + i * cpBtnWidth, y, cpBtnWidth - 2, kRowHeight);
    y += kRowHeight;
    // Row 2: Set buttons (compact, secondary)
    for (int i = 0; i < kNumCuepoints; ++i)
        cuepointSetBtns_[static_cast<size_t>(i)]->setBounds(
            area.getX() + i * cpBtnWidth, y, cpBtnWidth - 2, 14);
    y += 14 + kSectionGap;

    // --- Autopilot ---
    y += kSectionHeaderHeight;
    autopilotActionSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
    y += kRowHeight;
    {
        auto row = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
        autopilotDurationSelector_.setBounds(row.removeFromLeft(row.getWidth() / 2 - 2));
        row.removeFromLeft(4);
        beatSnapSelector_.setBounds(row);
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
        pc->setDefaultValue(sp.defaultValue);
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
    int transportH = kTimelineHeight + 2 + kRowHeight + 2 + kRowHeight + kRowHeight; // timeline+2 + buttons+2 + speed + duration (mode in header)
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
    h += kSectionHeaderHeight + kRowHeight + 14 + kSectionGap; // Cuepoints (trigger + set rows)
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
        g.setColour(juce::Colour(0xff282838));
    g.fillRect(bounds);

    // Subtle top border for separation
    g.setColour(juce::Colour(0xff3a3a4a));
    g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), 1);

    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.5f)).boldened());

    auto textBounds = bounds.withTrimmedLeft(6);
    // Downward-pointing triangle (expanded state indicator)
    juce::Path tri;
    float tx = textBounds.getX() + 3.0f;
    float ty = static_cast<float>(textBounds.getCentreY());
    tri.addTriangle(tx - 3.0f, ty - 2.0f, tx + 3.0f, ty - 2.0f, tx, ty + 2.5f);
    g.fillPath(tri);

    g.drawText(title, textBounds.withTrimmedLeft(12), juce::Justification::centredLeft, false);

    if (hasPButton)
    {
        auto pBounds = juce::Rectangle<int>(bounds.getRight() - 20, bounds.getY(), 20, bounds.getHeight());
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText("P.", pBounds, juce::Justification::centred, false);
    }
}

// === Timeline bar painting ===

void ClipInspector::paintTimeline(juce::Graphics& g, const juce::Rectangle<int>& bounds) const
{
    if (!clip_) return;

    float inP = clip_->inPoint;
    float outP = clip_->outPoint;
    int x = bounds.getX();
    int w = bounds.getWidth();
    int playheadZone = 14;  // top area for playhead triangle
    int handleZone = 8;     // bottom area for in/out handle tabs
    int trackY = bounds.getY() + playheadZone;
    int trackH = bounds.getHeight() - playheadZone - handleZone;
    int handleY = trackY + trackH; // bottom edge

    int inX = x + static_cast<int>(inP * static_cast<float>(w));
    int outX = x + static_cast<int>(outP * static_cast<float>(w));

    // --- Dark regions outside in/out range ---
    g.setColour(juce::Colour(0xff0e0e0e));
    if (inX > x)
        g.fillRect(x, trackY, inX - x, trackH);
    if (outX < x + w)
        g.fillRect(outX, trackY, x + w - outX, trackH);

    // --- Active region (lighter) ---
    g.setColour(juce::Colour(0xff333340));
    g.fillRect(inX, trackY, std::max(1, outX - inX), trackH);

    // --- Beat division lines (BPM Sync mode) ---
    if (clip_->transportMode == Clip::TransportMode::BPMSync && clip_->beatDivision > 0.0f)
    {
        int numBeats = static_cast<int>(clip_->beatDivision);
        if (numBeats >= 2)
        {
            g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f));
            for (int b = 1; b < numBeats; ++b)
            {
                float beatNorm = inP + (outP - inP) * static_cast<float>(b) / static_cast<float>(numBeats);
                int bx = x + static_cast<int>(beatNorm * static_cast<float>(w));
                g.fillRect(bx, trackY + 1, 1, trackH - 2);
            }
        }
    }

    // --- In/Out point handle tabs (bottom edge, bracket-style) ---
    auto handleColor = juce::Colour(AudioDNALookAndFeel::kAccentCyan);

    // In-point: vertical bar + right-pointing bracket tab below track
    g.setColour(handleColor);
    g.fillRect(inX, trackY, 2, trackH);  // vertical bar
    {
        // Small bracket tab: [ shape pointing right
        juce::Path inTab;
        float ix = static_cast<float>(inX);
        float hy = static_cast<float>(handleY);
        inTab.startNewSubPath(ix, hy);
        inTab.lineTo(ix, hy + static_cast<float>(handleZone));
        inTab.lineTo(ix + 8.0f, hy + static_cast<float>(handleZone));
        inTab.lineTo(ix + 8.0f, hy + static_cast<float>(handleZone) - 2.0f);
        inTab.lineTo(ix + 2.0f, hy + static_cast<float>(handleZone) - 2.0f);
        inTab.lineTo(ix + 2.0f, hy);
        inTab.closeSubPath();
        g.fillPath(inTab);
    }

    // Out-point: vertical bar + left-pointing bracket tab below track
    g.fillRect(outX - 2, trackY, 2, trackH);  // vertical bar
    {
        juce::Path outTab;
        float ox = static_cast<float>(outX);
        float hy = static_cast<float>(handleY);
        outTab.startNewSubPath(ox, hy);
        outTab.lineTo(ox, hy + static_cast<float>(handleZone));
        outTab.lineTo(ox - 8.0f, hy + static_cast<float>(handleZone));
        outTab.lineTo(ox - 8.0f, hy + static_cast<float>(handleZone) - 2.0f);
        outTab.lineTo(ox - 2.0f, hy + static_cast<float>(handleZone) - 2.0f);
        outTab.lineTo(ox - 2.0f, hy);
        outTab.closeSubPath();
        g.fillPath(outTab);
    }

    // --- Playhead: downward triangle + vertical line ---
    float pos = static_cast<float>(clip_->playheadPosition);
    int phX = x + static_cast<int>(pos * static_cast<float>(w));

    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    juce::Path tri;
    float triW = 6.0f;
    float triTop = static_cast<float>(bounds.getY() + 2);
    float triBot = static_cast<float>(trackY);
    tri.addTriangle(static_cast<float>(phX) - triW, triTop,
                    static_cast<float>(phX) + triW, triTop,
                    static_cast<float>(phX), triBot);
    g.fillPath(tri);

    // Playhead line through the track
    g.fillRect(phX, trackY, 1, trackH);
}

float ClipInspector::timelineXToNormalized(int mouseX) const
{
    if (timelineBounds_.isEmpty()) return 0.0f;
    float norm = static_cast<float>(mouseX - timelineBounds_.getX())
               / static_cast<float>(timelineBounds_.getWidth());
    return std::clamp(norm, 0.0f, 1.0f);
}

int ClipInspector::normalizedToTimelineX(float norm) const
{
    return timelineBounds_.getX() + static_cast<int>(norm * static_cast<float>(timelineBounds_.getWidth()));
}

void ClipInspector::mouseDown(const juce::MouseEvent& event)
{
    if (!clip_ || timelineBounds_.isEmpty()) return;

    auto pos = event.getPosition();
    if (!timelineBounds_.contains(pos))
        return;

    int mx = pos.getX();
    int inX = normalizedToTimelineX(clip_->inPoint);
    int outX = normalizedToTimelineX(clip_->outPoint);

    // Check proximity to in/out markers (8px hit zone)
    if (std::abs(mx - inX) < 8)
        currentDrag_ = DragTarget::InPoint;
    else if (std::abs(mx - outX) < 8)
        currentDrag_ = DragTarget::OutPoint;
    else
        currentDrag_ = DragTarget::Playhead;

    if (currentDrag_ == DragTarget::Playhead)
    {
        // Click on timeline = scrub playhead
        float norm = timelineXToNormalized(mx);
        clip_->playheadPosition = static_cast<double>(norm);
        if (onCuepointJump)
            onCuepointJump(clip_, static_cast<double>(norm));
        repaint();
    }
}

void ClipInspector::mouseDrag(const juce::MouseEvent& event)
{
    if (!clip_ || currentDrag_ == DragTarget::None) return;

    float norm = timelineXToNormalized(event.getPosition().getX());

    switch (currentDrag_)
    {
        case DragTarget::InPoint:
            clip_->inPoint = std::min(norm, clip_->outPoint - 0.01f);
            repaint();
            break;
        case DragTarget::OutPoint:
            clip_->outPoint = std::max(norm, clip_->inPoint + 0.01f);
            repaint();
            break;
        case DragTarget::Playhead:
            clip_->playheadPosition = static_cast<double>(norm);
            if (onCuepointJump)
                onCuepointJump(clip_, static_cast<double>(norm));
            repaint();
            break;
        default:
            break;
    }
}

void ClipInspector::mouseUp(const juce::MouseEvent&)
{
    currentDrag_ = DragTarget::None;
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
    beatSnapSelector_.setSelectedId(static_cast<int>(clip_->beatSnapMode) + 1, juce::dontSendNotification);

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

    // Cuepoint colors and tooltips
    for (int i = 0; i < kNumCuepoints; ++i)
    {
        bool hasCue = i < clip_->numCuepoints;
        auto& btn = *cuepointBtns_[static_cast<size_t>(i)];
        btn.setColour(juce::TextButton::buttonColourId,
            hasCue ? juce::Colour(0xff3a4a3a) : juce::Colour(AudioDNALookAndFeel::kSurface));
        if (hasCue)
            btn.setTooltip("Position: " + juce::String(clip_->cuepoints[i], 3)
                         + " (Ctrl+click to clear)");
        else if (i == clip_->numCuepoints)
            btn.setTooltip("Click to set cuepoint at current position");
        else
            btn.setTooltip("");
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
    scaleControl_.setParamValue(std::log2(std::max(0.01f, clip_->scale)) / 2.0f + 0.5f);
    rotationControl_.setParamValue(clip_->rotation / 720.0f + 0.5f);
    anchorControl_.setParamValue(clip_->anchorX / 3840.0f + 0.5f);

    updateTransportHighlights();
}

void ClipInspector::updateTransportHighlights()
{
    auto defaultCol = juce::Colour(AudioDNALookAndFeel::kSurface);
    auto activeCol = juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f);

    bool playing = clip_ && clip_->playing;
    bool reverse = clip_ && clip_->reverse;

    playBtn_.setColour(juce::TextButton::buttonColourId,
                       (playing && !reverse) ? activeCol : defaultCol);
    pauseBtn_.setColour(juce::TextButton::buttonColourId,
                        (!playing && clip_) ? activeCol : defaultCol);
    playBackBtn_.setColour(juce::TextButton::buttonColourId,
                           (playing && reverse) ? activeCol : defaultCol);
}

bool ClipInspector::isInterestedInDragSource(const SourceDetails& details)
{
    return clip_ != nullptr && details.description.toString().startsWith("fx:");
}

void ClipInspector::itemDragEnter(const SourceDetails&)
{
    fxDropHighlight_ = true;
    repaint();
}

void ClipInspector::itemDragExit(const SourceDetails&)
{
    fxDropHighlight_ = false;
    repaint();
}

void ClipInspector::itemDropped(const SourceDetails& details)
{
    fxDropHighlight_ = false;
    if (!clip_) return;
    effectStackView_.itemDropped(details);
    repaint();
}
