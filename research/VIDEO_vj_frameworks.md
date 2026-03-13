# VJ Frameworks & Audio-Visual Tools: Technical Survey

> **Scope**: Exhaustive comparison of existing frameworks and tools that combine real-time audio analysis with visual output. Evaluates each for potential integration with a custom C++ real-time audio analysis engine, or as a replacement for parts of the pipeline.

> **Related documents**: [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md) | [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) | [LIB_essentia.md](LIB_essentia.md) | [LIB_aubio.md](LIB_aubio.md) | [LIB_juce.md](LIB_juce.md) | [ARCH_pipeline.md](ARCH_pipeline.md)

---

## 1. openFrameworks

### 1.1 Architecture Overview

openFrameworks (oF) is an open-source C++ toolkit designed for "creative coding." It wraps low-level libraries (OpenGL, GLFW, FreeType, FreeImage, rtAudio, POCO) behind a consistent API. The core design philosophy is a thin wrapper: oF does not abstract away the underlying libraries so much as make them easier to set up together.

The application lifecycle follows a simple `setup()` / `update()` / `draw()` pattern with an `ofApp` base class. Audio callbacks (`audioIn()`, `audioOut()`) are invoked on a separate thread by the audio backend.

### 1.2 ofSoundStream: Audio I/O

`ofSoundStream` is the core audio I/O abstraction. It wraps rtAudio (or PulseAudio on Linux) and provides:

- Device enumeration via `ofSoundStream::getDeviceList()`
- Configurable sample rate (typically 44100 or 48000), buffer size (64--2048), and channel count
- Callback-based I/O: the audio thread invokes `ofApp::audioIn(ofSoundBuffer &buffer)` and `ofApp::audioOut(ofSoundBuffer &buffer)`
- `ofSoundBuffer` carries interleaved float samples plus metadata (sample rate, channel count, tick count)

```cpp
// ofApp.h
void audioIn(ofSoundBuffer &input) override;

// ofApp.cpp
void setup() {
    ofSoundStreamSettings settings;
    settings.setInDevice(ofSoundStreamListDevices()[0]);
    settings.sampleRate = 44100;
    settings.bufferSize = 512;
    settings.numInputChannels = 2;
    settings.numOutputChannels = 0;
    settings.setApi(ofSoundDevice::Api::COREAUDIO);
    soundStream.setup(settings);
}

void audioIn(ofSoundBuffer &input) {
    // Runs on audio thread -- copy data, do not allocate
    std::lock_guard<std::mutex> lock(audioMutex);
    audioBuffer = input;  // ofSoundBuffer has copy semantics
}
```

Latency is determined by `bufferSize / sampleRate`. At 512 samples / 44100 Hz, that is ~11.6 ms per callback. The audio thread runs at elevated priority on all platforms.

### 1.3 ofxAudioAnalyzer: Essentia-Based Analysis

