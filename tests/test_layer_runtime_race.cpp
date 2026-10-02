// test_layer_runtime_race -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T0, ruling
// ruling-tsan.md amendments 2 / 3 / 18). Threaded stress of the Layer trigger tuple and the clip runtime fields, the
// message thread against the GL thread, run with REAL model code (Layer / Deck / Autopilot / LayerClock / DeckClock /
// CrossfadeStartDetector / TriggerClipCmd + UndoManager).
//   R1 [tsan] message-thread triggers / undo / clear / cancel vs the render clock + autopilot on one deck, plus a
//      third (httplib-like) reader thread and live Layer copies.
//   R2 [tsan] clip runtime fields: trigger writes vs the render transport write-back. The render side is a MIRROR
//      of Renderer::syncMedia (Renderer.cpp is not linkable headless), written in plain assignment syntax on
//      purpose: with Relaxed<> fields the same source becomes atomic operations.
//   R4 [tsan] tuple consistency and no lost fade under a paced trigger storm (invariants I1-I4, also checked in
//      the normal build: a torn tuple or a lost trigger is a counted violation).
// Registered with LABELS tsan + FAIL_REGULAR_EXPRESSION "WARNING: ThreadSanitizer" (tests/CMakeLists.txt): in a
// -DADNA_SANITIZE=thread build a single data race fails the case; in a normal build only the value checks bite.
// The tuple is only ever read through captureLayerRuntime (never a field), so this file compiles against both the
// pre-lane five plain fields and the lane's one-word cell. Run under TSan: .harmony/probe-tsan-unit.sh.
#include <catch2/catch_test_macros.hpp>
#include "model/Deck.h"
#include "model/Autopilot.h"
#include "analysis/FeatureSnapshot.h"
#include "core/UndoManager.h"
#include "core/DeckCommands.h"
#include "core/TriggerCommands.h"
#include "render/LayerClock.h"
#include "render/DeckClock.h"
#include "render/CrossfadeHistory.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

namespace
{
constexpr int kColumns = 4;

Clip videoClip(const std::string& name)
{
    Clip c;
    c.name = name;
    c.mediaType = Clip::MediaType::Video;   // playable: DeckClock ticks it, Autopilot advances it
    c.autopilotAction = Clip::AutopilotAction::PlayNext;
    c.autopilotDuration = Clip::AutopilotDuration::Beat1;   // the shortest duration
    return c;
}

// Value checks on one render-side snapshot: every column in [-1, numColumns), progress in [0, 1], and I1 (an
// active clip is never its own previous).
bool snapshotOk(const LayerRuntimeSnapshot& rt, int numColumns)
{
    auto colOk = [numColumns](int c) { return c >= -1 && c < numColumns; };
    if (!colOk(rt.activeClipColumn) || !colOk(rt.previousClipColumn) || !colOk(rt.pendingTriggerColumn))
        return false;
    if (!(rt.crossfadeProgress >= 0.0f && rt.crossfadeProgress <= 1.0f))
        return false;
    if (rt.activeClipColumn >= 0 && rt.previousClipColumn == rt.activeClipColumn)
        return false;   // I1
    return true;
}

// The third reader (GET /api/composition / /api/state shape): the whole tuple plus plain-syntax reads of the clip
// runtime fields and the layer opacity, for every layer.
double readAll(Deck& deck)
{
    double sink = 0.0;
    for (auto& layer : deck.layers)
    {
        const LayerRuntimeSnapshot rt = captureLayerRuntime(layer);
        sink += rt.activeClipColumn + rt.previousClipColumn + rt.crossfadeProgress + rt.pendingTriggerColumn;
        sink += layer.opacity;
        for (int c = 0; c < static_cast<int>(layer.clips.size()); ++c)
            if (const Clip* clip = layer.getClipAt(c))
            {
                const bool playing = clip->playing;
                const double head = clip->playheadPosition;
                sink += (playing ? 1.0 : 0.0) + head;
            }
    }
    return sink;
}

// Start / finish handshake (s-rta-1002 fix round, ruling F1): the render thread publishes "running" by counting its
// first frame. Without it, a loaded machine may not schedule the render thread inside the main loop's ~10 ms (a
// normal build), and the case false-fails `render frames 0` -- or passes having overlapped 1-2 frames. Bounded: a
// render thread that never runs is a FAIL, never a hang.
constexpr long kMinRenderFrames = 100;
bool waitForFrames(const std::atomic<long>& frames, long atLeast,
                   std::chrono::seconds limit = std::chrono::seconds(10))
{
    const auto deadline = std::chrono::steady_clock::now() + limit;
    while (frames.load() < atLeast)
    {
        if (std::chrono::steady_clock::now() > deadline)
            return false;
        std::this_thread::yield();
    }
    return true;
}
} // namespace

