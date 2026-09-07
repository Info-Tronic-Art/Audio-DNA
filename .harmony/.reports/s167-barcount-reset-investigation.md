# barCount reset investigation — RealTimeAudio (Audio-DNA), s167

STATUS: DONE
Repo: /Users/boriskarpman/projects/RealTimeAudio (read-only investigation, no edits, no build)

## VERDICT

`barCount_` is DESIGNED to be monotonic (cumulative bars since the last reset), not
periodic/wrapping-by-itself — the `% phraseBars_` folding for `phrasePhase_` and the
`/beatDuration_` folding in the oscillator formula happen downstream, on read. It is
documented (`src/signal/OscillatorSignal.h:33-40`) to reset only RARELY, on genuine
musical structural transitions or explicit user Resync — never on a fixed period.
**What actually resets it live is `BPMTracker::updatePhrase()`'s structural-transition
branch** (`BPMTracker.cpp:481-489`), fed by `StructuralDetector::classifyState()`
(`StructuralDetector.cpp:53-78`), a pure ratio-of-EMA classifier whose near-zero guard
(1e-8 RMS, ≈ -160 dBFS) is far below any real microphone noise floor. In near-silence
or manual mode the classifier keeps running, unguarded, on nothing but ambient
mic/room noise, and its fast(100ms)/slow(4s) RMS ratio sporadically crosses the 0.5×
("breakdown") and 1.8× ("drop") thresholds on pure noise — because the classification
is scale-invariant, it doesn't matter that the absolute levels are tiny.
**The newly-landed P24 fix (`e437872`) is INCOMPLETE, not irrelevant, relative to the
owner's stated intent.** P24 gated the phase-ADVANCE path (`predictedBeatRegime_`) so
counters no longer freeze in silence/manual mode. It never touched this second,
pre-existing, fully independent RESET path in `updatePhrase()`, which is called
unconditionally every hop regardless of `manualMode_`/`inSilence_`/
`predictedBeatRegime_`. Net effect for any oscillator/envelope with `beatDuration_ > 4`
(cycles longer than one bar): the phase no longer freezes, but it now periodically
JUMPS BACKWARD instead — arguably the exact "retrace" half of the owner's complaint,
now happening on a ~1-3 Hz cadence instead of never advancing at all.

## Evidence

### 1. Every write to `barCount_` — exhaustively grepped, only 3 sites total

```
grep -rn "barCount_\s*=" src/ --include="*.cpp" --include="*.h"
BPMTracker.cpp:464   barCount_ = 0;   (updatePhrase, lockedBPM_ <= 0.0f guard)
BPMTracker.cpp:487   barCount_ = 0;   (updatePhrase, structural-transition branch)
BPMTracker.cpp:505   barCount_ = 0;   (resetPhrase(), external API)
BPMTracker.cpp:476   ++barCount_;     (updatePhrase, downbeatDetected_ rising edge)
```
No other file writes `barCount_` (confirmed by grepping every `barCount(` call site
across the codebase — all others are reads via the public accessor or copies into
`FeatureSnapshot`/UI/mapping code). **The prior review's claim that
`updatePhrase()`'s rising-edge detector is the SOLE incrementer is confirmed exactly**
(`BPMTracker.cpp:474-477`). But there are THREE resetters, not one:

- `BPMTracker.cpp:459-468` — `updatePhrase()`, whenever `lockedBPM_ <= 0.0f` (never
  locked, or lock lost). Ruled out for the live measurement: manual mode's
  `setManualBPM()` (`BPMTracker.cpp:510-520`) sets `lockedBPM_` to a positive folded
  value and nothing in the `manualMode_` branch of `runPipeline()`
  (`BPMTracker.cpp:74-82`) ever touches `lockedBPM_` afterward.
- `BPMTracker.cpp:479-489` — structural-transition reset. **This is the live culprit**
  (full mechanism below).
- `BPMTracker.cpp:503-508` — `resetPhrase()`. Only called from two sites, both explicit
  user actions: TopBar's Resync button (`MainComponent.cpp:588-596`) and a
  `Binding::Action::Resync` (MIDI/OSC-bindable, `MainComponent.cpp:5852-5862`). Ruled
  out unless a controller bound to Resync was live and firing during the poll — worth
  a 10-second sanity check but not indicated by anything in the measurement.

### 2. `barCount_` is monotonic-by-epoch, not wrapping — confirmed from the consumer's own comment

`src/signal/OscillatorSignal.h:33-46` (written in an earlier session, S166-L5a, when
this exact tradeoff was already reasoned about):
```
// barCount is bars since the last phrase reset (FeatureSnapshot.h) and
// grows monotonically except on rare structural-transition resets
// (BPMTracker::updatePhrase resets it only on entering a drop or
// leaving a breakdown -- NOT on a fixed period). Folding in
// 4*barCount (beats/bar) extends the phase across bars...
```
This is the design contract the live measurement violates: the comment's "rare" is
only true if `StructuralDetector`'s drop/breakdown classifications are reliable. They
are not reliable at low signal levels, so resets are not rare in that regime — they
happen roughly every 1-3 seconds, an order of magnitude more often than "rare"
structural events in real music.

