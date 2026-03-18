#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include "ui/LookAndFeel.h"
#include <vector>

// CompDecksBrowser: shows saved compositions and decks.
// Two sections: Compositions (click to load full state) and Decks (click to switch).
// Supports save/rename/delete operations.
class CompDecksBrowser : public juce::Component
{
public:
    CompDecksBrowser();
    ~CompDecksBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the current composition (for saving)
    void setComposition(Composition* comp) { composition_ = comp; }

    // Refresh the file lists
    void refresh();

    // Callbacks
    std::function<void(const juce::File&)> onCompositionLoad;
    std::function<void(const juce::File&)> onDeckLoad;
    std::function<void()> onCompositionSave;

private:
    Composition* composition_ = nullptr;

    // Compositions section
    struct SavedEntry
    {
        juce::File file;
        juce::String name;
        juce::String dateStr;
    };

    std::vector<SavedEntry> compositions_;
    std::vector<SavedEntry> decks_;

    bool compositionsExpanded_ = true;
    bool decksExpanded_ = true;

    // Buttons
    juce::TextButton saveCompBtn_{"Save Composition"};
    juce::TextButton saveDeckBtn_{"Save Deck"};

    // Scrollable content
    juce::Viewport viewport_;
    class CompDeckListContent;
    std::unique_ptr<CompDeckListContent> listContent_;

    void scanForFiles();

    static juce::File getCompositionsDir();
    static juce::File getDecksDir();

    static constexpr int kButtonBarHeight = 28;
    static constexpr int kSectionHeaderHeight = 22;
    static constexpr int kEntryRowHeight = 24;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompDecksBrowser)
};
