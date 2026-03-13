cmake_minimum_required(VERSION 3.24)
project(AudioDNA VERSION 0.1.0 LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CompilerWarnings.cmake)

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

    # Features
    src/features/FeatureBus.h
    src/features/FeatureBus.cpp

    # UI
    src/ui/LookAndFeel.h
    src/ui/LookAndFeel.cpp
    src/ui/WaveformDisplay.h
    src/ui/WaveformDisplay.cpp
    src/ui/AudioReadoutPanel.h
    src/ui/AudioReadoutPanel.cpp
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
)

set_project_warnings(AudioDNA)

# Suppress warnings in JUCE headers
target_compile_options(AudioDNA PRIVATE
    -Wno-old-style-cast
    -Wno-conversion
    -Wno-sign-conversion
)
