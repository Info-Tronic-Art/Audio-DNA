# Audio-DNA v2: Task Plan

> Phased implementation from current M6-complete state to v2 architecture.
> M1-M6 remain complete. M7 (keyboard launcher) is replaced by this plan.

---

## Phase 1: BPM Stabilization (Can Ship Independently)

**Goal**: Rock-solid BPM lock and smooth beat phase. No UI changes needed.

- [ ] **P1.1** Implement BPM stabilization pipeline in `BPMTracker.cpp`
  - Range gate (60-200 BPM), confidence gate, octave error correction
  - Median filter (window of 48), hysteresis lock (2-second persistence)
  - All buffers pre-allocated in constructor. Zero steady-state allocation.
  - Files: `src/analysis/BPMTracker.h`, `src/analysis/BPMTracker.cpp`

- [ ] **P1.2** Implement free-running beat phase from locked BPM
  - Phase ramps linearly from locked BPM, wraps at 1.0
  - Hard reset on high-confidence beat detections from aubio
  - Files: `src/analysis/BPMTracker.h`, `src/analysis/BPMTracker.cpp`

- [ ] **P1.3** Add tracker state to FeatureSnapshot
  - New field: `trackerState` (0=searching, 1=locking, 2=locked)
  - Update BPM display in AudioReadoutPanel to show state (color-coded)
  - Files: `src/analysis/FeatureSnapshot.h`, `src/ui/AudioReadoutPanel.cpp`

- [ ] **P1.4** Unit tests for BPM stabilization
  - Test median filter with outlier rejection
  - Test octave error correction (60→120, 240→120)
  - Test hysteresis lock (stable hold, real tempo change detection)
  - Test beat phase smoothness
  - Files: `tests/test_bpm_stabilization.cpp`

---

## Phase 2: Downbeat Detection

**Goal**: Automatic beat-in-bar detection. No commercial VJ tool does this from live audio.

- [ ] **P2.1** Implement downbeat scoring in BPMTracker or new DownbeatDetector
  - Score each beat: `0.5 * bassEnergy + 0.3 * spectralFlux + 0.2 * harmonicChange`
  - Circular buffer of 16+ beat scores, 4-beat window analysis
  - Lock downbeat position after 8+ consistent beats
  - Files: `src/analysis/BPMTracker.h/cpp` or new `src/analysis/DownbeatDetector.h/cpp`

- [ ] **P2.2** Add metrical hierarchy fields to FeatureSnapshot
  - `beatInBar` (0-3), `barPhase` (0-1 over 4 beats), `downbeatDetected` (trigger)
  - Wire into AnalysisThread pipeline after BPM stage
  - Files: `src/analysis/FeatureSnapshot.h`, `src/analysis/AnalysisThread.cpp`

- [ ] **P2.3** Update audio readout panel for bar display
  - Show 4-beat bar indicator (highlight current beat)
  - Show bar phase sawtooth
  - Show downbeat flash
  - Files: `src/ui/AudioReadoutPanel.cpp`

- [ ] **P2.4** Unit tests for downbeat detection
  - Test with synthetic 4/4 kick patterns
  - Test beat-in-bar accuracy over 32-beat sequences
  - Files: `tests/test_downbeat_detector.cpp`

---

## Phase 3: Architecture Foundation

**Goal**: New data model classes, undo system, and routing engine. No UI yet.

- [ ] **P3.1** Implement Command pattern and UndoManager
  - `Command` base class with `execute()`, `undo()`, `description()`
  - `UndoManager` with history stack, ⌘Z/⇧⌘Z support
  - Files: `src/core/Command.h`, `src/core/UndoManager.h/cpp`

- [ ] **P3.2** Implement Composition data model
  - `Composition`, `Deck`, `Layer`, `Clip`, `Column` classes
  - Layer types enum: Opaque, Transparent, FXOnly, ThreeD, Mask
  - Clip: media reference + effect chain + transport state + autopilot config
  - JSON serialization for save/load
  - Files: `src/model/Composition.h/cpp`, `src/model/Deck.h/cpp`, `src/model/Layer.h/cpp`, `src/model/Clip.h/cpp`

