# Graph Report - RealTimeAudio  (2026-07-22)

## Corpus Check
- 267 files · ~395,760 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 4901 nodes · 7880 edges · 242 communities (197 shown, 45 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 660 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `316a2bf8`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- MainComponent
- Renderer
- Clip
- BPMTracker
- ClipInspector
- ApiServer
- MilkDropBrowser
- DeckView
- AnalysisThread
- LayerInspector
- ClipCell
- Layer
- VideoRecorder
- CompDecksBrowser
- TestServer
- TopBar
- SourcesBrowser
- Composition
- FXBrowser
- LayerStrip
- CompositorEngine
- MidiLearnOverlay
- UniversalParamControl
- EffectStackView
- PipelineRunner
- AudioDNALookAndFeel
- FeatureSnapshot
- SignalInspector
- Command
- AdvancedAudioAnalyzer
- AudioReadoutPanel
- SessionRecorder
- ProjectMPresetManager
- string
- MainComponent.cpp
- CompositionInspector
- TextureManager
- BindingOverlay
- GenreDetector
- ProjectMSource
- BindingManager
- FilesBrowser
- MidiOutputHandler
- PresetSelector
- Renderer.cpp
- test_render_pipeline.py
- MappingEditor
- EffectChain
- Effect
- MappingEngine
- MilkDropBrowser.cpp
- test_fractals.py
- Binding
- OscHandler
- RecordPanel
- EnvelopeSignal
- CompositorEngine.cpp
- ax_inspector.py
- ProceduralSource
- CurveTransforms.h
- InspectorPanel
- EffectsRackPanel
- PresetManager
- Composition
- StructuralDetector
- AudioEngine
- WaveformDisplay
- ShaderManager
- DeckCommands.h
- Deck
- UndoManager
- MidiHandler
- FeedbackProcessor
- SpectralFeatures
- PreferencesDialog.cpp
- MacroPanel
- MFCCExtractor
- SyphonOutput
- EffectLibrary
- FullscreenQuad
- SignalRegistry
- TimingWindow
- Knob
- MilkDropBrowser::PresetListContent
- SignalStrip
- SpectrumDisplay
- MacroBank
- KeyDetector
- FXBrowser::FXListContent
- Autopilot
- Signal
- RoutingEngine
- SourceRegistry
- vector
- BPMTracker.cpp
- BrowserPanel
- OutputRenderer
- FFTProcessor
- AudioDNAMenuBar
- Route
- FeatureBus
- LinkSync
- SignalBar
- LoudnessAnalyzer
- OutputRenderer::OutputRenderer
- ClipInspector.cpp
- BindingManager.cpp
- _make_mock_element
- OscillatorSignal
- ClearLayerClipsCmd
- LayerInspector.cpp
- test_effects.py
- test_range_quality.py
- ChromaExtractor
- InspectorPanel.cpp
- SignalBar.cpp
- UniversalParamControl.cpp
- TestSignalRouteEndToEnd
- SetColumnCountCmd
- ._post
- PreviewPanel
- AudioEngine.cpp
- test_undo_commands.cpp
- fileListContent_
- FilesBrowser.cpp
- CaretOnlyComboBoxLookAndFeel
- LayerStrip.cpp
- SignalStrip.cpp
- TestCLI
- test_sources.py
- VJAppController
- AudioCallback
- OutputWindow
- Main.cpp
- RemoveColumnCmd
- OnsetDetector
- PitchTracker
- pushClipEdits
- ISFShaderLoader.cpp
- Colour
- RingBuffer
- FXBrowser.cpp
- ClipPositionSignal
- BrowserPanel.cpp
- CompositeCommand
- Smoother
- CompositionInspector.cpp
- string
- TopBar::TopBar
- test_ax_inspector.py
- FeedbackConfig
- LayerStrip::LayerStrip
- TopBar.cpp
- test_audio_reactivity.py
- RecordPanel.cpp
- fromVar
- WaveformSeqlock
- conftest.py
- TestAppLookup
- test_performance.py
- vision_check.py
- RouteTarget
- scan_all_presets.py
- test_milkdrop.py
- TestIntegration
- test_time_sweep.py
- MouseEvent
- .mcp.json
- drawSignalTriangle
- File
- AudioSignal
- mouseDown
- .paintGrid
- ResettableSlider
- EffectSlot
- final_default_validation.py
- .fromVar
- .getActiveClip
- paintSectionHeader
- TestJsonSerialization
- fromVar
- Renderer.h
- unordered_map
- paintSectionHeader
- test_downbeat_detector.cpp
- fromVar
- function
- beatSyncRandomize
- test_effect
- changeListenerCallback
- filesDropped
- Deck
- File
- SignalRegistry
- .setPinned
- .getMenuBarModel
- .getActiveDeck
- paint
- .setPerTypeAutopilotConfig
- setSignalRegistry
- MainComponent::imageReceived
- setSignalRegistry
- mouseDown
- setSignalRegistry
- .inject_features
- .add_route
- .set_macro
- .stop
- .load_image
- getDeviceStatus
- setDisplaySize
- BindableTarget
- Graphics
- Image
- KeyPress
- MouseEvent
- StringArray
- ComboBox
- FileDragAndDropTarget
- JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR
- KeyListener
- Label
- MenuBarModel
- Rectangle
- Slider
- TextButton
- Timer
- ToggleButton
- PresetEntry
- SourceParam
- T
- SetValueCmd
- TestEffects
- SourceDetails
- TestSourceRegistry
- test_bpm_stabilization.cpp
- getTransitionShaderName
- setComposition
- setupColumnTriggers
- setupDeckTabs
- Viewport

## God Nodes (most connected - your core abstractions)
1. `MainComponent` - 203 edges
2. `Renderer` - 170 edges
3. `Clip` - 125 edges
4. `ClipInspector` - 106 edges
5. `Layer` - 101 edges
6. `MilkDropBrowser` - 94 edges
7. `BPMTracker` - 92 edges
8. `LayerInspector` - 89 edges
9. `FeatureSnapshot` - 85 edges
10. `CompositorEngine` - 80 edges

## Surprising Connections (you probably didn't know these)
- `PipelineRunner` --references--> `kBlockSize`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/AnalysisThread.h
- `runPipeline()` --references--> `FeatureSnapshot`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/FeatureSnapshot.h
- `makeSnapshot()` --references--> `FeatureSnapshot`  [EXTRACTED]
  tests/test_mapping_engine.cpp → src/analysis/FeatureSnapshot.h
- `LogCmd` --inherits--> `Command`  [EXTRACTED]
  tests/test_composition.cpp → src/core/Command.h
- `SetCmd` --inherits--> `Command`  [EXTRACTED]
  tests/test_composition.cpp → src/core/Command.h

## Import Cycles
- None detected.

## Communities (242 total, 45 thin omitted)

### Community 0 - "MainComponent"
Cohesion: 0.01
Nodes (145): AnalysisThread, ApiServer, Array, AudioDNALookAndFeel, AudioDNAMenuBar, AudioEngine, AudioReadoutPanel, BindingManager (+137 more)

### Community 1 - "Renderer"
Cohesion: 0.02
Nodes (93): promise, atomic, Composition, FeatureBus, GLuint, Image, mutex, OpenGLContext (+85 more)

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
Nodes (62): ApiServer, handleComposition, handleGetBpm, handleGetFeatures, handleHealth, handleInjectFeatures, handleListEffects, handleListSources (+54 more)

### Community 6 - "MilkDropBrowser"
Cohesion: 0.03
Nodes (63): PresetListContent, ComboBox, Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, PlayMode, ProjectMPresetManager (+55 more)

### Community 7 - "DeckView"
Cohesion: 0.07
Nodes (28): CellPos, DeckView, activeColumn_, clipCells_, columnTriggers_, composition_, deckTabs_, gridContent_ (+20 more)

### Community 8 - "AnalysisThread"
Cohesion: 0.04
Nodes (58): AdvancedAudioAnalyzer, AnalysisThread, advancedAnalyzer_, analysisBuffer_, bpmTracker_, chromaExtractor_, cpuLoad_, currentPeak_ (+50 more)

### Community 9 - "LayerInspector"
Cohesion: 0.03
Nodes (61): ComboBox, Component, DragAndDropTarget, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, SignalRegistry, TextButton (+53 more)

### Community 10 - "ClipCell"
Cohesion: 0.06
Nodes (53): ClipCell, active_, clip_, column_, dragHover_, fileDragEnter, fileDragExit, filesDropped (+45 more)

### Community 11 - "Layer"
Cohesion: 0.03
Nodes (60): AutoSizeMode, KeyingMode, AutopilotAction, AutopilotDuration, EffectSlot, MixMode, optional, Type (+52 more)

### Community 12 - "VideoRecorder"
Cohesion: 0.05
Nodes (48): condition_variable, PixelBuffer, AVCodecContext, AVFormatContext, AVFrame, AVPacket, AVStream, Config (+40 more)

### Community 13 - "CompDecksBrowser"
Cohesion: 0.08
Nodes (25): CompDeckListContent, CompDecksBrowser, composition_, compositions_, compositionsExpanded_, decks_, decksExpanded_, kButtonBarHeight (+17 more)

### Community 14 - "TestServer"
Cohesion: 0.12
Nodes (47): Composition, Composition, EffectChain, FeatureBus, Renderer, Request, Response, RoutingEngine (+39 more)

### Community 15 - "TopBar"
Cohesion: 0.04
Nodes (51): array, ComboBox, Component, Composition, FeatureBus, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label (+43 more)

### Community 16 - "SourcesBrowser"
Cohesion: 0.06
Nodes (40): SourceEntry, SourceListContent, Component, Graphics, MouseEvent, String, CategoryInfo, Component (+32 more)

### Community 17 - "Composition"
Cohesion: 0.04
Nodes (47): AutopilotDirection, AutopilotDurationMode, CrossfaderBehaviour, CrossfaderBlendMode, CrossfaderCurve, QuantizeMode, Composition, activeDeckIndex (+39 more)

### Community 18 - "FXBrowser"
Cohesion: 0.09
Nodes (22): EffectEntry, FXListContent, FXBrowser, categories_, effectLibrary_, effects_, kCategoryHeaderHeight, kEffectRowHeight (+14 more)

### Community 19 - "LayerStrip"
Cohesion: 0.04
Nodes (44): ComboBox, Component, function, Image, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Rectangle, String, TextButton (+36 more)

### Community 20 - "CompositorEngine"
Cohesion: 0.05
Nodes (37): SourceRenderFn, CompositorEngine, accumulatorFBO_, accumulatorTex_, effectFBO_A_, effectFBO_B_, effectLibrary_, effectTex_A_ (+29 more)

### Community 21 - "MidiLearnOverlay"
Cohesion: 0.06
Nodes (44): AudioDeviceManager, BindableTarget, BindingManager, Component, Composition, Graphics, KeyPress, MidiInput (+36 more)

### Community 22 - "UniversalParamControl"
Cohesion: 0.05
Nodes (34): Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, SignalRegistry, SourceMode, String, TextButton (+26 more)

### Community 23 - "EffectStackView"
Cohesion: 0.07
Nodes (40): EffectRow, EffectSlot, Graphics, MouseEvent, SourceDetails, vector, EffectStackView, effectLibrary_ (+32 more)

### Community 24 - "PipelineRunner"
Cohesion: 0.07
Nodes (34): ChromaExtractor, FFTProcessor, KeyDetector, LoudnessAnalyzer, MFCCExtractor, OnsetDetector, PitchTracker, kBlockSize (+26 more)

### Community 25 - "AudioDNALookAndFeel"
Cohesion: 0.09
Nodes (40): Drawable, Font, ScrollBar, AudioDNALookAndFeel, AudioDNALookAndFeel::AudioDNALookAndFeel(), drawButtonBackground, drawButtonText, drawComboBox (+32 more)

### Community 26 - "FeatureSnapshot"
Cohesion: 0.05
Nodes (42): FeatureBus, FeatureSnapshot, bandEnergies, barCount, barPhase, beatInBar, beatPhase, bpm (+34 more)

### Community 27 - "SignalInspector"
Cohesion: 0.07
Nodes (39): Graphics, Rectangle, String, ComboBox, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Rectangle, ToggleButton (+31 more)

### Community 28 - "Command"
Cohesion: 0.14
Nodes (7): Command, description, execute, undo, MergeCmd, after, before

### Community 29 - "AdvancedAudioAnalyzer"
Cohesion: 0.05
Nodes (31): AdvancedAudioAnalyzer, bassHistory_, envelopeFull_, envelopePos_, fftSize_, formantBinHigh_, formantBinLow_, formantMax_ (+23 more)

### Community 30 - "AudioReadoutPanel"
Cohesion: 0.16
Nodes (35): AudioReadoutPanel, displayBands_, displaySnap_, downbeatFlash_, drawBandMeters, drawBarIndicator, drawBeatPhase, drawDbMeter (+27 more)

### Community 31 - "SessionRecorder"
Cohesion: 0.09
Nodes (34): CriticalSection, Event, File, string, vector, atomic, Event, vector (+26 more)

### Community 32 - "ProjectMPresetManager"
Cohesion: 0.13
Nodes (33): map, PresetInfo, string, vector, function, PresetInfo, string, vector (+25 more)

### Community 33 - "string"
Cohesion: 0.08
Nodes (25): ClipDeckResolver, ClipMediaHook, optional, SetClipCmd, after_, before_, column_, deckIndex_ (+17 more)

### Community 34 - "MainComponent.cpp"
Cohesion: 0.08
Nodes (38): Binding, Graphics, Image, MouseEvent, ClipDeckResolver, CompositionResolver, DeckActivateHook, advanceSlideshow (+30 more)

### Community 35 - "CompositionInspector"
Cohesion: 0.04
Nodes (55): CompositionInspector, anchorControl_, apClipLoopsSlider_, apDurationSelector_, apForwardBtn_, apLoopToggle_, apMasterLayerSelector_, apOffBtn_ (+47 more)

### Community 36 - "TextureManager"
Cohesion: 0.08
Nodes (24): File, GLuint, LUTLoader, loadCubeFile, releaseLUT, File, GLuint, Image (+16 more)

### Community 37 - "BindingOverlay"
Cohesion: 0.07
Nodes (36): BindingManager, Composition, BindingOverlay, active_, BindingOverlay::BindingOverlay(), exitBindingMode, findExistingBinding, getKeyDescription (+28 more)

### Community 38 - "GenreDetector"
Cohesion: 0.08
Nodes (28): Features, GenreDetector, candidateGenre_, chromaticComplexity, classify, computeEnergyState, computeScores, confidence_ (+20 more)

### Community 39 - "ProjectMSource"
Cohesion: 0.10
Nodes (32): GLState, GLuint, string, mutex, string, vector, ProjectMSource, applyParams (+24 more)

### Community 40 - "BindingManager"
Cohesion: 0.16
Nodes (8): ActionCallback, BindingCaptureCallback, BindingManager, bindingMode_, bindings_, nextId_, relativeCCValues_, vector

### Community 41 - "FilesBrowser"
Cohesion: 0.07
Nodes (30): FileListContent, FilesBrowser, currentDir_, entries_, favorites_, gridView_, gridViewBtn_, kNavBarHeight (+22 more)

### Community 42 - "MidiOutputHandler"
Cohesion: 0.09
Nodes (30): MidiOutput, Array, Deck, MidiDeviceInfo, MidiMessage, PadState, String, array (+22 more)

### Community 43 - "PresetSelector"
Cohesion: 0.08
Nodes (19): deque, mutex, function, string, PresetSelector, barsSinceLastSwitch_, enabled_, energyMatching_ (+11 more)

### Community 44 - "Renderer.cpp"
Cohesion: 0.09
Nodes (35): Component, FeatureBus, GLuint, Image, ImageSequence, SourceParam, string, vector (+27 more)

### Community 45 - "test_render_pipeline.py"
Cohesion: 0.11
Nodes (13): Eyes Visual Tests — Render Pipeline  Tests the core rendering pipeline: image lo, Verify audio feature injection works., Injecting features should succeed., Verify state endpoint works., State endpoint should list all effects., Verify reset clears state properly., After reset, no effects should be enabled., Verify the test server is responsive. (+5 more)

### Community 46 - "MappingEditor"
Cohesion: 0.05
Nodes (46): ListenerList, Graphics, Listener, MappingCurve, MappingSource, String, getCurveName(), getSourceName() (+38 more)

### Community 47 - "EffectChain"
Cohesion: 0.10
Nodes (26): GLint, GLuint, OpenGLShaderProgram, unique_ptr, EffectChain, addEffect, applyDryWet, effects_ (+18 more)

### Community 48 - "Effect"
Cohesion: 0.08
Nodes (21): String, Effect, addParam, category_, dryWet_, Effect::Effect(), enabled_, name_ (+13 more)

### Community 49 - "MappingEngine"
Cohesion: 0.07
Nodes (31): EffectChain, MappingCurve, MappingSource, vector, MappingEngine, addMapping, applyCurve, clearAll (+23 more)

### Community 50 - "MilkDropBrowser.cpp"
Cohesion: 0.12
Nodes (28): PlayMode, ProjectMPresetManager, string, SubTab, vector, addToRecent, calculateContentHeight, firePresetSelected (+20 more)

### Community 51 - "test_fractals.py"
Cohesion: 0.11
Nodes (17): frame_is_not_black(), frames_are_different(), Eyes Visual Tests — Comprehensive Fractal Source Validation  Tests EVERY control, Every fractal must render a visible frame at defaults., Every parameter must produce a visible change when modified., Zoom at ALL positions (0, 0.25, 0.5, 0.75, 1.0) must be non-black., Dive speed at various levels must not go black., Power at all positions must not go black. (+9 more)

### Community 52 - "Binding"
Cohesion: 0.07
Nodes (28): Action, CCMode, InputType, Binding, action, ccMode, ccStepSize, enabled (+20 more)

### Community 53 - "OscHandler"
Cohesion: 0.11
Nodes (24): OSCMessage, OSCReceiver, OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>, Composition, atomic, function, OscHandler, listening_ (+16 more)

### Community 54 - "RecordPanel"
Cohesion: 0.06
Nodes (34): SessionRecorder, Graphics, ComboBox, Component, File, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label (+26 more)

### Community 55 - "EnvelopeSignal"
Cohesion: 0.08
Nodes (12): ControlPoint, CurveType, EnvelopeSignal, amplitude_, beatDuration_, curveType_, looping_, oneShot_ (+4 more)

### Community 56 - "CompositorEngine.cpp"
Cohesion: 0.19
Nodes (26): applyClipTransform, applyLayerKeying, applyLayerTransform, applyMaskLayer, applyTransition, blendLayerOntoAccumulator, compositeDeck, compositePersistentLayers (+18 more)

### Community 57 - "ax_inspector.py"
Cohesion: 0.13
Nodes (25): Any, Exception, AccessibilityPermissionError, AppNotFoundError, AXInspectorError, check_accessibility_permissions(), find_pid_by_name(), _get_ax_attribute() (+17 more)

### Community 58 - "ProceduralSource"
Cohesion: 0.08
Nodes (34): Param, GLuint, OpenGLShaderProgram, GLuint, string, vector, ProceduralSource, category_ (+26 more)

### Community 59 - "CurveTransforms.h"
Cohesion: 0.16
Nodes (25): applyCurve(), backIn(), backInOut(), backOut(), bounceIn(), bounceInOut(), bounceOut(), circularIn() (+17 more)

### Community 60 - "InspectorPanel"
Cohesion: 0.08
Nodes (24): Component, Composition, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Tab, TextButton, Viewport, InspectorPanel, activeTab_ (+16 more)

### Community 61 - "EffectsRackPanel"
Cohesion: 0.09
Nodes (36): CategoryHeader, EffectSection, EffectChain, Graphics, EffectsRackPanel, activeMappingEditor_, categoryHeaders_, closeMappingEditor (+28 more)

### Community 62 - "PresetManager"
Cohesion: 0.22
Nodes (20): DeckState, Array, EffectChain, File, MappingCurve, MappingSource, String, PresetManager (+12 more)

### Community 63 - "Composition"
Cohesion: 0.10
Nodes (15): ClearLayerClipsCmd, after_, before_, deckIndex_, layerIndex_, mediaHook_, resolver_, Command (+7 more)

### Community 64 - "StructuralDetector"
Cohesion: 0.09
Nodes (21): StructuralDetector, candidateState_, classifyState, confirmedState_, fluxAlpha_, fluxEnv_, holdCounter_, holdThreshold_ (+13 more)

### Community 65 - "AudioEngine"
Cohesion: 0.09
Nodes (21): AudioFormatManager, AudioFormatReaderSource, AudioSourcePlayer, AudioTransportSource, ChangeListener, CombinedCallback, AudioEngine, audioCallback_ (+13 more)

### Community 66 - "WaveformDisplay"
Cohesion: 0.09
Nodes (22): Column, kWaveformBufferSize, Graphics, array, Component, kMaxColumns, Timer, WaveformDisplay (+14 more)

### Community 67 - "ShaderManager"
Cohesion: 0.13
Nodes (21): ProgramEntry, updateFeedbackBuffer, File, GLint, OpenGLContext, OpenGLShaderProgram, String, File (+13 more)

### Community 68 - "DeckCommands.h"
Cohesion: 0.18
Nodes (14): Clip, optional, vector, LayerClipsSnapshot, clips, runtime, layerClipsSnapshotHasContent(), RemoveColumnCmd (+6 more)

### Community 69 - "Deck"
Cohesion: 0.11
Nodes (11): Deck, id, kDefaultColumns, kDefaultLayers, layers, name, nextLayerId_, numColumns (+3 more)

### Community 70 - "UndoManager"
Cohesion: 0.15
Nodes (18): string, unique_ptr, function, unique_ptr, vector, UndoManager, canRedo, canUndo (+10 more)

### Community 71 - "MidiHandler"
Cohesion: 0.12
Nodes (22): Array, AudioDeviceManager, BindingManager, MidiDeviceInfo, MidiInput, MidiMessage, String, AudioDeviceManager (+14 more)

### Community 72 - "FeedbackProcessor"
Cohesion: 0.14
Nodes (15): GLuint, string, FeedbackProcessor, applyPreset, currentBuffer_, ensureSize, fbos_, height_ (+7 more)

### Community 73 - "SpectralFeatures"
Cohesion: 0.10
Nodes (15): BandRange, array, SpectralFeatures, bandMaxEnergy_, bandRanges_, computeBandBinRanges, fftSize_, fluxMax_ (+7 more)

### Community 74 - "PreferencesDialog.cpp"
Cohesion: 0.12
Nodes (16): DialogWindow, Component, function, Graphics, Rectangle, Tab, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, PreferencesDialog (+8 more)

### Community 75 - "MacroPanel"
Cohesion: 0.13
Nodes (20): MacroSlot, Graphics, array, Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, SignalRegistry, MacroPanel (+12 more)

### Community 76 - "MFCCExtractor"
Cohesion: 0.12
Nodes (20): MelFilter, array, MFCCExtractor, buildDCTMatrix, buildFilterbank, dctMatrix_, fftSize_, filterWeightOffsets_ (+12 more)

### Community 77 - "SyphonOutput"
Cohesion: 0.10
Nodes (17): NSObject, atomic, string, SyphonOutput, enabled_, impl_, init, publishTexture (+9 more)

### Community 78 - "EffectLibrary"
Cohesion: 0.16
Nodes (14): EffectDef, String, StringArray, unique_ptr, EffectLibrary, createEffect, defs_, getEffectDef (+6 more)

### Community 79 - "FullscreenQuad"
Cohesion: 0.14
Nodes (19): applyClipEffects, applyFXOnlyLayer, applyScreenSplit, getFrameFromRing, pushFrameToRing, saveToTemporalBuffer, uploadAudioUniforms, EffectSlot (+11 more)

### Community 80 - "SignalRegistry"
Cohesion: 0.12
Nodes (20): Category, string, unique_ptr, vector, unique_ptr, vector, SignalRegistry, addSignal (+12 more)

### Community 81 - "TimingWindow"
Cohesion: 0.11
Nodes (18): setActive, updateTabButtonColors, Graphics, Tab, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Tab, TextButton (+10 more)

### Community 82 - "Knob"
Cohesion: 0.12
Nodes (18): Graphics, String, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, String, Knob, kPreferredHeight (+10 more)

### Community 83 - "MilkDropBrowser::PresetListContent"
Cohesion: 0.23
Nodes (8): Colour, Component, Graphics, MouseEvent, PresetInfo, Section, paint, MilkDropBrowser::PresetListContent

### Community 84 - "SignalStrip"
Cohesion: 0.09
Nodes (22): DisplaySize, MouseEvent, Component, DisplaySize, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, SignalRegistry, SignalStrip (+14 more)

### Community 85 - "SpectrumDisplay"
Cohesion: 0.10
Nodes (20): FeatureBus, Graphics, array, Component, FeatureBus, Timer, uint32, SpectrumDisplay (+12 more)

### Community 86 - "MacroBank"
Cohesion: 0.14
Nodes (8): Macro, Scope, array, SignalRegistry, MacroBank, kNumMacros, macros_, scope_

### Community 87 - "KeyDetector"
Cohesion: 0.11
Nodes (15): array, KeyDetector, candidateCount_, candidateKey_, candidateMajor_, kHysteresisFrames, majorProfile_, minorProfile_ (+7 more)

### Community 88 - "FXBrowser::FXListContent"
Cohesion: 0.18
Nodes (9): Component, MouseEvent, String, FXBrowser::FXListContent, draggedEffectName_, dragStarted_, kCategoryHeaderHeight, kEffectRowHeight (+1 more)

### Community 89 - "Autopilot"
Cohesion: 0.18
Nodes (16): Autopilot, advanceClip, getActionForClip, getBeatsForClip, getPerTypeAction, getPerTypeBeats, lastBeatPhase_, lastEnergyState_ (+8 more)

### Community 90 - "Signal"
Cohesion: 0.11
Nodes (11): SignalRegistry, Category, string, Type, Signal, category_, getValue, id_ (+3 more)

### Community 91 - "RoutingEngine"
Cohesion: 0.14
Nodes (17): ParamWriter, SignalRegistry, vector, vector, RoutingEngine, addRoute, clearAll, getRoute (+9 more)

### Community 92 - "SourceRegistry"
Cohesion: 0.17
Nodes (18): SourceFactory, SourceInfo, SourceRegistry, string, unique_ptr, vector, string, unordered_map (+10 more)

### Community 93 - "vector"
Cohesion: 0.15
Nodes (6): atomic, Thread, vector, Array, Deck, set

### Community 94 - "BPMTracker.cpp"
Cohesion: 0.16
Nodes (18): analyzeDownbeatPosition, correctOctaveError, feedDownbeatFeatures, feedSilenceDetection, foldBPMToRange, process, processRawBPM, pushAndMedian (+10 more)

### Community 95 - "BrowserPanel"
Cohesion: 0.08
Nodes (30): BrowserPanel, activeTab_, compDecksBrowser_, compDecksTabBtn_, filesBrowser_, filesTabBtn_, fxBrowser_, fxTabBtn_ (+22 more)

### Community 96 - "OutputRenderer"
Cohesion: 0.07
Nodes (36): Component, EffectChain, FeatureBus, File, Image, EffectChain, FeatureBus, File (+28 more)

### Community 97 - "FFTProcessor"
Cohesion: 0.12
Nodes (13): FFT, FFTProcessor, buildHannWindow, fft_, fftData_, FFTProcessor::FFTProcessor(), hannWindow_, kFFTOrder (+5 more)

### Community 98 - "AudioDNAMenuBar"
Cohesion: 0.15
Nodes (16): pair, AudioDNAMenuBar, getMenuBarNames, getMenuForIndex, getRedoState, getUndoState, isSyphonOutputEnabled, menuItemSelected (+8 more)

### Community 99 - "Route"
Cohesion: 0.11
Nodes (19): SourceType, Route, dialRangeMax, dialRangeMin, enabled, falloff, gain, id (+11 more)

### Community 100 - "FeatureBus"
Cohesion: 0.15
Nodes (17): FeatureBus, acquireRead, acquireWrite, buffers_, FeatureBus::FeatureBus(), getLatestRead, hasNewData, kLatestMask (+9 more)

### Community 101 - "LinkSync"
Cohesion: 0.13
Nodes (11): atomic, LinkSync, beatPhase_, bpm_, enabled_, numPeers_, quantum_, requestBeatAtTime (+3 more)

### Community 102 - "SignalBar"
Cohesion: 0.10
Nodes (20): Graphics, Component, DisplaySize, FeatureBus, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, SignalRegistry, TextButton (+12 more)

### Community 103 - "LoudnessAnalyzer"
Cohesion: 0.13
Nodes (12): BiquadState, BiquadState, LoudnessAnalyzer, fillCount_, process, processBiquad, runningSum_, stage1_ (+4 more)

### Community 104 - "OutputRenderer::OutputRenderer"
Cohesion: 0.11
Nodes (11): unordered_map, ClipLayerResolver, Deck, string, ToggleClipLockCmd, after_, before_, column_ (+3 more)

### Community 105 - "ClipInspector.cpp"
Cohesion: 0.18
Nodes (17): buildSourceParamControls, ClipInspector::ClipInspector(), getPreferredHeight, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped, onCuepointSet (+9 more)

### Community 106 - "BindingManager.cpp"
Cohesion: 0.38
Nodes (7): actionCallback_, captureCallback_, processKeyDown, processKeyUp, processMidiCC, processMidiNoteOff, processMidiNoteOn

### Community 107 - "_make_mock_element"
Cohesion: 0.14
Nodes (12): _make_mock_element(), Tests for the recursive tree walker., Walk a leaf element with no children., Walk a tree with nested children., Walk an element with no attributes set., Tests for the --depth recursion limiter., Depth 0 returns the root element without walking children., Depth 1 walks immediate children but not grandchildren. (+4 more)

### Community 108 - "OscillatorSignal"
Cohesion: 0.13
Nodes (7): string, OscillatorSignal, amplitude_, beatDuration_, phaseOffset_, shape_, WaveShape

### Community 109 - "ClearLayerClipsCmd"
Cohesion: 0.12
Nodes (14): applyLayerRuntime(), ClearActiveClipCmd, after_, before_, deckIndex_, layerIndex_, resolver_, ClipLayerResolver (+6 more)

### Community 110 - "LayerInspector.cpp"
Cohesion: 0.13
Nodes (21): MouseEvent, SignalRegistry, SourceDetails, getPreferredHeight, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped (+13 more)

### Community 111 - "test_effects.py"
Cohesion: 0.14
Nodes (13): all_effects(), brightness(), image_loaded(), psnr_between(), Auto-Discovering Effect Verification — Tests ALL registered effects.  Queries /a, No effect should turn the image completely black or white., Discover all effects from the running app., Ensure test image is loaded. (+5 more)

### Community 112 - "test_range_quality.py"
Cohesion: 0.17
Nodes (14): analyze_sweep(), brightness(), Tier 2: Range Quality Analysis  For each source parameter, renders at 11 positio, Sweep every parameter and verify quality metrics., Each parameter must have >70% useful range, no discontinuities, no dead zones., Auto-discover all sources and sweep all their params.      This class discovers, Sweep every param on every source. Generates CSV reports., Render a source at multiple parameter positions, return list of (value, path) tu (+6 more)

### Community 113 - "ChromaExtractor"
Cohesion: 0.15
Nodes (12): ChromaExtractor, binToChroma_, ChromaExtractor::ChromaExtractor(), computeBinToChromaMap, fftSize_, hasPrevFrame_, kNumChroma, numBins_ (+4 more)

### Community 114 - "InspectorPanel.cpp"
Cohesion: 0.12
Nodes (15): Composition, Graphics, Tab, inspectClip, inspectLayer, inspectSignal, paint, refresh (+7 more)

### Community 115 - "SignalBar.cpp"
Cohesion: 0.16
Nodes (18): BindableTarget, KeyPress, Component, DeckFenceHook, buildBindableTargets, closeOutput, enterKeyboardBindingMode, enterMidiLearnMode (+10 more)

### Community 116 - "UniversalParamControl.cpp"
Cohesion: 0.15
Nodes (20): MouseEvent, PopupMenu, String, buildSourcePickerMenu, getPreferredHeight, handleSourcePickerResult, mouseDown, onExpandToggled (+12 more)

### Community 117 - "TestSignalRouteEndToEnd"
Cohesion: 0.08
Nodes (19): psnr_between(), Signal Routing Verification — Tests the complete signal→route→parameter→shader p, RMS should change effect output via u_rms uniform., End-to-end: create route from audio signal to effect parameter,     inject featu, Check if signal API endpoints are available., Route Volume signal → Ripple intensity → verify RMS changes ripple., Route with threshold=0.5 should only activate above 0.5 RMS., Inverted route: high RMS should DECREASE the parameter. (+11 more)

### Community 118 - "SetColumnCountCmd"
Cohesion: 0.18
Nodes (12): ClipDeckResolver, string, MoveLayerCmd, deckIndex_, fence_, fromIndex_, toIndex_, SetColumnCountCmd (+4 more)

### Community 119 - "._post"
Cohesion: 0.12
Nodes (8): Configure the entire effect chain.          Disables all existing effects, then, Render a single frame and save it to disk.          Args:             output_pat, Reset all effects, clear images, restore defaults., Load a procedural source into the active clip.          Args:             source, Update parameters on the currently active source.          Args:             par, Remove a signal route by ID., Send a POST request with JSON body., Enable/disable an effect and optionally set parameters.          Args:

### Community 120 - "PreviewPanel"
Cohesion: 0.05
Nodes (44): DeckView, GLHost, InspectorPanel, ReinspectTarget, function, DeckView, Composition, Deck (+36 more)

### Community 121 - "AudioEngine.cpp"
Cohesion: 0.15
Nodes (14): AudioEngine::AudioEngine(), getCurrentSampleRate, getDeviceStatus, hasAudioDevice, isPlaying, loadFile, onError, pause (+6 more)

### Community 122 - "test_undo_commands.cpp"
Cohesion: 0.07
Nodes (33): EffectSlot, FeedbackConfig, PresetEntry, SourceParam, File, needsVideoReopen(), UndoService, T (+25 more)

### Community 123 - "fileListContent_"
Cohesion: 0.17
Nodes (10): Component, MouseEvent, Point, fileListContent_, dragStarted_, kLabelHeight, kListRowHeight, kPadding (+2 more)

### Community 124 - "FilesBrowser.cpp"
Cohesion: 0.29
Nodes (12): File, Image, FilesBrowser::~FilesBrowser(), filterBySearch, generateThumbnail, isMediaFile, loadFavorites, navigateTo (+4 more)

### Community 125 - "CaretOnlyComboBoxLookAndFeel"
Cohesion: 0.28
Nodes (6): CaretOnlyComboBoxLookAndFeel, ComboBox, Label, LookAndFeel_V4, FlatButtonLookAndFeel, FlatComboBoxLookAndFeel

### Community 126 - "LayerStrip.cpp"
Cohesion: 0.21
Nodes (14): MouseEvent, Point, mouseDown, mouseDrag, populateBlendDropdown, populateTransitionDropdown, refresh, resized (+6 more)

### Community 127 - "SignalStrip.cpp"
Cohesion: 0.19
Nodes (13): DisplaySize, FeatureBus, SignalRegistry, grow, onSignalSelected, onSizeChanged, rebuildStrips, resized (+5 more)

### Community 128 - "TestCLI"
Cohesion: 0.14
Nodes (8): Tests for the argparse-based CLI entry point., --help prints usage and exits without calling inspect_app., Exit code 2 when app is not found., Exit code 1 when permissions denied., Default output goes to stdout as valid JSON., --output writes JSON to a file., --depth flag is forwarded to inspect_app., TestCLI

### Community 129 - "test_sources.py"
Cohesion: 0.18
Nodes (11): all_sources(), brightness(), psnr_between(), Auto-Discovering Source Verification — Tests ALL registered procedural sources., Sweep critical params across 5 positions to check for discontinuities., Discover all sources from the running app., Every registered source must render a non-black frame at defaults., Every param on every source must have a visible effect. (+3 more)

### Community 130 - "VJAppController"
Cohesion: 0.13
Nodes (8): Get the current engine state (effects, FPS, etc.)., Controls Audio-DNA via the Eyes HTTP API., Get all registered procedural sources with their parameters., Get all signals with cached values., Get all active routes with current output values., Spawn the app in test mode and wait for it to become ready.          Args:, Check if the app is running and ready., VJAppController

### Community 131 - "AudioCallback"
Cohesion: 0.19
Nodes (10): AudioIODevice, AudioIODeviceCallback, AudioIODeviceCallbackContext, AudioCallback, AudioCallback::AudioCallback(), audioDeviceAboutToStart, audioDeviceIOCallbackWithContext, audioDeviceStopped (+2 more)

### Community 132 - "OutputWindow"
Cohesion: 0.16
Nodes (13): Display, OutputComponent, enterBindingMode, KeyPress, DocumentWindow, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, OutputWindow, closeButtonPressed (+5 more)

### Community 133 - "Main.cpp"
Cohesion: 0.20
Nodes (9): DocumentWindow, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, String, unique_ptr, getApplicationName(), getApplicationVersion(), initialise(), MainWindow (+1 more)

### Community 134 - "RemoveColumnCmd"
Cohesion: 0.16
Nodes (6): RemoveDeckCmd, compResolver_, deckIndex_, priorActiveIndex_, removed_, mediaHook_

### Community 135 - "OnsetDetector"
Cohesion: 0.18
Nodes (8): aubio_onset_t, fvec_t, OnsetDetector, hopSize_, input_, onset_, output_, process

### Community 136 - "PitchTracker"
Cohesion: 0.18
Nodes (8): aubio_pitch_t, fvec_t, PitchTracker, hopSize_, input_, output_, pitch_, process

### Community 137 - "pushClipEdits"
Cohesion: 0.15
Nodes (21): CellEdit, Clip, ClipLayerResolver, ClipMediaHook, Command, File, Layer, optional (+13 more)

### Community 138 - "ISFShaderLoader.cpp"
Cohesion: 0.39
Nodes (10): ISFShader, File, string, ISFShaderLoader, convertToGLSL, extractGLSLBody, extractJSONBlock, getISFDirectory (+2 more)

### Community 139 - "Colour"
Cohesion: 0.19
Nodes (13): SliderLayout, Button, Colour, Graphics, Slider, SliderStyle, FadeSpeedSliderLookAndFeel, FullBoundsSliderLAF (+5 more)

### Community 140 - "RingBuffer"
Cohesion: 0.18
Nodes (7): atomic, T, RingBuffer, buffer_, mask_, readPos_, writePos_

### Community 141 - "FXBrowser.cpp"
Cohesion: 0.23
Nodes (8): Graphics, buildCategoryList, FXBrowser::~FXBrowser(), paint, refresh, resized, setEffectLibrary, toggleCategory

### Community 142 - "ClipPositionSignal"
Cohesion: 0.25
Nodes (3): ClipPositionSignal, currentPosition_, atomic

### Community 143 - "BrowserPanel.cpp"
Cohesion: 0.16
Nodes (8): Flag, ToggleLayerFlagCmd, after_, before_, deckIndex_, flag_, layerIndex_, resolver_

### Community 144 - "CompositeCommand"
Cohesion: 0.21
Nodes (5): CompositeCommand, children_, string, unique_ptr, vector

### Community 145 - "Smoother"
Cohesion: 0.17
Nodes (3): Smoother, initialized_, Composition

### Community 146 - "CompositionInspector.cpp"
Cohesion: 0.18
Nodes (8): function, AddLayerCmd, added_, addedIndex_, deckIndex_, deckResolver_, fence_, fence_

### Community 147 - "string"
Cohesion: 0.21
Nodes (10): captureLayerClips(), captureLayerRuntime(), ClipMediaHook, Layer, RemoveLayerCmd, deckIndex_, deckResolver_, fence_ (+2 more)

### Community 148 - "TopBar::TopBar"
Cohesion: 0.17
Nodes (12): Composition, FeatureBus, handleMultiplierButton, onBpmMultiplierChanged, onManualBpmChanged, onPause, onPlay, onQuantizeChanged (+4 more)

### Community 149 - "test_ax_inspector.py"
Cohesion: 0.17
Nodes (11): app(), _audio_dna_running(), _ensure_ax_mocks(), _mock_copy_attribute(), Tests for the accessibility tree inspector.  Unit tests mock the AX API so they, Mock for AXUIElementCopyAttributeValue that reads from element._attrs., Install mock modules for ApplicationServices and Cocoa if needed., Check if Audio-DNA is running (for integration test gating). (+3 more)

### Community 150 - "FeedbackConfig"
Cohesion: 0.12
Nodes (15): FeedbackConfig, amount, enabled, lumaKey, offsetX, offsetY, presetName, rotation (+7 more)

### Community 151 - "LayerStrip::LayerStrip"
Cohesion: 0.18
Nodes (11): TextButton, LayerStrip::LayerStrip(), onBlendModeChanged, onBypass, onClearClip, onSolo, onTransportBack, onTransportForward (+3 more)

### Community 152 - "TopBar.cpp"
Cohesion: 0.31
Nodes (9): Graphics, paint, paintBarPhraseDisplay, paintBeatWheel, resized, setDspLoad, setFps, timerCallback (+1 more)

### Community 153 - "test_audio_reactivity.py"
Cohesion: 0.22
Nodes (8): psnr_between(), Audio Reactivity Verification — Tests that injected audio features change visual, Audio features should visibly change source output., RMS, bass, and beat phase should affect most sources that use u_rms/u_beatPhase., Audio features should change effect output when effects use audio uniforms., Effects that use u_rms should respond to RMS changes., TestAudioFeaturesAffectEffects, TestAudioFeaturesAffectSources

### Community 154 - "RecordPanel.cpp"
Cohesion: 0.36
Nodes (12): Colour, Graphics, Rectangle, SignalRegistry, String, getFormattedValue, getSignalColour, paint (+4 more)

### Community 155 - "fromVar"
Cohesion: 0.19
Nodes (12): addBinding, clearAll, fromVar, getBinding, getBindingAt, getRelativeCCValue, loadFromFile, removeBinding (+4 more)

### Community 156 - "WaveformSeqlock"
Cohesion: 0.24
Nodes (8): array, atomic, uint32_t, WaveformSeqlock, buffer_, count_, kSize, seq_

### Community 157 - "conftest.py"
Cohesion: 0.22
Nodes (8): app(), _default_executable(), Pytest configuration for Eyes visual tests.  Provides fixtures that spawn the Au, Find the built executable., Spawn Audio-DNA in test mode for the entire test session.      The app starts on, Reset app state before each test for isolation., reset_between_tests(), VJ App Controller — Python client for the Eyes test harness HTTP API.  Wraps the

### Community 158 - "TestAppLookup"
Cohesion: 0.20
Nodes (6): Tests for application discovery and error handling., Raises AccessibilityPermissionError when AX permissions are denied., Raises AppNotFoundError when the app is not running., inspect_app with explicit PID skips name lookup., inspect_app finds PID by app name when pid is not provided., TestAppLookup

### Community 159 - "test_performance.py"
Cohesion: 0.20
Nodes (5): Performance Verification — Tests that sources and effects render within budget., Every source must render within budget., Effects should not significantly slow down rendering., TestEffectPerformance, TestSourcePerformance

### Community 160 - "vision_check.py"
Cohesion: 0.31
Nodes (8): ndarray, compute_psnr(), compute_ssim(), Vision Check — Image comparison for the Eyes visual testing harness.  Compares r, Compute Peak Signal-to-Noise Ratio between two images.      Returns float('inf'), Compute Structural Similarity Index between two images.      Uses scikit-image's, Compare a rendered frame against a golden reference.      Args:         rendered, verify_frame()

### Community 161 - "RouteTarget"
Cohesion: 0.22
Nodes (7): RouteTarget, clipId, effectIndex, layerId, paramIndex, scope, TargetScope

### Community 162 - "scan_all_presets.py"
Cohesion: 0.31
Nodes (7): classify_vibe(), load_progress(), main(), Load progress from previous run., Save progress for resume., Analyze a rendered frame and classify into a vibe category.     Returns (vibe, s, save_progress()

### Community 163 - "test_milkdrop.py"
Cohesion: 0.33
Nodes (8): load_preset(), main(), Load a MilkDrop preset via the test API., Capture a rendered frame., Score a rendered image on visual interest (0-100).      Criteria:     - Non-blac, render_frame(), reset(), score_image()

### Community 164 - "TestIntegration"
Cohesion: 0.25
Nodes (5): Integration tests that connect to a live Audio-DNA instance., Read the real accessibility tree and verify basic structure., A running JUCE app should have at least one window child., Live tree serializes to JSON and parses back correctly., TestIntegration

### Community 165 - "test_time_sweep.py"
Cohesion: 0.32
Nodes (5): brightness(), psnr_between(), Time Sweep Verification — Tests that animated sources/effects change over time., Animated sources must produce different frames at different times., TestSourcesAnimateOverTime

### Community 167 - ".mcp.json"
Cohesion: 0.29
Nodes (6): /opt/homebrew/bin/codegraph, /Users/boriskarpman/.local/bin/clangd-mcp, /Users/boriskarpman/.local/share/uv/tools/graphifyy/bin/python3, clangd-rta, codegraph-rta, graphify-rta

### Community 168 - "drawSignalTriangle"
Cohesion: 0.50
Nodes (4): Graphics, Rectangle, drawSignalTriangle, paint

### Community 169 - "File"
Cohesion: 0.33
Nodes (7): File, captureFrame, getVideoPlayerFile, loadImage, openImageSequenceForClip, openVideoForClip, takeSnapshot

### Community 170 - "AudioSignal"
Cohesion: 0.32
Nodes (5): AudioSignal, source_, Category, MappingSource, string

### Community 171 - "mouseDown"
Cohesion: 0.38
Nodes (7): mouseDown, mouseDrag, mouseUp, normalizedToTimelineX, onCuepointJump, timelineXToNormalized, MouseEvent

### Community 172 - ".paintGrid"
Cohesion: 0.48
Nodes (4): FileEntry, Graphics, vector, paint

### Community 173 - "ResettableSlider"
Cohesion: 0.25
Nodes (5): MouseEvent, Slider, ResettableSlider, defaultVal_, hasDefault_

### Community 175 - "final_default_validation.py"
Cohesion: 0.38
Nodes (6): compute_psnr(), main(), Render clean baseline with no effects., Enable effect with explicit default params, render, compare to baseline., render_baseline(), test_effect_visual()

### Community 177 - ".getActiveClip"
Cohesion: 0.21
Nodes (8): AddDeckCmd, added_, addedIndex_, compResolver_, fence_, priorActiveIndex_, CompositionResolver, DeckFenceHook

### Community 178 - "paintSectionHeader"
Cohesion: 0.47
Nodes (6): paint, paintSectionHeader, paintTimeline, Graphics, Rectangle, String

### Community 179 - "TestJsonSerialization"
Cohesion: 0.33
Nodes (4): Tests for JSON output correctness., The walk result serializes to valid JSON., A nested tree round-trips through JSON correctly., TestJsonSerialization

### Community 180 - "fromVar"
Cohesion: 0.24
Nodes (6): fromVar, toVar, var, var, fromVar, toVar

### Community 181 - "Renderer.h"
Cohesion: 0.40
Nodes (3): ImageSequence, RoutingEngine, VideoPlayer

### Community 182 - "unordered_map"
Cohesion: 0.24
Nodes (6): CompDecksBrowser::CompDeckListContent, kEntryRowHeight, kSectionHeaderHeight, resized, Component, MouseEvent

### Community 183 - "paintSectionHeader"
Cohesion: 0.40
Nodes (5): Graphics, Rectangle, String, paint, paintSectionHeader

### Community 184 - "test_downbeat_detector.cpp"
Cohesion: 0.33
Nodes (7): BPMTracker, feedConstantBPM(), feedWithBeats(), BPMTracker, feedBeatWithFeatures(), feedNonBeatHops(), lockBPM()

### Community 187 - "beatSyncRandomize"
Cohesion: 0.36
Nodes (6): paint, Colour, Graphics, SavedEntry, String, vector

### Community 188 - "test_effect"
Cohesion: 0.67
Nodes (3): psnr(), Test if an effect with defaults produces visible change. Returns PSNR., test_effect()

### Community 189 - "changeListenerCallback"
Cohesion: 0.67
Nodes (3): ChangeBroadcaster, changeListenerCallback, onTransportStateChanged

### Community 190 - "filesDropped"
Cohesion: 0.46
Nodes (7): CompDecksBrowser::~CompDecksBrowser(), getCompositionsDir, getDecksDir, onCompositionSave, refresh, scanForFiles, File

### Community 201 - "MainComponent::imageReceived"
Cohesion: 0.33
Nodes (4): AnalysisThread::AnalysisThread(), getPCMSamples, getWaveformSamples, run

### Community 203 - "mouseDown"
Cohesion: 0.13
Nodes (15): onClipMoved, onClipSelected, onClipTriggered, onEffectDropped, onFileDropped, onLayerBypass, onLayerClearClip, onLayerSelected (+7 more)

### Community 204 - "setSignalRegistry"
Cohesion: 0.50
Nodes (4): SpectralFeatures, vector, flatMagnitudeSpectrum(), sineMagnitudeSpectrum()

### Community 210 - "getDeviceStatus"
Cohesion: 0.21
Nodes (11): Graphics, clearSelection, getNaturalHeight, layoutGrid, paint, refresh, resized, selectCell (+3 more)

### Community 211 - "setDisplaySize"
Cohesion: 0.50
Nodes (3): ClipCell, Deck, LayerStrip

### Community 213 - "Graphics"
Cohesion: 0.18
Nodes (4): string, SetCmd, newVal, oldVal

### Community 220 - "JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR"
Cohesion: 0.25
Nodes (5): Verify basic frame capture works., With no image loaded, capture should produce a frame (may be black)., Loading an image and capturing should produce a non-empty PNG., Two renders at the same time should produce identical frames., TestFrameCapture

### Community 226 - "TextButton"
Cohesion: 0.40
Nodes (3): vector, LogCmd, id

### Community 232 - "SetValueCmd"
Cohesion: 0.33
Nodes (3): SetValueCmd, newVal, oldVal

### Community 233 - "TestEffects"
Cohesion: 0.33
Nodes (4): Two effects chained should both apply., Verify effects can be enabled and produce visible changes., Enabling an effect should change the rendered output., TestEffects

## Knowledge Gaps
- **1583 isolated node(s):** `lookAndFeel_`, `ringBuffer_`, `audioEngine_`, `analysisThread_`, `openImageButton_` (+1578 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **45 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Renderer` connect `Renderer` to `AnalysisThread`, `VideoRecorder`, `CompositorEngine`, `TextureManager`, `File`, `Renderer.cpp`, `MappingEngine`, `Renderer.h`, `function`, `ProceduralSource`, `Deck`, `File`, `SignalRegistry`, `ShaderManager`, `.setPerTypeAutopilotConfig`, `SyphonOutput`, `EffectLibrary`, `FullscreenQuad`, `Autopilot`, `SourceRegistry`?**
  _High betweenness centrality (0.124) - this node is a cross-community bridge._
- **Why does `Layer` connect `Layer` to `Clip`, `Deck`, `Signal`, `OutputRenderer::OutputRenderer`, `FeedbackProcessor`, `MidiOutputHandler`, `LayerInspector`, `LayerInspector.cpp`, `FullscreenQuad`, `InspectorPanel.cpp`, `LayerStrip`, `fromVar`, `FeedbackConfig`, `UniversalParamControl`, `PreviewPanel`, `Autopilot`, `CompositorEngine.cpp`, `LayerStrip.cpp`?**
  _High betweenness centrality (0.094) - this node is a cross-community bridge._
- **Why does `EffectLibrary` connect `EffectLibrary` to `Renderer`, `ApiServer`, `ISFShaderLoader.cpp`, `FXBrowser.cpp`, `FXBrowser`, `CompositorEngine`, `EffectStackView`, `CompositionInspector`, `BindingOverlay`, `MappingEngine`, `Renderer.h`, `EffectsRackPanel`, `FeedbackProcessor`, `MacroBank`, `Signal`, `vector`, `BrowserPanel`, `OutputRenderer::OutputRenderer`, `ClipInspector.cpp`, `LayerInspector.cpp`, `InspectorPanel.cpp`?**
  _High betweenness centrality (0.092) - this node is a cross-community bridge._
- **What connects `lookAndFeel_`, `ringBuffer_`, `audioEngine_` to the rest of the system?**
  _1735 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `MainComponent` be split into smaller, more focused modules?**
  _Cohesion score 0.013793103448275862 - nodes in this community are weakly interconnected._
- **Should `Renderer` be split into smaller, more focused modules?**
  _Cohesion score 0.018867924528301886 - nodes in this community are weakly interconnected._
- **Should `Clip` be split into smaller, more focused modules?**
  _Cohesion score 0.024684102262709375 - nodes in this community are weakly interconnected._