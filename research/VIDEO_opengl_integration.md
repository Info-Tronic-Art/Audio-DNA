# OpenGL Integration for Real-Time Audio Visualization

> **Scope**: Complete technical reference for connecting the audio analysis pipeline to an OpenGL rendering backend. Covers GPU feature delivery mechanisms, thread synchronization between analysis and render threads, render loop architecture, GLSL shader patterns for audio-reactive visuals, JUCE OpenGL integration, modern OpenGL patterns, Vulkan/Metal considerations, and performance optimization. Target: sub-20ms audio-to-visual latency at 60fps.

> **Related documents**: [ARCH_pipeline.md](ARCH_pipeline.md) | [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) | [VIDEO_vj_frameworks.md](VIDEO_vj_frameworks.md) | [LIB_juce.md](LIB_juce.md) | [ARCH_audio_io.md](ARCH_audio_io.md)

---

## 1. Feature Delivery to GPU

The analysis thread produces a `FeatureSnapshot` (see [ARCH_pipeline.md](ARCH_pipeline.md)) containing scalar values (RMS, spectral centroid, BPM), fixed-size arrays (frequency bands, MFCCs), and variable-length buffers (full magnitude spectrum, mel spectrogram history). Each data shape maps to a different GPU upload mechanism. Choosing the wrong mechanism wastes either CPU cycles or GPU bandwidth.

```
┌─────────────────────────────────────────────────────────────────┐
│                  FEATURE DATA CLASSIFICATION                     │
├──────────────────┬───────────────┬───────────────────────────────┤
│  Data Shape      │  Size (bytes) │  GPU Mechanism                │
├──────────────────┼───────────────┼───────────────────────────────┤
│  Scalar          │  4            │  glUniform1f                  │
│  Small vec       │  8–64         │  glUniform{2,3,4}f            │
│  Band array      │  32–512       │  UBO (std140 layout)          │
│  Spectrum        │  2048–8192    │  1D texture (GL_R32F)         │
│  Mel spectrogram │  128×64–256   │  2D texture (GL_R32F)         │
│  Large feature   │  >16 KB       │  SSBO (std430 layout)         │
│  set / history   │               │                               │
└──────────────────┴───────────────┴───────────────────────────────┘
```

### 1.1 Shader Uniforms for Scalar Features

Shader uniforms are the simplest upload path. Each `glUniform*` call writes directly into the program's uniform storage. For a handful of scalars this is the fastest approach -- zero indirection, immediate availability in the shader.

#### C++ Upload

```cpp
// Called once per frame, after binding the shader program.
void uploadScalarFeatures(GLuint program, const FeatureSnapshot& snap) {
    // Cache uniform locations at init time -- glGetUniformLocation
    // is expensive and its result never changes for a linked program.
    static GLint loc_rms      = glGetUniformLocation(program, "u_rms");
    static GLint loc_centroid = glGetUniformLocation(program, "u_spectralCentroid");
    static GLint loc_bpm      = glGetUniformLocation(program, "u_bpm");
    static GLint loc_beatPhase= glGetUniformLocation(program, "u_beatPhase");
    static GLint loc_onset    = glGetUniformLocation(program, "u_onset");
    static GLint loc_time     = glGetUniformLocation(program, "u_time");

    glUniform1f(loc_rms,       snap.rms);
    glUniform1f(loc_centroid,  snap.spectralCentroid);
    glUniform1f(loc_bpm,       snap.bpm);
    glUniform1f(loc_beatPhase, snap.beatPhase);     // 0.0–1.0 sawtooth
    glUniform1f(loc_onset,     snap.onsetStrength);  // 0.0 = no onset
    glUniform1f(loc_time,      snap.timestampSec);
}
```

#### GLSL Declaration

```glsl
#version 410 core

// Scalar audio features -- uploaded via glUniform1f each frame
uniform float u_rms;
uniform float u_spectralCentroid;   // Hz, typically 200–8000
uniform float u_bpm;
uniform float u_beatPhase;          // 0.0–1.0 sawtooth synced to beat
uniform float u_onset;              // 0.0–1.0 onset detection strength
uniform float u_time;               // seconds since start
```

**Cost**: Each `glUniform1f` is roughly 50--200 ns on modern drivers (validated via NVIDIA Nsight). For 6 scalars, total upload is under 1.2 microseconds. Negligible.

**Caveat**: Uniform locations are per-program. If you switch programs mid-frame, you must re-upload. Consider UBOs for shared data across programs.

### 1.2 Uniform Buffer Objects (UBO) for Feature Arrays

When you have structured arrays -- frequency bands (8--32 floats), MFCCs (13--20 floats), chroma (12 floats) -- a UBO is the right tool. One `glBufferSubData` call uploads the entire block, and the same UBO can be bound to multiple shader programs simultaneously.

#### C++ Setup and Upload

```cpp
// ── Data layout (must match GLSL std140) ──────────────────────
// std140 rules: each vec4 starts on a 16-byte boundary.
// float arrays: each element occupies 16 bytes (padded to vec4).
// To avoid waste, pack floats into vec4s manually.

struct AudioFeatureBlock {
    // Pack 8 frequency bands into 2 vec4s
    float bands[8];          // offset 0, 8×4 = 32 bytes
    float padding0[8];       // pad each float to vec4 boundary?
    // Actually, std140 packs arrays of float as array of vec4.
    // So we must use vec4 packing explicitly:
};

// Better approach: use vec4 arrays to match std140 exactly.
struct alignas(16) AudioFeatureUBO {
    float bands[32];         // 8 vec4s = 8 × 16 = 128 bytes
                             // bands[0..3] = first vec4, etc.
    float mfccs[16];         // 4 vec4s = 64 bytes (13 used, 3 padding)
    float chroma[12];        // 3 vec4s = 48 bytes
    float padding_chroma[4]; // pad to vec4 boundary
    float rms;               // offset 244
    float centroid;
    float bpm;
    float beatPhase;         // completes a vec4
};
// Total: 128 + 64 + 48 + 16 + 16 = 272 bytes -- well under the
// minimum UBO size guarantee of 16 KB (GL_MAX_UNIFORM_BLOCK_SIZE).

// ── Initialization ────────────────────────────────────────────
GLuint uboAudioFeatures;

void initFeatureUBO() {
    glGenBuffers(1, &uboAudioFeatures);
    glBindBuffer(GL_UNIFORM_BUFFER, uboAudioFeatures);

    // GL_DYNAMIC_DRAW: updated every frame, read by GPU.
    glBufferData(GL_UNIFORM_BUFFER, sizeof(AudioFeatureUBO),
                 nullptr, GL_DYNAMIC_DRAW);

    // Bind to binding point 0 (shared across all programs).
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboAudioFeatures);
}

// ── Per-frame upload ──────────────────────────────────────────
void uploadFeatureUBO(const FeatureSnapshot& snap) {
    AudioFeatureUBO block{};

    // Pack frequency bands into vec4-aligned array.
    // std140: array stride for float[] is sizeof(vec4) = 16 bytes.
    // So bands[i] goes at byte offset i*16.
    for (int i = 0; i < 8; ++i) {
        block.bands[i * 4] = snap.frequencyBands[i];
        // bands[i*4+1..3] are padding (zeroed by value-init).
    }

    for (int i = 0; i < 13; ++i) {
        block.mfccs[i * 4] = snap.mfccs[i];  // Wait -- this only works
        // if we declared mfccs as float[16*4]. Let's fix the struct.
    }

    // ... Actually, the cleaner way is to match the GLSL layout:
    // Use float[N] in GLSL where each element has 16-byte stride.
    // On CPU, write at 16-byte intervals.

    // OR: use vec4 arrays in GLSL and pack manually:
    block.rms       = snap.rms;
    block.centroid  = snap.spectralCentroid;
    block.bpm       = snap.bpm;
    block.beatPhase = snap.beatPhase;

    glBindBuffer(GL_UNIFORM_BUFFER, uboAudioFeatures);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(AudioFeatureUBO), &block);
}
```

The std140 layout rules make packing arrays of scalars wasteful (each `float` element consumes 16 bytes). The practical approach is to use `vec4` arrays in GLSL and pack four band values per vec4:

#### GLSL UBO Declaration

```glsl
#version 410 core

layout(std140, binding = 0) uniform AudioFeatures {
    vec4 bands[2];       // bands[0].xyzw = bands 0–3, bands[1].xyzw = bands 4–7
    vec4 mfccs[4];       // mfccs[0].xyzw = coeffs 0–3, etc. (13 used, 3 padding)
    vec4 chroma[3];      // chroma[0].xyzw = C,C#,D,D#, etc.
    float rms;
    float spectralCentroid;
    float bpm;
    float beatPhase;
};

// Access: float band3 = bands[0].w;
// Access: float mfcc5 = mfccs[1].y;
```

#### Revised C++ Packing

