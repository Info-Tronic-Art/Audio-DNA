# Recording / Audio Store Reference

> Moved from CLAUDE.md (claudemd-split). Audio Store (Ruling 28) and the performance take recorder.

---

### Audio Store (Ruling 28)

`AudioStore` (`src/recording/AudioStore.h/cpp`) is the shared audio store recorded audio lives in, not the take folder: `~/Documents/Audio-DNA/Audio/<id>.adna-audio/{audio.wav, audio.json}`, id-keyed, sidecar written LAST (its presence is the "complete" flag). Take format v3's `AudioRef::Segment` references an asset by `{id, fingerprint, firstSample, frames, rate, channels}` — `file` is read-only legacy (pre-v3 in-folder audio). Content identity is `fp1` (a cheap deterministic head+tail+length SHA-256, `AudioStore::fingerprint`), recomputed at every `resolve()`. `AudioTap` re-patches the WAV header every 10s of audio (`AudioTap::kHeaderFlushSeconds`) so a crashed show is readable up to the last flush. Nothing is deleted automatically except a failed arm's own just-minted, never-finalized asset (`AudioStore::abandonAsset`, one narrow exception).

**Step 3 (record→store→take wiring): LIVE.** `MainComponent` owns a `RecorderHost` (`recorderHost_`) that drives the whole lifecycle; the production REST API exposes it as `/api/perf/record` (arm, file-mode or live input, optional onset markers), `/api/perf/stop`, `/api/perf/load`, `/api/perf/play` (`withAudio`: true replays audio points through the transport, false is silent wall-clock replay), `/api/perf/stop_play`, `/api/perf/repair` (crash recovery — re-derives a truncated/incomplete asset's frame count), and `/api/perf/status` (recording/playing/overdub state, take folder, asset id, take-clock `t`/`beat`/`sample`, `deviceRate`, `rateChangedSinceArm`, `sourceSampleRate`, lane/gesture/marker counts, `lastError`, `humanRefused`, `lastFinalizeError` (AudioStore::finalize's verdict for the last stop, "" = clean, cleared at arm), `finalizeErrors` (count of stops whose finalize reported a problem since app start, never reset) — s-rta-0924b; a finalize problem is also mirrored into `lastError`). Onset markers (`onsetMarkers: true` at arm) tag `take.json`'s `markers[]` with `action: "onset"` on every tick where the analysis snapshot's `onsetDetected` is true, deduped per onset event (not per 120 Hz tick) since `FeatureBus::read()` is always-latest and analysis publishes at only ~93.75 Hz. `rateChangedSinceArm` (R13-C, replacing the retired `rateMismatch`/"device != 48 kHz" meaning) is the one rate hazard that survives R13's resampler: it is true only if the DEVICE rate itself changed since arm (sample stamps before/after such a change are in different domains) — a device that never changes rate, even a non-48 kHz one, never sets it, because the analysis thread now resamples to its own fixed 48 kHz regardless of what the device is doing. **Replay restore (Boris ruling 2026-09-25, s-rta-0925):** `/api/perf/play` first restores checkpoint 0 — the look at Record time (active deck, quantize, every layer's flags/opacity/layer-effect values, active clip, each captured clip's effect values/scalars/play-pause) — before playing the recorded moves; NOT restored: video playheads, crossfade progress, pending quantized triggers, tempo, or the audio transport. `/api/perf/status` publishes `preambleCount`/`preambleFired`/`preambleRefused`/`preambleUnresolved` (all 0 while not playing) so a refused control (a human grip already held it) or a deck/layer/clip that no longer exists is counted, never silent. **End of replay (Boris ruling 2026-09-25, s-rta-0925):** the replay holds at the take's end — `playing` stays true, `finished` true, position pinned, the Player stopped; a with-audio replay gives the live input back; Stop Playback is the exit. `/api/perf/status` += `finished`, `inputSource` ("input"|"file"); `POST /api/audio/source` (`{"mode":"input"|"file"}`) is a dev/probe control that switches it.

