// test_recorder_host -- s-rta-0923 step 3 / s-rta-0924 Lane S3-A: RecorderHost's
// arm -> tick -> disarm lifecycle, the provisional/periodic Take::save path
// (v2 5.6 #1), the R14 gapDetection glue, discrete/continuous capture and
// replay through a fake Dispatch, the overdub asset-frame clock (5.2), a
// mid-take self-stop (R-A5), and PerfStateCapture's round trip.
//
// Drives AudioTap DIRECTLY (prepare/start/push/stop), the same idiom
// test_audio_store.cpp uses -- not the full CombinedCallback/FakeAudioIODevice
// harness test_audio_tap_sync.cpp needs (which proves AudioTap's own
// sample-alignment contract, already covered there). RecorderHost's contract
// is arm()/tick()/disarm() calling AudioTap correctly, not AudioTap's own
// internals -- driving it directly keeps every test here about RecorderHost.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Composition.h"
#include "recording/RecorderHost.h"
#include "recording/PerfStateCapture.h"
#include "recording/Take.h"
#include "recording/Program.h"
#include "recording/Player.h"
#include "recording/AudioTap.h"
#include "recording/AudioStore.h"
#include "recording/Lane.h"
#include "connect/AutomationCurve.h"
#include "analysis/FeatureSnapshot.h"
#include <atomic>
#include <cmath>
#include <optional>
#include <thread>
#include <vector>

using Catch::Approx;

namespace
{
    ControlPath layerKey(int layer, const std::string& control)
    {
        ControlPath k;
        k.scope = ControlPath::Scope::Layer;
        k.deckRelative = true;
        k.layer = layer;
        k.control = control;
        return k;
    }

    ControlPath compKey(const std::string& control)
    {
        ControlPath k;
        k.scope = ControlPath::Scope::Comp;
        k.control = control;
        return k;
    }

    FeatureSnapshot makeSnap(float bpm = 120.0f, float phase = 0.0f)
    {
        FeatureSnapshot s;
        s.clear();
        s.bpm = bpm;
        s.beatPhase = phase;
        return s;
    }

    // s167 D10.3 onset-marker dedupe: a snapshot with onsetDetected set, stamped with the given
    // FeatureBus timestamp -- two ticks reading the SAME timestamp simulate FeatureBus::read()'s
    // always-latest semantics returning the same published snapshot twice.
    FeatureSnapshot makeOnsetSnap(uint64_t timestamp, float bpm = 120.0f, float phase = 0.0f)
    {
        FeatureSnapshot s = makeSnap(bpm, phase);
        s.onsetDetected = true;
        s.timestamp = timestamp;
        return s;
    }

    struct TempDir
    {
        juce::File dir;
        explicit TempDir(const juce::String& tag)
            : dir(juce::File::getSpecialLocation(juce::File::tempDirectory)
                      .getChildFile("audiodna_recorder_host_test_" + tag + "_"
                                    + juce::String(juce::Random::getSystemRandom().nextInt64())))
        {
            dir.createDirectory();
        }
        ~TempDir() { dir.deleteRecursively(); }
    };

    // Minimal one-deck, three-layer, default composition (Deck::initDefault) --
    // enough for arm()'s checkpoint0/checkpointEnd capture and Program::compile's
    // Layer-scope (deckRelative) resolution.
    Composition makeComposition()
    {
        Composition c;
        c.decks.resize(1);
        c.decks[0].initDefault();
        c.activeDeckIndex = 0;
        return c;
    }

    // Wires every Dispatch slot RecorderHost needs, recording every call for
    // inspection -- the `dispatch.continuous` half is wired too (tests that
    // need it EMPTY, e.g. test 10, wire fire/capturePerfState/notify by hand
    // instead of using this helper).
    struct FakeDispatch
    {
        std::vector<Fired> fired;
        std::vector<std::pair<ControlPath, std::string>> touches;
        std::vector<std::pair<ControlPath, float>> sets;
        std::vector<ControlPath> releases;
        std::vector<std::string> notices;
        bool refuseFire = false;

        void wire(RecorderHost& host, const Composition* comp)
        {
            host.dispatch.fire = [this](const Fired& f) { fired.push_back(f); return !refuseFire; };
            host.dispatch.continuous.touch = [this](const ControlPath& k, const std::string& g)
            { touches.emplace_back(k, g); return true; };
            host.dispatch.continuous.set = [this](const ControlPath& k, float v)
            { sets.emplace_back(k, v); return true; };
            host.dispatch.continuous.release = [this](const ControlPath& k) { releases.push_back(k); };
            host.dispatch.capturePerfState = [comp]() { return comp ? capturePerfState(*comp, 120.0f, "") : PerfState{}; };
            host.dispatch.notify = [this](const std::string& s) { notices.push_back(s); };
        }
    };

    // Drives a real AudioTap through `n` blocks of silence at `rate`/`blockSize`,
    // with a valid, gap-free hostTimeNs -- the common "just advance the tap"
    // shape several tests below share.
    void pushCleanBlocks(AudioTap& tap, double rate, int blockSize, int n,
                         uint64_t& delivered, uint64_t& hostTimeNs)
    {
        std::vector<float> inL(static_cast<size_t>(blockSize), 0.0f), inR(static_cast<size_t>(blockSize), 0.0f);
        const float* ch[2] = { inL.data(), inR.data() };
        for (int b = 0; b < n; ++b)
        {
            hostTimeNs += static_cast<uint64_t>((static_cast<double>(blockSize) / rate) * 1.0e9);
            juce::AudioIODeviceCallbackContext ctx;
            ctx.hostTimeNs = &hostTimeNs;
            const uint32_t gap = tap.push(ch, 2, blockSize, delivered, ctx);
            delivered += static_cast<uint64_t>(blockSize) + gap;
        }
    }
}

