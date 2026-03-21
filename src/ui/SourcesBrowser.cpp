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
        draggedSourceId_ = {};
        draggedSourceName_ = {};
        dragStarted_ = false;

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
                        draggedSourceId_ = s->sourceId;
                        draggedSourceName_ = s->name;
                        return;
                    }
                    y += kSourceRowHeight;
                }
            }
        }
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        // Click (no drag) activates the source directly
        if (draggedSourceId_.isNotEmpty() && !dragStarted_)
        {
            if (owner_.onSourceActivated)
                owner_.onSourceActivated(draggedSourceId_);
        }
        draggedSourceId_ = {};
        draggedSourceName_ = {};
        dragStarted_ = false;
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (draggedSourceId_.isEmpty() || dragStarted_)
            return;

        if (event.getDistanceFromDragStart() < 5)
            return;

        dragStarted_ = true;

        if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
        {
            juce::var desc("source:" + draggedSourceId_);

            // Create drag image
            int imgW = 120, imgH = 24;
            juce::Image dragImg(juce::Image::ARGB, imgW, imgH, true);
            {
                juce::Graphics g(dragImg);
                g.setColour(juce::Colour(0xdd2a2040));
                g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(imgW),
                                       static_cast<float>(imgH), 4.0f);
                g.setColour(juce::Colour(0xffbb88ff));
                g.drawRoundedRectangle(0.5f, 0.5f, static_cast<float>(imgW - 1),
                                       static_cast<float>(imgH - 1), 4.0f, 1.0f);
                g.setColour(juce::Colours::white);
                g.setFont(juce::Font(juce::FontOptions(11.0f)));
                g.drawText(draggedSourceName_, 8, 0, imgW - 16, imgH,
                           juce::Justification::centredLeft, true);
            }

            container->startDragging(desc, this, juce::ScaledImage(dragImg), true);
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
    juce::String draggedSourceId_;
    juce::String draggedSourceName_;
    bool dragStarted_ = false;
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

    // 16 categories: 6 original + 10 new (P13.5)
    categories_.push_back({"Input",         juce::Colour(0xffef5350)});
    categories_.push_back({"Fractal",       juce::Colour(0xffab47bc)});
    categories_.push_back({"Noise",         juce::Colour(0xff66bb6a)});
    categories_.push_back({"Geometric",     juce::Colour(0xff4fc3f7)});
    categories_.push_back({"Audio-Visual",  juce::Colour(0xffff7043)});
    categories_.push_back({"Nature",        juce::Colour(0xff26c6da)});
    // P13.5: New categories for future phases
    categories_.push_back({"Math",          juce::Colour(0xffab47bc)});
    categories_.push_back({"3D",            juce::Colour(0xffffca28)});
    categories_.push_back({"Organic",       juce::Colour(0xff66bb6a)});
    categories_.push_back({"Pattern",       juce::Colour(0xff4fc3f7)});
    categories_.push_back({"Particle",      juce::Colour(0xffff7043)});
    categories_.push_back({"Utility",       juce::Colour(0xff78909c)});
    categories_.push_back({"Lighting",      juce::Colour(0xffffd54f)});
    categories_.push_back({"Simulation",    juce::Colour(0xffef5350)});
    categories_.push_back({"Text",          juce::Colour(0xff8d6e63)});
    categories_.push_back({"Routing",       juce::Colour(0xff00897b)});

    // Input sources — live feeds
    sources_.push_back({"Camera Input",             "camera",              "Input",        juce::Colour(0xffef5350)});

    // 10 Tier 1 procedural sources
    sources_.push_back({"Mandelbrot / Julia",       "mandelbrot",          "Fractal",      juce::Colour(0xffab47bc)});
    sources_.push_back({"Kaleidoscopic Fractal",    "kaleido_fractal",     "Fractal",      juce::Colour(0xffab47bc)});
    sources_.push_back({"Perlin Noise",             "perlin_noise",        "Noise",        juce::Colour(0xff66bb6a)});
    sources_.push_back({"Plasma",                   "plasma",              "Noise",        juce::Colour(0xff66bb6a)});
    sources_.push_back({"Voronoi",                  "voronoi",             "Noise",        juce::Colour(0xff66bb6a)});
    sources_.push_back({"Geometric Tunnel",         "geometric_tunnel",    "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Color Gradient",           "color_gradient",      "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Audio Waveform",           "audio_waveform",      "Audio-Visual", juce::Colour(0xffff7043)});
    sources_.push_back({"Reaction-Diffusion",       "reaction_diffusion",  "Nature",       juce::Colour(0xff26c6da)});
    sources_.push_back({"Cellular Automata",        "cellular_automata",   "Nature",       juce::Colour(0xff26c6da)});

    // P15: 12 new Resolume-class sources
    sources_.push_back({"Solid Color",              "solid_color",         "Utility",      juce::Colour(0xff78909c)});
    sources_.push_back({"Strobe Light",             "strobe_light",        "Utility",      juce::Colour(0xff78909c)});
    sources_.push_back({"Checkerboard",             "checkerboard",        "Pattern",      juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Line Pattern",             "line_pattern",        "Pattern",      juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Concentric Rings",         "concentric_rings",    "Pattern",      juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Sine Oscillator",          "sine_oscillator",     "Pattern",      juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Spiral Pattern",           "spiral_pattern",      "Pattern",      juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Terrain Lines",            "terrain_lines",       "Pattern",      juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Shape Generator",          "shape_generator",     "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Metaballs",                "metaballs",           "Organic",      juce::Colour(0xff66bb6a)});
    sources_.push_back({"Bump Light",               "bump_light",          "Pattern",      juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Infinite Zoom",            "infinite_zoom",       "Geometric",    juce::Colour(0xff4fc3f7)});
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
