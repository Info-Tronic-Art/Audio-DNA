# UX Critic — s-rta-0925 visual gate

Reviewed all 25 PNGs (00_full_* full-window shots, A/B/C/D1-D8/E series crops, sheet.png contact sheet) in
`.harmony/.reports/s-rta-0925/visual-gate/`. Confirmed against the shooter log (no safety incidents, clean
teardown).

## MUST

1. **Browser tab label is an abbreviation, violating the project's whole-word UI rule.**
   `C_browser_tabs.png` shows the tab row `Files | FX | Sources | Comp/Decks | Record | MilkDrop`.
   `Comp/Decks` is an abbreviation of "Composition/Decks" (or similar). CLAUDE.md's UI Text Rule is explicit:
   *"Always display whole words in the UI — never use abbreviations... Labels, button text, dropdown items,
   and tooltips must all use complete words."* This is not a truncation/clipping issue (the log confirms the
   tab renders fully unclipped) — it is the label text itself that is abbreviated. At performance distance a
   non-technical VJ scanning tabs needs the full word, not a shorthand. Fix: rename the tab to a whole-word
   label (e.g. "Composition" or "Decks", matching whatever the panel is actually titled elsewhere — the
   Composition inspector tab is spelled out in full in `B_composition_inspector.png`).

2. **Overlapping/garbled text on the Composition inspector's "Per-Type" row.**
   `B_composition_inspector.png` (also reproduced at 0.30 master in `E_B_composition_master_0p3.png`, so it is
   not a one-off render glitch) shows the "Per-Type Autopilot" section's enable-checkbox row: a grayed-out
   partial word (visible as `...d...`/`...a...` fragments) is rendered directly behind/under the "Per-Type 16"
   label and its own checkbox, producing double-exposed, partially legible text. Crop:
   `/tmp/crop_pertype.png` (4x zoom of the region at x=0-300,y=270-320 in `B_composition_inspector.png`) shows
   the checkbox glyph and a hidden label fragment sitting directly under "Per-Type". This fails "notices plain
   and whole-word" — a VJ glancing at this row at performance distance cannot read what the checkbox controls.
   This is a layout/z-order bug (a stale or overlapping label component), not a copy problem, and should be
   fixed at the component level in `CompositionInspector`.

## SHOULD

1. **Top-bar Master fader has no numeric readout.** `A_topbar_master.png` shows only a slider track + thumb
   labeled "Master:" with no percentage/value text, while the Composition inspector's Master control
   (`B_composition_inspector.png`, `E_B_composition_master_0p3.png`) shows both the slider AND a numeric value
   ("1.00" / "0.30"). The shooter log's own E-series confirms these two controls are the *same* master value
   (linked via REST/OSC), so a VJ glancing only at the top bar (the control most likely to be visible during a
   set, since the Composition inspector is a scrollable tab away) cannot tell whether master is at 100%, 30%,
   or anywhere in between without also opening the Composition tab. Add a numeric readout next to the top-bar
   slider to match the inspector.

2. **D7's combined record+playback status line is dense for glance-reading.** `D7_record_over_during_playback.png`
   packs two independent states into one line: `Recording over shot_rec 0:09 · 0 lanes · 0 moves — Playing
   0:17 / 0:17 · unresolved: 0`. Every other Record-panel state (D2, D5, D6, D8) uses one state fact per line.
   At performance distance this single overloaded line (record clock, lane count, move count, an em-dash, then
   playback clock and unresolved count) is harder to parse in a fast glance than the two-line pattern used
   elsewhere. Suggest splitting into a "Recording over..." line and a "Playing..." line, consistent with D6.

## NICE

1. **Active-button color intensity varies between shots without an obvious state cue.** `Stop Playback` renders
   bright green in `D6_record_playing.png` but a visibly muted/darker green in `D7_record_over_during_playback.png`
   and `D8_record_stop_during_over.png` (the latter two also show the muted green while `Stop Recording`/action
   buttons are bright). This reads as a hover/press vs. resting-state difference rather than a meaning change,
   but a VJ under time pressure could misread the dimmer green as "disabled." Consider a slightly higher
   resting-state saturation for active toggle buttons so "on" always reads unambiguously at a glance.

## Verdict

VERDICT=FAIL

MUST list:
1. `C_browser_tabs.png` — "Comp/Decks" tab abbreviates a whole word, violating CLAUDE.md's UI Text Rule.
2. `B_composition_inspector.png` (also `E_B_composition_master_0p3.png`) — overlapping/garbled text under the
   "Per-Type Autopilot" checkbox row makes that control's label unreadable.
