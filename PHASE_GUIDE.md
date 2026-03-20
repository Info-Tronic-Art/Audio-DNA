# Audio-DNA v2: Phase Kickoff Guide

> This file tells Claude how to start any phase in a fresh session.
> User says: "kick off phase N" → Claude reads this file, finds Phase N, and executes.

---

## How To Use

1. Open a new Claude Code session
2. Say: **"kick off phase 1"** (or whatever phase number)
3. Claude reads this file, CLAUDE.md, ARCHITECTURE_V2.md, and TASKPLAN_V2.md
4. Claude executes ALL tasks in that phase without stopping for validation
5. Claude self-validates: builds, greps for rule violations, runs tests
6. Claude only stops when there is a **UI-visible change the user can verify by launching the app**
7. If the phase has NO UI changes, Claude completes it fully, commits, and reports done

---

## Phase Status Tracker

| Phase | Description | Status | UI Validation Needed? |
|-------|-------------|--------|----------------------|
| **P1** | BPM Stabilization | **COMPLETE** | NO — backend only. Self-validate: build + tests pass. |
| **P2** | Downbeat Detection | **COMPLETE** | YES — beat bar indicator in audio readout panel |
| **P3** | Architecture Foundation | **COMPLETE** | NO — backend only. Self-validate: build + tests pass. |
| **P4** | Signal Bar + Top Bar | **COMPLETE** | YES — new top section visible in app |
| **P5** | Deck View | **COMPLETE** | YES — new center section visible |
| **P6** | Inspector | **COMPLETE** | YES — new right-center section visible |
| **P7** | Browser | **COMPLETE** | YES — new right section visible |
| **P8** | Preview, Output, Layout, Menus, Prefs | **COMPLETE** | YES — complete layout visible |
| **P9** | Binding System & MIDI | **COMPLETE** | YES — bind mode overlay visible |
| **P10** | Procedural Sources | **COMPLETE** | YES — sources in browser, renderable |
| **P10.5** | Inspector Overhaul (Resolume-style) | **COMPLETE** | YES — full inspector with signal triangles, transform, video, 8-link dashboard |
| **P11** | Video Playback | **COMPLETE** | YES — video clips play in deck |
| **P12** | Phrase Tracking & Polish | **COMPLETE** | YES — phrase signals, cuepoints, recording |

---

## Per-Phase Instructions

### Phase 1: BPM Stabilization

**Read first**: `CLAUDE.md`, `ARCHITECTURE_V2.md` (Section 8), `research/BPM_STABILITY_RESEARCH.md`
**Read before editing**: `src/analysis/BPMTracker.h`, `src/analysis/BPMTracker.cpp`, `src/analysis/FeatureSnapshot.h`

**Tasks**: P1.1 through P1.4 in TASKPLAN_V2.md

**Self-validation**:
- Build: `cmake --build build --config Release -j$(sysctl -n hw.ncpu)` exits 0
- Tests: all existing tests + new `test_bpm_stabilization.cpp` pass
- Grep: no `new`/`malloc` in BPMTracker steady-state path, all buffers pre-allocated in constructor
- No UI validation needed — this is pure backend

**When done**: Commit, update this file (mark P1 COMPLETE), report to user.

---

### Phase 2: Downbeat Detection

**Read first**: `CLAUDE.md`, `ARCHITECTURE_V2.md` (Section 8), `research/BPM_STABILITY_RESEARCH.md` (Part 5)
**Read before editing**: `src/analysis/BPMTracker.h/cpp`, `src/analysis/AnalysisThread.h/cpp`, `src/analysis/FeatureSnapshot.h`, `src/ui/AudioReadoutPanel.h/cpp`

**Tasks**: P2.1 through P2.4 in TASKPLAN_V2.md

**Self-validation**:
- Build passes
- Tests pass (including new downbeat tests)
- Grep: no RT violations in analysis path

**UI validation**: User should see a 4-beat bar indicator in the audio readout panel when music plays. Downbeat (beat 1) should flash or highlight differently.

**When done**: Commit, update this file, report to user with what to look for in the UI.

---

### Phase 3: Architecture Foundation

**Read first**: `CLAUDE.md`, `ARCHITECTURE_V2.md` (Sections 2-7, 10-14, 17, 25-26)
**Read before editing**: All existing `src/` files relevant to each class being created

