# LIB_juce.md -- JUCE Framework Technical Reference

> **Scope**: Complete technical reference for using JUCE 7.x/8.x to build a real-time audio analysis and visual rendering application (VJ tool). Covers audio I/O, DSP, graph-based processing, OpenGL integration, build systems, licensing, and integration with external analysis libraries.
>
> **Cross-references**: [LIB_essentia.md](LIB_essentia.md), [LIB_aubio.md](LIB_aubio.md), [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md), [ARCH_pipeline.md](ARCH_pipeline.md), [ARCH_audio_io.md](ARCH_audio_io.md), [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md)

---

## 1. Audio I/O in JUCE

JUCE provides a high-level abstraction over platform-native audio APIs through `AudioDeviceManager`. This is the central object that owns the audio device lifecycle, manages device enumeration, and dispatches audio callbacks.

### 1.1 AudioDeviceManager

`AudioDeviceManager` wraps device discovery, configuration persistence, and callback routing. It is not itself real-time -- it lives on the message thread and configures the underlying `AudioIODevice`.

```cpp
#include <juce_audio_devices/juce_audio_devices.h>

class AudioApp : public juce::JUCEApplication
{
    juce::AudioDeviceManager deviceManager;

public:
    void initialise(const juce::String&) override
    {
        // Initialise with default stereo input, stereo output
        // The string argument loads saved XML state (empty = use defaults)
        auto error = deviceManager.initialise(
            2,      // numInputChannelsNeeded
            2,      // numOutputChannelsNeeded
            nullptr, // savedState (XML element, or nullptr for defaults)
            true     // selectDefaultDeviceOnFailure
        );

        if (error.isNotEmpty())
            juce::Logger::writeToLog("Audio init error: " + error);
    }
};
```

**Device enumeration** iterates available `AudioIODeviceType` objects. Each type (CoreAudio, ASIO, etc.) provides its own device list:

```cpp
void enumerateDevices(juce::AudioDeviceManager& dm)
{
    for (auto& type : dm.getAvailableDeviceTypes())
    {
        type->scanForDevices();

        juce::Logger::writeToLog("Device type: " + type->getTypeName());

        auto inputNames  = type->getDeviceNames(true);  // input devices
        auto outputNames = type->getDeviceNames(false); // output devices

        for (auto& name : inputNames)
            juce::Logger::writeToLog("  Input:  " + name);
        for (auto& name : outputNames)
            juce::Logger::writeToLog("  Output: " + name);
    }
}
```

**Setting a specific device type** (e.g., forcing ASIO on Windows):

```cpp
deviceManager.setCurrentAudioDeviceType("ASIO", true);
```

### 1.2 Platform Device Types

| Platform | Device Type Class | API | Notes |
|----------|------------------|-----|-------|
| macOS | `CoreAudioIODeviceType` | CoreAudio | Default on macOS. Low latency. Aggregate devices supported. |
| Windows | `ASIOAudioIODeviceType` | ASIO | Lowest latency on Windows. Requires ASIO SDK header in build. |
| Windows | `WASAPIAudioIODeviceType` | WASAPI | Shared or exclusive mode. Default on Windows if ASIO unavailable. |
| Windows | `DirectSoundAudioIODeviceType` | DirectSound | Legacy. Higher latency. |
| Linux | `ALSAAudioIODeviceType` | ALSA | Default on Linux. Direct hardware access. |
| Linux | `JackAudioIODeviceType` | JACK | Professional Linux audio. Requires JACK server running. |
| iOS | `iOSAudioIODeviceType` | CoreAudio (iOS) | `AVAudioSession` category management. |
| Android | `OpenSLAudioIODeviceType` / `OboeAudioIODeviceType` | OpenSL ES / AAudio | Oboe preferred on API 27+. |

To enable ASIO support, you must place the Steinberg ASIO SDK headers in your include path and define `JUCE_ASIO=1` in your project. JACK requires `JUCE_JACK=1` and linking against `libjack`.

### 1.3 AudioIODeviceCallback

This is the interface you implement to receive audio data on the real-time audio thread:

```cpp
class MyAudioCallback : public juce::AudioIODeviceCallback
{
public:
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override
    {
        // REAL-TIME THREAD -- no allocations, no locks, no syscalls

        // Process input: inputChannelData[channel][sample]
        for (int ch = 0; ch < numInputChannels; ++ch)
        {
            const float* in = inputChannelData[ch];
            for (int i = 0; i < numSamples; ++i)
            {
                // Accumulate into ring buffer for analysis...
                ringBuffer.push(in[i]);
            }
        }

        // Pass-through or silence output
        for (int ch = 0; ch < numOutputChannels; ++ch)
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
        sampleRate = device->getCurrentSampleRate();
        bufferSize = device->getCurrentBufferSizeSamples();
        // Allocate analysis buffers here
    }

    void audioDeviceStopped() override
    {
        // Cleanup
    }

private:
    double sampleRate = 0.0;
    int bufferSize = 0;
    LockFreeRingBuffer ringBuffer; // your implementation
};
```

Register the callback:

```cpp
MyAudioCallback callback;
deviceManager.addAudioCallback(&callback);
// ...
deviceManager.removeAudioCallback(&callback); // before destruction
```

### 1.4 AudioBuffer<float>

`AudioBuffer<float>` is JUCE's multi-channel sample container. It owns a contiguous block of memory with per-channel pointers. Key access patterns:

