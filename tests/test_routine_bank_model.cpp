// test_routine_bank_model -- s-rta-0926 lane 3 (plan-routines-s1-final.md section 5.4/6.1 case 14):
// the Routines pad row's click-through matrix, one TEST_CASE per state. Pure: the model derives
// everything from RoutineEngine::Status, so no GUI, no engine, no clock.
#include <catch2/catch_test_macros.hpp>
#include "ui/RoutineBankModel.h"

TEST_CASE("RoutineBankModel pad 0 empty -- disabled, numbered", "[routine][bank]")
{
    RoutineEngine::Status s;   // every slot defaults to state "empty", name ""
    const auto v = deriveRoutineBankView(s);
    CHECK(v.pads[0].text == "1: Empty");
    CHECK_FALSE(v.pads[0].enabled);
    CHECK(v.pads[0].firing);
    CHECK(v.pads[0].tone == RoutineBankView::Tone::Neutral);
}

TEST_CASE("RoutineBankModel pad idle -- named, enabled, fires", "[routine][bank]")
{
    RoutineEngine::Status s;
    s.slots[0].state = "idle";
    s.slots[0].name = "Drop 1";
    const auto v = deriveRoutineBankView(s);
    CHECK(v.pads[0].text == "1: Drop 1");
    CHECK(v.pads[0].enabled);
    CHECK(v.pads[0].firing);
    CHECK(v.pads[0].tone == RoutineBankView::Tone::Neutral);
}

TEST_CASE("RoutineBankModel pad pending -- Warning tone, next-bar text, stops on press", "[routine][bank]")
{
    RoutineEngine::Status s;
    s.slots[0].state = "pending";
    s.slots[0].name = "Drop 1";
    const auto v = deriveRoutineBankView(s);
    CHECK(v.pads[0].text == "1: Drop 1 (next bar)");
    CHECK(v.pads[0].enabled);
    CHECK_FALSE(v.pads[0].firing);
    CHECK(v.pads[0].tone == RoutineBankView::Tone::Warning);
}

TEST_CASE("RoutineBankModel pad running -- Playing tone, current bar text, stops on press", "[routine][bank]")
{
    RoutineEngine::Status s;
    s.slots[0].state = "running";
    s.slots[0].name = "Drop 1";
    s.slots[0].position = 5.0;   // beat 5 of the cycle -> bar 2 (4 beats/bar, 1-based)
    const auto v = deriveRoutineBankView(s);
    CHECK(v.pads[0].text == "1: Drop 1 (bar 2)");
    CHECK(v.pads[0].enabled);
    CHECK_FALSE(v.pads[0].firing);
    CHECK(v.pads[0].tone == RoutineBankView::Tone::Playing);
}

TEST_CASE("RoutineBankModel running bar 1 at the very start of the cycle", "[routine][bank]")
{
    RoutineEngine::Status s;
    s.slots[3].state = "running";
    s.slots[3].name = "Loop";
    s.slots[3].position = 0.0;
    const auto v = deriveRoutineBankView(s);
    CHECK(v.pads[3].text == "4: Loop (bar 1)");
}

TEST_CASE("RoutineBankModel pads are independent -- one running does not affect the rest", "[routine][bank]")
{
    RoutineEngine::Status s;
    s.slots[1].state = "running";
    s.slots[1].name = "Bass Drop";
    s.slots[1].position = 1.0;
    s.slots[5].state = "idle";
    s.slots[5].name = "Ambient";
    const auto v = deriveRoutineBankView(s);
    CHECK(v.pads[0].text == "1: Empty");
    CHECK(v.pads[1].text == "2: Bass Drop (bar 1)");
    CHECK(v.pads[1].tone == RoutineBankView::Tone::Playing);
    CHECK(v.pads[5].text == "6: Ambient");
    CHECK(v.pads[5].tone == RoutineBankView::Tone::Neutral);
    CHECK(v.pads[7].text == "8: Empty");
    CHECK_FALSE(v.pads[7].enabled);
}
