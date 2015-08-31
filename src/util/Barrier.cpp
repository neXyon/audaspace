#include "util/Barrier.h"

AUD_NAMESPACE_BEGIN
Barrier::Barrier(unsigned int count) :
	m_threshold(count), m_count(count), m_generation(0)
{
}

void Barrier::wait() 
{
	std::unique_lock<std::mutex> lck(m_mutex);
	int gen = m_generation;
	if (!--m_count) 
	{
		m_count = m_threshold;
		m_generation++;
		m_condition.notify_all();
	}
	else
		m_condition.wait(lck, [this, gen] { return gen != m_generation; });
}
AUD_NAMESPACE_END