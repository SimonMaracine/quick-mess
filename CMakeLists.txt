cmake_minimum_required(VERSION 3.20)

project("quick-mess" LANGUAGES CXX)

add_subdirectory(extern/json)
add_subdirectory(extern/Rain-Net)
add_subdirectory(extern/tiny-gui-base)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
