#pragma once
// ShowFixture (lane bf9b, s-rta-1002b, plan-bf9b S2.12): ONE pattern for every test that needs a show -- a shared layer
// stack (Composition::layers) over deck boxes of clip rows (Deck::rows). A layer names its clip by ClipRef (deck id,
// column); row N of every deck feeds shared layer N.
#include "model/Composition.h"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <utility>

namespace ShowFixture
{
// A show of `decks` decks x `layers` shared layers x `columns` columns (layer 0 Opaque, the others Transparent, as
// Composition::initDefault). With `fill`, every cell holds an Image clip with the unique id 100 * deck + 10 * row +
// column + 1, so a test can tell clips apart; without it every cell is empty.
inline Composition makeShow(int decks, int layers, int columns, bool fill = true)
{
    Composition c;
    c.initDefault();
    while (c.getNumLayers() < layers)
        c.insertLayer(c.getNumLayers(), c.makeLayer());
    while (c.getNumLayers() > layers)
        c.eraseLayer(c.getNumLayers() - 1);
    c.decks.front().numColumns = columns;
    for (auto& r : c.decks.front().rows)
        r.ensureColumns(columns);
    while (static_cast<int>(c.decks.size()) < decks)
    {
        Deck d;
        d.name = "Deck " + std::to_string(c.decks.size() + 1);
        d.numColumns = columns;
        d.initDefault(layers);
        REQUIRE(c.appendDeck(std::move(d)) >= 0);
    }
    if (fill)
        for (int d = 0; d < decks; ++d)
            for (int r = 0; r < layers; ++r)
                for (int col = 0; col < columns; ++col)
                {
                    Clip clip;
                    clip.id = static_cast<uint32_t>(100 * d + 10 * r + col + 1);
                    clip.name = "d" + std::to_string(d) + "r" + std::to_string(r) + "c" + std::to_string(col);
                    clip.mediaType = Clip::MediaType::Image;
                    c.decks[static_cast<size_t>(d)].setClip(r, col, clip);
                }
    return c;
}

// Deck `d`'s row `i` (the clips shared layer i plays from that deck).
inline ClipRow& row(Composition& c, int d, int i)
{
    ClipRow* r = c.decks.at(static_cast<size_t>(d)).getRow(i);
    REQUIRE(r != nullptr);
    return *r;
}

// Fire deck `d`'s column `col` into shared layer `i` (Composition::fire: beat snap honoured unless `immediate`).
inline LayerRuntimeTransition fire(Composition& c, int i, int d, int col, bool immediate = false)
{
    return c.fire(i, d, col, Clip::BeatSnapMode::Off, immediate);
}

// The ClipRef of (live deck index d, column col).
inline ClipRef ref(const Composition& c, int d, int col)
{
    return ClipRef{ c.decks.at(static_cast<size_t>(d)).id, col };
}
} // namespace ShowFixture
