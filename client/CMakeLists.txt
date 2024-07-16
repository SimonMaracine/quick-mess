cmake_minimum_required(VERSION 3.20)

add_subdirectory(extern/tiny-gui-base)

add_executable(quick_mess
    "src/client.cpp"
    "src/client.hpp"
    "src/data.cpp"
    "src/data.hpp"
    "src/main.cpp"
    "src/window.cpp"
    "src/window.hpp"
)

target_link_libraries(quick_mess PRIVATE rain_net common gui_base nlohmann_json)

if(UNIX)
    target_compile_options(quick_mess PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wconversion")
elseif(MSVC)
    target_compile_options(quick_mess PRIVATE "/W4")
else()
    message(WARNING "Warnings are not enabled")
endif()

target_compile_features(quick_mess PRIVATE cxx_std_17)
set_target_properties(quick_mess PROPERTIES CXX_EXTENSIONS OFF)

if(UNIX AND QUICK_MESS_ASAN)
    target_compile_options(quick_mess PRIVATE "-fsanitize=address" "-fsanitize=undefined" "-g" "-fno-omit-frame-pointer")
    target_link_options(quick_mess PRIVATE "-fsanitize=address" "-fsanitize=undefined")
endif()

target_compile_definitions(quick_mess PRIVATE "_CRT_SECURE_NO_WARNINGS")
