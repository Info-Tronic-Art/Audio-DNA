# Slice 01 — Audio Pipeline + Analysis Engine (Feature Audit)

Scope: `src/audio/*`, `src/analysis/*`, `src/features/*`.
All line citations are absolute file paths.

---

## Section 1: Audio Analysis Features

The canonical data structure is `FeatureSnapshot` (POD, `alignas(64)`) at `/Users/boriskarpman/Documents/RealTimeAudio/src/analysis/FeatureSnapshot.h:5`. It is produced once per analysis hop (every 512 samples = ~10.7 ms @ 48 kHz) by `AnalysisThread::run()` and published through a lock-free triple-buffer `FeatureBus`.

All fields — one row each — in structural declaration order:

| # | Field | C++ Type | Default | Algorithm Source (file:line) | Output Range / Unit | Source Module |
|---|---|---|---|---|---|---|
| 1 | `timestamp` | `uint64_t` | `0` | `AnalysisThread.cpp:298` (`snap->timestamp = totalSamplesProcessed_`) | sample clock (samples since start) | `AnalysisThread.cpp:85` counter |
| 2 | `wallClockSeconds` | `double` | `0.0` | `AnalysisThread.cpp:299-300` (`totalSamplesProcessed_ / kSampleRate`) | seconds | derived timing |
| 3 | `rms` | `float` | `0.0f` | `AnalysisThread.cpp:111-112` — `sqrt(sumSq / kBlockSize)` over 2048-sample analysis window | [0, 1] (unipolar magnitude) | Stage 1 raw time-domain |
| 4 | `peak` | `float` | `0.0f` | `AnalysisThread.cpp:100-108,113` — `max(abs(sample))` over 2048 samples | [0, 1] | Stage 1 raw time-domain |
| 5 | `rmsDB` | `float` | `-100.0f` | `AnalysisThread.cpp:114` — `20*log10(rms)` else `-100.0` | [-100, 0] dBFS | Stage 1 |
| 6 | `lufs` | `float` | `-100.0f` | `LoudnessAnalyzer.cpp:82` — `-0.691 + 10*log10(meanSquare)`. K-weighting = two cascaded biquads (high-shelf `LoudnessAnalyzer.cpp:13-19` + high-pass `LoudnessAnalyzer.cpp:23-29`), 400 ms sliding window (`LoudnessAnalyzer.cpp:6`) | LUFS (dB) | Stage 10 — `LoudnessAnalyzer` |
| 7 | `dynamicRange` | `float` | `0.0f` | `LoudnessAnalyzer.cpp:97-100` — crest factor `rawPeak / rms` | ratio (unbounded, ~1+) | Stage 10 |
| 8 | `transientDensity` | `float` | `0.0f` | `AnalysisThread.cpp:229-238` — onset count in sliding window of `kOnsetWindowSize=256` hops (~2.73 s). `onsetCount / windowDurationSec` | onsets/sec | Stage 12 (computed before Structural) |
| 9 | `spectralCentroid` | `float` | `0.0f` | `SpectralFeatures.cpp:42-51` — `sum(f_k * M[k]) / sum(M[k])` over bins 1..1024 | Hz | Stage 3 — `SpectralFeatures` |
| 10 | `spectralFlux` | `float` | `0.0f` | `SpectralFeatures.cpp:54-77` — half-wave rectified L2 of frame-to-frame magnitude diff, with adaptive running-max normalization (decay factor `0.9995`, `.cpp:69`) | [0, 1] normalized per-session | Stage 3 |
| 11 | `spectralFlatness` | `float` | `0.0f` | `SpectralFeatures.cpp:86-110` — Wiener entropy: `exp(mean(ln P[k])) / mean(P[k])`, log-domain, clamped to [0,1] (`:105`) | [0, 1] | Stage 3 |
| 12 | `spectralRolloff` | `float` | `0.0f` | `SpectralFeatures.cpp:115-128` — frequency below which 85% of magnitude resides (threshold `0.85` at `:116`); defaults to Nyquist | Hz | Stage 3 |
| 13 | `bandEnergies[7]` | `float[7]` | all `0.0f` | `SpectralFeatures.cpp:132-150`. 7 bands with edges `{20, 60, 250, 500, 2000, 4000, 6000, 20000}` Hz (`SpectralFeatures.cpp:21-23`). Power = `sum(|M[k]|^2)`, sqrt, then adaptive normalization via running max (decay `0.9995`, `:145`) | Each [0, 1] normalized | Stage 3 |
|  | &nbsp;&nbsp;• `bandEnergies[0]` | — | — | Sub 20-60 Hz | — | — |
|  | &nbsp;&nbsp;• `bandEnergies[1]` | — | — | Bass 60-250 Hz | — | — |
|  | &nbsp;&nbsp;• `bandEnergies[2]` | — | — | LowMid 250-500 Hz | — | — |
|  | &nbsp;&nbsp;• `bandEnergies[3]` | — | — | Mid 500-2000 Hz | — | — |
|  | &nbsp;&nbsp;• `bandEnergies[4]` | — | — | HighMid 2000-4000 Hz | — | — |
|  | &nbsp;&nbsp;• `bandEnergies[5]` | — | — | Presence 4000-6000 Hz | — | — |
|  | &nbsp;&nbsp;• `bandEnergies[6]` | — | — | Brilliance 6000-20000 Hz | — | — |
| 14 | `onsetDetected` | `bool` | `false` | `OnsetDetector.cpp:37-39` — Aubio `new_aubio_onset("specflux", …)` (`:8-13`); `fvec_get_sample(output_, 0) != 0` | boolean flag per hop | Stage 4 — `OnsetDetector` |
| 15 | `onsetStrength` | `float` | `0.0f` | `OnsetDetector.cpp:40` — `aubio_onset_get_descriptor(onset_)` (raw detection function value) | unbounded positive | Stage 4 |
| 16 | `bpm` | `float` | `0.0f` | `BPMTracker.h:94` exposes `lockedBPM_`; stabilized via range gate / confidence / octave / median / hysteresis (pipeline constants `BPMTracker.h:40-57`). Manual override via `setManualBPM()` `BPMTracker.h:134` | BPM (floored to 0 when unknown) | Stage 5 — `BPMTracker` (Aubio `tempo`) |
| 17 | `beatPhase` | `float` | `0.0f` | `BPMTracker.h:95` — free-running sawtooth from locked BPM with hard reset on confident beat | [0, 1) sawtooth | Stage 5 |
| 18 | `trackerState` | `uint8_t` | `0` | `BPMTracker.h:98`; constants `STATE_SEARCHING=0`, `STATE_LOCKING=1`, `STATE_LOCKED=2` (`.h:35-37`) | 0 \| 1 \| 2 | Stage 5 |
| 19 | `beatInBar` | `uint8_t` | `0` | `BPMTracker.h:104`. Downbeat detection scoring beats with bass+flux+HCDF (`.h:62-64`: weights `0.5/0.3/0.2`), 16-beat circular buffer (`.h:59`), locks after 8+ consistent beats (`.h:61`) | 0-3 (0 = downbeat) | Stage 5 |
| 20 | `barPhase` | `float` | `0.0f` | `BPMTracker.h:105` — bar-level sawtooth over 4 beats | [0, 1) | Stage 5 |
| 21 | `downbeatDetected` | `bool` | `false` | `BPMTracker.h:106` — true on the hop where beat 1 lands | boolean | Stage 5 |
| 22 | `barCount` | `uint16_t` | `0` | `BPMTracker.h:110` — bars since last phrase reset | integer | Stage 5 |
| 23 | `phrasePhase` | `float` | `0.0f` | `BPMTracker.h:111` — sawtooth over `phraseBars_` (default 8, range 1-32: `.h:67-69`). Resets on structural transitions via `feedDownbeatFeatures(..., structuralState)` | [0, 1) | Stage 5 |
| 24 | `structuralState` | `uint8_t` | `0` | `StructuralDetector.cpp:53-78` classifier over 4 EMA scales (100 ms / 1 s / 4 s / 16 s `:6`), `holdThreshold_` ≈ 200 ms (`:22`), `onsetRateThreshold_=3.0` onsets/sec (`StructuralDetector.h:59`). Codes: `kNormal=0, kBuildup=1, kDrop=2, kBreakdown=3` (`StructuralDetector.h:39-42`) | {0,1,2,3} | Stage 11 |
| 25 | `chromagram[12]` | `float[12]` | all `0.0f` | `ChromaExtractor.cpp:48-73` — accumulate `\|M[k]\|^2` into pitch class via pre-computed bin→chroma map (`.cpp:16-46`), `kMinFreq=65 Hz` (C2, `.cpp:26`), normalized so sum=1 (`.cpp:64-74`). C=0..B=11 | Each [0, 1], sum=1 | Stage 7 |
| 26 | `dominantPitch` | `float` | `0.0f` | `PitchTracker.cpp:38` — Aubio `new_aubio_pitch("yinfft", …)` (`.cpp:8-13`); unit "Hz" (`.cpp:19`); tolerance `0.7` (`.cpp:20`); silence gate `-60 dB` (`.cpp:21`) | Hz (0 if unvoiced) | Stage 9 |
| 27 | `pitchConfidence` | `float` | `0.0f` | `PitchTracker.cpp:39` — `aubio_pitch_get_confidence(pitch_)` | [0, 1] | Stage 9 |
| 28 | `detectedKey` | `int` | `-1` | `KeyDetector.cpp:38-110` — Krumhansl-Schmuckler correlation with 24 templates (12 major + 12 minor), hysteresis `kHysteresisFrames=10` (`KeyDetector.h:58`), correlation floor `< 0.1` → unknown (`KeyDetector.cpp:76`). Profiles at `KeyDetector.h:36-43` | 0..11 (C=0), `-1` unknown | Stage 8 |
| 29 | `keyIsMajor` | `bool` | `true` | `KeyDetector.h:28`, `.cpp:82-85` — best-of-24 template pick | boolean | Stage 8 |
| 30 | `mfccs[13]` | `float[13]` | all `0.0f` | `MFCCExtractor.cpp:123-165`. 40-band mel filterbank (`MFCCExtractor.h:15`) over 20-8000 Hz (`MFCCExtractor.cpp:34-35`), log-floor `1e-10` (`.cpp:144`), DCT-II 13 coefs (`.cpp:105-121`) | DCT coefficients (unbounded) | Stage 6 |
| 31 | `harmonicChangeDetection` | `float` | `0.0f` | `ChromaExtractor.cpp:77-87` — Euclidean distance between current and previous 12-d chroma | ≥ 0 | Stage 7 (HCDF) |
| 32 | `detectedGenre` | `uint8_t` | `6` | `GenreDetector.h:62`, confirmed post-hysteresis. IDs `GenreDetector.h:26-33`: `kHouse=0, kTechno=1, kDnB=2, kHipHop=3, kAmbient=4, kRock=5, kPopElectronic=6, kJazzOther=7` | 0..7 | Stage 13 |
| 33 | `genreConfidence` | `float` | `0.0f` | `GenreDetector.h:63` — dominance of top genre after EMA smoothing (α ≈ 2 s window `.h:95`) | [0, 1] | Stage 13 |
| 34 | `energyState` | `uint8_t` | `1` | `GenreDetector.h:64,102` — low/medium/high from EMA of energy features | 0 \| 1 \| 2 | Stage 13 |
| 35 | `genreScores[8]` | `float[8]` | all `0.0f` | `GenreDetector.h:70` — smoothed raw per-genre scores (EMA) | each ≥ 0 | Stage 13 |
| 36 | `sidechainPump` | `float` | `0.0f` | `AdvancedAudioAnalyzer.h:27`. Bass/mid envelope anti-correlation over 64-hop window (`.h:43`, ~680 ms) | [0, 1] | Stage 14 (P25) |
| 37 | `swingRatio` | `float` | `0.5f` | `AdvancedAudioAnalyzer.h:28`. Inter-onset interval histogram over 32-onset window (`.h:53`) | [0.5, ~0.67], 0.5=straight | Stage 14 |
| 38 | `formantPresence` | `float` | `0.0f` | `AdvancedAudioAnalyzer.h:29`. Spectral energy concentration in 300-3000 Hz, adaptive normalization | [0, 1] | Stage 14 |
| 39 | `resonancePeak` | `float` | `0.0f` | `AdvancedAudioAnalyzer.h:30`. Spectral kurtosis in 200-8000 Hz | [0, 1] | Stage 14 |
| 40 | `reeseBass` | `float` | `0.0f` | `AdvancedAudioAnalyzer.h:31`. Spectral spread (std dev) in 30-200 Hz | [0, 1] | Stage 14 |

