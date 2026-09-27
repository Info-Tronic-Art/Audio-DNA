// test_routine_deck_view -- s-rta-0927 routine display, slice A (plan-routine-display-A.md 3.5 / 5.1): the
// deck's ROUTINES row, the layer strips' routine bands and the pad settings menu, derived PURELY from
// RoutineEngine::Status (src/ui/RoutineDeckView.h). juce_core only: no GUI, no engine, no clock.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "ui/RoutineDeckView.h"

using Catch::Approx;
using State = RoutineDeckView::State;
using M = RoutineDeckView::PadMenu;

namespace
{
    const std::vector<juce::String> kDecks{ "A", "B" };
    const std::vector<juce::String> kLayers{ "Layer 1", "Layer 2", "Layer 3" };
    const juce::String kDot = juce::String::fromUTF8("\xc2\xb7");

    RoutineEngine::Status::Slot idle(int slot, const std::string& name, double lengthBeats = 32.0)
    {
        RoutineEngine::Status::Slot s;
        s.slot = slot;
        s.uuid = "u" + std::to_string(slot);
        s.name = name;
        s.state = "idle";
        s.lengthBeats = lengthBeats;
        s.quantize = "bar";
        return s;
    }

    RoutineEngine::Status::Slot live(int slot, const std::string& name, const std::string& state, int deck,
                                     std::vector<int> layers, uint32_t fireSeq, double position = 0.0)
    {
        auto s = idle(slot, name);
        s.state = state;
        s.deck = deck;
        s.layers = std::move(layers);
        s.fireSeq = fireSeq;
        s.position = position;
        if (state == "pending")
            s.startsOn = "bar";
        return s;
    }
}

TEST_CASE("RoutineDeckView empty pad: a dim number, no name, the tooltip names the Record tab", "[routine][deckview]")
{
    RoutineEngine::Status s;
    s.slots[1] = idle(1, "Build");   // one saved routine elsewhere: no "save one" corner note
    const auto v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    CHECK(v.pads[0].number == 1);
    CHECK(v.pads[0].state == State::Empty);
    CHECK(v.pads[0].name.isEmpty());
    CHECK(v.pads[0].tooltip == "No routine is saved on this pad yet. Save one in the Record tab.");
    CHECK(v.cornerNote.isEmpty());
}

TEST_CASE("RoutineDeckView idle pad with LOOP: named, Press to play", "[routine][deckview]")
{
    RoutineEngine::Status s;
    s.slots[1] = idle(1, "Build", 16.0);
    s.slots[1].loop = true;
    const auto v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    const auto& p = v.pads[1];
    CHECK(p.number == 2);
    CHECK(p.name == "Build");
    CHECK(p.state == State::Idle);
    CHECK(p.loop);
    CHECK(p.onShownDeck);
    CHECK_FALSE(p.warning);
    CHECK(p.tooltip == "Press to play. Right-click for settings.");
    CHECK(v.bandsByLayer.empty());
}

TEST_CASE("RoutineDeckView waiting pad: Waiting, no progress, dim bands on exactly its layers", "[routine][deckview]")
{
    RoutineEngine::Status s;
    s.slots[0] = live(0, "Drop", "pending", 0, { 0, 2 }, 1);
    const auto v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    const auto& p = v.pads[0];
    CHECK(p.state == State::Waiting);
    CHECK(p.progress01 == 0.0f);
    CHECK(p.bar == 0);
    CHECK(p.tooltip == "Starting on the next bar.");
    REQUIRE(v.bandsByLayer.count(0) == 1);
    REQUIRE(v.bandsByLayer.count(2) == 1);
    CHECK(v.bandsByLayer.count(1) == 0);
    CHECK(v.bandsByLayer.at(0).size() == 1);
    CHECK(v.bandsByLayer.at(0)[0].state == State::Waiting);
    CHECK(v.bandsByLayer.at(0)[0].name == "Drop");
    CHECK(v.bandsByLayer.at(0)[0].slot == 0);
    CHECK(v.bandsByLayer.at(2)[0].state == State::Waiting);

    SECTION("the tooltip names the grid it will start on")
    {
        auto t = s;
        t.slots[0].startsOn = "beat";
        CHECK(deriveRoutineDeckView(t, 0, kDecks, kLayers).pads[0].tooltip == "Starting on the next beat.");
        t.slots[0].startsOn = "2bar";
        CHECK(deriveRoutineDeckView(t, 0, kDecks, kLayers).pads[0].tooltip == "Starting on the next two-bar line.");
        t.slots[0].startsOn = "4bar";
        CHECK(deriveRoutineDeckView(t, 0, kDecks, kLayers).pads[0].tooltip == "Starting on the next four-bar line.");
        t.slots[0].startsOn = "now";
        CHECK(deriveRoutineDeckView(t, 0, kDecks, kLayers).pads[0].tooltip == "Starting now.");
    }
}