```cpp
juce::AudioBuffer<float> buffer(2, 512); // 2 channels, 512 samples

// Direct pointer access (fastest)
float* left  = buffer.getWritePointer(0);
float* right = buffer.getWritePointer(1);

// Read-only access
const float* constLeft = buffer.getReadPointer(0);

// Sample-level access (bounds-checked in debug)
float sample = buffer.getSample(0, 42);

// Utility operations
buffer.clear();                          // zero all samples
buffer.applyGain(0.5f);                  // scale all channels
buffer.addFrom(0, 0, otherBuf, 0, 0, 512); // mix channel
buffer.copyFrom(1, 0, otherBuf, 0, 0, 512); // copy channel

// RMS level for metering
float rms = buffer.getRMSLevel(0, 0, 512);
```

### 1.5 AudioSourcePlayer and AudioTransportSource

For file playback (useful when the VJ app needs to analyze pre-recorded audio):

```cpp
juce::AudioFormatManager formatManager;
formatManager.registerBasicFormats(); // WAV, AIFF, FLAC, Ogg, etc.

std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
juce::AudioTransportSource transportSource;
juce::AudioSourcePlayer sourcePlayer;

void loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        transportSource.setSource(readerSource.get(), 32768, // readAheadBufferSize
                                   &threadPool,               // background read thread
                                   reader->sampleRate);
        sourcePlayer.setSource(&transportSource);
        deviceManager.addAudioCallback(&sourcePlayer);
        transportSource.start();
    }
}
```

`AudioTransportSource` handles sample-rate conversion, read-ahead buffering, and transport controls (play, stop, setPosition). It delegates disk reading to a background thread, keeping the audio callback lock-free.

---

## 2. JUCE DSP Module

The `juce_dsp` module (added in JUCE 5, significantly expanded in JUCE 6+) provides a collection of DSP primitives designed for real-time use. All operate on `juce::dsp::AudioBlock<float>` or `juce::dsp::ProcessContextReplacing`.

### 2.1 FFT: juce::dsp::FFT

JUCE's FFT uses platform-optimized backends (vDSP on macOS, Intel IPP if available, fallback to internal implementation).

```cpp
// FFT order = log2(fftSize). Order 10 = 1024-point FFT.
static constexpr int fftOrder = 10;
static constexpr int fftSize  = 1 << fftOrder; // 1024

juce::dsp::FFT forwardFFT{ fftOrder };

// Input buffer must be 2x fftSize (real input in first half, complex output fills both halves)
std::array<float, fftSize * 2> fftData{};

void processFFT(const float* audioSamples)
{
    // Copy samples into first half
    std::copy(audioSamples, audioSamples + fftSize, fftData.begin());

    // Zero the second half
    std::fill(fftData.begin() + fftSize, fftData.end(), 0.0f);

    // Perform forward FFT (in-place)
    forwardFFT.performRealOnlyForwardTransform(fftData.data());

    // Output is interleaved complex: [re0, im0, re1, im1, ..., re(N/2), im(N/2)]
    // Bin count: fftSize/2 + 1 complex bins

    // Compute magnitude spectrum
    std::array<float, fftSize / 2 + 1> magnitudes{};
    for (int i = 0; i <= fftSize / 2; ++i)
    {
        float re = fftData[i * 2];
        float im = fftData[i * 2 + 1];
        magnitudes[i] = std::sqrt(re * re + im * im);
    }

    // Convert to dB
    for (auto& m : magnitudes)
        m = juce::Decibels::gainToDecibels(m, -100.0f);
}
```

**Frequency resolution**: `sampleRate / fftSize`. At 44100 Hz with 1024-point FFT: ~43 Hz per bin. For a VJ application needing sub-bass resolution (20-60 Hz), use at least 4096-point FFT (~10.8 Hz/bin) or 8192-point (~5.4 Hz/bin), accepting the increased latency.

**Overlap-add analysis**: For smooth spectral updates, use 50-75% overlap with a hop size of `fftSize / 2` or `fftSize / 4`. Accumulate samples in a FIFO, trigger FFT when hop size is reached.

### 2.2 Windowing Functions

Apply a window before FFT to reduce spectral leakage:

```cpp
juce::dsp::WindowingFunction<float> window(fftSize,
    juce::dsp::WindowingFunction<float>::hann);

// Apply in-place before FFT
window.multiplyWithWindowingTable(fftData.data(), fftSize);
```

| Window | Main Lobe Width | Side Lobe Level | Use Case |
|--------|----------------|-----------------|----------|
| `hann` | Moderate | -31 dB | General-purpose spectral analysis |
| `hamming` | Moderate | -43 dB | Speech, slightly better side lobes |
| `blackman` | Wide | -58 dB | High dynamic range measurement |
| `blackmanHarris` | Widest | -92 dB | Maximum side lobe rejection |
| `rectangular` | Narrowest | -13 dB | Transient analysis (no windowing) |
| `triangular` | Moderate | -27 dB | Simple overlap-add |
| `flatTop` | Very wide | -93 dB | Amplitude-accurate measurement |
| `kaiser` | Configurable | Configurable | Tunable via beta parameter |

For VJ/music visualization, **Hann** is the standard choice -- good frequency resolution with acceptable leakage.

### 2.3 IIR Filters

`juce::dsp::IIR::Filter<float>` implements second-order (biquad) IIR filters. Design coefficients using the factory methods:

```cpp
using Coefficients = juce::dsp::IIR::Coefficients<float>;

double sampleRate = 44100.0;

// Low-pass at 200 Hz, Q = 0.707 (Butterworth)
auto lowpass = Coefficients::makeLowPass(sampleRate, 200.0, 0.707);

// Band-pass centered at 1000 Hz, Q = 2.0
auto bandpass = Coefficients::makeBandPass(sampleRate, 1000.0, 2.0);

// High-pass at 8000 Hz
auto highpass = Coefficients::makeHighPass(sampleRate, 8000.0);

// Peak/notch: center 440 Hz, gain 6 dB, Q = 1.0
auto peak = Coefficients::makePeakFilter(sampleRate, 440.0, 1.0,
                juce::Decibels::decibelsToGain(6.0f));

// Low shelf, high shelf
auto lowShelf  = Coefficients::makeLowShelf(sampleRate, 100.0, 0.707,
                    juce::Decibels::decibelsToGain(3.0f));
auto highShelf = Coefficients::makeHighShelf(sampleRate, 8000.0, 0.707,
                    juce::Decibels::decibelsToGain(-3.0f));
```

Using the filter in the audio callback:

```cpp
juce::dsp::IIR::Filter<float> filter;

void prepare(double sr, int blockSize)
{
    juce::dsp::ProcessSpec spec{ sr, (juce::uint32)blockSize, 1 };
    filter.prepare(spec);
    filter.coefficients = Coefficients::makeLowPass(sr, 200.0);
}

void processBlock(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);
}
```

For band-splitting in a VJ application (sub-bass, bass, mid, high), cascade four bandpass filters or use `LinkwitzRileyFilter` from JUCE 7+ for phase-coherent crossovers.

### 2.4 FIR Filters

`juce::dsp::FIR::Filter<float>` supports arbitrary-length FIR filters. Useful for linear-phase filtering or custom spectral shaping:

```cpp
// Design a FIR low-pass using windowed-sinc
auto* coefficients = juce::dsp::FilterDesign<float>::designFIRLowpassWindowMethod(
    200.0f,           // cutoff frequency
    sampleRate,       // sample rate
    127,              // filter order (num taps - 1)
    juce::dsp::WindowingFunction<float>::hamming
);

juce::dsp::FIR::Filter<float> firFilter;
firFilter.coefficients = coefficients;
```

FIR filters introduce latency equal to `(order / 2)` samples. For a 128-tap filter at 44.1 kHz, that is ~1.45 ms. JUCE's FIR implementation uses block processing and can exploit SIMD.

### 2.5 ProcessorChain

Chain multiple DSP processors into a single processing unit:

```cpp
using FilterBand = juce::dsp::IIR::Filter<float>;
using Gain       = juce::dsp::Gain<float>;

juce::dsp::ProcessorChain<FilterBand, FilterBand, Gain> chain;

void prepare(double sr, int blockSize)
{
    juce::dsp::ProcessSpec spec{ sr, (juce::uint32)blockSize, 2 };
    chain.prepare(spec);

    // Access individual processors by index
    auto& lowCut  = chain.get<0>();
    auto& highCut = chain.get<1>();
    auto& gain    = chain.get<2>();

    lowCut.coefficients  = Coefficients::makeHighPass(sr, 30.0);
    highCut.coefficients = Coefficients::makeLowPass(sr, 16000.0);
    gain.setGainDecibels(-3.0f);
}
```

`ProcessorChain` calls `process()` on each element sequentially. It is a compile-time chain (variadic template), so the topology is fixed at compile time. For dynamic routing, use `AudioProcessorGraph`.

### 2.6 Convolution Engine

```cpp
juce::dsp::Convolution convolution;

void prepare(double sr, int blockSize)
{
    juce::dsp::ProcessSpec spec{ sr, (juce::uint32)blockSize, 2 };
    convolution.prepare(spec);
    convolution.loadImpulseResponse(
        juce::File("/path/to/ir.wav"),
        juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::yes,
        0 // size (0 = use full IR)
    );
}
```

The convolution engine uses uniform-partitioned FFT convolution internally. IR loading happens on a background thread. This is relevant for a VJ app if you want to apply room simulation or creative reverb effects to monitored audio.

### 2.7 Dynamics Processors

```cpp
juce::dsp::Compressor<float> compressor;
compressor.setThreshold(-20.0f);  // dB
compressor.setRatio(4.0f);        // 4:1
compressor.setAttack(5.0f);       // ms
compressor.setRelease(100.0f);    // ms

juce::dsp::Limiter<float> limiter;
limiter.setThreshold(-1.0f);
limiter.setRelease(100.0f);

juce::dsp::NoiseGate<float> gate;
gate.setThreshold(-40.0f);
gate.setRatio(10.0f);
gate.setAttack(1.0f);
gate.setRelease(50.0f);
```

These are primarily useful for protecting output or conditioning input signals. For a VJ app doing analysis-only (no audio output), they are less relevant than the FFT and filter tools.

---

## 3. AudioProcessorGraph

`AudioProcessorGraph` allows you to build a dynamic signal processing graph at runtime. Each node wraps an `AudioProcessor` subclass. This is the architecture used by DAW plugin hosts and is well-suited for a modular analysis pipeline.

### 3.1 Creating a Graph

```cpp
class AnalysisGraph
{
    juce::AudioProcessorGraph graph;
    juce::AudioProcessorGraph::Node::Ptr fftNode;
    juce::AudioProcessorGraph::Node::Ptr bandSplitNode;

public:
    void buildGraph()
    {
        graph.clear();

        // Audio input/output nodes (required)
        auto audioInputNode  = graph.addNode(
            std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
                juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
        auto audioOutputNode = graph.addNode(
            std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
                juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));

        // Custom analysis processors
        fftNode       = graph.addNode(std::make_unique<FFTAnalysisProcessor>());
        bandSplitNode = graph.addNode(std::make_unique<BandSplitProcessor>());

        // Wire: input -> fftNode -> output
        //        input -> bandSplitNode -> output
        graph.addConnection({
            { audioInputNode->nodeID, 0 },
            { fftNode->nodeID, 0 }
        });
        graph.addConnection({
            { audioInputNode->nodeID, 0 },
            { bandSplitNode->nodeID, 0 }
        });
        graph.addConnection({
            { fftNode->nodeID, 0 },
            { audioOutputNode->nodeID, 0 }
        });
    }
};
```

