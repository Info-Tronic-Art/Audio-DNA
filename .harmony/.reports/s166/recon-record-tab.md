# Recon: "Record" tab — what exists, what's missing (s166)

Read-only recon. Repo: `/Users/boriskarpman/projects/RealTimeAudio`. No edits, no build, no run.

## Headline finding (surprise)

There are **two completely separate "recording" subsystems** in this codebase, and the
"Record" tab the owner is looking at only exposes ONE of them — the one that is NOT
video. A second, real, working, FFmpeg-based **video-to-disk recorder already exists**
and is wired into the live render loop and two menu/binding entry points, but it lives
under **Menu → Output → Start/Stop Recording**, not under the Record tab. VERIFIED.

---

## 1. Where is the Record tab defined? What UI does it present today?

VERIFIED. `BrowserPanel` is a 6-tab container (`src/ui/BrowserPanel.h:14,39`:
`Files, FX, Sources, CompDecks, Record, MilkDrop`). The Record tab's content is
`RecordPanel` (`src/ui/RecordPanel.h`, `src/ui/RecordPanel.cpp`), instantiated at
`BrowserPanel.h:59` and shown/hidden by `BrowserPanel::showActiveTab()`
(`BrowserPanel.cpp:146`: `recordPanel_.setVisible(activeTab_ == Tab::Record)`).

The tab button itself is NOT disabled — `BrowserPanel.cpp:19` wires
`recordTabBtn_.onClick` to `setActiveTab(Tab::Record)` like every other tab; there is
no `setEnabled(false)` anywhere on the tab button or panel (checked both
`BrowserPanel.cpp` and `MainComponent.cpp` — no hits). Inactive tabs are painted very
dark with dim text (`BrowserPanel.cpp:126-128`, colour `0xff1a1a2e`/`0xff606070`)
vs. bright when active (`:119-121`) — this styling could visually read as "greyed
out/disabled" even though the tab is clickable and switches normally.

**UI presented** (`RecordPanel.cpp:4-125`, layout `:153-192`):
- `Record` / `Stop` / `Play` / `Save` / `Load` buttons (`RecordPanel.h:37-41`)
- A "Format" combo box: `"JSON Events"` (id 1, selected by default) and
  `"Video (Future)"` (id 2) — **the id-2 item has no `onChange` handler at all**
  (`RecordPanel.cpp:89-93`; grepped for `formatSelector_` across both files —
  only construction/bounds, never read). Selecting "Video (Future)" does nothing;
  it's a label, not a feature switch.
- Status label ("Ready" / "Recording..." / "Stopped (N events)")
- Event-count label
- "Output Folder..." browse button + a label showing the chosen directory

This is a **session/performance event recorder UI**, not a video recorder UI — the
class comment says so explicitly: `RecordPanel.h:7-8`, "settings recording controls
... Records parameter changes and clip triggers as timestamped events."

## 2. Is there any real capture/encode/write-to-disk code in this repo?

VERIFIED — yes, in two unrelated places:

### 2a. `SessionRecorder` (behind the Record tab) — JSON event log, not video
`src/recording/SessionRecorder.h/.cpp`. Records timestamped events (7 types:
ParameterChange, ClipTrigger, ColumnTrigger, MacroChange, TransportChange,
EffectToggle, CuepointJump — `SessionRecorder.h:26-35`) and serializes to JSON
(`SessionRecorder.cpp:165-217` save, `:219-279` load). This writes a small JSON
file describing *what you did*, not a video.

- **Only one of the 7 record methods is ever actually called** in the whole app:
  `sessionRecorder_.recordClipTrigger(layerIndex, column)` at
  `MainComponent.cpp:3767`. Grepped for the other six (`recordParameterChange`,
  `recordColumnTrigger`, `recordMacroChange`, `recordTransportChange`,
  `recordEffectToggle`, `recordCuepointJump`) across `MainComponent.cpp` — zero
  hits. So today, recording a session only logs which clips you triggered and
  when; knob turns, macro moves, transport changes, effect toggles are silently
  not captured despite the class fully supporting them.
- **Play does nothing observable.** `RecordPanel.cpp:38` calls
  `recorder_->startPlayback()`, which only flips a flag
  (`SessionRecorder.cpp:108-113`). The method that actually replays events,
  `advancePlayback(dt)` (`SessionRecorder.cpp:120-140`), has **zero callers**
  anywhere outside its own class — grepped `MainComponent.cpp` for
  `advancePlayback`: no hits. Nothing drives it, so pressing Play sets an
  internal "playing" flag and nothing visible ever happens. This is almost
  certainly the concrete behavior that got this tab classified "does nothing."
- `RecordPanel::refresh()` (updates the live event-count label) also has zero
  callers in `MainComponent.cpp` — grepped, no hits. The label only updates once,
  inline, inside the Stop-button click handler (`RecordPanel.cpp:27-28`).
