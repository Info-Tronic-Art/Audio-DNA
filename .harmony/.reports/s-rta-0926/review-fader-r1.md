# Reviewer Verdict — fader r1
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES

## Scope
Diff: `git -C .claude/worktrees/wf_23099956-236-2 diff main...worktree-wf_23099956-236-2`
Files: `src/ui/TopBar.cpp`, `src/ui/TopBar.h`, `tests/test_master_signal_link.cpp`.
Read-only review; no build/run performed (per task instruction). Font-width
claims below are derived from a standalone Python/PIL measurement of system
fonts, not from the actual JUCE `GlyphArrangement` — labeled INFERRED, not
VERIFIED, and flagged for Tester to confirm by actually running the test.

## FILE: src/ui/TopBar.cpp / TopBar.h

[OK] Spec fidelity — MUST #1 (label/readout/accent) is genuinely present at
this worktree HEAD: `masterSignalLabel_{"", "Master Signal:"}` (TopBar.h:~121),
`TextBoxRight` readouts on both `masterSignalSlider_` and `masterLevelSlider_`
(TopBar.cpp:217,253), and `kAccentMagenta` thumb colour (TopBar.cpp:220-221).
`kAccentMagenta` is confirmed pre-existing (`LookAndFeel.h:14`, already used
in `SignalStrip.cpp`, `Knob.cpp`, `MidiLearnOverlay.cpp`, etc.) — not a new
palette entry, matches the comment's claim.

[OK] Pattern reuse — `setTextBoxStyle(TextBoxRight, false, 35, 20)` is
byte-identical to `fadeSlider_`'s own setup (TopBar.cpp:187), so the "matches
Fade's own readout" claim is verified, not just asserted.

[OK] LookAndFeel verification — `AudioDNALookAndFeel::drawLinearSlider`
(LookAndFeel.cpp:176-217) paints BOTH the filled-track portion and the thumb
from `Slider::thumbColourId`. Setting only `thumbColourId` therefore recolors
both track-fill and thumb, so the header comment "magenta thumb/track accent"
is accurate — this is not a case of an overclaiming comment (dimension 7).

[OK] Faders' model/connection code (`onValueChange`, `onDragStart`/`onDragEnd`,
`onResetToDefault`, `syncMasterFromComposition`/`syncMasterSignalFromComposition`,
the 15 Hz timer) is untouched by this diff — confirmed by diff inspection, only
`setTextBoxStyle`/`setColour`/`setTooltip`/label text/layout lines changed.
No feedback loop introduced: syncs use `juce::dontSendNotification` (unchanged),
same pattern as before this diff. REST (`/api/set_master_signal`), OSC
(`/audiodna/signal`), composition load, and replay restore all write
`composition_.masterSignal`/`masterOpacity` and are picked up by the unchanged
15 Hz `timerCallback → syncMaster(Signal)FromComposition` path — readout
correctness on those paths is inherited, not re-verified by new code, and I
did not find a path that bypasses the timer.

[OK] `ResettableSlider::setDefaultValue(1.0)` unchanged for both sliders.

[OK] No unrelated changes — diff is exactly the 3 files needed for this fix;
no drive-by edits elsewhere.