```cpp
struct alignas(16) AudioFeatureUBO {
    float bands[8];      // packed as 2 × vec4 = 32 bytes
    float mfccs[16];     // 4 × vec4 = 64 bytes (indices 13–15 unused)
    float chroma[12];    // 3 × vec4 = 48 bytes
    float rms;
    float centroid;
    float bpm;
    float beatPhase;     // 4 floats = 1 vec4 = 16 bytes
};
// Total: 32 + 64 + 48 + 16 = 160 bytes. Fits trivially in any UBO.

void packFeatureUBO(AudioFeatureUBO& block, const FeatureSnapshot& snap) {
    std::memcpy(block.bands, snap.frequencyBands.data(), 8 * sizeof(float));
    std::memcpy(block.mfccs, snap.mfccs.data(), 13 * sizeof(float));
    std::memset(block.mfccs + 13, 0, 3 * sizeof(float));  // zero padding
    std::memcpy(block.chroma, snap.chroma.data(), 12 * sizeof(float));
    block.rms       = snap.rms;
    block.centroid  = snap.spectralCentroid;
    block.bpm       = snap.bpm;
    block.beatPhase = snap.beatPhase;
}
```

**Important**: When `vec4` arrays are declared in GLSL under `std140`, consecutive `vec4`s are tightly packed at 16-byte stride. So a C++ struct with `float[8]` maps directly to `vec4[2]` without gaps, as long as the struct itself is 16-byte aligned. This is the most memory-efficient UBO layout for audio features.

### 1.3 Texture-Based Feature Passing

Textures are the natural choice for spectral data because the shader gets hardware-accelerated linear interpolation for free via `GL_LINEAR` filtering. This is valuable when the spectrum has 1024 bins but the shader samples it at arbitrary frequencies.

#### 1D Texture: Magnitude Spectrum

```cpp
GLuint spectrumTex;

void initSpectrumTexture(int numBins) {
    glGenTextures(1, &spectrumTex);
    glBindTexture(GL_TEXTURE_1D, spectrumTex);

    // GL_R32F: single-channel 32-bit float.
    // No mipmaps needed -- we sample at full resolution.
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F,
                 numBins, 0, GL_RED, GL_FLOAT, nullptr);

    // LINEAR filtering gives free interpolation between bins.
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // CLAMP_TO_EDGE: prevent wrapping at spectrum boundaries.
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
}

void uploadSpectrum(const float* magnitudes, int numBins) {
    glBindTexture(GL_TEXTURE_1D, spectrumTex);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, numBins,
                    GL_RED, GL_FLOAT, magnitudes);
}
```

#### GLSL Sampling

```glsl
uniform sampler1D u_spectrum;

// Sample spectrum at a normalized frequency (0.0 = DC, 1.0 = Nyquist)
float getSpectrumAt(float normFreq) {
    return texture(u_spectrum, normFreq).r;
}

// Log-frequency sampling for perceptually uniform spacing
float getSpectrumLog(float normFreq) {
    // Map linear [0,1] to log-frequency space.
    // This gives more resolution to bass frequencies.
    float logFreq = pow(normFreq, 2.0);  // simple quadratic approximation
    return texture(u_spectrum, logFreq).r;
}
```

#### 2D Texture: Mel Spectrogram (Scrolling History)

For visualizations that need temporal context (spectrograms, "waterfall" displays), a 2D texture stores a rolling window of mel-frequency frames. The X axis is mel band index, the Y axis is time (frame history).

```cpp
static constexpr int kMelBands     = 128;
static constexpr int kHistoryDepth = 256;  // ~3 sec at 86 fps analysis rate

GLuint melSpectrogramTex;
int    melWriteRow = 0;  // circular write head

void initMelSpectrogram() {
    glGenTextures(1, &melSpectrogramTex);
    glBindTexture(GL_TEXTURE_2D, melSpectrogramTex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F,
                 kMelBands, kHistoryDepth, 0,
                 GL_RED, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  // wrap Y for circular buffer
}

void uploadMelFrame(const float* melBands) {
    glBindTexture(GL_TEXTURE_2D, melSpectrogramTex);

    // Overwrite one row in the circular buffer.
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, melWriteRow,              // x=0, y=current row
                    kMelBands, 1,                // width, height
                    GL_RED, GL_FLOAT, melBands);

    melWriteRow = (melWriteRow + 1) % kHistoryDepth;
}
```

#### GLSL Circular Sampling

```glsl
uniform sampler2D u_melSpectrogram;
uniform int   u_melWriteRow;       // current write head position
uniform float u_melHistoryDepth;   // 256.0

// Sample mel spectrogram at (melBand, framesAgo).
// framesAgo=0 is the most recent frame.
float getMelAt(float normBand, float framesAgo) {
    float row = float(u_melWriteRow) - framesAgo;
    float normRow = row / u_melHistoryDepth;  // GL_REPEAT handles wrapping
    return texture(u_melSpectrogram, vec2(normBand, normRow)).r;
}
```

**Cost**: `glTexSubImage1D` for 1024 floats is roughly 4 KB -- trivial DMA. The 2D mel upload is 512 bytes per frame (128 floats). The GPU texture cache is optimized for this access pattern.

### 1.4 SSBO (Shader Storage Buffer Objects) for Large Feature Sets

SSBOs (OpenGL 4.3+) are the unbounded alternative to UBOs. They support read-write access from shaders, arbitrarily large buffers (limited only by GPU memory), and `std430` layout (no vec4 padding waste for scalar arrays).

Use SSBOs when:
- The feature set exceeds the UBO size limit (typically 64 KB, minimum guaranteed 16 KB)
- You need write-back from compute shaders (e.g., GPU-side beat tracking)
- You want `std430` layout to avoid the `float[N]` padding problem

```cpp
struct FeatureSSBO {
    float magnitudeSpectrum[1024];   // std430: tightly packed, 4 bytes each
    float phaseSpectrum[1024];
    float melBands[128];
    float mfccs[13];
    float chromagram[12];
    float rms;
    float spectralCentroid;
    float spectralFlatness;
    float spectralRolloff;
    float zeroCrossingRate;
    float bpm;
    float beatPhase;
    float onsetStrength;
    int   frameIndex;
    float padding[2];               // pad to 16-byte alignment
};
// Total: (1024+1024+128+13+12+8+1)*4 + 8 = ~8900 bytes

GLuint featureSSBO;

void initFeatureSSBO() {
    glGenBuffers(1, &featureSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, featureSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(FeatureSSBO),
                 nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, featureSSBO);
}

void uploadFeatureSSBO(const FeatureSnapshot& snap) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, featureSSBO);
    // Map the buffer for writing. GL_MAP_INVALIDATE_BUFFER_BIT tells the
    // driver we will overwrite everything -- it can allocate a new buffer
    // internally to avoid stalling on the previous frame's GPU read.
    void* ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                 sizeof(FeatureSSBO),
                                 GL_MAP_WRITE_BIT |
                                 GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr) {
        auto* ssbo = static_cast<FeatureSSBO*>(ptr);
        std::memcpy(ssbo->magnitudeSpectrum,
                    snap.magnitudeSpectrum.data(), 1024 * sizeof(float));
        std::memcpy(ssbo->melBands, snap.melBands.data(), 128 * sizeof(float));
        ssbo->rms       = snap.rms;
        ssbo->beatPhase = snap.beatPhase;
        // ... remaining fields ...
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }
}
```

#### GLSL SSBO Declaration

```glsl
#version 430 core

layout(std430, binding = 0) readonly buffer AudioFeatures {
    float magnitudeSpectrum[1024];
    float phaseSpectrum[1024];
    float melBands[128];
    float mfccs[13];
    float chromagram[12];
    float rms;
    float spectralCentroid;
    float spectralFlatness;
    float spectralRolloff;
    float zeroCrossingRate;
    float bpm;
    float beatPhase;
    float onsetStrength;
    int   frameIndex;
};

// Access: float bass = magnitudeSpectrum[10];
// No vec4 padding -- std430 packs float[] at 4-byte stride.
```

**UBO vs SSBO decision matrix**:

| Criterion            | UBO                      | SSBO                         |
|----------------------|--------------------------|------------------------------|
| Max size             | 16 KB minimum (64 KB typical) | 128 MB+ (GPU VRAM)     |
| Layout               | std140 (wasteful for float[]) | std430 (tight packing)  |
| GPU access speed     | Fastest (uniform cache)  | Slightly slower (global memory) |
| Shader write-back    | No                       | Yes                          |
| Minimum GL version   | 3.1                      | 4.3                          |
| Recommended for      | <4 KB structured data    | >4 KB or when write needed   |

---

## 2. Thread Synchronization

