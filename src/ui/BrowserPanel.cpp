#include "ui/BrowserPanel.h"

BrowserPanel::BrowserPanel()
{
    // Tab buttons
    auto setupTabBtn = [this](juce::TextButton& btn, Tab tab) {
        btn.setColour(juce::TextButton::buttonColourId,
                      juce::Colour(AudioDNALookAndFeel::kSurface));
        btn.setColour(juce::TextButton::textColourOffId,
                      juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        btn.onClick = [this, tab] { setActiveTab(tab); };
        addAndMakeVisible(btn);
    };

    setupTabBtn(filesTabBtn_, Tab::Files);
    setupTabBtn(fxTabBtn_, Tab::FX);
    setupTabBtn(sourcesTabBtn_, Tab::Sources);
    setupTabBtn(compDecksTabBtn_, Tab::CompDecks);
    setupTabBtn(recordTabBtn_, Tab::Record);

    // Tab content
    addAndMakeVisible(filesBrowser_);
    addAndMakeVisible(fxBrowser_);
    addAndMakeVisible(sourcesBrowser_);
    addAndMakeVisible(compDecksBrowser_);
    addAndMakeVisible(recordPanel_);

    updateTabButtonColors();
    showActiveTab();
}

void BrowserPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Tab bar background
    auto tabBar = getLocalBounds().removeFromTop(kTabBarHeight).toFloat();
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(tabBar);

    // Bottom border of tab bar
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawLine(tabBar.getX(), tabBar.getBottom(),
               tabBar.getRight(), tabBar.getBottom(), 1.0f);

    // Active tab accent line
    juce::Rectangle<int> activeTabBounds;
    switch (activeTab_)
    {
        case Tab::Files:     activeTabBounds = filesTabBtn_.getBounds(); break;
        case Tab::FX:        activeTabBounds = fxTabBtn_.getBounds(); break;
        case Tab::Sources:   activeTabBounds = sourcesTabBtn_.getBounds(); break;
        case Tab::CompDecks: activeTabBounds = compDecksTabBtn_.getBounds(); break;
        case Tab::Record:    activeTabBounds = recordTabBtn_.getBounds(); break;
    }
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    g.fillRect(activeTabBounds.getX(), activeTabBounds.getBottom() - 3,
               activeTabBounds.getWidth(), 3);
}

void BrowserPanel::resized()
{
    auto area = getLocalBounds();

    // Tab bar
    auto tabBar = area.removeFromTop(kTabBarHeight);
    int tabWidth = tabBar.getWidth() / 5;
    filesTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    fxTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    sourcesTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    compDecksTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    recordTabBtn_.setBounds(tabBar);

    // Content area
    filesBrowser_.setBounds(area);
    fxBrowser_.setBounds(area);
    sourcesBrowser_.setBounds(area);
    compDecksBrowser_.setBounds(area);
    recordPanel_.setBounds(area);

    showActiveTab();
}

void BrowserPanel::setActiveTab(Tab tab)
{
    activeTab_ = tab;
    updateTabButtonColors();
    showActiveTab();
    repaint();
}

void BrowserPanel::setEffectLibrary(EffectLibrary* lib)
{
    fxBrowser_.setEffectLibrary(lib);
}

void BrowserPanel::setComposition(Composition* comp)
{
    compDecksBrowser_.setComposition(comp);
}

void BrowserPanel::refresh()
{
    fxBrowser_.refresh();
    compDecksBrowser_.refresh();
}

void BrowserPanel::updateTabButtonColors()
{
    auto setTabColor = [this](juce::TextButton& btn, Tab tab) {
        if (activeTab_ == tab)
        {
            // Active: solid bright background + white text — unmistakable
            btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a3a5c));
            btn.setColour(juce::TextButton::textColourOffId,
                          juce::Colour(0xffffffff));
        }
        else
        {
            // Inactive: very dark + dim text
            btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a1a2e));
            btn.setColour(juce::TextButton::textColourOffId,
                          juce::Colour(0xff606070));
        }
    };

    setTabColor(filesTabBtn_, Tab::Files);
    setTabColor(fxTabBtn_, Tab::FX);
    setTabColor(sourcesTabBtn_, Tab::Sources);
    setTabColor(compDecksTabBtn_, Tab::CompDecks);
    setTabColor(recordTabBtn_, Tab::Record);
}

void BrowserPanel::showActiveTab()
{
    filesBrowser_.setVisible(activeTab_ == Tab::Files);
    fxBrowser_.setVisible(activeTab_ == Tab::FX);
    sourcesBrowser_.setVisible(activeTab_ == Tab::Sources);
    compDecksBrowser_.setVisible(activeTab_ == Tab::CompDecks);
    recordPanel_.setVisible(activeTab_ == Tab::Record);
}
