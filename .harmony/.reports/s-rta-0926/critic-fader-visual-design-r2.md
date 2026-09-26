# Critic — Fader Visual Design, Round 2

**Item under review:** TopBar "Master" / "Master Signal" fader pair
**Verdict: PASS**

## What round 2 fixed (verified against shots, pixel-decoded, not hash-compared)

`after-crop-topbar-mastersignal.png` (1350x160) shows, left to right:
`Fade: [cyan slider] 0.30` · `Master Signal: [magenta slider] 1.00` · `Master: [cyan slider] 1.00` · `Output: Off`

- **Label is the full word "Master Signal:"** — not the bare "Signal:" that failed round 1. Confirmed by direct pixel read of the crop, not inferred.
- **The two faders are no longer visually identical.** Master Signal's thumb/track render in magenta (matches `kAccentMagenta`, already an established accent elsewhere in the app — the "Hit" trigger pad and the Beat/Tempo readouts in the deck strip use the same magenta, so this is not a new, unexplained color introduced only here). Master keeps the standard cyan accent used throughout the rest of the TopBar (Fade, Gain, etc.).
- **Both faders now show a numeric readout** — "1.00" next to Master Signal, "1.00" next to Master, styled identically (same font, size, weight, decimal precision) to the existing readout on Fade ("0.30"). This matches the app's established value-readout convention rather than introducing a new one.
- **No clipping or overlap.** In the full 3456x2158 window capture, the whole TopBar row (Audio through FPS/DSP) lays out cleanly with normal spacing; the Master Signal / Master pair does not crowd its neighbors ("Output:" dropdown, "FPS:120").
- Full-window shot confirms this is the live app's default startup state, not a mocked/cropped fabrication — the deck grid, inspector tabs, and browser panel around it are all present and consistent with the rest of CLAUDE.md's documented UI.

## Judge criteria checklist

| Criterion | Verdict |
|---|---|
| Legibility at a glance mid-performance | Pass — label text size/weight matches the other TopBar labels (Fade:, Gain:, Quantize:); readouts are same size/contrast as Fade's |
| Whole words, no truncation/abbreviation | Pass — "Master Signal:" and "Master:" are both spelled out in full, per the UI Text Rules |
| Can a performer grab the wrong one | Pass, with a caveat (see SHOULD) — color (magenta vs cyan) plus distinct label text differentiate them; thumb/track shape and slider length are otherwise identical |
| Value readouts consistent with the app's other controls | Pass — same TextBoxRight numeric style as Fade's existing "0.30" |
| Fits without clipping/overlap with neighbours | Pass — confirmed in both the crop and the full window capture |
| Colour accent consistent with theme | Pass — magenta is an existing accent (Hit pad, Beat/Tempo bar), not a novel color; cyan remains Master's/the majority accent |

## MUST (blocks showing Boris)

None. Both round-1 MUST failures (bare "Signal:" label, visually-identical faders with no value shown) are resolved and verified directly from the shots.

## SHOULD

1. **Shared "Master" prefix still invites a fast misread.** "Master Signal:" and "Master:" sit close together and both start with the same word; at a glance under stage lighting, a performer scanning left-to-right for "Master" could grab the first one they see rather than reading the full label. The magenta/cyan color coding mitigates this, but the fix is currently 100% dependent on color — there is no shape, icon, or weight difference backing it up. Consider bolding just the differentiating second word ("Master **Signal**:") or adding a small icon prefix, so the differentiation survives even if a performer's eye catches the slider before the label.
2. **Identical slider geometry.** Track length, thumb size, and shape are the same for both controls — only hue and adjacent text differ. This is standard practice for a mixer-strip-style control group and is not a blocker, but for a control this consequential (Master Signal changes how every other signal-driven control on stage behaves, per CLAUDE.md's Master Signal note), a slightly different thumb shape (e.g., diamond vs. round) would add a second, non-color channel of differentiation — useful for color-vision-deficient performers, since magenta/cyan is a safer pairing than red/green but not guaranteed distinguishable for every observer under colored stage light.

## NICE

1. A thin visual separator (e.g., a 1px vertical divider or a few extra px of gutter) between the Master Signal group and the Master group would reinforce that they are two independent controls, not one compound widget, at a glance.
2. Given Master Signal's outsized behavioral effect (it scales every signal→parameter connection app-wide), a tooltip reinforcing "0% = manual hand values only, 100% = normal" (already implied by CLAUDE.md's Master Signal semantics) would help a performer who hasn't memorized the ARCHITECTURE_V2 semantics — not verified one way or the other from these static shots since tooltips require a hover state, so this is a suggestion, not a finding against the current work.

## Pre-existing, out of scope

- Fade and Master share the same cyan accent color and are visually adjacent; a performer distinguishing "Fade" from "Master" relies on label text only, no color differentiation. This existed before this round's work and is outside the Master/Master-Signal pair under review.
- General TopBar label font size/contrast (all labels, not just this pair) is small and low-contrast (gray-on-navy); if this is a legibility concern for the whole bar, it predates this change and applies uniformly across Audio/Gain/Quantize/Fade/Master Signal/Master/Output alike.

## Notes on verification

Findings are based on direct pixel/text inspection of both provided shots (`after-full.png`, `after-crop-topbar-mastersignal.png`), not on the builder's summary text. Both round-1 MUST items (bare "Signal:" label; visually-identical, valueless faders) are confirmed resolved in the shots themselves.