The analysis thread and the render thread run at different rates and must never block each other. The analysis thread produces feature snapshots at the analysis hop rate (e.g., 86 Hz for a 512-sample hop at 44.1 kHz). The render thread consumes them at the display refresh rate (60 Hz with VSync, or unbounded without). These rates are not synchronized and neither thread has priority over the other for data access.

```
┌──────────────────────────────────────────────────────────────────────┐
│                   THREAD TIMING DIAGRAM                              │
│                                                                      │
│  Analysis    ╔═╗   ╔═╗   ╔═╗   ╔═╗   ╔═╗   ╔═╗   ╔═╗   ╔═╗       │
│  (86 Hz)     ╚═╝   ╚═╝   ╚═╝   ╚═╝   ╚═╝   ╚═╝   ╚═╝   ╚═╝      │
│              |11.6ms|                                                │
│                                                                      │
│  Render      ╔══╗       ╔══╗       ╔══╗       ╔══╗       ╔══╗       │
│  (60 Hz)     ╚══╝       ╚══╝       ╚══╝       ╚══╝       ╚══╝      │
│              |  16.6ms  |                                            │
│                                                                      │
│  Feature     ──A──B──C──D──E──F──G──H──I──J──K──                    │
│  snapshots        ↑           ↑           ↑                          │
│                   │           │           │                          │
│              Render reads D   Render reads G   Render reads J        │
│              (B,C skipped)    (E,F skipped)    (H,I skipped)         │
└──────────────────────────────────────────────────────────────────────┘
```

The render thread always reads the **latest** snapshot. Intermediate snapshots are silently dropped. This is correct for a visualization -- we never need "every" analysis frame, only the freshest one.

### 2.1 Double-Buffering with Atomic Swap

The simplest correct scheme uses two `FeatureSnapshot` buffers and an atomic index.

```cpp
#include <atomic>
#include <array>

struct FeatureSnapshot {
    float rms;
    float spectralCentroid;
    float bpm;
    float beatPhase;
    float onsetStrength;
    float frequencyBands[8];
    float mfccs[13];
    float chroma[12];
    float magnitudeSpectrum[1024];
    float timestampSec;
    uint64_t frameIndex;  // monotonic analysis frame counter
};

class FeatureDoubleBuffer {
public:
    // ── Producer API (analysis thread) ────────────────────────
    FeatureSnapshot& getWriteBuffer() {
        int writeIdx = 1 - readIdx_.load(std::memory_order_acquire);
        return buffers_[writeIdx];
    }

    void publishWrite() {
        int writeIdx = 1 - readIdx_.load(std::memory_order_acquire);
        // Release: all writes to buffers_[writeIdx] become visible
        // before the reader sees the swapped index.
        readIdx_.store(writeIdx, std::memory_order_release);
    }

    // ── Consumer API (render thread) ──────────────────────────
    const FeatureSnapshot& getReadBuffer() const {
        int idx = readIdx_.load(std::memory_order_acquire);
        return buffers_[idx];
    }

private:
    std::array<FeatureSnapshot, 2> buffers_{};
    std::atomic<int> readIdx_{0};
};
```

**How it works**:
1. The analysis thread writes into the buffer that is NOT currently pointed to by `readIdx_`.
2. After finishing the write, it atomically swaps `readIdx_` to point to the newly written buffer.
3. The render thread loads `readIdx_` and reads from that buffer.

**Correctness argument**: The analysis thread and render thread never access the same buffer simultaneously. The analysis thread writes to `buffers_[1 - readIdx_]`, which is by definition not the buffer the render thread is reading. The atomic release/acquire pair ensures the render thread sees fully written data when it loads the new index.

**Limitation**: If the analysis thread produces two snapshots between render frames, the first is silently overwritten. This is intentional -- we always want the freshest data. However, there is a subtle race: if the render thread is mid-read when the analysis thread calls `publishWrite()`, the render thread's `getReadBuffer()` call already returned a reference to the old read buffer, and it continues reading from that buffer safely. The analysis thread is now writing to the other buffer. No conflict.

### 2.2 Triple-Buffering for Smoother Operation

Double-buffering has a subtle issue: if the analysis thread is faster than the render thread and produces a new snapshot while the render thread is still reading, the analysis thread must wait (or skip) because both buffers are "in use" -- one being read, the other being the only write target but also the next read target.

Triple-buffering eliminates this by adding a third buffer:

```cpp
class FeatureTripleBuffer {
public:
    // ── Producer (analysis thread) ────────────────────────────
    FeatureSnapshot& getWriteBuffer() {
        return buffers_[writeIdx_];
    }

    void publishWrite() {
        // Swap the write buffer with the "middle" buffer.
        // The middle buffer becomes the latest available for reading.
        int mid = middleIdx_.exchange(writeIdx_, std::memory_order_acq_rel);
        writeIdx_ = mid;
        newDataAvailable_.store(true, std::memory_order_release);
    }

    // ── Consumer (render thread) ──────────────────────────────
    const FeatureSnapshot& getReadBuffer() {
        // If new data is available, swap our read buffer with the middle.
        if (newDataAvailable_.exchange(false, std::memory_order_acq_rel)) {
            int mid = middleIdx_.exchange(readIdx_, std::memory_order_acq_rel);
            readIdx_ = mid;
        }
        return buffers_[readIdx_];
    }

private:
    std::array<FeatureSnapshot, 3> buffers_{};
    int writeIdx_ = 0;                             // owned by writer thread
    int readIdx_  = 2;                             // owned by reader thread
    std::atomic<int>  middleIdx_{1};               // shared, atomic
    std::atomic<bool> newDataAvailable_{false};     // flag: new data ready
};
```

**Why triple is better**: The analysis thread always has a buffer to write into (it owns `writeIdx_`). The render thread always has a buffer to read from (it owns `readIdx_`). The `middleIdx_` acts as a mailbox -- the analysis thread drops new data there, the render thread picks it up when ready. No thread ever waits.

**Memory ordering details**: The `exchange` operations use `memory_order_acq_rel` because they are both a store (releasing the old buffer) and a load (acquiring the new buffer). This ensures that all writes to the buffer data are visible after the swap.

### 2.3 Memory Barriers Between Threads

On x86, store-release and load-acquire are free -- the x86 memory model (TSO: Total Store Order) provides these guarantees automatically. The `std::atomic` operations compile to plain MOV instructions with no fencing.

On ARM (Apple Silicon, Android), the situation is different. ARM has a weakly-ordered memory model. `memory_order_release` compiles to `STLR` (Store-Release), and `memory_order_acquire` compiles to `LDAR` (Load-Acquire). These are more expensive than plain stores/loads but much cheaper than full memory barriers (`DMB`).

**Never use `memory_order_relaxed` for the shared index.** Relaxed ordering on ARM can cause the render thread to see a stale index while reading partially written buffer data -- a classic torn-read bug that manifests as visual glitches (flickering values, NaN artifacts).

### 2.4 Practical Integration

```cpp
// Global shared state
FeatureTripleBuffer g_featureBus;

// ── Analysis thread main loop ─────────────────────────────────
void analysisThreadFunc(SPSCRingBuffer<float, 8192>& ringBuffer) {
    constexpr int kHopSize = 512;
    std::vector<float> analysisBlock(kHopSize);
    uint64_t frameCount = 0;

    while (running_.load(std::memory_order_relaxed)) {
        if (ringBuffer.available() >= kHopSize) {
            ringBuffer.try_pop(analysisBlock.data(), kHopSize);

            // Perform analysis (FFT, feature extraction, etc.)
            auto& snap = g_featureBus.getWriteBuffer();
            computeFeatures(analysisBlock.data(), kHopSize, snap);
            snap.frameIndex = ++frameCount;
            snap.timestampSec = frameCount * kHopSize / 44100.0;

            g_featureBus.publishWrite();
        } else {
            std::this_thread::yield();
        }
    }
}

// ── Render thread (called each frame) ─────────────────────────
void renderFrame(GLuint program) {
    const auto& snap = g_featureBus.getReadBuffer();

    // Upload to GPU using whichever mechanism is appropriate
    uploadScalarFeatures(program, snap);
    uploadSpectrum(snap.magnitudeSpectrum, 1024);

    // Draw
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}
```

---

## 3. Render Loop Architecture

### 3.1 Frame Timing and Rate Mismatch

The render loop runs at the display refresh rate (60 Hz with VSync enabled, 120 Hz or 144 Hz on high-refresh displays). The audio analysis rate depends on the hop size and sample rate:

```
Analysis rate = sampleRate / hopSize
             = 44100 / 512 ≈ 86.13 Hz
             = 48000 / 512 = 93.75 Hz
             = 44100 / 256 ≈ 172.27 Hz
```

At 86 Hz analysis vs 60 Hz render, roughly 1.43 analysis frames are produced per render frame. Some render frames will see one new analysis frame; others will see two. This is handled naturally by the triple buffer -- the render thread always gets the latest snapshot regardless of how many were produced.

### 3.2 Interpolating Features Between Analysis Frames

