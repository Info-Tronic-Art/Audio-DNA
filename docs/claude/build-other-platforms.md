# Build Instructions -- Windows / Linux / Extras

> Moved from CLAUDE.md (claudemd-split). macOS build essentials stay in CLAUDE.md; this covers the common build issues (moved s-rta-0928), the other platforms, Aubio setup and dependency policy.

---

### Common Build Issues

| Issue | Fix |
|-------|-----|
| `FetchContent` download fails | Check internet connection; JUCE repo is ~200MB |
| macOS: "OpenGL deprecated" warnings | Expected (GL 4.1 still works); suppress with `-Wno-deprecated` |
| Linux: missing X11/ALSA headers | The `apt` packages in `docs/claude/build-other-platforms.md` (Linux) |
| Windows: long path errors | `git config --system core.longpaths true` |

---

### Windows

```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
build\AudioDNA_artefacts\Release\Audio-DNA.exe
```

Required: Visual Studio 2022 with C++ workload.

---

### Linux

```bash
# Ubuntu/Debian — install JUCE dependencies
sudo apt install libasound2-dev libcurl4-openssl-dev libfreetype6-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxinerama-dev libxrandr-dev \
  libxrender-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev mesa-common-dev

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
./build/AudioDNA_artefacts/Release/Audio-DNA
```

---

### Adding Aubio (Milestone 2)

Aubio will be added via system install or FetchContent. On macOS: `brew install aubio`. On Linux: `sudo apt install libaubio-dev`. The `FindAubio.cmake` module will locate it.

---

### Adding Dependencies

- Check `CMakeLists.txt` before adding any dependency
- Prefer JUCE built-in functionality over new libraries
- Any new runtime dependency must be justified against the "Why not X" column in the tech stack table in `docs/claude/architecture.md`

---
