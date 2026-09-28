// test_output_plan -- s-rta-0927 outputs-c3 = plan5 slice C3 (.harmony/.reports/s-rta-0926b/plan5-final.md sections
// 5, 9, 10.1): which connected display is which across a display change (matchDisplay), what one reconcile does
// (diffOutputs: unplug, replug, move, MODE change -- Harmony ruling (a) on the C2 review), and the settings.json
// "outputs" shape. Pure/headless: the code under test is src/output/OutputTargets.cpp, exactly what
// OutputManager::reconcile() and "Restore Last Outputs" run. The real cable pull is Boris check 4.
#include <catch2/catch_test_macros.hpp>
#include "output/OutputTargets.h"

using output::DisplayInfo;
using output::MatchRule;
using output::OutputDiff;

namespace
{
const DisplayInfo kLaptop { 0, 0, 1728, 1117, 2.0, true };              // main, as [NSScreen screens] lists it first
const DisplayInfo kProjector { 1728, 0, 1920, 1080, 1.0, false };
const DisplayInfo kProjector2 { 3648, 0, 1920, 1080, 1.0, false };      // an identical projector further right
const DisplayInfo kProjectorMoved { -1920, 0, 1920, 1080, 1.0, false }; // the projector dragged to the left
const DisplayInfo kProjector720 { 1728, 0, 1280, 720, 1.0, false };     // the projector switched to 1280x720
const DisplayInfo kTv4k { 1728, 0, 3840, 2160, 1.0, false };            // another display, same top-left

bool hasPair(const std::vector<OutputDiff::Pair>& v, int from, int display)
{
    for (const auto& p : v)
        if (p.from == from && p.display == display)
            return true;
    return false;
}
} // namespace

TEST_CASE("matchDisplay: exact, moved, main, duplicates, none", "[output_plan]")
{
    SECTION("exact fingerprint")
    {
        CHECK(output::matchDisplay(kProjector, { kLaptop, kProjector }, {}) == 1);
    }
    SECTION("same size and scale at another position = the same display, moved")
    {
        CHECK(output::matchDisplay(kProjector, { kLaptop, kProjectorMoved }, {}) == 1);
    }
    SECTION("the main display falls back to whatever display is main now")
    {
        const DisplayInfo laptopOtherMode { 0, 0, 1512, 982, 2.0, true };
        CHECK(output::matchDisplay(kLaptop, { laptopOtherMode, kProjector }, {}) == 0);
    }
    SECTION("two identical displays: position wins, then the first unused one")
    {
        CHECK(output::matchDisplay(kProjector2, { kLaptop, kProjector, kProjector2 }, {}) == 2);   // exact beats moved
        CHECK(output::matchDisplay(kProjector, { kLaptop, kProjector, kProjector2 }, {}) == 1);
        const DisplayInfo elsewhere { 9000, 0, 1920, 1080, 1.0, false };
        CHECK(output::matchDisplay(elsewhere, { kLaptop, kProjector, kProjector2 }, {}) == 1);     // first unused
        CHECK(output::matchDisplay(elsewhere, { kLaptop, kProjector, kProjector2 }, { false, true }) == 2);
    }
    SECTION("used displays are skipped, even on an exact fingerprint")
    {
        CHECK_FALSE(output::matchDisplay(kProjector, { kLaptop, kProjector }, { false, true }).has_value());
    }
    SECTION("no match: the display is gone")
    {
        CHECK_FALSE(output::matchDisplay(kProjector, { kLaptop }, {}).has_value());
        CHECK_FALSE(output::matchDisplay(kProjector, {}, {}).has_value());
    }
    SECTION("a MODE change (same top-left, other size) is followed by a live output only")
    {
        CHECK(output::matchDisplay(kProjector, { kLaptop, kProjector720 }, {}, MatchRule::Track) == 1);
        CHECK_FALSE(output::matchDisplay(kProjector, { kLaptop, kProjector720 }, {}, MatchRule::Reopen).has_value());
    }
}

