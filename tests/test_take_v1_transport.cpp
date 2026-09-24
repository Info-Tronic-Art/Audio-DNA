// test_take_v1_transport -- s-rta-0923 Lane R28, review-fix L1: the v1
// TransportChange bridge. play/pause/stop are faithful (v2's Comp/"audio"
// control is action-valued and carries no value); speed/reverse have no v2
// equivalent and are dropped, counted in LoadStats::v1Dropped (D12: never
// silently dropped), never emitted as an `audio` point.
#include <catch2/catch_test_macros.hpp>
#include "model/Composition.h"
#include "recording/Take.h"
#include "recording/Program.h"
#include <algorithm>

TEST_CASE("v1 TransportChange bridges play/pause/stop faithfully and counts speed/reverse as dropped", "[take][v1][transport]")
{
    juce::File fixture = juce::File(TEST_FIXTURES_DIR).getChildFile("take_v1_transport.json");
    REQUIRE(fixture.existsAsFile());

    LoadStats stats;
    auto take = Take::load(fixture, stats);
    REQUIRE(take.has_value());
    REQUIRE_FALSE(stats.refused);
    REQUIRE(stats.wasV1);

    ControlPath audioKey;
    audioKey.scope = ControlPath::Scope::Comp;
    audioKey.control = "audio";
    REQUIRE(take->lanes.count(audioKey) == 1);

    const auto& audioLane = take->lanes.at(audioKey);
    REQUIRE(audioLane.points.size() == 3);

    REQUIRE(audioLane.points[0].action == "play");
    REQUIRE(audioLane.points[0].s.t == 1.0);
    REQUIRE(audioLane.points[0].v == 0);

    REQUIRE(audioLane.points[1].action == "pause");
    REQUIRE(audioLane.points[1].s.t == 3.0);
    REQUIRE(audioLane.points[1].v == 0);

    REQUIRE(audioLane.points[2].action == "stop");
    REQUIRE(audioLane.points[2].s.t == 4.0);
    REQUIRE(audioLane.points[2].v == 0);

    REQUIRE(stats.v1Dropped.size() == 2);
    REQUIRE(stats.v1Dropped.count("TransportChange:speed") == 1);
    REQUIRE(stats.v1Dropped.at("TransportChange:speed") == 1);
    REQUIRE(stats.v1Dropped.count("TransportChange:reverse") == 1);
    REQUIRE(stats.v1Dropped.at("TransportChange:reverse") == 1);

    // 1 ClipTrigger + 3 faithful TransportChange points = 4 seqs minted;
    // the 2 dropped events mint none (L1: seq minted AFTER the switch).
    REQUIRE(take->nextSeq == 5);
}

TEST_CASE("compile of a bridged v1 take dispatches only faithful audio points", "[take][v1][transport]")
{
    juce::File fixture = juce::File(TEST_FIXTURES_DIR).getChildFile("take_v1_transport.json");
    REQUIRE(fixture.existsAsFile());

    LoadStats stats;
    auto take = Take::load(fixture, stats);
    REQUIRE(take.has_value());

    Composition comp;
    comp.initDefault();

    auto program = compile(*take, comp, DriveClock::Wall);
    REQUIRE(program->discrete.size() == 4);

    int audioPoints = 0;
    for (const auto& fired : program->discrete)
    {
        if (fired.key.control != "audio") continue;
        ++audioPoints;
        REQUIRE(fired.p.v == 0);
        REQUIRE((fired.p.action == "play" || fired.p.action == "pause" || fired.p.action == "stop"));
        REQUIRE(fired.p.action != "speed");
        REQUIRE(fired.p.action != "reverse");
    }
    REQUIRE(audioPoints == 3);

    REQUIRE(program->report.unresolved.empty());
    REQUIRE(program->report.resolvedCount == 1);
    // The v1 activeClip lane is deck-relative with an empty layerName
    // against Composition::initDefault()'s named "Layer 1" -> PositionOnly.
    REQUIRE(program->report.reboundByPosition.size() == 1);
}
