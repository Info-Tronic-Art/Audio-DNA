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
    ~DeckView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the composition to display (reads active deck)
    void setComposition(Composition* comp);
    Composition* getComposition() const { return composition_; }

    // Rebuild the grid from the current deck state
    void rebuildGrid();

    // s-rta-0929 g4cpu (probe-idle-paint a1's pass classes): the ROUTINES pads' union and the strip column of the grid
    // viewport, in DeckView coordinates.
    juce::Rectangle<int> getRoutinePadRowBounds() const;
    juce::Rectangle<int> getStripColumnBounds() const
    {
        const auto v = gridViewport_.getBounds();
        return v.withWidth(std::min(v.getWidth(), kLayerStripWidth - v.getX()));
    }

    // Refresh display state (active clips, button states, etc.)
    void refresh();

    // s-rta-0928: the grid's image thumbnails, decoded off the message thread; every ClipCell / LayerStrip pulls from
    // it. Public for tests (setBackendsForTests before setComposition).
    ClipThumbnails& getThumbnails() { return thumbnails_; }

    // s-rta-1002b ui U2.3 (BF3): fan-out only. Every clip cell reads a Video clip's codec / size / rate through this
    // source (MainComponent::videoInfoFor) when its tooltip is asked for, and its menu's "Show in Finder" bubbles up as
    // onRevealInFinder(layerIndex, column). Message thread.
    void setVideoInfoSource(VideoInfoSource source) { videoInfoSource_ = std::move(source); }
    std::function<void(int layerIndex, int column)> onRevealInFinder;
    // TEST-ONLY hooks (ruling-ui.md AM10; /api/debug/reveal_clip and /api/debug/clip_media): the cell showing (layer,
    // column) -- nullptr if none -- and its menu's completion path, menuChosen(1), as if "Show in Finder" was chosen.
    ClipCell* cellForTests(int layerIndex, int column) const;
    void revealCellForTests(int layerIndex, int column);

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

    // s-rta-1002b ui U3.1 (BF8): how many times the tab row was built (setupDeckTabs) -- the witness that a click on the
    // deck already showing rebuilds nothing.
    int tabRowBuilds() const { return tabRowBuilds_; }

    // s-rta-1002b ui U3.2 (BF8 "rename each deck with double click"; ruling-ui.md AM2-AM5): a double-click on the tab of
    // the deck showing opens a text box over that tab. Enter, Tab and a click anywhere else keep the typed name, Esc
    // discards it; an empty or unchanged name keeps the old one (no callback). Bound to the deck's ID: it follows that
    // deck through every rebuild and closes (discarding) when the deck is gone.
    std::function<void(int deckIndex, const juce::String& name)> onDeckRenamed;   // MainComponent::applyDeckRename
    // Every close while keyboard focus is inside DeckView or nowhere -- called BEFORE the box hides, so JUCE never parks
    // focus on a column trigger or a strip's X button (ruling E-R4 / E-R5). MainComponent takes the focus back.
    std::function<void()> onRenameClosed;
    void beginRename(int deckIndex);
    void cancelDeckRename();                       // close and discard, no onDeckRenamed (a composition swap)
    bool isRenaming() const { return renaming_; }
    int renamingDeckIndex() const;                 // the deck the box is on, -1 when closed
    // Tests: the nested tab-row listener (driven in JUCE's order) and the box itself.
    juce::MouseListener& tabRowMouseForTests() { return tabRowMouse_; }
    juce::TextEditor* renameEditorForTests() { return &renameEditor_; }
    // Tests: the keyboard focus the close reads (headless, no component can hold focus: it is always nowhere).
    // Empty = juce::Component::getCurrentlyFocusedComponent().
    void setFocusedComponentForTests(std::function<juce::Component*()> f) { focusedForTests_ = std::move(f); }
    // s-rta-1002b ui U3.4 (ruling AM6; the TEST-ONLY REST routes /api/debug/deck_tabs, deck_rename, tab_click,
    // tab_dblclick): the same functions a click / key reaches. clickTabForTests runs the tab button's own onClick (what
    // Button::mouseUp reaches) through a copy that outlives a rebuild; doubleClickTabForTests replays a double-click in
    // JUCE's order (ruling E-R3); renameOpForTests runs one box op ("begin" "type" "enter" "tab" "escape" "focus_lost"
    // "outside_click"; false for any other); tabRowStateForTests answers the row and the box (message thread only).
    void clickTabForTests(int deckIndex);
    void doubleClickTabForTests(int deckIndex);
    bool renameOpForTests(const juce::String& op, int deckIndex, const juce::String& text);
    juce::var tabRowStateForTests() const;

