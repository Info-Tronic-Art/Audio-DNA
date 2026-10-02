#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Clip.h"
#include "ui/ClipMediaText.h"
#include "ui/LookAndFeel.h"

class ClipThumbnails;

// ClipCell: a single cell in the deck grid (layer × column intersection).
// Two interaction zones:
//   - Thumbnail area: click = trigger/retrigger clip
//   - Name bar: click = select for inspection (no trigger)
// Right-click (a real right button) on a clip with a file (video, picture, image sequence) = a menu headed by the
// clip's name with "Show in Finder"; any other right-click does nothing, and Ctrl+left-click is a left click (s-rta-1002b
// ui U2.2, ruling-ui.md AM10). Hover = the clip's file name, codec / size / rate and the menu hint (clipmedia::).
// Supports drag-and-drop (receive images from Finder or browser).
class ClipCell : public juce::Component,
                 public juce::FileDragAndDropTarget,
                 public juce::DragAndDropTarget,
                 public juce::SettableTooltipClient
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

    // s-rta-0928: where an Image clip's thumbnail comes from (DeckView's store; set before setClip).
    void setThumbnails(ClipThumbnails* store) { thumbs_ = store; }

    // s-rta-1002b ui U2.2 (BF3): where a Video clip's codec / size / rate come from (DeckView's source, which outlives
    // the cells -- the setThumbnails shape). The tooltip is built when JUCE asks for it (hover only), never stored.
    void setVideoInfoSource(const VideoInfoSource* source) { videoInfo_ = source; }
    juce::String getTooltip() override;

    // s-rta-1002b ui U2.2 (ruling-ui.md AM10): the right-click menu. The launcher shows it (default: a PopupMenu, the
    // DeckView::showDeckTabMenu idiom) and calls done(result) when it closes; menuChosen is the ONE completion path
    // (the menu, a test and /api/debug/reveal_clip all end there). A launcher is injectable so no test opens a menu.
    using MenuLauncher = std::function<void(const juce::String& header, const juce::StringArray& items,
                                            std::function<void(int)> done)>;
    void setMenuLauncherForTests(MenuLauncher launcher) { menuLauncher_ = std::move(launcher); }
    void menuChosen(int result);

    // Set active state (cyan border highlight — this clip is playing)
    void setActive(bool active);
    bool isActive() const { return active_; }
    bool hasThumbnail() const { return thumbnail_.isValid(); }   // tests

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
    // Mixed Finder drop (2026-07-30): images AND videos dropped together — one
    // cell for the image(s) at column, videos in sequential cells after.
    std::function<void(int layerIndex, int column, const std::vector<juce::File>& images, const std::vector<juce::File>& videos)> onMixedFilesDrop;
    std::function<void(int layerIndex, int column, const juce::String& effectName)> onEffectDrop; // FX dropped from browser
    std::function<void(int layerIndex, int column, const juce::String& sourceId)> onSourceDrop; // Source dropped from browser
    std::function<void(int srcLayer, int srcCol, int dstLayer, int dstCol)> onClipMove; // Clip dragged from one cell to another
    std::function<void(int layerIndex, int column, const std::string& presetPath)> onMilkDropDrop; // Single MilkDrop preset
    std::function<void(int layerIndex, int column, const std::vector<std::string>& presetPaths)> onMilkDropPlaylistDrop; // Multi-preset playlist
    std::function<void(int layerIndex, int column)> onRevealInFinder;  // s-rta-1002b ui U2.2: the menu's "Show in Finder"

    // Update the thumbnail from the clip's media -- never decodes (s-rta-0928)
    void updateThumbnail();

    // s-rta-0928b mediaopen: a Finder drop's split (images / videos by extension, the videos in natural order) and its
    // dispatch (mixed -> one video -> several videos -> several images -> one image), shared by filesDropped and the
    // TEST-ONLY /api/debug/drop_files (MainComponent::debugDropFiles) so both reach the same handler for the same files.
    struct DropRoute { std::vector<juce::File> images, videos; };
    static DropRoute classifyDrop(const juce::StringArray& files);
    static void dispatchDrop(const DropRoute& route, int layerIndex, int column,
                             const std::function<void(int, int, const juce::File&)>& fileDrop,
                             const std::function<void(int, int, const std::vector<juce::File>&)>& multiFileDrop,
                             const std::function<void(int, int, const std::vector<juce::File>&)>& multiVideoDrop,
                             const std::function<void(int, int, const std::vector<juce::File>&,
                                                      const std::vector<juce::File>&)>& mixedFilesDrop);

private:
    juce::Rectangle<int> getThumbnailBounds() const;
    juce::Rectangle<int> getNameBarBounds() const;
    bool isInThumbnailArea(const juce::Point<int>& pos) const;
    void showContextMenu();
    std::optional<VideoInfo> lookupVideo() const;   // the source's answer for a Video clip, else nothing

    Clip* clip_ = nullptr;
    int layerIndex_ = 0;
    int column_ = 0;
    bool active_ = false;
    bool selected_ = false;
    bool dragHover_ = false;
    bool fxDragHover_ = false;
    bool sourceDragHover_ = false;

    juce::Image thumbnail_;
    // s-rta-0928: what thumbnail_ was made from -- re-derived only when this changes.
    ClipThumbnails* thumbs_ = nullptr;
    Clip::MediaType shownType_ = Clip::MediaType::None;
    juce::String shownPath_;
    const VideoInfoSource* videoInfo_ = nullptr;   // s-rta-1002b ui U2.2: DeckView's; nullptr = no video info
    MenuLauncher menuLauncher_;                     // empty = the real PopupMenu

    static constexpr int kNameBarHeight = 20;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipCell)
};
