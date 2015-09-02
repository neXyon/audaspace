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
* This represents pool of threads.
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

	/**
	* The number fo threads.
	*/
	unsigned int m_numThreads;

	// delete copy constructor and operator=
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
public:
	/**
	* Creates a new ThreadPool object.
	* \param count The number of threads of the pool. It must not be 0.
	*/
	ThreadPool(unsigned int count) :
		m_stopFlag(false), m_numThreads(count)
	{
		for (unsigned int i = 0; i < count; i++)
			m_threads.emplace_back(&ThreadPool::threadFunction, this);
	}
	virtual ~ThreadPool()
	{
		m_mutex.lock();
		m_stopFlag = true;
		m_mutex.unlock();
		m_condition.notify_all();
		for (unsigned int i = 0; i < m_threads.size(); i++)
			m_threads[i].join();
	}

	/**
	* Enqueues a new task for the threads to realize.
	* \param t A function that realices a task.
	* \param args The arguments of the task.
	* \return A future of the same type as the return type of the task.
	*/
	template<class T, class... Args>
	std::future<typename std::result_of<T(Args...)>::type> enqueue(T&& t, Args&&... args)
	{
		using pkgdTask = std::packaged_task<typename std::result_of<T(Args...)>::type()>;

		std::shared_ptr<pkgdTask> task = std::make_shared<pkgdTask>(std::bind(std::forward<T>(t), std::forward<Args>(args)...));
		auto result = task->get_future();

		m_mutex.lock();
		m_queue.emplace([task]() { (*task)(); });
		m_mutex.unlock();

		m_condition.notify_one();
		return result;
	}

	/**
	* Retrieves the number of threads of the pool.
	* \return The number of threads.
	*/
	unsigned int getNumOfThreads()
	{
		return m_numThreads;
	}

private:
	void threadFunction()
	{
		while (true)
		{
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_condition.wait(lock, [this] { return m_stopFlag || !m_queue.empty(); });
				if (m_stopFlag && m_queue.empty())
					return;
				task = std::move(m_queue.front());
				this->m_queue.pop();
			}
			task();
		}
	}
};
AUD_NAMESPACE_END