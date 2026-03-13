# Project Setup Guide: Cross-Platform Real-Time Audio-Visual Application

**Document ID:** IMPL_project_setup
**Status:** Active
**Cross-references:** [IMPL_minimal_prototype.md](IMPL_minimal_prototype.md) | [LIB_juce.md](LIB_juce.md) | [LIB_essentia.md](LIB_essentia.md) | [LIB_aubio.md](LIB_aubio.md) | [LIB_fft_comparison.md](LIB_fft_comparison.md) | [ARCH_pipeline.md](ARCH_pipeline.md) | [ARCH_audio_io.md](ARCH_audio_io.md) | [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md)

---

## 1. Source Tree Structure

The project follows a module-oriented source layout. Each directory maps to a logical subsystem in the pipeline described in [ARCH_pipeline.md](ARCH_pipeline.md). The separation enforces compile-time isolation between stages -- the audio callback code in `src/audio/` never includes headers from `src/render/`, preventing accidental introduction of OpenGL or heap-allocating types into the real-time path (see [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md), Rule 1).

```
project/
├── CMakeLists.txt              # Root build configuration
├── cmake/
│   ├── CompilerWarnings.cmake  # Warning flags per compiler
│   ├── Sanitizers.cmake        # ASAN/TSAN/UBSAN toggle
│   ├── FindEssentia.cmake      # Custom find module
│   ├── FindAubio.cmake         # Custom find module
│   └── Platform.cmake          # Per-OS framework linking
├── src/
│   ├── audio/                  # Audio I/O layer
│   │   ├── AudioDeviceManager.h/cpp
│   │   ├── AudioCallback.h/cpp
│   │   ├── RingBuffer.h        # Lock-free SPSC ring buffer
│   │   └── AudioConfig.h       # Sample rate, buffer size constants
│   ├── analysis/               # DSP and feature extraction
│   │   ├── FFTProcessor.h/cpp  # FFTW3/Accelerate wrapper
│   │   ├── SpectralAnalyzer.h/cpp
│   │   ├── OnsetDetector.h/cpp # Aubio onset wrapper
│   │   ├── PitchTracker.h/cpp  # Aubio/Essentia pitch
│   │   ├── BeatTracker.h/cpp   # Essentia beat tracker
│   │   └── MFCCExtractor.h/cpp # Essentia MFCC
│   ├── features/               # Feature bus and temporal smoothing
│   │   ├── FeatureBus.h/cpp    # Thread-safe feature store
│   │   ├── Smoother.h/cpp      # Exponential smoothing, envelope followers
│   │   └── FeatureTypes.h      # POD structs for all feature data
│   ├── render/                 # OpenGL rendering
│   │   ├── Renderer.h/cpp      # GL context, frame loop
│   │   ├── ShaderProgram.h/cpp # Shader compile/link wrapper
│   │   ├── FrameBuffer.h/cpp   # FBO management
│   │   └── UniformBridge.h/cpp # Maps feature bus to shader uniforms
│   ├── ui/                     # User interface and configuration
│   │   ├── MainWindow.h/cpp    # JUCE or Dear ImGui window
│   │   ├── ParameterPanel.h/cpp
│   │   └── PresetManager.h/cpp
│   └── main.cpp                # Entry point, subsystem init
├── shaders/
│   ├── vertex.glsl
│   ├── fragment.glsl
│   └── compute_spectrum.glsl   # Optional compute shader for spectrum vis
├── tests/
│   ├── CMakeLists.txt
│   ├── test_ring_buffer.cpp
│   ├── test_fft_processor.cpp
│   ├── test_feature_bus.cpp
│   ├── test_smoother.cpp
│   └── test_onset_detector.cpp
├── third_party/
│   ├── CMakeLists.txt          # Aggregates submodule/FetchContent deps
│   ├── juce/                   # Git submodule or FetchContent
│   └── imgui/                  # If using Dear ImGui instead of JUCE UI
├── vcpkg.json                  # vcpkg manifest (if using vcpkg)
├── conanfile.txt               # Conan manifest (if using Conan)
├── .clang-format
├── .clang-tidy
└── .github/
    └── workflows/
        └── build.yml           # CI/CD matrix build
```

### Directory Purposes in Detail

**`src/audio/`** -- This is the real-time boundary. Code here runs on the OS audio thread (CoreAudio render callback, WASAPI event-driven thread, JACK process callback). Every type in this directory must obey the constraints in [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md): no heap allocation, no locks, no system calls, no I/O. The `RingBuffer.h` implements an SPSC (single-producer, single-consumer) lock-free queue using `std::atomic` with `memory_order_acquire`/`memory_order_release` semantics. The audio callback writes raw PCM into this ring buffer; the analysis thread reads from it. This is the only coupling point between Stage 1 (Audio Capture) and Stage 2 (Analysis) of the pipeline.

**`src/analysis/`** -- The DSP workhorse. Runs on a dedicated analysis thread (not the audio thread). This code may allocate on initialization but must not allocate per-frame during steady-state operation. The `FFTProcessor` wraps either FFTW3 (Linux/Windows) or vDSP via Accelerate (macOS) behind a compile-time abstraction. `OnsetDetector` and `PitchTracker` wrap Aubio's `aubio_onset_t` and `aubio_pitch_t` objects. `BeatTracker` and `MFCCExtractor` wrap Essentia algorithms. All wrappers follow RAII -- Aubio/Essentia objects are created in the constructor and destroyed in the destructor.

**`src/features/`** -- The "feature bus" is a fixed-size struct (`FeatureSnapshot`) containing all extracted features for a single analysis frame: RMS, spectral centroid, spectral flux, onset strength, pitch, beat phase, MFCC coefficients (typically 13), and any derived features. The `FeatureBus` uses a triple-buffer pattern (write buffer, ready buffer, read buffer) with a single `std::atomic<int>` index swap, allowing the analysis thread to publish new snapshots without blocking the render thread. The `Smoother` applies exponential moving averages and attack/release envelope followers to prevent visual jitter.

**`src/render/`** -- OpenGL 4.1+ core profile rendering. This code runs on the main thread (which owns the GL context). The `UniformBridge` reads the latest `FeatureSnapshot` from the feature bus and maps each field to a named shader uniform. The `Renderer` drives the frame loop: poll features, update uniforms, bind framebuffer, draw fullscreen quad, swap buffers. Frame timing targets 60 fps via vsync or a manual timer.

**`src/ui/`** -- If using JUCE, this wraps `juce::DocumentWindow` and `juce::Component` subclasses for parameter sliders, device selectors, and preset management. If using Dear ImGui, it integrates via the OpenGL backend (`imgui_impl_opengl3.h`). The UI never touches the audio callback directly -- it writes to atomic configuration variables that the audio or analysis threads read.

**`shaders/`** -- GLSL source files loaded at runtime via `ShaderProgram::loadFromFile()`. Kept outside `src/` so they can be edited and hot-reloaded without recompilation. The build system copies them to the output directory via `configure_file()` or a custom command.

**`tests/`** -- Unit tests using Catch2 or Google Test. Tests verify the lock-free ring buffer under concurrent access, FFT output against known signals (sine wave -> single bin), feature bus triple-buffer correctness, and smoother convergence behavior. Tests do NOT test audio hardware -- they use synthetic PCM buffers.

**`cmake/`** -- Modular CMake scripts included from the root `CMakeLists.txt`. Separating concerns (warnings, sanitizers, platform detection, find modules) keeps the root file readable and maintainable.

**`third_party/`** -- External dependencies managed as git submodules or pulled via FetchContent. Has its own `CMakeLists.txt` that calls `add_subdirectory()` on each dependency. This isolates third-party warning noise from the main build (see compiler flags section).

