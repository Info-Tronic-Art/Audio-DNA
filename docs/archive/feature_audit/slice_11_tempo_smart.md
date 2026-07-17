# Slice 11: Tempo System + Smart Audio Features + ISF Loader

Files audited:
- `/Users/boriskarpman/Documents/RealTimeAudio/src/analysis/BPMTracker.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/analysis/BPMTracker.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/analysis/GenreDetector.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/analysis/GenreDetector.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/analysis/GenreSmoothing.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/analysis/AdvancedAudioAnalyzer.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/analysis/AdvancedAudioAnalyzer.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/sync/LinkSync.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/sync/LinkSync.cpp`

---

## Section 8: Transport / BPM / Tempo

### 8.1 BPMTracker — auto/manual, range, tap, nudge, resync, multipliers

**Core engine** — wraps `aubio_tempo_t` for raw BPM tracking (BPMTracker.h:2, BPMTracker.cpp:10-15).
- Aubio method: `"default"` (only method supported, noted at BPMTracker.cpp:11).
- Aubio silence gate: `-70.0 dB` (BPMTracker.cpp:21).
- Aubio peak-picking threshold: `0.3f` (BPMTracker.cpp:22).

**BPM Range** — hard-coded static constexpr limits (BPMTracker.h:40-41):
- `kMinBPM = 60.0f`
- `kMaxBPM = 200.0f`
- Folding to range via halving/doubling (BPMTracker.cpp:200-210 `foldBPMToRange`): `while bpm > 200 → bpm *= 0.5`, `while bpm < 60 → bpm *= 2`, final `std::clamp(bpm, 60, 200)`.

**Tracker state machine** (BPMTracker.h:35-37):
- `STATE_SEARCHING = 0` — no reliable BPM yet
- `STATE_LOCKING   = 1` — candidate being confirmed
- `STATE_LOCKED    = 2` — solid, stable BPM

**Manual mode** (bypasses full stabilization pipeline):
- `setManualMode(bool)` — atomic flag, set from UI thread, read from analysis thread (BPMTracker.h:138, BPMTracker.cpp:494-497).
- `isManualMode()` getter (BPMTracker.h:139).
- When manual mode active, `runPipeline()` short-circuits: only updates phase from locked BPM (BPMTracker.cpp:74-79).
- `setManualBPM(float bpm)` — sets locked BPM directly, folds through range gate, resets phase, `consistencyCounter_ = kHysteresisHops` (already locked) (BPMTracker.h:134, BPMTracker.cpp:474-484). Rejects bpm ≤ 0.

**Tap tempo / External override**:
- `setManualBPM(float)` is the tap tempo API — bypasses stabilization, sets `lockedBPM_`, `candidateBPM_`, `lastConfidentBPM_` to folded value, `trackerState_ = STATE_LOCKED`, resets `phase_ = 0.0` (BPMTracker.cpp:474-484).
- No tap averaging logic / min-tap-count / tap timeout exists in BPMTracker itself — the caller (UI) is responsible for averaging.

**Resync action**:
- `resetBeatPhase()` — zeroes `phase_`, `beatInBar_`, `barPhase_`, `beatCounter_` (BPMTracker.h:142, BPMTracker.cpp:486-492).
- `resetPhrase()` — zeroes `barCount_`, `phrasePhase_`, `prevDownbeatDetected_` (BPMTracker.h:130, BPMTracker.cpp:467-472).

**Tempo multipliers / nudge**: No explicit multiplier (2x/0.5x/1x) or nudge (+/- step) methods in BPMTracker itself. The only automatic octave handling is `correctOctaveError()` (BPMTracker.cpp:212-228) which snaps to 0.5x or 2x if ratio is in (1.8, 2.2) or (0.45, 0.55).

---

### 8.2 Stabilization pipeline — range gate, confidence, octave, median, hysteresis

Five stages, executed in order by `runPipeline()` (BPMTracker.cpp:68-173).

