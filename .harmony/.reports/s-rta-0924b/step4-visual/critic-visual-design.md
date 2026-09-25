# Visual design critique — Record panel (Step 4)

VERDICT: PASS-WITH-FIXES. The colour system is correctly and consistently
implemented; the one real defect is a text-truncation bug that violates the
project's own "always display whole words" UI rule, repeated at two distinct
sites in every single screenshot supplied.

## Colour system — verified against code, not just eyeballed

`src/ui/RecordPanel.cpp:222-226` maps `RecordPanelView::Tone` to colour
directly: `Tone::Recording → kMeterRed`, `Tone::Playing → kMeterGreen`,
else → `kTextPrimary` (white). `warningLabel_` is hard-set to `kMeterYellow`
(line 112), `noticeLabel_` to `kAccentCyan` (line 116). The "Stop Recording"
button uses `kMeterRed` (line 177), "Stop Playback" uses `kMeterGreen`
(line 181) — the same tokens the status text uses. This is a single,
disciplined palette, not ad hoc per-state colouring, and the screenshots
confirm it renders as designed: red "Recording 0:15 …" text and red Stop
Recording button together (crop-armed-or-recording.png,
crop-recording.png); green "Playing 0:02 / 0:00 …" text and green Stop
Playback button together (crop-playing-with-audio.png); yellow "Replaying
with the take's audio…" warning under both (crop-overdub.png); cyan
`perf/…` notices distinct from all three (crop-notice-no-take-loaded.png,
crop-idle-restored.png, crop-after-stopplay-during-overdub.png). No
colour collision was found across all 10 crops — recording-red,
playing-green, warning-yellow and notice-cyan never overlap or get reused
for a different meaning. This is a clean, legible implementation of the
brief's colour spec.

Checkboxes ("Record audio", "Play with audio") are left at the
LookAndFeel's default cyan-outline style in every state, never recoloured
red or green (confirmed: no `setColour` call on either toggle in
RecordPanel.cpp). That's the right call — they are inputs, not status
readouts, and giving them state colour would compete with the status line.

## Disabled dimming — verified uniform

Every disabled control goes through one alpha constant: `button.setAlpha
(spec.enabled ? 1.0f : kDisabledAlpha)` (line 173) and the identical
pattern for `recordAudioToggle_`, `playWithAudioToggle_`, `nameEditor_`
(lines 211, 215, 220). One knob, one look. Visually this holds up:
"Repair Audio" reads at the same dim grey whether it's disabled because
nothing is loaded (crop-idle-nothing-loaded.png) or because audio isn't
ready during recording (crop-recording.png). No stray brighter-grey or
different-opacity outlier was found in any crop.

## MUST — placeholder text truncated to a dangling word fragment

Every one of the 10 crops shows the take-name field as `Name (blank =
date and t...`. The full placeholder is "Name (blank = date and time)"
(`RecordPanel.cpp:92`), set via `setTextToShowWhenEmpty`. `resized()`
(lines 258-263) gives the name field whatever's left of Row C after two
fixed-width toggles (130 + 110 + 4px gap), and that remainder isn't wide
enough for the string at the app's default label font — it clips mid-word
("t...") rather than wrapping, shrinking, or reflowing the row. This is
not a one-off rendering glitch — it is present identically in every state,
meaning the row-C layout math is simply undersized for its content. It
also directly contradicts this project's own stated UI rule ("Always
display whole words in the UI... never use abbreviations") — a
mid-word ellipsis is worse than an abbreviation; it looks broken, not
stylistically terse, and a non-technical live performer has no way to
recover the word "time" without hovering for the tooltip.

Fix: either widen Row C's allotment to the name field (e.g. stack the two
toggles into two narrower rows so the name field gets more horizontal
room), reduce the placeholder string to something that already fits at
current width without a dangling fragment (JUCE will still ellipsize
cleanly at a word boundary if the string is short enough), or use a
smaller/lighter font specifically for the placeholder. Any of these is a
one-line-of-layout-code fix; the current state ships a visibly truncated
label in 100% of observed states.

## SHOULD — same truncation defect one level up, in the tab bar every crop also shows

Not in RecordPanel.cpp itself, but present in every supplied screenshot's
top strip: the browser tab reads "Comp/Deck" — the actual button label is
`"Comp/Decks"` (`BrowserPanel.h:50`), and the trailing "s" is clipped off
by the tab button's fixed width. Same defect class as the name-field
truncation (content wider than its container, silently clipped rather than
sized to fit), and it sits directly above the Record panel in every state
Harmony captured, so a viewer's first visual impression of this screen is
two clipped labels, not one. I flag it SHOULD rather than MUST only
because the file is out of this dispatch's named source set
(`BrowserPanel.h`, not `RecordPanel.*`) — but it's visible, real, and the
same root cause (row/column budget not checked against actual string
width) so it's worth fixing in the same pass rather than filed separately
and forgotten.

## Hierarchy, spacing, alignment — no defects found

Row order (lifecycle buttons → housekeeping buttons → name+toggles →
status/warning/notice stack → format/takes-root) is consistent across all
10 states; nothing shifts vertically when text length or button count
changes, because `resized()` uses fixed-height row removal
(`kControlHeight`, `kRowSpacing`) rather than content-driven sizing. Text
that grows long (e.g. the combined overdub status line "Recording over
s4gate 0:33 · 0 lanes · 0 moves — Playing 0:04 / 0:00 · unresolved: 0")
stays on one line and doesn't collide with the warning line beneath it in
any crop — margins hold under the longest observed string. Button widths
in Row B (100/110/100) are uneven but track label length sensibly ("Show
in Finder" is the longest label and gets the widest slot); this isn't a
grid misalignment, it's intentional per-label sizing and reads fine.

## Strongest counterargument to this verdict

One could argue PASS outright: the truncation is cosmetic, the tooltip on
the field states the full text, and every functional/semantic aspect of
the colour and dimming system — the actual "is this legible and
consistent" brief — passes cleanly with code-level confirmation, not just
a screenshot guess. A pure colour-system critic could reasonably call this
zero-defect. I hold PASS-WITH-FIXES instead because the truncation isn't a
single rare edge case — it is the same defect, unconditionally, in all 10
states, of a field literally named "Name" that a live performer types into
under time pressure without necessarily discovering the tooltip, and it
directly violates a rule this project's own CLAUDE.md states in capital
terms. A visual-design seat that let a 100%-reproducible, rule-violating
truncation through as merely cosmetic would be underselling its mandate.
