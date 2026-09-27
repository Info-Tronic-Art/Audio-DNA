#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <atomic>
#include <cmath>
#include <thread>
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
    // s-rta-0926b: the tapped tempo is applied by the first hop (a request, not a direct
    // write), so it is checked after the run rather than right after the call.
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 0.5));
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
// s-rta-0925 (Boris ruling 2026-09-25, call 9): MANUAL Resync. requestResync()
// is a single relaxed atomic bump, callable from any thread; the analysis
// thread applies it (applyResync()) at the END of its next
// feedDownbeatFeatures() hop, AFTER updatePhrase() -- so a bar edge landing
// on that same hop is counted first, and the origin captures the
// post-increment totalBarCount(). The first three tests run in manual mode
// (predicted-phase-wrap regime, no audio); the fourth uses feedRealOnsets to
// exercise the real-onset regime and the automatic structural-reset branch.
// ============================================================================

TEST_CASE("requestResync is applied by the next analysis hop, not by the caller",
          "[bpm][resync][s-rta-0925]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);

    // Run from cold until mid-bar (beatInBar != 0, so downbeatDetected() is
    // false and beatPhase() > 0) with at least 2 bars elapsed.
    uint16_t bc0 = 0; uint8_t bib0 = 0; float ph0 = 0.0f;
    bool found = false;
    for (int i = 0; i < 500 && !found; ++i)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
        if (tracker.totalBarCount() >= 2 && tracker.beatInBar() != 0)
        {
            bc0 = tracker.barCount(); bib0 = tracker.beatInBar(); ph0 = tracker.beatPhase();
            found = true;
        }
    }
    REQUIRE(found);
    REQUIRE(ph0 > 0.0f);
    REQUIRE_FALSE(tracker.downbeatDetected());
    const uint32_t totalBarCountBefore = tracker.totalBarCount();

    tracker.requestResync();

    // The caller writes nothing -- state is untouched before the next hop.
    REQUIRE(tracker.barCount() == bc0);
    REQUIRE(tracker.beatInBar() == bib0);
    REQUIRE_THAT(tracker.beatPhase(), WithinAbs(ph0, 1e-6));
    REQUIRE(tracker.resyncBarOrigin() == 0);   // no Resync applied to this tracker yet

    // ONE hop: this hop's published state IS the new downbeat.
    tracker.processRawBPM(0.0f, 0.0f, false);
    tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);

    REQUIRE_THAT(tracker.beatPhase(), WithinAbs(0.0, 1e-4));   // RED on C0: keeps free-running from ph0
    REQUIRE(tracker.beatInBar() == 0);
    REQUIRE_THAT(tracker.barPhase(), WithinAbs(0.0, 1e-4));
    REQUIRE(tracker.barCount() == 0);
    REQUIRE_THAT(tracker.phrasePhase(), WithinAbs(0.0, 1e-4));
    REQUIRE(tracker.downbeatDetected());
    REQUIRE(tracker.resyncBarOrigin() == totalBarCountBefore);
    REQUIRE(tracker.totalBarCount() == totalBarCountBefore);   // S168: not rewound, no double-count this hop

    // Three more hops: applied once, then free-running again.
    for (int i = 0; i < 3; ++i)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    }
    REQUIRE(tracker.beatPhase() > 0.0f);
}

TEST_CASE("no phantom bar when the Resync lands during the first beat",
          "[bpm][resync][s-rta-0925]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);

    // Run from cold until JUST PAST the first bar wrap (~hop 188 at 120 BPM,
    // 187.5 hops/bar): totalBarCount becomes 1, beatInBar wraps to 0,
    // downbeatDetected() (the LEVEL) becomes true for the whole first beat.
    int hop = 0;
    for (; hop < 300; ++hop)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
        if (tracker.totalBarCount() == 1 && tracker.beatInBar() == 0)
            break;
    }
    INFO("wrap landed at hop " << hop);
    REQUIRE(hop >= 186);
    REQUIRE(hop <= 190);
    REQUIRE(tracker.downbeatDetected());

    // The Resync lands WHILE downbeatDetected_ is already true (mid-beat-1) --
    // this is the trap: an implementation that reuses resetPhrase()'s
    // prevDownbeatDetected_ = false would mint a phantom rising edge on the
    // very next hop (downbeatDetected_ still true, prevDownbeatDetected_
    // wrongly false again).
    tracker.requestResync();
    tracker.processRawBPM(0.0f, 0.0f, false);
    tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    REQUIRE(tracker.resyncBarOrigin() == 1);

    for (int i = 0; i < 10; ++i)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    }
    REQUIRE(tracker.totalBarCount() == 1);   // the resetPhrase()-reuse trap reads 2 here
    REQUIRE(tracker.barCount() == 0);
}

