#pragma once
#include "model/Autopilot.h"
#include <atomic>
#include <cstddef>
#include <vector>

// AutopilotBank: one Autopilot per deck INDEX (s-rta-0926b plan4 item 2, T5) -- every deck's autopilot runs each
// frame, the active deck's and (plan4 B2) the ones that are not on screen.
//
// Why one per deck: Autopilot keeps ONE beat-crossing baseline (lastBeatPhase_, Autopilot.h). Calling a single
// Autopilot::processFrame for two decks in one frame lets only the first see the crossing (docs/claude/pitfalls.md
// Pitfall 37). Keyed by INDEX, not Deck::id: ids are not unique (Deck > New Deck leaves id 0, plan4 F1). Removing
// a deck shifts the indices above it, so one instance's baseline moves to its neighbour deck once -- at most one
// missed or extra beat crossing, once.
//
// Threading: pilots_ is touched ONLY on the GL thread (forIndex grows it there). The two setters may be called
// from any thread and only store; forIndex applies the stored config to the instance it hands out, so the vector is
// never walked or reallocated off the GL thread.
class AutopilotBank
{
public:
    // GL thread. Grows on demand; applies the current per-type config and smart-random flag.
    Autopilot& forIndex(size_t index)
    {
        if (pilots_.size() <= index)
            pilots_.resize(index + 1);
        Autopilot& a = pilots_[index];
        a.setPerTypeConfig(perTypeConfig_.load(std::memory_order_acquire));
        a.setSmartRandomEnabled(smartRandom_.load(std::memory_order_relaxed));
        return a;
    }

    // Any thread (P20 / P23 config, owned by the Composition / MainComponent).
    void setPerTypeConfig(const Composition::PerTypeAutopilotConfig* config)
    {
        perTypeConfig_.store(config, std::memory_order_release);
    }
    void setSmartRandomEnabled(bool enabled) { smartRandom_.store(enabled, std::memory_order_relaxed); }

    size_t size() const { return pilots_.size(); }   // GL thread / tests

private:
    std::vector<Autopilot> pilots_;
    std::atomic<const Composition::PerTypeAutopilotConfig*> perTypeConfig_{ nullptr };
    std::atomic<bool> smartRandom_{ false };
};
