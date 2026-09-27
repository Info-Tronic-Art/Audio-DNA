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

# --- test_clip_fit (s-rta-0926b plan-fitmode: per-clip Stretch / Bars / Crop).
# The pure header src/model/ClipFit.h (scale / barsRect / cropRect / clampMode);
# CompositorEngine::applyClipTransform only consumes scale(). Headless, no JUCE,
# registered like test_clip_replace_media_retire above.
add_executable(test_clip_fit test_clip_fit.cpp)
target_include_directories(test_clip_fit PRIVATE ${SRC_DIR})
target_link_libraries(test_clip_fit PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_clip_fit)
catch_discover_tests(test_clip_fit)

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

# --- test_compositor_opacity_alias (s-rta-0925 ms-white: GL FBO/texture
# feedback-loop hazard -- applyClipTransform's needsTransform branch handed
# effectTex_A_ back verbatim through applyClipOpacity's identity-opacity
# no-op, aliasing applyClipEffects' first write target. s-rta-0926 xfade
# replaced the hardcoded effectTex_A_/B_ targets with ScratchPool.h's pick()
# via CompositorEngine::pickEffectTarget). Headless: mirrors the
# transform->opacity texture-handle hand-off, driving the REAL pick()
# function for target selection (not a re-derived index formula), same
# reason and approach as test_layer_transport_reverse above --
# CompositorEngine.cpp issues real GL calls and is not linked into any ctest
# target here.
add_executable(test_compositor_opacity_alias test_compositor_opacity_alias.cpp)
target_include_directories(test_compositor_opacity_alias PRIVATE ${SRC_DIR})
target_link_libraries(test_compositor_opacity_alias PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_compositor_opacity_alias)
catch_discover_tests(test_compositor_opacity_alias)

# --- test_compositor_effects_parity (s-rta-0925 ms-white2: a SEPARATE
# instance of the same GL FBO/texture feedback-loop aliasing class as
# test_compositor_opacity_alias above -- applyClipEffects' ping-pong always
# started its first write at effectFBO_A_/effectTex_A_ regardless of its own
# inputTex, so a second back-to-back applyClipEffects call (per-clip effects
# then per-layer effects in compositeDeck) aliased whenever the first call's
# output happened to land on effectTex_A_ (odd enabled-effect count).
# s-rta-0926 xfade replaced the whole hardcoded-starting-index concept with
# a per-PASS pick() (ScratchPool.h via CompositorEngine::pickEffectTarget),
# parity-independent by construction. Headless: drives the REAL pick()
# function through the same two-chained-call sequence, across odd/even/zero
# per-clip effect counts, same reason and approach as
# test_compositor_opacity_alias above -- CompositorEngine.cpp issues real GL
# calls and is not linked into any ctest target here.
add_executable(test_compositor_effects_parity test_compositor_effects_parity.cpp)
target_include_directories(test_compositor_effects_parity PRIVATE ${SRC_DIR})
target_link_libraries(test_compositor_effects_parity PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_compositor_effects_parity)
catch_discover_tests(test_compositor_effects_parity)

# --- test_scratch_pool (s-rta-0926 xfade: the compositor's one rule for its
# effect scratch pool -- a pass never renders into the texture it samples or a
# texture its caller still holds). Tests the REAL picker (render/ScratchPool.h,
# pure, no GL) that CompositorEngine::pickEffectTarget calls; the GL call sites
# themselves are covered live by .harmony/probe-crossfade.sh.
add_executable(test_scratch_pool test_scratch_pool.cpp)
target_include_directories(test_scratch_pool PRIVATE ${SRC_DIR})
target_link_libraries(test_scratch_pool PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_scratch_pool)
catch_discover_tests(test_scratch_pool)

# --- test_layer_state_key (s-rta-0926b render R2: a layer's temporal buffer /
# frame ring / feedback state is keyed by deck AND layer id -- layer ids repeat
# across decks). Tests the REAL key function (render/LayerStateKey.h, pure, no
# GL) against decks built by the real Deck::initDefault; the GL call sites are
# covered live by .harmony/probe-render-state.sh.
add_executable(test_layer_state_key
    test_layer_state_key.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
)
target_include_directories(test_layer_state_key PRIVATE ${SRC_DIR})
target_link_libraries(test_layer_state_key PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_graphics
)
target_compile_definitions(test_layer_state_key PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_layer_state_key PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_layer_state_key)
catch_discover_tests(test_layer_state_key)

