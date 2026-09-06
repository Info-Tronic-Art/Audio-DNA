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

// Helper: feed `numBeats` real synthetic onsets at `bpm`, driving both the
// BPM pipeline (processRawBPM) and the downbeat scorer (feedDownbeatFeatures)
// on every hop -- mirrors AnalysisThread's per-hop call order. Bass energy is
// biased high whenever (startBeatIndex + local beat index) % kBeatsPerBar
// == 0, so the downbeat detector consistently locks position 0 as the
// downbeat. `startBeatIndex` lets callers make consecutive calls continue
// the same bias phase (so a second call doesn't look like a phase shift to
// the already-locked downbeat position).
static void feedRealOnsets(BPMTracker& tracker, float bpm, int numBeats,
                           int startBeatIndex = 0, int sampleRate = 48000, int hopSize = 512,
                           uint8_t structuralState = 0)
{
    int hopsPerBeat = static_cast<int>(std::lround(
        (static_cast<double>(sampleRate) * 60.0) / (static_cast<double>(bpm) * hopSize)));

    for (int b = 0; b < numBeats; ++b)
    {
        int beatIndex = startBeatIndex + b;
        for (int h = 0; h < hopsPerBeat; ++h)
        {
            bool beat = (h == 0);
            tracker.processRawBPM(bpm, 1.0f, beat);
            float bass = (beat && (beatIndex % BPMTracker::kBeatsPerBar) == 0) ? 1.0f : 0.0f;
            tracker.feedDownbeatFeatures(bass, 0.0f, 0.0f, structuralState);
        }
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

// ============================================================================
// P24: Predicted Beat Advance (silence / manual mode must not freeze
// beatInBar/barCount) -- see BPMTracker::advancePredictedBeat().
//
// Nothing above this point exercises feedSilenceDetection(), isSilent(), or
// downbeat/phrase state at all -- this is the first coverage of that path.
// ============================================================================

TEST_CASE("Silence: beatInBar and barCount keep advancing off the predicted phase wrap",
          "[bpm][silence][p24]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock BPM and downbeat position with real onsets first (mirrors a track
    // that was already playing and tracked before silence hits).
    feedRealOnsets(tracker, 120.0f, 24);
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
    REQUIRE(tracker.downbeatLocked());

    uint16_t barCountBeforeSilence = tracker.barCount();

    // Feed ~8 seconds of real silence (RMS held below threshold via
    // feedSilenceDetection, mirroring AnalysisThread's call order). At
    // 120 BPM that is 16 beats == 4 bars.
    const float hopsPerSec = 48000.0f / 512.0f;
    const int silenceHops = static_cast<int>(8.0f * hopsPerSec);

    bool seenBeatInBar[BPMTracker::kBeatsPerBar] = {};
    for (int i = 0; i < silenceHops; ++i)
    {
        tracker.feedSilenceDetection(0.0001f); // well below silenceRmsThreshold_ (0.005)
        tracker.processRawBPM(120.0f, 1.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
        seenBeatInBar[tracker.beatInBar()] = true;
    }

    REQUIRE(tracker.isSilent());

    // Not frozen: every position in the bar was visited during silence.
    for (int p = 0; p < BPMTracker::kBeatsPerBar; ++p)
        REQUIRE(seenBeatInBar[p]);

    // ~4 bars advanced over ~8 seconds of held silence at 120 BPM.
    uint16_t barsAdvanced = static_cast<uint16_t>(tracker.barCount() - barCountBeforeSilence);
    REQUIRE(barsAdvanced >= 3);
    REQUIRE(barsAdvanced <= 5);
}

TEST_CASE("Manual mode: beatInBar and barCount advance from cold with no audio",
          "[bpm][manual][p24]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Tap tempo from cold -- no prior onsets, no downbeat lock, no audio.
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);
    REQUIRE(tracker.isManualMode());
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 0.5));
    REQUIRE_FALSE(tracker.downbeatLocked());

    const float hopsPerSec = 48000.0f / 512.0f;
    const int hops = static_cast<int>(8.0f * hopsPerSec); // ~8s == 16 beats == 4 bars at 120 BPM

    uint16_t barCountStart = tracker.barCount();
    bool seenBeatInBar[BPMTracker::kBeatsPerBar] = {};
    for (int i = 0; i < hops; ++i)
    {
        // "No audio at all" -- aubio would report no usable raw BPM/confidence/beat.
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
        seenBeatInBar[tracker.beatInBar()] = true;
    }

    // Not frozen: every position in the bar was visited, with no onsets and
    // no downbeat lock ever established.
    for (int p = 0; p < BPMTracker::kBeatsPerBar; ++p)
        REQUIRE(seenBeatInBar[p]);

    uint16_t barsAdvanced = static_cast<uint16_t>(tracker.barCount() - barCountStart);
    REQUIRE(barsAdvanced >= 3);
    REQUIRE(barsAdvanced <= 5);
}

