# BPM Stability Research: How Professional Systems Detect, Lock, and Structure Tempo

## The Problem

Our current `BPMTracker` passes aubio's raw BPM output directly to `FeatureSnapshot` with zero post-processing. Aubio's `aubio_tempo_get_bpm()` returns a new estimate every hop (10.7ms), and these estimates fluctuate frame-to-frame. The result: BPM readout jumps erratically, beat phase is unstable, and any visual mapping driven by BPM/beatPhase looks jittery and unreliable.

No musician changes tempo every 10 milliseconds. The BPM display should lock onto a value and hold it — exactly like a Pioneer CDJ or Traktor deck display. DJs rarely change tempo at all. The tempo is a property of the track, not a moment-to-moment measurement.

Beyond BPM stability, we're also missing the **metrical hierarchy** that professional DJ tools provide: beats within bars, bars within phrases. A Pioneer CDJ display (see reference image) shows individual beat markers (white lines) AND downbeat markers (orange/red lines at beat 1 of each bar). When a DJ hits "Sync," they're locking to the **downbeat** — beat 1 — not just any beat.

---

## Part 1: How Professional Systems Lock BPM

### The Fundamental Insight: DJ Software Cheats

**Pioneer rekordbox/CDJ, Serato DJ Pro, Traktor, VirtualDJ** — all of them perform **offline pre-analysis** of the entire track before playback. They know every beat position in advance. The BPM displayed on a CDJ is not being detected in real-time; it's read from a pre-computed beat grid.

- **Rekordbox**: Static analysis (single BPM) or Dynamic analysis (multiple BPM markers for variable-tempo tracks). Offers "High Precision" mode. Since v6.6.4, supports cloud-sourced beat grids — if another DJ analyzed that track, you download their grid.
- **Serato**: Three-stage offline pipeline (read → analyze → write BPM to ID3 tags). Elastic beatgrids for live recordings.
- **Traktor**: Offline analysis with 78–155 BPM default range. Considered the most accurate for electronic music.
- **VirtualDJ**: Stable mode (single BPM) or Variable mode (adapting grid). Auto mode follows tempo changes.

**Key takeaway**: These systems achieve perfect BPM stability by *not doing real-time BPM detection at all*. The hard problem — real-time BPM tracking of arbitrary audio — is what we face with Audio-DNA.

### Ableton Link: Tempo Consensus, Not Detection

Ableton Link solves tempo *sharing* between devices, not detection. Uses a decentralized UDP multicast protocol where the last tempo change wins. The key concept is the **quantum** — a number of beats (typically 4) that defines the phase alignment unit. With quantum=4, all Link peers agree on where beat 1 of the bar falls. Stability comes from the "lock and hold" principle: tempo doesn't change unless someone explicitly changes it.

---

## Part 2: Real-Time BPM Tracking Algorithms

Since we must detect BPM from live audio (no pre-analysis possible), these are the relevant approaches:

### 1. Aubio's Algorithm: Davies & Plumbley Two-State Model (What We Use)

**Paper**: Davies & Plumbley 2007, "Context-Dependent Beat Tracking of Musical Audio" (IEEE Trans. Audio, Speech, Language Processing).

Two-state architecture:
1. **General State**: Tempo induction via adaptively-weighted comb filterbank matrices. Searches for tempo candidates.
2. **Context-Dependent State**: Activated when a consistent beat period is observed. Maintains contextual continuity, attenuates off-beat energy, resists tempo changes.

The algorithm *already has* built-in tempo inertia via the two-state switching. **The problem is that we're reading its raw output without any additional stabilization layer.**

Aubio also applies **Rayleigh weighting** to bias tempo estimates toward ~120 BPM.

### 2. Ellis 2007: Dynamic Programming Beat Tracker

**Paper**: Ellis 2007, "Beat Tracking by Dynamic Programming" (Journal of New Music Research).

The **tightness parameter** (default 100) is the key stability concept. It penalizes deviations from the expected inter-beat interval with a log-Gaussian cost: `P(delta) = -(log2(delta / delta_hat))²`. Higher tightness = more tempo inertia.

