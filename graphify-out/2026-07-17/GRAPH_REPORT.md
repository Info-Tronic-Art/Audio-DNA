# Graph Report - RealTimeAudio  (2026-07-17)

## Corpus Check
- 260 files · ~381,773 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 4630 nodes · 7176 edges · 287 communities (187 shown, 100 thin omitted)
- Extraction: 91% EXTRACTED · 9% INFERRED · 0% AMBIGUOUS · INFERRED: 610 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `fb271e31`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Main Component Controller
- Renderer Core
- Compositor Engine
- API Server
- MilkDrop Preset Browser
- Signal Registry
- Layer Strip UI
- Clip Inspector
- Test Server
- Clip Cell UI
- BPM Tracker
- ProjectM Preset Manager
- Audio Readout Panel
- Curve Transforms
- Universal Param Control
- Layer Inspector
- Inspector Panel
- Session Recorder
- ProjectM Source
- Image Sequence Player
- Effect Stack View
- Deck View
- Renderer Accessors
- Files Browser
- Output Window
- Binding Manager
- Preferences Dialog
- MIDI Learn Overlay
- Signal Inspector
- Mapping Editor
- Look and Feel
- Effects Rack Panel
- Procedural Source
- Binding Overlay
- Composition Inspector
- Preset Manager
- Video Player
- Signal Bar
- MIDI Output Handler
- Video Recorder
- Browser Panel
- Comp Decks Browser
- FX Browser
- Top Bar
- Genre Detector
- Audio Engine
- Routing Engine
- Sources Browser
- Preview Panel
- MIDI Input Handler
- Texture Manager
- Shader Manager
- Source Registry
- Undo Manager
- Effect Library
- Effect Chain
- Mapping Engine
- Mapping Suggester
- Macro Panel
- MFCC Extractor
- Feature Bus
- Feedback Processor
- Autopilot
- Complete Source Table
- Timing Window
- Audio Feature Catalog from FEATURES_*.md
- Analysis Thread
- OSC Handler
- Section 10: Bindings
- Record Panel
- Knob Widget
- Menu Bar
- Claude Working Instructions
- Internals (extra)
- Ring Buffer
- Deck Model
- Link Sync
- OurAppFeatures_1.md — master feature list
- Slice 06: Browsers, Deck Grid, Clip Cells, Layer Strips, Signal Bar/Strip, Effect Stack
- Spectral Features
- Structural Detector
- Key Detector
- Loudness Analyzer
- FFT Processor
- Audio Engine IO
- Section 2: Signal System
- Section 11: Smart / AI Features
- Spectrum Display
- Waveform Display
- Advanced Audio Analyzer
- Pitch Tracker
- FEATURE_INVENTORY.md
- Composition Model
- App Entry Point
- LUT Loader
- Layer Serialization
- Section 8 partial: BPM / Tempo (TopBar controls)
- Section 13: Inspector Tabs and Sections
- UNIFIED_BUILD_PLAN.md — phase features
- Top Bar Header
- Preset Manager Header
- Audio Readout Header
- Undo Manager Header
- Command Pattern
- Feature Snapshot
- Session Recorder Header
- Syphon Input
- Syphon Output Header
- NDI Output
- NDI Input
- Spout Output
- Compositor Header
- Section 3: Deck / Layer / Clip Model
- Per-Phase Instructions
- Binding Manager Header
- Binding Target Mode
- Audio Callback Header
- Autopilot Header
- Layer Mix Mode
- Clip Playlist Trigger
- Preset Selector Logic
- Preset Selector Header
- Route Target Scope
- Link Sync Header
- Uniform Bridge Header
- Uniform Bridge Logic
- Effect Chain Header
- ISF Loader Header
- Clip Position Signal
- Envelope Signal
- Audio Signal
- Signal Category
- Chained Signal Header
- Handoff to Claude Code: Integrate Audio-DNA Design Directive
- Chained Signal Logic
- Main Component Header
- Mapping Types
- Spectrum Display Header
- Effects Rack Header
- Preferences Header
- Waveform Display Header
- Section 9: Input / Output
- SOURCES_procedural_generators.md — source catalog
- Syphon Input Impl
- Knob.cpp
- ChromaExtractor.cpp
- KeyDetector.cpp
- LoudnessAnalyzer.cpp
- SpectralFeatures.cpp
- StructuralDetector.cpp
- SyphonOutput.mm
- RingBuffer.h
- Deck.h
- LinkSync.cpp
- FFTProcessor.cpp
- AdvancedAudioAnalyzer
- PitchTracker.cpp
- AudioEngine.h
- AudioCallback.cpp
- Section 9: Input / Output
- Slice 05: Top Chrome Feature Audit
- SpectrumDisplay.cpp
- WaveformDisplay.cpp
- Smoother.h
- Composition.h
- test_integration_pipeline.cpp
- test_downbeat_detector.cpp
- test_effect
- Main.cpp
- LUTLoader.cpp
- Layer.cpp
- MacroBank.h
- Clip.cpp
- test_bpm_stabilization.cpp
- test_spectral_features.cpp
- test_mapping_engine.cpp
- MappingEngine.h
- displaySnap_
- PresetManager.h
- displaySnap_
- UndoManager.h
- FeatureSnapshot
- SessionRecorder.h
- SyphonInput.h
- SyphonOutput.h
- NdiOutput.h
- NdiInput.h
- SpoutOutput.h
- CompositorEngine
- LUTLoader.h
- EmbeddedShaders
- BindingManager
- Binding.h
- AudioIODeviceCallback
- Autopilot
- Layer.h
- Clip.h
- PresetSelector.cpp
- PresetSelector.h
- Route.h
- LinkSync.h
- UniformBridge.h
- applyDemoMappings
- EffectChain
- ISFShaderLoader.h
- ClipPositionSignal
- EnvelopeSignal
- AudioSignal
- Category
- ChainedSignal
- OscillatorSignal.h
- ChainedSignal.cpp
- Section 4: Effects Library (135 effects)
- test_composition.cpp
- Slice 12 — Top-Level Docs Feature Audit
- UI/UX design patterns adopted
- test_routing_engine.cpp
- test_compositor.cpp
- test_smoother.cpp
- test_ring_buffer.cpp
- test_feature_bus.cpp
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
- Renderer
- Request
- Response
- RoutingEngine
- SessionRecorder
- SignalRegistry
- SourceRegistry
- atomic
- function
- Server
- thread
- File
- Image
- vector
- EffectsRackPanel.cpp
- FeatureBus
- SyphonOutputImpl
- AnalysisThread.cpp
- paintSectionHeader
- test_bpm_stabilization.cpp
- advancePlayback
- .getComposition
- String
- StringArray
- Component
- Graphics
- Rectangle
- String
- Tab
- JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR
- VideoRecorder
- MenuBarModel
- String
- StringArray
- Layer::fromVar
- Composition.h
- EffectSlot
- File
- PerTypeAutopilotConfig
- vector
- Graphics
- Label
- Rectangle
- RingBuffer
- kOnsetWindowSize
- String
- AudioDeviceManager
- AnalysisThread
- SyphonOutput
- VideoRecorder
- TextEditor
- TextButton
- Timer
- ToggleButton
- TopBar

