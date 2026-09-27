#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include <vector>

// CompDecksBrowser: the library -- saved compositions and decks.
// Two sections: Compositions (click = open, replacing everything after a confirm) and Decks (click = append as a
// new deck tab, undoable); right-click a row = Open / Show in Finder / Delete... (confirmed, moved to the Trash).
// Save Composition lives here; deck save/load live in the deck tab row (DeckView) -- plan6 §8.
class CompDecksBrowser : public juce::Component
{
public:
    CompDecksBrowser();
    ~CompDecksBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Refresh the file lists
    void refresh();

    // plan6 §8: right-click a row -> "Open" (compositions) / "Open as New Deck" (decks), "Show in Finder",
    // "Delete..." -- the menu at the click. confirmDelete asks, then moves the file to the Trash (never a
    // silent delete). Both re-resolve `row` in their callbacks (the list can refresh while they are open).
    void showRowMenu(bool decksSection, int row, juce::Point<int> screenPos);
    void confirmDelete(bool decksSection, int row);

    // Callbacks -- wired in MainComponent's constructor (the browser wiring next to setComposition):
    //   onCompositionLoad -> confirmReplaceShow -> loadComposition (the fence + undo-clear rule this comment used
    //                        to demand is satisfied there: loadComposition -> swapCompositionModel)
    //   onDeckLoad        -> appendDeckFromFile (one undoable InsertDeckCmd, fenced)
    //   onCompositionSave -> saveComposition (reads the model only)
    std::function<void(const juce::File&)> onCompositionLoad;
    std::function<void(const juce::File&)> onDeckLoad;
    std::function<void()> onCompositionSave;

    // L3 (2026-09): public so MainComponent's Open/Save/Save As can default
    // the FileChooser to these directories, same as PresetManager's own
    // getPresetsDirectory() convention.
    static juce::File getCompositionsDir();
    static juce::File getDecksDir();

private:
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

    // Scrollable content
    juce::Viewport viewport_;
    class CompDeckListContent;
    std::unique_ptr<CompDeckListContent> listContent_;

    void scanForFiles();

    static constexpr int kButtonBarHeight = 28;
    static constexpr int kSectionHeaderHeight = 22;
    static constexpr int kEntryRowHeight = 24;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompDecksBrowser)
};
