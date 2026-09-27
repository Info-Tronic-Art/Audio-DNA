// test_topbar_model -- s-rta-0926b plan3-final.md section 3 (ITEM B, B.4): the TopBar's one-line
// bar readout, "Bar 1".."Bar 4", pinned against the pure barReadoutText function. Pure: no GUI,
// no engine, no clock -- same posture as test_routine_bank_model.cpp.
#include <catch2/catch_test_macros.hpp>
#include "ui/TopBarModel.h"

TEST_CASE("barReadoutText wraps 1-2-3-4-1", "[topbar][bar]")
{
    CHECK(barReadoutText(true, 0) == "Bar 1");
    CHECK(barReadoutText(true, 3) == "Bar 4");
    CHECK(barReadoutText(true, 4) == "Bar 1");
    CHECK(barReadoutText(true, 7) == "Bar 4");
    CHECK(barReadoutText(true, 65535) == "Bar 4");
}

TEST_CASE("barReadoutText with no BPM reads Bar -", "[topbar][bar]")
{
    CHECK(barReadoutText(false, 9) == "Bar -");
}
