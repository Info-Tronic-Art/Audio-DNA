#pragma once
#include "model/Deck.h"
#include "model/Layer.h"
#include <juce_core/juce_core.h>
#include <algorithm>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

// ShowMigration (lane bf9b, s-rta-1002b; plan-bf9b F7 + ruling-bf9b amendments 8, 9): reading files written before
// decks became boxes of clips. A pre-bf9b composition gave every deck its own layers (settings + clips); bf9b has ONE
// shared layer stack (Composition::layers) and decks of clip rows. This is the ONLY src reader of the two legacy keys
// "persistent" and "globalTransitionSpeed" -- both are read here only to name them in the one conversion note.
namespace ShowMigration
{
// A pre-bf9b composition has no top-level "layers" array (no version key exists; none is needed).
inline bool isLegacyShow(const juce::var& v)
{
    auto* obj = v.getDynamicObject();
    return obj != nullptr && obj->getProperty("layers").getArray() == nullptr;
}

// A deck row written by a pre-bf9b build: Layer::toVar always wrote "type" (ruling-bf9b R-F17); a bf9b row carries
// "clips" only.
inline bool isLegacyRow(const juce::var& row)
{
    auto* obj = row.getDynamicObject();
    return obj != nullptr && obj->hasProperty("type");
}

// A layer's settings as JSON for "do two rows look the same": Layer::toVar minus "id" and "name" (and "clips",
// which bf9b's toVar never writes).
inline juce::String settingsKey(const Layer& l)
{
    juce::var v = l.toVar();
    if (auto* obj = v.getDynamicObject())
    {
        obj->removeProperty("id");
        obj->removeProperty("name");
        obj->removeProperty("clips");
    }
    return juce::JSON::toString(v, true);
}

inline int connectedScalars(const Layer& l)
{
    int n = 0;
    for (const auto& c : l.scalarConns)
        if (c.isConnected())
            ++n;
    return n;
}

inline std::string plural(int n, const char* one, const char* many)
{
    return std::to_string(n) + " " + (n == 1 ? one : many);
}

// F7: a legacy composition var -> the shared layers (layer i = the settings of the FIRST deck in file order that has
// row i; colliding ids re-minted, id 0 stays valid -- Pitfall 15) and the deck boxes (every deck padded to the
// shared count). nextLayerId (in / out) is the composition's layer-id counter. Returns the note (amendment 9(b)): ""
// when nothing was dropped, else one line "old show converted: ..." naming each dropped row whose settings differ
// from the winning row's (with the layer effects / connections it carried), every layer whose file said
// "persistent": true, and the deck fade when the show had >= 2 decks.
inline std::string convertShow(const juce::var& v, std::vector<Layer>& layersOut, std::vector<Deck>& decksOut,
                               uint32_t& nextLayerId)
{
    layersOut.clear();
    decksOut.clear();
    auto* obj = v.getDynamicObject();
    if (obj == nullptr)
        return {};

    struct LegacyRow { Layer settings; bool persistent = false; };
    std::vector<std::vector<LegacyRow>> rowsPerDeck;
    if (auto* deckArray = obj->getProperty("decks").getArray())
    {
        for (const auto& deckVar : *deckArray)
        {
            Deck deck;
            deck.fromVar(deckVar);   // the clips of every row
            std::vector<LegacyRow> legacy;
            if (auto* deckObj = deckVar.getDynamicObject())
                if (auto* rowArray = deckObj->getProperty("layers").getArray())
                    for (const auto& rowVar : *rowArray)
                    {
                        LegacyRow r;
                        r.settings.fromVar(rowVar);
                        if (auto* rowObj = rowVar.getDynamicObject())
                            r.persistent = static_cast<bool>(rowObj->getProperty("persistent"));
                        legacy.push_back(std::move(r));
                    }
            rowsPerDeck.push_back(std::move(legacy));
            decksOut.push_back(std::move(deck));
        }
    }

    size_t shared = 0;
    for (const auto& d : rowsPerDeck)
        shared = std::max(shared, d.size());

    std::vector<std::string> dropped;
    for (size_t i = 0; i < shared; ++i)
    {
        size_t winner = 0;
        while (winner < rowsPerDeck.size() && rowsPerDeck[winner].size() <= i)
            ++winner;
        const Layer& win = rowsPerDeck[winner][i].settings;
        const juce::String winKey = settingsKey(win);
        layersOut.push_back(win);
        for (size_t d = winner + 1; d < rowsPerDeck.size(); ++d)
        {
            if (rowsPerDeck[d].size() <= i)
                continue;
            const Layer& lost = rowsPerDeck[d][i].settings;
            if (settingsKey(lost) == winKey)
                continue;
            std::string line = decksOut[d].name + " row " + std::to_string(i + 1) + ": settings dropped";
            const int fx = static_cast<int>(lost.layerEffects.size());
            const int conns = connectedScalars(lost);
            if (fx > 0 || conns > 0)
            {
                line += "; ";
                if (fx > 0)
                    line += plural(fx, "layer effect", "layer effects");
                if (fx > 0 && conns > 0)
                    line += ", ";
                if (conns > 0)
                    line += plural(conns, "connection", "connections");
                line += " dropped";
            }
            dropped.push_back(std::move(line));
        }
    }

    // Unique layer ids (a legacy show numbered every deck's layers 0, 1, 2 ...).
    uint32_t maxId = 0;
    for (const auto& l : layersOut)
        maxId = std::max(maxId, l.id);
    nextLayerId = std::max(nextLayerId, maxId + 1u);
    for (size_t i = 0; i < layersOut.size(); ++i)
        for (size_t j = 0; j < i; ++j)
            if (layersOut[j].id == layersOut[i].id)
            {
                layersOut[i].id = nextLayerId++;
                break;
            }

    for (auto& deck : decksOut)
        if (deck.rows.size() < shared)
        {
            const size_t from = deck.rows.size();
            deck.rows.resize(shared);
            for (size_t r = from; r < shared; ++r)
                deck.rows[r].ensureColumns(deck.numColumns);
        }

    std::vector<std::string> persistent;
    for (size_t d = 0; d < rowsPerDeck.size(); ++d)
        for (const auto& r : rowsPerDeck[d])
            if (r.persistent)
                persistent.push_back(decksOut[d].name + " / " + r.settings.name);

    std::string fade;
    if (decksOut.size() >= 2 && obj->hasProperty("globalTransitionSpeed"))
    {
        const double secs = static_cast<double>(obj->getProperty("globalTransitionSpeed"));
        if (secs > 0.001)
        {
            char buf[48];
            std::snprintf(buf, sizeof(buf), "deck fade %.2f s dropped", secs);
            fade = buf;
        }
    }

    if (dropped.empty() && persistent.empty() && fade.empty())
        return {};

    std::string note = "old show converted: layer settings come from the first deck that has each row";
    for (const auto& line : dropped)
        note += "; " + line;
    if (!persistent.empty())
    {
        note += "; 'persistent' ignored on: ";
        for (size_t k = 0; k < persistent.size(); ++k)
            note += (k > 0 ? ", " : "") + persistent[k];
    }
    if (!fade.empty())
        note += "; " + fade;
    return note;
}

// Load Deck (amendment 8): a deck file's rows decided PER ROW. A legacy row (it has "type") carries layer settings,
// returned here so the caller can use them ONLY for a row that ADDS a shared layer (F6); a bf9b row (no "type") and an
// empty row ({} or {"clips": []}) return nullopt (a layer it adds is what Add Layer makes). One entry per row of the
// deck var, in order.
inline std::vector<std::optional<Layer>> legacyRowSettings(const juce::var& deckVar)
{
    std::vector<std::optional<Layer>> out;
    if (auto* deckObj = deckVar.getDynamicObject())
        if (auto* rowArray = deckObj->getProperty("layers").getArray())
            for (const auto& rowVar : *rowArray)
            {
                if (isLegacyRow(rowVar))
                {
                    Layer l;
                    l.fromVar(rowVar);
                    out.push_back(std::move(l));
                }
                else
                {
                    out.push_back(std::nullopt);
                }
            }
    return out;
}
} // namespace ShowMigration
