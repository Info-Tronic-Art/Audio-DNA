// Lane bf9b (s-rta-1002b), plan-bf9b S2.12 / ruling-bf9b amendments 2(c), 12: the show model -- ONE shared layer stack
// (Composition::layers) over deck boxes of clip rows (Deck::rows); a layer names its clip by ClipRef (deck id, column).
// S2a lands the first cases (T2, T3, T9, T10, T15 -- the model API S2a introduces); S2b adds the rest of plan S2.12
// (T1, T4-T8, T11-T14, T16, M1-M7; ruling-bf9b amendments 4, 6, 7, 8, 9, 12, 20, 21, 22).
// RED on STAGE_P_HEAD / S1 head: does not compile (no Composition::layers, no ClipRef trigger API).
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include "model/Autopilot.h"
#include "model/Routine.h"
#include "model/ShowMigration.h"
#include "core/UndoManager.h"
#include "core/UndoService.h"
#include "core/ClipCommands.h"
#include "core/DeckCommands.h"
#include "core/TriggerCommands.h"
#include "core/CompositionLoad.h"
#include "render/LayerClock.h"
#include "recording/Program.h"
#include "recording/RoutineEngine.h"
#include "recording/PerfState.h"
#include "recording/Take.h"
#include "midi/MidiOutputHandler.h"
#include "binding/BindingTarget.h"
#include "connect/ManualWrite.h"
#include "routing/MacroBank.h"
#include "ui/DeckView.h"
#include "ui/LayerStrip.h"
#include "analysis/FeatureSnapshot.h"
#include "ShowFixture.h"
#include <cmath>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

using ShowFixture::makeShow;
using Catch::Approx;
using Snap = Clip::BeatSnapMode;

namespace
{
DeckFenceHook passFence() { return [](const std::function<void()>& m) { if (m) m(); }; }
CompositionResolver resolverFor(Composition& c) { return [&c]() -> Composition* { return &c; }; }
ClipMediaHook noMedia() { return [](const Clip&) {}; }
ClipMediaDisposeHook noDispose() { return [](const Clip&) {}; }

// The UndoService fence of the app, headless (no renderer): withDeckDetached runs the mutation and REAPS (amendment
// 4(a)) -- every reaped deck id is recorded.
struct Fenced
{
    UndoService svc;
    std::vector<uint32_t> reaped;
    explicit Fenced(Composition& c)
    {
        svc.setCollaborators(&c, nullptr, nullptr);
        svc.onDecksReaped = [this](std::vector<Deck>&& ds) {
            for (const auto& d : ds)
                reaped.push_back(d.id);
        };
    }
    DeckFenceHook hook() { return [this](const std::function<void()>& m) { svc.withDeckDetached(m); }; }
};

// T1 (ruling-bf9b amendment 12): what plays -- every shared layer's raw tuple (each slot's ClipRef, the fade, the
// queue and its snap override), every clip of every live and retired deck (playing / playheadPosition / beatsPlayed /
// hasBeenTriggered), the undo history size and the routine engine's running set. Floats in hex: byte-identical.
std::string fingerprint(const Composition& c, const UndoManager& mgr, const RoutineEngine& eng)
{
    std::ostringstream o;
    o << std::hexfloat;
    for (int i = 0; i < c.getNumLayers(); ++i)
    {
        const auto r = c.layers[static_cast<size_t>(i)].runtime();
        o << "L" << i << " id" << c.layers[static_cast<size_t>(i)].id << " a" << r.activeDeckId << ":"
          << r.activeClipColumn << " p" << r.previousDeckId << ":" << r.previousClipColumn << " f"
          << r.crossfadeProgress << " q" << r.pendingDeckId << ":" << r.pendingTriggerColumn << " s"
          << static_cast<int>(r.pendingTriggerSnapOverride) << "\n";
    }
    c.forEachClip([&o](const Clip& clip, const ClipSite& s) {
        const bool playing = clip.playing;
        const double head = clip.playheadPosition;
        const int beats = clip.beatsPlayed;
        const bool triggered = clip.hasBeenTriggered;
        o << "C" << s.deckId << "/" << s.row << "/" << s.column << (s.retired ? "r" : "") << " id" << clip.id
          << " pl" << playing << " ph" << head << " b" << beats << " t" << triggered << "\n";
    });
    o << "H" << mgr.historySize() << "\n";
    const auto st = eng.status();
    for (int k = 0; k < RoutineEngine::kBankSize; ++k)
        o << "R" << k << " " << st.slots[k].state << " " << st.slots[k].fireSeq << " " << st.slots[k].cycle << "\n";
    return o.str();
}

// A stub dispatch: the engine runs, the model is untouched (only the engine's own state matters to T1).
void wireStub(RoutineEngine& eng)
{
    eng.dispatch.fire = [](const Fired&) { return true; };
    eng.dispatch.touch = [](const ControlPath&, const std::string&) { return true; };
    eng.dispatch.set = [](const ControlPath&, float) { return true; };
    eng.dispatch.release = [](const ControlPath&) {};
    eng.dispatch.notify = [](const std::string&) {};
    eng.dispatch.read = [](const ControlPath&) -> std::optional<float> { return std::nullopt; };
}

FeatureSnapshot beatSnap(double beat)
{
    FeatureSnapshot s;
    s.clear();
    s.bpm = 120.0f;
    s.trackerState = 2;
    s.beatPhase = static_cast<float>(beat - std::floor(beat));
    s.totalBeatCount = static_cast<uint32_t>(std::floor(beat));
    s.beatInBar = static_cast<uint8_t>(static_cast<int>(std::floor(beat)) % 4);
    s.totalBarCount = static_cast<uint32_t>(std::floor(beat / 4.0));
    s.barCount = static_cast<uint16_t>(std::floor(beat / 4.0));
    return s;
}

ControlPath layerKey(int layer, const std::string& control, const std::string& scalar = "")
{
    ControlPath k;
    k.scope = ControlPath::Scope::Layer;
    k.deckRelative = true;
    k.deck = 0;
    k.deckName = "Deck 1";
    k.layer = layer;
    k.layerName = "Layer " + std::to_string(layer + 1);
    k.control = control;
    k.scalar = scalar;
    return k;
}

// A 64-beat looping routine with one continuous lane (layer 0 opacity): it RUNS (T1 needs a running set).
Routine runningRoutine()
{
    Routine r;
    r.uuid = "t1";
    r.name = "T1";
    r.lengthBeats = 64.0;
    r.quantize = Snap::Off;
    r.loop = true;
    const ControlPath key = layerKey(0, "scalar", "opacity");
    Lane lane;
    lane.key = key;
    lane.kind = Lane::Kind::Continuous;
    Gesture g;
    g.grip = "held";
    g.curve.pts = { { 0.0, 0.2f }, { 32.0, 0.8f } };
    lane.gestures = { g };
    r.lanes[key] = lane;
    return r;
}

template <typename T>
void collect(juce::Component& root, std::vector<T*>& out)
{
    for (int i = 0; i < root.getNumChildComponents(); ++i)
    {
        auto* child = root.getChildComponent(i);
        if (auto* t = dynamic_cast<T*>(child))
            out.push_back(t);
        collect(*child, out);
    }
}

// Every live and retired deck holds exactly one row per shared layer.
bool rowsEqualLayers(const Composition& c)
{
    for (const auto& d : c.decks)
        if (d.getNumRows() != c.getNumLayers())
            return false;
    for (const auto& d : c.retiredDecks())
        if (d.getNumRows() != c.getNumLayers())
            return false;
    return true;
}

// The clip ids of a row (0 = an empty cell) -- a deep-enough row compare for the structure tests.
std::vector<uint32_t> rowIds(const ClipRow& r)
{
    std::vector<uint32_t> ids;
    for (const auto& cell : r.clips)
        ids.push_back(cell.has_value() ? cell->id : 0u);
    return ids;
}
} // namespace

TEST_CASE("T2 firing a cell of a deck that is not shown puts its ClipRef in that row's shared layer (bf9b)", "[show]")
{
    Composition c = makeShow(2, 3, 4);
    c.activeDeckIndex = 0;
    const uint32_t deck1 = c.decks[1].id;
    const auto t = c.fire(1, 1, 2);
    CHECK(t.changed());
    CHECK(c.layers[1].runtime().activeRef() == ClipRef{ deck1, 2 });
    REQUIRE(c.playingClip(1) != nullptr);
    CHECK(c.playingClip(1) == c.decks[1].getClip(1, 2));
    CHECK(c.playingClip(1)->id == 100u * 1 + 10u * 1 + 2 + 1);
    CHECK(c.playingClip(0) == nullptr);   // other layers untouched
    CHECK(c.activeDeckIndex == 0);        // a fire never moves the grid
}

TEST_CASE("T3 the same column from another deck is a new clip: a crossfade, not a retrigger (bf9b)", "[show]")
{
    Composition c = makeShow(2, 2, 4);
    c.layers[0].transitionSpeed = 1.0f;   // a real fade
    c.fire(0, 0, 2);
    const auto t = c.fire(0, 1, 2);
    const auto rt = c.layers[0].runtime();
    CHECK(t.changed());
    CHECK(rt.activeRef() == ClipRef{ c.decks[1].id, 2 });
    CHECK(rt.previousRef() == ClipRef{ c.decks[0].id, 2 });
    CHECK(rt.crossfadeProgress == 0.0f);
    // and the same ref again IS a retrigger: the tuple keeps its fade
    const auto again = c.fire(0, 1, 2);
    CHECK_FALSE(again.changed());
}

