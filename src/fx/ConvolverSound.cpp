#include "fx/ConvolverSound.h"
#include "fx/ConvolverReader.h"
#include "Exception.h"

#include "fftw3.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

ConvolverSound::ConvolverSound(std::shared_ptr<ISound> sound, std::shared_ptr<ImpulseResponse> impulseResponse, std::shared_ptr<ThreadPool> threadPool) :
	m_sound(sound), m_impulseResponse(impulseResponse), m_threadPool(threadPool)
{
}

std::shared_ptr<IReader> ConvolverSound::createReader()
{
	return std::make_shared<ConvolverReader>(m_sound->createReader(), m_impulseResponse, m_threadPool);
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