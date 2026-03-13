# Audio I/O Library Comparison and Deep Dive

> **Scope**: Evaluation of low-level audio I/O libraries for real-time audio analysis applications (spectrum analysis, beat detection, envelope following, visualization).
>
> **Cross-references**: [ARCH_audio_io.md](ARCH_audio_io.md) | [ARCH_pipeline.md](ARCH_pipeline.md) | [LIB_juce.md](LIB_juce.md) | [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md) | [IMPL_project_setup.md](IMPL_project_setup.md)

---

## Table of Contents

1. [miniaudio Deep Dive](#1-miniaudio-deep-dive)
2. [RtAudio](#2-rtaudio)
3. [PortAudio](#3-portaudio)
4. [libsoundio](#4-libsoundio)
5. [JUCE AudioDeviceManager](#5-juce-audiodevicemanager)
6. [Comparison Table](#6-comparison-table)
7. [Integration with Analysis Pipeline](#7-integration-with-analysis-pipeline)
8. [When to Use Each](#8-when-to-use-each)

---

## 1. miniaudio Deep Dive

### Overview

miniaudio is a single-header C audio library released under a dual public domain / MIT-0 license. Originally named `mini_al`, it was created by David Reid and has matured into one of the most comprehensive zero-dependency audio libraries available. The entire library ships as a single `miniaudio.h` file (~90,000 lines), compiled by defining `MA_IMPLEMENTATION` in exactly one translation unit.

```c
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
```

This single-header design eliminates build system complexity entirely -- no CMake modules, no pkg-config files, no shared library versioning. It compiles directly into whatever translation unit includes it.

### Architecture

miniaudio is structured into three distinct layers, each usable independently:

**Low-Level Layer (ma_device)**
The foundational layer provides direct access to audio hardware through a unified `ma_device` abstraction. A device represents a single audio endpoint (input or output) and manages the platform-specific backend, buffer negotiation, sample rate conversion, and channel mapping. This layer operates in callback mode by default: the backend's audio thread invokes a user-supplied function whenever audio data is needed (playback) or available (capture).

**Decoding Layer (ma_decoder)**
A built-in decoder handles WAV, FLAC, and MP3 formats natively (no external dependencies). Vorbis support is available by bundling `stb_vorbis.h`. The decoder provides a simple read-based interface:

```c
ma_decoder decoder;
ma_decoder_init_file("audio.wav", NULL, &decoder);
ma_uint64 framesRead;
ma_decoder_read_pcm_frames(&decoder, buffer, frameCount, &framesRead);
```

**High-Level Layer (ma_engine / ma_sound)**
The engine layer provides a full mixing graph with spatial audio, effects, and resource management. For real-time analysis applications, this layer is rarely needed -- the low-level device API is the primary integration point.

**Data Conversion (ma_data_converter)**
Handles sample format conversion (u8, s16, s24, s32, f32), sample rate conversion (linear, sinc), and channel conversion (upmixing, downmixing, custom channel maps). This runs automatically within `ma_device` when the application's requested format differs from the hardware's native format.

### Backend Support

miniaudio selects backends at compile time and probes them at runtime in priority order:

| Platform | Backends (Priority Order) | Notes |
|----------|--------------------------|-------|
| Windows  | WASAPI, DirectSound, WinMM | WASAPI is preferred; supports exclusive mode and loopback |
| macOS    | CoreAudio (AudioUnit) | Both default and aggregate devices supported |
| Linux    | PulseAudio, ALSA, JACK | PulseAudio preferred for desktop; ALSA for embedded/headless |
| iOS      | CoreAudio (AudioUnit) | With proper audio session category configuration |
| Android  | AAudio, OpenSL\|ES | AAudio preferred on API 26+; OpenSL fallback |
| BSD      | sndio, audio(4) | NetBSD/OpenBSD support |
| Web      | Web Audio (Emscripten) | AudioWorklet-based, requires `-pthread` flag |

You can force a specific backend or exclude backends at compile time:

```c
#define MA_NO_PULSEAUDIO
#define MA_NO_JACK
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
```

Or select at runtime:

```c
ma_context_config contextConfig = ma_context_config_init();
ma_backend backends[] = { ma_backend_wasapi };
ma_context context;
ma_context_init(backends, 1, &contextConfig, &context);
```

### Loopback Capture (System Audio)

miniaudio supports loopback capture on Windows via WASAPI. This captures all audio being sent to an output device -- essential for "what you hear" analysis applications (visualizers reacting to system audio, monitoring tools).

```c
ma_device_config config = ma_device_config_init(ma_device_type_loopback);
config.capture.pDeviceID = NULL; // Default playback device's loopback
config.capture.format    = ma_format_f32;
config.capture.channels  = 2;
config.sampleRate        = 44100;
config.dataCallback      = loopback_callback;
```

**Platform limitations**: Loopback capture is a WASAPI-specific feature. On macOS, system audio capture requires a virtual audio device (e.g., BlackHole, Loopback by Rogue Amoeba) that routes system output to a standard input device. On Linux, PulseAudio monitor sources achieve the same effect but must be configured through PulseAudio's own device enumeration, not through miniaudio's loopback type.

### Device Enumeration and Configuration

```c
ma_context context;
ma_context_init(NULL, 0, NULL, &context);

ma_device_info* pPlaybackInfos;
ma_uint32 playbackCount;
ma_device_info* pCaptureInfos;
ma_uint32 captureCount;

ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount,
                       &pCaptureInfos, &captureCount);

for (ma_uint32 i = 0; i < captureCount; i++) {
    printf("Capture Device %u: %s\n", i, pCaptureInfos[i].name);

    // Query detailed info (supported formats, sample rates, channels)
    ma_device_info detailedInfo;
    ma_context_get_device_info(&context, ma_device_type_capture,
                               &pCaptureInfos[i].id, &detailedInfo);

    printf("  Min channels: %u\n", detailedInfo.minChannels);
    printf("  Max channels: %u\n", detailedInfo.maxChannels);
    printf("  Min sample rate: %u\n", detailedInfo.minSampleRate);
    printf("  Max sample rate: %u\n", detailedInfo.maxSampleRate);
}

ma_context_uninit(&context);
```

### Callback-Based vs Polling-Based Modes

**Callback mode** (standard): The backend's audio thread calls your function directly. This is the lowest-latency path because data flows without any intermediate buffering or signaling overhead.

```c
void data_callback(ma_device* pDevice, void* pOutput,
                   const void* pInput, ma_uint32 frameCount) {
    // Called on the audio thread -- must be lock-free and fast
    // pInput contains capture data (interleaved)
    // pOutput is where you write playback data
}
```

**Polling mode** (via `ma_device_config.noPreSilencedOutputBuffer` and ring buffers): miniaudio does not have a native blocking read/write API at the device level. For polling-style access, you use the callback to push frames into a lock-free ring buffer (`ma_rb` or `ma_pcm_rb`), then read from the ring buffer on your own thread:

```c
ma_pcm_rb ringBuffer;
ma_pcm_rb_init(ma_format_f32, 2, 4096, NULL, NULL, &ringBuffer);

// In the callback:
void data_callback(ma_device* pDevice, void* pOutput,
                   const void* pInput, ma_uint32 frameCount) {
    void* pWrite;
    ma_pcm_rb_acquire_write(&ringBuffer, &frameCount, &pWrite);
    memcpy(pWrite, pInput, frameCount * ma_get_bytes_per_frame(ma_format_f32, 2));
    ma_pcm_rb_commit_write(&ringBuffer, frameCount);
}

// On the analysis thread:
ma_uint32 framesToRead = 1024;
void* pRead;
ma_pcm_rb_acquire_read(&ringBuffer, &framesToRead, &pRead);
// Process pRead...
ma_pcm_rb_commit_read(&ringBuffer, framesToRead);
```

### Complete Code Example: Capture Device for Analysis

```c
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdio.h>
#include <math.h>

// Lock-free ring buffer for passing audio to analysis thread
static ma_pcm_rb g_ringBuffer;

// Simple RMS calculation for demonstration
static float calculate_rms(const float* samples, ma_uint32 count) {
    float sum = 0.0f;
    for (ma_uint32 i = 0; i < count; i++) {
        sum += samples[i] * samples[i];
    }
    return sqrtf(sum / (float)count);
}

// Audio callback -- runs on the backend's audio thread
static void capture_callback(ma_device* pDevice, void* pOutput,
                              const void* pInput, ma_uint32 frameCount) {
    (void)pOutput; // Not used for capture-only device

    // Write captured frames into ring buffer (non-blocking)
    ma_uint32 framesToWrite = frameCount;
    void* pWriteBuffer;

    ma_result result = ma_pcm_rb_acquire_write(&g_ringBuffer,
                                                &framesToWrite, &pWriteBuffer);
    if (result == MA_SUCCESS && framesToWrite > 0) {
        ma_uint32 bytesPerFrame = ma_get_bytes_per_frame(ma_format_f32, 1);
        memcpy(pWriteBuffer, pInput, framesToWrite * bytesPerFrame);
        ma_pcm_rb_commit_write(&g_ringBuffer, framesToWrite);
    }
    // If ring buffer is full, frames are silently dropped.
    // For analysis (non-safety-critical), this is acceptable.
}

int main(void) {
    ma_result result;

    // Initialize ring buffer: f32 mono, 8192 frames deep
    result = ma_pcm_rb_init(ma_format_f32, 1, 8192, NULL, NULL, &g_ringBuffer);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to init ring buffer: %d\n", result);
        return 1;
    }

    // Configure capture device
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.format   = ma_format_f32;
    deviceConfig.capture.channels = 1;   // Mono for analysis
    deviceConfig.sampleRate       = 44100;
    deviceConfig.dataCallback     = capture_callback;
    deviceConfig.pUserData        = NULL;

    // Optional: set buffer size for latency control
    deviceConfig.periodSizeInFrames = 512;  // ~11.6ms at 44100 Hz
    deviceConfig.periods            = 2;    // Double-buffered

    ma_device device;
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to init device: %d\n", result);
        ma_pcm_rb_uninit(&g_ringBuffer);
        return 1;
    }

    printf("Device: %s\n", device.capture.name);
    printf("Format: f32, %u Hz, %u ch\n",
           device.sampleRate, device.capture.channels);
    printf("Buffer: %u frames/period, %u periods\n",
           device.capture.internalPeriodSizeInFrames,
           device.capture.internalPeriods);

    // Start the device -- callback begins firing
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to start device: %d\n", result);
        ma_device_uninit(&device);
        ma_pcm_rb_uninit(&g_ringBuffer);
        return 1;
    }

    printf("Capturing... Press Enter to stop.\n");

    // Analysis loop (main thread)
    float analysisBuffer[1024];
    while (1) {
        ma_uint32 framesToRead = 1024;
        void* pReadBuffer;

        result = ma_pcm_rb_acquire_read(&g_ringBuffer,
                                         &framesToRead, &pReadBuffer);
        if (result == MA_SUCCESS && framesToRead > 0) {
            memcpy(analysisBuffer, pReadBuffer,
                   framesToRead * sizeof(float));
            ma_pcm_rb_commit_read(&g_ringBuffer, framesToRead);

            float rms = calculate_rms(analysisBuffer, framesToRead);
            float dBFS = 20.0f * log10f(rms + 1e-10f);
            printf("\rRMS: %.4f (%.1f dBFS)  ", rms, dBFS);
            fflush(stdout);
        } else {
            // No data available yet -- yield briefly
            ma_sleep(1);
        }
    }

    // Cleanup (unreachable in this example; use signal handler in production)
    ma_device_stop(&device);
    ma_device_uninit(&device);
    ma_pcm_rb_uninit(&g_ringBuffer);

    return 0;
}
```

**Compilation** (no dependencies beyond system audio frameworks):

```bash
# macOS
cc -o capture capture.c -framework CoreAudio -framework AudioUnit -framework CoreFoundation -lpthread -lm

# Linux
cc -o capture capture.c -lpthread -lm -ldl

# Windows (MSVC)
cl capture.c /link ole32.lib
```

### Performance Characteristics

- **Callback latency**: Determined by `periodSizeInFrames`. At 512 frames / 44100 Hz = 11.6 ms per period. Two periods = 23.2 ms total round-trip. Can go as low as 64 frames (~1.5 ms) on well-configured systems.
- **CPU overhead**: Negligible for the I/O layer itself. The internal resampler (sinc) adds measurable cost only when sample rate conversion is active.
- **Memory footprint**: The compiled code for all backends on a given platform is typically 200-400 KB. The ring buffer and device internals add a few KB.
- **Thread model**: One audio thread per device, managed by the backend. The callback must be realtime-safe: no allocations, no locks, no I/O, no system calls that could block.
- **Binary size contribution**: ~150-300 KB on macOS/Linux with a single backend compiled in; up to ~500 KB with all backends.

---

## 2. RtAudio

### Overview

RtAudio is a C++ class library providing a common interface to audio hardware across multiple operating systems. Created and maintained by Gary Scavone at McGill University, it has been used in academic and production audio software since the early 2000s. Licensed under a permissive MIT-like license.

### Architecture

RtAudio uses a straightforward class-based design. The `RtAudio` class encapsulates device management and audio streaming. You instantiate it, optionally specifying a backend API, then open a stream with format/channel/buffer parameters and provide a callback or use blocking I/O.

### Supported APIs

| Platform | APIs |
|----------|------|
| Windows  | ASIO, WASAPI, DirectSound |
| macOS    | CoreAudio |
| Linux    | ALSA, JACK, PulseAudio |

Notable: RtAudio is one of the few open-source libraries that wraps ASIO (Steinberg's low-latency professional audio API) on Windows. This makes it attractive for pro audio applications targeting ASIO-equipped hardware.

### Callback vs Blocking Mode

**Callback mode** (preferred): A user function is called on the audio thread whenever a buffer boundary is reached. The callback receives input and output buffers, frame count, stream time, and a status flags parameter indicating overrun/underrun conditions.

**Blocking mode**: `RtAudio::openStream()` with a null callback pointer enables blocking I/O via `tickStream()`. This is simpler but introduces additional latency from the internal thread synchronization.

### Code Example: Capture with Callback

```cpp
#include "RtAudio.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <atomic>

static std::atomic<float> g_rms{0.0f};

int captureCallback(void* outputBuffer, void* inputBuffer,
                    unsigned int nFrames,
                    double streamTime,
                    RtAudioStreamStatus status,
                    void* userData)
{
    if (status) {
        fprintf(stderr, "Stream overflow/underflow detected!\n");
    }

    const float* input = static_cast<const float*>(inputBuffer);
    if (input == nullptr) return 0;

    // Compute RMS of this buffer
    float sum = 0.0f;
    for (unsigned int i = 0; i < nFrames; i++) {
        sum += input[i] * input[i];
    }
    g_rms.store(std::sqrt(sum / static_cast<float>(nFrames)),
                std::memory_order_relaxed);

    return 0; // 0 = continue, 1 = stop stream, 2 = drain and stop
}

int main() {
    RtAudio adc;

    if (adc.getDeviceCount() < 1) {
        fprintf(stderr, "No audio devices found!\n");
        return 1;
    }

    // List devices
    std::vector<unsigned int> deviceIds = adc.getDeviceIds();
    for (unsigned int id : deviceIds) {
        RtAudio::DeviceInfo info = adc.getDeviceInfo(id);
        printf("Device %u: %s (in: %u, out: %u)\n",
               id, info.name.c_str(),
               info.inputChannels, info.outputChannels);
    }

    // Configure input stream
    RtAudio::StreamParameters inputParams;
    inputParams.deviceId    = adc.getDefaultInputDevice();
    inputParams.nChannels   = 1;
    inputParams.firstChannel = 0;

    unsigned int sampleRate   = 44100;
    unsigned int bufferFrames = 512;

    try {
        adc.openStream(
            nullptr,          // No output
            &inputParams,     // Input parameters
            RTAUDIO_FLOAT32,  // Sample format
            sampleRate,
            &bufferFrames,    // May be modified by RtAudio
            &captureCallback,
            nullptr           // User data
        );
        adc.startStream();
    } catch (RtAudioErrorType& e) {
        fprintf(stderr, "RtAudio error: %s\n", adc.getErrorText().c_str());
        return 1;
    }

    printf("Capturing at %u Hz, buffer=%u frames. Press Enter to stop.\n",
           sampleRate, bufferFrames);

    // Monitor loop
    while (true) {
        float rms = g_rms.load(std::memory_order_relaxed);
        float dBFS = 20.0f * std::log10(rms + 1e-10f);
        printf("\rRMS: %.4f (%.1f dBFS)  ", rms, dBFS);
        fflush(stdout);
        pa_sleep(10); // or platform-specific sleep
    }

    adc.stopStream();
    if (adc.isStreamOpen()) adc.closeStream();

    return 0;
}
```

### Limitations

- **No native loopback capture**: RtAudio does not expose loopback/monitor devices. On Windows, you would need to use WASAPI loopback outside of RtAudio, or use a virtual audio cable.
- **Single stream per instance**: Each `RtAudio` object manages one stream. For simultaneous capture and playback on different devices, you need two instances.
- **C++ only**: No C API. Integration with C projects or FFI-based languages requires a wrapper.
- **Build complexity**: Requires compiling multiple source files and linking platform-specific libraries. Not as simple as miniaudio's single-header approach.
- **Maintenance cadence**: Updates are less frequent than miniaudio's, though the codebase is stable.

---

## 3. PortAudio

### Overview

PortAudio is a mature, widely-used C library for cross-platform audio I/O. It has been in active development since 1999 and serves as the audio backend for major applications including Audacity, SuperCollider, and numerous academic projects. Licensed under MIT.

PortAudio's design philosophy emphasizes simplicity and portability. Its API is intentionally minimal: initialize, open a stream, start it, process audio, stop, terminate.

### API Structure

The core lifecycle:

```
Pa_Initialize() -> Pa_OpenStream() / Pa_OpenDefaultStream()
    -> Pa_StartStream() -> [callback fires] -> Pa_StopStream()
    -> Pa_CloseStream() -> Pa_Terminate()
```

### Callback and Blocking Modes

**Callback mode**: The standard approach. PortAudio manages the audio thread internally and invokes a user callback with input/output buffers.

**Blocking mode**: Using `Pa_ReadStream()` and `Pa_WriteStream()` after opening the stream without a callback. This adds latency (internal ring buffer synchronization) but simplifies single-threaded code.

### Backend Support

| Platform | APIs |
|----------|------|
| Windows  | WASAPI, DirectSound, WinMM, ASIO (via separate SDK) |
| macOS    | CoreAudio |
| Linux    | ALSA, JACK, OSS |

### Code Example: Capture with Callback

```c
#include "portaudio.h"
#include <stdio.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512

typedef struct {
    float rms;
} AnalysisData;

static int captureCallback(const void* inputBuffer,
                            void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void* userData)
{
    AnalysisData* data = (AnalysisData*)userData;
    const float* in = (const float*)inputBuffer;

    (void)outputBuffer;
    (void)timeInfo;

    if (statusFlags & paInputOverflow) {
        fprintf(stderr, "Input overflow!\n");
    }

    if (in == NULL) return paContinue;

    float sum = 0.0f;
    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        sum += in[i] * in[i];
    }
    data->rms = sqrtf(sum / (float)framesPerBuffer);

    return paContinue;  // paContinue, paComplete, or paAbort
}

int main(void) {
    PaError err;
    PaStream* stream;
    AnalysisData analysisData = { 0.0f };

    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "PortAudio init failed: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    // List devices
    int numDevices = Pa_GetDeviceCount();
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info->maxInputChannels > 0) {
            printf("Input Device %d: %s (%d ch, %.0f Hz default)\n",
                   i, info->name,
                   info->maxInputChannels,
                   info->defaultSampleRate);
        }
    }

    // Open default input stream
    err = Pa_OpenDefaultStream(
        &stream,
        1,                  // Input channels (mono)
        0,                  // Output channels (none)
        paFloat32,          // Sample format
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        captureCallback,
        &analysisData
    );
    if (err != paNoError) {
        fprintf(stderr, "Failed to open stream: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "Failed to start stream: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    printf("Capturing... Press Enter to stop.\n");

    // Monitor loop
    while (1) {
        Pa_Sleep(10);
        float dBFS = 20.0f * log10f(analysisData.rms + 1e-10f);
        printf("\rRMS: %.4f (%.1f dBFS)  ", analysisData.rms, dBFS);
        fflush(stdout);
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
```

### Strengths and Weaknesses

**Strengths**:
- Battle-tested in production for 25+ years
- ASIO support (with Steinberg SDK)
- `Pa_GetStreamInfo()` provides actual latency values negotiated with hardware
- Rich device enumeration with detailed capability queries
- Extensive documentation and community knowledge

**Weaknesses**:
- Build system is more complex than miniaudio (CMake or autotools)
- V19 API had some design issues (V20 development has stalled)
- No built-in sample rate conversion (format conversion is limited)
- No loopback capture abstraction
- Multiple source files required (not single-header)

---

## 4. libsoundio

### Overview

libsoundio is a cross-platform audio I/O library written in C by Andrew Kelley (creator of the Zig programming language). It was designed from scratch with a focus on API clarity, correctness, and minimal latency. Licensed under MIT.

The library's design reflects Kelley's philosophy of systems-level clarity: the API is small, orthogonal, and avoids hidden state. It was written to replace PortAudio in specific use cases where PortAudio's design showed its age.

### Backend Support

| Platform | APIs |
|----------|------|
| Windows  | WASAPI |
| macOS    | CoreAudio |
| Linux    | PulseAudio, JACK, ALSA |
| (Dummy)  | Null backend for testing |

### API Design

libsoundio uses a ring-buffer-based approach. Rather than providing a simple callback with pre-allocated buffers (like PortAudio/RtAudio), it exposes a write/read callback where you request buffer regions from a ring buffer, write into them, and advance the pointer. This gives you more control over buffer management.

### Code Example: Capture

```c
#include <soundio/soundio.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

static void read_callback(struct SoundIoInStream* instream,
                           int frame_count_min,
                           int frame_count_max)
{
    struct SoundIoChannelArea* areas;
    int frames_left = frame_count_max;
    float sum = 0.0f;
    int total_frames = 0;

    while (frames_left > 0) {
        int frame_count = frames_left;
        enum SoundIoError err = soundio_instream_begin_read(
            instream, &areas, &frame_count);

        if (err) {
            fprintf(stderr, "Read error: %s\n", soundio_strerror(err));
            return;
        }

        if (frame_count == 0) break;

        if (areas) {
            // areas[0] is channel 0
            const float* samples = (const float*)areas[0].ptr;
            int step = areas[0].step / sizeof(float);

            for (int i = 0; i < frame_count; i++) {
                float s = samples[i * step];
                sum += s * s;
            }
            total_frames += frame_count;
        }
        // else: hole in the buffer (silence), skip

        soundio_instream_end_read(instream);
        frames_left -= frame_count;
    }

    if (total_frames > 0) {
        float rms = sqrtf(sum / (float)total_frames);
        float dBFS = 20.0f * log10f(rms + 1e-10f);
        printf("\rRMS: %.4f (%.1f dBFS)  ", rms, dBFS);
        fflush(stdout);
    }
}

static void overflow_callback(struct SoundIoInStream* instream) {
    fprintf(stderr, "Buffer overflow!\n");
}

int main(void) {
    struct SoundIo* soundio = soundio_create();
    if (!soundio) {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }

    enum SoundIoError err = soundio_connect(soundio);
    if (err) {
        fprintf(stderr, "Connect error: %s\n", soundio_strerror(err));
        soundio_destroy(soundio);
        return 1;
    }

    soundio_flush_events(soundio);

    // Enumerate devices
    int input_count = soundio_input_device_count(soundio);
    printf("Input devices:\n");
    for (int i = 0; i < input_count; i++) {
        struct SoundIoDevice* device = soundio_get_input_device(soundio, i);
        printf("  %d: %s\n", i, device->name);
        soundio_device_unref(device);
    }

    // Get default input device
    int default_idx = soundio_default_input_device_index(soundio);
    struct SoundIoDevice* device = soundio_get_input_device(soundio, default_idx);
    printf("Using: %s\n", device->name);

    // Create input stream
    struct SoundIoInStream* instream = soundio_instream_create(device);
    instream->format = SoundIoFormatFloat32NE;  // Native-endian float
    instream->sample_rate = 44100;
    instream->layout = device->current_layout;
    instream->software_latency = 0.01;  // Request 10ms
    instream->read_callback = read_callback;
    instream->overflow_callback = overflow_callback;

    err = soundio_instream_open(instream);
    if (err) {
        fprintf(stderr, "Open error: %s\n", soundio_strerror(err));
        return 1;
    }

    printf("Actual latency: %.3f ms\n",
           instream->software_latency * 1000.0);

    err = soundio_instream_start(instream);
    if (err) {
        fprintf(stderr, "Start error: %s\n", soundio_strerror(err));
        return 1;
    }

    printf("Capturing... Press Enter to stop.\n");

    // Event loop
    while (1) {
        soundio_wait_events(soundio);
    }

    soundio_instream_destroy(instream);
    soundio_device_unref(device);
    soundio_destroy(soundio);

    return 0;
}
```

### Strengths and Weaknesses

**Strengths**:
- Clean, well-designed C API with minimal footprint
- Excellent error reporting (every function returns an error enum)
- Built-in ring buffer primitive (`SoundIoRingBuffer`)
- Buffer regions expose stride information (non-interleaved support)
- Small codebase (~8,000 lines of C)

**Weaknesses**:
- **Maintenance status is uncertain** -- development slowed significantly after Kelley focused on Zig full-time. Last major commit activity was around 2020.
- No ASIO support on Windows
- No built-in decoding, conversion, or mixing
- Smaller community than PortAudio or miniaudio
- No loopback capture abstraction
- No mobile platform support (iOS/Android)

---

## 5. JUCE AudioDeviceManager

### Overview

JUCE (Jules' Utility Class Extensions) is a comprehensive C++ framework for building audio applications and plugins. Its `AudioDeviceManager` class provides audio I/O as part of a much larger ecosystem that includes GUI (with OpenGL), DSP primitives, plugin hosting/creation (VST/AU/AAX), MIDI, networking, and more.

JUCE is developed by PACE Anti-Piracy (formerly ROLI). It is dual-licensed: GPLv3 for open-source projects, or a commercial license (starting at $50/month for individual developers, scaling up for enterprises).

### Architecture

```
AudioDeviceManager
  └── AudioIODevice (backend-specific)
       ├── CoreAudioIODevice (macOS/iOS)
       ├── WASAPIAudioIODevice (Windows)
       ├── ASIOAudioIODevice (Windows)
       ├── ALSAAudioIODevice (Linux)
       └── JACKAudioIODevice (Linux)
```

The `AudioDeviceManager` coordinates device selection, format negotiation, and callback dispatch. Your application implements `AudioIODeviceCallback`:

```cpp
class MyAudioProcessor : public juce::AudioIODeviceCallback {
public:
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override
    {
        // Non-interleaved buffers -- one float* per channel
        // Process inputChannelData[0..numInputChannels-1]
        // Each is numSamples long
        for (int ch = 0; ch < numInputChannels; ch++) {
            const float* channelData = inputChannelData[ch];
            // Feed to analysis pipeline...
        }

        // Clear output if not producing audio
        for (int ch = 0; ch < numOutputChannels; ch++) {
            std::memset(outputChannelData[ch], 0,
                       sizeof(float) * numSamples);
        }
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {
        // Called before streaming begins
        sampleRate_ = device->getCurrentSampleRate();
        bufferSize_ = device->getCurrentBufferSizeSamples();
    }

    void audioDeviceStopped() override {
        // Called after streaming stops
    }

private:
    double sampleRate_ = 0;
    int bufferSize_ = 0;
};

// Usage:
juce::AudioDeviceManager deviceManager;
deviceManager.initialiseWithDefaultDevices(1, 0); // 1 input, 0 outputs

MyAudioProcessor processor;
deviceManager.addAudioCallback(&processor);
// ... later ...
deviceManager.removeAudioCallback(&processor);
```

### When JUCE is the Right Choice

**Use JUCE when**:
- You are building a full GUI application with audio visualization (JUCE provides OpenGL integration, custom component rendering, and a mature layout system)
- You need to host or create VST/AU/AAX plugins
- You need MIDI I/O alongside audio
- You want `AudioProcessorGraph` for modular DSP routing
- Your team already has JUCE expertise
- You are building a commercial product and will pay for the license

**Avoid JUCE when**:
- You need a lightweight, embeddable audio I/O layer (JUCE pulls in hundreds of thousands of lines of code)
- You need a C API (JUCE is C++ only with heavy use of inheritance and smart pointers)
- Your project is open-source and you do not want the GPL constraint
- You only need audio capture, not the full application framework
- Build times matter -- JUCE modules add significant compilation time

### Binary Size Impact

A minimal JUCE application using only `juce_audio_devices` and `juce_audio_basics` modules compiles to roughly 2-5 MB. Including `juce_gui_basics` and `juce_opengl` pushes this to 8-15 MB. In contrast, an equivalent miniaudio-based application compiles to 200-500 KB.

---

## 6. Comparison Table

### Core Feature Comparison

| Feature | miniaudio | RtAudio | PortAudio | libsoundio | JUCE |
|---------|-----------|---------|-----------|------------|------|
| **Language** | C99 | C++11 | C89 | C11 | C++17 |
| **License** | Public Domain / MIT-0 | MIT-like | MIT | MIT | GPL3 / Commercial |
| **API Style** | Procedural C | Class-based C++ | Procedural C | Procedural C | OOP C++ (framework) |
| **Integration** | Single header | Multi-file, CMake | Multi-file, CMake/autotools | Multi-file, CMake | Module system, Projucer/CMake |
| **Binary Size** | 150-500 KB | 100-300 KB | 150-400 KB | 80-200 KB | 2-15 MB |
| **Dependencies** | None (zero) | System audio libs | System audio libs | System audio libs | System audio + UI libs |

### Platform and Backend Support

| Backend | miniaudio | RtAudio | PortAudio | libsoundio | JUCE |
|---------|-----------|---------|-----------|------------|------|
| **CoreAudio (macOS)** | Yes | Yes | Yes | Yes | Yes |
| **WASAPI (Windows)** | Yes | Yes | Yes | Yes | Yes |
| **ASIO (Windows)** | No | Yes | Yes (SDK) | No | Yes |
| **DirectSound** | Yes | Yes | Yes | No | Yes |
| **WinMM** | Yes | No | Yes | No | No |
| **ALSA (Linux)** | Yes | Yes | Yes | Yes | Yes |
| **PulseAudio** | Yes | Yes | No | Yes | No |
| **JACK** | Yes | Yes | Yes | Yes | Yes |
| **AAudio (Android)** | Yes | No | No | No | Yes |
| **OpenSL ES (Android)** | Yes | No | No | No | Yes |
| **iOS (CoreAudio)** | Yes | No | No | No | Yes |
| **Web Audio** | Yes | No | No | No | No |

### Capability Comparison

| Capability | miniaudio | RtAudio | PortAudio | libsoundio | JUCE |
|------------|-----------|---------|-----------|------------|------|
| **Callback mode** | Yes | Yes | Yes | Yes | Yes |
| **Blocking/polling mode** | Via ring buffer | Yes (native) | Yes (native) | No | No |
| **Loopback capture** | WASAPI only | No | No | No | No |
| **Built-in resampling** | Yes (linear, sinc) | No | No | No | Yes |
| **Format conversion** | Yes (all formats) | Limited | Limited | No | Yes |
| **Channel conversion** | Yes | No | No | No | Yes |
| **Built-in ring buffer** | Yes (ma_rb, ma_pcm_rb) | No | No | Yes (SoundIoRingBuffer) | Yes (AbstractFifo) |
| **Decoding (WAV/MP3/FLAC)** | Yes | No | No | No | Yes |
| **Mixing engine** | Yes (ma_engine) | No | No | No | Yes (AudioProcessorGraph) |
| **Spatial audio** | Yes (ma_spatializer) | No | No | No | Yes |
| **MIDI support** | No | No | No | No | Yes |
| **Device hot-plug detection** | Yes (via notification) | No | No | Yes | Yes |

### Maintenance and Community

| Aspect | miniaudio | RtAudio | PortAudio | libsoundio | JUCE |
|--------|-----------|---------|-----------|------------|------|
| **Primary maintainer** | David Reid | Gary Scavone | PortAudio team | Andrew Kelley | PACE |
| **Last significant release** | Active (2024-2025) | Periodic (2023) | v19.7.0 (2021) | Stalled (~2020) | Active (monthly) |
| **GitHub stars** | ~4,000+ | ~1,200+ | N/A (hosted on assembla/github) | ~1,800+ | ~6,000+ |
| **Community size** | Growing rapidly | Moderate, academic | Large, mature | Small | Large, commercial |
| **Documentation quality** | Good (examples + API docs) | Adequate | Extensive | Good | Excellent (tutorials, forum) |

### Latency Characteristics

All libraries in callback mode achieve similar minimum latencies because they all wrap the same platform APIs. The differences arise from:

| Factor | miniaudio | RtAudio | PortAudio | libsoundio | JUCE |
|--------|-----------|---------|-----------|------------|------|
| **Minimum buffer size** | 32-64 frames | 32-64 frames | 64 frames | 32 frames | 32-64 frames |
| **Internal buffering overhead** | Minimal | Minimal | 1 extra copy in some paths | Minimal | Minimal |
| **Resampling latency** | If active: ~100 samples (sinc) | N/A | N/A | N/A | If active: configurable |
| **Typical practical latency** | 3-12 ms | 3-12 ms | 5-15 ms | 3-10 ms | 3-12 ms |

---

## 7. Integration with Analysis Pipeline

The core challenge: audio callbacks run on a real-time thread managed by the OS audio subsystem. Analysis code (FFT, spectral processing, ML inference) runs on a separate worker thread. The bridge between them must be lock-free.

### Architecture Pattern

```
┌─────────────┐     Lock-Free      ┌──────────────┐
│ Audio Thread │ ──── Ring ─────── │ Analysis     │
│ (callback)   │     Buffer        │ Thread        │
│              │                   │               │
│ Writes PCM   │                   │ Reads PCM     │
│ frames       │                   │ Runs FFT      │
│              │                   │ Updates state  │
└─────────────┘                   └──────────────┘
                                        │
                                   Atomic / Lock-Free
                                        │
                                  ┌──────────────┐
                                  │ Render/UI     │
                                  │ Thread        │
                                  │               │
                                  │ Reads results │
                                  │ Draws visuals │
                                  └──────────────┘
```

### Buffer Management Patterns

**Pattern 1: Single-Producer Single-Consumer (SPSC) Ring Buffer**

This is the standard pattern. The audio callback writes, the analysis thread reads. All libraries either provide a built-in ring buffer or you use a third-party one.

| Library | Built-in Ring Buffer | Notes |
|---------|---------------------|-------|
| miniaudio | `ma_pcm_rb` | PCM-aware: handles frame alignment, format info |
| libsoundio | `SoundIoRingBuffer` | Generic byte-level ring buffer |
| JUCE | `AbstractFifo` | Index-based; you manage the storage array |
| RtAudio | None | Use `boost::lockfree::spsc_queue` or `readerwriterqueue` |
| PortAudio | `PaUtil_WriteRingBuffer` | Available in pa_ringbuffer.h (part of PortAudio utils) |

**Implementation with miniaudio** (shown in the complete example above).

**Implementation with a generic SPSC queue** (for RtAudio / PortAudio):

```cpp
// Using moodycamel::ReaderWriterQueue (header-only, lock-free)
#include "readerwriterqueue.h"

struct AudioBlock {
    std::vector<float> samples;
    int sampleRate;
    int channels;
};

moodycamel::ReaderWriterQueue<AudioBlock> audioQueue(64);

// In audio callback:
int callback(void* out, void* in, unsigned int nFrames, ...) {
    AudioBlock block;
    block.samples.assign(static_cast<float*>(in),
                         static_cast<float*>(in) + nFrames);
    block.sampleRate = 44100;
    block.channels = 1;
    audioQueue.try_enqueue(std::move(block));
    return 0;
}

// On analysis thread:
void analysisLoop() {
    AudioBlock block;
    while (running) {
        if (audioQueue.try_dequeue(block)) {
            processFFT(block.samples.data(), block.samples.size());
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    }
}
```

**Warning about the above pattern**: The `AudioBlock` with `std::vector` performs heap allocation inside the audio callback, which violates real-time safety. For production code, use a pre-allocated pool or fixed-size buffers:

```cpp
// Real-time safe version: fixed-size blocks, no allocation in callback
struct AudioBlock {
    float samples[1024]; // Fixed max size
    int frameCount;
};

moodycamel::ReaderWriterQueue<AudioBlock> audioQueue(64); // Pre-allocated

// In callback: no allocation occurs
int callback(void* out, void* in, unsigned int nFrames, ...) {
    AudioBlock block;
    block.frameCount = std::min(nFrames, 1024u);
    std::memcpy(block.samples, in, block.frameCount * sizeof(float));
    audioQueue.try_enqueue(block); // Fails gracefully if full
    return 0;
}
```

### Sample Format Conversion

Audio hardware may provide samples in formats other than float32. Conversion considerations:

| Source Format | Conversion to float32 | Notes |
|---------------|----------------------|-------|
| int16 (s16) | `float = s16 / 32768.0f` | Most common hardware format |
| int24 (s24) | `float = s24 / 8388608.0f` | Pro audio interfaces |
| int32 (s32) | `float = s32 / 2147483648.0f` | Rare but exists |
| float32 (f32) | Identity | Preferred for analysis |

**miniaudio** handles this automatically via `ma_data_converter` when you request `ma_format_f32` but the hardware provides a different format. Other libraries vary:

- **RtAudio**: Requests format conversion from the backend; may fail if format is not natively supported.
- **PortAudio**: `paFloat32` flag requests conversion; most backends comply.
- **libsoundio**: Must check `SoundIoDevice.formats` array and handle conversion yourself if float32 is not available.
- **JUCE**: Always delivers float32 to the callback.

### Multi-Channel Handling

For analysis applications, mono is typically sufficient (sum to mono or pick a channel). But when multi-channel data arrives:

```c
// Deinterleave stereo to mono (average L+R)
void stereo_to_mono(const float* interleaved, float* mono,
                    int frameCount) {
    for (int i = 0; i < frameCount; i++) {
        mono[i] = (interleaved[i * 2] + interleaved[i * 2 + 1]) * 0.5f;
    }
}

// Extract single channel from interleaved N-channel
void extract_channel(const float* interleaved, float* output,
                     int frameCount, int totalChannels, int channel) {
    for (int i = 0; i < frameCount; i++) {
        output[i] = interleaved[i * totalChannels + channel];
    }
}
```

**JUCE note**: JUCE delivers non-interleaved (planar) audio -- `inputChannelData[ch][sample]` -- so deinterleaving is unnecessary.

**libsoundio note**: libsoundio exposes channel areas with a `step` field that indicates the byte stride between samples. This naturally handles both interleaved and non-interleaved layouts.

### Hop-Size Accumulation Pattern

FFT-based analysis typically requires a fixed-size window (e.g., 2048 or 4096 samples) advanced by a hop size (e.g., 512 samples). Audio callbacks deliver variable-size buffers. An accumulation buffer bridges this mismatch:

```c
#define FFT_SIZE 2048
#define HOP_SIZE 512

typedef struct {
    float buffer[FFT_SIZE];
    int writePos;
} HopAccumulator;

// Called from analysis thread after reading from ring buffer
void accumulate_and_analyze(HopAccumulator* acc, const float* newSamples,
                            int count, void (*onWindowReady)(const float*, int)) {
    for (int i = 0; i < count; i++) {
        acc->buffer[acc->writePos++] = newSamples[i];

        if (acc->writePos >= FFT_SIZE) {
            // Window is full -- process it
            onWindowReady(acc->buffer, FFT_SIZE);

            // Shift by hop size: keep (FFT_SIZE - HOP_SIZE) samples
            int retain = FFT_SIZE - HOP_SIZE;
            memmove(acc->buffer, acc->buffer + HOP_SIZE,
                    retain * sizeof(float));
            acc->writePos = retain;
        }
    }
}
```

---

## 8. When to Use Each

### Decision Matrix

| Scenario | Recommended | Rationale |
|----------|-------------|-----------|
| **Quick prototype / proof of concept** | miniaudio | Zero dependencies, single header, compiles in seconds, works immediately |
| **Production cross-platform desktop app** | miniaudio or RtAudio | miniaudio for C/zero-dep; RtAudio for C++ with ASIO support |
| **Pro audio with ASIO requirement** | RtAudio or PortAudio | Only libraries wrapping ASIO without a commercial license |
| **Full GUI application with visualization** | JUCE | Integrated UI + audio + OpenGL in one framework |
| **Audio plugin (VST/AU/AAX)** | JUCE | Industry standard for plugin development |
| **Embedded / resource-constrained** | miniaudio | Smallest footprint, no dynamic linking, no dependencies |
| **Mobile (iOS + Android)** | miniaudio or JUCE | Only options with native mobile backend support |
| **Web (Emscripten/WASM)** | miniaudio | Only library with Web Audio backend |
| **System audio capture (loopback)** | miniaudio (Windows) | Only library with built-in loopback; macOS requires virtual device regardless |
| **Academic / research with JACK** | RtAudio or PortAudio | Mature JACK support, well-documented in academic contexts |
| **Headless Linux server analysis** | miniaudio (ALSA) or PortAudio (ALSA) | Direct ALSA access without PulseAudio overhead |
| **Existing C codebase integration** | miniaudio, PortAudio, or libsoundio | C API, no C++ runtime dependency |
| **Existing C++ codebase integration** | RtAudio or miniaudio | RtAudio is idiomatic C++; miniaudio works via extern "C" |

### Recommendation for This Project (Real-Time Audio Analysis)

For a real-time audio analysis pipeline (capture -> FFT -> visualization), the primary requirements are:

1. **Low-latency capture**: All libraries meet this requirement in callback mode.
2. **Cross-platform**: miniaudio has the widest platform coverage.
3. **Minimal integration overhead**: miniaudio's single-header design is unmatched.
4. **Loopback capture**: miniaudio is the only option with built-in support (WASAPI).
5. **Built-in ring buffer**: miniaudio provides `ma_pcm_rb`, eliminating a third-party dependency.
6. **Format conversion**: miniaudio handles this transparently.
7. **Maintenance trajectory**: miniaudio is actively maintained and growing.

**Primary recommendation: miniaudio** for the audio I/O layer. It provides the shortest path from zero to working audio capture, requires no build system changes, and covers every platform including mobile and web. Its built-in ring buffer and format conversion eliminate two common integration challenges.

**Secondary recommendation: RtAudio** if ASIO support is required for professional audio interfaces on Windows. RtAudio's C++ API integrates naturally into C++ analysis codebases, and its ASIO wrapper avoids the licensing constraints of JUCE.

**Tertiary option: PortAudio** if the project must align with an existing ecosystem that already uses PortAudio (e.g., extending Audacity, working within SuperCollider's architecture).

**Avoid libsoundio** for new projects due to uncertain maintenance status. While its API design is elegant, the lack of active development means bugs and platform compatibility issues may go unresolved.

**JUCE only if** the project evolves into a full desktop application with its own GUI, plugin hosting, or needs the broader JUCE DSP ecosystem. The licensing and binary size overhead are not justified for an audio capture + analysis pipeline.

### Migration Path

A practical approach for iterative development:

1. **Start with miniaudio**: Get capture working in an afternoon. Validate the analysis pipeline.
2. **Abstract the I/O layer**: Define a simple interface (`AudioSource::start()`, `AudioSource::readFrames()`, `AudioSource::stop()`).
3. **Add backends as needed**: If ASIO becomes necessary, add an RtAudio backend behind the same interface.
4. **Consider JUCE later**: If the project grows into a full application with UI requirements, JUCE can replace the entire stack.

This layered approach avoids premature commitment while keeping the door open for every library evaluated here.

---

## Appendix: Build Instructions Summary

### miniaudio

```bash
# No build step required. Just include the header.
# Compile your .c/.cpp file and link platform audio libs:

# macOS:
cc app.c -framework CoreAudio -framework AudioUnit -framework CoreFoundation -lpthread -lm

# Linux:
cc app.c -lpthread -lm -ldl  # PulseAudio/ALSA loaded dynamically

# Windows (MSVC):
cl app.c /link ole32.lib
```

### RtAudio

```bash
# CMake build:
git clone https://github.com/thestk/rtaudio.git
cd rtaudio && mkdir build && cd build
cmake .. -DRTAUDIO_API_JACK=OFF  # Disable JACK if not needed
cmake --build .

# Link: -lrtaudio plus platform libs
```

### PortAudio

```bash
# CMake build:
git clone https://github.com/PortAudio/portaudio.git
cd portaudio && mkdir build && cd build
cmake ..
cmake --build .

# Link: -lportaudio plus platform libs
```

### libsoundio

```bash
# CMake build:
git clone https://github.com/andrewrk/libsoundio.git
cd libsoundio && mkdir build && cd build
cmake ..
cmake --build .

# Link: -lsoundio plus platform libs
```

### JUCE

```bash
# Option 1: Projucer (GUI tool generates IDE projects)
# Option 2: CMake:
git clone https://github.com/juce-framework/JUCE.git
# In your CMakeLists.txt:
add_subdirectory(JUCE)
target_link_libraries(MyApp PRIVATE
    juce::juce_audio_devices
    juce::juce_audio_basics
    juce::juce_audio_utils)
```

---

*Last updated: 2026-03-13*