**Not suitable for us**: This is an offline algorithm (requires DP backtracking over the whole signal). But the *concept* of tightness — making tempo changes expensive — is what we need to implement as post-processing.

### 3. Comb Filter Banks (Scheirer 1998)

Theoretical ancestor of aubio's approach. Banks of parallel comb filters, each tuned to a different tempo, resonating when input energy matches. We already get this through aubio.

### 4. Multi-Hypothesis Tracking (Advanced)

- **Comb Filter Matrix (Stark 2011)**: Multiple tempo+phase hypotheses in a matrix
- **Beatroot Multi-Agent (Dixon)**: Multiple agents track different beat hypotheses
- **Particle Filtering (Hainsworth & Macleod)**: Monte Carlo (tempo, phase) probability distribution
- **BeatNet (Heydari, ISMIR 2021)**: CRNN + cascade particle filter, state-of-the-art

These would require significant new dependencies. Not recommended for our first improvement.

---

## Part 3: Tempo Stabilization Pipeline (Post-Processing)

These techniques are applied **on top of** aubio's raw output. This is the practical path — no new dependencies, all pre-allocatable, fits within our RT budget.

### A. Median Filter (Primary Defense)

A sliding-window median over the last N BPM estimates. **Much better than EMA at rejecting outlier spikes** (half/double BPM glitches, momentary confusion during breakdowns).

```
Window: [120, 120, 120, 60, 120, 120, 120]  → Median: 120
                         ↑ outlier rejected
```

- Window size: 32–64 estimates (~340ms–680ms at our hop rate)
- **Key property**: The median snaps between discrete values rather than drifting gradually. This matches how BPM actually works in music.

### B. Octave Error Correction

The most common BPM instability is the **tempo octave error** — jumping between 60/120/240 BPM. These are all "correct" at different metrical levels.

Once a BPM is locked, check if new estimates are within a factor of 2:
```
ratio = new_bpm / locked_bpm
IF 1.8 < ratio < 2.2:  new_bpm /= 2   // was double
IF 0.45 < ratio < 0.55: new_bpm *= 2   // was half
```

### C. BPM Range Constraint

All DJ software constrains to a user-specified range. Default: 60–200 BPM. Values outside are either rejected or folded via halving/doubling.

### D. Confidence Gating

Aubio provides `aubio_tempo_get_confidence()`. Only update the BPM when confidence exceeds a threshold. During low-confidence periods (breakdowns, silence, noise), hold the last confident value.

### E. Hysteresis Lock (The "DJ Display" Behavior)

Only accept a new BPM if:
1. It differs from the locked BPM by more than ±2 BPM, AND
2. The new value has been consistent for N consecutive frames (~2–4 seconds)

This creates "tempo lock" behavior identical to what DJ hardware displays:
```
IF |candidate_bpm - locked_bpm| > 2.0:
    increment consistency_counter
    IF consistency_counter > 200 hops (~2.1 seconds):
        locked_bpm = candidate_bpm
        consistency_counter = 0
ELSE:
    consistency_counter = 0
    output locked_bpm  // unchanged
```

### F. Light EMA (Cosmetic Only)

After all the above, a very light EMA (alpha ~0.05) for final display smoothing. Applied ONLY after median + hysteresis have done the heavy lifting.

### Recommended Full Pipeline

```
aubio_tempo_get_bpm()  [raw, jittery, ~93 estimates/sec]
        │
        ▼
   BPM Range Gate  [reject < 60 or > 200, fold via halving/doubling]
        │
        ▼
   Confidence Gate  [reject if confidence < threshold; hold last good value]
        │
        ▼
   Octave Error Correction  [snap to current locked octave if within 2x/0.5x]
        │
        ▼
   Median Filter  [window of 48 estimates (~500ms); reject outlier spikes]
        │
        ▼
   Hysteresis Lock  [only change if new median persists for ~2-4 seconds]
        │
        ▼
   snap->bpm  [stable, locked, changes only on real tempo shifts]
```