Total FeatureSnapshot fields documented: **40** (counting each `bandEnergies` entry separately; 33 top-level C++ declarations). The struct also defines `FeatureSnapshot::clear()` (`FeatureSnapshot.h:81-89`) which zero-memsets and restores non-zero defaults (`rmsDB=-100`, `lufs=-100`, `detectedKey=-1`, `keyIsMajor=true`, `swingRatio=0.5`).

### Per-analyzer tunable constants / defaults

**OnsetDetector** — `OnsetDetector.cpp:19-22`:
- Method: `"specflux"` (half-wave rectified spectral flux, `.cpp:9`)
- Threshold: `0.3` (moderate sensitivity; runtime setter `OnsetDetector.h:33`)
- Silence gate: `-70 dB` (runtime setter `OnsetDetector.h:34`)
- Min inter-onset interval: `50 ms` (runtime setter `OnsetDetector.h:35`)
- Adaptive whitening: ON (`aubio_onset_set_awhitening(onset_, 1)`)
- Internal `bufSize`: 1024 (passed from `AnalysisThread.cpp:26`)

**PitchTracker** — `PitchTracker.cpp:19-21`:
- Method: `"yinfft"` (`.cpp:9`)
- Unit: `Hz`
- Tolerance: `0.7`
- Silence gate: `-60 dB`
- `bufSize`: `FFTProcessor::kFFTSize` = 2048 (`AnalysisThread.cpp:36`)

