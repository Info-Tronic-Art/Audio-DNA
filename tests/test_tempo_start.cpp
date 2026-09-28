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
#include <iostream>
#include <optional>
#include <sstream>
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
        s.trackerRequestSeq = tr.appliedRequestSeq();   // AnalysisThread.cpp's copy (Pitfall 48)
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
            // s-rta-0928: the request sequence posted before Record (perfRecord outside test mode). The bus at
            // arm is no longer read: the take's start and bar grid come from the snapshot that carries it.
            (void) busAtArm;
            o.startAfterTrackerRequest = tr.postedRequestSeq();
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

// === the analysis side: BPMTracker's request sequence (Pitfall 48) ===

namespace
{
    void hopOnly(BPMTracker& tr)
    {
        tr.processRawBPM(0.0f, 0.0f, false);
        tr.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    }

    std::string readSource(const char* relative)
    {
        return juce::File(juce::String(AUDIODNA_SRC_DIR)).getChildFile(relative).loadFileAsString().toStdString();
    }
}

TEST_CASE("every request raises the posted sequence; the next hop latches it", "[bpm][request-seq]")
{
    BPMTracker tr(512, 1024, 48000);
    CHECK(tr.postedRequestSeq() == 0u);
    CHECK(tr.appliedRequestSeq() == 0u);

    tr.followExternalTempo(120.0f);
    CHECK(tr.postedRequestSeq() == 1u);
    CHECK(tr.appliedRequestSeq() == 0u);
    CHECK(tr.bpm() == 0.0f);                       // a request, not a write
    hopOnly(tr);
    CHECK(tr.appliedRequestSeq() == 1u);
    CHECK(tr.bpm() == Approx(120.0f));

    tr.setManualMode(true);
    CHECK(tr.postedRequestSeq() == 2u);
    hopOnly(tr);
    CHECK(tr.appliedRequestSeq() == 2u);

    tr.setManualBPM(100.0f);
    CHECK(tr.postedRequestSeq() == 3u);
    hopOnly(tr);
    CHECK(tr.appliedRequestSeq() == 3u);
    CHECK(tr.bpm() == Approx(100.0f));
    CHECK(tr.beatPhase() == Approx(512.0 / 28800.0));   // realigned, then one manual advance

    tr.requestResync();
    CHECK(tr.postedRequestSeq() == 4u);
    tr.processRawBPM(0.0f, 0.0f, false);
    CHECK(tr.appliedRequestSeq() == 4u);
    tr.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    CHECK(tr.beatInBar() == 0);
    CHECK(tr.beatPhase() == 0.0f);

    tr.setManualMode(false);
    CHECK(tr.postedRequestSeq() == 5u);
    hopOnly(tr);
    CHECK(tr.appliedRequestSeq() == 5u);

    // An early return posts nothing and raises nothing.
    tr.setManualBPM(0.0f);
    tr.followExternalTempo(-1.0f);
    CHECK(tr.postedRequestSeq() == 5u);
}

TEST_CASE("a request posted after the hop's latch is claimed by the next hop, never this one", "[bpm][request-seq]")
{
    BPMTracker tr(512, 1024, 48000);
    tr.setManualMode(true);
    tr.followExternalTempo(120.0f);
    int guard = 0;
    do
    {
        hopOnly(tr);
        REQUIRE(++guard < 2000);
    } while (tr.beatInBar() == 0);

    // A tempo value posted between runPipeline (the latch) and the end of the hop.
    tr.processRawBPM(0.0f, 0.0f, false);
    const uint32_t L = tr.appliedRequestSeq();
    tr.followExternalTempo(90.0f);
    tr.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    CHECK(tr.appliedRequestSeq() == L);
    CHECK(tr.bpm() == Approx(120.0f));
    tr.processRawBPM(0.0f, 0.0f, false);
    CHECK(tr.appliedRequestSeq() == L + 1);
    CHECK(tr.bpm() == Approx(90.0f));
    tr.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);

    guard = 0;
    while (tr.beatInBar() == 0)
    {
        hopOnly(tr);
        REQUIRE(++guard < 2000);
    }

    // A Resync posted after the latch IS applied at the end of the same hop -- but not claimed (conservative:
    // a waiter waits one more hop, never less).
    tr.processRawBPM(0.0f, 0.0f, false);
    const uint32_t M = tr.appliedRequestSeq();
    tr.requestResync();
    tr.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    CHECK(tr.beatInBar() == 0);
    CHECK(tr.appliedRequestSeq() == M);
}

TEST_CASE("[lint] AnalysisThread publishes the request sequence before it publishes the snapshot", "[tempo-start][lint]")
{
    // A structural lint (a source-text scan), not behaviour proof: the live probe-tempo-start.sh W1-W3 prove
    // the published sequence reaches the recorder.
    const std::string src = readSource("analysis/AnalysisThread.cpp");
    REQUIRE_FALSE(src.empty());
    const auto copy = src.find("snap->trackerRequestSeq = bpmTracker_->appliedRequestSeq();");
    const auto publish = src.find("featureBusWriter_.publishWrite();");
    REQUIRE(publish != std::string::npos);
    CHECK(copy != std::string::npos);
    CHECK(copy < publish);
}

