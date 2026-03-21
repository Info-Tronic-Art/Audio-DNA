#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Clip.h"
#include "ui/LookAndFeel.h"

// ClipCell: a single cell in the deck grid (layer × column intersection).
// Two interaction zones:
//   - Thumbnail area: click = trigger/retrigger clip
//   - Name bar: click = select for inspection (no trigger), right-click = context menu
// Supports drag-and-drop (receive images from Finder or browser).
class ClipCell : public juce::Component,
                 public juce::FileDragAndDropTarget,
                 public juce::DragAndDropTarget
{
public:
    ClipCell();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // DragAndDropTarget (for internal FX drags)
    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;

    // Set the clip data this cell displays (nullptr for empty)
    void setClip(Clip* clip);
    Clip* getClip() const { return clip_; }

    // Set active state (cyan border highlight — this clip is playing)
    void setActive(bool active);
    bool isActive() const { return active_; }

    // Set selected state (white border highlight — user has selected for inspection)
    void setSelected(bool selected);
    bool isSelected() const { return selected_; }

    // Set position in the grid
    void setGridPosition(int layerIndex, int column);
    int getLayerIndex() const { return layerIndex_; }
    int getColumn() const { return column_; }

    // Callbacks
    std::function<void(int layerIndex, int column)> onTrigger;      // Thumbnail click
    std::function<void(int layerIndex, int column, bool addToSelection)> onSelect; // Name bar click
    std::function<void(int layerIndex, int column, const juce::File&)> onFileDrop; // Single file dropped
    std::function<void(int layerIndex, int column, const std::vector<juce::File>&)> onMultiFileDrop; // Multi-image sequence dropped
    std::function<void(int layerIndex, int column, const std::vector<juce::File>&)> onMultiVideoDrop; // Multi-video dropped → sequential cells
    std::function<void(int layerIndex, int column, const juce::String& effectName)> onEffectDrop; // FX dropped from browser
    std::function<void(int layerIndex, int column, const juce::String& sourceId)> onSourceDrop; // Source dropped from browser
    std::function<void(int srcLayer, int srcCol, int dstLayer, int dstCol)> onClipMove; // Clip dragged from one cell to another

    // Load/update thumbnail from clip's media file
    void updateThumbnail();

private:
    juce::Rectangle<int> getThumbnailBounds() const;
    juce::Rectangle<int> getNameBarBounds() const;
    bool isInThumbnailArea(const juce::Point<int>& pos) const;

    Clip* clip_ = nullptr;
    int layerIndex_ = 0;
    int column_ = 0;
    bool active_ = false;
    bool selected_ = false;
    bool dragHover_ = false;
    bool fxDragHover_ = false;
    bool sourceDragHover_ = false;

    juce::Image thumbnail_;

    static constexpr int kNameBarHeight = 20;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipCell)
};