**BPMTracker** (`BPMTracker.h:40-69`):
- `kMinBPM = 60`, `kMaxBPM = 200`
- `kMedianWindowSize = 48` (~500 ms)
- `kHysteresisHops = 200` (~2.1 s)
- `kConfidenceThreshold = 0.1`
- `kBPMChangeThreshold = 2.0` BPM
- `kBeatResetConfidence = 0.5`
- `kBeatScoreBufferSize = 16`, `kBeatsPerBar = 4`, `kDownbeatLockThreshold = 8`
- Downbeat weights: bass `0.5`, flux `0.3`, HCDF `0.2`
- Phrase bars default `8`, range [1, 32]
- Silence detection: `silenceRmsThreshold_ = 0.005` (`.h:221`); entry ~300 ms, exit ~100 ms

**StructuralDetector** (`StructuralDetector.h:32-59`, `.cpp:6`):
- 4 time scales: 0.1 s, 1.0 s, 4.0 s, 16.0 s
- Hold threshold: ~200 ms (`.cpp:22`)
- Onset rate threshold: `3.0` onsets/sec
- Drop trigger: `fast > slow * 1.8 AND onsetRate > 3` (`.cpp:66`)
- Buildup trigger: `fast > slow * 1.3 AND fluxFast > fluxSlow * 1.2` (`.cpp:70`)
- Breakdown trigger: `fast < slow * 0.5` (`.cpp:74`)

