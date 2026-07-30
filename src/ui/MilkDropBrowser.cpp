#include "ui/MilkDropBrowser.h"
#include <algorithm>
#include <random>

// === PresetListContent — scrollable list inside the viewport ===

class MilkDropBrowser::PresetListContent : public juce::Component
{
public:
    PresetListContent(MilkDropBrowser& owner) : owner_(owner) {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1a));

        if (!owner_.presetManager_ || owner_.presetManager_->getPresetCount() == 0)
        {
            g.setColour(juce::Colour(0xff606070));
            g.setFont(juce::Font(juce::FontOptions(12.0f)));
            juce::String msg = "No presets loaded.\n\n"
                "To add MilkDrop presets:\n"
                "1. Place .milk files in the app's\n"
                "   resources/projectm_presets/ folder\n"
                "2. Or set a directory in Preferences > Video";
            g.drawText(msg, getLocalBounds().reduced(16), juce::Justification::centred, true);
            return;
        }

        int y = 0;
        switch (owner_.activeSubTab_)
        {
            case SubTab::Curated:   paintGrouped(g, y, true); break;
            case SubTab::Favorites: paintFlat(g, y, owner_.presetManager_->getFavorites()); break;
            case SubTab::Recent:    paintRecent(g, y); break;
            case SubTab::All:       paintGrouped(g, y, false); break;
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (!owner_.presetManager_) return;

        int idx = presetIndexAtY(e.y);
        if (idx >= 0)
        {
            owner_.pressedSectionPresetPaths_.clear();
            if (e.mods.isRightButtonDown())
            {
                owner_.presetManager_->toggleFavorite(idx);
                repaint();
                return;
            }
            owner_.handlePresetClick(idx, e.mods.isShiftDown(), e.mods.isCommandDown());
        }
        else
        {
            // Record the section under the cursor before toggling, so a
            // subsequent mouseDrag can still build a whole-group playlist
            // payload from it even though toggling shifts row layout below
            // the header. Click-to-toggle keeps firing immediately, same as
            // before; drag-to-playlist is decided by mouseDrag's own
            // drag-threshold check.
            owner_.pressedSectionPresetPaths_.clear();
            for (auto* p : sectionAtY(e.y))
                owner_.pressedSectionPresetPaths_.push_back(p->path);
            handleSectionHeaderClick(e.y);
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (!owner_.presetManager_) return;
        if (e.getDistanceFromDragStart() < 5) return;

        auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this);
        if (!container) return;
        if (container->isDragAndDropActive()) return;

        if (!owner_.pressedSectionPresetPaths_.empty())
        {
            // Header drag: whole section as a playlist.
            auto desc = MilkDropBrowser::buildPlaylistDragDescription(owner_.pressedSectionPresetPaths_);
            auto img = juce::Image(juce::Image::ARGB, 120, 24, true);
            juce::Graphics ig(img);
            ig.setColour(juce::Colour(0xcc2a2a5e));
            ig.fillRoundedRectangle(0, 0, 120, 24, 4);
            ig.setColour(juce::Colours::white);
            ig.setFont(juce::Font(juce::FontOptions(11.0f)));
            ig.drawText(juce::String(static_cast<int>(owner_.pressedSectionPresetPaths_.size())) + " presets",
                        0, 0, 120, 24, juce::Justification::centred);
            container->startDragging(desc, this, juce::ScaledImage(img), true);
        }
        else if (owner_.isMultiSelectMode() && owner_.selectedIndices_.size() > 1)
        {
            // Multi-select: drag as playlist
            std::vector<std::string> paths;
            for (int i : owner_.selectedIndices_)
            {
                auto* p = owner_.presetManager_->getPreset(i);
                if (p)
                    paths.push_back(p->path);
            }
            auto desc = MilkDropBrowser::buildPlaylistDragDescription(paths);
            auto img = juce::Image(juce::Image::ARGB, 120, 24, true);
            juce::Graphics ig(img);
            ig.setColour(juce::Colour(0xcc2a2a5e));
            ig.fillRoundedRectangle(0, 0, 120, 24, 4);
            ig.setColour(juce::Colours::white);
            ig.setFont(juce::Font(juce::FontOptions(11.0f)));
            ig.drawText(juce::String(static_cast<int>(owner_.selectedIndices_.size())) + " presets",
                        0, 0, 120, 24, juce::Justification::centred);
            container->startDragging(desc, this, juce::ScaledImage(img), true);
        }
        else if (owner_.lastClickedIndex_ >= 0)
        {
            // Single drag
            auto* p = owner_.presetManager_->getPreset(owner_.lastClickedIndex_);
            if (p)
            {
                juce::String desc = "milkdrop:" + juce::String(p->path);
                auto img = juce::Image(juce::Image::ARGB, 140, 20, true);
                juce::Graphics ig(img);
                ig.setColour(juce::Colour(0xcc2a2a5e));
                ig.fillRoundedRectangle(0, 0, 140, 20, 3);
                ig.setColour(juce::Colours::white);
                ig.setFont(juce::Font(juce::FontOptions(10.0f)));
                ig.drawText(juce::String(p->name), 4, 0, 132, 20, juce::Justification::centredLeft);
                container->startDragging(desc, this, juce::ScaledImage(img), true);
            }
        }
    }

