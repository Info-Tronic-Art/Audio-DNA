#include "ui/TimingWindow.h"

TimingWindow::TimingWindow()
{
    auto setupTabBtn = [this](juce::TextButton& btn, Tab tab) {
        btn.setColour(juce::TextButton::buttonColourId,
                      juce::Colour(AudioDNALookAndFeel::kSurface));
        btn.setColour(juce::TextButton::textColourOffId,
                      juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        btn.onClick = [this, tab] { setActiveTab(tab); };
        addAndMakeVisible(btn);
    };

    setupTabBtn(bpmTabBtn_, Tab::BPM);
    setupTabBtn(routingTabBtn_, Tab::Routing);
    setupTabBtn(oscTabBtn_, Tab::Oscillators);

    updateTabButtonColors();
}

void TimingWindow::paint(juce::Graphics& g)
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
        case Tab::BPM:         activeTabBounds = bpmTabBtn_.getBounds(); break;
        case Tab::Routing:     activeTabBounds = routingTabBtn_.getBounds(); break;
        case Tab::Oscillators: activeTabBounds = oscTabBtn_.getBounds(); break;
    }
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    g.fillRect(activeTabBounds.getX(), activeTabBounds.getBottom() - 3,
               activeTabBounds.getWidth(), 3);

    // Panel border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawRect(getLocalBounds().toFloat(), 1.0f);

    // Placeholder content text
    auto contentArea = getLocalBounds().reduced(8);
    contentArea.removeFromTop(kTabBarHeight);

    juce::String tabName;
    switch (activeTab_)
    {
        case Tab::BPM:         tabName = "BPM"; break;
        case Tab::Routing:     tabName = "Routing"; break;
        case Tab::Oscillators: tabName = "Oscillators"; break;
    }

    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.4f));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText(tabName, contentArea, juce::Justification::centred, false);
}

void TimingWindow::resized()
{
    auto tabBar = getLocalBounds().removeFromTop(kTabBarHeight);
    int tabWidth = tabBar.getWidth() / 3;
    bpmTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    routingTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    oscTabBtn_.setBounds(tabBar);
}

void TimingWindow::setActiveTab(Tab tab)
{
    activeTab_ = tab;
    updateTabButtonColors();
    repaint();
}

void TimingWindow::updateTabButtonColors()
{
    auto setActive = [](juce::TextButton& btn, bool active) {
        btn.setColour(juce::TextButton::buttonColourId,
            active ? juce::Colour(0xff3a3a5c)
                   : juce::Colour(0xff1a1a2e));
        btn.setColour(juce::TextButton::textColourOffId,
            active ? juce::Colour(0xffffffff)
                   : juce::Colour(0xff606070));
    };

    setActive(bpmTabBtn_, activeTab_ == Tab::BPM);
    setActive(routingTabBtn_, activeTab_ == Tab::Routing);
    setActive(oscTabBtn_, activeTab_ == Tab::Oscillators);
}
