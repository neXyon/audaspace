################################################################################
# Copyright 2009-2013 Jörg Müller
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

cmake_minimum_required(VERSION 3.0)
include(CMakeDependentOption)

project(audaspace)

set(AUDASPACE_VERSION 0.1)

# sources

file(GLOB_RECURSE SRC src/*.cpp)
file(GLOB_RECURSE PRIVATE_HDR src/*.h)
file(GLOB_RECURSE PUBLIC_HDR include/*.h)

set(HDR ${PRIVATE_HDR} ${PUBLIC_HDR})

set(INCLUDE include)
if(${WIN32})
	set(LIBRARIES)
else()
	set(LIBRARIES ${CMAKE_DL_LIBS} -lpthread)
endif()

# dependencies

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

option(BUILD_DEMOS "Build and install demos" TRUE)

option(SHARED_LIBRARY "Build Shared Library" TRUE)

option(WITH_C "Build C Module" TRUE)
option(WITH_DOCS "Build C++ HTML Documentation with Doxygen" FALSE)
option(WITH_BINDING_DOCS "Build C/Python HTML Documentation with Sphinx" FALSE)
option(WITH_FFMPEG "Build With FFMPEG" TRUE)
option(WITH_JACK "Build With Jack" FALSE)
option(WITH_LIBSNDFILE "Build With LibSndFile" TRUE)
option(WITH_OPENAL "Build With OpenAL" TRUE)
option(WITH_PYTHON "Build Python Module" FALSE)
option(WITH_SDL "Build With SDL" TRUE)

set(DEFAULT_PLUGIN_PATH "." CACHE PATH "Default plugin installation and loading path.")
set(DOCUMENTATION_INSTALL_PATH "doc" CACHE PATH "Path where the documentation is installed.")

cmake_dependent_option(SEPARATE_C "Build C Binding as separate library" TRUE "WITH_C;SHARED_LIBRARY" FALSE)
cmake_dependent_option(PLUGIN_FFMPEG "Build FFMPEG Plugin" TRUE "WITH_FFMPEG;SHARED_LIBRARY;NOT WIN32" FALSE)
cmake_dependent_option(PLUGIN_JACK "Build Jack Plugin" TRUE "WITH_JACK;SHARED_LIBRARY;NOT WIN32" FALSE)
cmake_dependent_option(PLUGIN_LIBSNDFILE "Build LibSndFile Plugin" TRUE "WITH_LIBSNDFILE;SHARED_LIBRARY;NOT WIN32" FALSE)
cmake_dependent_option(PLUGIN_OPENAL "Build OpenAL Plugin" TRUE "WITH_OPENAL;SHARED_LIBRARY;NOT WIN32" FALSE)
cmake_dependent_option(PLUGIN_SDL "Build SDL Plugin" TRUE "WITH_SDL;SHARED_LIBRARY;NOT WIN32" FALSE)

# C
if(${WITH_C})
	file(GLOB_RECURSE C_SRC bindings/C/*.cpp)
	file(GLOB_RECURSE C_HDR bindings/C/*.h)

	if(NOT ${SEPARATE_C})
		list(APPEND SRC ${C_SRC})
		list(APPEND HDR ${C_HDR})
	else()
		set(CAUDASPACE_LIBRARY -lcaudaspace)
	endif()
endif()

# FFMPEG
if(${WITH_FFMPEG})
	find_package(FFMPEG REQUIRED)
	file(GLOB_RECURSE FFMPEG_SRC plugins/ffmpeg/*.cpp)
	file(GLOB_RECURSE FFMPEG_HDR plugins/ffmpeg/*.h)

	if(NOT ${PLUGIN_FFMPEG})
		list(APPEND INCLUDE ${FFMPEG_INCLUDE_DIRS})
		list(APPEND LIBRARIES ${FFMPEG_LIBRARIES})
		list(APPEND SRC ${FFMPEG_SRC})
		list(APPEND HDR ${FFMPEG_HDR})
	endif()
endif()

# Jack
if(${WITH_JACK})
	find_package(Jack REQUIRED)
	file(GLOB_RECURSE JACK_SRC plugins/jack/*.cpp)
	file(GLOB_RECURSE JACK_HDR plugins/jack/*.h)

	if(NOT ${PLUGIN_JACK})
		list(APPEND INCLUDE ${JACK_INCLUDE_DIRS})
		list(APPEND LIBRARIES ${JACK_LIBRARIES})
		list(APPEND SRC ${JACK_SRC})
		list(APPEND HDR ${JACK_HDR})
	endif()
endif()

# LibSndFile
if(${WITH_LIBSNDFILE})
	find_package(LibSndFile REQUIRED)
	file(GLOB_RECURSE LIBSNDFILE_SRC plugins/libsndfile/*.cpp)
	file(GLOB_RECURSE LIBSNDFILE_HDR plugins/libsndfile/*.h)

	if(NOT ${PLUGIN_LIBSNDFILE})
		list(APPEND INCLUDE ${LIBSNDFILE_INCLUDE_DIRS})
		list(APPEND LIBRARIES ${LIBSNDFILE_LIBRARIES})
		list(APPEND SRC ${LIBSNDFILE_SRC})
		list(APPEND HDR ${LIBSNDFILE_HDR})
	endif()
endif()

# OpenAL
if(${WITH_OPENAL})
	find_package(OpenAL REQUIRED)
	file(GLOB_RECURSE OPENAL_SRC plugins/openal/*.cpp)
	file(GLOB_RECURSE OPENAL_HDR plugins/openal/*.h)

	if(NOT ${PLUGIN_OPENAL})
		list(APPEND INCLUDE ${OPENAL_INCLUDE_DIR})
		list(APPEND LIBRARIES ${OPENAL_LIBRARY})
		list(APPEND SRC ${OPENAL_SRC})
		list(APPEND HDR ${OPENAL_HDR})
	endif()
endif()

# Python
if(${WITH_PYTHON})
	set(Python_ADDITIONAL_VERSIONS 3.3)
	find_package(PythonLibs REQUIRED)
	list(APPEND INCLUDE ${PYTHON_INCLUDE_DIRS})
	set(PYAUDASPACE_LIBRARY -lpyaudaspace)
endif()

# SDL
if(${WITH_SDL})
	find_package(SDL REQUIRED)
	file(GLOB_RECURSE SDL_SRC plugins/sdl/*.cpp)
	file(GLOB_RECURSE SDL_HDR plugins/sdl/*.h)

	if(NOT ${PLUGIN_SDL})
		list(APPEND INCLUDE ${SDL_INCLUDE_DIR})
		list(APPEND LIBRARIES ${SDL_LIBRARY})
		list(APPEND SRC ${SDL_SRC})
		list(APPEND HDR ${SDL_HDR})
	endif()
endif()

# file configuration
set(PLUGINMANAGER_FILENAME ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/PluginManager.cpp)
list(REMOVE_ITEM SRC ${PLUGINMANAGER_FILENAME})
configure_file(${PLUGINMANAGER_FILENAME} ${CMAKE_CURRENT_BINARY_DIR}/PluginManager.cpp ESCAPE_QUOTES @ONLY)
list(APPEND SRC ${CMAKE_CURRENT_BINARY_DIR}/PluginManager.cpp)

# directories and compiler options

include_directories(${INCLUDE})
link_directories()
add_definitions(-std=c++11)

# library

if(${SHARED_LIBRARY})
	set(LIBRARY_TYPE SHARED)
else()
	set(LIBRARY_TYPE STATIC)
endif()

add_library(audaspace ${LIBRARY_TYPE} ${SRC} ${HDR})
target_link_libraries(audaspace ${LIBRARIES})

install(TARGETS audaspace
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION "lib${LIB_SUFFIX}"
	ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include/audaspace)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages/pkgconfig/audaspace.pc.in ${CMAKE_CURRENT_BINARY_DIR}/audaspace.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/audaspace.pc DESTINATION "lib${LIB_SUFFIX}/pkgconfig")

# plugins

if(${PLUGIN_FFMPEG})
	add_definitions(-DFFMPEG_PLUGIN)
	include_directories(${INCLUDE} ${FFMPEG_INCLUDE_DIRS})
	add_library(audffmpeg SHARED ${FFMPEG_SRC} ${FFMPEG_HDR} ${HDR})
	target_link_libraries(audffmpeg audaspace ${FFMPEG_LIBRARIES})
	install(TARGETS audffmpeg DESTINATION ${DEFAULT_PLUGIN_PATH})
endif()

if(${PLUGIN_JACK})
	add_definitions(-DJACK_PLUGIN)
	include_directories(${INCLUDE} ${JACK_INCLUDE_DIRS})
	add_library(audjack SHARED ${JACK_SRC} ${JACK_HDR} ${HDR})
	target_link_libraries(audjack audaspace ${JACK_LIBRARIES})
	install(TARGETS audjack DESTINATION ${DEFAULT_PLUGIN_PATH})
endif()

if(${PLUGIN_LIBSNDFILE})
	add_definitions(-DLIBSNDFILE_PLUGIN)
	include_directories(${INCLUDE} ${LIBSNDFILE_INCLUDE_DIRS})
	add_library(audlibsndfile SHARED ${LIBSNDFILE_SRC} ${LIBSNDFILE_HDR} ${HDR})
	target_link_libraries(audlibsndfile audaspace ${LIBSNDFILE_LIBRARIES})
	install(TARGETS audlibsndfile DESTINATION ${DEFAULT_PLUGIN_PATH})
endif()

if(${PLUGIN_OPENAL})
	add_definitions(-DOPENAL_PLUGIN)
	include_directories(${INCLUDE} ${OPENAL_INCLUDE_DIR})
	add_library(audopenal SHARED ${OPENAL_SRC} ${OPENAL_HDR} ${HDR})
	target_link_libraries(audopenal audaspace ${OPENAL_LIBRARY})
	install(TARGETS audopenal DESTINATION ${DEFAULT_PLUGIN_PATH})
endif()

if(${PLUGIN_SDL})
	add_definitions(-DSDL_PLUGIN)
	include_directories(${INCLUDE} ${SDL_INCLUDE_DIR})
	add_library(audsdl SHARED ${SDL_SRC} ${SDL_HDR} ${HDR})
	target_link_libraries(audsdl audaspace ${SDL_LIBRARY})
	install(TARGETS audsdl DESTINATION ${DEFAULT_PLUGIN_PATH})
endif()

# demos

if(${BUILD_DEMOS})
	include_directories(${INCLUDE})

	set(DEMOS audaplay audaconvert audaremap signalgen)

	add_executable(audaplay demos/audaplay.cpp)
	target_link_libraries(audaplay audaspace)

	add_executable(audaconvert demos/audaconvert.cpp)
	target_link_libraries(audaconvert audaspace)

	add_executable(audaremap demos/audaremap.cpp)
	target_link_libraries(audaremap audaspace)

	add_executable(signalgen demos/signalgen.cpp)
	target_link_libraries(signalgen audaspace)

	if(${WITH_OPENAL})
		list(APPEND DEMOS openaldevices)

		add_executable(openaldevices demos/openaldevices.cpp)
		if(${PLUGIN_OPENAL})
			target_link_libraries(openaldevices audaspace audopenal)
		else()
			target_link_libraries(openaldevices audaspace)
		endif()
	endif()

	install(TARGETS ${DEMOS}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION "lib${LIB_SUFFIX}"
		ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
	)
endif()

# bindings

if(${WITH_C} AND ${SEPARATE_C})
	add_library(caudaspace ${LIBRARY_TYPE} ${C_SRC} ${C_HDR})
	target_link_libraries(caudaspace audaspace)
	install(TARGETS caudaspace
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION "lib${LIB_SUFFIX}"
		ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
	)

	install(FILES ${C_HDR} DESTINATION include/audaspace)
endif()

if(${WITH_PYTHON})
	file(GLOB_RECURSE PYTHON_SRC bindings/python/*.cpp)
	file(GLOB_RECURSE PYTHON_HDR bindings/python/*.h)

	add_library(pyaudaspace ${LIBRARY_TYPE} ${PYTHON_SRC} ${PYTHON_HDR})
	target_link_libraries(pyaudaspace audaspace ${PYTHON_LIBRARIES})

	install(TARGETS pyaudaspace
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION "lib${LIB_SUFFIX}"
		ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
	)

	install(FILES ${PYTHON_HDR} DESTINATION include/audaspace/python)

	set(PYTHON_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bindings/python)
	configure_file(${PYTHON_SOURCE_DIRECTORY}/setup.py ${CMAKE_CURRENT_BINARY_DIR}/setup.py ESCAPE_QUOTES @ONLY)

	add_custom_command(OUTPUT build COMMAND python setup.py build DEPENDS ${PYTHON_SRC} ${PYTHON_HDR})
	add_custom_target(pythonmodule ALL DEPENDS build SOURCES ${PYTHON_SOURCE_DIRECTORY}/setup.py ${PYTHON_SRC} ${PYTHON_HDR})
	add_dependencies(pythonmodule audaspace)

	install(CODE "EXECUTE_PROCESS(COMMAND python setup.py install --root=\$ENV{DESTDIR} --prefix=${CMAKE_INSTALL_PREFIX})")
endif()

# docs

if(${WITH_DOCS})
	find_package(Doxygen REQUIRED)

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

	add_custom_target(audaspace_doc ALL ${DOXYGEN_EXECUTABLE} Doxyfile COMMENT "Building C++ HTML documentation with Doxygen.")
endif()

if(${WITH_BINDING_DOCS})
	find_package(Sphinx REQUIRED)

	add_custom_target(bindings_doc ALL ${SPHINX_EXECUTABLE} -q -b html -d "${CMAKE_CURRENT_BINARY_DIR}/_doctrees" "${CMAKE_CURRENT_SOURCE_DIR}/bindings/doc" "${CMAKE_CURRENT_BINARY_DIR}/doc/bindings" COMMENT "Building C/Python HTML documentation with Sphinx.")
endif()

if(${WITH_DOCS} OR ${WITH_BINDING_DOCS})
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/ DESTINATION ${DOCUMENTATION_INSTALL_PATH})
endif()
