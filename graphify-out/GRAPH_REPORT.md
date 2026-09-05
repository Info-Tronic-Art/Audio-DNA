# Graph Report - RealTimeAudio  (2026-09-05)

## Corpus Check
- 281 files · ~434,709 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 5319 nodes · 8567 edges · 280 communities (210 shown, 70 thin omitted)
- Extraction: 92% EXTRACTED · 8% INFERRED · 0% AMBIGUOUS · INFERRED: 654 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `8354b0ad`
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
- AnalysisThread
- MilkDropBrowser
- LayerInspector
- TestServer
- ProjectMPresetManager
- VideoRecorder
- MainComponent.cpp
- SwapClipsCmd
- CompDecksBrowser
- vector
- TopBar
- ShaderManager
- LayerStrip
- InspectorPanel
- SourcesBrowser
- Composition
- FXBrowser
- EffectStackView
- CompositorEngine
- MidiLearnOverlay
- UniversalParamControl
- PipelineRunner
- AudioDNALookAndFeel
- FeatureSnapshot
- string
- MainComponent.h
- SignalInspector
- CompositionInspector
- AdvancedAudioAnalyzer
- AudioReadoutPanel
- SessionRecorder
- FilesBrowser
- EffectsRackPanel
- TextureManager
- TriggerClipCmd
- BindingOverlay
- GenreDetector
- OutputRenderer
- MidiOutputHandler
- DeckView
- EffectChainGLState
- test_signals.py
- MappingEditor
- Renderer.cpp
- PresetSelector
- Binding
- EnvelopeSignal
- RecordPanel
- loadPreset
- OscHandler
- Deck
- SwitchDeckCmd
- ClipInspector.cpp
- ax_inspector.py
- CurveTransforms.h
- StructuralDetector
- test_fractals.py
- AudioEngine
- WaveformDisplay
- UndoManager
- Autopilot
- ClipCell
- SignalStrip
- MFCCExtractor
- ProceduralSource
- MidiHandler
- SignalRegistry
- MilkDropBrowser.cpp
- Command
- SpectralFeatures
- PreferencesDialog.cpp
- MacroPanel
- ShaderManager.cpp
- .runFenced
- TimingWindow
- Knob
- LayerInspector.cpp
- BindingManager
- ThumbnailCache
- ToggleLayerFlagCmd
- KeyDetector
- EffectStackCmd
- Effect
- EffectLibrary
- MappingEngine
- MilkDropBrowser::PresetListContent
- SignalBar
- SpectrumDisplay
- UniversalParamControl.cpp
- RoutingEngine
- BPMTracker.cpp
- Mapping
- Signal
- BrowserPanel
- test_mapping_tick.py
- FFTProcessor
- AudioDNAMenuBar
- Colour
- SourceRegistry
- Route
- FeatureBus
- FeedbackProcessor
- LinkSync
- FilesBrowser.cpp
- ProjectMSource
- LoudnessAnalyzer
- UndoService
- OscillatorSignal
- MidiLearnOverlay.cpp
- ClipCell.cpp
- _make_mock_element
- test_render_pipeline.py
- test_effects.py
- test_range_quality.py
- VJAppController
- MacroBank
- ChromaExtractor
- rebuildGrid
- MappingEditor.cpp
- makeSetClipCmd
- PreviewPanel
- TestSignalRouteEndToEnd
- ClearActiveClipCmd
- FeedbackConfig
- SyphonOutput
- CompositionInspector.cpp
- DeckView.cpp
- fileListContent_
- LayerStrip.cpp
- SignalBar.cpp
- TestCLI
- test_sources.py
- AudioCallback
- OutputWindow
- AddDeckCmd
- Main.cpp
- PreviewPanel.cpp
- operator==
- OnsetDetector
- PitchTracker
- ISFShaderLoader.cpp
- EffectScope
- RingBuffer
- MoveLayerCmd
- CompositeCommand
- BrowserPanel.cpp
- string
- Smoother
- AudioSignal
- ClearLayerClipsCmd
- TopBar::TopBar
- test_ax_inspector.py
- ._post
- itemDropped
- LayerStrip::LayerStrip
- SignalStrip.cpp
- test_audio_reactivity.py
- SetColumnCountCmd
- RecordPanel.cpp
- ClipPositionSignal
- TopBar.cpp
- WaveformSeqlock
- conftest.py
- TestAppLookup
- test_performance.py
- vision_check.py
- fence_
- RouteTarget
- CaretOnlyComboBoxLookAndFeel
- scan_all_presets.py
- test_milkdrop.py
- SyphonOutputImpl
- fromVar
- ResettableSlider
- TestIntegration
- TestFrameCapture
- test_time_sweep.py
- UndoService.cpp
- .mcp.json
- .paintGrid
- final_default_validation.py
- AnalysisThread.cpp
- EffectParam
- MouseEvent
- .fromVar
- File
- paintSectionHeader
- test_thumbnail_cache.cpp
- TestJsonSerialization
- TestEffects
- TestWalkElement
- getThumbnailBounds
- isInThumbnailArea
- paintSectionHeader
- scrubPlayhead
- drawSignalTriangle
- test_downbeat_detector.cpp
- TestSourceRegistry
- test_spectral_features.cpp
- beatSyncRandomize
- function
- addParam
- changeListenerCallback
- OutputRenderer::OutputRenderer
- test_bpm_stabilization.cpp
- Composition
- verify_defaults.py
- setComposition
- paint
- filesDropped
- setSignalRegistry
- Deck
- File
- SignalRegistry
- .getMenuBarModel
- .addDeck
- .getActiveDeck
- attachTo
- queueCameraFrame
- getImageSequence
- getVideoPlayer
- .setPerTypeAutopilotConfig
- .setEffectPerformEdit
- Component
- BindableTarget
- ClipDeckResolver
- setupColumnTriggers
- setupDeckTabs
- .setEffectFenceHook
- .setEffectPerformEdit
- .render_frame
- .load_source
- .update_source_params
- .remove_route
- .set_macro
- .remove_mapping
- .load_image
- ClipPositionSignal
- mouseDown
- FeatureSnapshot
- RouteTarget
- File
- TestEffects
- paintSectionHeader
- TestSourceRegistry
- CompositorEngine::getTransitionShaderName
- handlePresetClick
- CompositorEngine::uploadAudioUniforms
- .getComposition
- .setEffectFenceHook
- .setEffectPerformEdit
- setSignalRegistry
- setupColumnTriggers
- .render_frame
- .load_source
- .update_source_params
- .remove_route
- .set_macro
- .remove_mapping
- .load_image
- .set_effect
- Deck
- EffectSlot
- File
- FrameRingBuffer
- GLuint
- MixMode
- OpenGLShaderProgram
- String
- TemporalBuffer

## God Nodes (most connected - your core abstractions)
1. `MainComponent` - 212 edges
2. `Renderer` - 181 edges
3. `Clip` - 124 edges
4. `ClipInspector` - 108 edges
5. `Layer` - 104 edges
6. `MilkDropBrowser` - 98 edges
7. `BPMTracker` - 92 edges
8. `LayerInspector` - 91 edges
9. `LayerStrip` - 82 edges
10. `CompositorEngine` - 78 edges

## Surprising Connections (you probably didn't know these)
- `PipelineRunner` --references--> `kBlockSize`  [EXTRACTED]
  tests/test_integration_pipeline.cpp → src/analysis/AnalysisThread.h