## God Nodes (most connected - your core abstractions)
1. `MainComponent` - 189 edges
2. `Renderer` - 169 edges
3. `Clip` - 114 edges
4. `ClipInspector` - 106 edges
5. `Layer` - 99 edges
6. `MilkDropBrowser` - 94 edges
7. `BPMTracker` - 92 edges
8. `LayerInspector` - 89 edges
9. `FeatureSnapshot` - 82 edges
10. `CompositorEngine` - 78 edges

## Surprising Connections (you probably didn't know these)
- `PipelineRunner` --references--> `kBlockSize`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/AnalysisThread.h
- `runPipeline()` --references--> `FeatureSnapshot`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/FeatureSnapshot.h
- `makeSnapshot()` --references--> `FeatureSnapshot`  [EXTRACTED]
  tests/test_mapping_engine.cpp → src/analysis/FeatureSnapshot.h
- `MainComponent::MainComponent()` --calls--> `closeOutput`  [INFERRED]
  src/MainComponent.cpp → src/MainComponent.h
- `MainComponent::MainComponent()` --calls--> `handleMenuCommand`  [INFERRED]
  src/MainComponent.cpp → src/MainComponent.h

## Import Cycles
- None detected.

## Communities (287 total, 100 thin omitted)

### Community 0 - "Main Component Controller"
Cohesion: 0.01
Nodes (137): ApiServer, AudioDNALookAndFeel, AudioDNAMenuBar, AudioReadoutPanel, BindingManager, BindingOverlay, BrowserPanel, CameraDevice::Listener (+129 more)

### Community 1 - "Renderer Core"
Cohesion: 0.02
Nodes (87): OpenGLContext, OpenGLRenderer, promise, GLuint, Image, ProceduralSource, SourceParam, Renderer (+79 more)

### Community 2 - "Compositor Engine"
Cohesion: 0.02
Nodes (76): AlphaType, BeatSnapMode, BlendOverride, LoopMode, MediaType, PlaylistCycleMode, PlaylistTrigger, PresetEntry (+68 more)

### Community 3 - "API Server"
Cohesion: 0.03
Nodes (57): aubio_tempo_t, BPMTracker, beatCounter_, beatScorePos_, beatScores_, cachedBassEnergy_, cachedHarmonicChange_, cachedSpectralFlux_ (+49 more)

### Community 4 - "MilkDrop Preset Browser"
Cohesion: 0.03
Nodes (71): DragTarget, ClipInspector, anchorControl_, autopilotActionSelector_, autopilotDurationSelector_, beatDivisionLabel_, beatDivisionSelector_, beatSnapSelector_ (+63 more)

### Community 5 - "Signal Registry"
Cohesion: 0.09
Nodes (61): atomic, function, Request, Response, Server, ApiServer, handleComposition, handleGetBpm (+53 more)

### Community 6 - "Layer Strip UI"
Cohesion: 0.03
Nodes (59): AutoSizeMode, KeyingMode, AutopilotAction, AutopilotDuration, EffectSlot, MixMode, Type, vector (+51 more)

### Community 7 - "Clip Inspector"
Cohesion: 0.03
Nodes (63): PresetListContent, ComboBox, Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, PlayMode, ProjectMPresetManager (+55 more)

### Community 8 - "Test Server"
Cohesion: 0.05
Nodes (57): CellPos, Composition, Graphics, DeckView, activeColumn_, clearSelection, clipCells_, columnTriggers_ (+49 more)

### Community 9 - "Clip Cell UI"
Cohesion: 0.06
Nodes (54): ClipCell, active_, clip_, column_, dragHover_, fileDragEnter, fileDragExit, filesDropped (+46 more)

### Community 10 - "BPM Tracker"
Cohesion: 0.03
Nodes (61): ComboBox, Component, DragAndDropTarget, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, SignalRegistry, TextButton (+53 more)

### Community 11 - "ProjectM Preset Manager"
Cohesion: 0.04
Nodes (44): AnalysisThread, advancedAnalyzer_, analysisBuffer_, bpmTracker_, chromaExtractor_, cpuLoad_, currentPeak_, currentRMS_ (+36 more)

### Community 12 - "Audio Readout Panel"
Cohesion: 0.04
Nodes (55): CompositionInspector, anchorControl_, apClipLoopsSlider_, apDurationSelector_, apForwardBtn_, apLoopToggle_, apMasterLayerSelector_, apOffBtn_ (+47 more)

### Community 13 - "Curve Transforms"
Cohesion: 0.06
Nodes (30): PixelBuffer, array, atomic, Config, File, function, thread, VideoRecorder (+22 more)

### Community 14 - "Universal Param Control"
Cohesion: 0.06
Nodes (44): CompDeckListContent, CompDecksBrowser, CompDecksBrowser::CompDeckListContent, kEntryRowHeight, kSectionHeaderHeight, CompDecksBrowser::~CompDecksBrowser(), composition_, compositions_ (+36 more)

### Community 15 - "Layer Inspector"
Cohesion: 0.11
Nodes (47): Composition, Composition, EffectChain, FeatureBus, Renderer, Request, Response, RoutingEngine (+39 more)

### Community 16 - "Inspector Panel"
Cohesion: 0.04
Nodes (52): ResettableSlider, array, ComboBox, Component, Composition, FeatureBus, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (+44 more)

### Community 17 - "Session Recorder"
Cohesion: 0.04
Nodes (44): AutopilotDirection, AutopilotDurationMode, CrossfaderBehaviour, CrossfaderBlendMode, CrossfaderCurve, EffectSlot, QuantizeMode, Composition (+36 more)

### Community 18 - "ProjectM Source"
Cohesion: 0.05
Nodes (46): ListenerList, Graphics, Listener, MappingCurve, MappingSource, String, getCurveName(), getSourceName() (+38 more)

### Community 19 - "Image Sequence Player"
Cohesion: 0.05
Nodes (41): SourceEntry, SourceListContent, SourcesBrowser, Component, Graphics, MouseEvent, String, CategoryInfo (+33 more)

### Community 20 - "Effect Stack View"
Cohesion: 0.06
Nodes (39): EffectEntry, FXListContent, Component, Graphics, MouseEvent, String, FXBrowser, buildCategoryList (+31 more)

### Community 21 - "Deck View"
Cohesion: 0.06
Nodes (44): Composition, Graphics, SignalRegistry, Tab, Component, Composition, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Tab (+36 more)