### 3.2 Custom Analysis Processor

Each node is an `AudioProcessor` subclass. The key override is `processBlock()`:

```cpp
class FFTAnalysisProcessor : public juce::AudioProcessor
{
    juce::dsp::FFT fft{ 10 }; // 1024-point
    juce::dsp::WindowingFunction<float> window{ 1024,
        juce::dsp::WindowingFunction<float>::hann };
    std::array<float, 2048> fftData{};
    std::atomic<bool> newDataAvailable{ false };

    // Lock-free output for the render thread
    std::array<float, 513> magnitudeSpectrum{};

public:
    const juce::String getName() const override { return "FFT Analysis"; }
    // ... other required AudioProcessor overrides (most return defaults)

    void prepareToPlay(double sr, int maxBlock) override
    {
        sampleRate = sr;
    }

    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer&) override
    {
        // Mono analysis from first channel
        const float* data = buffer.getReadPointer(0);
        int numSamples = buffer.getNumSamples();

        // Accumulate into FIFO, run FFT when full
        for (int i = 0; i < numSamples; ++i)
        {
            fifo[fifoIndex++] = data[i];
            if (fifoIndex == 1024)
            {
                std::copy(fifo.begin(), fifo.end(), fftData.begin());
                std::fill(fftData.begin() + 1024, fftData.end(), 0.0f);
                window.multiplyWithWindowingTable(fftData.data(), 1024);
                fft.performRealOnlyForwardTransform(fftData.data());

                for (int bin = 0; bin <= 512; ++bin)
                {
                    float re = fftData[bin * 2];
                    float im = fftData[bin * 2 + 1];
                    magnitudeSpectrum[bin] = std::sqrt(re * re + im * im);
                }
                newDataAvailable.store(true);
                fifoIndex = 0;
            }
        }

        // Pass audio through unchanged
    }

    bool hasNewData() const { return newDataAvailable.load(); }
    void getMagnitudes(float* dest)
    {
        std::copy(magnitudeSpectrum.begin(), magnitudeSpectrum.end(), dest);
        newDataAvailable.store(false);
    }

private:
    double sampleRate = 44100.0;
    std::array<float, 1024> fifo{};
    int fifoIndex = 0;
};
```

### 3.3 Real-Time Safe Graph Modifications

`AudioProcessorGraph` is **not lock-free** for topology changes. Adding/removing nodes or connections acquires an internal lock. JUCE mitigates this by deferring the actual graph rebuild:

- Call `addNode()` / `removeNode()` / `addConnection()` / `removeConnection()` from the message thread.
- The graph internally rebuilds the rendering sequence on the next `processBlock()` call using a lock-free swap of the internal rendering order.

**Do not** modify the graph from the audio thread. If you need dynamic reconfiguration, do so from the message thread and let the internal mechanism handle the transition.

---

## 4. OpenGL Integration

JUCE provides `juce::OpenGLContext` and `juce::OpenGLRenderer` for hardware-accelerated rendering within the JUCE component hierarchy. The OpenGL context runs on its own dedicated thread, separate from both the message thread and the audio thread.

### 4.1 OpenGLContext and OpenGLRenderer

```cpp
class AudioVisualizer : public juce::Component,
                         public juce::OpenGLRenderer
{
    juce::OpenGLContext openGLContext;

public:
    AudioVisualizer()
    {
        openGLContext.setRenderer(this);
        openGLContext.setContinuousRepainting(true); // render loop, not just on repaint()
        openGLContext.setComponentPaintingEnabled(false); // skip software paint
        openGLContext.attachTo(*this);
    }

    ~AudioVisualizer() override
    {
        openGLContext.detach();
    }

    // Called on the OpenGL thread
    void newOpenGLContextCreated() override
    {
        // Compile shaders, create VBOs, upload textures
        compileShaders();
        createBuffers();
    }

    // Called on the OpenGL thread, ~60fps (vsync) or as fast as possible
    void renderOpenGL() override
    {
        using namespace juce::gl;

        // Fetch latest audio features (lock-free read)
        float spectrum[513];
        if (fftProcessor->hasNewData())
            fftProcessor->getMagnitudes(spectrum);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Upload spectrum as uniform array or texture
        shaderProgram->use();
        glUniform1fv(spectrumUniformLocation, 513, spectrum);
        glUniform1f(timeUniformLocation, (float)juce::Time::getMillisecondCounterHiRes() / 1000.0f);

        // Draw fullscreen quad or geometry
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void openGLContextClosing() override
    {
        // Delete GPU resources
    }
};
```

### 4.2 Thread Architecture

```
Message Thread       Audio Thread          OpenGL Thread
     |                    |                      |
 UI events          audioDeviceIOCallback    renderOpenGL()
 Component paint    writes to lock-free      reads from lock-free
 Graph changes      ring buffer/atomics      ring buffer/atomics
```

The three threads communicate through lock-free mechanisms:

- **Audio -> OpenGL**: Single-producer single-consumer FIFO or atomic snapshot of analysis results. Never use `std::mutex` on the audio thread.
- **Message -> OpenGL**: `openGLContext.executeOnGLThread()` for deferred operations.
- **OpenGL -> Message**: `juce::MessageManager::callAsync()` or `triggerAsyncUpdate()`.

### 4.3 Passing Audio Features to Shaders

For a VJ application, the key data to pass from audio analysis to shaders includes:

| Feature | Data Size | Transfer Method |
|---------|-----------|-----------------|
| Magnitude spectrum (512 bins) | 2 KB | `glUniform1fv` or 1D texture |
| Band energies (4-8 bands) | 32 B | `glUniform1fv` |
| RMS / peak level | 4-8 B | `glUniform1f` |
| Onset detection flag | 1 B | `glUniform1i` |
| Spectral centroid | 4 B | `glUniform1f` |
| Mel spectrogram (128 bands x N frames) | Variable | 2D texture via `glTexSubImage2D` |

**Spectrum as 1D texture** (better for large data, enables GPU interpolation):

```cpp
void uploadSpectrumTexture(const float* spectrum, int numBins)
{
    using namespace juce::gl;

    if (spectrumTexture == 0)
    {
        glGenTextures(1, &spectrumTexture);
        glBindTexture(GL_TEXTURE_1D, spectrumTexture);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, numBins, 0, GL_RED, GL_FLOAT, spectrum);
    }
    else
    {
        glBindTexture(GL_TEXTURE_1D, spectrumTexture);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, numBins, GL_RED, GL_FLOAT, spectrum);
    }
}
```

### 4.4 GLSL Fragment Shader Example

```glsl
#version 330 core

uniform sampler1D spectrumTex;
uniform float time;
uniform float rmsLevel;
uniform vec2 resolution;

out vec4 fragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;

    // Sample spectrum at x position
    float freq = texture(spectrumTex, uv.x).r;

    // Normalize to visual range (assuming dB values in -100..0 range)
    float barHeight = (freq + 100.0) / 100.0;

    // Render as vertical bars
    float bar = step(uv.y, barHeight);

    // Color based on frequency band
    vec3 color = mix(vec3(0.1, 0.2, 0.8), vec3(0.9, 0.1, 0.2), uv.x);
    color *= bar;

    // Pulse brightness with RMS
    color *= 0.7 + 0.3 * rmsLevel;

    fragColor = vec4(color, 1.0);
}
```

### 4.5 OpenGL Version Considerations

JUCE's OpenGL integration uses the platform's available OpenGL version. On macOS, this is limited to OpenGL 4.1 (Apple deprecated OpenGL in favor of Metal). JUCE does not currently have a Metal backend. For advanced GPU compute (compute shaders require OpenGL 4.3+), macOS is a limitation. On Windows and Linux, OpenGL 4.6 is typically available.

If you need Metal on macOS or Vulkan cross-platform, you will need to manage the graphics context outside JUCE and embed it in a JUCE component via `NSView` / `HWND` interop.

---

## 5. Cross-Platform Build System

### 5.1 Projucer vs CMake

**Projucer** is JUCE's legacy project generator GUI. It produces IDE-specific project files (Xcode, Visual Studio, Makefile). It is being phased out in favor of CMake.

**CMake** (recommended since JUCE 6) is the modern approach. JUCE provides CMake helper functions that handle module dependencies, platform defines, and binary packaging.

### 5.2 CMakeLists.txt for a GUI Application

```cmake
cmake_minimum_required(VERSION 3.22)
project(AudioVisualizer VERSION 1.0.0)

# Fetch JUCE or use add_subdirectory
add_subdirectory(JUCE)  # assuming JUCE is a git submodule at ./JUCE

juce_add_gui_app(AudioVisualizer
    PRODUCT_NAME "Audio Visualizer"
    COMPANY_NAME "YourCompany"
    BUNDLE_ID "com.yourcompany.audiovisualizer"
    ICON_BIG   "resources/icon_512.png"
    ICON_SMALL "resources/icon_128.png"
)

target_sources(AudioVisualizer PRIVATE
    src/Main.cpp
    src/MainComponent.cpp
    src/AudioAnalyzer.cpp
    src/GLRenderer.cpp
)

target_compile_definitions(AudioVisualizer PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0  # requires paid license
    JUCE_ASIO=1                    # enable ASIO on Windows
    JUCE_JACK=1                    # enable JACK on Linux
)

target_link_libraries(AudioVisualizer PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
```

### 5.3 Plugin Target (VST3/AU)

If you want the analysis engine to also work as a plugin:

```cmake
juce_add_plugin(AudioAnalyzerPlugin
    PLUGIN_MANUFACTURER_CODE Yrco
    PLUGIN_CODE Aavz
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Audio Analyzer"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)
```

### 5.4 Platform-Specific Build Instructions

| Platform | Generator | Build Commands |
|----------|-----------|---------------|
| macOS | Xcode | `cmake -B build -G Xcode && cmake --build build --config Release` |
| macOS | Ninja | `cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && ninja -C build` |
| Windows | Visual Studio 17 | `cmake -B build -G "Visual Studio 17 2022" && cmake --build build --config Release` |
| Linux | Makefiles | `cmake -B build -DCMAKE_BUILD_TYPE=Release && make -C build -j$(nproc)` |

**Linux dependencies** (Debian/Ubuntu):

```bash
sudo apt install libasound2-dev libjack-jackd2-dev libfreetype6-dev \
    libx11-dev libxrandr-dev libxcursor-dev libxinerama-dev \
    libgl1-mesa-dev libcurl4-openssl-dev
```

---

## 6. Licensing

| Tier | Cost | Terms |
|------|------|-------|
| **Personal** | Free | Revenue < $50K/year. Must display JUCE splash screen. |
| **Indie** | $40/month | Revenue < $500K/year. No splash screen. |
| **Pro** | $130/month | Unlimited revenue. Priority support. |
| **Educational** | Free | For accredited institutions. |
| **GPLv3** | Free | Open-source your entire application under GPL. No revenue limit. |

