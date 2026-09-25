# Logic critic (source) — Record panel step 4: findings (1) and (2)

## VERDICT
Both observations are real bugs with precise, separable root causes in
`src/recording/`, not in the panel/funnel. (1) is a wrong LENGTH SOURCE in
`Program::compile`. (2) is a clock that is NEVER RESET at `arm()` because
`RecorderClock` is a single long-lived member that latches its `t=0` origin
on the app's FIRST-EVER tick, not on the take's arm. Both fixes stay inside
`RecorderHost`/`RecorderClock`/`Program.cpp`; the message-thread funnel
(`MainComponent::perfRecord/perfPlay/...`, `MainComponent.cpp:5074-5178`) is
confirmed to be a pure move (arm-opts marshalling + `dispatch.notify` on
failure only) and must stay that way — do not patch the symptom there.

---

## (2) "Recording 0:15" one second after arming — root cause

`RecordPanelModel.h:189` prints `formatClock(s.t)`, and `s.t` comes straight
from `RecorderHost::publishStatus()` (`RecorderHost.cpp:715`, `s.t = now.t;`
where `now = clock_.now()`). That part is a faithful mirror of the host.

The bug is upstream, in `RecorderClock` itself:

- `RecorderHost.h:249` — `RecorderClock clock_;` is ONE instance owned by the
  host for its entire lifetime; nothing re-constructs or resets it.
- `RecorderClock::tick()` (`RecorderClock.cpp:13-23`) sets `t=0` and latches
  `haveTicked_ = true` only on the very FIRST call it EVER receives —
  `startWall_ = wallNow` on that first call, and every later `t` is
  `wallNow - startWall_` (`RecorderClock.cpp:27`). The header comment at
  `RecorderClock.h:39` claims "the FIRST tick() call establishes t=0
  (recording start)" — true only if the first-ever call happens at
  recording start. It doesn't.
- `MainComponent.cpp:3358` calls `recorderHost_.tick(snap, now, ...)`
  **unconditionally** on the 120 Hz message-thread timer, every frame the
  app runs, regardless of whether anything is recording.
- `RecorderHost::tick()` (`RecorderHost.cpp:392`) calls `clock_.tick(...)`
  at the top, before the `if (recording_)` guard at line 395 — so it too
  runs unconditionally.
- `RecorderHost::arm()` (`RecorderHost.cpp:143-256`) resets every other
  per-take field (`gaps_.clear()`, `markers_.clear()`, `lastError_.clear()`,
  `humanRefused_ = 0`, etc.) but never touches `clock_`. There is no
  `RecorderClock::reset()` method to call even if it wanted to
  (`RecorderClock.h` exposes only `tick()`, `now()`, `tempo()`).

Net effect: `haveTicked_` latches on the app's first-ever 120 Hz tick
(effectively at app launch/first frame), so `startWall_` is the app-launch
wall time, not the arm wall time. Every take thereafter — including the
first — reads `t = wallNow - appLaunchWall`, i.e. wall-clock-since-launch,
not wall-clock-since-arm. "Recording 0:15" one second after arming is
consistent with the app having been sitting idle ~14 s before the user
pressed Record Take; "Recording over s4gate 0:33" is the same defect on an
overdub. `RecorderClock::tempo_`/`wholeBeats_`/`beatOffset_`/`lastPhase_`/
`lastBpm_`/`lastAnchorBeat_` are equally stale carry-over from whatever the
clock was doing (or not doing, pre-arm) before this take started, so `beat`/
`bpm` stamps in the saved take (`Fired::at`, `Stamp`) may also be wrong for
every take after the first live session.

**What it should be:** `t` must read 0 at the moment `arm()` succeeds and
count up from there. Fix belongs in `RecorderClock` (add a `reset()` that
clears `haveTicked_`, `startWall_`, `wholeBeats_`, `beatOffset_`,
`lastPhase_`, `lastBpm_`, `tempo_`, `lastAnchorBeat_`, `current_`) called
from `RecorderHost::arm()` (`RecorderHost.cpp:143`, alongside the other
per-take resets at lines 160-171, before `recorder_.start(comp, clock_,
takeFolder_)` at line 232) — a local state mutation on the same
`RECORDER_HOST_ASSERT_MESSAGE_THREAD()`-guarded call already running on the
message thread (`RecorderHost.cpp:145`). This touches no `dispatch.notify`
path and calls nothing outside `RecorderHost`/`RecorderClock`, so it cannot
re-enter `RecorderHost` through the notify fan-out (confirmed below).

---

## (1) "Playing 0:02 / 0:00" though the take is 0:26 — root cause

`RecordPanelModel.h:85` prints `formatClock(s.lengthSeconds)`, fed by
`RecorderHost::publishStatus()` (`RecorderHost.cpp:759-766`). For
`PlayMode::WithAudio` the length comes from `s.length`
(`RecorderHost.cpp:741`, `s.length = program_ ? program_->length : 0.0;`)
divided by `playAssetRate_`.

`program_->length` is NOT the take's duration. It comes from
`Program::compile()` (`src/recording/Program.cpp`):

