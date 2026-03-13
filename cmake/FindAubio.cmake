# FindAubio.cmake — Locate the Aubio audio analysis library
#
# Sets:
#   Aubio_FOUND        — TRUE if found
#   Aubio_INCLUDE_DIRS — header directory
#   Aubio_LIBRARIES    — library path
#
# Creates imported target: Aubio::Aubio

find_path(Aubio_INCLUDE_DIR
    NAMES aubio/aubio.h
    HINTS
        /opt/homebrew/include
        /usr/local/include
        /usr/include
)

find_library(Aubio_LIBRARY
    NAMES aubio
    HINTS
        /opt/homebrew/lib
        /usr/local/lib
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Aubio
    REQUIRED_VARS Aubio_LIBRARY Aubio_INCLUDE_DIR
)

if(Aubio_FOUND)
    set(Aubio_INCLUDE_DIRS ${Aubio_INCLUDE_DIR})
    set(Aubio_LIBRARIES ${Aubio_LIBRARY})

    if(NOT TARGET Aubio::Aubio)
        add_library(Aubio::Aubio UNKNOWN IMPORTED)
        set_target_properties(Aubio::Aubio PROPERTIES
            IMPORTED_LOCATION "${Aubio_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Aubio_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(Aubio_INCLUDE_DIR Aubio_LIBRARY)