TEST_CASE("T9 C1 playing(i): one load, the incoming clip, nullptr when clear or empty, any deck incl. a retired one (bf9b)",
          "[show]")
{
    Composition c = makeShow(3, 2, 3);
    CHECK(c.playingClip(0) == nullptr);                   // clear
    c.fire(0, 2, 1);                                      // deck 2 (not shown)
    auto p = c.playing(0);
    REQUIRE(p.clip != nullptr);
    CHECK(p.ref == ClipRef{ c.decks[2].id, 1 });
    CHECK(p.deckIndex == 2);
    CHECK_FALSE(p.retired);
    CHECK(p.clip == c.decks[2].getClip(0, 1));

    // An empty target cell clears the layer (F12 / Layer::triggerClip's empty-cell branch) -> nullptr.
    c.decks[0].clearCell(1, 0);
    c.fire(1, 0, 0);
    CHECK(c.playingClip(1) == nullptr);

    // Retire deck 2 while layer 0 plays from it: the clip still resolves, at the same address.
    const Clip* before = p.clip;
    const uint32_t id2 = c.decks[2].id;
    CHECK(c.retireOrEraseDeck(2));
    REQUIRE(c.decks.size() == 2);
    CHECK(c.getNumRetiredDecks() == 1);
    p = c.playing(0);
    CHECK(p.clip == before);
    CHECK(p.retired);
    CHECK(p.deckIndex == -1);
    CHECK(p.ref.deckId == id2);
}

TEST_CASE("T10 C2 forEachLayer visits each shared layer once; forEachClip live decks by index, then retired, rows and "
          "columns ascending (bf9b)", "[show]")
{
    Composition c = makeShow(3, 2, 2);
    std::vector<int> seenLayers;
    c.forEachLayer([&](Layer&, int i) { seenLayers.push_back(i); });
    CHECK(seenLayers == std::vector<int>{ 0, 1 });

    c.fire(0, 1, 0);                   // deck 1 plays -> it is retired, not erased
    const uint32_t id1 = c.decks[1].id;
    REQUIRE(c.retireOrEraseDeck(1));

    std::vector<uint32_t> order;
    std::vector<int> deckIdx;
    std::vector<bool> retired;
    c.forEachClip([&](Clip& clip, const ClipSite& site) {
        order.push_back(clip.id);
        deckIdx.push_back(site.deckIndex);
        retired.push_back(site.retired);
        if (site.retired) CHECK(site.deckId == id1);
    });
    // live deck 0, live deck 2 (now index 1), then the retired deck 1; rows then columns ascending
    const std::vector<uint32_t> expected{ 1, 2, 11, 12, 201, 202, 211, 212, 101, 102, 111, 112 };
    CHECK(order == expected);
    CHECK(deckIdx == std::vector<int>{ 0, 0, 0, 0, 1, 1, 1, 1, -1, -1, -1, -1 });
    CHECK(retired.back());
}

TEST_CASE("T15 a ref the tuple cannot hold is refused at every tuple-writing entry: the tuple is byte-identical (bf9b, "
          "ruling-bf9b amendment 2(c))", "[show]")
{
    Composition c = makeShow(2, 2, 3);
    c.fire(0, 0, 1);
    const auto before0 = c.layers[0].runtime();
    const auto before1 = c.layers[1].runtime();
    auto unchanged = [&] {
        CHECK(c.layers[0].runtime() == before0);
        CHECK(c.layers[1].runtime() == before1);
    };
    const RowClips rows0 = c.rowClips(0);

    // column > kMaxColumn
    CHECK_FALSE(c.fire(0, 0, ClipRef::kMaxColumn + 1).changed());
    unchanged();
    CHECK_FALSE(c.layers[0].triggerClip(ClipRef{ c.decks[0].id, ClipRef::kMaxColumn + 1 }, rows0).changed());
    CHECK_FALSE(c.layers[0].triggerClipImmediate(ClipRef{ c.decks[0].id, ClipRef::kMaxColumn + 1 }, rows0).changed());
    unchanged();
    // deck > kMaxDeckId
    CHECK_FALSE(c.layers[0].triggerClip(ClipRef{ ClipRef::kMaxDeckId + 1, 0 }, rows0).changed());
    CHECK_FALSE(c.layers[0].releaseMomentary(ClipRef{ ClipRef::kMaxDeckId + 1, 1 }, rows0).changed());
    unchanged();
    // deck == kNoDeck with a column (the deck-less S1 form)
    CHECK_FALSE(c.layers[0].triggerClip(ClipRef{ ClipRef::kNoDeck, 1 }, rows0).changed());
    CHECK_FALSE(c.layers[0].triggerClipImmediate(ClipRef{ ClipRef::kNoDeck, 1 }, rows0).changed());
    CHECK_FALSE(c.layers[0].releaseMomentary(ClipRef{ ClipRef::kNoDeck, 1 }, rows0).changed());
    CHECK_FALSE(c.layers[0].clearActiveClip(rows0, ClipRef{ ClipRef::kNoDeck, 1 }).changed());
    unchanged();
    // an unknown deck index (Composition::fire / triggerColumn resolve a deck-less ref)
    CHECK_FALSE(c.fire(0, 7, 1).changed());
    std::vector<std::optional<LayerRuntimeTransition>> out;
    c.triggerColumn(7, 1, Clip::BeatSnapMode::Off, &out);
    unchanged();
    c.triggerColumn(0, ClipRef::kMaxColumn + 1, Clip::BeatSnapMode::Off, &out);
    unchanged();
}

TEST_CASE("T1 every deck-switch path and deck command leaves what plays byte-identical: tuples, every clip's runtime, "
          "the undo history, the running routines (bf9b, ruling-bf9b amendment 12)", "[show]")
{
    // 6 filled decks + one EMPTY deck (6) no ref names (the Remove Deck subject). Layer 0 mid-fade from deck 0 into
    // deck 1; layer 1 plays deck 2 and queues deck 0 (Bar); layer 2 queues deck 1 (Beat); a routine running.
    Composition c = makeShow(6, 3, 4);
    {
        Deck empty;
        empty.name = "Empty";
        empty.numColumns = 4;
        empty.initDefault(3);
        REQUIRE(c.appendDeck(std::move(empty)) == 6);
    }
    c.layers[0].transitionSpeed = 4.0f;
    c.fire(0, 0, 1, Snap::Off, true);
    c.fire(0, 1, 2, Snap::Off, true);
    LayerClock::advanceCrossfade(c.layers[0], 1.0f);   // progress 0.25, previous = deck 0 col 1
    c.fire(1, 2, 3, Snap::Off, true);
    c.fire(1, 0, 0, Snap::Bar);
    c.fire(2, 1, 1, Snap::Beat);
    c.decks[1].getClip(0, 2)->playheadPosition = 0.4;
    REQUIRE(c.layers[0].runtime().previousRef() == (ClipRef{ c.decks[0].id, 1 }));
    REQUIRE(c.layers[1].runtime().pendingRef() == (ClipRef{ c.decks[0].id, 0 }));
    REQUIRE(c.layers[2].runtime().pendingRef() == (ClipRef{ c.decks[1].id, 1 }));
    c.activeDeckIndex = 0;

    UndoManager mgr;
    mgr.perform(std::make_unique<RenameDeckCmd>(resolverFor(c), 0, "Deck 1", "Deck 1", "Rename Deck"));   // a history
    RoutineEngine eng;
    wireStub(eng);
    c.routines.push_back(runningRoutine());
    REQUIRE(c.assignRoutineSlot(0, "t1"));
    eng.tick(beatSnap(0.0), 0.0, c, RoutineSnap::Off, true);
    REQUIRE(eng.fire(c, 0, RoutineSnap::Off, true).empty());
    eng.tick(beatSnap(1.0), 0.5, c, RoutineSnap::Off, true);
    REQUIRE(eng.status().slots[0].state == "running");

    const std::string f0 = fingerprint(c, mgr, eng);

    // bf9b S2c (ruling-bf9b amendment 10): a deck switch is no Undo step any more (no SwitchDeckCmd); the switch the
    // app makes is the index + DeckView::showDeck (handleDeckSwitch; its renderer fence token is not headless).
    SECTION("the model-level shown-deck walk 0 -> 1 -> 0")
    {
        for (int d : { 1, 0 })
        {
            c.activeDeckIndex = d;
            CHECK(fingerprint(c, mgr, eng) == f0);
        }
    }

    SECTION("a headless DeckView::showDeck walk 0 -> 5 -> 0: what plays is untouched, every LayerStrip the same object")
    {
        juce::ScopedJuceInitialiser_GUI gui;
        DeckView dv;
        dv.setSize(1400, 600);
        dv.setComposition(&c);
        std::vector<LayerStrip*> before;
        collect(dv, before);
        REQUIRE(before.size() == 3);
        for (int d : { 5, 0 })
        {
            c.activeDeckIndex = d;
            dv.showDeck();
            CHECK(fingerprint(c, mgr, eng) == f0);
            std::vector<LayerStrip*> now;
            collect(dv, now);
            CHECK(now == before);
        }
    }

    SECTION("AddDeckCmd, InsertDeckCmd (an empty box) and RemoveDeckCmd (of a deck no ref names) execute / undo / redo")
    {
        AddDeckCmd add(resolverFor(c), passFence(), "Add Deck");
        add.execute();
        CHECK(fingerprint(c, mgr, eng) == f0);
        add.undo();
        CHECK(fingerprint(c, mgr, eng) == f0);
        add.execute();
        CHECK(fingerprint(c, mgr, eng) == f0);
        add.undo();

        Deck box;
        box.name = "Loaded";
        box.numColumns = 4;
        box.initDefault(3);
        InsertDeckCmd ins(resolverFor(c), passFence(), noMedia(), noDispose(), box, "Load Deck");
        ins.execute();
        CHECK(fingerprint(c, mgr, eng) == f0);
        ins.undo();
        CHECK(fingerprint(c, mgr, eng) == f0);
        ins.execute();
        CHECK(fingerprint(c, mgr, eng) == f0);
        ins.undo();

        REQUIRE(c.decks[6].name == "Empty");
        RemoveDeckCmd rem(resolverFor(c), passFence(), noMedia(), noDispose(), 6, c.decks[6], c.activeDeckIndex,
                          "Remove Deck");
        rem.execute();
        CHECK_FALSE(rem.retired());
        CHECK(fingerprint(c, mgr, eng) == f0);
        rem.undo();
        CHECK(fingerprint(c, mgr, eng) == f0);
        rem.execute();
        CHECK(fingerprint(c, mgr, eng) == f0);
    }
}

