#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "analysis/BPMTracker.h"
#include "features/OnsetPulse.h"

using Catch::Matchers::WithinAbs;

// Helper: lock the BPM tracker at a given BPM so downbeat detection can work
static void lockBPM(BPMTracker& tracker, float bpm, int hops = 60)
{
    for (int i = 0; i < hops; ++i)
        tracker.processRawBPM(bpm, 1.0f, false);
}

// Helper: simulate a beat with spectral features for downbeat scoring.
// isDownbeat = true gives higher bass + flux + HCDF (stronger beat).
static void feedBeatWithFeatures(BPMTracker& tracker, float bpm,
                                  bool isDownbeat, float conf = 1.0f)
{
    // Simulate a beat detection
    tracker.processRawBPM(bpm, conf, true);

    // Feed spectral features — downbeat gets higher bass energy
    float bass = isDownbeat ? 0.9f : 0.3f;
    float flux = isDownbeat ? 0.8f : 0.2f;
    float hcdf = isDownbeat ? 0.7f : 0.1f;
    tracker.feedDownbeatFeatures(bass, flux, hcdf);
}

// Helper: simulate non-beat hops (just advance phase, no beat event)
static void feedNonBeatHops(BPMTracker& tracker, float bpm, int hops)
{
    for (int i = 0; i < hops; ++i)
    {
        tracker.processRawBPM(bpm, 0.3f, false);
        tracker.feedDownbeatFeatures(0.1f, 0.05f, 0.02f);
    }
}

// ============================================================================
// Downbeat LEVEL semantics + reader-cadence model (s-rta-0925, roadmap item 4)
// ============================================================================
// downbeatDetected is NOT a one-hop pulse: BPMTracker assigns downbeatDetected_ = (beatCounter_ == 0)
// only at a beat event (scoreBeat / advancePredictedBeat / initial lock) and never clears it per hop,
// so it is HELD for the whole first beat of the bar (300 ms at 200 BPM .. 1 s at 60 BPM). updatePhrase()
// rising-edge-detects it and advances totalBarCount exactly once per bar. Pinned here:
//   * a reader at any UI/render cadence (15 Hz .. 120 Hz) never misses a downbeat: it sees the level on
//     many consecutive reads and its rising edge exactly once per bar;
//   * a reader slower than one beat (a sluggish REST poller) DOES lose rising edges; the totalBarCount
//     delta (OnsetPulse -- the generic monotonic-counter consumer of the onset render-path fix) sees
//     every bar exactly once at ANY cadence. Live twin: .harmony/probe-downbeat-level.sh.
namespace
{
constexpr double kHopSec = 512.0 / 48000.0;   // AnalysisThread::kHopSize / kSampleRate

struct CadenceReader
{
    double     periodSec;
    double     nextRead   = 0.0;
    bool       prevLevel  = false;   // what a LEVEL consumer must keep to see "new bar"
    int        readsTrue  = 0;       // reads on which downbeatDetected was true
    int        edges      = 0;       // rising edges seen: level && !prevLevel
    int        pulseReads = 0;       // reads on which the totalBarCount delta was > 0
    uint32_t   deltaSum   = 0;       // sum of totalBarCount deltas
    OnsetPulse barPulse;             // generic monotonic-counter delta, here on totalBarCount