TEST_CASE("one bar after a Resync: bars since resync == 1, the phrase restarted, the origin held",
          "[bpm][resync][s-rta-0925]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);

    bool found = false;
    for (int i = 0; i < 500 && !found; ++i)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
        if (tracker.totalBarCount() >= 2 && tracker.beatInBar() != 0)
            found = true;
    }
    REQUIRE(found);

    tracker.requestResync();
    tracker.processRawBPM(0.0f, 0.0f, false);
    tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    const uint32_t origin = tracker.resyncBarOrigin();
    REQUIRE(origin > 0);

    int hop = 0; bool wrapped = false;
    for (; hop < 200; ++hop)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
        if (tracker.totalBarCount() == origin + 1) { wrapped = true; break; }
    }
    REQUIRE(wrapped);
    INFO("one bar after the Resync landed at hop " << hop);
    REQUIRE(hop >= 186);
    REQUIRE(hop <= 190);
    REQUIRE(tracker.beatInBar() == 0);
    REQUIRE(tracker.barCount() == 1);
    REQUIRE(tracker.downbeatDetected());
    REQUIRE_THAT(tracker.phrasePhase(), WithinAbs(1.0 / 8.0, 0.02));
    REQUIRE(tracker.resyncBarOrigin() == origin);
}

TEST_CASE("automatic structural reset leaves resyncBarOrigin untouched (real onsets)",
          "[bpm][resync][s-rta-0925][regression]")
{
    BPMTracker tracker(512, 1024, 48000);

    feedRealOnsets(tracker, 120.0f, 24, /*startBeatIndex=*/0, 48000, 512, /*structuralState=*/0);
    REQUIRE(tracker.trackerState() == BPMTracker::STATE_LOCKED);
    REQUIRE(tracker.downbeatLocked());

    // Manual Resync request, applied on the next hop -- which lands inside
    // this feedRealOnsets(1 beat) call. A real onset can still arrive
    // (predictedBeatRegime_ is false throughout), so this exercises the
    // real-audio path, not the predicted-phase-wrap regime the three tests
    // above exercise in manual mode.
    tracker.requestResync();
    feedRealOnsets(tracker, 120.0f, 1, /*startBeatIndex=*/24, 48000, 512, /*structuralState=*/0);

    REQUIRE(tracker.resyncBarOrigin() > 0);   // non-tautological: never set pre-fix, stays 0
    REQUIRE(tracker.resyncBarOrigin() == tracker.totalBarCount());
    const uint32_t origin = tracker.resyncBarOrigin();

    feedRealOnsets(tracker, 120.0f, 8, /*startBeatIndex=*/25, 48000, 512, /*structuralState=*/0);   // 25..32
    feedRealOnsets(tracker, 120.0f, 3, /*startBeatIndex=*/33, 48000, 512, /*structuralState=*/0);   // 33..35, filler (no downbeat in range)
    const uint32_t totalBarCountBeforeDrop = tracker.totalBarCount();

    // The drop-entry branch in updatePhrase() must still fire on a REAL
    // structural transition (ruling 25: automatic resets keep flowing,
    // unchanged) -- and must not touch resyncBarOrigin_ either way. Beat 36
    // is deliberately a downbeat position (36 % kBeatsPerBar == 0) so this
    // hop ALSO crosses a bar edge -- mirroring the existing sibling test's
    // exact pattern ("Real audio: structural transition into drop zeroes
    // barCount while totalBarCount keeps climbing", above) where the newBar
    // and the drop coincide on the same hop.
    feedRealOnsets(tracker, 120.0f, 1, /*startBeatIndex=*/36, 48000, 512, /*structuralState=*/2);

    REQUIRE(tracker.barCount() == 0);
    REQUIRE(tracker.totalBarCount() > totalBarCountBeforeDrop);
    REQUIRE(tracker.resyncBarOrigin() == origin);
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

// ============================================================================
// s-rta-0926 manual-bpm: in Manual BPM mode (TopBar "Manual" / set_bpm / OSC
// bpm / Link) the beat phase free-runs from the manual BPM. A detected beat
// (aubio's beat flag at confidence >= kBeatResetConfidence) must never move
// it -- the room playing something rhythmic at another tempo used to reset
// the phase to 0 on every detected beat, so at a foreign tempo faster than
// the manual one the phase never wrapped and bars stretched (live: 11-13 s
// bars at manual 120 BPM). The only manual realignments are Resync
// (requestResync) and Tap / a new set_bpm (setManualBPM). AUTO mode keeps its
// hard reset on high-confidence beats ("Beat phase resets to 0 on
// high-confidence beat", above).
// ============================================================================

namespace
{
    // Per-hop increment of the free-running phase at `bpm` (48 kHz, 512-sample hop).
    constexpr float manualPhaseInc(float bpm) { return 512.0f * bpm / (48000.0f * 60.0f); }

    // Feeds ONE manual-mode hop the way AnalysisThread does while the room plays
    // something rhythmic at `foreignBpm`: aubio keeps reporting a raw BPM at full
    // confidence and raises its beat flag once per foreign beat period. Returns
    // whether this hop carried a detected beat.
    struct ForeignBeatFeeder
    {
        float foreignBpm;
        float acc = 0.0f;
        bool hop(BPMTracker& tracker)
        {
            acc += manualPhaseInc(foreignBpm);
            const bool beat = (acc >= 1.0f);
            if (beat) acc -= 1.0f;
            tracker.processRawBPM(foreignBpm, 1.0f, beat);
            tracker.feedDownbeatFeatures(beat ? 1.0f : 0.0f, beat ? 1.0f : 0.0f, 0.0f, 0);
            return beat;
        }
    };

    // True when the phase moved by anything other than one free-running step
    // (modulo the wrap) -- i.e. something reset or nudged it this hop.
    bool phaseJumped(float prev, float now, float inc)
    {
        float step = now - prev;
        if (step < 0.0f) step += 1.0f;
        return std::fabs(step - inc) > 1e-4f;
    }
}

TEST_CASE("Manual mode: detected beats at another tempo never move the beat phase",
          "[bpm][manual][phase][s-rta-0926]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);

    const float inc = manualPhaseInc(120.0f);
    ForeignBeatFeeder room{ 142.0f };

    // ~20.3 s: 40 manual beats == 10 bars at 120 BPM; ~48 foreign beats at 142.
    constexpr int kHops = 1900;
    int foreignBeats = 0, jumps = 0, firstJumpHop = -1;
    std::vector<int> barHops;
    uint32_t bars = tracker.totalBarCount();
    float prev = tracker.beatPhase();
    for (int h = 0; h < kHops; ++h)
    {
        if (room.hop(tracker)) ++foreignBeats;
        const float now = tracker.beatPhase();
        if (phaseJumped(prev, now, inc)) { ++jumps; if (firstJumpHop < 0) firstJumpHop = h; }
        prev = now;
        if (tracker.totalBarCount() != bars) { bars = tracker.totalBarCount(); barHops.push_back(h); }
    }

    INFO("foreign beats fed " << foreignBeats << ", phase jumps " << jumps
         << " (first at hop " << firstJumpHop << "), bars " << barHops.size());
    REQUIRE(foreignBeats >= 40);                              // non-vacuous: the room really was beating
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1e-3));      // the manual tempo holds
    REQUIRE(jumps == 0);                                      // RED pre-fix: one reset per foreign beat
    REQUIRE(barHops.size() == 10);                            // RED pre-fix: bars stall (phase never wraps)
    for (size_t i = 1; i < barHops.size(); ++i)
    {
        INFO("bar " << i << " took " << (barHops[i] - barHops[i - 1]) << " hops (187.5 = 2.000 s)");
        REQUIRE(barHops[i] - barHops[i - 1] >= 187);
        REQUIRE(barHops[i] - barHops[i - 1] <= 188);
    }
}

