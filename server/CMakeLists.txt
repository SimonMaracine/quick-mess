cmake_minimum_required(VERSION 3.20)

add_executable(quick_mess_server
    "src/chat.cpp"
    "src/chat.hpp"
    "src/main.cpp"
    "src/server.cpp"
    "src/server.hpp"
)

target_link_libraries(quick_mess_server PRIVATE rain_net common nlohmann_json)

target_compile_options(quick_mess_server PRIVATE "-Wall" "-Wextra" "-Wpedantic")
