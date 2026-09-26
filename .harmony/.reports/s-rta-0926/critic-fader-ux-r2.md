# Critic Report — TopBar "Master Signal" / "Master" faders, round 2

**Verdict: FAIL**

## Verification method

The builder's summary claims both MUST items from round 1 were fixed at
"commit 6583441... made before this dispatch," on "this worktree branch."
I checked the actual worktree this dispatch is scoped to review:

```
/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_23099956-236-1
HEAD: 138af1e (branch worktree-wf_23099956-236-1)
```

`git worktree list` shows commit `6583441` — the one described in the
builder's own commit message ("TopBar Master Signal fader now reads
'Master Signal', shows a value, and gets its own accent colour") — lives
on a **different, sibling worktree**: `wf_23099956-236-2`. It is not an
ancestor of `wf_23099956-236-1`'s HEAD. The builder's claim that "the
critic's MUST #1 was checking `main`... the fix was never absent from the
worktree" is false for the worktree this round is actually reviewing: the
fix is absent from it.

Confirmed directly by reading `src/ui/TopBar.h`/`.cpp` at HEAD `138af1e` in
that worktree:

- `src/ui/TopBar.h:118` — `juce::Label masterSignalLabel_{"", "Signal:"};`
  Still bare "Signal:", not "Master Signal:". No `setText()` override exists
  anywhere in `TopBar.cpp`.
- `src/ui/TopBar.cpp:206` — `masterSignalSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);`
  No numeric readout on the Master Signal fader.
- `src/ui/TopBar.cpp:235` — `masterLevelSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);`
  No numeric readout on the Master fader either.
- `grep -n kAccentMagenta src/ui/TopBar.cpp` — zero matches. No colour
  override is applied to `masterSignalSlider_`; it uses the same default
  `ResettableSlider` styling as `masterLevelSlider_`, i.e. visually
  identical faders.

All three defects that failed round 1 are still present, unchanged, in the
worktree under review.

## On the submitted screenshots

`after-crop-topbar-mastersignal.png` shows "Master Signal:" with a magenta
thumb and a "1.00" readout, and "Master:" with a cyan thumb and a "1.00"
readout — exactly matching commit `6583441`'s description. That commit is
not in this worktree's history, so these screenshots cannot have been
rendered from this worktree's build. Either the app binary launched for the
capture was built from `wf_23099956-236-2` (or `main`, which also carries
some form of the fix per `git worktree list`), or the wrong artifact was
attached to this dispatch. Either way, the evidence does not verify the
code that will actually ship from this branch.

## MUST (blocks showing Boris)

1. **Label still reads bare "Signal:", not "Master Signal:"** — `src/ui/TopBar.h:118`.
   Fails the project's own "always display whole words" UI rule and repeats
   round 1's exact finding: at a glance mid-performance, "Signal:" next to
   "Master:" reads as two views of the same thing, not two different
   controls with different jobs (output level vs. how hard signals pull).

2. **Neither fader shows a value** — `masterSignalSlider_` and
   `masterLevelSlider_` are both `NoTextBox` (`TopBar.cpp:206`, `:235`).
   Every other comparable control in this bar (`Fade:`) has a
   `TextBoxRight` readout. A performer riding these faders live has no way
   to confirm the exact value they're at (e.g. "am I at 0.98 or 1.00
   Master Signal") without hovering for a tooltip — a real risk when
   recovering a fader position mid-show.

3. **The two faders are visually identical** — no accent-colour
   differentiation exists in `TopBar.cpp` for `masterSignalSlider_`
   (confirmed: zero `kAccentMagenta` references in the file). A performer
   can grab the wrong one under stage lighting/low light without looking
   twice — this is the exact "wrong fader grabbed live" risk round 1 called
   out, and it is unaddressed on this branch.

## SHOULD

- Once fixed, confirm the "Master Signal:" label width is sized to its
  actual text (not a fixed guessed pixel width) so it can't silently
  truncate at other window sizes/DPI scales — this was called out as a fix
  in the sibling worktree's commit message; verify it here too once ported.
- Add a plain-words tooltip on both faders distinguishing "output level of
  the whole composition" vs. "how strongly signal connections move their
  targets" (per this task's own framing) so a performer unsure which is
  which mid-show has a fallback.

## NICE

- Consider a very short static caption difference beyond colour (e.g. a
  small icon or bracket) for colour-blind performers, since colour is
  currently the only differentiator planned between the two faders.

## Pre-existing, out of scope

- Everything else in the captured TopBar (Fade slider styling, Output
  selector, FPS/DSP readouts, deck/inspector/browser panels in
  `after-full.png`) is unrelated to this pair and out of scope for this
  review.