TEST_CASE("Manual mode with detected beats at another tempo: Resync realigns once, then free-runs",
          "[bpm][manual][resync][s-rta-0926]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);

    const float inc = manualPhaseInc(120.0f);
    ForeignBeatFeeder room{ 142.0f };

    // Mid-bar, a couple of bars in, with the room beating the whole time.
    bool found = false;
    for (int i = 0; i < 1000 && !found; ++i)
    {
        room.hop(tracker);
        found = (tracker.totalBarCount() >= 2 && tracker.beatInBar() != 0);
    }
    REQUIRE(found);   // RED pre-fix: bars stall, totalBarCount never reaches 2
    REQUIRE(tracker.beatPhase() > 0.0f);

    tracker.requestResync();
    room.hop(tracker);   // ONE hop: its published state IS the new downbeat

    REQUIRE_THAT(tracker.beatPhase(), WithinAbs(0.0, 1e-4));
    REQUIRE(tracker.beatInBar() == 0);
    REQUIRE(tracker.barCount() == 0);
    REQUIRE(tracker.downbeatDetected());
    const uint32_t origin = tracker.resyncBarOrigin();
    REQUIRE(origin == tracker.totalBarCount());

    // After the single realignment the phase free-runs again despite the room,
    // and exactly one bar (187.5 hops) later the next bar lands.
    int jumps = 0, hop = 0;
    bool nextBar = false;
    float prev = tracker.beatPhase();
    for (; hop < 400 && !nextBar; ++hop)
    {
        room.hop(tracker);
        const float now = tracker.beatPhase();
        if (phaseJumped(prev, now, inc)) ++jumps;
        prev = now;
        nextBar = (tracker.totalBarCount() == origin + 1);
    }
    INFO("next bar after the Resync at hop " << hop << ", phase jumps " << jumps);
    REQUIRE(nextBar);
    REQUIRE(jumps == 0);
    REQUIRE(hop >= 186);
    REQUIRE(hop <= 190);
    REQUIRE(tracker.beatInBar() == 0);
    REQUIRE(tracker.barCount() == 1);
    REQUIRE(tracker.resyncBarOrigin() == origin);
}