**LoudnessAnalyzer** (`LoudnessAnalyzer.cpp:6-29`):
- Window: 400 ms
- K-weighting Stage 1 biquad (high-shelf): `b0=1.535, b1=-2.692, b2=1.198, a1=-1.691, a2=0.732`
- K-weighting Stage 2 biquad (high-pass): `b0=1.0, b1=-2.0, b2=1.0, a1=-1.990, a2=0.990`
- LUFS offset: `-0.691` (ITU-R BS.1770)

**KeyDetector** (`KeyDetector.h:36-43, 58`):
- Major profile (Krumhansl-Kessler): `[6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88]`
- Minor profile: `[6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17]`
- Hysteresis: 10 frames
- Correlation floor: `0.1`

**ChromaExtractor** (`ChromaExtractor.cpp:25-26`):
- C0 reference: `16.3515978312874 Hz`
- Min frequency cutoff: `65.0 Hz` (C2) — bins below this ignored

**MFCCExtractor** (`MFCCExtractor.h:15-16`, `.cpp:34-35, 144`):
- 40 mel bands, 13 MFCCs
- Frequency range: 20-8000 Hz
- Log floor: `1e-10`

**SpectralFeatures** (`SpectralFeatures.cpp:21-23, 69, 145, 116`):
- Band edges: `{20, 60, 250, 500, 2000, 4000, 6000, 20000}` Hz (7 bands)
- Flux max decay: `0.9995` (~5 s half-life)
- Band max decay: `0.9995`
- Rolloff threshold: `0.85`

**GenreDetector** (`GenreDetector.h:23, 98, 107`):
- 8 genres (enumerated in Field 32 above)
- Default confirmed genre on startup: `kPopElectronic` (6)
- Hysteresis hold: ~2 s of consistency (`holdThreshold_`, `.h:107`)
- Smoothing α: ~2 s window (`.h:95`)
- Features struct `GenreDetector::Features` (`.h:36-52`): bpm, rms, spectralCentroid, spectralFlux, spectralFlatness, spectralRolloff, transientDensity, bandEnergies[7], chromagram[12], mfccs[13], dynamicRange, structuralState, trackerState, harmonicChangeDetection

**AdvancedAudioAnalyzer** (`AdvancedAudioAnalyzer.h:43, 53`):
- Envelope length: 64 hops (~680 ms)
- IOI history: last 32 onset intervals

---

## Section 9 (partial): Audio Input

### Source mode selection

`AudioEngine::SourceMode` enum — `AudioEngine.h:36`:
- `File` — Playback of an audio file (loaded via `AudioEngine::loadFile`, `AudioEngine.h:17`)
- `MicInput` — Live microphone / audio input (enables input channels)

API surface:
- `AudioEngine::setSourceMode(SourceMode mode)` — `AudioEngine.h:37`, implementation `AudioEngine.cpp:89-120`
- `AudioEngine::getSourceMode()` — `AudioEngine.h:38`
- Default mode on construction: `SourceMode::File` (`AudioEngine.h:148`)

No explicit "system audio" loopback mode is implemented as a separate enum value. Loopback would have to be exposed through the OS as an input device selectable in `AudioDeviceManager`.

### Audio file format support

- `juce::AudioFormatManager::registerBasicFormats()` called at `AudioEngine.cpp:7`. Per JUCE, registered formats are: WAV, AIFF, FLAC, Ogg Vorbis, and MP3 (on platforms where MP3 decode is available).
- `loadFile(const juce::File&)` — `AudioEngine.h:17`, `AudioEngine.cpp:32-52`. On failure, invokes `onError` callback with `"Failed to load: <filename>"`.
- Read-ahead buffer: `32768` samples (`AudioEngine.cpp:49`).
- Background read thread: `juce::TimeSliceThread{"AudioReadAhead"}` (`AudioEngine.h:50`), started at `juce::Thread::Priority::normal` (`AudioEngine.cpp:10`).

### Transport controls

Exposed by `AudioEngine`:
- `play()` — `AudioEngine.h:18`, `.cpp:54-57`
- `pause()` — `AudioEngine.h:19`, `.cpp:59-62`
- `stop()` — `AudioEngine.h:20`, `.cpp:64-68` (stops and resets position to 0)
- `isPlaying()` — `AudioEngine.h:21`, `.cpp:70-73`
- Change-listener callback forwards to `onTransportStateChanged(bool)` (`AudioEngine.h:29`, `.cpp:122-126`)

### Device / driver configuration

