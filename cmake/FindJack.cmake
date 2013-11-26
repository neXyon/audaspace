# - Try to find Jack
# Once done, this will define
#
#  JACK_FOUND - system has Jack
#  JACK_INCLUDE_DIRS - the Jack include directories
#  JACK_LIBRARIES - link these to use Jack

# Use pkg-config to get hints about paths
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(JACK_PKGCONF jack)
endif(PKG_CONFIG_FOUND)

# Include dir
find_path(JACK_INCLUDE_DIR
	NAMES jack.h
	PATH_SUFFIXES include/jack include
	PATHS ${JACK_PKGCONF_INCLUDE_DIRS}
)

# Library
find_library(JACK_LIBRARY
	NAMES jack
	PATHS ${JACK_PKGCONF_LIBRARY_DIRS}
)

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(Jack  DEFAULT_MSG  JACK_LIBRARY JACK_INCLUDE_DIR)

if(JACK_FOUND)
  set(JACK_LIBRARIES ${JACK_LIBRARY})
  set(JACK_INCLUDE_DIRS ${JACK_INCLUDE_DIR})
endif(JACK_FOUND)

mark_as_advanced(JACK_LIBRARY JACK_LIBRARIES JACK_INCLUDE_DIR JACK_INCLUDE_DIRS)