TEST_CASE("R1 message-thread triggers vs render clock / autopilot on one deck", "[tsan][layer_runtime]")
{
    Deck deck;
    deck.numColumns = kColumns;
    deck.initDefault();   // 3 layers x kColumns
    for (int l = 0; l < deck.getNumLayers(); ++l)
    {
        for (int c = 0; c < kColumns; ++c)
            deck.setClip(l, c, videoClip("L" + std::to_string(l) + "C" + std::to_string(c)));
        deck.layers[static_cast<size_t>(l)].transitionSpeed = 0.05f;   // fades on
    }
    deck.layers[2].autopilotEnabled = true;   // PlayNext at Beat1 (the clips' own action / duration)
    deck.layers[2].triggerClipImmediate(0);   // autopilot needs an active playing clip

    ClipLayerResolver resolver = [&deck](int d, int l) -> Layer* { return d == 0 ? deck.getLayer(l) : nullptr; };
    UndoManager mgr;

    std::atomic<bool> go{ false }, stop{ false };
    std::atomic<int> violations{ 0 };
    std::atomic<long> renderFrames{ 0 };
    double renderSink = 0.0, readerSink = 0.0;

    std::thread render([&] {
        while (!go.load()) std::this_thread::yield();
        Autopilot autopilot;
        FeatureSnapshot snap;
        std::vector<CrossfadeStartDetector> detectors(deck.layers.size());
        while (!stop.load())
        {
            snap.totalBeatCount++;
            snap.beatInBar = static_cast<uint8_t>(snap.totalBeatCount % 4);
            snap.barCount = static_cast<uint16_t>(snap.totalBeatCount / 4);
            autopilot.processFrame(deck, snap);
            for (size_t l = 0; l < deck.layers.size(); ++l)
            {
                Layer& L = deck.layers[l];
                LayerClock::advanceCrossfade(L, 0.004f);
                const LayerRuntimeSnapshot rt = captureLayerRuntime(L);
                if (!snapshotOk(rt, kColumns))
                    violations.fetch_add(1);
                if (const Clip* c = L.getActiveClip())
                    renderSink += c->inPoint;
                if (detectors[l].observe(rt.previousClipColumn, rt.activeClipColumn, rt.crossfadeProgress))
                    renderSink += 1.0;
            }
            DeckClock::tick(deck, 0.004f, [&renderSink](const Clip* c, float) {
                const bool playing = c->playing;
                renderSink += playing ? 1.0 : 0.0;
            });
            renderFrames.fetch_add(1);
        }
    });

    std::thread reader([&] {
        while (!go.load()) std::this_thread::yield();
        while (!stop.load())
            readerSink += readAll(deck);
    });

    go.store(true);
    if (!waitForFrames(renderFrames, 1))
    {
        stop.store(true);
        render.join();
        reader.join();
        FAIL("render thread never started");
    }
    constexpr int kIterations = 20000;
    for (int i = 0; i < kIterations; ++i)
    {
        const int li = (i / kColumns) % deck.getNumLayers();
        const int col = i % kColumns;
        Layer& L = deck.layers[static_cast<size_t>(li)];

        const LayerRuntimeSnapshot before = captureLayerRuntime(L);
        std::optional<bool> playBefore, playAfter;
        if (const Clip* tc = L.getClipAt(col)) { const bool p = tc->playing; playBefore = p; }
        L.triggerClip(col, i % 5 == 0 ? Clip::BeatSnapMode::Beat : Clip::BeatSnapMode::Off);
        const LayerRuntimeSnapshot after = captureLayerRuntime(L);
        if (const Clip* tc = L.getClipAt(col)) { const bool p = tc->playing; playAfter = p; }
        mgr.perform(std::make_unique<TriggerClipCmd>(resolver, 0, li, col, before, after,
                                                     playBefore, playAfter, "Trigger Clip"));
        if (i % 7 == 0) mgr.undo();
        if (i % 11 == 0) L.clearActiveClip();
        if (i % 13 == 0) cancelPendingTriggers(deck);
        if (i % 17 == 0) { Layer copy = L; REQUIRE(copy.clips.size() == L.clips.size()); }
    }
    waitForFrames(renderFrames, kMinRenderFrames);   // bounded; the `> 0` check below stays the vacuity guard
    stop.store(true);
    render.join();
    reader.join();

    INFO("render frames " << renderFrames.load() << ", sinks " << renderSink << " / " << readerSink);
    CHECK(renderFrames.load() > 0);
    CHECK(violations.load() == 0);
    for (auto& L : deck.layers)
        CHECK(snapshotOk(captureLayerRuntime(L), kColumns));
}

