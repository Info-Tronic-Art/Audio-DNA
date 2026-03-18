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
                        // Right-click: delete
                        auto& entry = owner_.compositions_[i];
                        entry.file.deleteFile();
                        owner_.refresh();
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
                        auto& entry = owner_.decks_[i];
                        entry.file.deleteFile();
                        owner_.refresh();
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
    addAndMakeVisible(saveDeckBtn_);

    saveCompBtn_.onClick = [this] {
        if (onCompositionSave) onCompositionSave();
    };

    saveDeckBtn_.onClick = [this] {
        // Save current deck as a standalone deck file
        if (!composition_) return;
        auto* deck = composition_->getActiveDeck();
        if (!deck) return;

        auto dir = getDecksDir();
        dir.createDirectory();
        auto file = dir.getChildFile(juce::String(deck->name) + ".json");
        auto json = juce::JSON::toString(deck->toVar());
        file.replaceWithText(json);
        refresh();
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
    int half = btnBar.getWidth() / 2;
    saveCompBtn_.setBounds(btnBar.removeFromLeft(half).reduced(1));
    saveDeckBtn_.setBounds(btnBar.reduced(1));

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