**Take start (s-rta-0928).** A take's t = 0 -- its `"start"` tempo anchor, beat 0 and `meta.startBeatInBar` (where beat 0 sits in its bar, read from the same analysis snapshot; written only while the tracker is LOCKED, else unknown) -- is the first 120 Hz tick after Record whose snapshot already carries every tempo command sent before Record: Tap, Resync, set_bpm over REST/OSC, the typed manual BPM and the manual/auto switch, Link, a replayed tempo point (`FeatureSnapshot::trackerRequestSeq` against `BPMTracker::postedRequestSeq()` read at Record; Pitfall 48). With nothing in flight that is the first tick, as before; right after a command it is one or two analysis hops later (~10-20 ms), so a set_bpm or Resync sent just before Record is in the take's start and bar grid. If no snapshot carries it within 0.25 s of the first tick (no audio device, a stalled device), the take starts from the latest snapshot anyway (one `take start:` line in the app's stderr); a take stopped before its start is started from the last tick it saw. The audio still starts at Record; moves captured between Record and t = 0 are stamped t = 0, and an onset seen while t = 0 waited is marked at t = 0. A take started while the tracker is truly unlocked is an unmetered start (a bpm-0 `"start"` anchor, then `"lock"` when a tempo arrives), and a routine cut starting at its beat 0 is refused -- by design. Takes recorded before this fix keep the start they have. Test mode (no analysis thread) does not wait. Live: `.harmony/probe-tempo-start.sh`.

### Routines (s-rta-0926, slice 1)

A Routine is a saved PIECE of a recorded take — a named, beat-native lane set plus an explicit
PREAMBLE (the state its lanes need first) — fired as its own unit from an 8-slot composition-owned
bank (`Composition::routines`/`routineBank`, `src/model/Routine.h`; `RoutineEngine::kBankSize == 8`).
Slice 1 covers save/fire/restore/replay/loop/stop for one routine at a time, plus stacking (two
routines reaching for the same control):

- **Save**: `sliceRoutine()` (`src/recording/RoutineSlice.{h,cpp}`) cuts every kept lane of the
  currently LOADED take between `fromBeat`/`toBeat`, rebases x to the routine's own beat 0,
  synthesizes begin/end breakpoints for a gesture that straddles the cut, drops transport-class
  lanes by name (`tempo`/`audio`/`activeDeck`/`quantize`/macro/routine scopes — counted, never
  silent), and refuses an unmetered stretch. The PREAMBLE — what to restore before playing — is
  read for each kept lane's control: the last value before the cut if the lane touched it earlier,
  else checkpoint 0 (`PerfState`), else the library/scalar default; comp-level scalars are unknown
  to `PerfState` and are counted, not restored.
  `POST /api/routine/save` (`fromBeat`/`toBeat` or 1-based `fromBar`/`toBar`, optional `slot`,
  `loop`, `restoreState`, `quantize`, `wholeBars`) compiles it via `compileRoutine()`
  (`src/recording/Program.{h,cpp}`, the same `Program`/`Player` pair that already runs replay) and
  assigns it a bank slot (default: first free; an occupied slot is replaced, said in the notice).
- **Fire / restore / replay**: `POST /api/routine/fire` queues the routine PENDING until the next
  bar (its own `quantize`, default Bar; the global Quantize setting overrides when it is on). At
  that boundary `RoutineEngine` (`src/recording/RoutineEngine.{h,cpp}`, message thread, ticked
  right after `RecorderHost::tick`) restores the routine's preamble by default (Boris ruling 26,
  "restore" — `restoreState = false` is the per-routine "Start from now" switch, ruling 26's
  accepted exception) through the SAME `Player` preamble path replay restore already uses,
  then plays its lanes on its OWN beat clock (a `RecorderClock` shared across all running
  routines -- it integrates `totalBeatCount + beatPhase`, the tracker's continuous beat time, so a
  message-thread stall loses no beats and a loop folds every whole cycle a gap covers at once;
  Pitfall 42). It either plays once and holds the last look, or loops (Boris ruling 22 — a
  per-routine setting, re-firing the preamble each cycle).
  **The restore GLIDES** (s-rta-0926b plan3 C, Boris "glide is better"): the discrete half (clips,
  flags, play/pause — `Player::firePreambleDiscrete`) fires ON the boundary; each continuous entry
  is a glide the engine drives itself — this routine's lane-rank hand on the knob, a straight line
  from what the control shows (`Dispatch::read`) to the recorded start value over ONE BEAT ENDING ON
  THE BOUNDARY (shorter when fired later; a quarter-beat floor that spills past the boundary when
  fired with less than that to go, or with Quantize Off). The loop return (over each cycle's last
  beat, landing on the loop point) and a re-fire restart use the same rule, never touching a knob
  while the recording's own hand is on it before that boundary. A human hand or the recording's own
  move on the knob cancels the glide (no release: the knob is theirs); a stop — or a once-end / loop
  switched off with a glide in flight — lets go where it is. No `read` wired, or no beat when fired:
  the continuous restore lands in one call at the boundary, as before. `/api/routine/status`
  `bank[].glides` = glides started and not yet released. The take replay's own restore stays a cut.
  **Restore style** (s-rta-0926b, Boris "controls for jump or ease in each"): each routine's
  `restoreStyle` is `"ease"` (default — the glide above) or `"jump"` (the whole restore in one call ON
  the boundary at the start, every loop return and a restart, `read` never called), saved with the show
  (a file without it loads as ease), set by `POST /api/routine/set {"restoreStyle": ...}` and reported as
  `/api/routine/status` `bank[].restoreStyle`.
  **Restore cost (s-rta-0928).** A start / loop return / restart restores synchronously in its tick. Its discrete half
  refreshes the deck grid once per entry, and a refresh never decodes a file: image thumbnails come from `DeckView`'s
  `ClipThumbnails` (`src/ui/ClipThumbnails.h`), decoded once per file off the message thread, and a cell or strip
  re-derives its thumbnail only when its source changed (Pitfall 51). Before this, one refresh per entry decoded every
  image thumbnail on the deck: 54 ms on the probe deck, 178 ms with one 4K still, k x (cells + layers) x decode in
  general. `/api/routine/status` `bank[].holdMs` / `holdMsMax` = how long the last start / loop return held the message
  thread inside the engine, and the longest this run (ms, -1 before the first); probe-routines rows 5h / 8h / 11h hold it
  to 16 ms. **A move always lands its end:** when a tick passes a recorded gesture's end -- including a stall that
  stepped over the whole gesture or its tail -- `Player::advanceTo` writes the gesture's final value before it lets go
  (routines and take replay alike; a refused write releases nothing); probe-routines row 7m.
