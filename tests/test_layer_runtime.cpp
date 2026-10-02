// test_layer_runtime -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T2, ruling
// ruling-tsan.md amendments 4 / 5 / 10). The Layer trigger tuple as ONE atomic word (LayerRuntimeCell, src/model/
// Layer.h): pack / unpack, the exact transition pair every trigger returns, the compare-exchange that keeps a stale
// GL-thread write from erasing a trigger, a cancelled queued trigger never firing, value copies, the activation-tail
// ordering contract, and the momentary release. GREEN-only (normal build); the teeth are the named mutants of
// amendment 16 (casRuntime as a plain store; releaseMomentary without its pending branch; the activation tail after
// the CAS).
#include <catch2/catch_test_macros.hpp>
#include "model/Deck.h"
#include "core/DeckCommands.h"
#include <atomic>
#include <chrono>
#include <climits>
#include <thread>
#include <type_traits>
#include <utility>

static_assert(std::is_nothrow_move_constructible_v<Layer>);   // holds on the base 02b2913 too (checked)

namespace
{
// A layer at rest on column 0 (no fade), clips in columns 0..n-1.
Layer restingLayer(int columns = 4, float transitionSpeed = 0.5f)
{
    Layer L;
    L.ensureColumns(columns);
    for (int c = 0; c < columns; ++c)
    {
        Clip clip;
        clip.name = "c" + std::to_string(c);
        clip.inPoint = 0.25f;
        L.clips[static_cast<size_t>(c)] = clip;
    }
    L.transitionSpeed = transitionSpeed;
    L.setRuntime({ 0, -1, 1.0f, -1, Clip::BeatSnapMode::Off });
    return L;
}
} // namespace

TEST_CASE("LayerRuntimeCell: pack / unpack round trip over the whole range", "[layer_runtime]")
{
    const int cols[] = { -1, 0, 9999, 10000, INT_MAX };
    const int pendings[] = { -1, 0, 9999, LayerRuntimeCell::kMaxPendingColumn };
    const Clip::BeatSnapMode snaps[] = { Clip::BeatSnapMode::Off, Clip::BeatSnapMode::Beat, Clip::BeatSnapMode::Bar,
                                         Clip::BeatSnapMode::TwoBar, Clip::BeatSnapMode::FourBar };
    REQUIRE(LayerRuntimeCell::kMaxPendingColumn == 268435454);
    Layer L;
    int checked = 0;
    for (int a : cols)
        for (int p : cols)
            for (int q : pendings)
                for (auto s : snaps)
                {
                    const LayerRuntimeSnapshot r{ a, p, 0.375f, q, s };
                    REQUIRE(LayerRuntimeCell::unpack(LayerRuntimeCell::pack(r)) == r);
                    L.setRuntime(r);
                    REQUIRE(L.runtime() == r);
                    ++checked;
                }
    CHECK(checked == 5 * 5 * 4 * 5);
}

TEST_CASE("Layer triggers return the exact transition pair", "[layer_runtime]")
{
    SECTION("queue: a beat-snapped clip that is not active is queued, the active clip untouched")
    {
        Layer L = restingLayer();
        L.clips[2]->beatSnapMode = Clip::BeatSnapMode::Bar;
        const auto before = L.runtime();
        const auto t = L.triggerClip(2);
        CHECK(t.applied);
        CHECK(t.before == before);
        CHECK(t.after == LayerRuntimeSnapshot{ 0, -1, 1.0f, 2, Clip::BeatSnapMode::Off });
        CHECK(L.runtime() == t.after);
        CHECK(L.clips[2]->playing == false);   // queued: no activation tail
    }
    SECTION("immediate new: previous = old active, progress 0 (fade), auto-play on first activation, in-point")
    {
        Layer L = restingLayer();
        L.clips[1]->playheadPosition = 0.9;
        L.clips[1]->beatsPlayed = 7;
        const auto t = L.triggerClip(1);
        CHECK(t.before == LayerRuntimeSnapshot{ 0, -1, 1.0f, -1, Clip::BeatSnapMode::Off });
        CHECK(t.after == LayerRuntimeSnapshot{ 1, 0, 0.0f, -1, Clip::BeatSnapMode::Off });
        CHECK(L.runtime() == t.after);
        CHECK(L.clips[1]->playing == true);
        CHECK(L.clips[1]->playheadPosition == 0.25);
        CHECK(L.clips[1]->beatsPlayed == 0);
    }
    SECTION("immediate new with no fade (transitionSpeed 0): a cut")
    {
        Layer L = restingLayer(4, 0.0f);
        const auto t = L.triggerClipImmediate(3);
        CHECK(t.after == LayerRuntimeSnapshot{ 3, 0, 1.0f, -1, Clip::BeatSnapMode::Off });
    }
    SECTION("retrigger: the tuple is unchanged, the clip restarts from its in-point and keeps its play state")
    {
        Layer L = restingLayer();
        L.clips[0]->playing = false;
        L.clips[0]->playheadPosition = 0.8;
        L.clips[0]->beatsPlayed = 5;
        const auto t = L.triggerClip(0);
        CHECK(t.applied);
        CHECK_FALSE(t.changed());
        CHECK(t.after == LayerRuntimeSnapshot{ 0, -1, 1.0f, -1, Clip::BeatSnapMode::Off });
        CHECK(L.clips[0]->playheadPosition == 0.25);
        CHECK(L.clips[0]->beatsPlayed == 0);
        CHECK(L.clips[0]->playing == false);
    }
    SECTION("clear: previous = old active, active -1, progress 1, the queue cancelled, the old clip stops")
    {
        Layer L = restingLayer();
        L.clips[0]->playing = true;
        L.setRuntime({ 0, -1, 1.0f, 2, Clip::BeatSnapMode::Bar });
        const auto t = L.clearActiveClip();
        CHECK(t.before == LayerRuntimeSnapshot{ 0, -1, 1.0f, 2, Clip::BeatSnapMode::Bar });
        CHECK(t.after == LayerRuntimeSnapshot{ -1, 0, 1.0f, -1, Clip::BeatSnapMode::Off });
        CHECK(L.runtime() == t.after);
        CHECK(L.clips[0]->playing == false);
    }
    SECTION("clear-if-mismatch: clearActiveClip(onlyIfActive) on another column is a no-op")
    {
        Layer L = restingLayer();
        L.clips[0]->playing = true;
        const auto before = L.runtime();
        const auto t = L.clearActiveClip(2);
        CHECK(t.applied);
        CHECK_FALSE(t.changed());
        CHECK(t.before == before);
        CHECK(L.runtime() == before);
        CHECK(L.clips[0]->playing == true);
        const auto hit = L.clearActiveClip(0);   // the matching column clears
        CHECK(hit.after.activeClipColumn == -1);
    }
}