// === 1: [host][arm] provisional take.json exists before the first tick ===

TEST_CASE("RecorderHost arm -- provisional take.json exists before the first tick and carries the asset id", "[host][arm]")
{
    TempDir storeRoot("arm_store");
    TempDir takeFolder("arm_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap tap;
    tap.prepare(48000.0, 2, 512);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = 48000.0;
    opts.deviceChannels = 2;
    opts.appVersion = "test";
    opts.gripHoldMs = 250.0f;

    const auto armRes = host.arm(comp, tap, opts);
    REQUIRE(armRes.ok);
    REQUIRE_FALSE(armRes.assetId.empty());
    CHECK(host.isRecording());

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->audio.segments.size() == 1);
    CHECK(loaded->audio.segments[0].id == armRes.assetId);
    CHECK(loaded->audio.segments[0].fingerprint.empty());
    CHECK(loaded->audio.segments[0].frames == 0);
    CHECK(loaded->audio.mode == "input");

    const auto resolution = store.resolve(loaded->audio);
    CHECK(resolution.status == AudioStore::Status::Incomplete);   // wav exists, no sidecar -- D-A3's repairable state

    host.disarm(comp, tap);
}

// === 2: [host][periodic] a save lands every 60s of clock time and refreshes ===

TEST_CASE("RecorderHost periodic -- a save lands every 60s of clock time and refreshes frames/firstSample/gaps/gapDetection", "[host][periodic]")
{
    TempDir storeRoot("periodic_store");
    TempDir takeFolder("periodic_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    AudioTap tap;
    tap.prepare(rate, 2, blockSize);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = rate;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, tap, opts).ok);

    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;

    host.tick(makeSnap(), 0.0, delivered, tap, std::nullopt, rate);   // seeds t=0 -- no save yet
    pushCleanBlocks(tap, rate, blockSize, 5, delivered, hostTimeNs);

    host.tick(makeSnap(), 65.0, delivered, tap, std::nullopt, rate);   // crosses the 60s boundary

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->audio.segments.size() == 1);
    CHECK(loaded->audio.segments[0].frames == tap.framesWritten());
    CHECK(loaded->audio.segments[0].firstSample == tap.firstSample());
    CHECK(loaded->audio.gapDetection == tap.gapDetectionSupported());
    CHECK(loaded->meta.duration == Approx(65.0));

    host.disarm(comp, tap);
}

// === 3: [host][r14] gapDetection false reaches the PERIODIC and the FINAL take ===

TEST_CASE("RecorderHost R14 -- gapDetection false reaches the periodic and the final take", "[host][r14]")
{
    TempDir storeRoot("r14_store");
    TempDir takeFolder("r14_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    AudioTap tap;
    tap.prepare(rate, 2, blockSize);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = rate;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, tap, opts).ok);
    CHECK(tap.gapDetectionSupported());   // optimistic default until a null-hostTimeNs push proves otherwise

    uint64_t delivered = 0;
    std::vector<float> inL(blockSize, 0.0f), inR(blockSize, 0.0f);
    const float* ch[2] = { inL.data(), inR.data() };
    for (int b = 0; b < 5; ++b)
    {
        juce::AudioIODeviceCallbackContext ctx;   // hostTimeNs defaults nullptr -- R14's "host doesn't provide this"
        const uint32_t gap = tap.push(ch, 2, blockSize, delivered, ctx);
        delivered += static_cast<uint64_t>(blockSize) + gap;
    }
    REQUIRE_FALSE(tap.gapDetectionSupported());   // latched off after the first null-hostTimeNs push

    host.tick(makeSnap(), 0.0, delivered, tap, std::nullopt, rate);
    host.tick(makeSnap(), 65.0, delivered, tap, std::nullopt, rate);

    LoadStats stats;
    auto periodic = Take::load(takeFolder.dir, stats);
    REQUIRE(periodic.has_value());
    CHECK_FALSE(periodic->audio.gapDetection);

    const auto stopRes = host.disarm(comp, tap);
    CHECK_FALSE(stopRes.gapDetection);

    auto final_ = Take::load(takeFolder.dir, stats);
    REQUIRE(final_.has_value());
    CHECK_FALSE(final_->audio.gapDetection);
}

// === 4: [host][gaps] 80 gaps survive the per-tick drain ===

TEST_CASE("RecorderHost gaps -- 80 gaps survive the per-tick drain", "[host][gaps]")
{
    TempDir storeRoot("gaps_store");
    TempDir takeFolder("gaps_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    constexpr double rate = 48000.0;
    constexpr int blockSize = 256;
    AudioTap tap;
    tap.prepare(rate, 2, blockSize);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = rate;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, tap, opts).ok);

    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    std::vector<float> inL(blockSize, 0.0f), inR(blockSize, 0.0f);
    const float* ch[2] = { inL.data(), inR.data() };

    // Baseline block -- haveLastHostTime_ starts false right after tap.start(), so this one
    // establishes the reference and cannot itself produce a gap.
    hostTimeNs += static_cast<uint64_t>((static_cast<double>(blockSize) / rate) * 1.0e9);
    {
        juce::AudioIODeviceCallbackContext ctx; ctx.hostTimeNs = &hostTimeNs;
        const uint32_t gap = tap.push(ch, 2, blockSize, delivered, ctx);
        delivered += static_cast<uint64_t>(blockSize) + gap;
    }
    host.tick(makeSnap(), 0.0, delivered, tap, std::nullopt, rate);

    for (int i = 0; i < 80; ++i)
    {
        // A 2x delta -> exactly one gap of blockSize frames each iteration.
        hostTimeNs += static_cast<uint64_t>((2.0 * blockSize / rate) * 1.0e9);
        juce::AudioIODeviceCallbackContext ctx; ctx.hostTimeNs = &hostTimeNs;
        const uint32_t gap = tap.push(ch, 2, blockSize, delivered, ctx);
        delivered += static_cast<uint64_t>(blockSize) + gap;

        host.tick(makeSnap(), 0.01 * static_cast<double>(i + 1), delivered, tap, std::nullopt, rate);   // R-A4: drained EVERY tick
    }

    const auto stopRes = host.disarm(comp, tap);
    CHECK(stopRes.gaps == 80);

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    CHECK(loaded->audio.gaps.size() == 80);
}

