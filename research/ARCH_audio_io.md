# ARCH_audio_io.md -- Cross-Platform Audio I/O Architecture

**Scope:** Deep technical reference for audio input/output subsystem design in a real-time audio analysis application targeting Windows, macOS, and Linux.

**Cross-references:** [ARCH_pipeline.md](ARCH_pipeline.md) | [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md) | [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md) | [LIB_juce.md](LIB_juce.md) | [REF_latency_numbers.md](REF_latency_numbers.md)

---

## 1. Platform-Specific Audio APIs

### 1.1 Windows

Windows exposes multiple audio API layers with radically different latency and capability profiles. Understanding the stack is essential for choosing the right approach.

#### WASAPI (Windows Audio Session API)

WASAPI is the modern native audio API on Windows (Vista+). It operates in two modes:

**Shared Mode:** Audio streams are mixed by the Windows Audio Engine (audiodg.exe). The application submits buffers to the audio engine, which resamples and mixes all streams before delivering them to the hardware. Minimum practical latency is ~10ms (typically 10--20ms depending on the audio engine's internal buffer period). The sample rate and format are dictated by the shared mode format configured in Sound Settings.

**Exclusive Mode:** The application bypasses the audio engine entirely and writes directly to the hardware endpoint buffer. This yields the lowest possible latency on Windows without third-party drivers -- often 3--5ms with well-written drivers. However, exclusive mode locks out all other audio applications from that device.

Key COM interfaces:

- `IMMDeviceEnumerator` -- Enumerates audio endpoints (render and capture). Use `EnumAudioEndpoints()` with `eRender`, `eCapture`, or `eAll`. Register `IMMNotificationClient` for hot-plug events.
- `IMMDevice` -- Represents a single endpoint. Call `Activate()` to obtain `IAudioClient`.
- `IAudioClient` / `IAudioClient3` -- Core interface for stream management. `Initialize()` sets up the stream with format, buffer size, and flags. `IAudioClient3` (Windows 10 1703+) adds `InitializeSharedAudioStream()` for low-latency shared mode with explicit periodicity control.
- `IAudioCaptureClient` / `IAudioRenderClient` -- Read captured audio packets or write render packets.

```cpp
// WASAPI shared-mode capture initialization (simplified, error handling omitted)
#include <mmdeviceapi.h>
#include <audioclient.h>

CoInitializeEx(nullptr, COINIT_MULTITHREADED);

IMMDeviceEnumerator* enumerator = nullptr;
CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                 __uuidof(IMMDeviceEnumerator), (void**)&enumerator);

IMMDevice* device = nullptr;
enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &device);

IAudioClient* audioClient = nullptr;
device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient);

WAVEFORMATEX* mixFormat = nullptr;
audioClient->GetMixFormat(&mixFormat);

// Shared mode, event-driven
REFERENCE_TIME bufferDuration = 10 * 10000; // 10ms in 100ns units
audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                        bufferDuration, 0, mixFormat, nullptr);

HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
audioClient->SetEventHandle(hEvent);

IAudioCaptureClient* captureClient = nullptr;
audioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&captureClient);

audioClient->Start();

// Audio capture loop (run on dedicated thread)
while (running) {
    WaitForSingleObject(hEvent, 2000);
    BYTE* data;
    UINT32 numFramesAvailable;
    DWORD flags;
    captureClient->GetBuffer(&data, &numFramesAvailable, &flags, nullptr, nullptr);
    if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
        // Handle silence -- buffer contains zeros
    } else {
        processAudioBuffer(data, numFramesAvailable, mixFormat);
    }
    captureClient->ReleaseBuffer(numFramesAvailable);
}
```

**Exclusive mode** replaces `AUDCLNT_SHAREMODE_SHARED` with `AUDCLNT_SHAREMODE_EXCLUSIVE` and requires negotiating an exact format the hardware supports via `IsFormatSupported()`. The periodicity parameter becomes meaningful -- set it equal to `bufferDuration` for the minimum buffer.

#### ASIO (Audio Stream Input/Output)

Steinberg's ASIO protocol provides kernel-bypass audio with latencies under 3ms. It communicates directly with the driver, skipping the entire Windows audio stack. ASIO is the de facto standard for professional audio on Windows.

Key characteristics:
- Single-client: only one application can use an ASIO device at a time.
- Fixed buffer sizes: the driver offers a set of supported buffer sizes. Common values: 64, 128, 256, 512 samples.
- Callback-driven: the driver invokes `bufferSwitch()` or `bufferSwitchTimeInfo()` on a high-priority thread.
- Sample-accurate timing via `ASIOTime` structures.

ASIO4ALL is a widely-used generic ASIO wrapper over WDM/KS drivers, providing ASIO-like latency to devices without native ASIO drivers.

```cpp
// ASIO pseudo-code (actual ASIO SDK required)
ASIOInit(&driverInfo);
ASIOGetChannels(&numInputs, &numOutputs);
ASIOGetBufferSize(&minSize, &maxSize, &preferredSize, &granularity);
ASIOCreateBuffers(bufferInfos, numChannels, preferredSize, &callbacks);
ASIOStart();

// The driver calls this on its own high-priority thread:
ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex,
                                ASIOBool directProcess) {
    for (int i = 0; i < numChannels; i++) {
        float* buf = (float*)bufferInfos[i].buffers[doubleBufferIndex];
        processChannel(i, buf, preferredSize);
    }
    ASIOOutputReady();
    return params;
}
```

#### DirectSound (Legacy)

DirectSound (dsound.dll) is the legacy audio API from DirectX. It uses a circular buffer model with play/write cursor management. Minimum latency is ~30--50ms. **Do not use for new projects.** It exists only for backward compatibility with Windows XP-era applications.

### 1.2 macOS

#### CoreAudio

CoreAudio is the umbrella term for Apple's audio infrastructure. It is a layered architecture:

**Audio HAL (Hardware Abstraction Layer):** The lowest public API layer. It exposes `AudioDevice`, `AudioStream`, and `AudioControl` objects. You interact via `AudioObjectGetPropertyData()` / `AudioObjectSetPropertyData()` using property selectors like `kAudioDevicePropertyStreamConfiguration`, `kAudioDevicePropertyBufferFrameSize`, `kAudioDevicePropertyNominalSampleRate`. The HAL supports registering property listeners for device change notifications.

**AudioUnit (Audio Units):** The workhorse API for real-time audio processing. An AudioUnit is a plugin-like processing node. `kAudioUnitSubType_HALOutput` (or `kAudioUnitSubType_DefaultOutput`) provides direct hardware I/O. Audio Units use an `AURenderCallback` invoked on the audio I/O thread (a real-time thread managed by CoreAudio).

```cpp
// CoreAudio AudioUnit capture setup
#include <AudioToolbox/AudioToolbox.h>

AudioComponentDescription desc = {
    .componentType = kAudioUnitType_Output,
    .componentSubType = kAudioUnitSubType_HALOutput,
    .componentManufacturer = kAudioUnitManufacturer_Apple
};

AudioComponent comp = AudioComponentFindNext(nullptr, &desc);
AudioUnit auHAL;
AudioComponentInstanceNew(comp, &auHAL);

// Enable input on the HAL unit (element 1 = input bus)
UInt32 enableIO = 1;
AudioUnitSetProperty(auHAL, kAudioOutputUnitProperty_EnableIO,
                     kAudioUnitScope_Input, 1, &enableIO, sizeof(enableIO));

// Disable output (element 0 = output bus) if capture-only
UInt32 disableIO = 0;
AudioUnitSetProperty(auHAL, kAudioOutputUnitProperty_EnableIO,
                     kAudioUnitScope_Output, 0, &disableIO, sizeof(disableIO));

// Set the device
AudioDeviceID inputDevice = /* obtained from enumeration */;
AudioUnitSetProperty(auHAL, kAudioOutputUnitProperty_CurrentDevice,
                     kAudioUnitScope_Global, 0, &inputDevice, sizeof(inputDevice));

// Set the render callback
AURenderCallbackStruct callbackStruct = { inputCallback, nullptr };
AudioUnitSetProperty(auHAL, kAudioOutputUnitProperty_SetInputCallback,
                     kAudioUnitScope_Global, 0, &callbackStruct, sizeof(callbackStruct));

AudioUnitInitialize(auHAL);
AudioOutputUnitStart(auHAL);

// Callback -- runs on CoreAudio's real-time I/O thread
OSStatus inputCallback(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags,
                       const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
                       UInt32 inNumberFrames, AudioBufferList* ioData) {
    AudioBufferList bufferList;
    bufferList.mNumberBuffers = 1;
    bufferList.mBuffers[0].mNumberChannels = 2;
    bufferList.mBuffers[0].mDataByteSize = inNumberFrames * sizeof(float) * 2;
    bufferList.mBuffers[0].mData = malloc(bufferList.mBuffers[0].mDataByteSize);

    AudioUnitRender(auHAL, ioActionFlags, inTimeStamp, 1, inNumberFrames, &bufferList);
    processAudio((float*)bufferList.mBuffers[0].mData, inNumberFrames);
    free(bufferList.mBuffers[0].mData);
    return noErr;
}
```

**AudioQueue:** Higher-level buffer-queue API. Simpler than AudioUnit but higher latency. Suitable for playback and recording where sub-10ms latency is not required. Enqueue buffers; the system calls your callback when a buffer is consumed.

**AVAudioEngine:** Objective-C/Swift API wrapping AudioUnit graphs. Provides `AVAudioInputNode`, `AVAudioOutputNode`, and `AVAudioMixerNode`. The `installTap(onBus:bufferSize:format:block:)` method is the simplest way to capture audio in Swift/ObjC. Good for rapid prototyping but adds a layer of abstraction and overhead versus raw AudioUnit.

#### Aggregate Devices

macOS allows creating aggregate devices that combine multiple physical audio interfaces into a single logical device. Created via `AudioHardwareCreateAggregateDevice()` or the Audio MIDI Setup utility. This is essential for combining a USB microphone with a built-in output, or for combining a virtual loopback device with a hardware input. Drift correction must be enabled when combining devices with independent clocks -- CoreAudio uses a sample rate converter to compensate.

### 1.3 Linux

Linux audio is notoriously fragmented. The stack from bottom to top:

#### ALSA (Advanced Linux Sound Architecture)

ALSA is the kernel-level audio interface. It provides direct access to audio hardware via device files (`/dev/snd/*`). The userspace library `libasound` (`alsa-lib`) wraps the kernel interface.

Key concepts:
- **hw:X,Y** -- Direct hardware access (card X, device Y). Minimal latency but no mixing.
- **plughw:X,Y** -- Adds automatic format/rate conversion.
- **default** -- Usually routes through a higher-level server (PulseAudio/PipeWire).
- Period/buffer size configured via `snd_pcm_hw_params_set_period_size()` and `snd_pcm_hw_params_set_buffer_size()`.

```cpp
#include <alsa/asoundlib.h>

snd_pcm_t* capture;
snd_pcm_open(&capture, "hw:0,0", SND_PCM_STREAM_CAPTURE, 0);

snd_pcm_hw_params_t* hwparams;
snd_pcm_hw_params_alloca(&hwparams);
snd_pcm_hw_params_any(capture, hwparams);
snd_pcm_hw_params_set_access(capture, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
snd_pcm_hw_params_set_format(capture, hwparams, SND_PCM_FORMAT_FLOAT_LE);
snd_pcm_hw_params_set_channels(capture, hwparams, 2);
unsigned int sampleRate = 48000;
snd_pcm_hw_params_set_rate_near(capture, hwparams, &sampleRate, nullptr);
snd_pcm_uframes_t periodSize = 256;
snd_pcm_hw_params_set_period_size_near(capture, hwparams, &periodSize, nullptr);
snd_pcm_hw_params(capture, hwparams);

snd_pcm_prepare(capture);

// Capture loop
std::vector<float> buffer(periodSize * 2); // stereo
while (running) {
    snd_pcm_sframes_t frames = snd_pcm_readi(capture, buffer.data(), periodSize);
    if (frames < 0) {
        snd_pcm_recover(capture, frames, 0); // handle xrun
    } else {
        processAudio(buffer.data(), frames);
    }
}
```

#### PulseAudio

PulseAudio is a sound server that sits above ALSA, providing per-application volume control, network audio, automatic device switching, and audio routing. It adds ~20--40ms of latency due to internal buffering.

For our purposes, PulseAudio's most important feature is **monitor sources** -- every sink (output) has a corresponding `.monitor` source that captures what is being played, enabling loopback capture without special drivers (see Section 2.3).

The PulseAudio `simple` API (`pa_simple`) is trivial to use but offers no control over latency. The `async` API (`pa_context`, `pa_stream`) provides full control but requires a `pa_mainloop` integration.

#### PipeWire

PipeWire is the modern replacement for both PulseAudio and JACK on Linux (default on Fedora 34+, Ubuntu 22.10+). It provides:
- PulseAudio compatibility (drop-in replacement via `pipewire-pulse`).
- JACK compatibility (drop-in replacement via `pipewire-jack`).
- Low-latency operation comparable to JACK.
- Video stream handling (used by screen sharing in Wayland).
- Session/policy management via WirePlumber.

PipeWire uses a graph-based processing model. Audio nodes are connected via links. Buffer sizes are negotiated per-link and can be as low as 16 frames (sub-millisecond at 48kHz).

#### JACK (JACK Audio Connection Kit)

JACK provides professional-grade low-latency audio on Linux (and macOS/Windows). It enforces a fixed buffer size and sample rate across all connected clients. Typical JACK setups achieve 2--5ms round-trip latency.

JACK uses a callback model where the server invokes `jack_process_callback` on a real-time thread. Clients register ports and connect them via the JACK graph.

```cpp
#include <jack/jack.h>

jack_client_t* client = jack_client_open("analyzer", JackNullOption, nullptr);
jack_port_t* inputPort = jack_port_register(client, "input",
                                             JACK_DEFAULT_AUDIO_TYPE,
                                             JackPortIsInput, 0);

int processCallback(jack_nframes_t nframes, void* arg) {
    float* in = (float*)jack_port_get_buffer(inputPort, nframes);
    processAudio(in, nframes);
    return 0;
}

jack_set_process_callback(client, processCallback, nullptr);
jack_activate(client);

// Auto-connect to system capture ports
const char** ports = jack_get_ports(client, nullptr, nullptr,
                                     JackPortIsPhysical | JackPortIsOutput);
if (ports) {
    jack_connect(client, ports[0], jack_port_name(inputPort));
    jack_free(ports);
}
```

---

## 2. Loopback Audio Capture

Loopback capture -- intercepting the system's audio output -- is a key requirement for a real-time analysis application that should be able to analyze any audio playing on the system.

### 2.1 Windows: WASAPI Loopback

WASAPI provides native loopback capture via the `AUDCLNT_STREAMFLAGS_LOOPBACK` flag. This captures the mix of all audio being rendered to a specific output endpoint.

Implementation details:
- Open a **render** endpoint (not capture) with `eRender`.
- Initialize with `AUDCLNT_STREAMFLAGS_LOOPBACK`. This tells WASAPI to give you the post-mix audio from that render endpoint.
- The format is dictated by the endpoint's shared mode format.
- If no audio is playing, the capture client returns silence-flagged buffers.
- You cannot capture exclusive-mode streams from other applications (they bypass the mixer).

```cpp
// WASAPI Loopback capture
IMMDevice* renderDevice = nullptr;
enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &renderDevice);

IAudioClient* loopbackClient = nullptr;
renderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
                       nullptr, (void**)&loopbackClient);

WAVEFORMATEX* mixFormat = nullptr;
loopbackClient->GetMixFormat(&mixFormat);

// Key: LOOPBACK flag on a render device
loopbackClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
    10 * 10000, // 10ms buffer
    0,
    mixFormat,
    nullptr
);

HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
loopbackClient->SetEventHandle(hEvent);

IAudioCaptureClient* captureClient = nullptr;
loopbackClient->GetService(__uuidof(IAudioCaptureClient), (void**)&captureClient);

loopbackClient->Start();

// Capture thread
while (running) {
    WaitForSingleObject(hEvent, 2000);
    BYTE* data;
    UINT32 frames;
    DWORD flags;
    while (SUCCEEDED(captureClient->GetBuffer(&data, &frames, &flags, nullptr, nullptr))) {
        if (frames == 0) break;
        bool silent = (flags & AUDCLNT_BUFFERFLAGS_SILENT);
        if (!silent) {
            // data is float32 interleaved at mixFormat->nSamplesPerSec
            processLoopbackAudio((float*)data, frames, mixFormat->nChannels);
        }
        captureClient->ReleaseBuffer(frames);
    }
}
```

**Important caveats:**
- Loopback capture only works in shared mode.
- The captured audio has already been through the Windows audio engine's processing (volume, spatial audio, etc.).
- `IAudioClient3::InitializeSharedAudioStream()` does NOT support the loopback flag on all driver versions.

### 2.2 macOS: Virtual Audio Devices and ScreenCaptureKit

macOS has no native loopback capture API equivalent to WASAPI loopback. Three approaches exist:

#### BlackHole (Recommended)

[BlackHole](https://github.com/ExistentialAudio/BlackHole) is an open-source (MIT) virtual audio driver that creates a zero-latency pass-through device. Install it, then create an aggregate device combining BlackHole with the physical output. Set the aggregate device as the system output. Now BlackHole appears as a capture source containing all system audio.

Programmatic setup:
1. Enumerate devices via `AudioObjectGetPropertyData` with `kAudioHardwarePropertyDevices`.
2. Find the BlackHole device by matching the device name or UID.
3. Create an aggregate device combining BlackHole and the desired output.
4. Set the aggregate as the default output.
5. Open a capture stream on the BlackHole device.

#### ScreenCaptureKit (macOS 13+)

Starting with macOS 13 (Ventura), `SCStreamConfiguration` supports audio-only capture of system audio without needing a virtual device. This is the cleanest approach on supported macOS versions.

```objc
// ScreenCaptureKit audio-only capture (Objective-C, macOS 13+)
SCStreamConfiguration *config = [[SCStreamConfiguration alloc] init];
config.capturesAudio = YES;
config.excludesCurrentProcessAudio = NO; // Include own audio if desired
config.sampleRate = 48000;
config.channelCount = 2;

// Capture entire display audio (not a specific window)
[SCShareableContent getShareableContentWithCompletionHandler:^(SCShareableContent *content, NSError *error) {
    SCDisplay *mainDisplay = content.displays.firstObject;
    SCContentFilter *filter = [[SCContentFilter alloc] initWithDisplay:mainDisplay
                                                      excludingWindows:@[]];
    SCStream *stream = [[SCStream alloc] initWithFilter:filter
                                          configuration:config
                                               delegate:self];
    [stream addStreamOutput:self type:SCStreamOutputTypeAudio
               sampleHandlerQueue:audioQueue error:nil];
    [stream startCaptureWithCompletionHandler:^(NSError *error) { /* started */ }];
}];

// SCStreamOutput delegate
- (void)stream:(SCStream *)stream
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
                   ofType:(SCStreamOutputType)type {
    if (type == SCStreamOutputTypeAudio) {
        CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
        size_t length;
        char *dataPointer;
        CMBlockBufferGetDataPointer(blockBuffer, 0, nullptr, &length, &dataPointer);
        // dataPointer contains Float32 interleaved PCM
        processAudio((float*)dataPointer, length / sizeof(float) / 2);
    }
}
```

**Permissions:** ScreenCaptureKit requires screen recording permission in System Preferences. The user will see a system prompt on first use.

#### Soundflower (Legacy)

Soundflower was the original macOS virtual audio device. It is no longer maintained and does not work reliably on macOS 11+. Replaced by BlackHole.

### 2.3 Linux: PulseAudio Monitor Sources / PipeWire

#### PulseAudio Monitor Sources

Every PulseAudio sink has an automatically created monitor source. List them:

```bash
pactl list sources short
# Output includes entries like:
# 0  alsa_output.pci-0000_00_1f.3.analog-stereo.monitor  ...
```

Capture from the monitor source to get loopback audio:

```cpp
#include <pulse/simple.h>

pa_sample_spec spec = {
    .format = PA_SAMPLE_FLOAT32LE,
    .rate = 48000,
    .channels = 2
};

// The ".monitor" source name captures the sink's output
pa_simple* s = pa_simple_new(nullptr, "AudioAnalyzer", PA_STREAM_RECORD,
                              "alsa_output.pci-0000_00_1f.3.analog-stereo.monitor",
                              "loopback", &spec, nullptr, nullptr, nullptr);

std::vector<float> buffer(1024);
while (running) {
    pa_simple_read(s, buffer.data(), buffer.size() * sizeof(float), nullptr);
    processAudio(buffer.data(), buffer.size() / spec.channels);
}
```

#### PipeWire Loopback

PipeWire exposes the same monitor sources via its PulseAudio compatibility layer. Additionally, you can use `pw-loopback` to create explicit loopback nodes, or use the PipeWire native API (`libpipewire`) to create a stream that connects to any node in the graph.

```bash
# Create a loopback capture from default output
pw-loopback --capture-props="media.class=Audio/Sink" \
            --playback-props="media.class=Audio/Source"
```

---

## 3. Library Comparison

### 3.1 Feature Matrix

| Feature | PortAudio | RtAudio | miniaudio | JUCE | libsoundio |
|---|---|---|---|---|---|
| **License** | MIT | MIT-like | MIT-0 (public domain) | GPLv3 / Commercial | MIT |
| **Windows backends** | WASAPI, MME, DS, ASIO | WASAPI, ASIO, DS | WASAPI | WASAPI, ASIO, DS | WASAPI |
| **macOS backends** | CoreAudio | CoreAudio | CoreAudio | CoreAudio | CoreAudio |
| **Linux backends** | ALSA, JACK, OSS | ALSA, JACK, PulseAudio | ALSA, PulseAudio, JACK | ALSA, JACK | ALSA, PulseAudio, JACK |
| **Loopback support** | No (manual WASAPI) | No | Yes (built-in) | No (manual WASAPI) | No |
| **API style** | C callback | C++ callback | C callback / polling | C++ callback + GUI | C callback |
| **Header-only** | No | No | Yes (single file) | No (framework) | No |
| **Device hot-plug** | Limited | Limited | Yes | Yes | Yes |
| **Maintenance (2025)** | Active | Moderate | Very active | Very active | Dormant |
| **Duplex (simultaneous I/O)** | Yes | Yes | Yes | Yes | Yes |
| **Resampling** | No | No | Built-in | Built-in | No |
| **Min practical latency** | ~5ms | ~5ms | ~5ms | ~3ms (ASIO) | ~5ms |
| **Build complexity** | cmake | cmake | Copy 1 file | Large framework | cmake |

### 3.2 Code Examples

#### PortAudio

```cpp
#include <portaudio.h>

static int paCallback(const void* input, void* output,
                      unsigned long frameCount,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags, void* userData) {
    const float* in = (const float*)input;
    processAudio(in, frameCount);
    return paContinue;
}

Pa_Initialize();

PaStreamParameters inputParams;
inputParams.device = Pa_GetDefaultInputDevice();
inputParams.channelCount = 2;
inputParams.sampleFormat = paFloat32;
inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
inputParams.hostApiSpecificStreamInfo = nullptr;

PaStream* stream;
Pa_OpenStream(&stream, &inputParams, nullptr, 48000, 256, paClipOff, paCallback, nullptr);
Pa_StartStream(stream);
```

#### RtAudio

```cpp
#include <RtAudio.h>

int rtCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames,
               double streamTime, RtAudioStreamStatus status, void* userData) {
    float* in = (float*)inputBuffer;
    processAudio(in, nFrames);
    return 0;
}

RtAudio adc;
RtAudio::StreamParameters params;
params.deviceId = adc.getDefaultInputDevice();
params.nChannels = 2;

unsigned int bufferFrames = 256;
adc.openStream(nullptr, &params, RTAUDIO_FLOAT32, 48000, &bufferFrames, &rtCallback);
adc.startStream();
```

#### miniaudio

```cpp
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    const float* in = (const float*)pInput;
    processAudio(in, frameCount);
}

ma_device_config config = ma_device_config_init(ma_device_type_capture);
config.capture.format = ma_format_f32;
config.capture.channels = 2;
config.sampleRate = 48000;
config.periodSizeInFrames = 256;
config.dataCallback = dataCallback;

ma_device device;
ma_device_init(nullptr, &config, &device);
ma_device_start(&device);
```

miniaudio also supports loopback capture natively on Windows:

```cpp
ma_device_config config = ma_device_config_init(ma_device_type_loopback);
config.capture.format = ma_format_f32;
config.capture.channels = 2;
config.sampleRate = 48000;
config.dataCallback = loopbackCallback;
// Uses WASAPI loopback internally on Windows
```

#### JUCE

```cpp
class AudioAnalyzer : public juce::AudioIODeviceCallback {
public:
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& ctx) override {
        processAudio(inputChannelData, numInputChannels, numSamples);
    }
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {}
    void audioDeviceStopped() override {}
};

juce::AudioDeviceManager deviceManager;
deviceManager.initialiseWithDefaultDevices(2, 0); // 2 inputs, 0 outputs

AudioAnalyzer analyzer;
deviceManager.addAudioCallback(&analyzer);
```

#### libsoundio

```cpp
#include <soundio/soundio.h>

void readCallback(struct SoundIoInStream* instream, int frameCountMin, int frameCountMax) {
    struct SoundIoChannelArea* areas;
    int frameCount = frameCountMax;
    soundio_instream_begin_read(instream, &areas, &frameCount);
    // areas[ch].ptr + areas[ch].step * frame gives sample data
    processAudioFromAreas(areas, instream->layout.channel_count, frameCount);
    soundio_instream_end_read(instream);
}

struct SoundIo* soundio = soundio_create();
soundio_connect(soundio);
soundio_flush_events(soundio);

struct SoundIoDevice* device = soundio_get_input_device(soundio,
                                 soundio_default_input_device_index(soundio));

struct SoundIoInStream* instream = soundio_instream_create(device);
instream->format = SoundIoFormatFloat32LE;
instream->sample_rate = 48000;
instream->read_callback = readCallback;

soundio_instream_open(instream);
soundio_instream_start(instream);
```

### 3.3 Recommendations by Use Case

| Use Case | Recommended Library | Rationale |
|---|---|---|
| **Lightweight analysis tool** | miniaudio | Single header, built-in loopback on Windows, built-in resampling, minimal dependencies |
| **Professional audio application** | JUCE | Complete framework with GUI, plugin hosting, ASIO support, extensive ecosystem |
| **Cross-platform with JACK** | RtAudio or PortAudio | Mature JACK backends, well-tested in production |
| **Embedded / resource-constrained** | miniaudio | Zero dependencies, tiny footprint, public domain |
| **Maximum control** | Platform-native APIs | When you need features no library exposes (WASAPI exclusive, CoreAudio aggregate devices) |

**For this project (real-time audio analysis):** miniaudio is the strongest candidate. It provides built-in loopback capture on Windows, handles device enumeration and hot-plugging, supports all three platforms, requires no build system integration (single header), and is under very active development. For macOS loopback, supplement with ScreenCaptureKit (macOS 13+) or BlackHole.

---

## 4. Buffer Sizes and Sample Rates

### 4.1 Latency by Buffer Size

The latency introduced by an audio buffer is:

```
latency_ms = (buffer_size_samples / sample_rate) * 1000
```

| Buffer Size (samples) | Latency @ 44.1kHz | Latency @ 48kHz | Typical Use Case |
|---|---|---|---|
| 32 | 0.73ms | 0.67ms | Ultra-low-latency monitoring (ASIO/JACK only) |
| 64 | 1.45ms | 1.33ms | Professional live monitoring |
| 128 | 2.90ms | 2.67ms | Low-latency recording/performance |
| 256 | 5.80ms | 5.33ms | General-purpose low-latency |
| 512 | 11.61ms | 10.67ms | Default for many consumer interfaces |
| 1024 | 23.22ms | 21.33ms | Analysis / non-interactive |
| 2048 | 46.44ms | 42.67ms | High-stability batch processing |

Note: Total round-trip latency is typically 2x the buffer latency (one buffer in, one buffer out) plus hardware/driver overhead (typically 1--3ms additional).

### 4.2 Buffer Size and FFT Analysis

For frequency-domain analysis, the FFT window size determines frequency resolution:

```
frequency_resolution = sample_rate / fft_size
```

At 48kHz with a 2048-point FFT: resolution = 48000 / 2048 = 23.4 Hz per bin.
At 48kHz with a 4096-point FFT: resolution = 48000 / 4096 = 11.7 Hz per bin.

**The audio callback buffer size and FFT size are independent.** A common pattern is to use a small callback buffer (e.g., 256 samples for low latency) and accumulate samples in a ring buffer until enough are available for the desired FFT window. This decouples I/O latency from analysis resolution.

```
Callback buffer: 256 samples @ 48kHz = 5.33ms latency
FFT window: 4096 samples = accumulate 16 callbacks worth
Analysis update rate: every 256 samples = ~187.5 Hz (every 5.33ms)
Frequency resolution: 11.7 Hz per bin
```

For overlapping FFT windows (standard practice -- 50% or 75% overlap), hop size equals `fft_size / overlap_factor`. With 4096 FFT and 75% overlap, hop = 1024. Analysis updates every 1024 samples (21.3ms at 48kHz).

### 4.3 Optimal Sample Rates for Music Analysis

| Sample Rate | Nyquist Limit | Notes |
|---|---|---|
| 44100 Hz | 22050 Hz | CD standard. Sufficient for all audible analysis. |
| 48000 Hz | 24000 Hz | Professional video standard. Slightly better high-frequency resolution. Most hardware defaults to this. **Recommended.** |
| 96000 Hz | 48000 Hz | Overkill for analysis. Doubles CPU load for inaudible benefit. Only useful for ultrasonic analysis. |

**Recommendation:** Use 48kHz. It is the most universally supported native rate on modern hardware, avoids resampling on most devices, and provides adequate bandwidth for any music analysis task.

---

## 5. Bit Depths and Sample Formats

### 5.1 Format Characteristics

| Format | Range | Dynamic Range | Bytes/Sample | Notes |
|---|---|---|---|---|
| Int16 (PCM) | -32768 to 32767 | 96 dB | 2 | CD quality. Adequate for playback, insufficient for processing headroom. |
| Int24 (PCM) | -8388608 to 8388607 | 144 dB | 3 | Professional recording standard. Awkward alignment (3 bytes). |
| Int32 (PCM) | -2147483648 to 2147483647 | 192 dB | 4 | Rare in hardware. Sometimes used as a container for 24-bit samples (left-justified). |
| Float32 (IEEE 754) | -1.0 to +1.0 (nominal) | ~150 dB (within 0..1 range) | 4 | **Processing standard.** Can exceed 0 dBFS without clipping (values > 1.0 are valid). |
| Float64 (IEEE 754) | -1.0 to +1.0 (nominal) | ~300 dB | 8 | Scientific/mastering use only. 2x memory/bandwidth for negligible audible benefit. |

### 5.2 Format Conversion

```cpp
// Int16 to Float32
inline float int16ToFloat(int16_t sample) {
    return sample / 32768.0f;
}

// Float32 to Int16 (with clamping)
inline int16_t floatToInt16(float sample) {
    sample = std::clamp(sample, -1.0f, 1.0f);
    return static_cast<int16_t>(sample * 32767.0f);
}

// Int24 (packed 3-byte) to Float32
inline float int24ToFloat(const uint8_t* bytes) {
    int32_t value = (bytes[0]) | (bytes[1] << 8) | (static_cast<int8_t>(bytes[2]) << 16);
    return value / 8388608.0f;
}

// Int32 to Float32
inline float int32ToFloat(int32_t sample) {
    return sample / 2147483648.0f;
}
```

### 5.3 Why Float32 Is the Processing Standard

1. **Headroom:** Float32 naturally handles values outside the nominal -1.0 to 1.0 range. Intermediate processing stages (EQ, FFT, summing) routinely produce values exceeding 0 dBFS. Integer formats clip silently; float preserves the signal.

2. **Uniformity:** The quantization step is relative to magnitude, not fixed. Near zero, float32 has finer resolution than int24. Near full scale, it has comparable resolution to int24.

3. **CPU efficiency:** Modern CPUs (SSE, AVX, NEON) have dedicated SIMD instructions for float32 arithmetic. Float32 FFT (via FFTW, pffft, KFR) is highly optimized on all platforms.

4. **API convergence:** WASAPI shared mode natively uses float32. CoreAudio defaults to float32. ALSA supports float32 natively on most hardware. All audio libraries listed in Section 3 default to float32.

5. **No conversion overhead:** Receiving float32 from the audio API and processing in float32 eliminates format conversion entirely.

---

## 6. Multi-Device and Virtual Audio

### 6.1 Aggregate Devices (macOS)

macOS aggregate devices combine multiple audio interfaces into one logical device. This is the canonical way to use a virtual loopback device (e.g., BlackHole) alongside physical hardware.

```cpp
// Programmatic aggregate device creation (macOS)
#include <CoreAudio/CoreAudio.h>

AudioObjectID createAggregateDevice(AudioDeviceID device1, AudioDeviceID device2) {
    CFMutableDictionaryRef aggDesc = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    // Unique UID for the aggregate
    CFStringRef aggUID = CFSTR("com.myapp.aggregate");
    CFDictionarySetValue(aggDesc, CFSTR(kAudioAggregateDeviceUIDKey), aggUID);
    CFDictionarySetValue(aggDesc, CFSTR(kAudioAggregateDeviceNameKey),
                         CFSTR("Analysis Aggregate"));

    // Sub-device list
    CFMutableArrayRef subDevices = CFArrayCreateMutable(kCFAllocatorDefault, 0,
                                                        &kCFTypeArrayCallBacks);

    // Add sub-device dictionaries (each needs UID string of the physical device)
    CFStringRef uid1 = getDeviceUID(device1); // helper to get kAudioDevicePropertyDeviceUID
    CFStringRef uid2 = getDeviceUID(device2);

    CFMutableDictionaryRef sub1 = CFDictionaryCreateMutable(/*...*/);
    CFDictionarySetValue(sub1, CFSTR(kAudioSubDeviceUIDKey), uid1);
    CFArrayAppendValue(subDevices, sub1);

    CFMutableDictionaryRef sub2 = CFDictionaryCreateMutable(/*...*/);
    CFDictionarySetValue(sub2, CFSTR(kAudioSubDeviceUIDKey), uid2);
    CFArrayAppendValue(subDevices, sub2);

    CFDictionarySetValue(aggDesc, CFSTR(kAudioAggregateDeviceSubDeviceListKey), subDevices);

    // Enable drift correction for the non-clock-master device
    CFDictionarySetValue(aggDesc, CFSTR(kAudioAggregateDeviceMasterSubDeviceKey), uid1);

    AudioObjectID aggDevice;
    UInt32 size = sizeof(aggDevice);
    AudioObjectID systemObj = kAudioObjectSystemObject;
    AudioObjectSetPropertyData(systemObj,
        &(AudioObjectPropertyAddress){kAudioHardwarePropertyPlugInForBundleID, /*...*/},
        /*qualifier*/ 0, nullptr, sizeof(aggDesc), &aggDesc);

    // Use AudioHardwareCreateAggregateDevice (simpler, macOS 10.11+)
    OSStatus status = AudioHardwareCreateAggregateDevice(aggDesc, &aggDevice);
    // aggDevice is now a valid AudioDeviceID combining both sub-devices
    return aggDevice;
}
```

### 6.2 ASIO Multi-Device

ASIO is inherently single-device. Only one ASIO driver can be active at a time. Workarounds:

- **ASIO multi-client drivers:** Some interfaces (RME, MOTU) support multiple ASIO clients.
- **Virtual ASIO aggregation:** Tools like VoiceMeeter can present multiple devices as a single ASIO device.
- **Fallback to WASAPI:** For multi-device scenarios on Windows, use WASAPI (which supports opening multiple devices simultaneously) and reserve ASIO for the primary low-latency interface.

### 6.3 Virtual Audio Routing

| Tool | Platform | License | Channels | Notes |
|---|---|---|---|---|
| BlackHole | macOS | MIT | 2, 16, or 64 ch | Zero-latency, kernel extension (kext) or DriverKit |
| VB-Cable / VoiceMeeter | Windows | Donationware | 2--8 ch | Virtual cable + mixer. VoiceMeeter adds routing matrix |
| JACK | Linux/macOS/Win | LGPL | Arbitrary | Full routing graph, pro-grade, requires JACK server |
| PipeWire | Linux | MIT | Arbitrary | Native graph routing, replaces JACK/PulseAudio |
| Loopback (Rogue Amoeba) | macOS | Commercial | Up to 64 ch | Most flexible macOS routing, per-app capture |

---

## 7. Device Enumeration and Hot-Plugging

### 7.1 Enumeration

#### Windows (WASAPI)

```cpp
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

void enumerateDevices() {
    IMMDeviceEnumerator* enumerator;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                     __uuidof(IMMDeviceEnumerator), (void**)&enumerator);

    IMMDeviceCollection* collection;
    enumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &collection);

    UINT count;
    collection->GetCount(&count);

    for (UINT i = 0; i < count; i++) {
        IMMDevice* device;
        collection->Item(i, &device);

        LPWSTR deviceId;
        device->GetId(&deviceId);

        IPropertyStore* props;
        device->OpenPropertyStore(STGM_READ, &props);

        PROPVARIANT varName;
        PropVariantInit(&varName);
        props->GetValue(PKEY_Device_FriendlyName, &varName);

        wprintf(L"Device %u: %s (ID: %s)\n", i, varName.pwszVal, deviceId);

        PropVariantClear(&varName);
        props->Release();
        CoTaskMemFree(deviceId);
        device->Release();
    }
    collection->Release();
    enumerator->Release();
}
```

#### macOS (CoreAudio)

```cpp
#include <CoreAudio/CoreAudio.h>

void enumerateDevices() {
    AudioObjectPropertyAddress prop = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    UInt32 dataSize = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &prop, 0, nullptr, &dataSize);

    int deviceCount = dataSize / sizeof(AudioDeviceID);
    std::vector<AudioDeviceID> devices(deviceCount);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &prop, 0, nullptr,
                               &dataSize, devices.data());

    for (auto deviceId : devices) {
        CFStringRef name = nullptr;
        UInt32 nameSize = sizeof(name);
        AudioObjectPropertyAddress nameProp = {
            kAudioDevicePropertyDeviceNameCFString,
            kAudioObjectPropertyScopeGlobal,
            kAudioObjectPropertyElementMain
        };
        AudioObjectGetPropertyData(deviceId, &nameProp, 0, nullptr, &nameSize, &name);

        char nameBuf[256];
        CFStringGetCString(name, nameBuf, sizeof(nameBuf), kCFStringEncodingUTF8);
        printf("Device: %s (ID: %u)\n", nameBuf, deviceId);
        CFRelease(name);
    }
}
```

#### Linux (ALSA)

```cpp
#include <alsa/asoundlib.h>

void enumerateDevices() {
    int card = -1;
    while (snd_card_next(&card) == 0 && card >= 0) {
        char* name;
        snd_card_get_name(card, &name);

        char hwName[32];
        snprintf(hwName, sizeof(hwName), "hw:%d", card);

        snd_ctl_t* ctl;
        snd_ctl_open(&ctl, hwName, 0);

        int device = -1;
        while (snd_ctl_pcm_next_device(ctl, &device) == 0 && device >= 0) {
            snd_pcm_info_t* pcmInfo;
            snd_pcm_info_alloca(&pcmInfo);
            snd_pcm_info_set_device(pcmInfo, device);
            snd_pcm_info_set_subdevice(pcmInfo, 0);
            snd_pcm_info_set_stream(pcmInfo, SND_PCM_STREAM_CAPTURE);

            if (snd_ctl_pcm_info(ctl, pcmInfo) == 0) {
                printf("Card %d (%s), Device %d: %s [CAPTURE]\n",
                       card, name, device, snd_pcm_info_get_name(pcmInfo));
            }
        }
        snd_ctl_close(ctl);
        free(name);
    }
}
```

### 7.2 Hot-Plug Detection

#### Windows

Register an `IMMNotificationClient` with the device enumerator:

```cpp
class DeviceNotification : public IMMNotificationClient {
    // IUnknown methods (AddRef, Release, QueryInterface) ...

    HRESULT OnDeviceAdded(LPCWSTR deviceId) override {
        // New device available -- refresh device list
        return S_OK;
    }

    HRESULT OnDeviceRemoved(LPCWSTR deviceId) override {
        // Device gone -- if it was our active device, switch to default
        return S_OK;
    }

    HRESULT OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR deviceId) override {
        // Default device changed -- optionally follow the default
        return S_OK;
    }

    HRESULT OnDeviceStateChanged(LPCWSTR deviceId, DWORD newState) override { return S_OK; }
    HRESULT OnPropertyValueChanged(LPCWSTR deviceId, const PROPERTYKEY key) override { return S_OK; }
};

DeviceNotification notifier;
enumerator->RegisterEndpointNotificationCallback(&notifier);
```

#### macOS

Register a property listener on the system object:

```cpp
AudioObjectPropertyAddress devicesAddr = {
    kAudioHardwarePropertyDevices,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMain
};

AudioObjectAddPropertyListener(kAudioObjectSystemObject, &devicesAddr,
    [](AudioObjectID objectID, UInt32 numberAddresses,
       const AudioObjectPropertyAddress* addresses, void* clientData) -> OSStatus {
        // Device list changed -- re-enumerate
        return noErr;
    }, nullptr);

// Also listen for default device changes:
AudioObjectPropertyAddress defaultAddr = {
    kAudioHardwarePropertyDefaultInputDevice,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMain
};
AudioObjectAddPropertyListener(kAudioObjectSystemObject, &defaultAddr,
                               deviceChangeCallback, nullptr);
```

#### Linux (PipeWire/PulseAudio)

PulseAudio: use `pa_context_subscribe()` with `PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE` and a `pa_context_subscribe_cb_t` callback.

PipeWire: register a `spa_hook` on the registry to receive `global` and `global_remove` events.

### 7.3 Graceful Disconnection Handling

When the active audio device is disconnected mid-stream, the application must:

1. **Detect the event** via the notification mechanisms above.
2. **Stop the current stream** cleanly. On WASAPI, the stream will start returning `AUDCLNT_E_DEVICE_INVALIDATED`. On CoreAudio, the HAL sends a `kAudioDevicePropertyDeviceIsAlive` notification with value 0.
3. **Drain any in-flight buffers** -- do not process partial data from a dying device.
4. **Switch to the new default device** or prompt the user to select one.
5. **Restart the stream** with the new device's native format and capabilities.

A robust implementation wraps this in a state machine:

```
IDLE -> OPENING -> RUNNING -> DEVICE_LOST -> REOPENING -> RUNNING
                     |                          |
                     +-- STOPPING -> IDLE       +-- FAILED -> IDLE
```

Key design principle: the audio processing pipeline should be device-agnostic. It receives float32 buffers at a known sample rate. The I/O layer handles all device management, format conversion, and resampling. When a device is lost and replaced, the pipeline continues uninterrupted -- only the I/O layer restarts.

---

## 8. Architecture Recommendation Summary

For a cross-platform real-time audio analysis application:

1. **Primary library:** miniaudio for device I/O. It covers 90% of needs with zero dependency overhead. Use its built-in loopback on Windows.

2. **macOS loopback:** Implement ScreenCaptureKit audio capture for macOS 13+. Fall back to requiring BlackHole for macOS 12 and earlier.

3. **Linux loopback:** Connect to PulseAudio/PipeWire monitor sources. miniaudio supports this via its PulseAudio backend -- specify the monitor source name as the device.

4. **Buffer strategy:** 256-sample callback buffer at 48kHz (5.33ms latency). Accumulate into a lock-free ring buffer. FFT consumer reads 4096-sample windows with 75% overlap from the ring buffer.

5. **Sample format:** Float32 throughout. No format conversion in the hot path.

6. **Device management:** Implement hot-plug listeners per-platform. Wrap device I/O in a state machine that can restart transparently.

7. **Platform-specific fallback:** If miniaudio proves insufficient for a specific platform feature (e.g., ASIO exclusive mode on Windows, aggregate device creation on macOS), drop to the native API for that feature only. Keep the abstraction boundary clean.

See [ARCH_pipeline.md](ARCH_pipeline.md) for how audio buffers flow from I/O into the analysis pipeline, and [REF_latency_numbers.md](REF_latency_numbers.md) for measured latency benchmarks across platforms and configurations.