TEST_CASE("T1 a 20-deck walk leaves what plays byte-identical (bf9b, ruling-bf9b amendment 12)", "[show]")
{
    Composition c = makeShow(20, 3, 2);
    c.fire(0, 3, 1, Snap::Off, true);
    c.fire(1, 7, 0, Snap::Off, true);
    c.fire(2, 13, 1, Snap::Off, true);
    c.fire(0, 19, 0, Snap::Bar);
    UndoManager mgr;
    RoutineEngine eng;
    wireStub(eng);
    const std::string f0 = fingerprint(c, mgr, eng);
    for (int d = 0; d < 20; ++d)
    {
        c.activeDeckIndex = d;
        CHECK(fingerprint(c, mgr, eng) == f0);
    }
    for (int d = 19; d >= 0; --d)
    {
        c.activeDeckIndex = d;
        CHECK(fingerprint(c, mgr, eng) == f0);
    }
}

TEST_CASE("T4 a column fire fires the shown deck's column into every non-ignoring layer; an Ignore Column layer keeps "
          "its other-deck clip; an empty cell clears its layer (bf9b)", "[show]")
{
    Composition c = makeShow(2, 3, 4);
    c.fire(2, 1, 3, Snap::Off, true);          // layer 2 plays deck 1's (2, 3) ...
    c.layers[2].ignoreColumnTrigger = true;    // ... and ignores column triggers
    c.fire(1, 1, 0, Snap::Off, true);          // layer 1 plays deck 1's (1, 0)
    c.decks[0].clearCell(1, 2);                // deck 0's (1, 2) is empty
    c.activeDeckIndex = 0;

    std::vector<std::optional<LayerRuntimeTransition>> out;
    c.triggerColumn(c.activeDeckIndex, 2, Snap::Off, &out);
    REQUIRE(out.size() == 3);
    CHECK(c.layers[0].runtime().activeRef() == (ClipRef{ c.decks[0].id, 2 }));
    CHECK(c.playingClip(0) == c.decks[0].getClip(0, 2));
    CHECK(c.layers[1].runtime().activeClipColumn == -1);   // F12 / Q5 default: the empty cell cleared layer 1
    CHECK(c.playingClip(1) == nullptr);
    CHECK_FALSE(out[2].has_value());                       // the ignoring layer was not touched
    CHECK(c.layers[2].runtime().activeRef() == (ClipRef{ c.decks[1].id, 3 }));
    CHECK(c.playingClip(2) == c.decks[1].getClip(2, 3));
}

TEST_CASE("T5 a bar-snapped trigger queued on deck 0 survives a switch to deck 1 and fires on its bar (bf9b, K5)",
          "[show]")
{
    Composition c = makeShow(2, 2, 4);
    c.fire(0, 0, 0, Snap::Off, true);
    c.decks[0].getClip(0, 2)->beatSnapMode = Snap::Bar;
    c.fire(0, 0, 2);                           // queued for the bar (the clip's own snap)
    const LayerRuntimeSnapshot queued = c.layers[0].runtime();
    REQUIRE(queued.pendingRef() == (ClipRef{ c.decks[0].id, 2 }));

    // The switch as handleDeckSwitch makes it (bf9b S2c: no SwitchDeckCmd): the index, then the grid's cells
    // (DeckView::showDeck, headless; the renderer's fence token is not).
    juce::ScopedJuceInitialiser_GUI gui;
    DeckView dv;
    dv.setSize(1400, 600);
    dv.setComposition(&c);
    c.activeDeckIndex = 1;
    dv.showDeck();
    REQUIRE(c.activeDeckIndex == 1);
    CHECK(c.layers[0].runtime() == queued);    // the queue survives the switch (F11)

    const auto notYet = c.layers[0].processPendingTrigger(2, 0, c.rowClips(0));   // beat 3 of the bar: waits
    CHECK_FALSE(notYet.changed());
    const auto fired = c.layers[0].processPendingTrigger(0, 1, c.rowClips(0));    // the bar
    CHECK(fired.changed());
    CHECK(c.layers[0].runtime().activeRef() == (ClipRef{ c.decks[0].id, 2 }));
    CHECK(c.playingClip(0) == c.decks[0].getClip(0, 2));
}

TEST_CASE("T6 Remove Deck while its clip plays: retired, still resolves, a queue into it cancelled; undo moves it back "
          "with the live playhead; replaced and a fenced edit later it is reaped (bf9b, plan F3)", "[show]")
{
    Composition c = makeShow(3, 2, 4);
    const uint32_t id1 = c.decks[1].id;
    c.fire(0, 1, 2, Snap::Off, true);                     // layer 0 plays deck 1 (a default layer: Cut transitions)
    Clip* const playing = c.playingClip(0);
    REQUIRE(playing == c.decks[1].getClip(0, 2));
    c.decks[1].getClip(1, 3)->beatSnapMode = Snap::Bar;
    c.fire(1, 1, 3);                                      // a queue INTO deck 1 on layer 1
    REQUIRE(c.layers[1].runtime().pendingRef() == (ClipRef{ id1, 3 }));

    Fenced fenced(c);
    RemoveDeckCmd remove(resolverFor(c), fenced.hook(), noMedia(), noDispose(), 1, c.decks[1], c.activeDeckIndex,
                         "Remove Deck");
    remove.execute();
    CHECK(remove.retired());
    CHECK(c.decks.size() == 2);
    CHECK(c.getNumRetiredDecks() == 1);
    CHECK(c.playingClip(0) == playing);                   // the same Clip, at its address
    CHECK(c.playing(0).retired);
    CHECK(c.playing(0).deckIndex == -1);
    CHECK(c.layers[1].runtime().pendingTriggerColumn == -1);   // the queue into the removed deck is cancelled
    CHECK(fenced.reaped.empty());                         // still playing: not reaped by its own fence

    playing->playheadPosition = 0.8;                      // it keeps playing while retired
    remove.undo();
    CHECK(c.decks.size() == 3);
    CHECK(c.decks[1].id == id1);
    CHECK(c.getNumRetiredDecks() == 0);
    CHECK(c.playingClip(0) == playing);                   // moved back: the live clip, not the snapshot copy
    CHECK(static_cast<double>(playing->playheadPosition) == Approx(0.8));
    CHECK_FALSE(c.playing(0).retired);

    remove.execute();                                     // retire again ...
    REQUIRE(c.getNumRetiredDecks() == 1);
    c.fire(0, 0, 0, Snap::Off, true);                     // ... the layer is replaced (a Cut: previous names deck 1)
    CHECK(c.getNumRetiredDecks() == 1);                   // nothing reaps outside a fence
    fenced.svc.withDeckDetached([] {});                   // any fenced edit
    CHECK(c.getNumRetiredDecks() == 0);
    CHECK(fenced.reaped == std::vector<uint32_t>{ id1 });
    CHECK(c.findDeckById(id1) == nullptr);
}

TEST_CASE("T6c Remove Deck mid-crossfade with the PREVIOUS ref in it: retired until the fade completes, then the first "
          "fenced edit reaps it (bf9b, ruling-bf9b amendment 4(b)(f))", "[show]")
{
    Composition c = makeShow(2, 1, 4);
    const uint32_t id1 = c.decks[1].id;
    Layer& L = c.layers[0];
    L.transitionSpeed = 1.0f;
    c.fire(0, 1, 0, Snap::Off, true);                     // deck 1's clip ...
    c.fire(0, 0, 0, Snap::Off, true);                     // ... fades OUT into deck 0's
    REQUIRE(L.runtime().previousRef() == (ClipRef{ id1, 0 }));
    REQUIRE(L.runtime().crossfadeProgress == 0.0f);
    const Clip* const out = c.decks[1].getClip(0, 0);
    const Clip* const in = c.decks[0].getClip(0, 0);

    Fenced fenced(c);
    RemoveDeckCmd remove(resolverFor(c), fenced.hook(), noMedia(), noDispose(), 1, c.decks[1], c.activeDeckIndex,
                         "Remove Deck");
    remove.execute();
    REQUIRE(remove.retired());                            // only the fading-out (previous) ref names it

    CHECK(LayerClock::advanceCrossfade(L, 0.5f));         // the fade runs on the same OUT -> IN clips
    CHECK(L.runtime().crossfadeProgress == Approx(0.5f));
    CHECK(c.clipAt(L.runtime().previousRef(), 0) == out);
    CHECK(c.clipAt(L.runtime().activeRef(), 0) == in);
    fenced.svc.withDeckDetached([] {});                   // a fenced edit while progress < 1: kept
    CHECK(c.getNumRetiredDecks() == 1);
    CHECK(fenced.reaped.empty());

    CHECK(LayerClock::advanceCrossfade(L, 0.6f));         // complete: previous cleared
    CHECK(L.runtime().previousClipColumn == -1);
    fenced.svc.withDeckDetached([] {});                   // the first fenced edit after completion reaps it
    CHECK(c.getNumRetiredDecks() == 0);
    CHECK(fenced.reaped == std::vector<uint32_t>{ id1 });
}