---

## 2. CMakeLists.txt -- Complete Build Configuration

```cmake
cmake_minimum_required(VERSION 3.24)

# ─── vcpkg toolchain (must be before project()) ──────────────────────────────
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain")
endif()

project(RealTimeAudioVis
    VERSION 0.1.0
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # For clangd

# ─── Build type defaults ─────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE
        PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel" "Profile")
endif()

# ─── Options ──────────────────────────────────────────────────────────────────
option(RTAV_USE_JUCE        "Use JUCE for audio I/O and UI"          ON)
option(RTAV_USE_ESSENTIA    "Enable Essentia-based feature extraction" ON)
option(RTAV_USE_AUBIO       "Enable Aubio-based onset/pitch detection" ON)
option(RTAV_USE_FFTW        "Use FFTW3 (OFF = use vDSP on macOS)"   ON)
option(RTAV_BUILD_TESTS     "Build unit tests"                       ON)
option(RTAV_ENABLE_ASAN     "Enable AddressSanitizer"                OFF)
option(RTAV_ENABLE_TSAN     "Enable ThreadSanitizer"                 OFF)
option(RTAV_ENABLE_UBSAN    "Enable UndefinedBehaviorSanitizer"      OFF)
option(RTAV_ENABLE_LTO      "Enable Link-Time Optimization"          OFF)

# ─── Include modular cmake scripts ───────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Sanitizers)
include(Platform)

# ─── LTO ──────────────────────────────────────────────────────────────────────
if(RTAV_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT lto_error)
    if(lto_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "LTO enabled")
    else()
        message(WARNING "LTO not supported: ${lto_error}")
    endif()
endif()

# ═══════════════════════════════════════════════════════════════════════════════
# DEPENDENCIES
# ═══════════════════════════════════════════════════════════════════════════════

# ─── JUCE ─────────────────────────────────────────────────────────────────────
if(RTAV_USE_JUCE)
    # Option A: FetchContent (downloads at configure time)
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.12   # Pin to a specific release tag
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)

    # Option B (alternative): git submodule in third_party/juce
    # add_subdirectory(third_party/juce)
endif()

# ─── FFTW3 ────────────────────────────────────────────────────────────────────
if(RTAV_USE_FFTW)
    # vcpkg or system-installed FFTW3
    find_package(FFTW3 CONFIG)
    if(NOT FFTW3_FOUND)
        find_package(FFTW3 REQUIRED)  # Falls back to FindFFTW3.cmake
    endif()
    # FFTW3 provides FFTW3::fftw3 or FFTW3::fftw3f (single-precision)
    # We use single-precision for audio (float, not double)
    if(TARGET FFTW3::fftw3f)
        set(FFTW_TARGET FFTW3::fftw3f)
    elseif(TARGET FFTW3::fftw3)
        set(FFTW_TARGET FFTW3::fftw3)
    else()
        # Manual fallback
        find_library(FFTW3F_LIB NAMES fftw3f)
        find_path(FFTW3_INCLUDE_DIR NAMES fftw3.h)
        add_library(fftw3f_imported IMPORTED INTERFACE)
        target_link_libraries(fftw3f_imported INTERFACE ${FFTW3F_LIB})
        target_include_directories(fftw3f_imported INTERFACE ${FFTW3_INCLUDE_DIR})
        set(FFTW_TARGET fftw3f_imported)
    endif()
endif()

# ─── Essentia ─────────────────────────────────────────────────────────────────
if(RTAV_USE_ESSENTIA)
    # Essentia does not ship a CMake config; use a custom find module
    # or link manually after building with waf
    find_package(Essentia)
    if(NOT Essentia_FOUND)
        # Manual fallback: user must set ESSENTIA_ROOT
        if(NOT DEFINED ESSENTIA_ROOT)
            message(FATAL_ERROR
                "Essentia not found. Set ESSENTIA_ROOT to the Essentia install prefix, "
                "or install via: ./waf configure --prefix=/usr/local && ./waf && ./waf install"
            )
        endif()
        add_library(essentia IMPORTED STATIC)
        set_target_properties(essentia PROPERTIES
            IMPORTED_LOCATION "${ESSENTIA_ROOT}/lib/libessentia.a"
            INTERFACE_INCLUDE_DIRECTORIES "${ESSENTIA_ROOT}/include/essentia"
        )
        # Essentia depends on: yaml-cpp, fftw3f, samplerate, taglib, chromaprint
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(YAML REQUIRED yaml-0.1)
        pkg_check_modules(SAMPLERATE REQUIRED samplerate)
        target_link_libraries(essentia INTERFACE
            ${YAML_LIBRARIES} ${SAMPLERATE_LIBRARIES}
        )
        set(ESSENTIA_TARGET essentia)
    else()
        set(ESSENTIA_TARGET Essentia::Essentia)
    endif()
endif()

# ─── Aubio ────────────────────────────────────────────────────────────────────
if(RTAV_USE_AUBIO)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(AUBIO REQUIRED aubio>=0.4.9)
    # Creates AUBIO_INCLUDE_DIRS, AUBIO_LIBRARIES, AUBIO_LINK_LIBRARIES
    add_library(aubio_imported IMPORTED INTERFACE)
    target_include_directories(aubio_imported INTERFACE ${AUBIO_INCLUDE_DIRS})
    target_link_libraries(aubio_imported INTERFACE ${AUBIO_LINK_LIBRARIES})
    set(AUBIO_TARGET aubio_imported)
endif()

# ─── OpenGL ───────────────────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)

# GLAD or GLEW for extension loading (FetchContent example with GLAD)
include(FetchContent)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.6
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad_gl REPRODUCIBLE API gl:core=4.1)

# GLFW for windowing (if not using JUCE for the GL window)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ─── GLM (header-only math) ──────────────────────────────────────────────────
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glm)

# ═══════════════════════════════════════════════════════════════════════════════
# APPLICATION TARGET
# ═══════════════════════════════════════════════════════════════════════════════

set(RTAV_SOURCES
    src/main.cpp
    src/audio/AudioDeviceManager.cpp
    src/audio/AudioCallback.cpp
    src/analysis/FFTProcessor.cpp
    src/analysis/SpectralAnalyzer.cpp
    src/analysis/OnsetDetector.cpp
    src/analysis/PitchTracker.cpp
    src/analysis/BeatTracker.cpp
    src/analysis/MFCCExtractor.cpp
    src/features/FeatureBus.cpp
    src/features/Smoother.cpp
    src/render/Renderer.cpp
    src/render/ShaderProgram.cpp
    src/render/FrameBuffer.cpp
    src/render/UniformBridge.cpp
    src/ui/MainWindow.cpp
    src/ui/ParameterPanel.cpp
    src/ui/PresetManager.cpp
)

if(RTAV_USE_JUCE)
    # JUCE applications use juce_add_gui_app or juce_add_plugin
    juce_add_gui_app(RealTimeAudioVis
        PRODUCT_NAME "RealTimeAudioVis"
        COMPANY_NAME "RealTimeAudioVis"
        VERSION "${PROJECT_VERSION}"
    )
    target_sources(RealTimeAudioVis PRIVATE ${RTAV_SOURCES})

    # JUCE modules
    target_link_libraries(RealTimeAudioVis PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_opengl
    )

    target_compile_definitions(RealTimeAudioVis PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:RealTimeAudioVis,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:RealTimeAudioVis,JUCE_VERSION>"
        RTAV_USE_JUCE=1
    )
else()
    add_executable(RealTimeAudioVis ${RTAV_SOURCES})
    target_compile_definitions(RealTimeAudioVis PRIVATE RTAV_USE_JUCE=0)
endif()

# ─── Include directories ─────────────────────────────────────────────────────
target_include_directories(RealTimeAudioVis PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ─── Link dependencies ───────────────────────────────────────────────────────
target_link_libraries(RealTimeAudioVis PRIVATE
    OpenGL::GL
    glad_gl
    glfw
    glm::glm
)

if(RTAV_USE_FFTW)
    target_link_libraries(RealTimeAudioVis PRIVATE ${FFTW_TARGET})
    target_compile_definitions(RealTimeAudioVis PRIVATE RTAV_USE_FFTW=1)
endif()

if(RTAV_USE_ESSENTIA)
    target_link_libraries(RealTimeAudioVis PRIVATE ${ESSENTIA_TARGET})
    target_compile_definitions(RealTimeAudioVis PRIVATE RTAV_USE_ESSENTIA=1)
endif()

if(RTAV_USE_AUBIO)
    target_link_libraries(RealTimeAudioVis PRIVATE ${AUBIO_TARGET})
    target_compile_definitions(RealTimeAudioVis PRIVATE RTAV_USE_AUBIO=1)
endif()

# ─── Apply compiler warnings (from cmake/CompilerWarnings.cmake) ─────────────
rtav_set_warnings(RealTimeAudioVis)

# ─── Apply sanitizers (from cmake/Sanitizers.cmake) ──────────────────────────
rtav_set_sanitizers(RealTimeAudioVis)

# ─── Platform frameworks (from cmake/Platform.cmake) ─────────────────────────
rtav_link_platform(RealTimeAudioVis)

# ─── Copy shaders to build directory ─────────────────────────────────────────
add_custom_command(TARGET RealTimeAudioVis POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:RealTimeAudioVis>/shaders"
    COMMENT "Copying shaders to output directory"
)

# ═══════════════════════════════════════════════════════════════════════════════
# TESTS
# ═══════════════════════════════════════════════════════════════════════════════

if(RTAV_BUILD_TESTS)
    enable_testing()

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.6.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)

    add_executable(rtav_tests
        tests/test_ring_buffer.cpp
        tests/test_fft_processor.cpp
        tests/test_feature_bus.cpp
        tests/test_smoother.cpp
        tests/test_onset_detector.cpp
    )

    target_include_directories(rtav_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(rtav_tests PRIVATE
        Catch2::Catch2WithMain
    )

    # Link only the subsystems under test (not the full app)
    # Build analysis/features as a static library for testability
    if(RTAV_USE_FFTW)
        target_link_libraries(rtav_tests PRIVATE ${FFTW_TARGET})
        target_compile_definitions(rtav_tests PRIVATE RTAV_USE_FFTW=1)
    endif()
    if(RTAV_USE_AUBIO)
        target_link_libraries(rtav_tests PRIVATE ${AUBIO_TARGET})
        target_compile_definitions(rtav_tests PRIVATE RTAV_USE_AUBIO=1)
    endif()

    catch_discover_tests(rtav_tests)
endif()

# ─── Install ──────────────────────────────────────────────────────────────────
install(TARGETS RealTimeAudioVis
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)
install(DIRECTORY shaders/ DESTINATION share/RealTimeAudioVis/shaders)
```

