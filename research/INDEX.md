# Real-Time Audio Analysis Research Library — Master Index

> A comprehensive, deeply technical research library for building a cross-platform desktop application that analyzes any audio source (microphone, line-in, system loopback) with minimum latency, feeding extracted music features into a real-time video synthesis and VJ performance engine.

**29 documents** | **~100,000+ words** | **C++ code throughout** | **Cross-referenced**

---

## Quick Navigation

| Category | Documents | Focus |
|----------|-----------|-------|
| [Core Architecture](#core-architecture) | 3 | Pipeline, I/O, real-time rules |
| [Audio Features](#audio-features--complete-catalog) | 9 | Every extractable feature |
| [Libraries](#libraries--deep-dives) | 6 | Essentia, Aubio, JUCE, FFT, I/O, Rust |
| [Video/Visual](#video--visual-engine-integration) | 3 | Mapping, OpenGL, frameworks |
| [Implementation](#implementation-guides) | 4 | Setup, prototype, testing, calibration |
| [Reference](#reference) | 4 | Latency, math, genre presets, resources |

---

## Recommended Reading Orders

### Path 1: "Build the Prototype Fast" (5 docs)
1. [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md) — Get a working audio analyzer running in one session
2. [ARCH_pipeline.md](ARCH_pipeline.md) — Understand the full pipeline architecture
3. [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md) — Choose your audio I/O library
4. [FEATURES_spectral.md](FEATURES_spectral.md) — Add spectral features beyond the prototype
5. [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) — Map features to visuals

### Path 2: "Architect a Production System" (8 docs)
1. [ARCH_pipeline.md](ARCH_pipeline.md) — Pipeline architecture and thread model
2. [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md) — Real-time programming rules
3. [ARCH_audio_io.md](ARCH_audio_io.md) — Cross-platform audio I/O
4. [REF_latency_numbers.md](REF_latency_numbers.md) — Latency budget and measurement
5. [IMPL_project_setup.md](IMPL_project_setup.md) — CMake, dependencies, CI/CD
6. [LIB_juce.md](LIB_juce.md) — JUCE framework for the application shell
7. [LIB_essentia.md](LIB_essentia.md) — Essentia for feature extraction
8. [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md) — Audio-to-GPU rendering pipeline

### Path 3: "Deep Dive on Audio Features" (9 docs)
1. [REF_math_reference.md](REF_math_reference.md) — Mathematical foundations
2. [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md) — Amplitude and loudness
3. [FEATURES_spectral.md](FEATURES_spectral.md) — Spectral descriptors
4. [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md) — Band decomposition
5. [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md) — MFCCs, chroma, Tonnetz
6. [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) — Beat tracking and tempo
7. [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md) — Pitch and harmony
8. [FEATURES_transients_texture.md](FEATURES_transients_texture.md) — Transients and texture
9. [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md) — Perceptual features

### Path 4: "VJ Performance System" (6 docs)
1. [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) — Audio→visual mapping science
2. [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md) — GPU integration and shaders
3. [VIDEO_vj_frameworks.md](VIDEO_vj_frameworks.md) — Existing frameworks comparison
4. [FEATURES_structural.md](FEATURES_structural.md) — Buildup/drop/breakdown detection
5. [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md) — Live calibration
6. [REF_genre_parameter_presets.md](REF_genre_parameter_presets.md) — Genre-specific tuning

### Path 5: "Evaluate Rust vs C++" (3 docs)
1. [LIB_rust_ecosystem.md](LIB_rust_ecosystem.md) — Rust audio ecosystem
2. [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md) — RT constraints (compare approaches)
3. [LIB_fft_comparison.md](LIB_fft_comparison.md) — FFT libraries (rustfft vs FFTW)

---

## Core Architecture

### [ARCH_pipeline.md](ARCH_pipeline.md)
The complete real-time audio analysis pipeline architecture. Covers the full data flow from audio callback through ring buffer, analysis thread, feature bus, to render thread. Includes a production-grade SPSC lock-free ring buffer implementation, triple-buffer feature bus with atomic swap, platform-specific thread priority code (macOS/Windows/Linux), CPU affinity strategies, and a stage-by-stage latency budget breakdown.

- **Key topics**: Lock-free SPSC queue, triple buffering, thread priority, memory pre-allocation, xrun detection, graceful degradation
- **Related**: [ARCH_audio_io.md](ARCH_audio_io.md), [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md), [REF_latency_numbers.md](REF_latency_numbers.md), [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md)

### [ARCH_audio_io.md](ARCH_audio_io.md)
Cross-platform audio I/O deep dive covering every major audio API. Platform-specific implementation details for WASAPI, CoreAudio, ALSA, JACK, PulseAudio, and PipeWire. Complete loopback capture code for each platform. Five-library comparison (RtAudio, PortAudio, miniaudio, JUCE, libsoundio) with working code examples for each.

- **Key topics**: WASAPI loopback, CoreAudio aggregate devices, ScreenCaptureKit, buffer sizes, sample formats, device hot-plugging, virtual audio routing
- **Related**: [ARCH_pipeline.md](ARCH_pipeline.md), [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md), [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md), [LIB_juce.md](LIB_juce.md), [REF_latency_numbers.md](REF_latency_numbers.md)

### [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md)
The golden rules of real-time audio programming with detailed explanations of why each rule exists. Complete lock-free programming patterns with proper memory ordering. Real-time-safe memory pool implementation. Platform-specific thread priority and scheduling code. Top 10 common mistakes catalog with fixes.

- **Key topics**: No-malloc/no-mutex rules, acquire/release semantics, SPSC queue, priority inversion, SCHED_FIFO, MMCSS, RTSan, denormalized floats, false sharing
- **Related**: [ARCH_pipeline.md](ARCH_pipeline.md), [ARCH_audio_io.md](ARCH_audio_io.md), [REF_latency_numbers.md](REF_latency_numbers.md), [IMPL_testing_validation.md](IMPL_testing_validation.md)

---

## Audio Features — Complete Catalog

### [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md)
Every amplitude and dynamics feature: RMS, peak, true peak (ITU-R BS.1770-4), crest factor, LUFS (momentary/short-term/integrated with complete K-weighting implementation), loudness range, noise floor estimation, compression detection, clipping detection, envelope followers. Each with formula, C++ code, typical ranges, and visual mapping.

- **Key topics**: ITU-R BS.1770 K-weighting biquads, LUFS gating, envelope follower ballistics, multiband envelope
- **Related**: [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md), [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md)

### [FEATURES_spectral.md](FEATURES_spectral.md)
All 14 spectral features with mathematical formulas and C++ implementations: centroid, flux, rolloff, flatness, contrast, bandwidth/spread, skewness, kurtosis, entropy, irregularity (Jensen + Krimphoff), decrease, slope, odd/even harmonic ratio, tristimulus. Complete FFT setup guide with 6 window functions, FFTW wrapper class, and 55μs timing budget.

- **Key topics**: FFT setup, window functions, FFTW3 wrapper, magnitude processing, spectral smoothing, update rate decimation
- **Related**: [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md), [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md), [LIB_fft_comparison.md](LIB_fft_comparison.md), [REF_math_reference.md](REF_math_reference.md)

### [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md)
Frequency band decomposition: standard 7-band model, Bark scale (24 critical bands with Zwicker table), Mel scale (triangular filterbank construction), ERB scale. CQT vs STFT comparison. IIR filterbank design (Butterworth biquads). A/C/K-weighting filters with coefficients. Complete 7-band and 24-band analyzer C++ classes. Octave band analysis (1/1, 1/3, 1/6).

- **Key topics**: Bark/Mel/ERB scale formulas, CQT Brown-Puckette algorithm, Butterworth bandpass design, A-weighting IIR filter, ISO 266 preferred frequencies
- **Related**: [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md), [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md), [REF_math_reference.md](REF_math_reference.md)

### [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md)
Complete MFCC pipeline (pre-emphasis → FFT → mel filterbank → log → DCT), configuration tradeoffs (13/20/40 coefficients), delta/delta-delta MFCCs. Mel spectrogram for neural network input. Chroma features (12 pitch classes from STFT). Tonnetz (6D tonal centroid). Streaming architecture with ring buffer overlap management.

- **Key topics**: MFCCExtractor C++ class, ChromaExtractor, TonnetzExtractor, coefficient semantics, timbre classification
- **Related**: [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md), [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md), [LIB_essentia.md](LIB_essentia.md)

### [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md)
Complete rhythm analysis: 6 onset detection functions (spectral flux, HFC, complex domain, phase deviation, modified KL) with comparison table. BPM detection (autocorrelation, comb filter, Fourier of onset envelope). Beat tracking (Ellis DP, particle filter, DBN). Downbeat detection, meter estimation. Groove/swing quantification. Aubio API integration. Video-to-beat synchronization with beat prediction.

- **Key topics**: Onset detection functions, adaptive threshold peak picking, half/double tempo correction, beat phase, bar tracking, beat prediction for zero-latency sync
- **Related**: [FEATURES_transients_texture.md](FEATURES_transients_texture.md), [LIB_aubio.md](LIB_aubio.md), [LIB_essentia.md](LIB_essentia.md), [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md)

### [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md)
Pitch detection algorithms: YIN (full C++ implementation), pYIN, autocorrelation, HPS, MPM, CREPE — with accuracy/latency comparison. Key detection (Krumhansl-Schmuckler with C++ code). Chord recognition from chroma templates. Harmonic features: HCDF, inharmonicity, Tonnetz, HNR. Pitch-to-hue visual mapping.

- **Key topics**: YIN algorithm step-by-step, pitch confidence, polyphonic pitch detection, key profiles, chord templates, harmonic tension
- **Related**: [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md), [FEATURES_spectral.md](FEATURES_spectral.md), [LIB_aubio.md](LIB_aubio.md), [REF_math_reference.md](REF_math_reference.md)

### [FEATURES_transients_texture.md](FEATURES_transients_texture.md)
HPSS (harmonic-percussive separation) with real-time causal approximation. Transient analysis: attack time, decay, density, sharpness. ZCR with branchless C++ implementation. Roughness models (Sethares, Vassilakis). Fluctuation strength. Textural complexity metrics. Noise vs tonal content. Percussive element isolation (kick/snare/hat/crash band-limited detection).

- **Key topics**: Median filtering HPSS, Wiener masks, sensory dissonance, spectral peak roughness computation, drum detection
- **Related**: [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md), [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md)

### [FEATURES_structural.md](FEATURES_structural.md)
High-level structural analysis in real-time: novelty functions (spectral/chroma/MFCC-based), multi-scale energy envelopes (100ms to 16s), buildup detection (rising RMS + centroid + flux), drop detection (energy spike after buildup), breakdown detection. Phrase detection from beat tracker. Real-time self-similarity with 320KB ring buffer. Segment labeling (intro/verse/chorus/drop).

- **Key topics**: Novelty peak picking, multi-scale EMA, buildup/drop/breakdown state machines, self-similarity recurrence, 22μs processing budget
- **Related**: [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md), [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md), [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md), [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md)

### [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md)
Perceptually meaningful features: Zwicker/Moore-Glasberg perceived loudness, Aures sharpness, Fastl roughness, fluctuation strength, sensory pleasantness, tonalness, perceptual brightness/warmth, stereo width/spread/imbalance, binaural cues. Cross-modal correspondence research for visual mapping rationale. 10-dimensional visual state vector.

- **Key topics**: Zwicker loudness model, specific loudness, DIN 45692 sharpness, Daniel & Weber roughness, stereo correlation, ITD/ILD, Weber-Fechner law
- **Related**: [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md), [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md), [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md)

---

## Libraries — Deep Dives

### [LIB_essentia.md](LIB_essentia.md)
Essentia library complete reference: standard vs streaming mode architecture, dataflow graph construction, 30+ algorithms cataloged with parameters/CPU cost/VJ use cases across 6 categories. Performance tiers (light/medium/heavy). Cross-platform build instructions. Full EssentiaAnalyzer C++ class with triple-buffered lock-free bridge. Python prototyping workflow. Essentia vs Aubio comparison.

- **Key topics**: Streaming mode VectorInput, algorithm catalog, AGPL licensing, JUCE integration, background thread scheduling
- **Related**: [LIB_aubio.md](LIB_aubio.md), [LIB_juce.md](LIB_juce.md), [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md)

### [LIB_aubio.md](LIB_aubio.md)
Aubio library complete reference: C API patterns (new/do/get/del), all onset methods (9 types with comparison), tempo/beat tracking API, pitch detection (7 methods), 16 spectral descriptors, filterbank and MFCC. Building from source (waf). Audio callback integration with accumulator pattern. Genre-specific parameter tuning tables. Performance benchmarks. Aubio vs Essentia comparison.

- **Key topics**: aubio_onset, aubio_tempo, aubio_pitch, aubio_specdesc, GPLv3 license, parameter tuning by genre
- **Related**: [LIB_essentia.md](LIB_essentia.md), [LIB_juce.md](LIB_juce.md), [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md), [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md)

### [LIB_juce.md](LIB_juce.md)
JUCE framework reference: AudioDeviceManager, AudioIODeviceCallback, DSP module (FFT, filters, processors), AudioProcessorGraph for analysis chains, OpenGL integration (OpenGLContext + OpenGLRenderer with GLSL). CMake build patterns. Licensing tiers. Integration with Essentia and Aubio. Module dependency graph. Limitations and alternatives.

- **Key topics**: juce::dsp::FFT, ProcessorChain, OpenGLRenderer, three-thread architecture, CMake FetchContent
- **Related**: [LIB_essentia.md](LIB_essentia.md), [LIB_aubio.md](LIB_aubio.md), [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md), [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md)

### [LIB_fft_comparison.md](LIB_fft_comparison.md)
FFT library shootout: FFTW3 (wisdom, planning, SIMD), KissFFT (simple, BSD), PFFFT (SIMD-optimized), Kfr (C++17), Apple vDSP, Intel IPP/MKL, ne10 (ARM). Code examples for each. Benchmark table (256-4096 sizes). Recommendation matrix by use case. Windowing function implementations. Overlap-add and overlap-save streaming patterns.

- **Key topics**: FFTW wisdom system, PFFFT internal reordering, vDSP split complex, OLA/OLS streaming, cached windowing
- **Related**: [FEATURES_spectral.md](FEATURES_spectral.md), [LIB_essentia.md](LIB_essentia.md), [LIB_juce.md](LIB_juce.md), [REF_math_reference.md](REF_math_reference.md)

### [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md)
Audio I/O library comparison: miniaudio deep dive (single-header, loopback via WASAPI), RtAudio (ASIO support), PortAudio (battle-tested), libsoundio (clean API), JUCE AudioDeviceManager. Five comparison tables (features, backends, capabilities, maintenance, latency). Complete code examples for each. SPSC ring buffer integration pattern. 12-scenario decision matrix.

- **Key topics**: miniaudio callback/polling, WASAPI loopback, ASIO via RtAudio, hop-size accumulation, migration path strategy
- **Related**: [ARCH_audio_io.md](ARCH_audio_io.md), [ARCH_pipeline.md](ARCH_pipeline.md), [LIB_juce.md](LIB_juce.md), [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md)

### [LIB_rust_ecosystem.md](LIB_rust_ecosystem.md)
Rust audio ecosystem: cpal (I/O), rustfft (competitive with FFTW), dasp (DSP primitives), aubio-rs, pitch-detection, fundsp, spectrum-analyzer. Complete pipeline architecture in Rust with rtrb ring buffer and crossbeam channels. Real-time safety patterns (no-alloc, ArrayVec, thread priority via libc). Rust vs C++ decision matrix.

- **Key topics**: cpal backends, rustfft SIMD, Send/Sync for thread safety, criterion benchmarks, FFI overhead analysis
- **Related**: [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md), [LIB_fft_comparison.md](LIB_fft_comparison.md), [ARCH_pipeline.md](ARCH_pipeline.md), [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md)

---

## Video / Visual Engine Integration

### [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md)
The science of audio-to-visual mapping: comprehensive 13-feature → 25+ visual parameter mapping table. Smoothing strategies (EMA, One-Euro Filter, Critically Damped Spring) with C++ implementations. Normalization (adaptive min/max, percentile, histogram equalization). Mapping curves (log, sigmoid, bezier). Hysteresis. Beat-synced quantized triggers. MappingEngine class. Stevens' power law for perceptual mapping. Three practical presets.

- **Key topics**: One-Euro Filter, critically damped spring, dead zones, beat phase quantization, Weber-Fechner law, cross-modal exponents
- **Related**: [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md), [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md), [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md)

### [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md)
GPU integration for audio-reactive rendering: feature delivery via uniforms, UBOs (std140), textures (1D spectrum, 2D mel spectrogram), SSBOs. Triple-buffer thread synchronization. 5 complete GLSL shaders (spectrum color, beat-pulse geometry, radial bands, onset particles, post-processing bloom/chromatic aberration). JUCE OpenGL integration. Compute shaders. PBO async texture upload. Vulkan/Metal comparison.

- **Key topics**: std140 layout, atomic buffer swap, instanced rendering, FBO post-processing chain, adaptive quality system, 8,629 words
- **Related**: [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md), [VIDEO_vj_frameworks.md](VIDEO_vj_frameworks.md), [ARCH_pipeline.md](ARCH_pipeline.md), [LIB_juce.md](LIB_juce.md)

### [VIDEO_vj_frameworks.md](VIDEO_vj_frameworks.md)
Existing frameworks comparison: openFrameworks (ofxAudioAnalyzer/ofxAubio), Cinder (node-graph audio), Processing/p5.js, TouchDesigner (CHOPs), VVVV, Resolume (FFGL), Max/MSP+Jitter (FluCoMa). OSC protocol for inter-process feature transport with C++ sender code. Three comparison matrices. Build-vs-extend decision criteria. Hybrid architecture diagram.

- **Key topics**: ofxAudioAnalyzer, TouchDesigner CHOPs, FFGL 2.0, oscpack/liblo, shared memory transport, 8 frameworks compared
- **Related**: [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md), [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md), [LIB_essentia.md](LIB_essentia.md), [LIB_aubio.md](LIB_aubio.md)

---

## Implementation Guides

### [IMPL_project_setup.md](IMPL_project_setup.md)
Step-by-step project setup: source tree structure, complete CMakeLists.txt (JUCE + Essentia + Aubio + FFTW + OpenGL), dependency management (vcpkg manifest, Conan, FetchContent comparison), platform build instructions (Windows MSVC, macOS Xcode, Linux GCC), compiler flags for performance, IDE setup (CLion, VS Code, Visual Studio, Xcode), CMakePresets.json, GitHub Actions CI with 4-platform matrix.

- **Key topics**: FindEssentia.cmake, vcpkg triplets, -ffast-math caveats, sanitizer configs, Profile build type
- **Related**: [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md), [LIB_juce.md](LIB_juce.md), [LIB_essentia.md](LIB_essentia.md), [LIB_aubio.md](LIB_aubio.md)

### [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md)
Complete compilable "hello world" of audio analysis: miniaudio + KissFFT, ~380 lines of C++ (ring_buffer.h + main.cpp). Opens audio device, performs FFT, extracts 7 frequency bands + RMS + spectral centroid + onset detection, prints to console at 30fps. Full CMakeLists.txt and build commands for all platforms. Lock-free SPSC ring buffer implementation.

- **Key topics**: miniaudio callback, KissFFT real FFT, Hann windowing, spectral flux onset detection, band energy aggregation
- **Related**: [ARCH_pipeline.md](ARCH_pipeline.md), [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md), [LIB_fft_comparison.md](LIB_fft_comparison.md), [FEATURES_spectral.md](FEATURES_spectral.md)

### [IMPL_testing_validation.md](IMPL_testing_validation.md)
Testing and validation: test signal generators (sine, noise, clicks, chirps). Catch2 unit tests for FFT, RMS, centroid, onset, BPM. Offline/online validation. Reference datasets (GTZAN, RWC, MIREX). Latency measurement techniques. Profiling (Instruments, perf, VTune). Dropout detection. Stress testing at minimum buffer sizes. Accuracy metrics (P-Score, F-measure, MIREX scoring). GitHub Actions CI pipeline.

- **Key topics**: AnalysisProfiler class, xrun detection callbacks, CPU load simulation, performance regression testing
- **Related**: [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md), [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md), [REF_latency_numbers.md](REF_latency_numbers.md), [LIB_aubio.md](LIB_aubio.md)

### [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md)
Runtime calibration for live performance: adaptive RMS normalization (Lemire's algorithm), auto-gain, onset threshold auto-calibration (median + MAD). BPM management (tap-tempo, BPM lock, hybrid auto/manual). Feature normalization engine with 4 curve types. Genre detection with parameter auto-switching. Silence detection. JSON preset system. OSC/MIDI runtime parameter tuning.

- **Key topics**: Percentile normalization, BPM half/double correction, genre crossfader, AtomicParamSwap, MIDI CC mapping
- **Related**: [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md), [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md), [REF_genre_parameter_presets.md](REF_genre_parameter_presets.md)

---

## Reference

### [REF_latency_numbers.md](REF_latency_numbers.md)
Definitive latency reference: 10 audio APIs with typical/best/worst latency tables. Algorithm latency for 15+ analysis methods. Stage-by-stage pipeline budget (40ms achievable). Human perception thresholds with research citations. Measurement techniques (hardware loopback, software timestamps). Buffer size tuning with adaptive sizing C++ class. Latency hiding (beat prediction, feature interpolation). Display latency (LCD/OLED/VRR, 18.5ms achievable at 120Hz).

- **Key topics**: ASIO/CoreAudio/WASAPI/JACK latency data, audio-visual sync perception (±20-80ms), beat prediction extrapolation
- **Related**: [ARCH_pipeline.md](ARCH_pipeline.md), [ARCH_audio_io.md](ARCH_audio_io.md), [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md), [IMPL_testing_validation.md](IMPL_testing_validation.md)

### [REF_math_reference.md](REF_math_reference.md)
Every formula needed for implementation: DFT/FFT derivation, 7 window functions with spectral properties table, Mel/Bark/ERB scale formulas with conversion tables, MFCC derivation, autocorrelation + Wiener-Khinchin theorem, YIN algorithm math with C++ code, onset detection function formulas, all spectral feature formulas, biquad filter design (7 types), bilinear transform, A/K-weighting coefficients, complex number operations, convolution theorem, Parseval's theorem.

- **Key topics**: Bin frequency calculation, frequency resolution, uncertainty principle, DCT-II, CMND, biquad coefficient formulas, phase unwrapping
- **Related**: [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md), [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md), [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md), [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md)

### [REF_genre_parameter_presets.md](REF_genre_parameter_presets.md)
Algorithm parameters tuned for 8 genres: Techno, House, DnB, Ambient, Hip Hop, Rock/Metal, Classical, Pop. Each with detection strategies, genre-specific algorithms (sidechain detection, 808 bass, Reese bass, vocal formants, swing ratio). 7 consolidated parameter tables. Real-time k-NN genre classifier C++ code. Auto-switch system with crossfade interpolation and hysteresis.

- **Key topics**: BPM range constraints, onset method per genre, band energy weighting, envelope attack/release, genre feature profiles
- **Related**: [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md), [FEATURES_spectral.md](FEATURES_spectral.md), [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md), [LIB_aubio.md](LIB_aubio.md)

### [REF_resources_links.md](REF_resources_links.md)
Curated, annotated resource list: 33 academic papers (onset detection, beat tracking, pitch, spectral analysis, psychoacoustics, deep learning), 12 books, 25+ GitHub repositories, courses (Coursera, CCRMA), communities (music-dsp, KVR, JUCE forum, ISMIR), 10 datasets for validation, online tools (Sonic Visualiser, Friture), standards (ITU-R BS.1770, EBU R128, ISO 226/532). Cross-reference index mapping resources to research documents.

- **Key topics**: Dixon, Bello, de Cheveigné, Ellis, Müller, Zwicker, Fitzgerald — all key researchers and their contributions
- **Related**: All documents in this library

---

## Document Dependency Graph

```
                    REF_math_reference
                    /    |    |    \
                   /     |    |     \
    FEATURES_spectral  FEATURES_frequency_bands  FEATURES_mfcc_mel  FEATURES_pitch_harmonic
         |    \            |                         |                    |
         |     \           |                         |                    |
    FEATURES_amplitude  FEATURES_rhythm_tempo  FEATURES_transients  FEATURES_structural
         |                 |         |               |
         \                 |         |              /
          \                |         |             /
           FEATURES_psychoacoustic   |            /
                    \                |           /
                     \               |          /
              VIDEO_feature_to_visual_mapping
                          |
                   VIDEO_opengl_integration
                          |
                   VIDEO_vj_frameworks

    ARCH_pipeline ←→ ARCH_audio_io ←→ ARCH_realtime_constraints
         |                |                    |
         |         LIB_rtaudio_miniaudio       |
         |                |                    |
    LIB_juce ←→ LIB_essentia ←→ LIB_aubio     |
         |                                     |
    LIB_fft_comparison    LIB_rust_ecosystem   |
                                               |
    IMPL_project_setup → IMPL_minimal_prototype
                              |
                    IMPL_testing_validation
                              |
                  IMPL_calibration_adaptation ←→ REF_genre_parameter_presets
                                                        |
                                              REF_latency_numbers
                                                        |
                                              REF_resources_links
```

---

*Generated 2026-03-13 | 29 documents | Cross-platform C++ focus | Production-grade technical depth*