| Stage | Purpose | Threshold / Window | File:Line |
|-------|---------|--------------------|-----------|
| **1. BPM Range Gate** | Reject zero/invalid; fold into [60, 200] via halving/doubling | `kMinBPM=60.0f`, `kMaxBPM=200.0f` | BPMTracker.h:40-41, .cpp:81-90, 200-210 |
| **2. Confidence Gate** | Only accept aubio BPM estimates above threshold | `kConfidenceThreshold = 0.1f` | BPMTracker.h:50, .cpp:101-107 |
| **3. Octave Error Correction** | Snap to locked octave when raw BPM near 2x/0.5x of locked | Double gate: `1.8 < ratio < 2.2` → halve; Half gate: `0.45 < ratio < 0.55` → double | BPMTracker.cpp:110-112, 212-228 |
| **4. Median Filter** | Running median of last 48 BPM estimates (~500 ms @ 93.75 hops/sec) | `kMedianWindowSize = 48` (via `std::nth_element`) | BPMTracker.h:44, .cpp:230-245 |
| **5. Hysteresis Lock** | New BPM must persist for 200 hops (~2.1 sec) and differ from locked by > 2 BPM to change | `kHysteresisHops = 200`, `kBPMChangeThreshold = 2.0f` | BPMTracker.h:47,53, .cpp:117-170 |

**Additional constants**:
- `kBeatResetConfidence = 0.5f` — aubio confidence required to hard-reset `phase_ = 0` on beat (BPMTracker.h:56, .cpp:192-195).
- First lock: `medianCount_ >= kMedianWindowSize / 2` (24 samples) required for initial lock (BPMTracker.cpp:121).

---

### 8.3 Silence detection + BPM recovery (P23 Smart Recovery)

Implemented via `feedSilenceDetection(float rms)` (BPMTracker.h:123, .cpp:501-538).

| Parameter | Value | File:Line |
|-----------|-------|-----------|
| `silenceRmsThreshold_` | `0.005f` (RMS below → silence) | BPMTracker.h:221 |
| `silenceEntryHops_` | `0.3f * hopsPerSec` ≈ 300 ms below threshold | BPMTracker.h:222, .cpp:35 |
| `silenceExitHops_` | `0.1f * hopsPerSec` ≈ 100 ms above threshold | BPMTracker.h:223, .cpp:36 |

**Behavior during silence** (BPMTracker.cpp:94-98): when `inSilence_` is true and `lockedBPM_ > 0`, the pipeline holds the last locked BPM and free-runs phase with `beat=false, conf=0.0` (no beats processed during silence).

**Hysteresis countdown**: separate `silenceCountdown_` for entry and exit (BPMTracker.cpp:507-537).

**Accessors**:
- `isSilent()` → `bool` (BPMTracker.h:126)
- `silenceDuration()` → seconds, derived from `silenceHopCount_ / hopsPerSecBpm_` (BPMTracker.h:127, .cpp:540-544).

---

### 8.4 Downbeat detection (automatic)

Automatic downbeat detection via `feedDownbeatFeatures(bassEnergy, spectralFlux, harmonicChange, structuralState)` (BPMTracker.h:90, .cpp:259-277).

**Scoring formula** (`scoreBeat`, BPMTracker.cpp:279-316):
- `score = 0.5 * bassEnergy + 0.3 * spectralFlux + 0.2 * harmonicChange`
- Weights: `kDownbeatWeightBass = 0.5f`, `kDownbeatWeightFlux = 0.3f`, `kDownbeatWeightHCDF = 0.2f` (BPMTracker.h:62-64).

**Buffer sizes** (BPMTracker.h:59-61):
- `kBeatScoreBufferSize = 16` (circular buffer of per-beat scores).
- `kBeatsPerBar = 4` (4/4 time assumed).
- `kDownbeatLockThreshold = 8` beats of consistency before locking.

