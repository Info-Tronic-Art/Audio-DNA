// test_tempo_start -- s-rta-0928 take start race (.harmony/.reports/s-rta-0928/plan-tempo.md).
//
// A tempo / Tap / Resync command sent just before Record is only a BPMTracker REQUEST: the analysis thread
// applies it at its next hop (~10.7 ms later). The take's t = 0 (its "start" tempo anchor, beat 0) and
// meta.startBeatInBar must describe the tempo and bar grid the performer asked for BEFORE Record, not the
// snapshot a 120 Hz tick happened to read before that hop published.
//
// E1/E2 script the race with the REAL BPMTracker, RecorderHost, RecorderClock/TempoMap, Take save/load and
// RoutineSlice, single-threaded and deterministic: the analysis "hop" and the message-thread "tick" are
// interleaved in the order the s-rta-0927 tempo0 diagnosis traced (its run 03). F1 pins that a raced take
// recorded before the fix still loads and slices exactly as before (the fix is forward-only).
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Composition.h"
#include "recording/RecorderHost.h"
#include "recording/PerfStateCapture.h"
#include "recording/Take.h"
#include "recording/AudioTap.h"
#include "recording/AudioStore.h"
#include "recording/RoutineSlice.h"
#include "effects/EffectLibrary.h"
#include "analysis/FeatureSnapshot.h"
#include "analysis/BPMTracker.h"
#include <cmath>
#include <optional>
#include <string>
#include <vector>

using Catch::Approx;

namespace
{
    constexpr double kHopSeconds = 512.0 / 48000.0;    // AnalysisThread::kHopSize at the fixed 48 kHz
    constexpr double kTickSeconds = 1.0 / 120.0;       // MainComponent's 120 Hz feature tick

    struct TempDir
    {
        juce::File dir;
        explicit TempDir(const juce::String& tag)
            : dir(juce::File::getSpecialLocation(juce::File::tempDirectory)
                      .getChildFile("audiodna_tempo_start_test_" + tag + "_"
                                    + juce::String(juce::Random::getSystemRandom().nextInt64())))
        {
            dir.createDirectory();
        }
        ~TempDir() { dir.deleteRecursively(); }
    };

    Composition makeComposition()
    {
        Composition c;
        c.decks.resize(1);
        c.decks[0].initDefault();
        c.activeDeckIndex = 0;
        return c;
    }

    struct FakeDispatch
    {
        std::vector<std::string> notices;
        void wire(RecorderHost& host, const Composition* comp)
        {
            host.dispatch.capturePerfState = [comp]() { return comp ? capturePerfState(*comp, 120.0f, "") : PerfState{}; };
            host.dispatch.notify = [this](const std::string& s) { notices.push_back(s); };
        }
    };

    // One analysis hop, as AnalysisThread::run publishes it (AnalysisThread.cpp: processRawBPM via
    // process(), then feedDownbeatFeatures, then the tracker fields are copied into the snapshot).
    FeatureSnapshot publishHop(BPMTracker& tr, uint64_t& ts)
    {
        tr.processRawBPM(0.0f, 0.0f, false);
        tr.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
        ts += 512;
        FeatureSnapshot s;
        s.clear();
        s.timestamp = ts;
        s.bpm = tr.bpm();
        s.beatPhase = tr.beatPhase();
        s.trackerState = tr.trackerState();
        s.beatInBar = tr.beatInBar();
        s.barPhase = tr.barPhase();
        s.downbeatDetected = tr.downbeatDetected();
        s.barCount = tr.barCount();
        s.totalBarCount = tr.totalBarCount();
        s.resyncBarOrigin = tr.resyncBarOrigin();
        s.totalBeatCount = tr.totalBeatCount();
        s.phrasePhase = tr.phrasePhase();
        return s;
    }