### 3. What specifically resets it in manual mode with no audio

`AnalysisThread.cpp:160-171`: `bpmTracker_->feedDownbeatFeatures(bassEnergy,
snap->spectralFlux, prevHCDF_, prevStructuralState_)` is called **every hop,
unconditionally** — there is no `manualMode_`/`inSilence_` check anywhere in
`AnalysisThread::run()`'s per-hop pipeline; the ring-buffer hop loop runs on raw mic
input at all times regardless of the BPM tracker's mode.

`BPMTracker.cpp:295-318` (`feedDownbeatFeatures`) calls `updatePhrase(structuralState)`
unconditionally at line 317 — not gated on `predictedBeatRegime_`, `inSilence_`, or
`manualMode_` at all (unlike `scoreBeat()`, which IS gated on `!predictedBeatRegime_`
at line 308). This asymmetry is the actual gap: P24 added a gate to `scoreBeat()`'s
call site but not to `updatePhrase()`'s.

`BPMTracker.cpp:479-489`:
```cpp
if (structuralState != prevStructuralState_) {
    bool resetTransition = (structuralState == 2)
                        || (prevStructuralState_ == 3 && structuralState != 3);
    if (resetTransition) barCount_ = 0;
}
```
fires whenever `structuralState` (one hop stale, per `AnalysisThread.cpp:168,251`)
transitions into "drop" (2) or out of "breakdown" (3).

`StructuralDetector::classifyState()` (`StructuralDetector.cpp:53-78`) computes this
from a pure ratio of 100ms and 4s EMA envelopes of RMS/flux, fed unconditionally every
hop from raw mic RMS/flux (`AnalysisThread.cpp:248-250`, `structuralDetector_->process(rms,
flux, transientDensity)` — no silence check here either). The only silence protection
is `if (slow < 1e-8f) return kNormal;` (`StructuralDetector.cpp:62-63`) — 1e-8 RMS is
≈ -160 dBFS, far below any real mic self-noise floor (typically -60 to -90 dBFS), so
in a real "quiet room" this guard never engages. `BREAKDOWN` fires whenever
`fast < slow * 0.5` (line 74) and `DROP` whenever `fast > slow * 1.8 && onsetRate > 3`
(line 66) — both are scale-invariant ratios, so ordinary ambient-noise variance
(mic self-noise, HVAC, handling noise) is fully capable of crossing them, and does so
on a timescale bounded below by the ~200ms hysteresis hold (`holdThreshold_ =
sampleRate*0.2/hopSize ≈ 18.75 hops` at 93.75 Hz hop rate — `StructuralDetector.cpp:22-24`,
`AnalysisThread.h:45-47`), i.e. up to ~5 state changes/sec are structurally possible.

None of BPMTracker's own P23/P24 silence machinery (`inSilence_`,
`predictedBeatRegime_`) is consulted anywhere in this chain — `StructuralDetector` and
the structural-reset branch of `updatePhrase()` are entirely independent of it.

### 4. Non-reproducibility — two candidate contributors, not yet distinguishable from source alone

(a) **Genuine acoustic non-determinism**: the real ambient noise floor (mic self-noise,
HVAC compressor cycling, handling/room noise) differs slightly moment to moment, so
whether the fast/slow ratio crosses 0.5×/1.3×/1.8× is itself a nondeterministic
function of the room at that instant. A run in a moment of slightly steadier noise
would show no resets; a run during a noisier moment would show many.

(b) **Polling-rate aliasing**: the reset process can occur as often as ~5×/sec
(bounded by the 192ms hysteresis hold above); the observation method polls
`GET /api/bpm` at 1 Hz. A 1 Hz sample of a process changing every ~200ms only sees a
sparse, poll-phase-dependent slice of it. Two runs of the identical underlying
reset RATE could produce visibly different-looking 1 Hz sequences (clean climb to 4
vs. resetting every ~2s) purely from where the polling loop's ticks happen to land
relative to hop boundaries — i.e., part of the "non-reproducibility" may not be system
nondeterminism at all, just an artifact of measuring a fast process slowly.

Both are plausible from source; the experiment below (section OUTPUT) discriminates
between them without any code change.

### 5. Does the same reset happen in normal audio-driven operation, or only manual/silence?

