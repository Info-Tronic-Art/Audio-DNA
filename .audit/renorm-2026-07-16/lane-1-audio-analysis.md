# Lane 1 Census — Audio + Analysis + Features

**Scope:** `src/audio/`, `src/analysis/`, `src/features/`
**Method:** read-only source audit; every claim carries `file:line` evidence. App not built/run.
**Date:** 2026-07-16

---

## 0. Verdict

The lane is **solid and almost entirely live** — 20 of 22 classes are wired into the
running pipeline with real algorithms and zero-allocation RT discipline. Two classes
(`GenreSmoothing`, `OneEuroFilter`) are fully dead. The dominant documentation problem
is the **analysis pipeline order/count**: `CLAUDE.md` and `.harmony/FEATURES.md` describe
a **16-stage** pipeline with steps in the wrong order (ZCR, Pitch↔Key swap, HCDF and
Phrase tracking as separate late stages). The source runs **14 numbered compute stages
(13 profiled)** and computes HCDF inside Chroma (stage 7) and Phrase tracking inside BPM
(stage 5). `docs/FEATURE_INVENTORY.md` is the most accurate of the three docs.

---

## 1. Class / Component Census

| Class | File | Role | Status |
|-------|------|------|--------|
| `AudioEngine` | audio/AudioEngine.h:10 | Owns `AudioDeviceManager`, transport, format reader; File vs MicInput mode; silences output | LIVE |
| `AudioEngine::CombinedCallback` | audio/AudioEngine.h:54 | Inner `AudioIODeviceCallback`: routes file-output or mic-input to analysis, then silences output | LIVE |
| `AudioCallback` | audio/AudioCallback.h:7 | RT callback; mono-downmix of output channels → `RingBuffer::push` | LIVE |
| `RingBuffer<T>` | audio/RingBuffer.h:11 | SPSC lock-free ring, power-of-two, cache-line padded | LIVE |
| `AnalysisThread` | analysis/AnalysisThread.h:41 | Dedicated thread; pulls hops, runs pipeline, publishes `FeatureSnapshot` | LIVE |
| `FeatureSnapshot` | analysis/FeatureSnapshot.h:5 | `alignas(64)` POD, 40 member declarations | LIVE |
| `FFTProcessor` | analysis/FFTProcessor.h:9 | 2048-pt JUCE FFT + Hann → 1025 magnitude bins | LIVE |
| `SpectralFeatures` | analysis/SpectralFeatures.h:14 | centroid, flux, flatness, rolloff, 7-band energies | LIVE |
| `OnsetDetector` | analysis/OnsetDetector.h:11 | aubio "specflux" onset | LIVE |
| `BPMTracker` | analysis/BPMTracker.h:32 | aubio_tempo + 5-stage stabilization + downbeat + phrase | LIVE |
| `MFCCExtractor` | analysis/MFCCExtractor.h:12 | 40-band mel → log → DCT-II → 13 MFCC | LIVE |
| `ChromaExtractor` | analysis/ChromaExtractor.h:10 | 12 pitch classes + HCDF | LIVE |
| `KeyDetector` | analysis/KeyDetector.h:17 | Krumhansl-Kessler ×24 templates + hysteresis | LIVE |
| `LoudnessAnalyzer` | analysis/LoudnessAnalyzer.h:14 | K-weighting biquads + 400ms window → LUFS, crest | LIVE |
| `StructuralDetector` | analysis/StructuralDetector.h:16 | 4-scale EMA envelopes → state machine | LIVE |
| `PitchTracker` | analysis/PitchTracker.h:10 | aubio "yinfft" pitch | LIVE |
| `GenreDetector` | analysis/GenreDetector.h:20 | 8-genre scoring + energy state | LIVE |
| `AdvancedAudioAnalyzer` | analysis/AdvancedAudioAnalyzer.h:14 | sidechain/swing/formant/resonance/reese (P25) | LIVE (used AnalysisThread.cpp:280-291) |
| `FeatureBus` | features/FeatureBus.h:21 | Lock-free triple-buffer, packed-state CAS | LIVE |
| `Smoother` (EMA) | features/Smoother.h:7 | EMA smoother | LIVE (MappingEngine.h:54, RoutingEngine.h:52) |
| `OneEuroFilter` | features/Smoother.h:49 | Adaptive One-Euro filter | **DEAD** (only comment ref GenreSmoothing.h:16) |
| `GenreSmoothing` | analysis/GenreSmoothing.h:19 | Per-genre attack/release presets | **DEAD** (only self-refs) |

