// s-rta-0923 lane 3, Lane C0 scaffold: the picker -> ConnSource translation,
// fail-first. .harmony/specs/s-rta-0923-lane3-plan.md section 4.3 and
// section 5 (Lane C4), amended by the critic pass folded into this dispatch
// (finding #5). Every case below is written NOW against Lane C0's stub
// (src/connect/ConnPicker.cpp: sourceFromPicker returns ConnSource{},
// describeSource returns "") and is expected to FAIL until Lane C4
// implements it for real, EXCEPT the two PINS marked below, which pass on
// the stub by construction (critic finding #5).
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "connect/ConnPicker.h"
#include "connect/ParamConnection.h"
#include <cmath>

using Catch::Approx;

TEST_CASE("sourceFromPicker: BpmSync maps shape + division onto an Lfo", "[connpicker]")
{
    SECTION("shape 3 (Square), div 2 -> 1 beat")
    {
        PickerChoice choice;
        choice.kind = PickerChoice::Kind::BpmSync;
        choice.shapeIdx = 3;
        choice.divIdx = 2;
        ConnSource src = sourceFromPicker(choice);
        REQUIRE(src.kind == ConnSource::Kind::Lfo);
        REQUIRE(src.lfo.shape == ConnSource::Lfo::Shape::Square);
        REQUIRE(src.lfo.cycleBeats == Approx(1.0f).margin(0.001f));
    }
    SECTION("shape 1 (SawUp), div 5 -> 8 beats")
    {
        PickerChoice choice;
        choice.kind = PickerChoice::Kind::BpmSync;
        choice.shapeIdx = 1;
        choice.divIdx = 5;
        ConnSource src = sourceFromPicker(choice);
        REQUIRE(src.kind == ConnSource::Kind::Lfo);
        REQUIRE(src.lfo.shape == ConnSource::Lfo::Shape::SawUp);
        REQUIRE(src.lfo.cycleBeats == Approx(8.0f).margin(0.001f));
    }
}

TEST_CASE("sourceFromPicker: Signal carries the registry name straight through", "[connpicker]")
{
    PickerChoice choice;
    choice.kind = PickerChoice::Kind::Signal;
    choice.signalName = "Bass";
    ConnSource src = sourceFromPicker(choice);
    REQUIRE(src.kind == ConnSource::Kind::Signal);
    REQUIRE(src.signalName == "Bass");
}

TEST_CASE("sourceFromPicker: Macro 7 is reachable (pins the widget's <200+kNumMacros menu-id fix)", "[connpicker]")
{
    PickerChoice choice;
    choice.kind = PickerChoice::Kind::Macro;
    choice.macroIdx = 7;
    ConnSource src = sourceFromPicker(choice);
    REQUIRE(src.kind == ConnSource::Kind::Macro);
    REQUIRE(src.macroIndex == 7);
}

TEST_CASE("sourceFromPicker: Timeline becomes a real 4-beat linear ramp Envelope", "[connpicker]")
{
    PickerChoice choice;
    choice.kind = PickerChoice::Kind::Timeline;
    ConnSource src = sourceFromPicker(choice);
    REQUIRE(src.kind == ConnSource::Kind::Envelope);
    REQUIRE(src.env.clock == ConnSource::Envelope::Clock::Beats);
    REQUIRE(src.env.cycleBeats == Approx(4.0f).margin(0.001f));
    REQUIRE(src.env.curve.pts.size() == 2);
    REQUIRE(src.env.curve.eval(0.5) == Approx(0.5f).margin(0.001f));
}

TEST_CASE("sourceFromPicker: ClipPosition", "[connpicker]")
{
    PickerChoice choice;
    choice.kind = PickerChoice::Kind::ClipPosition;
    ConnSource src = sourceFromPicker(choice);
    REQUIRE(src.kind == ConnSource::Kind::ClipPosition);
}

TEST_CASE("[pin] sourceFromPicker: Manual is Kind::None", "[connpicker][pin]")
{
    PickerChoice choice;
    choice.kind = PickerChoice::Kind::Manual;
    ConnSource src = sourceFromPicker(choice);
    REQUIRE(src.kind == ConnSource::Kind::None);
}

TEST_CASE("describeSource: matches the picker menu strings", "[connpicker]")
{
    SECTION("Square 1 Beat")
    {
        ConnSource s;
        s.kind = ConnSource::Kind::Lfo;
        s.lfo.shape = ConnSource::Lfo::Shape::Square;
        s.lfo.cycleBeats = 1.0f;
        REQUIRE(describeSource(s) == "Square 1 Beat");
    }
    SECTION("Bass")
    {
        ConnSource s;
        s.kind = ConnSource::Kind::Signal;
        s.signalName = "Bass";
        REQUIRE(describeSource(s) == "Bass");
    }
    SECTION("Macro 8 (index 7, 1-based display)")
    {
        ConnSource s;
        s.kind = ConnSource::Kind::Macro;
        s.macroIndex = 7;
        REQUIRE(describeSource(s) == "Macro 8");
    }
    SECTION("Timeline")
    {
        ConnSource s;
        s.kind = ConnSource::Kind::Envelope;
        s.env.clock = ConnSource::Envelope::Clock::Beats;
        s.env.cycleBeats = 4.0f;
        REQUIRE(describeSource(s) == "Timeline");
    }
    SECTION("Clip Position")
    {
        ConnSource s;
        s.kind = ConnSource::Kind::ClipPosition;
        REQUIRE(describeSource(s) == "Clip Position");
    }
}

TEST_CASE("[pin] describeSource: Manual (Kind::None) describes as empty", "[connpicker][pin]")
{
    ConnSource s;   // default kind == None
    REQUIRE(describeSource(s) == "");
}