**The mechanism is identical in both regimes** — nothing in `updatePhrase()`,
`feedDownbeatFeatures()`, or `StructuralDetector::process()` branches on
`manualMode_`/`inSilence_`. The difference is only the INPUT: during real music, a
"drop"/"breakdown" classification is often musically legitimate (dance-music drops and
breakdowns are exactly what this reset was built to resync on), so many resets during
real playback are as-designed, not defects. But the same 1e-8 near-zero guard fails to
protect any genuinely quiet passage of real music (ambient intros, pre-drop
breakdowns, quiet verses) just as it fails to protect literal silence — so the SAME
unreliable-at-low-SNR classification is live during normal operation too, at exactly
the moments (quiet passages) where accurate phrase tracking for a VJ show matters
most. This is not confirmed to be equally frequent in real music (real tracks have a
much higher noise floor than room silence, so the ratio may or may not cross as
often) — the live experiment below (part 4) settles this rather than inferring it.

## Recommended fix shape (not implemented — for the owner to choose)

- Symmetric gate: apply the same `predictedBeatRegime_`-style condition P24 added to
  `scoreBeat()`'s call site to `updatePhrase()`'s structural-transition branch — e.g.
  skip/hold the reset while `inSilence_` (or more precisely, while the *StructuralDetector*
  itself has no reliable signal), rather than only gating the phase-advance path.
- Harden `StructuralDetector`'s near-zero guard: 1e-8 is not a real-world floor. Either
  raise it to a level near a real noise floor, or reuse/derive from BPMTracker's own
  `silenceRmsThreshold_` (0.005, `BPMTracker.h:227`) so the same "is there real signal"
  judgment is shared instead of two independent, disagreeing silence notions (the P24
  commit message already flags aubio's -70dB gate and the RMS hysteresis as two
  independently-tuned notions of silence that can disagree — this is a third).
  Require an absolute-level floor in addition to the ratio before trusting
  drop/breakdown, not ratio alone.
- Separate, larger design question the owner may want to weigh in on (already flagged
  in the S166-L5a comment as an accepted tradeoff, but the live measurement shows the
  "rare" assumption underlying that tradeoff doesn't hold): should long-cycle
  (`beatDuration_ > 4`) oscillators/envelopes ever be exposed to phrase-structure
  resets at all, or should they derive phase from an unbroken beat count that
  structural-transition logic never touches? That is a scope decision, not a bug fix.

## OUTPUT — the live experiment (executable now, no code change)

**Setup**: production mode (no `--test-mode`), quiet room, no music — same as the
original measurement. `POST /api/set_bpm {"bpm":120.0}`.

**Capture**: poll `GET /api/features` immediately followed by `GET /api/bpm` in a tight
loop, ~10 Hz (roughly every 100ms, no meaningful sleep needed beyond the HTTP
round-trip), for at least 20 seconds. Log per sample: wall-clock timestamp, `rms`,
`structuralState` (from `/api/features`), `barCount`, `beatInBar` (from `/api/bpm`).

**What discriminates:**

1. **Causation** — does every observed `barCount` drop-to-0 line up (same or
   immediately preceding sample) with `structuralState` transitioning into `2` or out
   of `3`? If yes on all of them, this confirms the mechanism above directly, live. If
   any reset has no adjacent structural transition, the 3-site-exhaustive grep above
   says there is no fourth write site in source — so that result would instead point
   to a FeatureBus read-tearing/race bug, not a fourth resetter, and should be reported
   back rather than assumed.

2. **(a) vs (b) non-reproducibility** — compute resets-per-second from this 10 Hz log,
   then repeat the same 20s capture 2-3 more times back-to-back, untouched room. If the
   reset RATE is roughly stable across repeats (within ~30%), the apparent
   run-to-run "non-reproducibility" in the original 1 Hz measurement was principally a
   polling-aliasing artifact (b) — the underlying process is fairly steady. If the rate
   itself swings wildly between repeats (e.g. ~0/20s vs ~15/20s), that points to genuine
   acoustic/environmental non-determinism (a) — worth checking in that case whether
   anything else touches the input device, or whether the OS/interface applies input
   auto-gain that drifts over tens of seconds.

3. **Numeric plausibility** — from the same log, check whether raw `rms` visibly
   swings by 2×+ between adjacent 100ms samples at the noise floor. `/api/features`
   only exposes instantaneous rms, not `StructuralDetector`'s internal EMA state, so
   this is a coarse proxy, not a direct read of the 0.5×/1.8× ratio — but it corroborates
   whether real-noise variance is of the right order to cross those thresholds.

4. **Real-audio scope (item 5)** — repeat the same dual 10 Hz poll during actual music
   playback that includes at least one genuinely quiet passage a listener would NOT
   call a drop or breakdown (e.g. a quiet intro). If `structuralState` flips to `2` or
   out of `3` — and `barCount` resets — during that passage, the defect is confirmed
   live in normal operation too, not just silence/manual mode, which would outrank the
   s167 lane that just landed.
