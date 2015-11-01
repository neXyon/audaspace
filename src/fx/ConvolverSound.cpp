#include "fx/ConvolverSound.h"
#include "fx/ConvolverReader.h"
#include "Exception.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

ConvolverSound::ConvolverSound(std::shared_ptr<ISound> sound, std::shared_ptr<ImpulseResponse> impulseResponse, std::shared_ptr<ThreadPool> threadPool) :
	ConvolverSound(sound, impulseResponse, threadPool, std::make_shared<FFTPlan>(false))
{
}

ConvolverSound::ConvolverSound(std::shared_ptr<ISound> sound, std::shared_ptr<ImpulseResponse> impulseResponse, std::shared_ptr<ThreadPool> threadPool, std::shared_ptr<FFTPlan> plan) :
	m_sound(sound), m_impulseResponse(impulseResponse), m_threadPool(threadPool), m_plan(plan)
{
}

std::shared_ptr<IReader> ConvolverSound::createReader()
{
	return std::make_shared<ConvolverReader>(m_sound->createReader(), m_impulseResponse, m_threadPool, m_plan);
}

std::shared_ptr<ImpulseResponse> ConvolverSound::getImpulseResponse()
{
	return m_impulseResponse;
}

void ConvolverSound::setImpulseResponse(std::shared_ptr<ImpulseResponse> impulseResponse)
{
	m_impulseResponse = impulseResponse;
}

AUD_NAMESPACE_END