private:
    MilkDropBrowser& owner_;

    // === Paint helpers ===

    void paintPresetRow(juce::Graphics& g, int y, const ProjectMPresetManager::PresetInfo* preset,
                        bool selected, int width)
    {
        bool multiSelected = false;
        if (owner_.isMultiSelectMode())
        {
            int idx = findGlobalIndex(preset);
            multiSelected = idx >= 0 && owner_.selectedIndices_.count(idx) > 0;
        }

        g.setColour(multiSelected ? juce::Colour(0xff4a3a6e)
                   : selected ? juce::Colour(0xff3a3a5e)
                   : juce::Colour(0xff1e1e2e));
        g.fillRect(0, y, width, kPresetRowHeight);

        // Favorite star
        g.setColour(preset->favorite ? juce::Colour(0xffffff00) : juce::Colour(0xff353540));
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.drawText(preset->favorite ? "*" : ".", 4, y, 14, kPresetRowHeight,
                   juce::Justification::centredLeft, false);

        // Mood color dot
        g.setColour(getMoodColor(preset->mood).withAlpha(0.6f));
        g.fillEllipse(20.0f, static_cast<float>(y) + 5.0f, 10.0f, 10.0f);

        // Preset name
        g.setColour((selected || multiSelected) ? juce::Colours::white
                    : juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawText(juce::String(preset->name), 34, y, width - 38, kPresetRowHeight,
                   juce::Justification::centredLeft, false);
    }

    void paintSectionHeader(juce::Graphics& g, int y, const Section& section, int count, int width)
    {
        g.setColour(juce::Colour(0xff222233));
        g.fillRect(0, y, width, kSectionHeaderHeight);
        g.setColour(section.color);
        g.fillRect(0, y, 3, kSectionHeaderHeight);

        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(section.expanded ? "v" : ">", 6, y, 12, kSectionHeaderHeight,
                   juce::Justification::centredLeft, false);
        g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        g.drawText(juce::String(section.name) + " (" + juce::String(count) + ")",
                   20, y, width - 24, kSectionHeaderHeight,
                   juce::Justification::centredLeft, false);
    }

    static juce::Colour getMoodColor(const std::string& mood)
    {
        if (mood == "Calm")        return juce::Colour(0xff4488ff);
        if (mood == "Energetic")   return juce::Colour(0xffff4444);
        if (mood == "Psychedelic") return juce::Colour(0xffcc44ff);
        if (mood == "Geometric")   return juce::Colour(0xff44cccc);
        if (mood == "Dark")        return juce::Colour(0xff666688);
        if (mood == "Minimal")     return juce::Colour(0xffaaaaaa);
        return juce::Colour(0xff888888);
    }

    bool isSelected(const ProjectMPresetManager::PresetInfo* preset) const
    {
        int idx = findGlobalIndex(preset);
        if (idx < 0) return false;
        return idx == owner_.lastClickedIndex_;
    }

    int findGlobalIndex(const ProjectMPresetManager::PresetInfo* preset) const
    {
        auto& all = owner_.presetManager_->getAllPresets();
        for (int i = 0; i < static_cast<int>(all.size()); ++i)
            if (&all[static_cast<size_t>(i)] == preset) return i;
        return -1;
    }

    // === Paint modes ===

    void paintGrouped(juce::Graphics& g, int& y, bool curatedOnly)
    {
        for (auto& section : owner_.sections_)
        {
            std::vector<const ProjectMPresetManager::PresetInfo*> presets;
            if (curatedOnly)
            {
                auto curated = owner_.getCuratedPresets();
                for (auto* p : curated)
                    if (p->mood == section.name) presets.push_back(p);
            }
            else
            {
                presets = owner_.getPresetsForSection(section.name);
            }
            if (presets.empty()) continue;

            paintSectionHeader(g, y, section, static_cast<int>(presets.size()), getWidth());
            y += kSectionHeaderHeight;

            if (section.expanded)
            {
                for (auto* p : presets)
                {
                    paintPresetRow(g, y, p, isSelected(p), getWidth());
                    y += kPresetRowHeight;
                }
            }
        }
    }

    void paintFlat(juce::Graphics& g, int& y,
                   const std::vector<const ProjectMPresetManager::PresetInfo*>& presets)
    {
        if (presets.empty())
        {
            g.setColour(juce::Colour(0xff606070));
            g.setFont(juce::Font(juce::FontOptions(11.0f)));
            g.drawText("No favorites yet. Right-click to star.",
                       getLocalBounds().reduced(16), juce::Justification::centred, true);
            return;
        }
        for (auto* p : presets)
        {
            paintPresetRow(g, y, p, isSelected(p), getWidth());
            y += kPresetRowHeight;
        }
    }

    void paintRecent(juce::Graphics& g, int& y)
    {
        if (owner_.recentPresets_.empty())
        {
            g.setColour(juce::Colour(0xff606070));
            g.setFont(juce::Font(juce::FontOptions(11.0f)));
            g.drawText("No recently used presets.",
                       getLocalBounds().reduced(16), juce::Justification::centred, true);
            return;
        }
        for (const auto& path : owner_.recentPresets_)
        {
            const ProjectMPresetManager::PresetInfo* found = nullptr;
            for (const auto& p : owner_.presetManager_->getAllPresets())
                if (p.path == path) { found = &p; break; }
            if (!found) continue;
            paintPresetRow(g, y, found, isSelected(found), getWidth());
            y += kPresetRowHeight;
        }
    }

    // === Hit testing ===

    int presetIndexAtY(int posY)
    {
        int y = 0;
        auto visitPresets = [&](const std::vector<const ProjectMPresetManager::PresetInfo*>& presets) -> int {
            for (auto* p : presets)
            {
                if (posY >= y && posY < y + kPresetRowHeight)
                    return findGlobalIndex(p);
                y += kPresetRowHeight;
            }
            return -1;
        };

        switch (owner_.activeSubTab_)
        {
            case SubTab::Curated:
            case SubTab::All:
            {
                bool curatedOnly = (owner_.activeSubTab_ == SubTab::Curated);
                for (auto& section : owner_.sections_)
                {
                    std::vector<const ProjectMPresetManager::PresetInfo*> presets;
                    if (curatedOnly)
                    {
                        auto curated = owner_.getCuratedPresets();
                        for (auto* p : curated)
                            if (p->mood == section.name) presets.push_back(p);
                    }
                    else
                        presets = owner_.getPresetsForSection(section.name);

                    if (presets.empty()) continue;

                    // Section header
                    if (posY >= y && posY < y + kSectionHeaderHeight)
                        return -2; // header click
                    y += kSectionHeaderHeight;

                    if (section.expanded)
                    {
                        int result = visitPresets(presets);
                        if (result >= 0) return result;
                    }
                }
                break;
            }
            case SubTab::Favorites:
            {
                auto favs = owner_.presetManager_->getFavorites();
                return visitPresets(favs);
            }
            case SubTab::Recent:
            {
                for (const auto& path : owner_.recentPresets_)
                {
                    const ProjectMPresetManager::PresetInfo* found = nullptr;
                    for (const auto& p : owner_.presetManager_->getAllPresets())
                        if (p.path == path) { found = &p; break; }
                    if (!found) continue;
                    if (posY >= y && posY < y + kPresetRowHeight)
                        return findGlobalIndex(found);
                    y += kPresetRowHeight;
                }
                break;
            }
        }
        return -1;
    }

    // Returns the presets belonging to whichever section header (if any) is at
    // posY, mirroring handleSectionHeaderClick's walk exactly but returning the
    // section's full preset list instead of toggling expand/collapse.
    std::vector<const ProjectMPresetManager::PresetInfo*> sectionAtY(int posY)
    {
        int y = 0;
        bool curatedOnly = (owner_.activeSubTab_ == SubTab::Curated);

        for (auto& section : owner_.sections_)
        {
            std::vector<const ProjectMPresetManager::PresetInfo*> presets;
            if (curatedOnly)
            {
                auto curated = owner_.getCuratedPresets();
                for (auto* p : curated)
                    if (p->mood == section.name) presets.push_back(p);
            }
            else if (owner_.activeSubTab_ == SubTab::All)
                presets = owner_.getPresetsForSection(section.name);
            else
                return {}; // No headers in other tabs

            if (presets.empty()) continue;

            if (posY >= y && posY < y + kSectionHeaderHeight)
                return presets;
            y += kSectionHeaderHeight;
            if (section.expanded)
                y += static_cast<int>(presets.size()) * kPresetRowHeight;
        }
        return {};
    }

    void handleSectionHeaderClick(int posY)
    {
        int y = 0;
        bool curatedOnly = (owner_.activeSubTab_ == SubTab::Curated);

        for (auto& section : owner_.sections_)
        {
            std::vector<const ProjectMPresetManager::PresetInfo*> presets;
            if (curatedOnly)
            {
                auto curated = owner_.getCuratedPresets();
                for (auto* p : curated)
                    if (p->mood == section.name) presets.push_back(p);
            }
            else if (owner_.activeSubTab_ == SubTab::All)
                presets = owner_.getPresetsForSection(section.name);
            else
                return; // No headers in other tabs

            if (presets.empty()) continue;

            if (posY >= y && posY < y + kSectionHeaderHeight)
            {
                section.expanded = !section.expanded;
                owner_.resized();
                repaint();
                return;
            }
            y += kSectionHeaderHeight;
            if (section.expanded)
                y += static_cast<int>(presets.size()) * kPresetRowHeight;
        }
    }
};

