// test_layer_runtime -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T2, ruling
// ruling-tsan.md amendments 4 / 5 / 10). The Layer trigger tuple as ONE atomic word (LayerRuntimeCell, src/model/
// Layer.h): pack / unpack, the exact transition pair every trigger returns, the compare-exchange that keeps a stale
// GL-thread write from erasing a trigger, a cancelled queued trigger never firing, value copies, the activation-tail
// ordering contract, and the momentary release; T3 (B2): the GL thread's fade tick (LayerClock::tick) adopting a
// concurrent trigger, and the amendment-4 contention test. GREEN-only (normal build); the teeth are the named
// mutants of amendment 16 (casRuntime as a plain store; tick as a plain store; releaseMomentary without its pending
// branch; the activation tail after the CAS). Fix round (ruling F2): a GL-thread trigger decided from a snapshot (the
// autopilot advance) is a no-op once the tuple no longer names the column it decided from -- the deterministic
// cases plus a bounded stress through the real Autopilot::processFrame (mutant: the guard removed).
#include <catch2/catch_test_macros.hpp>
#include "model/Autopilot.h"
#include "model/Deck.h"
#include "core/DeckCommands.h"
#include "render/LayerClock.h"
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
        if (calls > 1000)
            return r;   // a call cap (fix-round NIT): an ignored bound ends here and fails `calls == 16`, never hangs
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

TEST_CASE("LayerClock::tick on a stale tuple after a concurrent trigger adopts it and keeps the new fade's previous",
          "[layer_runtime][layer_clock]")
{
    // The GL thread loaded the tuple near the END of a fade (its tick would clear previous) ...
    Layer L = restingLayer();
    L.setRuntime({ 1, 0, 0.95f, -1, Clip::BeatSnapMode::Off });
    LayerRuntimeSnapshot rt = L.runtime();
    // ... the message thread triggers column 2 (a NEW fade from column 1) ...
    const auto trig = L.triggerClip(2);
    REQUIRE(trig.after == LayerRuntimeSnapshot{ 2, 1, 0.0f, -1, Clip::BeatSnapMode::Off });
    // ... and the GL thread publishes its tick from the stale tuple: it must lose, and adopt the trigger's tuple.
    CHECK(LayerClock::advanced(rt, L.transitionSpeed, 0.1f) == LayerRuntimeSnapshot{ 1, -1, 1.0f, -1,
                                                                                      Clip::BeatSnapMode::Off });
    CHECK_FALSE(LayerClock::tick(L, rt, 0.1f));
    CHECK(rt == trig.after);                          // the caller's tuple is the trigger's (adopt), un-advanced
    CHECK(L.runtime() == trig.after);                 // the trigger stands
    CHECK(L.runtime().previousClipColumn == 1);       // the new fade still fades FROM column 1
    // The next frame's tick advances the adopted fade normally.
    CHECK(LayerClock::tick(L, rt, 0.1f));
    CHECK(rt == LayerRuntimeSnapshot{ 2, 1, 0.2f, -1, Clip::BeatSnapMode::Off });
    CHECK(L.runtime() == rt);
    // No fade: nothing to publish, no CAS, true.
    L.setRuntime({ 2, -1, 1.0f, -1, Clip::BeatSnapMode::Off });
    LayerRuntimeSnapshot idle = L.runtime();
    CHECK(LayerClock::tick(L, idle, 0.1f));
    CHECK(idle == L.runtime());
}