TEST_CASE("R2 clip runtime fields: trigger writes vs render transport write-back", "[tsan][layer_runtime]")
{
    Deck deck;
    deck.numColumns = 2;
    deck.initDefault();
    deck.setClip(0, 0, videoClip("a"));
    deck.setClip(0, 1, videoClip("b"));
    deck.layers[0].getClipAt(1)->loopMode = Clip::LoopMode::OneShot;
    Layer& L = deck.layers[0];

    std::atomic<bool> go{ false }, stop{ false };
    std::atomic<long> renderFrames{ 0 };
    double readerSink = 0.0;

    // MIRROR of Renderer::syncMedia's video branch (Renderer.cpp, "Sync transport state from clip" .. OneShot stop)
    // against a fake player: read `playing` (twice, as today), write the player clock into playheadPosition, write
    // `playing` back from the player, test the out-point, OneShot stop; plus Autopilot's beatsPlayed increment.
    std::thread render([&] {
        while (!go.load()) std::this_thread::yield();
        bool playerPlaying = false;
        double playerClock = 0.0;
        while (!stop.load())
        {
            if (Clip* c = L.getActiveClip())
            {
                if (c->playing && !playerPlaying)
                    playerPlaying = true;
                else if (!c->playing)
                    playerPlaying = false;
                if (playerPlaying)
                    playerClock = playerClock >= 1.0 ? 0.0 : playerClock + 0.01;
                c->playheadPosition = playerClock;
                c->playing = playerPlaying;
                if (c->playheadPosition >= static_cast<double>(c->outPoint) - 0.001)
                {
                    if (c->loopMode == Clip::LoopMode::OneShot)
                    {
                        playerPlaying = false;
                        c->playing = false;
                    }
                    playerClock = 0.0;
                }
                c->beatsPlayed = c->beatsPlayed + 1;
            }
            renderFrames.fetch_add(1);
        }
    });

    std::thread reader([&] {
        while (!go.load()) std::this_thread::yield();
        while (!stop.load())
            readerSink += readAll(deck);
    });

    go.store(true);
    if (!waitForFrames(renderFrames, 1))
    {
        stop.store(true);
        render.join();
        reader.join();
        FAIL("render thread never started");
    }
    constexpr int kIterations = 40000;
    for (int i = 0; i < kIterations; ++i)
    {
        L.triggerClipImmediate(i % 2);
        if (i % 3 == 0) L.clearActiveClip();
        if (Clip* c = L.getClipAt(i % 2)) c->hasBeenTriggered = true;
    }
    waitForFrames(renderFrames, kMinRenderFrames);   // bounded; the `> 0` check below stays the vacuity guard
    stop.store(true);
    render.join();
    reader.join();

    INFO("render frames " << renderFrames.load() << ", reader sink " << readerSink);
    CHECK(renderFrames.load() > 0);
    for (int c = 0; c < 2; ++c)
    {
        const double head = L.getClipAt(c)->playheadPosition;
        CHECK(head >= 0.0);
        CHECK(head <= 1.01);
    }
}

