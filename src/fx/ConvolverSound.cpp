#include "fx/ConvolverSound.h"
#include "fx/ConvolverReader.h"
#include "Exception.h"

#include "fftw3.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

ConvolverSound::ConvolverSound(std::shared_ptr<ISound> sound, std::shared_ptr<ISound> impulseResponse, int nThreads) :
	m_sound(sound), m_impulseResponse(impulseResponse), m_nThreads(nThreads)
{
}

std::shared_ptr<IReader> ConvolverSound::createReader()
{
	return std::make_shared<ConvolverReader>(m_sound->createReader(), m_impulseResponse->createReader(), m_nThreads);
}

std::shared_ptr<ISound> ConvolverSound::getImpulseResponse()
{
	return m_impulseResponse;
}

void ConvolverSound::setImpulseResponse(std::shared_ptr<ISound> impulseResponse)
{
	m_impulseResponse = impulseResponse;
}

AUD_NAMESPACE_END