TEST_CASE("diffOutputs: what one reconcile does", "[output_plan]")
{
    SECTION("nothing changed -> nothing to do")
    {
        const std::vector<DisplayInfo> now { kLaptop, kProjector };
        CHECK(output::diffOutputs({ kProjector }, {}, now, now).empty());
        CHECK(output::diffOutputs({}, {}, now, now).empty());
    }
    SECTION("unplugged: the projector's output closes (its target is kept by the caller as interrupted)")
    {
        const auto diff = output::diffOutputs({ kLaptop, kProjector }, {}, { kLaptop }, { kLaptop, kProjector });
        CHECK(diff.toClose == std::vector<int> { 1 });
        CHECK(diff.toRebound.empty());
        CHECK(diff.toOpen.empty());
    }
    SECTION("replugged: the interrupted target reopens on its display (plan5 Q6)")
    {
        const auto diff = output::diffOutputs({ kLaptop }, { kProjector }, { kLaptop, kProjector }, { kLaptop });
        CHECK(diff.toClose.empty());
        CHECK(diff.toRebound.empty());
        REQUIRE(diff.toOpen.size() == 1);
        CHECK(hasPair(diff.toOpen, 0, 1));
    }
    SECTION("moved in the arrangement: the live output follows (rebound, same live index)")
    {
        const auto diff = output::diffOutputs({ kProjector }, {}, { kLaptop, kProjectorMoved }, { kLaptop, kProjector });
        CHECK(diff.toClose.empty());
        REQUIRE(diff.toRebound.size() == 1);
        CHECK(hasPair(diff.toRebound, 0, 1));
    }
    SECTION("MODE change (resolution/scale): the live output follows its display (Harmony ruling (a))")
    {
        const auto diff = output::diffOutputs({ kProjector }, {}, { kLaptop, kProjector720 }, { kLaptop, kProjector });
        CHECK(diff.toClose.empty());
        CHECK(hasPair(diff.toRebound, 0, 1));
        const DisplayInfo laptopMoreSpace { 0, 0, 2056, 1329, 2.0, true };   // "More Space" on the main display
        const auto diffMain = output::diffOutputs({ kLaptop }, {}, { laptopMoreSpace }, { kLaptop });
        CHECK(diffMain.toClose.empty());
        CHECK(hasPair(diffMain.toRebound, 0, 0));
    }
    SECTION("MODE change with an identical, untouched projector beside it: the output does NOT jump to it")
    {
        const auto diff = output::diffOutputs({ kProjector }, {}, { kLaptop, kProjector720, kProjector2 },
                                              { kLaptop, kProjector, kProjector2 });
        CHECK(diff.toClose.empty());
        REQUIRE(diff.toRebound.size() == 1);
        CHECK(hasPair(diff.toRebound, 0, 1));   // display 1 = the projector in its new mode, never 2
    }
    SECTION("the main display switched (menu bar dragged): both live outputs stay on their own displays")
    {
        const DisplayInfo laptopNotMain { -1728, 0, 1728, 1117, 2.0, false };
        const DisplayInfo projectorMain { 0, 0, 1920, 1080, 1.0, true };
        const auto diff = output::diffOutputs({ kLaptop, kProjector }, {}, { projectorMain, laptopNotMain },
                                              { kLaptop, kProjector });
        CHECK(diff.toClose.empty());
        CHECK(hasPair(diff.toRebound, 0, 1));   // the laptop output -> the laptop (now not main)
        CHECK(hasPair(diff.toRebound, 1, 0));   // the projector output -> the projector (now main)
    }
    SECTION("an interrupted target never reopens on an untouched identical display when something else changes")
    {
        const DisplayInfo laptopMoreSpace { 0, 0, 2056, 1329, 2.0, true };
        const auto diff = output::diffOutputs({}, { kProjector }, { laptopMoreSpace, kProjector2 },
                                              { kLaptop, kProjector2 });
        CHECK(diff.empty());
    }
    SECTION("an interrupted target returning on a new same-size display at another position reopens there")
    {
        const auto diff = output::diffOutputs({}, { kProjector }, { kLaptop, kProjectorMoved }, { kLaptop });
        CHECK(hasPair(diff.toOpen, 0, 1));
    }
    SECTION("a target already live is not reopened (its display is used)")
    {
        const auto diff = output::diffOutputs({ kProjector }, { kProjector }, { kLaptop, kProjector },
                                              { kLaptop });
        CHECK(diff.toOpen.empty());
        CHECK(diff.toClose.empty());
    }
    SECTION("an unrelated new display opens nothing")
    {
        const auto diff = output::diffOutputs({ kLaptop }, {}, { kLaptop, kProjector }, { kLaptop });
        CHECK(diff.empty());
        const auto diff2 = output::diffOutputs({}, { kProjector }, { kLaptop, kTv4k }, { kLaptop });
        CHECK(diff2.empty());   // same top-left, other mode: an interrupted target does not reopen there
    }
    SECTION("zero displays (a transient empty list): every live output closes; nothing opens")
    {
        const auto diff = output::diffOutputs({ kLaptop, kProjector }, {}, {}, { kLaptop, kProjector });
        CHECK(diff.toClose == std::vector<int> { 0, 1 });
        CHECK(diff.toOpen.empty());
    }
}