- `ReferenceOldMappingEngine` --references--> `Smoother`  [EXTRACTED]
  tests/test_mapping_engine.cpp → src/features/Smoother.h
- `clipsEq()` --references--> `Clip`  [EXTRACTED]
  tests/test_undo_commands.cpp → src/model/Clip.h
- `operator==()` --references--> `Clip`  [EXTRACTED]
  tests/test_undo_commands.cpp → src/model/Clip.h
- `richClip()` --references--> `Clip`  [EXTRACTED]
  tests/test_undo_commands.cpp → src/model/Clip.h

## Import Cycles
- None detected.

## Communities (280 total, 70 thin omitted)

### Community 0 - "MainComponent"
Cohesion: 0.01
Nodes (147): ApiServer, Array, AudioDNALookAndFeel, AudioDNAMenuBar, AudioEngine, AudioReadoutPanel, BindingManager, BindingOverlay (+139 more)

### Community 1 - "Renderer"
Cohesion: 0.02
Nodes (97): atomic, EffectChainGLState, OpenGLContext, OpenGLRenderer, promise, AnalysisThread, Composition, EffectLibrary (+89 more)

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
Cohesion: 0.08
Nodes (67): ApiServer, allowFeatureInjection_, handleComposition, handleGetBpm, handleGetFeatures, handleGetSyphon, handleHealth, handleInjectFeatures (+59 more)

### Community 6 - "Layer"
Cohesion: 0.03
Nodes (62): AutoSizeMode, KeyingMode, captureLayerClips(), captureLayerRuntime(), AutopilotAction, AutopilotDuration, EffectSlot, MixMode (+54 more)

### Community 7 - "AnalysisThread"
Cohesion: 0.04
Nodes (58): AdvancedAudioAnalyzer, AnalysisThread, advancedAnalyzer_, analysisBuffer_, bpmTracker_, chromaExtractor_, cpuLoad_, currentPeak_ (+50 more)

### Community 8 - "MilkDropBrowser"
Cohesion: 0.03
Nodes (64): deque, PresetListContent, ComboBox, Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, PlayMode (+56 more)

### Community 9 - "LayerInspector"
Cohesion: 0.03
Nodes (63): ComboBox, Component, DragAndDropTarget, EffectFenceHook, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, PerformEditFn (+55 more)

### Community 10 - "TestServer"
Cohesion: 0.08
Nodes (40): BindableTarget, ClipDeckResolver, ClipMediaHook, CompositionResolver, Graphics, KeyPress, MouseEvent, Component (+32 more)

### Community 11 - "ProjectMPresetManager"
Cohesion: 0.10
Nodes (54): Composition, Composition, EffectChain, FeatureSnapshot, Renderer, Request, Response, RoutingEngine (+46 more)

### Community 12 - "VideoRecorder"
Cohesion: 0.05
Nodes (49): PixelBuffer, AVCodecContext, AVFormatContext, AVFrame, AVPacket, AVStream, Config, File (+41 more)

### Community 13 - "MainComponent.cpp"
Cohesion: 0.08
Nodes (28): ClipDeckResolver, ClipMediaHook, DeckFenceHook, function, SetClipCmd, after_, before_, column_ (+20 more)

### Community 14 - "SwapClipsCmd"
Cohesion: 0.08
Nodes (24): Component, Composition, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Tab, TextButton, Viewport, InspectorPanel, activeTab_ (+16 more)

### Community 15 - "CompDecksBrowser"
Cohesion: 0.06
Nodes (44): CompDeckListContent, CompDecksBrowser, CompDecksBrowser::CompDeckListContent, kEntryRowHeight, kSectionHeaderHeight, CompDecksBrowser::~CompDecksBrowser(), composition_, compositions_ (+36 more)

### Community 16 - "vector"
Cohesion: 0.04
Nodes (52): array, ComboBox, Component, Composition, FeatureBus, FeatureSnapshot, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (+44 more)

### Community 17 - "TopBar"
Cohesion: 0.04
Nodes (47): ComboBox, Component, DragAndDropTarget, function, Image, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Rectangle, String (+39 more)

### Community 18 - "ShaderManager"
Cohesion: 0.09
Nodes (31): EffectRow, EffectStackView, effectLibrary_, effects_, fenceHook_, fxDropHighlight_, itemDropped, kHeaderHeight (+23 more)

### Community 19 - "LayerStrip"
Cohesion: 0.07
Nodes (37): ParamWriter, SourceType, Route, dialRangeMax, dialRangeMin, enabled, falloff, gain (+29 more)

### Community 20 - "InspectorPanel"
Cohesion: 0.06
Nodes (30): ListenerList, ComboBox, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, Listener, TextButton, ToggleButton (+22 more)

### Community 21 - "SourcesBrowser"
Cohesion: 0.06
Nodes (40): SourceEntry, SourceListContent, Component, Graphics, MouseEvent, String, CategoryInfo, Component (+32 more)

### Community 22 - "Composition"
Cohesion: 0.04
Nodes (47): AutopilotDirection, AutopilotDurationMode, CrossfaderBehaviour, CrossfaderBlendMode, CrossfaderCurve, QuantizeMode, Composition, activeDeckIndex (+39 more)

### Community 23 - "FXBrowser"
Cohesion: 0.05
Nodes (40): SourceRenderFn, CompositorEngine, accumulatorFBO_, accumulatorTex_, effectFBO_A_, effectFBO_B_, effectLibrary_, effectTex_A_ (+32 more)

### Community 24 - "EffectStackView"
Cohesion: 0.05
Nodes (40): CompositionInspector, anchorControl_, apClipLoopsSlider_, apDurationSelector_, apForwardBtn_, apLoopToggle_, apMasterLayerSelector_, apOffBtn_ (+32 more)

### Community 25 - "CompositorEngine"
Cohesion: 0.06
Nodes (44): AudioDeviceManager, BindableTarget, BindingManager, Component, Composition, Graphics, KeyPress, MidiInput (+36 more)

### Community 26 - "MidiLearnOverlay"
Cohesion: 0.12
Nodes (9): Get the current engine state (effects, FPS, etc.)., Controls Audio-DNA via the Eyes HTTP API., Get all registered procedural sources with their parameters., Get all signals with cached values., Get all active routes with current output values., Spawn the app in test mode and wait for it to become ready.          Args:, Stop the app subprocess gracefully., Check if the app is running and ready. (+1 more)

### Community 27 - "UniversalParamControl"
Cohesion: 0.05
Nodes (34): Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, SignalRegistry, SourceMode, String, TextButton (+26 more)

### Community 28 - "PipelineRunner"
Cohesion: 0.06
Nodes (39): ChromaExtractor, FFTProcessor, KeyDetector, LoudnessAnalyzer, MFCCExtractor, OnsetDetector, PitchTracker, SpectralFeatures (+31 more)

### Community 29 - "AudioDNALookAndFeel"
Cohesion: 0.09
Nodes (40): Drawable, Font, ScrollBar, AudioDNALookAndFeel, AudioDNALookAndFeel::AudioDNALookAndFeel(), drawButtonBackground, drawButtonText, drawComboBox (+32 more)

### Community 30 - "FeatureSnapshot"
Cohesion: 0.05
Nodes (41): FeatureSnapshot, bandEnergies, barCount, barPhase, beatInBar, beatPhase, bpm, chromagram (+33 more)

### Community 31 - "string"
Cohesion: 0.06
Nodes (17): Command, description, execute, undo, string, vector, LogCmd, id (+9 more)

