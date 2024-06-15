cmake_minimum_required(VERSION 3.20)

project("quick-mess" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Variable used to control the build type")

add_subdirectory(extern/Rain-Net)
add_subdirectory(extern/tiny-gui-base)
add_subdirectory(extern/json)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
