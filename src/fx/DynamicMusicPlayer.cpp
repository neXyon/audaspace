#include "fx/DynamicMusicPlayer.h"
#include "generator/Silence.h"
#include "fx/Fader.h"
#include "fx/Limiter.h"

#include <mutex>
#include <condition_variable>

AUD_NAMESPACE_BEGIN

DynamicMusicPlayer::DynamicMusicPlayer(std::shared_ptr<IDevice> device) :
m_device(device), m_fadeTime(1.0f)
{
	m_id = 0;
	m_transitioning = false;
	m_stopThread = false;
	m_volume = m_device->getVolume();
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
	if (id >= m_scenes.size() || m_transitioning)
		return false;
	else
	{
		if (m_fadeThread.joinable())
			m_fadeThread.join();
		m_device->lock();
		if (id == m_id)
		{
			//float time = m_currentHandle->getPosition();
			//m_currentHandle->stop();
			//m_currentHandle = m_device->play(m_scenes[m_id][m_id]);
			//m_currentHandle->seek(time);
			m_currentHandle->setVolume(m_volume);
			m_currentHandle->setLoopCount(-1);
		}
		else
		{
			m_soundTarget = id;
			if (m_scenes[m_id][id] == nullptr)
			{
				m_stopThread = false;
				if ((m_scenes[m_id][m_id] != nullptr && m_currentHandle->getStatus() != STATUS_INVALID) || m_scenes[m_soundTarget][m_soundTarget] != nullptr)
				{
					m_transitioning = true;
					if (m_scenes[m_id][m_id] == nullptr || m_currentHandle->getStatus() == STATUS_INVALID)
					{
						m_device->lock();
						m_currentHandle = m_device->play(m_scenes[m_soundTarget][m_soundTarget]);
						m_currentHandle->setVolume(0.0f);
						m_currentHandle->setLoopCount(-1);
						m_device->unlock();
						m_fadeThread = std::thread(&DynamicMusicPlayer::fadeInThread, this);
					}
					else
					{
						if (m_scenes[m_soundTarget][m_soundTarget] != nullptr)
						{
							m_device->lock();
							m_transitionHandle = m_currentHandle;
							m_currentHandle = m_device->play(m_scenes[m_soundTarget][m_soundTarget]);
							m_currentHandle->setVolume(0.0f);
							m_currentHandle->setLoopCount(-1);
							m_device->unlock();
							m_fadeThread = std::thread(&DynamicMusicPlayer::crossfadeThread, this);
						}
						else
						{
							m_transitionHandle = m_currentHandle;
							m_currentHandle = nullptr;
							m_fadeThread = std::thread(&DynamicMusicPlayer::fadeOutThread, this);
						}
					}
				}
				/*if (m_scenes[m_id][m_id] == nullptr || m_currentHandle->getStatus() == STATUS_INVALID)
				{
					if (m_scenes[m_soundTarget][m_soundTarget] != nullptr)
					{
						m_currentHandle = m_device->play(std::make_shared<Fader>(m_scenes[m_soundTarget][m_soundTarget], FADE_IN, 0.0f, m_fadeTime));
						m_currentHandle->setVolume(m_volume);
						m_currentHandle->setStopCallback(sceneCallback, this);
					}
					m_id = id;
				}
				else
				{
					m_transitioning = true;
					float time = m_currentHandle->getPosition();
					m_currentHandle->stop();
					m_transitionHandle = m_device->play(std::make_shared<Fader>(std::make_shared<Limiter>(m_scenes[m_id][m_id], time, time + m_fadeTime), FADE_OUT, 0.0f, m_fadeTime));
					m_currentHandle->setVolume(m_volume);
					m_transitionHandle->setStopCallback(fadeCallback, this);
					if (m_scenes[m_soundTarget][m_soundTarget] != nullptr)
					{
						m_currentHandle = m_device->play(std::make_shared<Fader>(m_scenes[m_soundTarget][m_soundTarget], FADE_IN, 0.0f, m_fadeTime));
						m_currentHandle->setVolume(m_volume);
						m_currentHandle->setStopCallback(sceneCallback, this);
					}
				}*/
			}
			else
			{
				if (m_scenes[m_id][m_id] == nullptr || m_currentHandle->getStatus() == STATUS_INVALID)
					transitionCallback(this);
				else
				{
					m_currentHandle->setLoopCount(0);
					m_currentHandle->setStopCallback(transitionCallback, this);
				}
			}
		}
		m_device->unlock();
		return true;
	}
}

int DynamicMusicPlayer::getScene()
{
	return m_id;
}

void DynamicMusicPlayer::addTransition(int init, int end, std::shared_ptr<ISound> sound)
{
	if (init != end && init < m_scenes.size() && end < m_scenes.size() && init >= 0 && end >= 0)
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
	bool result = false, resultTrans = false;

	if (m_currentHandle != nullptr)
		result = m_currentHandle->resume();
	if (m_transitionHandle != nullptr)
		resultTrans = m_transitionHandle->resume();

	return result || resultTrans;
}

