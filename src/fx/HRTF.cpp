#include "fx/HRTF.h"
#include "Exception.h"

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
}

bool HRTF::addImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse, float azimuth, float elevation)
{
	Specs spec = impulseResponse->getSpecs();
	if ((azimuth >= 360 || azimuth<0) || (spec.channels != CHANNELS_MONO) || (spec.rate != m_specs.rate && m_specs.rate > 0.0))
		return false;

	m_hrtfs[elevation][azimuth] = std::make_shared<ImpulseResponse>(impulseResponse, m_plan);
	m_specs.channels = CHANNELS_MONO;
	m_specs.rate = spec.rate;

	return true;
}

std::pair<std::shared_ptr<ImpulseResponse>, std::shared_ptr<ImpulseResponse>> HRTF::getImpulseResponse(float &azimuth, float &elevation)
{
	if (m_hrtfs.empty())
		return std::make_pair(nullptr, nullptr);

	float az = 0, el = 0, dif=0, minDif=360;
	for (auto elem : m_hrtfs)
	{
		dif = abs(elevation - elem.first);
		if (dif < minDif)
		{
			minDif = dif;
			el = elem.first;
		}
	}
	elevation = el;
	dif = 0; 
	minDif = 360;
	
	for (auto elem : m_hrtfs[elevation])
	{
		dif = abs(azimuth - elem.first);
		if (dif < minDif)
		{
			minDif = dif;
			az = elem.first;
		}
	}
	azimuth = az;
	float azL = 360 - azimuth;
	if (azL == 360)
		azL = 0;

	return std::make_pair(m_hrtfs[elevation][azL], m_hrtfs[elevation][azimuth]);
}

Specs HRTF::getSpecs()
{
	return m_specs;
}
AUD_NAMESPACE_END