    // The analysis thread, the FeatureBus (its latest published snapshot) and the take recorder on the
    // message thread -- stepped by hand so the hop / tick order is the test's to choose.
    struct Rig
    {
        BPMTracker tr{ 512, 1024, 48000 };
        uint64_t ts = 0;
        FeatureSnapshot bus;
        TempDir storeRoot{ "store" };
        TempDir takeFolder{ "take" };
        AudioStore store{ storeRoot.dir };
        RecorderHost host{ store };
        AudioTap tap;
        Composition comp = makeComposition();
        FakeDispatch fd;
        double w0 = 0.0;             // the wall time delivered samples count from
        double nextHop = 0.0, nextTick = 0.0;

        Rig() { bus.clear(); fd.wire(host, &comp); }

        uint64_t delivered(double wall) const
        {
            return 4800000ull + static_cast<uint64_t>(std::llround((wall - w0) * 48000.0));
        }
        void hop() { bus = publishHop(tr, ts); }
        void tick(double wall) { host.tick(bus, wall, delivered(wall), tap, std::nullopt, 48000.0); }

        // Hops every kHopSeconds from nextHop and ticks every kTickSeconds from nextTick, in time order
        // (a hop before a tick at the same instant), up to `until`.
        void drive(double until)
        {
            while (nextHop <= until || nextTick <= until)
            {
                if (nextHop <= nextTick && nextHop <= until) { hop(); nextHop += kHopSeconds; }
                else if (nextTick <= until) { tick(nextTick); nextTick += kTickSeconds; }
                else break;
            }
        }

        // perfRecord's ArmOptions (MainComponent.cpp perfRecord) -- the test's mirror of its two production
        // wiring lines, which no ctest compiles ([lint] L2 pins those lines in source).
        RecorderHost::ArmResult armLikePerfRecord(const FeatureSnapshot& busAtArm)
        {
            RecorderHost::ArmOptions o;
            o.takeFolder = takeFolder.dir;
            o.audio = false;
            o.appVersion = "0.1.0";
            // Today's rule: the bus read ONCE at arm, only while LOCKED.
            if (busAtArm.trackerState == BPMTracker::STATE_LOCKED)
                o.startBeatInBar = static_cast<double>(busAtArm.beatInBar) + static_cast<double>(busAtArm.beatPhase);
            return host.arm(comp, tap, o);
        }

        std::optional<Take> stopAndLoad()
        {
            const auto res = host.disarm(comp, tap);
            REQUIRE(res.ok);
            LoadStats stats;
            return Take::load(takeFolder.dir, stats);
        }
    };

    const EffectLibrary& library()
    {
        static EffectLibrary lib = [] { EffectLibrary l; l.registerDefaults(); return l; }();
        return lib;
    }
}

// === E1 ===

TEST_CASE("a set_bpm sent just before Record is in the take's start anchor and bar grid", "[tempo-start][race]")
{
    Rig r;
    r.w0 = 100.0;
    for (int i = 0; i < 3; ++i)
        r.hop();                                   // unlocked: bpm 0, SEARCHING
    REQUIRE(r.bus.bpm == 0.0f);

    // Message thread, back to back: REST /api/set_bpm's "link" action (applyTempoCommand), then Record.
    r.tr.setManualMode(true);
    r.tr.followExternalTempo(120.0f);
    REQUIRE(r.armLikePerfRecord(r.bus).ok);

    r.tick(100.0005);                              // a tick reads the bus before the applying hop publishes
    r.hop();                                       // the applying hop: bpm 120, LOCKED
    REQUIRE(r.bus.bpm == Approx(120.0f));
    r.tick(100.0088333);
    r.tick(100.0171667);
    CHECK(r.host.status().t == Approx(1.0 / 120.0).margin(1e-6));

    r.nextHop = 100.001 + kHopSeconds;
    r.nextTick = 100.0171667 + kTickSeconds;
    r.drive(102.0);

    const auto take = r.stopAndLoad();
    REQUIRE(take.has_value());
    const auto& a = take->tempo.a;
    REQUIRE_FALSE(a.empty());
    CHECK(a.size() == 1);
    CHECK(a[0].why == "start");
    CHECK(a[0].bpm == Approx(120.0f));
    CHECK(take->meta.startBeatInBar == Approx(512.0 / 24000.0));
    const auto slice = sliceRoutine(*take, { 0.0, 4.0, "r", true, r.takeFolder.dir.getFullPathName().toStdString() }, library());
    CHECK(slice.error == "");
    CHECK(takeBeatOfBar(*take, 1) == Approx(4.0 - 512.0 / 24000.0));
}

