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
    // FUTURE-FENCE REQUIREMENT (family-fence fix, 2026-07-28 — see
    // .harmony/notebook.md LAW entry): onCompositionLoad / onDeckLoad are
    // UNWIRED no-ops at HEAD (verified — no assignment exists anywhere in
    // MainComponent.cpp or elsewhere). Whoever wires them MUST apply the
    // loaded model under undoService_.withDeckDetached(...) and call
    // undoManager_.clear() in the same breath (the kCompNew precedent,
    // MainComponent.cpp's kCompNew handler) — otherwise the proven UAF class
    // returns (the model swap reallocates composition_.decks under an
    // unlocked GL read, exactly like kCompNew's initDefault()) — the
    // ClipCommands.h GL-FENCE EXEMPTION this used to also threaten was
    // closed in round 3 (SetClipCmd/SwapClipsCmd are now unconditionally
    // fenced), so this composition-swap hazard is the sole remaining reason,
    // not a joint one. onCompositionSave only reads the model (serializes
    // it) and does not need this treatment.
    std::function<void(const juce::File&)> onCompositionLoad;
    std::function<void(const juce::File&)> onDeckLoad;
    std::function<void()> onCompositionSave;

    // L3 (2026-09): public so MainComponent's Open/Save/Save As can default
    // the FileChooser to these directories, same as PresetManager's own
    // getPresetsDirectory() convention.
    static juce::File getCompositionsDir();
    static juce::File getDecksDir();

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

    static constexpr int kButtonBarHeight = 28;
    static constexpr int kSectionHeaderHeight = 22;
    static constexpr int kEntryRowHeight = 24;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompDecksBrowser)
};
