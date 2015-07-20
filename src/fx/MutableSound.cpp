#include "fx/MutableSound.h"
#include "fx/MutableReader.h"
#include "Exception.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

MutableSound::MutableSound(std::shared_ptr<ISound> sound) : 
m_sound(sound)
{
}

std::shared_ptr<IReader> MutableSound::createReader()
{
	return std::make_shared<MutableReader>(m_sound);
}

AUD_NAMESPACE_END