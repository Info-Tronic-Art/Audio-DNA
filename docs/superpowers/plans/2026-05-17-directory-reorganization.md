# Directory Reorganization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reorganize the Audio-DNA project root to eliminate deprecated files, fix stale documentation counts, and create a clear `docs/` and `design/` hierarchy — without breaking any builds, path references, or workflows.

**Architecture:** Move files in git (so history is preserved), fix all hardcoded cross-references in the same commit batch, then fix stale numeric counts in CLAUDE.md and ARCHITECTURE_V2.md as a separate documentation pass.

**Tech Stack:** git mv (history-preserving moves), bash, direct file edits

---

### Task 1: Create target directory structure

**Files:**
- Create dirs: `docs/`, `docs/archive/`, `docs/archive/v1/`, `docs/feature_audit/`, `design/`, `design/mockups/`, `design/mockups/exports/`, `design/mockups/html/`, `design/ux_analyses/`, `tests/fixtures/`

- [ ] **Step 1: Create all target directories**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
mkdir -p docs/archive/v1 \
         docs/feature_audit \
         design/mockups/exports \
         design/mockups/html \
         design/ux_analyses \
         tests/fixtures
```

- [ ] **Step 2: Verify dirs created**

```bash
find docs design tests/fixtures -type d | sort
```
Expected: all 8 dirs listed above.

---

### Task 2: Archive dead v1 docs (git mv)

These files describe a keyboard-launcher app that no longer exists. They should be preserved in git history but removed from the active root.

**Files:**
- Move: `ARCHITECTURE.md` → `docs/archive/v1/ARCHITECTURE_V1.md`
- Move: `TASKPLAN.md` → `docs/archive/v1/TASKPLAN_V1.md`
- Move: `TASKPLAN_V2.md` → `docs/archive/TASKPLAN_V2.md`
- Move: `BUILDLOG.md` → `docs/archive/BUILDLOG.md`
- Move: `VALIDATION_PROTOCOL.md` → `docs/archive/VALIDATION_PROTOCOL.md`

- [ ] **Step 1: git mv the five dead files**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
git mv ARCHITECTURE.md docs/archive/v1/ARCHITECTURE_V1.md
git mv TASKPLAN.md docs/archive/v1/TASKPLAN_V1.md
git mv TASKPLAN_V2.md docs/archive/TASKPLAN_V2.md
git mv BUILDLOG.md docs/archive/BUILDLOG.md
git mv VALIDATION_PROTOCOL.md docs/archive/VALIDATION_PROTOCOL.md
```

- [ ] **Step 2: Verify they are staged as renames**

```bash
git status --short | grep -E "^R "
```
Expected: 5 lines starting with `R `.

---

### Task 3: Move .feature_audit slices to docs/feature_audit/

The leading-dot prefix makes the directory invisible in Finder and most editors.

**Files:**
- Move: `.feature_audit/*.md` → `docs/feature_audit/*.md`

