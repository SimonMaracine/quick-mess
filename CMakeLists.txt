cmake_minimum_required(VERSION 3.20)

project("quick-mess" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Variable used to control the build type")

option(QUICK_MESS_ASAN "Enable sanitizers" OFF)

add_subdirectory(extern/Rain-Net)
add_subdirectory(extern/json)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

message(STATUS "quick-mess: Sanitizers: ${QUICK_MESS_ASAN}")
