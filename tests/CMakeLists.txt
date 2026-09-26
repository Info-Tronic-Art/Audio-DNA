# --- Catch2 via FetchContent ---
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

# Common include path for project sources
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# --- test_ring_buffer ---
add_executable(test_ring_buffer test_ring_buffer.cpp)
target_include_directories(test_ring_buffer PRIVATE ${SRC_DIR})
target_link_libraries(test_ring_buffer PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_ring_buffer)
catch_discover_tests(test_ring_buffer)

# --- test_waveform_snapshot (Wave 1-D: waveform seqlock torn-read guard) ---
add_executable(test_waveform_snapshot test_waveform_snapshot.cpp)
target_include_directories(test_waveform_snapshot PRIVATE ${SRC_DIR})
target_link_libraries(test_waveform_snapshot PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_waveform_snapshot)
catch_discover_tests(test_waveform_snapshot)

# --- test_renderer_source_confinement (activeSources_ owner-thread confinement) ---
add_executable(test_renderer_source_confinement test_renderer_source_confinement.cpp)
target_include_directories(test_renderer_source_confinement PRIVATE ${SRC_DIR})
target_link_libraries(test_renderer_source_confinement PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_renderer_source_confinement)
catch_discover_tests(test_renderer_source_confinement)

# --- test_clip_replace_media_retire (S166-LEAK: kClipReplaceContent's IMAGE
# branch never retired the outgoing video/image-sequence decoder). Headless:
# mirrors the videoPlayers_/imageSequences_ retire mechanism, same reason and
# approach as test_renderer_source_confinement above — MainComponent.cpp and
# Renderer.cpp are not linked into any ctest target here.
add_executable(test_clip_replace_media_retire test_clip_replace_media_retire.cpp)
target_include_directories(test_clip_replace_media_retire PRIVATE ${SRC_DIR})
target_link_libraries(test_clip_replace_media_retire PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_clip_replace_media_retire)
catch_discover_tests(test_clip_replace_media_retire)

# --- test_layer_transport_reverse (s-rta-0924 step3 S3-B follow-up: the
# LayerTransport binding's pause->play toggle must not reset clip->reverse
# the way TopBar's Play button intentionally does). Headless: mirrors
# MainComponent::applyClipPlaying's action switch and the LayerTransport
# binding's toggle decision, same reason and approach as
# test_clip_replace_media_retire above -- MainComponent.cpp is not linked
# into any ctest target here.
add_executable(test_layer_transport_reverse test_layer_transport_reverse.cpp)
target_include_directories(test_layer_transport_reverse PRIVATE ${SRC_DIR})
target_link_libraries(test_layer_transport_reverse PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_layer_transport_reverse)
catch_discover_tests(test_layer_transport_reverse)

# --- test_composition_tier_oracle (S166-L8: model-layer coverage for the
# composition-tier oracle endpoints in src/test/TestServer.cpp — see the
# file header for exact scope). Headless: bare Composition + EffectLibrary,
# same reason and approach as test_undo_commands below — TestServer.cpp,
# Renderer.cpp, and CompositorEngine.cpp are not linked into any ctest
# target here.
add_executable(test_composition_tier_oracle
    test_composition_tier_oracle.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
    ${SRC_DIR}/effects/Effect.cpp
)
target_include_directories(test_composition_tier_oracle PRIVATE ${SRC_DIR})
target_link_libraries(test_composition_tier_oracle PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_composition_tier_oracle PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_composition_tier_oracle PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_composition_tier_oracle)
catch_discover_tests(test_composition_tier_oracle)

# --- test_spectral_features ---
add_executable(test_spectral_features
    test_spectral_features.cpp
    ${SRC_DIR}/analysis/SpectralFeatures.cpp
)
target_include_directories(test_spectral_features PRIVATE ${SRC_DIR})
target_link_libraries(test_spectral_features PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_spectral_features)
catch_discover_tests(test_spectral_features)

# --- test_feature_bus ---
add_executable(test_feature_bus
    test_feature_bus.cpp
    ${SRC_DIR}/features/FeatureBus.cpp
)
target_include_directories(test_feature_bus PRIVATE ${SRC_DIR})
target_link_libraries(test_feature_bus PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_feature_bus)
catch_discover_tests(test_feature_bus)

# --- test_smoother ---
add_executable(test_smoother test_smoother.cpp)
target_include_directories(test_smoother PRIVATE ${SRC_DIR})
target_link_libraries(test_smoother PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_smoother)
catch_discover_tests(test_smoother)

# --- test_onset_pulse (onset render-path fix: header-only onsetCount delta helper) ---
add_executable(test_onset_pulse test_onset_pulse.cpp)
target_include_directories(test_onset_pulse PRIVATE ${SRC_DIR})
target_link_libraries(test_onset_pulse PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_onset_pulse)
catch_discover_tests(test_onset_pulse)

# --- test_oscillator_bar_fold (S166-L5a: multi-bar OscillatorSignal/EnvelopeSignal phase) ---
add_executable(test_oscillator_bar_fold test_oscillator_bar_fold.cpp)
target_include_directories(test_oscillator_bar_fold PRIVATE ${SRC_DIR})
target_link_libraries(test_oscillator_bar_fold PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_oscillator_bar_fold)
catch_discover_tests(test_oscillator_bar_fold)

