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
* This represents a barrier mechanism for thread sychronization.
*/
class Barrier 
{
private:
	/**
	* A mutex needed to use a condition variable.
	*/
	std::mutex m_mutex;

	/**
	* Condition varieble used to sync threads.
	*/
	std::condition_variable m_condition;
	
	/**
	* Number of threads that need to reach the barrier for it to lift.
	*/
	unsigned int m_threshold;

	/**
	* Conter that count from threshold to 0.
	*/
	unsigned int m_count;

	/**
	* Variable used for predicate check in the condition variable wait.
	*/
	unsigned int m_generation;

	// delete copy constructor and operator=
	Barrier(const Barrier&) = delete;
	Barrier& operator=(const Barrier&) = delete;
public:
	/**
	* Creates a new Barrier object.
	* \param count the number of threads that need to reach the barrier for it to lift.
	*/
	Barrier(unsigned int count);
	virtual ~Barrier();

	/**
	* Makes the caller thread wait until enough threads are stopped by this method.
	*/
	void wait();
};
AUD_NAMESPACE_END