The GPLv3 option is viable for open-source VJ tools. If you plan to distribute closed-source binaries, you need a commercial license. The splash screen requirement on the Personal tier displays a small JUCE logo on startup for ~2 seconds.

JUCE modules are licensed individually: the core modules are dual-licensed (GPL + commercial), while some third-party code within JUCE (e.g., the Steinberg VST3 SDK integration) has its own licensing terms.

---

## 7. Integration with Essentia and Aubio

A JUCE-based VJ application can call Essentia or Aubio from within the audio callback or from a dedicated analysis thread. The key constraint is real-time safety.

### 7.1 Linking External Libraries

In `CMakeLists.txt`:

```cmake
# Find system-installed Essentia
find_package(PkgConfig REQUIRED)
pkg_check_modules(ESSENTIA REQUIRED essentia)

# Or manually specify paths
set(ESSENTIA_INCLUDE_DIR "/usr/local/include/essentia")
set(ESSENTIA_LIB "/usr/local/lib/libessentia.a")

target_include_directories(AudioVisualizer PRIVATE
    ${ESSENTIA_INCLUDE_DIR}
)

target_link_libraries(AudioVisualizer PRIVATE
    ${ESSENTIA_LIB}
    # Essentia dependencies
    fftw3f
    yaml-cpp
    avcodec avformat avutil  # if using Essentia's audio loader
)

# Aubio (simpler, fewer deps)
find_library(AUBIO_LIB aubio)
find_path(AUBIO_INCLUDE aubio/aubio.h)

target_include_directories(AudioVisualizer PRIVATE ${AUBIO_INCLUDE})
target_link_libraries(AudioVisualizer PRIVATE ${AUBIO_LIB})
```

### 7.2 Calling Essentia from the Audio Callback

**Do not call Essentia's streaming mode from the real-time audio thread.** Essentia's standard mode algorithms are stateless function calls that can be real-time safe if you pre-allocate all buffers. However, many Essentia algorithms perform internal allocations, making them unsuitable for direct use on the audio thread.

**Recommended architecture**: Use a lock-free ring buffer to decouple the audio thread from an analysis thread.

```cpp
#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>

class EssentiaAnalysisThread : public juce::Thread
{
    LockFreeRingBuffer& ringBuffer; // shared with audio callback
    essentia::standard::Algorithm* spectralCentroid = nullptr;
    essentia::standard::Algorithm* mfcc = nullptr;

public:
    EssentiaAnalysisThread(LockFreeRingBuffer& rb)
        : Thread("Essentia Analysis"), ringBuffer(rb)
    {
        essentia::init();
        auto& factory = essentia::standard::AlgorithmFactory::instance();

        spectralCentroid = factory.create("SpectralCentroidTime",
            "sampleRate", 44100.0);

        mfcc = factory.create("MFCC",
            "inputSize", 513,
            "sampleRate", 44100.0,
            "numberCoefficients", 13);
    }

    ~EssentiaAnalysisThread() override
    {
        stopThread(1000);
        delete spectralCentroid;
        delete mfcc;
        essentia::shutdown();
    }

    void run() override
    {
        std::vector<float> frame(1024);
        std::vector<float> spectrum(513);
        std::vector<float> mfccBands, mfccCoeffs;
        float centroid;

        while (!threadShouldExit())
        {
            if (ringBuffer.getNumReady() >= 1024)
            {
                ringBuffer.read(frame.data(), 1024);

                // Compute spectrum (JUCE FFT, then pass to Essentia)
                // Or use Essentia's own Spectrum algorithm
                spectralCentroid->input("array").set(frame);
                spectralCentroid->output("centroid").set(centroid);
                spectralCentroid->compute();

                // Store results atomically for the render thread
                centroidResult.store(centroid);
            }
            else
            {
                wait(1); // sleep briefly if no data
            }
        }
    }

    std::atomic<float> centroidResult{ 0.0f };
};
```

### 7.3 Calling Aubio from the Audio Callback

Aubio is lightweight enough that its core functions (onset detection, pitch detection) can be called directly on the audio thread if buffers are pre-allocated. Aubio's C API does no internal allocation after `new_aubio_*()`:

```cpp
#include <aubio/aubio.h>

class AubioOnsetDetector
{
    aubio_onset_t* onset = nullptr;
    fvec_t* inputBuffer = nullptr;
    fvec_t* onsetOutput = nullptr;

public:
    void prepare(int hopSize, int sampleRate)
    {
        inputBuffer  = new_fvec(hopSize);
        onsetOutput  = new_fvec(1);
        onset = new_aubio_onset("default", hopSize * 2, hopSize, sampleRate);
        aubio_onset_set_threshold(onset, 0.3f);
        aubio_onset_set_silence(onset, -40.0f);
    }

    // Safe to call from audio thread (no allocations)
    bool processHop(const float* samples, int hopSize)
    {
        // Copy into aubio's fvec
        for (int i = 0; i < hopSize; ++i)
            inputBuffer->data[i] = samples[i];

        aubio_onset_do(onset, inputBuffer, onsetOutput);
        return onsetOutput->data[0] > 0.0f;
    }

    ~AubioOnsetDetector()
    {
        if (onset) del_aubio_onset(onset);
        if (inputBuffer) del_fvec(inputBuffer);
        if (onsetOutput) del_fvec(onsetOutput);
    }
};
```

### 7.4 Thread Safety Summary

| Library | Audio Thread Safe? | Recommended Pattern |
|---------|-------------------|-------------------|
| JUCE DSP (FFT, filters) | Yes | Direct call in `processBlock()` |
| Aubio (onset, pitch) | Yes (after init) | Direct call or analysis thread |
| Essentia (standard mode) | Mostly no (internal allocs) | Dedicated analysis thread with ring buffer |
| Essentia (streaming mode) | No | Never use in real-time path |