# --- test_crossfade_history (s-rta-0926b render R1: at the first frame of every
# clip-to-clip crossfade the layer's clip-chain history is handed to its
# outgoing slot). Tests the REAL detector (render/CrossfadeHistory.h, pure, no
# GL) that CompositorEngine::renderLayerStages calls; the GL hand-over is
# covered live by .harmony/probe-render-state.sh rows r1_*.
add_executable(test_crossfade_history test_crossfade_history.cpp)
target_include_directories(test_crossfade_history PRIVATE ${SRC_DIR})
target_link_libraries(test_crossfade_history PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_crossfade_history)
catch_discover_tests(test_crossfade_history)

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

# --- test_routine (s-rta-0926 routines slice 1, lane 1a --
# plan-routines-s1-final.md 6.1 cases 1-6): the Routine model + its
# composition serialization, sliceRoutine, takeBeatOfBar, compileRoutine.
# Same link set as test_program_preamble above (+ RoutineSlice.cpp).
add_executable(test_routine
    test_routine.cpp
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
    ${SRC_DIR}/recording/RoutineSlice.cpp
)
target_include_directories(test_routine PRIVATE ${SRC_DIR})
target_link_libraries(test_routine PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_routine PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_routine PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_routine)
catch_discover_tests(test_routine)

# --- test_routine_engine (s-rta-0926 routines slice 1, lane 1b --
# plan-routines-s1-final.md 6.1 cases 7-12 and 15): RoutineEngine's scheduler,
# loop/once/stop/restart, D9 stacking, barCount parity, post-loop compaction,
# Binding::Action::TriggerRoutine's round trip. The test_routine link set +
# Player/RecorderClock/BindingManager + RoutineEngine.cpp.
add_executable(test_routine_engine
    test_routine_engine.cpp
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
    ${SRC_DIR}/recording/RoutineSlice.cpp
    ${SRC_DIR}/recording/Player.cpp
    ${SRC_DIR}/recording/RecorderClock.cpp
    ${SRC_DIR}/binding/BindingManager.cpp
    ${SRC_DIR}/recording/RoutineEngine.cpp
)
target_include_directories(test_routine_engine PRIVATE ${SRC_DIR})
target_link_libraries(test_routine_engine PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
)
target_compile_definitions(test_routine_engine PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_routine_engine PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_routine_engine)
catch_discover_tests(test_routine_engine)

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

# --- test_render_geometry (s-rta-0926b plan4 item 1: the composition canvas --
# resolveCanvas / fitCanvas / ringDownscale / presentTaps in the pure header
# src/render/RenderGeometry.h; Renderer.cpp only consumes them). Headless,
# no GL -- registered like test_comp_transform_units above.
add_executable(test_render_geometry test_render_geometry.cpp)
target_include_directories(test_render_geometry PRIVATE ${SRC_DIR})
target_link_libraries(test_render_geometry PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
)
target_compile_definitions(test_render_geometry PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_render_geometry PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_render_geometry)
catch_discover_tests(test_render_geometry)

# --- test_deck_clock (s-rta-0926b plan4 item 2: decks that are not on screen
# keep time -- DeckClock::tick / LayerClock::advanceCrossfade, pure headers in
# src/render/; B2 adds AutopilotBank, src/model/AutopilotBank.h). Built like
# test_compositor: real Clip / Layer / Autopilot sources, juce_core +
# juce_graphics, no GL.
add_executable(test_deck_clock
    test_deck_clock.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
    ${SRC_DIR}/model/Autopilot.cpp
)
target_include_directories(test_deck_clock PRIVATE ${SRC_DIR})
target_link_libraries(test_deck_clock PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_graphics
)
target_compile_definitions(test_deck_clock PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_deck_clock PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_deck_clock)
catch_discover_tests(test_deck_clock)

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

