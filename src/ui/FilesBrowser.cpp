#include "ui/FilesBrowser.h"

// ── FileListContent: scrollable grid/list of file entries ──
class FilesBrowser::FileListContent : public juce::Component
{
public:
    FileListContent(FilesBrowser& owner) : owner_(owner) {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1a));

        auto& entries = owner_.entries_;
        if (entries.empty())
        {
            g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.5f));
            g.setFont(juce::Font(juce::FontOptions(11.0f)));
            g.drawText("Empty folder", getLocalBounds(), juce::Justification::centred, false);
            return;
        }

        if (owner_.gridView_)
            paintGrid(g, entries);
        else
            paintList(g, entries);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        dragStarted_ = false;
        int idx = hitTest(event.getPosition());

        if (event.mods.isRightButtonDown())
        {
            if (idx >= 0 && idx < static_cast<int>(owner_.entries_.size()))
            {
                owner_.toggleFavorite(owner_.entries_[static_cast<size_t>(idx)].file);
                repaint();
            }
            return;
        }

        if (idx < 0 || idx >= static_cast<int>(owner_.entries_.size()))
        {
            if (!event.mods.isCommandDown() && !event.mods.isShiftDown())
                owner_.selectedIndices_.clear();
            repaint();
            return;
        }

        auto& entry = owner_.entries_[static_cast<size_t>(idx)];

        if (entry.isDirectory)
        {
            owner_.navigateTo(entry.file);
            return;
        }

        // Multi-select
        if (event.mods.isCommandDown())
        {
            if (owner_.selectedIndices_.count(idx))
                owner_.selectedIndices_.erase(idx);
            else
                owner_.selectedIndices_.insert(idx);
            lastAnchorIdx_ = idx;
        }
        else if (event.mods.isShiftDown() && lastAnchorIdx_ >= 0)
        {
            int lo = std::min(lastAnchorIdx_, idx);
            int hi = std::max(lastAnchorIdx_, idx);
            for (int i = lo; i <= hi; ++i)
            {
                if (i < static_cast<int>(owner_.entries_.size()) && !owner_.entries_[static_cast<size_t>(i)].isDirectory)
                    owner_.selectedIndices_.insert(i);
            }
        }
        else
        {
            owner_.selectedIndices_.clear();
            owner_.selectedIndices_.insert(idx);
            lastAnchorIdx_ = idx;
        }
        repaint();
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        int idx = hitTest(event.getPosition());
        if (idx < 0 || idx >= static_cast<int>(owner_.entries_.size()))
            return;
        auto& entry = owner_.entries_[static_cast<size_t>(idx)];
        if (!entry.isDirectory && owner_.onFileActivated)
            owner_.onFileActivated(entry.file);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (dragStarted_ || owner_.selectedIndices_.empty())
            return;

        if (event.getDistanceFromDragStart() < 5)
            return;

        dragStarted_ = true;

        // Collect all selected files
        juce::StringArray files;
        for (int idx : owner_.selectedIndices_)
        {
            if (idx < static_cast<int>(owner_.entries_.size()))
            {
                auto& entry = owner_.entries_[static_cast<size_t>(idx)];
                if (!entry.isDirectory)
                    files.add(entry.file.getFullPathName());
            }
        }

        if (files.isEmpty()) return;

        // Use internal JUCE drag with "files:" prefix
        if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
        {
            juce::String desc = "files:" + files.joinIntoString("|");

            int count = files.size();
            juce::String label = count > 1
                ? juce::String(count) + " files"
                : juce::File(files[0]).getFileName();

            int imgW = 140, imgH = 24;
            juce::Image dragImg(juce::Image::ARGB, imgW, imgH, true);
            {
                juce::Graphics g(dragImg);
                g.setColour(juce::Colour(0xdd2a3040));
                g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(imgW),
                                       static_cast<float>(imgH), 4.0f);
                g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
                g.drawRoundedRectangle(0.5f, 0.5f, static_cast<float>(imgW - 1),
                                       static_cast<float>(imgH - 1), 4.0f, 1.0f);
                g.setColour(juce::Colours::white);
                g.setFont(juce::Font(juce::FontOptions(11.0f)));
                g.drawText(label, 8, 0, imgW - 16, imgH,
                           juce::Justification::centredLeft, true);
            }

            container->startDragging(juce::var(desc), this, juce::ScaledImage(dragImg), true);
        }
    }

    int getRequiredHeight() const
    {
        int count = static_cast<int>(owner_.entries_.size());
        if (count == 0) return 100;

        if (owner_.gridView_)
        {
            int cols = std::max(1, getWidth() / (kThumbSize + kPadding));
            int rows = (count + cols - 1) / cols;
            return rows * (kThumbSize + kLabelHeight + kPadding) + kPadding;
        }
        else
        {
            return count * kListRowHeight;
        }
    }

    void updateSize()
    {
        setSize(std::max(1, owner_.viewport_.getWidth() - 8), getRequiredHeight());
    }