For smooth visuals, raw feature values can be temporally smoothed on the render thread. This is especially important for features like RMS and spectral centroid, which can change abruptly between analysis frames and cause visual jitter.

```cpp
struct SmoothedFeatures {
    float rms            = 0.0f;
    float spectralCentroid = 0.0f;
    float beatPhase      = 0.0f;
    float onsetStrength  = 0.0f;

    void update(const FeatureSnapshot& snap, float smoothing) {
        // Exponential moving average (EMA).
        // smoothing = 0.0: instant (no smoothing)
        // smoothing = 0.9: heavy smoothing (~10 frames to settle)
        rms            = rms * smoothing + snap.rms * (1.0f - smoothing);
        spectralCentroid = spectralCentroid * smoothing
                         + snap.spectralCentroid * (1.0f - smoothing);

        // Beat phase should NOT be smoothed -- it's a sawtooth.
        beatPhase = snap.beatPhase;

        // Onset: use attack/release envelope.
        // Fast attack (instantly jump to new peak), slow release (decay).
        if (snap.onsetStrength > onsetStrength) {
            onsetStrength = snap.onsetStrength;          // instant attack
        } else {
            onsetStrength *= 0.92f;                       // ~170ms decay at 60fps
        }
    }
};

SmoothedFeatures g_smoothed;

void renderFrame(GLuint program) {
    const auto& snap = g_featureBus.getReadBuffer();
    g_smoothed.update(snap, 0.7f);

    glUniform1f(loc_rms, g_smoothed.rms);
    glUniform1f(loc_centroid, g_smoothed.spectralCentroid);
    glUniform1f(loc_onset, g_smoothed.onsetStrength);
    glUniform1f(loc_beatPhase, g_smoothed.beatPhase);
    // ...
}
```

### 3.3 VSync and Frame Budget

With VSync enabled, `SwapBuffers` blocks until the next vertical blanking interval. The total frame budget is:

```
Budget = 1 / refreshRate
       = 1 / 60 = 16.667 ms   (60 Hz)
       = 1 / 120 = 8.333 ms   (120 Hz)
       = 1 / 144 = 6.944 ms   (144 Hz)
```

The render budget must accommodate:
1. Feature bus read + smoothing: ~1 microsecond
2. Uniform/texture upload: ~5--50 microseconds
3. Draw calls + GPU work: varies (target < 10 ms)
4. SwapBuffers: blocks until VSync

```
┌───────────────── 16.667 ms frame budget ─────────────────┐
│ Feature │ Upload │      GPU Render        │   VSync wait  │
│ read    │ to GPU │      (draw calls)      │   (idle)      │
│ <1 μs   │ ~50 μs │      ~5-12 ms          │   ~4-11 ms    │
└─────────┴────────┴────────────────────────┴───────────────┘
```

### 3.4 Handling Render Misses

If a frame takes longer than 16.6 ms, VSync forces the frame to wait for the next blanking interval -- effectively halving the frame rate to 30 fps for that frame. Strategies:

1. **Adaptive quality**: Monitor frame time with `glQueryCounter` (timestamp queries). If the average exceeds 14 ms, reduce particle counts, lower resolution, or disable post-processing effects.

2. **Frame time measurement**:

```cpp
GLuint queryStart, queryEnd;
glGenQueries(1, &queryStart);
glGenQueries(1, &queryEnd);

// Begin frame
glQueryCounter(queryStart, GL_TIMESTAMP);

// ... all rendering ...

glQueryCounter(queryEnd, GL_TIMESTAMP);

// Read results (from previous frame to avoid stalling)
GLuint64 startTime, endTime;
glGetQueryObjectui64v(queryStart, GL_QUERY_RESULT, &startTime);
glGetQueryObjectui64v(queryEnd, GL_QUERY_RESULT, &endTime);
float gpuTimeMs = (endTime - startTime) / 1e6f;

if (gpuTimeMs > 14.0f) {
    reduceVisualComplexity();
}
```

3. **Decoupled update/render**: Update feature smoothing at a fixed timestep regardless of render rate, using an accumulator pattern. This keeps visual motion consistent even if frames drop.

---

## 4. GLSL Shader Examples

### 4.1 Audio-Reactive Color: Spectrum to Gradient

Maps the magnitude spectrum to a color gradient across the screen. Low frequencies drive warm colors (left), high frequencies drive cool colors (right). Brightness modulated by RMS.

```glsl
#version 410 core

in vec2 v_uv;  // 0..1 across the screen
out vec4 fragColor;

uniform sampler1D u_spectrum;
uniform float u_rms;
uniform float u_spectralCentroid;
uniform float u_time;

// HSV to RGB conversion
vec3 hsv2rgb(vec3 c) {
    vec3 p = abs(fract(c.xxx + vec3(1.0, 2.0/3.0, 1.0/3.0)) * 6.0 - 3.0);
    return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

void main() {
    // Sample spectrum at horizontal position (log-frequency mapping)
    float freq = pow(v_uv.x, 2.5);  // compress bass, expand treble
    float magnitude = texture(u_spectrum, freq).r;

    // Map magnitude to brightness and height
    float bar = smoothstep(1.0 - magnitude * 2.0, 1.0 - magnitude * 2.0 + 0.02,
                           v_uv.y);

    // Color: hue driven by frequency position + spectral centroid offset
    float hue = v_uv.x * 0.7 + u_spectralCentroid / 10000.0;
    float saturation = 0.8;
    float value = magnitude * u_rms * 3.0;

    vec3 color = hsv2rgb(vec3(hue, saturation, value));

    // Add glow at the bar edge
    float glow = exp(-abs(v_uv.y - (1.0 - magnitude)) * 30.0) * magnitude;
    color += vec3(glow) * 0.5;

    fragColor = vec4(color * bar + color * glow, 1.0);
}
```

### 4.2 Beat-Pulsing Geometry

Uses the beat phase (0.0--1.0 sawtooth synced to detected BPM) to pulse geometry scale. The vertex shader scales vertices based on beat phase with an attack/decay envelope.

```glsl
#version 410 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

out vec3 v_normal;
out vec2 v_uv;

uniform mat4 u_mvp;
uniform float u_beatPhase;   // 0.0–1.0 sawtooth
uniform float u_rms;
uniform float u_onset;

void main() {
    // Beat envelope: sharp attack, exponential decay
    // phase=0 is the beat hit, phase=1 is just before next beat
    float envelope = exp(-u_beatPhase * 5.0);  // fast decay

    // Scale factor: base scale + beat pulse + onset spike
    float scale = 1.0 + envelope * 0.3 * u_rms + u_onset * 0.15;

    vec3 pos = a_position * scale;

    // Optional: add per-vertex displacement along normal
    pos += a_normal * envelope * u_rms * 0.1;

    v_normal = a_normal;
    v_uv = a_uv;
    gl_Position = u_mvp * vec4(pos, 1.0);
}
```

### 4.3 Frequency Band Visualization: Radial Bars

Renders frequency bands as bars arranged in a circle. Each bar's height is driven by its corresponding frequency band amplitude.

```glsl
#version 410 core

in vec2 v_uv;
out vec4 fragColor;

layout(std140, binding = 0) uniform AudioFeatures {
    vec4 bands[2];   // 8 frequency bands packed into 2 vec4s
    vec4 mfccs[4];
    vec4 chroma[3];
    float rms;
    float spectralCentroid;
    float bpm;
    float beatPhase;
};

uniform float u_time;

const float PI = 3.14159265;
const int NUM_BANDS = 8;

float getBand(int i) {
    return (i < 4) ? bands[0][i] : bands[1][i - 4];
}

void main() {
    vec2 uv = v_uv * 2.0 - 1.0;  // -1..1
    float angle = atan(uv.y, uv.x);
    float radius = length(uv);

    // Which band sector are we in?
    float sectorAngle = 2.0 * PI / float(NUM_BANDS);
    float adjustedAngle = angle + PI;  // 0..2PI
    int bandIdx = int(adjustedAngle / sectorAngle);
    bandIdx = clamp(bandIdx, 0, NUM_BANDS - 1);

    float bandValue = getBand(bandIdx);

    // Bar geometry
    float sectorFrac = mod(adjustedAngle, sectorAngle) / sectorAngle;
    float barWidth = smoothstep(0.0, 0.1, sectorFrac)
                   * smoothstep(1.0, 0.9, sectorFrac);

    float innerRadius = 0.2;
    float maxOuterRadius = 0.9;
    float outerRadius = innerRadius + bandValue * (maxOuterRadius - innerRadius);

    float inBar = step(innerRadius, radius) * step(radius, outerRadius) * barWidth;

    // Color per band
    vec3 color = hsv2rgb(vec3(float(bandIdx) / float(NUM_BANDS), 0.8, 1.0));
    color *= inBar;

    // Center glow driven by RMS
    float centerGlow = exp(-radius * 8.0) * rms * 2.0;
    color += vec3(centerGlow);

    fragColor = vec4(color, 1.0);
}
```