- [ ] **P3.3** Implement Signal system
  - `Signal` base class with `getValue()`, `getName()`, `getType()`
  - `AudioSignal` — wraps a FeatureSnapshot field
  - `OscillatorSignal` — BPM-locked waveform (sine/saw/tri/square)
  - `EnvelopeSignal` — curve editor with control points
  - `SignalRegistry` — manages all active signals
  - Files: `src/signal/Signal.h`, `src/signal/AudioSignal.h/cpp`, `src/signal/OscillatorSignal.h/cpp`, `src/signal/EnvelopeSignal.h/cpp`, `src/signal/SignalRegistry.h/cpp`

- [ ] **P3.4** Implement Routing Engine (replaces MappingEngine)
  - `Route` — connects Signal/Macro to parameter with range/invert/dial-range
  - `RoutingEngine` — processes all routes each frame (called by render thread)
  - Per-route threshold, gain, falloff settings
  - Files: `src/routing/Route.h`, `src/routing/RoutingEngine.h/cpp`

- [ ] **P3.5** Implement Macro system
  - `MacroBank` — 6 knobs per scope (clip/layer/global)
  - Each macro: source signal, value, linked parameters with individual range/invert
  - Macros selectable as sources in route configuration
  - Files: `src/routing/MacroBank.h/cpp`

- [ ] **P3.6** Implement Binding system
  - `Binding` — maps key code or MIDI note/CC to an action
  - `BindingManager` — stores all bindings, processes input events
  - Actions: trigger clip, trigger column, toggle layer control, adjust macro, switch deck, tap tempo, resync
  - Files: `src/binding/Binding.h`, `src/binding/BindingManager.h/cpp`

- [ ] **P3.7** Implement multi-layer CompositorEngine (upgrade from v1)
  - Render N layers with per-layer blend mode, keying, opacity
  - Layer type handling: Opaque (replace), Transparent (blend), FX (apply to accumulator), Mask (alpha mask)
  - Per-clip effect chain + per-layer effect chain + global effect chain
  - Ping-pong FBO management for arbitrary layer count
  - Files: `src/render/CompositorEngine.h/cpp` (major rewrite)

- [ ] **P3.8** Implement Autopilot system
  - Per-clip and per-layer autopilot with action + duration
  - Bar-aware durations (using downbeat detector)
  - Integration with clip triggering
  - Files: `src/model/Autopilot.h/cpp`

- [ ] **P3.9** Integration tests for data model + routing + compositing
  - Test composition create/save/load roundtrip
  - Test route: signal → parameter value update
  - Test layer compositing: opaque + transparent blend
  - Test autopilot: clip auto-advance on beat
  - Files: `tests/test_composition.cpp`, `tests/test_routing_engine.cpp`, `tests/test_compositor.cpp`

---

## Phase 4: UI — Signal Bar & Top Bar

**Goal**: Signal Bar across the top, updated top bar with tempo/quantize/fade controls.

- [ ] **P4.1** Implement SignalStrip component
  - Vertical meter strip with three display sizes (minimized/normal/expanded)
  - Readout format based on signal type (numeric, note name, state text, etc.)
  - Click → selects signal for inspector
  - Color-coded by category
  - Files: `src/ui/SignalStrip.h/cpp`

- [ ] **P4.2** Implement SignalBar component
  - Horizontal container of SignalStrips, full window width
  - Collapsible signal groups with headers
  - [+] button to add signals from dropdown
  - Three size modes: minimized, normal, expanded (programming mode)
  - Horizontal scrolling if more strips than fit
  - Files: `src/ui/SignalBar.h/cpp`

