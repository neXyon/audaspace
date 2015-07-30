#include "fx/PlaybackManager.h"

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
	return nullptr;
}

void PlaybackManager::resume(std::string catName)
{

}

void PlaybackManager::pause(std::string catName)
{

}

float PlaybackManager::getVolume(std::string catName)
{
	return 0.0f;
}

void PlaybackManager::setVolume(float volume, std::string catName)
{

}

void PlaybackManager::stop(std::string catName)
{

}
AUD_NAMESPACE_END