### 4.4 Particle System Driven by Onset Detection

Onset events spawn bursts of particles. The particle system uses instanced rendering (see section 6.2) with per-particle attributes stored in a VBO. The CPU updates the particle buffer based on onset detection.

```cpp
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float     life;      // 0.0 = dead, 1.0 = just spawned
    float     size;
};

class OnsetParticleSystem {
    static constexpr int MAX_PARTICLES = 10000;
    std::vector<Particle> particles_{MAX_PARTICLES};
    int nextParticle_ = 0;
    GLuint particleVBO_;

public:
    void init() {
        glGenBuffers(1, &particleVBO_);
        glBindBuffer(GL_ARRAY_BUFFER, particleVBO_);
        glBufferData(GL_ARRAY_BUFFER,
                     MAX_PARTICLES * sizeof(Particle),
                     nullptr, GL_DYNAMIC_DRAW);
    }

    void update(float dt, const FeatureSnapshot& snap) {
        // Spawn particles on onset
        if (snap.onsetStrength > 0.5f) {
            int spawnCount = static_cast<int>(snap.onsetStrength * 200);
            for (int i = 0; i < spawnCount; ++i) {
                auto& p = particles_[nextParticle_];
                p.position = glm::vec3(0.0f);
                p.velocity = randomDirection() * (snap.rms * 5.0f + 1.0f);
                p.life = 1.0f;
                p.size = snap.rms * 0.1f + 0.01f;
                nextParticle_ = (nextParticle_ + 1) % MAX_PARTICLES;
            }
        }

        // Update existing particles
        for (auto& p : particles_) {
            if (p.life <= 0.0f) continue;
            p.position += p.velocity * dt;
            p.velocity *= 0.98f;  // drag
            p.life -= dt * 0.5f;  // fade out over 2 seconds
        }

        // Upload to GPU
        glBindBuffer(GL_ARRAY_BUFFER, particleVBO_);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        MAX_PARTICLES * sizeof(Particle),
                        particles_.data());
    }
};
```

#### Particle Vertex Shader (Instanced)

```glsl
#version 410 core

// Per-vertex (quad corners)
layout(location = 0) in vec2 a_corner;  // [-1,-1] to [1,1]

// Per-instance (particle data)
layout(location = 1) in vec3 a_particlePos;
layout(location = 2) in vec3 a_particleVel;  // unused in shader, but occupies space
layout(location = 3) in float a_particleLife;
layout(location = 4) in float a_particleSize;

out float v_life;
out vec2  v_uv;

uniform mat4 u_viewProj;

void main() {
    v_life = a_particleLife;
    v_uv = a_corner * 0.5 + 0.5;

    if (a_particleLife <= 0.0) {
        gl_Position = vec4(0.0, 0.0, -999.0, 1.0);  // cull dead particles
        return;
    }

    // Billboard: expand quad in screen space
    vec4 clipPos = u_viewProj * vec4(a_particlePos, 1.0);
    vec2 offset = a_corner * a_particleSize * a_particleLife;
    clipPos.xy += offset * clipPos.w;  // scale in clip space

    gl_Position = clipPos;
}
```

#### Particle Fragment Shader

```glsl
#version 410 core

in float v_life;
in vec2  v_uv;
out vec4 fragColor;

uniform float u_rms;

void main() {
    if (v_life <= 0.0) discard;

    // Soft circle
    float dist = length(v_uv - 0.5) * 2.0;
    float alpha = smoothstep(1.0, 0.3, dist) * v_life;

    // Hot-to-cold color based on life
    vec3 hotColor  = vec3(1.0, 0.6, 0.1);  // orange
    vec3 coldColor = vec3(0.1, 0.3, 1.0);  // blue
    vec3 color = mix(coldColor, hotColor, v_life);

    // Brighten with RMS
    color *= 1.0 + u_rms * 2.0;

    fragColor = vec4(color, alpha);
}
```

### 4.5 Post-Processing: Audio-Modulated Bloom and Chromatic Aberration

A fullscreen post-processing pass that applies bloom (glow) scaled by RMS and chromatic aberration triggered by onsets.

```glsl
#version 410 core

in vec2 v_uv;
out vec4 fragColor;

uniform sampler2D u_sceneTexture;
uniform float u_rms;
uniform float u_onset;
uniform float u_beatPhase;

// Bloom: multi-tap box blur weighted by RMS
vec3 bloom(sampler2D tex, vec2 uv, float intensity) {
    vec2 texelSize = 1.0 / textureSize(tex, 0);
    vec3 sum = vec3(0.0);

    // 9-tap blur kernel
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize * 3.0;
            vec3 sample_c = texture(tex, uv + offset).rgb;
            // Only bloom bright pixels
            float brightness = dot(sample_c, vec3(0.2126, 0.7152, 0.0722));
            sum += sample_c * smoothstep(0.5, 1.0, brightness);
        }
    }
    return sum / 9.0 * intensity;
}

// Chromatic aberration: offset R and B channels
vec3 chromaticAberration(sampler2D tex, vec2 uv, float strength) {
    vec2 dir = uv - 0.5;  // radial direction from center
    float r = texture(tex, uv + dir * strength).r;
    float g = texture(tex, uv).g;
    float b = texture(tex, uv - dir * strength).b;
    return vec3(r, g, b);
}

void main() {
    // Base color with chromatic aberration on onsets
    float caStrength = u_onset * 0.02;  // max 2% of screen at full onset
    vec3 color = chromaticAberration(u_sceneTexture, v_uv, caStrength);

    // Add bloom scaled by RMS
    float bloomIntensity = u_rms * 4.0;
    color += bloom(u_sceneTexture, v_uv, bloomIntensity);

    // Beat-synced vignette
    float vignette = 1.0 - length(v_uv - 0.5) * (0.8 + exp(-u_beatPhase * 3.0) * 0.4);
    color *= vignette;

    // Tone mapping (simple Reinhard)
    color = color / (color + 1.0);

    fragColor = vec4(color, 1.0);
}
```

---

## 5. JUCE OpenGLContext Integration

JUCE provides `juce::OpenGLContext` and `juce::OpenGLRenderer` for rendering OpenGL content within a JUCE component. This is the recommended path for JUCE-based audio applications that need custom GPU visuals.

### 5.1 Architecture

JUCE's OpenGL rendering runs on a dedicated GL thread, separate from both the audio thread and the message (UI) thread. The data flow is:

```
┌────────────────┐     ┌──────────────────┐     ┌────────────────┐
│  Audio Thread   │     │  Message Thread   │     │  GL Thread      │
│  (real-time)    │     │  (UI events)      │     │  (rendering)    │
│                 │     │                   │     │                 │
│  processBlock() │     │  Component paint  │     │  renderOpenGL() │
│  → write to     │     │  JUCE 2D drawing  │     │  → read from    │
│    FeatureBus   │     │                   │     │    FeatureBus   │
│                 │     │                   │     │  → GL draw      │
└───────┬─────────┘     └───────────────────┘     └───────┬─────────┘
        │                                                  │
        └──────────── FeatureTripleBuffer ────────────────┘
                     (lock-free, atomic swap)
```

### 5.2 Implementation

