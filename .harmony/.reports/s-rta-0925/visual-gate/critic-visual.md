# Visual-Design Critic — s-rta-0925 visual-gate

Reviewed all 25 PNGs in `.harmony/.reports/s-rta-0925/visual-gate/` (00-11 full-window shots, A-E labeled crops, sheet.png contact sheet).

## MUST (blocking)

1. **Overlapping/collided text in the Composition Inspector's "Per-Type Autopilot" section** — `B_composition_inspector.png` (also visible in `01_full_composition_tab.png` and `02_full_composition_scrolled.png`, row directly under the "Per-Type Autopilot" collapsible header). The row's checkbox and its bold "Per-Type 16" label are drawn on top of another, different, partially-legible label (reads as "...a..."/"...que..." fragments, most likely the "Opaque" row label bleeding up one row) — two text layers occupy the same pixels. Pixel-crop confirms this is a true rendering collision, not a screenshot artifact: at 4x zoom the ghost glyphs and "Per-Type" glyphs interleave stroke-for-stroke. This is a layout/z-order bug (likely a missing row-height reservation or a label component positioned at the wrong y-offset for the enable-checkbox row), not a color/contrast issue. Fix: give the per-type-enable checkbox row its own label and vertical slot separate from the "Opaque"/"Transparent"/"Effect" per-type-count rows.

## SHOULD (non-blocking, recommend before ship)

1. **Inconsistent section-header treatment in the Composition Inspector** — `B_composition_inspector.png`. "Transform" gets a teal-highlighted header bar with a "P." badge, while sibling collapsible headers at the same hierarchy level ("Per-Type Autopilot", "Composition", "Global Effects", "Output Settings") are plain dark bars with no badge. If the badge communicates a real state (e.g., "has a pinned/animated parameter"), fine — but visually it reads as an unexplained one-off inconsistency in an otherwise uniform accordion list. Worth a one-line tooltip or consistent badge treatment across sections that qualify.
2. **Record panel status-message color ladder is dense** — `D7_record_over_during_playback.png` stacks three colored status lines (red/yellow/cyan) with no visual separator or indent, in a fairly small type size against the dark background. Legible in isolation, but the three semantically-different messages (error/state, active-mode warning, informational) are close enough in size/weight that a first-time user has to read all three to know which is the "important" one. A slightly bolder weight or icon on the top (red) line would strengthen the hierarchy that already exists in the copy.

## NICE (polish, optional)

1. `A_topbar_master.png` / `E_A_topbar_master_0p3.png`: the top-bar Master fader has no numeric readout next to it (unlike every other slider in the app, which pairs a slider with a "0.30"-style value label, e.g. the Composition Inspector's Master knob in `E_B_composition_master_0p3.png`). Given this task's own finding that the top-bar fader and the Composition "Master" knob are the same value, adding the numeric readout to the top-bar fader would match the app's own established slider convention and let a user read the exact master level without opening the Composition tab.

## Composition tab / Video-Opacity-removal specific check

Confirmed clean via `B_composition_inspector.png`, `01_full_composition_tab.png`, `02_full_composition_scrolled.png`: after "Per-Type Autopilot" the accordion goes straight to "Composition" (Master, Speed) → "Transform" → "Global Effects" → "Output Settings" — there is no orphaned "Video"/"Opacity" header, no dangling divider, and no abnormal vertical gap where that section used to be. The removal was layout-clean; the only defect in this region is the unrelated MUST item above (Per-Type Autopilot label collision), which pre-exists independent of the Video Opacity removal (it sits one section above the removal site, in the Per-Type Autopilot block, not the Composition/Transform block itself).

## Other panels (spacing / contrast / dark-theme consistency)

- `C_browser_tabs.png`: all 6 tabs ("Files", "FX", "Sources", "Comp/Decks", "Record", "MilkDrop") render fully unclipped with consistent padding and the active-tab (white-on-dark-purple) vs inactive (gray-on-navy) contrast is clear and consistent with the rest of the app's dark VJ theme.
- `D1`-`D8` (Record panel state machine): button states (idle/recording-red/playing-green), checkbox styling, and text-field placeholder styling are all consistent with the rest of the app across all 8 states. No alignment or contrast regressions found in this panel.
- `00_full_initial.png`, `01_full_composition_tab.png`, `02_full_composition_scrolled.png`: overall dark-theme palette (near-black background, navy/purple panel chrome, cyan accent for active/selected controls) is applied consistently across TopBar, Deck grid, Preview/Output tabs, and the right-hand inspector/browser — no stray light-theme or off-palette elements found.

## VERDICT=FAIL

MUST list:
1. Overlapping/collided label text in the Composition Inspector's "Per-Type Autopilot" section (checkbox row text collides with a second, different label) — `B_composition_inspector.png`, `01_full_composition_tab.png`, `02_full_composition_scrolled.png`.
