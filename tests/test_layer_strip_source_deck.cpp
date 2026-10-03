// test_layer_strip_source_deck -- lane bf9b S3 (s-rta-1002b; plan-bf9b S3.1 / S3.2, ruling-bf9b amendments 16(a)-(c),
// 16(e), 17 and the B7 MACHINE checks M-a .. M-f). Decks are boxes of clips over ONE shared layer stack, so a layer may
// play a clip from a deck the grid does not show. The strip says which box: a BADGE on the thumbnail's bottom-left
// corner (the deck's 1-based tab position, "x" for a removed deck; dim when that deck is the shown one). A deck tab shows
// a DOT while a layer plays from it. A click on the badge shows that deck in the grid (never an Undo step, never a layer
// select). The grid lights a cell only for a ref into the SHOWN deck; the column header only on the deck it was fired
// from; a deck switch (DeckView::showDeck) changes nothing in the strip column but the badges' dimness.
// Headless JUCE widgets under ScopedJuceInitialiser_GUI (no window, no peer, no dispatch loop -- the strips' timers
// never fire; refresh() / syncTabDots() are called as the app does). Components are made visible (Pitfall 34).
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include "render/LayerClock.h"
#include "ui/DeckView.h"
#include "ui/LayerStrip.h"
#include "ShowFixture.h"
#include <cmath>
#include <set>
#include <utility>
#include <vector>

using namespace ShowFixture;
using Snap = Clip::BeatSnapMode;

