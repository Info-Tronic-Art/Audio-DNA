#include "ui/InspectorPanel.h"

InspectorPanel::InspectorPanel()
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

    setupTabBtn(clipTabBtn_, Tab::Clip);
    setupTabBtn(layerTabBtn_, Tab::Layer);
    setupTabBtn(compTabBtn_, Tab::Composition);
    setupTabBtn(signalTabBtn_, Tab::Signal);

    // Viewports — each wraps its inspector content for scrolling
    clipViewport_.setViewedComponent(&clipInspector_, false);
    clipViewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(clipViewport_);

    layerViewport_.setViewedComponent(&layerInspector_, false);
    layerViewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(layerViewport_);

    compViewport_.setViewedComponent(&compInspector_, false);
    compViewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(compViewport_);

    signalViewport_.setViewedComponent(&signalInspector_, false);
    signalViewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(signalViewport_);

    updateTabButtonColors();
    showActiveTab();
}

void InspectorPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Tab bar background
    auto tabBar = getLocalBounds().removeFromTop(kTabBarHeight).toFloat();
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(tabBar);

    // Bottom border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawLine(tabBar.getX(), tabBar.getBottom(),
               tabBar.getRight(), tabBar.getBottom(), 1.0f);

    // Active tab accent line (bright cyan bar under the active tab)
    juce::Rectangle<int> activeTabBounds;
    switch (activeTab_)
    {
        case Tab::Clip:        activeTabBounds = clipTabBtn_.getBounds(); break;
        case Tab::Layer:       activeTabBounds = layerTabBtn_.getBounds(); break;
        case Tab::Composition: activeTabBounds = compTabBtn_.getBounds(); break;
        case Tab::Signal:      activeTabBounds = signalTabBtn_.getBounds(); break;
    }
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    g.fillRect(activeTabBounds.getX(), activeTabBounds.getBottom() - 3,
               activeTabBounds.getWidth(), 3);
}

void InspectorPanel::resized()
{
    auto area = getLocalBounds();

    // Tab bar
    auto tabBar = area.removeFromTop(kTabBarHeight);
    int tabWidth = tabBar.getWidth() / 4;
    clipTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    layerTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    compTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    signalTabBtn_.setBounds(tabBar);

    // Content area — all viewports share the same bounds, only active one is visible
    clipViewport_.setBounds(area);
    layerViewport_.setBounds(area);
    compViewport_.setBounds(area);
    signalViewport_.setBounds(area);

    // Size the inspector content to its preferred height (for scrolling)
    int contentWidth = area.getWidth() - 12; // account for scrollbar
    clipInspector_.setSize(contentWidth, clipInspector_.getPreferredHeight());
    layerInspector_.setSize(contentWidth, layerInspector_.getPreferredHeight());
    compInspector_.setSize(contentWidth, compInspector_.getPreferredHeight());
    signalInspector_.setSize(contentWidth, signalInspector_.getPreferredHeight());

    showActiveTab();
}

void InspectorPanel::setComposition(Composition* comp)
{
    composition_ = comp;
    compInspector_.setComposition(comp);
}

void InspectorPanel::setEffectLibrary(EffectLibrary* lib)
{
    clipInspector_.setEffectLibrary(lib);
    layerInspector_.setEffectLibrary(lib);
    compInspector_.setEffectLibrary(lib);
}

void InspectorPanel::setSignalRegistry(SignalRegistry* reg)
{
    clipInspector_.setSignalRegistry(reg);
    layerInspector_.setSignalRegistry(reg);
    compInspector_.setSignalRegistry(reg);
}

void InspectorPanel::inspectClip(Clip* clip)
{
    clipInspector_.setClip(clip);
    setActiveTab(Tab::Clip);
}

void InspectorPanel::inspectLayer(Layer* layer)
{
    layerInspector_.setLayer(layer);
    setActiveTab(Tab::Layer);
}

void InspectorPanel::inspectSignal(Signal* signal)
{
    signalInspector_.setSignal(signal);
    setActiveTab(Tab::Signal);
}

void InspectorPanel::showCompositionTab()
{
    setActiveTab(Tab::Composition);
}

void InspectorPanel::refresh()
{
    switch (activeTab_)
    {
        case Tab::Clip:        clipInspector_.refresh(); break;
        case Tab::Layer:       layerInspector_.refresh(); break;
        case Tab::Composition: compInspector_.refresh(); break;
        case Tab::Signal:      signalInspector_.refresh(); break;
    }

    // Resize content for scrolling
    int contentWidth = clipViewport_.getWidth() - 12;
    if (contentWidth > 0)
    {
        clipInspector_.setSize(contentWidth, clipInspector_.getPreferredHeight());
        layerInspector_.setSize(contentWidth, layerInspector_.getPreferredHeight());
        compInspector_.setSize(contentWidth, compInspector_.getPreferredHeight());
        signalInspector_.setSize(contentWidth, signalInspector_.getPreferredHeight());
    }
}

void InspectorPanel::setActiveTab(Tab tab)
{
    activeTab_ = tab;
    updateTabButtonColors();
    showActiveTab();
    refresh();
}

void InspectorPanel::updateTabButtonColors()
{
    auto setActive = [](juce::TextButton& btn, bool active) {
        btn.setColour(juce::TextButton::buttonColourId,
            active ? juce::Colour(0xff3a3a5c)
                   : juce::Colour(0xff1a1a2e));
        btn.setColour(juce::TextButton::textColourOffId,
            active ? juce::Colour(0xffffffff)
                   : juce::Colour(0xff606070));
    };

    setActive(clipTabBtn_, activeTab_ == Tab::Clip);
    setActive(layerTabBtn_, activeTab_ == Tab::Layer);
    setActive(compTabBtn_, activeTab_ == Tab::Composition);
    setActive(signalTabBtn_, activeTab_ == Tab::Signal);
}

void InspectorPanel::showActiveTab()
{
    clipViewport_.setVisible(activeTab_ == Tab::Clip);
    layerViewport_.setVisible(activeTab_ == Tab::Layer);
    compViewport_.setVisible(activeTab_ == Tab::Composition);
    signalViewport_.setVisible(activeTab_ == Tab::Signal);
}