---

## 2. Audio Input Paths

| Path | Mechanism | Evidence |
|------|-----------|----------|
| **File playback** | `AudioEngine::loadFile` → `AudioFormatReaderSource` → `AudioTransportSource`; `CombinedCallback` runs `AudioSourcePlayer` into output buffers, feeds analysis, then silences output | AudioEngine.cpp:32-52; AudioEngine.h:110-127 |
| **Mic input** | `setSourceMode(MicInput)` sets `useInputForAnalysis`, re-opens device with stereo input; `CombinedCallback` passes input channels (optionally gain-scaled) to the analysis callback | AudioEngine.cpp:89-120; AudioEngine.h:72-108 |
| **System-audio loopback** | NOT a distinct mode — `SourceMode = {File, MicInput}` only; loopback would require an OS loopback device selected as "mic" | AudioEngine.h:36 |
| **Output** | Always silenced in every mode (analysis-only app) | AudioEngine.h:126-127 |
| Input gain / meter | `setInputGain` / `getInputLevel` atomics; peak metering with 0.92 release per block | AudioEngine.h:40-41,62-88 |
| Formats | `registerBasicFormats()`: WAV/AIFF/FLAC/Ogg/MP3 | AudioEngine.cpp:7 |
| Downmix | `AudioCallback` reads **output** channels, averages to mono (gain = 1/numOutputChannels) | AudioCallback.cpp:16-44 |
| Ring capacity | `RingBuffer<float> ringBuffer_{16384}` (~341ms @ 48kHz) | MainComponent.h:113 |

**Note:** `AudioCallback` ignores its `inputChannelData` parameter and always reads
`outputChannelData` (AudioCallback.cpp:16-18). In mic mode the enclosing
`CombinedCallback` hands the mic samples in through the *output* pointer argument, so the
mono-downmix path is shared.

---

## 3. Lock-Free Transfer / Plumbing

| Mechanism | Type | Key detail | Evidence |
|-----------|------|-----------|----------|
| Audio → Analysis | `RingBuffer<float>` SPSC | acquire/release atomics, power-of-two mask, `alignas(64)` write/read indices; overflow drops silently (push returns short) | RingBuffer.h:29-63,81-82 |
| Analysis → Render | `FeatureBus` triple-buffer | Single `atomic<uint8_t>` packs write/latest/read slots (2 bits each) + new-data flag; wait-free CAS on publish/acquire | FeatureBus.h:48-71; FeatureBus.cpp:19-69 |
| Waveform snapshot | `waveformBuffer_` + atomic count | 2048 floats; count published with release, buffer via non-atomic memcpy (torn-read possible) | AnalysisThread.cpp:337-343,357-362 |
| PCM snapshot (projectM) | double-buffer + atomic index | 512-sample, index-swap; consumed by Renderer.cpp:771 | AnalysisThread.h:70-93; AnalysisThread.cpp:345-372 |
| Display smoothing | `Smoother` (EMA) | per-mapping `std::vector<Smoother>` in MappingEngine/RoutingEngine | Smoother.h:7-45; MappingEngine.cpp:9,195-197 |

---

## 4. Analysis Pipeline — SOURCE TRUTH