// === 5: [host][stop] disarm finalizes, references, saves ===

TEST_CASE("RecorderHost stop -- disarm finalizes, references, saves; take folder has no audio.wav; resolve == Resolved", "[host][stop]")
{
    TempDir storeRoot("stop_store");
    TempDir takeFolder("stop_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    AudioTap tap;
    tap.prepare(rate, 2, blockSize);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = rate;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, tap, opts).ok);

    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    host.tick(makeSnap(), 0.0, delivered, tap, std::nullopt, rate);
    pushCleanBlocks(tap, rate, blockSize, 10, delivered, hostTimeNs);
    host.tick(makeSnap(), 1.0, delivered, tap, std::nullopt, rate);

    const auto stopRes = host.disarm(comp, tap);
    CHECK(stopRes.ok);
    CHECK_FALSE(host.isRecording());
    CHECK_FALSE(takeFolder.dir.getChildFile("audio.wav").existsAsFile());   // ruling 28: audio lives in the store, not the take folder

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    const auto resolution = store.resolve(loaded->audio);
    CHECK(resolution.status == AudioStore::Status::Resolved);
    CHECK(resolution.asset.frames == stopRes.frames);
    CHECK(loaded->checkpointEnd.activeDeckIndex == comp.activeDeckIndex);   // checkpointEnd was captured
}

// === 6: [host][arm] a store/tap failure refuses arm and touches nothing else ===

TEST_CASE("RecorderHost arm -- a store failure refuses arm and leaves nothing behind (R-A1)", "[host][arm]")
{
    TempDir storeRoot("arm_fail_store");
    TempDir takeFolder("arm_fail_take");

    // R-A1: beginAsset -> tap.start -> on EITHER failure, refuse and touch nothing else. Forcing
    // tap.start() itself to fail deterministically would need a pre-known (UUID-random) asset id;
    // making the store root unwritable exercises the SAME refuse-and-abandon contract one step
    // earlier (beginAsset), which is the deterministic, portable trigger available here.
    storeRoot.dir.setReadOnly(true, false);

    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap tap;
    tap.prepare(48000.0, 2, 512);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = 48000.0;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    const auto res = host.arm(comp, tap, opts);

    storeRoot.dir.setReadOnly(false, false);   // restore before the TempDir dtor deletes it

    CHECK_FALSE(res.ok);
    CHECK_FALSE(res.error.empty());
    CHECK(res.assetId.empty());
    CHECK_FALSE(host.isRecording());
    CHECK_FALSE(takeFolder.dir.getChildFile("take.json").existsAsFile());
    CHECK(storeRoot.dir.getNumberOfChildFiles(juce::File::findDirectories) == 0);   // nothing to abandon -- nothing was created
}

// === 7: [host][capture] discrete points get clock stamps; Origin::Replay is never recorded ===

TEST_CASE("RecorderHost capture -- discrete points get clock stamps; Origin::Replay is never recorded", "[host][capture]")
{
    TempDir storeRoot("capture_store");
    TempDir takeFolder("capture_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap dummyTap;

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = false;   // 5.5 -- no tap needed for this test
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, dummyTap, opts).ok);
    host.tick(makeSnap(), 0.0, 0, dummyTap, std::nullopt, 48000.0);   // seeds the clock

    const ControlPath key = layerKey(0, "activeClip");

    DiscretePoint human; human.origin = Origin::Human; human.v = 2;
    host.capture(key, std::move(human));

    DiscretePoint replay; replay.origin = Origin::Replay; replay.v = 3;   // R5: never recorded
    host.capture(key, std::move(replay));

    host.disarm(comp, dummyTap);

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->lanes.count(key) == 1);
    REQUIRE(loaded->lanes.at(key).points.size() == 1);   // only the Human point
    CHECK(loaded->lanes.at(key).points[0].v == 2);
    CHECK(loaded->lanes.at(key).points[0].origin == Origin::Human);
}

// === 8: [host][replay] discrete sequence fires through Dispatch in (at,seq) order ===

