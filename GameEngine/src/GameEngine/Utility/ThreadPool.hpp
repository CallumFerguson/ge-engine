#pragma once

// from: https://stackoverflow.com/questions/15752659/thread-pooling-in-c11

#include <queue>
#include <mutex>
#include <thread>

class ThreadPool {
public:
    static ThreadPool &instance();

    void queueJob(const std::function<void()> &job);

    bool busy();

    size_t currentJobCount();

private:
    static ThreadPool s_instance;

    ThreadPool();

    ~ThreadPool();

    void threadLoop();

    bool m_shouldTerminate = false;
    std::mutex m_queueMutex;
    std::condition_variable m_mutexCondition;
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_jobs;
};
