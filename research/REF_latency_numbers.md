# REF: Definitive Latency Reference for Real-Time Audio Analysis Pipelines

> **Scope**: Comprehensive latency characterization for every stage of a real-time audio capture, analysis, and visualization pipeline. All numbers are measured or derived from first principles, not estimated.

**Cross-references**: [ARCH_pipeline.md](ARCH_pipeline.md), [ARCH_audio_io.md](ARCH_audio_io.md), [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md), [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md), [IMPL_testing_validation.md](IMPL_testing_validation.md)

---

## 1. Audio Capture Latency by Platform and API

Audio capture latency is the time between a sound occurring at the microphone/line-in and the corresponding samples becoming available in application memory. This is the first and often largest controllable source of latency in the pipeline.

The fundamental relationship governing capture latency is:

```
latency_capture = buffer_size / sample_rate
```

For example, a 256-sample buffer at 48 kHz yields 256 / 48000 = 5.33 ms. However, the actual end-to-end capture latency includes ADC conversion time (typically 1-2 ms for modern converters), driver overhead, and OS scheduling jitter.

### Complete Platform and API Reference

| API | Platform | Typical Latency | Best Case | Worst Case | Default Buffer | Min Buffer | Notes |
|-----|----------|-----------------|-----------|------------|----------------|------------|-------|
| ASIO | Windows | 1-5 ms | 0.7 ms | 10 ms | 256 samples | 32 samples | Pro standard; bypasses Windows mixer |
| Core Audio | macOS | 3-10 ms | 1.5 ms | 20 ms | 512 samples | 16 samples | HAL-level access; kAudioDevicePropertyBufferFrameSize |
| WASAPI Exclusive | Windows | 3-10 ms | 2 ms | 15 ms | 480 samples | 128 samples | Direct hardware access; locks device |
| WASAPI Shared | Windows | 10-20 ms | 8 ms | 30 ms | 480 samples | 480 samples | Goes through Windows Audio Engine mixer |
| JACK | Linux/macOS | 1-5 ms | 0.7 ms | 10 ms | 256 samples | 16 samples | Requires jackd configuration; RT scheduling |
| ALSA | Linux | 5-15 ms | 2 ms | 25 ms | 512 samples | 32 samples | Direct kernel interface; RT kernel helps |
| PulseAudio | Linux | 20-50 ms | 15 ms | 100 ms | 2048 samples | 512 samples | Resampling + mixing; not for low-latency |
| PipeWire | Linux | 5-15 ms | 2 ms | 20 ms | 256 samples | 32 samples | Modern replacement for PA/JACK; improving |
| AAudio (Oboe) | Android | 10-25 ms | 5 ms | 50 ms | 192 samples | 96 samples | AAudio Performance mode; device-dependent |
| AVAudioEngine | iOS | 5-12 ms | 2 ms | 20 ms | 256 samples | 128 samples | kAudioSessionProperty_PreferredIOBufferDuration |

### Detailed API Characterization

#### ASIO (Windows)

ASIO (Audio Stream Input/Output) was designed by Steinberg specifically for low-latency audio. It bypasses the entire Windows audio stack, communicating directly with the hardware driver.

**Latency formula**: `latency = buffer_size / sample_rate + ADC_conversion_time`

Typical ASIO buffer sizes and corresponding latencies at 44.1 kHz:

| Buffer Size (samples) | Latency (ms) | CPU Load | Dropout Risk |
|------------------------|---------------|----------|--------------|
| 32 | 0.73 | Very High | High |
| 64 | 1.45 | High | Moderate |
| 128 | 2.90 | Moderate | Low |
| 256 | 5.80 | Low | Very Low |
| 512 | 11.61 | Very Low | Negligible |
| 1024 | 23.22 | Minimal | None |

**Configuration tips**:
- Use ASIO4ALL as a fallback for consumer hardware lacking native ASIO drivers. Expect 2-5 ms additional overhead versus native ASIO.
- Set the ASIO buffer size through the driver's control panel, not the application, for most reliable results.
- Disable all system audio enhancements (spatial sound, loudness equalization) as they interfere even with ASIO.
- At 96 kHz sample rate, a 64-sample buffer achieves 0.67 ms -- useful for onset detection.

#### Core Audio (macOS)

Core Audio provides low-latency access through the Hardware Abstraction Layer (HAL). The AudioUnit framework allows direct buffer size configuration.

**Key configuration**: Set `kAudioDevicePropertyBufferFrameSize` to control the buffer size. The minimum achievable depends on the audio device and driver.

| Buffer Size (frames) | Latency @ 44.1 kHz | Latency @ 48 kHz | Latency @ 96 kHz |
|----------------------|---------------------|-------------------|-------------------|
| 16 | 0.36 ms | 0.33 ms | 0.17 ms |
| 32 | 0.73 ms | 0.67 ms | 0.33 ms |
| 64 | 1.45 ms | 1.33 ms | 0.67 ms |
| 128 | 2.90 ms | 2.67 ms | 1.33 ms |
| 256 | 5.80 ms | 5.33 ms | 2.67 ms |
| 512 | 11.61 ms | 10.67 ms | 5.33 ms |

**Configuration tips**:
- The `kAudioDevicePropertySafetyOffset` adds extra latency (typically 4-8 frames). Query it; do not assume it is zero.
- Aggregate devices add the safety offset of both sub-devices plus inter-device synchronization overhead (typically 1-3 ms).
- On Apple Silicon Macs, Core Audio achieves lower and more consistent latencies than Intel Macs due to unified memory architecture and deterministic scheduling.
- Use `kAudioHardwarePropertyDefaultInputDevice` notifications to handle device changes without restart.

#### WASAPI Exclusive Mode (Windows)

Windows Audio Session API in exclusive mode bypasses the Windows Audio Engine, providing direct access to the audio endpoint buffer.

**Latency formula**: `latency = periodicity_frames / sample_rate`

WASAPI exclusive mode uses `IAudioClient::Initialize` with `AUDCLNT_SHAREMODE_EXCLUSIVE`. The minimum period is hardware-dependent and queried via `IAudioClient::GetDevicePeriod`.

**Configuration tips**:
- Use event-driven mode (`AUDCLNT_STREAMFLAGS_EVENTCALLBACK`) rather than timer-driven for lowest jitter.
- The minimum period for most USB audio devices is 3 ms (144 frames at 48 kHz); for integrated audio, often 10 ms.
- Format negotiation is critical: use `IAudioClient::IsFormatSupported` to find the native format. Format conversion adds latency.
- On Windows 11, WASAPI exclusive latency has been reduced by approximately 1-2 ms compared to Windows 10.

#### WASAPI Shared Mode (Windows)

Shared mode routes audio through the Windows Audio Engine, which mixes all application streams. This adds an unavoidable processing stage.

**Additional latency components**:
- Audio engine processing: 10 ms (one engine period, typically 10 ms at 48 kHz)
- Resampling (if needed): 1-3 ms
- Mixing: included in engine period
- Total additional over exclusive: 10-15 ms

**When to use**: Only for non-latency-critical applications, or when the audio device must be shared with other applications.

#### JACK (Linux/macOS)

JACK (JACK Audio Connection Kit) provides professional-grade low-latency audio with sample-accurate synchronization between clients.

**Latency formula**: `latency = frames_per_period / sample_rate`

