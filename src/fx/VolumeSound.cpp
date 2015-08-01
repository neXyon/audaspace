#include "fx/VolumeSound.h"
#include "fx/VolumeReader.h"
#include "Exception.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

VolumeSound::VolumeSound(std::shared_ptr<ISound> sound, std::shared_ptr<VolumeStorage> volumeStorage) :
	m_sound(sound), m_volumeStorage(volumeStorage)
{
}

std::shared_ptr<IReader> VolumeSound::createReader()
{
	return std::make_shared<VolumeReader>(m_sound, m_volumeStorage);
}

std::shared_ptr<VolumeStorage> VolumeSound::getSharedVolume()
{
	return m_volumeStorage;
}

void VolumeSound::setSharedVolume(std::shared_ptr<VolumeStorage> volumeStorage)
{
	m_volumeStorage = volumeStorage;
}

AUD_NAMESPACE_END