TEST_CASE("Anti-double-count: 16 live onsets advance barCount by exactly 4, not 8",
          "[bpm][downbeat][p24]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Establish BPM + downbeat lock first (needs >= kDownbeatLockThreshold beats).
    feedRealOnsets(tracker, 120.0f, 24);
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
    REQUIRE(tracker.downbeatLocked());

    uint16_t barCountBefore = tracker.barCount();

    // Feed exactly 16 more live onsets, continuing the same bias phase (no
    // relock churn). If the predicted-wrap path were active in parallel with
    // scoreBeat() during ordinary locked playback, this would double-count
    // to 8 bars instead of 4 -- the exact hazard this pipeline must avoid.
    feedRealOnsets(tracker, 120.0f, 16, /*startBeatIndex=*/24);

    uint16_t barsAdvanced = static_cast<uint16_t>(tracker.barCount() - barCountBefore);
    REQUIRE(barsAdvanced == 4);
}

TEST_CASE("Pre-existing BPM/beat-phase tests are unaffected by predicted beat advance",
          "[bpm][regression][p24]")
{
    // Sanity check that the P24 gating (predictedBeatRegime_) doesn't leak
    // into ordinary locked playback: repeat the existing hysteresis-lock
    // scenario and confirm behavior is identical to before.
    BPMTracker tracker(512, 1024, 48000);

    feedConstantBPM(tracker, 120.0f, 60);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1.0));
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);

    feedConstantBPM(tracker, 140.0f, 250);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(140.0, 2.0));
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
}

// ============================================================================
// Structural phrase-reset gating: updatePhrase()'s structural-transition
// reset branch (entering drop / leaving breakdown) must not fire while a
// real onset cannot arrive (predictedBeatRegime_) -- StructuralDetector
// keeps classifying off live RMS/flux/onset-rate the whole time, so a
// "drop"/"breakdown" it infers from room noise during silence/manual mode
// is meaningless and must not corrupt barCount_. A real transition during
// real playback must still reset it -- that's a genuine feature, not a bug.
// ============================================================================

TEST_CASE("Manual mode: structural transition into drop does not reset barCount",
          "[bpm][phrase][structural]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);

    const float hopsPerSec = 48000.0f / 512.0f;
    const int hopsPerPhase = static_cast<int>(4.0f * hopsPerSec); // ~4s, no audio at all

    uint16_t maxBarCountSeen = 0;
    bool sawDecrease = false;

    // Phase A: structuralState == 0 (normal) -- build up some bar progress
    // purely off the predicted phase wrap (no real onsets exist at all).
    for (int i = 0; i < hopsPerPhase; ++i)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, /*structuralState=*/0);
        uint16_t bc = tracker.barCount();
        if (bc < maxBarCountSeen) sawDecrease = true;
        if (bc > maxBarCountSeen) maxBarCountSeen = bc;
    }
    uint16_t barCountBeforeTransition = tracker.barCount();
    REQUIRE(barCountBeforeTransition > 0);

    // Phase B: structuralState flips to drop (2) on its very first hop --
    // exactly the transition updatePhrase() used to reset unconditionally.
    // Still manual mode, still no real onset possible.
    for (int i = 0; i < hopsPerPhase; ++i)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, /*structuralState=*/2);
        uint16_t bc = tracker.barCount();
        if (bc < maxBarCountSeen) sawDecrease = true;
        if (bc > maxBarCountSeen) maxBarCountSeen = bc;
    }

    REQUIRE_FALSE(sawDecrease); // never dipped below its running max -- no reset happened
    REQUIRE(tracker.barCount() > barCountBeforeTransition); // kept advancing through the transition
}

