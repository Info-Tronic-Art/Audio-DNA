# LIB_rust_ecosystem.md — Rust Audio Analysis Ecosystem

> **Scope**: Comprehensive survey of the Rust crate ecosystem for real-time audio analysis
> targeting music visualization and VJ applications.
>
> **Cross-references**: [ARCH_pipeline.md](ARCH_pipeline.md), ARCH_realtime_constraints.md,
> LIB_rtaudio_miniaudio.md, LIB_fft_comparison.md, IMPL_minimal_prototype.md

---

## 1. Why Rust for Real-Time Audio

### 1.1 Memory Safety Without a Garbage Collector

Real-time audio callbacks operate under a hard latency ceiling — typically 1–10 ms at
common buffer sizes (64–512 samples at 44.1/48 kHz). Garbage-collected languages (Java,
C#, Go, JavaScript) introduce unbounded pauses when the collector runs, producing audible
glitches. C and C++ avoid GC but trade safety: use-after-free, double-free, and buffer
overflows are classes of bug that routinely ship in production audio software.

Rust eliminates these categories of defect at compile time through the ownership and
borrowing system. Every value has exactly one owner; references are checked at compile
time for aliasing and lifetime violations. The result is C-equivalent machine code with
memory-safety guarantees that would otherwise require a runtime.

### 1.2 No Data Races at Compile Time

Audio pipelines are inherently multi-threaded: an audio callback on a high-priority
thread must exchange data with an analysis thread and a render/UI thread. In C++, this
is defended by code review, careful use of `std::atomic`, and hope. Rust's type system
encodes thread-safety as traits:

- `Send` — a type can be transferred to another thread.
- `Sync` — a type can be *shared* (via `&T`) across threads.

The compiler rejects programs that share non-`Sync` data or move non-`Send` values
across thread boundaries. This converts an entire class of runtime data-race bugs into
compile errors.

### 1.3 Zero-Cost Abstractions

Rust's generics are monomorphized: a function generic over `T: Sample` compiles to
specialized machine code for `f32`, `i16`, etc., with no vtable dispatch. Iterator
chains — `buffer.iter().map(...).zip(...).for_each(...)` — compile down to tight loops
identical to hand-written index-based C. This means high-level DSP code with strong type
safety produces the same assembly as unsafe pointer arithmetic.

### 1.4 Predictable Performance

No hidden allocations behind `operator+`. No implicit copies. No virtual dispatch unless
you opt into `dyn Trait`. Move semantics are the default and are zero-cost (a memcpy of
the struct, then forgetting the source). The developer has complete control over when heap
allocation occurs, which is critical for real-time paths.

### 1.5 Challenges

**Avoiding allocation in the audio callback.** Rust's standard library allocates freely —
`Vec::push` may reallocate, `String` is heap-backed, `Box::new` allocates. In a real-time
audio callback, *any* allocation is a potential priority inversion via the global allocator's
mutex. The discipline required is the same as in C++: pre-allocate all buffers before the
stream starts, use fixed-size stack arrays or `ArrayVec`/`tinyvec` in the hot path, and
never call `format!`, `println!`, or anything that touches the allocator.

**`unsafe` for FFI.** Interfacing with C audio backends (CoreAudio, ALSA, WASAPI) requires
`unsafe` blocks. Crates like `cpal` encapsulate this, but if you write your own backend
bindings or call into C DSP libraries (FFTW, libaubio), you accept responsibility for
correctness at the FFI boundary. The `unsafe` surface is typically small and auditable.

**Ecosystem maturity.** The Rust audio ecosystem is younger than the C/C++ ecosystem.
Some areas (VST plugin hosting, advanced resampling, comprehensive codec support) have
fewer battle-tested options. However, for the analysis-and-visualization pipeline we are
building, coverage is strong.

### 1.6 Comparison with C++

| Dimension                     | C++                              | Rust                                    |
|-------------------------------|----------------------------------|-----------------------------------------|
| Memory safety                 | Manual / sanitizers              | Compile-time ownership                  |
| Data-race prevention          | Code review + TSan               | `Send`/`Sync` type system               |
| Build system                  | CMake / Meson / Premake (fragmented) | Cargo (unified)                     |
| Dependency management         | vcpkg / Conan / manual           | Cargo + crates.io                       |
| FFI to C                      | Native                           | `extern "C"` + `unsafe`                 |
| Compile times                 | Moderate–slow                    | Slow (improving)                        |
| Runtime performance           | Equivalent                       | Equivalent (same LLVM backend)          |
| Real-time allocation control  | `operator new` overloads, arenas | `#[global_allocator]`, `no_std`, arenas |
| Library ecosystem for audio   | Mature (JUCE, PortAudio, etc.)   | Growing (cpal, fundsp, etc.)            |
| IDE support                   | Excellent (CLion, VS)            | Good (rust-analyzer, CLion plugin)      |

For a new visualization/analysis project where we control the full stack, Rust offers a
compelling trade-off: slower initial development (borrow checker learning curve) in
exchange for dramatically fewer runtime bugs and a unified build/dependency story.

---

## 2. cpal — Cross-Platform Audio Library

**Crate**: `cpal` v0.17.3 | **License**: Apache-2.0
**Repository**: <https://github.com/RustAudio/cpal>

### 2.1 Overview

`cpal` is the de facto standard for audio I/O in Rust. It provides a callback-based
streaming API across all major desktop and mobile platforms:

| Platform       | Backend(s)                    |
|----------------|-------------------------------|
| Windows        | WASAPI, ASIO                  |
| macOS / iOS    | CoreAudio                     |
| Linux          | ALSA, JACK                    |
| Android        | NDK (Oboe under the hood)     |
| Web            | WebAudio (via wasm-bindgen)   |

### 2.2 Core Abstractions

- **`Host`** — Entry point. `cpal::default_host()` returns the platform default. Provides
  device enumeration.
- **`Device`** — Represents a physical or virtual audio device. Supports querying
  supported configurations (`supported_input_configs()`, `supported_output_configs()`).
- **`StreamConfig`** — Sample rate, channel count, buffer size.
- **`Stream`** — An active audio stream. Created via `device.build_input_stream()` or
  `device.build_output_stream()`. The stream owns a high-priority audio thread internally.
- **`Sample`**, **`SizedSample`**, **`FromSample`** — Traits for generic sample handling
  (`f32`, `i16`, `u16`).

### 2.3 Code Example: Capturing Audio Input

```rust
use cpal::traits::{DeviceTrait, HostTrait, StreamTrait};
use std::sync::{Arc, Mutex};

fn main() {
    let host = cpal::default_host();
    let device = host.default_input_device()
        .expect("No input device available");

    let config = device.default_input_config()
        .expect("No default input config");

    println!("Input config: {:?}", config);

    let sample_rate = config.sample_rate().0;
    let channels = config.channels() as usize;

    // Shared ring buffer (in production, use a lock-free ring buffer)
    let buffer: Arc<Mutex<Vec<f32>>> = Arc::new(Mutex::new(
        Vec::with_capacity(sample_rate as usize * channels)
    ));
    let buffer_clone = buffer.clone();

    let stream = device.build_input_stream(
        &config.into(),
        move |data: &[f32], _info: &cpal::InputCallbackInfo| {
            // WARNING: Mutex::lock can block — use lock-free in production
            if let Ok(mut buf) = buffer_clone.try_lock() {
                buf.extend_from_slice(data);
            }
        },
        |err| eprintln!("Stream error: {}", err),
        None, // No timeout
    ).expect("Failed to build input stream");

    stream.play().expect("Failed to start stream");

    // Stream runs until dropped
    std::thread::sleep(std::time::Duration::from_secs(5));
}
```

### 2.4 Limitations

- **No universal loopback capture.** On macOS, capturing system audio output (loopback)
  requires a virtual audio device (e.g., BlackHole, Loopback by Rogue Amoeba). WASAPI on
  Windows supports loopback natively, but `cpal` does not expose it through a unified API.
- **Buffer size control** is limited on some backends. CoreAudio allows setting the
  hardware buffer size; ALSA's period/buffer negotiation is partially exposed.
- **No MIDI.** `cpal` is audio-only. Use `midir` for MIDI I/O.
- **Callback-only model.** There is no blocking/pull-based read API. You must process
  audio in the callback or copy it out to another thread.

> **See also**: [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md) for comparison with
> RtAudio and miniaudio C libraries.

---

## 3. rustfft — Pure Rust FFT

**Crate**: `rustfft` v6.4.1 | **License**: MIT / Apache-2.0
**Repository**: <https://github.com/ejmahler/RustFFT>

### 3.1 Overview

`rustfft` is a pure-Rust FFT implementation that is competitive with FFTW for many
transform sizes. It requires no C compiler, no system libraries, and no build-time
configuration. It ships optimized codepaths for:

| Instruction Set | Platform        | Activation                          |
|-----------------|-----------------|-------------------------------------|
| AVX             | x86_64          | Auto-detected by `FftPlanner`       |
| SSE4.1          | x86 / x86_64   | Auto-detected by `FftPlanner`       |
| Neon            | AArch64 (Apple Silicon, ARM) | Auto-detected by `FftPlanner` |
| WASM SIMD       | WebAssembly     | `FftPlannerWasmSimd`                |

The `FftPlanner` selects the best algorithm (mixed-radix, Rader's, Bluestein's, etc.)
and SIMD strategy for a given size automatically.

### 3.2 API

```rust
use rustfft::{FftPlanner, num_complex::Complex};

fn compute_spectrum(samples: &[f32]) -> Vec<f32> {
    let len = samples.len();
    let mut planner = FftPlanner::<f32>::new();
    let fft = planner.plan_fft_forward(len);

    // Convert real samples to complex (imaginary = 0)
    let mut buffer: Vec<Complex<f32>> = samples.iter()
        .map(|&s| Complex { re: s, im: 0.0 })
        .collect();

    // Scratch buffer (required for some algorithms)
    let mut scratch = vec![Complex { re: 0.0, im: 0.0 }; fft.get_inplace_scratch_len()];

    fft.process_with_scratch(&mut buffer, &mut scratch);

    // Compute magnitude spectrum (first half only — real input is symmetric)
    buffer[..len / 2]
        .iter()
        .map(|c| (c.re * c.re + c.im * c.im).sqrt() / len as f32)
        .collect()
}
```

### 3.3 Performance Notes

- For power-of-two sizes (512, 1024, 2048, 4096), `rustfft` is within 5–20% of FFTW
  on x86_64 with AVX. On Apple Silicon (Neon), it is often within 10%.
- For non-power-of-two sizes, the gap can widen — FFTW's codelet generator has decades
  of optimization for arbitrary sizes.
- **Pre-plan and reuse**: `FftPlanner::plan_fft_forward()` returns an `Arc<dyn Fft<f32>>`
  that can be cloned cheaply and reused across calls. The planner itself caches plans.
- **Scratch buffers**: Pre-allocate and reuse to avoid per-frame allocation.

### 3.4 Real-to-Complex

`rustfft` operates on complex buffers. For real-valued audio input, you zero the
imaginary part. The `realfft` crate (by the same author) provides an optimized
real-to-complex path that processes only N/2+1 complex bins, halving memory and nearly
halving compute.

```toml
# Cargo.toml
[dependencies]
rustfft = "6.4"
realfft = "3.4"
```

> **See also**: [LIB_fft_comparison.md](LIB_fft_comparison.md) for benchmarks against
> FFTW, KissFFT, and pffft.

---

## 4. dasp — Digital Audio Signal Processing

**Crate**: `dasp` v0.11.0 | **License**: MIT / Apache-2.0
**Repository**: <https://github.com/RustAudio/dasp>

### 4.1 Overview

`dasp` provides foundational primitives for audio DSP in Rust. It is not an effects
library — it is a *type system* for audio data, comparable to what Eigen is for linear
algebra.

### 4.2 Key Modules

| Module         | Purpose                                                        |
|----------------|----------------------------------------------------------------|
| `sample`       | `Sample` trait: generic over `f32`, `f64`, `i16`, `i24`, `u16` |
| `frame`        | `Frame` trait: mono, stereo, N-channel at a discrete instant   |
| `signal`       | Iterator-based signal generators (sine, noise, envelope)       |
| `slice`        | Utilities for `&[S]` and `&[Frame]` processing                |
| `interpolate`  | Sample-rate conversion: linear, sinc, floor/ceil               |
| `ring_buffer`  | Fixed-capacity and bounded ring buffers for delay/overlap      |
| `envelope`     | Envelope detection (peak, RMS)                                 |
| `window`       | Windowing functions (Hanning, Hamming, etc.)                   |

### 4.3 Sample Type Abstractions

```rust
use dasp::{Sample, Frame};
use dasp::frame::Stereo;

fn peak_amplitude<S: Sample>(frames: &[Stereo<S>]) -> f64 {
    frames.iter()
        .flat_map(|f| f.channels())
        .map(|s| s.to_sample::<f64>().abs())
        .fold(0.0f64, f64::max)
}
```

The `Sample` trait handles conversion between formats with proper scaling (e.g., `i16`
range `[-32768, 32767]` maps to `f32` range `[-1.0, 1.0]`).

### 4.4 Ring Buffer

`dasp::ring_buffer` provides `Fixed` and `Bounded` ring buffers that are useful for:

- Overlap-add FFT processing (store previous frames)
- Delay lines for comb/allpass filters
- Circular write buffers for the audio callback to fill and the analysis thread to read

```rust
use dasp::ring_buffer::Fixed;

let mut ring = Fixed::from([0.0f32; 2048]);
for sample in incoming_samples {
    ring.push(sample);
}
// ring now contains the most recent 2048 samples
let slice = ring.slices(); // returns (&[f32], &[f32]) — may wrap
```

### 4.5 Relevance to Our Pipeline

`dasp` is most useful as a utility layer: sample format conversion when interfacing with
`cpal` (which may deliver `i16` or `f32`), ring buffers for overlap management, and
envelope detection for simple amplitude-based features. It does *not* provide FFT,
spectral analysis, or onset/pitch detection.

---

## 5. aubio-rs — Rust Bindings to libaubio

**Crate**: `aubio-rs` v0.2.0 | **License**: GPL-3.0
**Repository**: <https://github.com/katyo/aubio-rs>

### 5.1 Overview

`aubio-rs` wraps the C library `libaubio`, providing access to battle-tested MIR
(Music Information Retrieval) algorithms:

- **Onset detection** — energy, HFC, complex, phase, spectral flux, KL divergence
- **Tempo / beat tracking** — autocorrelation-based BPM estimation and beat position
- **Pitch detection** — YIN, McLeod (YINFFT), Schmitt trigger, specacf
- **MFCC** — Mel-frequency cepstral coefficients
- **Spectral analysis** — phase vocoder, FFT, filterbank

### 5.2 Build Requirements

`aubio-rs` depends on `aubio-sys`, which can either:

1. **Link to a system-installed libaubio** via `pkg-config` (feature `pkg-config`).
2. **Build libaubio from source** (feature `builtin`) — requires a C compiler and CMake.
3. **Generate fresh bindings** (feature `bindgen`) — needed for non-standard architectures.

```toml
[dependencies]
aubio-rs = { version = "0.2", features = ["builtin"] }
```

### 5.3 License Warning

`libaubio` is GPL-3.0. This is viral — any binary linking `aubio-rs` must be distributed
under GPL-3.0. For a closed-source VJ application, this may be unacceptable. Consider
`pitch-detection` (MIT) + custom onset detection as a pure-Rust alternative.

### 5.4 Usage Example: Onset Detection

```rust
use aubio_rs::Onset;

fn detect_onsets(audio: &[f32], sample_rate: u32) -> Vec<usize> {
    let hop_size = 512;
    let buf_size = 1024;
    let mut onset = Onset::new(
        aubio_rs::OnsetMode::SpecFlux,
        buf_size,
        hop_size,
        sample_rate,
    ).expect("Failed to create onset detector");

    let mut onsets = Vec::new();
    for (i, chunk) in audio.chunks(hop_size).enumerate() {
        if chunk.len() == hop_size {
            let is_onset = onset.do_result(chunk)
                .expect("onset detection failed");
            if is_onset > 0.0 {
                onsets.push(i * hop_size);
            }
        }
    }
    onsets
}
```

---

## 6. pitch-detection — Pure Rust Pitch Detection

**Crate**: `pitch-detection` v0.3.0 | **License**: MIT
**Repository**: <https://github.com/alesgenova/pitch-detection>

### 6.1 Algorithms

| Algorithm              | Method                                   | Speed    | Accuracy |
|------------------------|------------------------------------------|----------|----------|
| `McLeodDetector`       | Normalized square difference + parabolic | Fast     | High     |
| `AutocorrelationDetector` | Classic autocorrelation               | Moderate | Moderate |
| `YINDetector`          | YIN algorithm (de Cheveigné, 2002)       | Moderate | High     |

### 6.2 API

```rust
use pitch_detection::detector::mcleod::McLeodDetector;
use pitch_detection::detector::PitchDetector;

const SIZE: usize = 1024;
const PADDING: usize = SIZE / 2;
const SAMPLE_RATE: usize = 44100;

fn detect_pitch(signal: &[f32]) -> Option<(f32, f32)> {
    let mut detector = McLeodDetector::<f32>::new(SIZE, PADDING);

    let pitch = detector.get_pitch(
        signal,
        SAMPLE_RATE,
        0.1,  // power_threshold: minimum signal power to attempt detection
        0.8,  // clarity_threshold: minimum confidence (0.0–1.0)
    )?;

    Some((pitch.frequency, pitch.clarity))
}
```

### 6.3 Relevance

The McLeod Pitch Method (MPM) is well-suited for real-time monophonic pitch tracking.
It is significantly faster than YIN for equivalent accuracy on clean signals. For
polyphonic content (full mixes), pitch detection is not meaningful — use spectral
centroid or chroma features instead.

Being pure Rust and MIT-licensed, this crate avoids the GPL and C build complications
of `aubio-rs`.

---

## 7. fundsp — Functional DSP Library

**Crate**: `fundsp` v0.23.0 | **License**: MIT / Apache-2.0
**Repository**: <https://github.com/SamiPerttu/fundsp>

### 7.1 Overview

`fundsp` is an expressive DSP library built on Rust's type system. It uses custom
operators to compose audio graphs inline:

| Operator | Meaning                             |
|----------|-------------------------------------|
| `>>`     | Pipe: serial connection             |
| `\|`     | Stack: parallel, independent        |
| `&`      | Bus: sum parallel outputs           |
| `^`      | Branch: split input to parallel     |
| `!`      | Thru: pass input + add side output  |

### 7.2 Signal Processing Components

- **Oscillators**: sine, saw, square, triangle, wavetable, PolyBLEP anti-aliased
- **Filters**: biquad (lowpass, highpass, bandpass, notch, allpass, shelving), Moog
  ladder, state-variable, one-pole
- **Delays**: tapped delay lines, feedback delay networks, allpass delay
- **Effects**: chorus, flanger, phaser, reverb (32-channel FDN)
- **Analysis**: FFT convolution, frequency response computation

### 7.3 Code Example: Bandpass Filter Chain

```rust
use fundsp::prelude::*;

// 3-band bandpass filter splitting input into sub, mid, high
fn three_band_split() -> impl AudioUnit {
    // Sub: 20–200 Hz
    // Mid: 200–4000 Hz
    // High: 4000–20000 Hz
    (lowpass_hz(200.0, 1.0) | bandpass_hz(2000.0, 1.0) | highpass_hz(4000.0, 1.0))
}
```

### 7.4 Relevance to Visualization

`fundsp` is most useful if you need to *process* audio (filtering, envelope following)
before analysis, or if you want to build audio-reactive synthesis for sonification
feedback. For pure analysis (FFT, onset, pitch), it is heavier than needed. However, its
filter design functions are excellent for building analysis filter banks (e.g., octave-band
splitting before per-band envelope extraction).

### 7.5 Real-Time Safety

`fundsp` is designed for real-time use. Its `Net` type supports lock-free graph
modification at runtime (adding/removing nodes while audio is playing). The `Sequencer`
component allows scheduling events without allocation in the audio thread.

---

## 8. spectrum-analyzer — FFT-Based Spectrum Analysis

**Crate**: `spectrum-analyzer` v1.7.0 | **License**: MIT
**Repository**: <https://github.com/phip1611/spectrum-analyzer>

### 8.1 Overview

`spectrum-analyzer` is a high-level convenience crate that wraps FFT computation with
windowing, magnitude scaling, and frequency-domain output. It is designed for exactly
the use case we need: taking a buffer of time-domain audio samples and producing a
frequency spectrum suitable for visualization.

### 8.2 API

```rust
use spectrum_analyzer::{
    samples_fft_to_spectrum,
    FrequencyLimit,
    FrequencySpectrum,
};
use spectrum_analyzer::windows::hann_window;
use spectrum_analyzer::scaling::divide_by_N_sqrt;

fn analyze_spectrum(samples: &[f32], sample_rate: u32) -> FrequencySpectrum {
    // Apply Hann window
    let windowed = hann_window(samples);

    // Compute spectrum with frequency limits
    samples_fft_to_spectrum(
        &windowed,
        sample_rate,
        FrequencyLimit::Range(20.0, 20000.0),
        Some(&divide_by_N_sqrt),
    ).expect("FFT failed")
}

// Usage:
// spectrum.data() -> &[(Frequency, FrequencyValue)]
// spectrum.max() -> (Frequency, FrequencyValue)
// spectrum.min() -> (Frequency, FrequencyValue)
// spectrum.average() -> FrequencyValue
```

### 8.3 Windowing Functions

The `windows` module provides:

- Hann (Hanning)
- Hamming
- Blackman-Harris
- Rectangular (no window)

### 8.4 Limitations

- Allocates internally on each call (the `hann_window` function returns a `Vec<f32>`).
  For real-time use, you would want to pre-allocate the windowed buffer and apply the
  window coefficients manually, then call `rustfft` directly.
- No `no_std` support beyond `alloc`.
- No overlap-add management — it processes one buffer at a time.

### 8.5 When to Use

`spectrum-analyzer` is excellent for prototyping and for applications where allocation
per frame is acceptable (render-loop visualization at 30–60 fps, where the analysis runs
on a dedicated thread with a 10+ ms budget). For the tightest real-time paths, use
`rustfft` directly with pre-allocated buffers.

---

## 9. Building a Complete Pipeline in Rust

### 9.1 Architecture Overview

```
┌─────────────┐     lock-free      ┌──────────────────┐     crossbeam      ┌──────────────┐
│ cpal audio   │──── ring buffer ──▶│ Analysis thread   │──── channel ──────▶│ Render thread │
│ callback     │                    │ (rustfft + feat.) │                    │ (wgpu / egui) │
│ (real-time)  │                    │ (near-real-time)  │                    │ (best-effort) │
└─────────────┘                    └──────────────────┘                    └──────────────┘
     ▲                                    │
     │                                    ▼
  OS audio                         Feature bus (atomics)
  thread                           - RMS, peak
  priority                         - spectrum [f32; N]
                                   - onset flag
                                   - pitch estimate
                                   - spectral centroid
                                   - beat phase
```

### 9.2 Lock-Free Communication

The audio callback runs on an OS-managed high-priority thread. It must *never* block.
Communication patterns:

| Mechanism                        | Use Case                              | Crate               |
|----------------------------------|---------------------------------------|----------------------|
| Lock-free SPSC ring buffer       | Raw audio from callback to analysis   | `ringbuf`, `rtrb`    |
| `crossbeam::channel::bounded`    | Feature structs from analysis to render | `crossbeam-channel` |
| `AtomicF32` / `AtomicU64`        | Single scalar features (RMS, BPM)     | `std::sync::atomic`  |
| Triple buffer                    | Spectrum array (writer never blocks)  | `triple_buffer`      |

### 9.3 Code Architecture

```rust
// ═══════════════════════════════════════════════════════════════
// main.rs — Pipeline skeleton
// ═══════════════════════════════════════════════════════════════

use cpal::traits::{DeviceTrait, HostTrait, StreamTrait};
use crossbeam_channel::{bounded, Receiver, Sender};
use rtrb::RingBuffer;
use rustfft::{FftPlanner, num_complex::Complex};
use std::sync::Arc;

/// Audio features extracted per analysis frame
#[derive(Clone, Debug)]
struct AudioFeatures {
    rms: f32,
    peak: f32,
    spectrum_magnitude: Vec<f32>,  // N/2 bins
    spectral_centroid: f32,
    onset: bool,
    pitch_hz: Option<f32>,
}

const FFT_SIZE: usize = 2048;
const HOP_SIZE: usize = 512;
const SAMPLE_RATE: u32 = 44100;

fn main() {
    // --- Audio I/O (cpal) ---
    let host = cpal::default_host();
    let device = host.default_input_device().expect("no input device");
    let config = cpal::StreamConfig {
        channels: 1,
        sample_rate: cpal::SampleRate(SAMPLE_RATE),
        buffer_size: cpal::BufferSize::Fixed(HOP_SIZE as u32),
    };

    // Lock-free SPSC ring buffer: audio callback -> analysis thread
    let (mut producer, mut consumer) = RingBuffer::<f32>::new(FFT_SIZE * 4);

    // Feature channel: analysis thread -> render thread
    let (feat_tx, feat_rx): (Sender<AudioFeatures>, Receiver<AudioFeatures>) =
        bounded(4); // small bound — render drops stale frames

    // --- Audio callback (real-time safe) ---
    let stream = device.build_input_stream(
        &config,
        move |data: &[f32], _: &cpal::InputCallbackInfo| {
            // Write samples to ring buffer — never blocks, drops if full
            let written = producer.write_chunk_uninit(data.len())
                .ok()
                .map(|mut chunk| {
                    chunk.fill_from_iter(data.iter().copied());
                    chunk.commit_all();
                });
            let _ = written; // Ignore if ring buffer is full
        },
        |err| eprintln!("Audio error: {err}"),
        None,
    ).expect("Failed to build stream");

    stream.play().expect("Failed to start stream");

    // --- Analysis thread ---
    let analysis_handle = std::thread::Builder::new()
        .name("audio-analysis".into())
        .spawn(move || {
            let mut planner = FftPlanner::<f32>::new();
            let fft = planner.plan_fft_forward(FFT_SIZE);
            let mut scratch = vec![Complex::new(0.0, 0.0); fft.get_inplace_scratch_len()];

            let mut frame_buffer = vec![0.0f32; FFT_SIZE];
            let mut complex_buf = vec![Complex::new(0.0f32, 0.0); FFT_SIZE];
            let mut window = vec![0.0f32; FFT_SIZE];

            // Pre-compute Hann window
            for (i, w) in window.iter_mut().enumerate() {
                *w = 0.5 * (1.0 - (2.0 * std::f32::consts::PI * i as f32
                    / (FFT_SIZE - 1) as f32).cos());
            }

            // Onset detection state
            let mut prev_spectral_flux = 0.0f32;
            let onset_threshold = 1.5;

            loop {
                // Read HOP_SIZE samples (blocking wait via spin — in production,
                // use a condvar or crossbeam select)
                if consumer.slots() < HOP_SIZE {
                    std::thread::sleep(std::time::Duration::from_micros(500));
                    continue;
                }

                // Shift buffer left by HOP_SIZE, read new samples into tail
                frame_buffer.copy_within(HOP_SIZE.., 0);
                let chunk = consumer.read_chunk(HOP_SIZE).unwrap();
                let (a, b) = chunk.as_slices();
                let tail = &mut frame_buffer[FFT_SIZE - HOP_SIZE..];
                tail[..a.len()].copy_from_slice(a);
                if !b.is_empty() {
                    tail[a.len()..a.len() + b.len()].copy_from_slice(b);
                }
                chunk.commit_all();

                // RMS & peak
                let rms = (frame_buffer.iter()
                    .map(|s| s * s).sum::<f32>() / FFT_SIZE as f32).sqrt();
                let peak = frame_buffer.iter()
                    .map(|s| s.abs()).fold(0.0f32, f32::max);

                // Apply window and convert to complex
                for (i, c) in complex_buf.iter_mut().enumerate() {
                    c.re = frame_buffer[i] * window[i];
                    c.im = 0.0;
                }

                // FFT (in-place, no allocation)
                fft.process_with_scratch(&mut complex_buf, &mut scratch);

                // Magnitude spectrum (first half)
                let n_bins = FFT_SIZE / 2;
                let mut spectrum = vec![0.0f32; n_bins]; // pre-alloc in production
                let mut spectral_flux = 0.0f32;
                let mut centroid_num = 0.0f32;
                let mut centroid_den = 0.0f32;

                for (i, bin) in complex_buf[..n_bins].iter().enumerate() {
                    let mag = (bin.re * bin.re + bin.im * bin.im).sqrt()
                        / FFT_SIZE as f32;
                    spectrum[i] = mag;

                    let freq = i as f32 * SAMPLE_RATE as f32 / FFT_SIZE as f32;
                    centroid_num += freq * mag;
                    centroid_den += mag;
                    spectral_flux += (mag - 0.0).max(0.0); // simplified
                }

                let spectral_centroid = if centroid_den > 1e-10 {
                    centroid_num / centroid_den
                } else { 0.0 };

                let onset = spectral_flux > prev_spectral_flux * onset_threshold;
                prev_spectral_flux = spectral_flux;

                let features = AudioFeatures {
                    rms,
                    peak,
                    spectrum_magnitude: spectrum,
                    spectral_centroid,
                    onset,
                    pitch_hz: None, // add pitch-detection crate here
                };

                // Non-blocking send — drops if render thread is behind
                let _ = feat_tx.try_send(features);
            }
        })
        .expect("Failed to spawn analysis thread");

    // --- Render loop (main thread or GPU thread) ---
    loop {
        match feat_rx.try_recv() {
            Ok(features) => {
                // Drive visualization with features
                // e.g., features.rms -> brightness
                //        features.spectrum_magnitude -> bar graph
                //        features.onset -> trigger flash
                //        features.spectral_centroid -> color hue
            }
            Err(_) => {
                // No new features — render with previous state
            }
        }
        std::thread::sleep(std::time::Duration::from_millis(16)); // ~60 fps
    }
}
```

### 9.4 Cargo.toml

```toml
[package]
name = "audio-viz-pipeline"
version = "0.1.0"
edition = "2021"

[dependencies]
cpal = "0.17"
rustfft = "6.4"
realfft = "3.4"
crossbeam-channel = "0.5"
rtrb = "0.3"
pitch-detection = "0.3"
dasp = { version = "0.11", features = ["sample", "frame", "ring_buffer", "envelope"] }

# Optional — GPL-3.0
# aubio-rs = { version = "0.2", features = ["builtin"] }

# Optional — for advanced DSP / filtering
# fundsp = "0.20"
# spectrum-analyzer = "1.7"

[profile.release]
opt-level = 3
lto = "thin"
codegen-units = 1
```

---

## 10. Real-Time Safety in Rust

### 10.1 The Real-Time Contract

The audio callback must complete within the buffer duration:

| Buffer Size | Sample Rate | Deadline     |
|-------------|-------------|--------------|
| 64 samples  | 48000 Hz    | 1.33 ms      |
| 128 samples | 48000 Hz    | 2.67 ms      |
| 256 samples | 44100 Hz    | 5.80 ms      |
| 512 samples | 44100 Hz    | 11.61 ms     |
| 1024 samples| 44100 Hz    | 23.22 ms     |

Violating this deadline causes buffer underrun — audible glitches in output, or dropped
input samples.

### 10.2 Avoiding Allocations

The Rust standard library does not distinguish between real-time-safe and
real-time-unsafe operations. The developer must enforce this discipline:

**Pre-allocated buffers:**

```rust
// BEFORE stream starts — allocate everything
let mut fft_buffer = vec![Complex::new(0.0f32, 0.0); FFT_SIZE];
let mut scratch = vec![Complex::new(0.0f32, 0.0); scratch_len];
let mut magnitude = vec![0.0f32; FFT_SIZE / 2];
let mut window_buf = vec![0.0f32; FFT_SIZE];
```

**Stack-allocated containers:**

```rust
use arrayvec::ArrayVec;
use tinyvec::ArrayVec as TinyArrayVec;

// Fixed-capacity stack vector — panics on overflow instead of allocating
let mut peaks: ArrayVec<(usize, f32), 64> = ArrayVec::new();
for (i, &mag) in spectrum.iter().enumerate() {
    if mag > threshold {
        peaks.push((i, mag));  // No heap allocation
    }
}
```

**Functions to avoid in real-time context:**

| Forbidden                        | Reason                              | Alternative                     |
|----------------------------------|-------------------------------------|---------------------------------|
| `Vec::push` (if at capacity)     | May reallocate                      | Pre-sized `Vec`, `ArrayVec`     |
| `String`, `format!`             | Heap allocation                     | Fixed-size `[u8; N]` buffers    |
| `println!`, `eprintln!`         | Acquires stdout mutex               | Write to atomic flag, log later |
| `Box::new`                       | Heap allocation                     | Stack or arena                  |
| `Mutex::lock`                    | Priority inversion                  | Lock-free SPSC, atomics         |
| `thread::spawn`                  | Allocates, syscall                  | Spawn before stream starts      |
| `HashMap` operations             | May rehash (allocate)               | Pre-sized or `ArrayVec` lookup  |

### 10.3 No-Alloc FFT Processing

`rustfft` itself does not allocate during `process_with_scratch()` — it operates on
caller-provided buffers. The key is to also pre-allocate the scratch buffer:

```rust
let fft = planner.plan_fft_forward(FFT_SIZE);
let scratch_len = fft.get_inplace_scratch_len();
let mut scratch = vec![Complex::new(0.0, 0.0); scratch_len]; // allocate ONCE

// In the hot loop — zero allocations:
fft.process_with_scratch(&mut buffer, &mut scratch);
```

If you call `fft.process()` instead of `process_with_scratch()`, `rustfft` will allocate
a scratch buffer internally on each call. Always use `process_with_scratch()` in
real-time code.

### 10.4 Thread Priority

Audio threads managed by `cpal` already run at elevated priority (set by the OS audio
subsystem). For a custom analysis thread, you may want to raise priority:

```rust
#[cfg(unix)]
fn set_realtime_priority() {
    unsafe {
        let param = libc::sched_param {
            sched_priority: 50, // 1–99 on Linux; macOS uses different API
        };
        let ret = libc::pthread_setschedparam(
            libc::pthread_self(),
            libc::SCHED_FIFO,
            &param,
        );
        if ret != 0 {
            eprintln!("Failed to set RT priority: {}", ret);
        }
    }
}

#[cfg(target_os = "macos")]
fn set_realtime_priority_macos() {
    // macOS uses thread_policy_set with THREAD_TIME_CONSTRAINT_POLICY
    // for true real-time scheduling. See mach/thread_policy.h.
    // For analysis threads (not audio callbacks), simply using
    // QOS_CLASS_USER_INTERACTIVE via pthread_set_qos_class_self_np
    // is often sufficient.
    unsafe {
        let ret = libc::pthread_set_qos_class_self_np(
            libc::QOS_CLASS_USER_INTERACTIVE,
            0,
        );
        if ret != 0 {
            eprintln!("Failed to set QoS class: {}", ret);
        }
    }
}
```

### 10.5 Benchmarking with criterion

```toml
[dev-dependencies]
criterion = { version = "0.5", features = ["html_reports"] }

[[bench]]
name = "fft_bench"
harness = false
```

```rust
// benches/fft_bench.rs
use criterion::{criterion_group, criterion_main, Criterion, black_box};
use rustfft::{FftPlanner, num_complex::Complex};

fn bench_fft_2048(c: &mut Criterion) {
    let mut planner = FftPlanner::<f32>::new();
    let fft = planner.plan_fft_forward(2048);
    let mut buffer = vec![Complex::new(0.0f32, 0.0); 2048];
    let mut scratch = vec![Complex::new(0.0, 0.0); fft.get_inplace_scratch_len()];

    c.bench_function("fft_2048_f32", |b| {
        b.iter(|| {
            fft.process_with_scratch(black_box(&mut buffer), &mut scratch);
        })
    });
}

criterion_group!(benches, bench_fft_2048);
criterion_main!(benches);
```

Expected results on Apple M2 (single core):

| FFT Size | `rustfft` f32 | `rustfft` f64 | FFTW f32 (via C) |
|----------|---------------|---------------|-------------------|
| 512      | ~1.5 us       | ~3.0 us       | ~1.2 us           |
| 1024     | ~3.5 us       | ~7.0 us       | ~2.8 us           |
| 2048     | ~8.0 us       | ~15 us        | ~6.0 us           |
| 4096     | ~18 us        | ~35 us        | ~14 us            |

These times are well within our per-hop budget (HOP_SIZE=512 at 44.1 kHz = 11.6 ms),
leaving ample room for feature extraction on top of the FFT.

---

## 11. Comparison: Rust vs C++ for This Application

### 11.1 Development Speed

Rust's initial development is slower due to the borrow checker. Audio DSP code that
"obviously works" in C++ may require restructuring to satisfy Rust's lifetime rules.
Typical pain points:

- Self-referential structs (e.g., a filter graph where nodes reference each other) require
  `Pin`, arenas, or index-based graphs.
- Callback closures that capture mutable state need `move` semantics and may require
  `Arc<Mutex<>>` or `unsafe` patterns for shared mutable state.

However, once the code compiles, an entire class of runtime debugging disappears.
Segfaults, data races, and use-after-free never occur. For a VJ application that must
run reliably during a live performance, this is a significant advantage.

### 11.2 Safety

| Bug Class                     | C++ Detection          | Rust Prevention            |
|-------------------------------|------------------------|----------------------------|
| Buffer overflow               | ASan (runtime)         | Bounds checking (compile)  |
| Use-after-free                | ASan (runtime)         | Ownership (compile)        |
| Data race                     | TSan (runtime)         | `Send`/`Sync` (compile)    |
| Double free                   | ASan (runtime)         | Ownership (compile)        |
| Dangling pointer              | Sometimes ASan         | Borrow checker (compile)   |
| Null pointer dereference      | Runtime crash          | `Option<T>` (compile)      |
| Uninitialized memory          | MSan (runtime)         | Initialization required    |
| Iterator invalidation         | Runtime UB             | Borrow checker (compile)   |

### 11.3 Library Maturity

| Capability                  | C++ Best Option             | Rust Best Option         | Maturity Gap |
|-----------------------------|-----------------------------|--------------------------|--------------|
| Audio I/O                   | PortAudio, RtAudio, JUCE    | cpal                     | Moderate     |
| FFT                         | FFTW, IPP, KissFFT          | rustfft, realfft         | Small        |
| Onset detection             | libaubio, Essentia           | aubio-rs (FFI), custom   | Large        |
| Pitch detection             | libaubio, WORLD, YIN (C)    | pitch-detection          | Moderate     |
| Spectral analysis           | Essentia, libxtract          | spectrum-analyzer, custom| Large        |
| Audio plugin framework      | JUCE                         | nih-plug, fundsp         | Large        |
| Sample rate conversion      | libsamplerate, Speex         | dasp, rubato             | Moderate     |
| MP3/AAC/FLAC decoding       | FFmpeg, libsndfile           | symphonia, hound         | Moderate     |

### 11.4 FFI Overhead

If you decide to use C libraries from Rust (e.g., FFTW via `fftw-sys`, or libaubio via
`aubio-rs`), the FFI call overhead is negligible — a single function call through a C ABI
costs ~1–2 ns, irrelevant compared to the microsecond-scale cost of the operation itself.
The real cost is build complexity (linking C libraries, cross-compilation, dependency
management).

### 11.5 When Rust Is the Better Choice

- **New project, greenfield codebase** — no existing C++ infrastructure to integrate with.
- **Reliability is paramount** — live performance software that must not crash.
- **Multi-threaded pipeline** — Rust's type system catches concurrency bugs at compile time.
- **Cross-platform targeting** — Cargo's build system handles platform differences cleanly.
- **Small team** — fewer runtime bugs means less debugging time per developer.

### 11.6 When C++ Is the Better Choice

- **Existing C++ codebase** — wrapping in Rust adds complexity without proportional benefit.
- **Dependency on JUCE** — no Rust equivalent for JUCE's comprehensive plugin/UI framework.
- **Team expertise** — experienced C++ audio developers will be faster in C++ for months
  before Rust productivity catches up.
- **Exotic hardware/SDK integration** — some audio hardware SDKs only provide C++ APIs
  with complex class hierarchies that are painful to wrap via FFI.
- **Rapid prototyping** — C++ with a "fast and loose" style (raw pointers, no sanitizers)
  can be faster for throwaway experiments, at the cost of safety.

---

## Summary: Recommended Crate Stack for Our Pipeline

| Layer                    | Crate                  | Version | License         |
|--------------------------|------------------------|---------|-----------------|
| Audio I/O                | `cpal`                 | 0.17    | Apache-2.0      |
| Lock-free ring buffer    | `rtrb`                 | 0.3     | MIT / Apache-2.0|
| FFT                      | `rustfft` + `realfft`  | 6.4 / 3.4 | MIT / Apache-2.0 |
| Windowing / spectrum     | `spectrum-analyzer`    | 1.7     | MIT             |
| Pitch detection          | `pitch-detection`      | 0.3     | MIT             |
| Sample types / DSP utils | `dasp`                 | 0.11    | MIT / Apache-2.0|
| Channel communication    | `crossbeam-channel`    | 0.5     | MIT / Apache-2.0|
| Stack containers         | `arrayvec`             | 0.7     | MIT / Apache-2.0|
| Benchmarking             | `criterion`            | 0.5     | MIT / Apache-2.0|

All MIT / Apache-2.0 — no GPL contamination. `aubio-rs` and `fundsp` are available as
optional additions depending on licensing requirements and feature needs.

> **Next steps**: See [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md) for a
> working prototype using this stack, and [ARCH_pipeline.md](ARCH_pipeline.md) for the
> full system architecture.
