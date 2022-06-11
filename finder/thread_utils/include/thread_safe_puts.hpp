#pragma once

#include <mutex>
#include <iostream>

namespace finder::thread_utils
{

extern std::recursive_mutex io_mx;

inline void thread_safe_puts() {}

template <typename Arg, typename ...Args>
void thread_safe_puts(const Arg& arg, Args... args)
{
    std::unique_lock lock{ io_mx };
    std::cout << arg;
    thread_safe_puts(args...);
}

}