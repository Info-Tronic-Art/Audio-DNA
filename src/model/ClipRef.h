#pragma once
#include <cstdint>

struct Clip;

// ClipRef: how a layer names the clip it plays -- the deck box (by Deck::id, never by index: indices shift on
// Insert / Remove Deck) and the column in that deck's row for the layer (lane bf9b, s-rta-1002b). Each slot of the
// Layer trigger tuple (active / previous / pending) packs one ClipRef into the SAME 16-byte atomic word
// (LayerRuntimeCell, src/model/Layer.h; Pitfall 63), so the ranges below are the word's bit budget:
// active / previous: 16-bit deck field + 16-bit column; pending: 14-bit deck field + 14-bit (column + 1) under the
// 4-bit snap override. Deck ids 0 .. kMaxDeckId; kNoDeck = "no deck" (0x3FFF / 0xFFFF in the word).
// Columns -1 (no clip) .. kMaxColumn.
struct ClipRef
{
    static constexpr uint32_t kNoDeck = 0xFFFFFFFFu;
    static constexpr uint32_t kMaxDeckId = 0x3FFE;   // 16,382
    static constexpr int kMaxColumn = 0x3FFE;        // 16,382 (validateDeck caps a deck at 10,000 columns)

    uint32_t deckId = kNoDeck;
    int column = -1;

    // Names a cell of a deck: a deck id and a column, both in range. A deck-less ref with a column (the column-only
    // tuple every pre-bf9b caller wrote) is not valid.
    bool valid() const { return deckId <= kMaxDeckId && column >= 0 && column <= kMaxColumn; }
    // The tuple may hold it: "none" (kNoDeck, column -1), a deck with column -1, or a valid ref (ruling-bf9b
    // amendment 2(c): out of range or deck-less with a column is refused at every tuple-writing entry).
    bool storable() const
    {
        if (deckId == kNoDeck)
            return column == -1;
        return deckId <= kMaxDeckId && column >= -1 && column <= kMaxColumn;
    }
    bool operator==(const ClipRef&) const = default;
};

// RowClips: how a layer reaches the clips of ITS row in any deck box (lane bf9b): row = the layer's index in
// Composition::layers, and a ClipRef names the deck and column. Trivially copyable, never std::function: the GL
// thread calls it (autopilot, queued triggers, the compositor) inside its deckActive gate (the fence, Pitfall 55).
// Composition::rowClips(i) builds one; a default RowClips resolves nothing.
struct RowClips
{
    // The clip at (deckId, row, column), nullptr when that cell is empty or does not exist.
    Clip* (*fn)(void* ctx, uint32_t deckId, int row, int column) = nullptr;
    // The number of cells in that deck's row, -1 when the deck (live or retired) or the row does not exist.
    int (*cellsFn)(void* ctx, uint32_t deckId, int row) = nullptr;
    void* ctx = nullptr;
    int row = -1;

    Clip* at(ClipRef r) const
    {
        return (fn != nullptr && r.column >= 0 && r.deckId != ClipRef::kNoDeck) ? fn(ctx, r.deckId, row, r.column)
                                                                                : nullptr;
    }
    // The cell exists (it may be empty): the deck is known and the column is inside its row.
    bool hasCell(ClipRef r) const
    {
        if (cellsFn == nullptr || r.column < 0 || r.deckId == ClipRef::kNoDeck)
            return false;
        return r.column < cellsFn(ctx, r.deckId, row);
    }
};
