#pragma once

// PerTypeAutopilotLayout -- s-rta-0925 visualfix. The Composition Inspector's "Per-Type
// Autopilot" section has 4 rows below its header: the enable checkbox, then the Opaque /
// Transparent / Effect cycle rows. CompositionInspector::resized() (control placement) and
// ::paint() (row labels) both need the same row y-offsets; before this fix they were two
// separate hand-written offset sequences that had drifted apart -- the enable-checkbox row
// and the "Opaque" label row both landed at offset 0, so the checkbox and the label were
// drawn on top of each other (B_composition_inspector.png, critic-visual.md /
// critic-ux.md MUST #1/#2). Routing both call sites through this one pure function makes
// that drift impossible to reintroduce. Pure -- no JUCE types; the caller turns an offset
// into a juce::Rectangle<int> at its own y + this row's offset. Pinned by
// tests/test_per_type_autopilot_layout.cpp.
struct PerTypeAutopilotRows
{
    int enableY;
    int opaqueY;
    int transparentY;
    int effectY;
    int totalHeight;  // rows + trailing section gap; does NOT include the section header
};

inline PerTypeAutopilotRows perTypeAutopilotRowsFor(int rowHeight, int sectionGap)
{
    PerTypeAutopilotRows r;
    r.enableY = 0;
    r.opaqueY = rowHeight;
    r.transparentY = rowHeight * 2;
    r.effectY = rowHeight * 3;
    r.totalHeight = rowHeight * 4 + sectionGap;
    return r;
}
