// test_routine_bank_model -- s-rta-0926 lane 3 (plan-routines-s1-final.md section 5.4/6.1 case 14): the Save
// Routine row's bar-range clamp. s-rta-0927: the Record panel's pad row (and its six view cases) moved to the
// deck -- tests/test_routine_deck_view.cpp pins the ROUTINES row. Pure, juce_core only.
#include <catch2/catch_test_macros.hpp>
#include "ui/RoutineBankModel.h"

TEST_CASE("clampRoutineBarRange leaves an already-valid range untouched", "[routine][bank][range]")
{
    const auto r = clampRoutineBarRange(1, 4);
    CHECK(r.fromBar == 1);
    CHECK(r.toBar == 4);
}

TEST_CASE("clampRoutineBarRange raises a blank/zero fromBar to 1", "[routine][bank][range]")
{
    // TextEditor::getText().getIntValue() on an empty field returns 0.
    const auto r = clampRoutineBarRange(0, 4);
    CHECK(r.fromBar == 1);
    CHECK(r.toBar == 4);
}

TEST_CASE("clampRoutineBarRange raises a blank/zero toBar to at least fromBar", "[routine][bank][range]")
{
    const auto r = clampRoutineBarRange(3, 0);
    CHECK(r.fromBar == 3);
    CHECK(r.toBar == 3);
}

TEST_CASE("clampRoutineBarRange raises a negative bar to 1", "[routine][bank][range]")
{
    const auto r = clampRoutineBarRange(-5, -2);
    CHECK(r.fromBar == 1);
    CHECK(r.toBar == 1);
}

TEST_CASE("clampRoutineBarRange raises an inverted range so toBar >= fromBar", "[routine][bank][range]")
{
    const auto r = clampRoutineBarRange(8, 2);
    CHECK(r.fromBar == 8);
    CHECK(r.toBar == 8);
}
