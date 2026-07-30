#pragma once
#include "core/Command.h"
#include <memory>
#include <string>
#include <vector>

// CompositeCommand: groups several child Commands into one atomic undo unit.
//
// Used to make a single user gesture that touches multiple cells/layers
// (multi-cell drops, multi-select clear, column trigger) cost one history
// slot. Children execute in forward order and undo in reverse order, so a
// composite behaves like the ordered application of its parts.
//
// The description is supplied by the caller (e.g. "Clear 3 Clips").
class CompositeCommand : public Command
{
public:
    explicit CompositeCommand(std::string description)
        : description_(std::move(description)) {}

    // Add a child command. Ownership transfers to the composite.
    void add(std::unique_ptr<Command> child)
    {
        if (child)
            children_.push_back(std::move(child));
    }

    bool isEmpty() const { return children_.empty(); }
    int size() const { return static_cast<int>(children_.size()); }

    // Execute children in order (also used on redo).
    void execute() override
    {
        for (auto& child : children_)
            child->execute();
    }

    // Undo children in reverse order.
    void undo() override
    {
        for (auto it = children_.rbegin(); it != children_.rend(); ++it)
            (*it)->undo();
    }

    std::string description() const override { return description_; }

    // Aggregates children: true if any child affects layer order (P24.13 —
    // see Command::affectsLayerOrder). A composite that bundles a layer
    // reorder with other edits still needs the DeckView selection cleared.
    bool affectsLayerOrder() const override
    {
        for (const auto& child : children_)
            if (child->affectsLayerOrder())
                return true;
        return false;
    }

private:
    std::string description_;
    std::vector<std::unique_ptr<Command>> children_;
};
