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