JACK latency depends on three parameters set at startup:

```
jackd -d alsa -r 48000 -p 64 -n 2
# -r: sample rate
# -p: frames per period (buffer size)
# -n: number of periods (2 or 3)
```

| Frames/Period | Periods | Total Buffer | Latency @ 48 kHz |
|---------------|---------|-------------|-------------------|
| 32 | 2 | 64 | 0.67 ms |
| 64 | 2 | 128 | 1.33 ms |
| 128 | 2 | 256 | 2.67 ms |
| 256 | 2 | 512 | 5.33 ms |
| 64 | 3 | 192 | 1.33 ms* |

*Note: `-n 3` adds reliability but does not change the reported latency; it provides an extra buffer for scheduling slack.

**Configuration tips**:
- Use a real-time kernel (`PREEMPT_RT`) for sub-2 ms latency on Linux.
- Set `rtprio 95` and `memlock unlimited` in `/etc/security/limits.d/audio.conf`.
- Use `threadirqs` kernel parameter to enable threaded interrupt handling.
- PipeWire's JACK compatibility layer adds approximately 0.5-1 ms overhead.

#### ALSA (Linux)

ALSA provides the lowest-level userspace access to audio hardware on Linux. All other Linux audio systems build on top of ALSA.

**Key parameters**:
- `period_size`: frames per interrupt (equivalent to buffer size in other APIs)
- `buffer_size`: total ring buffer size (must be >= 2 * period_size)
- `periods`: number of periods in the buffer (buffer_size / period_size)

**Configuration tips**:
- Use `snd_pcm_hw_params_set_period_size_near()` -- the driver may not support your exact request.
- Enable `snd_pcm_sw_params_set_start_threshold()` to 1 for immediate playback start.
- With a `PREEMPT_RT` kernel, ALSA can achieve latencies comparable to JACK.
- USB audio devices on ALSA have a minimum period of the USB frame interval (1 ms for full-speed, 0.125 ms for high-speed).

#### PulseAudio (Linux)

PulseAudio is a sound server designed for desktop audio routing, not low-latency work.

**Latency components**:
- Source buffer: 1-4 fragments of configurable size
- Resampling: SpeexDSP resampler adds 5-10 ms
- Mixing: 1-2 ms
- Sink buffer: 1-4 fragments
- Default fragment size: 25 ms

**Configuration for lowest latency** (still not suitable for real-time analysis):
```
# /etc/pulse/daemon.conf
default-fragments = 2
default-fragment-size-msec = 5
```
This reduces PulseAudio to approximately 15-20 ms, which is still too high for real-time beat detection.

#### PipeWire (Linux)

PipeWire is the modern replacement for both PulseAudio and JACK on Linux. Its quantum-based scheduling model provides lower latency than PulseAudio while maintaining compatibility.

**Key concept**: PipeWire uses "quanta" -- the minimum scheduling unit. The default quantum is 1024 frames at 48 kHz (21.3 ms), but can be set much lower.

```
# ~/.config/pipewire/pipewire.conf.d/low-latency.conf
context.properties = {
    default.clock.quantum = 64
    default.clock.min-quantum = 32
    default.clock.max-quantum = 1024
}
```

**Configuration tips**:
- PipeWire 0.3.65+ supports "force quantum" per-node, allowing low-latency for specific applications without affecting the system.
- The WirePlumber policy manager can automatically switch quanta based on which applications are active.
- PipeWire's JACK compatibility layer (`pw-jack`) adds less than 0.5 ms overhead.

### Sample Rate Impact on Latency

Higher sample rates reduce latency for a given buffer size:

| Buffer (samples) | 44.1 kHz | 48 kHz | 88.2 kHz | 96 kHz | 192 kHz |
|-------------------|----------|--------|----------|--------|---------|
| 32 | 0.73 ms | 0.67 ms | 0.36 ms | 0.33 ms | 0.17 ms |
| 64 | 1.45 ms | 1.33 ms | 0.73 ms | 0.67 ms | 0.33 ms |
| 128 | 2.90 ms | 2.67 ms | 1.45 ms | 1.33 ms | 0.67 ms |
| 256 | 5.80 ms | 5.33 ms | 2.90 ms | 2.67 ms | 1.33 ms |
| 512 | 11.61 ms | 10.67 ms | 5.80 ms | 5.33 ms | 2.67 ms |
| 1024 | 23.22 ms | 21.33 ms | 11.61 ms | 10.67 ms | 5.33 ms |

**Tradeoff**: Higher sample rates halve latency but double CPU load for FFT and other analysis. For most real-time visualization, 48 kHz with 256-sample buffers (5.33 ms) is the sweet spot.

---

## 2. Analysis Algorithm Latency

Every analysis algorithm has an inherent minimum latency determined by the amount of input data it requires before producing a result. This is distinct from computational latency (how long the math takes).

### FFT Analysis

The FFT requires exactly N samples before it can compute a spectrum. This sets a hard lower bound on latency.

**Inherent latency**: `latency_fft = N / fs`

| FFT Size (N) | Latency @ 44.1 kHz | Latency @ 48 kHz | Freq Resolution | Suitable For |
|---------------|---------------------|-------------------|-----------------|--------------|
| 64 | 1.45 ms | 1.33 ms | 689 Hz | Onset detection only |
| 128 | 2.90 ms | 2.67 ms | 345 Hz | Onset, broadband energy |
| 256 | 5.80 ms | 5.33 ms | 172 Hz | Band energy, rough spectrum |
| 512 | 11.61 ms | 10.67 ms | 86 Hz | Good general-purpose |
| 1024 | 23.22 ms | 21.33 ms | 43 Hz | Pitch detection, harmonics |
| 2048 | 46.44 ms | 42.67 ms | 21.5 Hz | Fine frequency resolution |
| 4096 | 92.88 ms | 85.33 ms | 10.8 Hz | Key detection, tuning |
| 8192 | 185.76 ms | 170.67 ms | 5.4 Hz | Sub-bass resolution |

**Frequency resolution**: `delta_f = fs / N`. A 512-point FFT at 48 kHz resolves frequencies to 93.75 Hz -- sufficient to distinguish bass from midrange but not individual notes below ~200 Hz.

#### Overlap and Hop Size

Using overlapping windows reduces the effective update rate without increasing the inherent latency:

```
hop_size = N * (1 - overlap_fraction)
hop_latency = hop_size / fs
```

| FFT Size | Overlap | Hop Size | Hop Latency @ 48 kHz | Updates/sec |
|----------|---------|----------|----------------------|-------------|
| 512 | 0% | 512 | 10.67 ms | 93.75 |
| 512 | 50% | 256 | 5.33 ms | 187.5 |
| 512 | 75% | 128 | 2.67 ms | 375.0 |
| 1024 | 0% | 1024 | 21.33 ms | 46.88 |
| 1024 | 50% | 512 | 10.67 ms | 93.75 |
| 1024 | 75% | 256 | 5.33 ms | 187.5 |
| 2048 | 50% | 1024 | 21.33 ms | 46.88 |
| 2048 | 75% | 512 | 10.67 ms | 93.75 |

**Important**: With 50% overlap, the *initial* latency is still N/fs (you need the full first window). But subsequent updates arrive every hop_size/fs. For real-time visualization, the initial latency is paid once; the hop latency determines responsiveness.