### Data Structures (All Pre-Allocatable)

```cpp
// Inside BPMTracker — zero steady-state allocation
float bpmHistory_[64];          // circular buffer for median filter
int   bpmHistoryPos_ = 0;
int   bpmHistoryCount_ = 0;
float lockedBPM_ = 0.0f;       // the stable, displayed BPM
float candidateBPM_ = 0.0f;    // potential new BPM being tested
int   consistencyCounter_ = 0; // hops the candidate has been stable
float lastConfidentBPM_ = 0.0f;
float sortBuffer_[64];          // scratch for nth_element median
```

---

## Part 4: Beat Phase — Driven by Locked BPM

### Current Problem

Our `beatPhase_` is computed from `samplesSinceLastBeat_ / currentPeriodSamples_` where `currentPeriodSamples_` comes directly from `aubio_tempo_get_period()`. When aubio's internal period fluctuates, the sawtooth jitters.

### Solution: Free-Running Phase with Beat Reset

Once BPM is locked, the beat phase should ramp linearly based on the locked BPM, and only reset on high-confidence beat detections from aubio:

```cpp
// Each hop:
float lockedPeriodSamples = (sampleRate_ * 60.0f) / lockedBPM_;
phase_ += hopSize_ / lockedPeriodSamples;

if (phase_ >= 1.0f) phase_ -= 1.0f;  // free-running wrap

// On aubio beat detection with high confidence:
if (beatDetected && confidence > 0.5f) {
    phase_ = 0.0f;  // hard reset to align with detected beat
}
```

This gives a perfectly smooth sawtooth that stays aligned with the actual beats via periodic corrections, but doesn't jitter between corrections.

---

## Part 5: Downbeat Detection — Beat 1 vs Beats 2/3/4

### What the CDJ Display Shows

The Pioneer CDJ screenshot shows two types of beat markers on the waveform:
- **White lines**: Individual beats (every beat)
- **Orange/red lines**: Downbeats (beat 1 of each bar)

When a DJ hits "Sync," they're aligning to the **downbeat**, not just any beat. The "138.0 MASTER" display shows locked BPM, and the beat grid shows the full metrical structure: beats within bars.

### Aubio Cannot Help Here