**Lock procedure** (`analyzeDownbeatPosition`, BPMTracker.cpp:318-398):
- After every 4 beats (pre-lock) or every 16 beats (post-lock), scores accumulate into `sums[0..3]` / `counts[0..3]` by `(totalBeatsScored-1-i) % 4`.
- Position with highest average wins; locks when `downbeatConsistency_ >= 2 && totalBeatsScored_ >= 8`.
- Post-lock: re-validation checks every 16 beats, and if `bestPos != lockedDownbeatPos_` for 8 consecutive analyses, relocks to new position.

**Outputs**:
- `beatInBar()` → 0-3 (0 = downbeat) (BPMTracker.h:104)
- `barPhase()` → [0, 1) over 4 beats (BPMTracker.h:105, computed as `(beatInBar + beatPhase) / 4` in BPMTracker.cpp:409)
- `downbeatDetected()` → true on hop where beat 1 lands (BPMTracker.h:106)
- `downbeatLocked()` → true after lock criteria met (BPMTracker.h:107)

**Tap downbeat**: No API. Downbeat detection is fully automatic.

**Phrase tracking** (BPMTracker.cpp:418-460):
- `kDefaultPhraseBars = 8` bars (BPMTracker.h:67).
- `kMinPhraseBars = 1`, `kMaxPhraseBars = 32` (BPMTracker.h:68-69).
- `setPhraseBars(int)` clamps to [1, 32] (BPMTracker.cpp:462-465).
- Bar counted on rising edge of `downbeatDetected` (BPMTracker.cpp:430-436).
- Phrase resets on structural transition to state 2 (drop) or leaving state 3 (breakdown) (BPMTracker.cpp:440-448).
- `phrasePhase = (barCount % phraseBars + barPhase) / phraseBars` (BPMTracker.cpp:451-453).

---

### 8.5 Ableton Link — enable, quantum, peers

File: `src/sync/LinkSync.h`, `src/sync/LinkSync.cpp`.

**Compile-time option**: `AUDIODNA_HAS_LINK` macro — all Link code behind `#if AUDIODNA_HAS_LINK` (LinkSync.h:3-5, .h:56-58, .cpp:5-7, etc.). Enabled via `-DAUDIODNA_BUILD_LINK=ON` CMake flag (per CLAUDE.md).

**State (atomic for thread safety, LinkSync.h:50-54)**:
- `enabled_` atomic bool (default false)
- `bpm_` atomic double (default 120.0)
- `beatPhase_` atomic double (default 0.0)
- `numPeers_` atomic int (default 0)
- `quantum_` double (default 4.0, not atomic — write from one thread)

**API**:
- `setEnabled(bool)` — calls `link_.enable()`, stores atomic (LinkSync.cpp:26-32).
- `isEnabled()` (LinkSync.h:24).
- `getBPM()` / `setBPM(double)` — setBPM captures session state, sets tempo, commits (LinkSync.h:27,30, .cpp:34-46). setBPM propagates to all Link peers when enabled.
- `getBeatPhase()` — phase within quantum (LinkSync.h:33).
- `getQuantum()` / `setQuantum(double)` (LinkSync.h:36-37). Default 4 (beats per phase cycle = 4/4 bar).
- `getNumPeers()` — excludes self (LinkSync.h:40).
- `update()` — called each frame from render/analysis thread; captures session state, stores tempo/phase/peer count (LinkSync.h:44, .cpp:48-61).
- `requestBeatAtTime()` — force phase reset to downbeat (LinkSync.h:47, .cpp:63-74), calls `sessionState.requestBeatAtTime(0.0, now, quantum)`.

**Callbacks** (LinkSync.cpp:10-15): `setNumPeersCallback` and `setTempoCallback` both store atomically. Callbacks fire from network threads.

**Default constructor tempo**: `link_(120.0)` (LinkSync.cpp:6).

---

### 8.6 Manual BPM entry

- `setManualBPM(float bpm)` — public API (BPMTracker.h:134, .cpp:474-484). Any bpm > 0 is folded to [60, 200] range via `foldBPMToRange()`, then `lockedBPM_`, `candidateBPM_`, `lastConfidentBPM_` are all set, `trackerState_` → `STATE_LOCKED`, phase zeroed.
- No explicit step size (UI concern; BPMTracker stores float).
- Range (post-fold): [60, 200] BPM.