---

## 8. JUCE Modules Relevant to a VJ Application

| Module | Purpose | Key Classes |
|--------|---------|-------------|
| `juce_core` | Threading, files, JSON, memory, logging | `Thread`, `File`, `var`, `MemoryBlock`, `ScopedPointer` |
| `juce_audio_basics` | Sample buffers, MIDI, audio math | `AudioBuffer`, `MidiMessage`, `Decibels`, `FloatVectorOperations` |
| `juce_audio_devices` | Audio hardware I/O | `AudioDeviceManager`, `AudioIODevice`, `AudioIODeviceCallback` |
| `juce_audio_formats` | File format reading/writing | `AudioFormatManager`, `AudioFormatReader`, `WavAudioFormat`, `FlacAudioFormat` |
| `juce_audio_processors` | Plugin hosting and processing graph | `AudioProcessor`, `AudioProcessorGraph`, `AudioPluginInstance` |
| `juce_dsp` | DSP primitives | `FFT`, `IIR::Filter`, `FIR::Filter`, `Convolution`, `WindowingFunction`, `ProcessorChain` |
| `juce_opengl` | OpenGL rendering context | `OpenGLContext`, `OpenGLRenderer`, `OpenGLShaderProgram`, `OpenGLTexture` |
| `juce_gui_basics` | UI components, layout, events | `Component`, `Slider`, `TextButton`, `LookAndFeel`, `Desktop` |
| `juce_graphics` | 2D drawing, fonts, images | `Graphics`, `Path`, `Image`, `Colour`, `Font` |
| `juce_events` | Message loop, timers, async callbacks | `MessageManager`, `Timer`, `AsyncUpdater`, `ActionListener` |

### Module Dependency Graph (subset for VJ app)

```
juce_opengl ──> juce_gui_basics ──> juce_graphics ──> juce_core
                                                        ^
juce_audio_devices ──> juce_audio_basics ───────────────┘
                            ^
juce_dsp ───────────────────┘
                            ^
juce_audio_processors ──────┘──> juce_audio_formats
```

All modules transitively depend on `juce_core`. When you list modules in `target_link_libraries`, CMake resolves transitive dependencies automatically.

---

## 9. Limitations and Alternatives

### 9.1 Where JUCE Falls Short

