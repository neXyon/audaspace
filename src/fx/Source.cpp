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

#include "fx/Source.h"

#include <cmath>

AUD_NAMESPACE_BEGIN
Source::Source(float azimuth, float elevation, float distance) :
	m_azimuth(azimuth), m_elevation(elevation), m_distance(1.0 - distance)
{
	if(m_distance < 0)
		m_distance = 0.0;
}

float Source::getAzimuth()
{
	return m_azimuth;
}

float Source::getElevation()
{
	return m_elevation;
}

float Source::getDistance()
{
	return m_distance;
}

void Source::setAzimuth(float azimuth)
{
	m_azimuth = azimuth;
}

void Source::setElevation(float elevation)
{
	m_elevation = elevation;
}

void Source::setDistance(float distance)
{
	m_distance = 1.0 - distance;
	if(m_distance < 0)
		m_distance = 0.0;
}
AUD_NAMESPACE_END