#include "PreferencesDialog.h"

// ============================================================
// PreferencesDialog
// ============================================================

PreferencesDialog::PreferencesDialog()
    : DialogWindow("Preferences",
                   juce::Colour(AudioDNALookAndFeel::kBackground),
                   true)
{
    setUsingNativeTitleBar(true);
    setContentOwned(new Content(), true);
    setResizable(true, true);
    setResizeLimits(500, 400, 1200, 900);
    centreWithSize(700, 500);
}

void PreferencesDialog::closeButtonPressed()
{
    setVisible(false);
}

void PreferencesDialog::show(juce::Component* parent)
{
    auto* dialog = new PreferencesDialog();
    dialog->setVisible(true);
    dialog->toFront(true);

    if (parent)
    {
        auto parentBounds = parent->getScreenBounds();
        dialog->setCentrePosition(parentBounds.getCentre());
    }
}

// ============================================================
// Content
// ============================================================

PreferencesDialog::Content::Content()
{
    // Tab buttons
    auto addTab = [this](juce::TextButton& btn, Tab tab) {
        addAndMakeVisible(btn);
        btn.onClick = [this, tab] { setActiveTab(tab); };
    };

    addTab(generalBtn_,   Tab::General);
    addTab(audioBtn_,     Tab::Audio);
    addTab(videoBtn_,     Tab::Video);
    addTab(midiBtn_,      Tab::MIDI);
    addTab(recordingBtn_, Tab::Recording);
    addTab(defaultsBtn_,  Tab::Defaults);
    addTab(feedbackBtn_,  Tab::Feedback);
    addTab(aboutBtn_,     Tab::About);

    // General tab controls
    addChildComponent(quitConfirmLabel_);
    addChildComponent(quitConfirmToggle_);
    quitConfirmToggle_.setToggleState(true, juce::dontSendNotification);

    addChildComponent(tooltipLabel_);
    addChildComponent(tooltipToggle_);
    tooltipToggle_.setToggleState(true, juce::dontSendNotification);
    tooltipToggle_.onStateChange = [this] {
        if (onTooltipToggled)
            onTooltipToggled(tooltipToggle_.getToggleState());
    };

    // Audio tab controls
    addChildComponent(sampleRateLabel_);
    addChildComponent(sampleRateSelector_);
    sampleRateSelector_.addItem("44100", 1);
    sampleRateSelector_.addItem("48000", 2);
    sampleRateSelector_.addItem("96000", 3);
    sampleRateSelector_.setSelectedId(2, juce::dontSendNotification);

    addChildComponent(bufferSizeLabel_);
    addChildComponent(bufferSizeSelector_);
    bufferSizeSelector_.addItem("64", 1);
    bufferSizeSelector_.addItem("128", 2);
    bufferSizeSelector_.addItem("256", 3);
    bufferSizeSelector_.addItem("512", 4);
    bufferSizeSelector_.addItem("1024", 5);
    bufferSizeSelector_.setSelectedId(2, juce::dontSendNotification);

    addChildComponent(bpmRangeLabel_);
    addChildComponent(bpmRangeSelector_);
    bpmRangeSelector_.addItem("60-200 (Standard)", 1);
    bpmRangeSelector_.addItem("80-180 (DJ)", 2);
    bpmRangeSelector_.addItem("40-240 (Extended)", 3);
    bpmRangeSelector_.setSelectedId(1, juce::dontSendNotification);

    // Video tab controls
    addChildComponent(fpsTargetLabel_);
    addChildComponent(fpsTargetSelector_);
    fpsTargetSelector_.addItem("30", 1);
    fpsTargetSelector_.addItem("60", 2);
    fpsTargetSelector_.addItem("120", 3);
    fpsTargetSelector_.setSelectedId(2, juce::dontSendNotification);

    addChildComponent(renderResLabel_);
    addChildComponent(renderResSelector_);
    renderResSelector_.addItem("Auto", 1);
    renderResSelector_.addItem("1280x720", 2);
    renderResSelector_.addItem("1920x1080", 3);
    renderResSelector_.addItem("2560x1440", 4);
    renderResSelector_.addItem("3840x2160", 5);
    renderResSelector_.setSelectedId(1, juce::dontSendNotification);

    // About tab
    addChildComponent(versionLabel_);
    versionLabel_.setText("Audio-DNA v0.1.0", juce::dontSendNotification);
    versionLabel_.setFont(juce::Font(juce::FontOptions(18.0f).withStyle("Bold")));
    versionLabel_.setColour(juce::Label::textColourId,
                             juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    versionLabel_.setJustificationType(juce::Justification::centred);

    addChildComponent(creditsLabel_);
    creditsLabel_.setText(
        "Audio-reactive visual performance tool.\n\n"
        "Built with JUCE, Aubio, OpenGL 4.1.\n\n"
        "C++20 / macOS / Windows / Linux",
        juce::dontSendNotification);
    creditsLabel_.setColour(juce::Label::textColourId,
                             juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    creditsLabel_.setJustificationType(juce::Justification::centred);

    // Style labels
    auto styleLabel = [](juce::Label& lbl) {
        lbl.setFont(juce::Font(juce::FontOptions(12.0f)));
        lbl.setColour(juce::Label::textColourId,
                       juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        lbl.setJustificationType(juce::Justification::centredRight);
    };
    styleLabel(quitConfirmLabel_);
    styleLabel(sampleRateLabel_);
    styleLabel(bufferSizeLabel_);
    styleLabel(bpmRangeLabel_);
    styleLabel(fpsTargetLabel_);
    styleLabel(renderResLabel_);

    updateTabButtonColors();
    showActiveTab();

    setSize(700, 500);
}

void PreferencesDialog::Content::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(AudioDNALookAndFeel::kBackground));

    // Tab bar background
    auto tabBar = getLocalBounds().removeFromTop(kTabBarHeight);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRect(tabBar);

    // Border below tabs
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawHorizontalLine(kTabBarHeight - 1, 0.0f, static_cast<float>(getWidth()));
}

void PreferencesDialog::Content::resized()
{
    auto area = getLocalBounds();
    auto tabBar = area.removeFromTop(kTabBarHeight);

    int tabWidth = tabBar.getWidth() / 8;
    generalBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    audioBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    videoBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    midiBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    recordingBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    defaultsBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    feedbackBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    aboutBtn_.setBounds(tabBar);

    auto content = area.reduced(20);

    switch (activeTab_)
    {
        case Tab::General:   layoutGeneralTab(content); break;
        case Tab::Audio:     layoutAudioTab(content); break;
        case Tab::Video:     layoutVideoTab(content); break;
        case Tab::MIDI:      layoutPlaceholderTab(content, "MIDI settings will be available when MIDI support is added."); break;
        case Tab::Recording: layoutPlaceholderTab(content, "Recording settings will be available when recording is implemented."); break;
        case Tab::Defaults:  layoutPlaceholderTab(content, "Default transport, play mode, and blend mode settings."); break;
        case Tab::Feedback:  layoutPlaceholderTab(content, "Send feedback about Audio-DNA."); break;
        case Tab::About:     layoutAboutTab(content); break;
    }
}

void PreferencesDialog::Content::setActiveTab(Tab tab)
{
    if (activeTab_ == tab)
        return;
    activeTab_ = tab;
    updateTabButtonColors();
    showActiveTab();
    resized();
    repaint();
}

void PreferencesDialog::Content::updateTabButtonColors()
{
    auto style = [this](juce::TextButton& btn, Tab tab) {
        bool active = (activeTab_ == tab);
        btn.setColour(juce::TextButton::buttonColourId,
                       juce::Colour(active ? AudioDNALookAndFeel::kSurfaceLight
                                           : AudioDNALookAndFeel::kSurface));
        btn.setColour(juce::TextButton::textColourOffId,
                       juce::Colour(active ? AudioDNALookAndFeel::kAccentCyan
                                           : AudioDNALookAndFeel::kTextSecondary));
    };

    style(generalBtn_,   Tab::General);
    style(audioBtn_,     Tab::Audio);
    style(videoBtn_,     Tab::Video);
    style(midiBtn_,      Tab::MIDI);
    style(recordingBtn_, Tab::Recording);
    style(defaultsBtn_,  Tab::Defaults);
    style(feedbackBtn_,  Tab::Feedback);
    style(aboutBtn_,     Tab::About);
}

void PreferencesDialog::Content::showActiveTab()
{
    // Hide all
    quitConfirmLabel_.setVisible(false);
    quitConfirmToggle_.setVisible(false);
    sampleRateLabel_.setVisible(false);
    sampleRateSelector_.setVisible(false);
    bufferSizeLabel_.setVisible(false);
    bufferSizeSelector_.setVisible(false);
    bpmRangeLabel_.setVisible(false);
    bpmRangeSelector_.setVisible(false);
    fpsTargetLabel_.setVisible(false);
    fpsTargetSelector_.setVisible(false);
    renderResLabel_.setVisible(false);
    renderResSelector_.setVisible(false);
    versionLabel_.setVisible(false);
    creditsLabel_.setVisible(false);
    tooltipLabel_.setVisible(false);
    tooltipToggle_.setVisible(false);

    switch (activeTab_)
    {
        case Tab::General:
            quitConfirmLabel_.setVisible(true);
            quitConfirmToggle_.setVisible(true);
            tooltipLabel_.setVisible(true);
            tooltipToggle_.setVisible(true);
            break;
        case Tab::Audio:
            sampleRateLabel_.setVisible(true);
            sampleRateSelector_.setVisible(true);
            bufferSizeLabel_.setVisible(true);
            bufferSizeSelector_.setVisible(true);
            bpmRangeLabel_.setVisible(true);
            bpmRangeSelector_.setVisible(true);
            break;
        case Tab::Video:
            fpsTargetLabel_.setVisible(true);
            fpsTargetSelector_.setVisible(true);
            renderResLabel_.setVisible(true);
            renderResSelector_.setVisible(true);
            break;
        case Tab::About:
            versionLabel_.setVisible(true);
            creditsLabel_.setVisible(true);
            break;
        default:
            break;
    }
}

void PreferencesDialog::Content::layoutGeneralTab(juce::Rectangle<int> area)
{
    auto row = area.removeFromTop(28);
    quitConfirmLabel_.setBounds(row.removeFromLeft(150));
    row.removeFromLeft(8);
    quitConfirmToggle_.setBounds(row.removeFromLeft(28));

    area.removeFromTop(4);
    auto row2 = area.removeFromTop(28);
    tooltipLabel_.setBounds(row2.removeFromLeft(150));
    row2.removeFromLeft(8);
    tooltipToggle_.setBounds(row2.removeFromLeft(28));
}

void PreferencesDialog::Content::layoutAudioTab(juce::Rectangle<int> area)
{
    int labelW = 160;
    int controlW = 200;

    auto row1 = area.removeFromTop(28);
    sampleRateLabel_.setBounds(row1.removeFromLeft(labelW));
    row1.removeFromLeft(8);
    sampleRateSelector_.setBounds(row1.removeFromLeft(controlW));

    area.removeFromTop(8);
    auto row2 = area.removeFromTop(28);
    bufferSizeLabel_.setBounds(row2.removeFromLeft(labelW));
    row2.removeFromLeft(8);
    bufferSizeSelector_.setBounds(row2.removeFromLeft(controlW));

    area.removeFromTop(8);
    auto row3 = area.removeFromTop(28);
    bpmRangeLabel_.setBounds(row3.removeFromLeft(labelW));
    row3.removeFromLeft(8);
    bpmRangeSelector_.setBounds(row3.removeFromLeft(controlW));
}

void PreferencesDialog::Content::layoutVideoTab(juce::Rectangle<int> area)
{
    int labelW = 160;
    int controlW = 200;

    auto row1 = area.removeFromTop(28);
    fpsTargetLabel_.setBounds(row1.removeFromLeft(labelW));
    row1.removeFromLeft(8);
    fpsTargetSelector_.setBounds(row1.removeFromLeft(controlW));

    area.removeFromTop(8);
    auto row2 = area.removeFromTop(28);
    renderResLabel_.setBounds(row2.removeFromLeft(labelW));
    row2.removeFromLeft(8);
    renderResSelector_.setBounds(row2.removeFromLeft(controlW));
}

void PreferencesDialog::Content::layoutAboutTab(juce::Rectangle<int> area)
{
    area.removeFromTop(40);
    versionLabel_.setBounds(area.removeFromTop(30));
    area.removeFromTop(20);
    creditsLabel_.setBounds(area.removeFromTop(120));
}

void PreferencesDialog::Content::layoutPlaceholderTab(juce::Rectangle<int> /*area*/,
                                                       const juce::String& /*tabName*/)
{
    // Placeholder tabs just show the message in paint() if needed.
    // For now, they are empty with the message shown via the tab content area.
}
