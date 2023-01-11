#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <assert.h>

class ThreadPool
{
public:
    explicit ThreadPool(size_t threadCount = 8);

    ~ThreadPool();

    ThreadPool(const ThreadPool& other) = delete;

    ThreadPool& operator = (const ThreadPool& other) = delete;

    template<class T>
    void addTask(T&& task);

private:
    void runInThread();

    std::vector<std::thread> workers;

    std::queue<std::function<void()>> tasks;

    std::mutex mtx;

    std::condition_variable cond;

    bool isClosed;
};

inline ThreadPool::ThreadPool(size_t threadCount)
{
    assert(threadCount > 0);
    isClosed = false;
    for (size_t i = 0; i < threadCount; i++)
    {
        workers.emplace_back(
            std::thread(std::bind(&ThreadPool::runInThread, this))
        );
    }
}

inline ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        isClosed = true;
        cond.notify_all();
    }
    for (auto& thr : workers)
    {
        thr.join();
    }
}

inline void ThreadPool::runInThread()
{
    while (!isClosed)
    {
        std::unique_lock<std::mutex> lock(mtx);
        while (tasks.empty() && !isClosed)
        {
            cond.wait(lock);
        }
        if (!tasks.empty())
        {
            auto task = std::move(tasks.front());
            tasks.pop();
            lock.unlock();
            task();
        }
    }
}

template<class T>
inline void ThreadPool::addTask(T&& task)
{
    if (workers.size() == 0)
    {
        task();
    }
    if (!isClosed)
    {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.emplace(std::forward<T>(task));
        cond.notify_one();
    }
}

#endif //THREADPOOL_H