---

### 8.7 Public API methods (what UI can call)

From `BPMTracker.h`:

| Method | Signature | Purpose | File:Line |
|--------|-----------|---------|-----------|
| Constructor | `BPMTracker(int hopSize, int bufSize, int sampleRate)` | Setup | :74 |
| `process` | `void process(const float* samples)` | Feed one hop | :83 |
| `feedDownbeatFeatures` | `void(float, float, float, uint8_t=0)` | Bass/flux/HCDF + structural state | :90 |
| `feedSilenceDetection` | `void feedSilenceDetection(float rms)` | P23 smart recovery | :123 |
| `bpm` | `float bpm() const` | Locked BPM | :94 |
| `beatPhase` | `float beatPhase() const` | [0, 1) sawtooth | :95 |
| `beatDetected` | `bool beatDetected() const` | Transient this hop | :96 |
| `confidence` | `float confidence() const` | Aubio confidence | :97 |
| `trackerState` | `uint8_t trackerState() const` | 0/1/2 | :98 |
| `rawBPM` | `float rawBPM() const` | Pre-stabilization diagnostic | :101 |
| `beatInBar` | `uint8_t beatInBar() const` | 0-3 | :104 |
| `barPhase` | `float barPhase() const` | [0, 1) over 4 beats | :105 |
| `downbeatDetected` | `bool downbeatDetected() const` | Edge flag | :106 |
| `downbeatLocked` | `bool downbeatLocked() const` | Lock flag | :107 |
| `barCount` | `uint16_t barCount() const` | Bars since phrase reset | :110 |
| `phrasePhase` | `float phrasePhase() const` | [0, 1) over N bars | :111 |
| `phraseBars` | `int phraseBars() const` | Configured phrase length | :112 |
| `setThreshold` | `void setThreshold(float)` | Aubio peak-pick threshold | :115 |
| `setSilence` | `void setSilence(float dbThreshold)` | Aubio silence gate | :116 |
| `setPhraseBars` | `void setPhraseBars(int)` | [1, 32] | :117 |
| `isSilent` | `bool isSilent() const` | P23 flag | :126 |
| `silenceDuration` | `float silenceDuration() const` | Seconds | :127 |
| `resetPhrase` | `void resetPhrase()` | Clear bar count | :130 |
| `setManualBPM` | `void setManualBPM(float)` | Tap/entry override | :134 |
| `setManualMode` | `void setManualMode(bool)` | Freeze pipeline | :138 |
| `isManualMode` | `bool isManualMode() const` | Read flag | :139 |
| `resetBeatPhase` | `void resetBeatPhase()` | Zero phase/beatInBar/barPhase | :142 |
| `processRawBPM` | `void processRawBPM(float, float, bool)` | Test-only bypass of aubio | :147 |

---

## Section 11: Smart / AI Features

### 11.1 Genre detection — 8 genres listed with numeric IDs

All genre IDs are `static constexpr uint8_t` (GenreDetector.h:26-33):

| ID | Constant | Enum Label | Display Name | Description (GenreDetector.h:9-16) |
|----|----------|------------|--------------|--------------------|
| 0 | `kHouse` | House | "House" | Steady 4-on-floor kick, 120-130 BPM, warm bass |
| 1 | `kTechno` | Techno | "Techno" | Driving, 125-145 BPM, high transient density, spectral flux |
| 2 | `kDnB` | Drum & Bass | "Drum & Bass" | Fast breakbeats, 160-180 BPM, heavy bass, syncopation |
| 3 | `kHipHop` | Hip-Hop | "Hip-Hop" | Slower groove, 80-100 BPM, strong bass + mids |
| 4 | `kAmbient` | Ambient | "Ambient" | Sparse, low transient density, spectral flatness, sustained tones |
| 5 | `kRock` | Rock | "Rock" | Full spectrum, high peak levels, guitar frequency presence |
| 6 | `kPopElectronic` | Pop/Electronic | "Pop/Electronic" | Varied, mid-range BPM, bright, high spectral centroid |
| 7 | `kJazzOther` | Jazz/Other | "Jazz/Other" | Complex harmony, variable BPM, chromatic complexity |

