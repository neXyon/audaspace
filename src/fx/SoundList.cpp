#include "fx/SoundList.h"
#include "Exception.h"

#include <cstring>
#include <chrono>

AUD_NAMESPACE_BEGIN

SoundList::SoundList()
{
	srand(time(NULL));
}


std::shared_ptr<IReader> SoundList::createReader()
{
	if (m_list.size() > 0)
	{
		if (!m_random){
			m_index++;
			if (m_index >= m_list.size())
				m_index = 0;
		}
		else
		{
			int temp;
			do{
				temp = rand() % m_list.size();
			} while (temp == m_index);
			m_index = temp;
		}

		return m_list[m_index]->createReader();
	}
	else
		AUD_THROW(FileException, "The sound list is empty");
}

int SoundList::addSound(std::shared_ptr<ISound> sound)
{
	m_list.push_back(sound);
	return m_list.size() - 1;
}

void SoundList::removeSound(int index)
{
	m_list.erase(m_list.begin() + index);
	if (m_index >= index)
		m_index--;
}

void SoundList::setRandomMode(bool random)
{
	m_random = random;
}

bool SoundList::getRandomMode()
{
	return m_random;
}

AUD_NAMESPACE_END