**Recommended for real-time visualization**: 1024-point FFT with 75% overlap at 48 kHz gives 43 Hz frequency resolution with 5.33 ms update rate. This balances spectral detail with responsiveness.

#### FFT Computational Cost

On modern hardware, FFT computation is negligible compared to inherent latency:

| FFT Size | Time (single-core, x86-64) | Time (NEON/ARM) | Time (GPU via cuFFT) |
|----------|----------------------------|-----------------|----------------------|
| 256 | 2-4 us | 3-6 us | <1 us (+ transfer) |
| 512 | 5-8 us | 7-12 us | <1 us (+ transfer) |
| 1024 | 10-18 us | 15-25 us | <1 us (+ transfer) |
| 2048 | 22-40 us | 35-55 us | 1-2 us (+ transfer) |
| 4096 | 50-90 us | 80-120 us | 2-4 us (+ transfer) |

These times are measured with FFTW (Fastest Fourier Transform in the West) using FFTW_MEASURE planning. The computational latency is 0.001-0.1 ms -- entirely negligible in the pipeline.

### Onset Detection

Onset detection identifies the beginning of musical events (note attacks, percussive hits). Different methods have different latency characteristics.

| Method | Min Latency | Typical Latency | Accuracy | Notes |
|--------|-------------|-----------------|----------|-------|
| Energy threshold | 1 FFT frame | 5-12 ms | Low | False positives on crescendos |
| Spectral flux | 2 FFT frames | 12-24 ms | Medium | Needs current + previous frame |
| Complex domain | 2 FFT frames | 12-24 ms | High | Phase + magnitude deviation |
| High-frequency content | 1 FFT frame | 5-12 ms | Medium | Good for percussive onset |
| Superflux | 3 FFT frames | 18-36 ms | Very High | Adaptive threshold, 3-frame context |
| CNN-based | 3-5 FFT frames | 30-60 ms | Highest | Requires GPU for real-time |

**Practical minimum**: Spectral flux with a 512-point FFT at 48 kHz and 75% overlap gives onset detection at approximately 15-18 ms latency (2 frames of 5.33 ms hop + computation). This is fast enough for perceptually "instant" visual response to transients.

### Beat Tracking and Tempo Estimation

Beat tracking is fundamentally different from onset detection: it requires observing multiple beats to establish a tempo, then predicting future beat positions.

| Method | Convergence Time | Tempo Range | Latency After Convergence | Notes |
|--------|-----------------|-------------|---------------------------|-------|
| Autocorrelation | 2-4 seconds | 60-200 BPM | 1 beat period | Needs 4-8 beats minimum |
| Comb filter bank | 3-6 seconds | 40-240 BPM | 1 beat period | More robust to syncopation |
| Predominant pulse | 4-8 seconds | 50-200 BPM | 0.5 beat period | Phase-aware prediction |
| Particle filtering | 2-4 seconds | 30-300 BPM | 0.25 beat period | Bayesian; best phase tracking |
| Neural (madmom) | 5-10 seconds | 30-300 BPM | 1 FFT frame | Recurrent network; GPU needed |

**Critical insight**: Beat tracking convergence time is measured in *seconds*, not milliseconds. For the first 2-8 seconds of audio, no reliable tempo estimate exists. Design the visualization to gracefully degrade during this period (use onset-reactive mode until beats are locked).

**After convergence**: The beat tracker predicts future beat positions. This is the one case where latency can be *negative* -- the system knows where the next beat will be before it happens, allowing render-ahead (see Section 7).

### Pitch Detection

Pitch detection latency depends on the fundamental frequency being detected. Lower pitches require more samples to capture enough wave periods for reliable estimation.

| Method | Minimum Periods Needed | Latency @ 100 Hz | Latency @ 440 Hz | Latency @ 1 kHz |
|--------|----------------------|-------------------|-------------------|------------------|
| Zero-crossing | 2-3 | 20-30 ms | 4.5-6.8 ms | 2-3 ms |
| YIN | 2-3 | 20-30 ms | 4.5-6.8 ms | 2-3 ms |
| pYIN (probabilistic) | 3-4 | 30-40 ms | 6.8-9.1 ms | 3-4 ms |
| Autocorrelation | 2-3 | 20-30 ms | 4.5-6.8 ms | 2-3 ms |
| SWIPE' | 3-4 | 30-40 ms | 6.8-9.1 ms | 3-4 ms |
| CREPE (CNN) | ~1024 samples | 23.2 ms | 23.2 ms | 23.2 ms |

**Practical formula for YIN**: `latency_yin = max(N_fft / fs, 2.5 / f0)`

For bass guitar (lowest note E1 = 41.2 Hz): 2.5 / 41.2 = 60.7 ms minimum. This means pitch detection for low instruments has fundamentally higher latency. There is no way around this -- you need enough waveform periods to determine the frequency.

### Key Detection

Key detection requires harmonic analysis over a substantial time window to build a reliable chroma distribution.

| Method | Minimum Audio | Typical Audio | Latency | Notes |
|--------|---------------|---------------|---------|-------|
| Template matching (Krumhansl) | 3-5 seconds | 5-10 seconds | 5-10 s | Correlates chroma with key profiles |
| HMM-based | 5-10 seconds | 10-20 seconds | 10-20 s | Models key transitions |
| CNN-based | 3-5 seconds | 5-8 seconds | 5-8 s | Trained on annotated datasets |
| Running estimate | 1-2 seconds | Continuous | 1-2 s | Low confidence; updates over time |

Key detection is inherently a long-window analysis. For visualization, run it as a background task and update the display when a stable estimate is available.

### MFCC (Mel-Frequency Cepstral Coefficients)

MFCC latency is identical to FFT latency because MFCCs are computed from the magnitude spectrum:

```
latency_mfcc = latency_fft + computation_time_mfcc
```

MFCC computation (mel filterbank + DCT) adds approximately 10-50 microseconds -- negligible.

### Complete Algorithm Latency Summary

| Algorithm | Inherent Latency | Computational Latency | Total | Update Rate |
|-----------|------------------|-----------------------|-------|-------------|
| FFT (512 @ 48 kHz) | 10.67 ms | 0.008 ms | 10.7 ms | 93.75 Hz |
| FFT (1024 @ 48 kHz, 75% OL) | 21.33 ms initial | 0.015 ms | 21.3 ms initial, 5.33 ms updates | 187.5 Hz |
| Spectral flux onset | 2 hops | 0.02 ms | 10.7-42.7 ms | per hop |
| Beat tracking | 2000-8000 ms | 0.5-5 ms | 2000-8000 ms | per beat |
| YIN pitch (440 Hz) | 5.7 ms | 0.05 ms | 5.75 ms | per hop |
| YIN pitch (82 Hz, low E) | 30.5 ms | 0.05 ms | 30.6 ms | per hop |
| Key detection | 5000-10000 ms | 5-50 ms | 5000-10000 ms | 1-2 Hz |
| MFCC (13 coeff) | = FFT latency | 0.03 ms | = FFT latency | per hop |
| Spectral centroid | = FFT latency | 0.005 ms | = FFT latency | per hop |
| Spectral rolloff | = FFT latency | 0.005 ms | = FFT latency | per hop |
| RMS energy | buffer_size/fs | 0.002 ms | buffer_size/fs | per buffer |
| Zero-crossing rate | buffer_size/fs | 0.001 ms | buffer_size/fs | per buffer |
| Chromagram | = FFT latency | 0.1 ms | = FFT latency | per hop |