### Community 32 - "MainComponent.h"
Cohesion: 0.09
Nodes (30): AddLayerCmd, added_, addedIndex_, deckIndex_, deckResolver_, fence_, ClipDeckResolver, ClipMediaHook (+22 more)

### Community 33 - "SignalInspector"
Cohesion: 0.08
Nodes (35): AnalysisThread, SyphonOutput, VideoRecorder, Clip, Component, FeatureBus, GLuint, Image (+27 more)

### Community 34 - "CompositionInspector"
Cohesion: 0.07
Nodes (39): Graphics, Rectangle, String, ComboBox, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Rectangle, ToggleButton (+31 more)

### Community 35 - "AdvancedAudioAnalyzer"
Cohesion: 0.05
Nodes (31): AdvancedAudioAnalyzer, bassHistory_, envelopeFull_, envelopePos_, fftSize_, formantBinHigh_, formantBinLow_, formantMax_ (+23 more)

### Community 36 - "AudioReadoutPanel"
Cohesion: 0.09
Nodes (34): CriticalSection, Event, File, string, vector, atomic, Event, vector (+26 more)

### Community 37 - "SessionRecorder"
Cohesion: 0.06
Nodes (35): FileListContent, FilesBrowser, currentDir_, decodeGeneration_, entries_, favorites_, gridView_, gridViewBtn_ (+27 more)

### Community 38 - "FilesBrowser"
Cohesion: 0.14
Nodes (38): AudioReadoutPanel, AudioReadoutPanel::AudioReadoutPanel(), displayBands_, displaySnap_, downbeatFlash_, drawBandMeters, drawBarIndicator, drawBeatPhase (+30 more)

### Community 39 - "EffectsRackPanel"
Cohesion: 0.08
Nodes (25): CategoryHeader, EffectSection, Graphics, EffectsRackPanel, activeMappingEditor_, categoryHeaders_, contentComponent_, editingEffectIndex_ (+17 more)

### Community 40 - "TextureManager"
Cohesion: 0.17
Nodes (4): atomic, Thread, Deck, Composition

### Community 41 - "TriggerClipCmd"
Cohesion: 0.08
Nodes (24): File, GLuint, LUTLoader, loadCubeFile, releaseLUT, File, GLuint, Image (+16 more)

### Community 42 - "BindingOverlay"
Cohesion: 0.07
Nodes (46): id, PresetInfo, string, vector, function, PresetInfo, string, vector (+38 more)

### Community 43 - "GenreDetector"
Cohesion: 0.07
Nodes (35): BindingManager, BindingOverlay, active_, BindingOverlay::BindingOverlay(), exitBindingMode, findExistingBinding, getKeyDescription, hitTestTarget (+27 more)

### Community 44 - "OutputRenderer"
Cohesion: 0.08
Nodes (39): FeatureSnapshot, GLState, FullscreenQuad, GLuint, ShaderManager, string, mutex, ProceduralSource (+31 more)

### Community 45 - "MidiOutputHandler"
Cohesion: 0.08
Nodes (28): Features, GenreDetector, candidateGenre_, chromaticComplexity, classify, computeEnergyState, computeScores, confidence_ (+20 more)

### Community 46 - "DeckView"
Cohesion: 0.19
Nodes (18): Clip, File, FrameRingBuffer, GLuint, CompositorEngine::applyClipEffects(), CompositorEngine::applyClipTransform(), CompositorEngine::applyMaskLayer(), CompositorEngine::createFBO() (+10 more)

### Community 47 - "EffectChainGLState"
Cohesion: 0.07
Nodes (37): Component, EffectChain, FeatureBus, File, Image, EffectChain, FeatureBus, File (+29 more)

### Community 48 - "test_signals.py"
Cohesion: 0.10
Nodes (32): FeatureSnapshot, GLint, GLuint, OpenGLShaderProgram, unique_ptr, EffectChain, addEffect, applyDryWet (+24 more)

### Community 49 - "MappingEditor"
Cohesion: 0.09
Nodes (30): MidiOutput, Array, Deck, MidiDeviceInfo, MidiMessage, PadState, String, array (+22 more)

### Community 50 - "Renderer.cpp"
Cohesion: 0.06
Nodes (29): CellPos, DeckView, activeColumn_, clipCells_, columnTriggers_, composition_, deckTabs_, gridContent_ (+21 more)

### Community 51 - "PresetSelector"
Cohesion: 0.08
Nodes (17): FeatureSnapshot, function, string, PresetSelector, barsSinceLastSwitch_, enabled_, energyMatching_, kMinBarsBetweenSwitches (+9 more)

### Community 52 - "Binding"
Cohesion: 0.08
Nodes (19): psnr_between(), Signal Routing Verification — Tests the complete signal→route→parameter→shader p, RMS should change effect output via u_rms uniform., End-to-end: create route from audio signal to effect parameter,     inject featu, Check if signal API endpoints are available., Route Volume signal → Ripple intensity → verify RMS changes ripple., Route with threshold=0.5 should only activate above 0.5 RMS., Inverted route: high RMS should DECREASE the parameter. (+11 more)

### Community 53 - "EnvelopeSignal"
Cohesion: 0.08
Nodes (27): GLHost, FeatureBus, File, Graphics, Image, Tab, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (+19 more)

### Community 54 - "RecordPanel"
Cohesion: 0.09
Nodes (21): String, Effect, addParam, category_, dryWet_, Effect::Effect(), enabled_, name_ (+13 more)

### Community 55 - "loadPreset"
Cohesion: 0.11
Nodes (29): _close_output_window(), _find_app_pids(), _find_output_windows(), _find_output_windows_any(), _is_ours(), _onscreen_windows(), _pgrep(), _process_alive() (+21 more)

### Community 56 - "OscHandler"
Cohesion: 0.07
Nodes (28): Action, CCMode, InputType, Binding, action, ccMode, ccStepSize, enabled (+20 more)

### Community 57 - "Deck"
Cohesion: 0.08
Nodes (13): ControlPoint, CurveType, EnvelopeSignal, amplitude_, beatDuration_, curveType_, looping_, oneShot_ (+5 more)

### Community 58 - "SwitchDeckCmd"
Cohesion: 0.13
Nodes (14): EffectFenceHook, PerformEditFn, inspectClip, InspectorPanel::InspectorPanel(), inspectSignal, refresh, resized, setEffectFenceHook (+6 more)

### Community 59 - "ClipInspector.cpp"
Cohesion: 0.08
Nodes (25): ComboBox, Component, File, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, TextButton, RecordPanel (+17 more)

### Community 60 - "ax_inspector.py"
Cohesion: 0.10
Nodes (29): _arm_watchdog(), _describe_pid_windows(), _discover_main_fullscreen_item(), _health_ok(), main(), _measure(), _open_output_window(), _pick_main_fullscreen_item() (+21 more)

### Community 61 - "CurveTransforms.h"
Cohesion: 0.12
Nodes (27): AccessibilityPermissionError, AmbiguousButtonError, AppNotFoundError, AXPressError, ButtonNotFoundError, collapse_signalbar(), expand_signalbar(), find_button() (+19 more)

### Community 62 - "StructuralDetector"
Cohesion: 0.18
Nodes (21): DeckState, LoadStats, Array, EffectChain, File, MappingCurve, MappingSource, String (+13 more)

### Community 63 - "test_fractals.py"
Cohesion: 0.06
Nodes (34): OSCMessage, OSCReceiver, OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>, atomic, function, OscHandler, listening_, onSetBpm (+26 more)

