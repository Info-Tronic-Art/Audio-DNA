#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include "model/Deck.h"
#include "ui/ClipCell.h"
#include "ui/ClipThumbnails.h"
#include "ui/LayerStrip.h"
#include "ui/LookAndFeel.h"
#include "ui/DeckTabRow.h"
#include "ui/RoutinePad.h"
#include "ui/RoutineDeckView.h"
#include <array>
#include <vector>
#include <memory>

// DeckView: Resolume-style layer × column deck grid.
// Displays the ROUTINES row (eight routine pads, s-rta-0927) and the column trigger buttons at top,
// layer strips on the left, clip cells in the grid, and deck tabs at the bottom.
// Supports horizontal/vertical scrolling when content exceeds viewport.
class DeckView : public juce::Component
{
public:
    DeckView();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the composition to display (reads active deck)
    void setComposition(Composition* comp);
    Composition* getComposition() const { return composition_; }

    // Rebuild the grid from the current deck state
    void rebuildGrid();

    // Refresh display state (active clips, button states, etc.)
    void refresh();

    // s-rta-0928: the grid's image thumbnails, decoded off the message thread; every ClipCell / LayerStrip pulls from
    // it. Public for tests (setBackendsForTests before setComposition).
    ClipThumbnails& getThumbnails() { return thumbnails_; }

    // Callbacks — forwarded from child components
    std::function<void(int layerIndex, int column)> onClipTriggered;
    std::function<void(int layerIndex, int column, bool addToSelection)> onClipSelected;
    std::function<void(int layerIndex)> onLayerSelected;
    std::function<void(int column)> onColumnTriggered;
    std::function<void(int layerIndex, int column, const juce::File&)> onFileDropped;
    std::function<void(int layerIndex, int column, const std::vector<juce::File>&)> onMultiFileDropped;
    std::function<void(int layerIndex, int column, const std::vector<juce::File>&)> onMultiVideoDropped;
    std::function<void(int layerIndex, int column, const std::vector<juce::File>& images, const std::vector<juce::File>& videos)> onMixedFilesDropped; // Mixed image+video Finder drop (2026-07-30)
    std::function<void(int layerIndex, int column, const juce::String& effectName)> onEffectDropped;
    std::function<void(int layerIndex, int column, const juce::String& sourceId)> onSourceDropped;
    std::function<void(int srcLayer, int srcCol, int dstLayer, int dstCol)> onClipMoved;
    std::function<void(int layerIndex, int column, const std::string& presetPath)> onMilkDropDropped;
    std::function<void(int layerIndex, int column, const std::vector<std::string>& presetPaths)> onMilkDropPlaylistDropped;
    std::function<void(int deckIndex)> onDeckSwitched;
    std::function<void(int layerIndex)> onLayerFoldToggle;         // P24.12
    std::function<void(int fromIndex, int toIndex)> onLayerReorder; // P24.13
    std::function<void(int layerIndex)> onLayerClearClip;          // Undo v1 #13 (X button)
    std::function<void(int layerIndex, bool bypassed)> onLayerBypass; // Undo v1 #14
    std::function<void(int layerIndex, bool solo)> onLayerSolo;    // Undo v1 #15
    std::function<void(int layerIndex, const juce::String& effectDesc)> onLayerEffectDropped; // FX-drop-target on the channel strip (2026-07-30)
    // plan6 §6.2: a deck tab row action chosen from a tab's right-click menu (deckIndex = that tab) or from the "+"
    // menu (deckIndex -1: NewDeck / LoadDeck). MainComponent runs the handler (message thread).
    std::function<void(int deckIndex, DeckTabRow::Action)> onDeckAction;
    // plan6 §6.2: the "Undo Remove" button in the tab row was clicked (MainComponent undoes iff the top of the
    // undo stack is still that removal).
    std::function<void()> onUndoHint;

    // s-rta-0927 routine display (plan-routine-display-A.md 2.1-2.3): the ROUTINES row above the column
    // numbers. A pad press fires (restart while playing); a band's x or the pad menu's "Remove from layers"
    // removes a routine (whole, every layer); the other menu rows are settings, Rename and Delete. MainComponent
    // runs each through its perfRoutine* funnel (message thread).
    std::function<void(int slot)> onRoutineFired;
    std::function<void(int slot)> onRoutineRemoved;
    std::function<void(int slot, const RoutineSettingsChange&)> onRoutineSet;
    std::function<void(int slot)> onRoutineRename;
    std::function<void(int slot)> onRoutineDeleted;