- `onStartRecording` / `onStopRecording` / `onPlayRecording` callbacks
  (`RecordPanel.h:21-23`) are never assigned by `MainComponent` — grepped, no
  hits. Only `setSessionRecorder(&sessionRecorder_)` is wired
  (`MainComponent.cpp:1479`).
- A prior planning pass already diagnosed exactly this and wrote a full build
  spec for it: `.harmony/specs/session-recorder-spec.md` (dated 2026-07-17,
  status "PLAN"). Its own "current state" section matches everything above
  line-for-line, meaning **the plan to fully wire this was written but never
  executed** — it's still in the exact pre-plan state today.

### 2b. `VideoRecorder` (NOT behind the Record tab) — real FFmpeg video capture
`src/recording/VideoRecorder.h/.cpp`. This is a complete, working, real-time
screen-capture-to-video-file pipeline:
- GL thread calls `submitFrame()` after every rendered frame
  (`VideoRecorder.h:63-66`); does `glReadPixels` into one of 3 ring-buffered CPU
  pixel buffers (`VideoRecorder.h:90-99`, `.cpp:101-139`).
- A dedicated encoder thread (`.cpp:141-178`) converts RGBA→YUV via
  `sws_scale` and encodes via real FFmpeg calls: `avformat_alloc_output_context2`,
  `avcodec_find_encoder_by_name`, `avcodec_open2`, `avformat_write_header`,
  `avcodec_send_frame`/`avcodec_receive_packet`,
  `av_interleaved_write_frame`, `av_write_trailer` (`.cpp:182-446` —
  `initEncoder`, `encodeFrame`, `flushEncoder`, `closeEncoder`). This is not a
  stub; it's a full encoder lifecycle.
- Three real codecs: H.264/libx264 (mp4, default), ProRes/prores_ks (mov),
  MJPEG (`.cpp:203-262`).
- Dropped-frame counting when the encoder falls behind
  (`.cpp:109-114`, `droppedFrames_`).

**It's live-wired, not orphaned:**
- `Renderer` holds a `VideoRecorder*` (`Renderer.h:165,421`) and calls
  `videoRecorder_->submitFrame(...)` inside the real per-frame render path,
  right after the composited frame is finished, every frame
  (`Renderer.cpp:692-693`).
- `MainComponent` owns the instance (`MainComponent.h:451`,
  `VideoRecorder videoRecorder_;`) and wires it to the renderer at construction:
  `previewPanel_.getRenderer().setVideoRecorder(&videoRecorder_);`
  (`MainComponent.cpp:467`).
- Two real, reachable UI/control entry points actually call
  `videoRecorder_.startRecording(...)`/`stopRecording()`:
  1. **Menu → Output → "Start Recording" / "Stop Recording"** — real, enabled
     menu items (`MenuBarModel.cpp:148-149`, `addItem(..., true, false)` = enabled,
     unticked), dispatched at `MainComponent.cpp:5167` (`case
     C::kOutputStartRecording`) and `:5189` (`kOutputStopRecording`). Start
     builds an `mp4` filename under `~/Documents/Audio-DNA/Recordings/` and
     calls `startRecording` with H.264 @ 1920x1080/30fps/CRF23
     (`MainComponent.cpp:5167-5187`).
  2. A **MIDI/OSC binding action**, `Binding::Action::ToggleRecording`
     (`MainComponent.cpp:5893-5915`) — same start/stop logic, reachable by
     mapping a MIDI controller or OSC message to it via the existing
     binding system.
- `videoRecorder_.onRecordingFinished` is wired to log success/failure
  (`MainComponent.cpp:1872-1875`).
- Compiled unconditionally: `CMakeLists.txt:19` does
  `find_package(FFmpeg REQUIRED)` (a hard, non-optional dependency),
  `:216-217` list `VideoRecorder.h/.cpp` as sources, `:510` links
  `FFmpeg::FFmpeg`. This is not behind a build flag — it's always built.

None of this touches `SessionRecorder`, `RecordPanel`, or the Record tab in any
way. Confirmed by grep: `VideoRecorder` never appears in `RecordPanel.cpp`,
`RecordPanel.h`, or `BrowserPanel.cpp`.

## 3. What is the app already capable of that a recording feature would build on?

VERIFIED, two distinct existing capabilities:

- **Continuous video capture already works end-to-end** (see 2b above) — GL
  framebuffer → FFmpeg H.264/ProRes/MJPEG → `.mp4`/`.mov` on disk, with
  triple-buffered capture so it doesn't stall the render thread. This is the
  actual "record my performance to a video file" feature, already built, just
  not on the tab the owner is looking at.