### Community 64 - "AudioEngine"
Cohesion: 0.11
Nodes (19): applyLayerRuntime(), LayerRuntimeSnapshot, activeClipColumn, crossfadeProgress, pendingTriggerColumn, previousClipColumn, operator==(), ClipLayerResolver (+11 more)

### Community 65 - "WaveformDisplay"
Cohesion: 0.09
Nodes (11): Deck, id, kDefaultColumns, kDefaultLayers, layers, name, nextLayerId_, numColumns (+3 more)

### Community 66 - "UndoManager"
Cohesion: 0.16
Nodes (25): applyCurve(), backIn(), backInOut(), backOut(), bounceIn(), bounceInOut(), bounceOut(), circularIn() (+17 more)

### Community 67 - "Autopilot"
Cohesion: 0.13
Nodes (20): Autopilot, advanceClip, getActionForClip, getBeatsForClip, getPerTypeAction, getPerTypeBeats, lastBeatPhase_, lastEnergyState_ (+12 more)

### Community 68 - "ClipCell"
Cohesion: 0.16
Nodes (18): getPreferredHeight, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped, mouseDown, mouseDrag, mouseUp (+10 more)

### Community 69 - "SignalStrip"
Cohesion: 0.13
Nodes (25): AccessibilityPermissionError, AppNotFoundError, AXInspectorError, check_accessibility_permissions(), find_pid_by_name(), _get_ax_attribute(), _get_position(), _get_size() (+17 more)

### Community 70 - "MFCCExtractor"
Cohesion: 0.06
Nodes (55): SettableTooltipClient, ClipCell, active_, clip_, column_, dragHover_, fileDragEnter, fileDragExit (+47 more)

### Community 71 - "ProceduralSource"
Cohesion: 0.09
Nodes (21): StructuralDetector, candidateState_, classifyState, confirmedState_, fluxAlpha_, fluxEnv_, holdCounter_, holdThreshold_ (+13 more)

### Community 72 - "MidiHandler"
Cohesion: 0.15
Nodes (20): string, unique_ptr, function, unique_ptr, vector, UndoManager, canRedo, canUndo (+12 more)

### Community 73 - "SignalRegistry"
Cohesion: 0.11
Nodes (17): frame_is_not_black(), frames_are_different(), Eyes Visual Tests — Comprehensive Fractal Source Validation  Tests EVERY control, Every fractal must render a visible frame at defaults., Every parameter must produce a visible change when modified., Zoom at ALL positions (0, 0.25, 0.5, 0.75, 1.0) must be non-black., Dive speed at various levels must not go black., Power at all positions must not go black. (+9 more)

### Community 74 - "MilkDropBrowser.cpp"
Cohesion: 0.09
Nodes (21): AudioFormatManager, AudioFormatReaderSource, AudioSourcePlayer, AudioTransportSource, ChangeListener, CombinedCallback, AudioEngine, audioCallback_ (+13 more)

### Community 75 - "Command"
Cohesion: 0.09
Nodes (22): Column, kWaveformBufferSize, Graphics, array, Component, kMaxColumns, Timer, WaveformDisplay (+14 more)

### Community 76 - "SpectralFeatures"
Cohesion: 0.12
Nodes (18): DeckView, function, DeckView, Composition, Deck, Renderer, UndoService, composition_ (+10 more)

### Community 77 - "PreferencesDialog.cpp"
Cohesion: 0.09
Nodes (23): EffectChain, FeatureSnapshot, MappingCurve, MappingSource, vector, MappingEngine, addMapping, applyCurve (+15 more)

### Community 78 - "MacroPanel"
Cohesion: 0.18
Nodes (14): MouseEvent, string, vector, addToRecent, buildPlaylistDragDescription, calculateContentHeight, firePresetSelected, getCuratedPresets (+6 more)

### Community 79 - "ShaderManager.cpp"
Cohesion: 0.09
Nodes (22): DisplaySize, MouseEvent, Component, DisplaySize, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, SignalRegistry, SignalStrip (+14 more)

### Community 80 - ".runFenced"
Cohesion: 0.13
Nodes (20): MacroSlot, Graphics, array, Component, function, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, SignalRegistry, MacroPanel (+12 more)

### Community 81 - "TimingWindow"
Cohesion: 0.12
Nodes (20): MelFilter, array, MFCCExtractor, buildDCTMatrix, buildFilterbank, dctMatrix_, fftSize_, filterWeightOffsets_ (+12 more)

### Community 82 - "Knob"
Cohesion: 0.09
Nodes (18): NSObject, atomic, string, SyphonOutput, enabled_, impl_, init, initialized_ (+10 more)

### Community 83 - "LayerInspector.cpp"
Cohesion: 0.05
Nodes (53): Param, SourceFactory, SourceInfo, SourceRegistry, FeatureSnapshot, GLuint, OpenGLShaderProgram, string (+45 more)

### Community 84 - "BindingManager"
Cohesion: 0.13
Nodes (10): Category, string, Type, Signal, category_, getValue, id_, name_ (+2 more)

### Community 85 - "ThumbnailCache"
Cohesion: 0.12
Nodes (22): Array, AudioDeviceManager, BindingManager, MidiDeviceInfo, MidiInput, MidiMessage, String, AudioDeviceManager (+14 more)

### Community 86 - "ToggleLayerFlagCmd"
Cohesion: 0.12
Nodes (20): Category, FeatureSnapshot, string, unique_ptr, vector, unique_ptr, vector, SignalRegistry (+12 more)

### Community 87 - "KeyDetector"
Cohesion: 0.26
Nodes (5): Component, Graphics, PresetInfo, Section, MilkDropBrowser::PresetListContent

### Community 88 - "EffectStackCmd"
Cohesion: 0.10
Nodes (15): BandRange, array, SpectralFeatures, bandMaxEnergy_, bandRanges_, computeBandBinRanges, fftSize_, fluxMax_ (+7 more)

### Community 89 - "Effect"
Cohesion: 0.12
Nodes (16): DialogWindow, Component, function, Graphics, Rectangle, Tab, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, PreferencesDialog (+8 more)

### Community 90 - "EffectLibrary"
Cohesion: 0.09
Nodes (22): EffectEntry, FXListContent, FXBrowser, categories_, effectLibrary_, effects_, kCategoryHeaderHeight, kEffectRowHeight (+14 more)

### Community 91 - "MappingEngine"
Cohesion: 0.33
Nodes (10): ProceduralSource, SourceParam, string, vector, compileShaderWithUtils, getOrCreateSource, getOrCreateSourceOnGLThread, renderSource (+2 more)

### Community 92 - "MilkDropBrowser::PresetListContent"
Cohesion: 0.12
Nodes (21): ProgramEntry, unordered_map, File, GLint, OpenGLContext, OpenGLShaderProgram, String, File (+13 more)

### Community 93 - "SignalBar"
Cohesion: 0.16
Nodes (16): EffectDef, String, StringArray, unique_ptr, EffectLibrary, createEffect, defs_, getEffectDef (+8 more)

### Community 94 - "SpectrumDisplay"
Cohesion: 0.17
Nodes (19): Colour, PlayMode, ProjectMPresetManager, SubTab, getPlaylistBlendSeconds, getPlaylistCycleModeId, getPlaylistTriggerBeats, initSections (+11 more)