TEST_CASE("a GL-style CAS with a stale expected fails after a trigger and leaves the trigger's tuple", "[layer_runtime]")
{
    Layer L = restingLayer();
    L.triggerClip(1);                            // a fade 0 -> 1 is running
    LayerRuntimeSnapshot stale = L.runtime();    // the GL thread loads the tuple ...
    const auto trig = L.triggerClip(2);          // ... the message thread triggers column 2 ...
    LayerRuntimeSnapshot ticked = stale;         // ... and the GL thread publishes its fade tick
    ticked.crossfadeProgress = 0.4f;
    CHECK_FALSE(L.casRuntime(stale, ticked));
    CHECK(stale == trig.after);                  // failure hands back the trigger's tuple (adopt)
    CHECK(L.runtime() == trig.after);
    CHECK(L.runtime() == LayerRuntimeSnapshot{ 2, 1, 0.0f, -1, Clip::BeatSnapMode::Off });
}

TEST_CASE("a queued trigger cancelled by cancelPendingTriggers never fires", "[layer_runtime][quantize]")
{
    Deck deck;
    deck.numColumns = 4;
    deck.initDefault();
    for (int c = 0; c < 4; ++c)
        deck.setClip(0, c, Clip{});
    Layer& L = deck.layers[0];
    L.setRuntime({ 0, -1, 1.0f, -1, Clip::BeatSnapMode::Off });
    L.triggerClip(3, Clip::BeatSnapMode::Beat);
    REQUIRE(L.runtime().pendingTriggerColumn == 3);

    const auto cancelled = cancelPendingTriggers(deck);
    REQUIRE(cancelled.size() == 1);
    CHECK(cancelled[0].pendingTriggerColumn == 3);
    CHECK(cancelled[0].pendingTriggerSnapOverride == Clip::BeatSnapMode::Beat);

    const auto before = L.runtime();
    const auto t = L.processPendingTrigger(0, 0);
    CHECK_FALSE(t.changed());
    CHECK(L.runtime() == before);
    CHECK(L.runtime().activeClipColumn == 0);
}

TEST_CASE("Layer copy and move keep the tuple", "[layer_runtime]")
{
    Layer L = restingLayer();
    L.setRuntime({ 2, 1, 0.5f, 3, Clip::BeatSnapMode::TwoBar });
    const Layer copy = L;
    CHECK(copy.runtime() == L.runtime());
    Layer moved = std::move(L);
    CHECK(moved.runtime() == copy.runtime());
    Layer assigned;
    assigned = copy;
    CHECK(assigned.runtime() == copy.runtime());
    Layer moveAssigned;
    moveAssigned = std::move(assigned);
    CHECK(moveAssigned.runtime() == copy.runtime());
}

TEST_CASE("updateRuntime: a bounded update gives up after exactly maxAttempts and keeps the concurrent tuple",
          "[layer_runtime]")
{
    Layer L = restingLayer();
    int calls = 0;
    const auto t = L.updateRuntime([&](LayerRuntimeSnapshot r) {
        ++calls;
        L.setRuntime({ 100 + calls, -1, 1.0f, -1, Clip::BeatSnapMode::Off });   // a concurrent writer wins each time
        r.crossfadeProgress = 0.5f;
        return r;
    }, 16);
    CHECK_FALSE(t.applied);
    CHECK(calls == 16);
    CHECK(L.runtime().activeClipColumn == 116);   // the concurrent value is intact
    CHECK(L.runtime().crossfadeProgress == 1.0f);
    CHECK(t.before == t.after);
}