- [ ] **P4.3** Update top bar
  - Add global transport (▶⏸■)
  - Add BPM display with tracker state (color-coded: searching/locking/locked)
  - Add Tap tempo button, Resync button
  - Add BPM multiplier buttons (÷4, ÷2, ×1, ×2, ×4)
  - Add Quantize dropdown (Off / Next Beat / Next Downbeat)
  - Add global Fade speed slider
  - Files: `src/MainComponent.cpp`, `src/ui/TopBar.h/cpp`

- [ ] **P4.4** Implement Programming Mode
  - Expanded signal bar fills most of screen
  - Each signal shows meter + connected parameters + controls
  - [+ Add Signal] button, route editing inline
  - Toggle via View menu
  - Files: `src/ui/ProgrammingMode.h/cpp`

---

## Phase 5: UI — Deck View

**Goal**: Resolume-style layer × column deck replaces keyboard grid.

- [ ] **P5.1** Implement ClipCell component
  - Thumbnail display (80×60px), name bar below
  - Click thumbnail = trigger, click name bar = select for inspection
  - Right-click name bar = context menu
  - Drag-and-drop support (receive from browser, drag to reorder)
  - Active clip highlight (cyan border)
  - Files: `src/ui/ClipCell.h/cpp`

- [ ] **P5.2** Implement LayerStrip component
  - Compact controls: X, B, S, M, A, V buttons, type dropdown, opacity knob, transport
  - ~45px tall per layer
  - Click = select for inspection
  - Dynamic controls based on layer type
  - Files: `src/ui/LayerStrip.h/cpp`

- [ ] **P5.3** Implement DeckView component
  - Grid of ClipCells organized by Layer × Column
  - Column trigger buttons at top
  - Deck tabs at bottom for switching
  - Scrolling (horizontal for columns, vertical for many layers)
  - Column trigger interaction (activate column, empty = clear)
  - Files: `src/ui/DeckView.h/cpp`

- [ ] **P5.4** Implement column triggering logic
  - Activate clips in column, clear empty layers
  - Quantize support (wait for beat/downbeat)
  - Beat snap for video clips (playhead alignment)
  - Active column highlight
  - Files: `src/model/Deck.cpp` (logic), `src/ui/DeckView.cpp` (visual)

- [ ] **P5.5** Implement clip triggering logic
  - Click = load and play. Click again = retrigger from start.
  - Click empty = stop/clear layer.
  - Transition crossfade between clips (per-layer speed)
  - Files: `src/model/Layer.cpp`, `src/render/CompositorEngine.cpp`

---

## Phase 6: UI — Inspector

**Goal**: 4-tab inspector panel for Clip/Layer/Composition/Signal properties.

- [ ] **P6.1** Implement UniversalParamControl component
  - Collapsed: label + value + -/+ + slider
  - Expanded: source picker (Manual/Signal/Macro), invert, range, dial range
  - Inline source visualization (mini meter showing how source drives value)
  - Envelope editor (curve with draggable points) when source = Envelope
  - Files: `src/ui/UniversalParamControl.h/cpp`

- [ ] **P6.2** Implement EffectStackView component
  - Vertical list of effects, each collapsible
  - Collapsed: [B] bypass + icon + name + main value (single line)
  - Expanded: all parameters as UniversalParamControl widgets
  - Drag to reorder, drag from browser to add
  - FX Presets nested under each effect
  - Files: `src/ui/EffectStackView.h/cpp`

- [ ] **P6.3** Implement MacroPanel component
  - 6 knobs with source picker buttons
  - Each knob renameable
  - Shows linked parameters list
  - Files: `src/ui/MacroPanel.h/cpp`

- [ ] **P6.4** Implement ClipInspector tab
  - Sections: Macros, Transport, Autopilot, Beat Snap, Effect Stack, Cuepoints
  - All controls use UniversalParamControl
  - Files: `src/ui/ClipInspector.h/cpp`

- [ ] **P6.5** Implement LayerInspector tab
  - Sections: Macros, Blend Mode, Keying, Layer Effects, Autopilot Defaults, Transition Speed
  - Dynamic sections based on layer type
  - Files: `src/ui/LayerInspector.h/cpp`

