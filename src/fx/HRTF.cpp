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

#include "fx/HRTF.h"
#include "Exception.h"

#include <cmath>

AUD_NAMESPACE_BEGIN
HRTF::HRTF() :
	HRTF(std::make_shared<FFTPlan>(false))
{
}

HRTF::HRTF(std::shared_ptr<FFTPlan> plan) :
	m_plan(plan)
{
	m_specs.channels = CHANNELS_INVALID;
	m_specs.rate = 0;
	m_empty = true;
}

bool HRTF::addImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse, float azimuth, float elevation)
{
	Specs spec = impulseResponse->getSpecs();

	azimuth = std::fmod(azimuth, 360);
	if(azimuth < 0)
		azimuth += 360;

	if((spec.channels != CHANNELS_MONO) || (spec.rate != m_specs.rate && m_specs.rate > 0.0))
		return false;

	m_hrtfs[elevation][azimuth] = std::make_shared<ImpulseResponse>(impulseResponse, m_plan);
	m_specs.channels = CHANNELS_MONO;
	m_specs.rate = spec.rate;
	m_empty = false;
	return true;
}

std::pair<std::shared_ptr<ImpulseResponse>, std::shared_ptr<ImpulseResponse>> HRTF::getImpulseResponse(float &azimuth, float &elevation)
{
	if(m_hrtfs.empty())
		return std::make_pair(nullptr, nullptr);
	azimuth = std::fmod(azimuth, 360);
	if(azimuth < 0)
		azimuth += 360;

	float az = 0, el = 0, dif=0, minDif=360;
	for(auto elem : m_hrtfs)
	{
		dif = std::fabs(elevation - elem.first);
		if(dif < minDif)
		{
			minDif = dif;
			el = elem.first;
		}
	}
	elevation = el;
	dif = 0; 
	minDif = 360;
	
	for(auto elem : m_hrtfs[elevation])
	{
		dif = std::fabs(azimuth - elem.first);
		if(dif < minDif)
		{
			minDif = dif;
			az = elem.first;
		}
	}
	azimuth = az;
	float azL = 360 - azimuth;
	if(azL == 360)
		azL = 0;

	return std::make_pair(m_hrtfs[elevation][azL], m_hrtfs[elevation][azimuth]);
}

Specs HRTF::getSpecs()
{
	return m_specs;
}

bool HRTF::isEmpty()
{
	return m_empty;
}
AUD_NAMESPACE_END