- `juce::AudioDeviceManager` owned at `AudioEngine.h:44`; exposed via `getDeviceManager()` (`.h:23`). This is the standard JUCE device picker; supports CoreAudio / WASAPI / ALSA / JACK input and output device selection, sample rate, buffer size, and channel enabling through JUCE's built-in `AudioDeviceSelectorComponent` (not instantiated in these files).
- Initialized with 2 input + 2 output channels: `deviceManager_.initialiseWithDefaultDevices(2, 2)` — `AudioEngine.cpp:14`.
- Sample rate: reported only via `device->getCurrentSampleRate()`. No explicit sample-rate setter here — it's driven by the user's selected audio device.
- `hasAudioDevice()` — `AudioEngine.h:32`, `.cpp:76-79`.
- `getDeviceStatus()` — `AudioEngine.h:33`, `.cpp:81-87` — returns `"<DeviceName> @ <rate>Hz"` or `"No audio device"`.
- On switching to `MicInput`: enables 2 stereo input channels via `setup.inputChannels.setRange(0, 2, true)` (`AudioEngine.cpp:103`).
- On switching to `File`: clears input channels to reduce latency (`AudioEngine.cpp:115`).

### Gain / level controls

- `setInputGain(float gain)` — `AudioEngine.h:40` (atomic store on `combinedCallback_.inputGain`)
- `getInputLevel()` — `AudioEngine.h:41` (atomic read of peak input meter)
- `inputGain` default: `1.0f` (`AudioEngine.h:62`)
- `inputLevel`: smoothed peak meter (`AudioEngine.h:63`); attack = instantaneous peak, release = `prev * 0.92f` per block (`AudioEngine.h:87`)
- Gain applied only in mic mode, multiplied per-sample (`AudioEngine.h:94-96`)

### Output behavior

- Output is **always silenced** — app is analysis-only: `juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples)` at `AudioEngine.h:127`, after both file playback (already read into output for analysis tap, then cleared) and mic paths.

### Audio callback signal flow

- `AudioEngine::CombinedCallback::audioDeviceIOCallbackWithContext` (`AudioEngine.h:65-128`) is registered with the device manager (`AudioEngine.cpp:21`).
- In File mode: `juce::AudioSourcePlayer` (with `AudioTransportSource` source) writes audio into `outputChannelData` → tapped to `AudioCallback` → then output zeroed (`AudioEngine.h:113-123, 127`).
- In Mic mode: `inputChannelData` peak-metered with EMA release (`AudioEngine.h:76-88`), optionally gain-scaled into `outputChannelData` (`AudioEngine.h:94-96`), then fed to `AudioCallback` (`AudioEngine.h:98-107`).
- `AudioCallback::audioDeviceIOCallbackWithContext` (`AudioCallback.cpp:8-45`) mono-downmixes output channels into `monoBuffer_` and pushes into the ring buffer. For N > 1 channels: `sum * (1/N)` gain (`.cpp:34-41`).
- `AudioCallback::audioDeviceAboutToStart` (`AudioCallback.cpp:47-50`) resizes `monoBuffer_` to current device buffer size — the only non-RT allocation, done at start.

### User-relevant UI-level configuration flags (from these files)

- Source mode toggle (File vs MicInput) — stored in `sourceMode_` (`AudioEngine.h:148`) + atomic `combinedCallback_.useInputForAnalysis` (`AudioEngine.h:61`).
- Input gain slider — backed by `combinedCallback_.inputGain` (`AudioEngine.h:62`).
- Input level meter — backed by `combinedCallback_.inputLevel` (`AudioEngine.h:63`).
- Transport play/pause/stop callbacks.
- `onError` callback for load-failure surfaces (`AudioEngine.h:30`).

---

## Section 20 (partial): Audio + Analysis Cross-References

**Audio path (RT callback → ring buffer)**
- Ring buffer type: `RingBuffer<float>` SPSC — `RingBuffer.h:10-83`
- Producer: `AudioCallback::audioDeviceIOCallbackWithContext` `AudioCallback.cpp:8-45`
- Consumer: `AnalysisThread::run` `AnalysisThread.cpp:53-355`
- Mono downmix: `AudioCallback.cpp:27-42`
- Push operation: `RingBuffer::push` `RingBuffer.h:29-41`
- Pop operation: `RingBuffer::pop` `RingBuffer.h:44-56`
- Available count: `RingBuffer::availableToRead` `RingBuffer.h:58-63`

**Analysis pipeline (stage by stage)**
- Stage 1 (RMS/peak/rmsDB): `AnalysisThread.cpp:99-121`
- Stage 2 (FFT): `AnalysisThread.cpp:123-129`, FFT impl `FFTProcessor.cpp:28-64`
- Stage 3 (SpectralFeatures): `AnalysisThread.cpp:131-142`, impl `SpectralFeatures.cpp:39-151`
- Stage 4 (OnsetDetector): `AnalysisThread.cpp:144-151`, impl `OnsetDetector.cpp:32-41`
- Stage 5 (BPMTracker incl. downbeat/phrase): `AnalysisThread.cpp:153-177`, header `BPMTracker.h`
- Stage 6 (MFCC): `AnalysisThread.cpp:179-185`, impl `MFCCExtractor.cpp:123-166`
- Stage 7 (Chroma + HCDF): `AnalysisThread.cpp:187-196`, impl `ChromaExtractor.cpp:48-96`
- Stage 8 (KeyDetector): `AnalysisThread.cpp:198-205`, impl `KeyDetector.cpp:38-110`
- Stage 9 (PitchTracker): `AnalysisThread.cpp:207-214`, impl `PitchTracker.cpp:31-40`
- Stage 10 (LoudnessAnalyzer LUFS + DR): `AnalysisThread.cpp:216-223`, impl `LoudnessAnalyzer.cpp:41-102`
- Stage 11 (StructuralDetector) and Stage 12 (Transient density computed first): `AnalysisThread.cpp:225-248`, impl `StructuralDetector.cpp:27-78`
- Stage 13 (GenreDetector): `AnalysisThread.cpp:250-278`
- Stage 14 (AdvancedAudioAnalyzer P25): `AnalysisThread.cpp:280-295`