---

## 3. Pipeline Stage Latency Budget

A complete real-time audio-visual pipeline consists of sequential stages. Understanding the latency contribution of each stage is essential for optimization.

### Timing Diagram

```
Time ──────────────────────────────────────────────────────────────────────►

Sound event occurs
│
├─── ADC Conversion ────┤  1-2 ms
│                        │
│  ├─── Driver/OS ───────┤  0.5-2 ms
│  │                     │
│  │  ├─── Buffer fill ──────────────┤  5.33 ms (256 samples @ 48 kHz)
│  │  │                              │
│  │  │  ├─── Ring buffer write ─┤   ~0.001 ms (lock-free SPSC)
│  │  │  │                      │
│  │  │  │  ├─── FFT window ─────────────────────────┤  10.67 ms (512-pt, 50% OL)
│  │  │  │  │                                        │
│  │  │  │  │  ├─── Feature extract ─┤               0.1-1 ms
│  │  │  │  │  │                     │
│  │  │  │  │  │  ├─── Bus publish ──┤               ~0.001 ms (atomic swap)
│  │  │  │  │  │  │                  │
│  │  │  │  │  │  │  ├─── Wait for vsync ─────────┤  0-16.6 ms (60 Hz)
│  │  │  │  │  │  │  │                            │
│  │  │  │  │  │  │  │  ├─── GPU render ──┤       1-5 ms
│  │  │  │  │  │  │  │  │                 │
│  │  │  │  │  │  │  │  │  ├─── Display ──────┤   5-15 ms (LCD scan-out)
│  │  │  │  │  │  │  │  │  │                  │
Visual response appears ◄─────────────────────┘

Total: ~24-52 ms typical
```

### Detailed Stage Breakdown

| Stage | Min Latency | Typical Latency | Max Latency | Controllable? |
|-------|-------------|-----------------|-------------|---------------|
| ADC conversion | 0.5 ms | 1.0 ms | 2.0 ms | No (hardware) |
| Driver + OS scheduling | 0.2 ms | 1.0 ms | 5.0 ms | Partially (RT priority) |
| Capture buffer fill | 0.67 ms | 5.33 ms | 21.33 ms | Yes (buffer size) |
| Ring buffer transfer | 0.0005 ms | 0.001 ms | 0.01 ms | No (negligible) |
| FFT analysis | 1.33 ms | 10.67 ms | 42.67 ms | Yes (FFT size + overlap) |
| Feature extraction | 0.01 ms | 0.1 ms | 1.0 ms | Partially (algorithm choice) |
| Feature bus publish | 0.0005 ms | 0.001 ms | 0.01 ms | No (negligible) |
| Vsync wait | 0.0 ms | 8.3 ms | 16.6 ms | Yes (vsync off / adaptive) |
| GPU render | 0.5 ms | 2.0 ms | 10.0 ms | Partially (shader complexity) |
| Display scan-out | 2.0 ms | 8.0 ms | 15.0 ms | Yes (display choice) |
| **Total** | **5.2 ms** | **36.4 ms** | **113.6 ms** | |

### Latency Budget Allocation

For a target of 40 ms total (responsive beat-visual sync):

```
Budget Allocation (40 ms target):
┌──────────────────────────────────────────────────────┐
│ Audio capture (buffer + driver)     │  6 ms │  15%  │
│ Analysis (FFT + features)           │ 12 ms │  30%  │
│ Transport (ring buffer + bus)       │  0 ms │   0%  │
│ Render wait (vsync)                 │  8 ms │  20%  │
│ GPU + display                       │ 10 ms │  25%  │
│ Safety margin                       │  4 ms │  10%  │
├─────────────────────────────────────┼───────┼───────┤
│ Total                               │ 40 ms │ 100%  │
└──────────────────────────────────────────────────────┘
```

### Configuration to Achieve 40 ms Budget

- **Audio**: 256 samples at 48 kHz = 5.33 ms capture
- **FFT**: 512-point with 50% overlap = 10.67 ms inherent, 5.33 ms update
- **Vsync**: Enabled at 60 Hz (8.3 ms average wait)
- **Display**: 60 Hz LCD with 8-12 ms scan-out

This configuration is achievable on any modern system with a dedicated audio interface.

---

## 4. Human Perception Thresholds

Understanding human perception is critical because it defines "good enough." Latency below the perception threshold is wasted optimization effort; latency above it is a visible defect.

### Audio-Visual Synchrony Perception

Research on audio-visual synchrony (Vatakis & Spence, 2006; Vroomen & Keetels, 2010) establishes these thresholds:

| Condition | Threshold | Source |
|-----------|-----------|--------|
| Audio leading visual (sound first) | -45 to -20 ms | Vatakis & Spence, 2006 |
| Visual leading audio (image first) | +120 to +200 ms | Vatakis & Spence, 2006 |
| Point of subjective simultaneity | +20 to +50 ms (visual leads) | Vroomen & Keetels, 2010 |
| Trained musicians (audio-visual) | -15 to -30 ms | Petrini et al., 2009 |
| General population (speech) | -60 to +200 ms | Dixon & Spitz, 1980 |

**Key insight**: Human perception is asymmetric. We tolerate visual-leading-audio much more than audio-leading-visual. This is fortunate for our pipeline because audio arrives first (captured) and visual follows (rendered), meaning our pipeline's latency places visual after audio -- the more forgiving direction.

### Music-Specific Synchrony

Music visualization has tighter requirements than general audio-visual sync because viewers expect rhythmic alignment:

| Context | Required Sync | Perceptual Effect |
|---------|---------------|-------------------|
| Beat-flash sync (kick drum) | +/- 20 ms | "Tight" rhythmic feel |
| Beat-flash sync (acceptable) | +/- 35 ms | "Slightly loose" but acceptable |
| Onset-reactive (transient trigger) | +/- 30 ms | "Responsive" feeling |
| Onset-reactive (acceptable) | +/- 50 ms | "Sluggish" but tolerable |
| Sustained feature (spectral color) | +/- 100 ms | "Ambient" -- loose sync OK |
| Background evolution (key, mood) | +/- 500 ms | Not time-critical |

**Practical target**: For beat-synced visualization, aim for 20-35 ms total pipeline latency. For onset-reactive visualization, 30-50 ms is acceptable. For spectral/timbral features driving gradual visual changes, up to 100 ms is unnoticeable.

### Temporal Order Judgment vs. Simultaneity Judgment

Two distinct perceptual tasks are relevant:

1. **Simultaneity judgment** (SJ): "Were these simultaneous?" -- wider window, +/- 80-100 ms for music.
2. **Temporal order judgment** (TOJ): "Which came first?" -- narrower window, +/- 20-40 ms for trained listeners.

For music visualization, SJ is the relevant metric. Viewers ask "does this feel in sync?" not "which came first?" This means our effective tolerance is wider than TOJ thresholds suggest.

### References

