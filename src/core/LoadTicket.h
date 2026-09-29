#pragma once
#include <juce_core/juce_core.h>
#include <atomic>

// s-rta-0929 asyncload (plan-asyncload.md 5.1 / R1): the waitable outcome of ONE composition load. The REST handler
// (POST /api/load_composition, an HTTP worker thread) waits on it, bounded; the message thread finishes it once -- Done
// at the staged swap, Failed on a synchronous refusal (parse / validate), Superseded when a newer load / New Composition
// / an explicit cancel retires the staged batch -- and ApiServer::stop() finishes every outstanding ticket Cancelled
// before httplib joins its workers (a blocked worker would hang the quit). The first finish wins (compare-exchange from
// Pending); a wait that times out reports Pending.
struct LoadTicket
{
    enum class Outcome : int { Pending = 0, Done, Failed, Superseded, Cancelled };

    // Any thread. True when this call set the outcome (then the waiter is released); false when it was already set.
    bool finish(Outcome o)
    {
        int expected = static_cast<int>(Outcome::Pending);
        if (!outcome_.compare_exchange_strong(expected, static_cast<int>(o), std::memory_order_acq_rel))
            return false;
        done_.signal();
        return true;
    }

    // Any thread. Blocks <= timeoutMs for a finish; returns the outcome (Pending on a timeout).
    Outcome wait(int timeoutMs)
    {
        if (outcome() == Outcome::Pending)
            done_.wait(timeoutMs);
        return outcome();
    }

    Outcome outcome() const { return static_cast<Outcome>(outcome_.load(std::memory_order_acquire)); }

private:
    std::atomic<int> outcome_{ 0 };
    juce::WaitableEvent done_{ true };   // manual reset: once finished, every wait returns at once
};
