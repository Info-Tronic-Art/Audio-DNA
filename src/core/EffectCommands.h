#pragma once
#include "core/Command.h"
#include "core/EffectScope.h"
#include "core/DeckCommands.h"   // CompositionResolver (reused; keeps the command pointer-free)
#include "model/Composition.h"   // Composition / Deck / Layer / Clip (effect-vector resolution)
#include <functional>
#include <string>
#include <utility>
#include <vector>

// Undo v1 step 7 — effect-stack ops (#27 add incl. append-to-chain, #28 remove,
// #29 bypass toggle). One EffectStackCmd per op; no merging (spec §3).

// resolveEffectVector: turn (Composition*, EffectScope) into the live
// std::vector<Clip::EffectSlot>* the scope names, re-resolved by coordinate every
// call. Returns nullptr on a stale coordinate (deck/layer/clip removed) or a None
// scope — callers treat null as a safe no-op. Inline + renderer-free so it links
// into the headless unit tests against a bare Composition.
inline std::vector<Clip::EffectSlot>* resolveEffectVector(Composition* comp,
                                                          const EffectScope& scope)
{
    if (comp == nullptr)
        return nullptr;
    switch (scope.kind)
    {
        case EffectScope::Kind::Global:
            return &comp->globalEffects;

        case EffectScope::Kind::Layer:
            if (scope.deckIndex >= 0
                && scope.deckIndex < static_cast<int>(comp->decks.size()))
            {
                Deck& deck = comp->decks[static_cast<size_t>(scope.deckIndex)];
                if (Layer* layer = deck.getLayer(scope.layerIndex))
                    return &layer->layerEffects;
            }
            return nullptr;

        case EffectScope::Kind::Clip:
            if (scope.deckIndex >= 0
                && scope.deckIndex < static_cast<int>(comp->decks.size()))
            {
                Deck& deck = comp->decks[static_cast<size_t>(scope.deckIndex)];
                if (Clip* clip = deck.getClip(scope.layerIndex, scope.column))
                    return &clip->effects;
            }
            return nullptr;

        case EffectScope::Kind::None:
        default:
            return nullptr;
    }
}

// EffectStackCmd: one effect-chain edit (add #27, remove #28, or bypass toggle
// #29) as one undo unit. Stores a whole-vector before/after value snapshot of the
// target chain plus an EffectScope. Undo/redo is a whole-vector value assignment
// (an IDEMPOTENT set), so per the lane's pattern rule this is the mutate-then-push
// shape: the EffectStackView performs the live mutation and rebuilds itself
// exactly as today, captures before/after, and this command re-applies the chosen
// snapshot. It re-resolves the target vector through the live Composition by
// EffectScope coordinates on every apply (never the raw effects_ pointer the view
// holds), so it survives a deck/layer vector reallocation. A stale coordinate
// resolves to nullptr -> safe no-op (and the refresh hook does not fire, since
// nothing changed). The EffectSlot data round-trips exactly on undo/redo
// (unit-proven). After assigning the snapshot it fires the injected refresh hook,
// but that is only a lightweight notification: it skips a row rebuild for this
// command's OWN apply() and does NOT itself rebuild the open inspector. In the
// app every undo/redo call site runs refreshAfterUndoRedo next, which
// unconditionally re-points the inspector stacks (rebuildRows hard-codes
// expanded=false, so expanded rows collapse) -- that is what makes a row-count
// change visible. Preserving row expansion across undo/redo is a pre-existing
// refresh-path limitation, tracked as a follow-up, not a guarantee of this command.
// GL fence (family-fence fix round 2, 2026-07-28): apply() does a whole-vector
// value assignment (*vec = snapshot), which reallocates the target vector —
// reviewer-ruled REAL, blocker-class exposure for Clip/Layer scopes (GL thread
// iterates clip.effects directly, CompositorEngine.cpp:242, and copy-reads
// layer.layerEffects at :752). Global scope (Composition::globalEffects) is
// NOT currently read anywhere on the GL side (verified — grep across
// src/render/ and src/render/CompositorEngine.cpp), so it does not strictly
// need fencing today, but EffectStackCmd is ONE shared class serving all three
// scopes — fencing unconditionally in apply() is simpler than branching on
// scope_.kind and costs nothing extra (one fence per discrete edit gesture,
// not a hot path) — it also future-proofs the Global case for free if a later
// render change starts reading globalEffects.
class EffectStackCmd : public Command
{
public:
    EffectStackCmd(CompositionResolver compResolver, DeckFenceHook fence,
                   EffectScope scope,
                   std::vector<Clip::EffectSlot> before,
                   std::vector<Clip::EffectSlot> after,
                   std::function<void()> refresh, std::string description)
        : compResolver_(std::move(compResolver)), fence_(std::move(fence)),
          scope_(scope),
          before_(std::move(before)), after_(std::move(after)),
          refresh_(std::move(refresh)), description_(std::move(description)) {}

    void execute() override { runFenced([this] { apply(after_); }); }
    void undo() override    { runFenced([this] { apply(before_); }); }
    std::string description() const override { return description_; }

private:
    void apply(const std::vector<Clip::EffectSlot>& snapshot)
    {
        Composition* comp = compResolver_ ? compResolver_() : nullptr;
        std::vector<Clip::EffectSlot>* vec = resolveEffectVector(comp, scope_);
        if (vec == nullptr)
            return;                    // stale coordinate / None scope -> safe no-op
        *vec = snapshot;               // whole-vector value assignment (idempotent set)
        if (refresh_)
            refresh_();                // lightweight refresh notification only — does
                                       // NOT rebuild the inspector rows (see class doc)
    }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    CompositionResolver compResolver_;
    DeckFenceHook fence_;
    EffectScope scope_;
    std::vector<Clip::EffectSlot> before_, after_;
    std::function<void()> refresh_;
    std::string description_;
};