### Community 22 - "Renderer Accessors"
Cohesion: 0.04
Nodes (44): ComboBox, Component, function, Image, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Rectangle, String, TextButton (+36 more)

### Community 23 - "Files Browser"
Cohesion: 0.05
Nodes (37): SourceRenderFn, CompositorEngine, accumulatorFBO_, accumulatorTex_, effectFBO_A_, effectFBO_B_, effectLibrary_, effectTex_A_ (+29 more)

### Community 24 - "Output Window"
Cohesion: 0.05
Nodes (34): Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, SignalRegistry, SourceMode, String, TextButton (+26 more)

### Community 25 - "Binding Manager"
Cohesion: 0.07
Nodes (40): EffectRow, EffectSlot, Graphics, MouseEvent, SourceDetails, vector, EffectStackView, effectLibrary_ (+32 more)

### Community 26 - "Preferences Dialog"
Cohesion: 0.09
Nodes (40): Drawable, Font, ScrollBar, AudioDNALookAndFeel, AudioDNALookAndFeel::AudioDNALookAndFeel(), drawButtonBackground, drawButtonText, drawComboBox (+32 more)

### Community 27 - "MIDI Learn Overlay"
Cohesion: 0.08
Nodes (22): AudioCallback, AudioDeviceManager, AudioFormatManager, AudioFormatReaderSource, AudioSourcePlayer, AudioTransportSource, ChangeListener, CombinedCallback (+14 more)

### Community 28 - "Signal Inspector"
Cohesion: 0.05
Nodes (41): FeatureSnapshot, bandEnergies, barCount, barPhase, beatInBar, beatPhase, bpm, chromagram (+33 more)

### Community 29 - "Mapping Editor"
Cohesion: 0.07
Nodes (39): Graphics, Rectangle, String, ComboBox, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Rectangle, ToggleButton (+31 more)

### Community 30 - "Look and Feel"
Cohesion: 0.11
Nodes (22): Command, description, execute, undo, string, unique_ptr, function, unique_ptr (+14 more)

### Community 31 - "Effects Rack Panel"
Cohesion: 0.15
Nodes (34): applyClipEffects, applyClipTransform, applyFXOnlyLayer, applyLayerKeying, applyLayerTransform, applyMaskLayer, applyScreenSplit, applyTransition (+26 more)

### Community 32 - "Procedural Source"
Cohesion: 0.08
Nodes (22): Param, OpenGLShaderProgram, GLuint, string, vector, ProceduralSource, category_, currentPing_ (+14 more)

### Community 33 - "Binding Overlay"
Cohesion: 0.05
Nodes (31): AdvancedAudioAnalyzer, bassHistory_, envelopeFull_, envelopePos_, fftSize_, formantBinHigh_, formantBinLow_, formantMax_ (+23 more)

### Community 34 - "Composition Inspector"
Cohesion: 0.11
Nodes (31): Binding, Image, File, Graphics, advanceSlideshow, beatSyncRandomize, fastSave, filesDropped (+23 more)

### Community 35 - "Preset Manager"
Cohesion: 0.16
Nodes (35): AudioReadoutPanel, displayBands_, displaySnap_, downbeatFlash_, drawBandMeters, drawBarIndicator, drawBeatPhase, drawDbMeter (+27 more)

### Community 36 - "Video Player"
Cohesion: 0.10
Nodes (31): CriticalSection, File, string, atomic, Event, vector, SessionRecorder, clear (+23 more)

### Community 37 - "Signal Bar"
Cohesion: 0.09
Nodes (36): CategoryHeader, EffectSection, EffectChain, Graphics, EffectsRackPanel, activeMappingEditor_, categoryHeaders_, closeMappingEditor (+28 more)

### Community 38 - "MIDI Output Handler"
Cohesion: 0.11
Nodes (19): DisplaySize, Component, DisplaySize, FeatureBus, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, SignalRegistry, TextButton (+11 more)

### Community 39 - "Video Recorder"
Cohesion: 0.08
Nodes (24): File, GLuint, LUTLoader, loadCubeFile, releaseLUT, File, GLuint, Image (+16 more)

### Community 40 - "Browser Panel"
Cohesion: 0.08
Nodes (19): deque, mutex, function, string, PresetSelector, barsSinceLastSwitch_, enabled_, energyMatching_ (+11 more)

### Community 41 - "Comp Decks Browser"
Cohesion: 0.09
Nodes (31): BindingOverlay, active_, exitBindingMode, findExistingBinding, getKeyDescription, hitTestTarget, keyPressed, mouseDown (+23 more)

### Community 42 - "FX Browser"
Cohesion: 0.10
Nodes (32): GLState, GLuint, string, mutex, string, vector, ProjectMSource, applyParams (+24 more)

### Community 43 - "Top Bar"
Cohesion: 0.09
Nodes (27): AnalysisThread, Component, FeatureBus, File, Image, ImageSequence, VideoPlayer, attachTo (+19 more)

### Community 44 - "Genre Detector"
Cohesion: 0.08
Nodes (28): Features, GenreDetector, candidateGenre_, chromaticComplexity, classify, computeEnergyState, computeScores, confidence_ (+20 more)

### Community 45 - "Audio Engine"
Cohesion: 0.07
Nodes (32): Component, File, Image, EffectChain, FeatureBus, File, Image, mutex (+24 more)

### Community 46 - "Routing Engine"
Cohesion: 0.08
Nodes (30): ChromaExtractor, FFTProcessor, KeyDetector, LoudnessAnalyzer, MFCCExtractor, OnsetDetector, PitchTracker, kBlockSize (+22 more)

### Community 47 - "Sources Browser"
Cohesion: 0.07
Nodes (30): FileListContent, FilesBrowser, currentDir_, entries_, favorites_, gridView_, gridViewBtn_, kNavBarHeight (+22 more)

### Community 48 - "Preview Panel"
Cohesion: 0.09
Nodes (30): MidiOutput, Array, Deck, MidiDeviceInfo, MidiMessage, PadState, String, array (+22 more)

### Community 49 - "MIDI Input Handler"
Cohesion: 0.14
Nodes (20): ProgramEntry, File, GLint, OpenGLContext, OpenGLShaderProgram, String, File, OpenGLContext (+12 more)

### Community 50 - "Texture Manager"
Cohesion: 0.08
Nodes (19): psnr_between(), Signal Routing Verification — Tests the complete signal→route→parameter→shader p, RMS should change effect output via u_rms uniform., End-to-end: create route from audio signal to effect parameter,     inject featu, Check if signal API endpoints are available., Route Volume signal → Ripple intensity → verify RMS changes ripple., Route with threshold=0.5 should only activate above 0.5 RMS., Inverted route: high RMS should DECREASE the parameter. (+11 more)

