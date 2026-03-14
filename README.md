# Audio-DNA

Real-time audio-reactive image effects engine. A VJ-style desktop application that analyzes audio and drives 75+ GLSL shader effects on images and live camera feeds.

Load a song and an image, wire audio features to visual effects, and watch the image react to the music in real-time at 60fps.

![Audio-DNA Screenshot](https://img.shields.io/badge/platform-macOS%20|%20Windows%20|%20Linux-blue) ![C++20](https://img.shields.io/badge/C%2B%2B-20-brightgreen) ![License](https://img.shields.io/badge/license-GPLv3-orange)

---

## What It Does

- Analyzes audio in real-time: RMS, spectral centroid, BPM, beat phase, onset detection, 7-band frequency analysis, MFCC, chroma, key detection, and more
- Applies any combination of 75+ visual effects to a loaded image or live camera feed
- Maps any audio feature to any effect parameter with configurable curves and smoothing
- Fullscreen VJ output on any connected display
- Beat-synced random effect switching for live performance
- Preset save/load, fast save slots, and full deck save/restore

## Features

### Audio Analysis (30+ features extracted in real-time)
- **Amplitude**: RMS, peak, dBFS, LUFS, crest factor
- **Spectral**: centroid, flux, flatness, rolloff, 7-band energies (sub through brilliance)
- **Rhythm**: BPM tracking, beat phase, onset detection, transient density
- **Pitch & Harmony**: dominant pitch, chroma (12 pitch classes), key detection, MFCC (13 coefficients), harmonic change
- **Structural**: buildup/drop/breakdown state machine

### 75+ Visual Effects (8 categories)

| Category | Effects |
|----------|---------|
| **3D / Depth** | Perspective Tilt, Cylinder Wrap, Sphere Wrap, Tunnel, Page Curl, Parallax Layers |
| **Warp** | Ripple, Bulge, Wave, Liquid, Kaleidoscope, Fisheye, Swirl, Polar Coords, Twirl, Shear, Elastic Bounce, Ripple Pond, Diamond Distort, Barrel Distort, Sine Grid, Glitch Displace |
| **Color** | Hue Shift, Saturation, Brightness, Duotone, Chromatic Aberration, Invert, Posterize, Color Shift, Thermal, Contrast, Sepia, Cross Process, Split Tone, Color Halftone, Dither, Heat Map, Selective Color, Film Grain, Gamma Levels, Solarize |
| **Glitch** | Pixel Scatter, RGB Split, Block Glitch, Scanlines, Digital Rain, Noise, Mirror, Pixelate |
| **Pattern / Style** | CRT, VHS, ASCII Art, Dot Matrix, Crosshatch, Emboss, Oil Paint, Pencil Sketch, Voronoi Glass, Cross Stitch, Night Vision |
| **Animation** | Strobe, Pulse, Slit Scan |
| **Blend / Composite** | Double Exposure, Frosted Glass, Prism, Rain on Glass, Hexagonalize |
| **Blur / Post** | Gaussian Blur, Zoom Blur, Shake, Vignette, Motion Blur, Glow, Edge Detect |

### Mapping System
- Route any audio feature to any effect parameter
- Curve transforms: Linear, Exponential, Logarithmic, S-Curve, Stepped
- Per-mapping smoothing (EMA / One-Euro filter)
- Adjustable input/output ranges

### Live Performance
- **Beat-synced randomization**: automatically switch effects on the beat (1, 2, 4, 8, 16, or 32 beats)
- **Per-effect lock**: protect effects from randomization
- **10 preset slots**: assign and recall effect setups instantly
- **Fast save**: one-click save without interrupting playback
- **Deck save/load**: save entire session (audio, image, effects, mappings, slot assignments)
- **Fullscreen output**: send to any connected display
- **Camera input**: live camera feed as texture source
- **Resolution lock**: set render resolution independent of window size with letterboxing

### Audio Sources
- Audio file playback (WAV, AIFF, MP3, FLAC, OGG) with loop
- Microphone / line input
- Drag-and-drop for audio and image files

---

## Download

Pre-built binaries are available from [GitHub Actions](https://github.com/Info-Tronic-Art/Audio-DNA/actions):

1. Click the latest **Build & Test** workflow run
2. Scroll to **Artifacts**
3. Download for your platform:
   - **Audio-DNA-Windows** — Windows 10/11
   - **Audio-DNA-macOS-ARM64** — Apple Silicon Macs
   - **Audio-DNA-macOS-x86_64** — Intel Macs
   - **Audio-DNA-Linux** — Ubuntu 22.04+

### macOS Note
If macOS blocks the app: right-click > Open > Open.

---

## Build From Source

### Prerequisites
- CMake 3.24+
- C++20 compiler
- Git

### macOS
```bash
xcode-select --install
brew install aubio
git clone https://github.com/Info-Tronic-Art/Audio-DNA.git
cd Audio-DNA
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(sysctl -n hw.ncpu)
./build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA
```

### Windows
```powershell
# Install Visual Studio 2022 with C++ workload
# Install vcpkg and aubio:
vcpkg install aubio:x64-windows

git clone https://github.com/Info-Tronic-Art/Audio-DNA.git
cd Audio-DNA
cmake -B build -DCMAKE_TOOLCHAIN_FILE="[vcpkg-root]/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
build\AudioDNA_artefacts\Release\Audio-DNA.exe
```

### Linux
```bash
sudo apt install libaubio-dev libasound2-dev libcurl4-openssl-dev \
  libfreetype6-dev libx11-dev libxcomposite-dev libxcursor-dev \
  libxinerama-dev libxrandr-dev libxrender-dev libwebkit2gtk-4.0-dev \
  libglu1-mesa-dev mesa-common-dev

git clone https://github.com/Info-Tronic-Art/Audio-DNA.git
cd Audio-DNA
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
./build/AudioDNA_artefacts/Release/Audio-DNA
```

---

## Quick Start

1. **Open an image** — click "Open Image" or drag-drop a PNG/JPG
2. **Open audio** — select "File" as audio source, click "Open Audio", pick a song
3. **Play** — hit Play or press Space
4. **Enable effects** — check effects in the right panel, tweak knobs
5. **Map audio to effects** — click "M" or "+" under a knob to open the mapping editor
6. **Randomize** — click "Random" in the Effects Rack to try random combinations
7. **Go fullscreen** — select a display from the Output dropdown, or press Cmd+F
8. **Save your setup** — "FX Save" for quick saves, "Deck Save" for full session

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Space | Play / Pause |
| Escape | Close output window / Stop |
| 1-9 | Toggle effects 1-9 |
| Cmd+S | Save preset |
| Cmd+O | Load preset |
| Cmd+F | Toggle fullscreen output |

---

## Architecture

Four-thread lock-free architecture designed for real-time audio-visual performance:

```
Audio Callback ──SPSC Ring Buffer──▸ Analysis Thread
Analysis Thread ──Triple Buffer────▸ Render Thread (OpenGL)
UI Thread ──────std::atomic─────────▸ Analysis / Render
```

- **Audio Callback** (RT priority): mono downmix, ring buffer push. <100μs budget.
- **Analysis Thread**: 2048-pt FFT, 12-stage feature pipeline. <2ms per hop.
- **Render Thread**: 60fps OpenGL 4.1, ping-pong FBO effect chain. <8ms budget.
- **UI Thread**: JUCE message loop, no blocking.

**Total audio-to-visual latency: ~15-25ms** (within the ±80ms perceptual sync window).

---

## Tech Stack

| Library | Purpose |
|---------|---------|
| [JUCE](https://juce.com/) 8.0 | Audio I/O, UI, OpenGL context, camera |
| [Aubio](https://aubio.org/) | BPM tracking, onset detection, pitch |
| OpenGL 4.1 / GLSL 410 | All visual effects rendering |
| CMake | Build system |

---

## License

GPLv3 — see [LICENSE](LICENSE) for details.

JUCE and Aubio are both GPLv3 licensed.
