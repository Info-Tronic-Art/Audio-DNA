#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include <vector>

// FilesBrowser: folder navigation with thumbnails, search, favorites.
// Users can browse images/videos/audio and drag them onto deck cells.
class FilesBrowser : public juce::Component,
                     public juce::FileDragAndDropTarget
{
public:
    FilesBrowser();
    ~FilesBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Navigate to a specific folder
    void navigateTo(const juce::File& folder);

    // Get the current directory
    juce::File getCurrentDirectory() const { return currentDir_; }

    // Callback when user wants to drag a file onto a deck cell
    std::function<void(const juce::File&)> onFileActivated;

    // FileDragAndDropTarget (for receiving drags INTO this panel — unused but required)
    bool isInterestedInFileDrag(const juce::StringArray&) override { return false; }
    void filesDropped(const juce::StringArray&, int, int) override {}

private:
    // Navigation
    juce::File currentDir_;
    juce::TextButton upButton_{"^"};
    juce::TextEditor pathBar_;
    juce::TextEditor searchField_;

    // View toggle
    bool gridView_ = true;  // true = thumbnail grid, false = list
    juce::TextButton gridViewBtn_{"Grid"};
    juce::TextButton listViewBtn_{"List"};

    // File list
    struct FileEntry
    {
        juce::File file;
        juce::Image thumbnail;
        bool isDirectory = false;
        bool isFavorite = false;
    };
    std::vector<FileEntry> entries_;

    // Scrollable content
    juce::Viewport viewport_;
    class FileListContent;
    std::unique_ptr<FileListContent> fileListContent_;

    // Favorites
    juce::StringArray favorites_;
    void loadFavorites();
    void saveFavorites();
    void toggleFavorite(const juce::File& file);

    // Helpers
    void refreshFileList();
    void filterBySearch();
    bool isMediaFile(const juce::File& file) const;
    juce::Image generateThumbnail(const juce::File& file);

    static constexpr int kNavBarHeight = 24;
    static constexpr int kSearchBarHeight = 22;
    static constexpr int kThumbSize = 64;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilesBrowser)
};