### Community 95 - "UniversalParamControl.cpp"
Cohesion: 0.12
Nodes (18): Graphics, String, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Label, String, Knob, kPreferredHeight (+10 more)

### Community 96 - "RoutingEngine"
Cohesion: 0.10
Nodes (20): FeatureBus, Graphics, array, Component, FeatureBus, Timer, uint32, SpectrumDisplay (+12 more)

### Community 97 - "BPMTracker.cpp"
Cohesion: 0.11
Nodes (27): ActionCallback, BindingCaptureCallback, BindingManager, actionCallback_, addBinding, bindingMode_, bindings_, captureCallback_ (+19 more)

### Community 98 - "Mapping"
Cohesion: 0.15
Nodes (14): Entry, iterator, Key, KeyHash, list, File, Image, Time (+6 more)

### Community 99 - "Signal"
Cohesion: 0.11
Nodes (15): array, KeyDetector, candidateCount_, candidateKey_, candidateMajor_, kHysteresisFrames, majorProfile_, minorProfile_ (+7 more)

### Community 100 - "BrowserPanel"
Cohesion: 0.19
Nodes (15): EffectStackCmd, after_, before_, compResolver_, fence_, refresh_, scope_, Composition (+7 more)

### Community 101 - "test_mapping_tick.py"
Cohesion: 0.08
Nodes (34): DisplaySize, FeatureBus, Graphics, SignalRegistry, Component, DisplaySize, FeatureBus, FeatureSnapshot (+26 more)

### Community 102 - "FFTProcessor"
Cohesion: 0.25
Nodes (10): PopupMenu, String, buildSourcePickerMenu, getPreferredHeight, handleSourcePickerResult, onSourceChanged, resized, setParamName (+2 more)

### Community 103 - "AudioDNAMenuBar"
Cohesion: 0.28
Nodes (8): buildSourceParamControls, ClipInspector::ClipInspector(), onCuepointSet, onSourceParamsChanged, refresh, setClip, syncFromClip, updateTransportHighlights

### Community 104 - "Colour"
Cohesion: 0.16
Nodes (18): analyzeDownbeatPosition, correctOctaveError, feedDownbeatFeatures, feedSilenceDetection, foldBPMToRange, process, processRawBPM, pushAndMedian (+10 more)

### Community 105 - "SourceRegistry"
Cohesion: 0.11
Nodes (19): BrowserPanel, activeTab_, compDecksBrowser_, compDecksTabBtn_, filesBrowser_, filesTabBtn_, fxBrowser_, fxTabBtn_ (+11 more)

### Community 106 - "Route"
Cohesion: 0.14
Nodes (17): AudioDNAMenuBar, getMenuBarNames, getMenuForIndex, getRedoState, getUndoState, hasClipSelection, isSyphonOutputEnabled, menuItemSelected (+9 more)

### Community 107 - "FeatureBus"
Cohesion: 0.11
Nodes (18): setActive, updateTabButtonColors, Graphics, Tab, Component, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, Tab, TextButton (+10 more)

### Community 108 - "FeedbackProcessor"
Cohesion: 0.14
Nodes (19): _assert_tracking(), mapped_param(), _pick_target(), MappingTick 4-state param-tracking probe (outputwindow-arc-design.md W7(ii)).  P, Set up content + one RMS->param mapping; tear both down after., Two full rms flip cycles; assert the param follows each edge., State 1: normal layout, no output window., State 2: normal layout + output window open. (+11 more)

### Community 109 - "LinkSync"
Cohesion: 0.18
Nodes (8): aubio_onset_t, fvec_t, OnsetDetector, hopSize_, input_, onset_, output_, process

### Community 110 - "FilesBrowser.cpp"
Cohesion: 0.12
Nodes (13): FFT, FFTProcessor, buildHannWindow, fft_, fftData_, FFTProcessor::FFTProcessor(), hannWindow_, kFFTOrder (+5 more)

### Community 111 - "ProjectMSource"
Cohesion: 0.19
Nodes (12): CaretOnlyComboBoxLookAndFeel, Button, Colour, ComboBox, Graphics, Label, LookAndFeel_V4, Slider (+4 more)

### Community 112 - "LoudnessAnalyzer"
Cohesion: 0.18
Nodes (6): RemoveDeckCmd, compResolver_, deckIndex_, priorActiveIndex_, removed_, mediaHook_

### Community 113 - "UndoService"
Cohesion: 0.15
Nodes (17): FeatureSnapshot, Writer, FeatureBus, createWriter, kMaxReadAttempts, kOddSeqSpinLimit, kSnapshotWords, loadStableSeq (+9 more)

### Community 114 - "OscillatorSignal"
Cohesion: 0.07
Nodes (30): FeedbackConfig, amount, enabled, lumaKey, offsetX, offsetY, presetName, rotation (+22 more)

### Community 115 - "MidiLearnOverlay.cpp"
Cohesion: 0.16
Nodes (16): Graphics, Listener, MappingCurve, MappingSource, String, getCurveName(), getSourceName(), addListener (+8 more)

### Community 116 - "ClipCell.cpp"
Cohesion: 0.12
Nodes (24): Graphics, MouseEvent, Rectangle, SignalRegistry, SourceDetails, String, getPreferredHeight, isInterestedInDragSource (+16 more)

### Community 117 - "_make_mock_element"
Cohesion: 0.13
Nodes (12): BiquadState, BiquadState, LoudnessAnalyzer, fillCount_, process, processBiquad, runningSum_, stage1_ (+4 more)

### Community 118 - "test_render_pipeline.py"
Cohesion: 0.12
Nodes (8): FeatureSnapshot, string, OscillatorSignal, amplitude_, beatDuration_, phaseOffset_, shape_, WaveShape

### Community 119 - "test_effects.py"
Cohesion: 0.12
Nodes (16): onClipMoved, onClipSelected, onClipTriggered, onFileDropped, onLayerBypass, onLayerClearClip, onLayerEffectDropped, onLayerSelected (+8 more)

### Community 120 - "test_range_quality.py"
Cohesion: 0.27
Nodes (16): File, Image, Time, FilesBrowser::~FilesBrowser(), filterBySearch, generateThumbnail, isMediaFile, loadFavorites (+8 more)

### Community 121 - "VJAppController"
Cohesion: 0.11
Nodes (13): Eyes Visual Tests — Render Pipeline  Tests the core rendering pipeline: image lo, Verify audio feature injection works., Injecting features should succeed., Verify state endpoint works., State endpoint should list all effects., Verify reset clears state properly., After reset, no effects should be enabled., Verify the test server is responsive. (+5 more)

### Community 122 - "MacroBank"
Cohesion: 0.19
Nodes (20): CellEdit, Command, DeckFenceHook, optional, Clip, File, String, unique_ptr (+12 more)

### Community 123 - "ChromaExtractor"
Cohesion: 0.23
Nodes (17): Deck, EffectSlot, FullscreenQuad, Layer, ShaderManager, CompositorEngine::applyFXOnlyLayer(), CompositorEngine::applyLayerKeying(), CompositorEngine::applyLayerTransform() (+9 more)

### Community 124 - "rebuildGrid"
Cohesion: 0.14
Nodes (13): all_effects(), brightness(), image_loaded(), psnr_between(), Auto-Discovering Effect Verification — Tests ALL registered effects.  Queries /a, No effect should turn the image completely black or white., Discover all effects from the running app., Ensure test image is loaded. (+5 more)

