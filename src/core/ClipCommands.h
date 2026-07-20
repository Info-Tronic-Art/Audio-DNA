#pragma once
#include "core/Command.h"
#include "model/Layer.h"   // Layer (and Clip)
#include <functional>
#include <optional>
#include <string>
#include <utility>

// Injected hooks that keep clip commands decoupled from the renderer/UI so they
// can be unit-tested headless against a bare Composition (spec §7).

// Re-resolve a Layer through the live model by coordinate. Returns nullptr if
// the coordinate no longer resolves (deck/layer removed). Commands NEVER store
// raw Layer*/Clip* — they hold coordinates and re-resolve on every apply.
using ClipLayerResolver = std::function<Layer*(int deckIndex, int layerIndex)>;

// Reconnect renderer-side media (video / image sequence, keyed by clip id) for
// a clip if it is missing. No-op in headless contexts. This is the spec risk #4
// guard: players are never closed today so redo reconnects for free, but the
// guard keeps redo correct if a future wave adds player disposal.
using ClipMediaHook = std::function<void(const Clip& clip)>;

// SetClipCmd: set or clear a single deck cell, addressed by (deckIndex,
// layerIndex, column). `before`/`after` are value-copied std::optional<Clip>
// snapshots (nullopt = empty cell). execute()/redo apply `after`; undo applies
// `before`.
class SetClipCmd : public Command
{
public:
    SetClipCmd(ClipLayerResolver resolver, ClipMediaHook mediaHook,
               int deckIndex, int layerIndex, int column,
               std::optional<Clip> before, std::optional<Clip> after,
               std::string description)
        : resolver_(std::move(resolver)), mediaHook_(std::move(mediaHook)),
          deckIndex_(deckIndex), layerIndex_(layerIndex), column_(column),
          before_(std::move(before)), after_(std::move(after)),
          description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(const std::optional<Clip>& state)
    {
        Layer* layer = resolver_ ? resolver_(deckIndex_, layerIndex_) : nullptr;
        if (layer == nullptr)
            return;
        layer->ensureColumns(column_ + 1);
        auto& cell = layer->clips[static_cast<size_t>(column_)];
        if (state.has_value())
        {
            cell = *state;                      // value copy
            if (mediaHook_) mediaHook_(*state); // reconnect media if missing
        }
        else
        {
            cell.reset();                       // empty cell
        }
    }

    ClipLayerResolver resolver_;
    ClipMediaHook mediaHook_;
    int deckIndex_, layerIndex_, column_;
    std::optional<Clip> before_, after_;
    std::string description_;
};

// ToggleClipLockCmd: flip a clip's contentLocked flag. Uses bool before/after
// (not a full-clip snapshot) so undo of a lock toggle does not rewind the
// clip's playback / runtime state (spec risk #5).
class ToggleClipLockCmd : public Command
{
public:
    ToggleClipLockCmd(ClipLayerResolver resolver, int deckIndex, int layerIndex,
                      int column, bool before, bool after, std::string description)
        : resolver_(std::move(resolver)), deckIndex_(deckIndex),
          layerIndex_(layerIndex), column_(column),
          before_(before), after_(after), description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(bool locked)
    {
        Layer* layer = resolver_ ? resolver_(deckIndex_, layerIndex_) : nullptr;
        if (layer == nullptr)
            return;
        if (Clip* clip = layer->getClipAt(column_))
            clip->contentLocked = locked;
    }

    ClipLayerResolver resolver_;
    int deckIndex_, layerIndex_, column_;
    bool before_, after_;
    std::string description_;
};
