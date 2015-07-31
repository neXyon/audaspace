#include "fx/PlaybackManager.h"

#include <stdexcept> 

AUD_NAMESPACE_BEGIN
PlaybackManager::PlaybackManager(std::shared_ptr<IDevice> device) :
	m_device(device)
{
}

void PlaybackManager::addHandle(std::shared_ptr<IHandle> handle, std::string catName)
{

}

void PlaybackManager::addCategory(std::shared_ptr<PlaybackCategory> category, std::string catName)
{

}

std::shared_ptr<IHandle> PlaybackManager::play(std::shared_ptr<ISound> sound, std::string catName)
{
	m_device->lock();
	auto handle = m_device->play(sound);
	m_categories[catName]->addHandle(handle);
	m_device->unlock();
	return handle;
}

bool PlaybackManager::resume(std::string catName)
{
	try
	{
		m_categories.at(catName)->resume();
		return true;
	}
	catch (std::out_of_range& oot) 
	{
		return false;
	}
}

bool PlaybackManager::pause(std::string catName)
{
	try
	{
		m_categories.at(catName)->pause();
		return true;
	}
	catch (std::out_of_range& oot)
	{
		return false;
	}
}

float PlaybackManager::getVolume(std::string catName)
{
	try
	{
		return m_categories.at(catName)->getVolume();
	}
	catch (std::out_of_range& oot)
	{
		return -1.0;
	}
}

bool PlaybackManager::setVolume(float volume, std::string catName)
{
	try
	{
		m_categories.at(catName)->setVolume(volume);
		return true;
	}
	catch (std::out_of_range& oot)
	{
		return false;
	}
}

bool PlaybackManager::stop(std::string catName)
{
	try
	{
		m_categories.at(catName)->stop();
		return true;
	}
	catch (std::out_of_range& oot)
	{
		return false;
	}
}
AUD_NAMESPACE_END