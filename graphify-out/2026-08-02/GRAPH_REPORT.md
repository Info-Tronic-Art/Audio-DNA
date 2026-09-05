# Graph Report - RealTimeAudio  (2026-08-02)

## Corpus Check
- 277 files · ~419,709 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 5104 nodes · 8233 edges · 280 communities (208 shown, 72 thin omitted)
- Extraction: 93% EXTRACTED · 7% INFERRED · 0% AMBIGUOUS · INFERRED: 591 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `be34bb2a`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- MainComponent
- Renderer
- Clip
- BPMTracker
- ClipInspector
- ApiServer
- Layer
- MilkDropBrowser
- AnalysisThread
- LayerInspector
- ProjectMPresetManager
- VideoRecorder
- MainComponent.cpp
- TestServer
- PreviewPanel
- LayerStrip
- TopBar
- InspectorPanel
- MappingEditor
- SourcesBrowser
- Composition
- EffectStackView
- CompositorEngine
- CompositionInspector
- UniversalParamControl
- MidiLearnOverlay
- AudioEngine
- AudioDNALookAndFeel
- FeatureSnapshot
- Renderer.cpp
- SignalInspector
- ._post
- ProceduralSource
- SwapClipsCmd
- AdvancedAudioAnalyzer
- RecordPanel
- AudioReadoutPanel
- SessionRecorder
- FilesBrowser
- TriggerClipCmd
- EffectSlot
- OutputRenderer
- EffectsRackPanel
- ProjectMSource
- string
- TextureManager
- ClearActiveClipCmd
- BindingOverlay
- GenreDetector
- MidiOutputHandler
- BrowserPanel
- DeckView
- PipelineRunner
- PresetSelector
- EffectChain
- test_render_pipeline.py
- TestSignalRouteEndToEnd
- Effect
- DeckCommands.h
- Binding
- MainComponent.h
- Deck
- EnvelopeSignal
- OscHandler
- MappingEngine
- ax_inspector.py
- CompDecksBrowser
- PresetManager
- MacroBank
- MidiHandler
- CurveTransforms.h
- atomic
- StructuralDetector
- SwitchDeckCmd
- UndoManager
- test_fractals.py
- RingBuffer
- WaveformDisplay
- CompositionInspector.cpp
- ClipCell
- SignalStrip
- UniversalParamControl.cpp
- MFCCExtractor
- Autopilot
- Signal
- Command
- SpectralFeatures
- File
- PreferencesDialog.cpp
- FXBrowser
- MacroPanel
- ShaderManager
- .runFenced
- EffectStackCmd
- SignalRegistry
- TimingWindow
- Knob
- SpectrumDisplay
- BindingManager
- ThumbnailCache
- KeyDetector
- EffectLibrary
- SignalBar
- AudioDNAMenuBar
- RoutingEngine
- BPMTracker.cpp
- UndoService
- FeedbackProcessor
- LinkSync
- MilkDropBrowser.cpp
- FFTProcessor
- Colour
- Route
- ClipInspector.cpp
- MilkDropBrowser::PresetListContent
- SourceRegistry
- LoudnessAnalyzer
- ClipCell.cpp
- FilesBrowser.cpp
- LayerInspector.cpp
- _make_mock_element
- OscillatorSignal
- test_effects.py
- test_range_quality.py
- ChromaExtractor
- SyphonOutput
- rebuildGrid
- fileListContent_
- FeedbackConfig
- DeckView.cpp
- LayerStrip.cpp
- string
- FeatureBus
- TestCLI
- test_sources.py
- AddDeckCmd
- Main.cpp
- FullscreenQuad
- FXBrowser::FXListContent
- OnsetDetector
- PitchTracker
- OutputWindow
- ISFShaderLoader.cpp
- EffectScope
- MappingEditor.cpp
- CompositeCommand
- AnalysisThread.h
- AudioEngine.cpp
- Smoother
- FXBrowser.cpp
- TopBar::TopBar
- test_ax_inspector.py
- ToggleLayerFlagCmd
- VJAppController
- itemDropped
- LayerStrip::LayerStrip
- TopBar.cpp
- test_audio_reactivity.py
- fromVar
- BrowserPanel.cpp
- WaveformSeqlock
- conftest.py
- TestAppLookup
- Time
- vision_check.py
- fence_
- ResettableSlider
- RouteTarget
- ClipPositionSignal
- EffectsRackPanel.cpp
- scan_all_presets.py
- test_milkdrop.py
- AudioCallback
- AudioSignal
- MoveLayerCmd
- RecordPanel.cpp
- TestIntegration
- test_time_sweep.py
- .mcp.json
- OutputWindow.cpp
- File
- .paintGrid
- final_default_validation.py
- .fromVar
- Renderer.h
- paintSectionHeader
- test_thumbnail_cache.cpp
- TestJsonSerialization
- test_signals.py
- getThumbnailBounds
- paintSectionHeader
- scrubPlayhead
- ClearLayerClipsCmd
- TestSourceRegistry
- SetColumnCountCmd
- TestFrameCapture
- isInThumbnailArea
- AnalysisThread.cpp
- .getActiveClip
- TestEffects
- StringArray
- TestSignalRegistrySpec
- MouseEvent
- paintSectionHeader
- OutputRenderer::OutputRenderer
- handlePresetClick
- test_downbeat_detector.cpp
- setComposition
- .getActiveDeck
- .setPerTypeAutopilotConfig
- .setEffectFenceHook
- .setEffectPerformEdit
- test_bpm_stabilization.cpp
- setupDeckTabs
- .setEffectFenceHook
- .setEffectPerformEdit
- changeListenerCallback
- MainComponent::keyPressed
- TopBar.h
- MainComponent::handleBindingAction
- MainComponent::makeDeckResolver
- MainComponent::makeClipMediaHook
- MainComponent::refreshPreviewFromActiveClip
- .getMenuBarModel
- .getComposition
- .setEffectFenceHook
- .setEffectPerformEdit
- attachTo
- queueCameraFrame
- .inject_features
- .add_route
- .set_macro
- .stop
- .load_image
- ChromaExtractor
- FFTProcessor
- KeyDetector
- LoudnessAnalyzer
- MFCCExtractor
- OnsetDetector
- PitchTracker
- uint32_t
- BindingManager
- SessionRecorder
- BindableTarget
- ClipDeckResolver
- ClipLayerResolver
- ClipMediaHook
- CompositionResolver
- DeckActivateHook
- DeckFenceHook
- MouseEvent
- optional
- MixMode
- PerTypeAutopilotConfig
- FeatureBus
- Colour
- DocumentWindow
- ComboBox
- Label
- Rectangle
- TextEditor
- ToggleButton
- StructuralDetector
- BPMTracker
- MainComponent::makeLayerResolver
- MainComponent::makeDeckFence
- MainComponent::paint
- paint
- setEffectPerformEdit
- setSignalRegistry
- mouseDown
- OpenGLShaderProgram
- Graphics
- Layer
- atomic
- EffectLibrary
- EffectChain

## God Nodes (most connected - your core abstractions)
1. `MainComponent` - 206 edges
2. `Renderer` - 176 edges
3. `Clip` - 122 edges
4. `ClipInspector` - 108 edges
5. `Layer` - 104 edges
6. `MilkDropBrowser` - 98 edges
7. `BPMTracker` - 92 edges
8. `LayerInspector` - 91 edges
9. `LayerStrip` - 82 edges
10. `AnalysisThread` - 78 edges

