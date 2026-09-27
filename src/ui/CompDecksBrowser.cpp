#include "ui/CompDecksBrowser.h"

class CompDecksBrowser::CompDeckListContent : public juce::Component
{
public:
    CompDeckListContent(CompDecksBrowser& owner) : owner_(owner) {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1a));
        int y = 0;

        // Compositions section
        y = paintSection(g, y, "Compositions", owner_.compositionsExpanded_,
                         owner_.compositions_, juce::Colour(AudioDNALookAndFeel::kAccentCyan));

        // Decks section
        y = paintSection(g, y, "Decks", owner_.decksExpanded_,
                         owner_.decks_, juce::Colour(AudioDNALookAndFeel::kAccentMagenta));

        // Empty state
        if (owner_.compositions_.empty() && owner_.decks_.empty())
        {
            g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.5f));
            g.setFont(juce::Font(juce::FontOptions(11.0f)));
            g.drawText("No saved compositions or decks",
                       0, y + 20, getWidth(), 20, juce::Justification::centred, false);
        }
    }

    int paintSection(juce::Graphics& g, int y, const juce::String& title, bool expanded,
                     const std::vector<SavedEntry>& entries, juce::Colour accent)
    {
        // Section header
        g.setColour(juce::Colour(0xff222233));
        g.fillRect(0, y, getWidth(), kSectionHeaderHeight);
        g.setColour(accent);
        g.fillRect(0, y, 3, kSectionHeaderHeight);

        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(expanded ? "v" : ">", 6, y, 12, kSectionHeaderHeight,
                   juce::Justification::centredLeft, false);
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        g.drawText(title + " (" + juce::String(static_cast<int>(entries.size())) + ")",
                   20, y, getWidth() - 24, kSectionHeaderHeight,
                   juce::Justification::centredLeft, false);
        y += kSectionHeaderHeight;

        if (expanded)
        {
            for (size_t i = 0; i < entries.size(); ++i)
            {
                auto& e = entries[i];

                g.setColour(i % 2 == 0 ? juce::Colour(0xff1e1e2e) : juce::Colour(0xff1a1a2a));
                g.fillRect(0, y, getWidth(), kEntryRowHeight);

                // Name
                g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
                g.setFont(juce::Font(juce::FontOptions(11.0f)));
                g.drawText(e.name, 10, y, getWidth() / 2, kEntryRowHeight,
                           juce::Justification::centredLeft, true);

                // Date
                g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
                g.setFont(juce::Font(juce::FontOptions(9.0f)));
                g.drawText(e.dateStr, getWidth() / 2, y, getWidth() / 2 - 10, kEntryRowHeight,
                           juce::Justification::centredRight, false);

                y += kEntryRowHeight;
            }
        }

        return y;
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        int y = 0;

        // Compositions header
        if (event.y >= y && event.y < y + kSectionHeaderHeight)
        {
            owner_.compositionsExpanded_ = !owner_.compositionsExpanded_;
            updateSize();
            repaint();
            return;
        }
        y += kSectionHeaderHeight;

        if (owner_.compositionsExpanded_)
        {
            for (size_t i = 0; i < owner_.compositions_.size(); ++i)
            {
                if (event.y >= y && event.y < y + kEntryRowHeight)
                {
                    if (event.mods.isRightButtonDown())
                    {
                        // Right-click: the row menu (Open / Show in Finder / Delete...) -- plan6 §8.
                        owner_.showRowMenu(false, static_cast<int>(i), event.getScreenPosition());
                    }
                    else if (owner_.onCompositionLoad)
                    {
                        owner_.onCompositionLoad(owner_.compositions_[i].file);
                    }
                    return;
                }
                y += kEntryRowHeight;
            }
        }

        // Decks header
        if (event.y >= y && event.y < y + kSectionHeaderHeight)
        {
            owner_.decksExpanded_ = !owner_.decksExpanded_;
            updateSize();
            repaint();
            return;
        }
        y += kSectionHeaderHeight;

        if (owner_.decksExpanded_)
        {
            for (size_t i = 0; i < owner_.decks_.size(); ++i)
            {
                if (event.y >= y && event.y < y + kEntryRowHeight)
                {
                    if (event.mods.isRightButtonDown())
                    {
                        owner_.showRowMenu(true, static_cast<int>(i), event.getScreenPosition());
                    }
                    else if (owner_.onDeckLoad)
                    {
                        owner_.onDeckLoad(owner_.decks_[i].file);
                    }
                    return;
                }
                y += kEntryRowHeight;
            }
        }
    }

    int getRequiredHeight() const
    {
        int h = kSectionHeaderHeight; // compositions header
        if (owner_.compositionsExpanded_)
            h += static_cast<int>(owner_.compositions_.size()) * kEntryRowHeight;
        h += kSectionHeaderHeight; // decks header
        if (owner_.decksExpanded_)
            h += static_cast<int>(owner_.decks_.size()) * kEntryRowHeight;
        return std::max(h, 100);
    }

    void updateSize()
    {
        setSize(std::max(1, owner_.viewport_.getWidth() - 8), getRequiredHeight());
    }

