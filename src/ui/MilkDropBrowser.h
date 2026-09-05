#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include "sources/ProjectMPresetManager.h"
#include "sources/PresetSelector.h"
#include <functional>
#include <deque>
#include <set>
#include <string>

// MilkDropBrowser: dedicated browser tab for MilkDrop presets.
//
// Sub-tabs: Curated | Favorites | Recent | All
//
// Three play modes (bottom panel):
//   Jukebox: classic autopilot — play/stop, pool, timing, blend
//   VJ Clip: click = preview, drag single preset to deck cell
//   Playlist: multi-select presets, drag group to deck cell as cycling playlist
class MilkDropBrowser : public juce::Component
{
public:
    MilkDropBrowser();
    ~MilkDropBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the preset manager to display from
    void setPresetManager(ProjectMPresetManager* mgr);

    // Set the preset selector for Auto-DJ controls
    void setPresetSelector(PresetSelector* sel);

    // Refresh the displayed preset list
    void refresh();

    // Record a preset as recently used
    void addToRecent(const std::string& presetPath);

    // Callbacks
    std::function<void(const std::string& presetPath)> onPresetSelected;

    // Fired when the user right-clicks a preset row to toggle its favorite
    // flag. MilkDropBrowser has no GL-context access of its own, so this
    // bubbles the request to MainComponent, which routes it through
    // Renderer::toggleFavoritePreset() — GL-thread confinement is required
    // because PresetInfo::favorite is read every frame on the GL thread by
    // PresetSelector::processFrame when Jukebox Pool = Favorites (L7-JUKE).
    std::function<void(int index)> onToggleFavoriteRequested;

    // Drag-drop: single preset → source clip in deck cell
    // Called by ClipCell/DeckView drop handler when desc starts with "milkdrop:"
    // Returns the preset path from the drag description
    static std::string parseDragDescription(const juce::String& desc);

    // Drag-drop: multiple presets → playlist clip in deck cell
    // Returns preset paths (comma-separated in drag desc "milkdrop_playlist:path1|path2|path3")
    static std::vector<std::string> parsePlaylistDragDescription(const juce::String& desc);

    // Build a "milkdrop_playlist:" drag description from preset paths (inverse
    // of parsePlaylistDragDescription). Shared by the multi-select and
    // header-drag gestures so both emit an identical payload format.
    static juce::String buildPlaylistDragDescription(const std::vector<std::string>& paths);

    // Get selected preset paths (for multi-select operations)
    std::vector<std::string> getSelectedPresetPaths() const;

    // Current Playlist-mode control values, read by the drop handler when
    // creating a playlist clip (see MainComponent's onMilkDropPlaylistDropped).
    int getPlaylistCycleModeId() const;    // playlistCycleSelector_: 1=Bag, 2=Random, 3=Sequential
    int getPlaylistTriggerBeats() const;   // playlistTimingSelector_ resolved to a beat count
    float getPlaylistBlendSeconds() const; // playlistBlendSlider_ value

private:
    // Internal list content component
    class PresetListContent;
    friend class PresetListContent;

    // Sub-tab enum
    enum class SubTab { Curated = 0, Favorites, Recent, All };
    SubTab activeSubTab_ = SubTab::Curated;

    // Play mode enum
    enum class PlayMode { Jukebox = 0, VJClip, Playlist };
    PlayMode activePlayMode_ = PlayMode::Jukebox;

    // Sub-tab buttons
    juce::TextButton curatedBtn_{"Curated"};
    juce::TextButton favoritesBtn_{"Favorites"};
    juce::TextButton recentBtn_{"Recent"};
    juce::TextButton allBtn_{"All"};

    // Search bar (visible in All tab)
    juce::TextEditor searchBox_;

    // List viewport
    juce::Viewport listViewport_;
    std::unique_ptr<PresetListContent> listContent_;

    // Navigation bar
    juce::TextButton prevBtn_{"<"};
    juce::TextButton nextBtn_{">"};
    juce::TextButton randomBtn_{"?"};
    juce::TextButton lockBtn_{"Lock"};

    // Play mode buttons
    juce::TextButton jukeboxModeBtn_{"Jukebox"};
    juce::TextButton vjModeBtn_{"VJ Clip"};
    juce::TextButton playlistModeBtn_{"Playlist"};

    // === Jukebox mode controls ===
    juce::TextButton jukeboxPlayBtn_{"Play"};
    juce::ComboBox jukeboxPoolSelector_;      // All, Curated, Favorites
    juce::ComboBox jukeboxModeSelector_;       // Random, Bag, Sequential
    juce::ComboBox jukeboxTimingSelector_;     // 4/8/16/32 beats, 10/30/60 sec
    juce::Label jukeboxBlendLabel_{"", "Blend:"};
    juce::Slider jukeboxBlendSlider_;

    // === Playlist mode controls ===
    juce::Label playlistInfoLabel_{"", "Select presets, drag to cell"};
    juce::ComboBox playlistCycleSelector_;     // Bag, Sequential, Random
    juce::ComboBox playlistTimingSelector_;    // 4/8/16/32 beats
    juce::Label playlistBlendLabel_{"", "Blend:"};
    juce::Slider playlistBlendSlider_;

    // Data
    ProjectMPresetManager* presetManager_ = nullptr;
    PresetSelector* presetSelector_ = nullptr;
    bool jukeboxPlaying_ = false;

    // Section state (for collapsible mood folders)
    struct Section
    {
        std::string name;
        bool expanded = true;
        juce::Colour color;
    };
    std::vector<Section> sections_;

    // Current search filter
    std::string searchFilter_;

    // Selected preset indices (supports multi-select in Playlist mode)
    std::set<int> selectedIndices_;
    int lastClickedIndex_ = -1;

    // Paths of presets in the section header last pressed (mouseDown), used by
    // mouseDrag to build a whole-group playlist payload. Empty when the last
    // press wasn't on a header.
    std::vector<std::string> pressedSectionPresetPaths_;

    // Recently used presets (paths, newest first)
    std::deque<std::string> recentPresets_;
    static constexpr int kMaxRecent = 20;

    // Layout constants
    static constexpr int kSubTabHeight = 24;
    static constexpr int kSearchBarHeight = 26;
    static constexpr int kSectionHeaderHeight = 22;
    static constexpr int kPresetRowHeight = 20;
    static constexpr int kNavBarHeight = 26;
    static constexpr int kModeBarHeight = 24;
    static constexpr int kBottomControlsHeight = 54;

    void initSections();
    void setSubTab(SubTab tab);
    void setPlayMode(PlayMode mode);
    void updateSubTabColors();
    void updateModeButtonColors();
    void updateSearch();
    int calculateContentHeight() const;
    void selectPreset(int globalIndex);
    void firePresetSelected(const std::string& path);
    void toggleJukeboxPlay();

    // Multi-select helpers
    bool isMultiSelectMode() const { return activePlayMode_ == PlayMode::Playlist; }
    void handlePresetClick(int globalIndex, bool shiftHeld, bool ctrlHeld);

    // Get presets for the active sub-tab and section
    std::vector<const ProjectMPresetManager::PresetInfo*> getPresetsForSection(const std::string& sectionName) const;
    std::vector<const ProjectMPresetManager::PresetInfo*> getCuratedPresets() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MilkDropBrowser)
};
