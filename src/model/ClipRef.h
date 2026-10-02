#pragma once
#include <cstdint>

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
    // tuple every pre-bf9b caller writes) is not valid.
    bool valid() const { return deckId <= kMaxDeckId && column >= 0 && column <= kMaxColumn; }
    bool operator==(const ClipRef&) const = default;
};
