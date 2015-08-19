#include "fx/PlaybackManager.h"
#include "fx/VolumeSound.h"

#include <stdexcept> 

AUD_NAMESPACE_BEGIN
PlaybackManager::PlaybackManager(std::shared_ptr<IDevice> device) :
	m_device(device)
{
}

bool PlaybackManager::addCategory(std::shared_ptr<PlaybackCategory> category, std::string catName)
{
	try
	{
		auto cat = m_categories.at(catName);
		return false;
	}
	catch (std::out_of_range& oor)
	{
		m_categories[catName] = category;
	}
}

std::shared_ptr<IHandle> PlaybackManager::play(std::shared_ptr<ISound> sound, std::string catName)
{
	std::shared_ptr<PlaybackCategory> category;
	try
	{
		category = m_categories.at(catName);
	}
	catch (std::out_of_range& oor)
	{
		category = std::make_shared<PlaybackCategory>(m_device);
		m_categories[catName] = category;
	}
	return category->play(sound);
}

bool PlaybackManager::resume(std::string catName)
{
	try
	{
		m_categories.at(catName)->resume();
		return true;
	}
	catch (std::out_of_range& oor) 
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
	catch (std::out_of_range& oor)
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
	catch (std::out_of_range& oor)
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
	catch (std::out_of_range& oor)
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
	catch (std::out_of_range& oor)
	{
		return false;
	}
}

void PlaybackManager::clean()
{
	for (auto cat : m_categories)
		cat.second->cleanHandles();
}

bool PlaybackManager::clean(std::string catName)
{
	try
	{
		m_categories.at(catName)->cleanHandles();
		return true;
	}
	catch (std::out_of_range& oor)
	{
		return false;
	}
}
AUD_NAMESPACE_END