TEST_CASE("T6d Remove Deck cancels a pending ref INTO the removed deck and leaves one into another deck (bf9b, "
          "ruling-bf9b amendment 4(c)(f))", "[show]")
{
    Composition c = makeShow(3, 2, 4);
    const uint32_t id1 = c.decks[1].id, id2 = c.decks[2].id;
    c.fire(0, 1, 1, Snap::Bar);                           // layer 0 queues deck 1
    c.fire(1, 2, 2, Snap::Bar);                           // layer 1 queues deck 2
    REQUIRE(c.layers[0].runtime().pendingRef() == (ClipRef{ id1, 1 }));
    const LayerRuntimeSnapshot other = c.layers[1].runtime();
    REQUIRE(other.pendingRef() == (ClipRef{ id2, 2 }));

    RemoveDeckCmd remove(resolverFor(c), passFence(), noMedia(), noDispose(), 1, c.decks[1], c.activeDeckIndex,
                         "Remove Deck");
    remove.execute();
    CHECK_FALSE(remove.retired());                        // nothing PLAYS from deck 1: erased
    CHECK(c.layers[0].runtime().pendingTriggerColumn == -1);
    CHECK(c.layers[0].runtime().pendingDeckId == ClipRef::kNoDeck);
    CHECK(c.layers[1].runtime() == other);                // the queue into deck 2 is untouched
}

TEST_CASE("T6e any fenced mutation reaps an unnamed retired deck and hands it to the hook exactly once (bf9b, "
          "ruling-bf9b amendment 4(a)(f))", "[show]")
{
    Composition c = makeShow(3, 2, 3);
    const uint32_t id2 = c.decks[2].id;
    c.fire(0, 2, 1, Snap::Off, true);
    REQUIRE(c.retireOrEraseDeck(2));                      // retired: layer 0 plays it
    Fenced fenced(c);
    fenced.svc.withDeckDetached([&c] { c.decks[0].setClip(1, 0, Clip{}); });   // a fenced cell edit
    CHECK(fenced.reaped.empty());                         // still playing: kept
    c.fire(0, 0, 1, Snap::Off, true);                     // replaced
    fenced.svc.withDeckDetached([&c] { c.decks[0].addColumn(); });             // any fenced mutation
    CHECK(fenced.reaped == std::vector<uint32_t>{ id2 });
    fenced.svc.withDeckDetached([] {});
    CHECK(fenced.reaped == std::vector<uint32_t>{ id2 });  // exactly once
    CHECK(c.getNumRetiredDecks() == 0);
}

TEST_CASE("T7 Add / Remove / Move layer keep rows == layers in every live and retired deck; RemoveLayerCmd's undo "
          "restores the layer and every deck's row (bf9b, ruling-bf9b amendment 22)", "[show]")
{
    Composition c = makeShow(3, 3, 4);
    c.fire(0, 2, 0, Snap::Off, true);
    REQUIRE(c.retireOrEraseDeck(2));                      // deck 2 retired (layer 0 plays it)
    REQUIRE(c.getNumRetiredDecks() == 1);
    REQUIRE(rowsEqualLayers(c));

    SECTION("AddLayerCmd execute / undo / redo")
    {
        AddLayerCmd add(resolverFor(c), passFence(), "Add Layer");
        add.execute();
        CHECK(c.getNumLayers() == 4);
        CHECK(rowsEqualLayers(c));
        add.undo();
        CHECK(c.getNumLayers() == 3);
        CHECK(rowsEqualLayers(c));
        add.execute();
        CHECK(rowsEqualLayers(c));
    }

    SECTION("RemoveLayerCmd: undo restores the layer and every live and retired deck's row")
    {
        const Layer before = c.layers[1];
        const auto d0 = rowIds(*c.decks[0].getRow(1)), d1 = rowIds(*c.decks[1].getRow(1)),
                   r2 = rowIds(*c.retiredDecks()[0].getRow(1));
        RemoveLayerCmd remove(resolverFor(c), passFence(), noMedia(), noDispose(), 1, c.layers[1], "Remove Layer");
        remove.execute();
        CHECK(c.getNumLayers() == 2);
        CHECK(rowsEqualLayers(c));
        remove.undo();
        CHECK(c.getNumLayers() == 3);
        CHECK(rowsEqualLayers(c));
        CHECK(c.layers[1].id == before.id);
        CHECK(c.layers[1].name == before.name);
        CHECK(c.layers[1].runtime() == before.runtime());
        CHECK(rowIds(*c.decks[0].getRow(1)) == d0);
        CHECK(rowIds(*c.decks[1].getRow(1)) == d1);
        CHECK(rowIds(*c.retiredDecks()[0].getRow(1)) == r2);
    }

    SECTION("RemoveLayerCmd undo after a deck was added and the retired deck reaped: an empty row for the new deck, "
            "the reaped deck's snapshot row dropped")
    {
        const auto d0 = rowIds(*c.decks[0].getRow(1));
        RemoveLayerCmd remove(resolverFor(c), passFence(), noMedia(), noDispose(), 1, c.layers[1], "Remove Layer");
        remove.execute();
        REQUIRE(c.addDeck("Deck 4"));                     // a deck the snapshot lacks
        c.fire(0, 0, 0, Snap::Off, true);                 // layer 0 leaves the retired deck ...
        REQUIRE(c.reapRetiredDecks().size() == 1);        // ... which is reaped
        remove.undo();
        CHECK(c.getNumLayers() == 3);
        CHECK(rowsEqualLayers(c));
        CHECK(c.getNumRetiredDecks() == 0);
        CHECK(rowIds(*c.decks[0].getRow(1)) == d0);
        const ClipRow* added = c.decks.back().getRow(1);
        REQUIRE(added != nullptr);
        CHECK(std::all_of(added->clips.begin(), added->clips.end(), [](const auto& cell) { return !cell.has_value(); }));
    }

    SECTION("MoveLayerCmd with a queued trigger and a crossfade in flight: the moved layer's refs resolve to the "
            "same Clip objects afterwards")
    {
        Layer& L1 = c.layers[1];
        L1.transitionSpeed = 2.0f;
        c.fire(1, 0, 0, Snap::Off, true);
        c.fire(1, 1, 1, Snap::Off, true);                 // a fade (deck 0 -> deck 1) in flight on layer 1
        c.fire(1, 0, 3, Snap::Bar);                       // and a queue
        const auto rt = c.layers[1].runtime();
        REQUIRE(rt.crossfadeProgress < 1.0f);
        REQUIRE(rt.pendingTriggerColumn == 3);
        const uint32_t movedId = c.layers[1].id;
        const Clip* active = c.clipAt(rt.activeRef(), 1);
        const Clip* previous = c.clipAt(rt.previousRef(), 1);
        const Clip* pending = c.clipAt(rt.pendingRef(), 1);
        REQUIRE((active != nullptr && previous != nullptr && pending != nullptr));

        MoveLayerCmd move(resolverFor(c), passFence(), 1, 0, "Move Layer Down");
        move.execute();
        CHECK(c.layers[0].id == movedId);
        CHECK(rowsEqualLayers(c));
        const auto rt2 = c.layers[0].runtime();
        CHECK(rt2 == rt);
        CHECK(c.clipAt(rt2.activeRef(), 0) == active);
        CHECK(c.clipAt(rt2.previousRef(), 0) == previous);
        CHECK(c.clipAt(rt2.pendingRef(), 0) == pending);
        move.undo();
        CHECK(c.layers[1].id == movedId);
        CHECK(c.clipAt(c.layers[1].runtime().activeRef(), 1) == active);
        CHECK(rowsEqualLayers(c));
    }
}