### cmake/CompilerWarnings.cmake

```cmake
function(rtav_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4         # High warning level (MSVC equivalent of -Wall -Wextra)
            /WX         # Warnings as errors
            /w14640     # Thread-unsafe static init
            /w14826     # Narrowing conversion
            /w14928     # Illegal copy-init
            /permissive-  # Strict standards conformance
            /Zc:__cplusplus  # Report correct __cplusplus value
        )
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror              # Warnings as errors in CI; remove locally if needed
            -Wshadow             # Variable shadowing
            -Wconversion         # Implicit narrowing conversions
            -Wsign-conversion    # Signed/unsigned mismatch
            -Wnon-virtual-dtor  # Classes with virtual functions lack virtual dtor
            -Wold-style-cast    # C-style casts in C++ code
            -Woverloaded-virtual # Overloaded (not overridden) virtual
            -Wnull-dereference  # Static analysis null deref
            -Wdouble-promotion  # Implicit float -> double
            -Wformat=2          # printf format string checks
        )
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target} PRIVATE
                -Wmisleading-indentation  # GCC-specific
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
                -Wuseless-cast
            )
        endif()
    endif()

    # Suppress warnings in third-party headers
    # (SYSTEM include dirs get -isystem which silences warnings)
    get_target_property(inc_dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
    # Third-party targets added via target_link_libraries already use SYSTEM
endfunction()
```

### cmake/Sanitizers.cmake

```cmake
function(rtav_set_sanitizers target)
    # Sanitizers are mutually exclusive in some combinations.
    # ASAN + UBSAN is fine. ASAN + TSAN is NOT (both instrument memory).
    if(RTAV_ENABLE_ASAN AND RTAV_ENABLE_TSAN)
        message(FATAL_ERROR "ASAN and TSAN cannot be enabled simultaneously")
    endif()

    set(SANITIZER_FLAGS "")

    if(RTAV_ENABLE_ASAN)
        list(APPEND SANITIZER_FLAGS -fsanitize=address -fno-omit-frame-pointer)
    endif()
    if(RTAV_ENABLE_TSAN)
        list(APPEND SANITIZER_FLAGS -fsanitize=thread)
    endif()
    if(RTAV_ENABLE_UBSAN)
        list(APPEND SANITIZER_FLAGS -fsanitize=undefined)
    endif()

    if(SANITIZER_FLAGS)
        target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${target} PRIVATE ${SANITIZER_FLAGS})
        message(STATUS "Sanitizers enabled: ${SANITIZER_FLAGS}")
    endif()
endfunction()
```

### cmake/Platform.cmake

```cmake
function(rtav_link_platform target)
    if(APPLE)
        # macOS frameworks for audio and GPU
        find_library(COREAUDIO_FRAMEWORK CoreAudio REQUIRED)
        find_library(AUDIOUNIT_FRAMEWORK AudioUnit REQUIRED)
        find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox REQUIRED)
        find_library(COREMIDI_FRAMEWORK CoreMIDI REQUIRED)
        find_library(ACCELERATE_FRAMEWORK Accelerate REQUIRED)
        find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
        find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
        find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)

        target_link_libraries(${target} PRIVATE
            ${COREAUDIO_FRAMEWORK}
            ${AUDIOUNIT_FRAMEWORK}
            ${AUDIOTOOLBOX_FRAMEWORK}
            ${COREMIDI_FRAMEWORK}
            ${ACCELERATE_FRAMEWORK}
            ${COCOA_FRAMEWORK}
            ${IOKIT_FRAMEWORK}
            ${COREGRAPHICS_FRAMEWORK}
        )

        # If NOT using FFTW, use Accelerate's vDSP for FFT
        if(NOT RTAV_USE_FFTW)
            target_compile_definitions(${target} PRIVATE RTAV_USE_VDSP=1)
        endif()

    elseif(WIN32)
        # Windows audio APIs
        target_link_libraries(${target} PRIVATE
            winmm       # Multimedia timers
            ole32        # COM (WASAPI)
            oleaut32
            uuid
        )
        # If ASIO SDK is available
        if(DEFINED ASIO_SDK_DIR)
            target_include_directories(${target} PRIVATE "${ASIO_SDK_DIR}/common")
            target_compile_definitions(${target} PRIVATE RTAV_HAS_ASIO=1)
        endif()

    elseif(UNIX)  # Linux
        find_package(PkgConfig REQUIRED)

        # ALSA (almost always present)
        pkg_check_modules(ALSA REQUIRED alsa)
        target_include_directories(${target} PRIVATE ${ALSA_INCLUDE_DIRS})
        target_link_libraries(${target} PRIVATE ${ALSA_LINK_LIBRARIES})

        # JACK (optional but preferred for low-latency)
        pkg_check_modules(JACK jack)
        if(JACK_FOUND)
            target_include_directories(${target} PRIVATE ${JACK_INCLUDE_DIRS})
            target_link_libraries(${target} PRIVATE ${JACK_LINK_LIBRARIES})
            target_compile_definitions(${target} PRIVATE RTAV_HAS_JACK=1)
        endif()

        # PulseAudio (for compatibility)
        pkg_check_modules(PULSE libpulse-simple)
        if(PULSE_FOUND)
            target_include_directories(${target} PRIVATE ${PULSE_INCLUDE_DIRS})
            target_link_libraries(${target} PRIVATE ${PULSE_LINK_LIBRARIES})
            target_compile_definitions(${target} PRIVATE RTAV_HAS_PULSE=1)
        endif()

        # X11 for windowing
        find_package(X11 REQUIRED)
        target_link_libraries(${target} PRIVATE X11::X11)

        # pthreads
        find_package(Threads REQUIRED)
        target_link_libraries(${target} PRIVATE Threads::Threads)
    endif()
endfunction()
```

