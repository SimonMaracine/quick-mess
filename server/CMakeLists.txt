cmake_minimum_required(VERSION 3.20)

add_executable(quick_mess_server
    "src/chat.cpp"
    "src/chat.hpp"
    "src/clock.cpp"
    "src/clock.hpp"
    "src/main.cpp"
    "src/server.cpp"
    "src/server.hpp"
)

target_link_libraries(quick_mess_server PRIVATE rain_net_server common nlohmann_json)

if(UNIX)
    target_compile_options(quick_mess_server PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wconversion")
elseif(MSVC)
    target_compile_options(quick_mess_server PRIVATE "/W4")
else()
    message(WARNING "Warnings are not enabled")
endif()

target_compile_features(quick_mess_server PRIVATE cxx_std_17)
set_target_properties(quick_mess_server PROPERTIES CXX_EXTENSIONS OFF)

if(UNIX AND QUICK_MESS_ASAN)
    target_compile_options(quick_mess_server PRIVATE "-fsanitize=address" "-fsanitize=undefined" "-g" "-fno-omit-frame-pointer")
    target_link_options(quick_mess_server PRIVATE "-fsanitize=address" "-fsanitize=undefined")
endif()

target_compile_definitions(quick_mess_server PRIVATE "_CRT_SECURE_NO_WARNINGS")
