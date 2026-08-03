#include "features/FeatureBus.h"
#include <cstring>

FeatureBus::FeatureBus()
{
    // Match the old bus's startup state: readers before the first publish
    // see a CLEARED snapshot (FeatureSnapshot::clear() semantics, which
    // differ from the struct defaults — see the .harmony gotcha on
    // clear()'s genre/energy values), not a default-constructed one.
    staging_.clear();

    uint32_t raw[kSnapshotWords];
    std::memcpy(raw, &staging_, sizeof(raw));
    for (size_t i = 0; i < kSnapshotWords; ++i)
        words_[i].store(raw[i], std::memory_order_relaxed);
}

FeatureBus::Writer FeatureBus::createWriter()
{
    bool expected = false;
    if (!writerClaimed_.compare_exchange_strong(expected, true,
                                                std::memory_order_acq_rel))
        return Writer{};  // double claim → deterministic invalid handle (R4)

    return Writer{*this};
}

void FeatureBus::publish()
{
    uint32_t raw[kSnapshotWords];
    std::memcpy(raw, &staging_, sizeof(raw));  // memcpy via a local (R2)

    // Seqlock write side — same construction as the waveform seqlock in
    // AnalysisThread::run(): enter odd, release fence, relaxed payload
    // stores, release fence, exit even. The fences pair with the acquire
    // loads/fence on the reader side so a reader that saw any of these
    // payload words also sees the odd seq on its re-check and retries.
    const uint64_t s = seq_.load(std::memory_order_relaxed);
    seq_.store(s + 1, std::memory_order_relaxed);  // enter publish (odd)
    std::atomic_thread_fence(std::memory_order_release);
    for (size_t i = 0; i < kSnapshotWords; ++i)
        words_[i].store(raw[i], std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_release);
    seq_.store(s + 2, std::memory_order_relaxed);  // exit publish (even)
}

// Load seq_, waiting out an in-progress publish with a bounded spin. May
// still return an odd value if the writer is wedged mid-publish (bounded —
// readers must never hang; the callers then fail that attempt).
uint64_t FeatureBus::loadStableSeq() const
{
    uint64_t s = seq_.load(std::memory_order_acquire);
    for (int spin = 0; (s & 1u) != 0 && spin < kOddSeqSpinLimit; ++spin)
        s = seq_.load(std::memory_order_acquire);
    return s;
}

FeatureSnapshot FeatureBus::read() const
{
    // R2's "last-good fallback": each reader THREAD keeps its most recent
    // verified copy. Returning it on retry exhaustion preserves the
    // per-reader monotonicity that protects edge detection downstream
    // (structuralState transitions, beat wraps) — an unverified splice
    // could step backwards. The bus pointer guards against one thread
    // touching several bus instances (tests); if a destroyed bus's address
    // is ever reused by a new bus, a stale slot could serve one stale read
    // on an exhausted attempt — acceptable: production has exactly one
    // long-lived bus, and exhaustion itself is a preemption-scale rarity.
    struct ThreadLastGood
    {
        const FeatureBus* bus = nullptr;
        FeatureSnapshot snap;
    };
    thread_local ThreadLastGood lastGood;

    FeatureSnapshot out;
    uint32_t raw[kSnapshotWords];

    for (int attempt = 0; attempt < kMaxReadAttempts; ++attempt)
    {
        const uint64_t s1 = loadStableSeq();
        if ((s1 & 1u) != 0)
            continue;  // writer wedged mid-publish — try again, bounded

        for (size_t i = 0; i < kSnapshotWords; ++i)
            raw[i] = words_[i].load(std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_acquire);

        if (seq_.load(std::memory_order_relaxed) == s1)
        {
            std::memcpy(&out, raw, sizeof(out));
            lastGood.bus = this;  // coherent copy — remember it
            lastGood.snap = out;
            return out;
        }
    }

    // Retry exhausted: the reader was lapped 4 times (preemption-scale
    // starvation, or a wedged writer). Serve this thread's last verified
    // copy — stale but coherent and never backwards for this reader.
    if (lastGood.bus == this)
        return lastGood.snap;

    // No verified copy on this thread yet: degrade to the freshest
    // available words (unverified — at worst a splice of nearby
    // generations) rather than block. Not remembered as last-good.
    for (size_t i = 0; i < kSnapshotWords; ++i)
        raw[i] = words_[i].load(std::memory_order_relaxed);
    std::memcpy(&out, raw, sizeof(out));
    return out;
}

bool FeatureBus::readIfNewer(FeatureSnapshot& out, uint64_t& lastSeq) const
{
    for (int attempt = 0; attempt < kMaxReadAttempts; ++attempt)
    {
        const uint64_t s1 = loadStableSeq();
        if ((s1 & 1u) != 0)
            continue;  // writer wedged mid-publish — try again, bounded

        if (s1 == lastSeq)
            return false;  // nothing published since the caller's copy

        uint32_t raw[kSnapshotWords];
        for (size_t i = 0; i < kSnapshotWords; ++i)
            raw[i] = words_[i].load(std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_acquire);

        if (seq_.load(std::memory_order_relaxed) == s1)
        {
            std::memcpy(&out, raw, sizeof(out));
            lastSeq = s1;
            return true;
        }
    }

    return false;  // retry exhausted — caller keeps its last good copy
}
