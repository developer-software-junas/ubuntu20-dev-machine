/********************************************************************
          FILE:         threadpool.cpp
   DESCRIPTION:         MYSQL export thread implementation
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include "threadpool.h"
#include <algorithm>
using std::string;
using std::size_t;
using std::stringstream;

ThreadPool::ThreadPool()
{
    const size_t threadCount = std::thread::hardware_concurrency();

    m_threads.reserve(threadCount);
    std::generate_n(
                    std::back_inserter(m_threads),
                    threadCount,
                    [this]()
                    {
                        return std::thread{ ThreadPool::ThreadTask, this };
                    });
}

ThreadPool::~ThreadPool()
{
    Clear();

    m_terminate.store(true);
    m_jobs_available.notify_all();

    for (auto& t : m_threads)
    {
        if (t.joinable())
            t.join();
    }
}

size_t ThreadPool::GetWaitingJobs() const
{
    std::lock_guard<std::mutex> lock{ m_jobs_mutex };

    return m_jobs.size();
}

void ThreadPool::Clear()
{
    std::lock_guard<std::mutex> lock{ m_jobs_mutex };

    while (! m_jobs.empty())
        m_jobs.pop();
}

void ThreadPool::Pause(const bool& state)
{
    m_paused = state;

    if (! m_paused)
        m_jobs_available.notify_all();
}
#include <iostream>
void ThreadPool::Wait()
{
    // we're done waiting once all threads are waiting
/*//////////////////////////////////////////////////////////////
    while (m_threads_waiting != m_threads.size());
/*//////////////////////////////////////////////////////////////
    std::cout << "\nbefore lock\n";
    std::unique_lock<std::mutex> lock{ m_wait_mutex };
    std::cout << "\nafter lock\n";
    m_threads_done.wait(lock, [&, this]()
                            {
                                return m_threads_waiting <= m_threads.size();
                            });
    lock.unlock();
///*//////////////////////////////////////////////////////////////
    std::cout << "\nafter un-lock\n";
}
// function each thread performs
void ThreadPool::ThreadTask(ThreadPool* pPool)
{
    // loop until we break (to keep thread alive)
    while (true)
    {
        if (pPool->m_terminate)
            break;

        std::unique_lock<std::mutex> lock{ pPool->m_jobs_mutex };

        // if there are no more jobs, or we're paused, go into waiting mode
        if (pPool->m_jobs.empty() || pPool->m_paused)
        {
            ++pPool->m_threads_waiting;

            pPool->m_jobs_available.wait(lock, [&]()
                                            {
                                                return pPool->IsTerminate() ||
                                                    ! (pPool->IsJobsEmpty() || pPool->IsPaused());
                                            });

            --pPool->m_threads_waiting;
        }

        if (pPool->m_terminate)
            break;

        auto job = std::move(pPool->m_jobs.front());
        pPool->m_jobs.pop();
        lock.unlock();
        job();
    }
}