---

## 3. Dependency Management

### 3.1 vcpkg (Recommended for This Project)

vcpkg is a C/C++ package manager from Microsoft. It integrates with CMake via a toolchain file, making `find_package()` work transparently for installed ports.

**Setup:**

```bash
# Clone vcpkg (one-time, system-wide)
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg && ./bootstrap-vcpkg.sh  # macOS/Linux
cd ~/vcpkg && .\bootstrap-vcpkg.bat  # Windows

# Set environment variable (add to .zshrc / .bashrc / system env)
export VCPKG_ROOT="$HOME/vcpkg"
export PATH="$VCPKG_ROOT:$PATH"
```

**vcpkg.json (manifest mode, lives in project root):**

```json
{
    "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
    "name": "realtimeaudiovis",
    "version-semver": "0.1.0",
    "description": "Cross-platform real-time audio analysis and visual rendering",
    "dependencies": [
        "fftw3",
        "aubio",
        "glfw3",
        "glm",
        "glad",
        "catch2"
    ],
    "overrides": [
        { "name": "fftw3", "version": "3.3.10#5" }
    ],
    "builtin-baseline": "2024.09.30"
}
```

Note: Essentia is NOT in vcpkg's official registry as of early 2026. It must be built from source (see Section 3.5). JUCE is also typically managed via FetchContent or submodule rather than vcpkg because JUCE's CMake integration expects `add_subdirectory()`.

**Triplets:**

vcpkg triplets define the target ABI. The default triplets are:

| Platform | Triplet | Notes |
|----------|---------|-------|
| macOS x86_64 | `x64-osx` | Dynamic libs by default |
| macOS arm64 | `arm64-osx` | Apple Silicon native |
| macOS universal | `x64-osx` + `arm64-osx` + lipo | Requires custom script |
| Windows x64 | `x64-windows` | Dynamic libs |
| Windows x64 static | `x64-windows-static` | Static CRT + static libs |
| Linux x64 | `x64-linux` | Dynamic libs |

For this project, use static linking on Windows (`x64-windows-static`) to produce a single binary without DLL dependencies, and dynamic linking on macOS/Linux (the default).

Custom triplet example (`cmake/triplets/x64-osx-release.cmake`):
```cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_BUILD_TYPE release)  # Skip debug builds to halve build time
set(VCPKG_OSX_DEPLOYMENT_TARGET 12.0)
```

**Build with vcpkg:**
```bash
# vcpkg manifest mode auto-installs dependencies at configure time
cmake -B build -S . \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=arm64-osx  # or x64-windows-static, x64-linux
cmake --build build --config Release -j$(nproc)
```

### 3.2 Conan

Conan is an alternative package manager popular in enterprise C++ shops. It uses Python-based recipes and supports binary caching out of the box.

**conanfile.txt:**
```ini
[requires]
fftw/3.3.10
aubio/0.4.9
glfw/3.4
glm/1.0.1
catch2/3.6.0

[generators]
CMakeDeps
CMakeToolchain

[options]
fftw/*:precision=single
fftw/*:threads=True

[layout]
cmake_layout
```

**Conan profile (example for macOS arm64 Release):**
```ini
[settings]
os=Macos
os.version=12.0
arch=armv8
compiler=apple-clang
compiler.version=15
compiler.libcxx=libc++
compiler.cppstd=20
build_type=Release

[buildenv]
CC=clang
CXX=clang++
```

**Build with Conan:**
```bash
conan install . --output-folder=build --build=missing -pr:b=default -pr:h=default
cmake --preset conan-release  # Conan generates CMake presets
cmake --build build --config Release -j$(nproc)
```

### 3.3 FetchContent for Header-Only Libraries

FetchContent is built into CMake 3.11+ and downloads dependencies at configure time. It is ideal for header-only or small libraries that build quickly. We already use it in the CMakeLists.txt above for JUCE, GLAD, GLFW, GLM, and Catch2.

Advantages: no external tool required, works on all platforms, version is pinned in CMakeLists.txt. Disadvantages: re-downloads on every clean configure (mitigated by `FETCHCONTENT_FULLY_DISCONNECTED` after first run), slow for large dependencies (Essentia takes 10+ minutes to build), pollutes the build directory.

**Best practice:** Cache FetchContent downloads:
```cmake
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.deps" CACHE PATH "FetchContent download dir")
```
This persists downloads across configure runs, and `.deps/` should be added to `.gitignore`.

### 3.4 Git Submodules

Git submodules pin a specific commit of an external repository into your source tree. They are the most transparent approach -- the dependency source is literally in your repo.

```bash
# Add JUCE as a submodule
git submodule add https://github.com/juce-framework/JUCE.git third_party/juce
cd third_party/juce && git checkout 7.0.12 && cd ../..
git add third_party/juce && git commit -m "Add JUCE 7.0.12 submodule"

# Clone with submodules
git clone --recurse-submodules <repo-url>

# Update submodules after pull
git submodule update --init --recursive
```

Advantages: full source available for debugging, works offline after initial clone, exact version control. Disadvantages: increases clone time and repo size, contributors must remember `--recurse-submodules`, nested submodules (JUCE has its own) create depth issues.

### 3.5 Comparison and Recommendation

| Criterion | vcpkg | Conan | FetchContent | Submodules |
|-----------|-------|-------|-------------|------------|
| Setup complexity | Low | Medium | None | None |
| Binary caching | Yes (NuGet/GHA) | Yes (built-in) | No | No |
| Build time (first) | Fast (prebuilt) | Fast (prebuilt) | Slow | Slow |
| Version pinning | Manifest + baseline | Lockfile | Git tag | Git commit |
| Essentia support | No | No | Manual | Manual |
| JUCE support | Partial | No | Excellent | Excellent |
| CI integration | Excellent | Good | Built-in | Built-in |

**Recommendation for this project:** Use a hybrid approach:

1. **vcpkg** for well-supported C libraries: FFTW3, Aubio, GLFW, GLM, Catch2.
2. **FetchContent** for JUCE (its CMake API expects `add_subdirectory()`-style inclusion) and GLAD (needs `glad_add_library()` generator).
3. **Manual build + find module** for Essentia (not available in any package manager; build with waf, install to a prefix, use `cmake/FindEssentia.cmake`).

### 3.6 Building Essentia from Source

Essentia uses the waf build system. Here is the procedure for each platform:

