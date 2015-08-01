#include "fx/VolumeReader.h"

#include <cstring>

AUD_NAMESPACE_BEGIN

VolumeReader::VolumeReader(std::shared_ptr<IReader> reader, std::shared_ptr<VolumeStorage> volumeStorage) :
	m_reader(reader), m_volumeStorage(volumeStorage)
{
}

bool VolumeReader::isSeekable() const
{
	return m_reader->isSeekable();
}

void VolumeReader::seek(int position)
{
	m_reader->seek(position);
}

int VolumeReader::getLength() const
{
	return m_reader->getLength();
}

int VolumeReader::getPosition() const
{
	return m_reader->getPosition();
}

Specs VolumeReader::getSpecs() const
{
	return m_reader->getSpecs();
}

void VolumeReader::read(int& length, bool& eos, sample_t* buffer)
{
	m_reader->read(length, eos, buffer);
	for (int i = 0; i < length * m_reader->getSpecs().channels; i++)
		buffer[i] = buffer[i] * m_volumeStorage->getVolume;
}

AUD_NAMESPACE_END