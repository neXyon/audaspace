/*******************************************************************************
* Copyright 2009-2015 Juan Francisco Crespo Galán
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
******************************************************************************/

#include "fx/PlaybackManager.h"
#include "fx/VolumeSound.h"

#include <stdexcept> 

AUD_NAMESPACE_BEGIN
PlaybackManager::PlaybackManager(std::shared_ptr<IDevice> device) :
	m_device(device), m_currentKey(0)
{
}

unsigned int PlaybackManager::addCategory(std::shared_ptr<PlaybackCategory> category)
{
	m_categories[m_currentKey] = category;
	unsigned int k = m_currentKey;
	m_currentKey++;
	return k;
}

std::shared_ptr<IHandle> PlaybackManager::play(std::shared_ptr<ISound> sound, unsigned int catKey)
{
	std::shared_ptr<PlaybackCategory> category;
	try
	{
		category = m_categories.at(catKey);
	}
	catch (std::out_of_range& oor)
	{
		category = std::make_shared<PlaybackCategory>(m_device);
		m_categories[catKey] = category;
		m_currentKey = catKey + 1;
	}
	return category->play(sound);
}

bool PlaybackManager::resume(unsigned int catKey)
{
	try
	{
		m_categories.at(catKey)->resume();
		return true;
	}
	catch (std::out_of_range& oor) 
	{
		return false;
	}
}

bool PlaybackManager::pause(unsigned int catKey)
{
	try
	{
		m_categories.at(catKey)->pause();
		return true;
	}
	catch (std::out_of_range& oor)
	{
		return false;
	}
}

float PlaybackManager::getVolume(unsigned int catKey)
{
	try
	{
		return m_categories.at(catKey)->getVolume();
	}
	catch (std::out_of_range& oor)
	{
		return -1.0;
	}
}

bool PlaybackManager::setVolume(float volume, unsigned int catKey)
{
	try
	{
		m_categories.at(catKey)->setVolume(volume);
		return true;
	}
	catch (std::out_of_range& oor)
	{
		return false;
	}
}

bool PlaybackManager::stop(unsigned int catKey)
{
	try
	{
		m_categories.at(catKey)->stop();
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

bool PlaybackManager::clean(unsigned int catKey)
{
	try
	{
		m_categories.at(catKey)->cleanHandles();
		return true;
	}
	catch (std::out_of_range& oor)
	{
		return false;
	}
}
AUD_NAMESPACE_END
