#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/FilesBrowser.h"
#include "ui/FXBrowser.h"
#include "ui/SourcesBrowser.h"
#include "ui/CompDecksBrowser.h"
#include "ui/RecordPanel.h"
#include "ui/MilkDropBrowser.h"
#include "ui/LookAndFeel.h"
#include "effects/EffectLibrary.h"
#include "model/Composition.h"
#include <memory>

// BrowserPanel: 6-tab container for Files, FX, Sources, Comp/Decks, Record, MilkDrop.
// Lives in the right-bottom section of the main layout.
class BrowserPanel : public juce::Component
{
public:
    BrowserPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Dependencies
    void setEffectLibrary(EffectLibrary* lib);
    void setComposition(Composition* comp);

    // Refresh all tabs
    void refresh();

    // Access individual tabs
    FilesBrowser& getFilesBrowser() { return filesBrowser_; }
    FXBrowser& getFXBrowser() { return fxBrowser_; }
    SourcesBrowser& getSourcesBrowser() { return sourcesBrowser_; }
    CompDecksBrowser& getCompDecksBrowser() { return compDecksBrowser_; }
    RecordPanel& getRecordPanel() { return recordPanel_; }
    MilkDropBrowser& getMilkDropBrowser() { return milkDropBrowser_; }

    enum class Tab : int { Files = 0, FX = 1, Sources = 2, CompDecks = 3, Record = 4, MilkDrop = 5 };
    void setActiveTab(Tab tab);
    Tab getActiveTab() const { return activeTab_; }

private:
    Tab activeTab_ = Tab::Files;

    // Tab buttons
    juce::TextButton filesTabBtn_{"Files"};
    juce::TextButton fxTabBtn_{"FX"};
    juce::TextButton sourcesTabBtn_{"Sources"};
    juce::TextButton compDecksTabBtn_{"Comp/Decks"};
    juce::TextButton recordTabBtn_{"Record"};
    juce::TextButton milkDropTabBtn_{"MilkDrop"};

    // Tab content
    FilesBrowser filesBrowser_;
    FXBrowser fxBrowser_;
    SourcesBrowser sourcesBrowser_;
    CompDecksBrowser compDecksBrowser_;
    RecordPanel recordPanel_;
    MilkDropBrowser milkDropBrowser_;

    static constexpr int kTabBarHeight = 26;

    void updateTabButtonColors();
    void showActiveTab();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserPanel)
};