private:
    static constexpr int kThumbSize = 64;
    static constexpr int kLabelHeight = 16;
    static constexpr int kPadding = 4;
    static constexpr int kListRowHeight = 20;

    FilesBrowser& owner_;
    bool dragStarted_ = false;
    int lastAnchorIdx_ = -1;

    void paintGrid(juce::Graphics& g, const std::vector<FileEntry>& entries)
    {
        int cols = std::max(1, getWidth() / (kThumbSize + kPadding));
        int x = kPadding, y = kPadding;

        for (size_t i = 0; i < entries.size(); ++i)
        {
            auto& e = entries[i];
            auto thumbRect = juce::Rectangle<int>(x, y, kThumbSize, kThumbSize);

            // Background
            bool selected = owner_.selectedIndices_.count(static_cast<int>(i)) > 0;
            g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
            g.fillRect(thumbRect);

            // Thumbnail or folder icon
            if (e.isDirectory)
            {
                g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.6f));
                g.setFont(juce::Font(juce::FontOptions(24.0f)));
                g.drawText(juce::String::charToString(0x1F4C1), thumbRect,
                           juce::Justification::centred, false);
            }
            else if (e.thumbnail.isValid())
            {
                g.drawImageWithin(e.thumbnail, x, y, kThumbSize, kThumbSize,
                                  juce::RectanglePlacement::centred);
            }
            else
            {
                g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.3f));
                g.drawRect(thumbRect, 1);
            }

            // Favorite star
            if (e.isFavorite)
            {
                g.setColour(juce::Colour(AudioDNALookAndFeel::kMeterYellow));
                g.setFont(juce::Font(juce::FontOptions(10.0f)));
                g.drawText("*", x + kThumbSize - 12, y + 1, 11, 11,
                           juce::Justification::centred, false);
            }

            // Name label
            g.setColour(selected ? juce::Colours::white : juce::Colour(AudioDNALookAndFeel::kTextPrimary));
            g.setFont(juce::Font(juce::FontOptions(9.0f)));
            g.drawText(e.file.getFileName(),
                       x, y + kThumbSize, kThumbSize, kLabelHeight,
                       juce::Justification::centredTop, true);

            // Selection border (drawn OVER thumbnail)
            if (selected)
            {
                g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
                g.drawRect(thumbRect, 2);
                // Tint overlay
                g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.15f));
                g.fillRect(thumbRect);
            }

            x += kThumbSize + kPadding;
            if ((static_cast<int>(i) + 1) % cols == 0)
            {
                x = kPadding;
                y += kThumbSize + kLabelHeight + kPadding;
            }
        }
    }

    void paintList(juce::Graphics& g, const std::vector<FileEntry>& entries)
    {
        int y = 0;
        for (size_t i = 0; i < entries.size(); ++i)
        {
            auto& e = entries[i];
            auto rowRect = juce::Rectangle<int>(0, y, getWidth(), kListRowHeight);

            // Row background (highlight if selected)
            bool selected = owner_.selectedIndices_.count(static_cast<int>(i)) > 0;
            if (selected)
                g.setColour(juce::Colour(0xff3a4a6e));
            else if (i % 2 == 0)
                g.setColour(juce::Colour(0xff1e1e1e));
            else
                g.setColour(juce::Colour(0xff1a1a1a));
            g.fillRect(rowRect);

            // Icon area
            if (e.isDirectory)
            {
                g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.6f));
                g.setFont(juce::Font(juce::FontOptions(12.0f)));
                g.drawText("D", 2, y, 16, kListRowHeight, juce::Justification::centred, false);
            }
            else if (e.isFavorite)
            {
                g.setColour(juce::Colour(AudioDNALookAndFeel::kMeterYellow));
                g.setFont(juce::Font(juce::FontOptions(10.0f)));
                g.drawText("*", 2, y, 16, kListRowHeight, juce::Justification::centred, false);
            }

            // File name
            g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
            g.setFont(juce::Font(juce::FontOptions(11.0f)));
            g.drawText(e.file.getFileName(), 20, y, getWidth() - 24, kListRowHeight,
                       juce::Justification::centredLeft, true);

            y += kListRowHeight;
        }
    }

    int hitTest(juce::Point<int> pos) const
    {
        int count = static_cast<int>(owner_.entries_.size());
        if (count == 0) return -1;

        if (owner_.gridView_)
        {
            int cols = std::max(1, getWidth() / (kThumbSize + kPadding));
            int col = (pos.x - kPadding) / (kThumbSize + kPadding);
            int row = (pos.y - kPadding) / (kThumbSize + kLabelHeight + kPadding);
            if (col < 0 || col >= cols) return -1;
            int idx = row * cols + col;
            return (idx >= 0 && idx < count) ? idx : -1;
        }
        else
        {
            int idx = pos.y / kListRowHeight;
            return (idx >= 0 && idx < count) ? idx : -1;
        }
    }
};