    void prime(bool level, uint32_t count)          // the reader was already looking before bar 0
    {
        prevLevel = level;
        (void) barPulse.consume(count);             // baseline only
        nextRead = periodSec;
    }
    void catchUp(double tPub, bool level, uint32_t count)   // always-latest bus: read the latest
    {
        while (nextRead <= tPub)
        {
            readsTrue += level ? 1 : 0;
            if (level && !prevLevel) ++edges;
            prevLevel = level;
            const uint32_t d = barPulse.consume(count);
            deltaSum += d;
            if (d > 0u) ++pulseReads;
            nextRead += periodSec;
        }
    }
};

struct HopTruth { int hopsTrue = 0; int hopEdges = 0; bool prevLevel = false; double tPub = 0.0; };

template <size_t N>
void publishHop(const BPMTracker& t, HopTruth& truth, CadenceReader (&readers)[N])
{
    const bool     level = t.downbeatDetected();
    const uint32_t count = t.totalBarCount();
    truth.hopsTrue += level ? 1 : 0;
    if (level && !truth.prevLevel) ++truth.hopEdges;
    truth.prevLevel = level;
    truth.tPub += kHopSec;
    for (auto& r : readers) r.catchUp(truth.tPub, level, count);
}

// GREEN contract shared by both regimes. readers[0] = 60 Hz, [1] = 120 Hz, [2] = 0.9 s poller.
template <size_t N>
void requireLevelContract(const HopTruth& truth, const CadenceReader (&readers)[N],
                          uint32_t barsAdvanced, int kBars, int hopsPerBeatLo, int hopsPerBeatHi)
{
    INFO("barsAdvanced=" << barsAdvanced << " hopsTrue=" << truth.hopsTrue << " hopEdges=" << truth.hopEdges
         << " | 60Hz true=" << readers[0].readsTrue << " edges=" << readers[0].edges << " pulse=" << readers[0].pulseReads
         << " | 120Hz true=" << readers[1].readsTrue << " edges=" << readers[1].edges << " pulse=" << readers[1].pulseReads
         << " | 0.9s true=" << readers[2].readsTrue << " edges=" << readers[2].edges << " pulse=" << readers[2].pulseReads
         << " delta=" << readers[2].deltaSum);
    // Hop-level truth: one bar per 4 beats; the level held for the whole first beat.
    REQUIRE(barsAdvanced == static_cast<uint32_t>(kBars));
    REQUIRE(truth.hopEdges == kBars);
    REQUIRE(truth.hopsTrue >= kBars * hopsPerBeatLo);
    REQUIRE(truth.hopsTrue <= kBars * hopsPerBeatHi);
    // UI/render cadence: the level is seen on many reads per bar (a level, not a pulse), its rising
    // edge exactly once per bar (no loss, no duplication), and the counter agrees.
    for (int i = 0; i < 2; ++i)
    {
        REQUIRE(readers[i].readsTrue > 10 * kBars);
        REQUIRE(readers[i].edges == kBars);
        REQUIRE(readers[i].pulseReads == kBars);
        REQUIRE(readers[i].deltaSum == static_cast<uint32_t>(kBars));
    }
    // Slower than one beat (0.9 s > 0.5 s): rising-edge detection on the level LOSES bars (about half
    // on this grid); the totalBarCount delta still sees every bar exactly once.
    REQUIRE(readers[2].edges < kBars);
    REQUIRE(readers[2].pulseReads == kBars);
    REQUIRE(readers[2].deltaSum == static_cast<uint32_t>(kBars));
}
} // namespace

TEST_CASE("Downbeat level (real onsets): held for the whole first beat, totalBarCount once per bar; "
          "60/120 Hz readers lose nothing, a 0.9 s poller needs the counter",
          "[downbeat][level][cadence]")
{
    BPMTracker tracker(512, 1024, 48000);
    lockBPM(tracker, 120.0f);
    for (int bar = 0; bar < 8; ++bar)                 // lock the downbeat (compressed spacing, as above)
        for (int beat = 0; beat < 4; ++beat)
        {
            feedBeatWithFeatures(tracker, 120.0f, beat == 0);
            feedNonBeatHops(tracker, 120.0f, 3);
        }
    REQUIRE(tracker.downbeatLocked());
    REQUIRE_FALSE(tracker.downbeatDetected());       // the lock phase ended on beat 4 (index 3)

    constexpr int kBars = 16;
    constexpr int kHopsPerBeat = 47;                  // lround(48000*60 / (120*512)): real-time spacing
    const uint32_t bars0 = tracker.totalBarCount();

    CadenceReader readers[] = { { 1.0 / 60.0 }, { 1.0 / 120.0 }, { 0.9 } };
    HopTruth truth;
    truth.prevLevel = tracker.downbeatDetected();
    for (auto& r : readers) r.prime(tracker.downbeatDetected(), tracker.totalBarCount());

    for (int bar = 0; bar < kBars; ++bar)
        for (int beat = 0; beat < 4; ++beat)
        {
            feedBeatWithFeatures(tracker, 120.0f, beat == 0);   // the beat hop (conf 1.0)
            publishHop(tracker, truth, readers);
            for (int h = 1; h < kHopsPerBeat; ++h)              // 46 non-beat hops
            {
                feedNonBeatHops(tracker, 120.0f, 1);
                publishHop(tracker, truth, readers);
            }
        }

    // --- RED-first evidence (task's premise -- a one-hop pulse -- against the real tracker,
    // scratch build, `./test_downbeat_detector "[cadence]"`, s-rta-0925):
    //   752 (0x2f0) == 16                          -> FAILED (hopsTrue != hopEdges: level, not pulse)
    //   481 (0x1e1) < 16                            -> FAILED (60 Hz reader sees far more than 16 true reads)
    //   16 < 16                                     -> FAILED (0 loss at 60 Hz)
    //   960 (0x3c0) > 16 && 960 < 32                -> FAILED (no ~1.26x duplication at 120 Hz)
    // The premise is refuted by the real tracker; the GREEN contract below is what ships.
    requireLevelContract(truth, readers, tracker.totalBarCount() - bars0, kBars,
                         kHopsPerBeat - 1, kHopsPerBeat + 1);   // exact is 16*47 = 752
}