private:
    Composition* composition_ = nullptr;
    ClipThumbnails thumbnails_;   // declared before the strips / cells: destroyed after them
    VideoInfoSource videoInfoSource_;   // s-rta-1002b ui U2.3: the cells hold its address -- declared before them too

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
    int tabRowBuilds_ = 0;                               // ++ in setupDeckTabs (U3.1 witness)
    // s-rta-1002b ui U3.1 (BF8): a tab's left click. A click on the deck already showing does NOTHING: it must not
    // rebuild the row, or the clicked tab dies before JUCE can deliver its double-click (plan-ui E4 / E5, ruling E-R3).
    void tabClicked(int deckIndex);

    // s-rta-1002b ui U3.2 (ruling AM3): the deck-name box. It handles Return / Tab / Esc in keyPressed, synchronously
    // (a TextEditor POSTS its own Return / Esc / focus-loss callbacks, ruling E-R9), lets the output keys
    // (Cmd+Shift+Esc, Cmd+`, Cmd+F) reach MainComponent (a TextEditor's Escape test is modifier-blind, plan E7), and
    // swallows every key-state change, so no key-up reaches MainComponent's momentary-release sweep while typing.
    struct DeckNameEditor : juce::TextEditor
    {
        std::function<void(bool keep)> onClose;
        bool keyPressed(const juce::KeyPress& key) override;
        bool keyStateChanged(bool isKeyDown) override;
    };
    DeckNameEditor renameEditor_;                        // created once, hidden (Pitfall 34), never rebuilt
    bool renaming_ = false;
    uint32_t renamingDeckId_ = 0;
    std::function<juce::Component*()> focusedForTests_;   // empty = the real focus (setFocusedComponentForTests)
    juce::Rectangle<int> tabRow_;                        // the tab row in DeckView coordinates (resized)
    void finishRename(bool keep);
    void placeRenameEditor();
    int deckIndexOfId(uint32_t deckId) const;

    // s-rta-1002b ui U3.2 (ruling AM2): ONE nested listener sees every mouseDown / double-click in DeckView. It never
    // reads a position: once the tab died in its own onClick, JUCE hands the parent the DEAD tab's local position
    // (ruling E-R3 S1), so the tab is identified by the mouseDown's originalComponent, which is still alive then.
    struct TabRowMouse : juce::MouseListener
    {
        explicit TabRowMouse(DeckView& d) : dv(d) {}
        void mouseDown(const juce::MouseEvent& e) override        { dv.tabRowMouseDown(e); }
        void mouseDoubleClick(const juce::MouseEvent& e) override { dv.tabRowDoubleClick(e); }
        DeckView& dv;
    };
    TabRowMouse tabRowMouse_ { *this };
    struct TabArm { uint32_t deckId = 0; bool wasShowing = false; bool valid = false; };
    TabArm firstClick_, armed_;
    // Boris Q1 default (a): a double-click renames only when its FIRST click landed on the tab already showing (a quick
    // double-click while flicking through decks never opens a box). Answer (b) => false.
    static constexpr bool kRenameOnlyTheShowingTab = true;
    int tabIndexOf(const juce::Component* c) const;      // index in deckTabs_, else -1
    void tabRowMouseDown(const juce::MouseEvent& e);
    void tabRowDoubleClick(const juce::MouseEvent& e);
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
    static juce::String tabTooltipFor(const Deck& deck, bool showing);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckView)
};
