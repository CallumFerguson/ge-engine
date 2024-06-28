#include "ThreadPool.hpp"

ThreadPool ThreadPool::s_instance;

ThreadPool &ThreadPool::instance() {
    return s_instance;
}

ThreadPool::ThreadPool() {
    const uint32_t num_threads = std::thread::hardware_concurrency();
    for (uint32_t ii = 0; ii < num_threads; ++ii) {
        m_threads.emplace_back(&ThreadPool::threadLoop, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_shouldTerminate = true;
    }
    m_mutexCondition.notify_all();
    for (std::thread &active_thread: m_threads) {
        active_thread.join();
    }
    m_threads.clear();
}

void ThreadPool::threadLoop() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_mutexCondition.wait(lock, [this] {
                return !m_jobs.empty() || m_shouldTerminate;
            });
            if (m_shouldTerminate) {
                return;
            }
            job = m_jobs.front();
            m_jobs.pop();
        }
        job();
    }
}

void ThreadPool::queueJob(std::function<void()> &&job) {
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_jobs.emplace(std::forward<std::function<void()>>(job));
    }
    m_mutexCondition.notify_one();
}

bool ThreadPool::busy() {
    bool poolBusy;
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        poolBusy = !m_jobs.empty();
    }
    return poolBusy;
}

size_t ThreadPool::currentJobCount() {
    size_t count;
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        count = m_jobs.size();
    }
    return count;
}