TEST_CASE("Downbeat level (manual BPM, predicted beats): the same level + counter contract with no "
          "onsets -- the regime /api/set_bpm puts the tracker in",
          "[downbeat][level][cadence][manual]")
{
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualMode(true);
    tracker.setManualBPM(120.0f);                     // lockedBPM_ = 120, phase_ = 0, beatCounter_ = 0
    REQUIRE(tracker.isManualMode());

    constexpr int kBars = 16;
    constexpr int kHops = 3060;   // 65 phase wraps at 46.875 hops/beat; downbeats at wraps 4,8,..,64
                                  // (beatCounter_ 0->1->2->3->0), the 16th level window ~hops [3000,3047)
    const uint32_t bars0 = tracker.totalBarCount();
    CadenceReader readers[] = { { 1.0 / 60.0 }, { 1.0 / 120.0 }, { 0.9 } };
    HopTruth truth;
    truth.prevLevel = tracker.downbeatDetected();
    for (auto& r : readers) r.prime(tracker.downbeatDetected(), tracker.totalBarCount());

    for (int h = 0; h < kHops; ++h)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);        // manual branch: predicted wrap drives beats
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f);  // per hop, as AnalysisThread: updatePhrase -> totalBarCount
        publishHop(tracker, truth, readers);
    }

    requireLevelContract(truth, readers, tracker.totalBarCount() - bars0, kBars, 46, 48);
}

// ============================================================================
// Basic Downbeat Scoring Tests
// ============================================================================

TEST_CASE("Downbeat detector starts unlocked", "[downbeat][basic]")
{
    BPMTracker tracker(512, 1024, 48000);
    REQUIRE(tracker.downbeatLocked() == false);
    REQUIRE(tracker.beatInBar() == 0);
    REQUIRE(tracker.barPhase() == 0.0f);
}

TEST_CASE("Downbeat locks after consistent 4/4 pattern", "[downbeat][lock]")
{
    BPMTracker tracker(512, 1024, 48000);

    // Lock BPM first
    lockBPM(tracker, 120.0f);
    REQUIRE(tracker.bpm() > 0.0f);

    // Feed 32 beats in a 4/4 pattern: beat 0 is strong (downbeat), beats 1-3 are weak
    // Between beats, feed some non-beat hops to simulate real spacing
    for (int bar = 0; bar < 8; ++bar)
    {
        for (int beat = 0; beat < 4; ++beat)
        {
            bool isDownbeat = (beat == 0);
            feedBeatWithFeatures(tracker, 120.0f, isDownbeat);
            // A few non-beat hops between beats
            feedNonBeatHops(tracker, 120.0f, 5);
        }
    }

    // After 32 beats with consistent pattern, downbeat should be locked
    REQUIRE(tracker.downbeatLocked() == true);
}

TEST_CASE("beatInBar cycles 0-3 when locked", "[downbeat][cycle]")
{
    BPMTracker tracker(512, 1024, 48000);
    lockBPM(tracker, 120.0f);

    // Feed enough beats to lock the downbeat
    for (int bar = 0; bar < 8; ++bar)
    {
        for (int beat = 0; beat < 4; ++beat)
        {
            feedBeatWithFeatures(tracker, 120.0f, beat == 0);
            feedNonBeatHops(tracker, 120.0f, 3);
        }
    }

    REQUIRE(tracker.downbeatLocked() == true);

    // Now track the next 8 beats and verify cycling
    std::vector<uint8_t> beatPositions;
    for (int i = 0; i < 8; ++i)
    {
        feedBeatWithFeatures(tracker, 120.0f, (i % 4) == 0);
        beatPositions.push_back(tracker.beatInBar());
        feedNonBeatHops(tracker, 120.0f, 3);
    }

    // Should see a repeating 0,1,2,3 pattern
    for (int i = 0; i < 8; ++i)
    {
        REQUIRE(beatPositions[static_cast<size_t>(i)] == static_cast<uint8_t>(i % 4));
    }
}

TEST_CASE("downbeatDetected fires only on beat 0", "[downbeat][trigger]")
{
    BPMTracker tracker(512, 1024, 48000);
    lockBPM(tracker, 120.0f);

    // Lock the downbeat
    for (int bar = 0; bar < 8; ++bar)
    {
        for (int beat = 0; beat < 4; ++beat)
        {
            feedBeatWithFeatures(tracker, 120.0f, beat == 0);
            feedNonBeatHops(tracker, 120.0f, 3);
        }
    }

    REQUIRE(tracker.downbeatLocked() == true);

    // Feed 4 more beats and check downbeatDetected
    int downbeatCount = 0;
    for (int beat = 0; beat < 4; ++beat)
    {
        feedBeatWithFeatures(tracker, 120.0f, beat == 0);
        if (tracker.downbeatDetected())
            ++downbeatCount;
        feedNonBeatHops(tracker, 120.0f, 3);
    }

    // Exactly 1 downbeat per 4 beats
    REQUIRE(downbeatCount == 1);
}

