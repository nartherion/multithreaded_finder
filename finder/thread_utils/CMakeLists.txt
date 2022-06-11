add_library(ThreadUtils 
    src/thread_pool.cpp
    src/thread_safe_puts.cpp
    include/thread_pool.hpp 
    include/thread_safe_puts.hpp 
    include/thread_safe_queue.hpp
    )

target_include_directories(ThreadUtils PUBLIC 
    include/
    )

target_link_libraries(ThreadUtils PUBLIC 
    Common
    )