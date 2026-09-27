# Recording / Audio Store Reference

> Moved from CLAUDE.md (claudemd-split). Audio Store (Ruling 28) and the performance take recorder.

---

### Audio Store (Ruling 28)

`AudioStore` (`src/recording/AudioStore.h/cpp`) is the shared audio store recorded audio lives in, not the take folder: `~/Documents/Audio-DNA/Audio/<id>.adna-audio/{audio.wav, audio.json}`, id-keyed, sidecar written LAST (its presence is the "complete" flag). Take format v3's `AudioRef::Segment` references an asset by `{id, fingerprint, firstSample, frames, rate, channels}` — `file` is read-only legacy (pre-v3 in-folder audio). Content identity is `fp1` (a cheap deterministic head+tail+length SHA-256, `AudioStore::fingerprint`), recomputed at every `resolve()`. `AudioTap` re-patches the WAV header every 10s of audio (`AudioTap::kHeaderFlushSeconds`) so a crashed show is readable up to the last flush. Nothing is deleted automatically except a failed arm's own just-minted, never-finalized asset (`AudioStore::abandonAsset`, one narrow exception).

**Step 3 (record→store→take wiring): LIVE.** `MainComponent` owns a `RecorderHost` (`recorderHost_`) that drives the whole lifecycle; the production REST API exposes it as `/api/perf/record` (arm, file-mode or live input, optional onset markers), `/api/perf/stop`, `/api/perf/load`, `/api/perf/play` (`withAudio`: true replays audio points through the transport, false is silent wall-clock replay), `/api/perf/stop_play`, `/api/perf/repair` (crash recovery — re-derives a truncated/incomplete asset's frame count), and `/api/perf/status` (recording/playing/overdub state, take folder, asset id, take-clock `t`/`beat`/`sample`, `deviceRate`, `rateChangedSinceArm`, `sourceSampleRate`, lane/gesture/marker counts, `lastError`, `humanRefused`, `lastFinalizeError` (AudioStore::finalize's verdict for the last stop, "" = clean, cleared at arm), `finalizeErrors` (count of stops whose finalize reported a problem since app start, never reset) — s-rta-0924b; a finalize problem is also mirrored into `lastError`). Onset markers (`onsetMarkers: true` at arm) tag `take.json`'s `markers[]` with `action: "onset"` on every tick where the analysis snapshot's `onsetDetected` is true, deduped per onset event (not per 120 Hz tick) since `FeatureBus::read()` is always-latest and analysis publishes at only ~93.75 Hz. `rateChangedSinceArm` (R13-C, replacing the retired `rateMismatch`/"device != 48 kHz" meaning) is the one rate hazard that survives R13's resampler: it is true only if the DEVICE rate itself changed since arm (sample stamps before/after such a change are in different domains) — a device that never changes rate, even a non-48 kHz one, never sets it, because the analysis thread now resamples to its own fixed 48 kHz regardless of what the device is doing. **Replay restore (Boris ruling 2026-09-25, s-rta-0925):** `/api/perf/play` first restores checkpoint 0 — the look at Record time (active deck, quantize, every layer's flags/opacity/layer-effect values, active clip, each captured clip's effect values/scalars/play-pause) — before playing the recorded moves; NOT restored: video playheads, crossfade progress, pending quantized triggers, tempo, or the audio transport. `/api/perf/status` publishes `preambleCount`/`preambleFired`/`preambleRefused`/`preambleUnresolved` (all 0 while not playing) so a refused control (a human grip already held it) or a deck/layer/clip that no longer exists is counted, never silent. **End of replay (Boris ruling 2026-09-25, s-rta-0925):** the replay holds at the take's end — `playing` stays true, `finished` true, position pinned, the Player stopped; a with-audio replay gives the live input back; Stop Playback is the exit. `/api/perf/status` += `finished`, `inputSource` ("input"|"file"); `POST /api/audio/source` (`{"mode":"input"|"file"}`) is a dev/probe control that switches it.

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
  routines). It either plays once and holds the last look, or loops (Boris ruling 22 — a
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
- **Stop**: `POST /api/routine/stop` (`{"slot":N}` or `{"all":true}`) releases every grip the
  routine holds.
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
  0-7) — Toggle mode fires/restarts, Momentary mode holds-to-run.
- **Deferred to slice 2+** (disclosed, not silent): nothing about a fired routine is recorded back
  into a take (no `routine` lane, no `via`-tagged children); no lane editor / range-select UI (a
  routine is saved by beat/bar numbers over REST in slice 1); no per-slot binding re-target table;
  no import/export file format; a routine cannot switch decks, change tempo, or drive the audio
  transport (those lane classes are dropped at save time); routine-vs-**replay** arbitration (as
  opposed to routine-vs-routine) is tick order only, not gesture-begin order — a routine fired over
  a running replay writes last every tick, disclosed as R13.

---