TEST_CASE("Tap still realigns in manual mode; leaving manual mode restores the AUTO beat reset",
          "[bpm][manual][phase][s-rta-0926][regression]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);
    ForeignBeatFeeder room{ 142.0f };

    for (int i = 0; i < 20; ++i) room.hop(tracker);
    // Tap / a new set_bpm (setManualBPM) is a deliberate realignment: phase to 0 -- even
    // with the SAME BPM (120 again here). s-rta-0926b: applied by the next hop, which then
    // advances one step from 0.
    REQUIRE(tracker.beatPhase() > 0.0f);
    tracker.setManualBPM(120.0f);
    room.hop(tracker);
    REQUIRE_THAT(tracker.beatPhase(), WithinAbs(manualPhaseInc(120.0f), 1e-6));

    // Back to AUTO: a high-confidence detected beat hard-resets the phase again,
    // exactly as before (AUTO behaviour unchanged).
    tracker.setManualMode(false);
    REQUIRE_FALSE(tracker.isManualMode());
    for (int i = 0; i < 20; ++i)
        tracker.processRawBPM(120.0f, 0.3f, false);   // low confidence: no reset, phase runs
    REQUIRE(tracker.beatPhase() > 0.0f);
    tracker.processRawBPM(120.0f, 1.0f, true);
    REQUIRE(tracker.beatPhase() == 0.0f);
}

