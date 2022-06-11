add_executable(Application src/application.cpp)

target_link_libraries(Application PRIVATE 
    Parser
    )