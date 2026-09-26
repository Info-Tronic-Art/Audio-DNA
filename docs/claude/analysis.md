# Audio Analysis Features Reference

> Moved from CLAUDE.md (claudemd-split).

---

## Audio Analysis Features

All features are computed per hop (512 samples = 10.7ms @ 48kHz) in the analysis thread.

### Amplitude & Dynamics

| Feature | Algorithm | Output Range | FeatureSnapshot Field | Update Rate |
|---------|-----------|-------------|----------------------|-------------|
| RMS | Root mean square of 2048-sample window | [0, 1] | `rms` | Per hop |
| Peak | Max absolute sample value | [0, 1] | `peak` | Per hop |
| RMS dB | 20 * log10(rms) | [-100, 0] dBFS | `rmsDB` | Per hop |
| LUFS | K-weighted RMS, 400ms window (ITU-R BS.1770) | LUFS scale | `lufs` | Per hop |
| Dynamic Range | Crest factor (peak/RMS) | ratio | `dynamicRange` | Per hop |
| Transient Density | Onset count in 2-second sliding window | onsets/sec | `transientDensity` | Per hop |

### Spectral (from 2048-pt FFT → 1025 magnitude bins)

| Feature | Algorithm | Output Range | FeatureSnapshot Field |
|---------|-----------|-------------|----------------------|
| Spectral Centroid | Weighted average frequency | Hz | `spectralCentroid` |
| Spectral Flux | Half-wave rectified frame-to-frame magnitude diff | Normalized per-session | `spectralFlux` |
| Spectral Flatness | Geometric mean / arithmetic mean of magnitudes | [0, 1] | `spectralFlatness` |
| Spectral Rolloff | Frequency below 85% of total energy | Hz | `spectralRolloff` |
| 7-Band Energies | Sum magnitudes per band (Sub 20-60, Bass 60-250, LowMid 250-500, Mid 500-2k, HighMid 2k-4k, Presence 4k-6k, Brilliance 6k-20k Hz) | Normalized | `bandEnergies[0..6]` |

### Rhythm & Onset

| Feature | Source Library | Output | FeatureSnapshot Field |
|---------|---------------|--------|----------------------|
| Onset Detection | Aubio `aubio_onset` (spectral flux method, adaptive threshold) | bool flag + strength | `onsetDetected`, `onsetStrength` |
| BPM | Aubio `aubio_tempo` (autocorrelation of onset accumulator) | BPM float | `bpm` |
| Beat Phase | Derived from BPM tracker | [0, 1) sawtooth | `beatPhase` |
| Bar Phase | (beatInBar + beatPhase) / 4 | [0, 1) over 4 beats | `barPhase` |
| Phrase Phase | Bar count mod N bars (default 8), resets on structural transitions | [0, 1) over N bars | `phrasePhase` |
| Bar Count | Bars since last phrase reset | uint16 | `barCount` |

### Pitch & Harmony

| Feature | Algorithm | Output | FeatureSnapshot Field |
|---------|-----------|--------|----------------------|
| Chroma | FFT magnitude bins → 12 pitch classes (C–B), sum=1 | float[12] | `chromagram[0..11]` |
| Dominant Pitch | YIN or `aubio_pitch` on time-domain signal | Hz | `dominantPitch` |
| Pitch Confidence | YIN confidence measure | [0, 1] | `pitchConfidence` |
| Key Detection | Krumhansl-Schmuckler: chroma × 24 key templates | key + mode | `detectedKey`, `keyIsMajor` |
| MFCC | Mel filterbank (40 bands, 20-8kHz) → log → DCT-II → 13 coefficients | float[13] | `mfccs[0..12]` |
| HCDF | Euclidean distance between consecutive chroma frames | float | `harmonicChangeDetection` |

### Structural

| Feature | Algorithm | Output | FeatureSnapshot Field |
|---------|-----------|--------|----------------------|
| Structural State | Multi-scale EMA envelopes (100ms/1s/4s/16s), short vs long comparison → state machine | 0=normal, 1=buildup, 2=drop, 3=breakdown | `structuralState` |

### Analysis Pipeline Order (each step depends on prior results)

```
0.  Resample to 48 kHz (R13, AnalysisResampler): bypass when the device is
    already 48 kHz -- runs BEFORE stage 1, not one of the 14 numbered stages
1.  Raw time-domain: RMS, peak (over the 2048-sample block)
2.  FFT → magnitude spectrum (2048-pt, Hann window)
3.  From magnitude: centroid, flux, flatness, rolloff, 7-band energies
4.  Onset detection: aubio "specflux" onset
5.  BPM + downbeat + phrase: aubio tempo → stabilization → beat phase, beatInBar, barPhase, downbeat, bar count, phrase phase (downbeat and phrase tracking live INSIDE this stage; phrase resets on structural transitions)
6.  MFCC: mel filterbank → log → DCT → 13 coefficients
7.  Chroma + HCDF: magnitude bins → 12 pitch classes; HCDF = Euclidean distance to the previous chroma frame (computed INSIDE the Chroma stage)
8.  Key detection: chroma profile → Krumhansl-Kessler correlation (runs BEFORE pitch)
9.  Pitch: aubio "yinfft" on the time-domain hop
10. LUFS: K-weighted RMS over 400ms window
11-12. Transient density → Structural: onset count in a sliding window, then multi-scale EMA → buildup/drop/breakdown state machine
13. Genre detection: multi-feature scoring → 8 genres + energy state (P23)
14. Advanced analysis: sidechain pump, swing ratio, formant presence, resonance peak, reese bass (P25)
```

14 numbered compute stages in code (13 have profiled timing slots). Stage 5 folds BPM stabilization, downbeat, and phrase tracking together; HCDF is computed inside the Chroma stage (7); Key detection (8) runs before Pitch (9). R13's Resample step is a 14th profiled slot appended to the profile block (`[Analysis Profile]` names it "Resample") but is NOT one of the 14 numbered pipeline stages above — it runs once per hop before stage 1, reads 0µs on a 48 kHz device (bypass), and ~20-60µs/hop otherwise.

---
