#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "features/FeatureBus.h"
#include <thread>
#include <atomic>
#include <vector>

using Catch::Approx;

TEST_CASE("FeatureBus initial state — no new data", "[featurebus]")
{
    FeatureBus bus;
    REQUIRE_FALSE(bus.hasNewData());
    REQUIRE(bus.acquireRead() == nullptr);
}

TEST_CASE("FeatureBus single write-read cycle", "[featurebus]")
{
    FeatureBus bus;

    // Write a snapshot
    FeatureSnapshot* ws = bus.acquireWrite();
    REQUIRE(ws != nullptr);
    ws->rms = 0.75f;
    ws->spectralCentroid = 440.0f;
    ws->bpm = 120.0f;
    bus.publishWrite();

    REQUIRE(bus.hasNewData());

    // Read it back
    const FeatureSnapshot* rs = bus.acquireRead();
    REQUIRE(rs != nullptr);
    REQUIRE(rs->rms == Approx(0.75f));
    REQUIRE(rs->spectralCentroid == Approx(440.0f));
    REQUIRE(rs->bpm == Approx(120.0f));

    // No new data after read
    REQUIRE_FALSE(bus.hasNewData());
    REQUIRE(bus.acquireRead() == nullptr);
}

TEST_CASE("FeatureBus getLatestRead returns last consumed snapshot", "[featurebus]")
{
    FeatureBus bus;

    // Write and read
    FeatureSnapshot* ws = bus.acquireWrite();
    ws->rms = 0.5f;
    bus.publishWrite();

    const FeatureSnapshot* rs = bus.acquireRead();
    REQUIRE(rs != nullptr);

    // getLatestRead should return the same snapshot even without new data
    const FeatureSnapshot* latest = bus.getLatestRead();
    REQUIRE(latest != nullptr);
    REQUIRE(latest->rms == Approx(0.5f));
}

TEST_CASE("FeatureBus multiple writes before read — reader gets latest", "[featurebus]")
{
    FeatureBus bus;

    // Write three times without reading
    for (int i = 1; i <= 3; ++i)
    {
        FeatureSnapshot* ws = bus.acquireWrite();
        ws->rms = static_cast<float>(i) * 0.1f;
        bus.publishWrite();
    }

    // Reader should get the most recent write (0.3)
    const FeatureSnapshot* rs = bus.acquireRead();
    REQUIRE(rs != nullptr);
    REQUIRE(rs->rms == Approx(0.3f));
}

TEST_CASE("FeatureBus writer does not clobber reader's buffer", "[featurebus]")
{
    FeatureBus bus;

    // Write and read
    FeatureSnapshot* ws = bus.acquireWrite();
    ws->rms = 0.5f;
    bus.publishWrite();

    const FeatureSnapshot* rs = bus.acquireRead();
    REQUIRE(rs != nullptr);
    REQUIRE(rs->rms == Approx(0.5f));

    // Write two more times — should not affect the reader's held pointer
    for (int i = 0; i < 2; ++i)
    {
        ws = bus.acquireWrite();
        ws->rms = 0.99f;
        bus.publishWrite();
    }

    // Reader's old pointer should still have 0.5 (writer can't touch read slot)
    REQUIRE(rs->rms == Approx(0.5f));
}

TEST_CASE("FeatureBus all slots use distinct indices", "[featurebus]")
{
    FeatureBus bus;

    // Write first snapshot
    FeatureSnapshot* w1 = bus.acquireWrite();
    w1->rms = 0.1f;
    bus.publishWrite();

    // Write second snapshot
    FeatureSnapshot* w2 = bus.acquireWrite();
    w2->rms = 0.2f;
    bus.publishWrite();

    // Read — gets 0.2
    const FeatureSnapshot* r = bus.acquireRead();
    REQUIRE(r != nullptr);
    REQUIRE(r->rms == Approx(0.2f));

    // Write third — should not alias with reader's slot
    FeatureSnapshot* w3 = bus.acquireWrite();
    REQUIRE(w3 != r);  // different pointers
}

TEST_CASE("FeatureBus concurrent write-read stress test", "[featurebus]")
{
    FeatureBus bus;
    constexpr int kIterations = 100000;

    std::atomic<bool> done{false};
    std::atomic<int> readCount{0};
    std::atomic<float> lastReadRms{0.0f};

    // Writer thread: writes incrementing RMS values
    std::thread writer([&]()
    {
        for (int i = 1; i <= kIterations; ++i)
        {
            FeatureSnapshot* ws = bus.acquireWrite();
            ws->rms = static_cast<float>(i);
            ws->timestamp = static_cast<uint64_t>(i);
            bus.publishWrite();
        }
        done.store(true, std::memory_order_release);
    });

    // Reader thread: reads and verifies monotonically increasing values
    std::thread reader([&]()
    {
        float prevRms = 0.0f;
        while (!done.load(std::memory_order_acquire) || bus.hasNewData())
        {
            const FeatureSnapshot* rs = bus.acquireRead();
            if (rs != nullptr)
            {
                // Values should be monotonically non-decreasing
                // (we might skip values but should never go backwards)
                REQUIRE(rs->rms >= prevRms);
                prevRms = rs->rms;
                readCount.fetch_add(1, std::memory_order_relaxed);
            }
            else
            {
                std::this_thread::yield();
            }
        }
        lastReadRms.store(prevRms, std::memory_order_relaxed);
    });

    writer.join();
    reader.join();

    // Reader should have consumed at least some snapshots
    REQUIRE(readCount.load() > 0);

    // Last value read should be <= kIterations
    REQUIRE(lastReadRms.load() <= static_cast<float>(kIterations));
}
