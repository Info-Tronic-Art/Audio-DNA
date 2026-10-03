#pragma once
#include "model/ClipRow.h"
#include <juce_core/juce_core.h>
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <type_traits>

// Deck: a box of clips -- rows x columns (lane bf9b, s-rta-1002b). Row N of every deck feeds the show's shared
// layer N (Composition::layers); a deck has no layers and no playing state of its own, and switching decks changes
// only which box the grid shows. Composition keeps every deck (live or retired) at exactly layers.size() rows.
struct Deck
{
    // === Identity ===
    std::string name = "Deck 1";
    uint32_t id = 0;
    juce::File sourceFile;  // library file this deck was loaded from / last saved to; NOT serialized (like Composition::filePath)

    // === Grid ===
    std::vector<ClipRow> rows;
    int numColumns = 12;

    // === Defaults ===
    static constexpr int kDefaultLayers = 3;
    static constexpr int kDefaultColumns = 12;

    // === Initialization ===
    void initDefault(int numRows = kDefaultLayers)
    {
        rows.clear();
        rows.resize(static_cast<size_t>(std::max(0, numRows)));
        for (auto& row : rows)
            row.ensureColumns(numColumns);
    }

    // === Rows ===
    ClipRow* getRow(int index)
    {
        if (index >= 0 && index < static_cast<int>(rows.size()))
            return &rows[static_cast<size_t>(index)];
        return nullptr;
    }

    const ClipRow* getRow(int index) const
    {
        if (index >= 0 && index < static_cast<int>(rows.size()))
            return &rows[static_cast<size_t>(index)];
        return nullptr;
    }

    int getNumRows() const { return static_cast<int>(rows.size()); }

    // === Column Management ===
    void addColumn()
    {
        ++numColumns;
        for (auto& row : rows)
            row.ensureColumns(numColumns);
    }

    bool removeColumn(int col)
    {
        if (col < 0 || col >= numColumns || numColumns <= 1)
            return false;
        for (auto& row : rows)
        {
            if (col < static_cast<int>(row.clips.size()))
                row.clips.erase(row.clips.begin() + col);
        }
        --numColumns;
        return true;
    }

    // === Clip Access ===
    Clip* getClip(int rowIndex, int column)
    {
        if (auto* row = getRow(rowIndex))
            return row->getClipAt(column);
        return nullptr;
    }

    const Clip* getClip(int rowIndex, int column) const
    {
        if (const auto* row = getRow(rowIndex))
            return row->getClipAt(column);
        return nullptr;
    }

    void setClip(int rowIndex, int column, const Clip& clip)
    {
        if (auto* row = getRow(rowIndex))
        {
            if (column < 0)
                return;
            row->setClip(column, clip);
            if (numColumns < column + 1)
                numColumns = column + 1;
        }
    }

    // A2 fix (2026-07-30): vacate a cell to GENUINELY empty (nullopt), not a
    // blank-but-occupied Clip{}. setClip() always assigns a value, so callers
    // that want to CLEAR a cell (kClipClear et al.) must use this instead —
    // has_value()/getClipAt() consumers (autopilot occupancy scans, grid
    // paint, serialization) all key off the optional being empty. No
    // ensureColumns() growth: clearing an out-of-range or already-empty cell
    // is a safe no-op.
    void clearCell(int rowIndex, int column)
    {
        if (auto* row = getRow(rowIndex))
            row->clearCell(column);
    }

    // === Serialization ===
    // {"name","id","numColumns","layers":[{"clips":[...]}, ...]} -- the key "layers" is kept so one reader serves old
    // and new deck files (Load Deck's shape check); a new row carries "clips" only (no "type": ShowMigration tells a
    // legacy row by its "type" key, ruling-bf9b amendment 8).
    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", juce::String(name));
        obj->setProperty("id", static_cast<int>(id));
        obj->setProperty("numColumns", numColumns);

        juce::Array<juce::var> rowArray;
        for (const auto& row : rows)
            rowArray.add(row.toVar());
        obj->setProperty("layers", rowArray);

        return juce::var(obj);
    }

    // Reads the clips of every row (old and new deck objects alike); a legacy row's layer settings are read by
    // ShowMigration, never here.
    void fromVar(const juce::var& v)
    {
        if (auto* obj = v.getDynamicObject())
        {
            name = obj->getProperty("name").toString().toStdString();
            id = static_cast<uint32_t>(static_cast<int>(obj->getProperty("id")));
            numColumns = static_cast<int>(obj->getProperty("numColumns"));

            rows.clear();
            if (auto* rowArray = obj->getProperty("layers").getArray())
            {
                for (const auto& rowVar : *rowArray)
                {
                    ClipRow row;
                    row.fromVar(rowVar);
                    rows.push_back(std::move(row));
                }
            }
        }
    }
};

// decks-followup ITEM 3: inspectors keep raw Clip* across a Composition::decks reallocation
// (New/Load/Duplicate deck). That is only safe because moving a Deck can never throw, so
// std::vector::push_back/insert on `decks` moves each Deck's row vector rather than copying or
// leaving it in a state where an old pointer could dangle mid-throw (decks.md UNKNOWNS 5). bf9b: retiring a deck
// moves it between Composition::decks and its retired list the same way -- every Clip keeps its address.
static_assert(std::is_nothrow_move_constructible_v<Deck>,
              "Deck must be nothrow-move-constructible: Composition::decks reallocation moves Deck "
              "(and its row/Clip buffers) while inspectors hold raw Clip* across the call");