### Community 51 - "Shader Manager"
Cohesion: 0.15
Nodes (11): paint, refresh, resized, setActiveTab, setComposition, setEffectLibrary, showActiveTab, updateTabButtonColors (+3 more)

### Community 52 - "Source Registry"
Cohesion: 0.14
Nodes (13): GLHost, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Tab, TextButton, PreviewPanel, activeTab_, glHost_ (+5 more)

### Community 53 - "Undo Manager"
Cohesion: 0.10
Nodes (26): GLint, GLuint, OpenGLShaderProgram, unique_ptr, EffectChain, addEffect, applyDryWet, effects_ (+18 more)

### Community 54 - "Effect Library"
Cohesion: 0.08
Nodes (25): ComboBox, Component, File, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, TextButton, RecordPanel (+17 more)

### Community 55 - "Effect Chain"
Cohesion: 0.07
Nodes (28): Action, CCMode, InputType, Binding, action, ccMode, ccStepSize, enabled (+20 more)

### Community 57 - "Mapping Suggester"
Cohesion: 0.08
Nodes (12): ControlPoint, CurveType, EnvelopeSignal, amplitude_, beatDuration_, curveType_, looping_, oneShot_ (+4 more)

### Community 58 - "Macro Panel"
Cohesion: 0.09
Nodes (12): Macro, Scope, Array, Composition, SignalRegistry, RoutingEngine, array, SignalRegistry (+4 more)

### Community 59 - "MFCC Extractor"
Cohesion: 0.08
Nodes (20): String, Effect, addParam, category_, dryWet_, Effect::Effect(), enabled_, name_ (+12 more)

### Community 60 - "Feature Bus"
Cohesion: 0.07
Nodes (31): EffectChain, MappingCurve, MappingSource, vector, MappingEngine, addMapping, applyCurve, clearAll (+23 more)

### Community 61 - "Feedback Processor"
Cohesion: 0.12
Nodes (23): OSCMessage, OSCReceiver, OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>, atomic, function, OscHandler, listening_, onSetBpm (+15 more)

### Community 62 - "Autopilot"
Cohesion: 0.13
Nodes (25): Any, Exception, AccessibilityPermissionError, AppNotFoundError, AXInspectorError, check_accessibility_permissions(), find_pid_by_name(), _get_ax_attribute() (+17 more)

### Community 63 - "Complete Source Table"
Cohesion: 0.15
Nodes (20): MouseEvent, PopupMenu, String, buildSourcePickerMenu, getPreferredHeight, handleSourcePickerResult, mouseDown, onExpandToggled (+12 more)

### Community 64 - "Timing Window"
Cohesion: 0.10
Nodes (23): BindingManager, Array, AudioDeviceManager, BindingManager, MidiDeviceInfo, MidiInput, MidiMessage, String (+15 more)

### Community 65 - "Audio Feature Catalog from FEATURES_*.md"
Cohesion: 0.16
Nodes (25): applyCurve(), backIn(), backInOut(), backOut(), bounceIn(), bounceInOut(), bounceOut(), circularIn() (+17 more)

### Community 66 - "Analysis Thread"
Cohesion: 0.10
Nodes (11): Deck, id, kDefaultColumns, kDefaultLayers, layers, name, nextLayerId_, numColumns (+3 more)

### Community 67 - "OSC Handler"
Cohesion: 0.09
Nodes (21): StructuralDetector, candidateState_, classifyState, confirmedState_, fluxAlpha_, fluxEnv_, holdCounter_, holdThreshold_ (+13 more)

### Community 68 - "Section 10: Bindings"
Cohesion: 0.09
Nodes (19): frame_is_not_black(), frames_are_different(), Eyes Visual Tests — Comprehensive Fractal Source Validation  Tests EVERY control, Every fractal must render a visible frame at defaults., Every parameter must produce a visible change when modified., Zoom at ALL positions (0, 0.25, 0.5, 0.75, 1.0) must be non-black., Dive speed at various levels must not go black., Power at all positions must not go black. (+11 more)

### Community 69 - "Record Panel"
Cohesion: 0.09
Nodes (22): Column, kWaveformBufferSize, Graphics, array, Component, kMaxColumns, Timer, WaveformDisplay (+14 more)

### Community 70 - "Knob Widget"
Cohesion: 0.14
Nodes (14): DialogWindow, Component, function, Graphics, Rectangle, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, PreferencesDialog, closeButtonPressed (+6 more)

### Community 71 - "Menu Bar"
Cohesion: 0.09
Nodes (22): DisplaySize, MouseEvent, Component, DisplaySize, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, SignalRegistry, SignalStrip (+14 more)

### Community 72 - "Claude Working Instructions"
Cohesion: 0.12
Nodes (20): MelFilter, array, MFCCExtractor, buildDCTMatrix, buildFilterbank, dctMatrix_, fftSize_, filterWeightOffsets_ (+12 more)

### Community 73 - "Internals (extra)"
Cohesion: 0.16
Nodes (18): analyzeDownbeatPosition, correctOctaveError, feedDownbeatFeatures, feedSilenceDetection, foldBPMToRange, process, processRawBPM, pushAndMedian (+10 more)

### Community 74 - "Ring Buffer"
Cohesion: 0.17
Nodes (14): EffectDef, String, StringArray, unique_ptr, EffectLibrary, createEffect, defs_, getEffectDef (+6 more)

### Community 75 - "Deck Model"
Cohesion: 0.11
Nodes (12): Category, string, Type, Signal, category_, getValue, id_, name_ (+4 more)

### Community 76 - "Link Sync"
Cohesion: 0.10
Nodes (15): BandRange, array, SpectralFeatures, bandMaxEnergy_, bandRanges_, computeBandBinRanges, fftSize_, fluxMax_ (+7 more)

### Community 77 - "OurAppFeatures_1.md — master feature list"
Cohesion: 0.13
Nodes (20): MacroSlot, Graphics, array, Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, SignalRegistry, MacroPanel (+12 more)

### Community 78 - "Slice 06: Browsers, Deck Grid, Clip Cells, Layer Strips, Signal Bar/Strip, Effect Stack"
Cohesion: 0.09
Nodes (35): Category, ClipPositionSignal, ISFShader, File, string, ISFShaderLoader, convertToGLSL, extractGLSLBody (+27 more)

### Community 79 - "Spectral Features"
Cohesion: 0.12
Nodes (18): Graphics, String, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, String, Knob, kPreferredHeight (+10 more)

### Community 80 - "Structural Detector"
Cohesion: 0.12
Nodes (28): PlayMode, ProjectMPresetManager, string, SubTab, vector, addToRecent, calculateContentHeight, firePresetSelected (+20 more)

### Community 81 - "Key Detector"
Cohesion: 0.11
Nodes (15): array, KeyDetector, candidateCount_, candidateKey_, candidateMajor_, kHysteresisFrames, majorProfile_, minorProfile_ (+7 more)

