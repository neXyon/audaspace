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

set(AUDASPACE_VERSION 1.0)

# sources

file(GLOB_RECURSE SRC src/*.cpp)
file(GLOB_RECURSE PRIVATE_HDR src/*.h)
file(GLOB_RECURSE PUBLIC_HDR include/*.h)

set(HDR ${PRIVATE_HDR} ${PUBLIC_HDR})

set(INCLUDE ${CMAKE_CURRENT_BINARY_DIR} include)
if(${WIN32})
	set(LIBRARIES)
	set(LIBRARY_PATH "../lib" CACHE PATH "Path which contains the libraries.")
	file(GLOB LIBRARY_DIRS ${LIBRARY_PATH}/*)
	list(APPEND CMAKE_PREFIX_PATH ${LIBRARY_DIRS})
else()
	set(LIBRARIES ${CMAKE_DL_LIBS} -lpthread)
endif()

set(STATIC_PLUGINS "")

# dependencies

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

option(BUILD_DEMOS "Build and install demos" TRUE)

option(SHARED_LIBRARY "Build Shared Library" TRUE)

option(WITH_C "Build C Module" TRUE)
option(WITH_DOCS "Build C++ HTML Documentation with Doxygen" FALSE)
option(WITH_BINDING_DOCS "Build C/Python HTML Documentation with Sphinx" FALSE)
option(WITH_FFMPEG "Build With FFMPEG" TRUE)
cmake_dependent_option(WITH_JACK "Build With Plugin" FALSE "NOT WIN32" FALSE)
option(WITH_LIBSNDFILE "Build With LibSndFile" TRUE)
option(WITH_OPENAL "Build With OpenAL" TRUE)
option(WITH_PYTHON "Build With Python Library" FALSE)
option(WITH_SDL "Build With SDL" TRUE)

set(DEFAULT_PLUGIN_PATH "." CACHE PATH "Default plugin installation and loading path.")
set(DOCUMENTATION_INSTALL_PATH "doc" CACHE PATH "Path where the documentation is installed.")

cmake_dependent_option(SEPARATE_C "Build C Binding as separate library" TRUE "WITH_C;SHARED_LIBRARY" FALSE)
cmake_dependent_option(PLUGIN_FFMPEG "Build FFMPEG Plugin" TRUE "WITH_FFMPEG;SHARED_LIBRARY" FALSE)
cmake_dependent_option(PLUGIN_JACK "Build Jack Plugin" TRUE "WITH_JACK;SHARED_LIBRARY" FALSE)
cmake_dependent_option(PLUGIN_LIBSNDFILE "Build LibSndFile Plugin" TRUE "WITH_LIBSNDFILE;SHARED_LIBRARY" FALSE)
cmake_dependent_option(PLUGIN_OPENAL "Build OpenAL Plugin" TRUE "WITH_OPENAL;SHARED_LIBRARY" FALSE)
cmake_dependent_option(PLUGIN_SDL "Build SDL Plugin" TRUE "WITH_SDL;SHARED_LIBRARY" FALSE)
cmake_dependent_option(WITH_PYTHON_MODULE "Build Python Module" TRUE "WITH_PYTHON;NOT WIN32" FALSE)

# compiler options

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	add_definitions(-std=c++11)
	list(APPEND CMAKE_C_COMPILER_FLAGS "-fvisibility=hidden")
	list(APPEND CMAKE_CXX_COMPILER_FLAGS "-fvisibility=hidden")
endif()

if(${MSVC})
	list(APPEND CMAKE_C_FLAGS_DEBUG "/Zi /Od")
	list(APPEND CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
	list(APPEND CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG")
	list(APPEND CMAKE_STATIC_LINKER_FLAGS_DEBUG "/DEBUG")
	list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
	add_definitions(/D_USE_MATH_DEFINES /EHsc)
	if(${SHARED_LIBRARY})
		include(GenerateExportHeader)
	endif()
endif()

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
		list(APPEND STATIC_PLUGINS FFMPEG)
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
		list(APPEND STATIC_PLUGINS JackDevice)
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
		list(APPEND STATIC_PLUGINS SndFile)
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
		list(APPEND STATIC_PLUGINS OpenALDevice)
	endif()
endif()

# Python
if(${WITH_PYTHON})
	set(Python_ADDITIONAL_VERSIONS 3.3)
	find_package(PythonLibs REQUIRED)
	list(APPEND INCLUDE ${PYTHON_INCLUDE_DIRS})

	if(${WITH_PYTHON_MODULE})
		find_package(PythonInterp REQUIRED)
	endif()
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
		list(APPEND STATIC_PLUGINS SDLDevice)
	endif()
endif()

# library configuration

if(${SHARED_LIBRARY})
	set(AUD_LIBRARY_TYPE AUD_SHARED_LIBRARY)
	set(LIBRARY_TYPE SHARED)
	add_definitions(-DAUD_BUILD_SHARED_LIBRARY)
else()
	set(AUD_LIBRARY_TYPE AUD_STATIC_LIBRARY)
	set(LIBRARY_TYPE STATIC)
endif()

# file configuration

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/Audaspace.h ${CMAKE_CURRENT_BINARY_DIR}/Audaspace.h ESCAPE_QUOTES @ONLY)

list(APPEND HDR ${CMAKE_CURRENT_BINARY_DIR}/Audaspace.h)

set(STATIC_PLUGIN_CLASSES "")
set(STATIC_PLUGIN_REGISTERS "")

foreach(PLUGIN ${STATIC_PLUGINS})
	list(APPEND STATIC_PLUGIN_CLASSES "STATIC_PLUGIN_CLASS(" ${PLUGIN} ")\n")
	list(APPEND STATIC_PLUGIN_REGISTERS "\tSTATIC_PLUGIN_REGISTER(" ${PLUGIN} ")\n")
endforeach()

string(CONCAT STATIC_PLUGIN_CLASSES ${STATIC_PLUGIN_CLASSES})
string(CONCAT STATIC_PLUGIN_REGISTERS ${STATIC_PLUGIN_REGISTERS})

file(GLOB PLUGINMANAGERS src/plugin/PluginManager*.cpp)
list(REMOVE_ITEM SRC ${PLUGINMANAGERS})

if(${WIN32})
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/PluginManagerWindows.cpp ${CMAKE_CURRENT_BINARY_DIR}/PluginManager.cpp ESCAPE_QUOTES @ONLY)
else()
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/PluginManagerUnix.cpp ${CMAKE_CURRENT_BINARY_DIR}/PluginManager.cpp ESCAPE_QUOTES @ONLY)
endif()

list(APPEND SRC ${CMAKE_CURRENT_BINARY_DIR}/PluginManager.cpp)

# directories

include_directories(${INCLUDE})
link_directories()

# library

add_library(audaspace ${LIBRARY_TYPE} ${SRC} ${HDR})
target_link_libraries(audaspace ${LIBRARIES})

install(TARGETS audaspace
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION "lib${LIB_SUFFIX}"
	ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include/audaspace)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Audaspace.h DESTINATION include/audaspace)

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

	if(${WITH_PYTHON_MODULE})
		set(PYTHON_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bindings/python)
		configure_file(${PYTHON_SOURCE_DIRECTORY}/setup.py ${CMAKE_CURRENT_BINARY_DIR}/setup.py ESCAPE_QUOTES @ONLY)

		add_custom_command(OUTPUT build COMMAND ${PYTHON_EXECUTABLE} setup.py build DEPENDS ${PYTHON_SRC} ${PYTHON_HDR})
		add_custom_target(pythonmodule ALL DEPENDS build SOURCES ${PYTHON_SOURCE_DIRECTORY}/setup.py ${PYTHON_SRC} ${PYTHON_HDR})
		add_dependencies(pythonmodule audaspace)

		install(CODE "EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} setup.py install --root=\$ENV{DESTDIR} --prefix=${CMAKE_INSTALL_PREFIX})")
	endif()
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