// === the recorder side: the start gate (RecorderHost) ===

namespace
{
    FeatureSnapshot snapOf(uint32_t seq, float bpm, bool locked, uint8_t beatInBar = 0, float phase = 0.0f,
                           uint32_t onsetCount = 0)
    {
        FeatureSnapshot s;
        s.clear();
        s.trackerRequestSeq = seq;
        s.bpm = bpm;
        s.trackerState = locked ? FeatureSnapshot::kTrackerLocked : 0;
        s.beatInBar = beatInBar;
        s.beatPhase = phase;
        s.onsetCount = onsetCount;
        return s;
    }

    uint64_t sampleAt(double wall) { return static_cast<uint64_t>(std::llround(wall * 48000.0)); }

    // A bare host: no tracker, snapshots built by hand.
    struct Gate
    {
        TempDir storeRoot{ "gstore" };
        TempDir takeFolder{ "gtake" };
        AudioStore store{ storeRoot.dir };
        RecorderHost host{ store };
        AudioTap tap;
        Composition comp = makeComposition();
        FakeDispatch fd;

        explicit Gate(std::optional<uint32_t> awaited, bool onsetMarkers = false)
        {
            fd.wire(host, &comp);
            RecorderHost::ArmOptions o;
            o.takeFolder = takeFolder.dir;
            o.audio = false;
            o.appVersion = "0.1.0";
            o.onsetMarkers = onsetMarkers;
            o.startAfterTrackerRequest = awaited;
            REQUIRE(host.arm(comp, tap, o).ok);
        }
        void tick(const FeatureSnapshot& s, double wall) { host.tick(s, wall, sampleAt(wall), tap, std::nullopt, 48000.0); }
        Take stopAndLoad()
        {
            REQUIRE(host.disarm(comp, tap).ok);
            LoadStats stats;
            auto t = Take::load(takeFolder.dir, stats);
            REQUIRE(t.has_value());
            return *t;
        }
    };

    // Captures std::cerr for the scope (the host's "take start:" lines).
    struct CerrCapture
    {
        std::ostringstream text;
        std::streambuf* old;
        CerrCapture() : old(std::cerr.rdbuf(text.rdbuf())) {}
        ~CerrCapture() { std::cerr.rdbuf(old); }
    };
}

TEST_CASE("nothing in flight -> t = 0 on the first tick; startBeatInBar from that tick", "[tempo-start][gate]")
{
    std::optional<uint32_t> awaited;
    SECTION("nothing awaited") { awaited = std::nullopt; }
    SECTION("the first tick already carries the awaited request") { awaited = 7u; }

    Gate g(awaited);
    const uint32_t seq = awaited.value_or(0u);
    g.tick(snapOf(seq, 120.0f, true, 3, 0.84f), 5.0);
    g.tick(snapOf(seq, 120.0f, true, 3, 0.84f), 5.05);
    CHECK(g.host.status().t == Approx(0.05));
    const Take take = g.stopAndLoad();
    REQUIRE(take.tempo.a.size() >= 1);
    CHECK(take.tempo.a[0].why == "start");
    CHECK(take.tempo.a[0].bpm == Approx(120.0f));
    CHECK(take.tempo.a[0].sample == sampleAt(5.0));
    CHECK(take.meta.startBeatInBar == Approx(3.84));
}

TEST_CASE("waits for the snapshot that carries the request", "[tempo-start][gate]")
{
    Gate g(5u);
    g.tick(snapOf(4, 0.0f, false), 10.000);
    g.tick(snapOf(4, 0.0f, false), 10.008);
    CHECK(g.host.status().t == 0.0);
    g.tick(snapOf(5, 120.0f, true, 1, 0.25f), 10.017);
    g.tick(snapOf(5, 120.0f, true, 1, 0.25f), 10.025);
    CHECK(g.host.status().t == Approx(0.008));
    const Take take = g.stopAndLoad();
    REQUIRE(take.tempo.a.size() == 1);
    CHECK(take.tempo.a[0].why == "start");
    CHECK(take.tempo.a[0].bpm == Approx(120.0f));
    CHECK(take.tempo.a[0].sample == sampleAt(10.017));
    CHECK(take.meta.startBeatInBar == Approx(1.25));
}

