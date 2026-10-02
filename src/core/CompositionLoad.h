#pragma once
// Header-only, model-only (no renderer/UI) so tests/test_composition.cpp can
// drive it headless — same shape as core/MediaReconnect.h. Used by
// MainComponent's composition/deck load paths (L3, 2026-09).
#include "model/Composition.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace compload
{
// Upper bound on a deck's numColumns, checked in validateDeck below. Unlike the
// `layers`/`clips` JSON arrays (whose parsed size is inherently bounded by how
// much array data is actually present in the file), numColumns is a single int
// field that reaches Layer::ensureColumns()'s unconditional `clips.resize(count)`
// with no data behind it — a hand-edited/corrupted file's "numColumns": 2000000000
// would otherwise attempt a multi-gigabyte std::vector<std::optional<Clip>>
// allocation INSIDE the validator whose entire purpose is to refuse bad files
// (OOM/crash-on-open). 10000 is far beyond any real deck (default is 12; the
// MIDI grid controller surface tops out at 20, MidiOutputHandler.h's
// kMaxColumns) while bounding the worst-case resize to a few MB even across
// many layers.
inline constexpr int kMaxNumColumns = 10000;

// Normalize + validate a freshly-deserialized Deck. "" = OK, else a human-readable
// refusal reason. REPAIRS (in place, on the staged copy only): numColumns >= 1 and
// >= every layer's clips.size(); every layer padded to numColumns.
inline std::string validateDeck(Deck& d)
{
    if (d.layers.empty())
        return "deck '" + d.name + "' has no layers";
    if (d.numColumns > kMaxNumColumns)
        return "deck '" + d.name + "' has an implausible column count ("
             + std::to_string(d.numColumns) + ")";
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

// A procedural source's CURRENT registered parameter (what a new clip is seeded with, MainComponent's source drop).
struct RegisteredSourceParam
{
    std::string name;
    std::string uniformName;
    float defaultValue = 0.5f;
};
// sourceType -> its registered params, or nullopt for a type the registry does not know (left untouched). A
// std::function so this header stays model-only (MainComponent passes a SourceRegistry lambda; tests pass a table).
using SourceParamLookup = std::function<std::optional<std::vector<RegisteredSourceParam>>(const std::string&)>;

// s-rta-0927 source-defects (plan-source-defects.md A2): bring every Source clip's param list to the registry's
// CURRENT list, matched by uniform name. A param the source no longer registers is dropped (a control no shader read,
// saved by an older build: it would otherwise stay in the inspector, ClipInspector builds rows from the CLIP's list);
// every kept param keeps its value and connection but takes the registry's name and default (right-click reset uses
// the clip's stored default, Pitfall 8 -- an old clip would reset to an old default); a param registered since the
// file was saved is added at its default; the order follows the registry. Returns the number of clips changed.
inline int reconcileSourceParams(Deck& d, const SourceParamLookup& lookup)
{
    std::map<std::string, std::optional<std::vector<RegisteredSourceParam>>> cache;
    int changed = 0;
    for (auto& layer : d.layers)
        for (auto& cell : layer.clips)
        {
            if (!cell.has_value() || cell->mediaType != Clip::MediaType::Source || cell->sourceType.empty())
                continue;
            auto it = cache.find(cell->sourceType);
            if (it == cache.end())
                it = cache.emplace(cell->sourceType, lookup(cell->sourceType)).first;
            if (!it->second.has_value())
                continue;
            const auto& reg = *it->second;
            auto& old = cell->sourceParams;
            std::vector<Clip::SourceParam> next;
            next.reserve(reg.size());
            bool same = old.size() == reg.size();
            for (size_t i = 0; i < reg.size(); ++i)
            {
                const auto& r = reg[i];
                auto found = std::find_if(old.begin(), old.end(),
                                          [&](const Clip::SourceParam& sp) { return sp.uniformName == r.uniformName; });
                Clip::SourceParam sp;
                if (found != old.end())
                    sp = std::move(*found);
                else
                    sp.value = r.defaultValue;
                if (same && (i >= old.size() || found != old.begin() + static_cast<long>(i) || sp.name != r.name
                             || sp.defaultValue != r.defaultValue))
                    same = false;
                sp.name = r.name;
                sp.uniformName = r.uniformName;
                sp.defaultValue = r.defaultValue;
                next.push_back(std::move(sp));
            }
            old = std::move(next);
            if (!same) ++changed;
        }
    return changed;
}
inline int reconcileSourceParams(Composition& c, const SourceParamLookup& lookup)
{
    int n = 0;
    for (auto& d : c.decks) n += reconcileSourceParams(d, lookup);
    return n;
}

// Duplicate = a value copy under "<name> copy", library link dropped, EVERY clip re-minted: the copy must never share
// a clip id with its source — MainComponent's dispose hook closes media by id (its FUTURE-FRAGILE note), and
// InsertDeckCmd/RemoveDeckCmd dispose by id on undo/execute. A queued (quantized) trigger is NOT copied: the copy
// becomes the active deck at once and a copied pending trigger would fire on it at the next beat.
inline Deck duplicateDeck(const Deck& src, uint32_t& nextClipId)
{
    Deck copy = src;
    copy.name = src.name + " copy";
    copy.sourceFile = juce::File();
    copy.id = 0;                     // re-minted by Composition::appendDeck
    remintClipIds(copy, nextClipId);
    for (auto& layer : copy.layers)
    {
        auto rt = layer.runtime();
        rt.pendingTriggerColumn = -1;
        rt.pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
        layer.setRuntime(rt);
    }
    return copy;
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

// s-rta-0928 renderleft R1.5: every Image clip's file path (full path, non-empty mediaFile), in the order the renderer
// should prefetch them: the active deck first -- within each layer its active clip first, then the other columns --
// then the other decks in index order. Deduplicated, the first occurrence kept.
inline std::vector<std::string> imagePaths(const Composition& c)
{
    std::vector<std::string> out;
    std::unordered_set<std::string> seen;   // dedup in O(1) per clip (renderleft-fix)
    auto add = [&](const Clip& clip) {
        if (clip.mediaType != Clip::MediaType::Image || clip.mediaFile == juce::File())
            return;
        auto p = clip.mediaFile.getFullPathName().toStdString();
        if (p.empty() || !seen.insert(p).second)
            return;
        out.push_back(std::move(p));
    };
    auto visitDeck = [&](const Deck& d) {
        for (const auto& l : d.layers)
            if (const Clip* a = l.getActiveClip())
                add(*a);
        for (const auto& l : d.layers)
        {
            const int activeCol = l.runtime().activeClipColumn;
            for (size_t ci = 0; ci < l.clips.size(); ++ci)
                if (l.clips[ci].has_value() && static_cast<int>(ci) != activeCol)
                    add(*l.clips[ci]);
        }
    };
    const int active = c.activeDeckIndex;
    if (active >= 0 && active < static_cast<int>(c.decks.size()))
        visitDeck(c.decks[static_cast<size_t>(active)]);
    for (size_t di = 0; di < c.decks.size(); ++di)
        if (static_cast<int>(di) != active)
            visitDeck(c.decks[di]);
    return out;
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
