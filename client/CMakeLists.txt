cmake_minimum_required(VERSION 3.20)

add_executable(quick_mess
    "src/chat.hpp"
    "src/client.cpp"
    "src/client.hpp"
    "src/main.cpp"
    "src/window.cpp"
    "src/window.hpp"
)

target_link_libraries(quick_mess PRIVATE rain_net common gui_base)

target_compile_options(quick_mess PRIVATE "-Wall" "-Wextra" "-Wpedantic")