```cpp
#include <juce_opengl/juce_opengl.h>

class AudioVisualizerComponent : public juce::Component,
                                  public juce::OpenGLRenderer {
public:
    AudioVisualizerComponent() {
        // Attach OpenGL context to this component.
        // This creates the GL thread.
        openGLContext_.setRenderer(this);
        openGLContext_.setContinuousRepainting(true);  // 60fps repaint
        openGLContext_.setComponentPaintingEnabled(false); // pure GL, no JUCE 2D overlay
        openGLContext_.attachTo(*this);
    }

    ~AudioVisualizerComponent() override {
        openGLContext_.detach();
    }

    // ── OpenGLRenderer callbacks (called on GL thread) ────────

    void newOpenGLContextCreated() override {
        // GL context is now current on this thread.
        // Compile shaders, create VAOs/VBOs, textures.
        compileShaders();
        initGeometry();
        initSpectrumTexture(1024);
    }

    void renderOpenGL() override {
        // Called once per frame on the GL thread.
        // GL context is current. Safe to call all GL functions.

        juce::OpenGLHelpers::clear(juce::Colours::black);

        const auto& snap = featureBus_.getReadBuffer();
        smoothed_.update(snap, 0.7f);

        // Upload features to GPU
        shader_->use();
        shader_->setUniform("u_rms", smoothed_.rms);
        shader_->setUniform("u_beatPhase", smoothed_.beatPhase);
        shader_->setUniform("u_onset", smoothed_.onsetStrength);
        shader_->setUniform("u_time", static_cast<float>(
            juce::Time::getMillisecondCounterHiRes() / 1000.0));
        shader_->setUniform("u_resolution",
            static_cast<float>(getWidth()),
            static_cast<float>(getHeight()));

        uploadSpectrum(snap.magnitudeSpectrum, 1024);

        // Draw fullscreen quad
        glBindVertexArray(quadVAO_);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void openGLContextClosing() override {
        // Clean up GL resources before context is destroyed.
        shader_.reset();
        glDeleteVertexArrays(1, &quadVAO_);
        glDeleteBuffers(1, &quadVBO_);
        glDeleteTextures(1, &spectrumTex_);
    }

    // ── Called from audio thread ──────────────────────────────
    void pushFeatures(const FeatureSnapshot& snap) {
        auto& buf = featureBus_.getWriteBuffer();
        buf = snap;
        featureBus_.publishWrite();
    }

private:
    juce::OpenGLContext openGLContext_;
    std::unique_ptr<juce::OpenGLShaderProgram> shader_;

    GLuint quadVAO_ = 0, quadVBO_ = 0;
    GLuint spectrumTex_ = 0;

    FeatureTripleBuffer featureBus_;
    SmoothedFeatures smoothed_;

    void compileShaders() {
        shader_ = std::make_unique<juce::OpenGLShaderProgram>(openGLContext_);

        // JUCE handles GL version differences and GLSL version prefixing.
        // On macOS, JUCE requests a 3.2 core profile by default.
        bool ok = shader_->addVertexShader(vertexShaderSource);
        jassert(ok);
        ok = shader_->addFragmentShader(fragmentShaderSource);
        jassert(ok);
        ok = shader_->link();
        jassert(ok);
    }

    void initGeometry() {
        // Fullscreen quad: 4 vertices, triangle strip
        float quadVertices[] = {
            // position (x,y),  uv (s,t)
            -1.0f, -1.0f,      0.0f, 0.0f,
             1.0f, -1.0f,      1.0f, 0.0f,
            -1.0f,  1.0f,      0.0f, 1.0f,
             1.0f,  1.0f,      1.0f, 1.0f,
        };

        glGenVertexArrays(1, &quadVAO_);
        glGenBuffers(1, &quadVBO_);

        glBindVertexArray(quadVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices),
                     quadVertices, GL_STATIC_DRAW);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(float), nullptr);
        // UV attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(float),
                              (void*)(2 * sizeof(float)));
    }
};
```

### 5.3 Connecting to the Audio Processor

```cpp
class AudioVisualizerProcessor : public juce::AudioProcessor {
public:
    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer&) override {
        // Push raw audio into the analysis pipeline ring buffer.
        const float* channelData = buffer.getReadPointer(0);
        int numSamples = buffer.getNumSamples();

        ringBuffer_.try_push(channelData, numSamples);

        // The analysis thread (separate from audio thread) consumes
        // from ringBuffer_ and publishes to the FeatureTripleBuffer.
        // The visualizer component reads from that buffer in renderOpenGL().
    }

    juce::AudioProcessorEditor* createEditor() override {
        return new AudioVisualizerEditor(*this);
    }

private:
    SPSCRingBuffer<float, 8192> ringBuffer_;
    // Analysis thread started in constructor, stopped in destructor.
};
```

**Key point**: The audio thread (`processBlock`) never directly writes to the GL thread's feature buffer. It writes raw samples to the ring buffer. The analysis thread (spawned at startup, separate from both audio and GL threads) reads from the ring buffer, computes features, and publishes to the `FeatureTripleBuffer`. The GL thread reads from the triple buffer in `renderOpenGL()`. Three threads, two lock-free data structures, zero mutexes.

---

## 6. Modern OpenGL Patterns

### 6.1 VAO/VBO Setup for Visualizations

Every visualization needs geometry. The two most common patterns:

**Fullscreen quad** (for fragment-shader-only visuals like spectrograms, waveforms rendered in the fragment shader):

```cpp
void initFullscreenQuad(GLuint& vao, GLuint& vbo) {
    float vertices[] = {
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}
```

**Dynamic geometry** (for waveform lines, bar charts with CPU-generated vertex data):

```cpp
void initDynamicLineBuffer(GLuint& vao, GLuint& vbo, int maxVertices) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Allocate with GL_DYNAMIC_DRAW -- updated every frame.
    glBufferData(GL_ARRAY_BUFFER, maxVertices * 2 * sizeof(float),
                 nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
}

void uploadAndDrawWaveform(GLuint vao, GLuint vbo,
                            const float* waveform, int numSamples) {
    // Convert samples to screen-space vertex positions
    std::vector<float> vertices(numSamples * 2);
    for (int i = 0; i < numSamples; ++i) {
        float x = (float)i / (numSamples - 1) * 2.0f - 1.0f;  // -1..1
        float y = waveform[i];  // already -1..1
        vertices[i * 2]     = x;
        vertices[i * 2 + 1] = y;
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    numSamples * 2 * sizeof(float), vertices.data());
    glDrawArrays(GL_LINE_STRIP, 0, numSamples);
}
```

### 6.2 Instanced Rendering for Particle Systems

Instanced rendering draws the same geometry (a quad) thousands of times with different per-instance attributes (position, color, size). This is far more efficient than individual draw calls.

```cpp
void setupInstancedParticles(GLuint& vao, GLuint& quadVBO,
                              GLuint& instanceVBO, int maxParticles) {
    // Quad vertices (shared by all instances)
    float quad[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(vao);

    // Per-vertex data (quad corners)
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Per-instance data (particle struct)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(Particle),
                 nullptr, GL_DYNAMIC_DRAW);

    // Particle position (vec3, location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void*)offsetof(Particle, position));
    glVertexAttribDivisor(1, 1);  // advance once per instance

    // Particle velocity (vec3, location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void*)offsetof(Particle, velocity));
    glVertexAttribDivisor(2, 1);

    // Particle life (float, location 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void*)offsetof(Particle, life));
    glVertexAttribDivisor(3, 1);

    // Particle size (float, location 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void*)offsetof(Particle, size));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);
}

// Draw all particles in one call
void drawParticles(GLuint vao, int activeCount) {
    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, activeCount);
}
```

### 6.3 Compute Shaders for Audio-Driven Simulations

OpenGL 4.3 compute shaders can run audio-reactive physics entirely on the GPU. This is ideal for large particle systems (100K+ particles) where CPU update is too slow.

```glsl
#version 430 core

layout(local_size_x = 256) in;

struct Particle {
    vec4 posLife;   // xyz = position, w = life
    vec4 velSize;   // xyz = velocity, w = size
};

layout(std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

layout(std430, binding = 1) readonly buffer AudioFeatures {
    float magnitudeSpectrum[1024];
    float rms;
    float beatPhase;
    float onsetStrength;
};

uniform float u_dt;
uniform float u_time;

// Pseudo-random hash
float hash(uint n) {
    n = (n << 13u) ^ n;
    n = n * (n * n * 15731u + 789221u) + 1376312589u;
    return float(n & 0x7fffffffu) / float(0x7fffffff);
}

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= particles.length()) return;

    Particle p = particles[idx];

    // Update
    p.posLife.xyz += p.velSize.xyz * u_dt;
    p.velSize.xyz *= 0.99;  // drag
    p.posLife.w   -= u_dt * 0.3;  // decay

    // Audio-reactive force: push particles outward on beats
    float beatForce = exp(-beatPhase * 4.0) * onsetStrength;
    vec3 dir = normalize(p.posLife.xyz + 0.001);
    p.velSize.xyz += dir * beatForce * 2.0 * u_dt;

    // Frequency-dependent color force (using spectrum to modulate velocity)
    float freqIdx = float(idx % 1024);
    float mag = magnitudeSpectrum[int(freqIdx)];
    p.velSize.xyz += vec3(0.0, mag * 0.5, 0.0) * u_dt;

    // Respawn dead particles
    if (p.posLife.w <= 0.0 && onsetStrength > 0.3) {
        float h = hash(idx + uint(u_time * 1000.0));
        float h2 = hash(idx * 7u + 13u);
        float h3 = hash(idx * 13u + 7u);

        p.posLife.xyz = vec3(0.0);
        p.velSize.xyz = normalize(vec3(h - 0.5, h2 - 0.5, h3 - 0.5))
                       * (rms * 3.0 + 0.5);
        p.posLife.w = 1.0;
        p.velSize.w = rms * 0.05 + 0.01;
    }

    particles[idx] = p;
}
```

#### Dispatch from C++

```cpp
void updateParticlesGPU(GLuint computeProgram, GLuint particleSSBO,
                         GLuint audioSSBO, int numParticles, float dt) {
    glUseProgram(computeProgram);
    glUniform1f(glGetUniformLocation(computeProgram, "u_dt"), dt);
    glUniform1f(glGetUniformLocation(computeProgram, "u_time"),
                glfwGetTime());

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, audioSSBO);

    int workGroups = (numParticles + 255) / 256;
    glDispatchCompute(workGroups, 1, 1);

    // Memory barrier: ensure compute writes are visible to vertex shader
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                    GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}
```