- [ ] **P6.6** Implement CompositionInspector tab
  - Sections: Global Macros, Global Effects, Master Opacity, Transition Speed, Output Settings
  - Files: `src/ui/CompositionInspector.h/cpp`

- [ ] **P6.7** Implement SignalInspector tab
  - Audio signal: threshold, gain, falloff per route
  - Oscillator: wave shape, beat duration, amplitude, phase
  - Envelope: curve editor, phase, octaves, curve type
  - Routes list: where this signal connects
  - Files: `src/ui/SignalInspector.h/cpp`

- [ ] **P6.8** Implement InspectorPanel container
  - 4 tabs: Clip, Layer, Composition, Signal
  - Auto-switches based on selection
  - Scrollable content
  - Files: `src/ui/InspectorPanel.h/cpp`

---

## Phase 7: UI — Browser

**Goal**: 5-tab content browser for Files, FX, Sources, Compositions/Decks, Record.

- [ ] **P7.1** Implement FilesBrowser tab
  - Folder navigation with path bar, up button
  - Search field
  - Favorites (heart icon)
  - Thumbnail / list view toggle
  - Supported formats: PNG, JPG, GIF, BMP, TIFF, WAV, AIFF, MP3, FLAC, OGG, AVI, MOV
  - Multi-select drag onto deck cells
  - Files: `src/ui/FilesBrowser.h/cpp`

- [ ] **P7.2** Implement FXBrowser tab
  - Effect icons organized by category (8 categories)
  - 76 effect icons (from AI generation)
  - Each effect expandable to show FX Presets
  - Drag effect onto clip cell or inspector effect stack
  - Save/load FX Presets with meaningful names
  - Files: `src/ui/FXBrowser.h/cpp`

- [ ] **P7.3** Implement SourcesBrowser tab
  - Procedural generator icons by category
  - 10 Tier 1 generators for launch
  - Auto-generated thumbnails
  - Drag onto clip cells
  - Files: `src/ui/SourcesBrowser.h/cpp`

- [ ] **P7.4** Implement CompDecksBrowser tab
  - Saved compositions list with preview
  - Saved decks list
  - Click to load
  - Save/rename/delete operations
  - Files: `src/ui/CompDecksBrowser.h/cpp`

- [ ] **P7.5** Implement RecordPanel tab
  - Start/stop settings recording
  - Playback controls for recorded sessions
  - Output directory selector
  - Format settings
  - Files: `src/ui/RecordPanel.h/cpp`

- [ ] **P7.6** Implement BrowserPanel container
  - 5 tabs: Files, FX, Sources, Comp/Decks, Record
  - Files: `src/ui/BrowserPanel.h/cpp`

---

## Phase 8: UI — Preview, Output, Layout Integration

**Goal**: Preview panel, fullscreen output, menu bar, complete layout.

- [ ] **P8.1** Update PreviewPanel for context-sensitive display
  - [Preview] [Output] tabs
  - Preview: shows selected clip/layer solo or full composition
  - Output: shows external display output
  - Files: `src/ui/PreviewPanel.h/cpp` (rewrite)

- [ ] **P8.2** Update OutputWindow for v2 rendering pipeline
  - Multi-layer composited output
  - Display selector
  - Files: `src/ui/OutputWindow.h/cpp` (update)

- [ ] **P8.3** Implement menu bar
  - 9 menus: Audio-DNA, Composition, Deck, Layer, Column, Clip, Output, Shortcuts, View
  - All menu items wired to actions
  - Keyboard shortcuts
  - Files: `src/ui/MenuBar.h/cpp`, `src/MainComponent.cpp`

- [ ] **P8.4** Implement Preferences dialog
  - 8 tabs: General, Audio, Video, MIDI, Recording, Defaults, Feedback, About
  - JSON-backed persistent settings
  - Files: `src/ui/PreferencesDialog.h/cpp`