- [ ] **Step 1: git mv each slice file**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
for f in .feature_audit/*.md; do
  git mv "$f" "docs/feature_audit/$(basename $f)"
done
```

- [ ] **Step 2: Verify**

```bash
ls docs/feature_audit/ | wc -l
```
Expected: 15

---

### Task 4: Move FEATURE_INVENTORY.md into docs/

The master inventory belongs with the other docs, not at the root.

**Files:**
- Move: `FEATURE_INVENTORY.md` → `docs/FEATURE_INVENTORY.md`

- [ ] **Step 1: git mv**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
git mv FEATURE_INVENTORY.md docs/FEATURE_INVENTORY.md
```

---

### Task 5: Move loose root PNGs to design/mockups/exports/

13 PNG files are sitting at the project root — they are UI mockup exports.

**Files:**
- Move: all `*.png` at root → `design/mockups/exports/`

- [ ] **Step 1: git mv all root PNGs**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
for f in *.png; do
  git mv "$f" "design/mockups/exports/$f"
done
```

- [ ] **Step 2: Verify none remain at root**

```bash
ls *.png 2>/dev/null && echo "FAIL — PNGs still at root" || echo "OK"
```
Expected: `OK`

---

### Task 6: Move research/mockups/ HTML files to design/mockups/html/

39 HTML mockup files live in `research/mockups/` — they are design artifacts, not research.

**Files:**
- Move: `research/mockups/*.html` → `design/mockups/html/`
- Move: `research/mockups/` subdirs if any

- [ ] **Step 1: git mv all HTML files**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
for f in research/mockups/*.html; do
  git mv "$f" "design/mockups/html/$(basename $f)"
done
# Move any remaining subdir
if [ -d "research/mockups/Good Audio DNA VJ UI" ]; then
  git mv "research/mockups/Good Audio DNA VJ UI" "design/mockups/html/Good Audio DNA VJ UI"
fi
```

- [ ] **Step 2: Verify research/mockups is empty or gone**

```bash
ls research/mockups/ 2>/dev/null && echo "files remain" || echo "OK — dir empty/gone"
```

---

### Task 7: Move UI/UX analysis MDs from research/ to design/ux_analyses/

These 12 files are competitive analysis for the new UI redesign — they belong in `design/`, not `research/`.

**Files to move** (all in `research/`):
- `Ableton_Live_12_UI_UX_Analysis.md`
- `Creative_Tools_UI_UX_Analysis.md`
- `DJ_Software_UI_UX_Analysis.md`
- `GrandMA3_UI_UX_Analysis.md`
- `Mapping_Software_UI_UX_Analysis.md`
- `Notch_Smode_UI_UX_Analysis.md`
- `OurAppFeatures_1.md`
- `RESOLUME_UI_UX_ANALYSIS.md`
- `TouchDesigner_UI_UX_Analysis.md`
- `UI_UX_LIVE_PERFORMANCE.md`
- `UI_UX_MASTER_COMPARISON.md`
- `VJ_Software_UI_UX_Analysis.md`

- [ ] **Step 1: git mv each file**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
for f in \
  "Ableton_Live_12_UI_UX_Analysis.md" \
  "Creative_Tools_UI_UX_Analysis.md" \
  "DJ_Software_UI_UX_Analysis.md" \
  "GrandMA3_UI_UX_Analysis.md" \
  "Mapping_Software_UI_UX_Analysis.md" \
  "Notch_Smode_UI_UX_Analysis.md" \
  "OurAppFeatures_1.md" \
  "RESOLUME_UI_UX_ANALYSIS.md" \
  "TouchDesigner_UI_UX_Analysis.md" \
  "UI_UX_LIVE_PERFORMANCE.md" \
  "UI_UX_MASTER_COMPARISON.md" \
  "VJ_Software_UI_UX_Analysis.md"; do
  git mv "research/$f" "design/ux_analyses/$f"
done
```

- [ ] **Step 2: Verify count**

```bash
ls design/ux_analyses/ | wc -l
```
Expected: 12

---

### Task 8: Move research/Resolume/ and Resolume assets to design/ux_analyses/

`research/Resolume/` subfolder and the two Resolume artefacts at research root.

**Files:**
- Move: `research/Resolume/` → `design/ux_analyses/Resolume/`
- Move: `research/Resolume Screen.png` → `design/ux_analyses/Resolume Screen.png`
- Move: `research/ArKaos-VJ-DMX_1.png` → `design/ux_analyses/ArKaos-VJ-DMX_1.png`
- Move: `research/ArKaos-3.6.1-ReleaseNotes.pdf` → `design/ux_analyses/ArKaos-3.6.1-ReleaseNotes.pdf`

- [ ] **Step 1: git mv**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
git mv "research/Resolume" "design/ux_analyses/Resolume"
git mv "research/Resolume Screen.png" "design/ux_analyses/Resolume Screen.png"
git mv "research/ArKaos-VJ-DMX_1.png" "design/ux_analyses/ArKaos-VJ-DMX_1.png"
git mv "research/ArKaos-3.6.1-ReleaseNotes.pdf" "design/ux_analyses/ArKaos-3.6.1-ReleaseNotes.pdf"
```

---

### Task 9: Rename "media to test with" folder

The space-in-name folder causes quoting issues. Rename to `media/`.

**Files:**
- Move: `"media to test with"/` → `media/`

- [ ] **Step 1: git mv**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
git mv "media to test with" media
```

- [ ] **Step 2: Verify**

```bash
ls -d media/
```
Expected: `media/`

---

### Task 10: Fix cross-reference paths in CLAUDE.md

CLAUDE.md has several stale references that the file moves above would break or that were already wrong.

**Files:**
- Modify: `CLAUDE.md`

Fixes needed:
1. Remove `src/keyboard/KeySlot.h` line from source tree (directory was removed in P9)
2. Fix JUCE version: `7.0.12` → `8.0.4`
3. Fix source count: `81 procedural sources` → `108 procedural sources`
4. Fix pipeline stage count: wherever it says "15 stages" in body text → `14`
5. Add deprecation note about `ARCHITECTURE.md` → now at `docs/archive/v1/ARCHITECTURE_V1.md`

- [ ] **Step 1: Remove the keyboard/ line from the source tree section**

Find and remove this line:
```
│   └── KeySlot.h                    # [M7] Per-key data model (media, effects, transparency, latch/random)
```

- [ ] **Step 2: Fix JUCE version string**

Replace `7.0.12` with `8.0.4` in the tech stack table.

- [ ] **Step 3: Fix source count in Project Identity paragraph**

Replace `81 procedural sources` with `108 procedural sources` in the opening description paragraph.

- [ ] **Step 4: Fix pipeline stage count in analysis section**

The analysis pipeline order section lists 16 numbered items (1–16) but the intro text says "15". Remove the text reference; let the numbered list speak for itself.

- [ ] **Step 5: Add archive note to the "Before Any Work" section**

After the bullet "Always read this CLAUDE.md before touching any file", add:
```
- Note: `ARCHITECTURE.md` (v1 keyboard launcher) has been moved to `docs/archive/v1/ARCHITECTURE_V1.md`. The current design spec is `ARCHITECTURE_V2.md`.
```

---

### Task 11: Fix stale counts in ARCHITECTURE_V2.md

**Files:**
- Modify: `ARCHITECTURE_V2.md`

- [ ] **Step 1: Fix source count in §19**

Find the line that says "40 sources" and replace with "108 sources".

- [ ] **Step 2: Fix effect count in §27**

Find "76 effects" and replace with "135 effects".

- [ ] **Step 3: Fix codec description in §24**

Find the reference to "HAP Alpha" as the primary recording codec. Replace with:
"H.264 (default), ProRes, MJPEG via FFmpeg. HAP Alpha is not implemented."

---

### Task 12: Fix research/INDEX.md document count

**Files:**
- Modify: `research/INDEX.md`

- [ ] **Step 1: Fix the document count**

Find "29 documents" (or whatever the stated count is) and replace with "36 documents" (48 minus the 12 UI/UX analyses now in design/).

- [ ] **Step 2: Remove entries for the 12 files moved to design/ux_analyses/**

Remove index entries for:
- Ableton_Live_12_UI_UX_Analysis.md
- Creative_Tools_UI_UX_Analysis.md
- DJ_Software_UI_UX_Analysis.md
- GrandMA3_UI_UX_Analysis.md
- Mapping_Software_UI_UX_Analysis.md
- Notch_Smode_UI_UX_Analysis.md
- OurAppFeatures_1.md
- RESOLUME_UI_UX_ANALYSIS.md
- TouchDesigner_UI_UX_Analysis.md
- UI_UX_LIVE_PERFORMANCE.md
- UI_UX_MASTER_COMPARISON.md
- VJ_Software_UI_UX_Analysis.md

Add a line pointing to the new location:
```
UI/UX competitive analysis documents have been moved to `design/ux_analyses/`.
```

---

### Task 13: Rewrite README.md for v2

The current README describes the v1 keyboard launcher — "75+ effects", "8 preset slots", "Effects Rack panel" — none of which exist.

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Rewrite README to describe the current v2 app**

Replace the entire content with:

```markdown
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
- `docs/FEATURE_INVENTORY.md` — Complete feature inventory (for UI redesign)
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
```

---

### Task 14: Commit everything

- [ ] **Step 1: Stage all untracked new dirs**

```bash
cd /Users/boriskarpman/Documents/RealTimeAudio
git add docs/ design/ media/ tests/fixtures/
```

- [ ] **Step 2: Verify full staged diff looks correct**

```bash
git status --short | head -60
```

- [ ] **Step 3: Commit**

```bash
git commit -m "$(cat <<'EOF'
chore: reorganize docs + design + archive v1 artifacts

- Archive v1 files (ARCHITECTURE.md, TASKPLAN.md, TASKPLAN_V2.md,
  BUILDLOG.md, VALIDATION_PROTOCOL.md) → docs/archive/
- Move .feature_audit/ → docs/feature_audit/ (was hidden dot-dir)
- Move FEATURE_INVENTORY.md → docs/
- Move 13 root PNGs → design/mockups/exports/
- Move research/mockups/*.html → design/mockups/html/
- Move 12 UI/UX analysis MDs → design/ux_analyses/
- Move Resolume assets → design/ux_analyses/
- Rename "media to test with" → media/
- Fix stale counts: CLAUDE.md (sources 108, JUCE 8.0.4, remove
  keyboard/ ref), ARCHITECTURE_V2.md (sources 108, effects 135),
  research/INDEX.md (updated count + removed moved entries)
- Rewrite README.md to describe current v2 app

Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
EOF
)"
```

---

## Self-Review

**Spec coverage check:**
- ✅ Archive 5 dead files → Task 2
- ✅ Rename .feature_audit/ → Task 3
- ✅ Move FEATURE_INVENTORY.md → Task 4
- ✅ Move 13 root PNGs → Task 5
- ✅ Move research/mockups/ HTMLs → Task 6
- ✅ Move UX analysis MDs → Task 7
- ✅ Move Resolume assets → Task 8
- ✅ Rename media folder → Task 9
- ✅ Fix CLAUDE.md stale refs → Task 10
- ✅ Fix ARCHITECTURE_V2.md counts → Task 11
- ✅ Fix research/INDEX.md → Task 12
- ✅ Rewrite README.md → Task 13
- ✅ Commit → Task 14

**No placeholders found.**
