#pragma once
#include "model/Layer.h"
#include <juce_core/juce_core.h>
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

// Deck: the clip grid — layers (rows) × columns.
// Multiple decks can exist within a Composition, switched via deck tabs.
struct Deck
{
    // === Identity ===
    std::string name = "Deck 1";
    uint32_t id = 0;

    // === Grid ===
    std::vector<Layer> layers;
    int numColumns = 12;

    // === Defaults ===
    static constexpr int kDefaultLayers = 3;
    static constexpr int kDefaultColumns = 12;

    // === Initialization ===
    void initDefault()
    {
        layers.clear();
        for (int i = 0; i < kDefaultLayers; ++i)
        {
            Layer layer;
            layer.name = "Layer " + std::to_string(i + 1);
            layer.id = static_cast<uint32_t>(i);
            layer.type = (i == 0) ? Layer::Type::Opaque : Layer::Type::Transparent;
            layer.ensureColumns(numColumns);
            layers.push_back(std::move(layer));
        }
    }

    // === Layer Management ===
    Layer* getLayer(int index)
    {
        if (index >= 0 && index < static_cast<int>(layers.size()))
            return &layers[static_cast<size_t>(index)];
        return nullptr;
    }

    int getNumLayers() const { return static_cast<int>(layers.size()); }

    void addLayer(Layer::Type type = Layer::Type::Transparent)
    {
        Layer layer;
        layer.name = "Layer " + std::to_string(layers.size() + 1);
        layer.id = nextLayerId_++;
        layer.type = type;
        layer.ensureColumns(numColumns);
        layers.push_back(std::move(layer));
    }

    bool removeLayer(int index)
    {
        if (index < 0 || index >= static_cast<int>(layers.size()))
            return false;
        if (layers.size() <= 1)
            return false; // Must have at least one layer
        layers.erase(layers.begin() + index);
        return true;
    }

    // P24.13: Move a layer from one index to another
    bool moveLayer(int fromIndex, int toIndex)
    {
        if (fromIndex < 0 || fromIndex >= static_cast<int>(layers.size()))
            return false;
        if (toIndex < 0 || toIndex >= static_cast<int>(layers.size()))
            return false;
        if (fromIndex == toIndex)
            return false;

        Layer temp = std::move(layers[static_cast<size_t>(fromIndex)]);
        layers.erase(layers.begin() + fromIndex);
        layers.insert(layers.begin() + toIndex, std::move(temp));
        return true;
    }

    // === Column Management ===
    void addColumn()
    {
        ++numColumns;
        for (auto& layer : layers)
            layer.ensureColumns(numColumns);
    }

    bool removeColumn(int col)
    {
        if (col < 0 || col >= numColumns || numColumns <= 1)
            return false;
        for (auto& layer : layers)
        {
            if (col < static_cast<int>(layer.clips.size()))
                layer.clips.erase(layer.clips.begin() + col);
        }
        --numColumns;
        return true;
    }

    // === Column Triggering ===
    void triggerColumn(int col, Clip::BeatSnapMode forcedSnap = Clip::BeatSnapMode::Off)
    {
        for (auto& layer : layers)
        {
            if (layer.ignoreColumnTrigger)
                continue;
            layer.triggerClip(col, forcedSnap);
        }
    }

    // === Clip Access ===
    Clip* getClip(int layerIndex, int column)
    {
        if (auto* layer = getLayer(layerIndex))
            return layer->getClipAt(column);
        return nullptr;
    }

    void setClip(int layerIndex, int column, const Clip& clip)
    {
        if (auto* layer = getLayer(layerIndex))
        {
            layer->ensureColumns(column + 1);
            if (numColumns < column + 1)
                numColumns = column + 1;
            layer->clips[static_cast<size_t>(column)] = clip;
        }
    }

    // A2 fix (2026-07-30): vacate a cell to GENUINELY empty (nullopt), not a
    // blank-but-occupied Clip{}. setClip() always assigns a value, so callers
    // that want to CLEAR a cell (kClipClear et al.) must use this instead —
    // has_value()/getClipAt() consumers (autopilot occupancy scans, grid
    // paint, serialization) all key off the optional being empty. No
    // ensureColumns() growth: clearing an out-of-range or already-empty cell
    // is a safe no-op.
    void clearCell(int layerIndex, int column)
    {
        if (auto* layer = getLayer(layerIndex))
        {
            if (column >= 0 && column < static_cast<int>(layer->clips.size()))
                layer->clips[static_cast<size_t>(column)].reset();
        }
    }

    // === Serialization ===
    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", juce::String(name));
        obj->setProperty("id", static_cast<int>(id));
        obj->setProperty("numColumns", numColumns);

        juce::Array<juce::var> layerArray;
        for (const auto& layer : layers)
            layerArray.add(layer.toVar());
        obj->setProperty("layers", layerArray);

        return juce::var(obj);
    }

    void fromVar(const juce::var& v)
    {
        if (auto* obj = v.getDynamicObject())
        {
            name = obj->getProperty("name").toString().toStdString();
            id = static_cast<uint32_t>(static_cast<int>(obj->getProperty("id")));
            numColumns = static_cast<int>(obj->getProperty("numColumns"));

            layers.clear();
            if (auto* layerArray = obj->getProperty("layers").getArray())
            {
                for (const auto& layerVar : *layerArray)
                {
                    Layer layer;
                    layer.fromVar(layerVar);
                    layers.push_back(std::move(layer));
                }
            }

            // L3: nextLayerId_ resets to its default on every Deck constructed by
            // fromVar; without this, a post-load addLayer() re-mints an id a loaded
            // layer already holds, aliasing two layers onto one GL resource set.
            for (const auto& layer : layers)
                nextLayerId_ = std::max(nextLayerId_, layer.id + 1u);
        }
    }

private:
    uint32_t nextLayerId_ = 100;
};
