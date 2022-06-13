#pragma once

#include <queue>
#include <mutex>

#include "noncopyable.hpp"

namespace finder::thread_utils
{

template <typename T>
class thread_safe_queue : public finder::noncopyable
{
    std::queue<T> raw_queue;
    mutable std::mutex queue_mx;
public:
    thread_safe_queue() = default;
    thread_safe_queue(thread_safe_queue&&) noexcept;
    thread_safe_queue& operator = (thread_safe_queue&&) noexcept;
public:
    void     push(const T&);
	void     push(T&&);
    void     pop();
    void     clear();
    T&       front();
    const T& front() const;
    bool     empty() const;
	size_t   size()  const;
};

// Implementation follows

template <typename T>
thread_safe_queue<T>::thread_safe_queue(thread_safe_queue&& rhs) noexcept : raw_queue{ std::move(rhs.raw_queue) } 
{}

template <typename T>
thread_safe_queue<T>& thread_safe_queue<T>::operator = (thread_safe_queue&& rhs) noexcept
{
	std::swap(raw_queue, rhs.raw_queue);
	return *this;
}

template <typename T>
void thread_safe_queue<T>::push(T&& object)
{
	std::unique_lock lock{ queue_mx };
	raw_queue.push(std::move(object));
}

template <typename T>
void thread_safe_queue<T>::push(const T& object)
{
	std::unique_lock lock{ queue_mx };
	raw_queue.push(object);
}

template <typename T>
void thread_safe_queue<T>::pop()
{
	std::unique_lock lock{ queue_mx };
	raw_queue.pop();
}

template <typename T>
const T& thread_safe_queue<T>::front() const
{
	std::unique_lock lock{ queue_mx };
	return raw_queue.front();
}

template <typename T>
T& thread_safe_queue<T>::front()
{
	std::unique_lock lock{ queue_mx };
	return raw_queue.front();
}

template <typename T>
bool thread_safe_queue<T>::empty() const
{
	std::unique_lock lock{ queue_mx };
	return raw_queue.empty();
}

template <typename T>
size_t thread_safe_queue<T>::size() const
{
	std::unique_lock lock{ queue_mx };
	return raw_queue.size();
}

template <typename T>
void thread_safe_queue<T>::clear()
{
	thread_safe_queue<T> t;
	std::swap(*this, t);
}

}