namespace
{
template <typename T>
void collect(juce::Component& root, std::vector<T*>& out)
{
    for (int i = 0; i < root.getNumChildComponents(); ++i)
    {
        auto* child = root.getChildComponent(i);
        if (auto* t = dynamic_cast<T*>(child))
            out.push_back(t);
        collect(*child, out);
    }
}

LayerStrip* stripFor(DeckView& dv, int layerIndex)
{
    std::vector<LayerStrip*> strips;
    collect(dv, strips);
    for (auto* s : strips)
        if (s->getLayerIndex() == layerIndex)
            return s;
    return nullptr;
}

// The set of lit cells the grid shows, as (layer, column).
std::set<std::pair<int, int>> litCells(DeckView& dv)
{
    std::vector<ClipCell*> cells;
    collect(dv, cells);
    std::set<std::pair<int, int>> lit;
    for (auto* cell : cells)
        if (cell->isActive())
            lit.insert({ cell->getLayerIndex(), cell->getColumn() });
    return lit;
}

// The model's answer: (row, col) such that layers[row]'s active ref is (shown deck id, col).
std::set<std::pair<int, int>> modelLit(const Composition& c)
{
    std::set<std::pair<int, int>> lit;
    const uint32_t shown = c.getActiveDeck()->id;
    for (int i = 0; i < c.getNumLayers(); ++i)
    {
        const ClipRef r = c.layers[static_cast<size_t>(i)].runtime().activeRef();
        if (r.column >= 0 && r.deckId == shown)
            lit.insert({ i, r.column });
    }
    return lit;
}

juce::MouseEvent leftPressAt(juce::Component& target, juce::Point<float> at)
{
    const juce::ModifierKeys mods(juce::ModifierKeys::leftButtonModifier);
    const auto now = juce::Time::getCurrentTime();
    return { juce::Desktop::getInstance().getMainMouseSource(), at, mods,
             0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &target, &target, now, at, now, 1, false };
}

// LayerStrip::mouseDown is private; it overrides juce::Component::mouseDown (public, virtual).
void press(LayerStrip& strip, juce::Point<int> at)
{
    static_cast<juce::Component&>(strip).mouseDown(leftPressAt(strip, at.toFloat()));
}

// WCAG 2 relative luminance and contrast ratio.
double luminance(juce::Colour c)
{
    auto lin = [](juce::uint8 v) {
        const double s = v / 255.0;
        return s <= 0.03928 ? s / 12.92 : std::pow((s + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * lin(c.getRed()) + 0.7152 * lin(c.getGreen()) + 0.0722 * lin(c.getBlue());
}

double contrast(juce::Colour a, juce::Colour b)
{
    const double la = luminance(a), lb = luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

juce::Image snapshot(juce::Component& comp, juce::Rectangle<int> area)
{
    return comp.createComponentSnapshot(area, true, 1.0f).convertedToFormat(juce::Image::ARGB);
}

// Pixels that differ between two same-size images, outside every rect in `skip`.
int diffOutside(const juce::Image& a, const juce::Image& b, const std::vector<juce::Rectangle<int>>& skip)
{
    REQUIRE(a.getBounds() == b.getBounds());
    int n = 0;
    for (int y = 0; y < a.getHeight(); ++y)
        for (int x = 0; x < a.getWidth(); ++x)
        {
            const juce::Point<int> p(x, y);
            if (std::any_of(skip.begin(), skip.end(), [p](const juce::Rectangle<int>& r) { return r.contains(p); }))
                continue;
            if (a.getPixelAt(x, y) != b.getPixelAt(x, y))
                ++n;
        }
    return n;
}

int diffInside(const juce::Image& a, const juce::Image& b, juce::Rectangle<int> r)
{
    int n = 0;
    for (int y = r.getY(); y < r.getBottom(); ++y)
        for (int x = r.getX(); x < r.getRight(); ++x)
            if (a.getPixelAt(x, y) != b.getPixelAt(x, y))
                ++n;
    return n;
}

// A headless grid over `c`, sized like the app's deck area, visible (Pitfall 34).
struct Grid
{
    juce::ScopedJuceInitialiser_GUI gui;
    DeckView dv;
    explicit Grid(Composition& c)
    {
        dv.setSize(1400, 600);
        dv.setVisible(true);
        dv.setComposition(&c);
    }
    void show(Composition& c, int deck)
    {
        c.activeDeckIndex = deck;
        dv.showDeck();   // handleDeckSwitch's grid half (the renderer's fence token is not headless)
    }
};

// What the badge must say for layer i, from the model alone (M-b).
juce::String expectedBadgeText(const Composition& c, int i)
{
    const auto p = c.playing(i);
    if (p.clip == nullptr)
        return {};
    if (p.deckIndex >= 0)
        return juce::String(1 + c.findDeckIndexById(p.ref.deckId));
    return p.retired ? juce::String("x") : juce::String();
}
} // namespace

TEST_CASE("S3.1 the strip badge names the deck a playing clip came from: its tab number, dim for the shown deck, normal "
          "for another, 'x' for a removed deck, none when clear (bf9b, ruling-bf9b 16(a), B7 M-b)", "[show][strip]")
{
    Composition c = makeShow(3, 3, 4);
    fire(c, 0, 0, 1, true);   // layer 0 <- deck 0 (shown)
    fire(c, 1, 1, 2, true);   // layer 1 <- deck 1 (not shown)
    Grid g(c);                // layer 2 clear

    auto check = [&](int i, const juce::String& text, bool dim) {
        auto* s = stripFor(g.dv, i);
        REQUIRE(s != nullptr);
        INFO("layer " << i);
        CHECK(s->getSourceBadge().text() == text);
        CHECK(s->getSourceBadge().text() == expectedBadgeText(c, i));
        CHECK(s->getSourceBadge().dim == dim);
        CHECK(s->sourceBadgeBounds().isEmpty() == text.isEmpty());
    };
    check(0, "1", true);
    check(1, "2", false);
    check(2, "", false);

    SECTION("showing deck 1 dims layer 1's badge and undims layer 0's")
    {
        g.show(c, 1);
        check(0, "1", false);
        check(1, "2", true);
        check(2, "", false);
    }

    SECTION("a removed deck whose clip still plays shows 'x' (normal text); the number follows the tab position")
    {
        REQUIRE(c.retireOrEraseDeck(1));   // retired: layer 1 plays it
        g.dv.rebuildGrid();                // removeDeck's grid rebuild
        check(0, "1", true);
        check(1, "x", false);
        CHECK(stripFor(g.dv, 1)->getSourceBadge().removed);
    }

    SECTION("the number is the tab position, not the id: removing the first deck renumbers the others")
    {
        fire(c, 2, 2, 0, true);   // layer 2 <- deck 2 (tab 3)
        g.dv.refresh();
        check(2, "3", false);
        REQUIRE(c.retireOrEraseDeck(0));   // retired (layer 0 plays it): deck 2 is now tab 2
        c.activeDeckIndex = 0;             // deck 1 (was tab 2) is shown
        g.dv.rebuildGrid();
        check(0, "x", false);
        check(1, "1", true);
        check(2, "2", false);
    }
}

TEST_CASE("S3.1 a folded row draws no badge; the badge sits inside the thumbnail's bottom-left corner, clear of the "
          "routine-band rows, wide enough for '20' (bf9b, ruling-bf9b 16(a), B7 M-d)", "[show][strip]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition c = makeShow(20, 2, 2);
    fire(c, 1, 19, 1, true);   // layer 1 <- deck 20 (tab "20")
    LayerStrip strip;
    strip.setVisible(true);
    strip.setBounds(0, 0, 250, 96);   // DeckView's kLayerStripWidth x kCellHeight
    strip.setLayer(&c.layers[1], 1, &c);
    REQUIRE(strip.getSourceBadge().text() == "20");

    const auto bb = strip.sourceBadgeBounds();
    const auto tb = strip.thumbnailBoundsForTest();
    INFO("badge " << bb.toString() << " thumbnail " << tb.toString());
    CHECK(tb.contains(bb));
    CHECK(bb.getBottom() <= tb.getBottom());
    CHECK(bb.getX() - tb.getX() <= 2);                       // the left edge
    CHECK(tb.getBottom() - bb.getBottom() <= 2);             // the bottom edge
    CHECK_FALSE(bb.intersects(strip.routineBandRowsForTest()));
    CHECK(bb.getWidth() >= juce::GlyphArrangement::getStringWidthInt(LayerStrip::badgeFont(), "20") + 6);
    CHECK(bb.getHeight() >= static_cast<int>(std::ceil(LayerStrip::badgeFont().getHeight())));

    // Folded (DeckView's 22-px row): the thumbnail is under 40 px, so no badge is drawn.
    strip.setBounds(0, 0, 250, 22);
    CHECK(strip.getSourceBadge().text() == "20");
    CHECK(strip.sourceBadgeBounds().isEmpty());
}

TEST_CASE("S3.1 the clip-name row never names the deck: a 20-char deck name changes no pixel, a 30-char clip name "
          "changes only the clip-name row (bf9b, B7 M-d)", "[show][strip]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Composition c = makeShow(2, 2, 2);
    Clip* cl = c.decks[1].getClip(1, 1);
    REQUIRE(cl != nullptr);
    cl->mediaFile = juce::File("/nonexistent/bf9b/short.png");
    fire(c, 1, 1, 1, true);   // layer 1 <- deck 1 (another deck than the shown one)
    LayerStrip strip;
    strip.setVisible(true);
    strip.setBounds(0, 0, 250, 96);
    strip.setLayer(&c.layers[1], 1, &c);
    const auto all = strip.getLocalBounds();

    c.decks[1].name = "D";
    strip.refresh();
    const auto shortDeck = snapshot(strip, all);
    c.decks[1].name = "A Twenty Char Deck N";   // 20 characters
    REQUIRE(c.decks[1].name.size() == 20);
    strip.refresh();
    const auto longDeck = snapshot(strip, all);
    CHECK(diffOutside(shortDeck, longDeck, {}) == 0);

    cl->mediaFile = juce::File("/nonexistent/bf9b/a_thirty_character_clip_name_x.png");   // 30-char name
    REQUIRE(cl->mediaFile.getFileNameWithoutExtension().length() == 30);
    strip.refresh();
    const auto longClip = snapshot(strip, all);
    CHECK(diffOutside(longDeck, longClip, { strip.clipNameBoundsForTest() }) == 0);
    CHECK(diffInside(longDeck, longClip, strip.clipNameBoundsForTest()) > 0);   // the name itself did change
}

TEST_CASE("S3.1 badge contrast against its opaque background: dim >= 3:1, normal >= 7:1, normal > dim; the painted "
          "badge uses those colours (bf9b, B7 M-f)", "[show][strip]")
{
    const juce::Colour bg(LayerStrip::kBadgeBg), normal(LayerStrip::kBadgeText), dim(LayerStrip::kBadgeTextDim);
    INFO("dim " << contrast(dim, bg) << ":1, normal " << contrast(normal, bg) << ":1");
    CHECK(bg.isOpaque());
    CHECK(contrast(dim, bg) >= 3.0);
    CHECK(contrast(normal, bg) >= 7.0);
    CHECK(contrast(normal, bg) > contrast(dim, bg));
    CHECK(LayerStrip::kBadgeText != AudioDNALookAndFeel::kRoutineCue);
    CHECK(LayerStrip::kBadgeTextDim != AudioDNALookAndFeel::kRoutineCue);

    juce::ScopedJuceInitialiser_GUI gui;
    Composition c = makeShow(2, 2, 2);
    fire(c, 0, 0, 0, true);   // dim (deck 0 shown)
    fire(c, 1, 1, 0, true);   // normal
    float brightestPx[2] = { 0.0f, 0.0f };
    for (int i : { 0, 1 })
    {
        LayerStrip strip;
        strip.setVisible(true);
        strip.setBounds(0, 0, 250, 96);
        strip.setLayer(&c.layers[static_cast<size_t>(i)], i, &c);
        const auto bb = strip.sourceBadgeBounds();
        REQUIRE_FALSE(bb.isEmpty());
        const auto img = snapshot(strip, strip.getLocalBounds());
        const juce::Colour text(i == 0 ? LayerStrip::kBadgeTextDim : LayerStrip::kBadgeText);
        // The glyphs are anti-aliased (10 pt): the brightest badge pixel is the text colour, within a small tolerance,
        // and nothing is brighter than it.
        int bgPx = 0;
        float brightest = 0.0f;
        for (int y = bb.getY(); y < bb.getBottom(); ++y)
            for (int x = bb.getX(); x < bb.getRight(); ++x)
            {
                const auto p = img.getPixelAt(x, y);
                bgPx += p == bg;
                brightest = std::max(brightest, p.getBrightness());
            }
        brightestPx[i] = brightest;
        INFO("layer " << i << ": background px " << bgPx << ", brightest " << brightest << ", text colour "
                      << text.getBrightness());
        CHECK(img.getPixelAt(bb.getX(), bb.getY()) == bg);   // opaque background at its corner
        CHECK(bgPx > bb.getWidth() * bb.getHeight() / 3);
        CHECK(brightest <= text.getBrightness() + 0.02f);
        CHECK(brightest >= text.getBrightness() - 0.15f);
    }
    CHECK(brightestPx[1] > brightestPx[0] + 0.2f);   // normal reads brighter than dim on screen
}

TEST_CASE("S3.1 a badge click shows that deck in the grid and never selects the layer; a removed deck's badge does "
          "nothing; the tooltip names the deck (bf9b, ruling-bf9b 16(c))", "[show][strip]")
{
    Composition c = makeShow(3, 2, 2);
    c.decks[2].name = "Breakdown";
    fire(c, 1, 2, 1, true);   // layer 1 <- deck 2 (tab 3)
    Grid g(c);
    std::vector<uint32_t> clicked;
    int selected = 0;
    g.dv.onSourceDeckClicked = [&](uint32_t id) { clicked.push_back(id); };
    g.dv.onLayerSelected = [&](int) { ++selected; };
    auto* s = stripFor(g.dv, 1);
    REQUIRE(s != nullptr);
    const auto bb = s->sourceBadgeBounds();
    REQUIRE_FALSE(bb.isEmpty());

    CHECK(s->tooltipAt(bb.getCentre()) == "From deck 'Breakdown' (tab 3)");
    press(*s, bb.getCentre());
    CHECK(clicked == std::vector<uint32_t>{ c.decks[2].id });
    CHECK(selected == 0);
    CHECK_FALSE(s->isSelected());

    press(*s, s->thumbnailBoundsForTest().getCentre());   // elsewhere on the picture: the strip's own select
    CHECK(clicked.size() == 1);
    CHECK(selected == 1);

    SECTION("a removed deck's badge: no switch, no select")
    {
        REQUIRE(c.retireOrEraseDeck(2));
        g.dv.rebuildGrid();
        s = stripFor(g.dv, 1);
        REQUIRE(s->getSourceBadge().text() == "x");
        CHECK(s->tooltipAt(s->sourceBadgeBounds().getCentre()) == "From a removed deck");
        clicked.clear();
        selected = 0;
        press(*s, s->sourceBadgeBounds().getCentre());
        CHECK(clicked.empty());
        CHECK(selected == 0);
    }
}

TEST_CASE("S3.1 a deck tab shows a dot iff some layer's active ref, or the previous ref of a running fade, names that "
          "deck: 20 decks (bf9b, ruling-bf9b 16(b), B7 M-e)", "[show][tabs]")
{
    Composition c = makeShow(20, 3, 2);
    fire(c, 0, 3, 0, true);                 // layer 0 <- deck 3
    c.layers[1].transitionSpeed = 4.0f;
    fire(c, 1, 12, 0, true);                // layer 1: deck 12 ...
    fire(c, 1, 7, 1, true);                 // ... fades out into deck 7 (previous = deck 12, progress 0)
    REQUIRE(c.layers[1].runtime().previousRef() == ref(c, 12, 0));
    Grid g(c);

    auto dots = [&] {
        std::set<int> on;
        for (int d = 0; d < 20; ++d)
            if (g.dv.tabDotShownForTest(d))
                on.insert(d);
        return on;
    };
    auto modelDots = [&] {
        std::set<int> on;
        for (int d = 0; d < 20; ++d)
            if (c.deckIsPlaying(c.decks[static_cast<size_t>(d)].id))
                on.insert(d);
        return on;
    };
    CHECK(dots() == std::set<int>{ 3, 7, 12 });
    CHECK(dots() == modelDots());

    // The fade completes on the GL thread (no grid refresh): the app's 30 Hz tick re-reads the dots.
    CHECK(LayerClock::advanceCrossfade(c.layers[1], 5.0f));
    g.dv.syncTabDots();
    CHECK(dots() == std::set<int>{ 3, 7 });
    CHECK(dots() == modelDots());

    // A switch changes no dot; clearing a layer drops its deck's dot.
    g.show(c, 7);
    CHECK(dots() == std::set<int>{ 3, 7 });
    c.layers[0].clearActiveClip(c.rowClips(0));
    g.dv.refresh();
    CHECK(dots() == std::set<int>{ 7 });
}

TEST_CASE("S3.2 the grid lights a cell iff it is the active ref of its row's layer in the SHOWN deck: same deck, other "
          "deck, removed deck (bf9b, plan S3.2, B7 M-a)", "[show][grid]")
{
    Composition c = makeShow(3, 3, 4);
    Grid g(c);

    SECTION("same deck")
    {
        fire(c, 1, 0, 2, true);
        g.dv.refresh();
        CHECK(litCells(g.dv) == std::set<std::pair<int, int>>{ { 1, 2 } });
        CHECK(litCells(g.dv) == modelLit(c));
    }
    SECTION("another deck: no cell of the shown deck lights; showing that deck lights it")
    {
        fire(c, 1, 1, 2, true);
        g.dv.refresh();
        CHECK(litCells(g.dv).empty());
        CHECK(litCells(g.dv) == modelLit(c));
        g.show(c, 1);
        CHECK(litCells(g.dv) == std::set<std::pair<int, int>>{ { 1, 2 } });
        CHECK(litCells(g.dv) == modelLit(c));
        g.show(c, 0);
        CHECK(litCells(g.dv).empty());
    }
    SECTION("a removed deck's clip lights nothing on any live deck")
    {
        fire(c, 1, 2, 2, true);
        REQUIRE(c.retireOrEraseDeck(2));
        g.dv.rebuildGrid();
        for (int d : { 0, 1, 0 })
        {
            g.show(c, d);
            CHECK(litCells(g.dv).empty());
            CHECK(litCells(g.dv) == modelLit(c));
        }
    }
}

TEST_CASE("S3.2 the column header is lit only on the deck it was fired from (bf9b, ruling-bf9b 16(e), B7 state 5)",
          "[show][grid]")
{
    Composition c = makeShow(2, 2, 4);
    Grid g(c);
    g.dv.setActiveColumn(2, c.decks[0].id);   // handleColumnTrigger: fired on deck 0
    auto lit = [&] {
        std::set<int> on;
        for (int col = 0; col < 4; ++col)
            if (g.dv.columnHeaderLitForTest(col))
                on.insert(col);
        return on;
    };
    CHECK(lit() == std::set<int>{ 2 });
    g.show(c, 1);
    CHECK(lit().empty());
    g.show(c, 0);
    CHECK(lit() == std::set<int>{ 2 });
    g.dv.setActiveColumn(1);                  // a column without a deck lights on no deck
    CHECK(lit().empty());
    g.dv.setActiveColumn(-1);
    CHECK(lit().empty());
}

TEST_CASE("S3.2 a 0 -> 5 -> 0 showDeck walk: every LayerStrip the same object, the strip column byte-equal outside the "
          "badge rects (bf9b, plan F16, B7 M-c)", "[show][grid]")
{
    Composition c = makeShow(6, 3, 3);
    fire(c, 0, 0, 1, true);   // layer 0 <- deck 0 (shown)
    fire(c, 1, 5, 2, true);   // layer 1 <- deck 5
    fire(c, 2, 3, 0, true);   // layer 2 <- deck 3
    Grid g(c);
    std::vector<LayerStrip*> before;
    collect(g.dv, before);
    REQUIRE(before.size() == 3);

    std::vector<juce::Rectangle<int>> badges;
    for (auto* s : before)
    {
        REQUIRE_FALSE(s->sourceBadgeBounds().isEmpty());
        badges.push_back(g.dv.getLocalArea(s, s->sourceBadgeBounds()));
    }
    const auto column = g.dv.getStripColumnBounds();
    REQUIRE_FALSE(column.isEmpty());
    for (auto& r : badges)
        r = r.translated(-column.getX(), -column.getY());
    const auto shot0 = snapshot(g.dv, column);

    g.show(c, 5);
    std::vector<LayerStrip*> now;
    collect(g.dv, now);
    CHECK(now == before);
    const auto shot5 = snapshot(g.dv, column);
    CHECK(diffOutside(shot0, shot5, badges) == 0);
    int changedInBadges = 0;
    for (const auto& r : badges)
        changedInBadges += diffInside(shot0, shot5, r);
    CHECK(changedInBadges > 0);   // layers 0 and 1 changed dimness

    g.show(c, 0);
    now.clear();
    collect(g.dv, now);
    CHECK(now == before);
    const auto shotBack = snapshot(g.dv, column);
    CHECK(diffOutside(shot0, shotBack, {}) == 0);   // back to the first deck: byte-equal everywhere
}
