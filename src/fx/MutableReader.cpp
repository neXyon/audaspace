#include "fx/MutableReader.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

MutableReader::MutableReader(std::shared_ptr<ISound> sound) :
m_sound(sound)
{
	m_reader = m_sound->createReader();
}

bool MutableReader::isSeekable() const
{
	return m_reader->isSeekable();
}

void MutableReader::seek(int position)
{
	if (position < m_reader->getPosition())
	{
		m_reader = m_sound->createReader();
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
