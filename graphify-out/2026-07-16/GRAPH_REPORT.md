# Graph Report - RealTimeAudio  (2026-07-16)

## Corpus Check
- 273 files · ~382,658 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 1609 nodes · 1485 edges · 230 communities (133 shown, 97 thin omitted)
- Extraction: 100% EXTRACTED · 0% INFERRED · 0% AMBIGUOUS · INFERRED: 1 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `1eff4f73`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Main Component Controller
- Compositor Engine
- API Server
- MilkDrop Preset Browser
- Curve Transforms
- Universal Param Control
- Layer Inspector
- Inspector Panel
- Session Recorder
- Image Sequence Player
- Effect Stack View
- Deck View
- Renderer Accessors
- Files Browser
- Output Window
- Binding Manager
- Preferences Dialog
- Signal Inspector
- Mapping Editor
- Look and Feel
- Effects Rack Panel
- Procedural Source
- Binding Overlay
- Composition Inspector
- Preset Manager
- Signal Bar
- MIDI Output Handler
- Video Recorder
- Browser Panel
- Comp Decks Browser
- FX Browser
- Genre Detector
- Audio Engine
- Routing Engine
- Sources Browser
- Preview Panel
- MIDI Input Handler
- Texture Manager
- Undo Manager
- Effect Library
- Feedback Processor
- Analysis Thread
- OSC Handler
- Record Panel
- Knob Widget
- Ring Buffer
- Deck Model
- Spectral Features
- Structural Detector
- Loudness Analyzer
- FFT Processor
- Audio Engine IO
- Spectrum Display
- Waveform Display
- Advanced Audio Analyzer
- Composition Model
- LUT Loader
- Layer Serialization
- Top Bar Header
- Preset Manager Header
- Audio Readout Header
- Undo Manager Header
- Feature Snapshot
- Syphon Input
- NDI Output
- NDI Input
- Compositor Header
- Binding Target Mode
- Audio Callback Header
- Layer Mix Mode
- Preset Selector Logic
- Preset Selector Header
- Route Target Scope
- Uniform Bridge Header
- Uniform Bridge Logic
- ISF Loader Header
- Clip Position Signal
- Envelope Signal
- Chained Signal Logic
- Main Component Header
- Mapping Types
- Spectrum Display Header
- Effects Rack Header
- Preferences Header
- Waveform Display Header
- Syphon Input Impl
- ChromaExtractor.cpp
- KeyDetector.cpp
- LoudnessAnalyzer.cpp
- SpectralFeatures.cpp
- StructuralDetector.cpp
- SyphonOutput.mm
- FFTProcessor.cpp
- AdvancedAudioAnalyzer
- PitchTracker.cpp
- test_effect
- Main.cpp
- Different feature values should produce different visual output on sources.
- Walk a leaf element with no children.
- Walk a tree with nested children.
- Walk an element with no attributes set.
- Depth 0 returns the root element without walking children.
- Depth 1 walks immediate children but not grandchildren.
- Depth -1 walks the full tree.
- The walk result serializes to valid JSON.
- A nested tree round-trips through JSON correctly.
- Raises AccessibilityPermissionError when AX permissions are denied.
- Raises AppNotFoundError when the app is not running.
- inspect_app with explicit PID skips name lookup.
- inspect_app finds PID by app name when pid is not provided.
- --help prints usage and exits without calling inspect_app.
- Exit code 2 when app is not found.
- Exit code 1 when permissions denied.
- Default output goes to stdout as valid JSON.
- --output writes JSON to a file.
- --depth flag is forwarded to inspect_app.
- --pid flag is forwarded to inspect_app.
- Each parameter must have >70% useful range, no discontinuities, no dead zones.
- RMS, bass, and beat phase should affect most sources that use u_rms/u_beatPhase.

## God Nodes (most connected - your core abstractions)
1. `VJAppController` - 23 edges
2. `_make_mock_element()` - 10 edges
3. `TestSignalRouteEndToEnd` - 9 edges
4. `inspect_app()` - 8 edges
5. `frame_is_not_black()` - 7 edges
6. `walk_element()` - 6 edges
7. `TestFrameCapture` - 5 edges
8. `TestIntegration` - 5 edges
9. `analyze_sweep()` - 5 edges
10. `main()` - 5 edges

## Surprising Connections (you probably didn't know these)
- `app()` --calls--> `VJAppController`  [INFERRED]
  tests/visual/conftest.py → tests/visual/vj_controller.py

## Import Cycles
- None detected.

## Communities (230 total, 97 thin omitted)

