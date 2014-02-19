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

cmake_minimum_required(VERSION 2.8)
include(CMakeDependentOption)

project(audaspace)

# sources

file(GLOB_RECURSE SRC src/*.cpp)
file(GLOB_RECURSE PRIVATE_HDR src/*.h)
file(GLOB_RECURSE PUBLIC_HDR include/*.h)

set(HDR ${PRIVATE_HDR} ${PUBLIC_HDR})

set(INCLUDE include)
if(${WIN32})
	set(LIBRARIES)
	file(GLOB_RECURSE REMOVE_SRC src/plugin/*.cpp)
	file(GLOB_RECURSE REMOVE_HDR include/plugin/*.h)
	list(REMOVE_ITEM SRC ${REMOVE_SRC})
	list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})
else()
	set(LIBRARIES ${CMAKE_DL_LIBS} -lpthread)
endif()

# dependencies

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

option(SHARED_LIBRARY "Build Shared Library" TRUE)

option(WITH_FFMPEG "Build With FFMPEG" TRUE)
option(WITH_JACK "Build With Jack" FALSE)
option(WITH_LIBSNDFILE "Build With LibSndFile" TRUE)
option(WITH_OPENAL "Build With OpenAL" TRUE)
option(WITH_PYTHON "Build Python Module" FALSE)
option(WITH_SDL "Build With SDL" FALSE)

cmake_dependent_option(PLUGIN_FFMPEG "Build FFMPEG Plugin" TRUE "WITH_FFMPEG;SHARED_LIBRARY;NOT WIN32" FALSE)
cmake_dependent_option(PLUGIN_JACK "Build Jack Plugin" TRUE "WITH_JACK;SHARED_LIBRARY;NOT WIN32" FALSE)
cmake_dependent_option(PLUGIN_LIBSNDFILE "Build LibSndFile Plugin" TRUE "WITH_LIBSNDFILE;SHARED_LIBRARY;NOT WIN32" FALSE)
cmake_dependent_option(PLUGIN_OPENAL "Build OpenAL Plugin" TRUE "WITH_OPENAL;SHARED_LIBRARY;NOT WIN32" FALSE)
cmake_dependent_option(PLUGIN_SDL "Build SDL Plugin" TRUE "WITH_SDL;SHARED_LIBRARY;NOT WIN32" FALSE)

file(GLOB_RECURSE REMOVE_SRC src/*FFMPEG*.cpp)
file(GLOB_RECURSE REMOVE_HDR include/*FFMPEG*.h)
list(REMOVE_ITEM SRC ${REMOVE_SRC})
list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})

file(GLOB_RECURSE REMOVE_SRC src/*Jack*.cpp)
file(GLOB_RECURSE REMOVE_HDR include/*Jack*.h)
list(REMOVE_ITEM SRC ${REMOVE_SRC})
list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})

file(GLOB_RECURSE REMOVE_SRC src/*SndFile*.cpp)
file(GLOB_RECURSE REMOVE_HDR include/*SndFile*.h)
list(REMOVE_ITEM SRC ${REMOVE_SRC})
list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})

file(GLOB_RECURSE REMOVE_SRC src/*OpenAL*.cpp)
file(GLOB_RECURSE REMOVE_HDR include/*OpenAL*.h)
list(REMOVE_ITEM SRC ${REMOVE_SRC})
list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})

file(GLOB_RECURSE REMOVE_SRC src/*SDL*.cpp)
file(GLOB_RECURSE REMOVE_HDR include/*SDL*.h)
list(REMOVE_ITEM SRC ${REMOVE_SRC})
list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})

# FFMPEG
if(${WITH_FFMPEG})
	find_package(FFMPEG REQUIRED)
	file(GLOB_RECURSE FFMPEG_SRC src/*FFMPEG*.cpp)
	file(GLOB_RECURSE FFMPEG_HDR include/*FFMPEG*.h)

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
	file(GLOB_RECURSE JACK_SRC src/*Jack*.cpp)
	file(GLOB_RECURSE JACK_HDR include/*Jack*.h)

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
	file(GLOB_RECURSE LIBSNDFILE_SRC src/*SndFile*.cpp)
	file(GLOB_RECURSE LIBSNDFILE_HDR include/*SndFile*.h)

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
	file(GLOB_RECURSE OPENAL_SRC src/*OpenAL*.cpp)
	file(GLOB_RECURSE OPENAL_HDR include/*OpenAL*.h)

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
endif()

# SDL
if(${WITH_SDL})
	find_package(SDL REQUIRED)
	file(GLOB_RECURSE SDL_SRC src/*SDL*.cpp)
	file(GLOB_RECURSE SDL_HDR include/*SDL*.h)

	if(NOT ${PLUGIN_SDL})
		list(APPEND INCLUDE ${SDL_INCLUDE_DIR})
		list(APPEND LIBRARIES ${SDL_LIBRARY})
		list(APPEND SRC ${SDL_SRC})
		list(APPEND HDR ${SDL_HDR})
	endif()
endif()

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

# plugin building

if(${PLUGIN_FFMPEG})
	add_definitions(-DFFMPEG_PLUGIN)
	include_directories(${INCLUDE} ${FFMPEG_INCLUDE_DIRS})
	add_library(audffmpeg SHARED ${FFMPEG_SRC} ${FFMPEG_HDR} ${HDR})
	target_link_libraries(audffmpeg audaspace ${FFMPEG_LIBRARIES})
endif()

if(${PLUGIN_JACK})
	add_definitions(-DJACK_PLUGIN)
	include_directories(${INCLUDE} ${JACK_INCLUDE_DIRS})
	add_library(audjack SHARED ${JACK_SRC} ${JACK_HDR} ${HDR})
	target_link_libraries(audjack audaspace ${JACK_LIBRARIES})
endif()

if(${PLUGIN_LIBSNDFILE})
	add_definitions(-DLIBSNDFILE_PLUGIN)
	include_directories(${INCLUDE} ${LIBSNDFILE_INCLUDE_DIRS})
	add_library(audlibsndfile SHARED ${LIBSNDFILE_SRC} ${LIBSNDFILE_HDR} ${HDR})
	target_link_libraries(audlibsndfile audaspace ${LIBSNDFILE_LIBRARIES})
endif()

if(${PLUGIN_OPENAL})
	add_definitions(-DOPENAL_PLUGIN)
	include_directories(${INCLUDE} ${OPENAL_INCLUDE_DIR})
	add_library(audopenal SHARED ${OPENAL_SRC} ${OPENAL_HDR} ${HDR})
	target_link_libraries(audopenal audaspace ${OPENAL_LIBRARY})
endif()

if(${PLUGIN_SDL})
	add_definitions(-DSDL_PLUGIN)
	include_directories(${INCLUDE} ${SDL_INCLUDE_DIR})
	add_library(audsdl SHARED ${SDL_SRC} ${SDL_HDR} ${HDR})
	target_link_libraries(audsdl audaspace ${SDL_LIBRARY})
endif()

# demos

include_directories(${INCLUDE})

add_executable(audaplay demos/audaplay.cpp)
target_link_libraries(audaplay audaspace)

add_executable(audaconvert demos/audaconvert.cpp)
target_link_libraries(audaconvert audaspace)

add_executable(audaremap demos/audaremap.cpp)
target_link_libraries(audaremap audaspace)

if(${WITH_OPENAL})
	add_executable(openaldevices demos/openaldevices.cpp)
	if(${PLUGIN_OPENAL})
		target_link_libraries(openaldevices audaspace audopenal)
	else()
		target_link_libraries(openaldevices audaspace)
	endif()
endif()

if(${WITH_PYTHON} AND ${WITH_JACK} AND ${WITH_OPENAL} AND ${WITH_SDL})
	set(PYTHON_PLUGIN_LIBS)

	if(${PLUGIN_OPENAL})
		list(APPEND PYTHON_PLUGIN_LIBS audopenal)
	endif()

	if(${PLUGIN_SDL})
		list(APPEND PYTHON_PLUGIN_LIBS audsdl)
	endif()

	if(${PLUGIN_JACK})
		list(APPEND PYTHON_PLUGIN_LIBS audjack)
	endif()

	add_executable(python bindings/python/PyAPI.cpp bindings/python/PyAPI.h bindings/python/PyInit.h)
	target_link_libraries(python audaspace ${PYTHON_LIBRARIES} ${PYTHON_PLUGIN_LIBS})
endif()