bool DynamicMusicPlayer::pause()
{
	bool result = false, resultTrans = false;

	if (m_currentHandle != nullptr)
		result = m_currentHandle->pause();
	if (m_transitionHandle != nullptr)
		resultTrans = m_transitionHandle->pause();

	return result || resultTrans;
}

bool DynamicMusicPlayer::seek(float position)
{
	bool result;

	if (m_currentHandle != nullptr) 
	{
		result = m_currentHandle->seek(position);
		if (m_transitionHandle != nullptr && result == true)
			m_transitionHandle->stop();
	}

	return result;
}

float DynamicMusicPlayer::getPosition()
{
	float result = 0.0f;

	if (m_currentHandle != nullptr)
		result = m_currentHandle->getPosition();

	return result;
}

float DynamicMusicPlayer::getVolume()
{
	return m_volume;
}

bool DynamicMusicPlayer::setVolume(float volume)
{
	m_volume = volume;
	bool result = false, resultTrans = false;

	if (m_currentHandle != nullptr)
		result = m_currentHandle->setVolume(volume);
	if (m_transitionHandle != nullptr)
	{
		m_device->lock();
		if(volume<m_transitionHandle->getVolume())
			resultTrans = m_transitionHandle->setVolume(0.0f);
		m_device->unlock();
	}

	return result || resultTrans;
}

Status DynamicMusicPlayer::getStatus()
{
	if (m_currentHandle != nullptr)
	{
		Status result = m_currentHandle->getStatus();
		return result;
	}
	else
		return STATUS_INVALID;
}

bool DynamicMusicPlayer::stop()
{
	m_stopThread = true;
	bool result = false, resultTrans = false;

	if (m_currentHandle != nullptr)
		result = m_currentHandle->stop();
	if (m_transitionHandle != nullptr)

		resultTrans = m_transitionHandle->stop();

	if (m_fadeThread.joinable())
		m_fadeThread.join();
	m_id = 0;

	return result || resultTrans;
}

void DynamicMusicPlayer::transitionCallback(void* player)
{
	auto dat = reinterpret_cast<DynamicMusicPlayer*>(player);
	dat->m_transitioning = true;
	dat->m_device->lock();
	dat->m_currentHandle = dat->m_device->play(dat->m_scenes[dat->m_id][dat->m_soundTarget]);
	dat->m_currentHandle->setVolume(dat->m_volume);
	if (dat->m_scenes[dat->m_soundTarget][dat->m_soundTarget] != nullptr)
		dat->m_currentHandle->setStopCallback(sceneCallback, player);	
	dat->m_device->unlock();
}

void DynamicMusicPlayer::sceneCallback(void* player)
{
	auto dat = reinterpret_cast<DynamicMusicPlayer*>(player);
	dat->m_device->lock();
	dat->m_currentHandle = dat->m_device->play(dat->m_scenes[dat->m_soundTarget][dat->m_soundTarget]);
	dat->m_currentHandle->setVolume(dat->m_volume);
	dat->m_currentHandle->setLoopCount(-1);
	dat->m_device->unlock();
	dat->m_id = int(dat->m_soundTarget);
	dat->m_soundTarget = -1;
	dat->m_transitioning = false;
}

void DynamicMusicPlayer::fadeCallback(void* player)
{
	auto dat = reinterpret_cast<DynamicMusicPlayer*>(player);
	dat->m_transitioning = false;
}

void DynamicMusicPlayer::crossfadeThread()
{
	float currentVol = m_transitionHandle->getVolume();
	float nextVol = m_currentHandle->getVolume();
	float increment;

	while (nextVol < m_volume && !m_stopThread)
	{
		increment = (m_volume / (m_fadeTime * 1000)) * 20;
		currentVol -= increment;
		nextVol += increment;
		if (currentVol < 0)
			currentVol = 0;
		if (nextVol > m_volume)
			nextVol = m_volume;
		m_transitionHandle->setVolume(currentVol);
		m_currentHandle->setVolume(nextVol);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	if (m_stopThread)
		m_transitionHandle->setVolume(m_volume);

	m_transitionHandle->stop();

	m_id = int(m_soundTarget);
	m_transitioning = false;
}

void DynamicMusicPlayer::fadeInThread()
{
	float nextVol = m_currentHandle->getVolume();
	float increment;

	while (nextVol < m_volume && !m_stopThread)
	{
		increment = (m_volume / (m_fadeTime * 1000)) * 20;
		nextVol += increment;
		if (nextVol > m_volume)
			nextVol = m_volume;
		m_currentHandle->setVolume(nextVol);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	if (m_stopThread)
		m_currentHandle->setVolume(m_volume);

	m_id = int(m_soundTarget);
	m_transitioning = false;
}

void DynamicMusicPlayer::fadeOutThread()
{
	float currentVol = m_transitionHandle->getVolume();
	float increment;

	while (currentVol > 0.0f && !m_stopThread)
	{
		increment = (m_volume / (m_fadeTime * 1000)) * 20;
		currentVol -= increment;
		if (currentVol < 0)
			currentVol = 0;
		m_transitionHandle->setVolume(currentVol);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	m_transitionHandle->stop();
	m_id = int(m_soundTarget);
	m_transitioning = false;
}
AUD_NAMESPACE_END
