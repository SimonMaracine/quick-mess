cmake_minimum_required(VERSION 3.20)

project("quick-mess" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Variable used to control the build type")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

add_subdirectory(extern/json)
add_subdirectory(extern/Rain-Net)
add_subdirectory(extern/tiny-gui-base)

add_subdirectory(common)
add_subdirectory(client)

if(UNIX)
    add_subdirectory(server)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
