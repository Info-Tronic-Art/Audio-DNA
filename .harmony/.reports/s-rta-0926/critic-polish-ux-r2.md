# UX Critic — Polish Round 2 (s-rta-0926)

**Verdict: FAIL** (1 MUST — pre-existing, out-of-fence item explicitly included in this round's judging scope)

## Scope
Three items judged, before/after: (1) Deck Load button label, (2) Clip tab empty state, (3) TopBar Master Signal fader (first-ever critic look).

---

## Item 1 — "Deck Load" button label
**Shot**: after-crop1-deckload.png

Confirmed fixed, no regression. The button now renders the full word "Deck Load" with visible padding on both sides; "No file loaded" sits clear of it with a visible gap. Row also shows Save / Load / FX Save / Deck Save / Deck Load in a consistent button family — all whole-word, no abbreviations. No overlap, no clipping.

**Verdict on this item: PASS.**

---

## Item 2 — Clip tab empty state
**Shot**: after-crop2-cliptab.png

Confirmed fixed, no regression. All 8 Dashboard knobs (Link 1–8) are fully visible with their 0.00 readouts and Manual buttons, none obstructed. "No clip selected" sits in its own row well below the knob row with a clear visible gap — no overlap with the dashboard.

**Verdict on this item: PASS.**

---

## Item 3 — TopBar Master Signal fader (first critic look)
**Shot**: after-crop3-topbar-mastersignal.png

This is the item this lane was told NOT to touch, and the shot confirms it is pixel-identical to round 1's finding. Judging the four sub-questions asked:

- **Does it read clearly?** No. The crop shows two sliders side by side, both filled roughly 60-70% with an identical cyan thumb and identical cyan-on-dark track styling. Nothing about the visual treatment distinguishes "this is the post-analysis signal depth control" from "this is the overall output level control" — a first-time viewer has no way to tell which one is more consequential (Master literally cuts output; Master Signal scales signal-routing depth per CLAUDE.md) without reading the CLAUDE.md doc.
- **Is the label whole-word?** No — this is the specific defect. The control is described everywhere else in the project (CLAUDE.md, session notes) as "Master Signal." The on-screen label reads only "Signal:". Per this project's own UI rule ("Always display whole words in the UI — never use abbreviations... for example, 'Inverted Luma is Alpha' not 'Inv. Luma is Alpha'"), a bare "Signal:" next to "Master:" is not an abbreviation in the letter-truncation sense, but it drops the disambiguating word entirely — functionally the same failure the rule exists to prevent. A user scanning the TopBar sees "Signal / Master" and has no cue that "Signal" IS a Master-level control (Master Signal), not some unrelated per-track signal meter.
- **Is it distinguishable from Master?** No. Same slider widget, same color, same fill style, same size, positioned immediately adjacent. The only differentiator is the one-word label, and that label is the ambiguous one described above.
- **Is there a value readout consistent with the app's other sliders?** No readout is visible in the crop for either fader. This project's own dashboard knobs (Item 2's shot) show a numeric value ("0.00") under every knob, and ResettableSliders elsewhere in the app are documented as carrying real values users can read and right-click-reset. Two faders that gate audio-reactivity depth and program output level, with zero numeric readout, is a real gap for a live-performance tool — Boris cannot know he's at "62%" vs "58%" without dragging and guessing, and can't verify a precise recall value between shows.

**Verdict on this item: FAIL.**

### MUST
1. **[Item 3] Master Signal fader is not distinguishable from Master and its label drops the disambiguating word.** `TopBar.cpp` — the label reads "Signal:" where the feature is named "Master Signal" everywhere else in the project; combined with using the identical slider style/color as the adjacent "Master:" fader, a viewer cannot tell these are two different controls with very different blast radii (one scales signal-routing depth, the other is the literal output level). Failure scenario: Boris or any operator drags what they believe is "Master" down to kill output for a blackout cue, but grabs "Signal" instead — output stays hot while all audio-reactive routing goes to a flat, unmusical zero-depth state, live, with no visual difference on this bar to explain why. This is the same open MUST carried forward from round 1 (this lane was correctly told not to touch TopBar.cpp, so it remains unresolved) — flagging it again because this round's brief explicitly asks this critic to judge it for the first time.

### SHOULD
1. **[Item 3] No numeric value readout on Signal or Master faders.** Every other value control shown in this session's shots (Dashboard Link knobs) displays its value as text under the control. The TopBar faders show none. Recommend a small numeric readout (e.g. "62%") next to or under each fader, consistent with the Dashboard pattern, so operators can read and recall exact levels rather than judging by thumb position alone.
2. **[Item 3, pre-existing, out of scope] Master Signal's real-world effect (per CLAUDE.md: "0.0 = every connected control sits at its own hand value... 1.0 = bit-identical to no fader at all") is non-obvious from the UI alone.** A tooltip explaining what dropping this fader actually does (vs. Master, which is an intuitive output-level cut) would reduce the risk above independent of the labeling fix.

### NICE
1. **[Item 1, pre-existing, out of scope]** The button row (Save/Load/FX Save/Deck Save/Deck Load) is getting long; as more buttons are added this row is a candidate for a dropdown/menu to avoid future re-clipping regressions like the one this round fixed.
2. **[Item 3]** Once distinguishable, consider grouping Signal and Master under a shared visual frame or divider with a small caption ("Depth" / "Output") so their relationship and difference is legible at a glance rather than inferred from label text alone.

---

## Summary
Items 1 and 2 are cleanly fixed with no regressions — confirmed via before/after comparison and the full-window shot showing no downstream layout breakage. Item 3, judged for the first time this round, has one MUST: the Master Signal fader's on-screen label ("Signal:") does not disambiguate it from the adjacent Master fader, and the two are visually identical apart from that ambiguous label, with no value readout on either. This is out-of-fence for this lane (TopBar.cpp, correctly untouched per instructions) but is included in this round's judging scope, so it is reported as a MUST and the overall verdict is FAIL.
