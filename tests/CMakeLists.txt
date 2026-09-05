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

# --- test_undo_commands (Undo v1 step 2: clip commands + coord resolution) ---
# Headless: bare Composition, no-op renderer/UI hooks. UndoService resolvers are
# inline (renderer-free) so this target does NOT link UndoService.cpp/Renderer.
add_executable(test_undo_commands
    test_undo_commands.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
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
