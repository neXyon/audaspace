#pragma once

/**
* @file Barrier.h
* @ingroup util
* The Barrier class.
*/

#include "Audaspace.h"

#include <mutex>
#include <condition_variable>

AUD_NAMESPACE_BEGIN
/**
* This class allows to convolve a sound with a very large impulse response.
*/
class Barrier 
{
private:
	std::mutex m_mutex;
	std::condition_variable m_condition;
	int m_threshold;
	int m_count;
	int m_generation;

public:
	Barrier(int count);
	void wait();
};
AUD_NAMESPACE_END