- `Program.cpp:187` — `double maxAt = 0.0;`
- `Program.cpp:212` — bumped only inside the discrete-lane loop, by each
  control point's own `at` (a knob/clip-trigger event's timestamp).
- `Program.cpp:248` — bumped only inside the continuous-lane loop, by each
  gesture's last breakpoint `x1`.
- `Program.cpp:273` — `program->length = maxAt;` — final value is simply the
  latest control-event timestamp across every lane, in whichever
  `DriveClock` domain was requested.

`maxAt` is never seeded from `Take::Meta::duration` or from the audio
asset's own frame count/rate. A take recorded with "Record audio" on but
with few or no knob/clip-trigger movements (or movements that all happened
early) compiles to a `Program` whose `length` is far shorter than the
actual take — in the extreme, `maxAt` stays `0.0` if the take has zero
lane events, giving exactly the observed "0:00" regardless of the true
0:26 audio duration. This is a systematic bug, not a one-off: any
mostly-static (few-gesture) take with audio will show a truncated or zero
length during playback.

Contrast: the SAME host already computes the correct duration elsewhere —
`RecorderHost.cpp:316`, `take.meta.duration = clock_.now().t;` at disarm,
and `RecorderHost.cpp:776`, `s.loadedDuration = loadedTake_->meta.duration;`
— which is exactly what the "Loaded: ... — 0:26 ..." status line shows
correctly right before Play is pressed (per the plan's own state list).
Once playback starts, the display switches to `playingReadout()`
(`RecordPanelModel.h:83-91`), which uses the wrong source (`program_->
length`) instead of the take's own known-good duration.

**What it should be:** the playing length should come from the take's
actual audio/take duration — either `loadedTake_->meta.duration` (already
published as `s.loadedDuration`, `RecorderHost.h:220`) for the WallClock
path, or the resolved audio asset's frame-count/rate for the WithAudio
path (same quantity `AudioStore::Resolution` already carries) — not from
`Program::compile`'s `maxAt`, which only measures control-lane activity and
was never meant to stand in for take length. Fix belongs in
`Program::compile` (seed `maxAt` from `take.meta.duration` converted into
the target `DriveClock` domain, `Program.cpp:187`) or in
`RecorderHost::publishStatus` (`RecorderHost.cpp:741`, prefer
`loadedTake_->meta.duration`-derived length over `program_->length`) — pick
one canonical source, not two competing ones.

---

## Message-thread-only / no re-entrancy / funnel-is-a-move — verified

- Both `arm()` (`RecorderHost.cpp:143`) and `tick()`
  (`RecorderHost.cpp:354`) start with `RECORDER_HOST_ASSERT_MESSAGE_THREAD()`
  — the proposed `clock_.reset()` call lands inside that same assertion, no
  new thread surface.
- `recorderHost_.dispatch.notify` (`MainComponent.cpp:2012-2018`) only
  logs to stderr and calls `browserPanel_->getRecordPanel().setNotice(msg)`
  (a stored string applied at the panel's next 4 Hz `refresh()`, per
  `RecordPanel.h:42-45`) — it never calls back into `recorderHost_`. The
  `perfRecord/perfStop/perfPlay/perfStopPlay` funnel functions
  (`MainComponent.cpp:5074-5178`) call `recorderHost_.arm/disarm/play/
  stopPlayback` exactly once each and use `dispatch.notify` only to report
  failure text — never to re-enter the host. So a clock reset added inside
  `arm()` cannot trigger any re-entrant call back into `RecorderHost`
  through the notify fan-out; the funnel stays a pure move.

## Strongest counterargument to this diagnosis

For (2): it's possible `RecorderClock::tick()`'s "first call ever" is
actually gated to only fire while `recording_` is true somewhere I haven't
traced (e.g. a guard inside `FeatureBus`/`AnalysisThread` that withholds
`FeatureSnapshot`s until a take is armed), which would make the first
observed tick coincide with arm and let this bug hide. I checked the call
site directly (`MainComponent.cpp:3358`, inside the same unconditional
120 Hz block that also drives `signalRegistry_.evaluateAll(snap)` and
`globalMacroBank_.updateValues(...)` a few lines above at 3332/3344, both of
which plainly run whether or not anything is recording) and found no such
gate — `recorderHost_.tick()` is called every tick, unconditionally, for
the life of the app. I hold the diagnosis, but flag this as the one
external fact (whether snapshots ever stop flowing before first arm) I
inferred from the surrounding code rather than from a live trace.

For (1): it's conceivable `program_->length` was deliberately scoped to
"length of the control-path program" (useful for e.g. a WallClock replay
with no audio, where audio length is meaningless) and the panel is simply
choosing the wrong field to display for the WithAudio case specifically —
i.e., the bug could be classified as a `publishStatus()` display-selection
bug rather than a `Program::compile` computation bug. Either framing points
at the same two candidate fix sites I named above; I favor fixing the
source (`Program::compile` or the `s.length` selection) over patching the
panel, since `RecordPanelModel.h` is contractually a pure function of
`Status` (its own doc comment, `RecordPanelModel.h:11-13`) and must not
special-case around an upstream field that means the wrong thing.