## Surprising Connections (you probably didn't know these)
- `PipelineRunner` --references--> `FFTProcessor`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/AnalysisThread.h
- `PipelineRunner` --references--> `SpectralFeatures`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/AnalysisThread.h
- `PipelineRunner` --references--> `OnsetDetector`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/AnalysisThread.h
- `PipelineRunner` --references--> `BPMTracker`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/AnalysisThread.h
- `PipelineRunner` --references--> `MFCCExtractor`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/AnalysisThread.h

## Import Cycles
- None detected.

## Communities (280 total, 72 thin omitted)

### Community 0 - "MainComponent"
Cohesion: 0.02
Nodes (115): CameraDevice::Listener, DragAndDropContainer, FileChooser, PresetSlot, ComboBox, Component, File, FileDragAndDropTarget (+107 more)

### Community 1 - "Renderer"
Cohesion: 0.02
Nodes (97): atomic, Composition, promise, SignalRegistry, File, function, GLuint, Image (+89 more)

### Community 2 - "Clip"
Cohesion: 0.02
Nodes (76): AlphaType, BeatSnapMode, BlendOverride, LoopMode, MediaType, PlaylistCycleMode, PlaylistTrigger, Clip (+68 more)

### Community 3 - "BPMTracker"
Cohesion: 0.03
Nodes (58): aubio_tempo_t, BPMTracker, beatCounter_, beatScorePos_, beatScores_, cachedBassEnergy_, cachedHarmonicChange_, cachedSpectralFlux_ (+50 more)

### Community 4 - "ClipInspector"
Cohesion: 0.03
Nodes (71): DragTarget, ClipInspector, anchorControl_, autopilotActionSelector_, autopilotDurationSelector_, beatDivisionLabel_, beatDivisionSelector_, beatSnapSelector_ (+63 more)

### Community 5 - "ApiServer"
Cohesion: 0.09
Nodes (64): ApiServer, allowFeatureInjection_, handleComposition, handleGetBpm, handleGetFeatures, handleGetSyphon, handleHealth, handleInjectFeatures (+56 more)

### Community 6 - "Layer"
Cohesion: 0.03
Nodes (60): AutoSizeMode, KeyingMode, AutopilotAction, AutopilotDuration, EffectSlot, MixMode, optional, Type (+52 more)

### Community 7 - "MilkDropBrowser"
Cohesion: 0.03
Nodes (63): PresetListContent, ComboBox, Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, PlayMode, ProjectMPresetManager (+55 more)

### Community 8 - "AnalysisThread"
Cohesion: 0.04
Nodes (45): AnalysisThread, advancedAnalyzer_, analysisBuffer_, bpmTracker_, chromaExtractor_, cpuLoad_, currentPeak_, currentRMS_ (+37 more)

### Community 9 - "LayerInspector"
Cohesion: 0.03
Nodes (61): ComboBox, Component, DragAndDropTarget, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, SignalRegistry, TextButton (+53 more)

### Community 10 - "ProjectMPresetManager"
Cohesion: 0.07
Nodes (46): id, PresetInfo, string, vector, function, PresetInfo, string, vector (+38 more)

### Community 11 - "VideoRecorder"
Cohesion: 0.06
Nodes (31): PixelBuffer, array, atomic, Config, File, function, mutex, thread (+23 more)

### Community 12 - "MainComponent.cpp"
Cohesion: 0.05
Nodes (40): CompositionResolver, MouseEvent, Deck, Image, MainComponent::advanceSlideshow(), MainComponent::beatSyncRandomize(), MainComponent::closeOutput(), MainComponent::enterKeyboardBindingMode() (+32 more)

### Community 13 - "TestServer"
Cohesion: 0.31
Nodes (24): Request, Response, string, handleAddRoute, handleHealth, handleInjectFeatures, handleListRoutes, handleListSignals (+16 more)

### Community 14 - "PreviewPanel"
Cohesion: 0.09
Nodes (27): GLHost, FeatureBus, File, Graphics, Image, Tab, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (+19 more)

### Community 15 - "LayerStrip"
Cohesion: 0.04
Nodes (47): ComboBox, Component, DragAndDropTarget, function, Image, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Rectangle, String (+39 more)

### Community 16 - "TopBar"
Cohesion: 0.04
Nodes (52): ComboBox, Label, Rectangle, ResettableSlider, array, Component, Composition, FeatureBus (+44 more)

### Community 17 - "InspectorPanel"
Cohesion: 0.08
Nodes (24): Component, Composition, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Tab, TextButton, Viewport, InspectorPanel, activeTab_ (+16 more)

### Community 18 - "MappingEditor"
Cohesion: 0.06
Nodes (30): ListenerList, ComboBox, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, Listener, TextButton, ToggleButton (+22 more)

### Community 19 - "SourcesBrowser"
Cohesion: 0.06
Nodes (40): SourceEntry, SourceListContent, Component, Graphics, MouseEvent, String, CategoryInfo, Component (+32 more)

### Community 20 - "Composition"
Cohesion: 0.04
Nodes (47): AutopilotDirection, AutopilotDurationMode, CrossfaderBehaviour, CrossfaderBlendMode, CrossfaderCurve, QuantizeMode, Composition, activeDeckIndex (+39 more)

### Community 21 - "EffectStackView"
Cohesion: 0.07
Nodes (45): EffectRow, EffectSlot, Graphics, MouseEvent, SourceDetails, vector, EffectStackView, effectLibrary_ (+37 more)

### Community 22 - "CompositorEngine"
Cohesion: 0.05
Nodes (80): EffectSlot, MixMode, SourceRenderFn, CompositorEngine, accumulatorFBO_, accumulatorTex_, applyClipEffects, applyClipTransform (+72 more)

### Community 23 - "CompositionInspector"
Cohesion: 0.05
Nodes (40): CompositionInspector, anchorControl_, apClipLoopsSlider_, apDurationSelector_, apForwardBtn_, apLoopToggle_, apMasterLayerSelector_, apOffBtn_ (+32 more)

### Community 24 - "UniversalParamControl"
Cohesion: 0.05
Nodes (34): Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, SignalRegistry, SourceMode, String, TextButton (+26 more)

### Community 25 - "MidiLearnOverlay"
Cohesion: 0.06
Nodes (44): AudioDeviceManager, BindableTarget, BindingManager, Component, Composition, Graphics, KeyPress, MidiInput (+36 more)

### Community 26 - "AudioEngine"
Cohesion: 0.09
Nodes (21): AudioFormatManager, AudioFormatReaderSource, AudioSourcePlayer, AudioTransportSource, ChangeListener, CombinedCallback, AudioEngine, audioCallback_ (+13 more)

### Community 27 - "AudioDNALookAndFeel"
Cohesion: 0.09
Nodes (40): Drawable, Font, ScrollBar, AudioDNALookAndFeel, AudioDNALookAndFeel::AudioDNALookAndFeel(), drawButtonBackground, drawButtonText, drawComboBox (+32 more)

### Community 28 - "FeatureSnapshot"
Cohesion: 0.05
Nodes (42): FeatureSnapshot, bandEnergies, barCount, barPhase, beatInBar, beatPhase, bpm, chromagram (+34 more)

### Community 29 - "Renderer.cpp"
Cohesion: 0.06
Nodes (51): AnalysisThread, Clip, Component, FeatureBus, File, GLuint, Image, ImageSequence (+43 more)