TEST_CASE("activation tail before the CAS: the GL thread never sees a fresh trigger's stale beatsPlayed",
          "[layer_runtime]")
{
    // The message thread parks beatsPlayed = 1000 on the INACTIVE clip it is about to trigger; the GL thread asserts
    // beatsPlayed != 1000 on its first observation of that clip active. The reset is written before the CAS that
    // installs the activation, so an acquire load naming the clip happens-after it (deterministic). A post-CAS tail
    // (the mutant) fails it only when the GL load lands between the CAS and the reset: rare, hence also a review item.
    Deck deck;
    deck.numColumns = 2;
    deck.initDefault();
    deck.setClip(0, 0, Clip{});
    deck.setClip(0, 1, Clip{});
    Layer& L = deck.layers[0];
    L.transitionSpeed = 0.0f;
    L.setRuntime({ 0, -1, 1.0f, -1, Clip::BeatSnapMode::Off });

    std::atomic<bool> stop{ false }, ready{ false };
    std::atomic<int> seenActivations{ 0 }, stale{ 0 };
    std::thread render([&] {
        int last = L.runtime().activeClipColumn;
        ready.store(true);   // the baseline is taken before the first trigger
        while (!stop.load())
        {
            const auto rt = L.runtime();
            if (rt.activeClipColumn != last)
            {
                if (const Clip* c = L.getClipAt(rt.activeClipColumn); c != nullptr && c->beatsPlayed == 1000)
                    stale.fetch_add(1);
                last = rt.activeClipColumn;
                seenActivations.fetch_add(1);
            }
        }
    });

    while (!ready.load())
        std::this_thread::yield();
    constexpr int kRounds = 20000;
    bool stalled = false;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    for (int i = 1; i <= kRounds && !stalled; ++i)
    {
        const int col = i % 2;
        L.getClipAt(col)->beatsPlayed = 1000;   // col is inactive here
        L.triggerClipImmediate(col);
        while (seenActivations.load() < i)
            if (std::chrono::steady_clock::now() > deadline) { stalled = true; break; }
    }
    stop.store(true);
    render.join();
    INFO("activations seen " << seenActivations.load() << ", stale beatsPlayed seen " << stale.load());
    REQUIRE_FALSE(stalled);
    CHECK(stale.load() == 0);
}

TEST_CASE("releaseMomentary: a release before the beat cancels the queued trigger, the clip does not latch",
          "[layer_runtime][momentary][quantize]")
{
    Layer L = restingLayer();
    L.clips[1]->beatSnapMode = Clip::BeatSnapMode::Beat;
    L.triggerClip(1);   // pressed: queued for the beat
    REQUIRE(L.runtime().pendingTriggerColumn == 1);

    const auto t = L.releaseMomentary(1);   // released before the beat
    CHECK(t.after == LayerRuntimeSnapshot{ 0, -1, 1.0f, -1, Clip::BeatSnapMode::Off });

    L.processPendingTrigger(0, 0);   // the beat arrives
    CHECK(L.runtime().activeClipColumn == 0);   // column 1 never latched on
    CHECK(L.clips[1]->playing == false);
}

TEST_CASE("releaseMomentary: press A, press B, release A leaves B queued", "[layer_runtime][momentary][quantize]")
{
    Layer L = restingLayer();
    L.clips[1]->beatSnapMode = Clip::BeatSnapMode::Beat;
    L.clips[2]->beatSnapMode = Clip::BeatSnapMode::Beat;
    L.triggerClip(1);   // press A: queued
    L.triggerClip(2);   // press B: replaces the one pending slot
    REQUIRE(L.runtime().pendingTriggerColumn == 2);

    const auto t = L.releaseMomentary(1);   // release A
    CHECK_FALSE(t.changed());
    CHECK(L.runtime().pendingTriggerColumn == 2);
    L.processPendingTrigger(0, 0);
    CHECK(L.runtime().activeClipColumn == 2);
}

TEST_CASE("releaseMomentary: a release after the trigger fired clears, as before", "[layer_runtime][momentary]")
{
    Layer L = restingLayer();
    L.clips[1]->beatSnapMode = Clip::BeatSnapMode::Beat;
    L.triggerClip(1);
    L.processPendingTrigger(0, 0);   // fired
    REQUIRE(L.runtime().activeClipColumn == 1);
    REQUIRE(L.clips[1]->playing == true);

    const auto t = L.releaseMomentary(1);
    CHECK(t.after.activeClipColumn == -1);
    CHECK(t.after.previousClipColumn == 1);
    CHECK(L.clips[1]->playing == false);
}