### 6.4 Framebuffer Objects for Post-Processing Chain

A post-processing chain renders the scene to an off-screen FBO, then applies audio-reactive effects as fullscreen passes.

```cpp
struct FBOTarget {
    GLuint fbo;
    GLuint colorTexture;
    int width, height;
};

FBOTarget createFBO(int width, int height) {
    FBOTarget target;
    target.width = width;
    target.height = height;

    glGenFramebuffers(1, &target.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);

    glGenTextures(1, &target.colorTexture);
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                 width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, target.colorTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(status == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return target;
}

// Post-processing pipeline
void renderWithPostProcessing(const FeatureSnapshot& snap) {
    // Pass 1: Render scene to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO_.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderScene(snap);

    // Pass 2: Bloom (horizontal blur) → second FBO
    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO_.fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    bloomHShader_->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneFBO_.colorTexture);
    glUniform1f(glGetUniformLocation(bloomHShader_->getID(), "u_rms"),
                snap.rms);
    drawFullscreenQuad();

    // Pass 3: Bloom (vertical blur) + composite → screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    compositeShader_->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneFBO_.colorTexture);      // original
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomFBO_.colorTexture);       // bloom
    glUniform1f(glGetUniformLocation(compositeShader_->getID(), "u_onset"),
                snap.onsetStrength);
    drawFullscreenQuad();
}
```

---

## 7. Vulkan and Metal Considerations

### 7.1 OpenGL vs Vulkan vs Metal for Audio-Visual Apps

| Aspect                    | OpenGL 4.x            | Vulkan                      | Metal                    |
|---------------------------|-----------------------|-----------------------------|--------------------------|
| Platform                  | Win/Linux/macOS*      | Win/Linux/Android           | macOS/iOS only           |
| macOS status              | Deprecated (4.1 max)  | Via MoltenVK (overhead)     | Native, first-class      |
| API complexity            | Low                   | Very high                   | Moderate                 |
| Driver overhead           | Moderate (driver does work) | Minimal (app does work) | Minimal                  |
| Threading model           | Single GL context per thread | Explicit multi-queue   | Command buffers, multi-thread |
| Compute shaders           | 4.3+ (not on macOS)   | Always                      | Always                   |
| Shader language           | GLSL                  | SPIR-V                      | MSL                      |
| Uniform upload            | glUniform* (implicit sync) | Descriptor sets + staging buffers | Argument buffers    |
| Best for prototype        | Yes                   | No                          | macOS-only OK? Yes       |

*macOS supports OpenGL up to 4.1 (deprecated since 10.14). No compute shaders, no SSBOs, no `std430` layout. This is a significant limitation.

### 7.2 When to Consider Vulkan or Metal

**Stay with OpenGL when**:
- Targeting Windows + Linux and prototyping is the priority
- Visual complexity is moderate (< 100K particles, < 5 post-processing passes)
- Team expertise is in OpenGL
- You want maximum reach via OpenGL 3.3 core (runs everywhere including old Intel GPUs)

**Move to Metal when**:
- macOS is the primary platform
- You need compute shaders on macOS (OpenGL 4.1 does not have them)
- You want the best performance on Apple Silicon (Metal is the native GPU API)
- JUCE supports Metal rendering context as of JUCE 7

**Move to Vulkan when**:
- You need explicit GPU memory management for very large feature buffers
- Your particle system exceeds 1M particles and you need multi-queue compute + render
- You are targeting Linux/Windows/Android and need consistent performance
- You want to use SPIR-V for shader distribution (no runtime compilation)

### 7.3 Abstraction Strategy

For a cross-platform VJ application, the practical approach is:

1. **Abstract the feature upload interface**: Define a `FeatureUploader` interface with methods like `uploadScalars()`, `uploadSpectrum()`, `uploadSSBO()`. Implement separately for OpenGL, Metal, and optionally Vulkan.

2. **Use a middleware library**: bgfx, Diligent Engine, or The Forge provide rendering abstraction across GL/Vulkan/Metal/DX12. They handle the boilerplate of buffer creation, shader compilation, and draw call submission.

3. **Start with OpenGL, profile, migrate hot paths**: If the visualizer is CPU-bound on feature smoothing and particle update, the graphics API choice does not matter. If it is GPU-bound on fill rate or compute, Vulkan/Metal will help -- but only after profiling confirms the bottleneck.

```cpp
// Abstract interface for graphics-API-agnostic feature upload
class IFeatureRenderer {
public:
    virtual ~IFeatureRenderer() = default;

    virtual void init(int windowWidth, int windowHeight) = 0;
    virtual void uploadFeatures(const FeatureSnapshot& snap) = 0;
    virtual void render(float dt) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void shutdown() = 0;
};

// OpenGL implementation
class OpenGLFeatureRenderer : public IFeatureRenderer { /* ... */ };

// Metal implementation (macOS)
class MetalFeatureRenderer : public IFeatureRenderer { /* ... */ };
```

---

## 8. Performance Optimization

### 8.1 Minimizing Uniform Uploads

Each `glUniform*` call has driver overhead (parameter validation, program state lookup). For a handful of scalars, this is negligible. For large numbers of uniforms, batch them:

1. **Use UBOs instead of individual uniforms**: One `glBufferSubData` call replaces N `glUniform*` calls. The driver can DMA the entire block in one operation.

2. **Cache uniform locations**: `glGetUniformLocation` parses the program's symbol table. Cache the result at shader link time, never call it per-frame.