TEST_CASE("sameTargets is order-insensitive (the settings file is written on a change only)", "[output_plan]")
{
    CHECK(output::sameTargets({ kLaptop, kProjector }, { kProjector, kLaptop }));
    CHECK(output::sameTargets({}, {}));
    CHECK_FALSE(output::sameTargets({ kLaptop }, { kLaptop, kProjector }));
    CHECK_FALSE(output::sameTargets({ kProjector }, { kProjector720 }));
    CHECK_FALSE(output::sameTargets({ kProjector, kProjector }, { kProjector, kProjector2 }));
}

TEST_CASE("the wanted set keeps every saved target not yet restored (settings.json \"outputs\")", "[output_plan]")
{
    // What OutputManager::persistWanted() writes: live + interrupted + the saved set Restore has not opened yet.
    // AppSettings::update() replaces the whole "outputs" key, so a saved target left out here is gone from disk.
    SECTION("a partial Restore: the connected display is live, the missing one stays saved")
    {
        // saved {laptop, projector}, the projector unplugged: Restore opens the laptop only
        CHECK(output::sameTargets(output::wantedSet({ kLaptop }, {}, { kProjector }), { kLaptop, kProjector }));
    }
    SECTION("an unrelated output change before any Restore keeps the saved set")
    {
        CHECK(output::sameTargets(output::wantedSet({ kProjector2 }, {}, { kLaptop, kProjector }),
                                  { kProjector2, kLaptop, kProjector }));
    }
    SECTION("All Outputs Off (nothing live or interrupted) keeps the saved set not yet restored")
    {
        CHECK(output::sameTargets(output::wantedSet({}, {}, { kProjector }), { kProjector }));
    }
    SECTION("live, interrupted and saved: each target once")
    {
        CHECK(output::sameTargets(output::wantedSet({ kLaptop }, { kProjector, kLaptop }, { kProjector, kProjector2 }),
                                  { kLaptop, kProjector, kProjector2 }));
    }
    SECTION("nothing live, interrupted or saved: empty")
    {
        CHECK(output::wantedSet({}, {}, {}).empty());
    }
}

TEST_CASE("settings.json \"outputs\": wantedToVar / wantedFromVar", "[output_plan]")
{
    SECTION("round trip, version present")
    {
        const std::vector<DisplayInfo> set { kLaptop, kProjector };
        const auto v = output::wantedToVar(set);
        REQUIRE(v.getDynamicObject() != nullptr);
        CHECK(static_cast<int>(v["version"]) == 1);
        CHECK(v["targets"].size() == 2);
        const auto back = output::wantedFromVar(juce::JSON::parse(juce::JSON::toString(v)));   // through text
        REQUIRE(back.size() == 2);
        CHECK(back[0] == kLaptop);
        CHECK(back[1] == kProjector);
    }
    SECTION("unknown keys are ignored (later fields: crop, windowed bounds, per-output vsync)")
    {
        const auto v = juce::JSON::parse(R"({"version": 2, "future": true, "targets": [
            {"x": 1728, "y": 0, "w": 1920, "h": 1080, "scale": 1.0, "main": false, "crop": [0, 0, 1, 1]}]})");
        const auto back = output::wantedFromVar(v);
        REQUIRE(back.size() == 1);
        CHECK(back[0] == kProjector);
    }
    SECTION("malformed entries are skipped; anything else reads as no targets")
    {
        const auto v = juce::JSON::parse(R"({"targets": [{"x": 0, "y": 0, "w": 0, "h": 1080}, 7, "x",
            {"x": 0, "y": 0, "w": 1728, "h": 1117, "scale": 2.0, "main": true}]})");
        const auto back = output::wantedFromVar(v);
        REQUIRE(back.size() == 1);
        CHECK(back[0] == kLaptop);
        CHECK(output::wantedFromVar(juce::var()).empty());
        CHECK(output::wantedFromVar(juce::JSON::parse("[1, 2]")).empty());
        CHECK(output::wantedFromVar(juce::JSON::parse(R"({"targets": 3})")).empty());
    }
    SECTION("an empty set is an empty targets array")
    {
        const auto v = output::wantedToVar({});
        CHECK(v["targets"].isArray());
        CHECK(v["targets"].size() == 0);
        CHECK(output::wantedFromVar(v).empty());
    }
}
