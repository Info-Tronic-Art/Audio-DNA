#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "features/FeatureBus.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>

using Catch::Approx;

TEST_CASE("FeatureBus initial state — cleared snapshot, nothing newer", "[featurebus]")
{
    FeatureBus bus;

    // read() before any publish: a cleared snapshot (clear() semantics)
    FeatureSnapshot snap = bus.read();
    REQUIRE(snap.rms == Approx(0.0f));
    REQUIRE(snap.rmsDB == Approx(-100.0f));
    REQUIRE(snap.detectedKey == -1);

    // readIfNewer with a fresh lastSeq: nothing published yet
    FeatureSnapshot out;
    out.rms = 42.0f;  // sentinel
    uint64_t lastSeq = 0;
    REQUIRE_FALSE(bus.readIfNewer(out, lastSeq));
    REQUIRE(out.rms == Approx(42.0f));  // untouched on false
    REQUIRE(lastSeq == 0);
}

TEST_CASE("FeatureBus single write-read cycle", "[featurebus]")
{
    FeatureBus bus;
    FeatureBus::Writer writer = bus.createWriter();
    REQUIRE(writer.isValid());

    // Write a snapshot
    FeatureSnapshot* ws = writer.acquireWrite();
    REQUIRE(ws != nullptr);
    ws->rms = 0.75f;
    ws->spectralCentroid = 440.0f;
    ws->bpm = 120.0f;
    writer.publishWrite();

    // readIfNewer sees exactly one fresh publish
    FeatureSnapshot out;
    uint64_t lastSeq = 0;
    REQUIRE(bus.readIfNewer(out, lastSeq));
    REQUIRE(out.rms == Approx(0.75f));
    REQUIRE(out.spectralCentroid == Approx(440.0f));
    REQUIRE(out.bpm == Approx(120.0f));

    // No second fresh copy until the next publish
    REQUIRE_FALSE(bus.readIfNewer(out, lastSeq));

    // read() has no freshness state — always the latest published data
    FeatureSnapshot latest = bus.read();
    REQUIRE(latest.rms == Approx(0.75f));
}

TEST_CASE("FeatureBus read returns the last published snapshot repeatedly", "[featurebus]")
{
    FeatureBus bus;
    FeatureBus::Writer writer = bus.createWriter();

    FeatureSnapshot* ws = writer.acquireWrite();
    ws->rms = 0.5f;
    writer.publishWrite();

    // Repeated reads with no new publish return the same snapshot
    REQUIRE(bus.read().rms == Approx(0.5f));
    REQUIRE(bus.read().rms == Approx(0.5f));
}

TEST_CASE("FeatureBus multiple writes before read — reader gets latest", "[featurebus]")
{
    FeatureBus bus;
    FeatureBus::Writer writer = bus.createWriter();

    // Write three times without reading
    for (int i = 1; i <= 3; ++i)
    {
        FeatureSnapshot* ws = writer.acquireWrite();
        ws->rms = static_cast<float>(i) * 0.1f;
        writer.publishWrite();
    }

    // Reader should get the most recent write (0.3)
    REQUIRE(bus.read().rms == Approx(0.3f));

    // readIfNewer skips straight to the latest generation too
    FeatureSnapshot out;
    uint64_t lastSeq = 0;
    REQUIRE(bus.readIfNewer(out, lastSeq));
    REQUIRE(out.rms == Approx(0.3f));
}

TEST_CASE("FeatureBus writer does not clobber a reader's copy", "[featurebus]")
{
    FeatureBus bus;
    FeatureBus::Writer writer = bus.createWriter();

    // Write and read
    FeatureSnapshot* ws = writer.acquireWrite();
    ws->rms = 0.5f;
    writer.publishWrite();

    const FeatureSnapshot copy = bus.read();
    REQUIRE(copy.rms == Approx(0.5f));

    // Write two more times — the caller-owned copy is untouchable (R1)
    for (int i = 0; i < 2; ++i)
    {
        ws = writer.acquireWrite();
        ws->rms = 0.99f;
        writer.publishWrite();
    }

    REQUIRE(copy.rms == Approx(0.5f));
    REQUIRE(bus.read().rms == Approx(0.99f));
}

TEST_CASE("FeatureBus staging buffer is stable and writer-private", "[featurebus]")
{
    FeatureBus bus;
    FeatureBus::Writer writer = bus.createWriter();

    // R3: acquireWrite() hands back the same private staging buffer every
    // time — the long analysis fill never touches reader-visible storage.
    FeatureSnapshot* s1 = writer.acquireWrite();
    s1->rms = 0.1f;
    writer.publishWrite();

    FeatureSnapshot* s2 = writer.acquireWrite();
    REQUIRE(s1 == s2);

    // An unpublished fill is invisible to readers
    s2->rms = 0.9f;
    REQUIRE(bus.read().rms == Approx(0.1f));

    writer.publishWrite();
    REQUIRE(bus.read().rms == Approx(0.9f));
}