TEST_CASE("no hop carries the request -> t = 0 by the fallback", "[tempo-start][fallback]")
{
    CerrCapture err;
    const double fb = RecorderHost::kStartWaitFallbackSeconds;
    Gate g(5u);
    // Never on the exact boundary in doubles.
    for (double w : { 10.0, 10.0 + 0.4 * fb, 10.0 + 0.8 * fb, 10.0 + fb - 0.01 })
    {
        g.tick(snapOf(4, 0.0f, false), w);
        CHECK(g.host.status().t == 0.0);
    }
    CHECK(err.text.str().find("take start:") == std::string::npos);
    const double start = 10.0 + fb + 0.025;
    g.tick(snapOf(4, 0.0f, false), start);                         // the fallback starts the take here
    CHECK(err.text.str().find("take start:") != std::string::npos);
    g.tick(snapOf(5, 120.0f, true), start + 0.010);                 // the request lands late: a "lock" anchor
    const Take take = g.stopAndLoad();
    REQUIRE(take.tempo.a.size() == 2);
    CHECK(take.tempo.a[0].why == "start");
    CHECK(take.tempo.a[0].t == 0.0);
    CHECK(take.tempo.a[0].bpm == 0.0f);
    CHECK(take.tempo.a[0].sample == sampleAt(start));
    CHECK(take.tempo.a[1].why == "lock");
    CHECK(take.tempo.a[1].t == Approx(0.010));
    CHECK(take.tempo.a[1].bpm == Approx(120.0f));
    CHECK(take.meta.startBeatInBar == -1.0);
}

TEST_CASE("a Stop before t = 0 still gives the take its start", "[tempo-start][stop]")
{
    Gate g(5u);
    g.tick(snapOf(4, 128.0f, true, 2, 0.5f), 20.000);
    g.tick(snapOf(4, 128.0f, true, 2, 0.5f), 20.008);
    const Take take = g.stopAndLoad();
    REQUIRE(take.tempo.a.size() == 1);
    CHECK(take.tempo.a[0].why == "start");
    CHECK(take.tempo.a[0].bpm == Approx(128.0f));
    CHECK(take.tempo.a[0].sample == sampleAt(20.008));
    CHECK(take.meta.startBeatInBar == Approx(2.5));
    CHECK(take.meta.duration == 0.0);
}

TEST_CASE("the sequence compare survives 2^32", "[tempo-start][wrap]")
{
    Gate g(0xFFFFFFFEu);
    g.tick(snapOf(0xFFFFFFFDu, 120.0f, true), 30.000);
    g.tick(snapOf(0xFFFFFFFDu, 120.0f, true), 30.008);
    CHECK(g.host.status().t == 0.0);
    g.tick(snapOf(1u, 120.0f, true), 30.016);                      // wrapped past 0xFFFFFFFE: carries it
    g.tick(snapOf(1u, 120.0f, true), 30.024);
    CHECK(g.host.status().t == Approx(0.008));
}

TEST_CASE("onset markers: none lost to the wait, none stamped before t = 0 (Pitfall 30)", "[tempo-start][onset]")
{
    // HARMONY ADOPTION A5: the onset baseline is the FIRST tick after arm (as before); an onset seen while
    // t = 0 waits is emitted at t = 0, stamped with the start tick's clock (t 0, its sample) -- never dropped,
    // never sample 0.
    Gate g(5u, true);
    g.tick(snapOf(4, 0.0f, false, 0, 0.0f, 10), 40.000);           // baseline 10
    g.tick(snapOf(4, 0.0f, false, 0, 0.0f, 11), 40.004);           // an onset while waiting
    g.tick(snapOf(5, 120.0f, true, 0, 0.0f, 11), 40.008);          // t = 0
    g.tick(snapOf(5, 120.0f, true, 0, 0.0f, 12), 40.016);          // an onset after t = 0
    const Take take = g.stopAndLoad();
    REQUIRE(take.markers.size() == 2);
    CHECK(take.markers[0].s.t == 0.0);
    CHECK(take.markers[0].s.sample == sampleAt(40.008));
    CHECK(take.markers[1].s.t == Approx(0.008));
    CHECK(take.markers[1].s.sample == sampleAt(40.016));
}

TEST_CASE("[lint] perfRecord arms with the request sequence, not an arm-time bus read", "[tempo-start][lint]")
{
    // A structural lint (a source-text scan), not behaviour proof: MainComponent.cpp is compiled by no ctest.
    // The arm -> postedRequestSeq wire and the startBeatInBar source are proven only by the live
    // probe-tempo-start.sh W1-W3 on the built app.
    const std::string src = readSource("MainComponent.cpp");
    const auto from = src.find("std::string MainComponent::perfRecord(");
    const auto to = src.find("std::string MainComponent::perfStop(");
    REQUIRE(from != std::string::npos);
    REQUIRE(to != std::string::npos);
    REQUIRE(from < to);
    const std::string body = src.substr(from, to - from);
    CHECK(body.find("postedRequestSeq()") != std::string::npos);
    CHECK(body.find("testMode_") != std::string::npos);
    CHECK(body.find("startBeatInBar") == std::string::npos);
    CHECK(body.find("getFeatureBus().read()") == std::string::npos);
}