# --- test_routine_bank_model (s-rta-0926 lane 3, plan-routines-s1-final.md section 5.4/6.1 case
# 14; s-rta-0927: only the Save Routine row's bar-range clamp is left -- the pad row moved to the deck,
# test_routine_deck_view). Header-only src/ui/RoutineBankModel.h; its include chain (RoutineEngine.h and
# the recording/ headers it pulls in) is juce_core-only, so this target links juce_core alone --
# NEVER juce_gui_basics (no display context under ctest), same posture as test_record_panel_model.
add_executable(test_routine_bank_model test_routine_bank_model.cpp)
target_include_directories(test_routine_bank_model PRIVATE ${SRC_DIR})
target_link_libraries(test_routine_bank_model PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
)
target_compile_definitions(test_routine_bank_model PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_routine_bank_model PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_routine_bank_model)
catch_discover_tests(test_routine_bank_model)

# --- test_routine_deck_view (s-rta-0927 routine display, plan-routine-display-A.md 3.5 / 5.1): the deck's
# ROUTINES row, the layer strips' routine bands and the pad settings menu. Header-only
# src/ui/RoutineDeckView.h is a pure function RoutineEngine::Status -> RoutineDeckView; juce_core only --
# NEVER juce_gui_basics, same posture as test_routine_bank_model above.
add_executable(test_routine_deck_view test_routine_deck_view.cpp)
target_include_directories(test_routine_deck_view PRIVATE ${SRC_DIR})
target_link_libraries(test_routine_deck_view PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
)
target_compile_definitions(test_routine_deck_view PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_routine_deck_view PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_routine_deck_view)
catch_discover_tests(test_routine_deck_view)