// === MilkDropBrowser ===

MilkDropBrowser::~MilkDropBrowser() = default;

MilkDropBrowser::MilkDropBrowser()
{
    // Sub-tab buttons
    auto setupSubTab = [this](juce::TextButton& btn, SubTab tab) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a1a2e));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff606070));
        btn.onClick = [this, tab] { setSubTab(tab); };
        addAndMakeVisible(btn);
    };
    setupSubTab(curatedBtn_, SubTab::Curated);
    setupSubTab(favoritesBtn_, SubTab::Favorites);
    setupSubTab(recentBtn_, SubTab::Recent);
    setupSubTab(allBtn_, SubTab::All);

    // Search box
    searchBox_.setTextToShowWhenEmpty("Search presets...", juce::Colour(0xff606070));
    searchBox_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    searchBox_.setColour(juce::TextEditor::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    searchBox_.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff333333));
    searchBox_.onTextChange = [this] { updateSearch(); };
    addAndMakeVisible(searchBox_);

    // List viewport
    listContent_ = std::make_unique<PresetListContent>(*this);
    listViewport_.setViewedComponent(listContent_.get(), false);
    listViewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(listViewport_);

    // Navigation buttons
    auto setupBtn = [this](juce::TextButton& btn) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a3e));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        addAndMakeVisible(btn);
    };
    setupBtn(prevBtn_);
    setupBtn(nextBtn_);
    setupBtn(randomBtn_);
    setupBtn(lockBtn_);

    prevBtn_.onClick = [this] {
        if (!presetManager_) return;
        auto* p = presetManager_->prevPreset();
        if (p) { lastClickedIndex_ = presetManager_->getCurrentIndex(); firePresetSelected(p->path); }
    };
    nextBtn_.onClick = [this] {
        if (!presetManager_) return;
        auto* p = presetManager_->nextPreset();
        if (p) { lastClickedIndex_ = presetManager_->getCurrentIndex(); firePresetSelected(p->path); }
    };
    randomBtn_.onClick = [this] {
        if (!presetManager_) return;
        auto* p = presetManager_->randomPreset();
        if (p) { lastClickedIndex_ = presetManager_->getCurrentIndex(); firePresetSelected(p->path); }
    };
    lockBtn_.onClick = [this] {
        if (presetSelector_)
        {
            presetSelector_->setEnabled(!presetSelector_->isEnabled());
            lockBtn_.setButtonText(presetSelector_->isEnabled() ? "Lock" : "Locked");
            lockBtn_.setColour(juce::TextButton::buttonColourId,
                               presetSelector_->isEnabled() ? juce::Colour(0xff2a2a3e) : juce::Colour(0xff5a2a2a));
        }
    };

    // === Play mode buttons ===
    auto setupModeBtn = [this](juce::TextButton& btn, PlayMode mode) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a1a2e));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff606070));
        btn.onClick = [this, mode] { setPlayMode(mode); };
        addAndMakeVisible(btn);
    };
    setupModeBtn(jukeboxModeBtn_, PlayMode::Jukebox);
    setupModeBtn(vjModeBtn_, PlayMode::VJClip);
    setupModeBtn(playlistModeBtn_, PlayMode::Playlist);

    // === Jukebox controls ===
    jukeboxPlayBtn_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a5a2a));
    jukeboxPlayBtn_.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    jukeboxPlayBtn_.onClick = [this] { toggleJukeboxPlay(); };
    addAndMakeVisible(jukeboxPlayBtn_);

    auto setupCombo = [](juce::ComboBox& cb) {
        cb.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a3e));
        cb.setColour(juce::ComboBox::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    };

    jukeboxPoolSelector_.addItem("All", 1);
    jukeboxPoolSelector_.addItem("Curated", 2);
    jukeboxPoolSelector_.addItem("Favorites", 3);
    jukeboxPoolSelector_.setSelectedId(1, juce::dontSendNotification);
    setupCombo(jukeboxPoolSelector_);
    addAndMakeVisible(jukeboxPoolSelector_);

    jukeboxModeSelector_.addItem("Bag", 1);
    jukeboxModeSelector_.addItem("Random", 2);
    jukeboxModeSelector_.addItem("Sequential", 3);
    jukeboxModeSelector_.setSelectedId(1, juce::dontSendNotification);
    setupCombo(jukeboxModeSelector_);
    addAndMakeVisible(jukeboxModeSelector_);

    jukeboxTimingSelector_.addItem("4 beats", 1);
    jukeboxTimingSelector_.addItem("8 beats", 2);
    jukeboxTimingSelector_.addItem("16 beats", 3);
    jukeboxTimingSelector_.addItem("32 beats", 4);
    jukeboxTimingSelector_.addItem("30 sec", 5);
    jukeboxTimingSelector_.addItem("60 sec", 6);
    jukeboxTimingSelector_.setSelectedId(3, juce::dontSendNotification);
    jukeboxTimingSelector_.onChange = [this] {
        if (!presetSelector_) return;
        int bars[] = {1, 2, 4, 8, 8, 16};
        int idx = jukeboxTimingSelector_.getSelectedId() - 1;
        if (idx >= 0 && idx < 6)
            presetSelector_->setTransitionBars(bars[idx]);
    };
    setupCombo(jukeboxTimingSelector_);
    addAndMakeVisible(jukeboxTimingSelector_);

    jukeboxBlendLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    jukeboxBlendLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(jukeboxBlendLabel_);

    jukeboxBlendSlider_.setRange(0.5, 5.0, 0.1);
    jukeboxBlendSlider_.setValue(2.0, juce::dontSendNotification);
    jukeboxBlendSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    jukeboxBlendSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 16);
    jukeboxBlendSlider_.setColour(juce::Slider::thumbColourId, juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    jukeboxBlendSlider_.setColour(juce::Slider::trackColourId, juce::Colour(0xff444466));
    jukeboxBlendSlider_.setColour(juce::Slider::textBoxTextColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    jukeboxBlendSlider_.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff2a2a3e));
    jukeboxBlendSlider_.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    jukeboxBlendSlider_.setTextValueSuffix("s");
    addAndMakeVisible(jukeboxBlendSlider_);

    // === Playlist controls ===
    playlistInfoLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff8888aa));
    playlistInfoLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(playlistInfoLabel_);

    playlistCycleSelector_.addItem("Bag", 1);
    playlistCycleSelector_.addItem("Random", 2);
    playlistCycleSelector_.addItem("Sequential", 3);
    playlistCycleSelector_.setSelectedId(1, juce::dontSendNotification);
    setupCombo(playlistCycleSelector_);
    addAndMakeVisible(playlistCycleSelector_);

    playlistTimingSelector_.addItem("4 beats", 1);
    playlistTimingSelector_.addItem("8 beats", 2);
    playlistTimingSelector_.addItem("16 beats", 3);
    playlistTimingSelector_.addItem("32 beats", 4);
    playlistTimingSelector_.setSelectedId(2, juce::dontSendNotification);
    setupCombo(playlistTimingSelector_);
    addAndMakeVisible(playlistTimingSelector_);

    playlistBlendLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    playlistBlendLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(playlistBlendLabel_);

    playlistBlendSlider_.setRange(0.3, 3.0, 0.1);
    playlistBlendSlider_.setValue(1.5, juce::dontSendNotification);
    playlistBlendSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    playlistBlendSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 16);
    playlistBlendSlider_.setColour(juce::Slider::thumbColourId, juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    playlistBlendSlider_.setColour(juce::Slider::trackColourId, juce::Colour(0xff444466));
    playlistBlendSlider_.setColour(juce::Slider::textBoxTextColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    playlistBlendSlider_.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff2a2a3e));
    playlistBlendSlider_.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    playlistBlendSlider_.setTextValueSuffix("s");
    addAndMakeVisible(playlistBlendSlider_);

    initSections();
    updateSubTabColors();
    updateModeButtonColors();
}

void MilkDropBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Active sub-tab accent line
    juce::TextButton* activeBtn = nullptr;
    switch (activeSubTab_)
    {
        case SubTab::Curated:   activeBtn = &curatedBtn_; break;
        case SubTab::Favorites: activeBtn = &favoritesBtn_; break;
        case SubTab::Recent:    activeBtn = &recentBtn_; break;
        case SubTab::All:       activeBtn = &allBtn_; break;
    }
    if (activeBtn)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        auto b = activeBtn->getBounds();
        g.fillRect(b.getX(), b.getBottom() - 2, b.getWidth(), 2);
    }

    // Mode bar separator
    g.setColour(juce::Colour(0xff333344));
    int modeBarY = getHeight() - kBottomControlsHeight - kModeBarHeight;
    g.drawHorizontalLine(modeBarY, 0.0f, static_cast<float>(getWidth()));

    // Active mode accent
    juce::TextButton* activeModeBtn = nullptr;
    switch (activePlayMode_)
    {
        case PlayMode::Jukebox:  activeModeBtn = &jukeboxModeBtn_; break;
        case PlayMode::VJClip:   activeModeBtn = &vjModeBtn_; break;
        case PlayMode::Playlist: activeModeBtn = &playlistModeBtn_; break;
    }
    if (activeModeBtn)
    {
        g.setColour(juce::Colour(0xff00ccaa));
        auto mb = activeModeBtn->getBounds();
        g.fillRect(mb.getX(), mb.getBottom() - 2, mb.getWidth(), 2);
    }
}

