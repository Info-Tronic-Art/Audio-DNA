# FindProjectM.cmake — locate libprojectM-4 via cmake config or manual search.
# Defines: ProjectM::ProjectM imported target.

# BF10 (s-rta-1002b): the PATCHED libprojectM 4.1.1 (adds projectm_opengl_render_frame_fbo) lives in its own
# prefix, installed by cmake/projectm/build-projectm.sh. Search it FIRST. A projectM4_DIR cached from an earlier
# configure (e.g. the stock ~/.local) that lies outside the prefix is dropped, so a re-configure picks the prefix.
set(AUDIODNA_PROJECTM_PREFIX "$ENV{HOME}/.local/opt/projectm-4.1.1-fbo1" CACHE PATH
    "Prefix of the patched libprojectM 4.1.1 (cmake/projectm/build-projectm.sh)")
if(DEFINED CACHE{projectM4_DIR})
    string(FIND "${projectM4_DIR}" "${AUDIODNA_PROJECTM_PREFIX}/" _audiodna_pm_pos)
    if(NOT _audiodna_pm_pos EQUAL 0)
        unset(projectM4_DIR CACHE)
    endif()
    unset(_audiodna_pm_pos)
endif()
find_package(projectM4 CONFIG QUIET PATHS ${AUDIODNA_PROJECTM_PREFIX}/lib/cmake NO_DEFAULT_PATH)

# Try CMake config mode first (libprojectM-4 installs cmake config files)
find_package(projectM4 QUIET CONFIG
    PATHS
        $ENV{HOME}/.local/lib/cmake
        /opt/homebrew/lib/cmake
        /usr/local/lib/cmake
)

if(projectM4_FOUND AND TARGET libprojectM::projectM)
    if(NOT TARGET ProjectM::ProjectM)
        add_library(ProjectM::ProjectM INTERFACE IMPORTED)
        target_link_libraries(ProjectM::ProjectM INTERFACE libprojectM::projectM)
    endif()
    set(ProjectM_FOUND TRUE)
    return()
endif()

# Try pkg-config
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PROJECTM QUIET libprojectM-4)
endif()

if(PROJECTM_FOUND)
    add_library(ProjectM::ProjectM INTERFACE IMPORTED)
    target_include_directories(ProjectM::ProjectM INTERFACE ${PROJECTM_INCLUDE_DIRS})
    target_link_libraries(ProjectM::ProjectM INTERFACE ${PROJECTM_LINK_LIBRARIES})
    set(ProjectM_FOUND TRUE)
    return()
endif()

# Fallback: manual search
find_path(PROJECTM_INCLUDE_DIR
    NAMES projectM-4/projectM.h
    PATHS
        $ENV{HOME}/.local/include
        /opt/homebrew/include
        /usr/local/include
        /usr/include
)

find_library(PROJECTM_LIBRARY
    NAMES projectM-4 projectm-4
    PATHS
        $ENV{HOME}/.local/lib
        /opt/homebrew/lib
        /usr/local/lib
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ProjectM
    REQUIRED_VARS PROJECTM_LIBRARY PROJECTM_INCLUDE_DIR
)

if(ProjectM_FOUND)
    add_library(ProjectM::ProjectM INTERFACE IMPORTED)
    target_include_directories(ProjectM::ProjectM INTERFACE ${PROJECTM_INCLUDE_DIR})
    target_link_libraries(ProjectM::ProjectM INTERFACE ${PROJECTM_LIBRARY})
endif()