### Community 30 - "SignalInspector"
Cohesion: 0.07
Nodes (39): Graphics, Rectangle, String, ComboBox, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Rectangle, ToggleButton (+31 more)

### Community 31 - "._post"
Cohesion: 0.12
Nodes (8): Configure the entire effect chain.          Disables all existing effects, then, Render a single frame and save it to disk.          Args:             output_pat, Reset all effects, clear images, restore defaults., Load a procedural source into the active clip.          Args:             source, Update parameters on the currently active source.          Args:             par, Remove a signal route by ID., Send a POST request with JSON body., Enable/disable an effect and optionally set parameters.          Args:

### Community 32 - "ProceduralSource"
Cohesion: 0.07
Nodes (36): Param, GLuint, OpenGLShaderProgram, string, GLuint, string, vector, ProceduralSource (+28 more)

### Community 33 - "SwapClipsCmd"
Cohesion: 0.07
Nodes (30): ClipDeckResolver, ClipLayerResolver, ClipMediaHook, DeckFenceHook, function, string, SetClipCmd, after_ (+22 more)

### Community 34 - "AdvancedAudioAnalyzer"
Cohesion: 0.05
Nodes (31): AdvancedAudioAnalyzer, bassHistory_, envelopeFull_, envelopePos_, fftSize_, formantBinHigh_, formantBinLow_, formantMax_ (+23 more)

### Community 35 - "RecordPanel"
Cohesion: 0.08
Nodes (25): ComboBox, Component, File, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, TextButton, RecordPanel (+17 more)

### Community 36 - "AudioReadoutPanel"
Cohesion: 0.14
Nodes (37): Colour, AudioReadoutPanel, AudioReadoutPanel::AudioReadoutPanel(), displayBands_, displaySnap_, downbeatFlash_, drawBandMeters, drawBarIndicator (+29 more)

### Community 37 - "SessionRecorder"
Cohesion: 0.09
Nodes (34): CriticalSection, Event, File, string, vector, atomic, Event, vector (+26 more)

### Community 38 - "FilesBrowser"
Cohesion: 0.06
Nodes (35): FileListContent, FilesBrowser, currentDir_, decodeGeneration_, entries_, favorites_, gridView_, gridViewBtn_ (+27 more)

### Community 39 - "TriggerClipCmd"
Cohesion: 0.15
Nodes (12): ClipLayerResolver, optional, string, TriggerClipCmd, after_, before_, column_, deckIndex_ (+4 more)

### Community 41 - "OutputRenderer"
Cohesion: 0.05
Nodes (51): Display, DocumentWindow, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, OutputComponent, Component, FeatureBus, File, Image (+43 more)

### Community 42 - "EffectsRackPanel"
Cohesion: 0.08
Nodes (25): CategoryHeader, EffectSection, Graphics, EffectsRackPanel, activeMappingEditor_, categoryHeaders_, contentComponent_, editingEffectIndex_ (+17 more)

### Community 43 - "ProjectMSource"
Cohesion: 0.10
Nodes (32): GLState, GLuint, string, mutex, string, vector, ProjectMSource, applyParams (+24 more)

### Community 44 - "string"
Cohesion: 0.10
Nodes (26): AddLayerCmd, added_, addedIndex_, deckIndex_, deckResolver_, fence_, ClipDeckResolver, ClipMediaHook (+18 more)

### Community 45 - "TextureManager"
Cohesion: 0.08
Nodes (24): File, GLuint, LUTLoader, loadCubeFile, releaseLUT, File, GLuint, Image (+16 more)

### Community 46 - "ClearActiveClipCmd"
Cohesion: 0.10
Nodes (14): Flag, ClearActiveClipCmd, after_, before_, deckIndex_, layerIndex_, resolver_, ClipLayerResolver (+6 more)

### Community 47 - "BindingOverlay"
Cohesion: 0.07
Nodes (36): BindingOverlay, active_, BindingOverlay::BindingOverlay(), enterBindingMode, exitBindingMode, findExistingBinding, getKeyDescription, hitTestTarget (+28 more)

### Community 48 - "GenreDetector"
Cohesion: 0.08
Nodes (28): Features, GenreDetector, candidateGenre_, chromaticComplexity, classify, computeEnergyState, computeScores, confidence_ (+20 more)

### Community 49 - "MidiOutputHandler"
Cohesion: 0.09
Nodes (30): MidiOutput, Array, Deck, MidiDeviceInfo, MidiMessage, PadState, String, array (+22 more)

### Community 50 - "BrowserPanel"
Cohesion: 0.11
Nodes (19): BrowserPanel, activeTab_, compDecksBrowser_, compDecksTabBtn_, filesBrowser_, filesTabBtn_, fxBrowser_, fxTabBtn_ (+11 more)

### Community 51 - "DeckView"
Cohesion: 0.06
Nodes (29): CellPos, DeckView, activeColumn_, clipCells_, columnTriggers_, composition_, deckTabs_, gridContent_ (+21 more)

### Community 52 - "PipelineRunner"
Cohesion: 0.08
Nodes (25): kBlockSize, array, kOnsetWindowSize, vector, generateSine(), PipelineRunner, analysisBuffer, bpm (+17 more)

### Community 53 - "PresetSelector"
Cohesion: 0.08
Nodes (19): deque, mutex, function, string, PresetSelector, barsSinceLastSwitch_, enabled_, energyMatching_ (+11 more)

### Community 54 - "EffectChain"
Cohesion: 0.12
Nodes (31): FeatureSnapshot, OpenGLShaderProgram, Effect, FullscreenQuad, GLint, GLuint, ShaderManager, TextureManager (+23 more)

### Community 55 - "test_render_pipeline.py"
Cohesion: 0.11
Nodes (13): Eyes Visual Tests — Render Pipeline  Tests the core rendering pipeline: image lo, Verify audio feature injection works., Injecting features should succeed., Verify state endpoint works., State endpoint should list all effects., Verify reset clears state properly., After reset, no effects should be enabled., Verify the test server is responsive. (+5 more)

### Community 56 - "TestSignalRouteEndToEnd"
Cohesion: 0.17
Nodes (9): End-to-end: create route from audio signal to effect parameter,     inject featu, Check if signal API endpoints are available., Route Volume signal → Ripple intensity → verify RMS changes ripple., Route with threshold=0.5 should only activate above 0.5 RMS., Inverted route: high RMS should DECREASE the parameter., Route with outputMin=0.2, outputMax=0.6 should clamp output., Macro endpoint responds (full MacroBank integration is pending)., Oscillator signal should be listed in the signal registry. (+1 more)

### Community 57 - "Effect"
Cohesion: 0.08
Nodes (21): String, Effect, addParam, category_, dryWet_, Effect::Effect(), enabled_, name_ (+13 more)

### Community 58 - "DeckCommands.h"
Cohesion: 0.10
Nodes (25): optional, File, needsVideoReopen(), clipsEq(), compResolverFor(), ClipMediaHook, Composition, CompositionResolver (+17 more)

### Community 59 - "Binding"
Cohesion: 0.07
Nodes (28): Action, CCMode, InputType, Binding, action, ccMode, ccStepSize, enabled (+20 more)

### Community 60 - "MainComponent.h"
Cohesion: 0.11
Nodes (8): atomic, vector, Deck, BindingManager, Composition, Deck, Composition, set