**Tasks**: P3.1 through P3.9 in TASKPLAN_V2.md

**This is the largest phase.** It creates the entire new data model, routing engine, compositor, undo system, and binding system. NO UI changes — all backend.

**Self-validation**:
- Build passes
- All existing tests still pass
- All new integration tests pass (composition roundtrip, routing, compositing)
- Grep: no RT violations in render/analysis paths
- JSON serialization roundtrip works for Composition save/load

**When done**: Commit, update this file, report to user. No UI to validate.

---

### Phase 4: Signal Bar + Top Bar

**Read first**: `CLAUDE.md`, `ARCHITECTURE_V2.md` (Sections 3, 4.1, 9)
**Read before editing**: `src/MainComponent.h/cpp`, `src/ui/AudioReadoutPanel.h/cpp`

**Tasks**: P4.1 through P4.4 in TASKPLAN_V2.md

**UI validation**: User should see:
- Signal Bar across the top with vertical meter strips for audio features
- Updated top bar with tempo display (showing tracker state), tap/resync buttons, BPM multiplier, quantize, fade
- Signal strips animate in real-time when audio plays
- Click a signal strip → something highlights (inspector not built yet, but interaction should work)

---

### Phase 5: Deck View

**Read first**: `CLAUDE.md`, `ARCHITECTURE_V2.md` (Sections 3, 4.2, 14, 15, 16)
**Read before editing**: `src/MainComponent.h/cpp`, existing keyboard/compositor code

**Tasks**: P5.1 through P5.5 in TASKPLAN_V2.md

**UI validation**: User should see:
- Resolume-style layer × column grid below the signal bar
- Layer strips on the left with X/B/S/M/A/V buttons and type dropdown
- Column trigger buttons at top of each column
- Deck tabs at bottom
- Can drag an image file from Finder onto a clip cell
- Can click a clip to trigger it (displays in preview)
- Can click an empty cell to clear the layer

---

### Phase 6: Inspector

**Read first**: `CLAUDE.md`, `ARCHITECTURE_V2.md` (Sections 4.3, 5, 6)

**Tasks**: P6.1 through P6.8 in TASKPLAN_V2.md

**UI validation**: User should see:
- Inspector panel with 4 tabs (Clip/Layer/Composition/Signal)
- Click a clip → Clip inspector shows macros, transport, effects stack
- Click a layer → Layer inspector shows blend/keying/layer effects
- Effects collapse to single line, expand to show parameters with sliders
- Each parameter has a source picker button

---

### Phase 7: Browser

**Read first**: `CLAUDE.md`, `ARCHITECTURE_V2.md` (Section 4.4)

**Tasks**: P7.1 through P7.6 in TASKPLAN_V2.md

**UI validation**: User should see:
- Browser panel with tabs (Files/FX/Sources/Comp-Decks/Record)
- Files tab: folder navigation, thumbnails
- FX tab: effect icons by category, can drag onto clip cells
- Can drag media from Files onto deck cells

---

### Phase 8: Preview, Output, Layout, Menus, Preferences

**Read first**: `CLAUDE.md`, `ARCHITECTURE_V2.md` (Sections 3, 4.5, 20, 21)

**Tasks**: P8.1 through P8.5 in TASKPLAN_V2.md

**UI validation**: User should see:
- Complete integrated layout: top bar → signal bar → deck → [preview | inspector | browser]
- Menu bar with all 9 menus functional
- Preferences dialog opens from Audio-DNA menu
- Preview shows composition output
- Output tab shows external display selector
- Panels resize with draggable dividers

---

### Phases 9-12: See TASKPLAN_V2.md for details

Each has specific UI validation requirements listed in the phase description.

---

## General Rules for All Phases

1. **Read CLAUDE.md first** — it has the sacred rules (no allocation in audio callback, etc.)
2. **Read ARCHITECTURE_V2.md** for the design spec
3. **Read existing source files** before modifying them
4. **Self-validate with two methods** before asking the user (see CLAUDE.md "kick off" protocol)
5. **Batch tasks** — don't stop between tasks within a phase unless blocked
6. **Only stop for the user** when there's a UI-visible change to verify
7. **Commit when phase is complete** (not after each task)
8. **Update this file** to mark the phase status
9. **Follow naming conventions** from ARCHITECTURE_V2.md Section 2
10. **UI text rule**: Always display whole words, never abbreviations
