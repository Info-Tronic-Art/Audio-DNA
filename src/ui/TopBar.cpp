#include "TopBar.h"
#include <cmath>

TopBar::TopBar(FeatureBus& featureBus, Composition& composition)
    : featureBus_(featureBus), composition_(composition)
{
    displaySnap_.clear();

    // Audio source
    addAndMakeVisible(audioSourceLabel_);
    audioSourceLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    audioSourceLabel_.setColour(juce::Label::textColourId,
                                juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    addAndMakeVisible(audioSourceSelector_);
    addAndMakeVisible(inputGainLabel_);
    inputGainLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    inputGainLabel_.setColour(juce::Label::textColourId,
                              juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(inputGainSlider_);
    inputGainSlider_.setRange(0.0, 4.0, 0.01);
    inputGainSlider_.setValue(1.0, juce::dontSendNotification);
    inputGainSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    inputGainSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // Transport
    addAndMakeVisible(playButton_);
    addAndMakeVisible(pauseButton_);
    addAndMakeVisible(stopButton_);

    // Tempo display
    addAndMakeVisible(tempoLabel_);
    tempoLabel_.setFont(juce::Font(juce::FontOptions(18.0f, juce::Font::bold)));
    tempoLabel_.setColour(juce::Label::textColourId,
                          juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    tempoLabel_.setText("---", juce::dontSendNotification);
    tempoLabel_.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(trackerStateLabel_);
    trackerStateLabel_.setFont(juce::Font(juce::FontOptions(9.0f)));
    trackerStateLabel_.setText("SEARCHING", juce::dontSendNotification);

    // Tap & Resync
    addAndMakeVisible(tapButton_);
    tapButton_.setTooltip("Tap rhythmically to set BPM manually");
    tapButton_.onClick = [this]
    {
        double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        if (now - lastTapTime_ > 2.0)
            tapCount_ = 0;  // reset if gap > 2s

        if (tapCount_ < static_cast<int>(tapTimes_.size()))
            tapTimes_[static_cast<size_t>(tapCount_)] = now;
        ++tapCount_;
        lastTapTime_ = now;

        // Compute BPM from tap intervals (need at least 2 taps)
        if (onTapTempo && tapCount_ >= 2)
        {
            int n = std::min(tapCount_, static_cast<int>(tapTimes_.size()));
            double totalInterval = tapTimes_[static_cast<size_t>(n - 1)] - tapTimes_[0];
            if (totalInterval > 0.0)
            {
                double avgInterval = totalInterval / (n - 1);
                float tappedBPM = static_cast<float>(60.0 / avgInterval);
                onTapTempo(tappedBPM);
            }
        }
    };

    addAndMakeVisible(resyncButton_);
    resyncButton_.setTooltip("Reset beat phase to sync with the music");
    resyncButton_.onClick = [this]
    {
        if (onResync)
            onResync();
        // Flash
        resyncButton_.setColour(juce::TextButton::buttonColourId,
                                juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        juce::Timer::callAfterDelay(200, [this]
        {
            resyncButton_.removeColour(juce::TextButton::buttonColourId);
        });
    };

    // Manual BPM mode
    addAndMakeVisible(manualModeBtn_);
    manualModeBtn_.setTooltip("Switch between auto-detect and manual BPM");
    manualModeBtn_.setColour(juce::ToggleButton::textColourId,
                             juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    manualModeBtn_.onStateChange = [this] {
        manualMode_ = manualModeBtn_.getToggleState();
        bpmEditField_.setVisible(manualMode_);
        trackerStateLabel_.setVisible(!manualMode_);
        if (onManualBpmChanged)
        {
            float bpm = bpmEditField_.getText().getFloatValue();
            onManualBpmChanged(manualMode_, bpm > 0 ? bpm : 120.0f);
        }
        // Must call resized() FIRST to set bounds, THEN grab focus
        resized();
        if (manualMode_)
        {
            int currentBpm = displaySnap_.bpm > 0.0f ? static_cast<int>(displaySnap_.bpm) : 120;
            bpmEditField_.setText(juce::String(currentBpm), false);
            bpmEditField_.grabKeyboardFocus();
            bpmEditField_.selectAll();
        }
    };

    bpmEditField_.setJustification(juce::Justification::centred);
    bpmEditField_.setFont(juce::Font(juce::FontOptions(14.0f)).boldened());
    bpmEditField_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    bpmEditField_.setColour(juce::TextEditor::textColourId,
                            juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    bpmEditField_.setInputRestrictions(6, "0123456789.");
    bpmEditField_.setTooltip("Type BPM value and press Enter");
    bpmEditField_.setWantsKeyboardFocus(true);
    bpmEditField_.setSelectAllWhenFocused(true);
    bpmEditField_.onReturnKey = [this] {
        float bpm = bpmEditField_.getText().getFloatValue();
        if (bpm > 0.0f && onManualBpmChanged)
            onManualBpmChanged(true, bpm);
    };
    addChildComponent(bpmEditField_);

    // BPM Multiplier buttons
    auto setupMultBtn = [this](juce::TextButton& btn, int mult)
    {
        addAndMakeVisible(btn);
        btn.onClick = [this, mult] { handleMultiplierButton(mult); };
    };
    setupMultBtn(multDiv4Button_, -4);
    setupMultBtn(multDiv2Button_, -2);
    setupMultBtn(multX1Button_, 1);
    setupMultBtn(multX2Button_, 2);
    setupMultBtn(multX4Button_, 4);

    // Highlight the default multiplier
    multX1Button_.setColour(juce::TextButton::buttonColourId,
                            juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f));

    // Quantize
    addAndMakeVisible(quantizeLabel_);
    quantizeLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    quantizeLabel_.setColour(juce::Label::textColourId,
                             juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(quantizeSelector_);
    quantizeSelector_.addItem("Off", 1);
    quantizeSelector_.addItem("Next Beat", 2);
    quantizeSelector_.addItem("Next Downbeat", 3);
    quantizeSelector_.setSelectedId(1, juce::dontSendNotification);
    quantizeSelector_.onChange = [this]
    {
        auto mode = static_cast<Composition::QuantizeMode>(quantizeSelector_.getSelectedId() - 1);
        composition_.quantizeMode = mode;
        if (onQuantizeChanged)
            onQuantizeChanged(mode);
    };

    // Fade
    addAndMakeVisible(fadeLabel_);
    fadeLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    fadeLabel_.setColour(juce::Label::textColourId,
                         juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(fadeSlider_);
    fadeSlider_.setRange(0.0, 5.0, 0.01);
    fadeSlider_.setValue(static_cast<double>(composition_.globalTransitionSpeed),
                        juce::dontSendNotification);
    fadeSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    fadeSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 35, 20);
    fadeSlider_.onValueChange = [this]
    {
        composition_.globalTransitionSpeed = static_cast<float>(fadeSlider_.getValue());
    };

    // Master level
    addAndMakeVisible(masterLabel_);
    masterLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    masterLabel_.setColour(juce::Label::textColourId,
                           juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(masterLevelSlider_);
    masterLevelSlider_.setRange(0.0, 1.0, 0.01);
    masterLevelSlider_.setValue(1.0, juce::dontSendNotification);
    masterLevelSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    masterLevelSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // Output
    addAndMakeVisible(outputLabel_);
    outputLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    outputLabel_.setColour(juce::Label::textColourId,
                           juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(displaySelector_);

    // Stats
    addAndMakeVisible(fpsLabel_);
    fpsLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    fpsLabel_.setColour(juce::Label::textColourId,
                        juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(dspLabel_);
    dspLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    dspLabel_.setColour(juce::Label::textColourId,
                        juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    startTimerHz(15);
}

void TopBar::timerCallback()
{
    const FeatureSnapshot* newSnap = featureBus_.acquireRead();
    const FeatureSnapshot* snap = newSnap ? newSnap : featureBus_.getLatestRead();
    if (snap != nullptr)
    {
        displaySnap_.bpm = snap->bpm;
        displaySnap_.trackerState = snap->trackerState;
        displaySnap_.beatInBar = snap->beatInBar;
        displaySnap_.barPhase = snap->barPhase;
        displaySnap_.beatPhase = snap->beatPhase;
        displaySnap_.downbeatDetected = snap->downbeatDetected;
        displaySnap_.phrasePhase = snap->phrasePhase;
        displaySnap_.barCount = snap->barCount;
    }

    updateBpmDisplay();
    // Repaint the beat wheel and bar/phrase area
    if (!beatWheelBounds_.isEmpty())
        repaint(beatWheelBounds_.getUnion(barPhraseBounds_).expanded(2));
}

void TopBar::updateBpmDisplay()
{
    // In manual mode, show the manual BPM and skip auto-display
    if (manualMode_)
    {
        tempoLabel_.setText(bpmEditField_.getText().isNotEmpty()
                            ? bpmEditField_.getText()
                            : "120",
                            juce::dontSendNotification);
        return;
    }

    // BPM value
    if (displaySnap_.bpm > 0.0f)
        tempoLabel_.setText(juce::String(static_cast<int>(displaySnap_.bpm + 0.5f)),
                            juce::dontSendNotification);
    else
        tempoLabel_.setText("---", juce::dontSendNotification);

    // Tracker state
    juce::String stateName;
    juce::Colour stateCol;
    switch (displaySnap_.trackerState)
    {
        case 0:
            stateName = "SEARCHING";
            stateCol = juce::Colour(AudioDNALookAndFeel::kMeterRed);
            break;
        case 1:
            stateName = "LOCKING";
            stateCol = juce::Colour(AudioDNALookAndFeel::kMeterYellow);
            break;
        case 2:
            stateName = "LOCKED";
            stateCol = juce::Colour(AudioDNALookAndFeel::kMeterGreen);
            break;
        default:
            stateName = "?";
            stateCol = juce::Colour(AudioDNALookAndFeel::kTextSecondary);
            break;
    }

    tempoLabel_.setColour(juce::Label::textColourId, stateCol);
    trackerStateLabel_.setColour(juce::Label::textColourId, stateCol);
    trackerStateLabel_.setText(stateName, juce::dontSendNotification);

    // Stats
    fpsLabel_.setText("FPS:" + juce::String(static_cast<int>(currentFps_)),
                      juce::dontSendNotification);
    dspLabel_.setText("DSP:" + juce::String(currentDspLoad_, 1) + "%",
                      juce::dontSendNotification);
}

void TopBar::setFps(float fps) { currentFps_ = fps; }
void TopBar::setDspLoad(float percent) { currentDspLoad_ = percent; }

void TopBar::handleMultiplierButton(int multiplier)
{
    composition_.bpmMultiplier = multiplier;

    // Update button highlights
    auto resetCol = [](juce::TextButton& btn) {
        btn.removeColour(juce::TextButton::buttonColourId);
    };
    resetCol(multDiv4Button_);
    resetCol(multDiv2Button_);
    resetCol(multX1Button_);
    resetCol(multX2Button_);
    resetCol(multX4Button_);

    auto highlight = juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f);
    switch (multiplier)
    {
        case -4: multDiv4Button_.setColour(juce::TextButton::buttonColourId, highlight); break;
        case -2: multDiv2Button_.setColour(juce::TextButton::buttonColourId, highlight); break;
        case  1: multX1Button_.setColour(juce::TextButton::buttonColourId, highlight); break;
        case  2: multX2Button_.setColour(juce::TextButton::buttonColourId, highlight); break;
        case  4: multX4Button_.setColour(juce::TextButton::buttonColourId, highlight); break;
    }

    if (onBpmMultiplierChanged)
        onBpmMultiplierChanged(multiplier);
}

void TopBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRect(bounds);

    // Bottom border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.fillRect(bounds.getX(), bounds.getBottom() - 1.0f, bounds.getWidth(), 1.0f);

    // "Tempo" label before the BPM number
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    auto tempoLabelRect = tempoLabel_.getBounds().toFloat();
    g.drawText("Tempo",
               juce::Rectangle<float>(tempoLabelRect.getX(), tempoLabelRect.getY() - 10.0f,
                                      60.0f, 10.0f),
               juce::Justification::centredLeft);

    // Beat wheel
    paintBeatWheel(g);

    // Bar / Phrase display
    paintBarPhraseDisplay(g);
}

void TopBar::paintBeatWheel(juce::Graphics& g) const
{
    if (beatWheelBounds_.isEmpty()) return;

    auto bounds = beatWheelBounds_.toFloat();
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.45f;
    float innerRadius = radius * 0.5f;
    float gapAngle = 0.12f; // radians gap between segments

    // 4 arc segments: top, right, bottom, left (beat 0 at top)
    // Each segment spans ~90 degrees minus gap
    auto accentCyan = juce::Colour(AudioDNALookAndFeel::kAccentCyan);
    auto dimColor = juce::Colour(0xff333333);
    int currentBeat = static_cast<int>(displaySnap_.beatInBar);
    bool hasBpm = displaySnap_.bpm > 0.0f && displaySnap_.trackerState >= 1;

    for (int i = 0; i < 4; ++i)
    {
        // Start angle: 12 o'clock = -pi/2, going clockwise
        float startAngle = -juce::MathConstants<float>::halfPi
                         + static_cast<float>(i) * juce::MathConstants<float>::halfPi
                         + gapAngle * 0.5f;
        float endAngle = startAngle + juce::MathConstants<float>::halfPi - gapAngle;

        // Color: bright for current beat, medium for previous, dim for others
        juce::Colour segColor;
        if (!hasBpm)
        {
            segColor = dimColor;
        }
        else if (i == currentBeat)
        {
            // Fade within the beat based on beatPhase (bright at start, dims toward end)
            float brightness = 1.0f - displaySnap_.beatPhase * 0.5f;
            segColor = accentCyan.withAlpha(brightness);
        }
        else
        {
            segColor = dimColor;
        }

        // Draw arc segment as filled path
        juce::Path arc;
        arc.addCentredArc(cx, cy, radius, radius,
                          0.0f, startAngle, endAngle, true);
        arc.addCentredArc(cx, cy, innerRadius, innerRadius,
                          0.0f, endAngle, startAngle, false);
        arc.closeSubPath();

        g.setColour(segColor);
        g.fillPath(arc);
    }
}

void TopBar::paintBarPhraseDisplay(juce::Graphics& g) const
{
    if (barPhraseBounds_.isEmpty()) return;
    bool hasBpm = displaySnap_.bpm > 0.0f && displaySnap_.trackerState >= 1;

    auto bounds = barPhraseBounds_.toFloat();
    g.setFont(juce::Font(juce::FontOptions(9.0f)));

    // Top line: "Bar N" (bar count since reset)
    auto topHalf = bounds.removeFromTop(bounds.getHeight() * 0.5f);
    g.setColour(hasBpm ? juce::Colour(AudioDNALookAndFeel::kTextPrimary)
                       : juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    if (hasBpm)
        g.drawText("Bar " + juce::String(displaySnap_.barCount + 1),
                   topHalf.toNearestInt(), juce::Justification::centredLeft, false);
    else
        g.drawText("Bar -", topHalf.toNearestInt(), juce::Justification::centredLeft, false);

    // Bottom line: "Phr 0.XX" (phrase phase)
    g.setColour(hasBpm ? juce::Colour(AudioDNALookAndFeel::kTextSecondary)
                       : juce::Colour(0xff444444));
    if (hasBpm)
        g.drawText("Phr " + juce::String(displaySnap_.phrasePhase, 2),
                   bounds.toNearestInt(), juce::Justification::centredLeft, false);
    else
        g.drawText("Phr -", bounds.toNearestInt(), juce::Justification::centredLeft, false);
}

void TopBar::resized()
{
    auto area = getLocalBounds().reduced(4, 2);

    // Audio source section
    audioSourceLabel_.setBounds(area.removeFromLeft(38));
    audioSourceSelector_.setBounds(area.removeFromLeft(90));
    area.removeFromLeft(4);
    inputGainLabel_.setBounds(area.removeFromLeft(30));
    inputGainSlider_.setBounds(area.removeFromLeft(70));
    area.removeFromLeft(6);

    // Separator
    area.removeFromLeft(2);

    // Transport
    int transportBtnW = 24;
    playButton_.setBounds(area.removeFromLeft(transportBtnW));
    area.removeFromLeft(1);
    pauseButton_.setBounds(area.removeFromLeft(transportBtnW));
    area.removeFromLeft(1);
    stopButton_.setBounds(area.removeFromLeft(transportBtnW));
    area.removeFromLeft(6);

    // Separator
    area.removeFromLeft(2);

    // Beat wheel (4-segment circle showing current beat)
    beatWheelBounds_ = area.removeFromLeft(26).reduced(1);
    area.removeFromLeft(2);

    // Bar/Phrase readout (small text: "Bar N" / "Phr 0.XX")
    barPhraseBounds_ = area.removeFromLeft(44);
    area.removeFromLeft(2);

    // Tempo display (BPM number + state)
    tempoLabel_.setBounds(area.removeFromLeft(50).withTrimmedTop(6));
    trackerStateLabel_.setBounds(juce::Rectangle<int>(
        tempoLabel_.getRight() + 2, tempoLabel_.getY() + 6,
        60, 14));
    area.removeFromLeft(64);

    // Tap + Resync + Manual
    tapButton_.setBounds(area.removeFromLeft(32));
    area.removeFromLeft(2);
    resyncButton_.setBounds(area.removeFromLeft(50));
    area.removeFromLeft(2);
    manualModeBtn_.setBounds(area.removeFromLeft(80));
    area.removeFromLeft(2);

    // BPM edit field: own space next to manual button when active
    if (manualMode_)
    {
        bpmEditField_.setBounds(area.removeFromLeft(60).withTrimmedTop(4).withTrimmedBottom(4));
        area.removeFromLeft(4);
    }

    // BPM Multiplier buttons
    int multBtnW = 26;
    multDiv4Button_.setBounds(area.removeFromLeft(multBtnW));
    area.removeFromLeft(1);
    multDiv2Button_.setBounds(area.removeFromLeft(multBtnW));
    area.removeFromLeft(1);
    multX1Button_.setBounds(area.removeFromLeft(multBtnW));
    area.removeFromLeft(1);
    multX2Button_.setBounds(area.removeFromLeft(multBtnW));
    area.removeFromLeft(1);
    multX4Button_.setBounds(area.removeFromLeft(multBtnW));
    area.removeFromLeft(6);

    // Quantize
    quantizeLabel_.setBounds(area.removeFromLeft(55));
    quantizeSelector_.setBounds(area.removeFromLeft(100));
    area.removeFromLeft(6);

    // Fade
    fadeLabel_.setBounds(area.removeFromLeft(30));
    fadeSlider_.setBounds(area.removeFromLeft(100));
    area.removeFromLeft(6);

    // Right side: stats + output + master
    auto rightSection = area;

    dspLabel_.setBounds(rightSection.removeFromRight(55));
    fpsLabel_.setBounds(rightSection.removeFromRight(45));
    rightSection.removeFromRight(6);

    displaySelector_.setBounds(rightSection.removeFromRight(100));
    outputLabel_.setBounds(rightSection.removeFromRight(42));
    rightSection.removeFromRight(4);

    masterLevelSlider_.setBounds(rightSection.removeFromRight(70));
    masterLabel_.setBounds(rightSection.removeFromRight(42));
}