- [ ] **P8.5** Integrate complete layout in MainComponent
  - Top bar → Signal bar → Deck → [Preview | Inspector | Browser]
  - Panel resizing with draggable dividers
  - View menu toggles for each panel
  - Files: `src/MainComponent.h/cpp` (major rewrite)

---

## Phase 9: Binding System & MIDI

**Goal**: Keyboard binding mode and MIDI learn mode.

- [ ] **P9.1** Implement keyboard binding mode UI
  - Enter/exit binding mode (⇧⌘K)
  - Highlight bindable elements
  - Click element → "waiting for key" state → press key → binding created
  - Visual indicator of bound key on/near element
  - Files: `src/ui/BindingOverlay.h/cpp`

- [ ] **P9.2** Implement MIDI learn mode UI
  - Enter/exit MIDI learn mode (⇧⌘M)
  - Same flow as keyboard binding but listens for MIDI input
  - Notes → triggers, CCs → continuous controls
  - Files: `src/ui/MidiLearnOverlay.h/cpp`

- [ ] **P9.3** MIDI input handling
  - JUCE MidiInput integration
  - MIDI device selection in preferences
  - Process MIDI messages → route to bound actions
  - Files: `src/midi/MidiHandler.h/cpp`

---

## Phase 10: Procedural Sources

**Goal**: Built-in procedural content generators.

- [ ] **P10.1** Implement Source base class and Source→clip integration
  - Source generates texture each frame via GLSL shader
  - Integrates with EffectChain as first element
  - Files: `src/sources/Source.h/cpp`

- [ ] **P10.2** Implement Tier 1 sources (10)
  - Perlin Noise, Plasma, Voronoi, Kaleidoscopic Fractal, Mandelbrot/Julia
  - Geometric Tunnel, Color Gradient, Audio Waveform visualization
  - Each as GLSL fragment shader + parameter registration
  - Files: `src/sources/NoiseSource.h/cpp`, `src/sources/FractalSource.h/cpp`, etc.
  - Shaders: `shaders/source_*.frag`

- [ ] **P10.3** Implement stateful sources (Reaction-Diffusion, Cellular Automata)
  - Ping-pong FBO pairs for state persistence
  - Files: `src/sources/ReactionDiffusionSource.h/cpp`

- [ ] **P10.4** Implement particle system source
  - CPU-side physics, texture upload for rendering
  - Files: `src/sources/ParticleSource.h/cpp`

---

## Phase 11: Video Playback

**Goal**: HAP Alpha video clip support.

- [x] **P11.1** Video decoder via FFmpeg
  - FFmpeg integration (libavformat/libavcodec/libavutil/libswscale)
  - Supports MP4, MOV, QuickTime, AVI, MKV, WebM, M4V, HAP Alpha
  - RGBA conversion + GL texture upload per frame
  - Alpha channel detection (HAP Alpha, YUVA, RGBA pixel formats)
  - Files: `src/media/VideoPlayer.h/cpp`, `cmake/FindFFmpeg.cmake`

- [x] **P11.2** Video transport controls + Image Sequence playback
  - Play/pause/reverse, speed control, loop/ping-pong/one-shot
  - Image sequences: drag multiple PNGs/JPEGs → treated as video clip
  - Configurable Images/Sec (0-6 fps) for image sequences
  - Thumbnails displayed in deck cells and layer strips
  - Files: `src/media/VideoPlayer.h/cpp`, `src/media/ImageSequence.h/cpp`

- [x] **P11.3** BPM Sync transport mode for video + image sequences
  - Beat division dropdown: 1/4 beat through 16 beats (4 bars)
  - Content Beats: how many beats the source contains (snaps to 1/2/4/8/16/32/64)
  - Video speed auto-calculated: `videoBeats / beatDivision`
  - Image sequence FPS auto-calculated from BPM + beat division
  - Signal connect triangle (cyan when BPM Sync active)
  - Files: `src/render/Renderer.cpp`, `src/ui/ClipInspector.h/cpp`