TEST_CASE("Manual mode: structural transition out of breakdown does not reset barCount",
          "[bpm][phrase][structural]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);

    const float hopsPerSec = 48000.0f / 512.0f;
    const int hopsPerPhase = static_cast<int>(4.0f * hopsPerSec);

    // Phase A: structuralState == 3 (breakdown) from the start -- build up
    // bar progress while "in breakdown" (entering breakdown itself is not a
    // reset transition).
    for (int i = 0; i < hopsPerPhase; ++i)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, /*structuralState=*/3);
    }
    uint16_t barCountBeforeTransition = tracker.barCount();
    REQUIRE(barCountBeforeTransition > 0);

    uint16_t maxBarCountSeen = barCountBeforeTransition;
    bool sawDecrease = false;

    // Phase B: leaves breakdown (3 -> 0) on its very first hop -- the other
    // unconditional reset branch (prevStructuralState_ == 3 && state != 3).
    for (int i = 0; i < hopsPerPhase; ++i)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, /*structuralState=*/0);
        uint16_t bc = tracker.barCount();
        if (bc < maxBarCountSeen) sawDecrease = true;
        if (bc > maxBarCountSeen) maxBarCountSeen = bc;
    }

    REQUIRE_FALSE(sawDecrease);
    REQUIRE(tracker.barCount() > barCountBeforeTransition);
}

TEST_CASE("Real audio: structural transition into drop still resets barCount",
          "[bpm][phrase][structural][regression]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Real onsets, normal structural state -- lock BPM/downbeat and build up
    // bar progress the way real playback would.
    feedRealOnsets(tracker, 120.0f, 24, /*startBeatIndex=*/0, 48000, 512, /*structuralState=*/0);
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
    REQUIRE(tracker.downbeatLocked());
    REQUIRE(tracker.barCount() > 0);

    // A real onset arrives on the very hop where structuralState flips into
    // drop (2). predictedBeatRegime_ is false throughout (a real onset
    // arrives every beat), so the reset must still fire -- a real drop
    // inferred from real audio is exactly what this branch exists for.
    feedRealOnsets(tracker, 120.0f, 1, /*startBeatIndex=*/24, 48000, 512, /*structuralState=*/2);

    REQUIRE(tracker.barCount() == 0);
}

