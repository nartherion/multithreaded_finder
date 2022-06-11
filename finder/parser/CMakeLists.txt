add_library(Parser
    include/parser.hpp
    src/parser.cpp
    )

target_include_directories(Parser PUBLIC 
    include/
    )

target_link_libraries(Parser PUBLIC 
    ThreadUtils
    )