// === E2 ===

TEST_CASE("a Resync sent just before Record makes beat 0 the new downbeat", "[tempo-start][race]")
{
    Rig r;
    r.w0 = 200.0;
    r.tr.setManualMode(true);
    r.tr.followExternalTempo(120.0f);
    int guard = 0;
    do
    {
        r.hop();
        REQUIRE(++guard < 2000);
    } while (!(r.bus.beatInBar == 2 && r.bus.beatPhase >= 0.55f));
    REQUIRE(r.bus.trackerState == BPMTracker::STATE_LOCKED);
    INFO("bus at Record: beatInBar " << int(r.bus.beatInBar) << " beatPhase " << r.bus.beatPhase);

    r.tr.requestResync();                          // REST /api/resync, then Record
    REQUIRE(r.armLikePerfRecord(r.bus).ok);

    r.tick(200.0005);                              // stale: the pre-Resync bar position
    r.hop();                                       // the resync hop: beatInBar 0, beatPhase 0
    REQUIRE(r.bus.beatInBar == 0);
    REQUIRE(r.bus.beatPhase == 0.0f);
    r.tick(200.0088333);
    CHECK(r.host.status().beat == Approx(0.0).margin(1e-9));

    r.nextHop = 200.001 + kHopSeconds;
    r.nextTick = 200.0088333 + kTickSeconds;
    r.drive(202.0);

    const auto take = r.stopAndLoad();
    REQUIRE(take.has_value());
    CHECK(take->meta.startBeatInBar == Approx(0.0).margin(1e-9));
    CHECK(takeBeatOfBar(*take, 1) == Approx(0.0).margin(1e-9));
}

// === F1 ===

TEST_CASE("a raced take recorded before the fix still loads and behaves identically", "[tempo-start][old-take]")
{
    LoadStats stats;
    const auto take = Take::load(juce::File(TEST_FIXTURES_DIR).getChildFile("take_v3_start_bpm0.json"), stats);
    REQUIRE(take.has_value());
    REQUIRE_FALSE(stats.refused);

    const auto& a = take->tempo.a;
    REQUIRE(a.size() == 2);
    CHECK(a[0].t == 0.0);
    CHECK(a[0].beat == 0.0);
    CHECK(a[0].sample == 128512u);
    CHECK(a[0].bpm == 0.0f);
    CHECK(a[0].why == "start");
    CHECK(a[1].t == Approx(0.01203954165976));
    CHECK(a[1].beat == 0.0);
    CHECK(a[1].sample == 129536u);
    CHECK(a[1].bpm == Approx(120.0f));
    CHECK(a[1].why == "lock");
    CHECK(take->meta.startBeatInBar == -1.0);

    CHECK(take->tempo.beatAt(0.5) == Approx((0.5 - 0.01203954165976) * 2.0));
    CHECK(take->tempo.tAt(1.0) == Approx(0.51203954165976));

    // The designed unmetered refusal: the bpm-0 "start" anchor sits at beat 0.
    const auto fromZero = sliceRoutine(*take, { 0.0, 2.0, "r", true, "" }, library());
    CHECK(fromZero.error == "this stretch has no beat; the tempo was unknown while it was recorded");
    const auto pastZero = sliceRoutine(*take, { 1e-6, 2.0, "r", true, "" }, library());
    CHECK(pastZero.error.empty());
    CHECK(takeBeatOfBar(*take, 1) == 0.0);
}