```bash
# Clone Essentia
git clone https://github.com/MTG/essentia.git
cd essentia
git checkout v2.1_beta5  # Or latest release tag

# macOS
brew install libyaml libsamplerate taglib chromaprint
python3 ./waf configure \
    --prefix=/usr/local \
    --with-examples=no \
    --with-python=no \
    --with-vamp=no
python3 ./waf
sudo python3 ./waf install

# Linux (Debian/Ubuntu)
sudo apt install libyaml-dev libsamplerate0-dev libtag1-dev \
    libchromaprint-dev libfftw3-dev
python3 ./waf configure --prefix=/usr/local
python3 ./waf && sudo python3 ./waf install

# Windows (MSYS2 or manual)
# Use pre-built static library from Essentia releases, or build with
# waf under MSYS2. Set ESSENTIA_ROOT in CMake to the install prefix.
```

**cmake/FindEssentia.cmake:**
```cmake
# FindEssentia.cmake - Find the Essentia library
# Sets: Essentia_FOUND, ESSENTIA_INCLUDE_DIRS, ESSENTIA_LIBRARIES

find_path(ESSENTIA_INCLUDE_DIR
    NAMES essentia/essentia.h
    PATHS
        /usr/local/include
        /usr/include
        ${ESSENTIA_ROOT}/include
    PATH_SUFFIXES essentia
)

find_library(ESSENTIA_LIBRARY
    NAMES essentia
    PATHS
        /usr/local/lib
        /usr/lib
        ${ESSENTIA_ROOT}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Essentia
    REQUIRED_VARS ESSENTIA_LIBRARY ESSENTIA_INCLUDE_DIR
)

if(Essentia_FOUND AND NOT TARGET Essentia::Essentia)
    add_library(Essentia::Essentia UNKNOWN IMPORTED)
    set_target_properties(Essentia::Essentia PROPERTIES
        IMPORTED_LOCATION "${ESSENTIA_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${ESSENTIA_INCLUDE_DIR}"
    )
endif()
```

---

## 4. Platform-Specific Build Instructions

### 4.1 Windows (MSVC 2022 / Clang-cl)

**Prerequisites:**
```powershell
# Visual Studio 2022 with C++ workload
winget install Microsoft.VisualStudio.2022.Community
# Or install via VS Installer: "Desktop development with C++"

# vcpkg
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
setx VCPKG_ROOT C:\vcpkg
setx PATH "%PATH%;C:\vcpkg"

# CMake (if not bundled with VS)
winget install Kitware.CMake
```

**ASIO SDK inclusion:** Steinberg's ASIO SDK is not redistributable. Download it from Steinberg's developer portal, extract to `third_party/asio_sdk/`, and pass `-DASIO_SDK_DIR=third_party/asio_sdk` to CMake. JUCE detects the ASIO SDK path and enables ASIO support automatically when the headers are found. Without ASIO, the application falls back to WASAPI shared mode (~10 ms latency vs. ~3 ms with ASIO).

**Build commands:**
```powershell
# MSVC (default generator is Visual Studio 17 2022)
cmake -B build -S . ^
    -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
    -DASIO_SDK_DIR=third_party/asio_sdk ^
    -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j%NUMBER_OF_PROCESSORS%

# Clang-cl (use Clang frontend with MSVC ABI)
cmake -B build -S . ^
    -G Ninja ^
    -DCMAKE_C_COMPILER=clang-cl ^
    -DCMAKE_CXX_COMPILER=clang-cl ^
    -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build --config Release -j%NUMBER_OF_PROCESSORS%
```

### 4.2 macOS (Xcode / Clang)

**Prerequisites:**
```bash
xcode-select --install  # Command line tools (includes Clang, make)
brew install cmake ninja fftw aubio
# Or use vcpkg as shown in Section 3.1
```

**Build commands:**
```bash
# Ninja generator (fastest)
cmake -B build -S . -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
    -DCMAKE_OSX_ARCHITECTURES="arm64"  # Or "arm64;x86_64" for universal
cmake --build build -j$(sysctl -n hw.logicalcpu)

# Xcode generator (for IDE-based development)
cmake -B build-xcode -S . -G Xcode \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
open build-xcode/RealTimeAudioVis.xcodeproj
```

**Accelerate framework:** On macOS, the Accelerate framework provides vDSP, which includes a hardware-optimized FFT implementation. When `RTAV_USE_FFTW=OFF`, the `FFTProcessor` class should use `vDSP_fft_zrip` (in-place real FFT). Accelerate is a system framework -- no installation needed. The `Platform.cmake` script links it automatically. For this project, vDSP is a viable alternative to FFTW3 on macOS only; it avoids an external dependency but locks that code path to Apple platforms.

**Universal binary (fat binary for x86_64 + arm64):**
```bash
cmake -B build -S . -G Ninja \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
cmake --build build
# Result: single binary runs natively on both Intel and Apple Silicon Macs
# Verify: lipo -info build/RealTimeAudioVis
```

### 4.3 Linux (GCC 12+ / Clang 15+)

**Prerequisites (Debian/Ubuntu):**
```bash
sudo apt update && sudo apt install -y \
    build-essential cmake ninja-build \
    gcc-12 g++-12 \
    libasound2-dev \
    libjack-jackd2-dev \
    libpulse-dev \
    libfftw3-dev \
    libaubio-dev \
    libgl-dev libglx-dev \
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
    pkg-config \
    libfreetype-dev libcurl4-openssl-dev  # For JUCE
```

**Prerequisites (Fedora):**
```bash
sudo dnf install -y \
    gcc gcc-c++ cmake ninja-build \
    alsa-lib-devel \
    jack-audio-connection-kit-devel \
    pulseaudio-libs-devel \
    fftw-devel \
    aubio-devel \
    mesa-libGL-devel \
    libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel \
    pkg-config \
    freetype-devel libcurl-devel
```

**Build commands:**
```bash
# GCC 12
CC=gcc-12 CXX=g++-12 cmake -B build -S . -G Ninja \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Clang 15
CC=clang-15 CXX=clang++-15 cmake -B build -S . -G Ninja \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 4.4 Cross-Compilation Considerations

Cross-compilation is rare for this project (it targets desktop platforms with native audio hardware), but may arise when building on an x86_64 CI runner for arm64 deployment.

**macOS cross-compile (Intel host to Apple Silicon target):** Set `CMAKE_OSX_ARCHITECTURES=arm64`. CMake handles the rest via Xcode's universal build support. vcpkg must use the `arm64-osx` triplet.

**Linux cross-compile (x86_64 to aarch64):** Requires a cross-compilation toolchain file:
```cmake
# cmake/toolchains/aarch64-linux.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

Use: `cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-linux.cmake`

All dependencies must also be cross-compiled or available as prebuilt aarch64 binaries. vcpkg supports this via community triplets (`arm64-linux`).

---

## 5. Compiler Flags for Performance

### 5.1 Optimization Levels

| Flag (GCC/Clang) | MSVC Equivalent | Effect |
|---|---|---|
| `-O0` | `/Od` | No optimization. Use for debugging. |
| `-O1` | `/O1` | Optimize for size. Minimal inlining. |
| `-O2` | `/O2` | Standard optimization. Best for most code. |
| `-O3` | `/O2` (no direct equivalent) | Aggressive: auto-vectorization, loop unrolling, function cloning. Can increase binary size 20-40%. |
| `-Os` | `/O1` | Optimize for size (like `-O2` minus size-increasing opts). |
| `-Ofast` | N/A | `-O3` + `-ffast-math`. See warning below. |

**Recommendation for this project:** Use `-O2` for the general build and `-O3` specifically for the `src/analysis/` translation units where FFT, spectral analysis, and feature extraction are CPU-bound. CMake allows per-source-file flags:

```cmake
set_source_files_properties(
    src/analysis/FFTProcessor.cpp
    src/analysis/SpectralAnalyzer.cpp
    PROPERTIES COMPILE_FLAGS "-O3"
)
```

