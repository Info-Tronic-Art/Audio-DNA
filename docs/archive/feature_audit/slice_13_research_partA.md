# Slice 13 — Research Documents Partial Audit (ARCH_*, FEATURES_*, LIB_*, REF_*, BPM_STABILITY_RESEARCH, INDEX.md)

Source documents under `/Users/boriskarpman/Documents/RealTimeAudio/research/`. All line references below are against those originals.

## Audio Feature Catalog from FEATURES_*.md

### FEATURES_amplitude_dynamics.md (11 features + sub-variants)
- **RMS Energy** — block-based, sliding-window, EMA variants (research/FEATURES_amplitude_dynamics.md:26-113)
- **Peak Amplitude** — sample-accurate peak (research/FEATURES_amplitude_dynamics.md:129-137)
- **True Peak (ITU-R BS.1770-4)** — 4× oversampled, 48-tap polyphase FIR interpolation (research/FEATURES_amplitude_dynamics.md:139-200)
- **Crest Factor** — Peak/RMS ratio, linear and dB (research/FEATURES_amplitude_dynamics.md:216-273)
- **Dynamic Range** — instantaneous, short-term, block, percentile (5th–95th) variants (research/FEATURES_amplitude_dynamics.md:275-341)
- **LUFS / LKFS Loudness (ITU-R BS.1770)** — K-weighting (pre-filter shelf + RLB high-pass biquads), momentary/short-term/integrated windows with 10 Hz / 1 Hz / end-of-file update rates (research/FEATURES_amplitude_dynamics.md:343-575)
- **Loudness Range (LRA / EBU R128)** — 95th-10th percentile distribution of short-term LUFS with absolute -70 LUFS + relative -20 LU gating (research/FEATURES_amplitude_dynamics.md:577-648)
- **Noise Floor Estimation** — min energy tracking, 5th-percentile robust, spectral-floor (research/FEATURES_amplitude_dynamics.md:650-695)
- **Signal-to-Noise Ratio (SNR)** — peak-to-floor, spectral, activity-based (research/FEATURES_amplitude_dynamics.md:697-733)
- **Dynamic Compression Detection** — crest analysis, loudness variance, gain-reduction, waveform flatness (research/FEATURES_amplitude_dynamics.md:735-786)
- **Clipping Detection** — sample-level threshold (>=0.9999), inter-sample-peak via true peak (research/FEATURES_amplitude_dynamics.md:787-841)
- **Envelope Followers** — attack/release, linear release, log-linear (dB), adaptive, multi-band (research/FEATURES_amplitude_dynamics.md:843-999)

### FEATURES_spectral.md (14 spectral features + 6 window functions)
Window functions catalogued: Hann, Hamming, Blackman, Blackman-Harris 4-term, Kaiser (tunable beta), Flat-Top (research/FEATURES_spectral.md:11-196).
- **Spectral Centroid** (research/FEATURES_spectral.md:321-345)
- **Spectral Flux** — L2 half-wave-rectified (research/FEATURES_spectral.md:347-389)
- **Spectral Rolloff** — 85% and 95% variants (research/FEATURES_spectral.md:391-420)
- **Spectral Flatness (Wiener Entropy)** (research/FEATURES_spectral.md:422-461)
- **Spectral Contrast** — 7 octave sub-bands, peak/valley dB (research/FEATURES_spectral.md:463-514)
- **Spectral Bandwidth / Spread** (research/FEATURES_spectral.md:516-542)
- **Spectral Skewness** — third standardized moment (research/FEATURES_spectral.md:544-571)
- **Spectral Kurtosis** — fourth standardized moment (research/FEATURES_spectral.md:573-600)
- **Spectral Entropy** — Shannon, normalized (research/FEATURES_spectral.md:602-635)
- **Spectral Irregularity** — Jensen + Krimphoff variants (research/FEATURES_spectral.md:637-680)
- **Spectral Decrease** (research/FEATURES_spectral.md:682-706)
- **Spectral Slope** — linear-regression slope magnitude/Hz (research/FEATURES_spectral.md:708-737)
- **Odd/Even Harmonic Ratio (OER)** — requires f0 (research/FEATURES_spectral.md:739-770)
- **Tristimulus (Pollard & Jansson)** — T1/T2/T3 — requires f0 (research/FEATURES_spectral.md:772-813)

