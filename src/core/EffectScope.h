#pragma once

// EffectScope (Undo v1 step 7): addresses WHICH effect chain an EffectStackView
// / EffectStackCmd targets, by COORDINATES rather than a raw pointer. There are
// exactly three effect-chain hosts in the app, one per scope kind:
//   Global               -> Composition::globalEffects
//   Layer(deck, layer)   -> Layer::layerEffects   at (deckIndex, layerIndex)
//   Clip(deck,layer,col) -> Clip::effects         at (deckIndex, layerIndex, column)
// Commands re-resolve the live vector through the Composition on every apply
// (see core/EffectCommands.h resolveEffectVector), so nothing dangles across a
// deck/layer vector reallocation. A None scope (the default) resolves to nullptr,
// which callers treat as a safe no-op: an inspection path that does not supply a
// real scope leaves effect gestures un-undoable rather than targeting the wrong
// chain. This header is deliberately dependency-free so the UI can hold a scope
// without pulling in the model.
struct EffectScope
{
    enum class Kind { None, Global, Layer, Clip };

    Kind kind = Kind::None;
    int deckIndex = -1;
    int layerIndex = -1;
    int column = -1;

    static EffectScope none()   { return {}; }
    static EffectScope global() { return { Kind::Global, -1, -1, -1 }; }
    static EffectScope layer(int deck, int layerIdx)
        { return { Kind::Layer, deck, layerIdx, -1 }; }
    static EffectScope clip(int deck, int layerIdx, int col)
        { return { Kind::Clip, deck, layerIdx, col }; }
};