TEST_CASE("T7b cell edits keep today's semantics: removing the last column empties only a layer playing from it; a "
          "swap on the shown deck -- the ref follows the CELL (bf9b, ruling-bf9b amendment 21)", "[show]")
{
    Composition c = makeShow(2, 3, 4);
    const uint32_t d0 = c.decks[0].id, d1 = c.decks[1].id;
    c.fire(0, 0, 3, Snap::Off, true);                     // layer 0 plays deck 0's LAST column
    c.fire(1, 0, 1, Snap::Off, true);                     // layer 1 plays deck 0's column 1
    c.fire(2, 1, 3, Snap::Off, true);                     // layer 2 plays deck 1's column 3 (another deck)
    const Clip* l1 = c.playingClip(1);
    const Clip* l2 = c.playingClip(2);

    SECTION("removing the last column")
    {
        std::vector<std::optional<Clip>> removed;
        for (const auto& row : c.decks[0].rows)
            removed.push_back(*row.getClipAt(3));
        REQUIRE(c.decks[0].removeColumn(3));              // live (mutate-then-push), as kColumnRemove
        RemoveColumnCmd cmd(
            [&c](int d) -> Deck* { return d >= 0 && d < static_cast<int>(c.decks.size()) ? &c.decks[static_cast<size_t>(d)] : nullptr; },
            passFence(), noMedia(), noDispose(), 0, 3, 4, std::move(removed), "Remove Column");
        cmd.execute();
        CHECK(c.layers[1].runtime().activeRef() == (ClipRef{ d0, 1 }));   // another column: unchanged
        CHECK(c.playingClip(1) == l1);
        CHECK(c.layers[2].runtime().activeRef() == (ClipRef{ d1, 3 }));   // another deck: unchanged
        CHECK(c.playingClip(2) == l2);
        CHECK(c.layers[0].runtime().activeRef() == (ClipRef{ d0, 3 }));   // the ref is kept (no fix-up) ...
        CHECK(c.playingClip(0) == nullptr);                                // ... and resolves to nothing: empty
    }

    SECTION("a swap on the shown deck: the ref follows the cell")
    {
        const uint32_t wasAt0 = c.decks[0].getClip(1, 0)->id;
        auto at = [&c](int r, int col) { return std::optional<Clip>(*c.decks[0].getClip(r, col)); };
        SwapClipsCmd swap(
            [&c](int d) -> Deck* { return d >= 0 && d < static_cast<int>(c.decks.size()) ? &c.decks[static_cast<size_t>(d)] : nullptr; },
            passFence(), noMedia(), noDispose(), 0, 1, 1, 1, 0, at(1, 1), at(1, 0), at(1, 0), at(1, 1), 4, 4,
            "Swap Clips");
        swap.execute();
        CHECK(c.layers[1].runtime().activeRef() == (ClipRef{ d0, 1 }));
        REQUIRE(c.playingClip(1) != nullptr);
        CHECK(c.playingClip(1)->id == wasAt0);            // the clip now IN the playing cell
    }
}

TEST_CASE("T8 Duplicate Deck: a new deck id, re-minted clip ids, no ref names the copy, the source keeps playing (bf9b, "
          "Pitfall 36)", "[show]")
{
    Composition c = makeShow(2, 2, 3);
    c.fire(0, 1, 2, Snap::Off, true);                     // layer 0 plays the SOURCE deck (1)
    const LayerRuntimeSnapshot rt = c.layers[0].runtime();
    const Clip* const playing = c.playingClip(0);
    uint32_t nextClipId = 5000;
    Deck copy = compload::duplicateDeck(c.decks[1], nextClipId);
    InsertDeckCmd ins(resolverFor(c), passFence(), noMedia(), noDispose(), std::move(copy), "Duplicate Deck");
    ins.execute();
    REQUIRE(c.decks.size() == 3);
    const Deck& dup = c.decks[2];
    CHECK(dup.id != c.decks[0].id);
    CHECK(dup.id != c.decks[1].id);
    std::vector<uint32_t> src, cp;
    for (const auto& r : c.decks[1].rows) for (const auto& cell : r.clips) if (cell) src.push_back(cell->id);
    for (const auto& r : dup.rows) for (const auto& cell : r.clips) if (cell) cp.push_back(cell->id);
    REQUIRE(cp.size() == src.size());
    for (uint32_t id : cp)
        CHECK(std::find(src.begin(), src.end(), id) == src.end());
    CHECK_FALSE(c.deckIsPlaying(dup.id));                 // no ref names the copy
    for (const auto& l : c.layers)
    {
        CHECK(l.runtime().activeDeckId != dup.id);
        CHECK(l.runtime().pendingDeckId != dup.id);
    }
    CHECK(c.layers[0].runtime() == rt);                   // the source keeps playing
    CHECK(c.playingClip(0) == playing);
}

namespace
{
// A bf9b-format composition var whose decks carry the given ids (one shared layer, one row each).
juce::var showWithDeckIds(const std::vector<uint32_t>& ids)
{
    Composition c = makeShow(1, 1, 2, false);
    juce::var v = c.toVar();
    juce::Array<juce::var> decks;
    for (uint32_t id : ids)
    {
        Deck d;
        d.name = "D" + std::to_string(id);
        d.id = id;
        d.numColumns = 2;
        d.initDefault(1);
        decks.add(d.toVar());
    }
    v.getDynamicObject()->setProperty("decks", decks);
    return v;
}
} // namespace

TEST_CASE("T11 deck ids are never reused in a session: oversized ids renumber at load, the mint refuses past "
          "kMaxDeckId, a reaped deck's id never comes back on another deck (bf9b, ruling-bf9b amendment 7)", "[show]")
{
    SECTION("a file's ids {50000, 7} load as {100, 101}")
    {
        Composition c;
        c.fromVar(showWithDeckIds({ 50000u, 7u }));
        REQUIRE(c.decks.size() == 2);
        CHECK(c.decks[0].id == 100u);
        CHECK(c.decks[1].id == 101u);
    }
    SECTION("ids {100, 250} are kept and the next mint is 251")
    {
        Composition c;
        c.fromVar(showWithDeckIds({ 100u, 250u }));
        CHECK(c.decks[0].id == 100u);
        CHECK(c.decks[1].id == 250u);
        REQUIRE(c.addDeck("Next"));
        CHECK(c.decks.back().id == 251u);
    }
    SECTION("appendDeck with the mint past kMaxDeckId refuses and leaves the decks unchanged")
    {
        Composition c = makeShow(1, 1, 1, false);
        uint32_t last = 0;
        int guard = 0;
        while (c.canMintDeckId() && guard++ < 20000)
        {
            Deck d;
            d.numColumns = 1;
            d.initDefault(1);
            const int idx = c.appendDeck(std::move(d));
            REQUIRE(idx == 1);
            last = c.decks[1].id;
            c.decks.erase(c.decks.begin() + 1);           // never reused: the mint marches on
        }
        CHECK(last == ClipRef::kMaxDeckId);
        const auto sizeBefore = c.decks.size();
        Deck more;
        more.initDefault(1);
        CHECK(c.appendDeck(std::move(more)) == -1);
        CHECK(c.decks.size() == sizeBefore);
        CHECK_FALSE(c.addDeck("Refused"));
        AddDeckCmd add(resolverFor(c), passFence(), "Add Deck");
        add.execute();
        CHECK(add.refused());
        CHECK(c.decks.size() == sizeBefore);
    }
    SECTION("remove -> reap -> append -> undo never resolves a ref into the wrong deck")
    {
        Composition c = makeShow(2, 1, 2);
        const uint32_t idA = c.decks[1].id;
        c.fire(0, 1, 1, Snap::Off, true);
        const ClipRef refA = c.layers[0].runtime().activeRef();
        const uint32_t clipA = c.playingClip(0)->id;
        Fenced fenced(c);
        RemoveDeckCmd remove(resolverFor(c), fenced.hook(), noMedia(), noDispose(), 1, c.decks[1], 0, "Remove Deck");
        remove.execute();
        REQUIRE(remove.retired());
        c.fire(0, 0, 0, Snap::Off, true);                 // replaced
        fenced.svc.withDeckDetached([] {});               // reaped
        REQUIRE(fenced.reaped == std::vector<uint32_t>{ idA });
        CHECK(c.clipAt(refA, 0) == nullptr);
        Deck b;
        b.name = "B";
        b.numColumns = 2;
        b.initDefault(1);
        Clip bc;
        bc.id = 7777;
        b.setClip(0, 1, bc);
        REQUIRE(c.appendDeck(std::move(b)) >= 0);
        CHECK(c.decks.back().id != idA);
        CHECK(c.clipAt(refA, 0) == nullptr);              // never B's clip
        remove.undo();                                    // the snapshot comes back under its own id
        REQUIRE(c.clipAt(refA, 0) != nullptr);
        CHECK(c.clipAt(refA, 0)->id == clipA);
    }
}

TEST_CASE("T12 one show autopilot: a layer advances within the deck its playing clip came from, whatever deck is "
          "shown; never from a retired deck; each layer once per beat crossing (bf9b, plan F10, Pitfall 38)", "[show]")
{
    auto beat = [](Autopilot& ap, Composition& c, FeatureSnapshot& s) {
        s.beatPhase = 0.99f;
        ap.processFrame(c, s);
        s.beatPhase = 0.01f;
        s.totalBeatCount++;
        ap.processFrame(c, s);
    };
    auto autopilotShow = [](int decks, int layers, int cols, Clip::AutopilotDuration dur) {
        Composition c = makeShow(decks, layers, cols);
        for (auto& d : c.decks)
            for (auto& r : d.rows)
                for (auto& cell : r.clips)
                    if (cell)
                    {
                        cell->autopilotAction = Clip::AutopilotAction::PlayNext;
                        cell->autopilotDuration = dur;
                    }
        for (auto& l : c.layers)
            l.autopilotEnabled = true;
        return c;
    };

    SECTION("the source deck, not the shown one; a retired source never advances")
    {
        Composition c = autopilotShow(2, 1, 4, Clip::AutopilotDuration::Beat1);
        c.fire(0, 1, 0, Snap::Off, true);                 // from deck 1 ...
        c.activeDeckIndex = 0;                            // ... while deck 0 is shown
        Autopilot ap;
        FeatureSnapshot s;
        beat(ap, c, s);
        CHECK(c.layers[0].runtime().activeRef() == (ClipRef{ c.decks[1].id, 1 }));
        beat(ap, c, s);
        CHECK(c.layers[0].runtime().activeRef() == (ClipRef{ c.decks[1].id, 2 }));

        REQUIRE(c.retireOrEraseDeck(1));                  // the source deck is removed while it plays
        const auto rt = c.layers[0].runtime();
        for (int i = 0; i < 3; ++i)
            beat(ap, c, s);
        CHECK(c.layers[0].runtime() == rt);               // no advance from a retired source (R7)
    }

    SECTION("processFrame once per frame advances each layer exactly once over 4 beats (Beat4)")
    {
        Composition c = autopilotShow(1, 3, 8, Clip::AutopilotDuration::Beat4);
        for (int l = 0; l < 3; ++l)
            c.fire(l, 0, 0, Snap::Off, true);
        Autopilot ap;
        FeatureSnapshot s;
        for (int i = 0; i < 4; ++i)
            beat(ap, c, s);
        for (int l = 0; l < 3; ++l)
            CHECK(c.layers[static_cast<size_t>(l)].runtime().activeClipColumn == 1);
    }
}

