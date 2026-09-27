// test_record_panel_pads_removed -- s-rta-0927 routine display, slice A (plan-routine-display-A.md 2.6 / 5.5):
// the Record tab no longer carries a routine pad row (its press stopped a playing routine -- a stop button in
// disguise); routines are fired from the deck's ROUTINES row. The Save Routine row stays.
// Headless JUCE widgets under ScopedJuceInitialiser_GUI -- no window, no peer.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/RecordPanel.h"

TEST_CASE("RecordPanel: no routine pad row; the Save Routine row is still there", "[recordpanel][routine]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    RecordPanel p;
    p.setSize(380, 600);
    for (int i = 0; i < 8; ++i)
        CHECK(p.findChildWithID("routinePad" + juce::String(i)) == nullptr);
    CHECK(p.findChildWithID("saveRoutine") != nullptr);
    CHECK(p.findChildWithID("routineFromBar") != nullptr);
    CHECK(p.findChildWithID("routineToBar") != nullptr);
    CHECK(p.findChildWithID("routineName") != nullptr);
}
