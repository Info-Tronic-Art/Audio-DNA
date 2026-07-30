# Feature Map — Audio-DNA (RealTimeAudio)

Generated: 2026-05-24 | Re-verified: 2026-07-16 (7-lane source audit, HEAD 9139dd4) | Graph: 4,231 nodes, 13,665 edges (committed d4d2c83, 2026-05-19; NOT regenerated 2026-07-16 — zero code delta; working-tree drift reverted)

**Verified counts (2026-07-16):** 135 effects / 11 categories / 333 params · 15 transitions (+1 deck transition) · 108 sources / 18 categories / 759 params · 30 audio features / 14-stage pipeline · 58 mapping sources / 24 curves · 22 REST endpoints (21 functional) · 11 OSC patterns (subsystem inert) · 113/113 unit tests PASS.

**Ghost/dead entry points listed below (kept for graph fidelity — flagged in `.harmony/APP-INVENTORY.md` §8):** `MappingSuggester::suggestMappings()` (never instantiated), `ChainedSignal` (never instantiated), `SyphonOutput::publishTexture()` (never called), `GenreSmoothing` (dead).

## Features → Graph Communities

| Feature | Communities | God Nodes | Entry Points |
|---------|------------|-----------|--------------|
| Audio I/O & Capture | C35 (AudioEngine), C40 (AudioCallback), C109 (RingBuffer) | AudioEngine() | AudioCallback::audioDeviceIOCallbackWithContext() |
| Audio Analysis Pipeline | C9 (AnalysisThread), C16 (BPMTracker), C19 (GenreDetector), C23 (MFCCExtractor), C24 (ChromaExtractor), C25 (SpectralFeatures), C27 (OnsetDetector), C28 (KeyDetector), C30 (LoudnessAnalyzer), C32 (FFTProcessor), C33 (PitchTracker), C34 (StructuralDetector), C56 (AdvancedAudioAnalyzer) | AnalysisThread::run() | AnalysisThread::processHop() |
| Feature Transport | C38 (FeatureBus), C39 (Smoother) | FeatureBus::publishWrite() | FeatureBus::acquireRead() |
| Visual Effects System | C11 (EffectLibrary), C12 (EffectChain), C13 (Effect), C14 (UniformBridge), C55 (ISFShaderLoader) | EffectLibrary::registerAll() | EffectChain::render() |
| Render Pipeline | C1 (Renderer), C2 (CompositorEngine), C15 (ShaderManager), C36 (TextureManager) | Renderer::renderOpenGL(), CompositorEngine::compositeDeck() | Renderer::renderOpenGL() |
| Audio-Visual Mapping | C10 (MappingEngine), C42 (MappingSuggester), C43 (CurveTransforms) | MappingEngine::processFrame() | MappingEngine::processFrame() |
| Signal Routing | C17 (SignalRegistry), C18 (RoutingEngine), C20 (ChainedSignal) | SignalRegistry::evaluateAll() | RoutingEngine::processFrame() |
| Clip & Layer Composition | C37 (Clip), C41 (Layer), C44 (Composition), C45 (Deck), C46 (UndoManager) | Composition::initDefault() | Layer::triggerClip() |
| Procedural Sources | C22 (SourceRegistry), C21 (ProceduralSource) | SourceRegistry::registerAll() | ProceduralSource::uploadUniforms() |
| Keyboard & MIDI | C47 (BindingManager), C48 (MidiHandler), C49 (MidiOutputHandler) | BindingManager::processKeyDown() | MidiHandler::handleIncomingMidiMessage() |
| Video Playback | C50 (VideoPlayer), C51 (ImageSequence) | VideoPlayer::open() | VideoPlayer::getNextFrame() |
| Video Recording | C52 (VideoRecorder), C53 (SessionRecorder) | VideoRecorder::startRecording() | VideoRecorder::submitFrame() |
| Output & Display | C54 (OutputWindow), C57 (SyphonOutput) | OutputWindow::show() | SyphonOutput::publishTexture() |
| External Control | C3 (ApiServer), C58 (OscHandler) | setupRoutes() | ApiServer::start() |
| Ableton Link | C59 (LinkSync) | LinkSync::update() | LinkSync::setEnabled() |
| Genre & Smart Features | C19 (GenreDetector — shared with Analysis) | GenreDetector::process() | MappingSuggester::suggestMappings() |

## God Nodes (Top 5 by degree)

1. **MainComponent()** — C0, 24 edges. Application hub: owns all systems, layout, drag-and-drop container. Bridges ALL features. Cohesion: 0.1 (god object).
2. **setupRoutes()** (ApiServer) — C3, 22 edges. REST API routing, connects to BPM, features, effects, sources, composition, signals.
3. **setupRoutes()** (TestServer) — C8, 19 edges. Test mirror of API server.
4. **paint()** (AudioReadoutPanel) — C26, 14 edges. Renders band meters, beat phase, dB, genre, onset, spectrum, waveform.
5. **jsonOk()** (ApiServer) — C3, 14 edges. Utility called by every successful API handler.

## Cross-Feature Bridges

- **MainComponent** — bridges ALL features (C0). Owns AudioEngine, AnalysisThread, FeatureBus, Renderer, Composition, BindingManager, MidiHandler, ApiServer, OscHandler, LinkSync. The single connection point.
- **Renderer** — bridges Render Pipeline ↔ Visual Effects ↔ Mapping ↔ Signal Routing ↔ Sources ↔ Composition (C1). Calls processFrame() on mapping, routing, signals every frame.
- **CompositorEngine** — bridges Render Pipeline ↔ Composition ↔ Effects (C2). Handles deck/layer compositing, temporal buffers, feedback.
- **FeatureSnapshot** — bridges Audio Analysis ↔ Feature Transport ↔ Mapping ↔ Signal Routing ↔ Genre Detection. POD struct crossing thread boundaries.

## Structural Insights

### Best-Structured Modules (highest cohesion)
- StructuralDetector: 0.50
- KeyDetector: 0.50
- LoudnessAnalyzer: 0.50
- Autopilot: 0.46
- MappingSuggester: 0.43

### Weakest Cohesion (refactoring candidates)
- MainComponent: 0.10 (45 nodes — slideshow, beat sync, camera, output, MIDI learn, binding, menus)
- Renderer: 0.10 (31 nodes — shader compilation, frame capture, compositing, source management)

### Fragmentation Note
144 communities for 1,264 nodes (avg 8.8 nodes/community) — community narrative derives from the earlier 1,264-node graph and has not been re-derived against the current 4,231-node graph. C++ header/source separation inflates community count. 46 communities have ≤2 nodes. The analysis pipeline (13 clean communities with cohesion 0.27-0.50) is the architectural gold standard in this codebase.

## Cross-Project Dependencies

| This Project | Depends On | Interface | Notes |
|-------------|-----------|-----------|-------|
| Audio-DNA | JUCE 8 (FetchContent) | C++ API | Core framework — audio, UI, OpenGL, threading |
| Audio-DNA | Aubio 0.4.9+ (system) | C API | BPM, onset, pitch detection |
| Audio-DNA | FFmpeg 8 (system) | C API | Video decode/encode |
| Audio-DNA | cpp-httplib (FetchContent) | C++ header-only | REST API server |
| Audio-DNA | Catch2 v3.7.1 (FetchContent) | C++ test framework | Test-only |
| Audio-DNA | Syphon (optional system) | Obj-C API | macOS texture sharing |
| Audio-DNA | Ableton Link (optional) | C++ header-only | Tempo sync |
