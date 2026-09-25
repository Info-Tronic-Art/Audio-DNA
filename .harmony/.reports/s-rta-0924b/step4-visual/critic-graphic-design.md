# Graphic-Design Critique — Record Panel (Step 4)

VERDICT: PASS-WITH-FIXES — the panel is legible and mostly consistent, but has
one measurable contrast failure (red "Stop Recording" button text), one
verified text-clipping bug (name placeholder), and a text-color inconsistency
between the two "Stop" buttons that a non-technical performer will notice
under stage lighting.

## Type sizes and weights

- Button labels are bold; status/warning/notice labels are regular weight at
  11pt/10pt/10pt (`src/ui/RecordPanel.cpp:107,111,115`). The three-line stack
  (status, warning, notice) is internally consistent in size -- no flag there.
- Tab labels ("Files/FX/Sources/.../Record/MilkDrop", sheet.png top row) sit
  well above the panel's own 10-11pt labels in visual weight (bold, larger),
  correctly establishing hierarchy: tabs over buttons over status text. Good.
- The truncated placeholder "Name (blank = date and t..." (all 10 states,
  e.g. crop-idle-nothing-loaded.png) is a real clipping bug, not a screenshot
  artifact: the full string is "Name (blank = date and time)"
  (`src/ui/RecordPanel.cpp:92`), and `nameEditor_` is sized to `rowC` minus
  130px (Play with Audio toggle) minus 110px (Record Audio toggle) minus 4px
  gutter (`RecordPanel.cpp:260-263`) -- the remaining width is too narrow for
  the full placeholder at this font size, so JUCE's own ellipsis clips it. A
  non-technical performer reading "date and t..." cannot tell what the field
  does from the UI text alone.

## Colour contrast (WCAG estimate, panel background hex 1a1a1a, `RecordPanel.cpp:233`)

Computed relative-luminance contrast ratios (sRGB, standard WCAG formula):

- Notice text (accent cyan, hex 00e5ff) on the panel background: roughly
  11.3 to 1 -- passes AAA.
- Warning text (meter yellow, hex ffea00) on the panel background: roughly
  14.1 to 1 -- passes AAA.
- Status text in Recording tone (meter red, hex ff1744) on the panel
  background: roughly 4.5 to 1 -- right at the AA threshold for the 11pt
  status label (`RecordPanel.cpp:107`, `223-226`); any anti-aliasing loss
  pushes it under. Borderline, not a hard failure, but the tightest margin
  on the panel.
- The "Stop Recording" button: text colour (text primary, hex e0e0e0) on
  button colour (meter red, hex ff1744) (`RecordPanel.cpp:177-178`) computes
  to roughly 2.9 to 1 -- this fails WCAG AA for normal text (4.5 to 1
  required) and also fails the large/bold-text minimum (3 to 1). This is
  visible in the screenshots themselves: in crop-recording.png and
  crop-armed-or-recording.png the "Stop Recording" label has a soft, hazy
  look against the red fill, versus the crisp "Stop Playback" label in
  crop-overdub.png.
- The "Stop Playback" button, for comparison: text colour (background hex
  1a1a2e) on button colour (meter green, hex 00e676) (`RecordPanel.cpp:181-182`)
  computes to roughly 10.2 to 1 -- excellent, dark text on a light-saturated
  fill.

This is the one finding I would call load-bearing: the two "Stop" buttons use
opposite text-color strategies (light text on red vs. dark text on green) and
only one of them clears WCAG AA. The fix is one line -- give the red button
dark text (or darken the red / lighten it), matching the pattern the green
button already uses correctly -- not a redesign.

## Button sizing and consistency

- Row A (Record Take / Stop Recording, Play Take): fixed 120px each
  (`RecordPanel.cpp:242,244`).
- Row B (Load Take..., Show in Finder, Repair Audio): 100/110/100px
  (`RecordPanel.cpp:250,252,254`).
Two different width sets across rows is a minor inconsistency, but each row
reads fine on its own in every screenshot -- labels fit with comfortable
padding, no visible crowding or excess whitespace. Not a fix-now item.
- Disabled buttons (Play Take when nothing is loaded, Repair Audio when no
  take is loaded, Load Take.../Show in Finder while recording) use a
  consistent dimmed-gray treatment across every state screenshot -- good,
  predictable affordance for "not available right now."

## Status line placement

Status, warning, and notice text stack directly under the button rows, flush
left, in every one of the 10 screenshots (`RecordPanel.cpp:267-270`,
confirmed visually across all crops) -- position is consistent and
predictable; a performer glancing down after pressing a button will find
feedback in the same place every time. No issue here.

## Overall polish

- The bottom audio-selector strip (Audio: Mic Input, Gain slider, and three
  small transport icon buttons) reads visibly more cramped than the panel
  above it -- small square icon buttons with little visible padding, and a
  cyan meter graphic that is cut off at the crop's right edge in several
  screenshots (crop-recording.png, crop-overdub.png). I cannot tell from a
  crop alone whether that cutoff is a real clipping bug in the top bar or
  just where Harmony's screenshot boundary falls, and I did not read that
  source file per the dispatch's file list -- this is ASSUMED, not verified;
  flag for someone with the full top-bar screenshot bounds to confirm.
- Checkboxes ("Record audio", "Play with audio") use a consistent cyan
  outline and fill across every state -- good, matches the notice-text
  accent color, giving the panel one coherent color language: cyan for
  active control or routine information, distinct from red/green for
  transport state and yellow for caution. That system is used correctly and
  consistently across all 10 states.
- The REST-jargon notice text (e.g. "perf/play failed: no take loaded") is a
  content problem, not strictly a graphic-design one -- outside this seat's
  lens -- but visually it does not stand out as an error the way the yellow
  warning line does: both are small, unbolded, single-line text, so a
  performer has no visual cue that this particular cyan line is more urgent
  than a routine cyan notice like "Loaded: s4gate -- 0:26." If it is meant
  to read as an error, it should probably borrow the warning line's yellow
  treatment rather than share the routine-notice cyan it uses now.

## Strongest counterargument to my own verdict

Every contrast number above is computed against a flat background color
using the standard sRGB WCAG formula, not measured from the actual rendered
screenshot pixels -- font anti-aliasing, sub-pixel blending, and JUCE's own
rendering can shift effective contrast a little in either direction. And
WCAG AA compliance matters much less for a live-performance tool used by one
trained operator in a controlled environment than it would in a
public-facing accessibility context -- this performer will learn the button
color meanings quickly regardless of exact ratio, and the red/green
danger-vs-safe semantic is culturally unambiguous even at lower contrast. I
still hold the "Stop Recording" text-color finding because it is a one-line
fix, measurably below the same app's own green-button precedent on the
companion button, and visibly softer in the screenshots themselves -- but I
would weight it SHOULD rather than MUST for exactly the reason above.
