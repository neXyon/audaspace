# - Try to find audaspace
# Once done, this will define
#
#  AUDASPACE_FOUND - system has audaspace
#  AUDASPACE_INCLUDE_DIRS - the audaspace include directories
#  AUDASPACE_LIBRARIES - link these to use audaspace
#  CAUDASPACE_FOUND - system has audaspace's C binding
#  CAUDASPACE_LIBRARIES - link these to use audaspace's C binding
#  PYAUDASPACE_FOUND - system has audaspace's python binding
#  PYAUDASPACE_LIBRARIES - link these to use audaspace's python binding

# Use pkg-config to get hints about paths
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(AUDASPACE_PKGCONF audaspace)
endif(PKG_CONFIG_FOUND)

# Include dir
find_path(AUDASPACE_INCLUDE_DIR
	NAMES audaspace/ISound.h
	PATHS ${AUDASPACE_PKGCONF_INCLUDE_DIRS}
)

# Library
find_library(AUDASPACE_LIBRARY
	NAMES audaspace
	PATHS ${AUDASPACE_PKGCONF_LIBRARY_DIRS}
)

# Library
find_library(CAUDASPACE_LIBRARY
	NAMES caudaspace
	PATHS ${AUDASPACE_PKGCONF_LIBRARY_DIRS}
)

# Library
find_library(PYAUDASPACE_LIBRARY
	NAMES pyaudaspace
	PATHS ${AUDASPACE_PKGCONF_LIBRARY_DIRS}
)

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(Audaspace  DEFAULT_MSG  AUDASPACE_LIBRARY AUDASPACE_INCLUDE_DIR)
find_package_handle_standard_args(CAudaspace  DEFAULT_MSG  CAUDASPACE_LIBRARY AUDASPACE_INCLUDE_DIR)
find_package_handle_standard_args(PyAudaspace  DEFAULT_MSG  PYAUDASPACE_LIBRARY AUDASPACE_INCLUDE_DIR)

if(AUDASPACE_FOUND)
  set(AUDASPACE_LIBRARIES ${AUDASPACE_LIBRARY})
  set(AUDASPACE_INCLUDE_DIRS ${AUDASPACE_INCLUDE_DIR})
endif(AUDASPACE_FOUND)

if(CAUDASPACE_FOUND)
  set(CAUDASPACE_LIBRARIES ${CAUDASPACE_LIBRARY})
endif(CAUDASPACE_FOUND)

if(PYAUDASPACE_FOUND)
  set(PYAUDASPACE_LIBRARIES ${PYAUDASPACE_LIBRARY})
endif(PYAUDASPACE_FOUND)

mark_as_advanced(AUDASPACE_LIBRARY AUDASPACE_LIBRARIES AUDASPACE_INCLUDE_DIR AUDASPACE_INCLUDE_DIRS CAUDASPACE_LIBRARY CAUDASPACE_LIBRARIES PYAUDASPACE_LIBRARY PYAUDASPACE_LIBRARIES)
