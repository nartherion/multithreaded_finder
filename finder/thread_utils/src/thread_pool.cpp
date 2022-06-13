#include "thread_pool.hpp"
#include "thread_safe_puts.hpp"

namespace finder::thread_utils
{

thread_pool::thread_pool(size_t pool_size) : m_work_done{ false }, m_started_work{ false }
{
    for (int i = 0; i < pool_size; ++i)
    {
        m_threads.emplace_back(std::thread(
            [this] () 
            {
                {
                    std::unique_lock lock{ m_work_st.mx };
                    m_work_st.cv.wait(lock, [this] { return m_started_work.load(); });
                }
                while (true)
                {
                    if (m_work_done.load())
                    {
                        thread_safe_puts("[INFO] thread #", m_threads.back().get_id(), 
                            " : returned, as the work is done", '\n');
                        return;
                    }
                    task_type task;
                    {
                        std::unique_lock lock{ m_tasks_st.mx };
                        thread_safe_puts("[WARNING] thread #", std::this_thread::get_id(), 
                            " : started waiting for tasks | waiting duration : 5s", '\n');
                        if (m_tasks_st.cv.wait_for(lock, std::chrono::seconds(5), [&] { return !m_pending.empty(); }))
                        {
                            task = std::move(m_pending.front());
                            m_pending.pop();
                        }
                        else
                        {
                            set_work_done();
                            continue;
                        }                            
                    }
                    if (!task)
                    {
                        thread_safe_puts("[WARNING] thread #", std::this_thread::get_id(), " : got STOP task", '\n');
                        push_stop_task();
                        return;
                    }
                    thread_safe_puts("[INFO] thread #", std::this_thread::get_id(), 
                        " : started doing some task", '\n');
					task();
					thread_safe_puts("[INFO] thread #", std::this_thread::get_id(), 
                        " : finished doing some task", '\n');
                }
            }
        ));
        thread_safe_puts("[INFO] thread #", m_threads.back().get_id(), " : started", '\n');
    }
} 

void thread_pool::start()
{
    if (m_started_work) return;
    m_started_work = true;
    m_work_st.cv.notify_all();
}

void thread_pool::stop()
{
    push_stop_task();
    for (auto& thread : m_threads)
        if (thread.joinable())
        {
            thread_safe_puts("[INFO] thread #", thread.get_id(), " : joined", '\n');
            thread.join();
        }
    clear();
    thread_safe_puts("[INFO] thread_pool : work done & threads joined & tasks queue emptied", '\n');
}

void thread_pool::push(task_type&& task)
{
    if (!task) 
        thread_safe_puts("[WARNING] thread #", std::this_thread::get_id(), " : task is not valid!", '\n');
    m_pending.push(std::move(task));
    m_tasks_st.cv.notify_one();
}

void thread_pool::push_stop_task()
{
    push(task_type{});
    thread_safe_puts("[WARNING] thread #", std::this_thread::get_id(), " : pushed STOP task", '\n');
}

void thread_pool::clear()
{
    thread_safe_queue<task_type> t;
    std::swap(t, m_pending);
}

void thread_pool::set_work_done() { m_work_done.store(true); }

bool thread_pool::is_work_done() { return m_work_done.load(); }

thread_pool::~thread_pool() { stop(); }

}