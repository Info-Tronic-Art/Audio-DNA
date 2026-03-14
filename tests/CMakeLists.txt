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
