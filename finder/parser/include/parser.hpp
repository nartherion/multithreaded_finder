#pragma once

#include <filesystem>
#include <string_view>

namespace finder::parser
{

void search_file(std::filesystem::directory_entry, std::string_view, size_t = 8);

}