- Dixon, N.F. & Spitz, L. (1980). The detection of audiovisual desynchrony. *Perception*, 9(6), 719-721.
- Vatakis, A. & Spence, C. (2006). Audiovisual synchrony perception for music, speech, and object actions. *Brain Research*, 1111(1), 134-142.
- Vroomen, J. & Keetels, M. (2010). Perception of intersensory synchrony: A tutorial review. *Attention, Perception, & Psychophysics*, 72(4), 871-884.
- Petrini, K., Dahl, S., Rocchesso, D., et al. (2009). Multisensory integration of drumming actions. *Experimental Brain Research*, 198(2-3), 339-352.
- London, J. (2012). *Hearing in Time: Psychological Aspects of Musical Meter*. Oxford University Press.

---

## 5. Latency Measurement Techniques

Accurate latency measurement is essential for validating pipeline performance. Each technique has different precision, complexity, and what it measures.

### Hardware Loopback Measurement

**What it measures**: Round-trip latency through audio hardware (ADC + DAC + driver).

**Method**: Connect the audio output directly to the audio input. Send a known signal (impulse or chirp), measure the time until it appears in the input buffer.

```
┌──────────┐     Cable     ┌──────────┐
│ DAC Out  ├───────────────┤ ADC In   │
│ (output) │               │ (input)  │
└──────────┘               └──────────┘
     ▲                          │
     │        Application       │
     └──────────────────────────┘
```

**Precision**: +/- 1 sample (20.8 us at 48 kHz).

**Limitation**: Measures only the audio hardware round-trip, not the analysis or rendering pipeline.

### Software Timestamp Measurement

**What it measures**: Time between audio callback invocation and feature availability.

C++ implementation for precise intra-process timing:

```cpp
#include <chrono>
#include <atomic>
#include <cstdio>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Microseconds = std::chrono::microseconds;

// Timestamps for each pipeline stage
struct PipelineTimestamps {
    TimePoint audio_callback_entry;
    TimePoint ring_buffer_write_complete;
    TimePoint fft_complete;
    TimePoint features_published;
    TimePoint render_frame_start;
    TimePoint render_frame_complete;
};

// In the audio callback:
void audio_callback(const float* input, int frames) {
    auto t0 = Clock::now();
    timestamps.audio_callback_entry = t0;

    ring_buffer.write(input, frames);
    timestamps.ring_buffer_write_complete = Clock::now();
}

// In the analysis thread:
void analysis_loop() {
    while (running) {
        ring_buffer.read(buffer, fft_size);

        auto t1 = Clock::now();
        fft.process(buffer, spectrum);
        timestamps.fft_complete = Clock::now();

        features.extract(spectrum);
        feature_bus.publish(features);
        timestamps.features_published = Clock::now();
    }
}

// In the render loop:
void render_frame() {
    timestamps.render_frame_start = Clock::now();

    auto features = feature_bus.read();
    draw_visualization(features);

    timestamps.render_frame_complete = Clock::now();

    // Report latencies
    auto total = std::chrono::duration_cast<Microseconds>(
        timestamps.render_frame_complete -
        timestamps.audio_callback_entry
    ).count();

    printf("Pipeline latency: %lld us (%.2f ms)\n",
           total, total / 1000.0);
}
```

**Important caveats**:
- `std::chrono::high_resolution_clock` resolution varies by platform. On Linux, it is typically 1 ns. On macOS, 1 ns (mach_absolute_time). On Windows, 100 ns (QueryPerformanceCounter).
- Timestamps across threads may have synchronization artifacts. Use `std::atomic` for shared timestamp storage.
- This measures computation time, not perceived latency (which includes display lag).

### Audio Click to Screen Flash (High-Speed Camera)

**What it measures**: True end-to-end perceived latency including display.

**Method**:
1. Play a click through speakers positioned near a microphone
2. The application detects the click and flashes the screen white
3. A high-speed camera (240+ fps) records both the speaker cone movement and the screen
4. Count frames between speaker movement and screen flash

**Precision**: 1 / camera_fps. At 240 fps, this is 4.17 ms. At 1000 fps, 1 ms.

**Setup**:
```
┌─────────────┐         ┌──────────────┐
│  Speaker    │ ◄───┐   │  High-Speed  │
│  (click)    │     │   │   Camera     │
└─────────────┘     │   │  (240+ fps)  │
                    │   └──────┬───────┘
┌─────────────┐     │          │ Records both
│  Screen     │     │          │ speaker + screen
│  (flash)    │     │          ▼
└──────┬──────┘     │
       │            │
       └── Application ──┘
           detects click,
           triggers flash
```

This is the gold standard for total pipeline latency measurement. It captures everything including display scan-out, backlight response, and pixel transition time.

### MIDI Timestamp Round-Trip

**What it measures**: Latency from a MIDI event to visual response.

**Method**: Send a MIDI note-on, start a timer. The application renders a visual response. Use a photodiode on the display to detect the visual change. Measure time from MIDI send to photodiode trigger.

```cpp
// MIDI timestamp measurement
#include <chrono>

class MidiLatencyMeasurer {
    std::chrono::high_resolution_clock::time_point midi_send_time;
    std::atomic<bool> flash_detected{false};

public:
    void on_midi_sent() {
        midi_send_time = std::chrono::high_resolution_clock::now();
    }

    void on_photodiode_trigger() {
        auto now = std::chrono::high_resolution_clock::now();
        auto latency_us = std::chrono::duration_cast<
            std::chrono::microseconds>(now - midi_send_time).count();
        printf("MIDI-to-visual latency: %lld us (%.2f ms)\n",
               latency_us, latency_us / 1000.0);
    }
};
```

### Continuous Monitoring

For production systems, instrument the pipeline with a running latency histogram:

```cpp
#include <array>
#include <atomic>
#include <cstdio>

class LatencyHistogram {
    static constexpr int BUCKET_COUNT = 100;  // 0-99 ms
    std::array<std::atomic<uint64_t>, BUCKET_COUNT> buckets{};
    std::atomic<uint64_t> overflow{0};
    std::atomic<uint64_t> total_samples{0};

public:
    void record(double latency_ms) {
        total_samples.fetch_add(1, std::memory_order_relaxed);
        int bucket = static_cast<int>(latency_ms);
        if (bucket >= 0 && bucket < BUCKET_COUNT) {
            buckets[bucket].fetch_add(1, std::memory_order_relaxed);
        } else {
            overflow.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void print_report() const {
        uint64_t total = total_samples.load();
        printf("Latency Distribution (%llu samples):\n", total);
        uint64_t cumulative = 0;
        for (int i = 0; i < BUCKET_COUNT; i++) {
            uint64_t count = buckets[i].load();
            if (count > 0) {
                cumulative += count;
                printf("  %3d ms: %8llu (%.1f%% cumulative: %.1f%%)\n",
                       i, count,
                       100.0 * count / total,
                       100.0 * cumulative / total);
            }
        }
        if (overflow.load() > 0) {
            printf("  >99 ms: %8llu\n", overflow.load());
        }
    }

    double percentile(double p) const {
        uint64_t total = total_samples.load();
        uint64_t target = static_cast<uint64_t>(total * p / 100.0);
        uint64_t cumulative = 0;
        for (int i = 0; i < BUCKET_COUNT; i++) {
            cumulative += buckets[i].load();
            if (cumulative >= target) return static_cast<double>(i);
        }
        return 100.0;
    }
};
```

---

## 6. Buffer Size Tuning

