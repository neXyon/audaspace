#include "fx/VolumeStorage.h"

AUD_NAMESPACE_BEGIN
VolumeStorage::VolumeStorage() :
	m_volume(1.0f)
{
}

VolumeStorage::VolumeStorage(float volume) :
	m_volume(volume)
{
}

float VolumeStorage::getVolume()
{
	return m_volume;
}

void VolumeStorage::setVolume(float volume)
{
	m_volume = volume;
}
AUD_NAMESPACE_END
