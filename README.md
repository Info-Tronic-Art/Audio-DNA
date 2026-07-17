# Audio-DNA

A cross-platform desktop application for live audio-reactive visual performance.
Built with C++20, JUCE 8, and OpenGL 4.1.

## What it does

Audio-DNA analyzes audio in real time — from a mic, system audio, or file — and drives
135 GLSL shader effects applied to images, videos, and 108 procedural sources via a
deck/layer/clip compositing system. Designed for live VJ performance at 60fps.

## Key features

- 14-stage audio analysis pipeline: BPM, beat phase, bar/phrase tracking, 7-band spectrum, MFCC, chroma, key, genre detection, and more
- 135 effects across 11 categories (warp, color, glitch, time, 3D, audio-native, etc.)
- 108 procedural sources (2D/3D fractals, audio-visual, noise, simulation, text)
- 15 clip-to-clip transitions
- Deck × Layer × Column compositing grid (Resolume-style)
- Universal signal routing: map any audio feature to any effect parameter
- MIDI/keyboard binding system with 3 targeting modes
- Fullscreen output to any display
- REST API (port 7070), OSC input, MIDI output (Launchpad/APC)
- Syphon output/input (macOS), video recording (H.264/ProRes/MJPEG)
- Ableton Link tempo sync (optional)

## Build

### macOS (primary)

```bash
brew install ffmpeg aubio
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(sysctl -n hw.ncpu)
./build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA
```

### Windows

```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### Linux

```bash
sudo apt install libasound2-dev libcurl4-openssl-dev libfreetype6-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxinerama-dev libxrandr-dev \
  libxrender-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev mesa-common-dev \
  libaubio-dev ffmpeg
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
```

## Documentation

- `CLAUDE.md` — Project instructions, architecture summary, development rules
- `ARCHITECTURE_V2.md` — Full v2 system design specification
- `PHASE_GUIDE.md` — Phase status tracker (P1-P25 complete, P26 tooltips pending)
- `LESSONS_LEARNED.md` — Verified bug fixes and hard-won lessons
- `.harmony/APP-INVENTORY.md` — Canonical counts + full surface/function inventory (source of truth)
- `design/FEATURE_INVENTORY.md` — Design-overhaul feature inventory (for UI redesign)
- `research/` — Audio analysis algorithms, library evaluations, implementation guides
- `design/` — UI mockups, UX analyses, competitive research

## Visual testing

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON
cmake --build build --config Release -j$(sysctl -n hw.ncpu)
./build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA --test-mode &
source .venv/bin/activate
AUDIODNA_NO_SPAWN=1 pytest tests/visual/ -v
```
