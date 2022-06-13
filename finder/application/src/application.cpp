#include <iostream>
#include <filesystem>

#include "parser.hpp"

int main()
{
    std::filesystem::directory_entry initial{ "/" };
    finder::parser::search_file(initial, "file.txt", 2);
    return 0;
}