[ofxAudioAnalyzer](https://github.com/leozimmerman/ofxAudioAnalyzer) wraps the Essentia library for real-time spectral and temporal analysis. It provides a unified interface to over 20 algorithms:

| Category | Algorithms |
|----------|-----------|
| Loudness | RMS, Energy, Instant Power |
| Spectral shape | Centroid, Rolloff, Flatness, Spectral Complexity, Inharmonicity |
| Pitch | YinFFT pitch frequency + confidence, Pitch Salience |
| Timbre | MFCC (13 coefficients), Mel Bands (24/40 bands), HPCP (Harmonic Pitch Class Profile), Tristimulus |
| Transients | HFC (High Frequency Content), Onsets, Strong Peak, Strong Decay |
| Harmony | Dissonance, Odd-to-Even Harmonic Ratio |

Usage pattern:

```cpp
// setup
ofxAudioAnalyzer analyzer;
analyzer.setup(44100, 512, 2);  // sampleRate, bufferSize, channels

// audioIn callback
void audioIn(ofSoundBuffer &buf) {
    analyzer.analyze(buf);
}

// draw (main thread)
void draw() {
    float rms = analyzer.getValue(RMS, 0, 0.5);           // channel 0, smoothing 0.5
    float pitch = analyzer.getValue(PITCH_FREQ, 0, 0.3);
    vector<float> spectrum = analyzer.getValues(SPECTRUM, 0, 0.2);
    vector<float> melBands = analyzer.getValues(MEL_BANDS, 0, 0.4);
    vector<float> mfcc = analyzer.getValues(MFCC, 0, 0.3);
    bool onset = analyzer.getOnsetValue(0);

    // Use values to drive OpenGL rendering...
}
```

The smoothing parameter applies exponential moving average (EMA) across frames, which reduces jitter but adds temporal lag. At smoothing = 0.5, the effective response time is roughly `bufferSize * 2 / sampleRate`.

**Compilation note**: Essentia must be compiled as a static library for the target platform. The addon includes precompiled binaries for macOS and Linux, but Windows requires manual compilation via `essentia_compilation.md` in the repository. This is a non-trivial process involving Meson, GCC or MSVC, and several dependencies (FFTW3, libsamplerate, TagLib).

### 1.4 ofxAubio: Aubio Integration

[ofxAubio](https://github.com/aubio/ofxAubio) provides a lighter-weight alternative focused on onset detection, pitch tracking, and beat tracking via the Aubio library:

```cpp
ofxAubioPitch pitch;
pitch.setup("yinfft", 2048, 512, 44100);

ofxAubioBeat beat;
beat.setup("default", 1024, 512, 44100);

// In audioIn:
pitch.audioIn(buffer.getBuffer().data(), bufferSize, channels);
beat.audioIn(buffer.getBuffer().data(), bufferSize, channels);

// In update/draw:
float freq = pitch.latestPitch;
float conf = pitch.pitchConfidence;
bool isBeat = beat.received;  // true on beat frames
float bpm = beat.bpm;
```

Aubio is significantly smaller than Essentia (~150 KB compiled vs ~15 MB) and easier to cross-compile, making it a better fit when only onset/pitch/beat are needed.

### 1.5 OpenGL Rendering

oF provides direct OpenGL access through `ofGLProgrammableRenderer` (OpenGL 3.2+ core profile or GLES). Key features:

- `ofShader` for GLSL vertex/fragment/geometry/compute shaders
- `ofFbo` for off-screen render targets (framebuffer objects)
- `ofVboMesh` for efficient geometry upload
- `ofTexture` for texture management (including float textures for data upload)
- `ofCamera` / `ofEasyCam` for 3D viewpoint control

The addon ecosystem extends rendering: `ofxGui` provides parameter tweaking UI, `ofxPostProcessing` adds bloom/blur/FXAA, and `ofxShader` enables live shader reloading.

### 1.6 Assessment

**Strengths**: Mature C++ ecosystem, tight integration between audio analysis and OpenGL rendering in the same process (zero IPC latency), large community with extensive addons (over 2000 on ofxaddons.com), good documentation.

**Weaknesses**: The build system (based on Makefiles and project generators) is fragile and frequently breaks with OS/compiler updates. The core architecture dates from 2005 and uses global state extensively (`ofGetWidth()`, etc.). No CMake support in mainline. The Essentia dependency in ofxAudioAnalyzer introduces significant build complexity. Development pace has slowed -- the last major release (0.12.0) was in 2023.

---

## 2. Cinder

### 2.1 Architecture Overview

[Cinder](https://libcinder.org/) is a C++ creative coding framework with a focus on modern C++ design (C++17, RAII, shared_ptr, lambdas). It is architecturally cleaner than oF, with namespaced modules (`ci::gl`, `ci::audio`, `ci::app`) and a node-graph-based audio system.

### 2.2 Audio Module: Node Graph

Cinder's audio system (`ci::audio`) is built around a directed acyclic graph of `Node` objects. Audio flows from input nodes through processing nodes to output nodes. Key node types:

| Node | Purpose |
|------|---------|
| `InputDeviceNode` | Captures audio from hardware input |
| `OutputDeviceNode` | Sends audio to hardware output |
| `MonitorNode` | Taps PCM samples for visualization (no modification) |
| `MonitorSpectralNode` | Provides FFT magnitude spectrum |
| `GainNode` | Volume control |
| `GenSineNode`, `GenNoiseNode` | Signal generators |
| `FilePlayerNode` | Audio file playback |
| `BufferRecorderNode` | Records to buffer |

The node graph is constructed programmatically:

```cpp
auto ctx = ci::audio::Context::master();

// Input → Monitor → MonitorSpectral → Output
mInputNode = ctx->createInputDeviceNode();
mMonitor = ctx->makeNode(new ci::audio::MonitorNode(
    ci::audio::MonitorNode::Format().windowSize(1024)
));
mMonitorSpectral = ctx->makeNode(new ci::audio::MonitorSpectralNode(
    ci::audio::MonitorSpectralNode::Format()
        .fftSize(2048)
        .windowSize(1024)
));

mInputNode >> mMonitor >> mMonitorSpectral;
mInputNode->enable();
ctx->enable();
```

`MonitorSpectralNode` internally manages an FFT (`ci::audio::dsp::Fft`) and applies a Hann window. The FFT size is automatically rounded up to the next power of 2. Retrieving spectral data on the main thread is thread-safe:

```cpp
void draw() {
    // Thread-safe copy of magnitude spectrum
    const std::vector<float> &mag = mMonitorSpectral->getMagSpectrum();
    float centroid = mMonitorSpectral->getFreqForBin(
        computeSpectralCentroid(mag)  // user function
    );

    // PCM buffer for waveform display
    const ci::audio::Buffer &pcm = mMonitor->getBuffer();
}
```

### 2.3 Rendering

Cinder's `ci::gl` namespace provides a modern OpenGL abstraction:

- `ci::gl::GlslProg` for shader programs with automatic uniform binding
- `ci::gl::Batch` combines geometry + shader for efficient draw calls
- `ci::gl::Fbo` for render-to-texture
- `ci::gl::Texture2d` with support for float/half-float formats
- Built-in stock shaders for common operations
- `ci::gl::ScopedModelMatrix`, `ScopedBlendAlpha` for RAII state management

```cpp
auto shader = ci::gl::GlslProg::create(
    ci::gl::GlslProg::Format()
        .vertex(CI_GLSL(150,
            uniform mat4 ciModelViewProjection;
            in vec4 ciPosition;
            void main() { gl_Position = ciModelViewProjection * ciPosition; }
        ))
        .fragment(CI_GLSL(150,
            uniform float uRms;
            uniform float uCentroid;
            out vec4 oColor;
            void main() {
                oColor = vec4(uCentroid, uRms, 0.5, 1.0);
            }
        ))
);
shader->uniform("uRms", rmsValue);
shader->uniform("uCentroid", centroidNormalized);
```

### 2.4 Assessment

**Strengths**: Elegant modern C++ API, node-graph audio architecture is well-designed, excellent OpenGL integration, RAII everywhere, good for audio-reactive art projects.

**Weaknesses**: Smaller community than oF (fewer addons, fewer StackOverflow answers). Development has slowed significantly since 2020 -- the GitHub repository shows intermittent commits. No built-in advanced audio analysis (no onset detection, no MFCC, no beat tracking -- only FFT magnitude and PCM monitoring). To get Essentia/Aubio-level analysis, you must integrate those libraries yourself, which negates much of the convenience. Windows/Linux support lags behind macOS.

---

## 3. Processing / p5.js

### 3.1 Processing (Java)

Processing is a Java-based creative coding environment. Audio analysis is provided by the **Minim** library (bundled by default):

```java
import ddf.minim.*;
import ddf.minim.analysis.*;

Minim minim;
AudioInput in;
FFT fft;
BeatDetect beat;

void setup() {
    size(800, 600);
    minim = new Minim(this);
    in = minim.getLineIn(Minim.STEREO, 1024);
    fft = new FFT(in.bufferSize(), in.sampleRate());
    fft.logAverages(60, 3);  // 60 Hz min bandwidth, 3 bands per octave
    beat = new BeatDetect(in.bufferSize(), in.sampleRate());
    beat.setSensitivity(200);  // ms cooldown between beats
}

void draw() {
    fft.forward(in.mix);
    beat.detect(in.mix);

    for (int i = 0; i < fft.avgSize(); i++) {
        float bandEnergy = fft.getAvg(i);
        rect(i * 10, height, 8, -bandEnergy * 5);
    }

    if (beat.isKick()) ellipse(width/4, height/2, 100, 100);
    if (beat.isSnare()) ellipse(width/2, height/2, 80, 80);
    if (beat.isHat()) ellipse(3*width/4, height/2, 60, 60);
}
```

Minim provides: FFT with linear and logarithmic averaging, BeatDetect (energy-based and frequency-based modes), AudioInput/AudioOutput, audio file playback, and basic synthesis (oscillators, envelopes, UGens).

### 3.2 p5.js + p5.sound

[p5.sound](https://p5js.org/reference/p5.sound/) is the JavaScript equivalent, built on the Web Audio API:

```javascript
let fft, amplitude, mic;

function setup() {
    createCanvas(800, 600);
    mic = new p5.AudioIn();
    mic.start();
    fft = new p5.FFT(0.8, 1024);  // smoothing, bins
    fft.setInput(mic);
    amplitude = new p5.Amplitude();
    amplitude.setInput(mic);
}

function draw() {
    background(0);

    // Frequency spectrum (0-255 per bin)
    let spectrum = fft.analyze();
    for (let i = 0; i < spectrum.length; i++) {
        let x = map(i, 0, spectrum.length, 0, width);
        let h = map(spectrum[i], 0, 255, 0, height);
        fill(spectrum[i], 100, 200);
        rect(x, height - h, width / spectrum.length, h);
    }

    // Waveform (-1.0 to 1.0)
    let waveform = fft.waveform();

    // Energy by band
    let bass = fft.getEnergy("bass");       // 20-140 Hz
    let mid = fft.getEnergy("mid");         // 400-2600 Hz
    let treble = fft.getEnergy("treble");   // 6000-20000 Hz

    // Amplitude level
    let level = amplitude.getLevel();       // 0.0 to 1.0

    // Peak detection
    // (PeakDetect monitors a frequency range for threshold crossings)
}
```

Under the hood, `p5.FFT` creates a Web Audio `AnalyserNode` with `fftSize = bins * 2`. The `analyze()` method calls `getByteFrequencyData()` which returns unsigned 8-bit values (0--255). The `getEnergy()` method maps named bands ("bass", "lowMid", "mid", "highMid", "treble") to predefined Hz ranges and averages the bins within that range.

### 3.3 Latency Characteristics

Web Audio's `AnalyserNode` operates with at least 2 buffer periods of latency (~128 samples per render quantum = ~2.9 ms at 44100 Hz, but the AnalyserNode typically has 2048-sample smoothing windows). In practice, p5.sound introduces 30--80 ms of audio-to-visual latency depending on browser, OS, and the `requestAnimationFrame` timing. This is acceptable for prototyping and installations but not for latency-critical performance contexts.

Processing (Java) achieves ~20--40 ms latency through JavaSound, which is better but still not competitive with native C++ solutions using CoreAudio or WASAPI exclusive mode.

### 3.4 Assessment

**Strengths**: Fastest path from idea to working prototype. Enormous community and educational resources. p5.js runs in any browser with zero installation. Good for sketching audio-visual mappings before implementing them in C++.

**Weaknesses**: Not production-grade latency. p5.js rendering performance hits a wall with complex visuals (Canvas2D is CPU-bound; WebGL mode helps but has its own overhead). Limited analysis depth: no MFCC, no pitch tracking, no onset detection in p5.sound (must use external libraries like Meyda.js). No system audio capture in browsers (only microphone input without extensions).

---

## 4. TouchDesigner

### 4.1 Architecture

[TouchDesigner](https://derivative.ca/) is a node-based visual programming environment built on a custom C++ engine. It organizes operators into five families:

| Family | Abbreviation | Domain |
|--------|-------------|--------|
| Channel | CHOP | Time-series data (audio, animation, sensor) |
| Texture | TOP | 2D images and video (GPU-resident) |
| Surface | SOP | 3D geometry |
| Data | DAT | Tables, text, scripts |
| Component | COMP | Containers, UI, networks |

The audio-visual pipeline is expressed as a graph of these operators with automatic data-type conversion between families.

### 4.2 Audio Analysis CHOPs

TouchDesigner provides several dedicated audio analysis CHOPs:

**AudioDeviceIn CHOP**: Captures audio from any system audio device. Supports ASIO, CoreAudio, WASAPI. Configurable sample rate and buffer size. Outputs raw PCM as channel samples.

**Audio Spectrum CHOP**: Computes the frequency spectrum of input channels via FFT. Two modes:
- **Visualization mode** (default): Emphasizes higher frequency levels and lower frequency ranges for perceptually meaningful display. Outputs magnitude as a single channel.
- **Time-to-Magnitude/Phase mode**: Full complex FFT output as `chan_m` (magnitude) and `chan_p` (phase) channels. Supports round-trip: spectrum manipulation followed by inverse FFT back to time domain.

**AudioBand CHOP**: Divides the spectrum into configurable frequency bands (e.g., sub-bass, bass, low-mid, mid, high-mid, presence, brilliance) and outputs the energy in each band as a separate channel. Band boundaries are user-configurable.

**AudioAnalysis Palette Component**: A higher-level component providing:
- Low / Mid / High level channels
- Kick and snare detection
- Rhythm extraction
- Spectral centroid
- Slow and fast spectral density

Additionally, the community-developed [audioAnalyzerCHOP](https://github.com/leozimmerman/audioAnalyzerCHOP) (by the same developer as ofxAudioAnalyzer) brings Essentia algorithms into TouchDesigner as a custom CHOP plugin, providing MFCC, mel bands, onset detection, and more.

### 4.3 Audio-to-Visual Pipeline

The typical audio-reactive workflow in TouchDesigner:

```
AudioDeviceIn CHOP → Audio Spectrum CHOP → Math CHOP (normalize)
    → CHOP-to-TOP (convert channels to texture)
    → GLSL TOP (custom shader reads texture as uniform)
    → Render TOP (final output)
```

CHOPs operate at the audio sample rate internally but are resampled to the project frame rate (typically 60 fps) when consumed by TOPs or SOPs. This resampling introduces up to one frame of latency (~16.7 ms at 60 fps).

### 4.4 Python and GLSL Integration

TouchDesigner embeds Python 3.x for scripting. Custom audio analysis can be implemented in Python DATs:

```python
# Script CHOP - executes per cook
import numpy as np

def onCook(scriptOp):
    audio = scriptOp.inputs[0]  # Input CHOP
    samples = np.array([audio[0][i] for i in range(audio.numSamples)])
    rms = np.sqrt(np.mean(samples ** 2))
    scriptOp.clear()
    scriptOp.appendChan('rms')
    scriptOp[0][0] = rms
```

GLSL shaders can be applied via GLSL TOP, GLSL MAT, or Compute Shader TOP. Audio data passed as textures (CHOP-to-TOP) or uniforms becomes available to fragment shaders for fully GPU-driven visualization.

### 4.5 Licensing

- **Non-Commercial**: Free, limited to 1280x1280 output resolution
- **Commercial**: ~$2,200 (perpetual license + annual maintenance)
- **Pro**: ~$4,400 (adds 4K+ output, multiple monitors, NDI output)
- **Educational**: Discounted

### 4.6 Assessment

**Strengths**: Fastest iteration cycle for audio-reactive visuals. GPU-accelerated throughout. Excellent for live performance and installations. The node graph makes the audio-to-visual mapping explicit and tweakable in real time. Built-in NDI, Spout, Syphon output for integration with other tools. Large VJ community.

**Weaknesses**: Proprietary and closed-source. Windows-primary (macOS version exists but lags in features and stability). Python scripting is slow for per-sample audio processing (fine for per-frame control logic). Cannot be embedded in another application. The internal audio analysis (without the Essentia CHOP plugin) is limited to spectrum and basic band energy -- no pitch tracking, no MFCC, no onset detection in the stock operators.

---

## 5. VVVV

### 5.1 Architecture

[VVVV](https://vvvv.org/) is a visual programming environment for real-time graphics, originating in the early 2000s. It has two variants:

- **vvvv beta**: The original, Windows-only, DirectX 9/11 based, uses a custom visual language
- **vvvv gamma**: The modern rewrite, built on .NET/C# (VL language), cross-platform, DirectX 11/Vulkan via Stride3D engine

vvvv gamma 7.0 (released 2025) represents the current mainline. It uses a dataflow graph where nodes ("patches") process data per frame.

### 5.2 Audio in VVVV Gamma

Audio is provided by the **VL.Audio** package (based on NAudio for .NET):

- Audio device input/output
- Audio file playback with automatic sample rate conversion
- Basic signal processing nodes (gain, mix, filter)
- FFT analysis nodes

The **VL.Audio.GPL** package (separate install, GPL-licensed) adds:
- Beat tracking
- Pitch analysis
- Additional spectral analysis algorithms

As of vvvv gamma 7.0, **VST3 plugin support** was added, allowing any VST3 audio plugin to be used as a node with audio I/O, parameter, and MIDI pins. This is significant because it means commercial audio analysis VST plugins (iZotope Insight, Voxengo SPAN, etc.) can feed data into the visual pipeline.

### 5.3 Rendering

vvvv gamma uses the **Stride3D** engine (open-source, C#, successor to Xenko) for rendering:

- Physically-based rendering (PBR)
- DirectX 11 and Vulkan backends
- Compute shader support
- Post-processing pipeline (bloom, SSAO, depth of field)
- VR support

Custom HLSL shaders can be written as TextureFX nodes, and compute shaders can process audio data on the GPU.

### 5.4 Assessment

**Strengths**: Modern architecture (vvvv gamma), strong 3D rendering via Stride3D, VST3 integration opens up professional audio analysis tools, active development, growing community. Exported patches can run as standalone executables.

**Weaknesses**: Smaller community than TouchDesigner or oF. Audio analysis depth in the stock packages is limited (VL.Audio.GPL helps but is not as comprehensive as Essentia). .NET/C# adds GC pauses that can cause frame drops in extreme cases. Documentation is improving but still patchy compared to TouchDesigner. The VL visual language has a learning curve.

---

## 6. Resolume Avenue/Arena

### 6.1 Audio-Reactive Architecture

[Resolume](https://resolume.com/) is a professional VJ software. Its audio analysis is relatively simple but deeply integrated into the effects pipeline:

- **BPM Sync**: Manual tap-tempo or automatic BPM detection. Drives animation speed, effect parameters, and clip triggering.
- **Audio Analysis**: Internal FFT-based analysis provides bass, mid, and treble energy levels. These can be mapped to any effect parameter via the dashboard.
- **Audio FFT → Effects**: The internal analyzer outputs a normalized spectrum that effects can consume. Effects parameters can be set to "audio reactive" mode with configurable frequency range and sensitivity.

### 6.2 FFGL Plugin Architecture

The **FreeFrame GL (FFGL)** plugin standard (version 2.0+) is Resolume's extension mechanism:

```cpp
// FFGL 2.0 Plugin Structure
class MyAudioPlugin : public CFFGLPlugin {
public:
    // Plugin receives audio FFT data automatically
    FFResult ProcessOpenGL(ProcessOpenGLStruct *pGL) override {
        // Access audio data
        const FFGLAudioData *audio = pGL->inputTextures[0]->audioData;
        if (audio && audio->samples) {
            // audio->samples: float array of FFT magnitudes
            // audio->numSamples: number of FFT bins
            float bass = averageBins(audio->samples, 0, 10);
            float treble = averageBins(audio->samples, 100, 512);
        }

        // Render with OpenGL using audio data
        glUseProgram(shaderProgram);
        glUniform1f(glGetUniformLocation(shaderProgram, "bass"), bass);
        // ...
        return FF_SUCCESS;
    }
};
```

FFGL 2.0 plugins:
- Receive audio FFT input natively (no separate audio capture needed)
- Run in the host's OpenGL context
- Can define parameters with custom ranges (not limited to 0.0--1.0)
- Support spinner and dropdown parameter types
- Must be compiled as universal binaries on macOS (ARM + x86_64) for Resolume 7.11.0+

### 6.3 OSC Integration

Resolume accepts OSC input on a configurable port. External audio analyzers can send feature data to Resolume via OSC to drive any parameter:

```
/composition/layers/1/clips/1/video/effect1/param1  [float: 0.0-1.0]
/composition/master/audio/level                      [float: 0.0-1.0]
```

This is the recommended approach for integrating a custom C++ audio analysis engine with Resolume: run the analyzer as a separate process, send normalized feature values via OSC, and map them to Resolume parameters.

### 6.4 Assessment

**Strengths**: Industry-standard VJ tool, rock-solid stability for live performance, FFGL plugin system allows custom effects, OSC integration enables external analysis engines, excellent multi-display and LED mapping support.

**Weaknesses**: Audio analysis is shallow (bass/mid/treble energy, BPM -- no spectral centroid, MFCC, onset, pitch). Closed-source, proprietary ($399 Avenue / $999 Arena). FFGL plugin development has a significant learning curve and limited documentation. Not suitable as a primary analysis engine -- better as a rendering target receiving data from an external analyzer.

---

## 7. Max/MSP + Jitter

### 7.1 Architecture

[Max](https://cycling74.com/products/max) (by Cycling '74, now part of Ableton) is a visual programming environment with three interleaved domains:

- **Max**: Event scheduling, MIDI, control flow
- **MSP**: Audio signal processing (sample-accurate, runs on audio thread)
- **Jitter**: Matrix and OpenGL operations (video, 3D, data)

Max has been in continuous development for over 30 years and has the deepest audio analysis ecosystem of any visual programming environment.

### 7.2 Audio Analysis Objects

MSP provides extensive analysis primitives:

| Object | Function |
|--------|----------|
| `fft~` / `ifft~` | Forward/inverse FFT |
| `pfft~` | Phase-locked FFT (overlap-add with custom sub-patches) |
| `spectroscope~` | Real-time spectrum display |
| `peakamp~` | Peak amplitude tracker |
| `average~` | Signal averaging |
| `zerox~` | Zero-crossing rate |
| `pitch~` | Pitch tracker (autocorrelation-based) |
| `sigmund~` | Sinusoidal analysis (pitch, loudness, partials) |
| `bonk~` | Onset/attack detector (by Miller Puckette) |
| `bark~` | Bark-scale spectral analysis |
| `fiddle~` | Pitch and amplitude follower (deprecated, use sigmund~) |

The `pfft~` object is particularly powerful: it creates a sub-patcher that runs in the frequency domain, processing each FFT bin as a separate signal. This allows arbitrary spectral manipulation without manually managing FFT/IFFT:

```
[pfft~ mySpectralPatch 1024 4]  // 1024-point FFT, 4x overlap

// Inside mySpectralPatch:
[fftin~ 1]           // Receive real part
[fftin~ 2]           // Receive imaginary part
[cartopol~]          // Convert to magnitude/phase
// ... process magnitude ...
[poltocar~]          // Convert back
[fftout~ 1]          // Send real part
[fftout~ 2]          // Send imaginary part
```

Third-party packages expand analysis further: **FluCoMa** (Fluid Corpus Manipulation) provides machine-learning-driven audio analysis including MFCC, spectral shape descriptors, novelty, and neural network embeddings. **Zsa.descriptors** provides a comprehensive set of audio descriptors.

### 7.3 Jitter for Visuals

Jitter processes N-dimensional matrices and OpenGL:

- `jit.gl.shader` for GLSL shaders
- `jit.gl.gridshape`, `jit.gl.mesh` for geometry
- `jit.gl.texture` for GPU textures
- `jit.gl.pix` for Gen-based pixel shaders (visual programming for shaders)
- `jit.gl.node` for scene graph hierarchy

Audio data flows to Jitter via `jit.catch~` (audio-to-matrix conversion) or direct snapshot~ to message conversion.

### 7.4 Assessment

**Strengths**: Deepest audio analysis ecosystem of any tool on this list. `pfft~` makes frequency-domain work intuitive. Massive community, decades of patches and examples. FluCoMa integration brings cutting-edge ML-driven analysis. Tight integration between audio and visuals in one environment. Ableton Live integration via Max for Live.

**Weaknesses**: Proprietary ($9.99/month or $399 perpetual). Visual programming becomes unwieldy for complex logic ("spaghetti patching"). Performance ceiling for CPU-intensive analysis (single-threaded per patcher). OpenGL rendering in Jitter is functional but not as polished as dedicated graphics frameworks. Not embeddable -- cannot be used as a library in a C++ application.

---

## 8. OSC (Open Sound Control) as Inter-Process Protocol

### 8.1 Protocol Overview

OSC is the de facto standard for sending real-time control data between audio/visual applications. It uses UDP (or TCP) transport with a hierarchical address namespace and typed arguments.

An OSC message consists of:
- **Address pattern**: A URI-like string (e.g., `/audio/features/rms`)
- **Type tag string**: Declares argument types (`f` = float32, `i` = int32, `s` = string, `b` = blob)
- **Arguments**: The actual data values, 4-byte aligned

OSC bundles group multiple messages with a shared timestamp for atomic delivery.

### 8.2 Why OSC for Audio-Visual Integration

For the architecture described in [ARCH_pipeline.md](ARCH_pipeline.md), OSC provides a clean inter-process boundary between the C++ audio analysis engine and any visual renderer:

```
┌─────────────────────┐     UDP/OSC      ┌─────────────────────┐
│  C++ Audio Engine   │ ───────────────→ │  Visual Renderer    │
│  (custom analysis)  │   localhost:9000  │  (TD / Resolume /   │
│                     │                  │   oF / custom GL)   │
└─────────────────────┘                  └─────────────────────┘
```

Advantages:
- Decouples analysis from rendering (independent crash domains, different languages)
- One analyzer can feed multiple renderers simultaneously (multicast)
- Standard protocol understood by TouchDesigner, Resolume, Max, vvvv, Processing, and virtually every creative coding tool
- Near-zero latency on localhost (UDP loopback is ~0.1 ms)

### 8.3 C++ OSC Libraries

**oscpack** ([source](https://github.com/RossBencina/oscpack)):
- Header-heavy C++ library by Ross Bencina
- Minimal dependencies (just platform socket headers)
- Provides `osc::OutboundPacketStream` for message construction and `UdpTransmitSocket` for sending
- No threading, no async -- you manage the send timing
- Easy to embed in any C++ project (just copy the source files)

**liblo** ([source](https://liblo.sourceforge.net/)):
- C library with C++11 wrapper (`lo::ServerThread`, `lo::Address`, `lo::Message`)
- Threaded server for receiving messages
- Supports multicast, broadcast, TCP
- More features than oscpack but heavier dependency (requires autotools build)

**oscpp** ([source](https://github.com/kaoskorobase/oscpp)):
- Header-only C++ library
- Zero-allocation message construction into pre-allocated buffers
- Ideal for real-time audio threads where allocation is forbidden

**lopack** ([source](https://github.com/danomatika/lopack)):
- C++ wrapper combining oscpack's API style with liblo's transport

### 8.4 Code Example: C++ OSC Sender for Audio Features

Using oscpack (recommended for embedding in the analysis engine due to simplicity):

```cpp
#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"

class AudioFeatureOSCSender {
public:
    AudioFeatureOSCSender(const char* host, int port)
        : socket_(IpEndpointName(host, port))
    {}

    // Call this from the analysis thread after each analysis frame
    void sendFeatures(const AudioFeatures& features) {
        char buffer[4096];
        osc::OutboundPacketStream p(buffer, sizeof(buffer));

        p << osc::BeginBundleImmediate

          // Scalar features
          << osc::BeginMessage("/audio/rms")
              << features.rms << osc::EndMessage
          << osc::BeginMessage("/audio/pitch")
              << features.pitchHz
              << features.pitchConfidence << osc::EndMessage
          << osc::BeginMessage("/audio/centroid")
              << features.spectralCentroid << osc::EndMessage
          << osc::BeginMessage("/audio/onset")
              << (features.isOnset ? 1 : 0) << osc::EndMessage
          << osc::BeginMessage("/audio/bpm")
              << features.bpm << osc::EndMessage

          // Vector features (mel bands as blob)
          << osc::BeginMessage("/audio/melbands")
              << osc::Blob(features.melBands.data(),
                           features.melBands.size() * sizeof(float))
              << osc::EndMessage

          // MFCC coefficients as individual floats
          << osc::BeginMessage("/audio/mfcc");
        for (float c : features.mfcc) {
            p << c;
        }
        p     << osc::EndMessage

          << osc::EndBundle;

        socket_.Send(p.Data(), p.Size());
    }

private:
    UdpTransmitSocket socket_;
};
```

### 8.5 OSC Message Format Conventions for Audio Features

A recommended namespace for audio features:

```
/audio/rms              f       [0.0 - 1.0]  Root mean square level
/audio/peak             f       [0.0 - 1.0]  Peak amplitude
/audio/pitch            ff      [Hz, confidence 0-1]
/audio/centroid         f       [Hz]          Spectral centroid
/audio/rolloff          f       [Hz]          Spectral rolloff (85%)
/audio/flatness         f       [0.0 - 1.0]  Spectral flatness
/audio/flux             f       [0.0 - ∞]    Spectral flux
/audio/onset            i       [0 or 1]     Onset detected
/audio/bpm              f       [BPM]        Estimated tempo
/audio/beat             i       [0 or 1]     Beat pulse
/audio/spectrum         b       [float blob]  Full magnitude spectrum
/audio/melbands         b       [float blob]  Mel band energies (24-40 bands)
/audio/mfcc             b       [float blob]  MFCC coefficients (13 values)
/audio/hpcp             b       [float blob]  Harmonic pitch class profile (12 values)
```

### 8.6 Network Latency Considerations

| Scenario | Typical Latency |
|----------|----------------|
| Localhost UDP loopback | 0.05 -- 0.2 ms |
| Same LAN (wired Ethernet) | 0.2 -- 1.0 ms |
| Same LAN (WiFi) | 1 -- 10 ms |
| WAN / Internet | 10 -- 200+ ms |

For the target latency budget of <20 ms ([ARCH_pipeline.md](ARCH_pipeline.md)), localhost or wired LAN OSC adds negligible overhead. The dominant latency is in the analysis itself (FFT window size / sample rate) and the rendering frame period.

**UDP packet size**: A typical feature bundle (10 scalar features + 40-band mel spectrum + 13 MFCC) fits in ~500 bytes, well under the MTU (1500 bytes for Ethernet). No fragmentation risk.

**Packet loss**: UDP provides no delivery guarantee. On localhost this is essentially zero. On LAN, occasional drops are acceptable because audio features are continuously re-sent every analysis frame (~10--30 ms). Stale data is preferable to blocking.

---

## 9. Comparison Matrix

### 9.1 Primary Comparison

| Framework | Language | Audio Analysis Depth | Visual Rendering | Typical A→V Latency | Extensibility | License | Platforms |
|-----------|----------|---------------------|------------------|---------------------|---------------|---------|-----------|
| **openFrameworks** | C++ | Deep (via ofxAudioAnalyzer/Essentia) | OpenGL 3.2+ / GLES | 5--15 ms | Addons (2000+) | MIT | macOS, Windows, Linux, iOS, Android |
| **Cinder** | C++17 | Basic (FFT, magnitude only) | OpenGL 3.2+ / Metal | 5--15 ms | Blocks (limited) | BSD 2-clause | macOS, Windows, Linux, iOS |
| **Processing** | Java | Moderate (Minim: FFT, beat) | OpenGL via JOGL | 20--40 ms | Libraries | GPL/LGPL | macOS, Windows, Linux |
| **p5.js** | JavaScript | Basic (FFT, amplitude) | Canvas2D / WebGL | 30--80 ms | npm packages | LGPL | Any browser |
| **TouchDesigner** | Visual/Python | Moderate (spectrum, bands, palette) | DirectX 11/12, OpenGL | 10--20 ms | CHOPs, Python | Proprietary ($0--$4,400) | Windows, macOS |
| **VVVV** | VL/C# | Basic--Moderate (VL.Audio, VST3) | DirectX 11, Vulkan | 10--20 ms | NuGet packages | Proprietary (free <$120K revenue) | Windows, (Linux planned) |
| **Resolume** | - | Basic (bass/mid/treble, BPM) | DirectX, OpenGL, Metal | 15--25 ms | FFGL plugins | Proprietary ($399--$999) | Windows, macOS |
| **Max/MSP** | Visual/JS/Gen | Very deep (pfft~, sigmund~, FluCoMa) | OpenGL via Jitter | 10--30 ms | Packages, externals | Proprietary ($10/mo or $399) | macOS, Windows |
| **Custom C++ + OSC** | C++ | Unlimited (Essentia, Aubio, FFTW) | Any (via OSC target) | 3--12 ms | Full control | Depends on libs | Any |

### 9.2 Audio Analysis Feature Support

| Feature | oF+Essentia | Cinder | Processing | p5.js | TouchDesigner | VVVV | Resolume | Max/MSP |
|---------|------------|--------|------------|-------|---------------|------|----------|---------|
| FFT Spectrum | Yes | Yes | Yes | Yes | Yes | Yes | Yes (internal) | Yes |
| Mel Bands | Yes (24/40) | No | No | No | No (stock) | No | No | Via externals |
| MFCC | Yes (13) | No | No | No | No (stock) | No | No | FluCoMa |
| Pitch (Hz) | Yes (YinFFT) | No | No | No | No (stock) | GPL pkg | No | sigmund~ |
| Onset Detection | Yes (HFC) | No | No | No | No (stock) | No | No | bonk~ |
| Beat/BPM | Via ofxAubio | No | BeatDetect | No | No (stock) | GPL pkg | Yes | Various |
| Spectral Centroid | Yes | User impl | No | No | Palette | No | No | Via zsa~ |
| Spectral Flux | Yes | No | No | No | No | No | No | User patch |
| HPCP / Chroma | Yes (12) | No | No | No | No | No | No | FluCoMa |
| Loudness (EBU) | Yes | No | No | No | No | No | No | Via ext |

### 9.3 Integration Characteristics

| Framework | Can Receive OSC | Can Send OSC | Embeddable as Library | Headless Mode | GPU Compute |
|-----------|-----------------|-------------|----------------------|---------------|-------------|
| openFrameworks | Yes (ofxOsc) | Yes | Partially (messy) | Yes (ofAppNoWindow) | Compute shaders |
| Cinder | Via addon | Via addon | No | No | Compute shaders |
| Processing | Via oscP5 | Yes | No | No | No |
| p5.js | Via WebSocket bridge | Yes | No | No | WebGPU (experimental) |
| TouchDesigner | Yes (built-in) | Yes | No | Yes (CLI mode) | Yes (Compute TOP) |
| VVVV | Yes (built-in) | Yes | No (but exports .exe) | Exported apps | Yes (Stride compute) |
| Resolume | Yes (built-in) | Yes | No | No | FFGL shaders |
| Max/MSP | Yes (built-in) | Yes | No | No | jit.gl.compute |
| Custom C++ | Via liblo/oscpack | Yes | N/A | Yes | Via OpenCL/CUDA/Vulkan |

---

## 10. Build vs. Extend Decision Framework

### 10.1 When to Build Custom

Build a custom C++ audio analysis engine when:

1. **Latency is critical**: Custom pipelines achieve 3--12 ms audio-to-visual latency by eliminating framework overhead. The lock-free ring buffer architecture in [ARCH_pipeline.md](ARCH_pipeline.md) provides deterministic timing that no general-purpose framework can match.

2. **Analysis depth matters**: You need specific combinations of features (e.g., MFCC + onset + pitch + custom descriptor) computed in a single optimized pass. Frameworks either offer shallow analysis (Cinder, p5.js, Resolume) or wrap Essentia/Aubio anyway (oF), at which point you gain nothing over direct library usage.

3. **Deployment constraints exist**: Embedded systems, custom hardware, or environments where a framework's runtime overhead is unacceptable. A minimal C++ binary linking Essentia, FFTW, and oscpack can be under 20 MB.

4. **Multiple visual targets**: If the same analysis must drive TouchDesigner, Resolume, a custom OpenGL renderer, and a web dashboard simultaneously, a standalone analyzer sending OSC/WebSocket is the cleanest architecture.

5. **Threading control**: Real-time audio requires careful thread priority management (see [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md)). Frameworks impose their own threading models that may conflict with optimal audio thread scheduling.

### 10.2 When to Extend an Existing Framework

Use an existing framework when:

1. **Prototyping speed matters more than latency**: p5.js or Processing gets a working audio-visual sketch in minutes. Use it to validate mapping ideas, then port to C++.

2. **The visual output is the primary deliverable**: If you need sophisticated 3D rendering, particle systems, physics, and post-processing, TouchDesigner or vvvv provides these out of the box. Building equivalent rendering in raw OpenGL takes weeks.

3. **Live performance workflow**: Resolume and TouchDesigner have production-tested workflows for managing clips, layers, transitions, and multi-display output. Replicating this infrastructure is a major engineering effort.

4. **The analysis requirements are simple**: If you only need FFT spectrum and amplitude, Cinder or oF provides this with zero additional dependencies.

5. **Team composition**: If the team includes artists comfortable with visual programming but not C++, Max/MSP or TouchDesigner will be more productive than a custom codebase.

### 10.3 Hybrid Architecture (Recommended)

The optimal architecture for a production audio-visual system combines both approaches:

```
┌─────────────────────────────────────────────────────────────────┐
│                    CUSTOM C++ ANALYSIS ENGINE                   │
│                                                                 │
│  Audio Input → Ring Buffer → Analysis Thread → Feature Store    │
│  (CoreAudio/     (lock-free)   (Essentia +      (atomic         │
│   WASAPI/JACK)                  Aubio +          snapshot)       │
│                                 custom)                         │
│                                                                 │
│  Feature Store → OSC Sender ─────→ UDP :9000                    │
│                → Shared Memory ──→ (same-machine renderer)      │
│                → WebSocket ──────→ (browser dashboard)          │
└─────────────────────────────────────────────────────────────────┘
         │              │                │
         ▼              ▼                ▼
   ┌──────────┐  ┌────────────┐  ┌─────────────┐
   │TouchDesign│  │ Resolume   │  │ Custom GL   │
   │er (visuals│  │ (VJ perf)  │  │ Renderer    │
   │+ mapping) │  │            │  │ (C++/OpenGL)│
   └──────────┘  └────────────┘  └─────────────┘
```

This architecture:
- Achieves the lowest possible analysis latency (custom C++ pipeline)
- Allows visual artists to work in their preferred tool (TD, Resolume, Max)
- Enables a custom OpenGL renderer for cases where framework overhead is unacceptable
- Keeps the analysis engine testable and deployable independently
- Supports multiple simultaneous visual outputs from one analysis stream

The shared memory path (for same-machine rendering) eliminates network serialization entirely, achieving sub-millisecond feature delivery. See [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md) for the custom OpenGL renderer design and [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) for how features drive visual parameters.

---

## References

- [ofxAudioAnalyzer - GitHub](https://github.com/leozimmerman/ofxAudioAnalyzer)
- [Cinder Audio Guide](https://libcinder.org/docs/guides/audio/index.html)
- [p5.sound Reference](https://p5js.org/reference/p5.sound/)
- [TouchDesigner Audio Spectrum CHOP](https://docs.derivative.ca/Audio_Spectrum_CHOP)
- [TouchDesigner audioAnalysis Palette](https://docs.derivative.ca/Palette:audioAnalysis)
- [vvvv gamma Audio Documentation](https://thegraybook.vvvv.org/reference/libraries/audio.html)
- [vvvv gamma 7.0 Release](https://vvvv.org/blog/2025/vvvv-gamma-7.0-release/)
- [Resolume FFGL Repository](https://github.com/resolume/ffgl)
- [Max/MSP Documentation](https://cycling74.com/products/max)
- [oscpack](https://github.com/RossBencina/oscpack)
- [liblo](https://liblo.sourceforge.net/)
- [oscpp](https://github.com/kaoskorobase/oscpp)
- [audioAnalyzerCHOP (Essentia for TouchDesigner)](https://github.com/leozimmerman/audioAnalyzerCHOP)