// ── FilesBrowser implementation ──

FilesBrowser::~FilesBrowser() = default;

FilesBrowser::FilesBrowser()
{
    fileListContent_ = std::make_unique<FileListContent>(*this);
    viewport_.setViewedComponent(fileListContent_.get(), false);
    viewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport_);

    // Navigation
    addAndMakeVisible(upButton_);
    upButton_.onClick = [this] {
        if (currentDir_.getParentDirectory() != currentDir_)
            navigateTo(currentDir_.getParentDirectory());
    };

    addAndMakeVisible(pathBar_);
    pathBar_.setFont(juce::Font(juce::FontOptions(11.0f)));
    pathBar_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(AudioDNALookAndFeel::kSurface));
    pathBar_.setColour(juce::TextEditor::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    pathBar_.setColour(juce::TextEditor::outlineColourId, juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    pathBar_.onReturnKey = [this] {
        auto path = pathBar_.getText();
        juce::File dir(path);
        if (dir.isDirectory())
            navigateTo(dir);
    };

    // Search
    addAndMakeVisible(searchField_);
    searchField_.setFont(juce::Font(juce::FontOptions(11.0f)));
    searchField_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(AudioDNALookAndFeel::kSurface));
    searchField_.setColour(juce::TextEditor::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    searchField_.setColour(juce::TextEditor::outlineColourId, juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    searchField_.setTextToShowWhenEmpty("Search...", juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    searchField_.onTextChange = [this] { filterBySearch(); };

    // View toggle
    addAndMakeVisible(gridViewBtn_);
    addAndMakeVisible(listViewBtn_);
    gridViewBtn_.onClick = [this] { gridView_ = true; refreshFileList(); repaint(); };
    listViewBtn_.onClick = [this] { gridView_ = false; refreshFileList(); repaint(); };

    // Start at user's home directory
    navigateTo(juce::File::getSpecialLocation(juce::File::userHomeDirectory));
    loadFavorites();
}

void FilesBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void FilesBrowser::resized()
{
    auto area = getLocalBounds();

    // Navigation bar: [Up] [Path..........................]
    auto navBar = area.removeFromTop(kNavBarHeight);
    upButton_.setBounds(navBar.removeFromLeft(24));
    navBar.removeFromLeft(2);
    pathBar_.setBounds(navBar);

    area.removeFromTop(1);

    // Search + view toggle: [Search...............] [Grid] [List]
    auto searchBar = area.removeFromTop(kSearchBarHeight);
    listViewBtn_.setBounds(searchBar.removeFromRight(32));
    searchBar.removeFromRight(1);
    gridViewBtn_.setBounds(searchBar.removeFromRight(32));
    searchBar.removeFromRight(2);
    searchField_.setBounds(searchBar);

    area.removeFromTop(1);

    // File list viewport
    viewport_.setBounds(area);
    if (fileListContent_)
        fileListContent_->updateSize();
}

void FilesBrowser::navigateTo(const juce::File& folder)
{
    if (!folder.isDirectory()) return;
    currentDir_ = folder;
    pathBar_.setText(folder.getFullPathName(), false);
    searchField_.clear();
    refreshFileList();
}

void FilesBrowser::refreshFileList()
{
    entries_.clear();
    if (!currentDir_.isDirectory()) return;

    // Directories first
    auto dirs = currentDir_.findChildFiles(juce::File::findDirectories, false);
    dirs.sort();
    for (auto& d : dirs)
    {
        if (d.getFileName().startsWithChar('.')) continue; // skip hidden
        entries_.push_back({d, {}, true, false});
    }

    // Then media files
    auto files = currentDir_.findChildFiles(juce::File::findFiles, false);
    files.sort();
    for (auto& f : files)
    {
        if (f.getFileName().startsWithChar('.')) continue;
        if (!isMediaFile(f)) continue;

        bool fav = favorites_.contains(f.getFullPathName());
        auto thumb = generateThumbnail(f);
        entries_.push_back({f, thumb, false, fav});
    }

    if (fileListContent_)
    {
        fileListContent_->updateSize();
        fileListContent_->repaint();
    }
    viewport_.setViewPosition(0, 0);
}

void FilesBrowser::filterBySearch()
{
    auto query = searchField_.getText().toLowerCase();
    if (query.isEmpty())
    {
        refreshFileList();
        return;
    }

    // Re-scan and filter
    entries_.clear();
    if (!currentDir_.isDirectory()) return;

    auto allFiles = currentDir_.findChildFiles(juce::File::findFilesAndDirectories, false);
    allFiles.sort();
    for (auto& f : allFiles)
    {
        if (f.getFileName().startsWithChar('.')) continue;
        if (!f.getFileName().toLowerCase().contains(query)) continue;

        if (f.isDirectory())
        {
            entries_.push_back({f, {}, true, false});
        }
        else if (isMediaFile(f))
        {
            bool fav = favorites_.contains(f.getFullPathName());
            entries_.push_back({f, generateThumbnail(f), false, fav});
        }
    }

    if (fileListContent_)
    {
        fileListContent_->updateSize();
        fileListContent_->repaint();
    }
}

bool FilesBrowser::isMediaFile(const juce::File& file) const
{
    auto ext = file.getFileExtension().toLowerCase();
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" ||
           ext == ".bmp" || ext == ".tiff" || ext == ".tif" ||
           ext == ".wav" || ext == ".aiff" || ext == ".aif" ||
           ext == ".mp3" || ext == ".flac" || ext == ".ogg" ||
           ext == ".avi" || ext == ".mov" || ext == ".mp4";
}

juce::Image FilesBrowser::generateThumbnail(const juce::File& file)
{
    auto ext = file.getFileExtension().toLowerCase();
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" ||
        ext == ".bmp" || ext == ".tiff" || ext == ".tif")
    {
        auto img = juce::ImageFileFormat::loadFrom(file);
        if (img.isValid())
            return img.rescaled(kThumbSize, kThumbSize, juce::Graphics::mediumResamplingQuality);
    }
    return {};
}

void FilesBrowser::loadFavorites()
{
    auto favFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("AudioDNA").getChildFile("browser_favorites.txt");
    if (favFile.existsAsFile())
    {
        juce::StringArray lines;
        lines.addLines(favFile.loadFileAsString());
        favorites_ = lines;
    }
}

void FilesBrowser::saveFavorites()
{
    auto favFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("AudioDNA").getChildFile("browser_favorites.txt");
    favFile.getParentDirectory().createDirectory();
    favFile.replaceWithText(favorites_.joinIntoString("\n"));
}

void FilesBrowser::toggleFavorite(const juce::File& file)
{
    auto path = file.getFullPathName();
    if (favorites_.contains(path))
        favorites_.removeString(path);
    else
        favorites_.add(path);

    // Update entry
    for (auto& e : entries_)
    {
        if (e.file == file)
            e.isFavorite = favorites_.contains(path);
    }

    saveFavorites();
}
