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
	m_device->lock();
	for (auto i = m_handles.begin(); i != m_handles.end();)
	{
		if ((*i)->getStatus() == STATUS_INVALID)
			i = m_handles.erase(i);
		else
		{
			(*i)->resume();
			i++;
		}
	}
	m_device->unlock();
	m_status = STATUS_PLAYING;
}

void PlaybackCategory::pause()
{
	m_device->lock();
	for (auto i = m_handles.begin(); i != m_handles.end();)
	{
		if ((*i)->getStatus() == STATUS_INVALID)
			i = m_handles.erase(i);
		else
		{
			(*i)->pause();
			i++;
		}
	}
	m_device->unlock();
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
	m_device->lock();
	for (auto i = m_handles.begin(); i != m_handles.end();)
	{
		(*i)->stop();
		if ((*i)->getStatus() == STATUS_INVALID)
			i = m_handles.erase(i);
		else
			i++;			
	}
	m_device->unlock();
	m_status = STATUS_STOPPED;
}

std::shared_ptr<VolumeStorage> PlaybackCategory::getSharedVolume()
{
	return m_volumeStorage;
}

void PlaybackCategory::cleanHandles()
{
	for (auto i = m_handles.begin(); i != m_handles.end();)
	{
		if ((*i)->getStatus() == STATUS_INVALID)
			i = m_handles.erase(i);
		else
			i++;
	}
}
AUD_NAMESPACE_END
