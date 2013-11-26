# - Try to find FFMPEG
# Once done, this will define
#
#  FFMPEG_FOUND - system has FFMPEG
#  FFMPEG_INCLUDE_DIRS - the FFMPEG include directories
#  FFMPEG_LIBRARIES - link these to use FFMPEG

# Use pkg-config to get hints about paths
#find_package(PkgConfig)
#if(PKG_CONFIG_FOUND)
#	pkg_check_modules(FFMPEG_PKGCONF ffmpeg)
#endif(PKG_CONFIG_FOUND)

# Include dir
find_path(FFMPEG_INCLUDE_DIR
	NAMES libavcodec/avcodec.h libavformat/avformat.h libavformat/avio.h
#	PATH_SUFFIXES include
#	PATHS ${FFMPEG_PKGCONF_INCLUDE_DIRS}
)

# Libraries
find_library(AVCODEC_LIBRARY
	NAMES libavcodec.so
#	PATHS ${FFMPEG_PKGCONF_LIBRARY_DIRS}
)

find_library(AVFORMAT_LIBRARY
	NAMES libavformat.so
#	PATHS ${FFMPEG_PKGCONF_LIBRARY_DIRS}
)

find_library(AVUTIL_LIBRARY
	NAMES libavutil.so
#	PATHS ${FFMPEG_PKGCONF_LIBRARY_DIRS}
)

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(FFMPEG  DEFAULT_MSG  AVCODEC_LIBRARY AVFORMAT_LIBRARY AVUTIL_LIBRARY FFMPEG_INCLUDE_DIR)

if(FFMPEG_FOUND)
  set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIR})
  set(FFMPEG_LIBRARIES ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY})
endif(FFMPEG_FOUND)

mark_as_advanced(FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIR FFMPEG_INCLUDE_DIRS AVCODEC_LIBRARY AVFORMAT_LIBRARY AVUTIL_LIBRARY)