### FEATURES_frequency_bands.md
- **7-band standard model** (Sub 20-60, Bass 60-250, Low-mid 250-500, Mid 500-2k, High-mid 2-4k, Presence 4-6k, Brilliance 6-20k) (research/FEATURES_frequency_bands.md:12-23)
- **Bark scale** (Traunmuller formula, Zwicker 24-band table) (research/FEATURES_frequency_bands.md:86-151)
- **Mel scale** (O'Shaughnessy + HTK variant, triangular filterbank) (research/FEATURES_frequency_bands.md:153-232)
- **ERB scale** (Glasberg & Moore) — ~39 bands over 20–20000 Hz (research/FEATURES_frequency_bands.md:233-272)
- **Constant-Q Transform (CQT)** — Brown-Puckette + kernel-based Schorkhuber-Klapuri (research/FEATURES_frequency_bands.md:274-362)
- **IIR filterbank (Butterworth biquad bandpass)** (research/FEATURES_frequency_bands.md:364-441)
- **FIR filterbank** — windowed sinc (research/FEATURES_frequency_bands.md:436-501)
- **A/C/K-weighting filters** with IEC 61672 and ITU-R BS.1770 coefficients (research/FEATURES_frequency_bands.md:505-622)
- **Octave bands** — 1/1, 1/3, 1/6 ISO-266 preferred (research/FEATURES_frequency_bands.md:863-995)

### FEATURES_mfcc_mel.md
- **MFCC pipeline** — pre-emphasis (α=0.95–0.97), framing (1024/512 @ 44.1 kHz), Hann window, FFT, power spectrum, mel filterbank (26/40/128 filters), log compression, DCT-II, retain first 13/20/40 coefficients, optional sinusoidal liftering (research/FEATURES_mfcc_mel.md:16-383)
- **Delta / delta-delta MFCCs** — velocity + acceleration (implied from deltas discussion in config section)
- **Mel Spectrogram** — CNN-oriented representation (referenced)
- **Chroma features** — 12 pitch classes (referenced in document header, full detail in FEATURES_pitch_harmonic)
- **Tonnetz** — 6D tonal-centroid representation (referenced in document header)

### FEATURES_pitch_harmonic.md
- **YIN** — difference function, CMND, absolute threshold, parabolic interpolation, best local estimate, threshold 0.10-0.20 (research/FEATURES_pitch_harmonic.md:11-157)
- **pYIN** — probabilistic with HMM Viterbi decoding (research/FEATURES_pitch_harmonic.md:158-177)
- **Autocorrelation** — baseline (research/FEATURES_pitch_harmonic.md:179-195)
- **Harmonic Product Spectrum (HPS)** — downsampling multiplication (research/FEATURES_pitch_harmonic.md:197-221)
- **McLeod Pitch Method (MPM)** — Normalized Square Difference Function (research/FEATURES_pitch_harmonic.md:222-242)
- **CREPE** — 6-layer CNN, 91% RPA at 50-cent tolerance (research/FEATURES_pitch_harmonic.md:243-257)
- **Voicing detection** — multi-stage (silence, ZCR, YIN confidence) (research/FEATURES_pitch_harmonic.md:277-323)
- **Polyphonic pitch detection** — NMF, Harmonic Summation (Melodia-style), Basic Pitch (Spotify), CREPE Multi-f0 (research/FEATURES_pitch_harmonic.md:325-379)
- **Key detection** — Krumhansl-Schmuckler with 12+12 key profiles: Major `[6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88]` and Minor `[6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17]` (research/FEATURES_pitch_harmonic.md:382-398)
- Further topics in file (based on IMP index): Chord recognition from chroma templates, HCDF, inharmonicity, HNR, Tonnetz, pitch-to-hue visual mapping (research/INDEX.md:124)

### FEATURES_psychoacoustic.md
- **Perceived Loudness (Zwicker ISO 532-1 and Moore-Glasberg ISO 532-2)** — critical-band decomposition, excitation patterns, upward/downward masking, specific loudness, total loudness (sone/phon) (research/FEATURES_psychoacoustic.md:24-117)
- **Simplified real-time approximation** — outer/middle ear weighting, 24 Bark bands, compressive power-law (exponent 0.23), attack 5 ms / release 50 ms (research/FEATURES_psychoacoustic.md:118-209)
- **Sharpness (Aures / DIN 45692:2009)** — weighting function with Bark threshold 15.8, normalization k=0.11; unit = acum (research/FEATURES_psychoacoustic.md:213-295)
- **Roughness (Fastl & Zwicker, Daniel & Weber)** — envelope modulation energy, MTF peak at 70 Hz; unit = asper (research/FEATURES_psychoacoustic.md:297-398)
- Based on INDEX: Fluctuation strength, Sensory pleasantness, Tonalness, Perceptual brightness/warmth, Stereo width/spread/imbalance, Binaural cues (ITD/ILD), Weber-Fechner law, Stevens' power law (research/INDEX.md:140-143)

### FEATURES_rhythm_tempo.md
- **Onset Detection Functions (6 methods)**: Spectral Flux, High Frequency Content (HFC), Complex Domain, Phase Deviation, Weighted Phase Deviation, Modified Kullback-Leibler (research/FEATURES_rhythm_tempo.md:36-167)
- **Peak Picking** — adaptive threshold (median-based), causal one-sided window variants (research/FEATURES_rhythm_tempo.md:170-231)
- **Onset Strength Envelope (OSE)** — smoothed non-negative ODF (research/FEATURES_rhythm_tempo.md:235-245)
- **BPM Detection** — Autocorrelation, Comb Filter Bank (Scheirer 1998), FFT of onset envelope (research/FEATURES_rhythm_tempo.md:248-309)
- **Half/double-tempo correction** — Rayleigh weighting at ~120 BPM, resonance-ratio check, genre priors (research/FEATURES_rhythm_tempo.md:319-329)
- Further topics: Beat tracking (Ellis DP, particle filter, DBN), downbeat detection, meter estimation, groove/swing quantification, beat prediction for zero-latency sync (research/INDEX.md:116-118)

### FEATURES_transients_texture.md
- **HPSS (Harmonic-Percussive Source Separation)** — median filtering (Fitzgerald 2010), Wiener masks, causal one-sided approximation for real-time (research/FEATURES_transients_texture.md:15-246)
- **Transient analysis** — attack time detection via envelope threshold crossings, Logarithmic Attack Time (LAT) (research/FEATURES_transients_texture.md:258-299)
- From INDEX: transient density, sharpness, ZCR, roughness models (Sethares, Vassilakis), textural complexity, noise vs tonal content, percussive element isolation (kick/snare/hat/crash band-limited detection) (research/INDEX.md:128-131)

### FEATURES_structural.md
- **Novelty functions** — spectral flux (broadband + 4 sub-bands), Harmonic Change Detection Function (HCDF) with L2 and cosine variants, MFCC-based timbral novelty, multi-feature fusion with weights w_sf=0.4, w_hc=0.35, w_tn=0.25 (research/FEATURES_structural.md:17-204)
- **Multi-scale energy envelopes** — 4 EMA trackers at 100 ms / 1 s / 4 s / 16 s with pairwise ratios (research/FEATURES_structural.md:209-298)
- Additional topics from INDEX: buildup detection, drop detection, breakdown detection, phrase detection, real-time self-similarity with 320 KB ring buffer, segment labeling (intro/verse/chorus/drop) (research/INDEX.md:133-136)

## Library decisions

**Chosen (from code audit, matches CLAUDE.md):**
- **Aubio 0.4.9+** (GPLv3) — onset detection (9 methods), tempo/beat tracking (Davies & Plumbley 2007 two-state model with Rayleigh weighting ~120 BPM), pitch detection (7 methods), spectral descriptors, MFCC. ~500 KB library, deterministic memory, C API `new_*/_do/_get_*/_set_*/del_*` (research/LIB_aubio.md:14-201)
- **JUCE 7.x/8.x** (GPLv3 / commercial) — AudioDeviceManager, AudioIODeviceCallback, juce::dsp::FFT (uses vDSP on macOS, IPP if available), OpenGL integration, message thread (research/LIB_juce.md:1-100)
- **juce::dsp::FFT** — bundled with JUCE, sufficient at 2048-point (implicit from JUCE choice)

**Rejected:**
- **Essentia** (AGPL, massive dep tree including FFTW/TagLib/yaml-cpp). Two modes: `essentia::standard` (pull) vs `essentia::streaming` (push dataflow graph with `>>` connector). Rejected for dependency weight (research/LIB_essentia.md:1-100)
- **Rust ecosystem** (cpal, rustfft, dasp, aubio-rs, pitch-detection, fundsp, spectrum-analyzer). Memory-safe, `Send`/`Sync` compile-time data-race prevention. Rejected — younger ecosystem, some gaps (VST hosting, advanced resampling) (research/LIB_rust_ecosystem.md:1-80)
- **FFTW3** — gold-standard FFT with planner/wisdom/SIMD, `FFTW_ESTIMATE/MEASURE/PATIENT/EXHAUSTIVE` planning modes. GPLv2 (requires commercial license for closed-source). Rejected in favor of juce::dsp::FFT at 2048-point where it suffices (research/LIB_fft_comparison.md:1-96)
- **miniaudio** (MIT-0 public domain single-header) — considered but JUCE subsumes I/O role (research/LIB_rtaudio_miniaudio.md:22-79)
- **PortAudio, RtAudio, libsoundio** — other I/O libs evaluated, all subsumed by JUCE (research/LIB_rtaudio_miniaudio.md:1-19)
- **KissFFT, PFFFT, Kfr, Apple vDSP, Intel IPP/MKL, ne10** — other FFT libs evaluated, not directly chosen (research/INDEX.md:167-171)

## Genre parameter presets (REF_genre_parameter_presets.md)

Eight genres with complete parameter values. All numbers verbatim from research/REF_genre_parameter_presets.md sections 1-8 + 9 consolidated tables.

### 1. Techno / Electronic (research/REF_genre_parameter_presets.md:25-96)
- BPM range: 120-140 BPM; beat confidence threshold 0.85
- FFT size 2048 / Hop 512 / Hann window / sample rate 44100
- Frame rate 86.1 Hz / Freq resolution 21.5 Hz / Time resolution 11.6 ms
- Tempo drift tolerance 1.0%; Meter 4/4 lock; Half-tempo rejection: No
- Onset: band-limited spectral flux, threshold 1.5, min-IOI 200 ms, silence -60 dBFS, band 0-200 Hz
- Hi-hat onsets: separate HF flux 8-16 kHz, threshold 0.8
- Band energy weights: Sub 0.30, Bass 0.25, Low-mid 0.15, Mid 0.15, High-mid 0.10, High 0.05
- Envelope: attack 5.0 ms / release 80.0 ms / smoothing window 0.5 s
- Sidechain detection: correlation > 0.7 of inverted sub-bass vs mid-range
- RMS dBFS range [-30, -6]; Centroid [200, 6000] Hz; Onset rate [1, 8]/s; Bass ratio [0.2, 0.8]
- Primary driver: sub-bass pulse; secondary: centroid sweep; tertiary: hi-hat density

### 2. House (research/REF_genre_parameter_presets.md:99-172)
- BPM range: 118-132 BPM; beat confidence threshold 0.80
- FFT 2048 / Hop 512 / Hann / 44100; Frame rate 86.1 Hz / 21.5 Hz / 11.6 ms
- Tempo drift tolerance 1.5%; Meter 4/4 lock; Half-tempo rejection: No
- Onset: band-limited SF, threshold 1.3, min-IOI 200 ms, silence -60 dBFS, band 0-200 Hz
- Vocal detection via LPC formants F1 (300-900 Hz) and F2 (800-2500 Hz); simpler proxy spectral flatness < 0.2 in 300-3000 Hz
- Band energy weights: Sub 0.20, Bass 0.20, Low-mid 0.20, Mid 0.25, High-mid 0.10, High 0.05
- Envelope: attack 5.0 ms / release 120.0 ms / smoothing 0.5 s
- RMS [-30, -6]; Centroid [300, 7000]; Onset rate [1, 8]; Bass ratio [0.1, 0.6]
- Primary driver: mid-range energy; secondary: vocal presence; tertiary: chroma (harmony)

### 3. Drum & Bass (research/REF_genre_parameter_presets.md:175-262)
- BPM range: 165-185 BPM; beat confidence threshold 0.55
- FFT 2048 / Hop **256** / Hann / 44100; Frame rate **172.3 Hz** / 21.5 Hz / **5.8 ms**
- Tempo drift tolerance 2.0%; Meter 4/4 half-bar; Half-tempo rejection: **Yes** (`while bpm < 150: *= 2; while bpm > 195: /= 2`)
- Onset: broadband SF, threshold 0.8, min-IOI **40 ms**, silence -55 dBFS, Broadband
  - Low band 20-200 Hz threshold 1.2 (kick), Mid 200-4000 threshold 1.0 (snare), High 4000-16000 threshold 0.8 (hi-hat)
- Reese bass detection: spectral_spread × AM rate × flux in 30-200 Hz; score > 0.3 indicates Reese
- Band energy weights: Sub 0.25, Bass 0.20, Low-mid 0.15, Mid 0.15, High-mid 0.15, High 0.10
- Envelope: attack **1.0 ms** / release **30.0 ms** / smoothing **0.25 s** (5× and 2.7× faster than Techno)
- RMS [-25, -6]; Centroid [200, 8000]; Onset rate **[4, 25]**; Bass ratio [0.15, 0.7]
- Primary driver: transient density; secondary: Reese bass phase; tertiary: multi-band onset triggers

### 4. Ambient (research/REF_genre_parameter_presets.md:265-332)
- BPM range: N/A; beat confidence threshold 0.95 (essentially never locks); fallback FREERUN
- FFT **4096** / Hop **1024** / Hann / 44100; Frame rate **43.1 Hz** / **10.8 Hz** / **23.2 ms**
- Tempo drift tolerance N/A; Meter Free; Half-tempo rejection: N/A
- Onset: **disabled** (N/A)
- Centroid EMA alpha: `1 - exp(-1 / (2.0 * fps))` (2-second time constant)
- Pad detection: spectral flux < 0.05 AND flatness < 0.3 AND high spread, persistent > 2 s
- Envelope window **16.0 s** / hop 0.5 s
- Band energy weights: Sub 0.10, Bass 0.10, Low-mid 0.20, Mid 0.25, High-mid 0.20, High 0.15
- Envelope ballistics: attack **50.0 ms** / release **4000.0 ms** / smoothing **16.0 s**
- RMS [-50, -20]; Centroid [100, 4000]; Onset rate [0, 2]; Bass ratio [0.0, 0.3]
- Primary driver: centroid drift; secondary: chroma/key; tertiary: dynamic level

### 5. Hip Hop (research/REF_genre_parameter_presets.md:335-392)
- BPM range: 75-110 BPM; beat confidence threshold 0.65
- FFT 2048 / Hop 512 / Hann / 44100; Frame rate 86.1 Hz / 21.5 Hz / 11.6 ms
- Tempo drift tolerance 3.0%; Meter 4/4; Half-tempo rejection: No
- Onset: band-limited SF, threshold 1.2, min-IOI 100 ms, silence -55 dBFS, band **20-200 Hz**
- Swing detection: inter-onset histogram → two-Gaussian fit; ratio 1.0=straight, 1.5=light, 2.0=triplet shuffle
- 808 bass detection: sub-bass energy > -20 dBFS AND flatness 20-80 Hz < 0.1 AND sustain > 150 ms above -30 dBFS
- Vocal onset from mid-band (200-4000 Hz) with drum subtraction
- Snare on 2/4: mid-band onset correlated with beat 2 and 4 within ±30 ms
- Band energy weights: Sub **0.35**, Bass 0.25, Low-mid 0.15, Mid 0.15, High-mid 0.05, High 0.05
- Envelope: attack 5.0 ms / release **200.0 ms** / smoothing 1.0 s
- RMS [-30, -8]; Centroid [200, 5000]; Onset rate [2, 12]; Bass ratio [0.3, 0.9]
- Primary driver: 808 bass sustain; secondary: swing/groove; tertiary: vocal rhythm

### 6. Rock / Metal (research/REF_genre_parameter_presets.md:395-455)
- BPM range: 100-**220** BPM; beat confidence threshold 0.60
- FFT 2048 / Hop 512 / **Blackman-Harris** window / 44100; Frame rate 86.1 Hz / 21.5 Hz / 11.6 ms
- Tempo drift tolerance 5.0%; Meter Variable; tempo re-estimate every 4 beats
- Onset: broadband SF, threshold 1.0, min-IOI 60 ms, silence -50 dBFS, Broadband
- Distortion: spectral slope in 100-5000 Hz: clean ~ -3 to -6 dB/oct, distorted ~ 0 to -1 dB/oct
- Crash detection: HF onset > 4 kHz + energy > -15 dBFS + 500-ms decay energy > 30% peak + high HF flatness
- Auto-gain compressor: `target + excess/ratio`; quiet expand up to 12 dB
- Band energy weights: Sub 0.10, Bass 0.20, Low-mid **0.25**, Mid 0.20, High-mid 0.15, High 0.10
- Envelope: attack **3.0 ms** / release 60.0 ms / smoothing 0.5 s
- RMS [-35, -6]; Centroid [300, 8000]; Onset rate [2, 16]; Bass ratio [0.05, 0.4]
- Primary driver: spectral slope (distortion); secondary: dynamic range; tertiary: crash triggers

### 7. Classical (research/REF_genre_parameter_presets.md:458-539)
- BPM range: **40-200** BPM; beat confidence threshold 0.40 (advisory mode); tempo smoothing 8 beats
- FFT **4096** / Hop **1024** / Hann / 44100; Frame rate 43.1 Hz / 10.8 Hz / 23.2 ms
- Tempo drift tolerance **15.0%**; Meter Variable; Half-tempo rejection: No
- Onset: broadband **HFC**, threshold 0.6, min-IOI 80 ms, silence **-65 dBFS**, Broadband
- Dynamic range expected: 50 dB; gain smoothing time 4.0 s
- Visual intensity curve: `pow(linear_amp / max, 0.3)` (gamma 0.3)
- Chroma: 2-s window for feature, 4-s analysis, median 16 s
- Polyphony: count chroma bins > 50% of max
- Section boundaries: silence > 500 ms below -50 dBFS, centroid jump > 1000 Hz in < 500 ms, chroma correlation drop below 0.5, RMS drop > 20 dB
- Band energy weights: Sub **0.05**, Bass 0.10, Low-mid 0.25, Mid **0.30**, High-mid 0.20, High 0.10
- Envelope: attack 10.0 ms / release 500.0 ms / smoothing 4.0 s
- RMS [-55, -10]; Centroid [200, 6000]; Onset rate [0.5, 10]; Bass ratio [0.0, 0.2]
- Primary driver: harmonic richness (chroma); secondary: dynamic level pp-ff; tertiary: section boundaries

### 8. Pop (research/REF_genre_parameter_presets.md:542-608)
- BPM range: 95-135 BPM; beat confidence threshold 0.75
- FFT 2048 / Hop 512 / Hann / 44100; Frame rate 86.1 Hz / 21.5 Hz / 11.6 ms
- Tempo drift tolerance 2.0%; Meter 4/4 lock; Half-tempo rejection: No
- Onset: band-limited SF, threshold 1.2, min-IOI 150 ms, silence -55 dBFS, band 0-200 Hz
- Vocal prominence: `(energy_300_3000 / energy_total) * (1 - flatness_300_3000)`; >0.4 vocal-dominant, <0.2 instrumental
- Chorus detection: 4-bar RMS window ratio > 1.5 (~3.5 dB increase)
- Drop detection: buildup < -15 dBFS for ~1 s then jump > 6 dB in 200 ms
- Band energy weights: Sub 0.15, Bass 0.20, Low-mid 0.20, Mid 0.25, High-mid 0.15, High 0.05
- Envelope: attack 5.0 ms / release 100.0 ms / smoothing 0.5 s
- RMS [-25, -6]; Centroid [300, 7000]; Onset rate [1, 8]; Bass ratio [0.1, 0.5]
- Primary driver: vocal prominence; secondary: verse/chorus energy; tertiary: drop detection

### Genre classifier feature profiles (research/REF_genre_parameter_presets.md:717-726)

| Genre | Centroid | Cent.Std | OnsetRate | BassRatio | BPM | Flatness | DynRange | BeatConf |
|-------|----------|----------|-----------|-----------|-----|----------|----------|----------|
| Techno | 1800 | 400 | 4.5 | 0.55 | 128 | 0.15 | 8 | 0.92 |
| House | 2200 | 600 | 4.0 | 0.40 | 124 | 0.20 | 10 | 0.88 |
| DnB | 2500 | 800 | 12.0 | 0.45 | 174 | 0.25 | 12 | 0.65 |
| Ambient | 1200 | 200 | 0.5 | 0.15 | 0 | 0.35 | 6 | 0.15 |
| Hip Hop | 1500 | 500 | 3.0 | 0.65 | 90 | 0.18 | 14 | 0.70 |
| Rock | 3000 | 900 | 6.0 | 0.20 | 130 | 0.30 | 20 | 0.60 |
| Classical | 2000 | 700 | 2.0 | 0.10 | 0 | 0.12 | 35 | 0.30 |
| Pop | 2500 | 500 | 4.0 | 0.30 | 115 | 0.22 | 12 | 0.80 |

Classifier: k-NN with k=5 on z-score-normalized features, or hand-crafted decision tree (research/REF_genre_parameter_presets.md:728-870). Genre smoothing: voting buffer size 8, switch threshold 6/8 (research/REF_genre_parameter_presets.md:895-934).

## Latency budgets (REF_latency_numbers.md)

### Audio capture latency by API (research/REF_latency_numbers.md:23-34)

| API | Platform | Typical | Best | Worst | Default Buffer | Min Buffer |
|-----|----------|---------|------|-------|----------------|------------|
| ASIO | Windows | 1-5 ms | 0.7 ms | 10 ms | 256 | 32 |
| Core Audio | macOS | 3-10 ms | 1.5 ms | 20 ms | 512 | 16 |
| WASAPI Exclusive | Windows | 3-10 ms | 2 ms | 15 ms | 480 | 128 |
| WASAPI Shared | Windows | 10-20 ms | 8 ms | 30 ms | 480 | 480 |
| JACK | Linux/macOS | 1-5 ms | 0.7 ms | 10 ms | 256 | 16 |
| ALSA | Linux | 5-15 ms | 2 ms | 25 ms | 512 | 32 |
| PulseAudio | Linux | 20-50 ms | 15 ms | 100 ms | 2048 | 512 |
| PipeWire | Linux | 5-15 ms | 2 ms | 20 ms | 256 | 32 |
| AAudio | Android | 10-25 ms | 5 ms | 50 ms | 192 | 96 |
| AVAudioEngine | iOS | 5-12 ms | 2 ms | 20 ms | 256 | 128 |

### Algorithm inherent latency (research/REF_latency_numbers.md:215-229, 342-358)

| Operation | Latency @ 48 kHz |
|-----------|-----------------|
| FFT 64 | 1.33 ms |
| FFT 128 | 2.67 ms |
| FFT 256 | 5.33 ms |
| FFT 512 | 10.67 ms |
| FFT 1024 | 21.33 ms |
| FFT 2048 | 42.67 ms |
| FFT 4096 | 85.33 ms |
| FFT 8192 | 170.67 ms |
| Spectral flux onset | 10.7-42.7 ms (per hop) |
| Beat tracking | 2000-8000 ms (convergence) |
| YIN pitch (440 Hz) | 5.75 ms |
| YIN pitch (82 Hz, low E) | 30.6 ms |
| Key detection | 5000-10000 ms |
| MFCC / spectral centroid / rolloff / chromagram | = FFT latency |

### Stage-by-stage pipeline budget (research/REF_latency_numbers.md:362-396 + research/ARCH_pipeline.md:619-649)

| Stage | Description | Typical |
|-------|-------------|---------|
| A. ADC conversion | Hardware ADC | 0.5-1.5 ms |
| B. OS audio buffer | Driver callback period | 2.67-10.67 ms (128-512 samples @ 48 kHz) |
| C. SPSC push | `try_push()` in callback | ~0.05 ms |
| D. Ring buffer transit | Hop period wait | 0-10.67 ms |
| E. FFT + feature extract | 2048-pt FFT + all features | 0.1-0.5 ms (FFT ~15-30 μs + features ~50-100 μs) |
| F. Feature bus publish | Atomic triple-buffer swap | ~0.01 ms |
| G. Render frame wait | Until VSync picks up snapshot | 0-16.67 ms |
| H. GPU render | Draw + shaders | 1-5 ms |
| I. Display scanout | VSync + panel | 4-16.67 ms |

Best case: ~8.4 ms; Typical ~24 ms; Worst case ~62 ms (research/ARCH_pipeline.md:633-652). Audio-visual perception tolerance ±20-80 ms (research/ARCH_pipeline.md:652).

### Benchmark targets (research/ARCH_pipeline.md:1119-1127)
- Audio callback duration: < 50 μs
- SPSC push latency: < 100 ns
- FFT 2048-point float: < 20 μs
- Full feature extraction: < 200 μs
- Feature bus publish/acquire: < 50 ns each
- End-to-end pipeline: < 35 ms avg

## BPM stabilization parameters

BPM_STABILITY_RESEARCH.md prescribes a multi-stage stabilization pipeline with the following user-facing thresholds and modes (research/BPM_STABILITY_RESEARCH.md:128-164):

- **BPM range gate**: min 60, max 200; outside → reject or fold via halving/doubling (research/BPM_STABILITY_RESEARCH.md:98-103, 134)
- **Confidence gate**: `aubio_tempo_get_confidence()` threshold (no specific value but "only update when confidence exceeds threshold") (research/BPM_STABILITY_RESEARCH.md:101-105)
- **Octave error correction**: `1.8 < ratio < 2.2 → /2`, `0.45 < ratio < 0.55 → *2` (research/BPM_STABILITY_RESEARCH.md:91-97)
- **Median filter window**: 32-64 estimates (~340-680 ms at 93 Hz hop rate); recommended 48 (~500 ms) (research/BPM_STABILITY_RESEARCH.md:84, 143-145)
- **Hysteresis lock**: new BPM must differ by ±2 BPM AND persist ~2-4 seconds (200 hops) to replace locked value (research/BPM_STABILITY_RESEARCH.md:108-122)
- **Light EMA alpha**: ~0.05 after median + hysteresis (cosmetic) (research/BPM_STABILITY_RESEARCH.md:125-127)
- **Tracker state UI**: 0=searching, 1=locking, 2=locked (research/BPM_STABILITY_RESEARCH.md:293)
- **Downbeat detection**: score = w1 × bassEnergy + w2 × spectralFlux + w3 × chromaChange; EDM weights `w1=0.5, w2=0.3, w3=0.2`; lock after 8+ consistent beats (research/BPM_STABILITY_RESEARCH.md:232-245)
- **Beat phase reset**: on aubio beat detection with confidence > 0.5 (research/BPM_STABILITY_RESEARCH.md:186-188)
- **Phrase tracking**: default 8-bar phrase; manual resync option (research/BPM_STABILITY_RESEARCH.md:261-267)
- **Ableton Link concept**: "quantum" = number of beats (typically 4) for phase-alignment agreement between devices (research/BPM_STABILITY_RESEARCH.md:27-28)

New FeatureSnapshot fields proposed (research/BPM_STABILITY_RESEARCH.md:282-296):
- `beatInBar` (uint8_t 0-3)
- `barPhase` (float [0,1))
- `downbeatDetected` (bool)
- `trackerState` (uint8_t: 0=searching, 1=locking, 2=locked)

## Section 18 partial: Features in research docs but possibly not in code

Cross-reference with the FeatureSnapshot fields in CLAUDE.md (the project's source of truth). The following features are extensively documented in the research but appear to be absent from `FeatureSnapshot` as listed in CLAUDE.md (or only partially implemented). This is a prioritized list of possible gaps:

**Likely absent (no corresponding snapshot field):**
- **True Peak (ITU-R BS.1770-4)** — FeatureSnapshot has `peak` but not the 4× oversampled true peak. Used for inter-sample clipping detection (research/FEATURES_amplitude_dynamics.md:139-200).
- **Loudness Range (LRA)** — Only `lufs` (momentary) is present; no short-term or integrated LUFS, no LRA (research/FEATURES_amplitude_dynamics.md:577-648).
- **Noise Floor estimate** (research/FEATURES_amplitude_dynamics.md:650-695).
- **SNR estimate** (research/FEATURES_amplitude_dynamics.md:697-733).
- **Compression detection** — The `dynamicRange` field stores crest factor but not sustained compression detection (research/FEATURES_amplitude_dynamics.md:735-786).
- **Clipping flag / inter-sample clipping** — No `isClipping` field (research/FEATURES_amplitude_dynamics.md:787-841).
- **Spectral Contrast** — 7-band peak/valley dB not in snapshot (research/FEATURES_spectral.md:463-514).
- **Spectral Bandwidth / Spread** (research/FEATURES_spectral.md:516-542).
- **Spectral Skewness** (research/FEATURES_spectral.md:544-571).
- **Spectral Kurtosis** — Note: P25 `resonancePeak` is described as "spectral kurtosis in 200-8000 Hz" so the band-limited kurtosis IS present; the general full-spectrum kurtosis is not.
- **Spectral Entropy** — Shannon entropy of the spectrum (research/FEATURES_spectral.md:602-635). Not to be confused with `spectralFlatness` (Wiener entropy), which IS present.
- **Spectral Irregularity** — Jensen and Krimphoff variants (research/FEATURES_spectral.md:637-680).
- **Spectral Decrease** (research/FEATURES_spectral.md:682-706).
- **Spectral Slope** — linear regression; used in rock/metal distortion detection per genre preset, but no snapshot field (research/FEATURES_spectral.md:708-737).
- **Odd/Even Harmonic Ratio (OER)** — requires f0 (research/FEATURES_spectral.md:739-770).
- **Tristimulus (T1/T2/T3)** — requires f0 (research/FEATURES_spectral.md:772-813).
- **24-band Bark energies** — CLAUDE.md has 7 `bandEnergies`, research documents a 24-Bark-band analyzer (research/FEATURES_frequency_bands.md:722-811).
- **Mel spectrogram (full 40 filters)** — MFCCs (13) are present; the raw mel filter outputs before DCT are not.
- **Delta / Delta-Delta MFCCs** — Velocity/acceleration of MFCCs (standard MFCC feature family).
- **Tonnetz (6D tonal centroid)** — listed in INDEX.md:110 as part of the MFCC/chroma module.
- **Chord recognition output** — chord templates from chroma (research/INDEX.md:124).
- **Pitch detection confidence/voicing decision** — partial: `pitchConfidence` exists, but no explicit multi-stage voicing gate (silence + ZCR + YIN confidence) (research/FEATURES_pitch_harmonic.md:297-323).
- **Zero-Crossing Rate (ZCR)** — standard time-domain feature, not in snapshot (research/INDEX.md:128).
- **Harmonic-to-Noise Ratio (HNR)** (research/INDEX.md:124).
- **Inharmonicity** (research/INDEX.md:124).
- **HPSS outputs** — harmonic-percussive separation with H/P ratio, percussive flux, harmonic stability (research/FEATURES_transients_texture.md:250-254).
- **Attack time / Logarithmic Attack Time (LAT)** (research/FEATURES_transients_texture.md:258-299).
- **Roughness models** — Sethares and Vassilakis (research/INDEX.md:131).
- **Roughness (asper, Daniel & Weber)** — research describes the model, but snapshot has no `roughness` field (research/FEATURES_psychoacoustic.md:297-398).
- **Sharpness (acum, Aures DIN 45692)** (research/FEATURES_psychoacoustic.md:213-295).
- **Perceived Loudness (sone, Zwicker / Moore-Glasberg)** — distinct from LUFS; uses compressive power law (0.23 exponent) on 24 Bark bands (research/FEATURES_psychoacoustic.md:24-209).
- **Fluctuation strength** (research/INDEX.md:140).
- **Tonalness, sensory pleasantness** (research/INDEX.md:140).
- **Stereo width / spread / imbalance, ITD/ILD binaural cues** (research/INDEX.md:142). The project appears to be mono-downmix only.
- **Novelty functions** — SF novelty, HCDF is present, MFCC-based timbral novelty, multi-feature fusion with z-score normalization (research/FEATURES_structural.md:17-204). Code may have structural state (buildup/drop/breakdown) but not the raw novelty curves.
- **Self-similarity recurrence** — real-time SSM, 320 KB ring buffer (research/INDEX.md:136).
- **Segment labeling** (intro/verse/chorus/drop/breakdown) (research/INDEX.md:137).
- **Beat prediction** — Extrapolation for zero-latency beat-sync rendering (research/INDEX.md:118; research/REF_latency_numbers.md:300-302).
- **Meter estimation** — beyond 4/4 lock (research/INDEX.md:118).
- **Tatum detection** — `aubio_tempo_was_tatum()` returns tatum subdivisions within a beat (research/BPM_STABILITY_RESEARCH.md:207).
- **Polyphony estimate** — used in classical preset (research/REF_genre_parameter_presets.md:498-505). Only `detectedKey`/`keyIsMajor` present.
- **Auto-gain / AGC** — IMPL_calibration_adaptation includes adaptive RMS normalization (Lemire's algorithm), percentile normalization (research/INDEX.md:230-232).
- **Feature normalization bounds per genre** — runtime auto-normalize vs genre-preset-based ranges (research/REF_genre_parameter_presets.md:669-678).
- **Silence detection module** — separate silence gate vs. the P23 BPM silence recovery (research/INDEX.md:232).

**Psychoacoustic weighting (A / C / K)**:
- A-weighting, C-weighting are documented (research/FEATURES_frequency_bands.md:505-598). K-weighting is present (via LUFS). A and C may be absent as explicit features.

**Platform-native I/O features** documented but unlikely to be exposed:
- Aggregate device creation (macOS) (research/ARCH_audio_io.md:750-800)
- Loopback capture via WASAPI flag, BlackHole driver, ScreenCaptureKit (macOS 13+), PulseAudio monitor sources (research/ARCH_audio_io.md:289-462)
- Device hot-plug notifications (research/ARCH_audio_io.md:823-1010)

## Section 20 partial: Cross-references

Research docs reference app code/architecture as follows:

- **FeatureSnapshot field-name parity**: Shader uniforms in the app use `u_[featureName]` matching FeatureSnapshot field names exactly (CLAUDE.md "Naming Conventions" reinforces this research-to-code invariant).
- **4-thread model** described in research/ARCH_pipeline.md:260-266 → implemented in src/audio/AudioCallback, src/analysis/AnalysisThread, src/render/Renderer (per CLAUDE.md section "The 4-Thread Model").
- **SPSC ring buffer** research design (research/ARCH_pipeline.md:106-241) → src/audio/RingBuffer.h (CLAUDE.md source tree).
- **Triple-buffer atomic swap** (research/ARCH_pipeline.md:717-815) → src/features/FeatureBus.h/cpp.
- **K-weighting biquad coefficients** at 48 kHz (research/FEATURES_amplitude_dynamics.md:361-378) → src/analysis/LoudnessAnalyzer.h/cpp.
- **Krumhansl-Schmuckler key profiles** (research/FEATURES_pitch_harmonic.md:392-397) → src/analysis/KeyDetector.h/cpp.
- **Aubio integration** (research/LIB_aubio.md:140-173) → src/analysis/OnsetDetector, BPMTracker, PitchTracker.
- **7-band analyzer config** (research/FEATURES_frequency_bands.md:644-718) → `bandEnergies[7]` in FeatureSnapshot.
- **BPM stabilization pipeline** (research/BPM_STABILITY_RESEARCH.md:128-164) → referenced in CLAUDE.md as "BPM stabilization pipeline (range gate → confidence → octave → median → hysteresis)".
- **Genre detection** 8 genres (research/REF_genre_parameter_presets.md) → src/analysis/GenreDetector.h/cpp (P23), implementing the 8-genre classifier described in section 10 of that file, with EMA smoothing matching the hysteresis concept. Genre smoothing params in src/analysis/GenreSmoothing.h.
- **Advanced analysis** (P25 features sidechain/swing/formant/resonance/reese) → derived from techniques described across multiple research docs:
  - Sidechain: research/REF_genre_parameter_presets.md:63-73 (Techno section)
  - Swing: research/REF_genre_parameter_presets.md:341-346 (Hip-Hop section)
  - Reese: research/REF_genre_parameter_presets.md:214-224 (DnB section)
  - Formant: research/REF_genre_parameter_presets.md:107-116 (House section)
  - Resonance peak (spectral kurtosis): research/FEATURES_spectral.md:573-600 + research/REF_genre_parameter_presets.md (Techno filter sweeps)
- **Structural state machine** (research/FEATURES_structural.md) → src/analysis/StructuralDetector.h/cpp.
- **JUCE AudioDeviceManager + AudioIODeviceCallback** (research/LIB_juce.md:10-100) → src/audio/AudioEngine.h/cpp.
- **Latency budget benchmark targets** (research/REF_latency_numbers.md + research/ARCH_pipeline.md:1119-1127) → constrain the project's render-to-visual ~15-25 ms goal stated in CLAUDE.md "Latency Budget" section.