3. **Skip unchanged uploads**: Track which features actually changed since the last frame. If the analysis thread did not produce a new snapshot (triple buffer's `newDataAvailable_` is false), skip all uploads entirely.

```cpp
void uploadFeaturesIfChanged(GLuint program, const FeatureTripleBuffer& bus) {
    static uint64_t lastFrameIndex = 0;

    const auto& snap = bus.getReadBuffer();
    if (snap.frameIndex == lastFrameIndex) {
        return;  // No new data -- skip all GL calls
    }
    lastFrameIndex = snap.frameIndex;

    // ... upload uniforms/UBOs/textures ...
}
```

### 8.2 Batching Feature Updates

If using textures for spectrum data, use Pixel Buffer Objects (PBO) for asynchronous upload. This allows the GPU to DMA the texture data while the CPU prepares the next frame.

```cpp
GLuint pboSpectrum[2];
int    pboIndex = 0;

void initSpectrumPBO(int numBins) {
    glGenBuffers(2, pboSpectrum);
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboSpectrum[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER,
                     numBins * sizeof(float), nullptr, GL_STREAM_DRAW);
    }
}

void uploadSpectrumAsync(const float* magnitudes, int numBins) {
    int nextPBO = 1 - pboIndex;

    // Start upload from current PBO (previously filled)
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboSpectrum[pboIndex]);
    glBindTexture(GL_TEXTURE_1D, spectrumTex);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, numBins,
                    GL_RED, GL_FLOAT, nullptr);  // offset 0 in PBO

    // Fill next PBO with new data (GPU is reading current PBO concurrently)
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboSpectrum[nextPBO]);
    void* ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0,
                                 numBins * sizeof(float),
                                 GL_MAP_WRITE_BIT |
                                 GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr) {
        std::memcpy(ptr, magnitudes, numBins * sizeof(float));
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    pboIndex = nextPBO;
}
```

### 8.3 GPU Profiling for Audio-Visual Apps

Two profiling dimensions matter: **GPU time per frame** and **CPU-GPU synchronization stalls**.

#### GPU Timestamp Queries

```cpp
// Ring buffer of timestamp queries to avoid pipeline stalls.
// We read results from 2 frames ago (queries need time to complete).
struct FrameTimingRing {
    static constexpr int RING_SIZE = 4;
    GLuint startQueries[RING_SIZE];
    GLuint endQueries[RING_SIZE];
    int writeHead = 0;
    int readHead = 0;
    int count = 0;

    void init() {
        glGenQueries(RING_SIZE, startQueries);
        glGenQueries(RING_SIZE, endQueries);
    }

    void beginFrame() {
        glQueryCounter(startQueries[writeHead], GL_TIMESTAMP);
    }

    void endFrame() {
        glQueryCounter(endQueries[writeHead], GL_TIMESTAMP);
        writeHead = (writeHead + 1) % RING_SIZE;
        count = std::min(count + 1, RING_SIZE);
    }

    // Returns GPU time in milliseconds, or -1 if not yet available.
    float readOldestResult() {
        if (count < 2) return -1.0f;  // need at least 2 frames buffered

        GLint available = GL_FALSE;
        glGetQueryObjectiv(endQueries[readHead],
                           GL_QUERY_RESULT_AVAILABLE, &available);
        if (!available) return -1.0f;

        GLuint64 start, end;
        glGetQueryObjectui64v(startQueries[readHead],
                              GL_QUERY_RESULT, &start);
        glGetQueryObjectui64v(endQueries[readHead],
                              GL_QUERY_RESULT, &end);

        readHead = (readHead + 1) % RING_SIZE;
        --count;

        return static_cast<float>(end - start) / 1e6f;  // ns → ms
    }
};
```

#### Key Performance Metrics to Track

```
┌─────────────────────────────────────────────────────────────────┐
│                    PERFORMANCE DASHBOARD                         │
├─────────────────────┬───────────────┬───────────────────────────┤
│  Metric             │  Target       │  Action if exceeded       │
├─────────────────────┼───────────────┼───────────────────────────┤
│  GPU frame time     │  < 12 ms      │  Reduce particles,        │
│                     │               │  lower FBO resolution     │
│  Feature upload     │  < 0.1 ms     │  Switch to PBO async      │
│  CPU render prep    │  < 2 ms       │  Reduce draw call count   │
│  Analysis latency   │  < 6 ms       │  Increase hop size        │
│  End-to-end latency │  < 20 ms      │  Profile entire chain     │
│  Frame drops/sec    │  < 1          │  Enable adaptive quality  │
│  VSync misses       │  0            │  Reduce GPU workload      │
└─────────────────────┴───────────────┴───────────────────────────┘
```

#### Adaptive Quality System

```cpp
class AdaptiveQuality {
    float gpuTimeEMA_ = 8.0f;  // start at a safe estimate
    int   particleCount_ = 10000;
    float fboScale_ = 1.0f;    // 1.0 = full resolution
    int   postProcessPasses_ = 3;

public:
    void update(float gpuTimeMs) {
        gpuTimeEMA_ = gpuTimeEMA_ * 0.9f + gpuTimeMs * 0.1f;

        if (gpuTimeEMA_ > 14.0f) {
            // Approaching budget -- reduce quality
            if (postProcessPasses_ > 1) {
                --postProcessPasses_;
            } else if (fboScale_ > 0.5f) {
                fboScale_ -= 0.1f;
            } else if (particleCount_ > 1000) {
                particleCount_ = particleCount_ * 3 / 4;
            }
        } else if (gpuTimeEMA_ < 10.0f) {
            // Headroom available -- increase quality
            if (particleCount_ < 10000) {
                particleCount_ = std::min(10000, particleCount_ * 5 / 4);
            } else if (fboScale_ < 1.0f) {
                fboScale_ = std::min(1.0f, fboScale_ + 0.1f);
            } else if (postProcessPasses_ < 3) {
                ++postProcessPasses_;
            }
        }
    }

    int   getParticleCount() const { return particleCount_; }
    float getFBOScale()      const { return fboScale_; }
    int   getPostPasses()    const { return postProcessPasses_; }
};
```

### 8.4 End-to-End Latency Budget

The total audio-to-visual latency is the sum of all pipeline stages:

```
┌────────────────────────────────────────────────────────────────────────┐
│               END-TO-END LATENCY BREAKDOWN                             │
│                                                                        │
│  Audio buffer    Analysis      Feature bus    Render + upload  Display  │
│  ┌──────────┐   ┌──────────┐  ┌──────────┐  ┌──────────────┐ ┌─────┐ │
│  │ 2.9 ms   │ → │ 3–6 ms   │→ │ ~0 ms    │→ │ 5–12 ms      │→│scan │ │
│  │(128 samp)│   │(FFT+feat)│  │(atomic)  │  │(upload+draw) │ │ out │ │
│  └──────────┘   └──────────┘  └──────────┘  └──────────────┘ └─────┘ │
│                                                                        │
│  Worst case total: 2.9 + 6 + 0 + 16.7 + 8 = ~34 ms                   │
│  Best case total:  2.9 + 3 + 0 + 5 + 0 = ~11 ms                      │
│  Target: < 20 ms average                                               │
│                                                                        │
│  Key: the render frame may arrive just after a VSync deadline,         │
│  adding up to 16.7 ms of display latency. This is the dominant         │
│  contributor and cannot be eliminated with VSync enabled.               │
│                                                                        │
│  Mitigation: use GL_NV_delay_before_swap or Wayland presentation       │
│  timestamps to submit frames as late as possible within the VSync      │
│  window, minimizing the gap between render and display.                 │
└────────────────────────────────────────────────────────────────────────┘
```

---

## Appendix A: Complete Minimal Example

A self-contained example using GLFW + OpenGL 4.1 that renders an audio-reactive fullscreen visualization. This combines the feature bus, spectrum texture, and reactive fragment shader from the sections above.

```cpp
// main.cpp -- Minimal audio-reactive OpenGL visualizer
// Requires: GLFW3, GLAD (or GLEW), a FeatureTripleBuffer producer

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// ... (include FeatureTripleBuffer, SmoothedFeatures from above) ...

extern FeatureTripleBuffer g_featureBus;  // populated by analysis thread

static const char* vertSrc = R"(
#version 410 core
layout(location=0) in vec2 a_pos;
layout(location=1) in vec2 a_uv;
out vec2 v_uv;
void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
)";

static const char* fragSrc = R"(
#version 410 core
in vec2 v_uv;
out vec4 fragColor;

uniform sampler1D u_spectrum;
uniform float u_rms;
uniform float u_beatPhase;
uniform float u_time;

vec3 hsv2rgb(vec3 c) {
    vec3 p = abs(fract(c.xxx + vec3(1.0, 2.0/3.0, 1.0/3.0)) * 6.0 - 3.0);
    return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

void main() {
    float freq = pow(v_uv.x, 2.5);
    float mag = texture(u_spectrum, freq).r;

    float pulse = exp(-u_beatPhase * 4.0);
    float height = mag * (1.0 + pulse * u_rms * 2.0);

    float bar = smoothstep(1.0 - height, 1.0 - height + 0.01, v_uv.y);
    float glow = exp(-abs(v_uv.y - (1.0 - height)) * 40.0) * mag;

    vec3 col = hsv2rgb(vec3(v_uv.x * 0.6 + u_time * 0.05, 0.85, mag + glow));
    fragColor = vec4(col * (bar + glow), 1.0);
}
)";

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // macOS

    GLFWwindow* window = glfwCreateWindow(1280, 720,
                                           "Audio Visualizer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);  // VSync

    // Compile shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertSrc, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragSrc, nullptr);
    glCompileShader(fs);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Fullscreen quad
    float quad[] = {
        -1,-1, 0,0,   1,-1, 1,0,   -1,1, 0,1,   1,1, 1,1
    };
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);

    // Spectrum texture
    GLuint specTex;
    glGenTextures(1, &specTex);
    glBindTexture(GL_TEXTURE_1D, specTex);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 1024, 0,
                 GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    SmoothedFeatures smoothed;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        const auto& snap = g_featureBus.getReadBuffer();
        smoothed.update(snap, 0.7f);

        // Upload spectrum
        glBindTexture(GL_TEXTURE_1D, specTex);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 1024,
                        GL_RED, GL_FLOAT, snap.magnitudeSpectrum);

        // Draw
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        glUniform1f(glGetUniformLocation(prog, "u_rms"), smoothed.rms);
        glUniform1f(glGetUniformLocation(prog, "u_beatPhase"),
                    smoothed.beatPhase);
        glUniform1f(glGetUniformLocation(prog, "u_time"),
                    (float)glfwGetTime());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, specTex);
        glUniform1i(glGetUniformLocation(prog, "u_spectrum"), 0);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(prog);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &specTex);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
```

---

## Appendix B: Quick Reference

| Task | Mechanism | GL Version | Upload Cost |
|------|-----------|------------|-------------|
| 1--6 scalar features | `glUniform1f` | 2.0+ | < 1 us |
| 8--32 band array | UBO (std140, vec4 packing) | 3.1+ | < 5 us |
| 1024-bin spectrum | 1D texture (GL_R32F) | 2.0+ | ~10 us |
| 128x256 mel spectrogram | 2D texture (row update) | 2.0+ | ~5 us/row |
| Full feature set (>4 KB) | SSBO (std430) | 4.3+ | ~20 us |
| 100K+ particle update | Compute shader | 4.3+ | 0 (GPU-only) |
| Async texture upload | PBO double-buffer | 2.1+ | ~0 us (async) |

---

*Cross-references*: For the upstream pipeline feeding features into this module, see [ARCH_pipeline.md](ARCH_pipeline.md). For mapping strategies from features to visual parameters, see [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md). For VJ framework options that wrap much of this OpenGL work, see [VIDEO_vj_frameworks.md](VIDEO_vj_frameworks.md). For JUCE-specific audio I/O and processor setup, see [LIB_juce.md](LIB_juce.md).
