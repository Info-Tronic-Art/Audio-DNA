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
catch_discover_tests(test_ring_buffer)

# --- test_spectral_features ---
add_executable(test_spectral_features
    test_spectral_features.cpp
    ${SRC_DIR}/analysis/SpectralFeatures.cpp
)
target_include_directories(test_spectral_features PRIVATE ${SRC_DIR})
target_link_libraries(test_spectral_features PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(test_spectral_features)

# --- test_feature_bus ---
add_executable(test_feature_bus
    test_feature_bus.cpp
    ${SRC_DIR}/features/FeatureBus.cpp
)
target_include_directories(test_feature_bus PRIVATE ${SRC_DIR})
target_link_libraries(test_feature_bus PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(test_feature_bus)

# --- test_smoother ---
add_executable(test_smoother test_smoother.cpp)
target_include_directories(test_smoother PRIVATE ${SRC_DIR})
target_link_libraries(test_smoother PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(test_smoother)

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
catch_discover_tests(test_integration_pipeline)

# --- test_mapping_engine ---
add_executable(test_mapping_engine
    test_mapping_engine.cpp
    ${SRC_DIR}/mapping/MappingEngine.cpp
    ${SRC_DIR}/effects/Effect.cpp
    ${SRC_DIR}/effects/EffectChain.cpp
    ${SRC_DIR}/render/ShaderManager.cpp
    ${SRC_DIR}/render/TextureManager.cpp
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
catch_discover_tests(test_mapping_engine)
