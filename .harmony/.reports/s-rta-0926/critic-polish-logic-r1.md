# Critic — Logic (interaction/state) — Polish Round 1
Lens: does each control mean what it shows in every state, including empty/disabled.
Scope: three named items only. Everything else noted as pre-existing/out of scope.

## Verdict: PASS (no MUST)

## Item 1 — "Deck Load" button label

Before: `before-crop1-deckload.png` shows the button clipped to "Deck Loa" (trailing "d" cut off), between "Deck Save" and "No file loaded". After: `after-crop1-deckload.png` and `after-full.png` show the full word "Deck Load" rendered, un-clipped, with normal padding, and "No file loaded" still fully readable to its right with no new overlap. Full-window shot confirms no downstream row/layout shift (deck grid, tab bar, browser panel all unchanged). No mismatch between what the button says and what it does — it is a plain Load action, correctly labeled now.

**Verdict on item 1: fixed, no defect.**

## Item 2 — Clip tab empty state vs Dashboard

Before: `before-crop2-cliptab.png` — "No clip selected" is drawn centered over the whole ClipInspector area, directly on top of the 4th/5th Dashboard knobs (text bisects the knob glyphs).

After: `after-crop2-cliptab.png` and `after-full.png` — all 8 Dashboard knobs are fully visible with their "0.00" value readouts, "Link 1".."Link 8" labels, and "Manual" buttons all unobstructed; "No clip selected" now renders in its own row underneath, no pixel overlap. The Dashboard (Global MacroBank) is correctly still live/interactive with no clip selected — the builder's note that it's the global (not clip-scoped) dashboard and must stay functional is consistent with CLAUDE.md ("only the Global dashboard/MacroBank is instantiated") and with what's on screen: the knobs are not greyed/disabled, which is the correct state since they are NOT clip-dependent controls. Good instance of "control means what it shows" — nothing implies these knobs are inert, and they aren't.

I also checked for the regression class the builder called out (getPreferredHeight returning too-small a box, silently swallowing the text) — in `after-crop2-cliptab.png` the "No clip selected" text is clearly present and fully legible, not clipped top or bottom, so that particular failure mode does not recur in this shot.

**Verdict on item 2: fixed, no defect.**

## Item 3 — TopBar "Master Signal" fader (first critic look)

`after-crop3-topbar-mastersignal.png` (identical to before — out of fence, unchanged this round) shows, left to right: `Signal: [slider]` then `Master: [slider]` then `Output: [dropdown]`.

Observations against the four sub-questions asked:

- **Reads clearly / whole-word label**: the on-screen label is "Signal:", not "Master Signal:". "Signal" is itself a whole word (not an abbreviation like "Sig."), so it does not violate the project's no-abbreviation rule. But it is a *shortened* name relative to what CLAUDE.md calls this control ("Master Signal fader — post-analysis signal depth"). Sitting immediately next to a separate "Master:" fader, "Signal:" reads as its own, unrelated control rather than as "Master Signal" — a first-time viewer has no on-screen text tying "Signal" to "Master" as one compound concept. This is a naming-completeness/clarity gap, not a functional bug (the slider does what the docs say it does) — **SHOULD**, not MUST.
- **Distinguishable from Master**: the two sliders are pixel-identical in style (same track length, same cyan thumb, same color, no icon or color-coding difference) and are placed directly adjacent to each other. The only differentiator is the ~40px text label to the left of each. At a glance during a live set this is easy to mis-grab. **SHOULD**: give it a distinct accent color or icon, not just a text label, before this is trusted for live performance muscle memory.
- **Value readout**: neither "Signal:" nor "Master:" shows a numeric value (no "0.50" style readout the way the Dashboard knobs below show "0.00" per knob). This is **not a new inconsistency introduced by this round** — the pre-existing "Master:" fader has the same lack of readout, so "Signal:" matches its sibling control exactly. Flagging as **NICE**: for consistency with the Dashboard's numeric-readout convention, both TopBar faders could gain a value label, but this is pre-existing UI debt, not something item 3 introduced or regressed.

None of the above are logic/state defects in the sense of "the control claims one thing and does another" — the fader is not mislabeled to the point of being misleading about its function once a user reads CLAUDE.md or hovers/tries it, and it is not broken. They are clarity/discoverability gaps on a control that, per the packet, has never had a critic pass before.

**Verdict on item 3: no MUST. Two SHOULDs (label completeness, visual distinguishability from Master), one NICE (value readout parity with Dashboard).**

## Findings

### SHOULD
1. TopBar "Master Signal" fader label reads only "Signal:" with no visual/textual tie to "Master" — first-time users can't tell from the UI alone that this is "Master Signal" (per CLAUDE.md) rather than an unrelated meter/control. Consider relabeling to "M.Signal:"-style whole-word variant (e.g. "Signal Depth:") or adding a tooltip that says "Master Signal".
   - `src/ui/TopBar.cpp` (or wherever the Signal/Master labels are drawn) — file not located in this read-only pass; builder should confirm exact path.
2. "Signal:" and "Master:" faders are visually identical (same slider skin, no color/icon differentiation) and directly adjacent — easy to grab the wrong one live. Recommend a distinct thumb/track color for one of them.

### NICE
1. Neither TopBar fader (Signal or Master) shows a numeric value readout, unlike the Dashboard's per-knob "0.00" convention. Pre-existing pattern (Master fader already lacked this before Signal was added), not a regression — optional polish for consistency.
2. Pre-existing, out of scope: BPM tab panel shows a large empty black area with only a faint "BPM" watermark centered — not part of this round's fence, noting for a future round.
