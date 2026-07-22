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

# --- FFmpeg (video decode: libavformat, libavcodec, libavutil, libswscale) ---
find_package(FFmpeg REQUIRED)

# --- libprojectM-4 (MilkDrop visualizer, optional) ---
find_package(ProjectM QUIET)

# --- JUCE via FetchContent ---
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# --- Ableton Link (network tempo sync, optional) ---
option(AUDIODNA_BUILD_LINK "Build with Ableton Link network tempo sync" OFF)
if(AUDIODNA_BUILD_LINK)
    FetchContent_Declare(
        AbletonLink
        GIT_REPOSITORY https://github.com/Ableton/link.git
        GIT_TAG        Link-3.1.2
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(AbletonLink)
endif()

# --- cpp-httplib (HTTP server: production API + test server) ---
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(httplib)

# --- Visual Test Server (Eyes, optional) ---
option(AUDIODNA_BUILD_TEST_SERVER "Build with embedded HTTP test server (Eyes)" OFF)

# --- Melatonin Inspector (JUCE component inspector, optional debug tool) ---
option(AUDIODNA_BUILD_INSPECTOR "Build with Melatonin Inspector debug overlay" OFF)
if(AUDIODNA_BUILD_INSPECTOR)
    FetchContent_Declare(
        melatonin_inspector
        GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
        GIT_TAG        cd25631
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(melatonin_inspector)
endif()

# --- Syphon (macOS inter-app GPU texture sharing, optional) ---
if(APPLE)
    option(AUDIODNA_BUILD_SYPHON "Build with Syphon output (macOS)" OFF)
endif()

# --- Application Target ---
juce_add_gui_app(AudioDNA
    PRODUCT_NAME "Audio-DNA"
    COMPANY_NAME "AudioDNA"
    BUNDLE_ID "com.audiodna.app"
    VERSION "${PROJECT_VERSION}"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Audio-DNA needs microphone access to analyze audio and drive visual effects."
    CAMERA_PERMISSION_ENABLED TRUE
    CAMERA_PERMISSION_TEXT "Audio-DNA needs camera access to use live video as a visual source."
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
    src/analysis/GenreDetector.h
    src/analysis/GenreDetector.cpp
    src/analysis/AdvancedAudioAnalyzer.h
    src/analysis/AdvancedAudioAnalyzer.cpp

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
    src/render/LUTLoader.h
    src/render/LUTLoader.cpp

    # Mapping
    src/mapping/MappingTypes.h
    src/mapping/CurveTransforms.h
    src/mapping/MappingEngine.h
    src/mapping/MappingEngine.cpp

    # Core (v2)
    src/core/Command.h
    src/core/CompositeCommand.h
    src/core/ClipCommands.h
    src/core/DeckCommands.h
    src/core/EffectScope.h
    src/core/EffectCommands.h
    src/core/MediaReconnect.h
    src/core/UndoManager.h
    src/core/UndoManager.cpp
    src/core/UndoService.h
    src/core/UndoService.cpp

    # Model (v2)
    src/model/Composition.h
    src/model/Deck.h
    src/model/Layer.h
    src/model/Layer.cpp
    src/model/Clip.h
    src/model/Clip.cpp
    src/model/Autopilot.h
    src/model/Autopilot.cpp

    # Signal (v2)
    src/signal/Signal.h
    src/signal/AudioSignal.h
    src/signal/OscillatorSignal.h
    src/signal/EnvelopeSignal.h
    src/signal/ClipPositionSignal.h
    src/signal/SignalRegistry.h
    src/signal/SignalRegistry.cpp

    # Routing (v2)
    src/routing/Route.h
    src/routing/RoutingEngine.h
    src/routing/RoutingEngine.cpp
    src/routing/MacroBank.h

    # Binding (v2)
    src/binding/Binding.h
    src/binding/BindingManager.h
    src/binding/BindingManager.cpp

    # MIDI (v2 + P22.10 output)
    src/midi/MidiHandler.h
    src/midi/MidiHandler.cpp
    src/midi/MidiOutputHandler.h
    src/midi/MidiOutputHandler.cpp

    # Sync (v2 - P21: Ableton Link)
    src/sync/LinkSync.h
    src/sync/LinkSync.cpp

    # Media (v2 - Phase 11: Video playback)
    src/media/VideoPlayer.h
    src/media/VideoPlayer.cpp
    src/media/ImageSequence.h
    src/media/ImageSequence.cpp

    # Recording (v2 - Phase 12 + P22.6 video recording)
    src/recording/SessionRecorder.h
    src/recording/SessionRecorder.cpp
    src/recording/VideoRecorder.h
    src/recording/VideoRecorder.cpp

    # Sources (v2 - Phase 10)
    src/sources/ProceduralSource.h
    src/sources/ProceduralSource.cpp
    src/sources/SourceRegistry.h
    src/sources/SourceRegistry.cpp
    src/sources/ProjectMSource.h
    src/sources/ProjectMSource.cpp
    src/sources/ProjectMPresetManager.h
    src/sources/ProjectMPresetManager.cpp
    src/sources/PresetSelector.h
    src/sources/PresetSelector.cpp

    # Effects
    src/effects/Effect.h
    src/effects/Effect.cpp
    src/effects/EffectChain.h
    src/effects/EffectChain.cpp
    src/effects/EffectLibrary.h
    src/effects/EffectLibrary.cpp
    src/effects/ISFShaderLoader.h
    src/effects/ISFShaderLoader.cpp

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
    src/ui/OutputWindow.h
    src/ui/OutputWindow.cpp
    src/ui/SignalStrip.h
    src/ui/SignalStrip.cpp
    src/ui/SignalBar.h
    src/ui/SignalBar.cpp
    src/ui/TopBar.h
    src/ui/TopBar.cpp
    src/ui/ClipCell.h
    src/ui/ClipCell.cpp
    src/ui/LayerStrip.h
    src/ui/LayerStrip.cpp
    src/ui/DeckView.h
    src/ui/DeckView.cpp
    src/ui/UniversalParamControl.h
    src/ui/UniversalParamControl.cpp
    src/ui/EffectStackView.h
    src/ui/EffectStackView.cpp
    src/ui/MacroPanel.h
    src/ui/MacroPanel.cpp
    src/ui/ClipInspector.h
    src/ui/ClipInspector.cpp
    src/ui/LayerInspector.h
    src/ui/LayerInspector.cpp
    src/ui/CompositionInspector.h
    src/ui/CompositionInspector.cpp
    src/ui/SignalInspector.h
    src/ui/SignalInspector.cpp
    src/ui/InspectorPanel.h
    src/ui/InspectorPanel.cpp
    src/ui/TimingWindow.h
    src/ui/TimingWindow.cpp
    src/ui/FilesBrowser.h
    src/ui/FilesBrowser.cpp
    src/ui/FXBrowser.h
    src/ui/FXBrowser.cpp
    src/ui/SourcesBrowser.h
    src/ui/SourcesBrowser.cpp
    src/ui/CompDecksBrowser.h
    src/ui/CompDecksBrowser.cpp
    src/ui/RecordPanel.h
    src/ui/RecordPanel.cpp
    src/ui/BrowserPanel.h
    src/ui/BrowserPanel.cpp
    src/ui/MilkDropBrowser.h
    src/ui/MilkDropBrowser.cpp
    src/ui/MenuBarModel.h
    src/ui/MenuBarModel.cpp
    src/ui/PreferencesDialog.h
    src/ui/PreferencesDialog.cpp
    src/ui/BindingOverlay.h
    src/ui/BindingOverlay.cpp
    src/ui/MidiLearnOverlay.h
    src/ui/MidiLearnOverlay.cpp
    src/render/CompositorEngine.h
    src/render/CompositorEngine.cpp
    src/render/FeedbackProcessor.h
    src/render/FeedbackProcessor.cpp
    src/render/EmbeddedShaders.h

    # API server (P22.8 — always available)
    src/api/ApiServer.h
    src/api/ApiServer.cpp

    # OSC handler (P22.9)
    src/osc/OscHandler.h
    src/osc/OscHandler.cpp

    # Output / texture sharing (P22.1-P22.5)
    src/output/SyphonOutput.h
)

# Conditionally add test server sources
if(AUDIODNA_BUILD_TEST_SERVER)
    target_sources(AudioDNA PRIVATE
        src/test/TestServer.h
        src/test/TestServer.cpp
    )
endif()

# Syphon Obj-C++ sources (macOS only)
if(APPLE)
    target_sources(AudioDNA PRIVATE
        src/output/SyphonOutput.mm
    )
endif()

target_include_directories(AudioDNA PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Camera support: macOS and Windows only (Linux lacks juce_video camera support)
if(APPLE OR WIN32)
    set(AUDIODNA_USE_CAMERA 1)
else()
    set(AUDIODNA_USE_CAMERA 0)
endif()

target_compile_definitions(AudioDNA PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_CAMERA=${AUDIODNA_USE_CAMERA}
    AUDIODNA_HAS_CAMERA=${AUDIODNA_USE_CAMERA}
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:AudioDNA,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:AudioDNA,JUCE_VERSION>"
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Add projectM support if found
if(ProjectM_FOUND)
    target_compile_definitions(AudioDNA PRIVATE AUDIODNA_HAS_PROJECTM=1)
    target_link_libraries(AudioDNA PRIVATE ProjectM::ProjectM)
endif()

# httplib is always linked (production API server + optional test server)
target_link_libraries(AudioDNA PRIVATE httplib::httplib)

# Add test server compile definition
if(AUDIODNA_BUILD_TEST_SERVER)
    target_compile_definitions(AudioDNA PRIVATE AUDIODNA_TEST_SERVER=1)
endif()

# Melatonin Inspector (debug overlay)
if(AUDIODNA_BUILD_INSPECTOR)
    target_compile_definitions(AudioDNA PRIVATE AUDIODNA_BUILD_INSPECTOR=1)
    target_link_libraries(AudioDNA PRIVATE melatonin_inspector)
endif()

# Syphon framework (optional, macOS only)
if(APPLE AND AUDIODNA_BUILD_SYPHON)
    find_library(SYPHON_FRAMEWORK Syphon PATHS /Library/Frameworks)
    if(SYPHON_FRAMEWORK)
        target_link_libraries(AudioDNA PRIVATE ${SYPHON_FRAMEWORK})
        target_compile_definitions(AudioDNA PRIVATE AUDIODNA_HAS_SYPHON=1)
    else()
        message(WARNING "Syphon.framework not found in /Library/Frameworks — Syphon disabled")
    endif()
endif()

# Add Ableton Link support if enabled
if(AUDIODNA_BUILD_LINK)
    target_compile_definitions(AudioDNA PRIVATE AUDIODNA_HAS_LINK=1 LINK_PLATFORM_MACOSX=1)
    target_include_directories(AudioDNA PRIVATE ${abletonlink_SOURCE_DIR}/include)
    target_include_directories(AudioDNA PRIVATE ${abletonlink_SOURCE_DIR}/modules/asio-standalone/asio/include)
endif()

set(AUDIODNA_LIBS
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
    juce::juce_osc
    Aubio::Aubio
    FFmpeg::FFmpeg
)
if(AUDIODNA_USE_CAMERA)
    list(APPEND AUDIODNA_LIBS juce::juce_video)
endif()
target_link_libraries(AudioDNA PRIVATE ${AUDIODNA_LIBS})

set_project_warnings(AudioDNA)

# --- Tests ---
enable_testing()
add_subdirectory(tests)

# Suppress warnings in JUCE headers (GCC/Clang only)
if(NOT MSVC)
    target_compile_options(AudioDNA PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
