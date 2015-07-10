#include "fx/MutableSound.h"
#include "fx/MutableReader.h"
#include "Exception.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

MutableSound::MutableSound() : 
m_sound(std::make_shared<SoundList>())
{
}

MutableSound::MutableSound(std::vector<std::shared_ptr<ISound>>& list) : 
m_sound(std::make_shared<SoundList>(list))
{
}

std::shared_ptr<IReader> MutableSound::createReader()
{
	return std::make_shared<MutableReader>(m_sound);
}

int MutableSound::addSound(std::shared_ptr<ISound> sound)
{
	return m_sound->addSound(sound);
}

void MutableSound::removeSound(int index)
{
	m_sound->removeSound(index);
}

void MutableSound::setRandomMode(bool random)
{
	m_sound->setRandomMode(random);
}

bool MutableSound::getRandomMode()
{
	return m_sound->getRandomMode();
}

int MutableSound::getSize()
{
	return m_sound->getSize();
}
AUD_NAMESPACE_END