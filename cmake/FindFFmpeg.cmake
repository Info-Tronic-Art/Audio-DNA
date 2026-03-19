# FindFFmpeg.cmake — Locate FFmpeg libraries (libavformat, libavcodec, libavutil, libswscale)
#
# Provides imported target FFmpeg::FFmpeg with all four libraries.
#
# Searches Homebrew prefix on macOS, standard paths on Linux/Windows.

# Homebrew prefix (arm64 macOS)
set(_FFMPEG_SEARCH_PATHS
    /opt/homebrew/opt/ffmpeg
    /usr/local/opt/ffmpeg
    /usr/local
    /usr
)

# Find headers
find_path(FFMPEG_INCLUDE_DIR
    NAMES libavformat/avformat.h
    PATHS ${_FFMPEG_SEARCH_PATHS}
    PATH_SUFFIXES include
)

# Find libraries
find_library(AVFORMAT_LIBRARY NAMES avformat PATHS ${_FFMPEG_SEARCH_PATHS} PATH_SUFFIXES lib)
find_library(AVCODEC_LIBRARY NAMES avcodec PATHS ${_FFMPEG_SEARCH_PATHS} PATH_SUFFIXES lib)
find_library(AVUTIL_LIBRARY NAMES avutil PATHS ${_FFMPEG_SEARCH_PATHS} PATH_SUFFIXES lib)
find_library(SWSCALE_LIBRARY NAMES swscale PATHS ${_FFMPEG_SEARCH_PATHS} PATH_SUFFIXES lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg
    REQUIRED_VARS
        FFMPEG_INCLUDE_DIR
        AVFORMAT_LIBRARY
        AVCODEC_LIBRARY
        AVUTIL_LIBRARY
        SWSCALE_LIBRARY
)

if(FFmpeg_FOUND AND NOT TARGET FFmpeg::FFmpeg)
    add_library(FFmpeg::FFmpeg INTERFACE IMPORTED)
    set_target_properties(FFmpeg::FFmpeg PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${AVFORMAT_LIBRARY};${AVCODEC_LIBRARY};${AVUTIL_LIBRARY};${SWSCALE_LIBRARY}"
    )
endif()
