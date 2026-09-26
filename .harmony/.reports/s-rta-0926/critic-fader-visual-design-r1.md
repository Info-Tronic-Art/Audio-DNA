# Critic (Visual Design) — TopBar "Master Signal" / "Master" pair — Round 1

**Verdict: PASS**

## What I checked
- Pixel-read (not inferred) crops: `before-crop-topbar-mastersignal.png` vs `after-crop-topbar-mastersignal.png`, plus both full-window screenshots.
- Source for the actual fix, in the worktree the screenshots were built from: `.claude/worktrees/wf_23099956-236-2/src/ui/TopBar.h` and `TopBar.cpp` (note: this fix is NOT present on `main` at HEAD `02bce7c` — `src/ui/TopBar.h` there still declares `masterSignalLabel_{"", "Signal:"}`. Confirmed this is expected/in-flight work in the worktree, not a stray regression, before judging it.)
- `tests/test_master_signal_link.cpp` in that worktree for the layout/colour/tooltip assertions backing the screenshots.

## Judged against the brief

- **Legibility at a glance**: "Master Signal:" and "Master:" render at the same 11px secondary-text style as every other TopBar label. Clear at the crop's resolution.
- **Whole words**: "Master Signal:", "Master:", both tooltips ("Master: the output level of the whole composition." / "Master Signal: how strongly audio, oscillators, and every other signal move the controls they are wired to...") — no abbreviations anywhere. Meets the project's whole-word UI rule.
- **Can a performer grab the wrong one**: Previously both sliders were identical cyan with no label distinction beyond bare "Signal:"/"Master:". Now Master Signal carries a magenta thumb/track (`AudioDNALookAndFeel::kAccentMagenta`) against Master's cyan (`kAccentCyan`), plus the full "Master Signal:" name instead of a bare, ambiguous "Signal:". Pixel-read confirms the fill colours are in fact different in the live render, not just in code. This directly resolves the original FAIL reason.
- **Value readouts consistent with the app's other controls**: Both faders now use `Slider::TextBoxRight` at the same 35×20 box Fade already uses in this same bar (`fadeSlider_.setTextBoxStyle(TextBoxRight, false, 35, 20)`), same convention, same visual weight. "1.00" reads in full (a narrower box was tried and clipped to "1...", caught before shipping — see code comment and widened to 35).
- **Fits without clipping/overlap**: Pixel-read of the full-width right edge shows "Output: Off" and "FPS:108 DSP:3.2%" fully unclipped with normal letter-spacing, no overlap with the new readouts. Label width is computed from actual glyph metrics (`GlyphArrangement::getStringWidthInt`) rather than a guessed constant, so it won't silently truncate if the label text changes again later.
- **Colour accent consistent with theme**: `kAccentMagenta` is an existing `LookAndFeel.h` theme colour (`0xffff00e5`), not a new one-off hue — the same magenta already appears elsewhere in this exact screenshot (the "Hit" clip cell). Cyan remains the app's default accent for Master, unchanged.

## Pre-existing, out of scope
- Secondary toolbar row: "Deck Save" / "Deck Loa[d]" clipping (`d` cut off) — visible in `after-full.png`, present because this build's base differs from `before-full.png`'s; it's a different control, unrelated to the Master/Master Signal pair, not touched by this fix.
- TopBar right-side density at the app's 1280px minimum resizable width — dsp+fps alone already consume nearly the full remaining budget there, pre-existing per `critic-polish-ux-r1.md` and reconfirmed by this round's own test comment. Not something a Master/Master-Signal-scoped fix can resolve without a wider TopBar audit.

## MUST
(none)

## SHOULD
(none — the previous FAIL's three specific defects — bare label, identical fader appearance, no readout — are all directly addressed and pixel-confirmed.)

## NICE
- Consider a secondary differentiator beyond hue alone (e.g. a subtle track-background tint) for colour-vision-deficient performers, though magenta/cyan is a high-contrast hue pair and not a red/green confusion case, so this is a nice-to-have, not a gap.
- The 1280px-minimum-width TopBar density (see "pre-existing, out of scope") would be worth a dedicated pass at some point, independent of this fader fix.