TEST_CASE("RecorderHost replay -- the discrete sequence fires through Dispatch in (at,seq) order; audio is skipped in WithAudio, fired in WallClock", "[host][replay]")
{
    TempDir storeRoot("replay_store");
    AudioStore store(storeRoot.dir);

    // A tiny, real, resolvable audio asset (test_audio_store.cpp's own direct-tap idiom) so
    // play(WithAudio) -- which refuses unless the audio actually resolves -- can run for real.
    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    AudioTap sourceTap;
    sourceTap.prepare(48000.0, 2, 512);
    REQUIRE(sourceTap.start(store.wavFile(*id)));
    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    pushCleanBlocks(sourceTap, 48000.0, 512, 20, delivered, hostTimeNs);
    sourceTap.stop();

    AudioStore::CaptureFacts facts;
    facts.mode = "input";
    facts.gapDetection = sourceTap.gapDetectionSupported();
    facts.firstSample = sourceTap.firstSample();
    facts.framesWritten = sourceTap.framesWritten();
    facts.rate = 48000.0;
    facts.channels = 2;
    facts.app = "test";
    const auto fin = store.finalize(*id, facts);
    REQUIRE(fin.error.empty());

    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath audioKey = compKey("audio");

    auto buildTake = [&](DriveClock clock)
    {
        Take take;
        DiscretePoint p1, p2, pa;
        p1.origin = Origin::Human; p1.v = 1;
        p2.origin = Origin::Human; p2.v = 2;
        pa.origin = Origin::Human; pa.action = "play"; pa.v = 0;
        if (clock == DriveClock::Wall)
        {
            p1.s.seq = 1; p1.s.t = 0.2; p1.beat = 1.0;
            p2.s.seq = 2; p2.s.t = 0.4; p2.beat = 2.0;
            pa.s.seq = 0; pa.s.t = 0.0; pa.beat = 0.0;
        }
        else
        {
            p1.s.seq = 1; p1.s.sample = 100; p1.beat = 1.0;
            p2.s.seq = 2; p2.s.sample = 200; p2.beat = 2.0;
            pa.s.seq = 0; pa.s.sample = 0; pa.beat = 0.0;
        }
        Lane laneClip; laneClip.key = clipKey; laneClip.kind = Lane::Kind::Discrete; laneClip.points = { p1, p2 };
        Lane laneAudio; laneAudio.key = audioKey; laneAudio.kind = Lane::Kind::Discrete; laneAudio.points = { pa };
        take.lanes[clipKey] = laneClip;
        take.lanes[audioKey] = laneAudio;
        take.nextSeq = 3;
        take.audio = AudioStore::referencing(fin.asset, 0);
        return take;
    };

    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    SECTION("WallClock -- audio point fires, (at,seq) order preserved")
    {
        TempDir folder("replay_wall_take");
        REQUIRE(buildTake(DriveClock::Wall).save(folder.dir));
        REQUIRE(host.load(folder.dir).ok);

        const double w0 = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        const auto playRes = host.play(RecorderHost::PlayMode::WallClock, comp);
        REQUIRE(playRes.ok);

        AudioTap dummyTap;
        host.tick(makeSnap(), w0 + 0.5, 0, dummyTap, std::nullopt, 48000.0);

        const auto st = host.status();
        CHECK(st.skipped == 0);
        REQUIRE(fake.fired.size() == 3);
        CHECK(fake.fired[0].key.control == "audio");   // at=0.0, earliest
        CHECK(fake.fired[1].p.v == 1);
        CHECK(fake.fired[2].p.v == 2);
        host.stopPlay();
    }

    SECTION("WithAudio -- the audio point is skipped, never reaches dispatch.fire")
    {
        TempDir folder("replay_audio_take");
        REQUIRE(buildTake(DriveClock::Sample).save(folder.dir));
        REQUIRE(host.load(folder.dir).ok);

        const auto playRes = host.play(RecorderHost::PlayMode::WithAudio, comp);
        REQUIRE(playRes.ok);

        AudioTap dummyTap;
        host.tick(makeSnap(), 0.0, 0, dummyTap, std::optional<int64_t>(300), 48000.0);

        const auto st = host.status();
        CHECK(st.skipped == 1);   // the `audio` point -- HostSink refuses it before dispatch.fire is even called
        int clipFires = 0;
        for (const auto& f : fake.fired)
            if (f.key.control == "activeClip")
                ++clipFires;
        CHECK(clipFires == 2);
        host.stopPlay();
    }
}

// === 9: [host][overdub] arm against a stored asset uses firstSample 0 and the asset-frame clock ===

TEST_CASE("RecorderHost overdub -- arm against a stored asset uses firstSample 0 and the asset-frame clock", "[host][overdub]")
{
    TempDir storeRoot("overdub_store");
    AudioStore store(storeRoot.dir);

    auto id = store.beginAsset();
    REQUIRE(id.has_value());
    AudioTap sourceTap;
    sourceTap.prepare(44100.0, 2, 512);
    REQUIRE(sourceTap.start(store.wavFile(*id)));
    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    pushCleanBlocks(sourceTap, 44100.0, 512, 10, delivered, hostTimeNs);
    sourceTap.stop();

    AudioStore::CaptureFacts facts;
    facts.mode = "input";
    facts.gapDetection = sourceTap.gapDetectionSupported();
    facts.firstSample = sourceTap.firstSample();
    facts.framesWritten = sourceTap.framesWritten();
    facts.rate = 44100.0;
    facts.channels = 2;
    facts.app = "test";
    const auto fin = store.finalize(*id, facts);
    REQUIRE(fin.error.empty());
    REQUIRE(fin.asset.rate == Approx(44100.0));

    RecorderHost host(store);
    TempDir takeFolder("overdub_take");
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap unusedTap;   // R-A6: no tap in overdub, still required by the API
    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = 48000.0;
    opts.deviceChannels = 2;
    opts.appVersion = "test";
    opts.overdubAssetId = *id;

    const auto armRes = host.arm(comp, unusedTap, opts);
    REQUIRE(armRes.ok);
    CHECK(armRes.assetId == *id);

    host.tick(makeSnap(), 0.0, 0, unusedTap, std::nullopt, 48000.0);   // seed t=0
    const int64_t transportFrames = 48000;   // 1 second of device-rate transport position
    host.tick(makeSnap(), 1.0, 0, unusedTap, std::optional<int64_t>(transportFrames), 48000.0);

    const auto st = host.status();
    const uint64_t expectedAssetFrame =
        static_cast<uint64_t>(std::llround(static_cast<double>(transportFrames) * 44100.0 / 48000.0));
    CHECK((st.sample <= expectedAssetFrame + 1 && st.sample + 1 >= expectedAssetFrame));   // +/- 1

    const auto stopRes = host.disarm(comp, unusedTap);
    CHECK(stopRes.ok);

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->audio.segments.size() == 1);
    CHECK(loaded->audio.segments[0].id == *id);
    CHECK(loaded->audio.segments[0].firstSample == 0);
}

// === 10: [host][continuous] empty Dispatch::continuous refuses loudly ===