# --- test_layer_strip_follows_model (s-rta-0927 routine display, plan-routine-display-A.md 2.5 / 5.3): the
# layer strip's V and S faders follow the model (a routine / REST / MIDI / OSC write moves them), the V fill
# turns cyan while a lane-rank hand grips opacity, and the routine bands' x removes the routine. Headless JUCE
# widgets, the test_right_click_reset harness + LayerStrip.cpp + LookAndFeel.cpp (each addition the ONE .cpp
# that defines a symbol the linker named, never MainComponent.cpp/Renderer.cpp).
add_executable(test_layer_strip_follows_model
    test_layer_strip_follows_model.cpp
    ${SRC_DIR}/ui/LayerStrip.cpp
    ${SRC_DIR}/ui/LookAndFeel.cpp
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
target_include_directories(test_layer_strip_follows_model PRIVATE ${SRC_DIR})
target_link_libraries(test_layer_strip_follows_model PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(test_layer_strip_follows_model PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_layer_strip_follows_model PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_layer_strip_follows_model)
catch_discover_tests(test_layer_strip_follows_model)

# --- test_record_panel_pads_removed (s-rta-0927 routine display, plan-routine-display-A.md 2.6 / 5.5): the
# Record tab has no routine pad row any more (the deck's ROUTINES row fires routines); the Save Routine row
# stays. Headless JUCE widgets under ScopedJuceInitialiser_GUI; RecordPanel.cpp + LookAndFeel.cpp, the
# retired tool_routine_strip_snapshot's link set.
add_executable(test_record_panel_pads_removed
    test_record_panel_pads_removed.cpp
    ${SRC_DIR}/ui/RecordPanel.cpp
    ${SRC_DIR}/ui/LookAndFeel.cpp
)
target_include_directories(test_record_panel_pads_removed PRIVATE ${SRC_DIR})
target_link_libraries(test_record_panel_pads_removed PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_record_panel_pads_removed PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_record_panel_pads_removed PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_record_panel_pads_removed)
catch_discover_tests(test_record_panel_pads_removed)

# --- test_routine_pad_press (s-rta-0927 routine display, plan-routine-display-A.md 2.1 / 5.4): a routine pad's
# ONE press action is Fire; a right-click opens the settings menu; an empty pad does nothing; RoutinePad has no
# stop callback (a compile-level pin of the removed press-to-stop grammar). Headless JUCE widgets under
# ScopedJuceInitialiser_GUI.
add_executable(test_routine_pad_press
    test_routine_pad_press.cpp
    ${SRC_DIR}/ui/RoutinePad.cpp
)
target_include_directories(test_routine_pad_press PRIVATE ${SRC_DIR})
target_link_libraries(test_routine_pad_press PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_routine_pad_press PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_routine_pad_press PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_routine_pad_press)
catch_discover_tests(test_routine_pad_press)

# --- test_param_control_routine_cue (s-rta-0927 routine display C7, plan-routine-display-A.md 5.6): a bound
# UniversalParamControl gripped by a lane-rank hand (a routine / a take replay) paints its value digits cyan and
# "ROUTINE" in the collapsed hint slot; a human hand shows nothing new. Pixel-decoded createComponentSnapshot;
# the test_right_click_reset link set.
add_executable(test_param_control_routine_cue
    test_param_control_routine_cue.cpp
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
target_include_directories(test_param_control_routine_cue PRIVATE ${SRC_DIR})
target_link_libraries(test_param_control_routine_cue PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(test_param_control_routine_cue PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_param_control_routine_cue PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_param_control_routine_cue)
catch_discover_tests(test_param_control_routine_cue)

# --- tool_routine_deck_snapshot (s-rta-0927 routine display, plan-routine-display-A.md section 4): a
# ctest-EXCLUDED headless snapshot tool -- deliberately never registered with catch_discover_tests. Renders the
# ROUTINES row (eight real RoutinePads) and one LayerStrip with routine bands, with the app's AudioDNALookAndFeel,
# for synthetic RoutineEngine::Status states through the real deriveRoutineDeckView, into PNGs via
# createComponentSnapshot -- no window, no peer. Run by hand:
#   ./tool_routine_deck_snapshot <output-directory>
add_executable(tool_routine_deck_snapshot
    tool_routine_deck_snapshot.cpp
    ${SRC_DIR}/ui/RoutinePad.cpp
    ${SRC_DIR}/ui/LayerStrip.cpp
    ${SRC_DIR}/ui/LookAndFeel.cpp
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
target_include_directories(tool_routine_deck_snapshot PRIVATE ${SRC_DIR})
target_link_libraries(tool_routine_deck_snapshot PRIVATE
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(tool_routine_deck_snapshot PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(tool_routine_deck_snapshot PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()

# --- test_topbar_model (s-rta-0926b plan3-final.md section 3, ITEM B, B.4): the TopBar's
# one-line bar readout. Header-only src/ui/TopBarModel.h is a pure function bool, uint16_t ->
# juce::String; juce_core only -- NEVER juce_gui_basics (no display context under ctest), same
# posture as test_routine_bank_model above.
add_executable(test_topbar_model test_topbar_model.cpp)
target_include_directories(test_topbar_model PRIVATE ${SRC_DIR})
target_link_libraries(test_topbar_model PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
)
target_compile_definitions(test_topbar_model PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_topbar_model PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_topbar_model)
catch_discover_tests(test_topbar_model)

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

# --- test_deck_tab_row (s-rta-0926b plan6 §6.1: the deck tab row's geometry --
# tabs, the "+" button, the Remove-Deck undo hint -- and its two menus). Pure
# header src/ui/DeckTabRow.h -- no JUCE; DeckView places its buttons from it.
add_executable(test_deck_tab_row test_deck_tab_row.cpp)
target_include_directories(test_deck_tab_row PRIVATE ${SRC_DIR})
target_link_libraries(test_deck_tab_row PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_deck_tab_row)
catch_discover_tests(test_deck_tab_row)

# --- test_lookandfeel_square (s-rta-0926b plan6 fix round: no rounded corners,
# anywhere -- the PopupMenu background and the AlertWindow dialogs drawn with the
# app's own AudioDNALookAndFeel, painted headless into images).
add_executable(test_lookandfeel_square
    test_lookandfeel_square.cpp
    ${SRC_DIR}/ui/LookAndFeel.cpp
)
target_include_directories(test_lookandfeel_square PRIVATE ${SRC_DIR})
target_link_libraries(test_lookandfeel_square PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_lookandfeel_square PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_lookandfeel_square PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_lookandfeel_square)
catch_discover_tests(test_lookandfeel_square)

# --- test_comp_decks_browser (decks-followup ITEM 2: the Compositions-browser
# library's Decks section lists only files that load as a v2 deck ("layers"),
# never a legacy v1 PresetManager::saveDeck file living in the same directory).
add_executable(test_comp_decks_browser
    test_comp_decks_browser.cpp
    ${SRC_DIR}/ui/CompDecksBrowser.cpp
    ${SRC_DIR}/ui/LookAndFeel.cpp
)
target_include_directories(test_comp_decks_browser PRIVATE ${SRC_DIR})
target_link_libraries(test_comp_decks_browser PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_comp_decks_browser PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_comp_decks_browser PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_comp_decks_browser)
catch_discover_tests(test_comp_decks_browser)

# --- test_per_type_autopilot_layout (s-rta-0925 visualfix: the Composition
# Inspector's Per-Type Autopilot enable-checkbox row and its Opaque cycle row
# used to land at the same y). Pure function in
# src/ui/PerTypeAutopilotLayout.h -- no JUCE.
add_executable(test_per_type_autopilot_layout test_per_type_autopilot_layout.cpp)
target_include_directories(test_per_type_autopilot_layout PRIVATE ${SRC_DIR})
target_link_libraries(test_per_type_autopilot_layout PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_per_type_autopilot_layout)
catch_discover_tests(test_per_type_autopilot_layout)

# --- test_signal_depth (s-rta-0925 mastersignal Step 1: SignalDepth.h's
# applyDepth -- pure function, no JUCE.
add_executable(test_signal_depth test_signal_depth.cpp)
target_include_directories(test_signal_depth PRIVATE ${SRC_DIR})
target_link_libraries(test_signal_depth PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_signal_depth)
catch_discover_tests(test_signal_depth)

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

# --- test_master_signal_link (s-rta-0925 mastersignal Step 1: the TopBar
# Signal fader is a second widget-grip view of Composition::masterSignal /
# CompScalar::Signal, built exactly like the Master fader above -- same
# source list, same link-closure rationale).
add_executable(test_master_signal_link
    test_master_signal_link.cpp
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
target_include_directories(test_master_signal_link PRIVATE ${SRC_DIR})
target_link_libraries(test_master_signal_link PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(test_master_signal_link PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_master_signal_link PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_master_signal_link)
catch_discover_tests(test_master_signal_link)

# --- test_topbar_link_toggle (s-rta-0926b bpm2: in a default build -- no
# AUDIODNA_HAS_LINK, as here -- the TopBar "Link" toggle is disabled, dimmed,
# says Link is not in this build, and keeps its bounds). Same TopBar link
# closure and harness as test_master_signal_link above.
add_executable(test_topbar_link_toggle
    test_topbar_link_toggle.cpp
    ${SRC_DIR}/ui/TopBar.cpp
    ${SRC_DIR}/ui/LookAndFeel.cpp
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
target_include_directories(test_topbar_link_toggle PRIVATE ${SRC_DIR})
target_link_libraries(test_topbar_link_toggle PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(test_topbar_link_toggle PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_topbar_link_toggle PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_topbar_link_toggle)
catch_discover_tests(test_topbar_link_toggle)

# --- test_link_sync (s-rta-0926b bpm2: in a default build -- no AUDIODNA_HAS_LINK,
# as here -- LinkSync never reports enabled and never hands out a tempo, so the
# ~30 Hz Link tick cannot force the tracker into manual mode at a fake 120 BPM).
add_executable(test_link_sync
    test_link_sync.cpp
    ${SRC_DIR}/sync/LinkSync.cpp
    ${SRC_DIR}/analysis/BPMTracker.cpp
)
target_include_directories(test_link_sync PRIVATE ${SRC_DIR})
target_link_libraries(test_link_sync PRIVATE
    Catch2::Catch2WithMain
    Aubio::Aubio
)
if(NOT MSVC)
    target_compile_options(test_link_sync PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_link_sync)
catch_discover_tests(test_link_sync)

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

# --- test_effect_stack_binding (s-rta-0925 mastersignal Step 0: EffectStackView
# rows bind to the model's ParamConnection/LiveValue twins; tickModulation()
# becomes display-only; forget-before-rebuild avoids a UAF when the backing
# effects vector is cleared/reallocated). Same headless-JUCE-widgets harness as
# test_right_click_reset above -- link closure grows the same way (Universal
# ParamControl.cpp pulls in the mapping/effects/render chain transitively),
# plus EffectStackView.cpp/EffectLibrary.cpp for the view under test.
add_executable(test_effect_stack_binding
    test_effect_stack_binding.cpp
    ${SRC_DIR}/ui/EffectStackView.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
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
target_include_directories(test_effect_stack_binding PRIVATE ${SRC_DIR})
target_link_libraries(test_effect_stack_binding PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(test_effect_stack_binding PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_effect_stack_binding PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_effect_stack_binding)
catch_discover_tests(test_effect_stack_binding)

# --- test_layer_inspector_persistent_toggle (s-rta-0926b uitoggle): the Persistent toggle's
# enable rule widens to canBePersistent(type) || layer->persistent, so a stale flag on a
# non-persistable type can be cleared but not re-set. Same headless-widgets harness and link
# closure as test_effect_stack_binding above, plus LayerInspector.cpp/MacroPanel.cpp/Knob.cpp
# for the inspector under test.
add_executable(test_layer_inspector_persistent_toggle
    test_layer_inspector_persistent_toggle.cpp
    ${SRC_DIR}/ui/LayerInspector.cpp
    ${SRC_DIR}/ui/MacroPanel.cpp
    ${SRC_DIR}/ui/Knob.cpp
    ${SRC_DIR}/render/FeedbackProcessor.cpp
    ${SRC_DIR}/ui/EffectStackView.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
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
target_include_directories(test_layer_inspector_persistent_toggle PRIVATE ${SRC_DIR})
target_link_libraries(test_layer_inspector_persistent_toggle PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(test_layer_inspector_persistent_toggle PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_layer_inspector_persistent_toggle PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_layer_inspector_persistent_toggle)
catch_discover_tests(test_layer_inspector_persistent_toggle)

# --- tool_uitoggle_snapshot (s-rta-0926b uitoggle SHOTS): a ctest-EXCLUDED headless snapshot
# tool -- deliberately never registered with catch_discover_tests, same posture as
# tool_routine_strip_snapshot above. Renders TopBar / LayerInspector / RecordPanel, with the
# app's own AudioDNALookAndFeel, into PNGs via juce::Component::createComponentSnapshot -- no
# window, no peer, no live app, no port 7070, no lock. Supplementary to (not a substitute for)
# the rig-mandated live-app window screenshots. No Catch2: a manual tool with its own main():
#   ./tool_uitoggle_snapshot <output-directory>
add_executable(tool_uitoggle_snapshot
    tool_uitoggle_snapshot.cpp
    ${SRC_DIR}/ui/TopBar.cpp
    ${SRC_DIR}/ui/LayerInspector.cpp
    ${SRC_DIR}/ui/MacroPanel.cpp
    ${SRC_DIR}/ui/Knob.cpp
    ${SRC_DIR}/render/FeedbackProcessor.cpp
    ${SRC_DIR}/ui/EffectStackView.cpp
    ${SRC_DIR}/effects/EffectLibrary.cpp
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
    ${SRC_DIR}/ui/RecordPanel.cpp
    ${SRC_DIR}/ui/LookAndFeel.cpp
    ${SRC_DIR}/features/FeatureBus.cpp
    ${SRC_DIR}/ui/MenuBarModel.cpp
)
target_include_directories(tool_uitoggle_snapshot PRIVATE ${SRC_DIR})
target_link_libraries(tool_uitoggle_snapshot PRIVATE
    juce::juce_gui_basics
    juce::juce_opengl
)
target_compile_definitions(tool_uitoggle_snapshot PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(tool_uitoggle_snapshot PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()

# --- test_canvas_size_combo (s-rta-0926b canvas fix round: the Composition
# inspector's Resolution dropdown matched on width only, had no fallback and no
# non-16:9 preset, so a 1080x1920 canvas read "1920x1080"). Header-only
# src/ui/CanvasSizeCombo.h -- the code CompositionInspector runs -- driven on a
# real juce::ComboBox, headless (ScopedJuceInitialiser_GUI, no window).
add_executable(test_canvas_size_combo test_canvas_size_combo.cpp)
target_include_directories(test_canvas_size_combo PRIVATE ${SRC_DIR})
target_link_libraries(test_canvas_size_combo PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_canvas_size_combo PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
apply_sanitizers(test_canvas_size_combo)
catch_discover_tests(test_canvas_size_combo)

# --- s-rta-0927 outputs-c1 (plan5 slice C1, .harmony/.reports/s-rta-0926b/plan5-final.md 10.1): the output
# frame path. macOS only (IOSurface), like syphon-check above.
if(APPLE)
    # test_surface_pool: the IOSurface half of output::SharedFrameSet, headless -- NO GL.
    add_executable(test_surface_pool
        test_surface_pool.cpp
        ${SRC_DIR}/output/SurfacePool.cpp
    )
    target_include_directories(test_surface_pool PRIVATE ${SRC_DIR})
    target_link_libraries(test_surface_pool PRIVATE
        Catch2::Catch2WithMain
        "-framework IOSurface"
        "-framework CoreFoundation"
    )
    apply_sanitizers(test_surface_pool)
    catch_discover_tests(test_surface_pool)

    # test_shared_frame_gl: THE OFFSCREEN GATE -- the real publish/present path across three private CGL contexts
    # (writer + two readers), no window. SKIPs loudly on a machine without a GL pixel format.
    add_executable(test_shared_frame_gl
        test_shared_frame_gl.cpp
        ${SRC_DIR}/output/SurfacePool.cpp
        ${SRC_DIR}/output/SharedFrameSet.cpp
        ${SRC_DIR}/output/OutputPresenter.cpp
    )
    target_include_directories(test_shared_frame_gl PRIVATE ${SRC_DIR})
    target_link_libraries(test_shared_frame_gl PRIVATE
        Catch2::Catch2WithMain
        juce::juce_core
        juce::juce_opengl
        "-framework OpenGL"
        "-framework IOSurface"
        "-framework CoreFoundation"
    )
    target_compile_definitions(test_shared_frame_gl PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        GL_SILENCE_DEPRECATION
    )
    if(NOT MSVC)
        target_compile_options(test_shared_frame_gl PRIVATE
            -Wno-old-style-cast
            -Wno-conversion
            -Wno-sign-conversion
        )
    endif()
    apply_sanitizers(test_shared_frame_gl)
    catch_discover_tests(test_shared_frame_gl)
endif()

# --- test_output_menu_model (s-rta-0927 outputs-c2 = plan5 C2, plan5-final.md 10.1): the ONE output item list (the
# Output menu and the TopBar "Outputs" button), the button text and the output keys (built from KeyPress
# descriptions -- no key is ever pressed) and the menu bar's Output menu built from the list. Headless: no window,
# no GL, no Desktop; the code under test is src/output/OutputMenuModel.h and src/ui/MenuBarModel.cpp (case 6).
add_executable(test_output_menu_model
    test_output_menu_model.cpp
    ${SRC_DIR}/ui/MenuBarModel.cpp
)
target_include_directories(test_output_menu_model PRIVATE ${SRC_DIR})
target_link_libraries(test_output_menu_model PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_output_menu_model PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
apply_sanitizers(test_output_menu_model)
catch_discover_tests(test_output_menu_model)

# --- test_output_law (s-rta-0927 outputs-c1, plan5-final.md 10.1): the output window's screen-safety law asserted in
# its SOURCE (no gate may ever open the window): normal level, never key (windowIgnoresKeyPresses), bounds before
# visible. Reads src/ui/OutputWindow.{h,cpp} at test time; comments and literals stripped.
add_executable(test_output_law test_output_law.cpp)
target_link_libraries(test_output_law PRIVATE Catch2::Catch2WithMain)
target_compile_definitions(test_output_law PRIVATE AUDIODNA_SRC_DIR="${SRC_DIR}")
apply_sanitizers(test_output_law)
catch_discover_tests(test_output_law)