### Community 61 - "Deck"
Cohesion: 0.11
Nodes (11): Deck, id, kDefaultColumns, kDefaultLayers, layers, name, nextLayerId_, numColumns (+3 more)

### Community 62 - "EnvelopeSignal"
Cohesion: 0.08
Nodes (12): ControlPoint, CurveType, EnvelopeSignal, amplitude_, beatDuration_, curveType_, looping_, oneShot_ (+4 more)

### Community 63 - "OscHandler"
Cohesion: 0.12
Nodes (23): OSCMessage, OSCReceiver, OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>, atomic, function, OscHandler, listening_, onSetBpm (+15 more)

### Community 64 - "MappingEngine"
Cohesion: 0.11
Nodes (17): EffectChain, MappingCurve, MappingSource, vector, MappingEngine, addMapping, applyCurve, clearAll (+9 more)

### Community 65 - "ax_inspector.py"
Cohesion: 0.13
Nodes (25): Any, Exception, AccessibilityPermissionError, AppNotFoundError, AXInspectorError, check_accessibility_permissions(), find_pid_by_name(), _get_ax_attribute() (+17 more)

### Community 66 - "CompDecksBrowser"
Cohesion: 0.06
Nodes (44): CompDeckListContent, CompDecksBrowser, CompDecksBrowser::CompDeckListContent, kEntryRowHeight, kSectionHeaderHeight, CompDecksBrowser::~CompDecksBrowser(), composition_, compositions_ (+36 more)

### Community 67 - "PresetManager"
Cohesion: 0.23
Nodes (20): DeckState, Array, EffectChain, File, MappingCurve, MappingSource, String, PresetManager (+12 more)

### Community 68 - "MacroBank"
Cohesion: 0.14
Nodes (10): Macro, Scope, array, SignalRegistry, MacroBank, kNumMacros, macros_, scope_ (+2 more)

### Community 69 - "MidiHandler"
Cohesion: 0.12
Nodes (22): Array, AudioDeviceManager, BindingManager, MidiDeviceInfo, MidiInput, MidiMessage, String, AudioDeviceManager (+14 more)

### Community 70 - "CurveTransforms.h"
Cohesion: 0.16
Nodes (25): applyCurve(), backIn(), backInOut(), backOut(), bounceIn(), bounceInOut(), bounceOut(), circularIn() (+17 more)

### Community 71 - "atomic"
Cohesion: 0.15
Nodes (7): SpectralFeatures, Array, SignalRegistry, RoutingEngine, vector, flatMagnitudeSpectrum(), sineMagnitudeSpectrum()

### Community 72 - "StructuralDetector"
Cohesion: 0.09
Nodes (21): StructuralDetector, candidateState_, classifyState, confirmedState_, fluxAlpha_, fluxEnv_, holdCounter_, holdThreshold_ (+13 more)

### Community 73 - "SwitchDeckCmd"
Cohesion: 0.10
Nodes (14): Composition, CompositionResolver, DeckActivateHook, SetColumnCountCmd, after_, before_, deckIndex_, deckResolver_ (+6 more)

### Community 74 - "UndoManager"
Cohesion: 0.15
Nodes (20): string, unique_ptr, function, unique_ptr, vector, UndoManager, canRedo, canUndo (+12 more)

### Community 75 - "test_fractals.py"
Cohesion: 0.11
Nodes (17): frame_is_not_black(), frames_are_different(), Eyes Visual Tests — Comprehensive Fractal Source Validation  Tests EVERY control, Every fractal must render a visible frame at defaults., Every parameter must produce a visible change when modified., Zoom at ALL positions (0, 0.25, 0.5, 0.75, 1.0) must be non-black., Dive speed at various levels must not go black., Power at all positions must not go black. (+9 more)

### Community 76 - "RingBuffer"
Cohesion: 0.18
Nodes (7): atomic, T, RingBuffer, buffer_, mask_, readPos_, writePos_

### Community 77 - "WaveformDisplay"
Cohesion: 0.09
Nodes (22): Column, kWaveformBufferSize, Graphics, array, Component, kMaxColumns, Timer, WaveformDisplay (+14 more)

### Community 78 - "CompositionInspector.cpp"
Cohesion: 0.18
Nodes (14): CompositionInspector::CompositionInspector(), getPreferredHeight, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped, refresh, resized (+6 more)

### Community 79 - "ClipCell"
Cohesion: 0.06
Nodes (54): ClipCell, active_, clip_, column_, dragHover_, fileDragEnter, fileDragExit, filesDropped (+46 more)

### Community 80 - "SignalStrip"
Cohesion: 0.10
Nodes (34): Colour, DisplaySize, Graphics, MouseEvent, Rectangle, SignalRegistry, String, Component (+26 more)

### Community 81 - "UniversalParamControl.cpp"
Cohesion: 0.15
Nodes (20): MouseEvent, PopupMenu, String, buildSourcePickerMenu, getPreferredHeight, handleSourcePickerResult, mouseDown, onExpandToggled (+12 more)

### Community 82 - "MFCCExtractor"
Cohesion: 0.12
Nodes (20): MelFilter, array, MFCCExtractor, buildDCTMatrix, buildFilterbank, dctMatrix_, fftSize_, filterWeightOffsets_ (+12 more)

### Community 83 - "Autopilot"
Cohesion: 0.14
Nodes (18): Autopilot, advanceClip, getActionForClip, getBeatsForClip, getPerTypeAction, getPerTypeBeats, lastBeatPhase_, lastEnergyState_ (+10 more)

### Community 84 - "Signal"
Cohesion: 0.15
Nodes (10): Category, string, Type, Signal, category_, getValue, id_, name_ (+2 more)

### Community 85 - "Command"
Cohesion: 0.06
Nodes (17): Command, description, execute, undo, string, vector, LogCmd, id (+9 more)

### Community 86 - "SpectralFeatures"
Cohesion: 0.10
Nodes (15): BandRange, array, SpectralFeatures, bandMaxEnergy_, bandRanges_, computeBandBinRanges, fftSize_, fluxMax_ (+7 more)

### Community 87 - "File"
Cohesion: 0.13
Nodes (21): BindableTarget, CellEdit, Command, Layer, optional, Clip, File, String (+13 more)

### Community 88 - "PreferencesDialog.cpp"
Cohesion: 0.12
Nodes (15): DialogWindow, Component, function, Graphics, Rectangle, Tab, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, PreferencesDialog (+7 more)

### Community 89 - "FXBrowser"
Cohesion: 0.08
Nodes (30): EffectEntry, FXListContent, Graphics, FXBrowser, buildCategoryList, categories_, effectLibrary_, effects_ (+22 more)

### Community 90 - "MacroPanel"
Cohesion: 0.13
Nodes (20): MacroSlot, Graphics, array, Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, SignalRegistry, MacroPanel (+12 more)

### Community 91 - "ShaderManager"
Cohesion: 0.14
Nodes (20): ProgramEntry, File, GLint, OpenGLContext, OpenGLShaderProgram, String, File, OpenGLContext (+12 more)

### Community 92 - ".runFenced"
Cohesion: 0.16
Nodes (6): RemoveDeckCmd, compResolver_, deckIndex_, priorActiveIndex_, removed_, mediaHook_

### Community 93 - "EffectStackCmd"
Cohesion: 0.19
Nodes (15): EffectStackCmd, after_, before_, compResolver_, fence_, refresh_, scope_, Composition (+7 more)