TEST_CASE("RecorderHost continuous -- empty Dispatch::continuous refuses loudly", "[host][continuous]")
{
    const ControlPath key = [] { ControlPath k = layerKey(0, "scalar"); k.scalar = "opacity"; return k; }();

    Gesture g;
    g.grip = "decaying";
    g.origin = Origin::Human;
    g.curve.pts = { { 0.0, 0.2f, Breakpoint::Interp::Linear }, { 1.0, 0.8f, Breakpoint::Interp::Linear } };
    g.stamps = { { 1, 0.0, 0 }, { 2, 1.0, 48000 } };

    Lane lane; lane.key = key; lane.kind = Lane::Kind::Continuous; lane.gestures = { g };
    Take take;
    take.lanes[key] = lane;
    take.nextSeq = 3;

    TempDir folder("continuous_take");
    REQUIRE(take.save(folder.dir));

    TempDir storeRoot("continuous_store");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();

    // Deliberately leave dispatch.continuous EMPTY -- only fire/capturePerfState/notify are wired.
    host.dispatch.fire = [](const Fired&) { return true; };
    host.dispatch.capturePerfState = [&comp]() { return capturePerfState(comp, 120.0f, ""); };

    REQUIRE(host.load(folder.dir).ok);

    const double w0 = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    const auto playRes = host.play(RecorderHost::PlayMode::WallClock, comp);
    REQUIRE(playRes.ok);

    AudioTap dummyTap;
    host.tick(makeSnap(), w0 + 0.5, 0, dummyTap, std::nullopt, 48000.0);

    const auto st = host.status();
    CHECK(st.continuousUnavailable > 0);
    CHECK(st.playing);   // no crash -- the lane is simply displaced (Player's existing TOUCH path)

    host.stopPlay();
}

// === 11: [host][selfstop] a mid-take rate change ends the audio early ===

TEST_CASE("RecorderHost selfstop -- a mid-take rate change ends the audio early", "[host][selfstop]")
{
    TempDir storeRoot("selfstop_store");
    TempDir takeFolder("selfstop_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap tap;
    tap.prepare(48000.0, 2, 512);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = 48000.0;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, tap, opts).ok);

    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    host.tick(makeSnap(), 0.0, delivered, tap, std::nullopt, 48000.0);
    pushCleanBlocks(tap, 48000.0, 512, 5, delivered, hostTimeNs);
    host.tick(makeSnap(), 0.1, delivered, tap, std::nullopt, 48000.0);

    const uint64_t firstSample = tap.firstSample();
    const uint64_t framesBefore = tap.framesWritten();
    REQUIRE(framesBefore == 2560);

    tap.prepare(44100.0, 2, 512);   // rate change mid-take -> self-stop (AudioTap.h/.cpp's own contract)
    REQUIRE_FALSE(tap.isRunning());

    host.tick(makeSnap(), 0.2, delivered, tap, std::nullopt, 44100.0);   // observes the self-stop

    const auto stAfter = host.status();
    CHECK_FALSE(stAfter.lastError.empty());

    const auto stopRes = host.disarm(comp, tap);
    CHECK(stopRes.ok);

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->audio.unreliableFrom.has_value());
    CHECK(*loaded->audio.unreliableFrom == firstSample + framesBefore);
}

// === 12: [host][status][concurrency] status() from a second thread while tick() runs ===

TEST_CASE("RecorderHost status -- concurrent status() reads while tick() runs", "[host][status][concurrency]")
{
    TempDir storeRoot("concurrency_store");
    TempDir takeFolder("concurrency_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap dummyTap;
    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = false;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, dummyTap, opts).ok);

    std::atomic<bool> stop{ false };
    std::thread reader([&]
    {
        while (!stop.load(std::memory_order_relaxed))
        {
            const auto st = host.status();
            (void) st;
        }
    });

    const double w0 = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    for (int i = 0; i < 200; ++i)
        host.tick(makeSnap(), w0 + static_cast<double>(i) * 0.01, 0, dummyTap, std::nullopt, 48000.0);

    stop.store(true, std::memory_order_relaxed);
    reader.join();

    host.disarm(comp, dummyTap);
    SUCCEED("status()/tick() ran concurrently with no diagnostic");
}

// === 13: [perfstate] capturePerfState from a Composition ===

TEST_CASE("PerfStateCapture -- captures non-default state from a Composition and round-trips", "[perfstate]")
{
    Composition comp = makeComposition();

    Layer& layer0 = comp.decks[0].layers[0];
    Clip clip; clip.name = "TestClip"; clip.playing = true;
    layer0.clips[0] = clip;
    layer0.activeClipColumn = 0;

    comp.decks[0].layers[1].opacity = 0.4f;
    comp.decks[0].layers[2].bypassed = true;

    const PerfState state = capturePerfState(comp, 128.0f, "play");

    const juce::var v = state.toVar();
    const PerfState round = PerfState::fromVar(v);

    CHECK(round.activeDeckIndex == 0);
    CHECK(round.bpm == Approx(128.0f));
    CHECK(round.audioAction == "play");
    REQUIRE(round.decks.count(0) == 1);
    REQUIRE(round.decks.at(0).layers.count(1) == 1);
    CHECK(round.decks.at(0).layers.at(1).opacity == Approx(0.4f));
    REQUIRE(round.decks.at(0).layers.count(2) == 1);
    CHECK(round.decks.at(0).layers.at(2).bypassed);
    REQUIRE(round.decks.at(0).layers.count(0) == 1);
    REQUIRE(round.decks.at(0).layers.at(0).clips.count(0) == 1);
    CHECK(round.decks.at(0).layers.at(0).clips.at(0).playing);
    CHECK(round.decks.at(0).layers.at(0).clips.at(0).clip == "TestClip");
}

// === 14: [host][decaying] a Decaying gesture with no release gets an exact end after gripHoldMs ===

