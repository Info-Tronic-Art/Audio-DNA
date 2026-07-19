#include "UndoManager.h"
#include <juce_events/juce_events.h>

// Commands mutate the model, which the GL render thread reads lock-free. They
// must therefore run on the message thread only. The guard lets headless unit
// tests (no MessageManager instance) call these directly while still enforcing
// the invariant whenever a MessageManager is running (i.e. in the app).
#define UNDO_ASSERT_MESSAGE_THREAD()                                            \
    jassert(juce::MessageManager::getInstanceWithoutCreating() == nullptr       \
            || juce::MessageManager::existsAndIsCurrentThread())

void UndoManager::perform(std::unique_ptr<Command> cmd)
{
    UNDO_ASSERT_MESSAGE_THREAD();
    cmd->execute();

    // Try to merge with previous command
    if (currentIndex_ > 0)
    {
        auto& prev = history_[static_cast<size_t>(currentIndex_ - 1)];
        if (prev->canMergeWith(*cmd))
        {
            prev->mergeWith(*cmd);
            // Don't add the new command — it was merged
            if (onHistoryChanged) onHistoryChanged();
            return;
        }
    }

    // Erase any forward (redo) history
    history_.resize(static_cast<size_t>(currentIndex_));

    history_.push_back(std::move(cmd));
    ++currentIndex_;

    // Enforce max history size
    if (static_cast<int>(history_.size()) > kMaxHistory)
    {
        int excess = static_cast<int>(history_.size()) - kMaxHistory;
        history_.erase(history_.begin(), history_.begin() + excess);
        currentIndex_ -= excess;
        if (currentIndex_ < 0) currentIndex_ = 0;
    }

    if (onHistoryChanged) onHistoryChanged();
}

bool UndoManager::undo()
{
    UNDO_ASSERT_MESSAGE_THREAD();
    if (!canUndo())
        return false;

    --currentIndex_;
    history_[static_cast<size_t>(currentIndex_)]->undo();

    if (onHistoryChanged) onHistoryChanged();
    return true;
}

bool UndoManager::redo()
{
    UNDO_ASSERT_MESSAGE_THREAD();
    if (!canRedo())
        return false;

    history_[static_cast<size_t>(currentIndex_)]->execute();
    ++currentIndex_;

    if (onHistoryChanged) onHistoryChanged();
    return true;
}

bool UndoManager::canUndo() const
{
    return currentIndex_ > 0;
}

bool UndoManager::canRedo() const
{
    return currentIndex_ < static_cast<int>(history_.size());
}

std::string UndoManager::undoDescription() const
{
    if (!canUndo()) return "";
    return history_[static_cast<size_t>(currentIndex_ - 1)]->description();
}

std::string UndoManager::redoDescription() const
{
    if (!canRedo()) return "";
    return history_[static_cast<size_t>(currentIndex_)]->description();
}

void UndoManager::clear()
{
    history_.clear();
    currentIndex_ = 0;
    if (onHistoryChanged) onHistoryChanged();
}