    // Pushed by MainComponent every 30 Hz tick (never via refresh()): the pads' specs, the corner note, and
    // each shown layer's bands (at most two drawn). Repaints only what changed.
    void setRoutineView(const RoutineDeckView& view);
    void showRoutinePadMenu(int slot);

    // plan6 §6.2: the deck tab row's menus (right-click a tab / click the "+"), and the 10-s "Undo Remove" button
    // flush right in the row. Every structural change (rebuildGrid) and every later undoable command hides it.
    void showDeckTabMenu(int deckIndex);
    void showPlusMenu();
    void showUndoHint(const juce::String& text);
    void hideUndoHint();

    // Get active column (-1 if none)
    int getActiveColumn() const { return activeColumn_; }
    void setActiveColumn(int col);

    // Multi-selection of clip cells
    struct CellPos { int layer; int column; };
    const std::vector<CellPos>& getSelectedCells() const { return selectedCells_; }
    void clearSelection();
    void selectCell(int layerIndex, int column, bool addToSelection);

    // Layer selection
    void selectLayer(int layerIndex);
    int getSelectedLayerIndex() const { return selectedLayerIndex_; }

    // Get the natural height that fits all layers + triggers + tabs exactly
    int getNaturalHeight() const;

private:
    Composition* composition_ = nullptr;
    ClipThumbnails thumbnails_;   // declared before the strips / cells: destroyed after them

    // Grid components
    std::vector<std::unique_ptr<LayerStrip>> layerStrips_;
    std::vector<std::vector<std::unique_ptr<ClipCell>>> clipCells_; // [layer][column]
    std::vector<std::unique_ptr<juce::TextButton>> columnTriggers_;
    // s-rta-0927: the ROUTINES row -- created ONCE in the constructor as direct children (rebuildGrid never
    // destroys them), laid out over the column triggers' x so pad N sits above column N.
    std::array<std::unique_ptr<RoutinePad>, RoutineEngine::kBankSize> routinePads_;
    juce::String routineCornerNote_;
    RoutineDeckView lastRoutineView_;   // the bands re-fan to freshly built strips after a rebuild
    // A deck tab. JUCE's Button fires onClick for ANY mouse button (Button::mouseDown/mouseUp check no button), so
    // a right-click (or Ctrl+click -- isPopupMenu()) is intercepted here: it opens the tab's menu and never reaches
    // the base class, so it never switches decks (plan6 §6.2).
    struct DeckTabButton : juce::TextButton
    {
        using juce::TextButton::TextButton;
        std::function<void()> onContextMenu;
        void mouseDown(const juce::MouseEvent& e) override
        {
            if (e.mods.isPopupMenu()) { if (onContextMenu) onContextMenu(); return; }
            juce::TextButton::mouseDown(e);
        }
        void mouseDrag(const juce::MouseEvent& e) override { if (! e.mods.isPopupMenu()) juce::TextButton::mouseDrag(e); }
        void mouseUp(const juce::MouseEvent& e) override   { if (! e.mods.isPopupMenu()) juce::TextButton::mouseUp(e); }
    };
    std::vector<std::unique_ptr<DeckTabButton>> deckTabs_;
    std::unique_ptr<juce::TextButton> plusTab_;          // "+" -- New Deck / Load Deck... (rebuilt with the tabs)
    std::unique_ptr<juce::TextButton> undoHintBtn_;      // "Undo Remove \"<name>\"" -- created once, hidden (Pitfall 34)
    int undoHintGeneration_ = 0;                         // bumps on every show/hide: a stale 10-s timer does nothing
    static constexpr int kUndoHintMs = 10000;

    // Scroll viewport for the grid
    juce::Viewport gridViewport_;
    std::unique_ptr<juce::Component> gridContent_;

    int activeColumn_ = -1;
    int selectedLayerIndex_ = -1;
    std::vector<CellPos> selectedCells_;

    void updateSelectionVisuals();

    // Layout constants — Resolume-style dense grid
    static constexpr int kLayerStripWidth = 250;
    static constexpr int kColumnTriggerHeight = 22;
    static constexpr int kRoutineRowHeight = 22;   // s-rta-0927: the ROUTINES row, above the triggers
    static constexpr int kCellWidth = 90;
    static constexpr int kCellHeight = 96; // 3-row layer strip height
    static constexpr int kDeckTabHeight = 24;
    static constexpr int kCellGap = 0; // flush, 1px borders drawn by cells

    void layoutGrid();
    void setupColumnTriggers();
    void setupDeckTabs();
    void fanRoutineBands();
    static juce::String tabTooltipFor(const Deck& deck);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckView)
};