// ============================================================================
// s-rta-0926b bpm-thread: every tempo writer -- TopBar Tap and manual field,
// REST /api/set_bpm, OSC /audiodna/bpm, a Tap binding, take/routine replay,
// the Ableton Link tick -- reaches the tracker on the MESSAGE thread
// (MainComponent::applyTempoCommand), while the ANALYSIS thread reads and
// writes the same tempo/phase fields every hop. setManualBPM used to write
// lockedBPM_/phase_/... straight from the calling thread: a data race. It now
// posts a request that the analysis thread applies at the START of its next
// hop -- the requestResync pattern above -- so that hop publishes exactly what
// the old direct write published on its next hop.
// ============================================================================

namespace
{
    // One analysis hop in AnalysisThread's stage-5 order, with no audio (the manual regime).
    void quietHop(BPMTracker& tracker)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    }
}

TEST_CASE("setManualBPM (Tap / set_bpm) is applied by the next analysis hop, not by the caller",
          "[bpm][manual][thread][s-rta-0926b]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualBPM(120.0f);
    tracker.setManualMode(true);
    for (int i = 0; i < 30; ++i) quietHop(tracker);
    const float bpm0 = tracker.bpm();
    const float phase0 = tracker.beatPhase();
    REQUIRE_THAT(bpm0, WithinAbs(120.0, 1e-3));
    REQUIRE(phase0 > 0.1f);

    tracker.setManualBPM(140.0f);   // the message thread: TopBar Tap / REST or OSC set_bpm

    // The caller writes nothing the analysis thread owns. RED pre-fix: bpm 140 and
    // phase 0 were written straight from the calling thread (the data race).
    REQUIRE(tracker.bpm() == bpm0);
    REQUIRE(tracker.beatPhase() == phase0);

    // ONE hop applies it, and publishes what the old direct write published on its
    // next hop: the new tempo, and the phase one hop's increment from 0 (realigned).
    quietHop(tracker);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(140.0, 1e-3));
    REQUIRE_THAT(tracker.beatPhase(), WithinAbs(manualPhaseInc(140.0f), 1e-6));
}

TEST_CASE("tempo writers on the message thread vs the analysis hop: no data race (run it under ThreadSanitizer)",
          "[bpm][thread][tsan][s-rta-0926b]")
{
    // An ADNA_SANITIZE=thread build reports any unsynchronised access between the two
    // threads below and the test binary exits non-zero (RED pre-fix: setManualBPM wrote
    // lockedBPM_/phase_ while the analysis hop read and wrote them). In a plain build it
    // runs the same interleaving and checks the tracker still ends coherent.
    BPMTracker tracker(512, 1024, 48000);
    constexpr int kHops = 3000;   // ~32 s of 512-sample hops
    std::atomic<bool> analysisDone{ false };
    float published = 0.0f;   // written by the analysis thread only; read after join()

    std::thread analysis([&tracker, &analysisDone, &published] {
        // AnalysisThread's stage 5, in its order, on a 120 BPM click (a 64-sample burst
        // every 24000 samples) so aubio really tracks beats between the tempo writes.
        std::vector<float> hop(512, 0.0f);
        float sink = 0.0f;
        for (int h = 0; h < kHops; ++h)
        {
            float sumSq = 0.0f;
            for (int i = 0; i < 512; ++i)
            {
                const int k = (h * 512 + i) % 24000;
                hop[static_cast<size_t>(i)] = (k < 64) ? 0.8f * (1.0f - static_cast<float>(k) / 64.0f) : 0.0f;
                sumSq += hop[static_cast<size_t>(i)] * hop[static_cast<size_t>(i)];
            }
            const float rms = std::sqrt(sumSq / 512.0f);
            tracker.feedSilenceDetection(rms);
            tracker.process(hop.data());
            tracker.feedDownbeatFeatures(rms, rms, 0.0f, 0);
            // the snapshot publish AnalysisThread does right after stage 5
            sink += tracker.bpm() + tracker.beatPhase() + tracker.barPhase() + tracker.phrasePhase()
                  + static_cast<float>(tracker.trackerState() + tracker.beatInBar() + tracker.barCount()
                                       + tracker.totalBarCount() + tracker.resyncBarOrigin())
                  + (tracker.downbeatDetected() ? 1.0f : 0.0f);
        }
        published = sink;   // no Catch2 assertion off the test thread (not thread-safe in v3.7)
        analysisDone.store(true);
    });

    // The message thread, meanwhile: exactly the tracker calls applyTempoCommand makes
    // for "tap" / "manual" / "auto" / "resync" / "link" (REST+OSC set_bpm, Link tick).
    int writes = 0;
    while (!analysisDone.load())
    {
        tracker.setManualMode(true);
        tracker.setManualBPM(100.0f + static_cast<float>(writes % 60));
        if (writes % 3 == 0) tracker.followExternalTempo(120.0f);   // the Link tick
        if (writes % 7 == 0) tracker.requestResync();
        if (writes % 11 == 0) tracker.setManualMode(false);
        ++writes;
        std::this_thread::yield();
    }
    analysis.join();
    REQUIRE(writes > 0);
    REQUIRE(std::isfinite(published));

    // Drain what the loop left pending (a tempo and/or a Resync), then one more request
    // lands on the next hop like any other.
    quietHop(tracker);
    tracker.setManualMode(true);
    tracker.setManualBPM(133.0f);
    quietHop(tracker);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(133.0, 1e-3));
    REQUIRE_THAT(tracker.beatPhase(), WithinAbs(manualPhaseInc(133.0f), 1e-6));
}

