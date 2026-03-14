#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "features/Smoother.h"
#include <cmath>

using Catch::Approx;

// === EMA Smoother Tests ===

TEST_CASE("Smoother first sample passes through unchanged", "[smoother]")
{
    Smoother s(0.3f);
    REQUIRE(s.process(1.0f) == Approx(1.0f));
}

TEST_CASE("Smoother alpha=1 is passthrough", "[smoother]")
{
    Smoother s(1.0f);
    s.process(0.0f);

    REQUIRE(s.process(1.0f) == Approx(1.0f));
    REQUIRE(s.process(0.5f) == Approx(0.5f));
    REQUIRE(s.process(0.0f) == Approx(0.0f));
}

TEST_CASE("Smoother converges to constant input", "[smoother]")
{
    Smoother s(0.1f);  // slow smoothing
    float target = 0.8f;

    // Feed constant value many times
    float val = 0.0f;
    for (int i = 0; i < 200; ++i)
        val = s.process(target);

    REQUIRE(val == Approx(target).margin(0.001f));
}

TEST_CASE("Smoother with low alpha is slower than high alpha", "[smoother]")
{
    Smoother slow(0.05f);
    Smoother fast(0.5f);

    slow.process(0.0f);
    fast.process(0.0f);

    // Step input of 1.0
    float slowVal = slow.process(1.0f);
    float fastVal = fast.process(1.0f);

    // Fast smoother should be closer to 1.0
    REQUIRE(fastVal > slowVal);
}

TEST_CASE("Smoother reset clears state", "[smoother]")
{
    Smoother s(0.3f);
    s.process(100.0f);
    s.process(100.0f);

    REQUIRE(s.value() == Approx(100.0f));

    s.reset();
    REQUIRE(s.value() == Approx(0.0f));

    // After reset, first sample passes through
    REQUIRE(s.process(5.0f) == Approx(5.0f));
}

TEST_CASE("Smoother setAlpha changes behavior", "[smoother]")
{
    Smoother s(0.01f);
    s.process(0.0f);
    float slowResult = s.process(1.0f);

    s.reset();
    s.setAlpha(0.9f);
    s.process(0.0f);
    float fastResult = s.process(1.0f);

    REQUIRE(fastResult > slowResult);
}

TEST_CASE("Smoother value() returns current without advancing", "[smoother]")
{
    Smoother s(0.5f);
    s.process(1.0f);
    float v1 = s.value();
    float v2 = s.value();
    REQUIRE(v1 == Approx(v2));
}

// === One-Euro Filter Tests ===

TEST_CASE("OneEuroFilter first sample passes through", "[oneeuro]")
{
    OneEuroFilter f;
    REQUIRE(f.process(0.5f) == Approx(0.5f));
}

TEST_CASE("OneEuroFilter converges to constant input", "[oneeuro]")
{
    OneEuroFilter f(93.75f, 1.0f, 0.007f, 1.0f);
    float target = 0.6f;

    float val = 0.0f;
    for (int i = 0; i < 500; ++i)
        val = f.process(target);

    REQUIRE(val == Approx(target).margin(0.01f));
}

TEST_CASE("OneEuroFilter smooths slow input", "[oneeuro]")
{
    OneEuroFilter f(93.75f, 1.0f, 0.007f, 1.0f);

    f.process(0.0f);
    // Slow change: 0 → 0.01 — should be heavily smoothed
    float result = f.process(0.01f);

    // Should be closer to 0 than to 0.01 (smoothed)
    REQUIRE(result < 0.008f);
}

TEST_CASE("OneEuroFilter reacts faster to rapid changes", "[oneeuro]")
{
    OneEuroFilter f(93.75f, 1.0f, 0.5f, 1.0f);  // high beta = fast response to speed

    f.process(0.0f);
    // Rapid jump
    float result = f.process(1.0f);

    // With high beta, the filter should track more closely on fast moves
    REQUIRE(result > 0.1f);
}

TEST_CASE("OneEuroFilter reset clears state", "[oneeuro]")
{
    OneEuroFilter f;
    f.process(100.0f);
    f.process(100.0f);

    f.reset();
    REQUIRE(f.value() == Approx(0.0f));

    // After reset, first sample passes through
    REQUIRE(f.process(5.0f) == Approx(5.0f));
}
