# FindProjectM.cmake — locate libprojectM-4 via cmake config or manual search.
# Defines: ProjectM::ProjectM imported target.

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
