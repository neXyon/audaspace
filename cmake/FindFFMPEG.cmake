# - Try to find FFMPEG
# Once done, this will define
#
#  FFMPEG_FOUND - system has FFMPEG
#  FFMPEG_INCLUDE_DIRS - the FFMPEG include directories
#  FFMPEG_LIBRARIES - link these to use FFMPEG

# Use pkg-config to get hints about paths
find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(AVCODEC_PKGCONF libavcodec)
	pkg_check_modules(AVFORMAT_PKGCONF libavformat)
	pkg_check_modules(AVUTIL_PKGCONF libavutil)
endif(PKG_CONFIG_FOUND)

# Include dir
find_path(FFMPEG_INCLUDE_DIR
	NAMES libavcodec/avcodec.h libavformat/avformat.h libavformat/avio.h
	PATHS ${AVCODEC_PKGCONF_INCLUDE_DIRS} ${AVFORMAT_PKGCONF_INCLUDE_DIRS} ${AVUTIL_PKGCONF_INCLUDE_DIRS}
)

# Libraries
find_library(AVCODEC_LIBRARY
	NAMES avcodec
	PATHS ${AVCODEC_PKGCONF_LIBRARY_DIRS}
)

find_library(AVFORMAT_LIBRARY
	NAMES avformat
	PATHS ${AVFORMAT_PKGCONF_LIBRARY_DIRS}
)

find_library(AVUTIL_LIBRARY
	NAMES avutil
	PATHS ${AVUTIL_PKGCONF_LIBRARY_DIRS}
)

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(FFMPEG  DEFAULT_MSG  AVCODEC_LIBRARY AVFORMAT_LIBRARY AVUTIL_LIBRARY FFMPEG_INCLUDE_DIR)

if(FFMPEG_FOUND)
  set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIR})
  set(FFMPEG_LIBRARIES ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY})
endif(FFMPEG_FOUND)

mark_as_advanced(FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIR FFMPEG_INCLUDE_DIRS AVCODEC_LIBRARY AVFORMAT_LIBRARY AVUTIL_LIBRARY)
