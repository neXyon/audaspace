#include "fx/MutableReader.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

MutableReader::MutableReader(std::vector<std::shared_ptr<ISound>>& sounds, bool random, int index) :
m_sounds(sounds), m_random(random), m_index(index)
{
	m_reader = m_sounds[m_index]->createReader();
}

bool MutableReader::isSeekable() const
{
	return m_reader->isSeekable();
}

void MutableReader::seek(int position)
{
	if (position == 0)
	{
		if (m_sounds.size() > 0)
		{
			if (!m_random){
				m_index++;
				if (m_index >= m_sounds.size())
					m_index = 0;
			}
			else
			{
				int temp;
				do{
					temp = rand() % m_sounds.size();
				} while (temp == m_index);
				m_index = temp;
			}
		}
		m_reader = m_sounds[m_index]->createReader();
	}
	else
		m_reader->seek(position);
}

int MutableReader::getLength() const
{
	return m_reader->getLength();
}

int MutableReader::getPosition() const
{
	return m_reader->getPosition();
}

Specs MutableReader::getSpecs() const
{
	return m_reader->getSpecs();
}

void MutableReader::read(int& length, bool& eos, sample_t* buffer)
{
	m_reader->read(length, eos, buffer);
}

AUD_NAMESPACE_END