TEST_CASE("RoutineDeckView playing pad: bar 5 of 8, progress, layer names, bands on exactly its layers", "[routine][deckview]")
{
    RoutineEngine::Status s;
    s.slots[0] = live(0, "Drop", "running", 0, { 0, 2 }, 1, 17.0);
    const auto v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    const auto& p = v.pads[0];
    CHECK(p.state == State::Playing);
    CHECK(p.bar == 5);
    CHECK(p.barsTotal == 8);
    CHECK(p.progress01 == Approx(0.53125f));
    CHECK(p.tooltip == "Playing on Layer 1, Layer 3. Press to restart from the top.");
    REQUIRE(v.bandsByLayer.size() == 2);
    CHECK(v.bandsByLayer.at(0)[0].state == State::Playing);
    CHECK(v.bandsByLayer.at(0)[0].progress01 == Approx(0.53125f));
    CHECK(v.bandsByLayer.at(2)[0].name == "Drop");
    CHECK(v.cornerNote.isEmpty());

    SECTION("the last beat of the last bar is still bar 8 of 8")
    {
        auto t = s;
        t.slots[0].position = 31.9;
        CHECK(deriveRoutineDeckView(t, 0, kDecks, kLayers).pads[0].bar == 8);
    }
}

TEST_CASE("RoutineDeckView off-deck: dimmed pad, no bands, the corner names the deck", "[routine][deckview]")
{
    RoutineEngine::Status s;
    s.slots[0] = live(0, "Drop", "running", 0, { 0, 2 }, 1, 4.0);
    const auto v = deriveRoutineDeckView(s, 1, kDecks, kLayers);
    CHECK_FALSE(v.pads[0].onShownDeck);
    CHECK(v.pads[0].state == State::Playing);
    CHECK(v.bandsByLayer.empty());
    CHECK(v.cornerNote == kDot + " Drop on A");
    CHECK(v.pads[0].tooltip == "Playing on A. Switch decks to see its layers.");

    SECTION("two routines on one other deck are named together")
    {
        auto t = s;
        t.slots[1] = live(1, "Build", "running", 0, { 0 }, 2, 1.0);
        CHECK(deriveRoutineDeckView(t, 1, kDecks, kLayers).cornerNote == kDot + " Drop, Build on A");
    }
}

TEST_CASE("RoutineDeckView two on one layer: newest first; bandsToDraw keeps two and folds the rest into +N", "[routine][deckview]")
{
    RoutineEngine::Status s;
    s.slots[0] = live(0, "Drop", "running", 0, { 0, 2 }, 1, 8.0);
    s.slots[1] = live(1, "Build", "running", 0, { 0 }, 3, 1.0);
    const auto v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    REQUIRE(v.bandsByLayer.at(0).size() == 2);
    CHECK(v.bandsByLayer.at(0)[0].name == "Build");   // fireSeq 3 above 1
    CHECK(v.bandsByLayer.at(0)[1].name == "Drop");
    REQUIRE(v.bandsByLayer.at(2).size() == 1);
    CHECK(v.bandsByLayer.at(2)[0].name == "Drop");

    auto three = s;
    three.slots[2] = live(2, "Wash", "running", 0, { 0 }, 2, 1.0);
    const auto v3 = deriveRoutineDeckView(three, 0, kDecks, kLayers);
    REQUIRE(v3.bandsByLayer.at(0).size() == 3);
    const auto drawn = bandsToDraw(v3.bandsByLayer.at(0));
    REQUIRE(drawn.size() == 2);
    CHECK(drawn[0].name == "Build");
    CHECK(drawn[1].name == "Wash +1");
    CHECK(bandsToDraw(v.bandsByLayer.at(0)).size() == 2);
    CHECK(bandsToDraw(v.bandsByLayer.at(0))[1].name == "Drop");
}

TEST_CASE("RoutineDeckView warning: the red ! and a tooltip composed from the counts", "[routine][deckview]")
{
    RoutineEngine::Status s;
    s.slots[2] = idle(2, "Wash", 4.0);
    s.slots[2].preambleUnresolved = 2;
    s.slots[2].unresolved = 1;
    s.slots[2].skipped = 3;
    auto v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    CHECK(v.pads[2].warning);
    CHECK(v.pads[2].tooltip == "2 settings could not be restored; 1 timeline points at a layer or clip that no longer "
                               "exists; 3 clip changes could not be made. Press to play. Right-click for settings.");

    s.slots[2].preambleUnresolved = 1;
    s.slots[2].unresolved = 2;
    s.slots[2].skipped = 1;
    v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    CHECK(v.pads[2].tooltip == "1 setting could not be restored; 2 timelines point at a layer or clip that no longer "
                               "exists; 1 clip change could not be made. Press to play. Right-click for settings.");

    s.slots[2].unresolved = 0;
    s.slots[2].skipped = 0;
    v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    CHECK(v.pads[2].tooltip == "1 setting could not be restored. Press to play. Right-click for settings.");

    s.slots[2].preambleUnresolved = 0;
    v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    CHECK_FALSE(v.pads[2].warning);
    CHECK(v.pads[2].tooltip == "Press to play. Right-click for settings.");
}