# --- test_integration_pipeline ---
# Full pipeline integration test: generates known audio signals and verifies
# feature extraction produces expected values. Requires JUCE (FFT) and Aubio.
add_executable(test_integration_pipeline
    test_integration_pipeline.cpp
    ${SRC_DIR}/analysis/AnalysisResampler.cpp
    ${SRC_DIR}/analysis/FFTProcessor.cpp
    ${SRC_DIR}/analysis/SpectralFeatures.cpp
    ${SRC_DIR}/analysis/OnsetDetector.cpp
    ${SRC_DIR}/analysis/BPMTracker.cpp
    ${SRC_DIR}/analysis/MFCCExtractor.cpp
    ${SRC_DIR}/analysis/ChromaExtractor.cpp
    ${SRC_DIR}/analysis/KeyDetector.cpp
    ${SRC_DIR}/analysis/LoudnessAnalyzer.cpp
    ${SRC_DIR}/analysis/StructuralDetector.cpp
    ${SRC_DIR}/analysis/PitchTracker.cpp
    ${SRC_DIR}/features/FeatureBus.cpp
)
target_include_directories(test_integration_pipeline PRIVATE ${SRC_DIR})
target_link_libraries(test_integration_pipeline PRIVATE
    Catch2::Catch2WithMain
    juce::juce_dsp
    juce::juce_core
    juce::juce_audio_basics
    Aubio::Aubio
)
target_compile_definitions(test_integration_pipeline PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_integration_pipeline PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_integration_pipeline)
catch_discover_tests(test_integration_pipeline)

# --- test_mapping_engine ---
add_executable(test_mapping_engine
    test_mapping_engine.cpp
    ${SRC_DIR}/mapping/MappingEngine.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/effects/EffectChain.cpp
    ${SRC_DIR}/render/ShaderManager.cpp
    ${SRC_DIR}/render/TextureManager.cpp
    ${SRC_DIR}/render/LUTLoader.cpp
    ${SRC_DIR}/render/FullscreenQuad.cpp
)
target_include_directories(test_mapping_engine PRIVATE ${SRC_DIR})
target_link_libraries(test_mapping_engine PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_opengl
)
target_compile_definitions(test_mapping_engine PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_mapping_engine PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_mapping_engine)
catch_discover_tests(test_mapping_engine)

# --- test_bpm_stabilization ---
add_executable(test_bpm_stabilization
    test_bpm_stabilization.cpp
    ${SRC_DIR}/analysis/BPMTracker.cpp
)
target_include_directories(test_bpm_stabilization PRIVATE ${SRC_DIR})
target_link_libraries(test_bpm_stabilization PRIVATE
    Catch2::Catch2WithMain
    Aubio::Aubio
)
if(NOT MSVC)
    target_compile_options(test_bpm_stabilization PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_bpm_stabilization)
catch_discover_tests(test_bpm_stabilization)

# --- test_downbeat_detector ---
add_executable(test_downbeat_detector
    test_downbeat_detector.cpp
    ${SRC_DIR}/analysis/BPMTracker.cpp
)
target_include_directories(test_downbeat_detector PRIVATE ${SRC_DIR})
target_link_libraries(test_downbeat_detector PRIVATE
    Catch2::Catch2WithMain
    Aubio::Aubio
)
if(NOT MSVC)
    target_compile_options(test_downbeat_detector PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_downbeat_detector)
catch_discover_tests(test_downbeat_detector)

# --- test_composition (P3: data model + undo) ---
add_executable(test_composition
    test_composition.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/core/UndoManager.cpp
)
target_include_directories(test_composition PRIVATE ${SRC_DIR})
target_link_libraries(test_composition PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_composition PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_composition PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_composition)
catch_discover_tests(test_composition)