[ISSUE — BLOCKING, INFERRED] Layout margin at 1728px is razor-thin, possibly
negative, contradicting the "fits, verified via screenshot" claim.
Quantitative derivation (all constants read directly from
`TopBar::resized()`, TopBar.cpp:538-648):

  - `getLocalBounds().reduced(4,2)` on a 1728-wide bar → area width = 1720.
  - Sum of every `removeFromLeft(...)` call before `rightSection` (audio
    source, transport, beat wheel, bar/phrase, tempo, tap/resync/manual/link,
    5 multiplier buttons, quantize, fade) = **1167px** (recomputed twice,
    itemized in scratch calc, `python3 -c "print(sum([38,90,4,30,70,6,2,24,1,24,1,24,6,2,26,2,44,2,50,64,32,2,50,2,80,2,50,2,26,1,26,1,26,1,26,1,26,6,55,100,6,30,100,6]))"` → 1167).
  - `rightSection` width = 1720 − 1167 = **553px**.
  - `dsp(55)+fps(45)+gap(6)+displaySelector(100)+outputLabel(42)+gap(4)
    +masterLevelSlider(90)+masterLabel(42)+gap(4)+masterSignalSlider(90)`
    = **478px**, consumed by `removeFromRight` calls BEFORE
    `masterSignalLabel_` is sized (TopBar.cpp:629-638).
  - Remaining budget for `masterSignalLabel_` = 553 − 478 = **75px**.
  - `sigLabelW = GlyphArrangement::getStringWidthInt(font(11pt), "Master Signal:") + 6`.
    A PIL/system-font approximation at 11px with HelveticaNeue/SFNS gives
    "Master Signal:" ≈ 65-70px text width → sigLabelW ≈ **71-76px**.
  - 75px available vs. ≈71-76px required is a **0 to +4px margin** — inside
    my measurement's own error bars. `Rectangle::removeFromRight` silently
    CLAMPS to whatever remains rather than erroring, so if the real JUCE
    metric is even slightly larger than my estimate, `masterSignalLabel_`
    gets a narrower rectangle than `textW`, i.e. the label IS truncated at
    the exact width (1728) the builder cites as the real-world maximized
    width and the width the "AFTER screenshot" was reportedly taken at.

  The new test (`"layout at 1728: ..."`) is NOT tautological/vacuous — it
  measures the same quantity via the same API the production code uses, so
  it would genuinely catch a real shortfall — but the margin is close enough
  to zero that (a) I cannot rule out a present-day failure without running
  it, and (b) even if it passes today, there is effectively no safety
  margin: a 1-2px difference from font hinting/OS version/display scaling
  could flip it silently, and no test would flag the loss of margin itself
  (only a hard failure). This is exactly the "can it pass vacuously" /
  "fits at 1728" check the task asked me to make, and the answer is:
  not vacuous, but unverified and margin-free.

  Action requested: Tester should actually run
  `ctest -R master_signal_link` (or the standalone binary) and report the
  real pass/fail + the real measured widths (e.g. temporarily log
  `sigLabelW` and `rightSection` remaining width). If it fails, or margin is
  under ~10-15px, recommend freeing width elsewhere in the same
  `rightSection` chain (e.g. trim `masterLevelSlider_`/`masterSignalSlider_`
  track width from 90 back toward 80 while keeping the 35px textbox, or
  trim the `+6` padding in `sigLabelW`) rather than shrinking the label text
  itself (CLAUDE.md's whole-word UI rule forbids abbreviating "Master
  Signal:").

[COMMENT — non-blocking] The 1280px test ("Signal/Master group never
overlaps itself") only asserts a structural invariant
(`removeFromRight`/`removeFromLeft` on a shared `Rectangle` never produces
overlapping siblings, by construction of the algorithm, regardless of
whether any individual widget's width has collapsed toward zero). This holds
true even in a fully-degenerate layout, so it does not verify anything about
visibility/usability at 1280 — but the test's own comment says so explicitly
and cites the prior critic note that full-width-fit at 1280 is pre-existing
and out of this fix's fence. Disclosed, not silent — acceptable as written,
but worth Harmony/Builder tracking as a known gap rather than closed.

## FILE: tests/test_master_signal_link.cpp

[OK] 6 new `TEST_CASE`s added (label text, dual readout, distinct accent,
tooltip content/difference, 1728 layout, 1280 layout) — matches the
builder's claimed count (14 total, up from 8).
[OK] Each new case follows the existing file's `ScopedJuceInitialiser_GUI` +
`Composition`/`initDefault()` + `TopBar` construction pattern — consistent
with the pre-existing 8 cases, no new test infra invented.
[OK] Assertions use real JUCE Slider/Label public API
(`getTextBoxPosition`, `getTextBoxWidth`, `findColour`, `getTooltip`) —
nothing exercises private state.
[ISSUE] Same file — the 1728-width test is the vehicle for the BLOCKING
finding above; the test itself is well-constructed, the risk is in what it
measures against (a razor-thin production budget), not in the test code.

## SLIM
No new files, no dead code introduced. `kMasterSignalTooltip`/`kMasterTooltip`
as constructor-local `static const juce::String` avoid duplicating the
tooltip string between label and slider — reasonable DRY, not
over-engineering (single short-lived local, not a new shared header/class).

## SUMMARY
3 files reviewed, 1 blocking issue (unverified/razor-thin 1728px layout
margin for the whole-word label — INFERRED via font-width approximation,
needs Tester's actual-run confirmation), 1 non-blocking comment (1280px test
is structural-invariant-only, disclosed honestly). Both critic MUST items
(label/readout/accent; 6 new tests) are VERIFIED present in the diff and
correctly implemented against the codebase's existing conventions. The
faders' model/connection/sync code is unchanged and behaviorally identical
to before this diff.

METADATA: reviewer=reviewer-fader-r1, builder_packet=fader,
date=2026-09-26T00:00:00Z, worktree=wf_23099956-236-2,
head=6583441
