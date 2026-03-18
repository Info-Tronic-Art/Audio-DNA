#include "ui/SourcesBrowser.h"

class SourcesBrowser::SourceListContent : public juce::Component
{
public:
    SourceListContent(SourcesBrowser& owner) : owner_(owner) {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1a));
        int y = 0;

        for (int ci = 0; ci < static_cast<int>(owner_.categories_.size()); ++ci)
        {
            auto& cat = owner_.categories_[static_cast<size_t>(ci)];

            // Collect sources in category
            std::vector<const SourceEntry*> catSources;
            for (auto& s : owner_.sources_)
            {
                if (s.category == cat.name)
                    catSources.push_back(&s);
            }
            if (catSources.empty()) continue;

            // Category header
            g.setColour(juce::Colour(0xff222233));
            g.fillRect(0, y, getWidth(), kCategoryHeaderHeight);
            g.setColour(cat.color);
            g.fillRect(0, y, 3, kCategoryHeaderHeight);

            g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
            g.setFont(juce::Font(juce::FontOptions(10.0f)));
            g.drawText(cat.expanded ? "v" : ">", 6, y, 12, kCategoryHeaderHeight,
                       juce::Justification::centredLeft, false);
            g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
            g.drawText(cat.name + " (" + juce::String(static_cast<int>(catSources.size())) + ")",
                       20, y, getWidth() - 24, kCategoryHeaderHeight,
                       juce::Justification::centredLeft, false);
            y += kCategoryHeaderHeight;

            if (cat.expanded)
            {
                for (auto* s : catSources)
                {
                    // Row background
                    g.setColour(juce::Colour(0xff1e1e2e));
                    g.fillRect(0, y, getWidth(), kSourceRowHeight);

                    // Color indicator
                    g.setColour(s->color.withAlpha(0.7f));
                    g.fillRoundedRectangle(10.0f, static_cast<float>(y) + 6.0f,
                                           16.0f, 16.0f, 3.0f);

                    // Source name
                    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
                    g.setFont(juce::Font(juce::FontOptions(11.0f)));
                    g.drawText(s->name, 32, y, getWidth() - 36, kSourceRowHeight,
                               juce::Justification::centredLeft, false);

                    y += kSourceRowHeight;
                }
            }
        }
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        int y = 0;
        for (int ci = 0; ci < static_cast<int>(owner_.categories_.size()); ++ci)
        {
            auto& cat = owner_.categories_[static_cast<size_t>(ci)];

            std::vector<const SourceEntry*> catSources;
            for (auto& s : owner_.sources_)
                if (s.category == cat.name) catSources.push_back(&s);
            if (catSources.empty()) continue;

            if (event.y >= y && event.y < y + kCategoryHeaderHeight)
            {
                owner_.toggleCategory(ci);
                return;
            }
            y += kCategoryHeaderHeight;

            if (cat.expanded)
            {
                for (auto* s : catSources)
                {
                    if (event.y >= y && event.y < y + kSourceRowHeight)
                    {
                        if (owner_.onSourceActivated)
                            owner_.onSourceActivated(s->name);
                        return;
                    }
                    y += kSourceRowHeight;
                }
            }
        }
    }

    int getRequiredHeight() const
    {
        int h = 0;
        for (int ci = 0; ci < static_cast<int>(owner_.categories_.size()); ++ci)
        {
            auto& cat = owner_.categories_[static_cast<size_t>(ci)];
            int count = 0;
            for (auto& s : owner_.sources_)
                if (s.category == cat.name) ++count;
            if (count == 0) continue;
            h += kCategoryHeaderHeight;
            if (cat.expanded) h += count * kSourceRowHeight;
        }
        return std::max(h, 100);
    }

    void updateSize()
    {
        setSize(std::max(1, owner_.viewport_.getWidth() - 8), getRequiredHeight());
    }

private:
    static constexpr int kCategoryHeaderHeight = 22;
    static constexpr int kSourceRowHeight = 28;
    SourcesBrowser& owner_;
};

// ── SourcesBrowser implementation ──

SourcesBrowser::~SourcesBrowser() = default;

SourcesBrowser::SourcesBrowser()
{
    listContent_ = std::make_unique<SourceListContent>(*this);
    viewport_.setViewedComponent(listContent_.get(), false);
    viewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport_);

    buildSourceList();
}

void SourcesBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    if (sources_.empty())
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.5f));
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawText("Sources available in Phase 10",
                   getLocalBounds(), juce::Justification::centred, false);
    }
}

void SourcesBrowser::resized()
{
    viewport_.setBounds(getLocalBounds());
    if (listContent_)
        listContent_->updateSize();
}

void SourcesBrowser::buildSourceList()
{
    categories_.clear();
    sources_.clear();

    // Categories from ARCHITECTURE_V2.md + Input
    categories_.push_back({"Input",         juce::Colour(0xffef5350)});
    categories_.push_back({"Fractal",       juce::Colour(0xffab47bc)});
    categories_.push_back({"Noise",         juce::Colour(0xff66bb6a)});
    categories_.push_back({"Geometric",     juce::Colour(0xff4fc3f7)});
    categories_.push_back({"Audio-Visual",  juce::Colour(0xffff7043)});
    categories_.push_back({"Nature",        juce::Colour(0xff26c6da)});

    // Input sources — live feeds
    sources_.push_back({"Camera Input",             "Input",        juce::Colour(0xffef5350)});

    // 10 Tier 1 procedural sources (actual shaders in Phase 10)
    sources_.push_back({"Mandelbrot / Julia",       "Fractal",      juce::Colour(0xffab47bc)});
    sources_.push_back({"Kaleidoscopic Fractal",    "Fractal",      juce::Colour(0xffab47bc)});
    sources_.push_back({"Perlin Noise",             "Noise",        juce::Colour(0xff66bb6a)});
    sources_.push_back({"Plasma",                   "Noise",        juce::Colour(0xff66bb6a)});
    sources_.push_back({"Voronoi",                  "Noise",        juce::Colour(0xff66bb6a)});
    sources_.push_back({"Geometric Tunnel",         "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Color Gradient",           "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Audio Waveform",           "Audio-Visual", juce::Colour(0xffff7043)});
    sources_.push_back({"Reaction-Diffusion",       "Nature",       juce::Colour(0xff26c6da)});
    sources_.push_back({"Cellular Automata",        "Nature",       juce::Colour(0xff26c6da)});
}

void SourcesBrowser::toggleCategory(int catIndex)
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