TEST_CASE("FeatureBus createWriter enforces the single-writer claim", "[featurebus]")
{
    FeatureBus bus;

    FeatureBus::Writer w1 = bus.createWriter();
    REQUIRE(w1.isValid());

    // R4: a second claim fails deterministically — invalid, inert handle
    FeatureBus::Writer w2 = bus.createWriter();
    REQUIRE_FALSE(w2.isValid());
    REQUIRE(w2.acquireWrite() == nullptr);
    w2.publishWrite();  // no-op, must not publish anything
    FeatureSnapshot out;
    uint64_t lastSeq = 0;
    REQUIRE_FALSE(bus.readIfNewer(out, lastSeq));

    // Moving the handle transfers the claim; the moved-from handle is inert
    FeatureBus::Writer w3 = std::move(w1);
    REQUIRE(w3.isValid());
    REQUIRE_FALSE(w1.isValid());
    REQUIRE(w1.acquireWrite() == nullptr);

    // While the claim is held (by w3), claims still fail
    REQUIRE_FALSE(bus.createWriter().isValid());

    // Destroying the live handle releases the claim for a fresh one
    {
        FeatureBus::Writer held = std::move(w3);
    }
    FeatureBus::Writer w4 = bus.createWriter();
    REQUIRE(w4.isValid());
}

TEST_CASE("FeatureBus concurrent write-read stress test", "[featurebus]")
{
    FeatureBus bus;
    FeatureBus::Writer writer = bus.createWriter();
    REQUIRE(writer.isValid());
    constexpr int kIterations = 100000;

    std::atomic<bool> done{false};
    std::atomic<int> readCount{0};
    std::atomic<float> lastReadRms{0.0f};

    // Writer thread: writes incrementing RMS values
    std::thread writerThread([&]()
    {
        for (int i = 1; i <= kIterations; ++i)
        {
            FeatureSnapshot* ws = writer.acquireWrite();
            ws->rms = static_cast<float>(i);
            ws->timestamp = static_cast<uint64_t>(i);
            writer.publishWrite();
        }
        done.store(true, std::memory_order_release);
    });

    // Reader thread: fresh copies must be monotonically non-decreasing
    std::thread reader([&]()
    {
        float prevRms = 0.0f;
        FeatureSnapshot snap;
        uint64_t lastSeq = 0;
        while (!done.load(std::memory_order_acquire))
        {
            if (bus.readIfNewer(snap, lastSeq))
            {
                // Values should be monotonically non-decreasing
                // (we might skip values but should never go backwards)
                REQUIRE(snap.rms >= prevRms);
                prevRms = snap.rms;
                readCount.fetch_add(1, std::memory_order_relaxed);
            }
            else
            {
                std::this_thread::yield();
            }
        }
        // Drain the final publish
        if (bus.readIfNewer(snap, lastSeq))
        {
            REQUIRE(snap.rms >= prevRms);
            prevRms = snap.rms;
            readCount.fetch_add(1, std::memory_order_relaxed);
        }
        lastReadRms.store(prevRms, std::memory_order_relaxed);
    });

    writerThread.join();
    reader.join();

    // Reader should have consumed at least some snapshots
    REQUIRE(readCount.load() > 0);

    // Last value read should be <= kIterations
    REQUIRE(lastReadRms.load() <= static_cast<float>(kIterations));
}

// =============================================================================
// S2 multi-reader concurrency gate (featurebus-thread-safety-design.md R9).
// 1 writer + 2 GL-cadence readers + 3 timer-cadence readers, all concurrent.
// The writer stamps rms == peak == float(i) and timestamp == i on every
// publish; readers assert intra-snapshot coherence (rms == peak, timestamp
// matching) and non-decreasing timestamps across successive reads. The
// single-reader stress case above is structurally blind to a multi-reader
// race; this case is not — it demonstrated both TSan data races and torn
// reads against the old triple-buffer protocol (.harmony/s2-tsan-before.log)
// and must stay TSan-clean against the seqlock.
//
// Catch2 REQUIRE is not thread-safe, so reader threads record violations in
// atomic counters and the main thread asserts after join.
// =============================================================================