### Community 125 - "MappingEditor.cpp"
Cohesion: 0.17
Nodes (14): analyze_sweep(), brightness(), Tier 2: Range Quality Analysis  For each source parameter, renders at 11 positio, Sweep every parameter and verify quality metrics., Each parameter must have >70% useful range, no discontinuities, no dead zones., Auto-discover all sources and sweep all their params.      This class discovers, Sweep every param on every source. Generates CSV reports., Render a source at multiple parameter positions, return list of (value, path) tu (+6 more)

### Community 126 - "makeSetClipCmd"
Cohesion: 0.10
Nodes (12): Macro, Scope, Array, SignalRegistry, array, SignalRegistry, MacroBank, kNumMacros (+4 more)

### Community 127 - "PreviewPanel"
Cohesion: 0.15
Nodes (12): ChromaExtractor, binToChroma_, ChromaExtractor::ChromaExtractor(), computeBinToChromaMap, fftSize_, hasPrevFrame_, kNumChroma, numBins_ (+4 more)

### Community 128 - "TestSignalRouteEndToEnd"
Cohesion: 0.17
Nodes (8): Composition, CompositionResolver, DeckActivateHook, SwitchDeckCmd, activate_, after_, before_, compResolver_

### Community 129 - "ClearActiveClipCmd"
Cohesion: 0.18
Nodes (14): CompositionInspector::CompositionInspector(), getPreferredHeight, isInterestedInDragSource, itemDragEnter, itemDragExit, itemDropped, rebuildEffectStack, refresh (+6 more)

### Community 130 - "FeedbackConfig"
Cohesion: 0.18
Nodes (14): clipsEq(), Deck, EffectSlot, optional, PresetEntry, SourceParam, string, T (+6 more)

### Community 131 - "SyphonOutput"
Cohesion: 0.16
Nodes (10): Component, MouseEvent, Point, fileListContent_, dragStarted_, kLabelHeight, kListRowHeight, kPadding (+2 more)

### Community 132 - "CompositionInspector.cpp"
Cohesion: 0.15
Nodes (11): paint, refresh, resized, setActiveTab, setComposition, setEffectLibrary, showActiveTab, updateTabButtonColors (+3 more)

### Community 133 - "DeckView.cpp"
Cohesion: 0.15
Nodes (14): AudioEngine::AudioEngine(), getCurrentSampleRate, getDeviceStatus, hasAudioDevice, isPlaying, loadFile, onError, pause (+6 more)

### Community 134 - "fileListContent_"
Cohesion: 0.16
Nodes (13): Composition, Graphics, clearSelection, getNaturalHeight, layoutGrid, paint, refresh, resized (+5 more)

### Community 135 - "LayerStrip.cpp"
Cohesion: 0.15
Nodes (18): SliderLayout, SourceDetails, FadeSpeedSliderLookAndFeel, FullBoundsSliderLAF, KeyingSliderLookAndFeel, isInterestedInDragSource, itemDragEnter, itemDragExit (+10 more)

### Community 136 - "SignalBar.cpp"
Cohesion: 0.14
Nodes (8): Tests for the argparse-based CLI entry point., --help prints usage and exits without calling inspect_app., Exit code 2 when app is not found., Exit code 1 when permissions denied., Default output goes to stdout as valid JSON., --output writes JSON to a file., --depth flag is forwarded to inspect_app., TestCLI

### Community 137 - "TestCLI"
Cohesion: 0.18
Nodes (11): all_sources(), brightness(), psnr_between(), Auto-Discovering Source Verification — Tests ALL registered procedural sources., Sweep critical params across 5 positions to check for discontinuities., Discover all sources from the running app., Every registered source must render a non-black frame at defaults., Every param on every source must have a visible effect. (+3 more)

### Community 138 - "test_sources.py"
Cohesion: 0.16
Nodes (13): Display, OutputComponent, enterBindingMode, KeyPress, DocumentWindow, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, OutputWindow, closeButtonPressed (+5 more)

### Community 139 - "AudioCallback"
Cohesion: 0.16
Nodes (8): Flag, ToggleLayerFlagCmd, after_, before_, deckIndex_, flag_, layerIndex_, resolver_

### Community 140 - "OutputWindow"
Cohesion: 0.18
Nodes (8): AddDeckCmd, added_, addedIndex_, compResolver_, fence_, priorActiveIndex_, Deck, deckResolver_

### Community 141 - "AddDeckCmd"
Cohesion: 0.20
Nodes (9): DocumentWindow, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, String, unique_ptr, getApplicationName(), getApplicationVersion(), initialise(), MainWindow (+1 more)

### Community 142 - "Main.cpp"
Cohesion: 0.18
Nodes (9): Component, MouseEvent, String, FXBrowser::FXListContent, draggedEffectName_, dragStarted_, kCategoryHeaderHeight, kEffectRowHeight (+1 more)

### Community 143 - "PreviewPanel.cpp"
Cohesion: 0.10
Nodes (16): vector, Deck, optional, string, File, needsVideoReopen(), Composition, set (+8 more)

### Community 144 - "operator=="
Cohesion: 0.12
Nodes (12): aubio_pitch_t, AnalysisThread::AnalysisThread(), getPCMSamples, getWaveformSamples, run, fvec_t, PitchTracker, hopSize_ (+4 more)

### Community 145 - "OnsetDetector"
Cohesion: 0.39
Nodes (10): ISFShader, File, string, ISFShaderLoader, convertToGLSL, extractGLSLBody, extractJSONBlock, getISFDirectory (+2 more)

### Community 146 - "PitchTracker"
Cohesion: 0.18
Nodes (7): atomic, T, RingBuffer, buffer_, mask_, readPos_, writePos_

### Community 147 - "ISFShaderLoader.cpp"
Cohesion: 0.19
Nodes (11): Graphics, MouseEvent, SourceDetails, getPreferredHeight, isInterestedInDragSource, itemDragEnter, itemDragExit, mouseDown (+3 more)

### Community 148 - "EffectScope"
Cohesion: 0.19
Nodes (5): CompositeCommand, children_, string, unique_ptr, vector

### Community 149 - "RingBuffer"
Cohesion: 0.09
Nodes (22): MappingCurve, MappingSource, Mapping, curve, enabled, inputMax, inputMin, outputMax (+14 more)

### Community 151 - "CompositeCommand"
Cohesion: 0.36
Nodes (12): Colour, Graphics, Rectangle, SignalRegistry, String, getFormattedValue, getSignalColour, paint (+4 more)

### Community 152 - "BrowserPanel.cpp"
Cohesion: 0.23
Nodes (8): FullscreenQuad, draw, init, initialized_, release, vao_, vbo_, GLuint

### Community 154 - "Smoother"
Cohesion: 0.17
Nodes (12): Composition, FeatureBus, handleMultiplierButton, onBpmMultiplierChanged, onManualBpmChanged, onPause, onPlay, onQuantizeChanged (+4 more)

### Community 156 - "ClearLayerClipsCmd"
Cohesion: 0.17
Nodes (11): app(), _audio_dna_running(), _ensure_ax_mocks(), _mock_copy_attribute(), Tests for the accessibility tree inspector.  Unit tests mock the AX API so they, Mock for AXUIElementCopyAttributeValue that reads from element._attrs., Install mock modules for ApplicationServices and Cocoa if needed., Check if Audio-DNA is running (for integration test gating). (+3 more)