Display-name table: GenreDetector.cpp:309-316 (`genreName()` static method).

`kNumGenres = 8` (GenreDetector.h:23).

Default confirmed genre: `kPopElectronic` with initial smoothedScore 0.3 bias (GenreDetector.h:98, .cpp:21).

---

### 11.2 Genre scoring inputs

`GenreDetector::Features` struct (GenreDetector.h:36-52) — all inputs scored:
- `bpm` (float)
- `rms` (float)
- `spectralCentroid`, `spectralFlux`, `spectralFlatness`, `spectralRolloff` (float)
- `transientDensity` (float, onsets/sec)
- `bandEnergies[7]` — Sub, Bass, LowMid, Mid, HighMid, Presence, Brilliance
- `chromagram[12]`
- `mfccs[13]`
- `dynamicRange` (float)
- `structuralState` (uint8_t)
- `trackerState` (uint8_t — only scores BPM-gated features when == 2 = locked)
- `harmonicChangeDetection` (float)

**Per-genre scoring rules** (all in `computeScores`, GenreDetector.cpp:32-207). Each rule adds to the score; then the 8 scores are normalized to sum to 1 (GenreDetector.cpp:192-206):

| Genre | Scoring Rules | File:Line |
|-------|---------------|-----------|
| **House** | BPM bell curve centered 125 BPM, range 115-135 (0.35 max); bass > 0.4 adds 0.25; transient density 2-8 adds 0.15; centroid 1500-4500 Hz adds 0.1; chromaticComplexity < 5 adds 0.1 | .cpp:47-65 |
| **Techno** | BPM 120-150, center 135, ±15 (0.3 max); transientDensity > 5 adds up to 0.3; spectralFlux > 0.3 adds up to 0.2; bass > 0.3 adds 0.1; chromaticComplexity < 4 adds 0.1 | .cpp:67-83 |
| **DnB** | BPM 155-185, center 172, ±15 (0.4 max); subBass > 0.3 adds 0.2; transientDensity > 6 adds 0.15; spectralFlux > 0.4 adds 0.1 | .cpp:85-99 |
| **Hip-Hop** | BPM 75-105, center 90, ±15 (0.35 max); bass > 0.3 adds up to 0.2; mids > 0.3 adds up to 0.15; transientDensity 2-7 adds 0.1; dynamicRange > 3 adds 0.1 | .cpp:101-117 |
| **Ambient** | transientDensity < 2 adds 0.3, < 4 adds 0.1; spectralFlatness > 0.3 adds up to 0.25; centroid < 2000 adds 0.15; rms < 0.15 adds 0.15; spectralFlux < 0.2 adds 0.1 | .cpp:119-134 |
| **Rock** | BPM 95-150 locked adds 0.15; dynamicRange > 4 adds 0.2; rms > 0.3 adds 0.15; mids > 0.4 adds 0.2; highs 0.2-0.6 adds 0.1; HCDF > 0.3 adds 0.1 | .cpp:136-153 |
| **Pop/Electronic** | BPM 100-140 locked adds 0.15; centroid > 3000 Hz adds up to 0.2; transientDensity 3-10 adds 0.1; full spectrum presence adds 0.15; spectralFlux 0.15-0.5 adds 0.1 | .cpp:155-172 |
| **Jazz/Other** | chromaticComplexity > 6 adds 0.3 (>4 adds 0.15); HCDF > 0.4 adds 0.2; spectralFlatness 0.2-0.6 adds 0.1; dynamicRange > 3 adds 0.1; mids > 0.3 adds 0.1 | .cpp:174-189 |