TEST_CASE("RecorderHost decaying -- a Decaying gesture with no release gets an exact end after gripHoldMs of silence", "[host][decaying]")
{
    TempDir storeRoot("decaying_store");
    TempDir takeFolder("decaying_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap dummyTap;
    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = false;
    opts.appVersion = "test";
    opts.gripHoldMs = 100.0f;

    REQUIRE(host.arm(comp, dummyTap, opts).ok);

    const ControlPath key = [] { ControlPath k = layerKey(0, "scalar"); k.scalar = "opacity"; return k; }();

    const double w0 = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    host.tick(makeSnap(), w0, dummyTap.framesWritten(), dummyTap, std::nullopt, 48000.0);   // seed t=0 on the SAME real-clock basis

    host.onHumanWrite(key, 0.5f, "decaying");

    host.tick(makeSnap(), w0 + 0.03, dummyTap.framesWritten(), dummyTap, std::nullopt, 48000.0);   // within gripHoldMs -- gesture still open
    const auto stMid = host.status();
    CHECK(stMid.gestures == 0);

    host.tick(makeSnap(), w0 + 0.2, dummyTap.framesWritten(), dummyTap, std::nullopt, 48000.0);    // past gripHoldMs -- synthesized end

    const auto stopRes = host.disarm(comp, dummyTap);
    (void) stopRes;

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->lanes.count(key) == 1);
    CHECK(loaded->lanes.at(key).kind == Lane::Kind::Continuous);
    REQUIRE(loaded->lanes.at(key).gestures.size() == 1);
    CHECK(loaded->lanes.at(key).gestures[0].grip == "decaying");
    CHECK(loaded->lanes.at(key).gestures[0].curve.pts.size() >= 2);   // begin + synthesized exact end
}

// === 15: [host][capture] onHumanWrite with no prior touch opens the gesture ===

TEST_CASE("RecorderHost onHumanWrite -- with no prior touch opens the gesture", "[host][capture]")
{
    // Fail-first against the UNAMENDED shape (plan's original onHumanSet(key,v)): every OSC/MIDI/
    // REST manual-write site is a single manualWrite call with no preceding manualTouch --
    // PerformanceRecorder::set() silently ignores a set() with no open gesture
    // (PerformanceRecorder.cpp's own "stray set()" comment), so onHumanSet without an open-gesture
    // check would drop this capture entirely (an EMPTY lane, or no lane at all). onHumanWrite (A2)
    // fixes this by opening the gesture itself when none is open.
    TempDir storeRoot("onhumanwrite_store");
    TempDir takeFolder("onhumanwrite_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap dummyTap;
    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = false;
    opts.appVersion = "test";
    opts.gripHoldMs = 250.0f;

    REQUIRE(host.arm(comp, dummyTap, opts).ok);

    const ControlPath key = [] { ControlPath k = layerKey(0, "scalar"); k.scalar = "opacity"; return k; }();

    host.tick(makeSnap(), 0.0, 0, dummyTap, std::nullopt, 48000.0);

    host.onHumanWrite(key, 0.3f, "decaying");   // NO preceding onHumanTouch -- the real-world shape
    host.onHumanWrite(key, 0.6f, "decaying");
    host.onHumanRelease(key);

    host.disarm(comp, dummyTap);

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->lanes.count(key) == 1);
    REQUIRE(loaded->lanes.at(key).kind == Lane::Kind::Continuous);
    REQUIRE(loaded->lanes.at(key).gestures.size() == 1);   // NOT empty/zero -- the amendment's whole point
    CHECK(loaded->lanes.at(key).gestures[0].curve.pts.size() >= 2);
}

// === 16: [host][selfstop] a tick before the first push is never misread as a self-stop ===
//
// Fail-first against the unfixed code: arm() used to seed `tapWasRunningLastTick_ = true`
// immediately after tap.start() -- but tap.start() only ARMS the tap; AudioTap::running_ flips true
// inside push(), on the audio thread's NEXT callback, not synchronously with start(). RecorderHost::
// tick() runs from an independent 120 Hz message-thread timer, so a tick landing between arm() and
// the first push() saw `tapWasRunningLastTick_ == true` but `tap.isRunning() == false` and fired a
// false-positive "audio tap self-stopped mid-take" notify on nearly every arm.

TEST_CASE("RecorderHost selfstop -- a tick before the first push is never misread as a self-stop", "[host][selfstop]")
{
    TempDir storeRoot("selfstop_prepush_store");
    TempDir takeFolder("selfstop_prepush_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap tap;
    tap.prepare(48000.0, 2, 512);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = 48000.0;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, tap, opts).ok);
    REQUIRE_FALSE(tap.isRunning());   // armed, not yet running -- the exact window the bug misread

    // Tick BEFORE any push() call -- the 120 Hz message-thread timer can and does land here.
    host.tick(makeSnap(), 0.0, 0, tap, std::nullopt, 48000.0);

    const auto st = host.status();
    CHECK(st.lastError.empty());
    for (const auto& n : fake.notices)
        CHECK(n.find("self-stopped") == std::string::npos);

    host.disarm(comp, tap);
}

// === 17: [host][selfstop] a genuine running-to-stopped transition is reported exactly once ===

