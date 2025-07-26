# - Try to find rubberband
# Once done, this will define
#
#  RUBBERBAND_FOUND - system has rubberband
#  RUBBERBAND_INCLUDE_DIRS - the rubberband include directories
#  RUBBERBAND_LIBRARIES - link these to use rubberband

# Use pkg-config to get hints about paths
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(RUBBERBAND_PKGCONF rubberband)
endif(PKG_CONFIG_FOUND)

# Include dir
find_path(RUBBERBAND_INCLUDE_DIR
	NAMES rubberband/RubberBandStretcher.h
	PATHS ${RUBBERBAND_PKGCONF_INCLUDE_DIRS}
)

# Library
find_library(RUBBERBAND_LIBRARY
	NAMES rubberband
	PATHS ${RUBBERBAND_PKGCONF_LIBRARY_DIRS}
)

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(Rubberband  DEFAULT_MSG  RUBBERBAND_LIBRARY RUBBERBAND_INCLUDE_DIR)

if(RUBBERBAND_FOUND)
  set(RUBBERBAND_LIBRARIES ${RUBBERBAND_LIBRARY})
  set(RUBBERBAND_INCLUDE_DIRS ${RUBBERBAND_INCLUDE_DIR})
endif(RUBBERBAND_FOUND)

mark_as_advanced(RUBBERBAND_LIBRARY RUBBERBAND_LIBRARIES RUBBERBAND_INCLUDE_DIR RUBBERBAND_INCLUDE_DIRS)
