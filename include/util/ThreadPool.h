#pragma once

/**
* @file ThreadPool.h
* @ingroup util
* The ThreadPool class.
*/

#include "Audaspace.h"

#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <queue>
#include <atomic>
#include <future>

AUD_NAMESPACE_BEGIN
/**
* This represents thread pool.
*/
class ThreadPool
{
private:
	/** 
	* A queue of tasks.
	*/
	std::queue<std::function<void()>> m_queue;

	/**
	* A vector of thread objects.
	*/
	std::vector<std::thread> m_threads;

	/**
	* A mutex for synchronization.
	*/
	std::mutex m_mutex;

	/**
	* A condition variable used to stop the threads when there are no tasks.
	*/
	std::condition_variable m_condition;

	/**
	* Stop flag.
	*/
	bool m_stopFlag;


	// delete copy constructor and operator=
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
public:
	/**
	* Creates a new ThreadPool object.
	* \param count The number of threads of the pool. It must not be 0.
	*/
	ThreadPool(unsigned int count);
	virtual ~ThreadPool();

	template<class T, class... Args>
	std::future<typename std::result_of<T(Args...)>::type> enqueue(T&& t, Args&&... args);

private:
	void threadFunction();
};
AUD_NAMESPACE_END