### 5.2 Architecture-Specific Tuning

```cmake
# GCC / Clang
if(NOT MSVC)
    # -march=native: generate code for the build machine's CPU
    # Enables SSE4.2, AVX, AVX2, FMA, etc. based on detection
    # WARNING: the resulting binary will NOT run on older CPUs
    target_compile_options(RealTimeAudioVis PRIVATE
        $<$<CONFIG:Release>:-march=native>
    )
endif()

# MSVC
if(MSVC)
    target_compile_options(RealTimeAudioVis PRIVATE
        $<$<CONFIG:Release>:/arch:AVX2>  # Enable AVX2 instructions
    )
endif()
```

For distributable binaries, avoid `-march=native`. Instead, target a baseline architecture:
- x86_64: `-march=x86-64-v3` (AVX2 + FMA, covers most CPUs from ~2013 onward)
- arm64: no flag needed (NEON is baseline for AArch64)
- Safe fallback: `-march=x86-64` (SSE2 only, maximum compatibility)

Runtime dispatch (compiling multiple code paths and selecting at runtime via `cpuid`) is how libraries like FFTW3 handle this. For our DSP code, relying on FFTW3's built-in dispatch is sufficient; hand-tuning our own analysis loops with `-march=native` during development is fine since we control the deployment machine.

### 5.3 Floating-Point Model: `-ffast-math` and When NOT to Use It

`-ffast-math` (GCC/Clang) or `/fp:fast` (MSVC) enables aggressive floating-point optimizations:

- Assumes no NaN or Inf values (removes checks)
- Allows reassociation of operations (changes rounding behavior)
- Enables reciprocal approximation (`1.0/x` becomes `rcpps` + Newton-Raphson)
- Allows contraction (`a*b + c` becomes a single FMA instruction)
- Flushes denormals to zero

**When to use it:** DSP inner loops where IEEE 754 edge cases are irrelevant and denormal flushing is actually desirable (denormals in IIR filters cause massive performance degradation on x86 -- see [ARCH_realtime_constraints.md](ARCH_realtime_constraints.md)). The analysis thread's FFT post-processing, spectral bin calculations, and envelope followers all benefit.

**When NOT to use it:**
- Code that checks for NaN/Inf (fast-math makes `isnan()` always return `false`)
- Code that relies on exact floating-point comparison or rounding
- Code interfacing with file I/O where bit-exact reproducibility matters
- Unit tests that compare against known-good values with tight tolerances

**Best practice:** Enable fast-math only on specific translation units, not globally:

```cmake
# Apply -ffast-math only to DSP-heavy files
set_source_files_properties(
    src/analysis/FFTProcessor.cpp
    src/analysis/SpectralAnalyzer.cpp
    src/features/Smoother.cpp
    PROPERTIES COMPILE_FLAGS "$<IF:$<CXX_COMPILER_ID:MSVC>,/fp:fast,-ffast-math>"
)
```

Alternatively, use GCC/Clang's `#pragma` to enable it for specific functions:
```cpp
#pragma GCC push_options
#pragma GCC optimize("fast-math")
void SpectralAnalyzer::computeMagnitudes(const fftwf_complex* fftOut,
                                         float* magnitudes, int numBins) {
    for (int i = 0; i < numBins; ++i) {
        float re = fftOut[i][0];
        float im = fftOut[i][1];
        magnitudes[i] = std::sqrt(re * re + im * im);  // Will use rsqrtps + NR
    }
}
#pragma GCC pop_options
```

### 5.4 Link-Time Optimization (LTO)

LTO (`-flto` / `/GL` + `/LTCG`) enables the compiler to optimize across translation unit boundaries. The linker sees IR (intermediate representation) instead of machine code, enabling cross-module inlining, dead code elimination, and interprocedural constant propagation.

**Impact on this project:** Moderate. The hot path (audio callback -> ring buffer -> analysis loop) crosses TU boundaries. LTO allows the compiler to inline the ring buffer read/write operations into the analysis loop, eliminating function call overhead. In benchmarks of similar projects, LTO yields 5-15% throughput improvement in the analysis stage.

**Caveats:**
- Increases link time by 2-5x (the linker performs full optimization)
- Incompatible with some sanitizers on some platforms
- All object files and libraries must be compiled with the same LTO mode
- Debug information quality may degrade

The CMakeLists.txt above includes an `RTAV_ENABLE_LTO` option that uses CMake's `CheckIPOSupported` module.

### 5.5 Debug Flags and Sanitizers

**AddressSanitizer (ASAN):** Detects heap buffer overflows, use-after-free, stack buffer overflows, and memory leaks. Essential during development. Adds ~2x runtime overhead and ~3x memory overhead.

```bash
cmake -B build-debug -S . -DCMAKE_BUILD_TYPE=Debug -DRTAV_ENABLE_ASAN=ON
```

**ThreadSanitizer (TSAN):** Detects data races. Critical for this project since data flows between three threads (audio, analysis, render). TSAN has ~5-15x overhead; do not use for real-time audio testing (the audio callback will xrun). Instead, use synthetic test harnesses that simulate the multi-threaded data flow at accelerated speed.

```bash
cmake -B build-tsan -S . -DCMAKE_BUILD_TYPE=Debug -DRTAV_ENABLE_TSAN=ON
```

**UndefinedBehaviorSanitizer (UBSAN):** Detects signed integer overflow, null pointer dereference, misaligned access, and other undefined behavior. Low overhead (~10-20%), safe to enable alongside ASAN.

```bash
cmake -B build-debug -S . -DCMAKE_BUILD_TYPE=Debug \
    -DRTAV_ENABLE_ASAN=ON -DRTAV_ENABLE_UBSAN=ON
```

**MSVC equivalents:** MSVC includes AddressSanitizer (`/fsanitize=address`) since VS 2019 16.9. TSAN and UBSAN are not available on MSVC; use Clang-cl on Windows for those.

### 5.6 Warning Flags

The `cmake/CompilerWarnings.cmake` module (Section 2) configures comprehensive warnings. Key flags explained:

- `-Wconversion` catches implicit narrowing, e.g., `float x = double_value;` -- critical in DSP code where accidental double-precision operations tank performance on SIMD paths.
- `-Wdouble-promotion` catches implicit `float` to `double` promotion, which prevents auto-vectorization (SSE operates on 4 floats or 2 doubles per register).
- `-Wshadow` prevents bugs where a local variable hides a member or outer-scope variable -- common source of subtle DSP bugs.
- `-Werror` in CI ensures no warnings accumulate. Developers may want `-Wno-error` locally to avoid blocking iteration.

---

## 6. IDE Setup

### 6.1 CLion with CMake

CLion uses CMake natively. Configuration:

1. Open the project root folder (containing `CMakeLists.txt`).
2. Go to **Settings > Build, Execution, Deployment > CMake**.
3. Add profiles:
   - **Debug:** `-DCMAKE_BUILD_TYPE=Debug -DRTAV_ENABLE_ASAN=ON -DRTAV_ENABLE_UBSAN=ON`
   - **Release:** `-DCMAKE_BUILD_TYPE=Release -DRTAV_ENABLE_LTO=ON`
   - **RelWithDebInfo:** `-DCMAKE_BUILD_TYPE=RelWithDebInfo` (default)
4. Set **Generator** to Ninja (faster than Make).
5. If using vcpkg, add to **CMake options:** `-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`
6. CLion auto-detects the `compile_commands.json` generated by `CMAKE_EXPORT_COMPILE_COMMANDS=ON` for code navigation and refactoring.

