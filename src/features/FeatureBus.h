#pragma once
#include <atomic>
#include <array>
#include <cstdint>
#include <cstddef>
#include "analysis/FeatureSnapshot.h"

// Seqlock snapshot bus (featurebus-thread-safety-design.md R1-R4).
//
// ONE writer — the analysis thread in production, the TestServer in test
// mode — publishes FeatureSnapshots; ANY number of readers on any thread
// (GL threads, UI timers, message thread, HTTP threads) take coherent value
// copies concurrently. No pointer to shared storage ever escapes.
//
// Mechanism:
//   - seq_: generation counter. Odd = a publish is in progress; each
//     publish advances it by 2.
//   - words_: the 320-byte snapshot payload as 80 atomic uint32 words.
//     Atomic words make concurrent payload access defined behavior — the
//     bus is TSan-clean with ZERO suppressions (std::atomic_ref is
//     unavailable on this toolchain; the atomic-word array is the
//     mechanism, same construction as the waveform seqlock in
//     AnalysisThread).
//   - staging_: writer-private fill buffer. acquireWrite() returns it, the
//     writer fills it at leisure (the ~200-line stage pipeline in
//     AnalysisThread.cpp), then publishWrite() copies it into words_
//     inside one odd/even seq_ window. Readers can never observe a
//     half-filled snapshot, no matter how long the fill takes.
//   - Readers copy words_ into a local and retry (bounded, 4 attempts)
//     when a publish overlapped the copy; see read()/readIfNewer() for the
//     exhaustion fallback.
//
// Single-writer enforcement (R4): all publishing goes through a move-only
// Writer handle — a second concurrent writer cannot be expressed in the
// type system (no copy), and a second createWriter() claim at runtime
// deterministically returns an INVALID handle (isValid() == false,
// acquireWrite() == nullptr) instead of a second path to staging_.

class FeatureBus
{
public:
    // Move-only writer handle. The bus itself exposes no mutating API;
    // destroying the handle releases the claim.
    class Writer
    {
    public:
        Writer() = default;
        Writer(Writer&& other) noexcept : bus_(other.bus_) { other.bus_ = nullptr; }
        Writer& operator=(Writer&& other) noexcept
        {
            if (this != &other)
            {
                release();
                bus_ = other.bus_;
                other.bus_ = nullptr;
            }
            return *this;
        }
        Writer(const Writer&) = delete;
        Writer& operator=(const Writer&) = delete;
        ~Writer() { release(); }

        // True when this handle holds the bus's single writer claim.
        bool isValid() const { return bus_ != nullptr; }

        // Get the writer-private staging buffer. Fill it, then call
        // publishWrite(). Returns nullptr on an invalid handle.
        FeatureSnapshot* acquireWrite() { return bus_ != nullptr ? &bus_->staging_ : nullptr; }

        // Publish the staging buffer to readers. No-op on an invalid handle.
        void publishWrite()
        {
            if (bus_ != nullptr)
                bus_->publish();
        }

    private:
        friend class FeatureBus;
        explicit Writer(FeatureBus& bus) : bus_(&bus) {}
        void release()
        {
            if (bus_ != nullptr)
            {
                bus_->writerClaimed_.store(false, std::memory_order_release);
                bus_ = nullptr;
            }
        }
        FeatureBus* bus_ = nullptr;
    };

    FeatureBus();

    // Claim the single writer role. The first call returns a valid handle;
    // any further call while that handle lives returns an invalid one
    // (deterministic double-claim failure, R4). Destroying the valid handle
    // releases the claim.
    Writer createWriter();

    // --- Reader API (any thread) ---

    // Coherent value copy of the most recently published snapshot. Before
    // the first publish this is a cleared snapshot (FeatureSnapshot::clear()
    // semantics — the old bus's startup state). On retry exhaustion (a
    // preemption-scale rarity) returns the calling thread's last verified
    // copy — stale but coherent, never backwards for that reader.
    FeatureSnapshot read() const;

    // Copy only if a publish has happened since *lastSeq. On fresh data,
    // fills `out`, updates lastSeq and returns true. Otherwise returns
    // false and leaves `out` untouched (the caller's last good copy).
    // Callers initialize their held lastSeq to 0.
    bool readIfNewer(FeatureSnapshot& out, uint64_t& lastSeq) const;

private:
    void publish();
    uint64_t loadStableSeq() const;

    // 4 verified attempts; an attempt only fails when a publish STARTS
    // during its copy (~1e-5 duty at the 10.67 ms publish cadence), so
    // exhausting all 4 is a ~1e-20 event — see read() for the fallback.
    static constexpr int kMaxReadAttempts = 4;
    // Bounded wait for an in-progress publish (~80 relaxed stores) to
    // finish. Generous vs. the real publish duration, but finite so a
    // wedged writer (killed mid-publish) degrades to the fallback instead
    // of hanging a render thread.
    static constexpr int kOddSeqSpinLimit = 1024;
    static constexpr size_t kSnapshotWords = 80;

    static_assert(sizeof(FeatureSnapshot) == 320,
                  "FeatureSnapshot size changed — resize kSnapshotWords so the "
                  "seqlock payload keeps covering the whole struct");
    static_assert(kSnapshotWords * sizeof(uint32_t) == sizeof(FeatureSnapshot),
                  "words_ must cover FeatureSnapshot exactly");
    static_assert(std::is_trivially_copyable_v<FeatureSnapshot>,
                  "seqlock payload transport is memcpy — FeatureSnapshot must "
                  "stay trivially copyable");
    static_assert(sizeof(std::atomic<uint32_t>) == sizeof(uint32_t),
                  "atomic payload word must be layout-identical to uint32_t");
    static_assert(std::atomic<uint32_t>::is_always_lock_free
                      && std::atomic<uint64_t>::is_always_lock_free,
                  "seqlock atomics must be lock-free on this platform");

    alignas(64) std::atomic<uint64_t> seq_{0};   // odd = publish in progress
    std::array<std::atomic<uint32_t>, kSnapshotWords> words_;
    FeatureSnapshot staging_;                    // writer-private (R3)
    std::atomic<bool> writerClaimed_{false};
};
