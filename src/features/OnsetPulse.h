#pragma once
#include <cstdint>

// Consumer-side view of FeatureSnapshot::onsetCount (a monotonic per-hop counter that
// AnalysisThread publishes in EVERY snapshot, AnalysisThread.cpp). Turns "how many onsets
// since MY previous look" into a per-read delta, so a consumer polling the always-latest
// FeatureBus at ANY cadence neither misses an onset (slower than analysis) nor double-counts
// one (faster than analysis). Exactly ONE instance per consumer THREAD, owner-thread-confined
// like Renderer's lastDetectedGenre_ edge detector -- never shared between threads, never
// shared between uploaders (the pulse is derived once per bus read and carried in the
// snapshot copy). RecorderHost::onsetCountBaseline_ is the same idea specialised for marker
// emission; this is the generic form.
class OnsetPulse
{
public:
    // Onsets since the previous consume(). The FIRST call only establishes the baseline and
    // returns 0 (never report onsets from before this consumer started looking). Unsigned
    // subtraction: exact across a 2^32 wrap. A backwards jump (delta >= 2^31) can only come
    // from a writer reset -- test-mode inject from a cleared snapshot / TestServer reset;
    // the production writer is monotonic for the process lifetime -- so it re-baselines
    // and returns 0 instead of a spurious ~4e9.
    uint32_t consume(uint32_t onsetCount) noexcept
    {
        if (!primed_) { primed_ = true; last_ = onsetCount; return 0u; }
        const uint32_t delta = onsetCount - last_;
        last_ = onsetCount;
        return (delta & 0x80000000u) != 0u ? 0u : delta;
    }
    // Forget the baseline; the next consume() re-primes with zero delta. Call where the
    // consumer starts looking afresh (GL context creation).
    void reset() noexcept { primed_ = false; last_ = 0u; }
    bool primed() const noexcept { return primed_; }
private:
    bool     primed_ = false;
    uint32_t last_   = 0u;
};

// Test-mode feature injection's onsetCount rule -- the WRITER-side mirror of AnalysisThread's
// bookkeeping (one increment per published snapshot whose onsetDetected the writer set).
// Built by an inject request handler from what the request EXPLICITLY carried, then resolved
// against the previously injected count in exactly ONE place: TestServer::injectSnapshot,
// under its injectMutex_. Resolving there (never from a caller-side read of the bus or of a
// last-count member) is what keeps two concurrent injects from both computing "previous + 1"
// and losing an onset.
struct InjectedOnsetCount
{
    bool     hasExplicit   = false;  // request carried "onsetCount": use it verbatim
    uint32_t explicitCount = 0u;
    bool     bump          = false;  // request explicitly set "onsetDetected": true

    uint32_t resolve(uint32_t previous) const noexcept
    {
        return hasExplicit ? explicitCount : previous + (bump ? 1u : 0u);
    }

    // A cleared publish (TestServer reset): the count restarts at 0, which every consumer's
    // OnsetPulse sees as a backwards jump -> re-baseline, no pulse.
    static InjectedOnsetCount zero() noexcept
    {
        InjectedOnsetCount c;
        c.hasExplicit = true;
        return c;
    }
};