Loop: pull 512-sample hop → maintain 2048-sample overlap window → run stages once a full
block is buffered → `publishWrite()`. (AnalysisThread.cpp:53-303)

**14 numbered compute steps in code (comments 1-14); 13 profiled timing slots (`stageTimesUs_[0..12]`); header docstring lists 13.**

| # (code) | Stage | Class / action | Writes FeatureSnapshot | Evidence |
|---------|-------|----------------|------------------------|----------|
| 1 | RMS/Peak | inline time-domain over 2048 block | rms, peak, rmsDB | .cpp:99-118 |
| 2 | FFT | `FFTProcessor::process` | (magnitude spectrum) | .cpp:123-125 |
| 3 | Spectral | `SpectralFeatures::process` | spectralCentroid/Flux/Flatness/Rolloff, bandEnergies[7] | .cpp:131-138 |
| 4 | Onset | `OnsetDetector::process` (hop) | onsetDetected, onsetStrength | .cpp:144-147 |
| 5 | BPM + downbeat + **phrase** | `feedSilenceDetection`, `process`, `feedDownbeatFeatures` | bpm, beatPhase, trackerState, beatInBar, barPhase, downbeatDetected, barCount, phrasePhase | .cpp:153-173 |
| 6 | MFCC | `MFCCExtractor::process` (mag) | mfccs[13] | .cpp:179-181 |
| 7 | Chroma + **HCDF** | `ChromaExtractor::process` (mag) | chromagram[12], harmonicChangeDetection | .cpp:187-192 |
| 8 | **Key** | `KeyDetector::process` (chroma) | detectedKey, keyIsMajor | .cpp:198-201 |
| 9 | **Pitch** | `PitchTracker::process` (hop) | dominantPitch, pitchConfidence | .cpp:207-210 |
| 10 | Loudness | `LoudnessAnalyzer::process` (hop) | lufs, dynamicRange | .cpp:216-219 |
| 11/12 | Transient density → Structural | inline sliding window, then `StructuralDetector::process` | transientDensity, structuralState | .cpp:225-244 |
| 13 | Genre | `GenreDetector::process` | detectedGenre, genreConfidence, energyState, genreScores[8] | .cpp:250-274 |
| 14 | Advanced (P25) | `AdvancedAudioAnalyzer::process` | sidechainPump, swingRatio, formantPresence, resonancePeak, reeseBass | .cpp:280-291 |

**One-frame lag (by design):** stage 5 (BPM downbeat scoring) consumes `prevHCDF_` and
`prevStructuralState_` because HCDF (stage 7) and structural (stage 11) aren't computed
yet on the current hop. (.cpp:162-164,192,244)

---

## 5. Per-Feature Algorithm Census