**Feature publishing / consumption**
- FeatureBus triple-buffer definition: `FeatureBus.h:21-72`, impl `FeatureBus.cpp:1-82`
- Publisher API: `acquireWrite()` / `publishWrite()` — `FeatureBus.cpp:12-38`
- Consumer API: `acquireRead()` / `getLatestRead()` / `hasNewData()` — `FeatureBus.cpp:40-81`
- State encoding: packed uint8 with `kWriteMask`, `kLatestShift/Mask`, `kReadShift/Mask`, `kNewFlag` — `FeatureBus.h:54-59`

**Render/UI helpers in AnalysisThread**
- Backward-compatible quick accessors: `getRMS()` / `getPeak()` — `AnalysisThread.h:54-55`
- Waveform display copy: `getWaveformSamples()` — `AnalysisThread.h:62`, impl `AnalysisThread.cpp:357-362`
- External PCM snapshot (for projectM etc.): `getPCMSamples()` — `AnalysisThread.h:73`, impl `AnalysisThread.cpp:364-372`
- BPM tracker access (for UI resync/phrase config): `getBpmTracker()` — `AnalysisThread.h:65`
- CPU load: `getCpuLoad()` — `AnalysisThread.h:140`

**Smoothers (display / mapping thread consumption)**
- `Smoother` (EMA): `Smoother.h:7-45`. Default α = 0.3 (`.h:12`).
- `OneEuroFilter`: `Smoother.h:49-106`. Default: rate `93.75 Hz`, `minCutoff 1.0 Hz`, `beta 0.007`, `dCutoff 1.0 Hz` (`.h:55-57`).

---

## Internals (extra)

### Threading model constants

| Constant | Value | File:Line |
|---|---|---|
| `AnalysisThread::kBlockSize` (FFT analysis window) | `2048` samples | `AnalysisThread.h:44` |
| `AnalysisThread::kHopSize` | `512` samples (≈10.67 ms @ 48 kHz) | `AnalysisThread.h:45` |
| `AnalysisThread::kSampleRate` (expected) | `48000` | `AnalysisThread.h:46` |
| `AnalysisThread::kWaveformBufferSize` | `2048` | `AnalysisThread.h:67` |
| `AnalysisThread::kPCMSnapshotSize` | `512` | `AnalysisThread.h:72` |
| `AnalysisThread::kOnsetWindowSize` (transient density window) | `256` hops (~2.73 s) | `AnalysisThread.h:113` |
| `AnalysisThread::kNumStages` (profiler buckets) | `14` | `AnalysisThread.h:134` |
| `AnalysisThread::kProfileInterval` (profile log cadence) | `500` hops (~5.3 s) | `AnalysisThread.h:137` |
| `FFTProcessor::kFFTOrder` | `11` | `FFTProcessor.h:12` |
| `FFTProcessor::kFFTSize` | `1 << 11 = 2048` | `FFTProcessor.h:13` |
| `FFTProcessor::kNumBins` | `1025` (N/2 + 1) | `FFTProcessor.h:14` |
| Ring buffer: instantiated elsewhere; power-of-two enforced | `nextPowerOfTwo(requested)` | `RingBuffer.h:17, 68-74` |
| Ring buffer padding | cache-line aligned atomics (`alignas(64)`) | `RingBuffer.h:81-82` |
| `read thread` priority | `juce::Thread::Priority::normal` | `AudioEngine.cpp:10` |

### Analysis pipeline execution order (as actually run in `AnalysisThread::run`)

1. Raw time-domain: RMS, peak, rmsDB — `AnalysisThread.cpp:99-121`
2. FFT (2048-pt Hann, magnitude spectrum) — `.cpp:123-129`
3. Spectral features: centroid, flux, flatness, rolloff, 7-band energies — `.cpp:131-142`
4. Onset detection — `.cpp:144-151`
5. BPM tracking + downbeat + bar + phrase — `.cpp:153-177` (uses `prevHCDF_` from last hop for downbeat score, `prevStructuralState_` for phrase reset; `feedSilenceDetection(rms)` called first at `.cpp:155`)
6. MFCC — `.cpp:179-185`
7. Chroma + HCDF (cached into `prevHCDF_`) — `.cpp:187-196`
8. Key detection — `.cpp:198-205`
9. Pitch detection — `.cpp:207-214`
10. Loudness (LUFS + dynamic range) — `.cpp:216-223`
11. Structural detection (depends on transient density computed first) — `.cpp:225-248`
12. Transient density (sliding-window onset count) — embedded inside Stage 11 block at `.cpp:229-238`
13. Genre detection — `.cpp:250-278`
14. Advanced audio analysis (P25: sidechain/swing/formant/resonance/reese) — `.cpp:280-295`

