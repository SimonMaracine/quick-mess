cmake_minimum_required(VERSION 3.20)

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
    target_compile_options(quick_mess PRIVATE "-Wall" "-Wextra" "-Wpedantic")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(quick_mess PRIVATE "/W3")
else()
    message(WARNING "Compiler is not MSVC or platform is not supported, warnings are not enabled")
endif()

target_compile_features(quick_mess PRIVATE cxx_std_17)
set_target_properties(quick_mess PROPERTIES CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
