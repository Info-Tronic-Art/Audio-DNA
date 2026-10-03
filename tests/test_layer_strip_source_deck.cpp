// test_layer_strip_source_deck -- lane bf9b S3 (s-rta-1002b; plan-bf9b S3.2, ruling-bf9b 16(e)) as restated by
// ruling-bf9b-merge AM-11 / AM-12 (s-rta-1003; the B7 MACHINE checks M-a, M-c, M-t, the BF14 pin, G1', G2, G3). Decks
// are boxes of clips over ONE shared layer stack, so a layer may play a clip from a deck the grid does not show -- and
// nothing on screen says which box (Boris: "The layer strip does not need to show the deck a clip is playing from.";
// asked whether the deck-tab dot stays: "drop"). THE STRIP IS THE TRUTH: it shows the clip its layer plays, identically
// whatever deck the clip came from. THE GRID IS THE BOX: a cell is lit only for a ref into the SHOWN deck; the column
// header only on the deck it was fired from, at once after any rebuild. THE TABS SAY NOTHING ABOUT PLAYING. A deck
// switch (DeckView::showDeck) changes nothing in the strip column; a rebuild keeps the selected layer's highlight.
// Headless JUCE widgets under ScopedJuceInitialiser_GUI (no window, no peer, no dispatch loop -- the strips' timers
// never fire; refresh() is called as the app does). Components are made visible (Pitfall 34).
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include "ui/DeckView.h"
#include "ui/LayerStrip.h"
#include "ShowFixture.h"
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

// The deck tab row in DeckView coordinates: the full width at the tabs' y (tabRowStateForTests is the REST witness).
juce::Rectangle<int> tabRowOf(DeckView& dv)
{
    const auto state = dv.tabRowStateForTests();
    const auto* tabs = state["tabs"].getArray();
    REQUIRE(tabs != nullptr);
    REQUIRE(tabs->size() > 0);
    return { 0, static_cast<int>((*tabs)[0]["y"]), dv.getWidth(), static_cast<int>((*tabs)[0]["h"]) };
}

// ADNA_BF9B_SHOT_DIR=<dir> writes the named snapshot there as a PNG (for a look; unset = nothing is written).
void saveShot(const juce::Image& img, const juce::String& name)
{
    const auto dir = juce::SystemStats::getEnvironmentVariable("ADNA_BF9B_SHOT_DIR", {});
    if (dir.isEmpty())
        return;
    const juce::File f = juce::File(dir).getChildFile(name + ".png");
    f.deleteFile();   // Pitfall 46: a FileOutputStream opens an existing file at its end
    juce::FileOutputStream os(f);
    REQUIRE(os.openedOk());
    REQUIRE(juce::PNGImageFormat().writeImageToStream(img, os));
}

// Fire deck d's column col into layer i after giving that clip a media file, so the strip shows a clip name (a
// ShowFixture clip has no file: its strip would look like an empty layer's).
void fireNamed(Composition& c, int i, int d, int col)
{
    Clip* cl = c.decks[static_cast<size_t>(d)].getClip(i, col);
    REQUIRE(cl != nullptr);
    cl->mediaFile = juce::File("/nonexistent/bf9b/" + juce::String(cl->name) + ".png");
    fire(c, i, d, col, true);
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

} // namespace

