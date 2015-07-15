#include "fx/DynamicMusicPlayer.h"
#include "generator/Silence.h"
#include "fx/Fader.h"

#include <mutex>
#include <condition_variable>

AUD_NAMESPACE_BEGIN

DynamicMusicPlayer::DynamicMusicPlayer(std::shared_ptr<IDevice> device) :
m_id(0), m_fadeTime(1.0f), m_device(device)
{
	m_scenes.push_back(new std::vector<std::shared_ptr<ISound>>(1, std::make_shared<Silence>()));
	m_device->lock();
	m_currentHandle = m_device->play((*m_scenes[m_id])[m_id]);
	m_currentHandle->setLoopCount(-1);
	m_device->unlock();
}

DynamicMusicPlayer::~DynamicMusicPlayer()
{
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
	if (id == m_id)
		return;

	if ((*m_scenes[m_id])[id] == NULL)
	{
		float time = m_currentHandle->getPosition();
		m_device->lock();
		m_currentHandle->stop();
		auto tempHandle = m_device->play(std::make_shared<Fader>((*m_scenes[m_id])[m_id], FADE_OUT, time, m_fadeTime));
		tempHandle->seek(time);
		m_currentHandle = m_device->play(std::make_shared<Fader>((*m_scenes[id])[id], FADE_IN, 0.0f, m_fadeTime));
		m_device->unlock();
	}
	else
	{
		//TODO
	}
	m_id = id;
}

void DynamicMusicPlayer::addTransition(int init, int end, std::shared_ptr<ISound> sound)
{
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

AUD_NAMESPACE_END