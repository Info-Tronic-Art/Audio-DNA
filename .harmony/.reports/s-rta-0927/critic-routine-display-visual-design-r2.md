# Visual-design critic — routine display, fix round 2 (s-rta-0927)

Worktree: `.claude/worktrees/rta0927-w3`, branch `lane/routine-display-0927`,
base `4b0c39a`, head `b6e49fa`. Read via `git diff`/`git show` only, plus the
30 listed PNGs (Read). Judged against the approved mockup
(`.harmony/.reports/s-rta-0926b/routine-ux/mockup.html`) and the app style
contract (no radius, no shadows, no gradients, 1px `#1a1a1a` hairlines, flush).

VERDICT: PASS. No MUST-level violation of the mockup's structure or the app's
flat-square-hairline style found in the diff or in any of the 30 screenshots.
The one open design question the task flags — chartreuse (`kRoutineCue`,
`#b4ff2e`) standing in for the mockup's cyan band/fader/inspector cue — reads
legibly and consistently everywhere it appears, and it fixes a real ambiguity
(cyan is already the app-wide mapped-knob accent) without introducing a new
one.

## What changed since round 1

`kRoutineCue` (`src/ui/LookAndFeel.h:36`, `0xffb4ff2e`) replaces the accent
cyan in the four places the routine "hand" shows on a control: the layer
strip's routine bands (name text, `LayerStrip.cpp:827`), the opacity
fader's fill while a routine's lane-rank hand grips it (`LayerStrip.cpp:768`,
routed through `OpacitySliderLookAndFeel::drawLinearSlider`'s
`trackColourId` override), and `UniversalParamControl`'s value digits /
slider / "ROUTINE" hint text (`UniversalParamControl.cpp:189-347`). The
routine pad itself (`RoutinePad.cpp`) was never touched — its frame/sweep/
progress-tick teal is untouched and still matches the mockup's
`--teal:#4a9a8a` exactly, so the two-tier read (teal = the routine's own
timing chrome; chartreuse = "a routine's hand is on this control right now")
survives the recolor with only the identity half of it moved off cyan.

## Legibility / consistency check (the judgment call the task asks for)

Compared `crop-fixround-before-*` (cyan) against `crop-after-*` (chartreuse)
directly:

- **Inspector** (`crop-after-03-playing-inspector-rows.png` vs.
  `crop-fixround-before-03-playing-inspector-rows.png`): chartreuse "ROUTINE"
  label, the `0.57` digits, and the slider fill all read at the same
  contrast the cyan version did against the dark navy panel — no legibility
  loss. The gain is real: in the cyan "before" shot, the routine-held Master
  row and the ordinary cyan "Additive" dropdown / cyan knob rings a few
  pixels away are the same color, so a glance can't tell "a routine has this"
  from "this is just an app-cyan control." The chartreuse "after" shot
  removes that ambiguity — it's the only chartreuse thing on screen.
- **Layer strip band + V-fader** (`crop-after-03-playing-L3-strip-band.png`,
  `after-13-restart-pending.png`'s L1 strip): chartreuse band name and fader
  fill sit on the black 70%-alpha band scrim and the dark slider track
  respectively; both have strong contrast and don't fight the teal
  progress-hairline underneath the band or the teal S/K sliders beside it.
- **Two stacked bands** (`crop-after-04-two-on-one-L1-strip-bands.png`):
  "Build" and "Drop," both chartreuse, over the picture thumbnail — both
  names stay legible against the busy black-and-white test card, the
  `#1a1a1a` divider between them reads clearly, matches the mockup's
  `.band-divider{border-top:1px solid #1a1a1a}` exactly.
- **Warning "!"** (`after-09-warning.png` pad 3 "Wash",
  `crop-after-09-warning-routines-row.png`): `kWarning = 0xffcc3333`
  (`RoutinePad.h:44`) is a byte-for-byte match to the mockup's
  `--warn:#cc3333`, and is unrelated to the chartreuse change — still reads
  as a distinct red, no confusion with the routine cue.
- **Pad menu** (`after-10-pad-menu-snapshot.png`,
  `crop-after-10-pad-menu.png`): "Delete routine" painted in
  `kMeterRed` via `addColouredItem`, square corners, flat fill, 1px
  divider — consistent with the rest of the app's popup menus; the
  `LookAndFeel::drawPopupMenuItem` change that makes an explicit item color
  survive highlighting (`LookAndFeel.cpp:387-391`) is the only reason this
  works and is a minimal, surgical addition.

Net: chartreuse is the better choice for the app's actual palette, not just
an acceptable substitute — it resolves a real collision the mockup (drawn
before the cyan-for-mapped-knobs convention was load-bearing everywhere)
didn't anticipate. This is a taste call and Boris owns it, but on the
legibility/consistency axis the task asks the critic to judge, chartreuse
wins over cyan.

## Style-contract check (no radius / no shadow / no gradient / 1px `#1a1a1a` hairlines / flush)

- `git diff 4b0c39a b6e49fa -- src/ui/` grep for `rounded|gradient|shadow`:
  **zero hits**. No `drawRoundedRectangle`, no `ColourGradient`, no
  `DropShadow` anywhere in the changed UI code.
- Hairline colors introduced/used: `RoutinePad::kHairline = 0xff1a1a1a`
  (`RoutinePad.h:40`), `LayerStrip::kBtnBorder = 0xff1a1a1a`
  (`LayerStrip.h:153`) — both match the mockup's `--divider`/`--strip-border`
  intent (`#1a1a1a`/`#3a3a3a` split the same way the mockup splits pad
  borders from menu dividers).
- Every screenshot (idle, waiting, playing, two-on-one, off-deck, removed,
  layer-X, empty bank, warning, pad menu, record tab, fader-follow,
  restart-pending) shows square corners throughout — pads, bands, sliders,
  menu, corner note — flush against the layer strip / column grid exactly as
  the mockup lays it out (ROUTINES row directly above the column-number row,
  pad N over column N, band pinned to the top of the clip thumbnail).

## Findings

None at MUST. No SHOULD or NIT findings survive scrutiny either — the one
candidate nit (chartreuse is visually the brightest/most saturated element
on most of these screens) is the intended behavior of an attention cue for
"a routine has its hand on this right now," not a defect, and the task
explicitly reserves that call for Boris.
