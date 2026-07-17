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

            // Collect sources in category with indices (filtered by search)
            std::vector<std::pair<int, const SourceEntry*>> catSources;
            for (int si = 0; si < static_cast<int>(owner_.sources_.size()); ++si)
            {
                auto& s = owner_.sources_[static_cast<size_t>(si)];
                if (s.category == cat.name && owner_.matchesSearch(s.name))
                    catSources.push_back({si, &s});
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
                for (auto& [idx, s] : catSources)
                {
                    bool selected = owner_.selectedIndices_.count(idx) > 0;

                    // Row background (highlight if selected)
                    g.setColour(selected ? juce::Colour(0xff3a3a5e) : juce::Colour(0xff1e1e2e));
                    g.fillRect(0, y, getWidth(), kSourceRowHeight);

                    // Color indicator
                    g.setColour(s->color.withAlpha(0.7f));
                    g.fillRoundedRectangle(10.0f, static_cast<float>(y) + 6.0f,
                                           16.0f, 16.0f, 3.0f);

                    // Source name
                    g.setColour(selected ? juce::Colours::white : juce::Colour(AudioDNALookAndFeel::kTextPrimary));
                    g.setFont(juce::Font(juce::FontOptions(11.0f)));
                    g.drawText(s->name, 32, y, getWidth() - 36, kSourceRowHeight,
                               juce::Justification::centredLeft, false);

                    y += kSourceRowHeight;
                }
            }
        }
    }

    // Find which source index is at a given y position
    int sourceIndexAtY(int posY)
    {
        int y = 0;
        for (int ci = 0; ci < static_cast<int>(owner_.categories_.size()); ++ci)
        {
            auto& cat = owner_.categories_[static_cast<size_t>(ci)];
            std::vector<int> catIndices;
            for (int si = 0; si < static_cast<int>(owner_.sources_.size()); ++si)
            {
                auto& s = owner_.sources_[static_cast<size_t>(si)];
                if (s.category != cat.name || !owner_.matchesSearch(s.name)) continue;
                catIndices.push_back(si);
            }
            if (catIndices.empty()) continue;
            if (posY >= y && posY < y + kCategoryHeaderHeight) return -1;
            y += kCategoryHeaderHeight;
            if (cat.expanded)
            {
                for (int idx : catIndices)
                {
                    if (posY >= y && posY < y + kSourceRowHeight) return idx;
                    y += kSourceRowHeight;
                }
            }
        }
        return -1;
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        draggedSourceId_ = {};
        draggedSourceName_ = {};
        dragStarted_ = false;

        int idx = sourceIndexAtY(event.y);

        if (idx < 0)
        {
            // Check for category header click
            int y = 0;
            for (int ci = 0; ci < static_cast<int>(owner_.categories_.size()); ++ci)
            {
                auto& cat = owner_.categories_[static_cast<size_t>(ci)];
                int count = 0;
                for (auto& s : owner_.sources_)
                    if (s.category == cat.name && owner_.matchesSearch(s.name)) count++;
                if (count == 0) continue;
                if (event.y >= y && event.y < y + kCategoryHeaderHeight)
                {
                    owner_.toggleCategory(ci);
                    if (!event.mods.isCommandDown() && !event.mods.isShiftDown())
                        owner_.selectedIndices_.clear();
                    repaint();
                    return;
                }
                y += kCategoryHeaderHeight;
                if (cat.expanded) y += count * kSourceRowHeight;
            }
            if (!event.mods.isCommandDown() && !event.mods.isShiftDown())
                owner_.selectedIndices_.clear();
            repaint();
            return;
        }

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
            // Select visible range from anchor to clicked
            std::vector<int> vis;
            for (int ci = 0; ci < static_cast<int>(owner_.categories_.size()); ++ci)
            {
                auto& cat = owner_.categories_[static_cast<size_t>(ci)];
                for (int si = 0; si < static_cast<int>(owner_.sources_.size()); ++si)
                {
                    auto& s = owner_.sources_[static_cast<size_t>(si)];
                    if (s.category != cat.name || !owner_.matchesSearch(s.name)) continue;
                    if (cat.expanded) vis.push_back(si);
                }
            }
            int ap = -1, cp = -1;
            for (int vi = 0; vi < static_cast<int>(vis.size()); ++vi)
            {
                if (vis[static_cast<size_t>(vi)] == lastAnchorIdx_) ap = vi;
                if (vis[static_cast<size_t>(vi)] == idx) cp = vi;
            }
            if (ap >= 0 && cp >= 0)
            {
                int lo = std::min(ap, cp), hi = std::max(ap, cp);
                for (int vi = lo; vi <= hi; ++vi)
                    owner_.selectedIndices_.insert(vis[static_cast<size_t>(vi)]);
            }
        }
        else
        {
            owner_.selectedIndices_.clear();
            owner_.selectedIndices_.insert(idx);
            lastAnchorIdx_ = idx;
        }

        draggedSourceId_ = owner_.sources_[static_cast<size_t>(idx)].sourceId;
        draggedSourceName_ = owner_.sources_[static_cast<size_t>(idx)].name;
        repaint();
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        draggedSourceId_ = {};
        draggedSourceName_ = {};
        dragStarted_ = false;
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (owner_.selectedIndices_.empty() || dragStarted_)
            return;

        if (event.getDistanceFromDragStart() < 5)
            return;

        dragStarted_ = true;

        if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
        {
            // Build comma-separated list of selected source IDs
            juce::String ids;
            for (int idx : owner_.selectedIndices_)
            {
                if (ids.isNotEmpty()) ids += ",";
                ids += owner_.sources_[static_cast<size_t>(idx)].sourceId;
            }
            juce::var desc("source:" + ids);

            int count = static_cast<int>(owner_.selectedIndices_.size());
            juce::String label = count > 1
                ? juce::String(count) + " sources"
                : owner_.sources_[static_cast<size_t>(*owner_.selectedIndices_.begin())].name;

            int imgW = 140, imgH = 24;
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
                g.drawText(label, 8, 0, imgW - 16, imgH,
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
                if (s.category == cat.name && owner_.matchesSearch(s.name)) ++count;
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
    int lastAnchorIdx_ = -1;
};

// ── SourcesBrowser implementation ──

SourcesBrowser::~SourcesBrowser() = default;

SourcesBrowser::SourcesBrowser()
{
    // Search box
    searchBox_.setTextToShowWhenEmpty("Search sources...", juce::Colour(0xff666666));
    searchBox_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    searchBox_.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff444444));
    searchBox_.setColour(juce::TextEditor::textColourId, juce::Colour(0xffe0e0e0));
    searchBox_.setFont(juce::Font(juce::FontOptions(12.0f)));
    searchBox_.onTextChange = [this] {
        searchFilter_ = searchBox_.getText().trim().toLowerCase();
        if (listContent_) {
            listContent_->updateSize();
            listContent_->repaint();
        }
    };
    addAndMakeVisible(searchBox_);

    listContent_ = std::make_unique<SourceListContent>(*this);
    viewport_.setViewedComponent(listContent_.get(), false);
    viewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport_);

    buildSourceList();
}

