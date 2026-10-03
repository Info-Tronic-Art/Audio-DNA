// test_layer_runtime_race -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T0, ruling
// ruling-tsan.md amendments 2 / 3 / 18). Threaded stress of the Layer trigger tuple and the clip runtime fields, the
// message thread against the GL thread, run with REAL model code (Layer / Composition / Autopilot / LayerClock /
// CrossfadeStartDetector / TriggerClipCmd + UndoManager / the deck commands).
//   R1 [tsan] message-thread triggers / undo / clear / cancel vs the render clock + autopilot on one deck, plus a
//      third (httplib-like) reader thread and live Layer copies. Lane bf9b (plan-bf9b 4.B): the GL driver is
//      LayerClock::tick per shared layer + the show autopilot (DeckClock is deleted); expectation unchanged.
//   R2 [tsan] clip runtime fields: trigger writes vs the render transport write-back. The render side drives the
//      REAL syncMedia entry points (src/render/ClipTransportSync.h pushIntent / writeBack -- the CAS write-back)
//      against a fake player, as Renderer::syncMedia does with a VideoPlayer (Renderer.cpp is not linkable
//      headless). At the T0 commit R2 was a plain-syntax mirror of the pre-lane syncMedia (its RED is recorded in
//      the lane report); the fix round (ruling F5) switched it to the real entry points.
//   R4 [tsan] tuple consistency and no lost fade under a paced trigger storm (invariants I1-I4, also checked in
//      the normal build: a torn tuple or a lost trigger is a counted violation).
//   R-bf9b [tsan] (ruling-bf9b amendment 3(c)) fenced box and stack edits -- cells, columns, a row clear, Remove
//      Deck / undo, retire + reap, layer insert / move / erase -- vs the GL thread resolving refs into ANY live or
//      retired deck, behind an emulated FencedPtrSlot fence; unfenced triggers interleave.
// Registered with LABELS tsan + FAIL_REGULAR_EXPRESSION "WARNING: ThreadSanitizer" (tests/CMakeLists.txt): in a
// -DADNA_SANITIZE=thread build a single data race fails the case; in a normal build only the value checks bite.
// The tuple is only ever read through captureLayerRuntime (never a field). Since the fix round R2 needs
// ClipTransportSync.h, so the file no longer compiles against the pre-lane tree. Run under TSan:
// .harmony/probe-tsan-unit.sh.
#include <catch2/catch_test_macros.hpp>
#include "model/Composition.h"
#include "ShowFixture.h"
#include "model/Autopilot.h"
#include "analysis/FeatureSnapshot.h"
#include "core/UndoManager.h"
#include "core/DeckCommands.h"
#include "core/TriggerCommands.h"
#include "render/LayerClock.h"
#include "render/FencedPtrSlot.h"
#include "render/CrossfadeHistory.h"
#include "render/ClipTransportSync.h"
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
    c.mediaType = Clip::MediaType::Video;   // playable: Autopilot advances it
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

// R2's player (the concept ClipTransportSync drives, as VideoPlayer / ImageSequence): play state, playhead, seek; its
// clock moves while playing and stops at the end of the media (as the real ones).
struct FakePlayer
{
    bool playingState = false;
    double head = 0.0;
    bool isPlaying() const { return playingState; }
    void setPlaying(bool p) { playingState = p; }
    double getPlayheadPosition() const { return head; }
    void seekTo(double t) { head = t; }
    void advance(double step)
    {
        if (!playingState)
            return;
        head += step;
        if (head >= 1.0) { head = 1.0; playingState = false; }
    }
};

