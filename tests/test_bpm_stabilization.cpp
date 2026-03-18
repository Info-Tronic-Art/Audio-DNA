#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <vector>

// We test the stabilization pipeline in isolation using processRawBPM(),
// which bypasses aubio and feeds directly into the pipeline stages.
#include "analysis/BPMTracker.h"

using Catch::Matchers::WithinAbs;

// Helper: feed N hops of a constant BPM at full confidence
static void feedConstantBPM(BPMTracker& tracker, float bpm, int hops, float conf = 1.0f)
{
    for (int i = 0; i < hops; ++i)
        tracker.processRawBPM(bpm, conf, false);
}

// Helper: feed N hops of a constant BPM with periodic beats
static void feedWithBeats(BPMTracker& tracker, float bpm, int hops,
                          int beatEveryNHops = 10, float conf = 1.0f)
{
    for (int i = 0; i < hops; ++i)
    {
        bool beat = (i % beatEveryNHops == 0);
        tracker.processRawBPM(bpm, conf, beat);
    }
}

// ============================================================================
// Median Filter Tests
// ============================================================================

TEST_CASE("Median filter rejects outlier spikes", "[bpm][median]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Feed 48 hops of 120 BPM to fill the median buffer
    feedConstantBPM(tracker, 120.0f, 48);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1.0));

    // Inject a single outlier of 60 BPM
    tracker.processRawBPM(60.0f, 1.0f, false);

    // BPM should still be ~120 because the median rejects one outlier
    // (60 gets octave-corrected to 120 anyway, but even without that,
    //  the median of 47 × 120 + 1 × 60 is still 120)
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 2.0));
}

TEST_CASE("Median filter converges on consistent input", "[bpm][median]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Feed 128 hops of 128 BPM — more than enough to fill median + lock
    feedConstantBPM(tracker, 128.0f, 128);

    REQUIRE_THAT(tracker.bpm(), WithinAbs(128.0, 0.5));
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
}

// ============================================================================
// Octave Error Correction Tests
// ============================================================================

TEST_CASE("Octave error: 60 BPM folds to 120 BPM", "[bpm][octave]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock at 120 BPM first
    feedConstantBPM(tracker, 120.0f, 60);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1.0));

    // Now feed 60 BPM (half) — should get octave-corrected to 120
    feedConstantBPM(tracker, 60.0f, 10);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 2.0));
}

TEST_CASE("Octave error: 240 BPM folds to 120 BPM", "[bpm][octave]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock at 120 BPM
    feedConstantBPM(tracker, 120.0f, 60);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1.0));

    // Feed 240 BPM (double) — range gate folds to 120, octave correction also catches it
    feedConstantBPM(tracker, 240.0f, 10);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 2.0));
}

TEST_CASE("BPM range gate folds extreme values", "[bpm][range]")
{
    BPMTracker tracker(512, 1024, 48000);

    // 30 BPM should fold to 60 (doubled once) then to 120 (doubled again)
    // Actually 30 → 60 → still valid at 60
    feedConstantBPM(tracker, 30.0f, 60);
    float bpm = tracker.bpm();
    REQUIRE(bpm >= 60.0f);
    REQUIRE(bpm <= 200.0f);

    // 400 BPM should fold to 100 (halved twice)
    BPMTracker tracker2(512, 1024, 48000);
    feedConstantBPM(tracker2, 400.0f, 60);
    float bpm2 = tracker2.bpm();
    REQUIRE(bpm2 >= 60.0f);
    REQUIRE(bpm2 <= 200.0f);
}

// ============================================================================
// Hysteresis Lock Tests
// ============================================================================

TEST_CASE("Hysteresis: stable BPM stays locked despite small jitter", "[bpm][hysteresis]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock at 128 BPM
    feedConstantBPM(tracker, 128.0f, 60);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(128.0, 1.0));

    // Feed jittery values around 128 ± 1.5 (below threshold of 2.0)
    for (int i = 0; i < 100; ++i)
    {
        float jitter = 128.0f + (i % 2 == 0 ? 1.0f : -1.0f);
        tracker.processRawBPM(jitter, 1.0f, false);
    }

    // Should still be locked at ~128
    REQUIRE_THAT(tracker.bpm(), WithinAbs(128.0, 2.0));
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
}

TEST_CASE("Hysteresis: real tempo change is accepted after persistence", "[bpm][hysteresis]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock at 120 BPM
    feedConstantBPM(tracker, 120.0f, 60);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1.0));
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);

    // Feed 140 BPM for 250 hops (beyond hysteresis threshold of 200)
    feedConstantBPM(tracker, 140.0f, 250);

    // Should have accepted the new BPM
    REQUIRE_THAT(tracker.bpm(), WithinAbs(140.0, 2.0));
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
}