### Community 94 - "SignalRegistry"
Cohesion: 0.13
Nodes (19): Category, string, unique_ptr, vector, unique_ptr, vector, SignalRegistry, addSignal (+11 more)

### Community 95 - "TimingWindow"
Cohesion: 0.11
Nodes (18): setActive, updateTabButtonColors, Graphics, Tab, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Tab, TextButton (+10 more)

### Community 96 - "Knob"
Cohesion: 0.12
Nodes (18): Graphics, String, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, String, Knob, kPreferredHeight (+10 more)

### Community 97 - "SpectrumDisplay"
Cohesion: 0.09
Nodes (21): array, FeatureBus, Graphics, array, Component, FeatureBus, Timer, uint32 (+13 more)

### Community 98 - "BindingManager"
Cohesion: 0.14
Nodes (15): ActionCallback, BindingCaptureCallback, BindingManager, actionCallback_, bindingMode_, bindings_, captureCallback_, nextId_ (+7 more)

### Community 99 - "ThumbnailCache"
Cohesion: 0.18
Nodes (12): Entry, iterator, Key, KeyHash, File, Image, unordered_map, ThumbnailCache (+4 more)

### Community 100 - "KeyDetector"
Cohesion: 0.11
Nodes (15): array, KeyDetector, candidateCount_, candidateKey_, candidateMajor_, kHysteresisFrames, majorProfile_, minorProfile_ (+7 more)

### Community 101 - "EffectLibrary"
Cohesion: 0.16
Nodes (15): EffectDef, String, StringArray, unique_ptr, EffectLibrary, createEffect, defs_, getEffectDef (+7 more)

### Community 102 - "SignalBar"
Cohesion: 0.09
Nodes (33): DisplaySize, FeatureBus, Graphics, SignalRegistry, Component, DisplaySize, FeatureBus, function (+25 more)

### Community 103 - "AudioDNAMenuBar"
Cohesion: 0.14
Nodes (17): pair, AudioDNAMenuBar, getMenuBarNames, getMenuForIndex, getRedoState, getUndoState, hasClipSelection, isSyphonOutputEnabled (+9 more)

### Community 104 - "RoutingEngine"
Cohesion: 0.14
Nodes (17): ParamWriter, SignalRegistry, vector, vector, RoutingEngine, addRoute, clearAll, getRoute (+9 more)

### Community 105 - "BPMTracker.cpp"
Cohesion: 0.16
Nodes (18): analyzeDownbeatPosition, correctOctaveError, feedDownbeatFeatures, feedSilenceDetection, foldBPMToRange, process, processRawBPM, pushAndMedian (+10 more)

### Community 106 - "UndoService"
Cohesion: 0.12
Nodes (18): DeckView, function, DeckView, Composition, Deck, Renderer, UndoService, composition_ (+10 more)

### Community 107 - "FeedbackProcessor"
Cohesion: 0.14
Nodes (15): GLuint, string, FeedbackProcessor, applyPreset, currentBuffer_, ensureSize, fbos_, height_ (+7 more)

### Community 108 - "LinkSync"
Cohesion: 0.13
Nodes (11): atomic, LinkSync, beatPhase_, bpm_, enabled_, numPeers_, quantum_, requestBeatAtTime (+3 more)

### Community 109 - "MilkDropBrowser.cpp"
Cohesion: 0.17
Nodes (19): Colour, PlayMode, ProjectMPresetManager, SubTab, getPlaylistBlendSeconds, getPlaylistCycleModeId, getPlaylistTriggerBeats, initSections (+11 more)

### Community 110 - "FFTProcessor"
Cohesion: 0.12
Nodes (13): FFT, FFTProcessor, buildHannWindow, fft_, fftData_, FFTProcessor::FFTProcessor(), hannWindow_, kFFTOrder (+5 more)

### Community 111 - "Colour"
Cohesion: 0.19
Nodes (13): SliderLayout, Button, Colour, Graphics, Slider, SliderStyle, FadeSpeedSliderLookAndFeel, FullBoundsSliderLAF (+5 more)

### Community 112 - "Route"
Cohesion: 0.11
Nodes (19): SourceType, Route, dialRangeMax, dialRangeMin, enabled, falloff, gain, id (+11 more)

### Community 113 - "ClipInspector.cpp"
Cohesion: 0.18
Nodes (16): getPreferredHeight, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped, mouseDown, mouseDrag, mouseUp (+8 more)

### Community 114 - "MilkDropBrowser::PresetListContent"
Cohesion: 0.26
Nodes (5): Component, Graphics, PresetInfo, Section, MilkDropBrowser::PresetListContent

### Community 115 - "SourceRegistry"
Cohesion: 0.17
Nodes (18): SourceFactory, SourceInfo, SourceRegistry, string, unique_ptr, vector, string, unordered_map (+10 more)

### Community 116 - "LoudnessAnalyzer"
Cohesion: 0.13
Nodes (12): BiquadState, BiquadState, LoudnessAnalyzer, fillCount_, process, processBiquad, runningSum_, stage1_ (+4 more)

### Community 117 - "ClipCell.cpp"
Cohesion: 0.14
Nodes (17): FeatureSnapshot, Writer, FeatureBus, createWriter, kMaxReadAttempts, kOddSeqSpinLimit, kSnapshotWords, loadStableSeq (+9 more)

### Community 118 - "FilesBrowser.cpp"
Cohesion: 0.27
Nodes (16): File, Image, Time, FilesBrowser::~FilesBrowser(), filterBySearch, generateThumbnail, isMediaFile, loadFavorites (+8 more)

### Community 119 - "LayerInspector.cpp"
Cohesion: 0.16
Nodes (18): SignalRegistry, SourceDetails, getPreferredHeight, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped, LayerInspector::LayerInspector() (+10 more)

### Community 120 - "_make_mock_element"
Cohesion: 0.14
Nodes (12): _make_mock_element(), Tests for the recursive tree walker., Walk a leaf element with no children., Walk a tree with nested children., Walk an element with no attributes set., Tests for the --depth recursion limiter., Depth 0 returns the root element without walking children., Depth 1 walks immediate children but not grandchildren. (+4 more)

### Community 121 - "OscillatorSignal"
Cohesion: 0.13
Nodes (7): string, OscillatorSignal, amplitude_, beatDuration_, phaseOffset_, shape_, WaveShape

### Community 122 - "test_effects.py"
Cohesion: 0.14
Nodes (13): all_effects(), brightness(), image_loaded(), psnr_between(), Auto-Discovering Effect Verification — Tests ALL registered effects.  Queries /a, No effect should turn the image completely black or white., Discover all effects from the running app., Ensure test image is loaded. (+5 more)

### Community 123 - "test_range_quality.py"
Cohesion: 0.17
Nodes (14): analyze_sweep(), brightness(), Tier 2: Range Quality Analysis  For each source parameter, renders at 11 positio, Sweep every parameter and verify quality metrics., Each parameter must have >70% useful range, no discontinuities, no dead zones., Auto-discover all sources and sweep all their params.      This class discovers, Sweep every param on every source. Generates CSV reports., Render a source at multiple parameter positions, return list of (value, path) tu (+6 more)

### Community 124 - "ChromaExtractor"
Cohesion: 0.15
Nodes (12): ChromaExtractor, binToChroma_, ChromaExtractor::ChromaExtractor(), computeBinToChromaMap, fftSize_, hasPrevFrame_, kNumChroma, numBins_ (+4 more)