TEST_CASE("FeatureBus multi-reader coherence under concurrent publish", "[featurebus]")
{
    FeatureBus bus;
    FeatureBus::Writer writer = bus.createWriter();
    REQUIRE(writer.isValid());
    constexpr int kIterations = 1000;
    constexpr int kGlReaders = 2;      // GL-cadence: continuous redraw loop
    constexpr int kTimerReaders = 3;   // timer-cadence: poll, yield when stale
    // Publish cadence: production publishes every 10.67 ms (one hop at
    // 48 kHz); 2 ms here keeps the test fast while preserving the design's
    // coherence contract — the bounded 4-attempt retry only exhausts when 4
    // publish STARTS land inside one read's attempts, impossible when the
    // publish period exceeds a few copy durations (even under TSan's ~10x
    // slowdown). A zero-gap saturating writer is OUT of the ratified
    // contract (R2's bounded retry deliberately degrades, never blocks), so
    // any tear at this cadence is a genuine protocol bug.
    constexpr auto kPublishPeriod = std::chrono::milliseconds(2);

    std::atomic<bool> done{false};
    std::atomic<int> tearViolations{0};       // mixed-generation fields in one copy
    std::atomic<int> stampViolations{0};      // timestamp != uint64(rms)
    std::atomic<int> monotonicViolations{0};  // timestamp went backwards
    std::atomic<int> totalReads{0};

    // Writer thread: analysis-thread stand-in. The real writer fills many
    // fields one by one across a long pipeline (AnalysisThread.cpp stages
    // 1-14) — all stamped == i here so readers can detect any mixing.
    std::thread writerThread([&]()
    {
        for (int i = 1; i <= kIterations; ++i)
        {
            const auto f = static_cast<float>(i);
            FeatureSnapshot* ws = writer.acquireWrite();
            ws->timestamp = static_cast<uint64_t>(i);
            ws->rms = f;
            ws->peak = f;
            for (int b = 0; b < 7; ++b)  ws->bandEnergies[b] = f;
            for (int c = 0; c < 12; ++c) ws->chromagram[c] = f;
            for (int m = 0; m < 13; ++m) ws->mfccs[m] = f;
            ws->wallClockSeconds = static_cast<double>(i);
            writer.publishWrite();
            std::this_thread::sleep_for(kPublishPeriod);
        }
        done.store(true, std::memory_order_release);
    });

    // Shared coherence check on a caller-owned copy
    auto check = [&](const FeatureSnapshot& local, uint64_t& prevStamp)
    {
        const float r = local.rms;
        int mixed = 0;
        for (int b = 0; b < 7; ++b)  mixed += (local.bandEnergies[b] != r);
        for (int c = 0; c < 12; ++c) mixed += (local.chromagram[c] != r);
        for (int m = 0; m < 13; ++m) mixed += (local.mfccs[m] != r);

        if (r != local.peak || mixed != 0
            || local.wallClockSeconds != static_cast<double>(local.timestamp))
            tearViolations.fetch_add(1, std::memory_order_relaxed);
        if (local.timestamp != static_cast<uint64_t>(r))
            stampViolations.fetch_add(1, std::memory_order_relaxed);
        if (local.timestamp < prevStamp)
            monotonicViolations.fetch_add(1, std::memory_order_relaxed);
        prevStamp = local.timestamp;
        totalReads.fetch_add(1, std::memory_order_relaxed);
    };

    // GL-cadence readers: latest-value read() every loop, like the renderers
    auto glReaderBody = [&]()
    {
        uint64_t prevStamp = 0;
        while (!done.load(std::memory_order_acquire))
        {
            const FeatureSnapshot local = bus.read();
            check(local, prevStamp);
        }
    };

    // Timer-cadence readers: freshness-gated readIfNewer, yielding when stale
    auto timerReaderBody = [&]()
    {
        uint64_t prevStamp = 0;
        uint64_t lastSeq = 0;
        FeatureSnapshot local;
        while (!done.load(std::memory_order_acquire))
        {
            if (bus.readIfNewer(local, lastSeq))
                check(local, prevStamp);
            else
                std::this_thread::yield();
        }
    };

    std::vector<std::thread> readers;
    for (int r = 0; r < kGlReaders; ++r)
        readers.emplace_back(glReaderBody);
    for (int r = 0; r < kTimerReaders; ++r)
        readers.emplace_back(timerReaderBody);

    writerThread.join();
    for (auto& t : readers)
        t.join();

    REQUIRE(totalReads.load() > 0);
    REQUIRE(tearViolations.load() == 0);
    REQUIRE(stampViolations.load() == 0);
    REQUIRE(monotonicViolations.load() == 0);
}

TEST_CASE("FeatureSnapshot clear restores struct-default genre/energy", "[featuresnapshot]")
{
    FeatureSnapshot fs;

    // Dirty the genre/energy fields away from their struct defaults.
    fs.detectedGenre = 2;   // DnB
    fs.energyState   = 0;   // low
    fs.rms           = 0.9f;

    fs.clear();

    // A cleared snapshot must carry the struct defaults, not memset zeros.
    REQUIRE(fs.detectedGenre == 6);   // Pop/Electronic
    REQUIRE(fs.energyState   == 1);   // medium
    // Sanity: memset-zeroed fields are actually zeroed.
    REQUIRE(fs.rms == Approx(0.0f));
}
