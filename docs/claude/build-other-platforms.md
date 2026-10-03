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

### libprojectM (MilkDrop) -- patched 4.1.1 (BF10, s-rta-1002b)

**Why.** libprojectM 4.1.1's `projectm_opengl_render_frame` always draws its final picture into framebuffer 0 (the Preview panel), never into a canvas-sized framebuffer (Pitfall 66). The app therefore links a PATCHED 4.1.1: `cmake/projectm/0001-render-frame-fbo.patch` backports the upstream-shaped (4.2.0) `projectm_opengl_render_frame_fbo(handle, fbo)` onto the exact 4.1.1 commit (03aa8a7); nothing else in the engine changes (G0.6: same shader strings, same exports plus the one function, same VCS SHA).

**Install (macOS, once per machine).** `cmake/projectm/build-projectm.sh` clones github.com/projectM-visualizer/projectm at 03aa8a7 with its submodules, applies the patch, builds Release + shared and installs into its OWN prefix, `$HOME/.local/opt/projectm-4.1.1-fbo1` (`PREFIX=` overrides; the stock `~/.local` install is never touched). It is idempotent (exits 0 when the prefix already exports the function).
- **Network pre-check:** without `PROJECTM_SRC` it first runs `git ls-remote` with a 20 s timeout; on failure it prints `BLOCKED: no network -- set PROJECTM_SRC=<dir|tarball>` and **exits 3 (= BLOCKED)**.
- **Offline:** `PROJECTM_SRC=<dir|.tar.gz>` builds from a 4.1.1 source tree with its submodules (a git tree must be at 03aa8a7). The script keeps the unpatched source archive at `$HOME/.local/src/projectm-03aa8a7.tar.gz`; pass that path to rebuild offline.
- It configures from inside the source dir and passes `-DPROJECTM_VCS_VERSION`: projectM's `VCSVersion.cmake` otherwise embeds the CALLER's git HEAD.

**Configure guard.** `cmake/FindProjectM.cmake` searches `AUDIODNA_PROJECTM_PREFIX` (default the prefix above) FIRST and drops a cached `projectM4_DIR` outside it. `CMakeLists.txt` then checks that the found library has `projectm_opengl_render_frame_fbo`; if not, configure FAILS with "Run cmake/projectm/build-projectm.sh, then re-configure". A good configure prints `-- libprojectM: <...>/projectm-4.1.1-fbo1/lib/cmake/projectM4 (render_frame_fbo: yes)`. After a rebase onto this change, re-configure. CI never finds projectM, so it is unaffected.

**Upgrading.** Once an upstream release exports the same function, drop the patch, point the script (or a normal install) at that release, and re-run G1-G3 of `.harmony/.reports/s-rta-1002b/ruling-bf10.md` (`tests/test_projectm_canvas_gl.cpp`, `.harmony/probe-milkdrop.sh`, the perf A/B).

**License.** LGPL-2.1, dynamically linked, locally patched: `THIRD_PARTY_LICENSES.md`.

---

### Adding Dependencies

- Check `CMakeLists.txt` before adding any dependency
- Prefer JUCE built-in functionality over new libraries
- Any new runtime dependency must be justified against the "Why not X" column in the tech stack table in `docs/claude/architecture.md`

---