Note: the source code pipeline **docstring** in `AnalysisThread.h:28-42` lists 13 stages; the code has **14** (Advanced Audio was added in P25). The profiler uses 13 labeled stages (`AnalysisThread.cpp:317-321`: "RMS/Peak", "FFT", "Spectral", "Onset", "BPM", "MFCC", "Chroma", "Key", "Pitch", "Loudness", "Structural", "Genre", "Advanced") — 13 names for 14 actual stages because "Structural" and "Transient density" are timed together.

### Threading / RT safety notes extracted from code

- Audio callback path never allocates — `AudioCallback::audioDeviceAboutToStart` (`.cpp:47-50`) is the only resize site, called before streaming begins.
- All analysis modules pre-allocate their buffers in constructors and use `alignas(64)` storage (`FFTProcessor.h:39-42`, `MFCCExtractor.h:56-66`, `ChromaExtractor.h:50`).
- Ring buffer uses `acquire`/`release` atomic ordering; writer and reader positions are on separate cache lines (`alignas(64)`) to avoid false sharing (`RingBuffer.h:81-82`).
- FeatureBus uses a single `uint8_t` atomic state with CAS for swaps; all operations are wait-free after at most one CAS retry (`FeatureBus.cpp:19-38, 40-69`).

### Dev-only / debug surfaces

- Per-stage profile logger: prints average microseconds per stage and % of hop period to `std::cerr` every 500 hops — `AnalysisThread.cpp:315-335`. Always-on (no compile guard).
- CPU load EMA (10% smoothing) stored in `cpuLoad_` atomic — `AnalysisThread.cpp:306-312`, exposed via `getCpuLoad()` (`AnalysisThread.h:140`).
- Error log on device init failure: `std::cerr << "[AudioEngine] Device init error: …"` — `AudioEngine.cpp:17`.
- Mode-switch log: `std::cerr << "[AudioEngine] Switched to …"` — `AudioEngine.cpp:106, 118`.
- `BPMTracker::processRawBPM(rawBpm, conf, beat)` — test-only direct pipeline injection (`BPMTracker.h:147`).
- `BPMTracker::rawBPM()` — diagnostics accessor for unstabilized aubio tempo (`BPMTracker.h:101`).

### Classes, structs, enums enumerated

- `AudioEngine` (class) — `AudioEngine.h:10`
- `AudioEngine::SourceMode` (enum class: `File`, `MicInput`) — `AudioEngine.h:36`
- `AudioEngine::CombinedCallback` (nested class, derives `juce::AudioIODeviceCallback`) — `AudioEngine.h:54`
- `AudioCallback` (class, derives `juce::AudioIODeviceCallback`) — `AudioCallback.h:7`
- `RingBuffer<T>` (class template) — `RingBuffer.h:10`
- `AnalysisThread` (class, derives `juce::Thread`) — `AnalysisThread.h:41`
- `FeatureSnapshot` (struct, `alignas(64)`) — `FeatureSnapshot.h:5`
- `FFTProcessor` (class) — `FFTProcessor.h:9`
- `SpectralFeatures` (class) — `SpectralFeatures.h:14`
- `SpectralFeatures::BandRange` (nested struct: `int low, high`) — `SpectralFeatures.h:55`
- `OnsetDetector` (class; wraps `aubio_onset_t`) — `OnsetDetector.h:11`
- `BPMTracker` (class; wraps `aubio_tempo_t`) — `BPMTracker.h:32`
- `MFCCExtractor` (class) — `MFCCExtractor.h:12`
- `MFCCExtractor::MelFilter` (nested struct: `int startBin, endBin`) — `MFCCExtractor.h:46`
- `ChromaExtractor` (class) — `ChromaExtractor.h:10`
- `KeyDetector` (class) — `KeyDetector.h:17`
- `LoudnessAnalyzer` (class) — `LoudnessAnalyzer.h:14`
- `LoudnessAnalyzer::BiquadState` (nested struct: `float b0,b1,b2,a1,a2,s1,s2`) — `LoudnessAnalyzer.h:31`
- `StructuralDetector` (class) — `StructuralDetector.h:16`
- `PitchTracker` (class; wraps `aubio_pitch_t`) — `PitchTracker.h:10`
- `GenreDetector` (class) — `GenreDetector.h:20`
- `GenreDetector::Features` (nested struct) — `GenreDetector.h:36`
- `AdvancedAudioAnalyzer` (class) — `AdvancedAudioAnalyzer.h:14`
- `FeatureBus` (class) — `FeatureBus.h:21`
- `Smoother` (class) — `Smoother.h:7`
- `OneEuroFilter` (class) — `Smoother.h:49`

### Public methods (non-exhaustive, trimmed to user/UI surface)

