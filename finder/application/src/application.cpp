#include <iostream>
#include <filesystem>

#include "parser.hpp"

int main()
{
    std::filesystem::directory_entry initial{ "C://" };
    finder::parser::search_file(initial, "find_me.txt", 8);
    return 0;
}