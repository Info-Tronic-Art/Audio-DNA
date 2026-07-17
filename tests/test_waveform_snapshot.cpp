#include <catch2/catch_test_macros.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

// Regression test for the torn-read invariant of AnalysisThread's waveform
// snapshot (src/analysis/AnalysisThread.cpp). Before Wave 1-D that path was a
// single buffer + a count published with release ordering, but the buffer was
// memcpy'd in place — so a UI reader could observe a half-overwritten (torn)
// buffer on any overlapping access.
//
// The fix is a seqlock: the writer bumps a version to odd, writes the buffer,
// then bumps to even; the reader copies the buffer and retries if the version
// was odd or changed during the copy — so it never returns torn data, regardless
// of scheduling. (A plain double buffer, tried first, only narrows the window: a
// reader preempted long enough for the single writer to lap the two slots still
// tears. This harness — configured as a double buffer — reliably fails that way,
// which is why the shipped path uses a seqlock.)
//
// This harness reproduces the seqlock mechanism (same memory orderings as
// AnalysisThread) and stresses it with concurrent writer + readers, asserting no
// torn reads. Driving the real AnalysisThread headless would pull the whole
// audio-analysis pipeline, so this validates the concurrency invariant itself.
namespace
{
struct WaveformSeqlock
{
    static constexpr int kSize = 2048;  // mirrors AnalysisThread::kWaveformBufferSize

    alignas(64) std::array<std::atomic<float>, kSize> buffer_{};
    std::atomic<std::uint32_t> seq_{0};  // even = stable, odd = write in progress
    std::atomic<int> count_{0};

    // Writer side — identical ordering to AnalysisThread's waveform publish.
    void publish(float value, int n)
    {
        const std::uint32_t s = seq_.load(std::memory_order_relaxed);
        seq_.store(s + 1, std::memory_order_relaxed);  // enter write (odd)
        std::atomic_thread_fence(std::memory_order_release);
        for (int i = 0; i < n; ++i)
            buffer_[static_cast<size_t>(i)].store(value, std::memory_order_relaxed);
        count_.store(n, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_release);
        seq_.store(s + 2, std::memory_order_relaxed);  // exit write (even)
    }

    // Reader side — identical ordering to AnalysisThread::getWaveformSamples.
    // Returns the count of a stable read, or -1 if it gave up under contention.
    int read(std::array<float, kSize>& dest) const
    {
        for (int attempt = 0; attempt < 16; ++attempt)
        {
            const std::uint32_t s0 = seq_.load(std::memory_order_acquire);
            if (s0 & 1u)
                continue;
            const int c = count_.load(std::memory_order_relaxed);
            for (int i = 0; i < c; ++i)
                dest[static_cast<size_t>(i)] = buffer_[static_cast<size_t>(i)].load(std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_acquire);
            if (seq_.load(std::memory_order_relaxed) == s0)
                return c;
        }
        return -1;
    }
};
}  // namespace

TEST_CASE("Waveform seqlock snapshot has no torn reads under thread stress",
          "[waveform][concurrency]")
{
    WaveformSeqlock snap;
    snap.publish(0.0f, WaveformSeqlock::kSize);  // seed

    std::atomic<bool> done{false};
    std::atomic<bool> tornRead{false};       // a stable read still mixed two generations
    std::atomic<bool> wentBackwards{false};  // a reader saw an older generation after a newer one
    std::atomic<long> stableReads{0};

    // Each writer generation fills the whole buffer with one increasing value, so
    // any intra-read inconsistency (elements not all equal) proves a torn read.
    auto readerFn = [&]
    {
        std::array<float, WaveformSeqlock::kSize> local{};
        float lastGen = -1.0f;
        while (!done.load(std::memory_order_relaxed))
        {
            int c = snap.read(local);
            if (c <= 0)
                continue;  // -1 (gave up) or empty
            float g = local[0];
            for (int i = 1; i < c; ++i)
            {
                if (local[static_cast<size_t>(i)] != g)
                {
                    tornRead.store(true, std::memory_order_relaxed);
                    break;
                }
            }
            if (g < lastGen)
                wentBackwards.store(true, std::memory_order_relaxed);
            lastGen = g;
            stableReads.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::thread r1(readerFn);
    std::thread r2(readerFn);

    // Bursts of aggressive, unpaced writes hammer the readers so they are frequently
    // forced to retry mid-copy — the exact scenario a lap-prone double buffer tears
    // on, and the seqlock must survive. A brief pause every few thousand generations
    // gives readers a guaranteed clean window so they log stable reads. Runtime well
    // under 2s.
    constexpr int kGenerations = 200000;
    for (int gen = 1; gen <= kGenerations; ++gen)
    {
        snap.publish(static_cast<float>(gen), WaveformSeqlock::kSize);
        if (gen % 4000 == 0)
            std::this_thread::sleep_for(std::chrono::microseconds(50));
    }

    done.store(true, std::memory_order_relaxed);
    r1.join();
    r2.join();

    REQUIRE_FALSE(tornRead.load());
    REQUIRE_FALSE(wentBackwards.load());
    REQUIRE(stableReads.load() > 0);  // readers actually got consistent reads concurrently
}
