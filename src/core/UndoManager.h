#pragma once
#include "core/Command.h"
#include <vector>
#include <memory>
#include <functional>

// UndoManager: manages a linear history of Commands with undo/redo.
// Thread safety: all methods must be called from the message thread only.
class UndoManager
{
public:
    UndoManager() = default;

    // Execute a command and add it to history.
    // Clears any redo history (forward commands).
    void perform(std::unique_ptr<Command> cmd);

    // Undo the most recent command. Returns false if nothing to undo.
    bool undo();

    // Redo the most recently undone command. Returns false if nothing to redo.
    bool redo();

    // Query state
    bool canUndo() const;
    bool canRedo() const;
    std::string undoDescription() const;
    std::string redoDescription() const;

    // P24.13: whether the command about to be undone/redone reorders a
    // deck's layers (Command::affectsLayerOrder) — mirrors undoDescription/
    // redoDescription exactly, but as a structural flag instead of a
    // stringly-typed description match, so callers (MainComponent's undo/
    // redo call sites) can decide whether to clear a coordinate-addressed UI
    // selection without matching on display text.
    bool undoAffectsLayerOrder() const;
    bool redoAffectsLayerOrder() const;

    // Clear all history.
    void clear();

    // Get history size (for testing).
    int historySize() const { return static_cast<int>(history_.size()); }
    int undoIndex() const { return currentIndex_; }

    // Optional callback when history changes (for menu updates).
    std::function<void()> onHistoryChanged;

private:
    std::vector<std::unique_ptr<Command>> history_;
    int currentIndex_ = 0; // Points to the next slot to write (everything before is undoable)

    static constexpr int kMaxHistory = 100;
};