void MilkDropBrowser::resized()
{
    auto area = getLocalBounds();

    // Sub-tab bar
    auto tabBar = area.removeFromTop(kSubTabHeight);
    int tabW = tabBar.getWidth() / 4;
    curatedBtn_.setBounds(tabBar.removeFromLeft(tabW));
    favoritesBtn_.setBounds(tabBar.removeFromLeft(tabW));
    recentBtn_.setBounds(tabBar.removeFromLeft(tabW));
    allBtn_.setBounds(tabBar);

    // Search bar (only in All tab)
    searchBox_.setVisible(activeSubTab_ == SubTab::All);
    if (activeSubTab_ == SubTab::All)
        searchBox_.setBounds(area.removeFromTop(kSearchBarHeight).reduced(3, 2));

    // Navigation bar
    auto navBar = area.removeFromTop(kNavBarHeight);
    int navBtnW = navBar.getWidth() / 4;
    prevBtn_.setBounds(navBar.removeFromLeft(navBtnW));
    nextBtn_.setBounds(navBar.removeFromLeft(navBtnW));
    randomBtn_.setBounds(navBar.removeFromLeft(navBtnW));
    lockBtn_.setBounds(navBar);

    // Bottom: mode bar + controls
    auto bottomBlock = area.removeFromBottom(kModeBarHeight + kBottomControlsHeight);
    auto modeBar = bottomBlock.removeFromTop(kModeBarHeight);
    int modeBtnW = modeBar.getWidth() / 3;
    jukeboxModeBtn_.setBounds(modeBar.removeFromLeft(modeBtnW));
    vjModeBtn_.setBounds(modeBar.removeFromLeft(modeBtnW));
    playlistModeBtn_.setBounds(modeBar);

    auto controls = bottomBlock;

    // Hide all mode controls first
    jukeboxPlayBtn_.setVisible(false);
    jukeboxPoolSelector_.setVisible(false);
    jukeboxModeSelector_.setVisible(false);
    jukeboxTimingSelector_.setVisible(false);
    jukeboxBlendLabel_.setVisible(false);
    jukeboxBlendSlider_.setVisible(false);
    playlistInfoLabel_.setVisible(false);
    playlistCycleSelector_.setVisible(false);
    playlistTimingSelector_.setVisible(false);
    playlistBlendLabel_.setVisible(false);
    playlistBlendSlider_.setVisible(false);

    switch (activePlayMode_)
    {
        case PlayMode::Jukebox:
        {
            auto row1 = controls.removeFromTop(controls.getHeight() / 2);
            jukeboxPlayBtn_.setVisible(true);
            jukeboxPlayBtn_.setBounds(row1.removeFromLeft(40).reduced(2));
            jukeboxPoolSelector_.setVisible(true);
            jukeboxPoolSelector_.setBounds(row1.removeFromLeft(row1.getWidth() / 2).reduced(2));
            jukeboxModeSelector_.setVisible(true);
            jukeboxModeSelector_.setBounds(row1.reduced(2));

            auto row2 = controls;
            jukeboxTimingSelector_.setVisible(true);
            jukeboxTimingSelector_.setBounds(row2.removeFromLeft(row2.getWidth() / 3).reduced(2));
            jukeboxBlendLabel_.setVisible(true);
            jukeboxBlendLabel_.setBounds(row2.removeFromLeft(34));
            jukeboxBlendSlider_.setVisible(true);
            jukeboxBlendSlider_.setBounds(row2.reduced(2));
            break;
        }
        case PlayMode::VJClip:
        {
            // VJ mode: just a label explaining how to use
            // (the preset list itself IS the interface — click or drag)
            break;
        }
        case PlayMode::Playlist:
        {
            auto row1 = controls.removeFromTop(controls.getHeight() / 2);
            playlistInfoLabel_.setVisible(true);
            int selCount = static_cast<int>(selectedIndices_.size());
            playlistInfoLabel_.setText(
                selCount > 0 ? juce::String(selCount) + " selected - drag to cell"
                             : "Shift/Cmd+click to select, drag to cell",
                juce::dontSendNotification);
            playlistInfoLabel_.setBounds(row1.reduced(4, 2));

            auto row2 = controls;
            playlistCycleSelector_.setVisible(true);
            playlistCycleSelector_.setBounds(row2.removeFromLeft(row2.getWidth() / 3).reduced(2));
            playlistTimingSelector_.setVisible(true);
            playlistTimingSelector_.setBounds(row2.removeFromLeft(row2.getWidth() / 2).reduced(2));
            playlistBlendLabel_.setVisible(true);
            playlistBlendLabel_.setBounds(row2.removeFromLeft(34));
            playlistBlendSlider_.setVisible(true);
            playlistBlendSlider_.setBounds(row2.reduced(2));
            break;
        }
    }

    // List viewport fills remaining space
    listViewport_.setBounds(area);
    int contentH = calculateContentHeight();
    listContent_->setSize(area.getWidth() - (contentH > area.getHeight() ? 8 : 0),
                          std::max(contentH, area.getHeight()));
}

