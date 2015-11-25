/*******************************************************************************
* Copyright 2009-2015 Juan Francisco Crespo Galán
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
******************************************************************************/

#pragma once

/**
* @file HRTFLoader.h
* @ingroup fx
* The HRTFLoader class.
*/

#include "Audaspace.h"
#include "fx/HRTF.h"
#include "util/FFTPlan.h"

#include <string>
#include <memory>

AUD_NAMESPACE_BEGIN

/**
* This loader provides a method to load all the HRTFs in one directory, provided they follow the following naming scheme:
* Example: L-10e210a.wav
* The first character refers to the ear from which the HRTF was recorded: 'L' for a left ear and 'R' for a right ear.
* Next is the elevation angle followed by the 'e' character. [-90, 90]
* Then is the azimuth angle followed by the 'a' character. [0, 360)
* For a sound source situated at the left of the listener the azimuth angle regarding the left ear is 90 while the angle regarding the right ear is 270.
* KEMAR HRTFs use this naming scheme.
*/
class AUD_API HRTFLoader
{
private:
	// delete normal constructor, copy constructor and operator=
	HRTFLoader(const HRTFLoader&) = delete;
	HRTFLoader& operator=(const HRTFLoader&) = delete;
	HRTFLoader() = delete;

public:
	static std::shared_ptr<HRTF> loadLeftHRTFs(std::shared_ptr<FFTPlan> plan, const std::string& fileExtension, const std::string& path = "");
	static std::shared_ptr<HRTF> loadRightHRTFs(std::shared_ptr<FFTPlan> plan, const std::string& fileExtension, const std::string& path = "");

private:
	static void loadHRTFs(std::shared_ptr<HRTF>hrtfs, char ear, const std::string& fileExtension, const std::string& path = "");
};

AUD_NAMESPACE_END
