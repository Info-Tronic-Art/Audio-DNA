# UX Critic — Polish Round 1 (s-rta-0926)
Lens: usability, clarity, discoverability. Read-only. Verdict below.

## Verdict: FAIL (1 MUST)

---

## Item 1 — "Deck Load" button label

**Before**: clipped to "Deck Loa" (crop1). **After**: full "Deck Load" renders (crop1, full-window crop confirms).

Verified against `before-crop1-deckload.png` / `after-crop1-deckload.png`: label is fully legible, whole word, matches sibling "Deck Save" button width/style, does not crowd "No file loaded" next to it. No regression to the row (Save/Load/FX Save still fit).

**Verdict: FIXED. No finding.**

---

## Item 2 — Clip tab empty state vs Dashboard knobs

**Before**: "No clip selected" text painted directly across the 4th/5th Dashboard knobs (crop2), knob labels/values not shown at all in the before shot.
**After**: all 8 knobs + "0.00" values + "Link 1".."Link 8" labels + "Manual" buttons render fully unobstructed; "No clip selected" sits in its own row below, no overlap (crop2, confirmed also in `after-full.png` at the Clip tab).

**Verdict: FIXED. No finding.** Builder's own catch of the `getPreferredHeight()` regression (message briefly disappearing) before reporting is exactly the right instinct — good.

---

## Item 3 — TopBar "Master Signal" fader (first critic look)

Per `CLAUDE.md`, this control is the **Master Signal** fader — "post-analysis signal depth" that scales every signal→parameter connection, distinct from Gain (untouched) and distinct from the Composition **Master** fader (overall output level) sitting immediately to its right. In the TopBar it renders only as:

```
Signal: [====o----]     Master: [=====o---]
```

### MUST — "Signal:" label does not read as "Master Signal" and is confusable with the adjacent "Master:" fader

- The control's documented name is **Master Signal**; the UI label is the single word "Signal:". That word alone gives no indication this is a *master-level depth multiplier for all audio-reactive routing* — nothing distinguishes it from, say, an input/audio signal meter, or the app's existing "Signal Bar"/"Signal Strip"/signal-routing vocabulary used elsewhere in this same app (CLAUDE.md: Signal Bar, SignalStrip, SignalInspector, signal routing engine, Universal Signal Routing). A user scanning the TopBar has no cue this fader affects every mapping's depth project-wide.
- It sits directly beside a visually identical fader labeled "Master:" (same track/thumb style, same width, no color or icon differentiation). Two adjacent near-identical sliders reading "Signal:" / "Master:" — when the actual pair of concepts is "Master Signal depth" vs. "Composition Master level" — invites mis-reading the pairing as ("Signal", "Master") = two unrelated controls, or worse, invites grabbing the wrong one live. Per the project's own state notes this session, these are two *conceptually different* faders (one scales audio-reactive routing depth, one is overall output level) and mixing them up during a live show is a real-world failure mode for a VJ tool, which is exactly the use case this app is built for.
- This is the first time this control has been critic-reviewed (per task brief), so there is no earlier pass this defers to.

**Failure scenario**: A performer needs to briefly zero out audio reactivity (Master Signal → 0) without touching output level, glances at the TopBar, and — because both faders read as short single words in the same style with no distinguishing name, tooltip, or grouping — pulls "Master:" down instead of "Signal:", killing the visual output entirely mid-set.

**Fix direction (not prescriptive)**: label it "Master Signal:" (matches the doc name, still whole-word) or visually group/label it under a shared "Signal Depth" vs "Output Level" framing; at minimum give it a distinct accent color from the general Master/output cluster, consistent with how other functionally-distinct controls in this TopBar are already separated (e.g., Quantize/Fade cluster vs Output/FPS/DSP cluster).

### SHOULD — no numeric value readout on Signal/Master faders

Unlike the Dashboard macro knobs (each shows "0.00" beneath the knob) and the app's general convention of readable values on `ResettableSlider` controls, the TopBar Signal/Master faders show no numeric readout at all — only slider position. This is consistent with the existing sibling "Gain:" fader in the same TopBar (also no readout), so it is not a regression introduced by this fix, but it is a pre-existing inconsistency with the Dashboard's value-readout convention and worth closing before ship, especially for a fader whose exact depth (e.g., "is this really at 0, or just close to it") matters for live diagnosis.

### NICE — no tooltip on either fader

CLAUDE.md already notes comprehensive tooltip coverage is scheduled for P26, so this is expected/known, not a new gap. Flagging only because Item 3's clarity problem above would be substantially mitigated by a tooltip stating "Master Signal — scales all audio-reactive routing depth (Gain unaffected)" even before the P26 pass lands everywhere.

---

## Pre-existing, out of scope (noticed incidentally, not part of the three judged items)

- TopBar is visually dense (Audio/Gain/Transport/Tempo/BPM-state/Tap/Resync/Manual/Link/x1-x4/Quantize/Fade/Signal/Master/Output/FPS/DSP all in one row) — a lot of same-style controls competing for attention. Not something this fix touched; flagging only as a general observation for a future pass.
- "No clip selected" and "No file loaded" both use the same greyed inline style as disabled-looking text; fine as empty states, just noting for consistency review elsewhere, not a defect of this fix.

---

## Summary

| Item | Verdict |
|---|---|
| 1. Deck Load label | Fixed, no finding |
| 2. Clip tab / Dashboard overlap | Fixed, no finding |
| 3. Master Signal fader clarity | **MUST** — label/adjacency ambiguity with Master fader; SHOULD — no value readout |

**Overall: FAIL** (1 MUST on item 3). Items 1 and 2 are ship-ready as verified in these screenshots.