TEST_CASE("contention: GL-thread fade ticks and beat-fired triggers vs a message-thread trigger storm",
          "[layer_runtime][layer_clock]")
{
    // Ruling amendment 4. Two threads, a start barrier, until the tick has adopted at least once (or 2 s):
    //  - the GL thread loads the tuple, checks R4's I1 / I2 on it, ticks the fade (LayerClock::tick, ONE CAS, counted
    //    adopts) and calls processPendingTrigger(0, 0, 16) (a queued trigger fires on every call);
    //  - the message thread triggers the columns cyclically (never a retrigger), alternately immediate and queued
    //    (forced Beat snap); after a queued one it waits until the GL thread fired it, so the activation order stays
    //    cyclic and I2 stays exact. Every message-thread transition must equal its intent.
    // Fades never end inside the run (transitionSpeed 1000 s), so every GL tick is a real CAS a trigger can beat.
    Deck deck;   // heap-allocated Layer (Deck::initDefault), as R4
    deck.numColumns = 4;
    deck.initDefault();
    for (int c = 0; c < 4; ++c)
        deck.setClip(0, c, Clip{});
    Layer& L = deck.layers[0];
    L.transitionSpeed = 1000.0f;
    L.setRuntime({ 0, -1, 1.0f, -1, Clip::BeatSnapMode::Off });

    std::atomic<int> arrived{ 0 };
    std::atomic<bool> stop{ false };
    std::atomic<long> adopts{ 0 }, ticks{ 0 }, i1{ 0 }, i2{ 0 }, fired{ 0 };
    std::thread render([&] {
        arrived.fetch_add(1);
        while (arrived.load() < 2) std::this_thread::yield();
        while (!stop.load())
        {
            LayerRuntimeSnapshot rt = L.runtime();
            if (rt.activeClipColumn >= 0 && rt.previousClipColumn == rt.activeClipColumn)
                i1.fetch_add(1);
            if (rt.previousClipColumn != -1 && rt.previousClipColumn != (rt.activeClipColumn + 3) % 4)
                i2.fetch_add(1);
            if (!LayerClock::tick(L, rt, 0.001f))
                adopts.fetch_add(1);
            ticks.fetch_add(1);
            if (L.processPendingTrigger(0, 0, 16).changed())
                fired.fetch_add(1);
        }
    });

    arrived.fetch_add(1);
    while (arrived.load() < 2) std::this_thread::yield();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    long intentMismatches = 0, triggers = 0;
    bool stalled = false;
    for (int i = 1; adopts.load() == 0 && std::chrono::steady_clock::now() < deadline && !stalled; ++i)
    {
        const int col = i % 4;
        const bool queued = (i % 2) == 0;
        const auto t = L.triggerClip(col, queued ? Clip::BeatSnapMode::Beat : Clip::BeatSnapMode::Off);
        ++triggers;
        if (queued ? t.after.pendingTriggerColumn != col : t.after.activeClipColumn != col)
            ++intentMismatches;
        if (queued)
            while (L.runtime().activeClipColumn != col)
                if (std::chrono::steady_clock::now() > deadline + std::chrono::seconds(5)) { stalled = true; break; }
    }
    stop.store(true);
    render.join();

    INFO("triggers " << triggers << ", GL ticks " << ticks.load() << ", adopts " << adopts.load() << ", fired "
         << fired.load() << "; intent mismatches " << intentMismatches << ", I1 " << i1.load() << " I2 " << i2.load());
    REQUIRE_FALSE(stalled);
    CHECK(intentMismatches == 0);
    REQUIRE(adopts.load() > 0);   // the adopt path ran
    CHECK(i1.load() == 0);
    CHECK(i2.load() == 0);
}

// ---------------------------------------------------------------------------
// s-rta-1002 fix round, ruling F2 (review-tsan-memmodel-r1 S1): Autopilot::processFrame loads a layer's tuple ONCE,
// decides an advance from it, then calls triggerClip(next, Off, 16, currentCol) (Autopilot.cpp advanceClip /
// smartAdvanceClip). The window between that load and the CAS is the whole advance path, so the trigger is
// conditioned on the column it decided from: a clear or a user trigger that landed first stands.
// ---------------------------------------------------------------------------

TEST_CASE("a render-side advance decided before a clear leaves the layer cleared", "[layer_runtime][autopilot]")
{
    Layer L = restingLayer();
    L.clips[0]->playing = true;
    const LayerRuntimeSnapshot decided = L.runtime();   // GL: processFrame's one load (active 0)
    REQUIRE(decided.activeClipColumn == 0);
    const auto cleared = L.clearActiveClip();           // message thread: the X clear lands first
    REQUIRE(cleared.after.activeClipColumn == -1);
    L.clips[1]->beatsPlayed = 9;
    L.clips[1]->playheadPosition = 0.8;

    // GL: the advance (PlayNext 0 -> 1) with the stale snapshot -- exactly Autopilot::advanceClip's call.
    const auto t = L.triggerClip(1, Clip::BeatSnapMode::Off, 16, decided.activeClipColumn);
    CHECK(t.applied);
    CHECK_FALSE(t.changed());
    CHECK(L.runtime() == cleared.after);           // the layer stays cleared
    CHECK(L.clips[1]->playing == false);           // no activation tail on the clip it would have started
    CHECK(L.clips[1]->beatsPlayed == 9);
    CHECK(L.clips[1]->playheadPosition == 0.8);
    CHECK(L.clips[0]->playing == false);           // the clear's tail stands

    // An advance decided from the CURRENT tuple still applies (the guard is not a blanket refusal).
    L.triggerClipImmediate(2);
    const auto fresh = L.triggerClip(3, Clip::BeatSnapMode::Off, 16, L.runtime().activeClipColumn);
    CHECK(fresh.changed());
    CHECK(L.runtime().activeClipColumn == 3);
}