### Community 157 - "TopBar::TopBar"
Cohesion: 0.19
Nodes (10): AudioIODevice, AudioIODeviceCallback, AudioIODeviceCallbackContext, AudioCallback, AudioCallback::AudioCallback(), audioDeviceAboutToStart, audioDeviceIOCallbackWithContext, audioDeviceStopped (+2 more)

### Community 158 - "test_ax_inspector.py"
Cohesion: 0.20
Nodes (7): ClearActiveClipCmd, after_, before_, deckIndex_, layerIndex_, resolver_, ClipLayerResolver

### Community 160 - "itemDropped"
Cohesion: 0.15
Nodes (14): TextButton, LayerStrip::LayerStrip(), onBlendModeChanged, onBypass, onClearClip, onSolo, onTransportBack, onTransportForward (+6 more)

### Community 161 - "LayerStrip::LayerStrip"
Cohesion: 0.22
Nodes (8): psnr_between(), Audio Reactivity Verification — Tests that injected audio features change visual, Audio features should visibly change source output., RMS, bass, and beat phase should affect most sources that use u_rms/u_beatPhase., Audio features should change effect output when effects use audio uniforms., Effects that use u_rms should respond to RMS changes., TestAudioFeaturesAffectEffects, TestAudioFeaturesAffectSources

### Community 162 - "SignalStrip.cpp"
Cohesion: 0.14
Nodes (10): Kind, EffectScope, column, deckIndex, kind, layerIndex, EffectSlot, vector (+2 more)

### Community 163 - "test_audio_reactivity.py"
Cohesion: 0.24
Nodes (5): MoveLayerCmd, deckIndex_, fence_, fromIndex_, toIndex_

### Community 166 - "ClipPositionSignal"
Cohesion: 0.24
Nodes (6): AudioSignal, source_, Category, FeatureSnapshot, MappingSource, string

### Community 169 - "conftest.py"
Cohesion: 0.31
Nodes (9): Graphics, paint, paintBarPhraseDisplay, paintBeatWheel, resized, setDspLoad, setFps, timerCallback (+1 more)

### Community 170 - "TestAppLookup"
Cohesion: 0.24
Nodes (8): array, atomic, uint32_t, WaveformSeqlock, buffer_, count_, kSize, seq_

### Community 171 - "test_performance.py"
Cohesion: 0.22
Nodes (8): app(), _default_executable(), Pytest configuration for Eyes visual tests.  Provides fixtures that spawn the Au, Find the built executable., Spawn Audio-DNA in test mode for the entire test session.      The app starts on, Reset app state before each test for isolation., reset_between_tests(), VJ App Controller — Python client for the Eyes test harness HTTP API.  Wraps the

### Community 172 - "vision_check.py"
Cohesion: 0.25
Nodes (5): Tests for the --depth recursion limiter., Depth 0 returns the root element without walking children., Depth 1 walks immediate children but not grandchildren., Depth -1 walks the full tree., TestDepthLimiting

### Community 173 - "fence_"
Cohesion: 0.20
Nodes (6): Tests for application discovery and error handling., Raises AccessibilityPermissionError when AX permissions are denied., Raises AppNotFoundError when the app is not running., inspect_app with explicit PID skips name lookup., inspect_app finds PID by app name when pid is not provided., TestAppLookup

### Community 174 - "RouteTarget"
Cohesion: 0.22
Nodes (10): _as_applescript_string(), _click_output_item(), _osascript(), _output_menu_items(), _press_escape(), Quote a Python string for embedding in AppleScript source., Run an AppleScript snippet and return its stdout.      Args:         script: App, Read the live item names of the Output menu.      The fullscreen item's label is (+2 more)

### Community 175 - "CaretOnlyComboBoxLookAndFeel"
Cohesion: 0.20
Nodes (5): Performance Verification — Tests that sources and effects render within budget., Every source must render within budget., Effects should not significantly slow down rendering., TestEffectPerformance, TestSourcePerformance

### Community 176 - "scan_all_presets.py"
Cohesion: 0.31
Nodes (8): ndarray, compute_psnr(), compute_ssim(), Vision Check — Image comparison for the Eyes visual testing harness.  Compares r, Compute Peak Signal-to-Noise Ratio between two images.      Returns float('inf'), Compute Structural Similarity Index between two images.      Uses scikit-image's, Compare a rendered frame against a golden reference.      Args:         rendered, verify_frame()

### Community 178 - "SyphonOutputImpl"
Cohesion: 0.22
Nodes (8): ClearLayerClipsCmd, after_, before_, deckIndex_, fence_, layerIndex_, mediaHook_, resolver_

### Community 179 - "fromVar"
Cohesion: 0.22
Nodes (6): SetColumnCountCmd, after_, before_, deckIndex_, deckResolver_, fence_

### Community 180 - "ResettableSlider"
Cohesion: 0.28
Nodes (6): fromVar, toVar, var, var, fromVar, toVar

### Community 181 - "TestIntegration"
Cohesion: 0.20
Nodes (8): ClipLayerResolver, ToggleClipLockCmd, after_, before_, column_, deckIndex_, layerIndex_, resolver_

### Community 182 - "TestFrameCapture"
Cohesion: 0.33
Nodes (7): BPMTracker, feedConstantBPM(), feedWithBeats(), BPMTracker, feedBeatWithFeatures(), feedNonBeatHops(), lockBPM()

