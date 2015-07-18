#include "fx/DynamicMusicPlayer.h"
#include "generator/Silence.h"
#include "fx/Fader.h"

#include <mutex>
#include <condition_variable>

AUD_NAMESPACE_BEGIN

DynamicMusicPlayer::DynamicMusicPlayer(std::shared_ptr<IDevice> device) :
m_id(0), m_fadeTime(1.0f), m_device(device)
{
	m_scenes.push_back(new std::vector<std::shared_ptr<ISound>>(1, NULL));
}

DynamicMusicPlayer::~DynamicMusicPlayer()
{
	stop();
	for (int i = 0; i < m_scenes.size(); i++)
		delete m_scenes[i];
}

int DynamicMusicPlayer::addScene(std::shared_ptr<ISound> sound)
{
	auto v = new std::vector<std::shared_ptr<ISound>>();
	for (int i = 0; i < m_scenes.size(); i++)
		v->push_back(NULL);
	m_scenes.push_back(v);
	for (int i = 0; i < m_scenes.size(); i++)
		m_scenes[i]->push_back(NULL);
	m_scenes.back()->back() = sound;

	return m_scenes.size() - 1;
}

void DynamicMusicPlayer::changeScene(int id)
{
	if (id == m_id || id >= m_scenes.size())
		return;
	else
	{
		stopCallback callback;
		if ((*m_scenes[m_id])[id] == NULL)
		{
			callback = [](void* pData)
						{
							auto dat = reinterpret_cast<PlayData*>(pData);
							dat->device->lock();
							*dat->handle = dat->device->play(dat->sound);
							(*dat->handle)->setLoopCount(-1);
							dat->device->unlock();
						};

			m_device->lock();
			m_pData.device = m_device;
			m_pData.sound = std::make_shared<Fader>((*m_scenes[id])[id], FADE_IN, 0.0f, m_fadeTime);
			m_pData.handle = &m_currentHandle;	
			m_device->unlock();
		}
		else
		{
			callback = [](void* pData)
						{
							auto callback2 = [](void* pData)
							{
								auto dat = reinterpret_cast<PlayData*>(pData);
								dat->device->lock();
								*dat->handle = dat->device->play(dat->sound);
								(*dat->handle)->setLoopCount(-1);
								dat->device->unlock();
							};
							auto dat = reinterpret_cast<PlayData*>(pData);
							dat->device->lock();
							*dat->handle = dat->device->play(dat->transition);
							if (dat->sound!=NULL)
								(*dat->handle)->setStopCallback(callback2, pData);
							dat->device->unlock();
						};

			m_device->lock();
			m_pData.device = m_device;
			m_pData.sound = (*m_scenes[id])[id];
			m_pData.transition = (*m_scenes[m_id])[id];
			m_pData.handle = &m_currentHandle;
			m_device->unlock();
		}

		if (m_id == 0 || m_currentHandle->getStatus() == STATUS_INVALID)
			callback(&m_pData);
		else
		{
			m_device->lock();
			m_currentHandle->setLoopCount(0);
			m_currentHandle->setStopCallback(callback, &m_pData);
			m_device->unlock();
		}

		m_id = id;
	}
}

int DynamicMusicPlayer::getScene()
{
	return m_id;
}

void DynamicMusicPlayer::addTransition(int init, int end, std::shared_ptr<ISound> sound)
{
	if (init != end && init < m_scenes.size() && end < m_scenes.size())
		(*m_scenes[init])[end] = sound;
}

void DynamicMusicPlayer::setFadeTime(float seconds)
{
	m_fadeTime = seconds;
}

float DynamicMusicPlayer::getFadeTime()
{
	return m_fadeTime;
}

bool DynamicMusicPlayer::resume()
{
	m_device->lock();
	bool result = m_currentHandle->resume();
	m_device->unlock();
	return result;
}

bool DynamicMusicPlayer::pause()
{
	m_device->lock();
	bool result = m_currentHandle->pause();
	m_device->unlock();
	return result;
}

bool DynamicMusicPlayer::seek(float position)
{
	m_device->lock();
	bool result = m_currentHandle->seek(position);
	m_device->unlock();
	return result;
}

float DynamicMusicPlayer::getPosition()
{
	m_device->lock();
	float result = m_currentHandle->getPosition();
	m_device->unlock();
	return result;
}

float DynamicMusicPlayer::getPitch()
{
	m_device->lock();
	float result = m_currentHandle->getPitch();
	m_device->unlock();
	return result;
}

bool DynamicMusicPlayer::setPitch(float pitch)
{
	m_device->lock();
	bool result = m_currentHandle->setPitch(pitch);
	m_device->unlock();
	return result;
}

float DynamicMusicPlayer::getVolume()
{
	m_device->lock();
	float result = m_currentHandle->getVolume();
	m_device->unlock();
	return result;
}

bool DynamicMusicPlayer::setVolume(float volume)
{
	m_device->lock();
	bool result = m_currentHandle->setVolume(volume);
	m_device->unlock();
	return result;
}

Status DynamicMusicPlayer::getStatus()
{
	m_device->lock();
	Status result = m_currentHandle->getStatus();
	m_device->unlock();
	return result;
}

bool DynamicMusicPlayer::stop()
{
	m_id = 0;
	m_device->lock();
	bool result = m_currentHandle->stop();
	m_device->unlock();
	return result;
}

AUD_NAMESPACE_END