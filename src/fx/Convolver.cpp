#include "fx/Convolver.h"

#include <math.h>
#include <algorithm>
#include <cstring>

AUD_NAMESPACE_BEGIN
Convolver::Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int irLength, int nThreads, bool measure) :
	Convolver(ir, FIXED_N/2, FIXED_N/2, FIXED_N, irLength, nThreads, measure)
{
}

Convolver::Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int M, int L, int N, int irLength, int nThreads, bool measure) :
	m_M(M), m_L(L), m_N(N), m_irBuffers(ir), m_irLength(irLength), m_inLength(0), m_readPosition(0), m_writePosition(0), m_soundEnded(false), m_maxThreads(nThreads), m_numThreads(std::min(m_maxThreads, (int)m_irBuffers->size() - 1)), m_mutexes(m_numThreads), m_conditions(m_numThreads)
{
	m_resetFlag = false;
	m_stopFlag = false;
	for (int i = 0; i < m_irBuffers->size(); i++)
	{
		m_fftConvolvers.push_back(std::unique_ptr<FFTConvolver>(new FFTConvolver((*m_irBuffers)[i], M, L, N, measure)));
		m_fftOutBuffers.push_back((sample_t*)std::malloc(m_L * sizeof(sample_t)));
	}

	m_bufLength = std::ceil((float)irLength / (float)m_L)*m_L * 2;
	m_endPosition = m_bufLength;
	m_outBuffer = (sample_t*)std::calloc(m_bufLength, sizeof(sample_t));
	m_inBuffer = (sample_t*)std::malloc(m_L*sizeof(sample_t));

	for (int i = 0; i < m_numThreads; i++)
		m_threads.push_back(std::thread(&Convolver::threadFunction, this, i));
}

Convolver::~Convolver()
{
	m_stopFlag = true;
	m_resetFlag = true;
	for (int i = 0; i < m_threads.size(); i++)
	{
		m_mutexes[i].lock();
		m_conditions[i].notify_all();
		m_mutexes[i].unlock();
		if (m_threads[i].joinable())
			m_threads[i].join();
	}
	std::free(m_outBuffer);
	std::free(m_inBuffer);
	for (auto buf : m_fftOutBuffers)
		std::free(buf);
}

void Convolver::getNext(sample_t* buffer, int& length)
{
	if (length > m_L || m_soundEnded)
	{
		length = 0;
		return;
	}

	m_fftConvolvers[0]->getNext(buffer, m_fftOutBuffers[0], length);
	for (int i = 0; i < m_threads.size(); i++)
		std::lock_guard<std::mutex> lck(m_mutexes[i]);
	
	m_writePosition += m_inLength;
	if (m_writePosition > m_bufLength - length)
	{
		m_writePosition = 0;
		std::memset(m_outBuffer + (m_bufLength / 2), 0, (m_bufLength / 2)*sizeof(sample_t));
	}
	else
		if (m_writePosition == m_bufLength / 2)
			std::memset(m_outBuffer, 0, (m_bufLength / 2)*sizeof(sample_t));

	m_inLength = length;
	std::memcpy(m_inBuffer, buffer, length*sizeof(sample_t));
	for (int i = 0; i < m_threads.size(); i++)
		m_conditions[i].notify_all();
	
	for (int i = 0; i < length; i++)
		m_outBuffer[i + m_writePosition] += m_fftOutBuffers[0][i];

	int nElem = length;
	m_readPosition = m_writePosition;
	std::memcpy(buffer, m_outBuffer + m_writePosition, nElem*sizeof(sample_t));
}

void Convolver::endSound()
{
	if (m_soundEnded)
		return;

	for (int i = 0; i < m_numThreads; i++)
		std::lock_guard<std::mutex> lck(m_mutexes[i]);

	m_soundEnded = true;
	m_endPosition = m_readPosition + m_inLength + m_irLength - 1;
	int length = m_M;
	bool eos = false;
	int pos = m_writePosition;
	pos += m_inLength;

	for (int i = 0; i < m_fftConvolvers.size(); i++)
	{
		m_fftConvolvers[i]->getTail(length, eos, m_fftOutBuffers[0]);
		length = m_M;
		if(eos)
			m_fftConvolvers[i]->clearTail();

		int delay = i*m_M;
		int position = 0;
		for (int j = 0; j < m_inLength; j++)
		{
			position = j + pos + delay;
			if (position >= m_bufLength)
				position -= m_bufLength;
			m_outBuffer[position] += m_fftOutBuffers[0][j];
		}
	}
}

void Convolver::getRest(int& length, bool& eos, sample_t* buffer)
{
	if (length <= 0)
	{
		length = 0;
		return;
	}

	if (!m_soundEnded)
		endSound();

	for (int i = 0; i < m_numThreads; i++)
		std::lock_guard<std::mutex> lck(m_mutexes[i]);

	m_readPosition += m_inLength;	

	if (m_readPosition + length > m_endPosition)
	{
		length = m_endPosition - m_readPosition;
		if (length < 0)
			length = 0;
		eos = true;
		m_readPosition = m_endPosition;
	}
	else
		m_inLength = length;

	int pos = m_readPosition;
	if (pos >= m_bufLength)
		pos -= m_bufLength;
	std::memcpy(buffer, m_outBuffer + pos, length*sizeof(sample_t));
}

void Convolver::reset()
{
	m_resetFlag = true;
	for (int i = 0; i < m_threads.size(); i++)
		std::lock_guard<std::mutex> lck(m_mutexes[i]);

	for (int i = 0; i < m_fftConvolvers.size(); i++)
		m_fftConvolvers[i]->clearTail();
	m_inLength = 0;
	m_readPosition = 0;
	m_writePosition = 0;
	m_endPosition = m_bufLength;
	std::memset(m_outBuffer, 0, m_bufLength*sizeof(sample_t));
	m_soundEnded = false;
	m_resetFlag = false;
}

void Convolver::threadFunction(int id)
{
	std::unique_lock<std::mutex> lck(m_mutexes[id]);
	while (!m_stopFlag)
	{
		m_conditions[id].wait(lck);
		processSignalFragment(id);
	}
}

void Convolver:: processSignalFragment(int id)
{
	int total = m_irBuffers->size();
	int share = std::ceil(((float)total - 1) / (float)m_numThreads);
	int start = id*share + 1;
	int end = std::min(start + share, total);

	for (int i = start; i < end && !m_resetFlag; i++)
	{
		m_fftConvolvers[i]->getNext(m_inBuffer, m_fftOutBuffers[id + 1], m_inLength);
		
		int delay = i*m_M;
		int position = 0;
		for (int j = 0; j < m_inLength; j++)
		{
			position = j + m_writePosition + delay;
			if (position >= m_bufLength)
				position -= m_bufLength;
			m_outBuffer[position] += m_fftOutBuffers[id + 1][j];
		}
	}
}
AUD_NAMESPACE_END
