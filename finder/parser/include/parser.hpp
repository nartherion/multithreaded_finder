#pragma once

#include <filesystem>
#include <string_view>

#include "thread_pool.hpp"

namespace finder::parser
{

void search_file(std::filesystem::directory_entry, std::string_view, size_t = 8);

}