| Feature | Class | Algorithm | Field | Evidence |
|---------|-------|-----------|-------|----------|
| RMS | inline | sqrt(mean(s²)) over 2048 block | rms | AnalysisThread.cpp:100-112 |
| Peak | inline | max|s| over block | peak | .cpp:101-108 |
| RMS dB | inline | 20·log10(rms), floor −100 | rmsDB | .cpp:114 |
| LUFS | LoudnessAnalyzer | 2 K-weighting biquads (ITU-R BS.1770 48k coeffs) → 400ms window → −0.691+10log10(meanSq) | lufs | LoudnessAnalyzer.cpp:13-30,77-85 |
| Dynamic range | LoudnessAnalyzer | crest = rawPeak/rms | dynamicRange | LoudnessAnalyzer.cpp:96-100 |
| Transient density | inline | onset count in 256-hop (~2.7s) window / windowSec | transientDensity | AnalysisThread.cpp:113,228-238 |
| Spectral centroid | SpectralFeatures | Σ(f·M)/ΣM, k≥1 | spectralCentroid | SpectralFeatures.cpp:42-51 |
| Spectral flux | SpectralFeatures | half-wave rect L2 of ΔM, running-max norm (0.9995 decay) | spectralFlux | SpectralFeatures.cpp:53-77 |
| Spectral flatness | SpectralFeatures | Wiener entropy exp(mean ln P)/mean P on power | spectralFlatness | SpectralFeatures.cpp:83-111 |
| Spectral rolloff | SpectralFeatures | freq at 85% cumulative magnitude | spectralRolloff | SpectralFeatures.cpp:113-128 |
| 7-band energies | SpectralFeatures | Σmag² per band → sqrt → running-max norm; edges {20,60,250,500,2k,4k,6k,20k} | bandEnergies[7] | SpectralFeatures.cpp:16-37,130-150 |
| Onset detected/strength | OnsetDetector | aubio "specflux", thr 0.3, silence −70dB, minioi 50ms, awhitening on | onsetDetected, onsetStrength | OnsetDetector.cpp:8-23,32-41 |
| BPM | BPMTracker | aubio_tempo "default" → fold[60,200]→conf gate 0.1→octave→median(48)→hysteresis(200 hops) | bpm | BPMTracker.cpp:46-173 |
| Beat phase | BPMTracker | free-run sawtooth from locked BPM; hard reset on conf≥0.5 beat | beatPhase | BPMTracker.cpp:175-196 |
| Tracker state | BPMTracker | 0 searching / 1 locking / 2 locked | trackerState | BPMTracker.cpp:118-170 |
| Beat-in-bar / downbeat | BPMTracker | per-beat score 0.5·bass+0.3·flux+0.2·HCDF, 16-beat buffer, lock ≥8 beats | beatInBar, downbeatDetected | BPMTracker.cpp:279-398 |
| Bar phase | BPMTracker | (beatInBar+phase)/4 | barPhase | BPMTracker.cpp:400-416 |
| Bar count / phrase phase | BPMTracker | downbeat edge → barCount; (bar%N+barPhase)/N, N default 8; reset on drop/leaving breakdown | barCount, phrasePhase | BPMTracker.cpp:418-460 |
| MFCC | MFCCExtractor | 40 mel filters (20-8kHz) → log(floor 1e-10) → DCT-II 13 | mfccs[13] | MFCCExtractor.cpp:31-166 |
| Chroma | ChromaExtractor | Σmag² per pitch class (C0=16.35Hz map, ≥65Hz), normalize sum=1 | chromagram[12] | ChromaExtractor.cpp:16-74 |
| HCDF | ChromaExtractor | Euclidean distance to previous chroma | harmonicChangeDetection | ChromaExtractor.cpp:76-92 |
| Key / mode | KeyDetector | Krumhansl-Kessler profiles × Pearson over 24 rotations + 10-frame hysteresis; conf=(r+1)/2 | detectedKey, keyIsMajor | KeyDetector.cpp:38-110 |
| Pitch / confidence | PitchTracker | aubio "yinfft", tol 0.7, silence −60dB, unit Hz | dominantPitch, pitchConfidence | PitchTracker.cpp:5-40 |
| Structural state | StructuralDetector | 4 EMA scales (0.1/1/4/16s) RMS+flux; drop/buildup/breakdown thresholds; ~200ms hysteresis | structuralState | StructuralDetector.cpp:3-78 |
| Genre + energy | GenreDetector | 8-genre weighted scoring, EMA ~2s, hysteresis ~3s (conf gate 0.15), energy EMA 3-band | detectedGenre, genreConfidence, energyState, genreScores[8] | GenreDetector.cpp:24-306 |
| Sidechain pump | AdvancedAudioAnalyzer | −Pearson(bass,mid) over 64-hop (~680ms) window, asym EMA | sidechainPump | AdvancedAudioAnalyzer.cpp:42-91 |
| Swing ratio | AdvancedAudioAnalyzer | IOI pairs (32 buf, ≥8), longer/(long+short), EMA 0.03 | swingRatio | AdvancedAudioAnalyzer.cpp:93-148 |
| Formant presence | AdvancedAudioAnalyzer | 300-3000Hz energy / total, running-max norm, EMA 0.08 | formantPresence | AdvancedAudioAnalyzer.cpp:150-182 |
| Resonance peak | AdvancedAudioAnalyzer | spectral kurtosis 200-8000Hz, map [3,30]→[0,1], asym EMA | resonancePeak | AdvancedAudioAnalyzer.cpp:184-225 |
| Reese bass | AdvancedAudioAnalyzer | power-weighted freq std-dev 30-200Hz, running-max norm, EMA 0.06 | reeseBass | AdvancedAudioAnalyzer.cpp:227-280 |

