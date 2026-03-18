#include "ui/FXBrowser.h"

// ── FXListContent: scrollable list of categories and effects ──
class FXBrowser::FXListContent : public juce::Component
{
public:
    FXListContent(FXBrowser& owner) : owner_(owner) {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1a));

        if (owner_.categories_.empty())
        {
            g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.5f));
            g.setFont(juce::Font(juce::FontOptions(11.0f)));
            g.drawText("No effects loaded", getLocalBounds(), juce::Justification::centred, false);
            return;
        }

        auto searchQuery = owner_.searchField_.getText().toLowerCase();
        int y = 0;

        for (int ci = 0; ci < static_cast<int>(owner_.categories_.size()); ++ci)
        {
            auto& cat = owner_.categories_[static_cast<size_t>(ci)];

            // Collect effects in this category
            std::vector<const EffectEntry*> catEffects;
            for (auto& e : owner_.effects_)
            {
                if (e.categoryIndex != ci) continue;
                if (!searchQuery.isEmpty() && !e.name.toLowerCase().contains(searchQuery))
                    continue;
                catEffects.push_back(&e);
            }

            // Skip empty categories when searching
            if (!searchQuery.isEmpty() && catEffects.empty())
                continue;

            // Category header
            auto headerRect = juce::Rectangle<int>(0, y, getWidth(), kCategoryHeaderHeight);
            g.setColour(juce::Colour(0xff222233));
            g.fillRect(headerRect);
            g.setColour(cat.color);
            g.fillRect(0, y, 3, kCategoryHeaderHeight);

            // Expand/collapse arrow
            g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
            g.setFont(juce::Font(juce::FontOptions(10.0f)));
            g.drawText(cat.expanded ? "v" : ">", 6, y, 12, kCategoryHeaderHeight,
                       juce::Justification::centredLeft, false);

            // Category name + count
            g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
            g.drawText(cat.displayName + " (" + juce::String(static_cast<int>(catEffects.size())) + ")",
                       20, y, getWidth() - 24, kCategoryHeaderHeight,
                       juce::Justification::centredLeft, false);

            y += kCategoryHeaderHeight;

            // Effects (if expanded)
            if (cat.expanded || !searchQuery.isEmpty())
            {
                for (auto* e : catEffects)
                {
                    auto rowRect = juce::Rectangle<int>(0, y, getWidth(), kEffectRowHeight);

                    // Subtle hover-like alternating bg
                    g.setColour(juce::Colour(0xff1e1e2e));
                    g.fillRect(rowRect);

                    // Color dot
                    g.setColour(cat.color.withAlpha(0.6f));
                    g.fillEllipse(10.0f, static_cast<float>(y) + 7.0f, 8.0f, 8.0f);

                    // Effect name
                    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
                    g.setFont(juce::Font(juce::FontOptions(11.0f)));
                    g.drawText(e->name, 24, y, getWidth() - 28, kEffectRowHeight,
                               juce::Justification::centredLeft, false);

                    y += kEffectRowHeight;
                }
            }
        }
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        auto pos = event.getPosition();
        auto searchQuery = owner_.searchField_.getText().toLowerCase();
        int y = 0;

        for (int ci = 0; ci < static_cast<int>(owner_.categories_.size()); ++ci)
        {
            auto& cat = owner_.categories_[static_cast<size_t>(ci)];

            std::vector<const EffectEntry*> catEffects;
            for (auto& e : owner_.effects_)
            {
                if (e.categoryIndex != ci) continue;
                if (!searchQuery.isEmpty() && !e.name.toLowerCase().contains(searchQuery))
                    continue;
                catEffects.push_back(&e);
            }

            if (!searchQuery.isEmpty() && catEffects.empty())
                continue;

            // Category header hit
            if (pos.y >= y && pos.y < y + kCategoryHeaderHeight)
            {
                owner_.toggleCategory(ci);
                return;
            }
            y += kCategoryHeaderHeight;

            if (cat.expanded || !searchQuery.isEmpty())
            {
                for (auto* e : catEffects)
                {
                    if (pos.y >= y && pos.y < y + kEffectRowHeight)
                    {
                        if (owner_.onEffectActivated)
                            owner_.onEffectActivated(e->name);
                        return;
                    }
                    y += kEffectRowHeight;
                }
            }
        }
    }

    int getRequiredHeight() const
    {
        auto searchQuery = owner_.searchField_.getText().toLowerCase();
        int h = 0;

        for (int ci = 0; ci < static_cast<int>(owner_.categories_.size()); ++ci)
        {
            auto& cat = owner_.categories_[static_cast<size_t>(ci)];

            int count = 0;
            for (auto& e : owner_.effects_)
            {
                if (e.categoryIndex != ci) continue;
                if (!searchQuery.isEmpty() && !e.name.toLowerCase().contains(searchQuery))
                    continue;
                ++count;
            }

            if (!searchQuery.isEmpty() && count == 0)
                continue;

            h += kCategoryHeaderHeight;
            if (cat.expanded || !searchQuery.isEmpty())
                h += count * kEffectRowHeight;
        }

        return std::max(h, 100);
    }

    void updateSize()
    {
        setSize(std::max(1, owner_.viewport_.getWidth() - 8), getRequiredHeight());
    }