### Community 0 - "Main Component Controller"
Cohesion: 0.13
Nodes (8): VJ App Controller — Python client for the Eyes test harness HTTP API.  Wraps the, Get the current engine state (effects, FPS, etc.)., Controls Audio-DNA via the Eyes HTTP API., Get all registered procedural sources with their parameters., Get all signals with cached values., Spawn the app in test mode and wait for it to become ready.          Args:, Check if the app is running and ready., VJAppController

### Community 2 - "Compositor Engine"
Cohesion: 0.20
Nodes (10): _make_mock_element(), Build a mock AXUIElement with attribute lookup support.      Args:         role:, test_depth_one_walks_immediate_children(), test_depth_zero_skips_children(), test_empty_tree(), test_nested_children(), test_nested_tree_serializes(), test_output_is_valid_json() (+2 more)

### Community 3 - "API Server"
Cohesion: 0.29
Nodes (7): app(), _default_executable(), Pytest configuration for Eyes visual tests.  Provides fixtures that spawn the Au, Find the built executable., Spawn Audio-DNA in test mode for the entire test session.      The app starts on, Reset app state before each test for isolation., reset_between_tests()

### Community 4 - "MilkDrop Preset Browser"
Cohesion: 0.25
Nodes (5): Integration tests that connect to a live Audio-DNA instance., Read the real accessibility tree and verify basic structure., A running JUCE app should have at least one window child., Live tree serializes to JSON and parses back correctly., TestIntegration

### Community 16 - "Inspector Panel"
Cohesion: 0.12
Nodes (8): Configure the entire effect chain.          Disables all existing effects, then, Render a single frame and save it to disk.          Args:             output_pat, Load a procedural source into the active clip.          Args:             source, Update parameters on the currently active source.          Args:             par, Create a signal→parameter route.          Args:             route: Dict with key, Remove a signal route by ID., Send a POST request with JSON body., Enable/disable an effect and optionally set parameters.          Args:

### Community 20 - "Effect Stack View"
Cohesion: 0.06
Nodes (22): Eyes Visual Tests — Render Pipeline  Tests the core rendering pipeline: image lo, Two effects chained should both apply., Verify audio feature injection works., Injecting features should succeed., Verify state endpoint works., State endpoint should list all effects., Verify reset clears state properly., After reset, no effects should be enabled. (+14 more)

### Community 28 - "Signal Inspector"
Cohesion: 0.08
Nodes (6): ApiServer(), handleSetEffectChain(), handleState(), jsonError(), jsonOk(), setupRoutes()

### Community 30 - "Look and Feel"
Cohesion: 0.08
Nodes (19): psnr_between(), Signal Routing Verification — Tests the complete signal→route→parameter→shader p, RMS should change effect output via u_rms uniform., End-to-end: create route from audio signal to effect parameter,     inject featu, Check if signal API endpoints are available., Route Volume signal → Ripple intensity → verify RMS changes ripple., Route with threshold=0.5 should only activate above 0.5 RMS., Inverted route: high RMS should DECREASE the parameter. (+11 more)

### Community 32 - "Procedural Source"
Cohesion: 0.09
Nodes (26): frame_is_not_black(), frames_are_different(), Eyes Visual Tests — Comprehensive Fractal Source Validation  Tests EVERY control, Every fractal must render a visible frame at defaults., Every parameter must produce a visible change when modified., Zoom at ALL positions (0, 0.25, 0.5, 0.75, 1.0) must be non-black., Dive speed at various levels must not go black., Power at all positions must not go black. (+18 more)

### Community 35 - "Preset Manager"
Cohesion: 0.07
Nodes (9): CaretOnlyComboBoxLookAndFeel, FadeSpeedSliderLookAndFeel, FlatButtonLookAndFeel, FlatComboBoxLookAndFeel, FullBoundsSliderLAF, KeyingSliderLookAndFeel, OpacitySliderLookAndFeel, resized() (+1 more)

### Community 50 - "Texture Manager"
Cohesion: 0.12
Nodes (24): Exception, AccessibilityPermissionError, AppNotFoundError, AXInspectorError, check_accessibility_permissions(), find_pid_by_name(), _get_ax_attribute(), _get_position() (+16 more)

### Community 74 - "Ring Buffer"
Cohesion: 0.14
Nodes (13): all_effects(), brightness(), image_loaded(), psnr_between(), Auto-Discovering Effect Verification — Tests ALL registered effects.  Queries /a, No effect should turn the image completely black or white., Discover all effects from the running app., Ensure test image is loaded. (+5 more)