bool SourcesBrowser::matchesSearch(const juce::String& name) const
{
    if (searchFilter_.isEmpty())
        return true;
    return name.toLowerCase().contains(searchFilter_);
}

void SourcesBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void SourcesBrowser::resized()
{
    auto bounds = getLocalBounds();
    int searchH = 24;
    searchBox_.setBounds(bounds.removeFromTop(searchH).reduced(2, 2));
    viewport_.setBounds(bounds);
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
    categories_.push_back({"3D",            juce::Colour(0xffffca28)});
    categories_.push_back({"Organic",       juce::Colour(0xff66bb6a)});
    categories_.push_back({"Pattern",       juce::Colour(0xff4fc3f7)});
    categories_.push_back({"Utility",       juce::Colour(0xff78909c)});
    categories_.push_back({"Wireframe",    juce::Colour(0xff90caf9)});
    categories_.push_back({"Lines",        juce::Colour(0xffce93d8)});
    categories_.push_back({"Math",         juce::Colour(0xffba68c8)});
    categories_.push_back({"Lighting",     juce::Colour(0xfffff176)});
    categories_.push_back({"Text",         juce::Colour(0xffe0e0e0)});
    categories_.push_back({"Particle",     juce::Colour(0xffef5350)});
    categories_.push_back({"Simulation",   juce::Colour(0xff7986cb)});
    categories_.push_back({"Routing",      juce::Colour(0xff80cbc4)});
    categories_.push_back({"MilkDrop",     juce::Colour(0xfff06292)});

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
    sources_.push_back({"Spiral Tunnel",            "spiral_tunnel",       "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Wireframe Sphere",          "wire_sphere",         "Wireframe",    juce::Colour(0xff90caf9)});
    sources_.push_back({"Wireframe Torus",           "wire_torus",          "Wireframe",    juce::Colour(0xff90caf9)});
    sources_.push_back({"Wireframe Cube",            "wire_cube",           "Wireframe",    juce::Colour(0xff90caf9)});
    sources_.push_back({"Wireframe Cylinder",        "wire_cylinder",       "Wireframe",    juce::Colour(0xff90caf9)});
    sources_.push_back({"Wireframe Cone",            "wire_cone",           "Wireframe",    juce::Colour(0xff90caf9)});
    sources_.push_back({"Wireframe Icosahedron",     "wire_icosahedron",    "Wireframe",    juce::Colour(0xff90caf9)});
    sources_.push_back({"Wireframe Wolf",            "wire_wolf",           "Wireframe",    juce::Colour(0xff90caf9)});
    sources_.push_back({"Line Generator",           "line_generator",      "Lines",        juce::Colour(0xffce93d8)});

    // Fractal sources
    sources_.push_back({"Julia Set",                "julia_set",           "Fractal",      juce::Colour(0xffab47bc)});
    sources_.push_back({"Burning Ship",             "burning_ship",        "Fractal",      juce::Colour(0xffab47bc)});
    sources_.push_back({"Newton Fractal",           "newton_fractal",      "Fractal",      juce::Colour(0xffab47bc)});
    sources_.push_back({"Sierpinski",               "sierpinski",          "Fractal",      juce::Colour(0xffab47bc)});
    sources_.push_back({"Apollonian Gasket",        "apollonian",          "Fractal",      juce::Colour(0xffab47bc)});

    // 3D Fractal sources
    sources_.push_back({"Mandelbulb",               "mandelbulb",          "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Menger Sponge",            "menger_sponge",       "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Kaleidoscopic IFS",        "kifs",                "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Julia Set 3D",             "julia_set_3d",        "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Burning Ship 3D",          "burning_ship_3d",     "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Newton 3D",                "newton_3d",           "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Sierpinski Tetrahedron",   "sierpinski_tetra",    "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Apollonian 3D",            "apollonian_3d",       "3D",           juce::Colour(0xffffca28)});

    // Raymarched Torus / Tunnel sources
    sources_.push_back({"Striped Torus",             "striped_torus",       "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Spiral Vortex",             "spiral_vortex",       "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Checker Torus",             "checker_torus",       "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Ribbed Vortex",             "ribbed_vortex",       "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Wormhole Tunnel",           "wormhole_tunnel",     "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Twisted Torus",             "twisted_torus",       "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Wormhole",                  "wormhole",            "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Torus Hole",                "torus_hole",          "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Zigzag Lines",             "zigzag_lines",        "Lines",        juce::Colour(0xffce93d8)});
    sources_.push_back({"Star Burst",               "star_burst",          "Lines",        juce::Colour(0xffce93d8)});
    sources_.push_back({"Polygon Lines",            "polygon_lines",       "Lines",        juce::Colour(0xffce93d8)});
    sources_.push_back({"Waveform Lines",           "waveform_lines",      "Lines",        juce::Colour(0xffce93d8)});
    sources_.push_back({"Lissajous",                "lissajous",           "Lines",        juce::Colour(0xffce93d8)});
    sources_.push_back({"Spirograph",               "spirograph",          "Lines",        juce::Colour(0xffce93d8)});
    sources_.push_back({"Angular Grid",             "angular_grid",        "Lines",        juce::Colour(0xffce93d8)});
    sources_.push_back({"Fractal Tree",             "fractal_tree",        "Lines",        juce::Colour(0xffce93d8)});
    sources_.push_back({"Laser Scan",               "laser_scan",          "Lines",        juce::Colour(0xffce93d8)});
    sources_.push_back({"Moire Lines",              "moire_lines",         "Lines",        juce::Colour(0xffce93d8)});

    // Phase 17: Math sources
    sources_.push_back({"Lissajous Weaver",         "lissajous_weaver",    "Math",         juce::Colour(0xffba68c8)});
    sources_.push_back({"Fermat Spiral Garden",     "fermat_spiral",       "Math",         juce::Colour(0xffba68c8)});
    sources_.push_back({"Hyperbolic Tiling",        "hyperbolic_tiling",   "Math",         juce::Colour(0xffba68c8)});
    sources_.push_back({"Penrose Pulse",            "penrose_pulse",       "Math",         juce::Colour(0xffba68c8)});

    // Phase 17: Geometric sources (new)
    sources_.push_back({"Moire Interference",       "moire_interference",  "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Astral Grid",              "astral_grid",         "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Radial Burst",             "radial_burst",        "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Hex Grid",                 "hex_grid",            "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Sacred Geometry",          "sacred_geometry",     "Geometric",    juce::Colour(0xff4fc3f7)});

    // Phase 17: 3D sources (new ray-marched + ArKaos)
    sources_.push_back({"Crystal Cavern",           "crystal_cavern",      "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Infinite Corridor",        "infinite_corridor",   "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Orbit Chamber",            "orbit_chamber",       "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Scroll Plane",             "scroll_plane",        "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Rotating Cube Map",        "rotating_cube_map",   "3D",           juce::Colour(0xffffca28)});
    sources_.push_back({"Dual Plane Drift",         "dual_plane_drift",    "3D",           juce::Colour(0xffffca28)});

    // Phase 17: Nature sources
    sources_.push_back({"Fire Wall",                "fire_wall",           "Nature",       juce::Colour(0xff26c6da)});
    sources_.push_back({"Water Caustics",           "water_caustics",      "Nature",       juce::Colour(0xff26c6da)});
    sources_.push_back({"Electric Arc",             "electric_arc",        "Nature",       juce::Colour(0xff26c6da)});

    // Phase 17: Lighting sources
    sources_.push_back({"Laser Scanner",            "laser_scanner",       "Lighting",     juce::Colour(0xfffff176)});

    // Phase 18: Audio-Native Sources
    sources_.push_back({"Spectrum Landscape",       "spectrum_landscape",  "Audio-Visual", juce::Colour(0xffff7043)});
    sources_.push_back({"Chromatic Ring",           "chromatic_ring",      "Audio-Visual", juce::Colour(0xffff7043)});
    sources_.push_back({"Band Tower",               "band_tower",          "Audio-Visual", juce::Colour(0xffff7043)});
    sources_.push_back({"Timbral Nebula",           "timbral_nebula",      "Audio-Visual", juce::Colour(0xffff7043)});
    sources_.push_back({"Structural Landscape",     "structural_landscape","Audio-Visual", juce::Colour(0xffff7043)});
    sources_.push_back({"Cymatics",                 "cymatics",            "Audio-Visual", juce::Colour(0xffff7043)});
    sources_.push_back({"Spectral Waterfall",       "spectral_waterfall",  "Audio-Visual", juce::Colour(0xffff7043)});
    sources_.push_back({"Spectral Ring",            "spectral_ring",       "Audio-Visual", juce::Colour(0xffff7043)});

    // Phase 18: Text source
    sources_.push_back({"Scrolling Text Wall",      "text_wall",           "Text",         juce::Colour(0xffe0e0e0)});

    // Phase 19: Math sources
    sources_.push_back({"Superformula",             "superformula",        "Math",         juce::Colour(0xffba68c8)});
    sources_.push_back({"Truchet Labyrinth",        "truchet_labyrinth",   "Math",         juce::Colour(0xffba68c8)});
    sources_.push_back({"Rose Curves",              "rose_curves",         "Math",         juce::Colour(0xffba68c8)});
    sources_.push_back({"Fibonacci Spiral",         "fibonacci_spiral",    "Math",         juce::Colour(0xffba68c8)});

    // Phase 19: Particle sources
    sources_.push_back({"Lightning Storm",          "lightning_storm",     "Particle",     juce::Colour(0xffef5350)});
    sources_.push_back({"Starfield",                "starfield",           "Particle",     juce::Colour(0xffef5350)});
    sources_.push_back({"Particle Nebula",          "particle_nebula",     "Particle",     juce::Colour(0xffef5350)});

    // Phase 19: Nature sources
    sources_.push_back({"Fire",                     "fire",                "Nature",       juce::Colour(0xff26c6da)});

    // Phase 19: Geometric sources
    sources_.push_back({"Radar Sweep",              "radar_sweep",         "Geometric",    juce::Colour(0xff4fc3f7)});
    sources_.push_back({"Dot Matrix Wave",          "dot_matrix_wave",     "Geometric",    juce::Colour(0xff4fc3f7)});

    // Phase 19: Pattern sources
    sources_.push_back({"Glitch Grid",              "glitch_grid",         "Pattern",      juce::Colour(0xff4fc3f7)});

    // Phase 19: 3D sources
    sources_.push_back({"DNA Helix",                "dna_helix",           "3D",           juce::Colour(0xffffca28)});

    // Phase 20: System sources (registry-registered, previously absent from this hand-maintained list)
    sources_.push_back({"Strange Attractor",        "strange_attractor",   "Simulation",   juce::Colour(0xff7986cb)});
    sources_.push_back({"Gravity Well",             "gravity_well",        "Simulation",   juce::Colour(0xff7986cb)});
    sources_.push_back({"Fluid Dynamics",           "fluid_dynamics",      "Simulation",   juce::Colour(0xff7986cb)});
    sources_.push_back({"Text Animator",            "text_animator",       "Text",         juce::Colour(0xffe0e0e0)});
    sources_.push_back({"Layer Router",             "layer_router",        "Routing",      juce::Colour(0xff80cbc4)});
    sources_.push_back({"MilkDrop Visualizer",      "projectm_visualizer", "MilkDrop",     juce::Colour(0xfff06292)});
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