### Community 82 - "Loudness Analyzer"
Cohesion: 0.17
Nodes (14): vector, vector, RoutingEngine, addRoute, clearAll, getRoute, getRouteAt, getRoutesForSource (+6 more)

### Community 83 - "FFT Processor"
Cohesion: 0.14
Nodes (17): Autopilot, advanceClip, getActionForClip, getBeatsForClip, getPerTypeAction, getPerTypeBeats, lastBeatPhase_, lastEnergyState_ (+9 more)

### Community 84 - "Audio Engine IO"
Cohesion: 0.20
Nodes (9): SessionRecorder, Graphics, onPlayRecording, onStartRecording, onStopRecording, paint, RecordPanel::RecordPanel(), refresh (+1 more)

### Community 85 - "Section 2: Signal System"
Cohesion: 0.11
Nodes (19): SourceType, Route, dialRangeMax, dialRangeMin, enabled, falloff, gain, id (+11 more)

### Community 86 - "Section 11: Smart / AI Features"
Cohesion: 0.18
Nodes (18): SourceFactory, SourceInfo, SourceRegistry, string, unique_ptr, vector, string, unordered_map (+10 more)

### Community 87 - "Spectrum Display"
Cohesion: 0.11
Nodes (19): BrowserPanel, activeTab_, compDecksBrowser_, compDecksTabBtn_, filesBrowser_, filesTabBtn_, fxBrowser_, fxTabBtn_ (+11 more)

### Community 88 - "Waveform Display"
Cohesion: 0.13
Nodes (16): Graphics, Tab, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Tab, TextButton, TimingWindow, activeTab_ (+8 more)

### Community 89 - "Advanced Audio Analyzer"
Cohesion: 0.12
Nodes (13): FFT, FFTProcessor, buildHannWindow, fft_, fftData_, FFTProcessor::FFTProcessor(), hannWindow_, kFFTOrder (+5 more)

### Community 90 - "Pitch Tracker"
Cohesion: 0.12
Nodes (16): optional, FeedbackConfig, amount, enabled, lumaKey, offsetX, offsetY, presetName (+8 more)

### Community 91 - "FEATURE_INVENTORY.md"
Cohesion: 0.15
Nodes (17): FeatureBus, acquireRead, acquireWrite, buffers_, FeatureBus::FeatureBus(), getLatestRead, hasNewData, kLatestMask (+9 more)

### Community 92 - "Composition Model"
Cohesion: 0.14
Nodes (15): GLuint, string, FeedbackProcessor, applyPreset, currentBuffer_, ensureSize, fbos_, height_ (+7 more)

### Community 93 - "App Entry Point"
Cohesion: 0.13
Nodes (11): atomic, LinkSync, beatPhase_, bpm_, enabled_, numPeers_, quantum_, requestBeatAtTime (+3 more)

### Community 94 - "LUT Loader"
Cohesion: 0.06
Nodes (44): AudioDeviceManager, BindableTarget, BindingManager, Component, Composition, Graphics, KeyPress, MidiInput (+36 more)

### Community 95 - "Layer Serialization"
Cohesion: 0.23
Nodes (8): Colour, Component, Graphics, MouseEvent, PresetInfo, Section, paint, MilkDropBrowser::PresetListContent

### Community 96 - "Section 8 partial: BPM / Tempo (TopBar controls)"
Cohesion: 0.13
Nodes (12): BiquadState, BiquadState, LoudnessAnalyzer, fillCount_, process, processBiquad, runningSum_, stage1_ (+4 more)

### Community 97 - "Section 13: Inspector Tabs and Sections"
Cohesion: 0.18
Nodes (17): buildSourceParamControls, ClipInspector::ClipInspector(), getPreferredHeight, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped, onCuepointSet (+9 more)

### Community 98 - "UNIFIED_BUILD_PLAN.md — phase features"
Cohesion: 0.14
Nodes (12): _make_mock_element(), Tests for the recursive tree walker., Walk a leaf element with no children., Walk a tree with nested children., Walk an element with no attributes set., Tests for the --depth recursion limiter., Depth 0 returns the root element without walking children., Depth 1 walks immediate children but not grandchildren. (+4 more)

### Community 99 - "Top Bar Header"
Cohesion: 0.06
Nodes (22): Eyes Visual Tests — Render Pipeline  Tests the core rendering pipeline: image lo, Two effects chained should both apply., Verify audio feature injection works., Injecting features should succeed., Verify state endpoint works., State endpoint should list all effects., Verify reset clears state properly., After reset, no effects should be enabled. (+14 more)

### Community 100 - "Preset Manager Header"
Cohesion: 0.13
Nodes (7): string, OscillatorSignal, amplitude_, beatDuration_, phaseOffset_, shape_, WaveShape

### Community 101 - "Audio Readout Header"
Cohesion: 0.14
Nodes (13): all_effects(), brightness(), image_loaded(), psnr_between(), Auto-Discovering Effect Verification — Tests ALL registered effects.  Queries /a, No effect should turn the image completely black or white., Discover all effects from the running app., Ensure test image is loaded. (+5 more)

### Community 102 - "Undo Manager Header"
Cohesion: 0.17
Nodes (14): analyze_sweep(), brightness(), Tier 2: Range Quality Analysis  For each source parameter, renders at 11 positio, Sweep every parameter and verify quality metrics., Each parameter must have >70% useful range, no discontinuities, no dead zones., Auto-discover all sources and sweep all their params.      This class discovers, Sweep every param on every source. Generates CSV reports., Render a source at multiple parameter positions, return list of (value, path) tu (+6 more)

### Community 103 - "Command Pattern"
Cohesion: 0.18
Nodes (8): aubio_onset_t, fvec_t, OnsetDetector, hopSize_, input_, onset_, output_, process

### Community 105 - "Session Recorder Header"
Cohesion: 0.15
Nodes (12): ChromaExtractor, binToChroma_, ChromaExtractor::ChromaExtractor(), computeBinToChromaMap, fftSize_, hasPrevFrame_, kNumChroma, numBins_ (+4 more)

### Community 106 - "Syphon Input"
Cohesion: 0.20
Nodes (14): MouseEvent, getPreferredHeight, LayerInspector::LayerInspector(), mouseDown, onLayerNameChanged, populateBlendModes, populateKeyingModes, refresh (+6 more)

### Community 107 - "Syphon Output Header"
Cohesion: 0.22
Nodes (20): DeckState, Array, EffectChain, File, MappingCurve, MappingSource, String, PresetManager (+12 more)

### Community 109 - "NDI Input"
Cohesion: 0.12
Nodes (8): Configure the entire effect chain.          Disables all existing effects, then, Render a single frame and save it to disk.          Args:             output_pat, Reset all effects, clear images, restore defaults., Load a procedural source into the active clip.          Args:             source, Update parameters on the currently active source.          Args:             par, Remove a signal route by ID., Send a POST request with JSON body., Enable/disable an effect and optionally set parameters.          Args:

### Community 110 - "Spout Output"
Cohesion: 0.16
Nodes (13): Display, OutputComponent, enterBindingMode, KeyPress, DocumentWindow, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, OutputWindow, closeButtonPressed (+5 more)

### Community 111 - "Compositor Header"
Cohesion: 0.19
Nodes (7): vector, unordered_map, string, set, BPMTracker, feedConstantBPM(), feedWithBeats()

### Community 112 - "Section 3: Deck / Layer / Clip Model"
Cohesion: 0.13
Nodes (33): map, PresetInfo, string, vector, function, PresetInfo, string, vector (+25 more)

### Community 113 - "Per-Phase Instructions"
Cohesion: 0.13
Nodes (13): AdvancedAudioAnalyzer, BPMTracker, ChromaExtractor, FFTProcessor, GenreDetector, KeyDetector, LoudnessAnalyzer, MFCCExtractor (+5 more)

### Community 114 - "Binding Manager Header"
Cohesion: 0.17
Nodes (10): Component, MouseEvent, Point, fileListContent_, dragStarted_, kLabelHeight, kListRowHeight, kPadding (+2 more)

### Community 115 - "Binding Target Mode"
Cohesion: 0.29
Nodes (12): File, Image, FilesBrowser::~FilesBrowser(), filterBySearch, generateThumbnail, isMediaFile, loadFavorites, navigateTo (+4 more)

### Community 116 - "Audio Callback Header"
Cohesion: 0.20
Nodes (11): CaretOnlyComboBoxLookAndFeel, Button, Colour, ComboBox, Graphics, Label, LookAndFeel_V4, FlatButtonLookAndFeel (+3 more)

### Community 117 - "Autopilot Header"
Cohesion: 0.21
Nodes (14): MouseEvent, Point, mouseDown, mouseDrag, populateBlendDropdown, populateTransitionDropdown, refresh, resized (+6 more)

### Community 118 - "Layer Mix Mode"
Cohesion: 0.14
Nodes (8): Tests for the argparse-based CLI entry point., --help prints usage and exits without calling inspect_app., Exit code 2 when app is not found., Exit code 1 when permissions denied., Default output goes to stdout as valid JSON., --output writes JSON to a file., --depth flag is forwarded to inspect_app., TestCLI

### Community 119 - "Clip Playlist Trigger"
Cohesion: 0.18
Nodes (11): all_sources(), brightness(), psnr_between(), Auto-Discovering Source Verification — Tests ALL registered procedural sources., Sweep critical params across 5 positions to check for discontinuities., Discover all sources from the running app., Every registered source must render a non-black frame at defaults., Every param on every source must have a visible effect. (+3 more)

### Community 120 - "Preset Selector Logic"
Cohesion: 0.13
Nodes (8): Get the current engine state (effects, FPS, etc.)., Controls Audio-DNA via the Eyes HTTP API., Get all registered procedural sources with their parameters., Get all signals with cached values., Get all active routes with current output values., Spawn the app in test mode and wait for it to become ready.          Args:, Check if the app is running and ready., VJAppController

### Community 121 - "Preset Selector Header"
Cohesion: 0.16
Nodes (8): ActionCallback, BindingCaptureCallback, BindingManager, bindingMode_, bindings_, nextId_, relativeCCValues_, vector

### Community 122 - "Route Target Scope"
Cohesion: 0.08
Nodes (19): Command, Composition, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, MenuBarModel, PopupMenu, AudioDNAMenuBar, getMenuBarNames, getMenuForIndex (+11 more)

### Community 123 - "Link Sync Header"
Cohesion: 0.13
Nodes (15): array, Component, FeatureBus, Timer, uint32, SpectrumDisplay, displayBands_, kAttackAlpha (+7 more)

### Community 124 - "Uniform Bridge Header"
Cohesion: 0.15
Nodes (10): atomic, string, SyphonOutput, enabled_, impl_, init, publishTexture, serverName_ (+2 more)

### Community 126 - "Effect Chain Header"
Cohesion: 0.18
Nodes (8): aubio_pitch_t, fvec_t, PitchTracker, hopSize_, input_, output_, pitch_, process

### Community 127 - "ISF Loader Header"
Cohesion: 0.24
Nodes (8): SliderLayout, Slider, SliderStyle, FadeSpeedSliderLookAndFeel, FullBoundsSliderLAF, KeyingSliderLookAndFeel, OpacitySliderLookAndFeel, SpeedSliderLookAndFeel

### Community 128 - "Clip Position Signal"
Cohesion: 0.18
Nodes (7): atomic, RingBuffer, buffer_, mask_, readPos_, writePos_, T

### Community 129 - "Envelope Signal"
Cohesion: 0.19
Nodes (12): addBinding, clearAll, fromVar, getBinding, getBindingAt, getRelativeCCValue, loadFromFile, removeBinding (+4 more)

### Community 131 - "Signal Category"
Cohesion: 0.20
Nodes (9): AudioEngine::AudioEngine(), getCurrentSampleRate, getDeviceStatus, hasAudioDevice, isPlaying, pause, play, RingBuffer (+1 more)

### Community 132 - "Chained Signal Header"
Cohesion: 0.17
Nodes (14): FeatureBus, Graphics, SignalRegistry, getPreferredHeight, grow, onSignalSelected, onSizeChanged, paint (+6 more)

### Community 133 - "Handoff to Claude Code: Integrate Audio-DNA Design Directive"
Cohesion: 0.14
Nodes (13): FeatureBus, File, Graphics, Image, Tab, clearImage, loadImage, paint (+5 more)

### Community 134 - "Chained Signal Logic"
Cohesion: 0.17
Nodes (3): Smoother, initialized_, Composition

### Community 135 - "Main Component Header"
Cohesion: 0.19
Nodes (15): BindableTarget, KeyPress, Component, buildBindableTargets, closeOutput, enterKeyboardBindingMode, enterMidiLearnMode, exitAllBindingModes (+7 more)

### Community 137 - "Spectrum Display Header"
Cohesion: 0.17
Nodes (11): app(), _audio_dna_running(), _ensure_ax_mocks(), _mock_copy_attribute(), Tests for the accessibility tree inspector.  Unit tests mock the AX API so they, Mock for AXUIElementCopyAttributeValue that reads from element._attrs., Install mock modules for ApplicationServices and Cocoa if needed., Check if Audio-DNA is running (for integration test gating). (+3 more)

### Community 138 - "Effects Rack Header"
Cohesion: 0.19
Nodes (10): AudioIODevice, AudioIODeviceCallback, AudioIODeviceCallbackContext, AudioCallback, AudioCallback::AudioCallback(), audioDeviceAboutToStart, audioDeviceIOCallbackWithContext, audioDeviceStopped (+2 more)