**FeatureSnapshot = 40 member declarations** (arrays counted once): timing 2, amplitude/
dynamics 6, spectral 4, bands 1 (×7), onset+rhythm 10 (rms/onset/bpm/beatPhase/trackerState/
beatInBar/barPhase/downbeatDetected/barCount/phrasePhase), structural 1, chroma+harmony 5,
mfccs 1 (×13), genre 4, advanced 5. (FeatureSnapshot.h:5-90)

---

## 6. Doc-Verify — Discrepancies (doc claim → source truth)

### CLAUDE.md

| Doc loc | Claim | Source truth |
|---------|-------|--------------|
| CLAUDE.md:373 | Pipeline step 1 "Raw time-domain: RMS, peak, **ZCR**" | No ZCR computed anywhere (grep `zcr/zeroCrossing` in src = 0). Stage 1 is RMS/peak only. AnalysisThread.cpp:99-118 |
| CLAUDE.md:380-381 | step 8 = **Pitch**, step 9 = **Key detection** | Code: stage 8 = Key (.cpp:198), stage 9 = Pitch (.cpp:207). Order swapped. Header docstring agrees with code (AnalysisThread.h:34-36) |
| CLAUDE.md:384 | step 12 = **HCDF** (separate, after structural) | HCDF computed inside Chroma stage 7 (ChromaExtractor.cpp:76-92; AnalysisThread.cpp:191) |
| CLAUDE.md:386 | step 14 = **Phrase tracking** (separate late stage) | Phrase tracking runs inside BPM stage 5 via `feedDownbeatFeatures`→`updatePhrase` (BPMTracker.cpp:418-460; AnalysisThread.cpp:163) |
| CLAUDE.md:372-389 | **16-stage** pipeline | 14 numbered compute steps, 13 profiled. AnalysisThread.cpp:317-321 (13 labels), kNumStages=14 (AnalysisThread.h:134) |
| CLAUDE.md:37-74 | FeatureSnapshot table (36 rows) | Omits 4 real fields: `trackerState`, `beatInBar`, `barPhase`, `downbeatDetected` (FeatureSnapshot.h:41,45-46) |
| CLAUDE.md:1131,1149 | Genre "stage 13", Advanced "stage 14" | Correct — matches code stages 13/14. (consistent) |

### .harmony/FEATURES.md

| Doc loc | Claim | Source truth |
|---------|-------|--------------|
| FEATURES.md:68 | "58 audio features … **16-stage** pipeline" | Pipeline populates 40 snapshot fields via 14/13 stages (58 = downstream MappingSource count, not pipeline stages) |
| FEATURES.md:77 | "Stage 1: Raw time-domain — RMS, peak, **ZCR**" | No ZCR (see above) |
| FEATURES.md:83-84 | Stage 8 = Pitch, Stage 9 = Key | Swapped vs code (Key 8, Pitch 9) |
| FEATURES.md:87 | "Stage 12: HCDF" | HCDF is in Chroma stage 7 (doc self-contradicts: gotcha :136 correctly notes prevHCDF one-frame lag) |
| FEATURES.md:91 | "Stage 16: Phrase tracking" | In BPM stage 5 (see above) |
| FEATURES.md:24 | "OS delivers **128 samples**… every 2.67ms" | Buffer size is OS-configured, not fixed 128; doc acknowledges this at :43. Low-risk assumption |
| FEATURES.md:155 | GenreSmoothing "used by the MappingEngine's per-mapping smoothers" | FALSE — `GenreSmoothing` has zero external references (grep = self-only). Dead struct |
| FEATURES.md:133,136 | Profiling bug (13 names vs kNumStages 14); one-frame HCDF lag | Correct — matches source |