**Debugger:** CLion bundles LLDB (macOS) or GDB (Linux). Set breakpoints in the analysis thread code. For the audio callback, use logging rather than breakpoints -- pausing the audio thread causes xruns and OS audio subsystem timeouts.

### 6.2 VS Code with clangd

VS Code with the clangd extension provides near-IDE-level code intelligence. Preferred over Microsoft's C/C++ extension for large projects because clangd is faster and more accurate.

**Extensions to install:**
- `llvm-vs-code-extensions.vscode-clangd` (clangd language server)
- `ms-vscode.cmake-tools` (CMake configure/build/debug integration)
- `vadimcn.vscode-lldb` (debugger, macOS/Linux) or `ms-vscode.cpptools` (debugger, Windows)
- `xaver.clang-format` (auto-format on save)

**`.vscode/settings.json`:**
```json
{
    "cmake.configureArgs": [
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        "-DCMAKE_TOOLCHAIN_FILE=${env:VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    ],
    "cmake.generator": "Ninja",
    "clangd.arguments": [
        "--background-index",
        "--clang-tidy",
        "--header-insertion=iwyu",
        "--completion-style=detailed",
        "-j=8",
        "--compile-commands-dir=${workspaceFolder}/build"
    ],
    "clangd.path": "/usr/bin/clangd",
    "editor.formatOnSave": true,
    "[cpp]": {
        "editor.defaultFormatter": "xaver.clang-format"
    }
}
```

**Key workflow:** After `cmake -B build`, clangd reads `build/compile_commands.json` for include paths, defines, and compiler flags. Code navigation (go-to-definition, find references) works across the entire project including JUCE headers.

### 6.3 Visual Studio with CMake

Visual Studio 2022 has native CMake support (no solution/project files needed).

1. **File > Open > Folder** and select the project root.
2. VS detects `CMakeLists.txt` and opens the CMake Settings Editor.
3. Add configurations:
   - **x64-Debug:** Default. Add `-DRTAV_ENABLE_ASAN=ON` to CMake command arguments.
   - **x64-Release:** Set configuration type to Release.
   - **x64-Clang-Release:** Change toolset to `ClangCl` for Clang-cl builds.
4. If using vcpkg, VS auto-detects it if `VCPKG_ROOT` is set. Otherwise, add `-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake` to CMake command arguments.
5. ASIO SDK: add `-DASIO_SDK_DIR=third_party/asio_sdk` to CMake arguments.

**CMakePresets.json (optional, preferred over VS-specific settings):**
```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "windows-msvc-debug",
            "displayName": "Windows MSVC Debug",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
                "VCPKG_TARGET_TRIPLET": "x64-windows-static",
                "RTAV_ENABLE_ASAN": "ON"
            },
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Windows"
            }
        },
        {
            "name": "windows-msvc-release",
            "displayName": "Windows MSVC Release",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
                "VCPKG_TARGET_TRIPLET": "x64-windows-static",
                "RTAV_ENABLE_LTO": "ON"
            },
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Windows"
            }
        },
        {
            "name": "macos-clang-debug",
            "displayName": "macOS Clang Debug",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_OSX_DEPLOYMENT_TARGET": "12.0",
                "RTAV_ENABLE_ASAN": "ON",
                "RTAV_ENABLE_UBSAN": "ON"
            },
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Darwin"
            }
        },
        {
            "name": "macos-clang-release",
            "displayName": "macOS Clang Release",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_OSX_DEPLOYMENT_TARGET": "12.0",
                "CMAKE_OSX_ARCHITECTURES": "arm64",
                "RTAV_ENABLE_LTO": "ON"
            },
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Darwin"
            }
        },
        {
            "name": "linux-gcc-debug",
            "displayName": "Linux GCC Debug",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_C_COMPILER": "gcc-12",
                "CMAKE_CXX_COMPILER": "g++-12",
                "RTAV_ENABLE_TSAN": "ON"
            },
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Linux"
            }
        },
        {
            "name": "linux-gcc-release",
            "displayName": "Linux GCC Release",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_C_COMPILER": "gcc-12",
                "CMAKE_CXX_COMPILER": "g++-12",
                "RTAV_ENABLE_LTO": "ON"
            },
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Linux"
            }
        }
    ],
    "buildPresets": [
        { "name": "windows-debug",  "configurePreset": "windows-msvc-debug" },
        { "name": "windows-release", "configurePreset": "windows-msvc-release" },
        { "name": "macos-debug",    "configurePreset": "macos-clang-debug" },
        { "name": "macos-release",  "configurePreset": "macos-clang-release" },
        { "name": "linux-debug",    "configurePreset": "linux-gcc-debug" },
        { "name": "linux-release",  "configurePreset": "linux-gcc-release" }
    ]
}
```

### 6.4 Xcode Project Generation

```bash
cmake -B build-xcode -S . -G Xcode \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
    -DCMAKE_OSX_ARCHITECTURES="arm64"
open build-xcode/RealTimeAudioVis.xcodeproj
```

Xcode provides excellent Instruments integration for profiling:
- **Time Profiler:** Identify hotspots in the analysis thread
- **System Trace:** Visualize thread scheduling and audio callback timing
- **Allocations:** Detect heap allocations in the real-time path
- **Leaks:** Memory leak detection

Set the scheme's executable to `RealTimeAudioVis` and configure the Arguments Passed On Launch as needed.

---

## 7. Debug vs Release Configurations

### 7.1 Configuration Matrix

| Setting | Debug | RelWithDebInfo | Release | Profile |
|---------|-------|----------------|---------|---------|
| Optimization | `-O0` / `/Od` | `-O2 -g` / `/O2 /Zi` | `-O3` / `/O2` | `-O2 -g` / `/O2 /Zi` |
| Debug info | Full | Full | None (or minimal) | Full |
| Assertions | ON | ON | OFF | ON |
| Sanitizers | ASAN+UBSAN | None | None | None |
| LTO | OFF | OFF | ON | OFF |
| `-ffast-math` | OFF | OFF | Per-file | OFF |
| Frame pointers | Kept | Kept | Omitted | Kept |
| `NDEBUG` | Not defined | Not defined | Defined | Not defined |

### 7.2 Conditional Compilation

```cpp
// Feature flags based on build configuration
#ifdef NDEBUG
    // Release mode: skip expensive validation
    #define RTAV_ASSERT(expr) ((void)0)
#else
    // Debug mode: full assertions with file/line
    #define RTAV_ASSERT(expr) \
        do { if (!(expr)) { \
            std::fprintf(stderr, "ASSERT FAILED: %s at %s:%d\n", \
                #expr, __FILE__, __LINE__); \
            std::abort(); \
        }} while(0)
#endif

// Ring buffer validation (debug only)
void RingBuffer::write(const float* data, int numSamples) {
    RTAV_ASSERT(numSamples <= capacity_ - size());  // Overflow check
    // ... lock-free write ...
}

// Performance instrumentation (profile build only)
#if defined(RTAV_PROFILE)
    #define RTAV_PROFILE_SCOPE(name) ScopedTimer timer##__LINE__(name)
#else
    #define RTAV_PROFILE_SCOPE(name) ((void)0)
#endif

void SpectralAnalyzer::process(const float* input, int numSamples) {
    RTAV_PROFILE_SCOPE("SpectralAnalyzer::process");
    // ...
}
```

### 7.3 Adding a Custom "Profile" Build Type

```cmake
# In root CMakeLists.txt, after project()
set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_RELWITHDEBINFO}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "")

# Keep frame pointers for profiling (perf, Instruments, VTune)
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_PROFILE
        "${CMAKE_CXX_FLAGS_PROFILE} -fno-omit-frame-pointer" CACHE STRING "" FORCE)
endif()

# Define RTAV_PROFILE for conditional profiling macros
target_compile_definitions(RealTimeAudioVis PRIVATE
    $<$<CONFIG:Profile>:RTAV_PROFILE=1>
)
```