TEST_CASE("RecorderHost selfstop -- a genuine running-to-stopped transition is reported exactly once", "[host][selfstop]")
{
    TempDir storeRoot("selfstop_once_store");
    TempDir takeFolder("selfstop_once_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap tap;
    tap.prepare(48000.0, 2, 512);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = 48000.0;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, tap, opts).ok);

    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    pushCleanBlocks(tap, 48000.0, 512, 5, delivered, hostTimeNs);
    REQUIRE(tap.isRunning());

    host.tick(makeSnap(), 0.1, delivered, tap, std::nullopt, 48000.0);   // observes it running -- no fire
    CHECK(host.status().lastError.empty());

    tap.prepare(44100.0, 2, 512);   // rate change mid-take -> genuine self-stop (AudioTap's own contract)
    REQUIRE_FALSE(tap.isRunning());

    host.tick(makeSnap(), 0.2, delivered, tap, std::nullopt, 44100.0);   // observes the self-stop -- fires once

    const auto st = host.status();
    CHECK_FALSE(st.lastError.empty());

    int selfStopNotices = 0;
    for (const auto& n : fake.notices)
        if (n.find("self-stopped") != std::string::npos)
            ++selfStopNotices;
    CHECK(selfStopNotices == 1);

    host.disarm(comp, tap);
}

// === 18: [host][selfstop] after a reported self-stop, a recovered tap re-arms the edge ===

TEST_CASE("RecorderHost selfstop -- after a reported self-stop, a recovered tap reports a genuine second stop (re-armed edge)", "[host][selfstop]")
{
    TempDir storeRoot("selfstop_rearm_store");
    TempDir takeFolder("selfstop_rearm_take");
    TempDir recoverDir("selfstop_rearm_recover");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap tap;
    tap.prepare(48000.0, 2, 512);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = 48000.0;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, tap, opts).ok);

    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    pushCleanBlocks(tap, 48000.0, 512, 5, delivered, hostTimeNs);
    host.tick(makeSnap(), 0.1, delivered, tap, std::nullopt, 48000.0);   // running -- no fire

    tap.prepare(44100.0, 2, 512);   // first genuine self-stop
    REQUIRE_FALSE(tap.isRunning());
    host.tick(makeSnap(), 0.2, delivered, tap, std::nullopt, 44100.0);

    auto countSelfStops = [&]
    {
        int n = 0;
        for (const auto& s : fake.notices)
            if (s.find("self-stopped") != std::string::npos)
                ++n;
        return n;
    };
    REQUIRE(countSelfStops() == 1);
    REQUIRE_FALSE(host.status().lastError.empty());

    // Recovery: the tap starts running again (e.g. the device came back) -- re-arm the SAME tap
    // object into a fresh file, the only way a real AudioTap goes from stopped back to running.
    REQUIRE(tap.start(recoverDir.dir.getChildFile("recovered.wav")));
    pushCleanBlocks(tap, 44100.0, 512, 5, delivered, hostTimeNs);
    REQUIRE(tap.isRunning());
    host.tick(makeSnap(), 0.3, delivered, tap, std::nullopt, 44100.0);   // observes recovery -- no new fire
    CHECK(countSelfStops() == 1);

    // A second genuine stop after recovery.
    tap.prepare(22050.0, 2, 512);
    REQUIRE_FALSE(tap.isRunning());
    host.tick(makeSnap(), 0.4, delivered, tap, std::nullopt, 22050.0);

    CHECK(countSelfStops() == 2);   // re-armed -- the second genuine stop is reported, not swallowed
    CHECK_FALSE(host.status().lastError.empty());

    host.disarm(comp, tap);
}

// === 19: [host][onsetmarker] one onset marker per onset EVENT, not per tick (s167 D10.3 dedupe) ===
//
// Diagnosis (step3gate1, 156 markers vs 108 detected onsets): 22 markers were DUPLICATES -- tick()
// fired marker("onset") on every 120 Hz tick where snap.onsetDetected was true, but FeatureBus::read()
// is always-latest and analysis publishes slower (~93.75 Hz), so two consecutive ticks read the SAME
// published snapshot ~14% of the time. Fix: dedupe on FeatureSnapshot::timestamp.

