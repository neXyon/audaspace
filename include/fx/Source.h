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
* @file Source.h
* @ingroup fx
* The Source class.
*/

#include "Audaspace.h"

#include <atomic>

AUD_NAMESPACE_BEGIN

/**
* This class stores the azimuth and elevation angles of a sound and allows to change them dynamically.
*/
class AUD_API Source
{
private:
	/**
	* Azimuth value.
	*/
	std::atomic<float> m_azimuth;

	/**
	* Elevation value.
	*/
	std::atomic<float> m_elevation;

	// delete copy constructor and operator=
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;

public:
	/**
	* Creates a Source instance with an initial value.
	* \param azimuth The value of the azimuth.
	* \param elevation The value of the elevation.
	*/
	Source(float azimuth, float elevation);

	/**
	* Retrieves the current azimuth value.
	* \return The current azimuth.
	*/
	float getAzimuth();

	/**
	* Retrieves the current elevation value.
	* \return The current elevation.
	*/
	float getElevation();

	/**
	* Changes the azimuth value.
	* \param azimuth The new value for the azimuth.
	*/
	void setAzimuth(float azimuth);

	/**
	* Changes the elevation value.
	* \param elevation The new value for the elevation.
	*/
	void setElevation(float elevation);
};

AUD_NAMESPACE_END