Build:
```bash
cmake -B build-profile -S . -DCMAKE_BUILD_TYPE=Profile
cmake --build build-profile -j$(nproc)
# Then run under perf (Linux) or Instruments (macOS)
perf record -g ./build-profile/RealTimeAudioVis
perf report
```

---

## 8. CI/CD: GitHub Actions for Cross-Platform Build Verification

The following workflow builds and tests on all three platforms. It uses vcpkg binary caching to avoid rebuilding dependencies on every push.

**`.github/workflows/build.yml`:**

```yaml
name: Build & Test

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

env:
  VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"

jobs:
  build:
    strategy:
      fail-fast: false
      matrix:
        include:
          - os: ubuntu-24.04
            preset: linux-gcc-release
            test-preset: linux-release
            install-deps: |
              sudo apt-get update && sudo apt-get install -y \
                gcc-12 g++-12 ninja-build \
                libasound2-dev libjack-jackd2-dev libpulse-dev \
                libgl-dev libx11-dev libxrandr-dev libxinerama-dev \
                libxcursor-dev libxi-dev libfreetype-dev libcurl4-openssl-dev \
                pkg-config

          - os: ubuntu-24.04
            preset: linux-gcc-debug
            test-preset: linux-debug
            install-deps: |
              sudo apt-get update && sudo apt-get install -y \
                gcc-12 g++-12 ninja-build \
                libasound2-dev libjack-jackd2-dev libpulse-dev \
                libgl-dev libx11-dev libxrandr-dev libxinerama-dev \
                libxcursor-dev libxi-dev libfreetype-dev libcurl4-openssl-dev \
                pkg-config

          - os: macos-14   # Apple Silicon runner
            preset: macos-clang-release
            test-preset: macos-release
            install-deps: brew install ninja

          - os: windows-2022
            preset: windows-msvc-release
            test-preset: windows-release
            install-deps: ""  # MSVC pre-installed

    runs-on: ${{ matrix.os }}

    steps:
      - name: Checkout
        uses: actions/checkout@v4
        with:
          submodules: recursive

      - name: Install system dependencies
        if: matrix.install-deps != ''
        run: ${{ matrix.install-deps }}

      - name: Setup vcpkg
        uses: lukka/run-vcpkg@v11
        with:
          vcpkgGitCommitId: "2024.09.30"

      - name: Export GitHub Actions cache variables
        uses: actions/github-script@v7
        with:
          script: |
            core.exportVariable('ACTIONS_CACHE_URL', process.env.ACTIONS_CACHE_URL || '');
            core.exportVariable('ACTIONS_RUNTIME_TOKEN', process.env.ACTIONS_RUNTIME_TOKEN || '');

      - name: Configure CMake
        run: cmake --preset ${{ matrix.preset }}

      - name: Build
        run: cmake --build build/${{ matrix.preset }} --config Release -j 4

      - name: Test
        run: ctest --test-dir build/${{ matrix.preset }} --output-on-failure -j 4
        # Tests that require audio hardware are tagged [hardware] and excluded:
        # ctest --exclude-regex "\[hardware\]"

  sanitizer-check:
    runs-on: ubuntu-24.04
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive

      - name: Install dependencies
        run: |
          sudo apt-get update && sudo apt-get install -y \
            gcc-12 g++-12 ninja-build \
            libasound2-dev libjack-jackd2-dev libpulse-dev \
            libgl-dev libx11-dev libxrandr-dev libxinerama-dev \
            libxcursor-dev libxi-dev libfreetype-dev libcurl4-openssl-dev \
            pkg-config

      - name: Setup vcpkg
        uses: lukka/run-vcpkg@v11
        with:
          vcpkgGitCommitId: "2024.09.30"

      - name: Configure with ASAN + UBSAN
        run: |
          CC=gcc-12 CXX=g++-12 cmake -B build-sanitizers -S . -G Ninja \
            -DCMAKE_BUILD_TYPE=Debug \
            -DRTAV_ENABLE_ASAN=ON \
            -DRTAV_ENABLE_UBSAN=ON \
            -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

      - name: Build
        run: cmake --build build-sanitizers -j 4

      - name: Run tests under sanitizers
        run: ctest --test-dir build-sanitizers --output-on-failure -j 1
        # -j 1 because ASAN adds overhead; parallel sanitized tests can OOM

  tsan-check:
    runs-on: ubuntu-24.04
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive

      - name: Install dependencies
        run: |
          sudo apt-get update && sudo apt-get install -y \
            clang-15 ninja-build \
            libasound2-dev libjack-jackd2-dev libpulse-dev \
            libgl-dev libx11-dev libxrandr-dev libxinerama-dev \
            libxcursor-dev libxi-dev libfreetype-dev libcurl4-openssl-dev \
            pkg-config

      - name: Setup vcpkg
        uses: lukka/run-vcpkg@v11
        with:
          vcpkgGitCommitId: "2024.09.30"

      - name: Configure with TSAN
        run: |
          CC=clang-15 CXX=clang++-15 cmake -B build-tsan -S . -G Ninja \
            -DCMAKE_BUILD_TYPE=Debug \
            -DRTAV_ENABLE_TSAN=ON \
            -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

      - name: Build
        run: cmake --build build-tsan -j 4

      - name: Run tests under TSAN
        run: ctest --test-dir build-tsan --output-on-failure -j 1
```

### CI Strategy Notes

**Why three separate sanitizer jobs:** ASAN+UBSAN and TSAN are mutually exclusive (both instrument memory accesses; enabling both causes linker errors or false positives). They run as separate jobs to test different failure modes: ASAN catches memory corruption, TSAN catches data races in the triple-buffer and ring buffer code.

**Hardware-dependent tests:** Tests that require an audio device (microphone, loopback) cannot run in CI. Tag them with `[hardware]` in Catch2 and exclude them:
```cpp
TEST_CASE("Audio device enumeration", "[hardware]") {
    // Requires real audio hardware
}
```

**vcpkg binary caching:** The `VCPKG_BINARY_SOURCES` env variable with `x-gha,readwrite` enables GitHub Actions' built-in cache for vcpkg packages. First build takes ~10 minutes to compile FFTW3, Aubio, etc.; subsequent builds restore from cache in ~30 seconds.

**Build matrix design:** The matrix covers four critical configurations:
1. Linux GCC Release -- primary CI build, catches GCC-specific warnings
2. Linux GCC Debug -- verifies debug assertions, -O0 compatibility
3. macOS Clang Release -- verifies Accelerate/CoreAudio linking, Apple Silicon
4. Windows MSVC Release -- verifies MSVC compatibility, static linking

---

## Quick Start: From Zero to Running Application

```bash
# 1. Clone the repository
git clone --recurse-submodules <repo-url>
cd project

# 2. Install system dependencies (macOS example)
brew install cmake ninja fftw aubio
# Build and install Essentia (see Section 3.6)

# 3. Setup vcpkg (if not already done)
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg

# 4. Configure
cmake --preset macos-clang-debug
# Or manually:
# cmake -B build -S . -G Ninja \
#     -DCMAKE_BUILD_TYPE=Debug \
#     -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
#     -DRTAV_ENABLE_ASAN=ON

# 5. Build
cmake --build build/macos-clang-debug -j$(sysctl -n hw.logicalcpu)

# 6. Run tests
ctest --test-dir build/macos-clang-debug --output-on-failure

# 7. Run the application
./build/macos-clang-debug/RealTimeAudioVis
```

---

*Document version: 0.1.0 | Last updated: 2026-03-13*
