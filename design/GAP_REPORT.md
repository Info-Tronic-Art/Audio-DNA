# Audio-DNA Feature Audit — Gap Report

**Date:** 2026-05-19 | **Audited by:** Harmony (3 parallel agents + doc analysis)
**Sources:** OurAppFeatures_1.md (Boris, 728 lines), FEATURES.md (engineering, 901 lines), source code (58K LOC), graphify (4231 nodes)

---

## 1. Count Discrepancies

| Item | Boris's Doc | FEATURES.md | Actual Code | Delta |
|------|-------------|-------------|-------------|-------|
| Procedural sources | 81 | 108 | 101 | +20 undocumented in Boris's doc |
| Effects | 135 | 135 | 135 | Match |
| Effect parameters | — | — | 333 | Never inventoried |
| Source parameters | — | — | 754 | Never inventoried (628 via grep; 754 actual with runtime helper expansion) |
| Total adjustable params | — | — | 1,379+ | ~1,176 without dedicated UI (corrected: 333 effect + 754 source + ~292 inspector) |
| Blend modes | 48 | — | 25 | Boris overcounts (includes transitions) |
| Transition modes | 45+ | — | 30 | Boris overcounts |
| Total MixMode enum | — | — | 55 | 25 blend + 30 transition |
| Keying modes | 13 | — | 13 | Match |
| Mapping sources | — | 40+ | 58 | 18 undocumented (57 was grep count; 58 actual enum excluding Count sentinel) |
| Curve types | 24 | 5 | 24 | FEATURES.md undercounts (only lists basic 5) |
| Default signals | 12 | — | 32 | 8 visible + 21 hidden audio + 2 modulation + 1 clip position |
| UI components | — | — | 37 | (34 active, 3 hidden v1) |
| HTML mockups | 31 (handoff) | — | 392 | Handoff vastly undercounts (v1-v9 iterations) |

---

## 2. Ghost Features (code exists, NO UI)

| Feature | Location | What it does | LOC |
|---------|----------|-------------|-----|
| MappingSuggester | src/mapping/MappingSuggester.h | Genre-aware AI mapping suggestions | ~200 |
| Clip/Layer MacroBanks | src/signal/ + src/model/ | Per-clip and per-layer macro scopes (only Global instantiated) | — |
| Crossfader | Composition model | Blend mode, curve, behaviour for crossfading between decks | — |
| LUT Loader | src/render/LUTLoader.h/cpp | Color grading via Look-Up Tables | 111 |
| Camera input | Clip::MediaType::Camera | Live camera as video source | — |
| LinkSync::requestBeatAtTime() | src/sync/LinkSync.h | Sync to specific beat position | — |
| Route TargetScope::Clip/Layer | src/routing/ | Per-clip and per-layer signal routing targets | — |

---

## 3. Undocumented in BOTH Existing Docs

| Feature | Status | Notes |
|---------|--------|-------|
| MilkDrop/projectM system | **MISSING from FEATURES.md** | 2K LOC, 9800 presets, 3 play modes. Boris's doc covers it; FEATURES.md doesn't mention it at all |
| Render pipeline effects | Partial | Screen Split, Freeze in code. Frame Stutter, Echo in screenshots. Not systematically documented |
| 21 hidden audio signals | Working | Registered but not visible in default signal bar (P25 advanced features etc.). Originally counted as 25; verified 21 by enum audit. |
| LUT Loader | Working | 111 LOC in render pipeline. Neither doc mentions it |
| Camera input | Defined | MediaType::Camera in Clip enum, cameraDeviceIndex field. Not wired to UI |
| v1/v2 layout split | Working | 3 hidden v1 components (AudioReadoutPanel, SpectrumDisplay, EffectsRackPanel) + many hidden v1 controls in MainComponent |
| TimingWindow content | Placeholder | 3 tabs (BPM/Routing/Oscillators) but content is placeholder — confirmed by code audit |

---

## 4. Critical Design Finding: Dual Mode System Does NOT Exist

**Boris's core design tension:** Sub-features need FULL access during programming but must be HIDDEN WELL in presentation mode.

**Current reality:** ProgrammingMode.cpp is 64 lines. It toggles the SignalBar between Normal (84px) and Expanded (fills everything). When expanded, ALL other panels (deck, inspector, browser, preview, timing) are `setVisible(false)`.

**What this means:**
- Current system is BINARY: see everything OR see only signal bar
- There is NO graduated mode where some controls are visible and others hidden
- There is NO per-component mode behavior (only 2 components respond to mode: SignalBar and ProgrammingMode overlay)
- The ProgrammingMode component itself is actually VESTIGIAL — always set to `setVisible(false)` in MainComponent::resized()