TEST_CASE("S3.1 the clip-name row never names the deck: a 20-char deck name changes no pixel, a 30-char clip name "
          "changes only the clip-name row (bf9b, B7 M-d)", "[show][strip]")
{
    // THE BF14 PIN (ruling-bf9b-merge AM-12 (1); Boris: "The layer strip does not need to show the deck a clip is
    // playing from."): a layer strip paints nothing that depends on the source deck. The same clip content played from
    // the shown deck, from another deck and from a removed deck gives byte-equal strips.
    juce::ScopedJuceInitialiser_GUI gui;
    Composition c = makeShow(6, 2, 2);
    for (int d : { 0, 3, 5 })   // the same clip content in three boxes (row 1, column 1)
    {
        Clip* cl = c.decks[static_cast<size_t>(d)].getClip(1, 1);
        REQUIRE(cl != nullptr);
        cl->mediaFile = juce::File("/nonexistent/bf9b/short.png");
    }
    LayerStrip strip;
    strip.setVisible(true);
    strip.setBounds(0, 0, 250, 96);
    strip.setLayer(&c.layers[1], 1, &c);
    const auto all = strip.getLocalBounds();
    const auto empty = snapshot(strip, all);   // nothing plays on the layer

    fire(c, 1, 0, 1, true);   // from the SHOWN deck
    REQUIRE(c.playing(1).deckIndex == 0);
    strip.refresh();
    const auto fromShown = snapshot(strip, all);

    fire(c, 1, 5, 1, true);   // from ANOTHER deck
    REQUIRE(c.playing(1).deckIndex == 5);
    strip.refresh();
    const auto fromOther = snapshot(strip, all);
    CHECK(diffOutside(fromShown, fromOther, {}) == 0);

    c.decks[5].name = "A Twenty Char Deck N";   // 20 characters
    REQUIRE(c.decks[5].name.size() == 20);
    strip.refresh();
    CHECK(diffOutside(fromOther, snapshot(strip, all), {}) == 0);

    fire(c, 1, 3, 1, true);   // from a deck that is then REMOVED while its clip plays
    REQUIRE(c.retireOrEraseDeck(3));
    REQUIRE(c.playing(1).retired);
    REQUIRE(c.playing(1).clip != nullptr);
    strip.refresh();
    const auto fromRemoved = snapshot(strip, all);
    CHECK(diffOutside(fromShown, fromRemoved, {}) == 0);
    saveShot(fromRemoved, "bf14-strip-from-removed-deck");

    // VALID: the snapshot sees the strip -- a playing clip differs from an empty layer.
    CHECK(diffOutside(empty, fromShown, {}) > 0);
    CHECK(diffOutside(empty, fromOther, {}) > 0);
    CHECK(diffOutside(empty, fromRemoved, {}) > 0);

    // A 30-char clip name changes only the clip-name row.
    Clip* playing = c.playing(1).clip;
    playing->mediaFile = juce::File("/nonexistent/bf9b/a_thirty_character_clip_name_x.png");
    REQUIRE(playing->mediaFile.getFileNameWithoutExtension().length() == 30);
    strip.refresh();
    const auto longClip = snapshot(strip, all);
    CHECK(diffOutside(fromRemoved, longClip, { strip.clipNameBoundsForTest() }) == 0);
    CHECK(diffInside(fromRemoved, longClip, strip.clipNameBoundsForTest()) > 0);   // the name itself did change
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

TEST_CASE("S3.2 a 0 -> 5 -> 0 showDeck walk on same-width decks: every LayerStrip the same object (SafePointer), the "
          "WHOLE strip column byte-equal in all three (bf9b, plan F16, ruling-bf9b-merge AM-12, B7 M-c)", "[show][grid]")
{
    Composition c = makeShow(6, 3, 3);
    fireNamed(c, 0, 0, 1);   // layer 0 <- deck 0 (shown)
    fireNamed(c, 1, 5, 2);   // layer 1 <- deck 5
    fireNamed(c, 2, 3, 0);   // layer 2 <- deck 3
    Grid g(c);
    g.dv.selectLayer(1);
    std::vector<LayerStrip*> before;
    collect(g.dv, before);
    REQUIRE(before.size() == 3);
    // Identity by SafePointer: a raw pointer compare can pass after destroy-and-recreate at the same address.
    std::vector<juce::Component::SafePointer<LayerStrip>> alive(before.begin(), before.end());

    const auto column = g.dv.getStripColumnBounds();
    REQUIRE_FALSE(column.isEmpty());
    g.show(c, 0);
    const auto shot0 = snapshot(g.dv, column);
    saveShot(shot0, "mc-strip-column-deck0-shown");

    auto sameStrips = [&] {
        for (const auto& sp : alive)
            if (sp == nullptr)
                return false;
        std::vector<LayerStrip*> now;
        collect(g.dv, now);
        return now == before;
    };
    g.show(c, 5);
    CHECK(sameStrips());
    const auto shot5 = snapshot(g.dv, column);
    saveShot(shot5, "mc-strip-column-deck5-shown");
    CHECK(diffOutside(shot0, shot5, {}) == 0);

    g.show(c, 0);
    CHECK(sameStrips());
    CHECK(diffOutside(shot0, snapshot(g.dv, column), {}) == 0);

    // VALID: the column snapshot sees the strips -- clearing a layer changes it.
    c.layers[1].clearActiveClip(c.rowClips(1));
    g.dv.refresh();
    CHECK(diffOutside(shot0, snapshot(g.dv, column), {}) > 0);
}


TEST_CASE("M-t the deck tabs say nothing about playing: the tab row is byte-equal whether or not layers play from its "
          "decks, also after a rebuild (bf9b, ruling-bf9b-merge AM-12 (3); Boris, asked whether the tab dot stays: "
          "\"drop\")", "[show][tabs]")
{
    Composition c = makeShow(20, 3, 2);
    Grid g(c);
    const auto row = tabRowOf(g.dv);
    REQUIRE_FALSE(row.isEmpty());
    const auto idle = snapshot(g.dv, row);   // nothing plays

    fire(c, 0, 3, 0, true);                  // layer 0 <- deck 3
    c.layers[1].transitionSpeed = 4.0f;
    fire(c, 1, 12, 0, true);                 // layer 1: deck 12 ...
    fire(c, 1, 7, 1, true);                  // ... fading into deck 7
    REQUIRE(c.deckIsPlaying(c.decks[3].id));
    REQUIRE(c.deckIsPlaying(c.decks[7].id));
    REQUIRE(c.deckIsPlaying(c.decks[12].id));
    g.dv.refresh();
    const auto playing = snapshot(g.dv, row);
    CHECK(diffOutside(idle, playing, {}) == 0);
    saveShot(playing, "mt-tab-row-three-decks-playing");

    g.dv.rebuildGrid();                      // a fresh row
    REQUIRE(tabRowOf(g.dv) == row);
    CHECK(diffOutside(idle, snapshot(g.dv, row), {}) == 0);

    // VALID: the snapshot sees the tabs -- showing another deck moves the highlight.
    g.show(c, 7);
    CHECK(diffOutside(idle, snapshot(g.dv, row), {}) > 0);
}

namespace
{
// A show whose deck 1 is 8 columns wide among 4-column decks (a switch to or from it rebuilds the grid).
Composition makeShowWithWideDeck(int decks, int layers)
{
    Composition c = makeShow(decks, layers, 4);
    c.decks[1].numColumns = 8;
    for (auto& r : c.decks[1].rows)
        r.ensureColumns(8);
    Clip wide;
    wide.id = 9001;
    wide.name = "wide c6";
    wide.mediaType = Clip::MediaType::Image;
    c.decks[1].setClip(1, 6, wide);
    return c;
}

std::set<int> litHeaders(DeckView& dv, int columns)
{
    std::set<int> on;
    for (int col = 0; col < columns; ++col)
        if (dv.columnHeaderLitForTest(col))
            on.insert(col);
    return on;
}
} // namespace

TEST_CASE("G1' a switch between a 4-column and an 8-column deck: the strip column is byte-equal across showDeck(0) / "
          "(wide) / (0) and the selected layer keeps its highlight (bf9b, ruling-bf9b-merge AM-11, FM-5)", "[show][grid]")
{
    Composition c = makeShowWithWideDeck(3, 3);
    fireNamed(c, 0, 0, 1);   // layer 0 <- deck 0
    fireNamed(c, 1, 1, 6);   // layer 1 <- the wide deck's column 6
    Grid g(c);
    g.dv.selectLayer(1);
    const auto column = g.dv.getStripColumnBounds();
    REQUIRE_FALSE(column.isEmpty());

    g.show(c, 0);
    const auto shot0 = snapshot(g.dv, column);
    REQUIRE(stripFor(g.dv, 1)->isSelected());

    const int builds = g.dv.tabRowBuilds();
    g.show(c, 1);
    REQUIRE(g.dv.tabRowBuilds() == builds + 1);   // VALID: the width differs, so this switch DID rebuild
    REQUIRE(g.dv.getStripColumnBounds() == column);
    const auto shotWide = snapshot(g.dv, column);
    saveShot(shot0, "g1-strip-column-4-columns");
    saveShot(shotWide, "g1-strip-column-8-columns");
    CHECK(stripFor(g.dv, 1)->isSelected());
    CHECK(diffOutside(shot0, shotWide, {}) == 0);

    g.show(c, 0);
    REQUIRE(g.dv.tabRowBuilds() == builds + 2);
    CHECK(stripFor(g.dv, 1)->isSelected());
    CHECK(diffOutside(shot0, snapshot(g.dv, column), {}) == 0);

    // VALID: the highlight and the playing clips are in the picture.
    g.dv.selectLayer(-1);
    const auto unselected = snapshot(g.dv, column);
    CHECK(diffOutside(shot0, unselected, {}) > 0);
    c.layers[1].clearActiveClip(c.rowClips(1));
    g.dv.refresh();
    CHECK(diffOutside(unselected, snapshot(g.dv, column), {}) > 0);
}

TEST_CASE("G2 the fired column's header is lit at once after any rebuild: column 3 fired on deck 0, showDeck(wide), "
          "showDeck(0), then a full rebuildGrid (bf9b, ruling-bf9b-merge AM-11 (a))", "[show][grid]")
{
    Composition c = makeShowWithWideDeck(2, 2);
    Grid g(c);
    g.dv.setActiveColumn(3, c.decks[0].id);   // handleColumnTrigger: fired on deck 0
    REQUIRE(litHeaders(g.dv, 4) == std::set<int>{ 3 });

    const int builds = g.dv.tabRowBuilds();
    g.show(c, 1);                             // the wide deck: a rebuild; the column was not fired from it
    REQUIRE(g.dv.tabRowBuilds() == builds + 1);
    CHECK(litHeaders(g.dv, 8).empty());
    g.show(c, 0);                             // back: a rebuild again, and NO refresh after it
    REQUIRE(g.dv.tabRowBuilds() == builds + 2);
    CHECK(litHeaders(g.dv, 4) == std::set<int>{ 3 });
    g.dv.rebuildGrid();
    CHECK(litHeaders(g.dv, 4) == std::set<int>{ 3 });
}

TEST_CASE("G3 after rebuildGrid the selected layer's strip is highlighted (bf9b, ruling-bf9b-merge AM-11 (b))",
          "[show][grid]")
{
    Composition c = makeShow(2, 3, 2);
    Grid g(c);
    g.dv.selectLayer(1);
    REQUIRE(stripFor(g.dv, 1)->isSelected());
    g.dv.rebuildGrid();
    CHECK(g.dv.getSelectedLayerIndex() == 1);
    CHECK(stripFor(g.dv, 1)->isSelected());
    CHECK_FALSE(stripFor(g.dv, 0)->isSelected());
    CHECK_FALSE(stripFor(g.dv, 2)->isSelected());
}