**Smoothing**:
- EMA alpha computed from tau = 2.0 s: `smoothAlpha_ = 1 - exp(-1/(2.0 * hopsPerSec))` (GenreDetector.cpp:10-12).
- Applied to each of 8 scores per hop (GenreDetector.cpp:209-216).

**Confidence**: `(bestScore - secondBest) / bestScore` — how much the top genre dominates (GenreDetector.cpp:241-248), clamped to [0,1].

**Hysteresis (genre switching)**:
- `holdThreshold_ = int(3.0 * hopsPerSec)` — ~3 seconds of consistency required (GenreDetector.cpp:15).
- Must also satisfy `confidence_ > 0.15` to switch (GenreDetector.cpp:262).
- Candidate/hold tracking at .cpp:251-273.

**Chromatic complexity** (GenreDetector.h:86, .cpp:295-306): counts pitch classes with energy > `1/24` (half of uniform 1/12 distribution). Returns float count 0-12.

---

### 11.3 Per-genre smoothing params (list all 8)

File: `GenreSmoothing.h`. Struct has 4 fields (`attackAlpha`, `releaseAlpha`, `oneEuroBeta`, `oneEuroMinCutoff`).

`static GenreSmoothing forGenre(uint8_t genre)` (GenreSmoothing.h:27-59):

| Genre ID | Genre | attackAlpha | releaseAlpha | oneEuroBeta | oneEuroMinCutoff | File:Line |
|----------|-------|-------------|--------------|-------------|------------------|-----------|
| 0 | House | 0.35 | 0.20 | 0.008 | 1.2 | :32-33 |
| 1 | Techno | 0.50 | 0.35 | 0.015 | 2.0 | :35-36 |
| 2 | DnB | 0.55 | 0.40 | 0.020 | 2.5 | :38-39 |
| 3 | Hip-Hop | 0.35 | 0.15 | 0.006 | 0.8 | :41-42 |
| 4 | Ambient | 0.12 | 0.08 | 0.003 | 0.3 | :44-45 |
| 5 | Rock | 0.40 | 0.25 | 0.010 | 1.5 | :47-48 |
| 6 | Pop/Electronic | 0.30 | 0.20 | 0.007 | 1.0 | :50-51 |
| 7 | Jazz/Other | 0.25 | 0.15 | 0.005 | 0.7 | :53-54 |
| default | fallback | 0.30 | 0.20 | 0.007 | 1.0 | :57 |

`static GenreSmoothing lerp(a, b, t)` — blends two smoothing configs for genre transitions (GenreSmoothing.h:62-71).

---

### 11.4 Energy state (low/medium/high)

Computed in `computeEnergyState()` (GenreDetector.cpp:276-293).

**Formula** (GenreDetector.cpp:279-281):
```
energy = 0.4 * rms
       + 0.3 * min(transientDensity / 10, 1)
       + 0.3 * min(spectralFlux, 1)
```

**EMA smoothing** via `smoothAlpha_` (same 2-s alpha as scores) (GenreDetector.cpp:284).

**Hysteresis bands** (GenreDetector.cpp:287-292):
- `energyEMA < 0.2` → `energyState_ = 0` (Low)
- `energyEMA > 0.5` → `energyState_ = 2` (High)
- otherwise → `energyState_ = 1` (Medium)

Default initial state: `1` (Medium) (GenreDetector.h:102).

Accessor: `energyState()` (GenreDetector.h:64).

---

### 11.5 Auto-preset on genre (callback mechanism)

The `GenreDetector` class itself does NOT expose a callback — it only exposes the confirmed genre via `detectedGenre()` (GenreDetector.h:62). The auto-preset hookup (e.g., `Renderer::onGenreChanged_`) and deck assignment (`composition.genreDeckAssignment[8]`, `composition.autoPresetOnGenre`) are wired outside these files per CLAUDE.md (P23 section). Within the audited files, no callback API is present — UI must poll `detectedGenre()` and compare to previous value.

