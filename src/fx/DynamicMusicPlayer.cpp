#include "fx/DynamicMusicPlayer.h"
#include "generator/Silence.h"
#include "fx/Fader.h"

#include <mutex>
#include <condition_variable>

AUD_NAMESPACE_BEGIN

DynamicMusicPlayer::DynamicMusicPlayer(std::shared_ptr<IDevice> device) :
m_device(device)
{
	m_id = 0;
	m_fadeTime = 1.0f;
	m_transitioning = false;
	m_scenes.push_back(std::vector<std::shared_ptr<ISound>>(1, nullptr));
}

DynamicMusicPlayer::~DynamicMusicPlayer()
{
	stop();
}

int DynamicMusicPlayer::addScene(std::shared_ptr<ISound> sound)
{
	std::vector<std::shared_ptr<ISound>> v;
	m_scenes.push_back(v);
	for (int i = 0; i < m_scenes.size()-1; i++)
		m_scenes.back().push_back(nullptr);
	for (int i = 0; i < m_scenes.size()-1; i++)
		m_scenes[i].push_back(nullptr);
	m_scenes.back().push_back(sound);

	return m_scenes.size() - 1;
}

bool DynamicMusicPlayer::changeScene(int id)
{
	if (id == m_id || id >= m_scenes.size() || m_transitioning)
		return false;
	else
	{
		m_soundTarget = id;
		stopCallback callback;
		if (m_scenes[m_id][id] == nullptr)
		{
			m_device->lock();
			if (m_scenes[m_id][m_id] == nullptr || m_currentHandle->getStatus() == STATUS_INVALID)
			{
				fadeCallback(this);
			}
			else
			{
				float time = m_currentHandle->getPosition();
				m_currentHandle->stop();
				auto reader = m_scenes[m_id][m_id]->createReader();
				m_currentHandle = m_device->play(std::make_shared<Fader>(m_scenes[m_id][m_id], FADE_OUT, (reader->getLength() / reader->getSpecs().rate) - m_fadeTime, m_fadeTime));
				m_currentHandle->seek(time);
				m_currentHandle->setStopCallback(fadeCallback, this);
			}
			m_device->unlock();
		}
		else
		{
			m_device->lock();
			if (m_scenes[m_id][m_id] == nullptr || m_currentHandle->getStatus() == STATUS_INVALID)
				transitionCallback(this);
			else
			{
				m_currentHandle->setLoopCount(0);
				m_currentHandle->setStopCallback(transitionCallback, this);			
			}
			m_device->unlock();
		}
	}
}

int DynamicMusicPlayer::getScene()
{
	return m_id;
}

void DynamicMusicPlayer::addTransition(int init, int end, std::shared_ptr<ISound> sound)
{
	if (init != end && init < m_scenes.size() && end < m_scenes.size())
		m_scenes[init][end] = sound;
}

void DynamicMusicPlayer::setFadeTime(float seconds)
{
	m_device->lock();
	m_fadeTime = seconds;
	m_device->unlock();
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

void DynamicMusicPlayer::transitionCallback(void* player)
{
	auto dat = reinterpret_cast<DynamicMusicPlayer*>(player);
	dat->m_transitioning = true;
	dat->m_device->lock();
	dat->m_currentHandle = dat->m_device->play(dat->m_scenes[dat->m_id][dat->m_soundTarget]);
	if (dat->m_scenes[dat->m_soundTarget][dat->m_soundTarget] != nullptr)
		dat->m_currentHandle->setStopCallback(sceneCallback, player);	
	dat->m_device->unlock();
}

void DynamicMusicPlayer::sceneCallback(void* player)
{
	auto dat = reinterpret_cast<DynamicMusicPlayer*>(player);
	dat->m_device->lock();
	dat->m_currentHandle = dat->m_device->play(dat->m_scenes[dat->m_soundTarget][dat->m_soundTarget]);
	dat->m_currentHandle->setLoopCount(-1);
	dat->m_device->unlock();
	dat->m_id = dat->m_soundTarget;
	dat->m_transitioning = false;
}

void DynamicMusicPlayer::fadeCallback(void* player)
{
	auto dat = reinterpret_cast<DynamicMusicPlayer*>(player);
	dat->m_transitioning = true;
	dat->m_device->lock();
	if (dat->m_scenes[dat->m_soundTarget][dat->m_soundTarget] != nullptr)
	{
		dat->m_currentHandle = dat->m_device->play(std::make_shared<Fader>(dat->m_scenes[dat->m_soundTarget][dat->m_soundTarget], FADE_IN, 0.0f, dat->m_fadeTime));
		dat->m_currentHandle->setStopCallback(sceneCallback, player);
	}
	dat->m_device->unlock();
	dat->m_id = dat->m_soundTarget;
	dat->m_transitioning = false;
}
AUD_NAMESPACE_END