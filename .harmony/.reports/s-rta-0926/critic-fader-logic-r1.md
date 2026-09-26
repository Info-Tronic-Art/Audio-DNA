# Critic — Fader Logic (Master / Master Signal), Round 1

**Role:** logic critic (read-only) — does each control show and mean the same thing in every state.
**Item:** TopBar's "Master" and "Master Signal" pair.
**Code reviewed:** commit `6583441` on worktree `wf_23099956-236-2` (`fix(s-rta-0926 fader): TopBar Master Signal fader now reads "Master Signal", shows a value, and gets its own accent colour`), `src/ui/TopBar.cpp`/`.h`, `tests/test_master_signal_link.cpp`. Working tree clean, commit is real (not uncommitted scratch).

**Note on scope:** the primary `main` checkout at the repo root (`/Users/boriskarpman/projects/RealTimeAudio`, HEAD `02bce7c`) does **not** contain this fix — `TopBar.h` there still declares `masterSignalLabel_{"", "Signal:"}`, both sliders still use `NoTextBox`, there is no `kAccentMagenta` reference, and `tests/test_master_signal_link.cpp` there still has only the original 8 test cases. The actual fix lives only in worktree `wf_23099956-236-2`. That worktree is what this review is against, per the builder's own screenshots and the 6-new-test-case claim, both of which match only that tree.

## Verdict: PASS

## MUST (blocks showing Boris)
None found.

## SHOULD
1. **The "layout at minimum window width (1280)" test doesn't actually verify the pair is visible — and the pair genuinely disappears at 1280.** Walking `TopBar::resized()`'s fixed pixel budget for the right-hand chrome (dspLabel 55 + fpsLabel 45 + gap 6 + displaySelector 100 + outputLabel 42 + gap 4 + masterLevelSlider 90 + masterLabel ~42 + gap 4 + masterSignalSlider 90 + masterSignalLabel ~85) totals ~560 px, against only ~105 px actually free in the right section at a 1280 px window (left-side chrome alone consumes ~1167 px of the ~1272 px available). `Rectangle::removeFromRight()` degrades by donating the *whole* remaining area once the budget runs out and leaving 0-width siblings — so at 1280, Master/Master Signal (and displaySelector/outputLabel before them) collapse to literally 0 px wide, i.e. invisible and ungrabbable, not just tight. The new "layout at the minimum window width (1280)" test (`test_master_signal_link.cpp:329`) only asserts non-*intersection* between the group's rectangles; a 0-width rectangle trivially never intersects anything, so the test passes green while the controls it's supposedly guarding are gone. This is a pre-existing TopBar over-crowding problem (already true before this fix, and already flagged by a prior critic per the test's own comment referencing `critic-polish-ux-r1.md`) — so it is not this fix's fault that 1280 doesn't fit. But this fix does add ~20 px of width per slider (70→90) plus two 35 px readouts, making the deficit measurably worse, and its own test's name overclaims coverage it doesn't provide. Recommend either widening the test's assertions to also require `sigSlider.getWidth() > 0` / `masterSlider.getWidth() > 0` at 1280 (so it fails honestly until the surrounding overcrowding is fixed), or renaming/annotating the test so "layout at minimum window width" doesn't imply the pair remains usable there.

## NICE
1. Magenta vs. cyan is a reasonable, theme-consistent (`kAccentMagenta` is already used in `Knob`, `SignalStrip`, `BindingOverlay`, etc., not a one-off) differentiator, and it's backed by a second independent cue (different label text), so a performer isn't relying on color alone. Worth double-checking under actual stage lighting/at a glance from a few feet away that magenta vs. cyan doesn't wash out — a low-cost thing to eyeball once in the live app (Tier 4), not a blocker.

## Verified good (checked directly against source, not inference)
- Label text is the full word "Master Signal:" (`TopBar.h:128`), not the bare "Signal:" that caused the prior FAIL — confirmed both in the label member default and in the widened-`GlyphArrangement` layout call at `TopBar.cpp:639-645` (uses real text width instead of the old guessed magic number).
- Both faders now use `TextBoxRight` with width 35, matching the exact convention `Fade` already uses in this bar (`fadeSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 35, 20)` at `TopBar.cpp:187`) — the readout convention is consistent across the bar, not a one-off for this pair.
- Distinct accent: `masterSignalSlider_` gets `kAccentMagenta` on `thumbColourId` (`TopBar.cpp:220-221`); `masterLevelSlider_` keeps the default (cyan) — visually and structurally distinct, confirmed via `sigSlider.findColour(...) != masterSlider.findColour(...)` in the new test at line 254-257.
- Both faders carry non-empty, distinct, plain-words tooltips (`kMasterSignalTooltip` / `kMasterTooltip`, `TopBar.cpp:198-203, 239-240`) — "Master" reads as composition output level, "Master Signal" explains the 0%-still-pulses nuance. No abbreviations, matching CLAUDE.md's whole-words rule.
- Both sliders are `ResettableSlider` (`TopBar.h:129,134`) with `setDefaultValue(1.0)` called on both (`TopBar.cpp:212, 249`) — compliant with the project's mandatory slider pattern.
- 6 new Catch2 cases actually exist and match the builder's description (label text, dual readout presence, accent-colour distinctness, tooltip presence/content/distinctness, layout at 1728, layout at 1280) — `tests/test_master_signal_link.cpp:204-360`.
- The "Deck Loa[d]" button clipping and the Dashboard/"No clip selected" positioning visible in both `before-full.png` and `after-full.png` are identical in both screenshots (same crop, same clipped state) — confirmed pre-existing, not introduced by this fix, correctly out of scope for this pair.
- Before/after crop screenshots pixel-match the claimed fix: "before" shows bare "Signal:" / "Master:", identical cyan thumbs, no readout; "after" shows "Master Signal:" in magenta with "1.00", "Master:" in cyan with "1.00", "Output: Off" unclipped.

## Anything outside this pair
- TopBar general density / 1280 min-width overcrowding (see SHOULD #1) — pre-existing, out of scope for this pair's fix itself, though this fix's own test claims coverage there that it doesn't provide.
- "Deck Loa" clipping and Dashboard/"No clip selected" layout — pre-existing per identical before/after screenshots, out of scope.
