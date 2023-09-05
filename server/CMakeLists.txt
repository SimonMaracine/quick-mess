cmake_minimum_required(VERSION 3.20)

add_executable(quick_mess_server
    "src/main.cpp"
)

target_link_libraries(quick_mess_server PRIVATE rain_net common)

target_compile_options(quick_mess_server PRIVATE "-Wall" "-Wextra" "-Wpedantic")