- [x] **P11.4** Beat snap for clip triggering
  - On trigger with Beat Snap enabled: seek to current beat phase position
  - Works for both video clips and image sequences
  - Files: `src/MainComponent.cpp`

- [x] **P11.5** Compositor integration
  - CompositorEngine now renders Video and ImageSequence media types
  - Renderer::renderOpenGL() calls compositeDeck() when active deck has content
  - Video frame callback: advance + upload per render frame
  - Files: `src/render/CompositorEngine.h/cpp`, `src/render/Renderer.h/cpp`

- [x] **P11.6** Cuepoints — deferred from P11, implemented in P12
  - 8 cuepoint save slots per clip with separate Set + Trigger button rows
  - Jump to cuepoint via trigger buttons, Ctrl+click to clear
  - Files: `src/model/Clip.h/cpp`, `src/ui/ClipInspector.cpp`

---

## Phase 12: Phrase Tracking & Polish

**Goal**: Phrase-level metrical tracking, settings recording, final polish.

- [x] **P12.1** Implement phrase tracking
  - Bar count from downbeat, phrasePhase [0,1) over configurable N bars (default 8)
  - Reset on structural transitions (drop enter, breakdown leave) + Resync button
  - Beat wheel indicator in TopBar (4-segment circle), bar/phrase readout
  - BarPhase, PhrasePhase, BarCount added to MappingSource + SignalRegistry
  - Files: `src/analysis/BPMTracker.h/cpp`, `src/analysis/FeatureSnapshot.h`, `src/ui/TopBar.h/cpp`

- [x] **P12.2** Implement settings recording
  - SessionRecorder: timestamped event logging (params, clips, columns, macros, transport, effects, cuepoints)
  - JSON save/load, playback with advancePlayback(dt)
  - RecordPanel wired with Record/Stop/Play/Save/Load buttons
  - Files: `src/recording/SessionRecorder.h/cpp`, `src/ui/RecordPanel.h/cpp`

- [ ] **P12.3** Implement Syphon/NDI texture sharing (Output menu) — DEFERRED
  - Syphon for macOS, NDI for cross-platform
  - Send composition output as shared texture
  - Files: `src/output/SyphonOutput.h/cpp`, `src/output/NDIOutput.h/cpp`

- [x] **P12.4** Performance optimization + UI polish
  - Early exit in MappingEngine when no mappings
  - ClipInspector redesign: timeline with in/out handles, playhead triangle, beat markers
  - Transport cleanup: mode selector in header, Speed/Duration labels, playhead sync
  - Cuepoint split buttons (Trigger + Set), draggable in/out points
  - Layer selection outline on name box
  - Removed CrossFader from CompositionInspector (unused)
  - Section headers with downward triangles, better spacing (8px gaps, 24px rows)

- [ ] **P12.5** Cross-platform testing — DEFERRED
  - macOS (ARM64 + x86_64)
  - Windows (MSVC)
  - Linux (GCC)
  - CI updates for new source files
  - Files: `.github/workflows/`

---

## Phase Summary

| Phase | Focus | Depends On | UI Visible |
|-------|-------|-----------|-----------|
| **P1** | BPM stabilization | Nothing | Improved BPM display only |
| **P2** | Downbeat detection | P1 | Beat bar indicator |
| **P3** | Architecture foundation (data model, routing, compositing) | P1, P2 | Nothing (backend) |
| **P4** | Signal Bar + Top Bar | P3 | New top section |
| **P5** | Deck View | P3 | New center section |
| **P6** | Inspector | P3, P4, P5 | New right-center section |
| **P7** | Browser | P3, P6 | New right section |
| **P8** | Preview, Output, Layout, Menus, Prefs | P4-P7 | Complete layout |
| **P9** | Binding system (keyboard + MIDI) | P8 | Bind mode overlay |
| **P10** | Procedural sources | P5, P6 | Sources in browser |
| **P11** | Video playback | P5, P6 | Video clips in deck |
| **P12** | Phrase tracking, recording, polish | All above | Final features |
