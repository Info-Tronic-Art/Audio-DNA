# Interaction-Logic critic — Step 4 Record panel

VERDICT: PASS-WITH-FIXES — no dead-ends or navigation traps found, but the panel presents
fabricated numbers as live facts in two MUST cases, and one enabled action silently does
less than its sibling button in the identical state.

## MUST-1 — "Playing X / Y" length is not the take's length; it is 0 for any audio-only take
`RecordPanelModel.h:83-91` `playingReadout()` prints `s.lengthSeconds` verbatim. RecorderHost
populates it (`RecorderHost.cpp:754-761`) from `s.length = program_->length` (line 741), which
is the compiled automation-point timeline's max `at` (plan G8: "length = maxAt"), not the audio
asset's duration. The gate take had 0 lanes/0 points (confirmed on-screen: "0 lanes · 0 moves"
in every recording/overdub crop), so `program_->length` is 0. Screenshot
`playing-with-audio.png` shows exactly "Playing 0:02 / 0:00 · unresolved: 0" — this confirms
and roots Harmony's observation #1. The correct duration (26s) is already published as
`s.loadedDuration` (RecorderHost.cpp:776) but `playingReadout()` never falls back to it. Any
take that is pure audio (no knob automation — a completely normal use case for a live
performer just capturing a take) will show "/ 0:00" forever, reading as "broken/empty take"
even while it audibly plays. Fix: `lengthSeconds` should fall back to `loadedDuration` when the
program has no points, or the host should compute length as `max(program_->length,
audio asset duration)`.

## MUST-2 — the recording clock shown (and saved) is seconds-since-launch, not seconds-since-arm
`RecordPanelModel.h:189` prints `formatClock(s.t)` as "Recording <t>". `s.t` comes from
`clock_.now().t` (`RecorderHost.cpp:714-715`), and `RecorderClock::tick()` anchors `startWall_`
only once, on the clock's very first tick ever (`RecorderClock.cpp:13-16`, `haveTicked_` never
reset anywhere in the class — no `reset()` method exists). `RecorderHost::tick()` calls
`clock_.tick()` unconditionally on every call regardless of `recording_` (`RecorderHost.cpp:392`),
and `arm()` (`RecorderHost.cpp:141-232`) never re-anchors `clock_`. So `t` is really "seconds
since RecorderHost's first tick" (effectively app launch), accumulating across every
arm/disarm/overdub cycle in the session — it is never the elapsed time of the take in progress.
This confirms Harmony's observation #2 (`armed-or-recording.png`: "Recording 0:15" one second
after the arm POST) with a root cause, not a coincidence. Worse: `take.meta.duration =
clock_.now().t` (`RecorderHost.cpp:316`) writes this same wrong value into the saved `take.json`
— the corruption is not cosmetic, it is persisted, and is exactly what later reappears as
`loadedDuration` in "Loaded: ... — <clock>" (row 2/3 of the matrix). A performer doing several
takes in one session will see take durations that only make sense for the very first take.

## MUST-3 — overdub's two "stop" buttons are asymmetric with no disclosure (the double-stop trap)
In row 10 (recording+overdub+playing) both "Stop Recording" and "Stop Playback" are enabled.
Pressing "Stop Playback" correctly stops both (by design — `RecorderHost::stopPlayback`,
`RecorderHost.cpp:665-680`, ruling 1). Pressing "Stop Recording" instead calls `perfStop()` →
`RecorderHost::disarm()` (`MainComponent.cpp:5127-5138`, `RecorderHost.cpp:258-352`), which never
touches `playing_`/`player_` — the replay keeps running after the take is finalized and saved.
The model transitions cleanly into row 9 ("Record Over"/"Stop Playback" green), so this is not a
dead end, but neither button's tooltip says so: "Stops this take. It is saved automatically."
(`RecordPanelModel.h:120`) gives no hint that the live replay audio you are hearing will keep
playing after this click, while the visually-identical-looking "stop" on the Play button ends
everything. A non-technical performer pressing the RED button expecting silence gets continued
audio instead — exactly the class of trap this seat exists to catch.

## SHOULD-1 — "armed" can swallow a concurrent REST-only playback state
`v.statusText` for `armed` (`RecordPanelModel.h:177-178`) is a bare assignment with no
`if (playing)` append, unlike the non-armed recording branch just below it
(`RecordPanelModel.h:195-196`). A plain record started via REST while wall-clock playback is
already running (row 12) would, for the brief `framesWritten==0` window, report only "Armed,
waiting for audio..." and hide that playback is also active. Narrow (sub-second) but real.

## Checked and clean
Stale-state-on-tab-switch: `refresh()` runs unconditionally at 4 Hz regardless of tab visibility
(`MainComponent.cpp:3386-3408`), so `visibilityChanged()` (`RecordPanel.cpp:148-153`) is
belt-and-suspenders, not the only path — no staleness found. Rows 11/12 (REST-only) are derived
purely from `Status`, never from UI history, and match the matrix exactly on inspection. The
audio-selector restore-to-Mic-Input (Harmony obs #5) is genuinely correct — verified pixel-level
in `crop-idle-restored.png`, the composite `sheet.png` thumbnail that appears to show "Audio
File" there is a JPEG/PNG-downscale artifact, not the real panel.

## Strongest counterargument, and why it doesn't change the verdict
One could argue MUST-1/2 are RecorderHost/RecorderClock bugs, out of RecordPanelModel's blast
radius, so a Record-panel critic should downgrade them to FYI. I hold MUST because the model is
the thing the user actually reads, has one branch (`playingReadout`) already carrying a working
fallback fact (`loadedDuration`) it doesn't use, and the panel's own click-through matrix
(plan §4.3) promises `Playing 12.3 / 45.6 s` as the verification bar for this exact row — the
matrix's own promise is unmet, which is this seat's mandate regardless of which file the fix
lands in.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0924b/step4-visual/critic-interaction-logic.md
