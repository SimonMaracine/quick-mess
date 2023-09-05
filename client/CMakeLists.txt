cmake_minimum_required(VERSION 3.20)

add_executable(quick_mess
    "src/main.cpp"
)

target_link_libraries(quick_mess PRIVATE rain_net)

target_compile_options(quick_mess PRIVATE "-Wall" "-Wextra" "-Wpedantic")