TEST_CASE("T13 a routine's clip fires are pinned to their deck by id: a switch, an Insert Deck before it, a Remove of "
          "it (bf9b, ruling-bf9b amendment 6)", "[show]")
{
    Composition c = makeShow(3, 2, 4);
    const uint32_t idA = c.decks[1].id;
    c.activeDeckIndex = 1;                                // deck A shown at the press
    Routine r;
    r.uuid = "pin";
    r.name = "Pin";
    r.lengthBeats = 8.0;
    r.quantize = Snap::Off;
    const ControlPath key = layerKey(0, "activeClip");    // deck-relative: the shown deck at the press (D2)
    Lane lane;
    lane.key = key;
    lane.kind = Lane::Kind::Discrete;
    DiscretePoint pt;
    pt.s = { 1, 0.0, 0 };
    pt.beat = 1.0;
    pt.bpm = 120.0f;
    pt.v = 2;
    lane.points = { pt };
    r.lanes[key] = lane;
    const auto prog = compileRoutine(r, c);
    REQUIRE(prog != nullptr);
    REQUIRE(prog->discrete.size() == 1);
    const Fired& f = prog->discrete[0];
    CHECK(f.target.deckId == idA);

    // The app's replay (dispatch.fire): the deck by pinnedDeckIndex, the fire through the model.
    auto replay = [&c, &f]() -> int {
        const int deck = pinnedDeckIndex(c, f.target);
        if (deck >= 0)
            c.fire(f.target.layer, deck, f.p.v, Snap::Off, true);
        return deck;
    };

    c.activeDeckIndex = 0;                                // switched to deck B
    CHECK(replay() == 1);
    CHECK(c.layers[0].runtime().activeRef() == (ClipRef{ idA, 2 }));
    CHECK(c.playingClip(0) == c.decks[1].getClip(0, 2));

    c.fire(0, 0, 0, Snap::Off, true);
    Deck before;
    before.name = "Inserted";
    before.id = 4000;
    before.numColumns = 4;
    before.initDefault(2);
    c.insertDeckKeepingId(0, std::move(before));          // a deck inserted BEFORE A: A is now index 2
    REQUIRE(c.findDeckIndexById(idA) == 2);
    CHECK(replay() == 2);
    CHECK(c.layers[0].runtime().activeRef() == (ClipRef{ idA, 2 }));

    c.fire(0, 0, 0, Snap::Off, true);
    const LayerRuntimeSnapshot rt = c.layers[0].runtime();
    RemoveDeckCmd remove(resolverFor(c), passFence(), noMedia(), noDispose(), 2, c.decks[2], c.activeDeckIndex,
                         "Remove Deck");
    remove.execute();                                     // deck A removed
    REQUIRE(c.findDeckIndexById(idA) == -1);
    CHECK(replay() == -1);                                // skipped: none lands in another deck
    CHECK(c.layers[0].runtime() == rt);
}

TEST_CASE("T14 a pad lights iff the shown deck's cell is the active ref of its row's layer; a retired source lights "
          "nothing (bf9b, ruling-bf9b amendment 12)", "[show]")
{
    using PS = MidiOutputHandler::PadState;
    Composition c = makeShow(2, 2, 3);
    c.fire(0, 1, 1, Snap::Off, true);                     // layer 0 plays deck B (1)'s (0, 1)
    auto lit = [](PS s) { return s == PS::Playing || s == PS::Triggered || s == PS::ActiveWithFx; };

    for (int row = 0; row < 2; ++row)                     // deck A (0) shown: no pad lights
        for (int col = 0; col < 3; ++col)
            CHECK_FALSE(lit(MidiOutputHandler::padStateFor(c, 0, row, col)));
    CHECK(MidiOutputHandler::padStateFor(c, 0, 0, 1) == PS::Loaded);
    CHECK(lit(MidiOutputHandler::padStateFor(c, 1, 0, 1)));   // deck B shown: (0, 1) lights
    CHECK_FALSE(lit(MidiOutputHandler::padStateFor(c, 1, 0, 0)));

    REQUIRE(c.retireOrEraseDeck(1));                      // its source deck retired: nothing lights anywhere
    for (int row = 0; row < 2; ++row)
        for (int col = 0; col < 3; ++col)
            CHECK_FALSE(lit(MidiOutputHandler::padStateFor(c, 0, row, col)));
}

TEST_CASE("T16 bindings: Selected on a layer playing a retired deck's clip fires nothing; ThisItem fires a clip from "
          "a non-shown deck into its row's layer and its velocity lands on that clip (bf9b, ruling-bf9b amendment 20)",
          "[show]")
{
    SECTION("Selected, its source retired")
    {
        Composition c = makeShow(2, 2, 3);
        c.fire(0, 1, 2, Snap::Off, true);
        REQUIRE(c.retireOrEraseDeck(1));
        const auto rt0 = c.layers[0].runtime(), rt1 = c.layers[1].runtime();
        Binding b;
        b.action = Binding::Action::TriggerClip;
        b.targetMode = Binding::TargetMode::Selected;
        const BindingTarget t = resolveBindingTarget(c, b);
        CHECK(t.layer == 0);
        CHECK(t.retired);                                 // handleBindingAction fires nothing
        CHECK(c.layers[0].runtime() == rt0);
        CHECK(c.layers[1].runtime() == rt1);
    }
    SECTION("ThisItem on a non-shown deck")
    {
        Composition c = makeShow(2, 2, 3);
        c.activeDeckIndex = 0;
        Clip* target = c.decks[1].getClip(1, 2);
        Binding b;
        b.action = Binding::Action::TriggerClip;
        b.targetMode = Binding::TargetMode::ThisItem;
        b.targetClipId = target->id;
        const BindingTarget t = resolveBindingTarget(c, b);
        CHECK(t.layer == 1);
        CHECK(t.column == 2);
        CHECK(t.deck == 1);
        CHECK_FALSE(t.retired);
        CHECK(t.velocityDeck(c) == 1);
        c.fire(t.layer, t.deck, t.column, Snap::Off, true);   // handleClipTrigger(layer, column, origin, deck)
        CHECK(c.layers[1].runtime().activeRef() == (ClipRef{ c.decks[1].id, 2 }));
        CHECK(c.playingClip(1) == target);

        MacroBank bank;
        ControlPath p;
        p.scope = ControlPath::Scope::Clip;
        p.deck = t.velocityDeck(c);
        p.layer = t.layer;
        p.col = t.column;
        p.control = "scalar";
        p.scalar = "opacity";
        const auto ref = resolveControl(c, bank, p);
        REQUIRE(ref.has_value());
        CHECK(ref->manual == &target->clipOpacity);       // the velocity write lands on that clip
    }
}

// ============================================================================================================
// Old files (plan-bf9b F6 / F7 / S2.3, ruling-bf9b amendments 8, 9): pre-bf9b compositions and deck files gave every
// deck its own layers (settings + clips); the converter (src/model/ShowMigration.h) is the only reader of their
// "persistent" / "globalTransitionSpeed" keys.
// ============================================================================================================
namespace
{
// A pre-bf9b row: Layer::toVar (it always wrote "type") + the row's "clips" (+ "persistent" when set).
juce::var legacyRow(const Layer& settings, const ClipRow& clips = {}, bool persistent = false)
{
    juce::var v = settings.toVar();
    auto* obj = v.getDynamicObject();
    obj->setProperty("clips", clips.toVar().getDynamicObject()->getProperty("clips"));
    if (persistent)
        obj->setProperty("persistent", true);
    return v;
}

juce::var deckVar(const std::string& name, uint32_t id, int cols, const std::vector<juce::var>& rows)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("name", juce::String(name));
    obj->setProperty("id", static_cast<int>(id));
    obj->setProperty("numColumns", cols);
    juce::Array<juce::var> arr;
    for (const auto& r : rows)
        arr.add(r);
    obj->setProperty("layers", arr);
    return juce::var(obj);
}

// A pre-bf9b composition: no top-level "layers"; the decks carry their layers; the old deck fade key.
juce::var legacyShow(const std::vector<juce::var>& decks, double fadeSeconds)
{
    Composition base;
    base.initDefault();
    juce::var v = base.toVar();
    auto* obj = v.getDynamicObject();
    obj->removeProperty("layers");
    juce::Array<juce::var> arr;
    for (const auto& d : decks)
        arr.add(d);
    obj->setProperty("decks", arr);
    obj->setProperty("globalTransitionSpeed", fadeSeconds);
    return v;
}