TEST_CASE("a render-side advance decided before a user trigger leaves the user's clip active", "[layer_runtime][autopilot]")
{
    SECTION("the user triggered another column (k = 2; the advance wanted 1)")
    {
        Layer L = restingLayer();
        const LayerRuntimeSnapshot decided = L.runtime();   // GL: active 0
        const auto user = L.triggerClipImmediate(2);         // message thread: the user's trigger lands first
        REQUIRE(user.after.activeClipColumn == 2);
        const auto t = L.triggerClip(1, Clip::BeatSnapMode::Off, 16, decided.activeClipColumn);
        CHECK_FALSE(t.changed());
        CHECK(L.runtime() == user.after);                   // k stays active, its fade untouched
        CHECK(L.clips[1]->playing == false);                // the advance's clip never started
    }
    SECTION("the user triggered the very column the advance wanted (k = 1): no second activation tail")
    {
        Layer L = restingLayer();
        const LayerRuntimeSnapshot decided = L.runtime();   // GL: active 0
        const auto user = L.triggerClipImmediate(1);
        REQUIRE(user.after.activeClipColumn == 1);
        L.clips[1]->beatsPlayed = 3;                        // the user's clip has started counting
        L.clips[1]->playheadPosition = 0.6;
        const auto t = L.triggerClip(1, Clip::BeatSnapMode::Off, 16, decided.activeClipColumn);
        CHECK_FALSE(t.changed());
        CHECK(L.runtime() == user.after);
        CHECK(L.clips[1]->beatsPlayed == 3);                // not re-zeroed: the stale advance did nothing
        CHECK(L.clips[1]->playheadPosition == 0.6);
    }
    SECTION("a stale advance onto an EMPTY cell does not clear the user's clip")
    {
        Layer L = restingLayer();
        L.clips[1].reset();                                 // column 1 empty: triggerClip(1) would clear
        const LayerRuntimeSnapshot decided = L.runtime();   // GL: active 0
        const auto user = L.triggerClipImmediate(2);
        const auto t = L.triggerClip(1, Clip::BeatSnapMode::Off, 16, decided.activeClipColumn);
        CHECK_FALSE(t.changed());
        CHECK(L.runtime() == user.after);
    }
}

TEST_CASE("stress: a real autopilot advancing every frame never re-activates a layer the message thread cleared",
          "[layer_runtime][autopilot]")
{
    // The reviewer's repro (review-tsan-memmodel-r1 S1), bounded: the GL thread runs Autopilot::processFrame with a
    // beat crossing EVERY frame (PlayNext at Beat1, so an advance is decided and attempted every frame); the message
    // thread triggers, yields, clears, and checks that the cleared layer is still cleared right after the clear and
    // again 300 yields later. Without the guard an advance decided before the clear re-activates the layer (99,352 of
    // 100,000 clears in the review); with it the count is 0 by construction, so this never false-fails.
    Deck deck;
    deck.numColumns = 4;
    deck.initDefault();
    for (int c = 0; c < 4; ++c)
    {
        Clip clip;
        clip.mediaType = Clip::MediaType::Video;
        clip.autopilotAction = Clip::AutopilotAction::PlayNext;
        clip.autopilotDuration = Clip::AutopilotDuration::Beat1;
        deck.setClip(0, c, clip);
    }
    Layer& L = deck.layers[0];
    L.autopilotEnabled = true;
    L.transitionSpeed = 0.0f;   // cuts
    L.triggerClipImmediate(0);

    std::atomic<bool> stop{ false };
    std::atomic<long> frames{ 0 }, advances{ 0 };
    std::thread render([&] {
        Autopilot autopilot;
        FeatureSnapshot snap;
        while (!stop.load())
        {
            snap.totalBeatCount++;
            Autopilot::FrameReport report;
            autopilot.processFrame(deck, snap, &report);
            advances.fetch_add(report.advances);
            frames.fetch_add(1);
        }
    });
    const auto startBy = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (frames.load() < 1 && std::chrono::steady_clock::now() < startBy)
        std::this_thread::yield();
    if (frames.load() < 1)
    {
        stop.store(true);
        render.join();
        FAIL("render thread never started");
    }

    long clears = 0, activeRightAfter = 0, activeLater = 0;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    for (int i = 0; i < 5000 && std::chrono::steady_clock::now() < deadline; ++i)
    {
        L.triggerClipImmediate(i % 4);
        std::this_thread::yield();   // let the GL thread load the tuple and decide an advance
        const auto t = L.clearActiveClip();
        if (!(t.before.activeClipColumn >= 0 && t.after.activeClipColumn < 0))
            continue;
        ++clears;
        if (L.runtime().activeClipColumn >= 0)
            ++activeRightAfter;
        for (int y = 0; y < 300; ++y)
            std::this_thread::yield();
        if (L.runtime().activeClipColumn >= 0)
            ++activeLater;
    }
    stop.store(true);
    render.join();

    INFO("clears " << clears << ", GL frames " << frames.load() << ", applied advances " << advances.load()
         << "; active right after the clear " << activeRightAfter << ", 300 yields later " << activeLater);
    REQUIRE(clears > 0);
    REQUIRE(advances.load() > 0);   // the autopilot really advanced (not a vacuous pass)
    CHECK(activeRightAfter == 0);
    CHECK(activeLater == 0);
}