TEST_CASE("Real audio: structural transition into drop zeroes barCount while totalBarCount keeps climbing",
          "[bpm][phrase][structural][regression]")
{
    // S168: same scripted transition as "...still resets barCount" above,
    // now pinning totalBarCount_'s side of the trade-off -- it must NOT be
    // reset by the same event that zeroes barCount_. Checking
    // totalBarCount's growth across Phase A ALONE, before any reset is even
    // fed, is what makes this non-tautological: a mis-wired ++totalBarCount_
    // that only fires inside the structural-reset branch (instead of the
    // newBar branch, where it belongs) would leave totalBarCount() sitting
    // at 0 through the entire build-up below, failing the very first
    // totalBarCount assertion well before the transition is ever reached.
    BPMTracker tracker(512, 1024, 48000);

    feedRealOnsets(tracker, 120.0f, 24, /*startBeatIndex=*/0, 48000, 512, /*structuralState=*/0);
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
    REQUIRE(tracker.downbeatLocked());
    uint16_t barCountBeforeTransition = tracker.barCount();
    uint32_t totalBarCountBeforeTransition = tracker.totalBarCount();
    REQUIRE(barCountBeforeTransition > 0);
    REQUIRE(totalBarCountBeforeTransition > 0);
    // No reset has happened yet -- the two counters have advanced in exact
    // lockstep since both started at 0.
    REQUIRE(totalBarCountBeforeTransition == static_cast<uint32_t>(barCountBeforeTransition));

    // Same transition hop as the sibling test above: a real onset lands on
    // the very hop structuralState flips into drop (2).
    feedRealOnsets(tracker, 120.0f, 1, /*startBeatIndex=*/24, 48000, 512, /*structuralState=*/2);

    REQUIRE(tracker.barCount() == 0);                                  // zeroed, as before
    REQUIRE(tracker.totalBarCount() > totalBarCountBeforeTransition);  // kept climbing through the SAME event
}

// ============================================================================
// Coverage gap: predictedBeatRegime_ can be true (runPipeline's inSilence_
// branch) on the very hop a real, high-confidence onset also arrives --
// isSilent()'s own exit hysteresis takes several consecutive above-threshold
// hops to clear, so a beat can land before it does. That's the one case
// where feedDownbeatFeatures()'s existing `!predictedBeatRegime_` guard on
// scoreBeat() (as opposed to updatePhase()'s hard hasBeat hard-reset branch)
// actually matters -- nothing above this point ever exercises it.
// ============================================================================

TEST_CASE("Silence-exit hysteresis: a real onset arriving while still officially silent "
          "is not double-scored",
          "[bpm][silence][downbeat]")
{
    BPMTracker tracker(512, 1024, 48000);

    feedRealOnsets(tracker, 120.0f, 24);
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
    REQUIRE(tracker.downbeatLocked());

    // Enter held silence (>= silenceEntryHops_, ~300ms at 48000/512).
    const float hopsPerSec = 48000.0f / 512.0f;
    const int silenceEntryHops = static_cast<int>(0.3f * hopsPerSec) + 2; // small safety margin
    for (int i = 0; i < silenceEntryHops; ++i)
    {
        tracker.feedSilenceDetection(0.0001f); // well below silenceRmsThreshold_ (0.005)
        tracker.processRawBPM(120.0f, 1.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    }
    REQUIRE(tracker.isSilent());

    // Pin beatInBar_/beatCounter_/phase_ to a known state so the single test
    // hop below can't coincidentally straddle a predicted phase wrap --  that
    // would make advancePredictedBeat() a legitimate source of an advance
    // and mask whether scoreBeat() also (wrongly) ran.
    tracker.resetBeatPhase();
    REQUIRE(tracker.beatInBar() == 0);

    // One hop of real, high-confidence audio -- but isSilent()'s own exit
    // hysteresis (silenceExitHops_, several consecutive above-threshold
    // hops) hasn't elapsed after just one loud reading, so runPipeline's
    // inSilence_ branch still wins and predictedBeatRegime_ stays true even
    // though a genuine beat arrived this hop. Phase was just reset, so this
    // hop's tiny phase increment cannot itself wrap.
    tracker.feedSilenceDetection(1.0f); // loud -- starts the exit countdown, doesn't clear it yet
    REQUIRE(tracker.isSilent());
    tracker.processRawBPM(120.0f, 1.0f, true);
    tracker.feedDownbeatFeatures(1.0f, 1.0f, 1.0f, 0);

    // With the guard: neither advancePredictedBeat() (no wrap this hop) nor
    // scoreBeat() (skipped by !predictedBeatRegime_) advances the counter --
    // beatInBar stays 0. Without the guard, scoreBeat() would run
    // (downbeatLocked_ is already true) and advance it to 1.
    REQUIRE(tracker.beatInBar() == 0);
}