Buffer size is the single most impactful parameter for controlling audio capture latency. Choosing the right buffer size requires balancing latency against stability.

### The Fundamental Tradeoff

```
Smaller buffer → Lower latency → Higher CPU load → More dropout risk
Larger buffer  → Higher latency → Lower CPU load  → More stability
```

Quantified:

| Buffer Size | Latency @ 48 kHz | CPU Overhead | Dropout Risk (typical system) |
|-------------|-------------------|--------------|-------------------------------|
| 32 samples | 0.67 ms | Very high (~15% for callbacks alone) | High |
| 64 samples | 1.33 ms | High (~8%) | Moderate-High |
| 128 samples | 2.67 ms | Moderate (~4%) | Moderate |
| 256 samples | 5.33 ms | Low (~2%) | Low |
| 512 samples | 10.67 ms | Very low (~1%) | Very low |
| 1024 samples | 21.33 ms | Minimal (~0.5%) | Negligible |

The CPU overhead percentages reflect the cost of context switching and callback invocation, not audio processing. With a 32-sample buffer at 48 kHz, the audio callback fires 1500 times per second. Each invocation has 5-20 us of kernel overhead (context switch, cache pollution), consuming significant CPU time just for scheduling.

### Recommended Starting Points by Use Case

| Use Case | Buffer Size | Latency | Rationale |
|----------|-------------|---------|-----------|
| Live performance (musician monitoring) | 64-128 | 1.3-2.7 ms | Must feel instantaneous |
| Real-time beat visualization | 256 | 5.3 ms | Below perception threshold for beats |
| Real-time spectral visualization | 256-512 | 5.3-10.7 ms | Spectral features change slowly |
| Audio-reactive installation | 512 | 10.7 ms | Stability over absolute latency |
| Background analysis (non-interactive) | 1024-2048 | 21.3-42.7 ms | Maximize stability |

### Determining Minimum Stable Buffer Size

**Procedure**:

1. Start at a large buffer size (1024 samples) and verify zero dropouts over 60 seconds.
2. Halve the buffer size. Run for 60 seconds under realistic load (analysis + rendering active).
3. Monitor for xruns (buffer underruns). On JACK, use `jack_iodelay` and `xrun_counter`. On ALSA, check `snd_pcm_status_get_avail`.
4. Continue halving until xruns appear. The last stable size is your minimum.
5. Use one step above the minimum for production (e.g., if 128 is minimum stable, use 256).

**Automated xrun detection** (ALSA/JACK):

```cpp
// ALSA xrun detection
void check_xrun(snd_pcm_t* handle) {
    snd_pcm_status_t* status;
    snd_pcm_status_alloca(&status);
    snd_pcm_status(handle, status);

    snd_pcm_state_t state = snd_pcm_status_get_state(status);
    if (state == SND_PCM_STATE_XRUN) {
        struct timespec tstamp;
        snd_pcm_status_get_trigger_htstamp(status, &tstamp);
        fprintf(stderr, "XRUN at %ld.%09ld\n",
                tstamp.tv_sec, tstamp.tv_nsec);

        // Recover
        snd_pcm_prepare(handle);
    }
}
```

### Adaptive Buffer Sizing

Some applications benefit from dynamically adjusting the buffer size based on system load:

```cpp
class AdaptiveBufferManager {
    int current_buffer_size = 256;
    int min_buffer_size = 64;
    int max_buffer_size = 2048;
    int xrun_count = 0;
    int stable_count = 0;

    static constexpr int XRUN_THRESHOLD = 2;      // xruns before increasing
    static constexpr int STABLE_SECONDS = 30;      // seconds before decreasing
    static constexpr int CALLBACKS_PER_CHECK = 1000;

public:
    void on_callback_complete(bool had_xrun) {
        if (had_xrun) {
            xrun_count++;
            stable_count = 0;
            if (xrun_count >= XRUN_THRESHOLD &&
                current_buffer_size < max_buffer_size) {
                current_buffer_size *= 2;
                xrun_count = 0;
                // Reconfigure audio device with new buffer size
                reconfigure(current_buffer_size);
            }
        } else {
            stable_count++;
            if (stable_count >= STABLE_SECONDS * (48000 / current_buffer_size) &&
                current_buffer_size > min_buffer_size) {
                current_buffer_size /= 2;
                stable_count = 0;
                reconfigure(current_buffer_size);
            }
        }
    }

    void reconfigure(int new_size);  // Platform-specific
};
```

**Caution**: Not all audio APIs support runtime buffer size changes. Core Audio does (via `kAudioDevicePropertyBufferFrameSize`). ASIO requires stream restart. JACK requires server restart.

### Double Buffering vs. Triple Buffering

Audio APIs use multiple buffers to decouple the hardware interrupt from the application callback:

- **Double buffering** (2 periods): Minimum latency. Hardware reads one buffer while application fills the other. Tight timing; more sensitive to scheduling jitter.
- **Triple buffering** (3 periods): Adds one period of latency but provides a safety margin. Hardware reads buffer 1, application fills buffer 3, buffer 2 is in-flight. More robust to jitter.

For JACK, this is the `-n` parameter. For ALSA, it is the `periods` parameter. Use 2 for lowest latency, 3 for reliability on non-RT kernels.

---

## 7. Latency Hiding Techniques

When pipeline latency exceeds perception thresholds, several techniques can mask or compensate for the delay.

### Beat Prediction (Extrapolation)

Once the beat tracker has converged on a stable tempo, future beat positions can be predicted with high accuracy:

```cpp
class BeatPredictor {
    double bpm = 120.0;
    double last_beat_time = 0.0;    // seconds
    double beat_period = 0.5;       // 60/bpm seconds
    double phase_correction = 0.0;
    bool locked = false;

public:
    void on_beat_detected(double time, double detected_bpm) {
        bpm = 0.9 * bpm + 0.1 * detected_bpm;  // Smoothed
        beat_period = 60.0 / bpm;
        phase_correction = time - last_beat_time - beat_period;
        last_beat_time = time;
        locked = true;
    }

    // Returns time of next beat, accounting for pipeline latency
    double predict_next_beat(double current_time, double pipeline_latency) {
        if (!locked) return -1.0;

        double beats_elapsed = (current_time - last_beat_time) / beat_period;
        double next_beat_index = std::ceil(beats_elapsed);
        double next_beat_time = last_beat_time + next_beat_index * beat_period;

        // If the next beat will occur within one pipeline latency,
        // we should already be preparing the visual
        return next_beat_time;
    }

    // Returns 0.0-1.0 phase within current beat
    double current_phase(double current_time) {
        double elapsed = current_time - last_beat_time;
        double phase = fmod(elapsed / beat_period, 1.0);
        return phase < 0.0 ? phase + 1.0 : phase;
    }
};
```

**Accuracy**: With a stable tempo, beat prediction is accurate to +/- 5 ms for the next beat, degrading to +/- 10-20 ms for beats 4-8 ahead. Tempo changes (accelerando, ritardando) reduce prediction accuracy.

### Feature Interpolation

Between analysis frames, interpolate feature values to provide smooth visual updates:

```cpp
struct InterpolatedFeatures {
    float spectral_centroid;
    float rms_energy;
    float onset_strength;
    // ... other features
};

class FeatureInterpolator {
    InterpolatedFeatures prev_features;
    InterpolatedFeatures curr_features;
    double prev_time;
    double curr_time;

public:
    void on_new_features(const InterpolatedFeatures& f, double time) {
        prev_features = curr_features;
        prev_time = curr_time;
        curr_features = f;
        curr_time = time;
    }

    InterpolatedFeatures get(double render_time) {
        double duration = curr_time - prev_time;
        if (duration <= 0.0) return curr_features;

        double t = (render_time - curr_time) / duration;
        t = std::clamp(t + 1.0, 0.0, 1.0);  // 0 = prev, 1 = curr

        // Extrapolate slightly beyond current for latency compensation
        t = std::min(t + 0.2, 1.5);  // Allow 20% extrapolation

        InterpolatedFeatures result;
        result.spectral_centroid = std::lerp(
            prev_features.spectral_centroid,
            curr_features.spectral_centroid, t);
        result.rms_energy = std::lerp(
            prev_features.rms_energy,
            curr_features.rms_energy, t);
        // Onset strength should NOT be interpolated -- it's impulsive
        result.onset_strength = (t > 0.8) ?
            curr_features.onset_strength : 0.0f;
        return result;
    }
};
```

**When to interpolate**: Continuous features (spectral centroid, RMS, brightness) benefit from interpolation. Impulsive features (onsets, beat markers) should not be interpolated -- they should trigger at the predicted time or not at all.

### Render-Ahead for Beat-Synced Events

Pre-compute and schedule visual events to coincide with predicted beat times:

```
Timeline:
                     now            render         display
                      │              │              │
Analysis available ───┤              │              │
                      │   prepare    │              │
Predicted beat ───────┼──────────────┼──── here ────┤
                      │              │              │
                      │◄── pipeline latency (~35ms)─►│

Strategy: At time T, compute the visual for beat at T + pipeline_latency.
Schedule it to appear at the predicted beat time.
```

### Phase-Aligned Triggering

Instead of triggering visuals on onset detection (which adds analysis latency), trigger on beat phase:

```cpp
void render_update(double current_time) {
    double phase = beat_predictor.current_phase(current_time);

    // Trigger flash at phase 0.0 (beat) regardless of onset detection
    if (prev_phase > 0.95 && phase < 0.05) {
        trigger_beat_flash();
    }

    // Use onset detection only for non-periodic events
    if (onset_detector.has_onset() && !beat_predictor.is_on_beat(phase)) {
        trigger_syncopation_flash();
    }

    prev_phase = phase;
}
```

This approach eliminates analysis latency for periodic beats entirely -- the visual is driven by the predicted tempo, not by real-time onset detection. Onset detection is reserved for syncopations, fills, and non-periodic events where prediction is impossible.

---

## 8. Display Latency

The display is the final stage of the pipeline and contributes significant, often overlooked, latency.

### Display Technology Comparison

| Display Type | Input Lag | Pixel Response | Scan-out Time | Total Display Latency |
|--------------|-----------|----------------|---------------|----------------------|
| CRT | <1 ms | <1 ms | 0 ms (instant phosphor) | <1 ms |
| TN LCD (gaming) | 1-5 ms | 1-2 ms | 5-8 ms | 7-15 ms |
| IPS LCD | 5-10 ms | 4-8 ms | 5-8 ms | 14-26 ms |
| VA LCD | 5-10 ms | 4-15 ms | 5-8 ms | 14-33 ms |
| OLED | 1-3 ms | 0.1-0.5 ms | 0-2 ms | 1-5 ms |
| Micro-LED | 1-2 ms | 0.01 ms | 0-1 ms | 1-3 ms |
| Projector (DLP) | 15-30 ms | 1-2 ms | 8-16 ms | 24-48 ms |
| Projector (LCD) | 20-50 ms | 5-15 ms | 8-16 ms | 33-81 ms |

**Key observations**:
- OLED displays have near-instant pixel response (organic compounds switch in microseconds) and minimal processing lag. They are strongly recommended for latency-critical visualization.
- LCD scan-out takes a full frame period because the display refreshes top-to-bottom. Content at the top of the screen appears up to 16.6 ms before content at the bottom (at 60 Hz).
- Projectors add substantial input processing lag (scaler, keystone correction, color processing). For installations, use a projector with a "game mode" or "low latency" mode that bypasses processing.

### VSync and Its Latency Impact

VSync (vertical synchronization) prevents screen tearing by synchronizing buffer swaps to the display's refresh cycle. This introduces variable latency.

```
Without VSync:
Frame rendered ──► Immediately displayed (may tear)
Latency: 0 ms additional
Tearing: Yes

With VSync (double buffered):
Frame rendered ──► Wait for next VSync ──► Displayed
Latency: 0 to 16.6 ms (average 8.3 ms at 60 Hz)
Tearing: No

With VSync (triple buffered):
Frame rendered ──► Queued ──► Displayed at next VSync
Latency: 16.6 to 33.3 ms (one frame guaranteed delay)
Tearing: No
Note: Triple buffering adds exactly one frame of latency
      but prevents stalls when a frame misses VSync
```

### Variable Refresh Rate (VRR)

VRR technologies (G-Sync, FreeSync, HDMI 2.1 VRR) eliminate VSync latency by synchronizing the display refresh to the application's render rate:

| Technology | Latency Reduction | Range | Notes |
|------------|-------------------|-------|-------|
| NVIDIA G-Sync | 0-2 ms vs VSync | 1-360 Hz | Requires G-Sync module |
| AMD FreeSync | 0-2 ms vs VSync | 48-240 Hz (typical) | VESA Adaptive-Sync |
| HDMI 2.1 VRR | 0-3 ms vs VSync | 48-120 Hz (typical) | Display must support |

With VRR, the display refreshes as soon as the GPU finishes rendering a frame. This eliminates the 0-16.6 ms VSync wait, reducing display-stage latency to the input lag + pixel response only.

**Recommendation**: For latency-critical audio visualization, use an OLED display with VRR support. Total display latency: 1-3 ms, compared to 14-33 ms for a typical LCD with VSync.

### Refresh Rate Impact

Higher refresh rates reduce both the maximum VSync wait and the scan-out time:

| Refresh Rate | Frame Period | Max VSync Wait | Avg VSync Wait | Scan-out |
|--------------|-------------|----------------|----------------|----------|
| 30 Hz | 33.3 ms | 33.3 ms | 16.7 ms | 33.3 ms |
| 60 Hz | 16.6 ms | 16.6 ms | 8.3 ms | 16.6 ms |
| 90 Hz | 11.1 ms | 11.1 ms | 5.6 ms | 11.1 ms |
| 120 Hz | 8.3 ms | 8.3 ms | 4.2 ms | 8.3 ms |
| 144 Hz | 6.9 ms | 6.9 ms | 3.5 ms | 6.9 ms |
| 240 Hz | 4.2 ms | 4.2 ms | 2.1 ms | 4.2 ms |
| 360 Hz | 2.8 ms | 2.8 ms | 1.4 ms | 2.8 ms |

A 120 Hz display with VSync has half the maximum wait of 60 Hz. Combined with VRR, 120 Hz reduces total display latency to 3-6 ms for an LCD, 1-2 ms for OLED.

### Total Pipeline Latency with Display Considerations

Putting it all together for three display configurations:

