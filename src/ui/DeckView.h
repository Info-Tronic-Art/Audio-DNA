#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include "model/Deck.h"
#include "ui/ClipCell.h"
#include "ui/LayerStrip.h"
#include "ui/LookAndFeel.h"
#include <vector>
#include <memory>

// DeckView: Resolume-style layer × column deck grid.
// Displays column trigger buttons at top, layer strips on the left,
// clip cells in the grid, and deck tabs at the bottom.
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

    // Callbacks — forwarded from child components
    std::function<void(int layerIndex, int column)> onClipTriggered;
    std::function<void(int layerIndex, int column, bool addToSelection)> onClipSelected;
    std::function<void(int layerIndex)> onLayerSelected;
    std::function<void(int column)> onColumnTriggered;
    std::function<void(int layerIndex, int column, const juce::File&)> onFileDropped;
    std::function<void(int layerIndex, int column, const std::vector<juce::File>&)> onMultiFileDropped;
    std::function<void(int layerIndex, int column, const std::vector<juce::File>&)> onMultiVideoDropped;
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

    // Grid components
    std::vector<std::unique_ptr<LayerStrip>> layerStrips_;
    std::vector<std::vector<std::unique_ptr<ClipCell>>> clipCells_; // [layer][column]
    std::vector<std::unique_ptr<juce::TextButton>> columnTriggers_;
    std::vector<std::unique_ptr<juce::TextButton>> deckTabs_;

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
    static constexpr int kCellWidth = 90;
    static constexpr int kCellHeight = 96; // 3-row layer strip height
    static constexpr int kDeckTabHeight = 24;
    static constexpr int kCellGap = 0; // flush, 1px borders drawn by cells

    void layoutGrid();
    void setupColumnTriggers();
    void setupDeckTabs();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckView)
};