### Community 183 - "test_time_sweep.py"
Cohesion: 0.31
Nodes (7): classify_vibe(), load_progress(), main(), Load progress from previous run., Save progress for resume., Analyze a rendered frame and classify into a vibe category.     Returns (vibe, s, save_progress()

### Community 184 - "UndoService.cpp"
Cohesion: 0.33
Nodes (8): load_preset(), main(), Load a MilkDrop preset via the test API., Capture a rendered frame., Score a rendered image on visual interest (0-100).      Criteria:     - Non-blac, render_frame(), reset(), score_image()

### Community 185 - ".mcp.json"
Cohesion: 0.22
Nodes (9): _assert_normal_level(), LevelMismatch, _main_display_bounds(), Exception, Assert the output window sits at NSNormalWindowLevel.      Raises:         Level, Raised when the window exists but is at the wrong layer (exit 1)., Raised by SIGALRM when the measurement phase exceeds RUN_TIMEOUT_S., Width/height of the main display, for diagnostic comparison only. (+1 more)

### Community 186 - ".paintGrid"
Cohesion: 0.25
Nodes (5): MouseEvent, Slider, ResettableSlider, defaultVal_, hasDefault_

### Community 187 - "final_default_validation.py"
Cohesion: 0.36
Nodes (11): EffectChain, closeMappingEditor, EffectsRackPanel::EffectsRackPanel(), findMappingForParam, isEffectLocked, mappingEditorCloseRequested, mappingEditorDeleteRequested, openMappingEditor (+3 more)

### Community 188 - "AnalysisThread.cpp"
Cohesion: 0.24
Nodes (7): _make_mock_element(), Tests for the recursive tree walker., Walk a leaf element with no children., Walk a tree with nested children., Walk an element with no attributes set., Build a mock AXUIElement with attribute lookup support.      Args:         role:, TestWalkElement

### Community 189 - "EffectParam"
Cohesion: 0.25
Nodes (5): Integration tests that connect to a live Audio-DNA instance., Read the real accessibility tree and verify basic structure., A running JUCE app should have at least one window child., Live tree serializes to JSON and parses back correctly., TestIntegration

### Community 190 - "MouseEvent"
Cohesion: 0.25
Nodes (5): Verify basic frame capture works., With no image loaded, capture should produce a frame (may be black)., Loading an image and capturing should produce a non-empty PNG., Two renders at the same time should produce identical frames., TestFrameCapture

### Community 191 - ".fromVar"
Cohesion: 0.32
Nodes (5): brightness(), psnr_between(), Time Sweep Verification — Tests that animated sources/effects change over time., Animated sources must produce different frames at different times., TestSourcesAnimateOverTime

### Community 192 - "File"
Cohesion: 0.29
Nodes (6): /opt/homebrew/bin/codegraph, /Users/boriskarpman/.local/bin/clangd-mcp, /Users/boriskarpman/.local/share/uv/tools/graphifyy/bin/python3, clangd-rta, codegraph-rta, graphify-rta

### Community 195 - "TestJsonSerialization"
Cohesion: 0.48
Nodes (4): FileEntry, Graphics, vector, paint

### Community 196 - "TestEffects"
Cohesion: 0.38
Nodes (6): compute_psnr(), main(), Render clean baseline with no effects., Enable effect with explicit default params, render, compare to baseline., render_baseline(), test_effect_visual()

### Community 198 - "getThumbnailBounds"
Cohesion: 0.47
Nodes (6): paint, paintSectionHeader, paintTimeline, Graphics, Rectangle, String

### Community 199 - "isInThumbnailArea"
Cohesion: 0.33
Nodes (5): File, Image, String, makeImage(), makeTempFile()

### Community 200 - "paintSectionHeader"
Cohesion: 0.33
Nodes (4): Tests for JSON output correctness., The walk result serializes to valid JSON., A nested tree round-trips through JSON correctly., TestJsonSerialization

### Community 202 - "drawSignalTriangle"
Cohesion: 0.08
Nodes (20): Autopilot, CompositorEngine, EffectChain, MappingEngine, PresetSelector, RoutingEngine, SourceRegistry, EffectLibrary (+12 more)

### Community 204 - "TestSourceRegistry"
Cohesion: 0.23
Nodes (8): Graphics, buildCategoryList, FXBrowser::~FXBrowser(), paint, refresh, resized, setEffectLibrary, toggleCategory

### Community 205 - "test_spectral_features.cpp"
Cohesion: 0.50
Nodes (5): MouseEvent, Point, mouseDown, mouseDrag, scrubPlayhead

### Community 206 - "beatSyncRandomize"
Cohesion: 0.50
Nodes (4): Graphics, Rectangle, drawSignalTriangle, paint

### Community 211 - "test_bpm_stabilization.cpp"
Cohesion: 0.50
Nodes (4): _default_app_bundle(), Absolute path of the repo root (this file lives in tests/visual/)., Default .app bundle path (the Release artefact conftest.py also uses)., _repo_root()

### Community 212 - "Composition"
Cohesion: 0.67
Nodes (3): psnr(), Test if an effect with defaults produces visible change. Returns PSNR., test_effect()

### Community 213 - "verify_defaults.py"
Cohesion: 0.67
Nodes (3): ChangeBroadcaster, changeListenerCallback, onTransportStateChanged

### Community 215 - "paint"
Cohesion: 0.13
Nodes (23): Binding, ClipLayerResolver, DeckActivateHook, Deck, function, fastSave, getFastSaveDir, handleBindingAction (+15 more)

### Community 224 - ".getActiveDeck"
Cohesion: 0.17
Nodes (6): Configure the entire effect chain.          Disables all existing effects, then, Inject synthetic audio features into the FeatureBus.          Args:, Reset all effects, clear images, restore defaults., Create a signal→parameter route.          Args:             route: Dict with key, Create an RMS→param mapping (test-mode enabler, TestServer only).          Args:, Send a POST request with JSON body.

### Community 229 - ".setPerTypeAutopilotConfig"
Cohesion: 0.20
Nodes (9): SessionRecorder, Graphics, onPlayRecording, onStartRecording, onStopRecording, paint, RecordPanel::RecordPanel(), refresh (+1 more)

### Community 246 - "ClipPositionSignal"
Cohesion: 0.20
Nodes (5): ClipPositionSignal, currentPosition_, atomic, FeatureSnapshot, getClipPositionSignal

### Community 247 - "mouseDown"
Cohesion: 0.24
Nodes (10): MouseEvent, mouseDown, onExpandToggled, onInvertChanged, onRangeChanged, onValueChanged, setExpanded, setParamValue (+2 more)

### Community 248 - "FeatureSnapshot"
Cohesion: 0.28
Nodes (3): FeatureBus, FeatureSnapshot, RoutingEngine

### Community 249 - "RouteTarget"
Cohesion: 0.22
Nodes (7): RouteTarget, clipId, effectIndex, layerId, paramIndex, scope, TargetScope

### Community 250 - "File"
Cohesion: 0.33
Nodes (7): File, captureFrame, getVideoPlayerFile, loadImage, openImageSequenceForClip, openVideoForClip, takeSnapshot

### Community 252 - "TestEffects"
Cohesion: 0.33
Nodes (4): Two effects chained should both apply., Verify effects can be enabled and produce visible changes., Enabling an effect should change the rendered output., TestEffects

### Community 253 - "paintSectionHeader"
Cohesion: 0.40
Nodes (5): paint, paintSectionHeader, Graphics, Rectangle, String

### Community 255 - "CompositorEngine::getTransitionShaderName"
Cohesion: 0.67
Nodes (3): MixMode, CompositorEngine::getTransitionShaderName(), String

## Knowledge Gaps
- **1636 isolated node(s):** `lookAndFeel_`, `presetManager_`, `ringBuffer_`, `audioEngine_`, `analysisThread_` (+1631 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **70 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Clip` connect `Clip` to `FeedbackConfig`, `ClipInspector`, `ApiServer`, `Layer`, `ProjectMPresetManager`, `MainComponent.cpp`, `PreviewPanel.cpp`, `MainComponent.h`, `SignalStrip.cpp`, `MappingEditor`, `ResettableSlider`, `TestIntegration`, `SwitchDeckCmd`, `WaveformDisplay`, `Autopilot`, `MFCCExtractor`, `SpectralFeatures`, `AudioDNAMenuBar`, `ClipPositionSignal`, `.getActiveClip`, `makeSetClipCmd`?**
  _High betweenness centrality (0.128) - this node is a cross-community bridge._
- **Why does `ThumbnailCache` connect `Mapping` to `isInThumbnailArea`, `SessionRecorder`, `PreviewPanel.cpp`?**
  _High betweenness centrality (0.125) - this node is a cross-community bridge._
- **Why does `MainComponent` connect `MainComponent` to `SignalInspector`, `drawSignalTriangle`, `TestServer`, `SpectralFeatures`, `PreviewPanel.cpp`, `MoveLayerCmd`, `paint`, `MacroBank`, `makeSetClipCmd`?**
  _High betweenness centrality (0.103) - this node is a cross-community bridge._
- **What connects `lookAndFeel_`, `presetManager_`, `ringBuffer_` to the rest of the system?**
  _1852 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `MainComponent` be split into smaller, more focused modules?**
  _Cohesion score 0.013605442176870748 - nodes in this community are weakly interconnected._
- **Should `Renderer` be split into smaller, more focused modules?**
  _Cohesion score 0.018018018018018018 - nodes in this community are weakly interconnected._
- **Should `Clip` be split into smaller, more focused modules?**
  _Cohesion score 0.02499247214694369 - nodes in this community are weakly interconnected._