Aubio has **no downbeat detection** (confirmed by maintainer Paul Brossier, GitHub issue #175, still open). `aubio_tempo_was_tatum()` returns tatum subdivisions *within* a beat, not bar-level structure. We must build downbeat detection ourselves.

### How Downbeats Differ Acoustically

In 4/4 time (virtually all EDM/pop/rock), the four beats have a characteristic accent pattern:

| Beat | Accent | Acoustic Markers |
|------|--------|-----------------|
| 1 (downbeat) | **Strong** | Heaviest kick, bass note onset, chord change, arrangement change |
| 2 | Weak | Often snare/clap in EDM, lighter kick |
| 3 | Medium-strong | Secondary kick accent, often similar to beat 1 but lighter |
| 4 | Weak | Snare/clap, fills, pickup notes |

Three feature categories distinguish the downbeat:

1. **Bass energy accent**: Sub-bass (20-60 Hz) and bass (60-250 Hz) energy is typically highest on beat 1. We already have `bandEnergies[0]` (Sub) and `bandEnergies[1]` (Bass).

2. **Harmonic change**: Chord changes align with bar boundaries. Our `harmonicChangeDetection` (HCDF) already captures this — chroma vector distance peaks at downbeats.

3. **Spectral flux accent**: Overall spectral change is greatest at beat 1 due to combined kick+bass+harmonic onset. We have `spectralFlux`.

### Practical Heuristic for EDM (Recommended Approach)

At each detected beat, compute a downbeat score from features we already have:

```
downbeatScore = w1 * bassEnergy + w2 * spectralFlux + w3 * chromaChange
```

Where:
- `bassEnergy = bandEnergies[0] + bandEnergies[1]` (Sub + Bass bands)
- `spectralFlux` = existing feature
- `chromaChange` = existing `harmonicChangeDetection`
- Weights for EDM: `w1=0.5, w2=0.3, w3=0.2` (bass-heavy because EDM kick patterns are extremely regular)

Maintain a circular buffer of the last 16+ beats of scores. Every 4 beats, check which position (0-3) has the highest average score. That position is the downbeat candidate.

After ~8 beats of consistent results, lock the downbeat position and start counting: beat 0 = downbeat, beat 1, beat 2, beat 3, beat 0 (downbeat), ...

### Bar Phase

Once we know which beat is the downbeat:

```cpp
float barPhase = (beatInBar + beatPhase) / beatsPerBar;
// beatInBar: 0, 1, 2, 3
// beatPhase: [0, 1) within current beat
// beatsPerBar: 4 (assuming 4/4 time)
// barPhase: [0, 1) over the entire bar
```

This gives us a smooth [0, 1) sawtooth over 4 beats — the bar-level equivalent of beat phase. Visual mappings driven by bar phase would cycle once per bar rather than once per beat.

### Phrase Phase (Aspirational)

Phrases in music are typically 4, 8, or 16 bars long. Detecting phrase boundaries from audio alone is much harder — there are no consistent acoustic markers. Options:

1. **Simple counting**: Once we have bars, count them: `phrasePhase = (barInPhrase * 4 + beatInBar + beatPhase) / (barsPerPhrase * 4)`. Assume 8-bar phrases.
2. **Structural alignment**: Our `structuralState` (buildup/drop/breakdown) already detects phrase-level changes. Use structural transitions to reset the phrase counter.
3. **Manual resync**: A keyboard shortcut that resets the phrase counter to bar 0, beat 0. **This is what every commercial VJ tool does** — Resolume, VDMX, TouchDesigner all rely on manual phrase sync.

### What Commercial VJ Software Does

**None** of the major VJ tools do automatic downbeat detection from audio:

| Software | Beat Tracking | Downbeat/Bar | Phrase |
|----------|--------------|--------------|--------|
| **Resolume Arena** | Manual BPM entry or tap tempo only | Blue square UI shows beat position in bar | User presses resync button |
| **VDMX** | "Waveclock" plugin for audio BPM | Clock plugin provides bar phase as data source | Manual |
| **TouchDesigner** | Beat CHOP with manual or Link sync | Configurable period (1/2/4/8/32 beats) | Manual |

**Implication**: If we implement even a basic heuristic downbeat detector, we are *ahead* of what commercial VJ software offers. This is a genuine differentiator for Audio-DNA.

---

## Part 6: New FeatureSnapshot Fields

To support the full metrical hierarchy:

```cpp
// Add to FeatureSnapshot:
float bpm = 0.0f;              // EXISTING — but now stabilized/locked
float beatPhase = 0.0f;        // EXISTING — but now driven by locked BPM

// NEW:
uint8_t beatInBar = 0;         // 0-3 (0 = downbeat) — which beat in the bar
float   barPhase = 0.0f;       // [0, 1) over 4 beats — bar-level sawtooth
bool    downbeatDetected = false; // true on the hop where beat 1 lands
uint8_t trackerState = 0;      // 0=searching, 1=locking, 2=locked
```

### How These Drive Visual Mappings

| Mapping Source | Period | Visual Use |
|----------------|--------|-----------|
| `beatPhase` | ~0.5s (120 BPM) | Strobe, pulse, per-beat flash |
| `barPhase` | ~2s (120 BPM) | Color sweep, warp cycle, pattern evolution |
| `phrasePhase` | ~16s (120 BPM, 8-bar phrase) | Scene transition, global intensity arc |
| `beatInBar` | Discrete 0-3 | Stepped effects (different color per beat) |
| `downbeatDetected` | Boolean trigger | Trigger scene change, image swap on beat 1 |

---

## Part 7: Implementation Plan

### Phase 1: BPM Stabilization (Highest Priority)

Modify `BPMTracker` to add the post-processing pipeline:
1. Range gate (60–200 BPM)
2. Confidence gate
3. Octave error correction
4. Median filter (window of 48)
5. Hysteresis lock (2-second persistence required)
6. Drive beat phase from locked BPM with beat-aligned resets

**All inside `BPMTracker.h/cpp`.** No new files. All buffers pre-allocated in constructor. Zero steady-state allocation.

### Phase 2: Downbeat Detection

Add a `DownbeatDetector` class (or extend `BPMTracker`):
1. On each beat detection from aubio, capture `bassEnergy`, `spectralFlux`, `harmonicChangeDetection` from the current FeatureSnapshot
2. Score each beat position (0-3) using weighted combination
3. After 8+ beats of consistent scoring, lock the downbeat position
4. Compute `beatInBar`, `barPhase`, `downbeatDetected`

**Requires**: Access to spectral features at beat time. Either pass them into `BPMTracker::process()` or create a separate class that the AnalysisThread feeds after both BPM and spectral stages.

### Phase 3: UI Enhancement

- BPM display: Show locked BPM with tracker state indicator (searching/locking/locked)
- Beat phase visualizer: Show 4-beat bar structure (like the CDJ's beat markers)
- Manual resync button: Reset downbeat position on user action

### Phase 4 (Optional): Phrase Tracking

- Count bars from downbeat
- Reset phrase counter on structural transitions
- Manual phrase resync shortcut

---

## Key Academic References

| Paper | Year | Key Contribution | Relevance |
|-------|------|-----------------|-----------|
| Scheirer, "Tempo and Beat Analysis" | 1998 | Comb filter banks for beat tracking | Foundation of aubio's approach |
| Davies & Plumbley, "Context-Dependent Beat Tracking" | 2007 | Two-state model with tempo inertia | **This is aubio's algorithm** |
| Ellis, "Beat Tracking by Dynamic Programming" | 2007 | Tightness parameter for tempo stability | Tightness concept applies to our post-processing |
| Stark, "Real-time Visual Beat Tracking using Comb Filter Matrix" | 2011 | Multi-hypothesis real-time tracking | More sophisticated future option |
| Heydari et al., "BeatNet" | 2021 | CRNN + particle filter, SOTA real-time | Would require ML dependency |
| Bock, Krebs, Widmer, "Multi-model Beat Tracking" | 2014 | Style-aware beat tracking | Genre-specific BPM ranges |
| Durand, David, Richard, "Downbeat Detection with Conditional Random Fields" | 2015 | Spectral difference downbeat approach | 72% accuracy without ML |

---

## Summary

### Why Our BPM Jumps
We pass aubio's raw per-hop BPM estimate directly to the render thread with zero stabilization. Aubio fluctuates ±2-5 BPM frame-to-frame with occasional octave jumps.

### What the Pros Do
Pre-analyze offline (not applicable to live audio). DJs essentially *never* need real-time BPM detection.

### What We Should Do

**BPM Lock** (Phase 1): Range gate → confidence gate → octave correction → median filter → hysteresis lock. Expected result: BPM locks in 2-4 seconds, stays rock-solid, changes only on genuine tempo shifts.

**Beat Phase** (Phase 1): Drive from locked BPM with periodic corrections on high-confidence beat detections. Perfectly smooth sawtooth.

**Downbeat Detection** (Phase 2): Score beats using bass energy + spectral flux + harmonic change. Lock downbeat position after 8+ consistent beats. Compute bar phase as [0,1) over 4 beats.

**Phrase Tracking** (Phase 4): Count bars from downbeat, reset on structural transitions, manual resync as fallback.

**No new dependencies required.** All post-processing is simple DSP (median, EMA, comparisons, scoring) that fits within our existing pre-allocated buffer paradigm and RT constraints. Total additional memory: ~512 bytes of pre-allocated buffers.

**Competitive advantage**: No commercial VJ software does automatic downbeat detection from live audio. If we get Phase 2 working, we're ahead of Resolume, VDMX, and TouchDesigner on this front.