void MilkDropBrowser::setPresetManager(ProjectMPresetManager* mgr)
{
    presetManager_ = mgr;
    refresh();
}

void MilkDropBrowser::setPresetSelector(PresetSelector* sel)
{
    presetSelector_ = sel;
}

void MilkDropBrowser::refresh()
{
    listContent_->repaint();
    resized();
}

void MilkDropBrowser::addToRecent(const std::string& presetPath)
{
    auto it = std::find(recentPresets_.begin(), recentPresets_.end(), presetPath);
    if (it != recentPresets_.end())
        recentPresets_.erase(it);
    recentPresets_.push_front(presetPath);
    while (static_cast<int>(recentPresets_.size()) > kMaxRecent)
        recentPresets_.pop_back();
    if (activeSubTab_ == SubTab::Recent)
        refresh();
}

void MilkDropBrowser::initSections()
{
    sections_ = {
        {"Energetic",    true,  juce::Colour(0xffff4444)},
        {"Psychedelic",  true,  juce::Colour(0xffcc44ff)},
        {"Geometric",    true,  juce::Colour(0xff44cccc)},
        {"Calm",         true,  juce::Colour(0xff4488ff)},
        {"Dark",         true,  juce::Colour(0xff666688)},
        {"Minimal",      true,  juce::Colour(0xffaaaaaa)},
    };
}

