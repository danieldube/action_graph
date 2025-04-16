cmake_minimum_required(VERSION 3.20)
project(action_graph)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)
enable_testing()
find_package(GTest REQUIRED)
add_subdirectory(tests)