- **Stop**: `POST /api/routine/stop` (`{"slot":N}` or `{"all":true}`) releases every grip the
  routine holds. The TopBar Stop (`[]`, tooltip "Stop all routines") and the `GlobalStop` key/MIDI
  binding stop every running and waiting routine and nothing else — no clip is stopped, paused or
  rewound (Boris 2026-09-26, "we can keep stop for routines only").
- **Stacking (Boris ruling 22/his "signal" framing; F1 in the build plan)**: two routines reaching
  for the same control — the one whose gesture BEGAN LATER wins for the rest of that gesture; the
  earlier routine stays displaced until its own NEXT gesture (it does not resume when the later one
  lets go). A human grip always outranks both. Arbitrated inside `RoutineEngine`'s own sink
  (`laneOwner_`) — `ManualWrite`/`Player` are untouched.
- **Quantize parity**: "2 Bar"/"4 Bar" for a routine counts bars from the last phrase reset
  (`FeatureSnapshot::barCount`), exactly like a quantized clip trigger (Boris ruling 27) — not the
  monotonic `totalBarCount` (that supplies the bar EDGE only, never the parity).
- **Surfaces**: `POST /api/routine/{save,fire,stop,set,remove}` + `GET /api/routine/status`
  (six REST routes, `src/api/ApiServer.cpp`); `/audiodna/routine/{slot}` over OSC (value > 0
  fires; `src/osc/OscHandler.{h,cpp}`); `Binding::Action::TriggerRoutine` (keyboard + MIDI,
  learnable from 8 whole-word overlay targets "Routine 1".."Routine 8", `targetRoutineSlot`
  0-7) — Toggle mode fires/restarts, Momentary mode holds-to-run. **On screen (s-rta-0927 routine
  display, slice A)**: a 22 px ROUTINES row at the top of the deck (`DeckView`) holds eight
  `RoutinePad`s over the column numbers -- a press fires (restart while playing; a waiting pad ignores
  it), right-click opens the pad's settings (Loop/Once, Restore first/Start from now, Start: Ease/Jump,
  Quantize, Rename..., Remove from layers, Delete routine...); a waiting pad has a thin teal frame, a
  playing one a thick frame, a teal sweep and "5/8" (pressed again: a drawn "back to the start" mark
  left of "5/8" until the restart lands; fix round); a red "!" marks a routine that could not restore
  or play something (kept after the run until Stop / a composition load); a routine playing on another
  deck dims and the corner names it. A settings edit made while a pad waits reaches that start, and one
  made while a pressed-again pad's restart waits reaches that restart (`RoutineEngine::resyncPending`, fix
  rounds 1-2). Every layer a waiting/playing routine drives on the shown deck
  carries a band with its name (the routine cue, chartreuse `kRoutineCue`) over the top of the strip's
  picture (two at most, "+N"); the band's
  x takes the whole routine off, and the layer X takes every routine off that layer
  (`RoutineEngine::stopOnLayer`). Driven every 30 Hz tick from `RoutineEngine::Status` (`deck`,
  `layers`, `fireSeq`, `startsOn`, `restartPending`, `touchesComp` -- also on `/api/routine/status`)
  through `deriveRoutineDeckView` (`src/ui/RoutineDeckView.h`); a pad repaints only when its painted state changes
  (`RoutinePad::paintKeyOf` -- the sweep in pixels, Pitfall 59), a band only when its hairline width changes. The
  Record tab keeps only Save
  Routine (its old pad row is gone). Live: `.harmony/probe-routine-display.sh`.
  **The UI pattern (moved verbatim from CLAUDE.md "UI Patterns", s-rta-0929 asyncload -- CLAUDE.md keeps a pointer):**
  **Routine pads and bands**: a routine pad's press is always Fire (restart while playing, no-op while waiting); there is no stop control -- a routine leaves by its band's x (the whole routine, every layer), the layer X (clears the layer of routines too), its own end, the pad menu's "Remove from layers", or Stop (routines only). "Delete routine" (pad menu, warning red via `addColouredItem` -- the app LookAndFeel honours an item colour -- behind a confirm) is the only path that erases one. Pads and bands are model-driven from `RoutineEngine::Status` via `deriveRoutineDeckView` (`src/ui/RoutineDeckView.h`) every 30 Hz tick, never from panel memory; a strip fader follows the model (`LayerStrip::syncFromModel`, Pitfall 41) and its V fill turns the routine cue while a routine's hand grips opacity; a bound `UniversalParamControl` shows its value (and slider) in the routine cue with ROUTINE in the hint slot while a routine's (lane-rank) hand holds it. The routine cue is `AudioDNALookAndFeel::kRoutineCue` (chartreuse, band names too) -- reserved: never the accent cyan every mapped knob wears, and never used for anything else; `docs/claude/recording.md` "Surfaces".
- **Deferred to slice 2+** (disclosed, not silent): nothing about a fired routine is recorded back
  into a take (no `routine` lane, no `via`-tagged children); no lane editor / range-select UI (a
  routine is saved by beat/bar numbers over REST in slice 1); no per-slot binding re-target table;
  no import/export file format; a routine cannot switch decks, change tempo, or drive the audio
  transport (those lane classes are dropped at save time); routine-vs-**replay** arbitration (as
  opposed to routine-vs-routine) is tick order only, not gesture-begin order — a routine fired over
  a running replay writes last every tick, disclosed as R13.

---
