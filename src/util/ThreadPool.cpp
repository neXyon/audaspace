#include "util/ThreadPool.h"

AUD_NAMESPACE_BEGIN
ThreadPool::ThreadPool(unsigned int count) :
	m_stopFlag(false)
{
	for (unsigned int i = 0; i < count; i++)
		m_threads.emplace_back(&ThreadPool::threadFunction, this);
}

ThreadPool::~ThreadPool()
{
	m_mutex.lock();
	m_stopFlag = true;
	m_mutex.unlock();
	m_condition.notify_all();
	for (unsigned int i = 0; i < m_threads.size(); i++)
		m_threads[i].join();
}

template<class T, class... Args>
std::future<typename std::result_of<T(Args...)>::type> ThreadPool::enqueue(T&& t, Args&&... args)
{
	using pkgdTask = std::packaged_task<typename std::result_of<F(Args...)>::type()>;

	std::shared_ptr<pkgdTask> task = std::make_shared<pkgdTask>(std::bind(std::forward<T>(t), std::forward<Args>(args)...));
	std::future<return_type> res = task->get_future();

	m_mutex->lock();
	tasks.emplace([task]() { (*task)(); });
	m_mutex->unlock();

	condition.notify_one();
	return res;
}

void ThreadPool::threadFunction()
{
	while(true)
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
AUD_NAMESPACE_END