### Community 125 - "SyphonOutput"
Cohesion: 0.09
Nodes (18): NSObject, atomic, string, SyphonOutput, enabled_, impl_, init, initialized_ (+10 more)

### Community 126 - "rebuildGrid"
Cohesion: 0.11
Nodes (18): Composition, onClipMoved, onClipSelected, onClipTriggered, onFileDropped, onLayerBypass, onLayerClearClip, onLayerEffectDropped (+10 more)

### Community 127 - "fileListContent_"
Cohesion: 0.16
Nodes (10): Component, MouseEvent, Point, fileListContent_, dragStarted_, kLabelHeight, kListRowHeight, kPadding (+2 more)

### Community 128 - "FeedbackConfig"
Cohesion: 0.12
Nodes (15): FeedbackConfig, amount, enabled, lumaKey, offsetX, offsetY, presetName, rotation (+7 more)

### Community 129 - "DeckView.cpp"
Cohesion: 0.16
Nodes (13): Graphics, clearSelection, getNaturalHeight, layoutGrid, onColumnTriggered, paint, refresh, resized (+5 more)

### Community 130 - "LayerStrip.cpp"
Cohesion: 0.21
Nodes (14): SourceDetails, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped, populateBlendDropdown, populateTransitionDropdown, refresh (+6 more)

### Community 131 - "string"
Cohesion: 0.18
Nodes (14): MouseEvent, string, vector, addToRecent, buildPlaylistDragDescription, calculateContentHeight, firePresetSelected, getCuratedPresets (+6 more)

### Community 132 - "FeatureBus"
Cohesion: 0.29
Nodes (5): SignalStrip, FeatureBus, vector, Composition, SignalRegistry

### Community 133 - "TestCLI"
Cohesion: 0.14
Nodes (8): Tests for the argparse-based CLI entry point., --help prints usage and exits without calling inspect_app., Exit code 2 when app is not found., Exit code 1 when permissions denied., Default output goes to stdout as valid JSON., --output writes JSON to a file., --depth flag is forwarded to inspect_app., TestCLI

### Community 134 - "test_sources.py"
Cohesion: 0.18
Nodes (11): all_sources(), brightness(), psnr_between(), Auto-Discovering Source Verification — Tests ALL registered procedural sources., Sweep critical params across 5 positions to check for discontinuities., Discover all sources from the running app., Every registered source must render a non-black frame at defaults., Every param on every source must have a visible effect. (+3 more)

### Community 135 - "AddDeckCmd"
Cohesion: 0.19
Nodes (7): Deck, MoveLayerCmd, deckIndex_, deckResolver_, fence_, fromIndex_, toIndex_

### Community 136 - "Main.cpp"
Cohesion: 0.20
Nodes (9): DocumentWindow, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, String, unique_ptr, getApplicationName(), getApplicationVersion(), initialise(), MainWindow (+1 more)

### Community 137 - "FullscreenQuad"
Cohesion: 0.23
Nodes (8): FullscreenQuad, draw, init, initialized_, release, vao_, vbo_, GLuint

### Community 138 - "FXBrowser::FXListContent"
Cohesion: 0.18
Nodes (9): Component, MouseEvent, String, FXBrowser::FXListContent, draggedEffectName_, dragStarted_, kCategoryHeaderHeight, kEffectRowHeight (+1 more)

### Community 139 - "OnsetDetector"
Cohesion: 0.18
Nodes (8): aubio_onset_t, fvec_t, OnsetDetector, hopSize_, input_, onset_, output_, process

### Community 140 - "PitchTracker"
Cohesion: 0.18
Nodes (8): aubio_pitch_t, fvec_t, PitchTracker, hopSize_, input_, output_, pitch_, process

### Community 142 - "ISFShaderLoader.cpp"
Cohesion: 0.39
Nodes (10): ISFShader, File, string, ISFShaderLoader, convertToGLSL, extractGLSLBody, extractJSONBlock, getISFDirectory (+2 more)

### Community 143 - "EffectScope"
Cohesion: 0.20
Nodes (6): Kind, EffectScope, column, deckIndex, kind, layerIndex

### Community 144 - "MappingEditor.cpp"
Cohesion: 0.16
Nodes (16): Graphics, Listener, MappingCurve, MappingSource, String, getCurveName(), getSourceName(), addListener (+8 more)

### Community 145 - "CompositeCommand"
Cohesion: 0.19
Nodes (5): CompositeCommand, children_, string, unique_ptr, vector

### Community 146 - "AnalysisThread.h"
Cohesion: 0.21
Nodes (13): AdvancedAudioAnalyzer, BPMTracker, ChromaExtractor, FFTProcessor, GenreDetector, RingBuffer, KeyDetector, LoudnessAnalyzer (+5 more)

### Community 147 - "AudioEngine.cpp"
Cohesion: 0.15
Nodes (14): AudioEngine::AudioEngine(), getCurrentSampleRate, getDeviceStatus, hasAudioDevice, isPlaying, loadFile, onError, pause (+6 more)

### Community 148 - "Smoother"
Cohesion: 0.17
Nodes (3): Smoother, initialized_, Composition

### Community 149 - "FXBrowser.cpp"
Cohesion: 0.12
Nodes (19): Composition, EffectChain, atomic, mutex, Server, thread, Writer, Renderer (+11 more)

### Community 150 - "TopBar::TopBar"
Cohesion: 0.17
Nodes (12): Composition, FeatureBus, handleMultiplierButton, onBpmMultiplierChanged, onManualBpmChanged, onPause, onPlay, onQuantizeChanged (+4 more)

### Community 151 - "test_ax_inspector.py"
Cohesion: 0.17
Nodes (11): app(), _audio_dna_running(), _ensure_ax_mocks(), _mock_copy_attribute(), Tests for the accessibility tree inspector.  Unit tests mock the AX API so they, Mock for AXUIElementCopyAttributeValue that reads from element._attrs., Install mock modules for ApplicationServices and Cocoa if needed., Check if Audio-DNA is running (for integration test gating). (+3 more)

### Community 152 - "ToggleLayerFlagCmd"
Cohesion: 0.12
Nodes (15): Composition, EffectFenceHook, Tab, inspectClip, inspectLayer, inspectSignal, refresh, resized (+7 more)

### Community 153 - "VJAppController"
Cohesion: 0.13
Nodes (8): Get the current engine state (effects, FPS, etc.)., Controls Audio-DNA via the Eyes HTTP API., Get all registered procedural sources with their parameters., Get all signals with cached values., Get all active routes with current output values., Spawn the app in test mode and wait for it to become ready.          Args:, Check if the app is running and ready., VJAppController

### Community 154 - "itemDropped"
Cohesion: 0.17
Nodes (9): Thread, AVCodecContext, AVFormatContext, AVFrame, AVPacket, AVStream, condition_variable, SwsContext (+1 more)

### Community 155 - "LayerStrip::LayerStrip"
Cohesion: 0.18
Nodes (11): TextButton, LayerStrip::LayerStrip(), onBlendModeChanged, onBypass, onClearClip, onSolo, onTransportBack, onTransportForward (+3 more)

### Community 156 - "TopBar.cpp"
Cohesion: 0.31
Nodes (9): Graphics, paint, paintBarPhraseDisplay, paintBeatWheel, resized, setDspLoad, setFps, timerCallback (+1 more)

