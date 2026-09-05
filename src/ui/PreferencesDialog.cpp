#include "PreferencesDialog.h"

// ============================================================
// PreferencesDialog
// ============================================================

PreferencesDialog::PreferencesDialog(bool tooltipsEnabled,
                                     std::function<void(bool)> onTooltipToggled,
                                     const juce::String& milkDropDir,
                                     std::function<void(juce::String)> onMilkDropDirChanged)
    : DialogWindow("Preferences",
                   juce::Colour(AudioDNALookAndFeel::kBackground),
                   true)
{
    setUsingNativeTitleBar(true);
    setContentOwned(new Content(tooltipsEnabled, std::move(onTooltipToggled),
                                milkDropDir, std::move(onMilkDropDirChanged)), true);
    setResizable(true, true);
    setResizeLimits(500, 400, 1200, 900);
    centreWithSize(700, 500);
}

void PreferencesDialog::closeButtonPressed()
{
    setVisible(false);
}

void PreferencesDialog::show(juce::Component* parent, bool tooltipsEnabled,
                             std::function<void(bool)> onTooltipToggled,
                             const juce::String& milkDropDir,
                             std::function<void(juce::String)> onMilkDropDirChanged)
{
    auto* dialog = new PreferencesDialog(tooltipsEnabled, std::move(onTooltipToggled),
                                         milkDropDir, std::move(onMilkDropDirChanged));
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

PreferencesDialog::Content::Content(bool tooltipsEnabled,
                                    std::function<void(bool)> onTooltipToggledCb,
                                    const juce::String& milkDropDir,
                                    std::function<void(juce::String)> onMilkDropDirChangedCb)
{
    onTooltipToggled = std::move(onTooltipToggledCb);
    onMilkDropDirChanged = std::move(onMilkDropDirChangedCb);

    // Tab buttons
    auto addTab = [this](juce::TextButton& btn, Tab tab) {
        addAndMakeVisible(btn);
        btn.onClick = [this, tab] { setActiveTab(tab); };
    };

    addTab(generalBtn_, Tab::General);
    addTab(videoBtn_,   Tab::Video);
    addTab(aboutBtn_,   Tab::About);

    // General tab controls — seed toggle to caller's current state (no callback fired)
    addChildComponent(tooltipLabel_);
    addChildComponent(tooltipToggle_);
    tooltipToggle_.setToggleState(tooltipsEnabled, juce::dontSendNotification);
    tooltipToggle_.onStateChange = [this] {
        if (onTooltipToggled)
            onTooltipToggled(tooltipToggle_.getToggleState());
    };

    // MilkDrop preset directory (Video tab). Seeds from the caller's
    // persisted value (no notification, matching tooltipToggle_'s seeding
    // above) — fires onMilkDropDirChanged only once the user commits a new
    // value (Return, focus-lost, or a Browse pick), not on every keystroke.
    addChildComponent(milkDropDirLabel_);
    addChildComponent(milkDropDirEdit_);
    milkDropDirEdit_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    milkDropDirEdit_.setColour(juce::TextEditor::textColourId,
                                juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    milkDropDirEdit_.setTextToShowWhenEmpty("Path to .milk preset folder...", juce::Colour(0xff606070));
    milkDropDirEdit_.setText(milkDropDir, juce::dontSendNotification);
    milkDropDirEdit_.onReturnKey = [this] { commitMilkDropDir(); };
    milkDropDirEdit_.onFocusLost = [this] { commitMilkDropDir(); };
    addChildComponent(milkDropBrowseBtn_);
    milkDropBrowseBtn_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a3e));
    milkDropBrowseBtn_.setColour(juce::TextButton::textColourOffId,
                                  juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    milkDropBrowseBtn_.onClick = [this] {
        auto chooser = std::make_shared<juce::FileChooser>(
            "Select MilkDrop Preset Directory", juce::File{}, "");
        chooser->launchAsync(juce::FileBrowserComponent::openMode
                           | juce::FileBrowserComponent::canSelectDirectories,
            [this, chooser](const juce::FileChooser& fc) {
                auto result = fc.getResult();
                if (result.isDirectory())
                {
                    milkDropDirEdit_.setText(result.getFullPathName(), juce::dontSendNotification);
                    commitMilkDropDir();
                }
            });
    };

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
    styleLabel(milkDropDirLabel_);

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

    int tabWidth = tabBar.getWidth() / 3;
    generalBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    videoBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    aboutBtn_.setBounds(tabBar);

    auto content = area.reduced(20);

    switch (activeTab_)
    {
        case Tab::General:   layoutGeneralTab(content); break;
        case Tab::Video:     layoutVideoTab(content); break;
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

    style(generalBtn_, Tab::General);
    style(videoBtn_,   Tab::Video);
    style(aboutBtn_,   Tab::About);
}

void PreferencesDialog::Content::showActiveTab()
{
    // Hide all
    milkDropDirLabel_.setVisible(false);
    milkDropDirEdit_.setVisible(false);
    milkDropBrowseBtn_.setVisible(false);
    versionLabel_.setVisible(false);
    creditsLabel_.setVisible(false);
    tooltipLabel_.setVisible(false);
    tooltipToggle_.setVisible(false);

    switch (activeTab_)
    {
        case Tab::General:
            tooltipLabel_.setVisible(true);
            tooltipToggle_.setVisible(true);
            break;
        case Tab::Video:
            milkDropDirLabel_.setVisible(true);
            milkDropDirEdit_.setVisible(true);
            milkDropBrowseBtn_.setVisible(true);
            break;
        case Tab::About:
            versionLabel_.setVisible(true);
            creditsLabel_.setVisible(true);
            break;
    }
}

void PreferencesDialog::Content::layoutGeneralTab(juce::Rectangle<int> area)
{
    auto row = area.removeFromTop(28);
    tooltipLabel_.setBounds(row.removeFromLeft(150));
    row.removeFromLeft(8);
    tooltipToggle_.setBounds(row.removeFromLeft(28));
}

void PreferencesDialog::Content::layoutVideoTab(juce::Rectangle<int> area)
{
    int labelW = 160;

    auto row = area.removeFromTop(28);
    milkDropDirLabel_.setBounds(row.removeFromLeft(labelW));
    row.removeFromLeft(8);
    milkDropBrowseBtn_.setBounds(row.removeFromRight(80));
    row.removeFromRight(4);
    milkDropDirEdit_.setBounds(row);
}

void PreferencesDialog::Content::layoutAboutTab(juce::Rectangle<int> area)
{
    area.removeFromTop(40);
    versionLabel_.setBounds(area.removeFromTop(30));
    area.removeFromTop(20);
    creditsLabel_.setBounds(area.removeFromTop(120));
}

void PreferencesDialog::Content::commitMilkDropDir()
{
    auto text = milkDropDirEdit_.getText().trim();
    if (text.isNotEmpty() && !juce::File(text).isDirectory())
        return; // ignore invalid typed path — matches FilesBrowser::pathBar_'s onReturnKey

    if (onMilkDropDirChanged)
        onMilkDropDirChanged(text);
}
