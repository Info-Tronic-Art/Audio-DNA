# Visual/Graphic-Design Critic — Polish Round 1 (s-rta-0926)
Lens: visual-design and graphic-design. Read-only. Scope: the three named items only.

## Verdict: PASS

## Method
Read all 8 shots directly (before/after full + 3 crop pairs each). Cross-checked crop3 (Master
Signal fader, first critic look ever) against source in `src/ui/TopBar.cpp` (label text, slider
styling, tooltip, onDragStart/End wiring) to judge label wording and readout-parity claims from
ground truth, not screenshot guesswork alone.

## Item 1 — "Deck Load" button label
BEFORE: clipped to "Deck Loa" (crop1). AFTER: reads "Deck Load" in full, same button height/style/
position as its "Deck Save" neighbor, whole word, no visual clipping against "No file loaded" to
its right. Confirmed in both the crop and the full-window shot.
**MUST: none.**

## Item 2 — Clip tab empty state vs Dashboard
BEFORE: "No clip selected" text painted directly across the 4th/5th macro knobs (crop2), illegible
overlap. AFTER: all 8 Dashboard knobs render cleanly with their arcs, "0.00" values, "Link 1"–
"Link 8" labels, and "Manual" buttons fully intact and unobstructed; "No clip selected" sits in its
own row below, vertically clear of the knob row by a visible gap. Confirmed in both the crop and
the full-window shot (right inspector column, Clip tab).
**MUST: none.**

## Item 3 — TopBar Master Signal fader (first critic look)
Present next to "Master:" at the expected TopBar position (crop3, full shot). Judged against the
four questions asked:
- **Reads clearly**: yes — legible label, standard-height horizontal slider, same visual weight/
  color (`kTextSecondary` label, cyan thumb) as its neighbors.
- **Whole-word label**: yes, technically — "Signal:" is not an abbreviation (no dropped letters).
- **Distinguishable from Master**: text differs ("Signal:" vs "Master:"), thumb positions differ
  (mid vs full) in the shots, so at a glance the two are visually separable.
- **Value readout consistent with the app's other sliders**: **no numeric readout at all**
  (`setTextBoxStyle(juce::Slider::NoTextBox, ...)`, confirmed in TopBar.cpp). This is *consistent
  with its immediate neighbor* (the existing "Master:" fader uses the identical NoTextBox style),
  so it is not a new inconsistency this fader introduces — but it does NOT match the app's general
  pattern elsewhere in this exact frame (the Dashboard knobs two rows below show "0.00" per knob).
  Since the question was asked against "the app's other sliders" broadly, this is a real gap, not
  a pass — filed as SHOULD, not MUST, because it copies an established, shipped TopBar pattern
  rather than being a new regression.

No MUST-level defect found in any of the three items — nothing here blocks Boris. All three
changes do what the builder's fix descriptions claim, verified pixel-content (not just presence)
in the AFTER crops.

## MUST (blocks ship)
None.

## SHOULD (recommend before ship)
1. **Master Signal fader has no value readout** (`src/ui/TopBar.cpp` `masterSignalSlider_`,
   `NoTextBox` style, no percentage/number anywhere in the TopBar row). A depth-scaling control
   that silently affects every connected signal in the app (per its own tooltip: "0% = everything
   sits at its hand-set value") is exactly the kind of control where an operator needs to glance
   at a number, not eyeball a thumb position, especially live under stage lighting. Recommend
   adding a small numeric readout (matches the Dashboard knob pattern already in the same
   screenshot) or at minimum a tooltip-on-hover value, before this ships to a live-performance
   audience.
2. **"Signal:" label is one word short of unambiguous.** The full concept (per TopBar.cpp comment
   and tooltip) is "Master Signal" — a sibling control to "Master" (level). The rendered label
   drops "Master" and reads bare "Signal:". CLAUDE.md's own vocabulary already uses "Signal Bar"
   for a *different* UI element (the mixer-strip audio-feature meters) and "Signal" as a mapping
   source category — so an operator scanning the TopBar could plausibly read "Signal:" as some
   other audio-meter control rather than the depth fader for the Master fader beside it. Consider
   "Signal Depth:" or "Master Signal:" (space permitting) to close the gap the tooltip currently
   has to do by itself.

## NICE (optional)
- Crop3's "Signal:"/"Master:" pair sit close together with identical thumb styling; a thin visual
  separator or slightly different accent tint for one of the two would help fast recognition at a
  glance during a live set, though current text labels are sufficient to disambiguate on a second
  look.

## Pre-existing, out of scope (noticed, not part of this fence)
- TopBar right side is visually dense (Signal/Master/Output/FPS/DSP all packed edge-to-edge with
  minimal breathing room) — a pre-existing layout characteristic of this row, not something this
  round's fixes touched or worsened.
- "DASHBOARD" section heading and the 8-knob grid use a slightly different label-casing convention
  (all-caps section header vs. Title Case "Link N" labels) — pre-existing, not in this fence.