### Community 157 - "test_audio_reactivity.py"
Cohesion: 0.22
Nodes (8): psnr_between(), Audio Reactivity Verification — Tests that injected audio features change visual, Audio features should visibly change source output., RMS, bass, and beat phase should affect most sources that use u_rms/u_beatPhase., Audio features should change effect output when effects use audio uniforms., Effects that use u_rms should respond to RMS changes., TestAudioFeaturesAffectEffects, TestAudioFeaturesAffectSources

### Community 158 - "fromVar"
Cohesion: 0.24
Nodes (6): fromVar, toVar, var, var, fromVar, toVar

### Community 159 - "BrowserPanel.cpp"
Cohesion: 0.15
Nodes (11): paint, refresh, resized, setActiveTab, setComposition, setEffectLibrary, showActiveTab, updateTabButtonColors (+3 more)

### Community 160 - "WaveformSeqlock"
Cohesion: 0.24
Nodes (8): array, atomic, uint32_t, WaveformSeqlock, buffer_, count_, kSize, seq_

### Community 161 - "conftest.py"
Cohesion: 0.22
Nodes (8): app(), _default_executable(), Pytest configuration for Eyes visual tests.  Provides fixtures that spawn the Au, Find the built executable., Spawn Audio-DNA in test mode for the entire test session.      The app starts on, Reset app state before each test for isolation., reset_between_tests(), VJ App Controller — Python client for the Eyes test harness HTTP API.  Wraps the

### Community 162 - "TestAppLookup"
Cohesion: 0.20
Nodes (6): Tests for application discovery and error handling., Raises AccessibilityPermissionError when AX permissions are denied., Raises AppNotFoundError when the app is not running., inspect_app with explicit PID skips name lookup., inspect_app finds PID by app name when pid is not provided., TestAppLookup

### Community 163 - "Time"
Cohesion: 0.14
Nodes (9): Time, Performance Verification — Tests that sources and effects render within budget., Every source must render within budget., Effects should not significantly slow down rendering., TestEffectPerformance, TestSourcePerformance, psnr(), Test if an effect with defaults produces visible change. Returns PSNR. (+1 more)

### Community 164 - "vision_check.py"
Cohesion: 0.31
Nodes (8): ndarray, compute_psnr(), compute_ssim(), Vision Check — Image comparison for the Eyes visual testing harness.  Compares r, Compute Peak Signal-to-Noise Ratio between two images.      Returns float('inf'), Compute Structural Similarity Index between two images.      Uses scikit-image's, Compare a rendered frame against a golden reference.      Args:         rendered, verify_frame()

### Community 165 - "fence_"
Cohesion: 0.17
Nodes (8): AddDeckCmd, added_, addedIndex_, compResolver_, fence_, priorActiveIndex_, function, fence_

### Community 166 - "ResettableSlider"
Cohesion: 0.25
Nodes (5): MouseEvent, Slider, ResettableSlider, defaultVal_, hasDefault_

### Community 167 - "RouteTarget"
Cohesion: 0.22
Nodes (7): RouteTarget, clipId, effectIndex, layerId, paramIndex, scope, TargetScope

### Community 168 - "ClipPositionSignal"
Cohesion: 0.17
Nodes (4): ClipPositionSignal, currentPosition_, atomic, getClipPositionSignal

### Community 169 - "EffectsRackPanel.cpp"
Cohesion: 0.36
Nodes (11): EffectChain, closeMappingEditor, EffectsRackPanel::EffectsRackPanel(), findMappingForParam, isEffectLocked, mappingEditorCloseRequested, mappingEditorDeleteRequested, openMappingEditor (+3 more)