void MilkDropBrowser::setSubTab(SubTab tab)
{
    activeSubTab_ = tab;
    updateSubTabColors();
    refresh();
}

void MilkDropBrowser::setPlayMode(PlayMode mode)
{
    activePlayMode_ = mode;
    if (mode != PlayMode::Playlist)
        selectedIndices_.clear(); // Clear multi-select when leaving playlist mode
    updateModeButtonColors();
    resized();
    listContent_->repaint();
}

void MilkDropBrowser::updateSubTabColors()
{
    auto setColor = [this](juce::TextButton& btn, SubTab tab) {
        bool active = (activeSubTab_ == tab);
        btn.setColour(juce::TextButton::buttonColourId,
                      juce::Colour(active ? 0xff3a3a5c : 0xff1a1a2e));
        btn.setColour(juce::TextButton::textColourOffId,
                      juce::Colour(active ? 0xffffffff : 0xff606070));
    };
    setColor(curatedBtn_, SubTab::Curated);
    setColor(favoritesBtn_, SubTab::Favorites);
    setColor(recentBtn_, SubTab::Recent);
    setColor(allBtn_, SubTab::All);
}

void MilkDropBrowser::updateModeButtonColors()
{
    auto setColor = [this](juce::TextButton& btn, PlayMode mode) {
        bool active = (activePlayMode_ == mode);
        btn.setColour(juce::TextButton::buttonColourId,
                      juce::Colour(active ? 0xff2a3a4a : 0xff1a1a2e));
        btn.setColour(juce::TextButton::textColourOffId,
                      juce::Colour(active ? 0xff00ccaa : 0xff606070));
    };
    setColor(jukeboxModeBtn_, PlayMode::Jukebox);
    setColor(vjModeBtn_, PlayMode::VJClip);
    setColor(playlistModeBtn_, PlayMode::Playlist);
}

void MilkDropBrowser::updateSearch()
{
    searchFilter_ = searchBox_.getText().toStdString();
    listContent_->repaint();
    resized();
}

void MilkDropBrowser::handlePresetClick(int globalIndex, bool shiftHeld, bool ctrlHeld)
{
    if (isMultiSelectMode())
    {
        // Multi-select logic
        if (ctrlHeld || shiftHeld)
        {
            if (selectedIndices_.count(globalIndex))
                selectedIndices_.erase(globalIndex);
            else
                selectedIndices_.insert(globalIndex);
        }
        else
        {
            selectedIndices_.clear();
            selectedIndices_.insert(globalIndex);
        }
        lastClickedIndex_ = globalIndex;
        resized(); // Update playlist info label count
        listContent_->repaint();

        // Also preview it
        selectPreset(globalIndex);
    }
    else
    {
        // Single select: load the preset
        lastClickedIndex_ = globalIndex;
        selectedIndices_.clear();
        selectPreset(globalIndex);
    }
}

void MilkDropBrowser::selectPreset(int globalIndex)
{
    if (!presetManager_) return;

    presetManager_->setCurrentIndex(globalIndex);

    auto* preset = presetManager_->getPreset(globalIndex);
    if (preset)
        firePresetSelected(preset->path);

    listContent_->repaint();
}

