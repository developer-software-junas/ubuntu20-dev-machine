/*******************************************************************
   FILE:                threadpool.h
   DESCRIPTION:         Thread for CSV file export creation
 *******************************************************************/
#ifndef __CYTHREAD_H
#define __CYTHREAD_H
///////////////////////////////////////////
//  Standard headers
#include <string.h>
#include <string>
#include <sstream>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>
///////////////////////////////////////////
//  User header file(s)
#include "cyw_aes.h"
#include "cyw_log.h"
#include "cyw_ini.h"
#include "cyw_cgi.h"
#include "cyw_util.h"
/***************************************************************************
         CLASS:         CYCommandLine
   DESCRIPTION:         Command line class
 ***************************************************************************/
class ThreadPool
{
public:
    ThreadPool();
    ThreadPool(const ThreadPool& rhs) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;
    ThreadPool(ThreadPool&& rhs);
    ThreadPool& operator=(ThreadPool&& rhs);
    ~ThreadPool();
    template<typename Func, typename... Args>
    auto Add(Func&& func, Args&&... args) -> std::future<typename std::result_of<Func(Args...)>::type>;
    size_t GetWaitingJobs() const;
    void Clear();
    void Pause(const bool& state);
    // blocks calling thread until job queue is empty
    void Wait();
    bool IsPaused() const { return m_paused; }
    bool IsTerminate() const { return m_terminate; }
    bool IsJobsEmpty() const { return m_jobs.empty(); }

private:
    using Job = std::function<void()>;
    // variables
    std::queue<Job> m_jobs;
    std::mutex m_wait_mutex;
    mutable std::mutex m_jobs_mutex;
    std::condition_variable m_threads_done;
    std::condition_variable m_jobs_available;   // notification variable for waiting threads
    std::vector<std::thread> m_threads;
    std::atomic<size_t> m_threads_waiting{ 0 };
    std::atomic<bool> m_terminate{ false };
    std::atomic<bool> m_paused{ false };
    // methods
    static void ThreadTask(ThreadPool* pPool);  // function each thread performs
};
#endif