### Community 170 - "scan_all_presets.py"
Cohesion: 0.31
Nodes (7): classify_vibe(), load_progress(), main(), Load progress from previous run., Save progress for resume., Analyze a rendered frame and classify into a vibe category.     Returns (vibe, s, save_progress()

### Community 171 - "test_milkdrop.py"
Cohesion: 0.33
Nodes (8): load_preset(), main(), Load a MilkDrop preset via the test API., Capture a rendered frame., Score a rendered image on visual interest (0-100).      Criteria:     - Non-blac, render_frame(), reset(), score_image()

### Community 172 - "AudioCallback"
Cohesion: 0.22
Nodes (10): AudioIODevice, AudioIODeviceCallback, AudioIODeviceCallbackContext, AudioCallback, AudioCallback::AudioCallback(), audioDeviceAboutToStart, audioDeviceIOCallbackWithContext, audioDeviceStopped (+2 more)

### Community 173 - "AudioSignal"
Cohesion: 0.38
Nodes (5): AudioSignal, source_, Category, MappingSource, string

### Community 174 - "MoveLayerCmd"
Cohesion: 0.21
Nodes (12): addBinding, clearAll, fromVar, getBinding, getBindingAt, getRelativeCCValue, loadFromFile, removeBinding (+4 more)

### Community 175 - "RecordPanel.cpp"
Cohesion: 0.20
Nodes (9): SessionRecorder, Graphics, onPlayRecording, onStartRecording, onStopRecording, paint, RecordPanel::RecordPanel(), refresh (+1 more)

### Community 176 - "TestIntegration"
Cohesion: 0.25
Nodes (5): Integration tests that connect to a live Audio-DNA instance., Read the real accessibility tree and verify basic structure., A running JUCE app should have at least one window child., Live tree serializes to JSON and parses back correctly., TestIntegration

### Community 177 - "test_time_sweep.py"
Cohesion: 0.32
Nodes (5): brightness(), psnr_between(), Time Sweep Verification — Tests that animated sources/effects change over time., Animated sources must produce different frames at different times., TestSourcesAnimateOverTime

### Community 178 - ".mcp.json"
Cohesion: 0.29
Nodes (6): /opt/homebrew/bin/codegraph, /Users/boriskarpman/.local/bin/clangd-mcp, /Users/boriskarpman/.local/share/uv/tools/graphifyy/bin/python3, clangd-rta, codegraph-rta, graphify-rta

### Community 179 - "OutputWindow.cpp"
Cohesion: 0.17
Nodes (7): ToggleClipLockCmd, after_, before_, column_, deckIndex_, layerIndex_, resolver_

### Community 180 - "File"
Cohesion: 0.15
Nodes (13): MappingCurve, MappingSource, Mapping, curve, enabled, inputMax, inputMin, outputMax (+5 more)

### Community 181 - ".paintGrid"
Cohesion: 0.48
Nodes (4): FileEntry, Graphics, vector, paint

### Community 182 - "final_default_validation.py"
Cohesion: 0.38
Nodes (6): compute_psnr(), main(), Render clean baseline with no effects., Enable effect with explicit default params, render, compare to baseline., render_baseline(), test_effect_visual()

### Community 184 - "Renderer.h"
Cohesion: 0.14
Nodes (17): Autopilot, CompositorEngine, EffectLibrary, FeedbackProcessor, RoutingEngine, SourceRegistry, Effect, string (+9 more)

### Community 185 - "paintSectionHeader"
Cohesion: 0.47
Nodes (6): paint, paintSectionHeader, paintTimeline, Graphics, Rectangle, String

### Community 186 - "test_thumbnail_cache.cpp"
Cohesion: 0.33
Nodes (5): File, Image, String, makeImage(), makeTempFile()

### Community 187 - "TestJsonSerialization"
Cohesion: 0.33
Nodes (4): Tests for JSON output correctness., The walk result serializes to valid JSON., A nested tree round-trips through JSON correctly., TestJsonSerialization

### Community 188 - "test_signals.py"
Cohesion: 0.24
Nodes (6): psnr_between(), Signal Routing Verification — Tests the complete signal→route→parameter→shader p, RMS should change effect output via u_rms uniform., Injecting different audio features should produce visually different frames., Different feature values should produce different visual output on sources., TestFeatureInjectionChangesOutput

### Community 189 - "getThumbnailBounds"
Cohesion: 0.22
Nodes (11): Config, File, closeEncoder, encodeFrame, encoderThreadFunc, flushEncoder, getRecordedDuration, initEncoder (+3 more)

### Community 190 - "paintSectionHeader"
Cohesion: 0.40
Nodes (5): Graphics, Rectangle, String, paint, paintSectionHeader

### Community 191 - "scrubPlayhead"
Cohesion: 0.50
Nodes (5): MouseEvent, Point, mouseDown, mouseDrag, scrubPlayhead

### Community 192 - "ClearLayerClipsCmd"
Cohesion: 0.11
Nodes (22): applyLayerRuntime(), captureLayerClips(), captureLayerRuntime(), ClearLayerClipsCmd, after_, before_, deckIndex_, fence_ (+14 more)

### Community 194 - "SetColumnCountCmd"
Cohesion: 0.22
Nodes (8): Composition, EffectChain, Renderer, RoutingEngine, SignalRegistry, SourceRegistry, Writer, stop

### Community 195 - "TestFrameCapture"
Cohesion: 0.25
Nodes (5): Verify basic frame capture works., With no image loaded, capture should produce a frame (may be black)., Loading an image and capturing should produce a non-empty PNG., Two renders at the same time should produce identical frames., TestFrameCapture

### Community 196 - "isInThumbnailArea"
Cohesion: 0.28
Nodes (6): CaretOnlyComboBoxLookAndFeel, ComboBox, Label, LookAndFeel_V4, FlatButtonLookAndFeel, FlatComboBoxLookAndFeel

### Community 197 - "AnalysisThread.cpp"
Cohesion: 0.33
Nodes (5): AnalysisThread::AnalysisThread(), getPCMSamples, getWaveformSamples, run, RingBuffer

### Community 198 - ".getActiveClip"
Cohesion: 0.32
Nodes (8): buildSourceParamControls, ClipInspector::ClipInspector(), onCuepointSet, onSourceParamsChanged, refresh, setClip, syncFromClip, updateTransportHighlights

### Community 199 - "TestEffects"
Cohesion: 0.33
Nodes (4): Two effects chained should both apply., Verify effects can be enabled and produce visible changes., Enabling an effect should change the rendered output., TestEffects

### Community 201 - "TestSignalRegistrySpec"
Cohesion: 0.33
Nodes (4): Specification for signal registry tests.     These define the minimum signal set, Registry should have at least 12 audio + 2 modulation signals., After injecting features, signal cached values should update., TestSignalRegistrySpec

### Community 203 - "paintSectionHeader"
Cohesion: 0.40
Nodes (5): paint, paintSectionHeader, Graphics, Rectangle, String

### Community 206 - "test_downbeat_detector.cpp"
Cohesion: 0.70
Nodes (4): BPMTracker, feedBeatWithFeatures(), feedNonBeatHops(), lockBPM()

### Community 207 - "setComposition"
Cohesion: 0.50
Nodes (3): rebuildEffectStack, setComposition, Composition

### Community 213 - "test_bpm_stabilization.cpp"
Cohesion: 0.83
Nodes (3): BPMTracker, feedConstantBPM(), feedWithBeats()

### Community 218 - "changeListenerCallback"
Cohesion: 0.67
Nodes (3): ChangeBroadcaster, changeListenerCallback, onTransportStateChanged

### Community 219 - "MainComponent::keyPressed"
Cohesion: 0.67
Nodes (3): Component, KeyPress, MainComponent::keyPressed()

### Community 220 - "TopBar.h"
Cohesion: 0.50
Nodes (4): Graphics, Rectangle, drawSignalTriangle, paint

### Community 224 - "MainComponent::refreshPreviewFromActiveClip"
Cohesion: 0.33
Nodes (4): DeckActivateHook, function, MainComponent::makeDeckActivateHook(), MainComponent::makeEffectStackRefresh()

### Community 229 - "attachTo"
Cohesion: 0.67
Nodes (3): MainComponent::filesDropped(), MainComponent::isInterestedInFileDrag(), StringArray

## Knowledge Gaps
- **1625 isolated node(s):** `prevFrameTexture`, `prevFrameFBO`, `prevFrameWidth`, `prevFrameHeight`, `uniformLocationCache` (+1620 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **72 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `MainComponent` connect `MainComponent` to `ApiServer`, `AnalysisThread`, `MainComponent::makeLayerResolver`, `MainComponent.cpp`, `MainComponent::makeDeckFence`, `MainComponent::paint`, `VideoRecorder`, `PreviewPanel`, `InspectorPanel`, `TopBar`, `MidiLearnOverlay`, `AudioEngine`, `AudioDNALookAndFeel`, `AudioReadoutPanel`, `OutputRenderer`, `EffectsRackPanel`, `RecordPanel.cpp`, `BindingOverlay`, `MidiOutputHandler`, `BrowserPanel`, `MainComponent.h`, `OscHandler`, `MacroBank`, `MidiHandler`, `atomic`, `UndoManager`, `RingBuffer`, `WaveformDisplay`, `File`, `MainComponent::keyPressed`, `MainComponent::handleBindingAction`, `MainComponent::makeDeckResolver`, `MainComponent::makeClipMediaHook`, `MainComponent::refreshPreviewFromActiveClip`, `.getMenuBarModel`, `SpectrumDisplay`, `TimingWindow`, `EffectLibrary`, `attachTo`, `AudioDNAMenuBar`, `SignalBar`, `UndoService`, `LinkSync`, `SyphonOutput`?**
  _High betweenness centrality (0.275) - this node is a cross-community bridge._
- **Why does `Clip` connect `Clip` to `ClipInspector`, `Layer`, `EffectScope`, `ToggleLayerFlagCmd`, `fromVar`, `SwapClipsCmd`, `ClipPositionSignal`, `string`, `MidiOutputHandler`, `OutputWindow.cpp`, `DeckCommands.h`, `MainComponent.h`, `Deck`, `ClearLayerClipsCmd`, `.getActiveClip`, `atomic`, `MouseEvent`, `ClipCell`, `Autopilot`, `UndoService`?**
  _High betweenness centrality (0.101) - this node is a cross-community bridge._
- **Why does `ThumbnailCache` connect `ThumbnailCache` to `test_thumbnail_cache.cpp`, `MouseEvent`, `MainComponent.h`, `FilesBrowser`?**
  _High betweenness centrality (0.089) - this node is a cross-community bridge._
- **What connects `prevFrameTexture`, `prevFrameFBO`, `prevFrameWidth` to the rest of the system?**
  _1777 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `MainComponent` be split into smaller, more focused modules?**
  _Cohesion score 0.017391304347826087 - nodes in this community are weakly interconnected._
- **Should `Renderer` be split into smaller, more focused modules?**
  _Cohesion score 0.018108068955526583 - nodes in this community are weakly interconnected._
- **Should `Clip` be split into smaller, more focused modules?**
  _Cohesion score 0.02499247214694369 - nodes in this community are weakly interconnected._