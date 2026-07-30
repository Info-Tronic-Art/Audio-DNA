#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include "ui/ThumbnailCache.h"
#include <cstdint>
#include <vector>
#include <set>

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
    // NOTE: thumbnailPool_ must finish draining (see ~FilesBrowser()) before
    // this is touched — a completed decode job writes into entries_ via
    // onThumbnailDecoded. The drain happens explicitly in the destructor
    // body, not via member-declaration order, so it holds regardless of
    // where entries_/thumbnailPool_ end up being declared relative to each
    // other.
    std::vector<FileEntry> entries_;
    std::set<int> selectedIndices_;  // multi-select tracking

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

    // Decodes + rescales a thumbnail. Static (no member access) so it is safe
    // to call from a background thread pool job.
    static juce::Image generateThumbnail(const juce::File& file);

    // Thumbnail cache + async decode (perf: avoid full-res image decode on
    // the message thread). Enumeration/list-building stays synchronous
    // (cheap); only the per-file decode is dispatched to thumbnailPool_.
    ThumbnailCache thumbnailCache_;
    // NOTE: must be drained (removeAllJobs) BEFORE entries_ is touched — see
    // ~FilesBrowser(), which does this explicitly as the first thing it does
    // rather than relying on this member's declaration position. A completed
    // job's onThumbnailDecoded callback reads/writes entries_, so the pool
    // must be fully stopped before teardown can safely proceed past it.
    juce::ThreadPool thumbnailPool_{juce::ThreadPoolOptions{}
                                         .withNumberOfThreads(2)
                                         .withThreadName("FilesBrowserThumbs")};
    static constexpr int kThumbnailPoolShutdownTimeoutMs = 5000;

    // Bumped on every refreshFileList()/filterBySearch() call. A completed
    // decode job compares its captured generation against this before
    // touching entries_, so a stale job from a folder the user already
    // navigated away from is dropped instead of corrupting the current list.
    uint64_t decodeGeneration_ = 0;

    void requestThumbnailAsync(const juce::File& file, uint64_t generation);

    // mtimeAtDecode is the file's mtime as read on the pool thread at the
    // moment its bytes were decoded — NOT re-queried here — so the cache
    // entry always matches the content that was actually decoded even if the
    // file changed again during the hop back to the message thread.
    void onThumbnailDecoded(const juce::File& file, juce::Time mtimeAtDecode,
                             uint64_t generation, juce::Image thumbnail);

    static constexpr int kNavBarHeight = 24;
    static constexpr int kSearchBarHeight = 22;
    static constexpr int kThumbSize = 64;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilesBrowser)
};
