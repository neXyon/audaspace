#include "fx/BinauralSound.h"
#include "fx/BinauralReader.h"
#include "Exception.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

BinauralSound::BinauralSound(std::shared_ptr<ISound> sound, std::shared_ptr<HRTF> hrtfs, std::shared_ptr<Source> source, std::shared_ptr<ThreadPool> threadPool) :
	BinauralSound(sound, hrtfs, source, threadPool, std::make_shared<FFTPlan>(false))
{
}

BinauralSound::BinauralSound(std::shared_ptr<ISound> sound, std::shared_ptr<HRTF> hrtfs, std::shared_ptr<Source> source, std::shared_ptr<ThreadPool> threadPool, std::shared_ptr<FFTPlan> plan) :
	m_sound(sound), m_hrtfs(hrtfs), m_source(source), m_threadPool(threadPool), m_plan(plan)
{
}

std::shared_ptr<IReader> BinauralSound::createReader()
{
	return std::make_shared<BinauralReader>(m_sound->createReader(), m_hrtfs, m_source, m_threadPool, m_plan);
}

std::shared_ptr<HRTF> BinauralSound::getHRTFs()
{
	return m_hrtfs;
}

void BinauralSound::setHRTFs(std::shared_ptr<HRTF> hrtfs)
{
	m_hrtfs = hrtfs;
}

std::shared_ptr<Source> BinauralSound::getSource()
{
	return m_source;
}

void BinauralSound::setSource(std::shared_ptr<Source> source)
{
	m_source = source;
}

AUD_NAMESPACE_END