### Community 139 - "Preferences Header"
Cohesion: 0.67
Nodes (3): Clip::fromVar(), Clip::toVar(), var

### Community 143 - "Syphon Input Impl"
Cohesion: 0.18
Nodes (11): TextButton, LayerStrip::LayerStrip(), onBlendModeChanged, onBypass, onClearClip, onSolo, onTransportBack, onTransportForward (+3 more)

### Community 144 - "Knob.cpp"
Cohesion: 0.24
Nodes (10): FeatureSnapshot, evaluateAll, Graphics, paint, paintBarPhraseDisplay, paintBeatWheel, setDspLoad, setFps (+2 more)

### Community 145 - "ChromaExtractor.cpp"
Cohesion: 0.22
Nodes (8): psnr_between(), Audio Reactivity Verification — Tests that injected audio features change visual, Audio features should visibly change source output., RMS, bass, and beat phase should affect most sources that use u_rms/u_beatPhase., Audio features should change effect output when effects use audio uniforms., Effects that use u_rms should respond to RMS changes., TestAudioFeaturesAffectEffects, TestAudioFeaturesAffectSources

### Community 146 - "KeyDetector.cpp"
Cohesion: 0.22
Nodes (11): Config, File, closeEncoder, encodeFrame, encoderThreadFunc, flushEncoder, getRecordedDuration, initEncoder (+3 more)

### Community 147 - "LoudnessAnalyzer.cpp"
Cohesion: 0.47
Nodes (10): Colour, Graphics, Rectangle, String, getFormattedValue, getSignalColour, paint, paintExpanded (+2 more)

### Community 148 - "SpectralFeatures.cpp"
Cohesion: 0.22
Nodes (8): app(), _default_executable(), Pytest configuration for Eyes visual tests.  Provides fixtures that spawn the Au, Find the built executable., Spawn Audio-DNA in test mode for the entire test session.      The app starts on, Reset app state before each test for isolation., reset_between_tests(), VJ App Controller — Python client for the Eyes test harness HTTP API.  Wraps the

### Community 149 - "StructuralDetector.cpp"
Cohesion: 0.20
Nodes (6): Tests for application discovery and error handling., Raises AccessibilityPermissionError when AX permissions are denied., Raises AppNotFoundError when the app is not running., inspect_app with explicit PID skips name lookup., inspect_app finds PID by app name when pid is not provided., TestAppLookup

### Community 150 - "SyphonOutput.mm"
Cohesion: 0.20
Nodes (5): Performance Verification — Tests that sources and effects render within budget., Every source must render within budget., Effects should not significantly slow down rendering., TestEffectPerformance, TestSourcePerformance

### Community 151 - "RingBuffer.h"
Cohesion: 0.31
Nodes (8): ndarray, compute_psnr(), compute_ssim(), Vision Check — Image comparison for the Eyes visual testing harness.  Compares r, Compute Peak Signal-to-Noise Ratio between two images.      Returns float('inf'), Compute Structural Similarity Index between two images.      Uses scikit-image's, Compare a rendered frame against a golden reference.      Args:         rendered, verify_frame()

### Community 152 - "Deck.h"
Cohesion: 0.15
Nodes (19): Clip, GLuint, ProceduralSource, SourceParam, string, vector, applyCompTransform, compileShaderWithUtils (+11 more)

### Community 153 - "LinkSync.cpp"
Cohesion: 0.20
Nodes (8): condition_variable, Thread, AVCodecContext, AVFormatContext, AVFrame, AVPacket, AVStream, SwsContext

### Community 154 - "FFTProcessor.cpp"
Cohesion: 0.17
Nodes (12): Composition, FeatureBus, handleMultiplierButton, onBpmMultiplierChanged, onManualBpmChanged, onPause, onPlay, onQuantizeChanged (+4 more)