### Community 79 - "Spectral Features"
Cohesion: 0.18
Nodes (14): analyze_sweep(), brightness(), Tier 2: Range Quality Analysis  For each source parameter, renders at 11 positio, Sweep every parameter and verify quality metrics., Auto-discover all sources and sweep all their params.      This class discovers, Sweep every param on every source. Generates CSV reports., Render a source at multiple parameter positions, return list of (value, path) tu, Analyze a parameter sweep for quality metrics. (+6 more)

### Community 87 - "Spectrum Display"
Cohesion: 0.18
Nodes (11): all_sources(), brightness(), psnr_between(), Auto-Discovering Source Verification — Tests ALL registered procedural sources., Sweep critical params across 5 positions to check for discontinuities., Discover all sources from the running app., Every registered source must render a non-black frame at defaults., Every param on every source must have a visible effect. (+3 more)

### Community 115 - "Binding Target Mode"
Cohesion: 0.20
Nodes (5): Performance Verification — Tests that sources and effects render within budget., Every source must render within budget., Effects should not significantly slow down rendering., TestEffectPerformance, TestSourcePerformance

### Community 116 - "Audio Callback Header"
Cohesion: 0.24
Nodes (8): psnr_between(), Audio Reactivity Verification — Tests that injected audio features change visual, Audio features should visibly change source output., Audio features should change effect output when effects use audio uniforms., Effects that use u_rms should respond to RMS changes., test_rms_bass_beat_affect_sources(), TestAudioFeaturesAffectEffects, TestAudioFeaturesAffectSources

### Community 124 - "Uniform Bridge Header"
Cohesion: 0.31
Nodes (7): classify_vibe(), load_progress(), main(), Load progress from previous run., Save progress for resume., Analyze a rendered frame and classify into a vibe category.     Returns (vibe, s, save_progress()

### Community 125 - "Uniform Bridge Logic"
Cohesion: 0.33
Nodes (8): load_preset(), main(), Load a MilkDrop preset via the test API., Capture a rendered frame., Score a rendered image on visual interest (0-100).      Criteria:     - Non-blac, render_frame(), reset(), score_image()

### Community 134 - "Chained Signal Logic"
Cohesion: 0.32
Nodes (7): compute_psnr(), compute_ssim(), Vision Check — Image comparison for the Eyes visual testing harness.  Compares r, Compute Peak Signal-to-Noise Ratio between two images.      Returns float('inf'), Compute Structural Similarity Index between two images.      Uses scikit-image's, Compare a rendered frame against a golden reference.      Args:         rendered, verify_frame()

### Community 135 - "Main Component Header"
Cohesion: 0.32
Nodes (5): brightness(), psnr_between(), Time Sweep Verification — Tests that animated sources/effects change over time., Animated sources must produce different frames at different times., TestSourcesAnimateOverTime

### Community 143 - "Syphon Input Impl"
Cohesion: 0.38
Nodes (6): compute_psnr(), main(), Render clean baseline with no effects., Enable effect with explicit default params, render, compare to baseline., render_baseline(), test_effect_visual()

### Community 167 - "test_effect"
Cohesion: 0.67
Nodes (3): psnr(), Test if an effect with defaults produces visible change. Returns PSNR., test_effect()

## Knowledge Gaps
- **19 isolated node(s):** `MainWindow`, `MilkDropBrowser::PresetListContent`, `FilesBrowser::FileListContent`, `SourcesBrowser::SourceListContent`, `CompDecksBrowser::CompDeckListContent` (+14 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **97 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `VJAppController` connect `Main Component Controller` to `Binding Overlay`, `API Server`, `Signal Bar`, `MIDI Output Handler`, `Comp Decks Browser`, `FX Browser`, `Inspector Panel`, `Mapping Editor`?**
  _High betweenness centrality (0.001) - this node is a cross-community bridge._
- **Why does `TestIntegration` connect `MilkDrop Preset Browser` to `Session Recorder`?**
  _High betweenness centrality (0.000) - this node is a cross-community bridge._
- **What connects `Pytest configuration for Eyes visual tests.  Provides fixtures that spawn the Au`, `Find the built executable.`, `Spawn Audio-DNA in test mode for the entire test session.      The app starts on` to the rest of the system?**
  _173 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Main Component Controller` be split into smaller, more focused modules?**
  _Cohesion score 0.13333333333333333 - nodes in this community are weakly interconnected._
- **Should `Inspector Panel` be split into smaller, more focused modules?**
  _Cohesion score 0.125 - nodes in this community are weakly interconnected._
- **Should `ProjectM Source` be split into smaller, more focused modules?**
  _Cohesion score 0.044444444444444446 - nodes in this community are weakly interconnected._
- **Should `Effect Stack View` be split into smaller, more focused modules?**
  _Cohesion score 0.0625 - nodes in this community are weakly interconnected._