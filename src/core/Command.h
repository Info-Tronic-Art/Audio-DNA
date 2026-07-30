#pragma once
#include <string>
#include <memory>

// Command pattern base class for undo/redo support.
// Every undoable state change creates a Command with execute() and undo().
class Command
{
public:
    virtual ~Command() = default;

    // Perform the action. Called on first execution and on redo.
    virtual void execute() = 0;

    // Reverse the action.
    virtual void undo() = 0;

    // Human-readable description for display in Edit menu.
    virtual std::string description() const = 0;

    // Whether this command can be merged with a subsequent command
    // of the same type (e.g., consecutive slider drags).
    virtual bool canMergeWith(const Command& /*other*/) const { return false; }
    virtual void mergeWith(const Command& /*other*/) {}

    // Whether this command reorders a deck's layers (Deck::moveLayer). A
    // layer reorder shifts indices with the layer count unchanged, so any
    // coordinate-addressed UI selection captured before the move can end up
    // silently naming a DIFFERENT layer afterward. UI layers that hold a
    // multi-cell selection (DeckView) use this to know when a just-executed/
    // undone/redone command requires clearing that selection, without a
    // stringly-typed description match. Default false; overridden by
    // MoveLayerCmd (true) and CompositeCommand (aggregates its children).
    virtual bool affectsLayerOrder() const { return false; }
};
