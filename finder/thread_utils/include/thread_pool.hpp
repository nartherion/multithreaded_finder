#pragma once

#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>

#include "noncopyable.hpp"
#include "thread_safe_queue.hpp"

namespace finder::thread_utils
{

class thread_pool : public finder::noncopyable
{
    struct synchro_tool
    {
        std::mutex mx;
        std::condition_variable cv;
    };
    using task_type = std::function<void()>;
private:
    std::vector<std::thread>     m_threads;
    thread_safe_queue<task_type> m_pending;
    synchro_tool                 m_tasks_st;
    synchro_tool                 m_work_st;
    std::atomic_bool             m_started_work;
    std::atomic_bool             m_work_done;    
public:
    thread_pool(size_t);
    void start();
    void stop();
    void push(task_type&&);
    void clear();
    void set_work_done();
    bool is_work_done();
    ~thread_pool();
private:
    void push_stop_task();
};

}