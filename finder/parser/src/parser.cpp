#include <optional>

#include "parser.hpp"

#include "thread_pool.hpp"
#include "thread_safe_puts.hpp"

namespace finder::parser
{
    namespace fs = std::filesystem;
    namespace tu = thread_utils;

    namespace 
    {

    std::optional<fs::directory_iterator> get_iterator_from(fs::directory_entry current)
    {
        try
        {
            fs::directory_iterator directory{ current, fs::directory_options::skip_permission_denied };
            return directory;
        }
        catch (const fs::filesystem_error& e) 
        {
            return std::nullopt;
        }
    }

    void search_file_impl(fs::directory_entry current, std::string_view filename, tu::thread_pool& pool)
    {
        if (pool.is_work_done())
        {
            tu::thread_safe_puts("[INFO] thread #", std::this_thread::get_id(), 
                " : stopped doing task, as the work is done", '\n');
            return;
        }
        tu::thread_safe_puts("[INFO] thread #", std::this_thread::get_id(), " : just entered directory : ", 
            current.path(), '\n');
        if (auto directory = get_iterator_from(current))
        {
            for (const auto& entry : directory.value())
            {
                try
                {
                    if (entry.is_directory())
                    {
                        pool.push(std::bind(search_file_impl, entry, filename, std::ref(pool)));
                        continue;
                    }
                    if (entry.is_regular_file() && entry.path().filename() == filename)
                    {
                        pool.set_work_done();
                        tu::thread_safe_puts("[SUCCESS] thread #", std::this_thread::get_id(), 
                            " : file has been found at path : ", entry.path(), '\n');
                    }
                }
                catch (const fs::filesystem_error& e) 
                {
                    tu::thread_safe_puts("[ERROR] thread #", std::this_thread::get_id(), " : no permission to enter ",
                        current.path().c_str(), '\n');
                }
            }
        }
        else
        {
            tu::thread_safe_puts("[ERROR] thread #", std::this_thread::get_id(), " : no permission to enter ",
                current.path().c_str(), '\n');
        }
    }

    } // End of anonymous namespace

    void search_file(fs::directory_entry initial, std::string_view filename, size_t parallel)
    {
        tu::thread_pool threads{ parallel };
        threads.push(std::bind(search_file_impl, initial, filename, std::ref(threads)));
        threads.start();
        while (!threads.is_work_done())
            continue;
    }

}