TEST_CASE("RecorderHost onset marker -- two ticks reading the SAME onset snapshot produce exactly one marker", "[host][onsetmarker]")
{
    TempDir storeRoot("onsetmarker_dedupe_store");
    TempDir takeFolder("onsetmarker_dedupe_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap dummyTap;   // 5.5: no audio needed for this test

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = false;
    opts.appVersion = "test";
    opts.onsetMarkers = true;   // T2 enabler

    REQUIRE(host.arm(comp, dummyTap, opts).ok);

    // Two ticks reading the SAME published FeatureSnapshot (same `timestamp`) -- the always-latest
    // FeatureBus::read() scenario that produced the 22 duplicate markers in step3gate1.
    const FeatureSnapshot snap = makeOnsetSnap(1000);
    host.tick(snap, 0.0, 0, dummyTap, std::nullopt, 48000.0);
    host.tick(snap, 0.01, 0, dummyTap, std::nullopt, 48000.0);

    CHECK(host.status().markers == 1);

    host.disarm(comp, dummyTap);
}

TEST_CASE("RecorderHost onset marker -- a new onset snapshot (different timestamp) produces a second marker", "[host][onsetmarker]")
{
    TempDir storeRoot("onsetmarker_newevent_store");
    TempDir takeFolder("onsetmarker_newevent_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap dummyTap;

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = false;
    opts.appVersion = "test";
    opts.onsetMarkers = true;

    REQUIRE(host.arm(comp, dummyTap, opts).ok);

    host.tick(makeOnsetSnap(1000), 0.0, 0, dummyTap, std::nullopt, 48000.0);
    host.tick(makeOnsetSnap(1000), 0.01, 0, dummyTap, std::nullopt, 48000.0);   // same snapshot -- no new marker
    CHECK(host.status().markers == 1);

    host.tick(makeOnsetSnap(2000), 0.02, 0, dummyTap, std::nullopt, 48000.0);   // a genuinely new onset event
    CHECK(host.status().markers == 2);

    host.disarm(comp, dummyTap);
}

TEST_CASE("RecorderHost onset marker -- a new arm resets the dedupe state", "[host][onsetmarker]")
{
    TempDir storeRoot("onsetmarker_rearm_store");
    TempDir takeFolder1("onsetmarker_rearm_take1");
    TempDir takeFolder2("onsetmarker_rearm_take2");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap dummyTap;

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder1.dir;
    opts.audio = false;
    opts.appVersion = "test";
    opts.onsetMarkers = true;

    REQUIRE(host.arm(comp, dummyTap, opts).ok);
    host.tick(makeOnsetSnap(1000), 0.0, 0, dummyTap, std::nullopt, 48000.0);
    CHECK(host.status().markers == 1);
    host.disarm(comp, dummyTap);

    // Re-arm into a fresh take, then feed the SAME timestamp (1000) that the previous take already
    // marked -- without a reset, the stale lastOnsetMarkerSnapshot_ would swallow this take's very
    // first onset.
    opts.takeFolder = takeFolder2.dir;
    REQUIRE(host.arm(comp, dummyTap, opts).ok);
    host.tick(makeOnsetSnap(1000), 0.0, 0, dummyTap, std::nullopt, 48000.0);
    CHECK(host.status().markers == 1);

    host.disarm(comp, dummyTap);
}

// === 21: [host][r13] a non-48 kHz device that never changes never reports a rate change ===
//
// R13-C (plan .harmony/.reports/s-rta-0924/r13-plan.md 3.7): `rateMismatch` ("device != 48000 ->
// beat clock unreliable") is RETIRED -- the analysis thread now resamples to its fixed internal
// rate regardless of device rate (AnalysisResampler, R13 lane A). Fail-first against the UNFIXED
// code: arming at 44100 Hz used to call dispatch.notify("device rate 44100 Hz != analysis rate
// 48000 Hz: beat clock unreliable (R13)") unconditionally at arm() -- a device that simply runs at
// a non-48 kHz rate and never changes should notify ZERO times and read rateChangedSinceArm == false
// throughout.

TEST_CASE("RecorderHost r13 -- a device armed at 44100 Hz that never changes rate never reports a change", "[host][r13]")
{
    TempDir storeRoot("r13_stable_store");
    TempDir takeFolder("r13_stable_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap tap;
    tap.prepare(44100.0, 2, 512);

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = true;
    opts.audioMode = "input";
    opts.deviceRate = 44100.0;
    opts.deviceChannels = 2;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, tap, opts).ok);
    CHECK(host.status().rateChangedSinceArm == false);
    CHECK(host.status().lastError.empty());
    CHECK(fake.notices.empty());   // fail-first: RED on unfixed code (arm-time "beat clock unreliable (R13)")

    uint64_t delivered = 0;
    uint64_t hostTimeNs = 1'000'000'000ULL;
    pushCleanBlocks(tap, 44100.0, 512, 5, delivered, hostTimeNs);

    host.tick(makeSnap(), 0.0, delivered, tap, std::nullopt, 44100.0);
    host.tick(makeSnap(), 0.1, delivered, tap, std::nullopt, 44100.0);
    host.tick(makeSnap(), 0.2, delivered, tap, std::nullopt, 44100.0);

    const auto st = host.status();
    CHECK(st.rateChangedSinceArm == false);
    CHECK(st.lastError.empty());
    CHECK(fake.notices.empty());

    const auto stopRes = host.disarm(comp, tap);
    CHECK(stopRes.ok);

    LoadStats stats;
    auto loaded = Take::load(takeFolder.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->audio.segments.size() == 1);
    CHECK(loaded->audio.segments[0].rate == 44100.0);
}

// === 22: [host][r13] a mid-take rate change with NO tap (audio == false) still reports the hazard ===
//
// R13-C 3.7 case (ii): the audio tap's own self-stop path (tick()'s tapWasStarted_ branch) can only
// fire when a tap exists -- this covers the audio == false case that path cannot see: a device rate
// change mid-take is still a hazard for the beat clock's sample domain even with no audio captured.

TEST_CASE("RecorderHost r13 -- a mid-take device rate change (no audio) reports rateChangedSinceArm exactly once", "[host][r13]")
{
    TempDir storeRoot("r13_change_store");
    TempDir takeFolder("r13_change_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);

    AudioTap dummyTap;   // opts.audio = false below -- never started, never pushed to.

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = false;
    opts.deviceRate = 48000.0;
    opts.appVersion = "test";

    REQUIRE(host.arm(comp, dummyTap, opts).ok);
    CHECK(host.status().rateChangedSinceArm == false);

    host.tick(makeSnap(), 0.0, 0, dummyTap, std::nullopt, 48000.0);
    CHECK(host.status().rateChangedSinceArm == false);
    CHECK(host.status().lastError.empty());
    CHECK(fake.notices.empty());

    // Device rate changes mid-take (e.g. Bluetooth HFP flip) -- no tap to self-stop, but the hazard
    // is real: sample stamps before/after this tick are in different domains.
    host.tick(makeSnap(), 0.1, 0, dummyTap, std::nullopt, 16000.0);

    auto st = host.status();
    CHECK(st.rateChangedSinceArm == true);
    CHECK_FALSE(st.lastError.empty());
    REQUIRE(fake.notices.size() == 1);
    CHECK(fake.notices[0].find("48000") != std::string::npos);
    CHECK(fake.notices[0].find("16000") != std::string::npos);

    // Three further ticks at the SAME changed rate -- exactly ONE notify total (edge-triggered).
    host.tick(makeSnap(), 0.2, 0, dummyTap, std::nullopt, 16000.0);
    host.tick(makeSnap(), 0.3, 0, dummyTap, std::nullopt, 16000.0);
    host.tick(makeSnap(), 0.4, 0, dummyTap, std::nullopt, 16000.0);

    st = host.status();
    CHECK(st.rateChangedSinceArm == true);
    CHECK(fake.notices.size() == 1);

    host.disarm(comp, dummyTap);
}
