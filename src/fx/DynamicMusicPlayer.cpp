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
	m_currentHandle->stop();
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

	if ((*m_scenes[m_id])[id] == NULL)
	{
		m_currentHandle->pause();
		float time = m_currentHandle->getPosition();
		m_currentHandle->stop();
		m_currentHandle = m_device->play(std::make_shared<Fader>((*m_scenes[id])[id], FADE_IN, 0.0f, m_fadeTime));

		m_device->lock();
		auto tempHandle = m_device->play(std::make_shared<Fader>((*m_scenes[m_id])[m_id], FADE_OUT, time, m_fadeTime));
		tempHandle->seek(time);
		m_device->unlock();
	}
	else
	{
		auto callback = [](void* pData)
		{
			auto dat = reinterpret_cast<PlayData*>(pData);
			*dat->handle = dat->device->play(dat->sound);
		};

		m_pData.device = m_device;
		m_pData.sound = (*m_scenes[id])[id];
		m_pData.handle = &m_currentHandle;

		m_currentHandle->stop();
		m_device->lock();
		m_currentHandle = m_device->play((*m_scenes[m_id])[id]);
		m_currentHandle->setStopCallback(callback, &m_pData);
		m_device->unlock();
	}
	m_id = id;
}

void DynamicMusicPlayer::addTransition(int init, int end, std::shared_ptr<ISound> sound)
{
	if (init != end)
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

void DynamicMusicPlayer::transition(int init, int end)
{
	std::condition_variable condition;
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);
	auto release = [](void* condition){reinterpret_cast<std::condition_variable*>(condition)->notify_all(); };

	m_device->lock();
	m_currentHandle = m_device->play((*m_scenes[init])[end]);
	m_currentHandle->setStopCallback(release, &condition);
	m_device->unlock();

	condition.wait(lock);

	m_device->lock();
	m_currentHandle = m_device->play((*m_scenes[end])[end]);
	m_device->unlock();
}

AUD_NAMESPACE_END