- **A separate offline single-frame render path exists**, requested via REST:
  `POST /api/render_frame` (`ApiServer.cpp:206` route registration,
  `:835` `handleRenderFrame`) → `renderer_.captureFrame(file, time)`
  (`Renderer.cpp:1737` declaration/impl, actual pixel readback + PNG write in
  `Renderer::processPendingCapture`, `Renderer.cpp:1779-1851`). This produces
  **one PNG image at an arbitrary timestamp**, not a video — it's a
  deterministic-testing/screenshot tool (used elsewhere by an internal test
  server per `docs/archive/feature_audit/slice_10_osc_api_recording_output.md`),
  not a video export path. `Renderer::takeSnapshot()` (`Renderer.cpp:1853`) is
  the user-facing PNG "snapshot" wrapper around the same `captureFrame` (bound to
  Menu → Output → Snapshot and OSC `/audiodna/snapshot`).

So the app already has both the continuous-video machinery (2b) and the
single-frame-to-disk machinery (this section) — the two things a "record my VJ
session to a video file" feature is built from. The hard, expensive part (a
working real-time GL-framebuffer→FFmpeg encoder) is **done**.

## 4. What's missing to make "record my performance to a video file" fully work

- **No audio track.** Verified with two independently-shaped greps against
  `VideoRecorder.cpp`: (a) `grep -n "VideoRecorder" src/**` style struct/field
  search found only one `AVStream* videoStream_` (`VideoRecorder.h:106`), no
  second stream; (b) `grep -niE "aac|pcm_s16|audioStream|AV_CODEC_ID_AAC|audio_st"
  src/recording/VideoRecorder.cpp` — zero matches. The class comment says this
  outright (`VideoRecorder.h:34-35`): "Audio is NOT included in the recording."
  A silent video of an audio-reactive VJ performance is a real gap for the
  stated use case — this is the single biggest missing piece.
- **Not exposed on the Record tab at all.** A user on that tab has no way to
  discover or start video recording — it's only reachable via the Output menu
  or a MIDI/OSC binding. Either the Record tab needs a video-record control
  wired to `VideoRecorder`, or the two features need to be clearly separated in
  the UI so people stop expecting the Record tab to do this.
- **No live status/UI feedback while recording.** `VideoRecorder` exposes
  `getRecordedDuration()`, `getRecordedFrameCount()`, `getDroppedFrameCount()`
  (`VideoRecorder.h:70-72`) but grepping `MainComponent.cpp` and every file in
  `src/ui/` for these three names found **zero consumers** — no on-screen
  recording indicator, elapsed-time readout, or dropped-frame warning exists.
  The only feedback today is a `std::cerr` log line.
- **Hardcoded settings.** Both real call sites hardcode H.264, 1920x1080, 30fps,
  quality 23, and the output folder (`MainComponent.cpp:5171-5177`,
  `:5901-5906`) — no UI to pick codec/resolution/fps/quality/output path for
  video, unlike the JSON recorder which already has a working "Output
  Folder..." file chooser (`RecordPanel.cpp:107-119`) that could be a model for it.
- **No tests.** Grepped `tests/` for `VideoRecorder` and `SessionRecorder` by
  filename and by content — no hits either way. No coverage exists for either
  recorder.
- **SessionRecorder's own gaps** (separate from video, but relevant since it's
  what's actually on the tab today): only clip-trigger events are captured (see
  §2a), and Play/replay is fully inert. If the owner's mental model of "Record
  tab" includes "record and replay my knob/macro moves," that specific plan
  already exists at `.harmony/specs/session-recorder-spec.md` but was never
  built.

## 5. Size verdict

**Medium, not a new subsystem** — because the expensive core (a real-time
GL-framebuffer capture + triple-buffered FFmpeg H.264/ProRes/MJPEG encoder,
already handling the hard threading problem) is fully built, tested-by-existing-
integration into the render loop, and already reachable today via the Output
menu. What remains is glue and completeness work, not new engineering:
add an AAC/PCM audio stream + mux with the existing `AudioEngine` output
(the one genuinely nontrivial remaining piece — needs a second AVStream, an
audio encoder, and a way to pull PCM off the live audio path), wire a
record button + status readout onto the Record tab (or retarget the tab's
purpose), expose codec/resolution/output-path settings, and add basic tests.
None of this requires inventing new capture technology — it requires finishing
the wiring around a working encoder. Rough shape: audio muxing is the one
"medium" sub-task; everything else (UI wiring, settings, status display) is
individually small.

## Anything surprising

The owner's framing was "the Record tab is a necessary feature, not yet
built." The accurate picture is closer to: **a working video recorder already
exists in the app and is one menu-click away, completely independent of the
tab he's looking at**, while the tab he IS looking at is for a different,
half-built feature (session/event JSON logging) whose Play button is a
confirmed no-op. The fix for "I want to record my performance to video" may be
far cheaper than "build the feature" — it may be closer to "surface the
feature that already works, and give it audio."