TEST_CASE("Ableton Link: an unchanged tempo re-sent every UI tick (~30 Hz) never realigns the phase",
          "[bpm][manual][link][s-rta-0926b]")
{
    // MainComponent's 30 Hz timer re-sends Link's tempo on every tick while Link is on
    // (applyTempoCommand("link") -> manual mode + the tempo). An unchanged tempo must
    // leave the phase free-running; RED pre-fix: every tick reset it to 0, so the phase
    // never passed ~0.07 and no bar ever landed.
    BPMTracker tracker(512, 1024, 48000);
    const float inc = manualPhaseInc(120.0f);
    const double hopsPerTick = (48000.0 / 512.0) / 30.0;   // 3.125 hops between UI ticks

    constexpr int kHops = 1900;   // ~20.3 s: 10 bars at 120 BPM
    int ticks = 0, jumps = 0, firstJumpHop = -1;
    std::vector<int> barHops;
    uint32_t bars = tracker.totalBarCount();
    double nextTick = 0.0;
    float prev = tracker.beatPhase();
    for (int h = 0; h < kHops; ++h)
    {
        if (h >= nextTick)
        {
            tracker.setManualMode(true);        // the Link tick's tracker calls
            tracker.followExternalTempo(120.0f);
            ++ticks;
            nextTick += hopsPerTick;
        }
        quietHop(tracker);
        const float now = tracker.beatPhase();
        if (phaseJumped(prev, now, inc)) { ++jumps; if (firstJumpHop < 0) firstJumpHop = h; }
        prev = now;
        if (tracker.totalBarCount() != bars) { bars = tracker.totalBarCount(); barHops.push_back(h); }
    }

    INFO("Link ticks " << ticks << ", phase jumps " << jumps << " (first at hop " << firstJumpHop
         << "), bars " << barHops.size());
    REQUIRE(ticks >= 600);                                    // non-vacuous: ~30 ticks/s for 20 s
    REQUIRE_THAT(tracker.bpm(), WithinAbs(120.0, 1e-3));
    REQUIRE(jumps == 0);                                      // RED pre-fix: a reset on every tick
    REQUIRE(barHops.size() == 10);                            // RED pre-fix: 0 bars
    for (size_t i = 1; i < barHops.size(); ++i)
    {
        INFO("bar " << i << " took " << (barHops[i] - barHops[i - 1]) << " hops (187.5 = 2.000 s)");
        REQUIRE(barHops[i] - barHops[i - 1] >= 187);
        REQUIRE(barHops[i] - barHops[i - 1] <= 188);
    }

    // A CHANGED Link tempo is applied on the next hop and realigns, as every tempo change did.
    tracker.setManualMode(true);
    tracker.followExternalTempo(128.0f);
    quietHop(tracker);
    REQUIRE_THAT(tracker.bpm(), WithinAbs(128.0, 1e-3));
    REQUIRE_THAT(tracker.beatPhase(), WithinAbs(manualPhaseInc(128.0f), 1e-6));
}