**Implication for design overhaul:** The dual-mode system that Boris considers "THE core design tension" needs to be DESIGNED and BUILT. It doesn't exist yet. The mockups should target this — not just rearrange existing panels.

**What DOES work in any mode:**
- Keyboard bindings (through BindingManager)
- MIDI input (through MidiHandler → BindingManager)
- OSC input (through OscHandler)
- REST API (always running on port 7070)
- Audio engine, analysis, render pipeline (background threads)
- Output window, Syphon, recording (independent windows/threads)
- Autopilot (runs on render thread)

---

## 5. Sub-Feature Depth

### Parameters per scope
- Clip Inspector: ~25 parameters
- Layer Inspector: ~35 parameters (+ conditional: 3D controls, keying, feedback)
- Composition Inspector: ~20 parameters
- Top Bar: ~12 parameters
- Signal Inspector: ~12 parameters (3 signal types)
- MilkDrop Browser: ~8 parameters

### Effect and source parameters (CONFIRMED by code audit)
- 135 effects × 1-8 params = **333 effect parameters**
- 101 sources × 1-16 params = **754 source parameters** (628 via grep; 754 actual with runtime helper expansion)
- **Total confirmed adjustable parameters: 1,379+** (333 effect + 754 source + ~292 inspector)
- **Parameters WITHOUT dedicated UI: ~1,176+**
- **Parameters that are automatable: ~1,226+**

### Every parameter that is a UniversalParamControl has 4 sub-features:
- Signal source selection (8 modes: Manual/Signal/BPM Sync/Oscillator/Envelope/Clip Position/Timeline/Macro)
- Invert toggle
- Range min/max
This multiplies the effective complexity significantly.

---

## 6. Taxonomy Bridge

| OurAppFeatures_1.md (UI-spatial) | FEATURES.md (Technical) | Coverage |
|----------------------------------|------------------------|----------|
| Menu Bar (9 menus) | scattered | Good in Boris's doc |
| Top Bar | Feature 1 (Audio I/O) + Feature 2 (Analysis) | Good in both |
| Signal Bar | Feature 3 (Transport) + Feature 7 (Routing) | Good in both |
| Deck Grid + Layer Strip + Clip Cell | Feature 8 (Composition) | Good in both |
| Clip Inspector (9 sections) | Features 4, 6, 8, 9 | Good in Boris's doc |
| Layer Inspector (12 sections) | Features 4, 5, 8 | Good in Boris's doc |
| Composition Inspector (9 sections) | Feature 8, 16 | Good in Boris's doc |
| Browser Panel (6 tabs) | Features 4, 9, 12, **MilkDrop MISSING** | Gap in FEATURES.md |
| Binding System | Feature 10 | Good in both |
| Signal & Routing System | Features 6, 7 | Good in both |
| Output & Recording | Features 12, 13 | Good in both |
| Preferences | scattered | Good in Boris's doc |
| — | Feature 14 (External Control/API) | Missing from Boris's doc |
| — | Feature 15 (Ableton Link) | Missing from Boris's doc |
| — | Feature 16 (Genre Detection) | Missing from Boris's doc |

---

## 7. Orphaned/Hidden Components

### v1 components (hidden in v2 layout)
1. **AudioReadoutPanel** — full audio feature readout panel, hidden
2. **SpectrumDisplay** — 7-band energy visualization, hidden
3. **EffectsRackPanel** — v1 effects panel with knobs and mapping, hidden (superseded by EffectStackView)

### v1 controls in MainComponent (hidden)
- audioSourceSelector_, inputGainSlider_, masterLevelSlider_, displaySelector_
- resolutionSelector_, randomLabel_, beatRandomToggle_, beatCountSelector_
- syncButton_, fpsLabel_, cpuLabel_
- openImageButton_, fileLabel_, imageSequenceButton_
- v1 preset slots (10 buttons + dropdowns)
- v1 file-loading controls

All of these are `setVisible(false)` in the v2 layout but the code remains.

---

## 8. Session 2 Priorities

1. **Merge audit data into FEATURE_INVENTORY.md** — Use functional domain taxonomy, not UI location
2. **Tag every sub-feature** with PROGRAMMING/PRESENTATION visibility classification
3. **Resolve ghost features** — Boris decides: include in design or defer
4. **Verify effect/source parameter counts** — Engine audit pending
5. **Draft presentation mode requirements** — Based on what MUST stay accessible vs what CAN be hidden
