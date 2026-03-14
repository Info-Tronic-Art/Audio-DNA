# Audio-DNA

Real-time audio-reactive image effects engine. A VJ-style desktop application that analyzes audio and drives 75+ GLSL shader effects on images and live camera feeds.

Feed it audio from a microphone or audio file, give it an image or a folder of images, and watch the visuals react to the music in real-time at 60fps.

![Audio-DNA Screenshot](https://img.shields.io/badge/platform-macOS%20|%20Windows%20|%20Linux-blue) ![C++20](https://img.shields.io/badge/C%2B%2B-20-brightgreen) ![License](https://img.shields.io/badge/license-GPLv3-orange)

---

## What It Does

- Analyzes audio in real-time: RMS, spectral centroid, BPM, beat phase, onset detection, 7-band frequency analysis, MFCC, chroma, key detection, and more
- Applies any combination of 75+ visual effects to a loaded image, image slideshow, or live camera feed
- Maps any audio feature to any effect parameter with configurable curves and smoothing
- Fullscreen VJ output on any connected display
- Beat-synced random effect switching for live performance
- Instant preset save/recall without interrupting the show

---

## How to Use It

### 1. Set Up Your Output

Select where your visuals will be displayed. Use the **Output** dropdown in the top-right to send fullscreen video to a secondary monitor or projector. Use the **Viewport** dropdown to lock the render resolution. The **Video Level** slider lets you fade the output to black for smooth transitions.

### 2. Set Up Your Audio Input

Choose your audio source from the **Audio Source** dropdown:

- **Mic Input** — analyzes whatever your microphone picks up (live music, room audio, a speaker near your laptop)
- **Audio File** — load a WAV, MP3, FLAC, OGG, or AIFF file for analysis (audio is analyzed silently — nothing plays out of your speakers)

Use the **Gain** slider to boost or reduce input sensitivity. The level meter next to it shows your live input level.

> **Tip:** To analyze audio playing on your computer (Spotify, YouTube, etc.), install a virtual audio loopback driver like [BlackHole](https://github.com/ExistentialAudio/BlackHole) (macOS) or [VB-Cable](https://vb-audio.com/Cable/) (Windows). It will appear as a mic input option.

### 3. Load Your Visuals

You have three options for visual input:

- **Open Image** — load a single image (PNG, JPG, GIF, BMP, TIFF)
- **Image Folder** — load an entire folder of images that cycle automatically on the beat. Use the **Beats per Image** dropdown to set how many beats each image plays for (2 to 128)
- **Camera** — select a connected webcam or capture device for a live video feed

You can also drag and drop image files directly onto the window.

### 4. Let It Rip

Once audio is flowing and an image is loaded, the effects react to the music automatically. The left panel shows all audio analysis data in real-time — levels, spectrum, BPM, key, and more.

### 5. Shape Your Effects

Browse the **Effects Rack** on the right panel. Effects are organized by category (3D, Warp, Color, Glitch, Pattern, Animation, Blend, Blur). For each effect:

- **Toggle** the checkbox to enable/disable it
- **Turn the knobs** to adjust parameters manually
- **Click M / +** below a knob to open the **Mapping Editor** — wire any audio feature to that parameter with a curve, range, and smoothing
- **Click R** to randomize that single effect's parameters and mappings
- **Click L** to lock an effect — locked effects are protected from all randomization

### 6. Go Random

Click **Random** at the top of the Effects Rack for instant inspiration — it randomly enables effects, sets parameters, and creates audio mappings, all at once. Locked effects stay untouched.

For beat-synced chaos, enable the **Beats** toggle in row 2 and select how many beats between randomizations (1, 2, 4, 8, 16, or 32). Hit **Sync** to align the beat counter to the current downbeat. The visuals will transform on every Nth beat while your locked effects stay solid.

### 7. Save What You Like

When you land on a look you love, you don't have to stop anything:

- **FX Save** — instantly saves the current effect setup as a numbered preset (FX_Save_1, FX_Save_2, ...). No dialog, no interruption
- **Save / Load** — traditional preset save/load with a file chooser
- **Deck Save / Deck Load** — saves everything: your image, audio file, all effects, all mappings, and all slot assignments as a single `.deck.json` file

### 8. Build Your Performance

The **bottom bar** has 10 numbered preset slots. Use the dropdown next to each slot to assign a saved FX preset. During a performance, click any slot button to instantly recall that look. Build a palette of 10 looks and switch between them live.

### 9. Use the Mapping Editor

Click any **M** or **+** button under a knob to open the Mapping Editor. Here you can:

- Choose which **audio feature** drives the parameter (RMS, bass, beat phase, spectral centroid, onset strength, etc.)
- Select a **curve** shape (Linear, Exponential, Logarithmic, S-Curve, Stepped)
- Set **input range** (what audio values to respond to) and **output range** (how far the parameter moves)
- Adjust **smoothing** for how quickly the parameter responds
- Click **Randomize** to try a random mapping configuration
- Click **Delete** to remove the mapping

---

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
- Randomize button for instant mapping experiments

### Live Performance Tools
- **Beat-synced randomization** — automatically switch effects every 1, 2, 4, 8, 16, or 32 beats
- **Sync button** — align the beat counter to the current downbeat
- **Per-effect lock** — protect individual effects from randomization (green L indicator)
- **Per-effect randomize** — randomize a single effect's params and mappings (R button)
- **10 preset slots** — assign and recall saved effect setups instantly from the bottom bar
- **FX Save** — one-click save without interrupting the show
- **Deck save/load** — save and restore entire sessions (image, audio, effects, mappings, slots)
- **Fullscreen output** — send to any connected display or projector
- **Video level** — master brightness control for fade-to-black transitions
- **Image slideshow** — cycle through a folder of images on the beat (2 to 128 beats per image)
- **Camera input** — use a live camera feed as the visual source
- **Resolution lock** — set render resolution independent of window size with aspect-correct letterboxing

### Audio Sources
- Microphone or line input (with gain control and level metering)
- Audio file analysis (WAV, AIFF, MP3, FLAC, OGG) — silent, no audio output
- System audio via virtual loopback (BlackHole, VB-Cable)
- Drag-and-drop for audio and image files

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Escape | Close fullscreen output window |
| 1-9 | Toggle effects 1-9 |
| Cmd+S | Save preset |
| Cmd+O | Load preset |
| Cmd+F | Toggle fullscreen output |

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

## Architecture

Four-thread lock-free architecture designed for real-time audio-visual performance:

```
Audio Callback ──SPSC Ring Buffer──> Analysis Thread
Analysis Thread ──Triple Buffer────> Render Thread (OpenGL)
UI Thread ──────std::atomic─────────> Analysis / Render
```

- **Audio Callback** (RT priority): mono downmix, ring buffer push. <100us budget.
- **Analysis Thread**: 2048-pt FFT, 12-stage feature pipeline. <2ms per hop.
- **Render Thread**: 60fps OpenGL 4.1, ping-pong FBO effect chain. <8ms budget.
- **UI Thread**: JUCE message loop, no blocking.

**Total audio-to-visual latency: ~15-25ms** (within the +/-80ms perceptual sync window).

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

GPLv3 -- see [LICENSE](LICENSE) for details.

JUCE and Aubio are both GPLv3 licensed.
