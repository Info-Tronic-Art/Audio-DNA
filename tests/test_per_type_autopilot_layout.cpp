// test_per_type_autopilot_layout -- s-rta-0925 visualfix: the Composition Inspector's
// Per-Type Autopilot enable-checkbox row and its "Opaque" cycle row used to land at the
// same y (both computed as offset 0 from independent hand-written sequences in
// CompositionInspector::resized() and ::paint()), so the checkbox and the "Opaque" label
// were drawn on top of each other (B_composition_inspector.png, critic-visual.md MUST #1,
// critic-ux.md MUST #2). Pure function in src/ui/PerTypeAutopilotLayout.h -- no JUCE; this
// pins the row-offset arithmetic both call sites now share.
#include <catch2/catch_test_macros.hpp>
#include "ui/PerTypeAutopilotLayout.h"

TEST_CASE("PerTypeAutopilotLayout -- the four rows never overlap", "[pertypeautopilotlayout]")
{
    const auto rows = perTypeAutopilotRowsFor(22, 4);

    // The exact bug: enableY and opaqueY used to both be 0.
    CHECK(rows.enableY != rows.opaqueY);

    // Strictly increasing, one row-height apart.
    CHECK(rows.opaqueY == rows.enableY + 22);
    CHECK(rows.transparentY == rows.opaqueY + 22);
    CHECK(rows.effectY == rows.transparentY + 22);

    // No two rows' [y, y+rowHeight) spans overlap.
    const int ys[] = { rows.enableY, rows.opaqueY, rows.transparentY, rows.effectY };
    for (size_t i = 0; i < 4; ++i)
        for (size_t j = i + 1; j < 4; ++j)
            CHECK((ys[i] + 22 <= ys[j] || ys[j] + 22 <= ys[i]));

    // Total height accounts for all 4 rows plus the trailing section gap.
    CHECK(rows.totalHeight == 22 * 4 + 4);
}

TEST_CASE("PerTypeAutopilotLayout -- no overlap holds for any row height/gap", "[pertypeautopilotlayout]")
{
    const int rowHeights[] = { 1, 16, 22, 30, 64 };
    const int gaps[] = { 0, 4, 10 };
    for (int rowHeight : rowHeights)
    {
        for (int gap : gaps)
        {
            const auto rows = perTypeAutopilotRowsFor(rowHeight, gap);
            const int ys[] = { rows.enableY, rows.opaqueY, rows.transparentY, rows.effectY };
            for (size_t i = 0; i < 4; ++i)
                for (size_t j = i + 1; j < 4; ++j)
                    CHECK((ys[i] + rowHeight <= ys[j] || ys[j] + rowHeight <= ys[i]));
            CHECK(rows.totalHeight == rowHeight * 4 + gap);
        }
    }
}