### docs/FEATURE_INVENTORY.md

| Doc loc | Claim | Source truth |
|---------|-------|--------------|
| FEATURE_INVENTORY.md:53,62 | "14 stages … header says 13; profiler 13 labels but timing is 14" | Accurate — most correct of the three docs |
| FEATURE_INVENTORY.md:63,675 | `clear()` leaves `detectedGenre=0` (House) instead of default 6 | Confirmed: `clear()` memsets then restores only rmsDB/lufs/detectedKey/keyIsMajor/swingRatio, not detectedGenre/energyState (FeatureSnapshot.h:81-89) |
| FEATURE_INVENTORY.md:40 | "40 rows (33 unique declarations + 7 band sub-entries)" | 40 total member declarations; the "33 + 7" split is loose arithmetic but the 40 total is right |
| FEATURE_INVENTORY.md:64 | PCM snapshot path undocumented in CLAUDE.md | Confirmed — `getPCMSamples` consumed only by Renderer.cpp:771 |

---

## 7. Health Read — FLAGGED

| Item | Category | Evidence |
|------|----------|----------|
| `GenreSmoothing` (analysis/GenreSmoothing.h) | **DEAD** — never instantiated; only self-references. FEATURES.md:155 wrongly claims it drives MappingEngine | grep src = self-only |
| `OneEuroFilter` (features/Smoother.h:49) | **DEAD** — never instantiated; only a comment ref (GenreSmoothing.h:16). Mapping uses EMA `Smoother` only. CLAUDE.md:631 documents its defaults as if live | MappingEngine.cpp:9,197 uses `Smoother` not OneEuro |
| Onset/BPM config setters: `OnsetDetector::setThreshold/setSilence/setMinInterOnsetMs`, `BPMTracker::setThreshold/setSilence/setPhraseBars` | **ORPHANED** — public tuning API with no callers (phrase length stays default 8) | grep = 0 external callers |
| `FeatureSnapshot::clear()` genre reset | **LATENT BUG** — leaves detectedGenre=0/energyState=0, not struct defaults 6/1; FeatureBus inits all 3 buffers via clear() | FeatureSnapshot.h:81-89; FeatureBus.cpp:5-6 |
| Waveform snapshot torn read | **RISK** — count published release, buffer via plain memcpy | AnalysisThread.cpp:340-343 |
| Aubio ctors unchecked | **RISK** — `new_aubio_*` can return null; used without guards | OnsetDetector.cpp:8; BPMTracker.cpp:10; PitchTracker.cpp:8 |
| Profiling: `stageNames[13]` vs `kNumStages=14`; slot [13] accumulated-never-logged | **MINOR** — dev stderr only | AnalysisThread.cpp:317-334; AnalysisThread.h:134 |
| 48kHz hardcoded (`kSampleRate=48000`, K-weighting coeffs, all freq maths) | **RISK** — no runtime SR validation | AnalysisThread.h:46; LoudnessAnalyzer.cpp:13-29 |

**WORKS:** RT discipline (pre-allocated aubio/FFT/buffers, SPSC + triple-buffer are correct
lock-free implementations); all 30 features compute with real algorithms and are wired
into the snapshot; File and Mic input paths both functional; per-stage profiler + CPU-load
EMA give live introspection.

**DOESN'T / GAPS:** GenreSmoothing + OneEuroFilter dead; several tuning setters orphaned;
`clear()` genre default bug; ZCR advertised but absent; 48kHz assumption unguarded.
