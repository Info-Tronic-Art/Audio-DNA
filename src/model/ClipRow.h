#pragma once
#include "model/Clip.h"
#include <juce_core/juce_core.h>
#include <optional>
#include <vector>

// ClipRow: one row of a deck box -- the clips of one shared layer in that deck, one per column (lane bf9b,
// s-rta-1002b). Row N of every deck feeds Composition::layers[N]; the row holds clips only (the layer's settings and
// its playing tuple live on the shared Layer). The clip half of the pre-bf9b Layer, moved verbatim.
struct ClipRow
{
    // Indexed by column. std::optional so empty cells are explicit.
    std::vector<std::optional<Clip>> clips;

    Clip* getClipAt(int column)
    {
        if (column >= 0 && column < static_cast<int>(clips.size()) && clips[static_cast<size_t>(column)].has_value())
            return &clips[static_cast<size_t>(column)].value();
        return nullptr;
    }

    const Clip* getClipAt(int column) const
    {
        if (column >= 0 && column < static_cast<int>(clips.size()) && clips[static_cast<size_t>(column)].has_value())
            return &clips[static_cast<size_t>(column)].value();
        return nullptr;
    }

    // Grows the row to column + 1 cells when needed, then writes the cell (a value copy).
    void setClip(int column, const Clip& clip)
    {
        if (column < 0)
            return;
        ensureColumns(column + 1);
        clips[static_cast<size_t>(column)] = clip;
    }

    // Vacates a cell to GENUINELY empty (nullopt); an out-of-range or already-empty cell is a no-op (no growth).
    void clearCell(int column)
    {
        if (column >= 0 && column < static_cast<int>(clips.size()))
            clips[static_cast<size_t>(column)].reset();
    }

    void ensureColumns(int count)
    {
        if (static_cast<int>(clips.size()) < count)
            clips.resize(static_cast<size_t>(count));
    }

    int getNumColumns() const { return static_cast<int>(clips.size()); }

    // {"clips": [clip or null, ...]}
    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        juce::Array<juce::var> clipArray;
        for (const auto& clipOpt : clips)
            clipArray.add(clipOpt.has_value() ? clipOpt->toVar() : juce::var());
        obj->setProperty("clips", clipArray);
        return juce::var(obj);
    }

    // Reads "clips" only (a legacy row's layer settings are ShowMigration's).
    void fromVar(const juce::var& v)
    {
        clips.clear();
        if (auto* obj = v.getDynamicObject())
        {
            if (auto* clipArray = obj->getProperty("clips").getArray())
            {
                for (const auto& clipVar : *clipArray)
                {
                    if (clipVar.isVoid() || clipVar.isUndefined())
                    {
                        clips.push_back(std::nullopt);
                    }
                    else
                    {
                        Clip clip;
                        clip.fromVar(clipVar);
                        clips.push_back(std::move(clip));
                    }
                }
            }
        }
    }
};