**Loopback / system audio capture**: JUCE has no built-in support for capturing system audio output (what's playing through speakers). On macOS, this requires a virtual audio device (e.g., BlackHole, Soundflower) or the ScreenCaptureKit API (macOS 13+). On Windows, WASAPI loopback mode is available but JUCE does not expose it directly -- you would need to use the Windows API alongside JUCE or patch the JUCE WASAPI device type. See [ARCH_audio_io.md](ARCH_audio_io.md) for loopback capture strategies.

**Advanced OpenGL / Modern GPU APIs**: JUCE's OpenGL wrapper is functional but minimal. It does not expose:
- Compute shaders (requires OpenGL 4.3, not available on macOS)
- Geometry / tessellation shaders (limited support)
- Vulkan or Metal backends
- Multi-pass render targets without manual FBO management

For advanced visual effects (particle systems, post-processing chains, GPGPU audio analysis), you may want to manage the graphics context yourself or use a dedicated renderer (e.g., bgfx, sokol_gfx, or raw Metal/Vulkan) and embed it in a JUCE window via native view interop.

**Video playback/output**: JUCE has no video module. Rendering to video files or playing back video requires FFmpeg integration or platform APIs.

**High-frequency UI updates**: JUCE's component painting is message-thread bound. For a waveform or spectrogram that updates at 60fps, use `OpenGLRenderer` (which has its own thread) rather than `Component::repaint()` which serializes through the message queue.

**Network audio (NDI, Dante, AES67)**: Not supported. Use dedicated SDKs.

### 9.2 When to Use Raw APIs Instead

| Scenario | JUCE Approach | Raw API Alternative |
|----------|--------------|-------------------|
| System audio loopback | Not supported | WASAPI loopback, ScreenCaptureKit, PulseAudio monitor |
| Vulkan/Metal rendering | Not supported | MoltenVK, raw Metal, embedded in JUCE window |
| Compute shaders for audio | Not on macOS | Metal compute, CUDA, OpenCL |
| Sub-millisecond latency | CoreAudio via JUCE (adds thin layer) | Direct CoreAudio/ASIO (marginal gain) |
| MIDI device hot-plug | Supported but basic | Platform-specific for advanced features |
| Video decode/encode | Not supported | FFmpeg, AVFoundation, MediaFoundation |

### 9.3 JUCE vs Alternative Frameworks

| Feature | JUCE | RtAudio + PortAudio | SDL2 Audio | openFrameworks |
|---------|------|---------------------|-----------|----------------|
| Audio device abstraction | Excellent | Good | Basic | Good (via RtAudio) |
| Built-in DSP | Comprehensive | None | None | Limited (ofxFft addon) |
| OpenGL integration | Integrated | Separate | Integrated | Integrated |
| Plugin format support | VST3, AU, AAX, LV2 | None | None | None |
| Cross-platform build | CMake + Projucer | CMake | CMake | Custom (project generator) |
| GUI toolkit | Full widget set | None | None | Immediate-mode + addons |
| Commercial support | Yes (paid tiers) | Community | Community | Community |
| Minimum binary size | ~2-5 MB | ~100 KB | ~1 MB | ~10 MB |

For a VJ application specifically, JUCE provides the strongest all-in-one package: audio I/O, DSP, and OpenGL in a single framework with a coherent threading model. The main gaps (loopback capture, advanced GPU) can be filled with targeted platform-specific code. See [LIB_rtaudio_miniaudio.md](LIB_rtaudio_miniaudio.md) for comparison with lighter-weight audio-only libraries.

---

## 10. Complete Example: Audio-Reactive Visualizer Architecture

Tying it all together, here is the skeleton of a JUCE application that captures audio, runs FFT analysis, and renders audio-reactive visuals via OpenGL:

```cpp
// MainComponent.h
#pragma once
#include <JuceHeader.h>

class MainComponent : public juce::Component,
                       public juce::OpenGLRenderer,
                       public juce::AudioIODeviceCallback
{
public:
    MainComponent();
    ~MainComponent() override;

    // AudioIODeviceCallback
    void audioDeviceIOCallbackWithContext(
        const float* const* input, int numInputCh,
        float* const* output, int numOutputCh,
        int numSamples,
        const juce::AudioIODeviceCallbackContext&) override;
    void audioDeviceAboutToStart(juce::AudioIODevice*) override;
    void audioDeviceStopped() override;

    // OpenGLRenderer
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void resized() override;

private:
    juce::AudioDeviceManager deviceManager;
    juce::OpenGLContext glContext;

    // FFT
    static constexpr int fftOrder = 11;       // 2048 points
    static constexpr int fftSize = 1 << fftOrder;
    juce::dsp::FFT fft{ fftOrder };
    juce::dsp::WindowingFunction<float> window{
        (size_t)fftSize, juce::dsp::WindowingFunction<float>::hann };

    // FIFO for overlap accumulation
    std::array<float, fftSize> fifo{};
    int fifoIndex = 0;

    // Double-buffered spectrum for lock-free audio->GL transfer
    std::array<float, fftSize * 2> fftWorkBuffer{};
    std::array<float, fftSize / 2 + 1> spectrumA{};
    std::array<float, fftSize / 2 + 1> spectrumB{};
    std::atomic<int> activeSpectrum{ 0 }; // 0 = A is readable, 1 = B is readable

    std::atomic<float> currentRMS{ 0.0f };

    // GL resources
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    GLuint vao = 0, vbo = 0;
    GLuint spectrumTexture = 0;
};
```

```cpp
// MainComponent.cpp (key methods)

MainComponent::MainComponent()
{
    deviceManager.initialise(1, 0, nullptr, true); // 1 input, 0 output
    deviceManager.addAudioCallback(this);

    glContext.setRenderer(this);
    glContext.setContinuousRepainting(true);
    glContext.attachTo(*this);

    setSize(1280, 720);
}

void MainComponent::audioDeviceIOCallbackWithContext(
    const float* const* input, int numInputCh,
    float* const*, int, int numSamples,
    const juce::AudioIODeviceCallbackContext&)
{
    if (numInputCh == 0) return;
    const float* samples = input[0];

    // RMS
    float sumSq = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sumSq += samples[i] * samples[i];
    currentRMS.store(std::sqrt(sumSq / (float)numSamples));

    // Accumulate into FIFO
    for (int i = 0; i < numSamples; ++i)
    {
        fifo[fifoIndex++] = samples[i];
        if (fifoIndex == fftSize)
        {
            // Copy to work buffer, apply window, FFT
            std::copy(fifo.begin(), fifo.end(), fftWorkBuffer.begin());
            std::fill(fftWorkBuffer.begin() + fftSize, fftWorkBuffer.end(), 0.0f);
            window.multiplyWithWindowingTable(fftWorkBuffer.data(), fftSize);
            fft.performRealOnlyForwardTransform(fftWorkBuffer.data());

            // Write to inactive buffer
            auto& dest = (activeSpectrum.load() == 0) ? spectrumB : spectrumA;
            for (int bin = 0; bin <= fftSize / 2; ++bin)
            {
                float re = fftWorkBuffer[bin * 2];
                float im = fftWorkBuffer[bin * 2 + 1];
                dest[bin] = std::sqrt(re * re + im * im);
            }
            // Swap active buffer
            activeSpectrum.store((activeSpectrum.load() == 0) ? 1 : 0);

            fifoIndex = 0;
        }
    }
}

void MainComponent::renderOpenGL()
{
    using namespace juce::gl;

    const auto& spectrum = (activeSpectrum.load() == 0) ? spectrumA : spectrumB;
    float rms = currentRMS.load();

    // Upload spectrum to texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, spectrumTexture);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, fftSize / 2 + 1,
                    GL_RED, GL_FLOAT, spectrum.data());

    auto bounds = getLocalBounds();
    glViewport(0, 0, bounds.getWidth(), bounds.getHeight());
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();
    shader->setUniform("spectrumTex", 0);
    shader->setUniform("rmsLevel", rms);
    shader->setUniform("resolution",
        (float)bounds.getWidth(), (float)bounds.getHeight());
    shader->setUniform("time",
        (float)(juce::Time::getMillisecondCounterHiRes() * 0.001));

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
```

This architecture provides:
- Audio capture at native device latency (typically 128-512 samples / 3-12 ms)
- FFT analysis running inline on the audio thread (2048-point FFT is fast enough)
- Lock-free double-buffered spectrum transfer to the GL thread
- 60fps vsync-locked rendering with audio-reactive uniforms
- No mutex contention between any threads

For more complex analysis (MFCCs, onset detection, beat tracking), add an analysis thread between audio and GL, fed by a ring buffer, as shown in section 7.2. See [ARCH_pipeline.md](ARCH_pipeline.md) for the full pipeline architecture.
