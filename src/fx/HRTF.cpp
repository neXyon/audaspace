#include "fx/HRTF.h"

#include <algorithm>

AUD_NAMESPACE_BEGIN
HRTF::HRTF()
{
}

void HRTF::addImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse, float azimuth, float elevation, std::shared_ptr<FFTPlan> plan)
{
	m_hrtfs[azimuth][elevation] = std::make_shared<ImpulseResponse>(impulseResponse, plan);
}

void HRTF::addImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse, float azimuth, float elevation)
{
	m_hrtfs[azimuth][elevation] = std::make_shared<ImpulseResponse>(impulseResponse);
}

void HRTF::addImpulseResponse(std::shared_ptr<ImpulseResponse> impulseResponse, float azimuth, float elevation)
{
	m_hrtfs[azimuth][elevation] = impulseResponse;
}

std::shared_ptr<ImpulseResponse> HRTF::getImpulseResponse(float &azimuth, float &elevation)
{
	if (m_hrtfs.empty())
		return nullptr;

	float az = 0, el = 0, dif=0, minDif=360;
	for (auto elem : m_hrtfs)
	{
		dif = abs(azimuth - elem.first);
		minDif = std::min(dif, minDif);
	}
	azimuth = minDif;
	for (auto elem : m_hrtfs[azimuth])
	{
		dif = abs(elevation - elem.first);
		minDif = std::min(dif, minDif);
	}
	elevation = minDif;
	return m_hrtfs[azimuth][elevation];
}
AUD_NAMESPACE_END