TEST_CASE("RoutineDeckView corner note when every pad is empty", "[routine][deckview]")
{
    RoutineEngine::Status s;
    const auto v = deriveRoutineDeckView(s, 0, kDecks, kLayers);
    CHECK(v.cornerNote == kDot + " Save one in the Record tab");
    for (const auto& p : v.pads)
        CHECK(p.state == State::Empty);
}

TEST_CASE("RoutineDeckView padMenu: order, ticks, Remove from layers only while live, Delete last after a separator", "[routine][deckview][menu]")
{
    RoutineDeckView::Pad pad;
    pad.state = State::Idle;
    pad.loop = true;
    pad.restoreFirst = true;
    pad.startEase = true;
    pad.quantize = "bar";
    auto items = padMenu(pad);
    const std::vector<M> order{ M::Loop, M::Once, M::RestoreFirst, M::StartFromNow, M::StartEase, M::StartJump,
                                M::QuantizeOff, M::QuantizeBeat, M::QuantizeBar, M::QuantizeTwoBar,
                                M::QuantizeFourBar, M::Rename, M::RemoveFromLayers, M::DeleteRoutine };
    REQUIRE(items.size() == order.size());
    for (size_t i = 0; i < order.size(); ++i)
        CHECK(items[i].id == order[i]);
    auto find = [&items](M id) { for (const auto& it : items) if (it.id == id) return it; FAIL("missing"); return items[0]; };
    CHECK(find(M::Loop).ticked);
    CHECK_FALSE(find(M::Once).ticked);
    CHECK(find(M::RestoreFirst).ticked);
    CHECK(find(M::StartEase).ticked);
    CHECK(find(M::StartEase).label == "Start: Ease");
    CHECK_FALSE(find(M::StartJump).ticked);
    CHECK(find(M::StartJump).label == "Start: Jump");
    CHECK(find(M::QuantizeBar).ticked);
    CHECK(find(M::QuantizeBar).inQuantizeSubmenu);
    CHECK_FALSE(find(M::QuantizeBeat).ticked);
    CHECK_FALSE(find(M::Rename).inQuantizeSubmenu);
    CHECK_FALSE(find(M::RemoveFromLayers).enabled);   // idle: nothing to remove
    CHECK(items.back().id == M::DeleteRoutine);
    CHECK(items.back().separatorBefore);
    CHECK(items.back().label == "Delete routine");
    for (const auto& it : items)
        CHECK_FALSE(it.label.containsIgnoreCase("stop"));   // no Stop row anywhere

    pad.startEase = false;
    pad.loop = false;
    items = padMenu(pad);
    CHECK_FALSE(find(M::StartEase).ticked);
    CHECK(find(M::StartJump).ticked);
    CHECK(find(M::Once).ticked);

    pad.state = State::Waiting;
    items = padMenu(pad);
    CHECK(find(M::RemoveFromLayers).enabled);
    pad.state = State::Playing;
    items = padMenu(pad);
    CHECK(find(M::RemoveFromLayers).enabled);
}

TEST_CASE("RoutineDeckView settingsChangeFor: each row writes one setting", "[routine][deckview][menu]")
{
    CHECK(settingsChangeFor(M::Loop).loop == std::optional<bool>(true));
    CHECK(settingsChangeFor(M::Once).loop == std::optional<bool>(false));
    CHECK(settingsChangeFor(M::RestoreFirst).restoreState == std::optional<bool>(true));
    CHECK(settingsChangeFor(M::StartFromNow).restoreState == std::optional<bool>(false));
    CHECK(settingsChangeFor(M::StartEase).restoreStyle == "ease");
    CHECK(settingsChangeFor(M::StartJump).restoreStyle == "jump");
    CHECK(settingsChangeFor(M::QuantizeOff).quantize == "off");
    CHECK(settingsChangeFor(M::QuantizeTwoBar).quantize == "2bar");
    CHECK(settingsChangeFor(M::QuantizeFourBar).quantize == "4bar");
    const auto none = settingsChangeFor(M::DeleteRoutine);
    CHECK_FALSE(none.loop.has_value());
    CHECK_FALSE(none.restoreState.has_value());
    CHECK(none.restoreStyle.isEmpty());
    CHECK(none.quantize.isEmpty());
    CHECK(settingsChangeFor(M::StartJump).quantize.isEmpty());
}
