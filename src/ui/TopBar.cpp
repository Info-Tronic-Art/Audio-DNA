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
    tapButton_.onClick = [this]
    {
        double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        if (now - lastTapTime_ > 2.0)
            tapCount_ = 0;  // reset if gap > 2s

        if (tapCount_ < static_cast<int>(tapTimes_.size()))
            tapTimes_[static_cast<size_t>(tapCount_)] = now;
        ++tapCount_;
        lastTapTime_ = now;

        if (onTapTempo)
            onTapTempo();
    };

    addAndMakeVisible(resyncButton_);
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
    }

    updateBpmDisplay();
}

void TopBar::updateBpmDisplay()
{
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

    // Tempo display (BPM number + state)
    tempoLabel_.setBounds(area.removeFromLeft(50).withTrimmedTop(6));
    trackerStateLabel_.setBounds(juce::Rectangle<int>(
        tempoLabel_.getRight() + 2, tempoLabel_.getY() + 6,
        60, 14));
    area.removeFromLeft(64);

    // Tap + Resync
    tapButton_.setBounds(area.removeFromLeft(32));
    area.removeFromLeft(2);
    resyncButton_.setBounds(area.removeFromLeft(50));
    area.removeFromLeft(6);

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
