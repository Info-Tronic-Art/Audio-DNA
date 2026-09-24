#include <catch2/catch_test_macros.hpp>
#include "features/OnsetPulse.h"
#include <cstdint>

// Onset render-path fix: OnsetPulse turns consecutive FeatureSnapshot::onsetCount values
// (monotonic, one increment per detected onset hop) into a per-read delta, so a consumer
// polling the always-latest FeatureBus at ANY cadence neither misses an onset (slower than
// analysis) nor double-counts one (faster than analysis).

TEST_CASE("OnsetPulse: first consume only establishes the baseline", "[onsetpulse]")
{
    OnsetPulse p;
    REQUIRE_FALSE(p.primed());
    REQUIRE(p.consume(7u) == 0u);
    REQUIRE(p.primed());
}

TEST_CASE("OnsetPulse: same count twice is not a new onset (fast poller, no duplicate)", "[onsetpulse]")
{
    OnsetPulse p;
    p.consume(7u);
    REQUIRE(p.consume(7u) == 0u);
    REQUIRE(p.consume(7u) == 0u);
}

TEST_CASE("OnsetPulse: deltas of one and of several (missed hops recovered)", "[onsetpulse]")
{
    OnsetPulse p;
    p.consume(7u);
    REQUIRE(p.consume(8u) == 1u);
    REQUIRE(p.consume(11u) == 3u);
    REQUIRE(p.consume(11u) == 0u);
}

TEST_CASE("OnsetPulse: exact across a 2^32 wrap", "[onsetpulse]")
{
    OnsetPulse p;
    p.consume(0xFFFFFFFEu);
    REQUIRE(p.consume(1u) == 3u);
}

TEST_CASE("OnsetPulse: a backwards jump (writer reset) re-baselines without a pulse", "[onsetpulse]")
{
    OnsetPulse p;
    p.consume(57u);
    REQUIRE(p.consume(0u) == 0u);
    REQUIRE(p.consume(1u) == 1u);
}

TEST_CASE("OnsetPulse: reset() forgets the baseline", "[onsetpulse]")
{
    OnsetPulse p;
    p.consume(3u);
    p.reset();
    REQUIRE_FALSE(p.primed());
    REQUIRE(p.consume(100u) == 0u);
    REQUIRE(p.consume(101u) == 1u);
}

TEST_CASE("OnsetPulse: 60 Hz and 120 Hz readers of a 93.75 Hz always-latest counter see every "
          "onset exactly once", "[onsetpulse]")
{
    // Arithmetic sampling model, no bus: the writer publishes one hop every 512/48000 s
    // (93.75 Hz) and raises the monotonic count on every 47th hop (~2 onsets/s -- far apart
    // compared with the 1.6 hops/frame a 60 Hz reader spans, so no two onsets can merge into
    // one read). A reader polls the LATEST published count at its own frame rate.
    constexpr double hopSec = 512.0 / 48000.0;
    constexpr double durationSec = 20.0;
    constexpr int onsetEveryHops = 47;

    for (double fps : { 60.0, 120.0 })
    {
        OnsetPulse pulse;
        uint32_t latestCount = 0;
        uint32_t totalOnsets = 0;
        uint32_t deltaSum = 0;
        uint32_t pulseReads = 0;
        double nextFrame = 0.0;

        for (int hop = 0; (hop + 1) * hopSec <= durationSec; ++hop)
        {
            if (hop % onsetEveryHops == onsetEveryHops - 1)
            {
                ++latestCount;
                ++totalOnsets;
            }
            const double tPub = (hop + 1) * hopSec;
            while (nextFrame <= tPub)
            {
                const uint32_t d = pulse.consume(latestCount);
                deltaSum += d;
                if (d > 0u)
                    ++pulseReads;
                nextFrame += 1.0 / fps;
            }
        }

        INFO("fps=" << fps << " totalOnsets=" << totalOnsets << " deltaSum=" << deltaSum
                    << " pulseReads=" << pulseReads);
        REQUIRE(totalOnsets > 30u);
        REQUIRE(deltaSum == totalOnsets);
        REQUIRE(pulseReads == totalOnsets);
    }
}

// Test-mode feature injection's onsetCount rule (Harmony ruling, s-rta-0924b): the count is
// derived in ONE place, under TestServer::injectSnapshot's lock, from the previously
// injected count and the request's intent. Mirrors AnalysisThread: one increment per
// published snapshot whose onsetDetected the writer set.
TEST_CASE("InjectedOnsetCount: carry, bump, explicit and reset", "[onsetpulse][inject]")
{
    InjectedOnsetCount carry;                  // request without onsetDetected/onsetCount
    REQUIRE(carry.resolve(41u) == 41u);

    InjectedOnsetCount bump;                   // request with "onsetDetected": true
    bump.bump = true;
    REQUIRE(bump.resolve(41u) == 42u);

    InjectedOnsetCount explicitCount;          // request with "onsetCount": 10
    explicitCount.hasExplicit = true;
    explicitCount.explicitCount = 10u;
    explicitCount.bump = true;                 // explicit count wins over the bump
    REQUIRE(explicitCount.resolve(41u) == 10u);

    const InjectedOnsetCount reset = InjectedOnsetCount::zero();   // TestServer reset
    REQUIRE(reset.resolve(41u) == 0u);

    // Two concurrent bump requests serialized under the lock land on +2, never +1:
    // each resolves against the count the previous one published.
    uint32_t published = 5u;
    published = bump.resolve(published);
    published = bump.resolve(published);
    REQUIRE(published == 7u);
}