// The third reader (GET /api/composition / /api/state shape): the whole tuple plus plain-syntax reads of the clip
// runtime fields and the layer opacity, for every layer.
double readAll(Composition& comp)
{
    double sink = 0.0;
    for (auto& layer : comp.layers)
    {
        const LayerRuntimeSnapshot rt = captureLayerRuntime(layer);
        sink += rt.activeClipColumn + rt.previousClipColumn + rt.crossfadeProgress + rt.pendingTriggerColumn;
        sink += layer.opacity;
    }
    comp.forEachClip([&sink](const Clip& clip, const ClipSite&) {
        const bool playing = clip.playing;
        const double head = clip.playheadPosition;
        sink += (playing ? 1.0 : 0.0) + head;
    });
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
    Composition comp = ShowFixture::makeShow(1, 3, kColumns, false);   // 3 shared layers x kColumns, one deck
    const uint32_t deckId = comp.decks[0].id;
    for (int l = 0; l < comp.getNumLayers(); ++l)
    {
        for (int c = 0; c < kColumns; ++c)
            comp.decks[0].setClip(l, c, videoClip("L" + std::to_string(l) + "C" + std::to_string(c)));
        comp.layers[static_cast<size_t>(l)].transitionSpeed = 0.05f;   // fades on
    }
    comp.layers[2].autopilotEnabled = true;   // PlayNext at Beat1 (the clips' own action / duration)
    comp.layers[2].triggerClipImmediate(ClipRef{ deckId, 0 }, comp.rowClips(2));   // autopilot needs an active clip

    CompositionResolver resolver = [&comp]() -> Composition* { return &comp; };
    UndoManager mgr;

    std::atomic<bool> go{ false }, stop{ false };
    std::atomic<int> violations{ 0 };
    std::atomic<long> renderFrames{ 0 };
    double renderSink = 0.0, readerSink = 0.0;

    std::thread render([&] {
        while (!go.load()) std::this_thread::yield();
        Autopilot autopilot;
        FeatureSnapshot snap;
        std::vector<CrossfadeStartDetector> detectors(comp.layers.size());
        while (!stop.load())
        {
            snap.totalBeatCount++;
            snap.beatInBar = static_cast<uint8_t>(snap.totalBeatCount % 4);
            snap.barCount = static_cast<uint16_t>(snap.totalBeatCount / 4);
            autopilot.processFrame(comp, snap);
            for (size_t l = 0; l < comp.layers.size(); ++l)
            {
                Layer& L = comp.layers[l];
                LayerRuntimeSnapshot rt = captureLayerRuntime(L);
                LayerClock::tick(L, rt, 0.004f);
                if (!snapshotOk(rt, kColumns))
                    violations.fetch_add(1);
                if (const Clip* c = comp.clipAt(rt.activeRef(), static_cast<int>(l)))
                {
                    const bool playing = c->playing;
                    renderSink += c->inPoint + (playing ? 1.0 : 0.0);
                }
                if (const Clip* c = comp.clipAt(rt.previousRef(), static_cast<int>(l)))
                {
                    const bool playing = c->playing;   // the outgoing clip during a fade
                    renderSink += playing ? 1.0 : 0.0;
                }
                if (detectors[l].observe(rt.previousClipColumn, rt.activeClipColumn, rt.crossfadeProgress))
                    renderSink += 1.0;
            }
            renderFrames.fetch_add(1);
        }
    });

    std::thread reader([&] {
        while (!go.load()) std::this_thread::yield();
        while (!stop.load())
            readerSink += readAll(comp);
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
        const int li = (i / kColumns) % comp.getNumLayers();
        const int col = i % kColumns;
        Layer& L = comp.layers[static_cast<size_t>(li)];
        const ClipRef target{ deckId, col };

        const LayerRuntimeSnapshot before = captureLayerRuntime(L);
        std::optional<bool> playBefore, playAfter;
        if (const Clip* tc = comp.clipAt(target, li)) { const bool p = tc->playing; playBefore = p; }
        L.triggerClip(target, comp.rowClips(li), i % 5 == 0 ? Clip::BeatSnapMode::Beat : Clip::BeatSnapMode::Off);
        const LayerRuntimeSnapshot after = captureLayerRuntime(L);
        if (const Clip* tc = comp.clipAt(target, li)) { const bool p = tc->playing; playAfter = p; }
        mgr.perform(std::make_unique<TriggerClipCmd>(resolver, li, target, before, after,
                                                     playBefore, playAfter, "Trigger Clip"));
        if (i % 7 == 0) mgr.undo();
        if (i % 11 == 0) L.clearActiveClip(comp.rowClips(li));
        if (i % 13 == 0) cancelPendingInto(comp, deckId);
        if (i % 17 == 0) { Layer copy = L; REQUIRE(copy.id == L.id); }
    }
    waitForFrames(renderFrames, kMinRenderFrames);   // bounded; the `> 0` check below stays the vacuity guard
    stop.store(true);
    render.join();
    reader.join();

    INFO("render frames " << renderFrames.load() << ", sinks " << renderSink << " / " << readerSink);
    CHECK(renderFrames.load() > 0);
    CHECK(violations.load() == 0);
    for (auto& L : comp.layers)
        CHECK(snapshotOk(captureLayerRuntime(L), kColumns));
}

TEST_CASE("R2 clip runtime fields: trigger writes vs render transport write-back", "[tsan][layer_runtime]")
{
    Composition comp = ShowFixture::makeShow(1, 3, 2, false);
    const uint32_t deckId = comp.decks[0].id;
    comp.decks[0].setClip(0, 0, videoClip("a"));
    comp.decks[0].setClip(0, 1, videoClip("b"));
    comp.decks[0].getClip(0, 1)->loopMode = Clip::LoopMode::OneShot;
    for (int c = 0; c < 2; ++c)
        comp.decks[0].getClip(0, c)->outPoint = 0.9f;   // inside the clip: writeBack's out-point branch runs
    Layer& L = comp.layers[0];
    const RowClips rows = comp.rowClips(0);

    std::atomic<bool> go{ false }, stop{ false };
    std::atomic<long> renderFrames{ 0 };
    double readerSink = 0.0;

    // The GL thread's syncMedia shape on the REAL entry points: one tuple load, the active clip's player (one per
    // column, owned by the GL thread like Renderer's), ClipTransportSync::pushIntent (read the intent once, push it),
    // the player's advance, ClipTransportSync::writeBack (the playhead, the CAS write-back of `playing`, the out-point
    // on the local playhead: clip 0 loops back to its in-point, clip 1 is a OneShot); plus Autopilot's beatsPlayed
    // fetchAdd. The message thread's triggers / clears / hasBeenTriggered writes race all of it.
    std::thread render([&] {
        while (!go.load()) std::this_thread::yield();
        FakePlayer players[2];
        while (!stop.load())
        {
            const LayerRuntimeSnapshot rt = captureLayerRuntime(L);
            if (Clip* c = rows.at(rt.activeRef()))
            {
                FakePlayer& player = players[rt.activeClipColumn];
                const bool wanted = ClipTransportSync::pushIntent(*c, player);
                player.advance(0.01);
                ClipTransportSync::writeBack(*c, player, wanted);
                c->beatsPlayed.fetchAdd(1);
            }
            renderFrames.fetch_add(1);
        }
    });

    std::thread reader([&] {
        while (!go.load()) std::this_thread::yield();
        while (!stop.load())
            readerSink += readAll(comp);
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
        L.triggerClipImmediate(ClipRef{ deckId, i % 2 }, rows);
        if (i % 3 == 0) L.clearActiveClip(rows);
        if (Clip* c = comp.decks[0].getClip(0, i % 2)) c->hasBeenTriggered = true;
    }
    waitForFrames(renderFrames, kMinRenderFrames);   // bounded; the `> 0` check below stays the vacuity guard
    stop.store(true);
    render.join();
    reader.join();

    INFO("render frames " << renderFrames.load() << ", reader sink " << readerSink);
    CHECK(renderFrames.load() > 0);
    for (int c = 0; c < 2; ++c)
    {
        const double head = comp.decks[0].getClip(0, c)->playheadPosition;
        CHECK(head >= 0.0);
        CHECK(head <= 1.01);
    }
}

TEST_CASE("R4 tuple consistency and no lost fade under a paced trigger storm", "[tsan][layer_runtime]")
{
    // The Layer lives in the shared stack (heap, Composition::layers): with a Layer on the main thread's stack this
    // runtime (Apple clang 17 TSan) reported NO race on the base tree in 4 of 4 runs although the value checks saw
    // thousands of torn tuples, and with every Layer call inlined only the allocation stack names a src/ file
    // (measured, s-rta-1002 B1).
    Composition comp = ShowFixture::makeShow(1, 3, kColumns, false);
    const uint32_t deckId = comp.decks[0].id;
    Layer& L = comp.layers[0];
    for (int c = 0; c < kColumns; ++c)
        comp.decks[0].setClip(0, c, Clip{});
    const RowClips rows = comp.rowClips(0);
    // step = dt / speed = 0.3 (LayerClock::advanceCrossfade): a fade runs 0 -> .3 -> .6 -> .9 -> 1.0.
    constexpr float kDt = 0.003f;
    L.transitionSpeed = 0.01f;
    const float step = kDt / L.transitionSpeed;
    L.triggerClipImmediate(ClipRef{ deckId, 0 }, rows);

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
    for (int i = 1; i <= kTriggers && !stalled; ++i)
    {
        const long k = 1 + (i % 5);   // cycles 1..5 render observations between triggers
        // 60 s PER WAIT (fix round, ruling F8): a slow-but-live run is not "stalled"; one wait over 60 s is.
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
        while (renderObs.load() < last + k)
        {
            if (std::chrono::steady_clock::now() > deadline) { stalled = true; break; }
            std::this_thread::yield();
        }
        last = renderObs.load();
        L.triggerClipImmediate(ClipRef{ deckId, i % kColumns }, rows);   // cyclic: never a retrigger
    }
    stop.store(true);
    render.join();

    INFO("render observations " << renderObs.load() << "; I1 " << v1.load() << " I2 " << v2.load()
         << " I3 " << v3.load() << " I4 " << v4.load());
    REQUIRE_FALSE(stalled);   // "render stalled"
    REQUIRE(v1.load() + v2.load() + v3.load() + v4.load() == 0);
}

namespace
{
// R-bf9b: is `p` the address of a clip inside a live or retired deck's row storage?
bool insideDeckStorage(const Composition& comp, const Clip* p)
{
    auto in = [p](const Deck& d) {
        for (const auto& row : d.rows)
            for (const auto& cell : row.clips)
                if (cell.has_value() && &*cell == p)
                    return true;
        return false;
    };
    for (const auto& d : comp.decks)
        if (in(d)) return true;
    for (const auto& d : comp.retiredDecks())
        if (in(d)) return true;
    return false;
}
} // namespace

TEST_CASE("R-bf9b fenced box and stack edits vs the GL resolve of refs into any deck", "[tsan][layer_runtime]")
{
    // Ruling-bf9b amendment 3(c). 3 decks x 3 rows x 6 columns of video clips; deck 0 is shown. Layer 0 plays
    // (deck 2, col 1) with Cut transitions, layer 1 plays (deck 0, col 3) under autopilot, layer 2 crossfades (1000 s)
    // from (deck 1, col 0) to (deck 0, col 0). A FencedPtrSlot<Deck> stands in for the renderer's slot.
    constexpr int kCols = 6;
    Composition comp = ShowFixture::makeShow(3, 3, kCols, false);
    for (int d = 0; d < 3; ++d)
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < kCols; ++c)
                comp.decks[static_cast<size_t>(d)].setClip(
                    r, c, videoClip("D" + std::to_string(d) + "R" + std::to_string(r) + "C" + std::to_string(c)));
    const uint32_t d0 = comp.decks[0].id, d1 = comp.decks[1].id, d2 = comp.decks[2].id;
    comp.activeDeckIndex = 0;
    comp.layers[0].transitionSpeed = 0.0f;
    comp.layers[1].transitionSpeed = 0.0f;
    comp.layers[1].autopilotEnabled = true;   // PlayNext at Beat1 within deck 0 (the playing clip's source deck)
    comp.layers[2].transitionSpeed = 1000.0f;
    comp.layers[0].triggerClipImmediate(ClipRef{ d2, 1 }, comp.rowClips(0));
    comp.layers[1].triggerClipImmediate(ClipRef{ d0, 3 }, comp.rowClips(1));
    comp.layers[2].triggerClipImmediate(ClipRef{ d1, 0 }, comp.rowClips(2));
    comp.layers[2].triggerClipImmediate(ClipRef{ d0, 0 }, comp.rowClips(2));
    REQUIRE(comp.layers[2].runtime().previousRef() == (ClipRef{ d1, 0 }));
    REQUIRE(comp.layers[2].runtime().crossfadeProgress < 1.0f);

    FencedPtrSlot<Deck> slot;
    slot.set(comp.getActiveDeck());

    std::atomic<bool> go{ false }, stop{ false };
    std::atomic<long> frames{ 0 }, loops{ 0 }, held{ 0 }, resolved{ 0 }, outside{ 0 };
    double sink = 0.0;

    // GL driver: one view() per frame; fenced or null -> a held frame. Else per layer ONE runtime() load, active and
    // previous resolved through Composition::clipAt (live, then retired decks), inPoint / playing / effects read,
    // LayerClock::tick, then the show autopilot.
    std::thread render([&] {
        while (!go.load()) std::this_thread::yield();
        Autopilot autopilot;
        FeatureSnapshot snap;
        while (!stop.load())
        {
            const auto v = slot.view();
            if (v.fenced || v.ptr == nullptr)
            {
                held.fetch_add(1);
                loops.fetch_add(1);
                continue;
            }
            for (size_t l = 0; l < comp.layers.size(); ++l)
            {
                Layer& L = comp.layers[l];
                LayerRuntimeSnapshot rt = L.runtime();
                for (const ClipRef ref : { rt.activeRef(), rt.previousRef() })
                    if (const Clip* c = comp.clipAt(ref, static_cast<int>(l)))
                    {
                        resolved.fetch_add(1);
                        if (!insideDeckStorage(comp, c))
                            outside.fetch_add(1);
                        const bool playing = c->playing;
                        sink += c->inPoint + (playing ? 1.0 : 0.0) + static_cast<double>(c->effects.size());
                    }
                LayerClock::tick(L, rt, 0.004f);
            }
            snap.totalBeatCount++;
            snap.beatInBar = static_cast<uint8_t>(snap.totalBeatCount % 4);
            snap.barCount = static_cast<uint16_t>(snap.totalBeatCount / 4);
            autopilot.processFrame(comp, snap);
            frames.fetch_add(1);
            loops.fetch_add(1);
        }
    });

    // The emulated fence (UndoService::withDeckDetached's shape): detach + mark, drain until the GL thread has run
    // two loop passes since (the frame in flight is done), mutate, reap what no ref names, restore the shown deck.
    bool stalled = false;
    long reapedTotal = 0;
    DeckFenceHook fence = [&](const std::function<void()>& m) {
        slot.detachFenced();
        const long l0 = loops.load();
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
        while (loops.load() < l0 + 2)
        {
            if (std::chrono::steady_clock::now() > deadline) { stalled = true; break; }
            std::this_thread::yield();
        }
        if (m) m();
        reapedTotal += static_cast<long>(comp.reapRetiredDecks().size());
        slot.set(comp.getActiveDeck());
    };
    CompositionResolver resolver = [&comp]() -> Composition* { return &comp; };
    auto liveIndex = [&comp](uint32_t id) { return comp.findDeckIndexById(id); };

    go.store(true);
    if (!waitForFrames(frames, 1))
    {
        stop.store(true);
        render.join();
        FAIL("render thread never started");
    }

    constexpr int kIterations = 2000;
    long retires = 0, restores = 0, reapsAfterRefire = 0;
    std::optional<ClearLayerClipsCmd> rowClear;
    std::optional<RemoveDeckCmd> removal;
    for (int i = 0; i < kIterations && !stalled; ++i)
    {
        // Unfenced triggers (CAS, as the app): layer 2 restarts its deck 1 <-> deck 0 crossfade; layer 1 queues a
        // deck-0 clip on the beat now and then.
        comp.fire(2, liveIndex(i % 2 == 0 ? d1 : d0), 0, Clip::BeatSnapMode::Off, true);
        if (i % 3 == 0)
            comp.fire(1, liveIndex(d0), i % kCols, Clip::BeatSnapMode::Beat);

        switch (i % 12)
        {
            case 0:   // SetClip on the NON-shown deck's playing cell (deck 2, col 1) ...
            case 1:   // ... and its neighbour
                fence([&] {
                    if (const int di = liveIndex(d2); di >= 0)
                        comp.decks[static_cast<size_t>(di)].setClip(0, 1 + i % 12, videoClip("set" + std::to_string(i)));
                });
                break;
            case 2:   // addColumn on a non-shown deck
                fence([&] { comp.decks[static_cast<size_t>(liveIndex(d1))].addColumn(); });
                break;
            case 3:   // removeColumn (the last) on a non-shown deck
                fence([&] {
                    Deck& d = comp.decks[static_cast<size_t>(liveIndex(d1))];
                    d.removeColumn(d.numColumns - 1);
                });
                break;
            case 4:   // ClearLayerClips(deck 1, row 2): layer 2's OUTGOING clip is in that row
            {
                auto before = captureLayerClips(comp, liveIndex(d1), 2);
                auto after = clearedLayerClips(before, d1);
                rowClear.emplace(resolver, fence, nullptr, nullptr, liveIndex(d1), 2, std::move(before), std::move(after),
                                 "Clear Layer Clips");
                rowClear->execute();
                break;
            }
            case 5:   // ... and its undo
                if (rowClear) { rowClear->undo(); rowClear.reset(); }
                break;
            case 6:   // Remove Deck 2 while layer 0 plays from it: retired
            {
                comp.fire(0, liveIndex(d2), 1, Clip::BeatSnapMode::Off, true);
                const int di = liveIndex(d2);
                removal.emplace(resolver, fence, nullptr, nullptr, di, comp.decks[static_cast<size_t>(di)],
                                comp.activeDeckIndex.load(), "Remove Deck");
                removal->execute();
                if (removal->retired()) ++retires;
                break;
            }
            case 7:   // ... then its undo: restored (live playheads kept)
                if (removal)
                {
                    removal->undo();
                    if (liveIndex(d2) >= 0) ++restores;
                    removal.reset();
                }
                break;
            case 8:   // retire, re-fire layer 0 elsewhere, reap (any fenced edit), then bring the deck back as an undo
            {         // of a reaped removal would: the snapshot re-inserted under its id
                const int di = liveIndex(d2);
                comp.fire(0, di, 1, Clip::BeatSnapMode::Off, true);
                Deck snapshot = comp.decks[static_cast<size_t>(di)];
                RemoveDeckCmd remove(resolver, fence, nullptr, nullptr, di, snapshot, comp.activeDeckIndex.load(),
                                     "Remove Deck");
                remove.execute();
                const long reapedBefore = reapedTotal;
                comp.fire(0, liveIndex(d0), 4, Clip::BeatSnapMode::Off, true);   // layer 0 leaves deck 2 (a Cut)
                fence([] {});                                                    // any fenced edit reaps
                if (reapedTotal == reapedBefore + 1) ++reapsAfterRefire;
                remove.undo();                                                   // reaped: the snapshot comes back
                comp.fire(0, liveIndex(d2), 1, Clip::BeatSnapMode::Off, true);
                break;
            }
            case 9:   // insertLayer (a 4th shared layer: an empty row in every live and retired deck)
                fence([&] { comp.insertLayer(comp.getNumLayers(), comp.makeLayer()); });
                break;
            case 10:  // moveLayer: the new top layer to the bottom (every deck's rows in step)
                fence([&] { comp.moveLayer(comp.topLayerIndex(), 0); });
                break;
            case 11:  // ... back, then eraseLayer
                fence([&] {
                    comp.moveLayer(0, comp.topLayerIndex());
                    comp.eraseLayer(comp.topLayerIndex());
                });
                break;
        }
    }
    waitForFrames(frames, kMinRenderFrames);
    stop.store(true);
    render.join();

    INFO("frames " << frames.load() << ", held " << held.load() << ", resolved " << resolved.load() << ", outside "
         << outside.load() << ", retires " << retires << ", restores " << restores << ", reaps after a re-fire "
         << reapsAfterRefire << ", sink " << sink);
    REQUIRE_FALSE(stalled);
    CHECK(frames.load() > 100);
    CHECK(resolved.load() > 0);
    CHECK(outside.load() == 0);
    CHECK(retires == kIterations / 12 + (kIterations % 12 > 6 ? 1 : 0));
    CHECK(restores == retires);
    CHECK(reapsAfterRefire == kIterations / 12 + (kIterations % 12 > 8 ? 1 : 0));
    for (const auto& d : comp.decks)
        CHECK(d.getNumRows() == comp.getNumLayers());
    CHECK(comp.getNumRetiredDecks() == 0);
}