private:
    static constexpr int kCategoryHeaderHeight = 22;
    static constexpr int kEffectRowHeight = 24;

    FXBrowser& owner_;
};

// ── FXBrowser implementation ──

FXBrowser::~FXBrowser() = default;

FXBrowser::FXBrowser()
{
    listContent_ = std::make_unique<FXListContent>(*this);
    viewport_.setViewedComponent(listContent_.get(), false);
    viewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport_);

    addAndMakeVisible(searchField_);
    searchField_.setFont(juce::Font(juce::FontOptions(11.0f)));
    searchField_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(AudioDNALookAndFeel::kSurface));
    searchField_.setColour(juce::TextEditor::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    searchField_.setColour(juce::TextEditor::outlineColourId, juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    searchField_.setTextToShowWhenEmpty("Search effects...", juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    searchField_.onTextChange = [this] {
        if (listContent_)
        {
            listContent_->updateSize();
            listContent_->repaint();
        }
    };

    buildCategoryList();
}

void FXBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void FXBrowser::resized()
{
    auto area = getLocalBounds();

    searchField_.setBounds(area.removeFromTop(kSearchBarHeight));
    area.removeFromTop(1);

    viewport_.setBounds(area);
    if (listContent_)
        listContent_->updateSize();
}

void FXBrowser::setEffectLibrary(EffectLibrary* lib)
{
    effectLibrary_ = lib;
    refresh();
}

void FXBrowser::refresh()
{
    effects_.clear();
    if (!effectLibrary_) return;

    auto names = effectLibrary_->getEffectNames();
    for (auto& name : names)
    {
        auto* def = effectLibrary_->getEffectDef(name);
        if (!def) continue;

        EffectEntry entry;
        entry.name = def->name;
        entry.category = def->category;

        // Find category index
        for (int ci = 0; ci < static_cast<int>(categories_.size()); ++ci)
        {
            if (categories_[static_cast<size_t>(ci)].name == def->category)
            {
                entry.categoryIndex = ci;
                break;
            }
        }

        effects_.push_back(std::move(entry));
    }

    if (listContent_)
    {
        listContent_->updateSize();
        listContent_->repaint();
    }
}

void FXBrowser::buildCategoryList()
{
    categories_.clear();
    // 8 categories matching CLAUDE.md + EffectLibrary
    categories_.push_back({"warp",      "Warp",         juce::Colour(0xff4fc3f7)});
    categories_.push_back({"color",     "Color",        juce::Colour(0xffff7043)});
    categories_.push_back({"glitch",    "Glitch",       juce::Colour(0xffab47bc)});
    categories_.push_back({"blur",      "Blur / Post",  juce::Colour(0xff66bb6a)});
    categories_.push_back({"3d",        "3D / Depth",   juce::Colour(0xffffca28)});
    categories_.push_back({"pattern",   "Pattern",      juce::Colour(0xff26c6da)});
    categories_.push_back({"animation", "Animation",    juce::Colour(0xffef5350)});
    categories_.push_back({"blend",     "Blend",        juce::Colour(0xff8d6e63)});
}

void FXBrowser::toggleCategory(int catIndex)
{
    if (catIndex >= 0 && catIndex < static_cast<int>(categories_.size()))
    {
        categories_[static_cast<size_t>(catIndex)].expanded =
            !categories_[static_cast<size_t>(catIndex)].expanded;

        if (listContent_)
        {
            listContent_->updateSize();
            listContent_->repaint();
        }
    }
}