TEST_CASE("R4 tuple consistency and no lost fade under a paced trigger storm", "[tsan][layer_runtime]")
{
    // The Layer lives in a Deck (heap, allocated by Deck::initDefault): with a Layer on the main thread's stack this
    // runtime (Apple clang 17 TSan) reported NO race on the base tree in 4 of 4 runs although the value checks saw
    // thousands of torn tuples, and with every Layer call inlined only the allocation stack names a src/ file
    // (measured, s-rta-1002 B1).
    Deck deck;
    deck.numColumns = kColumns;
    deck.initDefault();
    Layer& L = deck.layers[0];
    for (int c = 0; c < kColumns; ++c)
        L.clips[static_cast<size_t>(c)] = Clip{};
    // step = dt / speed = 0.3 (LayerClock::advanceCrossfade): a fade runs 0 -> .3 -> .6 -> .9 -> 1.0.
    constexpr float kDt = 0.003f;
    L.transitionSpeed = 0.01f;
    const float step = kDt / L.transitionSpeed;
    L.triggerClipImmediate(0);

    std::atomic<bool> go{ false }, stop{ false };
    std::atomic<long> renderObs{ 0 };
    std::atomic<int> v1{ 0 }, v2{ 0 }, v3{ 0 }, v4{ 0 };

    std::thread render([&] {
        while (!go.load()) std::this_thread::yield();
        int lastActive = captureLayerRuntime(L).activeClipColumn;
        while (!stop.load())
        {
            const LayerRuntimeSnapshot obs = captureLayerRuntime(L);
            if (obs.activeClipColumn >= 0 && obs.previousClipColumn == obs.activeClipColumn)
                v1.fetch_add(1);   // I1
            if (obs.previousClipColumn != -1 && obs.previousClipColumn != (obs.activeClipColumn + 3) % kColumns)
                v2.fetch_add(1);   // I2
            if (obs.activeClipColumn != lastActive && obs.crossfadeProgress > step + 1e-6f)
                v3.fetch_add(1);   // I3
            if (!(obs.crossfadeProgress >= 0.0f && obs.crossfadeProgress <= 1.0f))
                v4.fetch_add(1);   // I4
            lastActive = obs.activeClipColumn;
            renderObs.fetch_add(1);
            LayerClock::advanceCrossfade(L, kDt);
        }
    });

    go.store(true);
    constexpr int kTriggers = 20000;
    bool stalled = false;
    long last = renderObs.load();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
    for (int i = 1; i <= kTriggers && !stalled; ++i)
    {
        const long k = 1 + (i % 5);   // cycles 1..5 render observations between triggers
        while (renderObs.load() < last + k)
        {
            if (std::chrono::steady_clock::now() > deadline) { stalled = true; break; }
            std::this_thread::yield();
        }
        last = renderObs.load();
        L.triggerClipImmediate(i % kColumns);   // cyclic: never a retrigger
    }
    stop.store(true);
    render.join();

    INFO("render observations " << renderObs.load() << "; I1 " << v1.load() << " I2 " << v2.load()
         << " I3 " << v3.load() << " I4 " << v4.load());
    REQUIRE_FALSE(stalled);   // "render stalled"
    REQUIRE(v1.load() + v2.load() + v3.load() + v4.load() == 0);
}