### Community 155 - "AdvancedAudioAnalyzer"
Cohesion: 0.31
Nodes (7): classify_vibe(), load_progress(), main(), Load progress from previous run., Save progress for resume., Analyze a rendered frame and classify into a vibe category.     Returns (vibe, s, save_progress()

### Community 156 - "PitchTracker.cpp"
Cohesion: 0.33
Nodes (8): load_preset(), main(), Load a MilkDrop preset via the test API., Capture a rendered frame., Score a rendered image on visual interest (0-100).      Criteria:     - Non-blac, render_frame(), reset(), score_image()

### Community 157 - "AudioEngine.h"
Cohesion: 0.25
Nodes (7): NSObject, SyphonOutputImpl, -initWithContextname, -publishTexturewidthheight, -setName, -shutdown, SyphonServer

### Community 158 - "AudioCallback.cpp"
Cohesion: 0.28
Nodes (5): AudioSignal, source_, Category, MappingSource, string

### Community 159 - "Section 9: Input / Output"
Cohesion: 0.25
Nodes (5): MouseEvent, Slider, ResettableSlider, defaultVal_, hasDefault_

### Community 160 - "Slice 05: Top Chrome Feature Audit"
Cohesion: 0.25
Nodes (5): Integration tests that connect to a live Audio-DNA instance., Read the real accessibility tree and verify basic structure., A running JUCE app should have at least one window child., Live tree serializes to JSON and parses back correctly., TestIntegration

### Community 161 - "SpectrumDisplay.cpp"
Cohesion: 0.70
Nodes (4): BPMTracker, feedBeatWithFeatures(), feedNonBeatHops(), lockBPM()

### Community 162 - "WaveformDisplay.cpp"
Cohesion: 0.32
Nodes (5): brightness(), psnr_between(), Time Sweep Verification — Tests that animated sources/effects change over time., Animated sources must produce different frames at different times., TestSourcesAnimateOverTime

### Community 163 - "Smoother.h"
Cohesion: 0.29
Nodes (6): /opt/homebrew/bin/codegraph, /Users/boriskarpman/.local/bin/clangd-mcp, /Users/boriskarpman/.local/share/uv/tools/graphifyy/bin/python3, clangd-rta, codegraph-rta, graphify-rta

### Community 164 - "Composition.h"
Cohesion: 0.38
Nodes (7): actionCallback_, captureCallback_, processKeyDown, processKeyUp, processMidiCC, processMidiNoteOff, processMidiNoteOn

### Community 165 - "test_integration_pipeline.cpp"
Cohesion: 0.38
Nodes (7): mouseDown, mouseDrag, mouseUp, normalizedToTimelineX, onCuepointJump, timelineXToNormalized, MouseEvent

### Community 166 - "test_downbeat_detector.cpp"
Cohesion: 0.48
Nodes (4): FileEntry, Graphics, vector, paint

### Community 167 - "test_effect"
Cohesion: 0.38
Nodes (6): compute_psnr(), main(), Render clean baseline with no effects., Enable effect with explicit default params, render, compare to baseline., render_baseline(), test_effect_visual()

### Community 171 - "MacroBank.h"
Cohesion: 0.20
Nodes (7): FeatureBus, Renderer, FeatureBus, Graphics, paint, SpectrumDisplay::SpectrumDisplay(), timerCallback

### Community 172 - "Clip.cpp"
Cohesion: 0.17
Nodes (9): DocumentWindow, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, String, unique_ptr, getApplicationName(), getApplicationVersion(), initialise(), MainWindow (+1 more)

### Community 173 - "test_bpm_stabilization.cpp"
Cohesion: 0.47
Nodes (6): paint, paintSectionHeader, paintTimeline, Graphics, Rectangle, String

### Community 174 - "test_spectral_features.cpp"
Cohesion: 0.50
Nodes (4): SpectralFeatures, vector, flatMagnitudeSpectrum(), sineMagnitudeSpectrum()

### Community 175 - "test_mapping_engine.cpp"
Cohesion: 0.33
Nodes (4): Tests for JSON output correctness., The walk result serializes to valid JSON., A nested tree round-trips through JSON correctly., TestJsonSerialization

### Community 176 - "MappingEngine.h"
Cohesion: 0.67
Nodes (3): ChangeBroadcaster, changeListenerCallback, onTransportStateChanged

### Community 178 - "PresetManager.h"
Cohesion: 0.11
Nodes (16): Autopilot, CompositorEngine, EffectChain, EffectLibrary, FullscreenQuad, MappingEngine, mutex, RoutingEngine (+8 more)

### Community 179 - "displaySnap_"
Cohesion: 0.40
Nodes (5): Graphics, Rectangle, String, paint, paintSectionHeader

### Community 180 - "UndoManager.h"
Cohesion: 0.22
Nodes (7): RouteTarget, clipId, effectIndex, layerId, paramIndex, scope, TargetScope

### Community 182 - "FeatureSnapshot"
Cohesion: 0.22
Nodes (3): atomic, Deck, Composition

### Community 184 - "SyphonInput.h"
Cohesion: 0.33
Nodes (6): MouseEvent, mouseDown, mouseDrag, mouseMove, mouseUp, resized

### Community 185 - "SyphonOutput.h"
Cohesion: 0.17
Nodes (13): GLuint, string, addParam, createFBO, deleteFBO, initGL, releaseGL, reset (+5 more)

### Community 188 - "SpoutOutput.h"
Cohesion: 0.19
Nodes (10): updateFeedbackBuffer, FullscreenQuad, draw, init, initialized_, release, vao_, vbo_ (+2 more)

### Community 189 - "CompositorEngine"
Cohesion: 0.67
Nodes (3): psnr(), Test if an effect with defaults produces visible change. Returns PSNR., test_effect()

### Community 190 - "LUTLoader.h"
Cohesion: 0.25
Nodes (3): ClipPositionSignal, currentPosition_, atomic

### Community 196 - "Layer.h"
Cohesion: 0.67
Nodes (4): EffectChain, FeatureBus, OutputRenderer::OutputRenderer(), OutputWindow::OutputWindow()

### Community 201 - "LinkSync.h"
Cohesion: 0.33
Nodes (5): AnalysisThread::AnalysisThread(), getPCMSamples, getWaveformSamples, run, RingBuffer

### Community 202 - "UniformBridge.h"
Cohesion: 0.67
Nodes (3): Event, vector, advancePlayback

### Community 210 - "ChainedSignal"
Cohesion: 0.33
Nodes (6): loadFile, onError, setSourceMode, stop, File, SourceMode

### Community 211 - "OscillatorSignal.h"
Cohesion: 0.40
Nodes (5): SourceDetails, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped

### Community 213 - "Section 4: Effects Library (135 effects)"
Cohesion: 0.40
Nodes (4): vector, generateSine(), bpm, runPipeline()

### Community 214 - "test_composition.cpp"
Cohesion: 0.67
Nodes (3): ParamWriter, SignalRegistry, processFrame

### Community 215 - "Slice 12 — Top-Level Docs Feature Audit"
Cohesion: 0.67
Nodes (3): getTransitionShaderName, MixMode, String

### Community 243 - "File"
Cohesion: 0.67
Nodes (3): BindingOverlay::BindingOverlay(), BindingManager, Composition

### Community 244 - "Image"
Cohesion: 0.67
Nodes (3): PreferencesDialog::Content::setActiveTab(), resized, Tab

### Community 253 - ".getComposition"
Cohesion: 0.50
Nodes (4): Graphics, Rectangle, drawSignalTriangle, paint

### Community 266 - "Layer::fromVar"
Cohesion: 0.67
Nodes (3): var, Layer::fromVar(), Layer::toVar()

## Knowledge Gaps
- **1492 isolated node(s):** `lookAndFeel_`, `ringBuffer_`, `audioEngine_`, `analysisThread_`, `openImageButton_` (+1487 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **100 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `MainComponent` connect `Main Component Controller` to `Composition Inspector`, `Route Target Scope`, `Main Component Header`, `ProjectM Preset Manager`, `Clip.cpp`, `Top Bar`, `Inspector Panel`, `Per-Phase Instructions`, `PresetManager.h`, `SyphonInput.h`, `Macro Panel`, `MIDI Learn Overlay`?**
  _High betweenness centrality (0.173) - this node is a cross-community bridge._
- **Why does `AnalysisThread` connect `ProjectM Preset Manager` to `Main Component Controller`, `Preset Manager`, `Record Panel`, `Signal Registry`, `LinkSync.h`, `MacroBank.h`, `Routing Engine`, `Per-Phase Instructions`, `FeatureSnapshot`, `AnalysisThread.cpp`?**
  _High betweenness centrality (0.084) - this node is a cross-community bridge._
- **Why does `Layer` connect `Layer Strip UI` to `Compositor Engine`, `Analysis Thread`, `Macro Panel`, `Layer::fromVar`, `BPM Tracker`, `Syphon Input`, `Preview Panel`, `FFT Processor`, `Deck View`, `Renderer Accessors`, `Autopilot Header`, `Output Window`, `Pitch Tracker`, `Composition Model`, `Effects Rack Panel`?**
  _High betweenness centrality (0.083) - this node is a cross-community bridge._
- **What connects `lookAndFeel_`, `ringBuffer_`, `audioEngine_` to the rest of the system?**
  _1644 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Main Component Controller` be split into smaller, more focused modules?**
  _Cohesion score 0.014598540145985401 - nodes in this community are weakly interconnected._
- **Should `Renderer Core` be split into smaller, more focused modules?**
  _Cohesion score 0.02 - nodes in this community are weakly interconnected._
- **Should `Compositor Engine` be split into smaller, more focused modules?**
  _Cohesion score 0.024684102262709375 - nodes in this community are weakly interconnected._