#include "fx/PlaybackCategory.h"
#include "fx/VolumeSound.h"

AUD_NAMESPACE_BEGIN
PlaybackCategory::PlaybackCategory(std::shared_ptr<IDevice> device) :
	m_device(device), m_volumeStorage(std::make_shared<VolumeStorage>(1.0f)), m_status(STATUS_PLAYING)
{
}

std::shared_ptr<IHandle> PlaybackCategory::play(std::shared_ptr<ISound> sound)
{
	std::shared_ptr<ISound> vs(std::make_shared<VolumeSound>(sound, m_volumeStorage));
	m_device->lock();
	auto handle = m_device->play(vs);
	switch (m_status) 
	{
	case STATUS_STOPPED:
		handle->stop();
		break;
	case STATUS_PAUSED:
		handle->pause();
		break;
	};
	m_device->unlock();
	m_handles.push_back(handle);
	return handle;
}

void PlaybackCategory::resume()
{
	for (auto han : m_handles)
	{
		han->resume();
	}
	m_status = STATUS_PLAYING;
}

void PlaybackCategory::pause()
{
	for (auto han : m_handles)
	{
		han->pause();
	}
	m_status = STATUS_PAUSED;
}

float PlaybackCategory::getVolume()
{
	return m_volumeStorage->getVolume();
}

void PlaybackCategory::setVolume(float volume)
{
	m_volumeStorage->setVolume(volume);
}

void PlaybackCategory::stop() 
{
	for (auto han : m_handles)
	{
		han->stop();
	}
	m_status = STATUS_STOPPED;
}

std::shared_ptr<VolumeStorage> PlaybackCategory::getSharedVolume()
{
	return m_volumeStorage;
}
AUD_NAMESPACE_END
