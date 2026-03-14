cmake_minimum_required(VERSION 3.24)
project(AudioDNA VERSION 0.1.0 LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CompilerWarnings.cmake)

# --- Aubio (audio analysis: onset, tempo, pitch) ---
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Aubio REQUIRED)

# --- JUCE via FetchContent ---
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# --- Application Target ---
juce_add_gui_app(AudioDNA
    PRODUCT_NAME "Audio-DNA"
    COMPANY_NAME "AudioDNA"
    BUNDLE_ID "com.audiodna.app"
    VERSION "${PROJECT_VERSION}"
)

target_sources(AudioDNA PRIVATE
    # Entry point
    src/Main.cpp
    src/MainComponent.h
    src/MainComponent.cpp

    # Audio
    src/audio/AudioEngine.h
    src/audio/AudioEngine.cpp
    src/audio/AudioCallback.h
    src/audio/AudioCallback.cpp
    src/audio/RingBuffer.h

    # Analysis
    src/analysis/AnalysisThread.h
    src/analysis/AnalysisThread.cpp
    src/analysis/FeatureSnapshot.h
    src/analysis/FFTProcessor.h
    src/analysis/FFTProcessor.cpp
    src/analysis/SpectralFeatures.h
    src/analysis/SpectralFeatures.cpp
    src/analysis/OnsetDetector.h
    src/analysis/OnsetDetector.cpp
    src/analysis/BPMTracker.h
    src/analysis/BPMTracker.cpp
    src/analysis/MFCCExtractor.h
    src/analysis/MFCCExtractor.cpp
    src/analysis/ChromaExtractor.h
    src/analysis/ChromaExtractor.cpp
    src/analysis/KeyDetector.h
    src/analysis/KeyDetector.cpp
    src/analysis/LoudnessAnalyzer.h
    src/analysis/LoudnessAnalyzer.cpp
    src/analysis/StructuralDetector.h
    src/analysis/StructuralDetector.cpp
    src/analysis/PitchTracker.h
    src/analysis/PitchTracker.cpp

    # Features
    src/features/FeatureBus.h
    src/features/FeatureBus.cpp
    src/features/Smoother.h

    # Render
    src/render/Renderer.h
    src/render/Renderer.cpp
    src/render/FullscreenQuad.h
    src/render/FullscreenQuad.cpp
    src/render/ShaderManager.h
    src/render/ShaderManager.cpp
    src/render/TextureManager.h
    src/render/TextureManager.cpp

    # Mapping
    src/mapping/MappingTypes.h
    src/mapping/CurveTransforms.h
    src/mapping/MappingEngine.h
    src/mapping/MappingEngine.cpp

    # Effects
    src/effects/Effect.h
    src/effects/Effect.cpp
    src/effects/EffectChain.h
    src/effects/EffectChain.cpp
    src/effects/EffectLibrary.h
    src/effects/EffectLibrary.cpp
    src/effects/UniformBridge.h
    src/effects/UniformBridge.cpp

    # UI
    src/ui/LookAndFeel.h
    src/ui/LookAndFeel.cpp
    src/ui/WaveformDisplay.h
    src/ui/WaveformDisplay.cpp
    src/ui/AudioReadoutPanel.h
    src/ui/AudioReadoutPanel.cpp
    src/ui/SpectrumDisplay.h
    src/ui/SpectrumDisplay.cpp
    src/ui/PreviewPanel.h
    src/ui/PreviewPanel.cpp
    src/ui/MappingEditor.h
    src/ui/MappingEditor.cpp
    src/ui/EffectsRackPanel.h
    src/ui/EffectsRackPanel.cpp
    src/ui/Knob.h
    src/ui/Knob.cpp
    src/ui/PresetManager.h
    src/ui/PresetManager.cpp
)

target_include_directories(AudioDNA PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(AudioDNA PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:AudioDNA,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:AudioDNA,JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(AudioDNA PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_dsp
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_opengl
    Aubio::Aubio
)

set_project_warnings(AudioDNA)

# --- Tests ---
enable_testing()
add_subdirectory(tests)

# Suppress warnings in JUCE headers
target_compile_options(AudioDNA PRIVATE
    -Wno-old-style-cast
    -Wno-conversion
    -Wno-sign-conversion
)
