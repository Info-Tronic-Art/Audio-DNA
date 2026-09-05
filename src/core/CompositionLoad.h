#pragma once
// Header-only, model-only (no renderer/UI) so tests/test_composition.cpp can
// drive it headless — same shape as core/MediaReconnect.h. Used by
// MainComponent's composition/deck load paths (L3, 2026-09).
#include "model/Composition.h"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

namespace compload
{
// Normalize + validate a freshly-deserialized Deck. "" = OK, else a human-readable
// refusal reason. REPAIRS (in place, on the staged copy only): numColumns >= 1 and
// >= every layer's clips.size(); every layer padded to numColumns.
inline std::string validateDeck(Deck& d)
{
    if (d.layers.empty())
        return "deck '" + d.name + "' has no layers";
    int cols = std::max(1, d.numColumns);
    for (const auto& l : d.layers)
        cols = std::max(cols, static_cast<int>(l.clips.size()));
    d.numColumns = cols;
    for (auto& l : d.layers)
        l.ensureColumns(cols);
    return "";
}

// REFUSES: no decks (an FX-preset .json, a deck .json or arbitrary JSON all parse fine
// but have no "decks" array); any deck without layers. REPAIRS: activeDeckIndex out of
// range -> 0. Runs validateDeck on every deck.
inline std::string validateComposition(Composition& c)
{
    if (c.decks.empty())
        return "not a composition file (no decks)";
    for (auto& d : c.decks)
        if (auto r = validateDeck(d); !r.empty())
            return r;
    if (c.activeDeckIndex < 0 || c.activeDeckIndex >= static_cast<int>(c.decks.size()))
        c.activeDeckIndex = 0;
    return "";
}

// Give EVERY clip a fresh id from `nextId` (the caller passes MainComponent's file-static
// s_nextClipId). Saved ids are advisory: nothing persistent references them, the renderer's
// media maps are keyed by id, and files from older builds / hand edits carry 0 or duplicate
// ids. Returns the number of clips re-minted.
inline int remintClipIds(Deck& d, uint32_t& nextId)
{
    int n = 0;
    for (auto& layer : d.layers)
        for (auto& cell : layer.clips)
            if (cell.has_value()) { cell->id = nextId++; ++n; }
    return n;
}
inline int remintClipIds(Composition& c, uint32_t& nextId)
{
    int n = 0;
    for (auto& d : c.decks) n += remintClipIds(d, nextId);
    return n;
}

// Ids of every clip that owns renderer-side media (Video / ImageSequence), sorted.
inline std::vector<uint32_t> playableClipIds(const Composition& c)
{
    std::vector<uint32_t> ids;
    for (const auto& d : c.decks)
        for (const auto& l : d.layers)
            for (const auto& cell : l.clips)
                if (cell.has_value() && cell->isPlayable()) ids.push_back(cell->id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

// before \ after: the media ids a model mutation orphaned (what must be closed).
// Full swap / New -> all old ids. Deck append -> nothing. Inputs must be sorted.
inline std::vector<uint32_t> idsRetired(const std::vector<uint32_t>& before,
                                        const std::vector<uint32_t>& after)
{
    std::vector<uint32_t> out;
    std::set_difference(before.begin(), before.end(), after.begin(), after.end(),
                        std::back_inserter(out));
    return out;
}
} // namespace compload
