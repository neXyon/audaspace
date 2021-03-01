# - Try to find libpulse
# Once done, this will define
#
#  LIBPULSE_FOUND - system has libpulse
#  LIBPULSE_INCLUDE_DIRS - the libpulse include directories
#  LIBPULSE_LIBRARIES - link these to use libpulse

# Use pkg-config to get hints about paths
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBPULSE_PKGCONF libpulse)
endif(PKG_CONFIG_FOUND)

# Include dir
find_path(LIBPULSE_INCLUDE_DIR
	NAMES pulse/pulseaudio.h
	PATHS ${LIBPULSE_PKGCONF_INCLUDE_DIRS}
)

# Library
find_library(LIBPULSE_LIBRARY
	NAMES pulse
	PATHS ${LIBPULSE_PKGCONF_LIBRARY_DIRS}
)

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(LibPulse  DEFAULT_MSG  LIBPULSE_LIBRARY LIBPULSE_INCLUDE_DIR)

if(LIBPULSE_FOUND)
  set(LIBPULSE_LIBRARIES ${LIBPULSE_LIBRARY})
  set(LIBPULSE_INCLUDE_DIRS ${LIBPULSE_INCLUDE_DIR})
endif(LIBPULSE_FOUND)

mark_as_advanced(LIBPULSE_LIBRARY LIBPULSE_LIBRARIES LIBPULSE_INCLUDE_DIR LIBPULSE_INCLUDE_DIRS)
