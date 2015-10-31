#include "fx/BinauralReader.h"
#include "Exception.h"

#include <cstring>
#include <algorithm>

#define NUM_CONVOLVERS 4

AUD_NAMESPACE_BEGIN
BinauralReader::BinauralReader(std::shared_ptr<IReader> reader, std::shared_ptr<HRTF> hrtfs, std::shared_ptr<Source> source, std::shared_ptr<ThreadPool> threadPool, std::shared_ptr<FFTPlan> plan) :
	m_reader(reader), m_hrtfs(hrtfs), m_source(source), m_N(plan->getSize()), m_threadPool(threadPool)
{
	m_M = m_L = m_N / 2;

	m_RealAzimuth = m_Azimuth = m_source->getAzimuth();
	m_RealElevation = m_Elevation = m_source->getElevation();
	auto ir = m_hrtfs->getImpulseResponse(m_RealAzimuth, m_RealElevation);
	int irLength = ir->getLength();
	for (int i = 0; i < NUM_CONVOLVERS; i++)
		m_convolvers.push_back(std::unique_ptr<Convolver>(new Convolver(ir->getChannel(i % 2), irLength, m_threadPool, plan)));
	m_convolverThreads = std::min((int)threadPool->getNumOfThreads(), NUM_CONVOLVERS);
	m_futures.reserve(m_convolverThreads);
}

BinauralReader::~BinauralReader()
{

}

bool BinauralReader::isSeekable() const
{
	return m_reader->isSeekable();
}

void BinauralReader::seek(int position)
{

}

int BinauralReader::getLength() const
{
	return m_reader->getLength();
}

int BinauralReader::getPosition() const
{
	return m_reader->getPosition();
}

Specs BinauralReader::getSpecs() const
{
	return m_reader->getSpecs();
}

void BinauralReader::read(int& length, bool& eos, sample_t* buffer)
{

}

void BinauralReader::loadBuffer()
{

}

void BinauralReader::joinByChannel(int start, int len)
{

}

int BinauralReader::threadFunction(int id, bool input)
{
	return 0;
}

AUD_NAMESPACE_END