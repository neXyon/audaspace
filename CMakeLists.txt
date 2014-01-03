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

project(audaspace)

# sources

file(GLOB_RECURSE SRC src/*.cpp)
file(GLOB_RECURSE PRIVATE_HDR src/*.h)
file(GLOB_RECURSE PUBLIC_HDR include/*.h)

set(HDR ${PRIVATE_HDR} ${PUBLIC_HDR})

set(INCLUDE include)
set(LIBRARIES)

# dependencies

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

option(WITH_FFMPEG "Build FFMPEG Plugin" TRUE)
option(WITH_JACK "Build Jack Plugin" FALSE)
option(WITH_LIBSNDFILE "Build LibSndFile Plugin" TRUE)
option(WITH_OPENAL "Build OpenAL Plugin" TRUE)
option(WITH_PYTHON "Build Python Module" FALSE)
option(WITH_SDL "Build SDL Plugin" FALSE)

# FFMPEG
if(${WITH_FFMPEG})
	find_package(FFMPEG REQUIRED)
	list(APPEND INCLUDE ${FFMPEG_INCLUDE_DIRS})
	list(APPEND LIBRARIES ${FFMPEG_LIBRARIES})
else()
	file(GLOB_RECURSE REMOVE_SRC src/*FFMPEG*.cpp)
	file(GLOB_RECURSE REMOVE_HDR include/*FFMPEG*.h)
	list(REMOVE_ITEM SRC ${REMOVE_SRC})
	list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})
endif()

# Jack
if(${WITH_JACK})
	find_package(Jack REQUIRED)
	list(APPEND INCLUDE ${JACK_INCLUDE_DIRS})
	list(APPEND LIBRARIES ${JACK_LIBRARIES})
else()
	file(GLOB_RECURSE REMOVE_SRC src/*Jack*.cpp)
	file(GLOB_RECURSE REMOVE_HDR include/*Jack*.h)
	list(REMOVE_ITEM SRC ${REMOVE_SRC})
	list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})
endif()

# LibSndFile
if(${WITH_LIBSNDFILE})
	find_package(LibSndFile REQUIRED)
	list(APPEND INCLUDE ${LIBSNDFILE_INCLUDE_DIRS})
	list(APPEND LIBRARIES ${LIBSNDFILE_LIBRARIES})
else()
	file(GLOB_RECURSE REMOVE_SRC src/*SndFile*.cpp)
	file(GLOB_RECURSE REMOVE_HDR include/*SndFile*.h)
	list(REMOVE_ITEM SRC ${REMOVE_SRC})
	list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})
endif()

# OpenAL
if(${WITH_OPENAL})
	find_package(OpenAL REQUIRED)
	list(APPEND INCLUDE ${OPENAL_INCLUDE_DIR})
	list(APPEND LIBRARIES ${OPENAL_LIBRARY})
else()
	file(GLOB_RECURSE REMOVE_SRC src/*OpenAL*.cpp)
	file(GLOB_RECURSE REMOVE_HDR include/*OpenAL*.h)
	list(REMOVE_ITEM SRC ${REMOVE_SRC})
	list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})
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
	list(APPEND INCLUDE ${SDL_INCLUDE_DIR})
	list(APPEND LIBRARIES ${SDL_LIBRARY})
else()
	file(GLOB_RECURSE REMOVE_SRC src/*SDL*.cpp)
	file(GLOB_RECURSE REMOVE_HDR include/*SDL*.h)
	list(REMOVE_ITEM SRC ${REMOVE_SRC})
	list(REMOVE_ITEM PUBLIC_HDR ${REMOVE_SRC})
endif()

# directories and compiler options

include_directories(${INCLUDE})
link_directories()
ADD_DEFINITIONS(-std=c++11)

# library

add_library(audaspace SHARED ${SRC} ${HDR})
target_link_libraries(audaspace ${LIBRARIES})

# demos

add_executable(audaplay demos/audaplay.cpp)
target_link_libraries(audaplay audaspace)

add_executable(audaconvert demos/audaconvert.cpp)
target_link_libraries(audaconvert audaspace)

add_executable(audaremap demos/audaremap.cpp)
target_link_libraries(audaremap audaspace)

add_executable(openaldevices demos/openaldevices.cpp)
target_link_libraries(openaldevices audaspace)

if(${WITH_PYTHON})
	add_executable(python bindings/python/PyAPI.cpp bindings/python/PyAPI.h bindings/python/PyInit.h)
	target_link_libraries(python audaspace ${PYTHON_LIBRARIES})
endif()