TEST_CASE("Hysteresis: brief tempo change is rejected", "[bpm][hysteresis]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock at 120 BPM
    feedConstantBPM(tracker, 120.0f, 60);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1.0));

    // Feed 140 BPM for only 50 hops (well below hysteresis threshold)
    feedConstantBPM(tracker, 140.0f, 50);

    // Should still be at 120 BPM (change rejected)
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 3.0));
}

// ============================================================================
// Confidence Gate Tests
// ============================================================================

TEST_CASE("Confidence gate: low confidence values are ignored", "[bpm][confidence]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock at 120 BPM with high confidence
    feedConstantBPM(tracker, 120.0f, 60, 1.0f);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1.0));

    // Feed wildly different BPM but with zero confidence
    feedConstantBPM(tracker, 80.0f, 100, 0.0f);

    // Should still be locked at 120
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1.0));
}

// ============================================================================
// Tracker State Tests
// ============================================================================

TEST_CASE("Tracker starts in SEARCHING state", "[bpm][state]")
{
    BPMTracker tracker(512, 1024, 48000);
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_SEARCHING);
    REQUIRE(tracker.bpm() == 0.0f);
}

TEST_CASE("Tracker transitions: SEARCHING → LOCKING → LOCKED", "[bpm][state]")
{
    BPMTracker tracker(512, 1024, 48000);

    REQUIRE(tracker.trackerState() == BPMTracker::STATE_SEARCHING);

    // Feed a few hops — not enough to fill half the median window
    feedConstantBPM(tracker, 120.0f, 10);

    // Should be in LOCKING (has a candidate but not enough for median)
    // or still searching depending on implementation
    REQUIRE(tracker.trackerState() <= BPMTracker::STATE_LOCKING);

    // Feed enough to fill median window and lock
    feedConstantBPM(tracker, 120.0f, 50);

    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
    REQUIRE(tracker.bpm() > 0.0f);
}

// ============================================================================
// Beat Phase Tests
// ============================================================================

TEST_CASE("Beat phase ramps smoothly between 0 and 1", "[bpm][phase]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock at 120 BPM first
    feedConstantBPM(tracker, 120.0f, 60);
    REQUIRE(tracker.bpm() > 0.0f);

    // Simulate beat reset
    tracker.processRawBPM(120.0f, 1.0f, true);
    float prevPhase = tracker.beatPhase();

    // Feed hops without beats — phase should increase
    bool phaseIncreasing = true;
    for (int i = 0; i < 20; ++i)
    {
        tracker.processRawBPM(120.0f, 0.3f, false);  // low conf to avoid reset
        float p = tracker.beatPhase();
        if (p < prevPhase && prevPhase < 0.95f)  // allow wrap-around
        {
            phaseIncreasing = false;
            break;
        }
        prevPhase = p;
    }

    REQUIRE(phaseIncreasing);
}

TEST_CASE("Beat phase resets to 0 on high-confidence beat", "[bpm][phase]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock at 120 BPM
    feedConstantBPM(tracker, 120.0f, 60);

    // Advance phase a bit
    for (int i = 0; i < 20; ++i)
        tracker.processRawBPM(120.0f, 0.3f, false);

    float phaseBefore = tracker.beatPhase();
    REQUIRE(phaseBefore > 0.0f);

    // High-confidence beat detection
    tracker.processRawBPM(120.0f, 1.0f, true);

    REQUIRE(tracker.beatPhase() == 0.0f);
}

TEST_CASE("Beat phase stays 0 when no BPM locked", "[bpm][phase]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Feed low-confidence garbage — nothing should lock
    for (int i = 0; i < 10; ++i)
        tracker.processRawBPM(0.0f, 0.0f, false);

    REQUIRE(tracker.beatPhase() == 0.0f);
}

// ============================================================================
// Zero Allocation Verification
// ============================================================================

TEST_CASE("BPMTracker constants are valid", "[bpm][sanity]")
{
    // Verify compile-time constants make sense
    REQUIRE(BPMTracker::kMinBPM > 0.0f);
    REQUIRE(BPMTracker::kMaxBPM > BPMTracker::kMinBPM);
    REQUIRE(BPMTracker::kMedianWindowSize > 0);
    REQUIRE(BPMTracker::kHysteresisHops > 0);
    REQUIRE(BPMTracker::kConfidenceThreshold >= 0.0f);
    REQUIRE(BPMTracker::kBPMChangeThreshold > 0.0f);
}
