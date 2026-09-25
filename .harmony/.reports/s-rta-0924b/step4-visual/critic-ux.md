# UX Critic — Record Panel (Step 4)

VERDICT: PASS-WITH-FIXES

## Can a non-technical performer tell what state they're in?

Mostly yes. Record/Playback state is color-coded consistently across every screenshot:
red = recording (`RecordPanel.cpp:177-179`), green = playing (`RecordPanel.cpp:181-183`),
neutral = idle. Button labels flip correctly (Record Take↔Stop Recording, Play Take↔Stop
Playback) and are driven purely from `RecorderHost::Status`, not shadow UI state
(`RecordPanelModel.h:104-114`), so the label can't drift from truth. The status line
(`Recording 0:15 from live input · 0 lanes · 0 moves`, `Loaded: s4gate — 0:26 · 0 lanes ·
Audio: ready`) is plain, whole-word English and answers "where did my take go" — take name
plus root path (`Takes: ~/Documents/Audio-DNA/Takes`) is always visible, not hover-only.

Two things break this:

**1. Playing readout shows a broken total length ("Playing 0:02 / 0:00").** Confirmed in
both `crop-playing-with-audio.png` and `crop-overdub.png` even though the same panel just
said `Loaded: s4gate — 0:26` one screen earlier. Root cause: `RecorderHost.cpp:760/766`
sets `s.lengthSeconds` from `s.length`, which is evidently zero/unset on this path while
playing. To a non-technical performer mid-show, "/ 0:00" reads as "my 26-second take just
became zero seconds" — i.e., it looks like data loss, not a display quirk. Harmony's
own observation (2) is a related but distinct bug (recording clock not starting at 0:00);
this is the playback-side counterpart and is arguably worse because it implies the take
itself is gone.

**2. REST/dev jargon leaks straight into the notice line the performer reads.**
`MainComponent.cpp:5078,5117,5132,5159,5192-5193` hardcode strings like
`"perf/play failed: no take loaded"`, `"perf/stop_play: ..."`, `"perf/stop: not recording"`
and pass them verbatim to `RecordPanel::setNotice()` (`RecordPanel.cpp:136-140`), which
renders them unmodified in the cyan notice label (`RecordPanel.cpp:228`). Confirmed in
`crop-notice-no-take-loaded.png` and `crop-idle-restored.png`. This is squarely the class
of refusal that must be visible without hovering — it is — but it fails the whole-word
plain-language bar: "perf/stop_play" is an internal endpoint name a performer never typed
and has no way to parse under stage pressure.

## Record Over — is it explained without hovering?

No. The button relabels itself to "Record Over" the instant the performer presses Play
on a loaded take (`RecordPanelModel.h:121-126`, visible in `crop-playing-with-audio.png`),
but the only explanation of what it *does* — "Records a new take over this take's audio,
from the current playback position" — lives in the tooltip
(`RecordPanelModel.h:123-124`), which requires a hover. Nothing in the always-visible
status/warning text says this before the performer presses it. A performer who has never
seen this label before has no on-screen cue that pressing it will overwrite/append rather
than, say, branch to a new take. Once overdubbing starts, the warning line does correctly
announce consequence after the fact ("Recording over s4gate 0:33 ... Replaying with the
take's audio. Live input is paused until Stop Playback." — `RecordPanelModel.h:180-186,218`)
— but that's disclosure after commitment, not before.

## Why a button is dim — visible without hovering?

Partially. `RecordPanel.cpp:173` dims disabled buttons via alpha, which IS visible without
hovering — the performer can see something is off-limits. But the *reason* is tooltip-only
(`RecordPanelModel.h:138-160`: "Stop the recording first.", "Load a take first.", "No take
yet.", "Only needed when..."). In `crop-idle-nothing-loaded.png`, Play Take and Repair
Audio are both dimmed with zero on-screen explanation — the status line just says "Ready.
No take loaded.", which happens to cover Play Take's case but not Repair Audio's, and
there's no line-of-sight rule tying a specific dim button to a specific status sentence.
A performer has to guess or hover.

## Truncated name placeholder

`"Name (blank = date and t..."` is cut off in every single screenshot
(`RecordPanelModel` / `RecordPanel.cpp:92`: full text is `"Name (blank = date and time)"`).
This directly violates the project's own UI rule ("Always display whole words... never use
abbreviations") — not because the code abbreviates the word, but because the field is too
narrow to show it, so the effect on screen is identical to an abbreviation the user did not
choose. Confirmed via the fixed-width column in `resized()` — this is a layout bug, not
content.

## Positive notes

- Record/Stop-Recording is never ambiguous: consistent red across all 10 states.
- Overdub state genuinely explains the ruling-mandated behavior in plain words
  ("Live input is paused until Stop Playback").
- Harmony's observation (5) — stop_play during overdub correctly returns the audio
  selector to Mic Input — is consistent with what `RecordPanelModel.h:108,163-167` derives
  (`withAudio` false once playing stops, `recordAudioEnabled` true again), so that ruling
  is genuinely enforced in the model, not just in the one screenshot sampled.

## Strongest counterargument to this verdict

One could argue this deserves a straight FAIL: a performer-facing app showing raw REST
strings ("perf/stop_play") and a broken duration readout ("/ 0:00") right after a take is
loaded are not polish gaps, they're the two most basic trust signals (did my words make
sense, is my recording intact) failing simultaneously, in front of the exact user this
app's CLAUDE.md says is non-technical. I hold PASS-WITH-FIXES rather than FAIL because
every issue found is narrow and mechanically fixable without touching the model's control
flow: the notice strings are hardcoded literals in `MainComponent.cpp` (swap for
plain-language copy), the length bug is a single field assignment in `RecorderHost.cpp`,
the placeholder is a column-width fix in `resized()`, and "Record Over" needs one more
sentence surfaced in the status/warning line rather than a tooltip. None require
re-architecting `RecordPanelModel`'s pure-function design, which is otherwise sound and
correctly rules-compliant (color coding, disabled-state visibility, no shadow state).