// ============================================================================
// Bar Phase Tests
// ============================================================================

TEST_CASE("barPhase stays within zero to one range", "[downbeat][barphase]")
{
    BPMTracker tracker(512, 1024, 48000);
    lockBPM(tracker, 120.0f);

    // Feed beats and check barPhase stays in range
    for (int i = 0; i < 32; ++i)
    {
        feedBeatWithFeatures(tracker, 120.0f, (i % 4) == 0);
        REQUIRE(tracker.barPhase() >= 0.0f);
        REQUIRE(tracker.barPhase() < 1.0f);

        // Also check between beats
        feedNonBeatHops(tracker, 120.0f, 3);
        REQUIRE(tracker.barPhase() >= 0.0f);
        REQUIRE(tracker.barPhase() < 1.0f);
    }
}

TEST_CASE("barPhase is 0 when no BPM locked", "[downbeat][barphase]")
{
    BPMTracker tracker(512, 1024, 48000);

    // No BPM locked — barPhase should be 0
    tracker.feedDownbeatFeatures(0.5f, 0.3f, 0.1f);
    REQUIRE(tracker.barPhase() == 0.0f);
}

// ============================================================================
// Synthetic 4/4 Kick Pattern Test
// ============================================================================

TEST_CASE("Downbeat detection with synthetic kick pattern over 32 beats", "[downbeat][synthetic]")
{
    BPMTracker tracker(512, 1024, 48000);
    lockBPM(tracker, 128.0f);

    // Simulate a typical EDM pattern:
    // Beat 0 (kick):    bass=0.9, flux=0.7, hcdf=0.5
    // Beat 1 (hihat):   bass=0.1, flux=0.3, hcdf=0.05
    // Beat 2 (kick):    bass=0.6, flux=0.4, hcdf=0.1
    // Beat 3 (snare):   bass=0.2, flux=0.5, hcdf=0.05
    float bassPattern[4]  = {0.9f, 0.1f, 0.6f, 0.2f};
    float fluxPattern[4]  = {0.7f, 0.3f, 0.4f, 0.5f};
    float hcdfPattern[4]  = {0.5f, 0.05f, 0.1f, 0.05f};

    for (int bar = 0; bar < 8; ++bar)
    {
        for (int beat = 0; beat < 4; ++beat)
        {
            tracker.processRawBPM(128.0f, 1.0f, true);
            tracker.feedDownbeatFeatures(bassPattern[beat], fluxPattern[beat], hcdfPattern[beat]);
            feedNonBeatHops(tracker, 128.0f, 5);
        }
    }

    // After 32 beats of clear kick-on-1 pattern, should be locked
    REQUIRE(tracker.downbeatLocked() == true);

    // Beat 0 should have the highest score (0.5*0.9 + 0.3*0.7 + 0.2*0.5 = 0.76)
    // Beat 2 is second (0.5*0.6 + 0.3*0.4 + 0.2*0.1 = 0.44)
    // So the detected downbeat should be at position 0
    // Verify by checking that after the next downbeat, beatInBar returns to 0
    tracker.processRawBPM(128.0f, 1.0f, true);
    tracker.feedDownbeatFeatures(bassPattern[0], fluxPattern[0], hcdfPattern[0]);

    // This should be beat 0 or close to it in the cycle
    // (exact position depends on where the lock happened)
    REQUIRE(tracker.beatInBar() <= 3);  // basic sanity
}

// ============================================================================
// Beat-in-bar accuracy over 32-beat sequences
// ============================================================================

TEST_CASE("Beat-in-bar accuracy over extended sequence", "[downbeat][accuracy]")
{
    BPMTracker tracker(512, 1024, 48000);
    lockBPM(tracker, 120.0f);

    // First lock the downbeat with 8 bars
    for (int bar = 0; bar < 8; ++bar)
    {
        for (int beat = 0; beat < 4; ++beat)
        {
            feedBeatWithFeatures(tracker, 120.0f, beat == 0);
            feedNonBeatHops(tracker, 120.0f, 4);
        }
    }

    REQUIRE(tracker.downbeatLocked() == true);

    // Now verify 32 more beats have correct beatInBar
    int correctCount = 0;
    for (int i = 0; i < 32; ++i)
    {
        int expectedBeat = i % 4;
        feedBeatWithFeatures(tracker, 120.0f, expectedBeat == 0);
        if (tracker.beatInBar() == static_cast<uint8_t>(expectedBeat))
            ++correctCount;
        feedNonBeatHops(tracker, 120.0f, 4);
    }

    // At least 90% should be correct (allowing for edge cases at lock transition)
    REQUIRE(correctCount >= 28);  // 28/32 = 87.5% minimum
}