Layer settings(const std::string& name, uint32_t id, Layer::Type type, float opacity,
               Layer::MixMode blend = Layer::MixMode::Additive)
{
    Layer l;
    l.name = name;
    l.id = id;
    l.type = type;
    l.opacity = opacity;
    l.blendMode = blend;
    return l;
}

ClipRow rowOf(int cols, uint32_t firstId)
{
    ClipRow r;
    r.ensureColumns(cols);
    for (int c = 0; c < cols; ++c)
    {
        Clip clip;
        clip.id = firstId + static_cast<uint32_t>(c);
        clip.mediaType = Clip::MediaType::Image;
        r.clips[static_cast<size_t>(c)] = clip;
    }
    return r;
}

// M1's show: Deck 1 = 3 rows, Deck 2 = 4 rows (row 2 different: type, opacity, blend, 1 layer effect, 1 connection;
// row 4 only on Deck 2); Deck 1's "Layer 3" says "persistent": true; deck fade 1.2 s.
juce::var m1Show()
{
    const Layer a = settings("Layer 1", 0, Layer::Type::Opaque, 1.0f);
    const Layer b = settings("Layer 2", 1, Layer::Type::Transparent, 0.5f, Layer::MixMode::Screen);
    const Layer c3 = settings("Layer 3", 2, Layer::Type::Transparent, 1.0f);
    Layer b2 = settings("Layer 2", 1, Layer::Type::Mask, 0.9f, Layer::MixMode::Multiply);
    Clip::EffectSlot fx;
    fx.effectName = "ripple";
    b2.layerEffects.push_back(fx);
    b2.scalarConns[static_cast<size_t>(LayerScalar::Opacity)].source.kind = ConnSource::Kind::Signal;
    b2.scalarConns[static_cast<size_t>(LayerScalar::Opacity)].source.signalName = "rms";
    REQUIRE(b2.scalarConns[static_cast<size_t>(LayerScalar::Opacity)].isConnected());
    const Layer d4 = settings("Layer 4", 3, Layer::Type::Transparent, 0.7f);
    return legacyShow({ deckVar("Deck 1", 0, 2, { legacyRow(a, rowOf(2, 11)), legacyRow(b, rowOf(2, 21)),
                                                  legacyRow(c3, rowOf(2, 31), true) }),
                        deckVar("Deck 2", 1, 2, { legacyRow(a, rowOf(2, 111)), legacyRow(b2, rowOf(2, 121)),
                                                  legacyRow(c3, rowOf(2, 131)), legacyRow(d4, rowOf(2, 141)) }) },
                      1.2);
}

// Load Deck as MainComponent::loadDeck does it: the deck's clips (Deck::fromVar), validated, and its per-row legacy
// settings (ShowMigration::legacyRowSettings) into InsertDeckCmd.
std::unique_ptr<InsertDeckCmd> loadDeckCmd(Composition& c, const juce::var& v)
{
    Deck d;
    d.fromVar(v);
    REQUIRE(compload::validateDeck(d).empty());
    return std::make_unique<InsertDeckCmd>(resolverFor(c), passFence(), noMedia(), noDispose(), std::move(d),
                                           "Load Deck", ShowMigration::legacyRowSettings(v));
}
} // namespace

TEST_CASE("M1 an old 2-deck show converts to ONE shared stack: the first deck's settings win, the extra row adds a "
          "layer, every deck is padded, ONE note names the dropped set, the persistent layer and the fade (bf9b)",
          "[show][backcompat]")
{
    Composition c;
    c.fromVar(m1Show());
    REQUIRE(c.getNumLayers() == 4);
    CHECK(c.layers[0].type == Layer::Type::Opaque);
    CHECK(c.layers[1].type == Layer::Type::Transparent);          // Deck 1's row 2, not Deck 2's Mask (plan R5)
    CHECK(c.layers[1].opacity == Approx(0.5f));
    CHECK(c.layers[1].blendMode == Layer::MixMode::Screen);
    CHECK(c.layers[1].layerEffects.empty());
    CHECK(c.layers[2].name == "Layer 3");
    CHECK(c.layers[3].opacity == Approx(0.7f));                   // row 4 exists only on Deck 2: its settings
    REQUIRE(c.decks.size() == 2);
    for (const auto& d : c.decks)
        CHECK(d.getNumRows() == 4);
    CHECK(c.decks[0].getClip(0, 0)->id == 11u);                   // every clip kept in its row
    CHECK(c.decks[1].getClip(3, 1)->id == 142u);
    CHECK(c.decks[0].getClip(3, 0) == nullptr);                   // Deck 1 padded with an empty row
    const std::string& note = c.migrationNote;
    INFO("note: " << note);
    CHECK(note.rfind("old show converted:", 0) == 0);
    CHECK(note.find('\n') == std::string::npos);                  // ONE note
    CHECK(note.find("Deck 2 row 2: settings dropped; 1 layer effect, 1 connection dropped") != std::string::npos);
    CHECK(note.find("Deck 2 row 1") == std::string::npos);        // identical settings: not named
    CHECK(note.find("Deck 2 row 3") == std::string::npos);
    CHECK(note.find("'persistent' ignored on: Deck 1 / Layer 3") != std::string::npos);
    CHECK(note.find("deck fade 1.20 s dropped") != std::string::npos);
}

TEST_CASE("M2 an old show loads, saves, loads, saves: the two saves are equal; the saved form has top-level layers, "
          "rows with only clips, no persistent, no globalTransitionSpeed (bf9b, K7)", "[show][backcompat]")
{
    Composition first;
    first.fromVar(m1Show());
    REQUIRE_FALSE(first.migrationNote.empty());
    const juce::String save1 = juce::JSON::toString(first.toVar());
    Composition second;
    second.fromVar(juce::JSON::parse(save1));
    CHECK(second.migrationNote.empty());                          // a bf9b file: nothing converted
    const juce::String save2 = juce::JSON::toString(second.toVar());
    CHECK(save1 == save2);
    CHECK_FALSE(save1.contains("persistent"));
    CHECK_FALSE(save1.contains("globalTransitionSpeed"));
    const juce::var v = juce::JSON::parse(save1);
    REQUIRE(v.getProperty("layers", juce::var()).isArray());
    CHECK(v.getProperty("layers", juce::var()).getArray()->size() == 4);
    for (const auto& d : *v.getProperty("decks", juce::var()).getArray())
        for (const auto& row : *d.getProperty("layers", juce::var()).getArray())
        {
            auto* obj = row.getDynamicObject();
            REQUIRE(obj != nullptr);
            CHECK(obj->getProperties().size() == 1);
            CHECK(obj->hasProperty("clips"));
        }
}

TEST_CASE("M3 Load Deck: an old 5-row deck into a 3-layer show adds 2 layers with the file's settings, a new-format deck "
          "adds default layers, other decks are padded, undo removes the deck and the added layers (bf9b, F6)",
          "[show][backcompat]")
{
    Composition c = makeShow(2, 3, 2);
    std::vector<juce::String> keepBefore;
    for (const auto& l : c.layers)
        keepBefore.push_back(ShowMigration::settingsKey(l));

    SECTION("an old-format deck")
    {
        std::vector<juce::var> rows;
        for (int r = 0; r < 3; ++r)
            rows.push_back(legacyRow(settings("Old " + std::to_string(r), static_cast<uint32_t>(r), Layer::Type::Mask, 0.1f),
                                     rowOf(2, 500u + 10u * static_cast<uint32_t>(r))));
        rows.push_back(legacyRow(settings("Old 3", 3, Layer::Type::Transparent, 0.33f), rowOf(2, 530)));
        rows.push_back(legacyRow(settings("Old 4", 4, Layer::Type::FXOnly, 0.44f), rowOf(2, 540)));
        auto cmd = loadDeckCmd(c, deckVar("Old5", 77, 2, rows));
        cmd->execute();
        REQUIRE(c.getNumLayers() == 5);
        CHECK(cmd->addedLayerCount() == 2);
        CHECK(c.layers[3].opacity == Approx(0.33f));
        CHECK(c.layers[4].type == Layer::Type::FXOnly);
        for (int i = 0; i < 3; ++i)
            CHECK(ShowMigration::settingsKey(c.layers[static_cast<size_t>(i)]) == keepBefore[static_cast<size_t>(i)]);
        CHECK(rowsEqualLayers(c));
        CHECK(c.decks.back().getClip(4, 1)->id == 541u);
        cmd->undo();
        CHECK(c.decks.size() == 2);
        CHECK(c.getNumLayers() == 3);
        CHECK(rowsEqualLayers(c));
    }
    SECTION("a new-format deck")
    {
        std::vector<juce::var> rows;
        for (int r = 0; r < 5; ++r)
            rows.push_back(rowOf(2, 600u + 10u * static_cast<uint32_t>(r)).toVar());
        auto cmd = loadDeckCmd(c, deckVar("New5", 78, 2, rows));
        cmd->execute();
        REQUIRE(c.getNumLayers() == 5);
        CHECK(c.layers[3].type == Layer::Type::Transparent);       // what Add Layer makes
        CHECK(c.layers[3].name == "Layer 4");
        CHECK(c.layers[4].name == "Layer 5");
        CHECK(rowsEqualLayers(c));
        cmd->undo();
        CHECK(c.getNumLayers() == 3);
        CHECK(rowsEqualLayers(c));
    }
}

