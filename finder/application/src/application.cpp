#include <iostream>
#include <filesystem>

#include "parser.hpp"

namespace fs = std::filesystem;

int main()
{
    fs::directory_entry initial{ "/sys" };
    finder::parser::search_file(initial, "not_file.txt", 8);
    return 0;
}