# --- test_take (s168 step 1: recorder core -- ControlPath/Lane/Take/
# TempoMap/Program/Player/PerformanceRecorder/RecorderClock, headless.
# SessionRecorder is retired -- s167-performance-log-and-routines.md
# section 5 row 1. Links EffectLibrary.cpp/Effect.cpp for Program::compile's
# param-slot resolution against real Clip::EffectSlot data (same headless
# pattern as test_composition_tier_oracle above -- no OpenGL/ShaderManager
# needed, EffectLibrary/Effect are juce_core-only).
add_executable(test_take
    test_take.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/recording/TempoMap.cpp
    ${SRC_DIR}/recording/PerfState.cpp
    ${SRC_DIR}/recording/Take.cpp
    ${SRC_DIR}/recording/Program.cpp
    ${SRC_DIR}/recording/Player.cpp
    ${SRC_DIR}/recording/RecorderClock.cpp
    ${SRC_DIR}/recording/PerformanceRecorder.cpp
)
target_include_directories(test_take PRIVATE ${SRC_DIR})
target_link_libraries(test_take PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_take PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
if(NOT MSVC)
    target_compile_options(test_take PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_take)
catch_discover_tests(test_take)

# --- test_audio_tap_sync (s168 step 2: T1 -- D10.3's headless alignment
# proof for AudioTap/CombinedCallback/the delivered-sample counter/gap
# detection, s167-performance-log-and-routines.md section 2 D10.3, section 5
# Build order row 2). Drives CombinedCallback directly with a fake
# AudioIODevice -- no real device, headless, deterministic. Links
# AudioCallback.cpp (the analysis fan-out CombinedCallback also feeds) and
# AudioTap.cpp/Take.cpp/TempoMap.cpp/PerfState.cpp (the folder save/load
# completion case) -- none of Program/Player/RecorderClock/
# PerformanceRecorder are needed here, unlike test_take above.
# AUDIODNA_AUDIOTAP_TEST_HOOKS compiles in AudioTap's test-only forced-write-
# failure hook (see AudioTap.h) -- defined ONLY for this target, never for
# the AudioDNA app itself.
add_executable(test_audio_tap_sync
    test_audio_tap_sync.cpp
    ${SRC_DIR}/audio/AudioCallback.cpp
    ${SRC_DIR}/recording/AudioTap.cpp
    ${SRC_DIR}/recording/Take.cpp
    ${SRC_DIR}/recording/TempoMap.cpp
    ${SRC_DIR}/recording/PerfState.cpp
)
target_include_directories(test_audio_tap_sync PRIVATE ${SRC_DIR})
target_link_libraries(test_audio_tap_sync PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_utils
)
target_compile_definitions(test_audio_tap_sync PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    AUDIODNA_AUDIOTAP_TEST_HOOKS=1
)
if(NOT MSVC)
    target_compile_options(test_audio_tap_sync PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_audio_tap_sync)
catch_discover_tests(test_audio_tap_sync)

# --- test_undo_commands (Undo v1 step 2: clip commands + coord resolution) ---
# Headless: bare Composition, no-op renderer/UI hooks. UndoService resolvers are
# inline (renderer-free) so this target does NOT link UndoService.cpp/Renderer.
add_executable(test_undo_commands
    test_undo_commands.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/core/UndoManager.cpp
)
target_include_directories(test_undo_commands PRIVATE ${SRC_DIR})
target_link_libraries(test_undo_commands PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_undo_commands PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_undo_commands PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_undo_commands)
catch_discover_tests(test_undo_commands)

# --- test_connection (s167-l2: the universal connection model + engine
# core, headless -- .harmony/specs/s166-universal-connection-architecture.md
# section 5, Lane 2). Model + shaper + engine only, no renderer/UI: links
# Clip.cpp/Layer.cpp (scalar arrays + serialization), the connect/ sources,
# SignalRegistry.cpp (ConnectionEngine's Kind::Signal path resolves through
# it even when a test never exercises that kind), and UndoManager.cpp for
# the EffectStackCmd round-trip test (test (e)).
add_executable(test_connection
    test_connection.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnectionShaper.cpp
    ${SRC_DIR}/connect/ConnectionEngine.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/signal/SignalRegistry.cpp
    ${SRC_DIR}/mapping/MappingEngine.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/effects/EffectChain.cpp
    ${SRC_DIR}/render/ShaderManager.cpp
    ${SRC_DIR}/render/TextureManager.cpp
    ${SRC_DIR}/render/LUTLoader.cpp
    ${SRC_DIR}/render/FullscreenQuad.cpp
    ${SRC_DIR}/core/UndoManager.cpp
)
target_include_directories(test_connection PRIVATE ${SRC_DIR})
target_link_libraries(test_connection PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_opengl
)
target_compile_definitions(test_connection PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_connection PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_connection)
catch_discover_tests(test_connection)

# --- test_routing_engine (P3: signals + routing) ---
add_executable(test_routing_engine
    test_routing_engine.cpp
    ${SRC_DIR}/signal/SignalRegistry.cpp
    ${SRC_DIR}/routing/RoutingEngine.cpp
    ${SRC_DIR}/mapping/MappingEngine.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/effects/EffectChain.cpp
    ${SRC_DIR}/render/ShaderManager.cpp
    ${SRC_DIR}/render/TextureManager.cpp
    ${SRC_DIR}/render/LUTLoader.cpp
    ${SRC_DIR}/render/FullscreenQuad.cpp
)
target_include_directories(test_routing_engine PRIVATE ${SRC_DIR})
target_link_libraries(test_routing_engine PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_opengl
)
target_compile_definitions(test_routing_engine PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_routing_engine PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_routing_engine)
catch_discover_tests(test_routing_engine)

# --- test_thumbnail_cache (FilesBrowser perf: LRU thumbnail cache keying) ---
# Headless: header-only ThumbnailCache depends only on juce_core + juce_graphics,
# no juce_gui_basics, so it is unit-testable without a GUI/display context.
add_executable(test_thumbnail_cache test_thumbnail_cache.cpp)
target_include_directories(test_thumbnail_cache PRIVATE ${SRC_DIR})
target_link_libraries(test_thumbnail_cache PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_graphics
)
target_compile_definitions(test_thumbnail_cache PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_thumbnail_cache PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_thumbnail_cache)
catch_discover_tests(test_thumbnail_cache)

# --- test_compositor (P3: deck compositing data model + autopilot) ---
add_executable(test_compositor
    test_compositor.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/model/Autopilot.cpp
)
target_include_directories(test_compositor PRIVATE ${SRC_DIR})
target_link_libraries(test_compositor PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_graphics
)
target_compile_definitions(test_compositor PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_compositor PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_compositor)
catch_discover_tests(test_compositor)

# --- test_autopilot (autopilot advance-gate: EoV + on-beat + source parity) ---
add_executable(test_autopilot
    test_autopilot.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/model/Autopilot.cpp
)
target_include_directories(test_autopilot PRIVATE ${SRC_DIR})
target_link_libraries(test_autopilot PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_graphics
)
target_compile_definitions(test_autopilot PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_autopilot PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_autopilot)
catch_discover_tests(test_autopilot)

# --- test_preset_manager (preset-retarget-fix: dual-key targeting D1, N4 by-name
# param restore, D2 ISF dedupe, D4/D6 version/back-compat) ---
add_executable(test_preset_manager
    test_preset_manager.cpp
    ${SRC_DIR}/ui/PresetManager.cpp
    ${SRC_DIR}/mapping/MappingEngine.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/effects/EffectChain.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
    ${SRC_DIR}/render/ShaderManager.cpp
    ${SRC_DIR}/render/TextureManager.cpp
    ${SRC_DIR}/render/LUTLoader.cpp
    ${SRC_DIR}/render/FullscreenQuad.cpp
)
target_include_directories(test_preset_manager PRIVATE ${SRC_DIR})
target_link_libraries(test_preset_manager PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_opengl
)
target_compile_definitions(test_preset_manager PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_preset_manager PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_preset_manager)
catch_discover_tests(test_preset_manager)

# --- test_projectm_preset_manager (S166-FAV: MilkDrop favorites/user-preset
# persistence — ProjectMPresetManager::saveUserData()/loadUserData() round
# trip). Headless: needs only juce_core (real file/JSON I/O), no GUI/GL —
# mirrors test_thumbnail_cache's minimal style. Distinct from, and NOT to be
# confused with, test_preset_manager above (that one covers ui/PresetManager,
# the unrelated effect-chain/deck preset system).
add_executable(test_projectm_preset_manager
    test_projectm_preset_manager.cpp
    ${SRC_DIR}/sources/ProjectMPresetManager.cpp
)
target_include_directories(test_projectm_preset_manager PRIVATE ${SRC_DIR})
target_link_libraries(test_projectm_preset_manager PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
)
target_compile_definitions(test_projectm_preset_manager PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_projectm_preset_manager PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_projectm_preset_manager)
catch_discover_tests(test_projectm_preset_manager)

# --- syphon-check (headless Syphon server-discovery verification CLI) ---
# Not a Catch2 unit test: a standalone gate tool that confirms a running
# Audio-DNA instance is announcing a Syphon server (syphon_check.m). Needs
# the same vendored Syphon sources/headers as the AudioDNA target itself
# (syphon_SOURCE_DIR, SYPHON_STAGED_INCLUDE_DIR — set by the Syphon block in
# the root CMakeLists.txt, which runs before add_subdirectory(tests)).
if(APPLE AND AUDIODNA_BUILD_SYPHON)
    add_executable(syphon-check
        syphon_check.m
        ${syphon_SOURCE_DIR}/SyphonServerDirectory.m
        ${syphon_SOURCE_DIR}/SyphonMessaging.m
        ${syphon_SOURCE_DIR}/SyphonMessageQueue.m
        ${syphon_SOURCE_DIR}/SyphonMessageSender.m
        ${syphon_SOURCE_DIR}/SyphonMessageReceiver.m
        ${syphon_SOURCE_DIR}/SyphonCFMessageSender.m
        ${syphon_SOURCE_DIR}/SyphonCFMessageReceiver.m
        ${syphon_SOURCE_DIR}/SyphonPrivate.m
        ${syphon_SOURCE_DIR}/SyphonDispatch.c
    )
    set_source_files_properties(
        syphon_check.m
        ${syphon_SOURCE_DIR}/SyphonServerDirectory.m
        ${syphon_SOURCE_DIR}/SyphonMessaging.m
        ${syphon_SOURCE_DIR}/SyphonMessageQueue.m
        ${syphon_SOURCE_DIR}/SyphonMessageSender.m
        ${syphon_SOURCE_DIR}/SyphonMessageReceiver.m
        ${syphon_SOURCE_DIR}/SyphonCFMessageSender.m
        ${syphon_SOURCE_DIR}/SyphonCFMessageReceiver.m
        ${syphon_SOURCE_DIR}/SyphonPrivate.m
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc;-include;${syphon_SOURCE_DIR}/Syphon_Prefix.pch;-DGL_SILENCE_DEPRECATION"
    )
    target_include_directories(syphon-check PRIVATE "${SYPHON_STAGED_INCLUDE_DIR}" "${syphon_SOURCE_DIR}")
    target_link_libraries(syphon-check PRIVATE "-framework Cocoa")

    # Negative check only: with no app running under this (deliberately
    # nonexistent) app name, syphon-check must exit 1 — deterministic,
    # headless, no GL/display needed, so safe to run in the regular ctest
    # suite. WILL_FAIL TRUE because exit 1 IS the correct/expected outcome
    # here. The positive check (a real server found) needs a live,
    # Syphon-enabled Audio-DNA instance and is a manual/external gate step,
    # documented in the builder report, not part of this automated suite.
    add_test(NAME syphon_check_negative
             COMMAND syphon-check "Audio-DNA-Negative-Check-Nonexistent-App-9f3a" 0.5)
    set_tests_properties(syphon_check_negative PROPERTIES WILL_FAIL TRUE)
endif()

# --- test_program_stamps (s-rta-0923 lane L2: Program::compile must take
# each continuous breakpoint's x from its own gesture's parallel Stamp, not
# reconstruct it from the take's sparse TempoMap). Same headless link set as
# test_take above -- Composition/Clip/Layer/EffectLibrary/Effect are
# juce_core-only, no OpenGL/ShaderManager needed. A separate executable (not
# appended to test_take.cpp) because this lane does not own that file.
add_executable(test_program_stamps
    test_program_stamps.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/recording/TempoMap.cpp
    ${SRC_DIR}/recording/PerfState.cpp
    ${SRC_DIR}/recording/Take.cpp
    ${SRC_DIR}/recording/Program.cpp
)
target_include_directories(test_program_stamps PRIVATE ${SRC_DIR})
target_link_libraries(test_program_stamps PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_program_stamps PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_program_stamps PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_program_stamps)
catch_discover_tests(test_program_stamps)

# --- test_program_preamble (s-rta-0925: "replay restore" -- checkpoint 0
# becomes the Program preamble, fired at Play). Same link set as
# test_program_stamps above, plus PerfStateCapture.cpp (capturePerfState()
# builds this test's checkpoint0 fixtures from a live Composition).
add_executable(test_program_preamble
    test_program_preamble.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/recording/TempoMap.cpp
    ${SRC_DIR}/recording/PerfState.cpp
    ${SRC_DIR}/recording/PerfStateCapture.cpp
    ${SRC_DIR}/recording/Take.cpp
    ${SRC_DIR}/recording/Program.cpp
)
target_include_directories(test_program_preamble PRIVATE ${SRC_DIR})
target_link_libraries(test_program_preamble PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_program_preamble PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_program_preamble PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_program_preamble)
catch_discover_tests(test_program_preamble)

# --- test_recorder_double_touch (s-rta-0923 lane L3, addendum 4a):
# PerformanceRecorder::touch() must not silently discard an already-open
# gesture on the same key. Standalone target (this lane's worktree did
# not inherit an L0 stub-in-test_take, so the test lives in its own
# self-contained target rather than editing the shared test_take
# add_executable() block above) -- same headless dependency set as
# test_take (Composition/Clip/Layer/EffectLibrary for the data model,
# TempoMap/PerfState/Take/RecorderClock/PerformanceRecorder for the
# recorder core; Program/Player are not needed, this test never compiles
# or plays a Take).
add_executable(test_recorder_double_touch
    test_recorder_double_touch.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/recording/TempoMap.cpp
    ${SRC_DIR}/recording/PerfState.cpp
    ${SRC_DIR}/recording/Take.cpp
    ${SRC_DIR}/recording/RecorderClock.cpp
    ${SRC_DIR}/recording/PerformanceRecorder.cpp
)
target_include_directories(test_recorder_double_touch PRIVATE ${SRC_DIR})
target_link_libraries(test_recorder_double_touch PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_recorder_double_touch PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_recorder_double_touch PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_recorder_double_touch)
catch_discover_tests(test_recorder_double_touch)

# --- test_audio_store (Lane R28, s-rta-0923 Ruling 28: the shared audio
# store -- AudioStore/AudioAsset/fp1/resolve/GC-query, headless. Links the
# same Take-without-model list test_audio_tap_sync links: AudioTap.cpp
# (drives it directly via prepare/start/push/stop for the store-integration
# cases) + Take.cpp/TempoMap.cpp/PerfState.cpp. AUDIODNA_AUDIOTAP_TEST_HOOKS
# is NOT needed here (no forced-write-failure cases).
add_executable(test_audio_store
    test_audio_store.cpp
    ${SRC_DIR}/recording/AudioStore.cpp
    ${SRC_DIR}/recording/AudioTap.cpp
    ${SRC_DIR}/recording/Take.cpp
    ${SRC_DIR}/recording/TempoMap.cpp
    ${SRC_DIR}/recording/PerfState.cpp
)
target_include_directories(test_audio_store PRIVATE ${SRC_DIR})
target_link_libraries(test_audio_store PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_cryptography
)
target_compile_definitions(test_audio_store PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
if(NOT MSVC)
    target_compile_options(test_audio_store PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_audio_store)
catch_discover_tests(test_audio_store)

# --- test_take_v1_transport (Lane R28, review-fix L1: the v1 TransportChange
# bridge -- play/pause/stop faithful, speed/reverse dropped+counted. Links
# test_take's list minus Player/RecorderClock/PerformanceRecorder (this
# target only needs Take::fromV1Var + Program::compile's dispatch, not the
# live playback classes).
add_executable(test_take_v1_transport
    test_take_v1_transport.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/recording/TempoMap.cpp
    ${SRC_DIR}/recording/PerfState.cpp
    ${SRC_DIR}/recording/Take.cpp
    ${SRC_DIR}/recording/Program.cpp
)
target_include_directories(test_take_v1_transport PRIVATE ${SRC_DIR})
target_link_libraries(test_take_v1_transport PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_take_v1_transport PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
if(NOT MSVC)
    target_compile_options(test_take_v1_transport PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_take_v1_transport)
catch_discover_tests(test_take_v1_transport)

# --- test_bt_device_shapes (LANE H1 diagnostic, s-rta-0923: heap-corruption
# hypothesis H1 -- the s168 AudioTap fan-out under Bluetooth-headset-shaped
# device callbacks: 1 input channel, 16000 Hz, odd/variable block sizes, a
# device restart with a sample-rate change mid-stream). Drives
# CombinedCallback directly with the same FakeAudioIODevice pattern
# test_audio_tap_sync.cpp uses -- no real device, headless, deterministic.
# Links only AudioCallback.cpp and AudioTap.cpp (the fan-out under test);
# Take/TempoMap/PerfState are not touched by this diagnostic, unlike
# test_audio_tap_sync's folder-save/load case.
add_executable(test_bt_device_shapes
    test_bt_device_shapes.cpp
    ${SRC_DIR}/audio/AudioCallback.cpp
    ${SRC_DIR}/recording/AudioTap.cpp
)
target_include_directories(test_bt_device_shapes PRIVATE ${SRC_DIR})
target_link_libraries(test_bt_device_shapes PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_utils
)
target_compile_definitions(test_bt_device_shapes PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_bt_device_shapes PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_bt_device_shapes)
catch_discover_tests(test_bt_device_shapes)

# --- test_manual_write (s-rta-0923 lane 3, Lane C0 scaffold: the manualWrite
# funnel core, headless -- .harmony/specs/s-rta-0923-lane3-plan.md sections
# 3.2/5. Shaped exactly like test_connection above: same source list +
# ManualWrite.cpp. Every case in this file is written NOW against Lane C0's
# stub (src/connect/ManualWrite.cpp returns nullopt/false/no-op for every
# function) and is expected to FAIL until Lane C1 implements it for real --
# that failure is the fail-first evidence for C1's work. Case (i) is a PIN
# (round-trips Composition::scalarConns through toVar/fromVar; passes on
# HEAD already, does not touch ManualWrite at all).
add_executable(test_manual_write
    test_manual_write.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnectionShaper.cpp
    ${SRC_DIR}/connect/ConnectionEngine.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/connect/ManualWrite.cpp
    ${SRC_DIR}/signal/SignalRegistry.cpp
    ${SRC_DIR}/mapping/MappingEngine.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/effects/EffectChain.cpp
    ${SRC_DIR}/render/ShaderManager.cpp
    ${SRC_DIR}/render/TextureManager.cpp
    ${SRC_DIR}/render/LUTLoader.cpp
    ${SRC_DIR}/render/FullscreenQuad.cpp
    ${SRC_DIR}/core/UndoManager.cpp
)
target_include_directories(test_manual_write PRIVATE ${SRC_DIR})
target_link_libraries(test_manual_write PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_opengl
)
target_compile_definitions(test_manual_write PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_manual_write PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_manual_write)
catch_discover_tests(test_manual_write)

# --- test_comp_transform_units (comp-transform units fix: the composition
# position/anchor pixel-space values -- ScalarMath::posX3840/posY2160,
# range approx +/-1920/+/-1080 on a 3840x2160 reference canvas -- must be
# normalized to the comp_transform shader's u_comp_position/u_comp_anchor
# uniform convention before Renderer::applyCompTransform uploads them, or
# ANY non-zero composition position renders solid black. Headless: pins
# ScalarMath::posPxToCompUniformX/Y (ScalarParams.h) directly, no
# OpenGL/Renderer needed -- same minimal style as test_smoother, plus
# juce_core because ScalarParams.h -> ParamConnection.h -> AutomationCurve.h
# needs juce::var/juce::DynamicObject.
add_executable(test_comp_transform_units test_comp_transform_units.cpp)
target_include_directories(test_comp_transform_units PRIVATE ${SRC_DIR})
target_link_libraries(test_comp_transform_units PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
)
target_compile_definitions(test_comp_transform_units PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_comp_transform_units PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_comp_transform_units)
catch_discover_tests(test_comp_transform_units)

# --- test_conn_picker (s-rta-0923 lane 3, Lane C0 scaffold: the picker ->
# ConnSource translation, headless -- .harmony/specs/s-rta-0923-lane3-plan.md
# sections 4.3/5. Shaped exactly like test_connection above: same source
# list + ConnPicker.cpp. Every case here is written NOW against Lane C0's
# stub (src/connect/ConnPicker.cpp: sourceFromPicker returns ConnSource{},
# describeSource returns "") and is expected to FAIL until Lane C4 implements
# it for real, EXCEPT the two pins that pass on the stub by construction
# (Manual -> Kind::None, describeSource(None) == "").
add_executable(test_conn_picker
    test_conn_picker.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnectionShaper.cpp
    ${SRC_DIR}/connect/ConnectionEngine.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/connect/ConnPicker.cpp
    ${SRC_DIR}/signal/SignalRegistry.cpp
    ${SRC_DIR}/mapping/MappingEngine.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/effects/EffectChain.cpp
    ${SRC_DIR}/render/ShaderManager.cpp
    ${SRC_DIR}/render/TextureManager.cpp
    ${SRC_DIR}/render/LUTLoader.cpp
    ${SRC_DIR}/render/FullscreenQuad.cpp
    ${SRC_DIR}/core/UndoManager.cpp
)
target_include_directories(test_conn_picker PRIVATE ${SRC_DIR})
target_link_libraries(test_conn_picker PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_opengl
)
target_compile_definitions(test_conn_picker PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_conn_picker PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_conn_picker)
catch_discover_tests(test_conn_picker)

# --- test_recorder_host (s-rta-0923 step 3 / s-rta-0924 Lane S3-A: RecorderHost's
# arm -> tick -> disarm lifecycle, provisional/periodic Take::save, the R14
# gapDetection glue, discrete/continuous capture+replay through a fake Dispatch,
# the overdub asset-frame clock, a mid-take self-stop, and PerfStateCapture's
# round trip -- headless. Drives AudioTap DIRECTLY (prepare/start/push/stop),
# test_audio_store's own idiom -- not the CombinedCallback/FakeAudioIODevice
# harness test_audio_tap_sync.cpp needs (that proves AudioTap's OWN
# sample-alignment contract; RecorderHost's contract is calling AudioTap
# correctly, not AudioTap's internals), so AudioCallback.cpp/RingBuffer.h are
# NOT linked here. Source list otherwise mirrors test_take's (model/effects,
# for Program::compile's param-slot resolution and PerfStateCapture's
# non-default-effect-param check) plus AudioStore.cpp/AudioTap.cpp
# (test_audio_store's own list) and the two new S3-A sources.
add_executable(test_recorder_host
    test_recorder_host.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/recording/TempoMap.cpp
    ${SRC_DIR}/recording/PerfState.cpp
    ${SRC_DIR}/recording/Take.cpp
    ${SRC_DIR}/recording/Program.cpp
    ${SRC_DIR}/recording/Player.cpp
    ${SRC_DIR}/recording/RecorderClock.cpp
    ${SRC_DIR}/recording/PerformanceRecorder.cpp
    ${SRC_DIR}/recording/AudioTap.cpp
    ${SRC_DIR}/recording/AudioStore.cpp
    ${SRC_DIR}/recording/RecorderHost.cpp
    ${SRC_DIR}/recording/PerfStateCapture.cpp
)
target_include_directories(test_recorder_host PRIVATE ${SRC_DIR})
target_link_libraries(test_recorder_host PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_cryptography
)
target_compile_definitions(test_recorder_host PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
    # s-rta-0924b: AudioTap's test-only hooks (debugAddPhantomFramesForTest) -- this target
    # compiles its OWN AudioTap.cpp object, so the define never reaches the app.
    AUDIODNA_AUDIOTAP_TEST_HOOKS=1
)
if(NOT MSVC)
    target_compile_options(test_recorder_host PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_recorder_host)
catch_discover_tests(test_recorder_host)

# --- test_analysis_resampler (R13 lane A: AnalysisResampler -- T1 device-rate
# resampling fidelity/anti-alias/rate-switch cases, plus T1.6 the
# AudioCallback rate-cell wiring via a FakeAudioIODevice copied from
# test_audio_tap_sync.cpp). Headless: no real device.
add_executable(test_analysis_resampler
    test_analysis_resampler.cpp
    ${SRC_DIR}/analysis/AnalysisResampler.cpp
    ${SRC_DIR}/analysis/FFTProcessor.cpp
    ${SRC_DIR}/audio/AudioCallback.cpp
)
target_include_directories(test_analysis_resampler PRIVATE ${SRC_DIR})
target_link_libraries(test_analysis_resampler PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_utils
)
target_compile_definitions(test_analysis_resampler PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_analysis_resampler PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_analysis_resampler)
catch_discover_tests(test_analysis_resampler)

# --- test_record_panel_model (s-rta-0924b step 4, Lane S4-M: the Record panel's
# click-through matrix). Header-only src/ui/RecordPanelModel.h is a pure function
# RecorderHost::Status -> RecordPanelView; its include chain (RecorderHost.h and
# the recording/ headers it pulls in) is juce_core-only, so this target links
# juce_core alone -- NEVER juce_gui_basics (no display context under ctest).
add_executable(test_record_panel_model test_record_panel_model.cpp)
target_include_directories(test_record_panel_model PRIVATE ${SRC_DIR})
target_link_libraries(test_record_panel_model PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
)
target_compile_definitions(test_record_panel_model PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_record_panel_model PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_record_panel_model)
catch_discover_tests(test_record_panel_model)

# --- test_httplib_bodyless_post (s-rta-0924b: a POST with no Content-Length /
# Transfer-Encoding must not stall for CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND).
# Headless: httplib::Server on an ephemeral loopback port + raw POSIX-socket
# client (httplib::Client always sends Content-Length, so it cannot repro).
# Links httplib::httplib only -- no JUCE. POSIX sockets -> skipped on Windows.
if(NOT WIN32)
    add_executable(test_httplib_bodyless_post test_httplib_bodyless_post.cpp)
    target_link_libraries(test_httplib_bodyless_post PRIVATE
        Catch2::Catch2WithMain
        httplib::httplib
    )
    if(NOT MSVC)
        target_compile_options(test_httplib_bodyless_post PRIVATE
            -Wno-old-style-cast
            -Wno-conversion
            -Wno-sign-conversion
        )
    endif()
    apply_sanitizers(test_httplib_bodyless_post)
    catch_discover_tests(test_httplib_bodyless_post PROPERTIES TIMEOUT 60)
endif()

# --- test_tab_bar_layout (s-rta-0925 step-4 polish D2: the Browser's tab widths
# follow the measured label widths so the longest tab label is never clipped,
# e.g. "Compositions"). Pure function in src/ui/TabBarLayout.h -- no JUCE; the
# caller measures the labels.
add_executable(test_tab_bar_layout test_tab_bar_layout.cpp)
target_include_directories(test_tab_bar_layout PRIVATE ${SRC_DIR})
target_link_libraries(test_tab_bar_layout PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_tab_bar_layout)
catch_discover_tests(test_tab_bar_layout)

# --- test_per_type_autopilot_layout (s-rta-0925 visualfix: the Composition
# Inspector's Per-Type Autopilot enable-checkbox row and its Opaque cycle row
# used to land at the same y). Pure function in
# src/ui/PerTypeAutopilotLayout.h -- no JUCE.
add_executable(test_per_type_autopilot_layout test_per_type_autopilot_layout.cpp)
target_include_directories(test_per_type_autopilot_layout PRIVATE ${SRC_DIR})
target_link_libraries(test_per_type_autopilot_layout PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_per_type_autopilot_layout)
catch_discover_tests(test_per_type_autopilot_layout)

# --- test_right_click_reset (s-rta-0925 rclick: ResettableSlider right-click reset
# on the thumb AND on the IncDec text box; UniversalParamControl arms its inner
# slider by default and a reset touches the bound connection). Headless JUCE
# widgets under ScopedJuceInitialiser_GUI -- no window, no peer; mouse events are
# synthesised and delivered by calling mouseDown() directly.
add_executable(test_right_click_reset
    test_right_click_reset.cpp
    ${SRC_DIR}/ui/UniversalParamControl.cpp
    ${SRC_DIR}/connect/ConnPicker.cpp
    ${SRC_DIR}/connect/ManualWrite.cpp
    ${SRC_DIR}/signal/SignalRegistry.cpp
    ${SRC_DIR}/mapping/MappingEngine.cpp
    ${SRC_DIR}/effects/EffectChain.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/render/ShaderManager.cpp
    ${SRC_DIR}/render/TextureManager.cpp
    ${SRC_DIR}/render/FullscreenQuad.cpp
    ${SRC_DIR}/render/LUTLoader.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
)
target_include_directories(test_right_click_reset PRIVATE ${SRC_DIR})
target_link_libraries(test_right_click_reset PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(test_right_click_reset PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_right_click_reset PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_right_click_reset)
catch_discover_tests(test_right_click_reset)

# --- test_master_opacity_link (s-rta-0925 link: the TopBar master fader is a
# second widget-grip view of Composition::masterOpacity / CompScalar::Opacity --
# writes the model, grips Held on drag, follows the model / eff() on sync, and
# a right-click reset touches the grip). Headless JUCE widgets, same harness as
# test_right_click_reset; the timer read-back is exercised by calling
# TopBar::syncMasterFromComposition() directly (no dispatch loop is run).
# Link closure grew beyond the plan's original list (UniversalParamControl.cpp/
# TopBar.cpp pull in the mapping/effects/render chain transitively via
# SignalRegistry -> AudioSignal -> MappingEngine -> EffectChain -> ShaderManager/
# TextureManager/FullscreenQuad/LUTLoader) -- each addition here is the ONE .cpp
# that defines a symbol the linker named, never MainComponent.cpp/Renderer.cpp.
add_executable(test_master_opacity_link
    test_master_opacity_link.cpp
    ${SRC_DIR}/ui/TopBar.cpp
    ${SRC_DIR}/features/FeatureBus.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/connect/ManualWrite.cpp
    ${SRC_DIR}/mapping/MappingEngine.cpp
    ${SRC_DIR}/effects/EffectChain.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/render/ShaderManager.cpp
    ${SRC_DIR}/render/TextureManager.cpp
    ${SRC_DIR}/render/FullscreenQuad.cpp
    ${SRC_DIR}/render/LUTLoader.cpp
)
target_include_directories(test_master_opacity_link PRIVATE ${SRC_DIR})
target_link_libraries(test_master_opacity_link PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(test_master_opacity_link PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_master_opacity_link PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_master_opacity_link)
catch_discover_tests(test_master_opacity_link)

# --- test_resettable_slider (s-rta-0925 rclick2: reconciles lane/0925-rclick's
# never-merged ResettableSlider test cases onto main's actual design, landed via
# the opacity lane). Header-only class (src/ui/UniversalParamControl.h) -- no
# src/*.cpp is linked. Same per-TEST_CASE juce::ScopedJuceInitialiser_GUI local
# variable pattern as test_right_click_reset above (never a function-local
# static -- see .harmony/notebook.md's ScopedJuceInitialiser_GUI heap-corruption
# entry); links Catch2::Catch2WithMain like the rest of the suite.
add_executable(test_resettable_slider test_resettable_slider.cpp)
target_include_directories(test_resettable_slider PRIVATE ${SRC_DIR})
target_link_libraries(test_resettable_slider PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_resettable_slider PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_resettable_slider PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_resettable_slider)
catch_discover_tests(test_resettable_slider)