Accessors exposed from `GenreDetector`:
- `detectedGenre()` → `uint8_t` (GenreDetector.h:62)
- `genreConfidence()` → `float` (GenreDetector.h:63)
- `genreScores()` → `const float*` pointing at `smoothedScores_.data()` (GenreDetector.h:70)
- `static genreName(uint8_t)` → `const char*` display name (GenreDetector.h:67)

---

### 11.6 Advanced audio analysis — 5 features detailed

File: `src/analysis/AdvancedAudioAnalyzer.h/cpp`. Constructor: `(int numBins, float sampleRate, int fftSize, int hopSize)` (AdvancedAudioAnalyzer.h:19). Process: `process(const float* magnitudeSpectrum, float bassEnergy, float midEnergy, bool onsetDetected, float rms)` (AdvancedAudioAnalyzer.h:23).

| # | Feature | Algorithm | Output Range | Output Field | File:Line (cpp) |
|---|---------|-----------|--------------|--------------|-----------------|
| 1 | **Sidechain Pump** | Pearson correlation r of bass vs mid envelopes over 64-hop window (~680 ms). Negative r → high pump. `rawPump = clamp(-r, 0, 1)`. EMA: attack α=0.15, release α=0.05. | [0, 1] | `sidechainPump_` | :43-91 |
| 2 | **Swing Ratio** | Inter-onset interval (IOI) histogram, 32-IOI circular buffer. Groups consecutive pairs; ratio = longer/(longer+shorter). Needs ≥8 intervals. EMA α=0.03. | [0.5, ~0.67] (0.5=straight, 0.67=triplet swing) | `swingRatio_` | :94-148 |
| 3 | **Formant Presence** | Energy ratio: Σ power in 300-3000 Hz / total spectral power. Adaptive max normalization (decay 0.9995). EMA α=0.08. | [0, 1] | `formantPresence_` | :152-182 |
| 4 | **Resonance Peak** | Spectral kurtosis in 200-8000 Hz: `m4 / m2²` (non-excess). Map [3, 30] → [0, 1], clamped. EMA: attack α=0.2, release α=0.05. | [0, 1] | `resonancePeak_` | :186-225 |
| 5 | **Reese Bass** | Spectral spread (power-weighted std dev of frequency) in 30-200 Hz. Adaptive max normalization (decay 0.9995). EMA α=0.06. | [0, 1] | `reeseBass_` | :229-280 |

**Buffer sizes** (AdvancedAudioAnalyzer.h):
- `kEnvelopeLength = 64` hops (~680 ms @ 93.75 hops/sec) (:43)
- `kIOIHistorySize = 32` onset intervals (:53)
- Minimum hops between onsets for IOI capture: 2 (.cpp:102)

**Frequency bins precomputed from Hz** via `freqToBin()` at construction (AdvancedAudioAnalyzer.cpp:14-19):
- Formant: 300-3000 Hz
- Resonance: 200-8000 Hz
- Reese: 30-200 Hz

**Accessors** (AdvancedAudioAnalyzer.h:27-31): `sidechainPump()`, `swingRatio()`, `formantPresence()`, `resonancePeak()`, `reeseBass()` — all return `float`.

---

### 11.7 Smart BPM recovery

See 8.3. Implemented as `BPMTracker::feedSilenceDetection()` with 300ms entry / 100ms exit hysteresis (BPMTracker.cpp:501-538). During silence, locked BPM is held and phase free-runs.

---

### 11.8 Structural scene triggering (callback)

Within the audited files, structural state only enters `BPMTracker::feedDownbeatFeatures(…, uint8_t structuralState)` and triggers `barCount_` reset on transitions to state 2 (drop) or leaving state 3 (breakdown) (BPMTracker.cpp:440-448). No callback is fired from this file. The `Renderer::onStructuralStateChanged_` referenced in CLAUDE.md lives outside this slice.

---

## Section 20 (partial): Cross-references