void MilkDropBrowser::firePresetSelected(const std::string& path)
{
    addToRecent(path);
    if (onPresetSelected)
        onPresetSelected(path);
}

void MilkDropBrowser::toggleJukeboxPlay()
{
    jukeboxPlaying_ = !jukeboxPlaying_;
    jukeboxPlayBtn_.setButtonText(jukeboxPlaying_ ? "Stop" : "Play");
    jukeboxPlayBtn_.setColour(juce::TextButton::buttonColourId,
                               juce::Colour(jukeboxPlaying_ ? 0xff5a2a2a : 0xff2a5a2a));

    if (presetSelector_)
    {
        presetSelector_->setEnabled(jukeboxPlaying_);

        if (jukeboxPlaying_)
        {
            // Start with a random preset if none is playing
            if (presetManager_ && presetManager_->getPresetCount() > 0)
            {
                auto* p = presetManager_->randomPreset();
                if (p)
                {
                    lastClickedIndex_ = presetManager_->getCurrentIndex();
                    firePresetSelected(p->path);
                }
            }
        }
    }
}

std::vector<std::string> MilkDropBrowser::getSelectedPresetPaths() const
{
    std::vector<std::string> paths;
    if (!presetManager_) return paths;
    for (int idx : selectedIndices_)
    {
        auto* p = presetManager_->getPreset(idx);
        if (p)
            paths.push_back(p->path);
    }
    return paths;
}

std::string MilkDropBrowser::parseDragDescription(const juce::String& desc)
{
    if (desc.startsWith("milkdrop:"))
        return desc.substring(9).toStdString();
    return "";
}

std::vector<std::string> MilkDropBrowser::parsePlaylistDragDescription(const juce::String& desc)
{
    std::vector<std::string> paths;
    if (!desc.startsWith("milkdrop_playlist:"))
        return paths;
    auto pathList = desc.substring(18);
    auto tokens = juce::StringArray::fromTokens(pathList, "|", "");
    for (const auto& t : tokens)
    {
        if (t.isNotEmpty())
            paths.push_back(t.toStdString());
    }
    return paths;
}

juce::String MilkDropBrowser::buildPlaylistDragDescription(const std::vector<std::string>& paths)
{
    juce::String desc = "milkdrop_playlist:";
    bool first = true;
    for (const auto& path : paths)
    {
        if (!first) desc += "|";
        desc += juce::String(path);
        first = false;
    }
    return desc;
}

std::vector<const ProjectMPresetManager::PresetInfo*>
MilkDropBrowser::getPresetsForSection(const std::string& sectionName) const
{
    if (!presetManager_ || presetManager_->getPresetCount() == 0) return {};
    auto byMood = presetManager_->getPresetsByMood(sectionName);
    if (searchFilter_.empty()) return byMood;

    std::string lower = searchFilter_;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    std::vector<const ProjectMPresetManager::PresetInfo*> filtered;
    for (auto* p : byMood)
    {
        std::string lowerName = p->name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        if (lowerName.find(lower) != std::string::npos)
            filtered.push_back(p);
    }
    return filtered;
}

std::vector<const ProjectMPresetManager::PresetInfo*>
MilkDropBrowser::getCuratedPresets() const
{
    if (!presetManager_ || presetManager_->getPresetCount() == 0) return {};
    std::vector<const ProjectMPresetManager::PresetInfo*> result;
    for (const auto& p : presetManager_->getAllPresets())
    {
        if (p.energy > 0.1f)
            result.push_back(&p);
    }
    return result;
}

int MilkDropBrowser::calculateContentHeight() const
{
    if (!presetManager_ || presetManager_->getPresetCount() == 0)
        return 200;

    int h = 0;
    auto countGrouped = [&](bool curatedOnly) {
        for (const auto& section : sections_)
        {
            std::vector<const ProjectMPresetManager::PresetInfo*> presets;
            if (curatedOnly)
            {
                auto curated = getCuratedPresets();
                for (auto* p : curated)
                    if (p->mood == section.name) presets.push_back(p);
            }
            else
                presets = getPresetsForSection(section.name);

            if (presets.empty()) continue;
            h += kSectionHeaderHeight;
            if (section.expanded)
                h += static_cast<int>(presets.size()) * kPresetRowHeight;
        }
    };

    switch (activeSubTab_)
    {
        case SubTab::Curated:   countGrouped(true); break;
        case SubTab::Favorites: h = static_cast<int>(presetManager_->getFavorites().size()) * kPresetRowHeight; break;
        case SubTab::Recent:    h = static_cast<int>(recentPresets_.size()) * kPresetRowHeight; break;
        case SubTab::All:       countGrouped(false); break;
    }

    return std::max(h, 100);
}
