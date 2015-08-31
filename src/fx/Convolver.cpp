#include "fx/Convolver.h"

#include <math.h>
#include <algorithm>
#include <cstring>
#include <iostream>

AUD_NAMESPACE_BEGIN
Convolver::Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int irLength, int nThreads, bool measure) :
	Convolver(ir, FIXED_N/2, FIXED_N/2, FIXED_N, irLength, nThreads, measure)
{
}

Convolver::Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int M, int L, int N, int irLength, int nThreads, bool measure) :
	m_M(M), m_L(L), m_N(N), m_irBuffers(ir), m_irLength(irLength), m_inLength(0), m_readPosition(0), m_writePosition(0), m_soundEnded(false), m_maxThreads(nThreads), m_numThreads(std::min(m_maxThreads, (int)m_irBuffers->size() - 1)), m_mutexes(m_numThreads), m_conditions(m_numThreads), m_tailCounter(0)
{
	m_resetFlag = false;
	m_stopFlag = false;
	for (int i = 0; i < m_irBuffers->size(); i++)
	{
		m_fftConvolvers.push_back(std::unique_ptr<FFTConvolver>(new FFTConvolver((*m_irBuffers)[i], M, L, N, measure)));
		m_delayLine.push_front((fftwf_complex*)std::calloc((m_N / 2) + 1, sizeof(fftwf_complex)));
	}

	m_bufLength = std::ceil((float)irLength / (float)m_L)*m_L * 2;
	m_endPosition = m_bufLength;
	m_accBuffer = (fftwf_complex*)std::calloc((m_N / 2) + 1, sizeof(fftwf_complex));

	for (int i = 0; i < m_numThreads; i++)
	{
		m_threads.push_back(std::thread(&Convolver::threadFunction, this, i));
		m_threadAccBuffers.push_back((fftwf_complex*)std::calloc((m_N/2)+1, sizeof(fftwf_complex)));
	}
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
	std::free(m_accBuffer);
	for (auto buf : m_threadAccBuffers)
		std::free(buf);
	while (!m_delayLine.empty())
	{
		std::free(m_delayLine.front());
		m_delayLine.pop_front();
	}
}

void Convolver::getNext(sample_t* inBuffer, sample_t* outBuffer, int& length, bool& eos)
{
	if (length > m_L || m_soundEnded)
	{
		length = 0;
		eos = m_tailCounter >= m_delayLine.size();
		return;
	}

	eos = false;
	for (int i = 0; i < m_threads.size(); i++)
		std::lock_guard<std::mutex> lck(m_mutexes[i]);
	
	if (inBuffer != nullptr)
		m_fftConvolvers[0]->getNextFDL(inBuffer, m_accBuffer, length, m_delayLine[0]);
	else
	{
		m_tailCounter++;
		std::memset(m_delayLine[0], 0, ((m_N / 2) + 1)*sizeof(fftwf_complex));
	}
	m_delayLine.push_front(m_delayLine.back());
	m_delayLine.pop_back();
	m_fftConvolvers[0]->IFFT_FDL(m_accBuffer, outBuffer, length);
	std::memset(m_accBuffer, 0, ((m_N / 2) + 1)*sizeof(fftwf_complex));

	if (m_tailCounter >= m_delayLine.size() - 1 && inBuffer == nullptr)
	{
		eos = true;
		length = m_irLength%m_M;
		if (m_tailCounter > m_delayLine.size()-1)
			length = 0;
	}
	else
		for (int i = 0; i < m_threads.size(); i++)
			m_conditions[i].notify_all();
}

void Convolver::endSound()
{
	/*if (m_soundEnded)
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
			m_fftConvolvers[i]->clear();

		int delay = i*m_M;
		int position = 0;
		for (int j = 0; j < m_inLength; j++)
		{
			position = j + pos + delay;
			if (position >= m_bufLength)
				position -= m_bufLength;
			m_outBuffer[position] += m_fftOutBuffers[0][j];
		}
	}*/
}

void Convolver::getRest(int& length, bool& eos, sample_t* buffer)
{
	/*if (length <= 0)
	{
		length = 0;
		eos = m_readPosition >= m_endPosition;
		return;
	}

	eos = false;
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
	std::memcpy(buffer, m_outBuffer + pos, length*sizeof(sample_t));*/
}

void Convolver::reset()
{
	m_resetFlag = true;
	for (int i = 0; i < m_threads.size(); i++)
		std::lock_guard<std::mutex> lck(m_mutexes[i]);

	for (int i = 0; i < m_fftConvolvers.size(); i++)
		m_fftConvolvers[i]->clear();
	m_tailCounter = 0;
	m_endPosition = m_bufLength;
	std::memset(m_accBuffer, 0, ((m_N / 2) + 1)*sizeof(fftwf_complex));

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
	std::memset(m_threadAccBuffers[id], 0, ((m_N / 2) + 1)*sizeof(fftwf_complex));

	for (int i = start; i < end && !m_resetFlag; i++)
		m_fftConvolvers[i]->getNextFDL(m_delayLine[i], m_threadAccBuffers[id]);

	m_sumMutex.lock();
	for (int i = 0; (i < m_N / 2 + 1) && !m_resetFlag; i++)
	{
		m_accBuffer[i][0] += m_threadAccBuffers[id][i][0];
		m_accBuffer[i][1] += m_threadAccBuffers[id][i][1];
	}
	m_sumMutex.unlock();
}
AUD_NAMESPACE_END
