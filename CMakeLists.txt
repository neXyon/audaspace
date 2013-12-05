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

# dependencies

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

set(Python_ADDITIONAL_VERSIONS 3.3)

find_package(SDL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(LibSndFile REQUIRED)
find_package(Jack REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(PythonLibs REQUIRED)

# directories

include_directories(
	${SDL_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}
	${LIBSNDFILE_INCLUDE_DIRS}
	${JACK_INCLUDE_DIRS}
	${FFMPEG_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
	include
)

link_directories(
)

# sources

file(GLOB_RECURSE SRC src/*.cpp)
file(GLOB_RECURSE PRIVATE_HDR src/*.h)
file(GLOB_RECURSE PUBLIC_HDR include/*.h)

set(HDR ${PRIVATE_HDR} ${PUBLIC_HDR})

# options

ADD_DEFINITIONS(-std=c++11)

# library

add_library(audaspace SHARED ${SRC} ${HDR})

target_link_libraries(audaspace
	${SDL_LIBRARY}
	${OPENAL_LIBRARY}
	${LIBSNDFILE_LIBRARIES}
	${JACK_LIBRARIES}
	${FFMPEG_LIBRARIES}
)

# demos

add_executable(audaplay demos/audaplay.cpp)

target_link_libraries(audaplay
	audaspace
)

add_executable(audaconvert demos/audaconvert.cpp)

target_link_libraries(audaconvert
	audaspace
)

add_executable(audaremap demos/audaremap.cpp)

target_link_libraries(audaremap
	audaspace
)

add_executable(python bindings/python/PyAPI.cpp bindings/python/PyAPI.h bindings/python/PyInit.h)

target_link_libraries(python
	audaspace
	${PYTHON_LIBRARIES}
)