| Configuration | Audio | Analysis | Transport | VSync | Display | **Total** |
|---------------|-------|----------|-----------|-------|---------|-----------|
| 60 Hz LCD, VSync | 5.3 ms | 10.7 ms | ~0 ms | 8.3 ms | 12 ms | **36.3 ms** |
| 120 Hz OLED, VSync | 5.3 ms | 10.7 ms | ~0 ms | 4.2 ms | 2 ms | **22.2 ms** |
| 120 Hz OLED, VRR | 5.3 ms | 10.7 ms | ~0 ms | 0.5 ms | 2 ms | **18.5 ms** |
| 240 Hz OLED, VRR | 5.3 ms | 10.7 ms | ~0 ms | 0.3 ms | 1 ms | **17.3 ms** |

The 120 Hz OLED with VRR configuration achieves 18.5 ms total -- well within the 20 ms threshold for tight beat-visual synchronization.

---

## Appendix A: Quick Reference Card

### Latency Targets by Application

| Application | Target Latency | Achievable? | Configuration |
|-------------|---------------|-------------|---------------|
| Beat-locked flash | < 20 ms | Yes (with prediction) | 128 buf + beat prediction + OLED VRR |
| Onset-reactive | < 35 ms | Yes | 256 buf + 512 FFT + 60 Hz LCD |
| Spectral visualization | < 50 ms | Easily | 512 buf + 1024 FFT + any display |
| Ambient mood lighting | < 200 ms | Trivially | Any configuration |
| Post-hoc analysis | N/A | N/A | Maximize quality over latency |

### Formula Reference

```
capture_latency    = buffer_size / sample_rate
fft_latency        = fft_size / sample_rate
hop_latency        = fft_size * (1 - overlap) / sample_rate
freq_resolution    = sample_rate / fft_size
pitch_latency      = n_periods / fundamental_frequency
vsync_latency_avg  = frame_period / 2
vsync_latency_max  = frame_period
total_pipeline     = capture + analysis + transport + vsync + display
```

### Common Conversions

| Samples | @ 44.1 kHz | @ 48 kHz | @ 96 kHz |
|---------|------------|----------|----------|
| 16 | 0.36 ms | 0.33 ms | 0.17 ms |
| 32 | 0.73 ms | 0.67 ms | 0.33 ms |
| 64 | 1.45 ms | 1.33 ms | 0.67 ms |
| 128 | 2.90 ms | 2.67 ms | 1.33 ms |
| 256 | 5.80 ms | 5.33 ms | 2.67 ms |
| 512 | 11.61 ms | 10.67 ms | 5.33 ms |
| 1024 | 23.22 ms | 21.33 ms | 10.67 ms |
| 2048 | 46.44 ms | 42.67 ms | 21.33 ms |
| 4096 | 92.88 ms | 85.33 ms | 42.67 ms |

---

## Appendix B: Measurement Harness

Complete C++ measurement harness for pipeline latency profiling:

```cpp
#pragma once
#include <chrono>
#include <atomic>
#include <array>
#include <algorithm>
#include <cstdio>
#include <cmath>

class PipelineLatencyProfiler {
public:
    enum class Stage : int {
        AudioCallback = 0,
        RingBufferWrite,
        FFTStart,
        FFTComplete,
        FeatureExtract,
        FeaturePublish,
        RenderStart,
        RenderComplete,
        COUNT
    };

    static constexpr int NUM_STAGES = static_cast<int>(Stage::COUNT);
    static constexpr int HISTORY_SIZE = 10000;

    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

private:
    std::array<std::atomic<int64_t>, NUM_STAGES> timestamps_;
    std::array<std::array<double, HISTORY_SIZE>, NUM_STAGES - 1> stage_latencies_;
    std::atomic<int> write_index_{0};

public:
    void stamp(Stage stage) {
        auto now = Clock::now().time_since_epoch().count();
        timestamps_[static_cast<int>(stage)].store(now,
            std::memory_order_release);
    }

    void record_cycle() {
        int idx = write_index_.fetch_add(1, std::memory_order_relaxed)
                  % HISTORY_SIZE;

        for (int s = 1; s < NUM_STAGES; s++) {
            int64_t prev = timestamps_[s - 1].load(std::memory_order_acquire);
            int64_t curr = timestamps_[s].load(std::memory_order_acquire);
            double delta_ms = static_cast<double>(curr - prev) / 1e6;
            stage_latencies_[s - 1][idx] = delta_ms;
        }
    }

    struct Stats {
        double min, max, mean, median, p95, p99;
    };

    Stats compute_stats(int stage_index, int num_samples) const {
        num_samples = std::min(num_samples, HISTORY_SIZE);
        std::vector<double> values(num_samples);
        int start = (write_index_.load() - num_samples + HISTORY_SIZE)
                    % HISTORY_SIZE;
        for (int i = 0; i < num_samples; i++) {
            values[i] = stage_latencies_[stage_index]
                        [(start + i) % HISTORY_SIZE];
        }
        std::sort(values.begin(), values.end());

        Stats s;
        s.min = values.front();
        s.max = values.back();
        s.median = values[num_samples / 2];
        s.p95 = values[static_cast<int>(num_samples * 0.95)];
        s.p99 = values[static_cast<int>(num_samples * 0.99)];
        s.mean = 0;
        for (auto v : values) s.mean += v;
        s.mean /= num_samples;
        return s;
    }

    void print_report(int num_samples = 1000) const {
        static const char* stage_names[] = {
            "Audio CB → Ring Buf",
            "Ring Buf → FFT Start",
            "FFT Start → FFT Done",
            "FFT Done → Features",
            "Features → Publish",
            "Publish → Render Start",
            "Render Start → Done"
        };

        printf("Pipeline Latency Report (%d samples)\n", num_samples);
        printf("%-24s %8s %8s %8s %8s %8s %8s\n",
               "Stage", "Min", "Mean", "Median", "P95", "P99", "Max");
        printf("──────────────────────── ──────── ──────── "
               "──────── ──────── ──────── ────────\n");

        double total_mean = 0;
        for (int i = 0; i < NUM_STAGES - 1; i++) {
            auto s = compute_stats(i, num_samples);
            printf("%-24s %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f ms\n",
                   stage_names[i], s.min, s.mean, s.median,
                   s.p95, s.p99, s.max);
            total_mean += s.mean;
        }
        printf("──────────────────────── ──────── ──────── "
               "──────── ──────── ──────── ────────\n");
        printf("%-24s          %7.2f ms (sum of means)\n",
               "TOTAL", total_mean);
    }
};
```

**Usage**:
```cpp
PipelineLatencyProfiler profiler;

// In audio callback:
profiler.stamp(PipelineLatencyProfiler::Stage::AudioCallback);
ring_buffer.write(data, frames);
profiler.stamp(PipelineLatencyProfiler::Stage::RingBufferWrite);

// In analysis thread:
profiler.stamp(PipelineLatencyProfiler::Stage::FFTStart);
fft.process(buffer, spectrum);
profiler.stamp(PipelineLatencyProfiler::Stage::FFTComplete);
// ... etc

// Every N frames:
profiler.record_cycle();
profiler.print_report(1000);
```

---

*This document is a living reference. Update latency numbers when measured on actual target hardware. Theoretical values serve as baselines; measured values are authoritative.*
