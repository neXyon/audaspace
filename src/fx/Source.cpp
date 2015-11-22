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

AUD_NAMESPACE_BEGIN
Source::Source(float azimuth, float elevation) :
	m_azimuth(azimuth), m_elevation(elevation)
{
}

float Source::getAzimuth()
{
	return m_azimuth;
}

float Source::getElevation()
{
	return m_elevation;
}

void Source::setAzimuth(float azimuth)
{
	m_azimuth = azimuth;
}

void Source::setElevation(float elevation)
{
	m_elevation = elevation;
}
AUD_NAMESPACE_END