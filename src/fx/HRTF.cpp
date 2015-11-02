#include "fx/HRTF.h"

AUD_NAMESPACE_BEGIN
HRTF::HRTF() :
	HRTF(std::make_shared<FFTPlan>(false))
{
}

HRTF::HRTF(std::shared_ptr<FFTPlan> plan) :
	m_plan(plan)
{
}

void HRTF::addImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse, float azimuth, float elevation)
{
	m_hrtfs[azimuth][elevation] = std::make_shared<ImpulseResponse>(impulseResponse, m_plan);
}

std::shared_ptr<ImpulseResponse> HRTF::getImpulseResponse(float &azimuth, float &elevation)
{
	if (m_hrtfs.empty())
		return nullptr;

	float az = 0, el = 0, dif=0, minDif=360;
	for (auto elem : m_hrtfs)
	{
		dif = abs(azimuth - elem.first);
		if (dif < minDif)
		{
			minDif = dif;
			az = elem.first;
		}
	}
	azimuth = az;
	dif = 0; 
	minDif = 360;
	for (auto elem : m_hrtfs[azimuth])
	{
		dif = abs(elevation - elem.first);
		if (dif < minDif)
		{
			minDif = dif;
			el = elem.first;
		}
	}
	elevation = el;
	return m_hrtfs[azimuth][elevation];
}
AUD_NAMESPACE_END