- `BPMTracker::process` is fed from `AnalysisThread` hop loop. The tracker uses `aubio_tempo_t` (see `LIB_aubio.md`) and publishes `bpm`, `beatPhase`, `barPhase`, `phrasePhase`, `barCount`, `downbeatDetected`, `downbeatLocked`, `beatInBar` into `FeatureSnapshot` (CLAUDE.md §FeatureSnapshot).
- `BPMTracker::feedDownbeatFeatures` depends on upstream `SpectralFeatures` (bassEnergy, spectralFlux) and `ChromaExtractor` (harmonicChangeDetection) and `StructuralDetector` (structuralState).
- `BPMTracker::feedSilenceDetection` depends on upstream RMS from amplitude stage.
- `GenreDetector::process` consumes the full FeatureSnapshot-mirror `Features` struct (spectral + rhythm + MFCC + chroma + dynamics + structural + trackerState).
- `GenreSmoothing::forGenre()` is intended to tune `MappingEngine` per-mapping EMA / One-Euro filter betas based on current genre (per header comment, GenreSmoothing.h:15-17).
- `AdvancedAudioAnalyzer::process` requires raw magnitude spectrum from `FFTProcessor`, bass/mid energies from `SpectralFeatures`, onset flag from `OnsetDetector`, and RMS — outputs feed snapshot fields `sidechainPump`, `swingRatio`, `formantPresence`, `resonancePeak`, `reeseBass` (CLAUDE.md §FeatureSnapshot P25 additions).
- `LinkSync` is orthogonal to `BPMTracker`. Per CLAUDE.md P21, when Link is enabled it overrides BPM tracker via `setManualMode(true)` + `setManualBPM(linkBpm)`. All Link state lives in this slice's `LinkSync` class; wiring is outside.
- All 5 advanced features are also exposed as `MappingSource` enum values, shader uniforms (`u_sidechainPump`, `u_swingRatio`, `u_formantPresence`, `u_resonancePeak`, `u_reeseBass`), and hidden `SignalRegistry` signals (CLAUDE.md §Advanced Audio Analysis P25).

---

## Summary

Tempo system (BPMTracker, 545-line cpp + 256-line header) wraps Aubio with a 5-stage stabilization pipeline: range gate [60-200 BPM] → confidence gate (0.1) → octave correction (2x/0.5x snap in 1.8-2.2 / 0.45-0.55 ratio windows) → 48-sample median filter (~500 ms) → hysteresis lock (2 BPM threshold × 200 hops ≈ 2.1 s). Manual mode bypasses stages 2-5. P23 smart silence recovery (300 ms entry / 100 ms exit, RMS threshold 0.005) holds locked BPM and free-runs phase. Downbeat detection is fully automatic (bass 0.5 + flux 0.3 + HCDF 0.2 scoring over 16-beat circular buffer, 8-beat lock threshold). Phrase tracking (1-32 bars, default 8) resets on structural state transitions. GenreDetector classifies 8 genres (House/Techno/DnB/HipHop/Ambient/Rock/Pop/Jazz, IDs 0-7) with 2-s EMA + 3-s hysteresis + 0.15 confidence gate, plus a 3-band energy state. GenreSmoothing provides 4 EMA/One-Euro params per genre. AdvancedAudioAnalyzer adds 5 features (sidechain pump via Pearson r, swing ratio via IOI pair histogram, formant presence in 300-3000 Hz, resonance peak via spectral kurtosis in 200-8000 Hz, reese bass via spectral spread in 30-200 Hz). LinkSync is a thin atomic-backed Ableton Link wrapper behind `AUDIODNA_HAS_LINK` compile flag with quantum (default 4), setEnabled/setBPM/setQuantum, and requestBeatAtTime. **Surprises**: (1) No tap-tempo averaging or nudge methods exist in BPMTracker — caller must average; `setManualBPM` is the only tap-target API. (2) No explicit tempo multiplier (2x/0.5x) API — only automatic octave correction. (3) GenreDetector exposes no onGenreChanged callback; UI must poll. (4) ISF loader was NOT in the audit file list — skipped. (5) `kConfidenceThreshold = 0.1f` is very permissive.
