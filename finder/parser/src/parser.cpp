#include "parser.hpp"
#include "thread_safe_puts.hpp"

#include <sys/stat.h>

namespace finder::parser
{
    namespace 
    {
    
    uint32_t have_permission_to_enter(std::filesystem::directory_entry current, bool& can_enter)
    {
        std::string directory_name{ current.path().string() };
        struct stat info;
        if (stat(directory_name.c_str(), &info) == 0)
        {
            mode_t& permissions = info.st_mode;
            can_enter = (permissions & S_IROTH);
        }
        return errno;
    }

    void search_file_impl(std::filesystem::directory_entry current, std::string_view filename, 
        thread_utils::thread_pool& pool)
    {
        if (pool.is_work_done())
        {
            thread_utils::thread_safe_puts("[INFO] thread #", std::this_thread::get_id(), 
                " : stopped doing task, as the work is done", '\n');
            return;
        }
        thread_utils::thread_safe_puts("[INFO] thread #", std::this_thread::get_id(), " : just entered directory : ", 
            current.path(), '\n');
        try
        {

        std::filesystem::directory_iterator directory{ current };
        for (const auto& entry : directory)
        {

            if (entry.is_directory())
            {
                pool.push(std::bind(search_file_impl, entry, filename, std::ref(pool)));
                continue;
            }
            if (entry.is_regular_file() && entry.path().filename() == filename)
            {
                pool.set_work_done();
                thread_utils::thread_safe_puts("[SUCCESS] thread #", std::this_thread::get_id(), 
                    " : file has been found at path : ", entry.path(), '\n');
            }
            
        }

        }
        catch (const std::filesystem::filesystem_error& e) { std::cout << e.what() << '\n'; }
    }

    } // End of anonymous namespace

    void search_file(std::filesystem::directory_entry initial, std::string_view filename, size_t parallel)
    {
        thread_utils::thread_pool workers{ parallel };
        workers.push(std::bind(search_file_impl, initial, filename, std::ref(workers)));
        workers.start();
        while (workers.is_work_done())
            continue;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

}