AudioEngine:
- `AudioEngine(RingBuffer<float>& ringBuffer)` — `AudioEngine.h:13`
- `bool loadFile(const juce::File& file)` — `AudioEngine.h:17`
- `void play()` / `pause()` / `stop()` — `AudioEngine.h:18-20`
- `bool isPlaying() const` — `AudioEngine.h:21`
- `juce::AudioDeviceManager& getDeviceManager()` — `AudioEngine.h:23`
- `juce::AudioTransportSource& getTransportSource()` — `AudioEngine.h:24`
- `void setSourceMode(SourceMode mode)` — `AudioEngine.h:37`
- `SourceMode getSourceMode() const` — `AudioEngine.h:38`
- `void setInputGain(float gain)` — `AudioEngine.h:40`
- `float getInputLevel() const` — `AudioEngine.h:41`
- Callbacks (public fields): `onTransportStateChanged`, `onError` — `AudioEngine.h:29-30`
- `bool hasAudioDevice() const` — `AudioEngine.h:32`
- `juce::String getDeviceStatus() const` — `AudioEngine.h:33`

AnalysisThread:
- `explicit AnalysisThread(RingBuffer<float>&)` — `AnalysisThread.h:48`
- `void run() override` — `AnalysisThread.h:51`
- `float getRMS() / getPeak() const` — `.h:54-55`
- `FeatureBus& getFeatureBus()` — `.h:58`
- `void getWaveformSamples(float*, int&) const` — `.h:62`
- `BPMTracker* getBpmTracker()` — `.h:65`
- `int getPCMSamples(float*, int) const` — `.h:73`
- `float getCpuLoad() const` — `.h:140`

OnsetDetector runtime setters:
- `setThreshold(float)`, `setSilence(float)`, `setMinInterOnsetMs(float)` — `OnsetDetector.h:33-35`

BPMTracker runtime setters (user-facing):
- `setThreshold(float)`, `setSilence(float)` — `BPMTracker.h:115-116`
- `setPhraseBars(int)` — `.h:117`
- `setManualBPM(float)` — `.h:134`
- `setManualMode(bool)` / `isManualMode()` — `.h:138-139`
- `resetPhrase()` — `.h:130`
- `resetBeatPhase()` — `.h:142`
- `feedDownbeatFeatures(bass, flux, hcdf, structuralState)` — `.h:90-91`
- `feedSilenceDetection(rms)` — `.h:123`
- `isSilent()`, `silenceDuration()` — `.h:126-127`

FeatureBus:
- `acquireWrite() / publishWrite()` — writer (`FeatureBus.h:29, 32`)
- `acquireRead() / getLatestRead() / hasNewData()` — reader (`FeatureBus.h:39, 43, 46`)

Smoother:
- `process(float)`, `value()`, `reset()`, `setAlpha(float)`, `alpha()` — `Smoother.h:16-39`

OneEuroFilter:
- `process(float)`, `value()`, `reset()` — `Smoother.h:59-88`

### Audio-thread-to-analysis latency budget (from code constants)

- Hop period: `kHopSize / kSampleRate` = 512 / 48000 = 10.667 ms (`AnalysisThread.h:45-46`)
- Block / FFT size: 2048 samples = 42.67 ms window at 48 kHz (`AnalysisThread.h:44`)
- Transient density window: 256 hops × 10.667 ms = ~2.73 s (`AnalysisThread.h:113`)
- LUFS window: 400 ms (`LoudnessAnalyzer.cpp:6`)
- Structural EMA scales: 100 ms / 1 s / 4 s / 16 s (`StructuralDetector.cpp:6`)
- BPM median window: ~500 ms (`BPMTracker.h:44`)
- BPM hysteresis: ~2.1 s (`BPMTracker.h:47`)
- Key detector hysteresis: 10 hops ≈ 107 ms (`KeyDetector.h:58`)
- Genre detector hysteresis: ~2 s (`GenreDetector.h:107`)

### Notes / discrepancies vs project docs

1. CLAUDE.md lists 15 pipeline stages ("15. Genre detection", "16. Advanced analysis"). The code has **14** numbered stages (steps 11 and 12 are interleaved inside the same time bucket). The `kNumStages=14` profiler counter (`AnalysisThread.h:134`) tracks timing for 14 stages but labels only 13 in the log (`.cpp:317-321`).
2. CLAUDE.md's FeatureSnapshot reference table omits `trackerState`, `beatInBar`, `downbeatDetected` — these are present in `FeatureSnapshot.h:41, 44, 46` and fed by BPMTracker.
3. CLAUDE.md does not document the `pcmSnapshot_` / `getPCMSamples()` external-consumer path (for projectM integration) — present at `AnalysisThread.h:70-73, 91-93` and `.cpp:345-353, 364-372`.
4. `MFCCExtractor`'s flat-weight storage of `4096 * 40 = 163,840` floats (`MFCCExtractor.h:56`) is quite generous; most slots are zero due to small per-band bin widths — the "4096 bins" note is for an 8192-pt FFT headroom.
5. The `FeatureSnapshot::clear()` method zero-memsets the whole struct but re-initializes sentinel defaults (`-100.0` for dB fields, `-1` for `detectedKey`, `true` for `keyIsMajor`, `0.5` for `swingRatio`). It does **not** reset `detectedGenre` to `6` (the default on struct construction), so after `clear()`, `detectedGenre = 0` (House) — this may be a latent bug.
