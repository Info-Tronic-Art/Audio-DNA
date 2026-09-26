# Critic — Fader Logic, Round 2
Item: TopBar "Master" / "Master Signal" pair
Verdict: **FAIL**

## Method
Read the screenshots pixel-decoded (Read tool) and read the actual source at the
worktree HEAD the builder says was already fixed (`28b1f9c`, branch
`worktree-wf_23099956-236-1`), not a rendered claim. Re-derived every builder
assertion against `src/ui/TopBar.cpp`, `src/ui/TopBar.h`, `src/ui/LookAndFeel.cpp`,
and `tests/test_master_signal_link.cpp` directly, rather than trusting the summary
or the screenshots alone.

## MUST (blocks showing Boris)

1. **The label is still bare "Signal:" in the actual buildable source — round-1
   defect NOT fixed on this branch.**
   `src/ui/TopBar.h:118`:
   ```cpp
   juce::Label masterSignalLabel_{"", "Signal:"};
   ```
   Grepped every `masterSignalLabel_` reference in `TopBar.cpp` (lines 197-199,
   614-618) — the label's text is set once, in the constructor, to `"Signal:"`,
   and is never changed by `setText()` anywhere in the file. Whatever Boris
   builds from this HEAD renders `"Signal:"` next to the slider, not
   `"Master Signal:"`. This is the exact defect Round 1 FAILed on, still present
   in the source the builder points to.

2. **The two faders are still visually identical — no magenta accent exists in
   source.** `masterSignalSlider_` (TopBar.cpp:201-224) never calls
   `setColour(juce::Slider::thumbColourId, ...)` or any `kAccentMagenta`
   reference — confirmed by `grep kAccentMagenta src/ui/TopBar.cpp src/ui/TopBar.h`
   returning nothing. `LookAndFeel.cpp:18` sets the global default
   `thumbColourId` to `kAccentCyan` for every slider app-wide, and TopBar never
   overrides it for the Signal fader. So Master Signal renders in the same cyan
   as Master, Fade, and every other slider in the app — the identical-fader
   confusion Round 1 flagged is unresolved.

3. **Neither fader has a value readout in source — both explicitly disable the
   textbox.**
   ```cpp
   masterSignalSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);  // line 206
   masterLevelSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);   // line 235
   ```
   No other label or component in `TopBar.cpp` writes a numeric value string for
   either fader (grepped all `setText` calls in the file — only tempo, tracker
   state, FPS/DSP, and beat-wheel text are written). A performer looking at a
   build of this HEAD sees no "1.00" next to either fader.

4. **The screenshots do not correspond to this HEAD's source, so they cannot be
   used as evidence the MUSTs are resolved.** `after-crop-topbar-mastersignal.png`
   shows `Master Signal:` (full label), a magenta thumb, and `1.00` readouts on
   both faders — none of which the current `TopBar.cpp`/`TopBar.h`/`LookAndFeel.cpp`
   can produce (see #1-#3). `git log --oneline -- src/ui/TopBar.cpp src/ui/TopBar.h`
   shows the last commit touching these files is `e589efb` (s-rta-0925 mastersignal
   Step 1), predating this round entirely, and `git status` on the worktree is
   clean with no local modifications. Whatever binary produced these screenshots
   was not built from this worktree's checked-out source — the screenshot
   evidence is stale/mismatched, not proof of a fix.

5. **Test-count claim does not match the file.** Builder states
   "tests/test_master_signal_link.cpp has the 6 new Catch2 cases (14 total, up
   from 8)". `grep -c TEST_CASE tests/test_master_signal_link.cpp` returns **8**,
   not 14, and lists the same 8 case names that would have produced the
   original 8. No new cases for the label text or the color/value-readout fix
   exist in this file. This corroborates #1-#3: the fix the tests would need to
   cover was never made.

## SHOULD
- Once the label/color/readout code is actually present, add a Catch2 (or
  screenshot-diff) assertion that pins the label text to the full
  `"Master Signal:"` string and the thumb colour to something other than
  `kAccentCyan`, so a future regression back to `"Signal:"`/cyan fails CI
  instead of requiring a screenshot re-review each round.
- Tie screenshot capture to the exact commit hash it was built from (e.g. stamp
  it in the filename or an adjacent `.txt`), so a future round can verify
  evidence-to-source correspondence without re-deriving it from git log.

## NICE
- None beyond the above; the pair's intended visual language (label, distinct
  accent colour, numeric readout) is reasonable and matches the rest of the app
  once actually implemented.

## Pre-existing, out of scope
- Everything else visible in `after-full.png` (decks, browser panel, clip
  dashboard, BPM/Routing/Oscillators tabs) — unrelated to this pair.

## Judge checklist
- Legibility at a glance mid-performance: **FAIL** — bare "Signal:" is
  ambiguous against "Master Signal:" / "Master".
- Whole words: **FAIL** — "Signal:" is not the full control name; it drops
  "Master", which is the word that disambiguates it from the composition-level
  Master fader right next to it.
- Can a performer grab the wrong one: **FAIL** — same label style/position
  pattern, identical cyan thumb, no value readout to catch a wrong grab before
  it changes the mix.
- Value readouts consistent with the app's other controls: **FAIL** — both
  faders have `NoTextBox`; nothing else in TopBar with a value-bearing slider
  (e.g. Fade, which does use `TextBoxRight`) is treated this way.
- Fits without clipping/overlap: not verifiable as a fix since the labelled
  claims aren't in source; the 42px `masterSignalLabel_` bounds (TopBar.cpp:618)
  is sized for "Signal:" at 11pt, not "Master Signal:" — if the text is
  corrected without widening the bounds, expect truncation/clipping as a new
  regression.
- Colour accent consistent with theme: **FAIL** — no accent differentiation
  exists at all in source (see MUST #2).