TEST_CASE("M4 colliding layer ids across an old show's decks are re-minted unique; id 0 stays valid (bf9b, Pitfall 15)",
          "[show][backcompat]")
{
    const juce::var v = legacyShow(
        { deckVar("Deck 1", 0, 1, { legacyRow(settings("A", 0, Layer::Type::Opaque, 1.0f)),
                                    legacyRow(settings("B", 5, Layer::Type::Transparent, 1.0f)) }),
          deckVar("Deck 2", 1, 1, { legacyRow(settings("A", 0, Layer::Type::Opaque, 1.0f)),
                                    legacyRow(settings("B", 5, Layer::Type::Transparent, 1.0f)),
                                    legacyRow(settings("C", 0, Layer::Type::Transparent, 0.5f)),
                                    legacyRow(settings("D", 5, Layer::Type::Transparent, 0.6f)) }) },
        0.0);
    Composition c;
    c.fromVar(v);
    REQUIRE(c.getNumLayers() == 4);
    CHECK(c.layers[0].id == 0u);                                   // id 0 stays valid
    CHECK(c.layers[1].id == 5u);
    std::vector<uint32_t> ids;
    for (const auto& l : c.layers)
        ids.push_back(l.id);
    std::sort(ids.begin(), ids.end());
    CHECK(std::adjacent_find(ids.begin(), ids.end()) == ids.end());
    CHECK(c.makeLayer().id > ids.back());                          // the mint is past every id
}

TEST_CASE("M5 an old take (no shared layers) restores the shared layers from its captured active deck; a v2 take "
          "round-trips its layers (bf9b, plan F9)", "[show][backcompat]")
{
    Composition comp = makeShow(2, 2, 3);
    SECTION("v1: the captured ACTIVE deck's layers")
    {
        PerfState cp0;
        cp0.activeDeckIndex = 1;
        PerfState::DeckRuntime d0, d1;
        d0.deck = "Deck 1";
        d1.deck = "Deck 2";
        PerfState::LayerRuntime l;
        l.layer = "Layer 1"; l.opacity = 0.2f; d0.layers[0] = l;
        l.layer = "Layer 2"; l.opacity = 0.25f; d0.layers[1] = l;
        l.layer = "Layer 1"; l.opacity = 0.9f; d1.layers[0] = l;
        l.layer = "Layer 2"; l.opacity = 0.4f; d1.layers[1] = l;
        cp0.decks[0] = d0;
        cp0.decks[1] = d1;
        Take take;
        take.checkpoint0 = cp0;
        const auto p = compile(take, comp, DriveClock::Wall);
        std::map<int, float> opacity;
        for (const auto& e : p->preambleContinuous)
            if (e.key.scope == ControlPath::Scope::Layer && e.key.scalar == "opacity")
                opacity[e.target.layer] = e.v;
        REQUIRE(opacity.size() == 2);
        CHECK(opacity.at(0) == Approx(0.9f));                      // Deck 2's (the captured active deck)
        CHECK(opacity.at(1) == Approx(0.4f));
    }
    SECTION("v2: \"layers\" round-trips")
    {
        PerfState s;
        s.activeDeckIndex = 1;
        PerfState::LayerRuntime l;
        l.layer = "Layer 1";
        l.opacity = 0.3f;
        l.activeClipColumn = 2;
        l.activeDeck = 1;
        l.activeDeckName = "Deck 2";
        s.layers[0] = l;
        const juce::var v = s.toVar();
        CHECK_FALSE(v.getProperty("layers", juce::var()).isVoid());   // written as "layers"
        const PerfState back = PerfState::fromVar(juce::JSON::parse(juce::JSON::toString(v)));
        REQUIRE(back.layers.count(0) == 1);
        CHECK(back.layers.at(0).opacity == Approx(0.3f));
        CHECK(back.layers.at(0).activeClipColumn == 2);
        CHECK(back.layers.at(0).activeDeck == 1);
        CHECK(back.layers.at(0).activeDeckName == "Deck 2");
    }
}

TEST_CASE("M6 Load Deck reads a deck file per row: new rows add default layers, old rows add layers with their "
          "settings and leave the show's layers alone, a mixed file per row, {} rows are empty; undo removes it all "
          "(bf9b, ruling-bf9b amendment 8)", "[show][backcompat]")
{
    Composition c = makeShow(1, 3, 2);
    std::vector<juce::String> keepBefore;
    for (const auto& l : c.layers)
        keepBefore.push_back(ShowMigration::settingsKey(l));
    std::vector<juce::var> rows;
    for (int r = 0; r < 3; ++r)
        rows.push_back(legacyRow(settings("Old", static_cast<uint32_t>(r), Layer::Type::Mask, 0.2f), rowOf(2, 700)));

    SECTION("(c) a mixed file: row 4 old (its settings), row 5 new (a default layer)")
    {
        rows.push_back(legacyRow(settings("Old 4", 3, Layer::Type::ThreeD, 0.6f), rowOf(2, 730)));
        rows.push_back(rowOf(2, 740).toVar());
        auto cmd = loadDeckCmd(c, deckVar("Mixed", 9, 2, rows));
        cmd->execute();
        REQUIRE(c.getNumLayers() == 5);
        CHECK(c.layers[3].type == Layer::Type::ThreeD);
        CHECK(c.layers[3].opacity == Approx(0.6f));
        CHECK(c.layers[4].type == Layer::Type::Transparent);
        CHECK(c.layers[4].opacity == Approx(1.0f));
        for (int i = 0; i < 3; ++i)                                // (b) the show's layers untouched
            CHECK(ShowMigration::settingsKey(c.layers[static_cast<size_t>(i)]) == keepBefore[static_cast<size_t>(i)]);
        cmd->undo();                                               // (e)
        CHECK(c.decks.size() == 1);
        CHECK(c.getNumLayers() == 3);
        CHECK(rowsEqualLayers(c));
    }
    SECTION("(d) {} and {\"clips\": []} rows load as empty rows")
    {
        auto* emptyClips = new juce::DynamicObject();
        emptyClips->setProperty("clips", juce::Array<juce::var>());
        rows.push_back(juce::var(new juce::DynamicObject()));
        rows.push_back(juce::var(emptyClips));
        auto cmd = loadDeckCmd(c, deckVar("Empties", 10, 2, rows));
        cmd->execute();
        REQUIRE(c.getNumLayers() == 5);
        const Deck& d = c.decks.back();
        REQUIRE(d.getNumRows() == 5);
        for (int r = 3; r < 5; ++r)
            for (int col = 0; col < 2; ++col)
                CHECK(d.getClip(r, col) == nullptr);
        CHECK(c.layers[3].type == Layer::Type::Transparent);       // no settings: what Add Layer makes
        cmd->undo();
        CHECK(c.getNumLayers() == 3);
    }
}

TEST_CASE("M7 the note: none for a 1-deck old show without persistent; Boris's show shape gives exactly one row line "
          "(Deck 2 row 3) plus the fade (bf9b, ruling-bf9b amendment 9(b)(c))", "[show][backcompat]")
{
    SECTION("a 1-deck old show, no persistent: no note")
    {
        Composition c;
        c.fromVar(legacyShow({ deckVar("Deck 1", 0, 2, { legacyRow(settings("A", 0, Layer::Type::Opaque, 1.0f)),
                                                         legacyRow(settings("B", 1, Layer::Type::Transparent, 1.0f)) }) },
                             0.3));
        CHECK(c.migrationNote.empty());
        CHECK(c.getNumLayers() == 2);
    }
    SECTION("Boris's show (R-F13): 2 decks x 3 layers, row 3 differs only in blendMode, Deck 2's row 3 holds no clip")
    {
        const Layer a = settings("Layer 1", 0, Layer::Type::Opaque, 1.0f);
        const Layer b = settings("Layer 2", 1, Layer::Type::Transparent, 1.0f);
        const Layer c1 = settings("Layer 3", 2, Layer::Type::Transparent, 1.0f, static_cast<Layer::MixMode>(46));
        const Layer c2 = settings("Layer 3", 2, Layer::Type::Transparent, 1.0f, static_cast<Layer::MixMode>(1));
        Composition c;
        c.fromVar(legacyShow({ deckVar("Deck 1", 0, 2, { legacyRow(a, rowOf(2, 1)), legacyRow(b, rowOf(2, 11)),
                                                         legacyRow(c1, rowOf(2, 21)) }),
                               deckVar("Deck 2", 1, 2, { legacyRow(a, rowOf(2, 31)), legacyRow(b, rowOf(2, 41)),
                                                         legacyRow(c2) }) },
                             0.3));
        const std::string& note = c.migrationNote;
        INFO("note: " << note);
        size_t rowLines = 0;
        for (size_t at = note.find(" row "); at != std::string::npos; at = note.find(" row ", at + 1))
            ++rowLines;
        CHECK(rowLines == 1);
        CHECK(note.find("Deck 2 row 3: settings dropped") != std::string::npos);
        CHECK(note.find("deck fade 0.30 s dropped") != std::string::npos);
        CHECK(note.find("persistent") == std::string::npos);
        CHECK(c.layers[2].blendMode == static_cast<Layer::MixMode>(46));   // Deck 1's look kept
    }
}