private:
    static constexpr int kSectionHeaderHeight = 22;
    static constexpr int kEntryRowHeight = 24;
    CompDecksBrowser& owner_;
};

// ── CompDecksBrowser implementation ──

CompDecksBrowser::~CompDecksBrowser() = default;

CompDecksBrowser::CompDecksBrowser()
{
    listContent_ = std::make_unique<CompDeckListContent>(*this);
    viewport_.setViewedComponent(listContent_.get(), false);
    viewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport_);

    addAndMakeVisible(saveCompBtn_);

    saveCompBtn_.onClick = [this] {
        if (onCompositionSave) onCompositionSave();
    };

    scanForFiles();
}

void CompDecksBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void CompDecksBrowser::resized()
{
    auto area = getLocalBounds();

    // Button bar
    auto btnBar = area.removeFromTop(kButtonBarHeight);
    saveCompBtn_.setBounds(btnBar.reduced(1));

    area.removeFromTop(1);
    viewport_.setBounds(area);
    if (listContent_)
        listContent_->updateSize();
}

void CompDecksBrowser::refresh()
{
    scanForFiles();
    if (listContent_)
    {
        listContent_->updateSize();
        listContent_->repaint();
    }
}

void CompDecksBrowser::showRowMenu(bool decksSection, int row, juce::Point<int> screenPos)
{
    const auto& entries = decksSection ? decks_ : compositions_;
    if (row < 0 || row >= static_cast<int>(entries.size()))
        return;
    const juce::File file = entries[static_cast<size_t>(row)].file;

    juce::PopupMenu menu;
    menu.addItem(1, decksSection ? "Open as New Deck" : "Open");
    menu.addItem(2, "Show in Finder");
    menu.addSeparator();
    menu.addItem(3, "Delete...");
    menu.showMenuAsync(juce::PopupMenu::Options()
                           .withParentComponent(getTopLevelComponent())
                           .withTargetScreenArea({ screenPos.x, screenPos.y, 1, 1 }),
                       [this, decksSection, row, file](int result) {
                           // Re-resolve: the list can refresh while the menu is open -- act only if the row
                           // still names the same file.
                           const auto& list = decksSection ? decks_ : compositions_;
                           if (result <= 0 || row >= static_cast<int>(list.size())
                               || list[static_cast<size_t>(row)].file != file)
                               return;
                           if (result == 1)
                           {
                               // The same callback the left-click uses.
                               if (decksSection) { if (onDeckLoad) onDeckLoad(file); }
                               else if (onCompositionLoad) onCompositionLoad(file);
                           }
                           else if (result == 2)
                               file.revealToUser();
                           else if (result == 3)
                               confirmDelete(decksSection, row);
                       });
}

void CompDecksBrowser::confirmDelete(bool decksSection, int row)
{
    const auto& entries = decksSection ? decks_ : compositions_;
    if (row < 0 || row >= static_cast<int>(entries.size()))
        return;
    const juce::File file = entries[static_cast<size_t>(row)].file;
    const juce::String name = entries[static_cast<size_t>(row)].name;

    // NoIcon + associatedComponent = this: the dialog is created by -- and draws with -- the app LookAndFeel.
    juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::NoIcon, "Delete from Library",
        "Delete \"" + name + "\" from the library?\n\nThe file will be moved to the Trash.",
        "Delete", "Cancel", this,
        juce::ModalCallbackFunction::create([this, file](int result) {
            if (result == 1)
            {
                file.moveToTrash();
                refresh();
            }
        }));
}

void CompDecksBrowser::scanForFiles()
{
    compositions_.clear();
    decks_.clear();

    // Compositions
    auto compDir = getCompositionsDir();
    if (compDir.isDirectory())
    {
        auto files = compDir.findChildFiles(juce::File::findFiles, false, "*.json");
        files.sort();
        for (auto& f : files)
        {
            SavedEntry entry;
            entry.file = f;
            entry.name = f.getFileNameWithoutExtension();
            entry.dateStr = f.getLastModificationTime().toString(true, true, false, true);
            compositions_.push_back(std::move(entry));
        }
    }

    // Decks
    auto deckDir = getDecksDir();
    if (deckDir.isDirectory())
    {
        auto files = deckDir.findChildFiles(juce::File::findFiles, false, "*.json");
        files.sort();
        for (auto& f : files)
        {
            SavedEntry entry;
            entry.file = f;
            entry.name = f.getFileNameWithoutExtension();
            entry.dateStr = f.getLastModificationTime().toString(true, true, false, true);
            decks_.push_back(std::move(entry));
        }
    }
}

juce::File CompDecksBrowser::getCompositionsDir()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("AudioDNA").getChildFile("compositions");
}

juce::File CompDecksBrowser::getDecksDir()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("AudioDNA").getChildFile("decks");
}
