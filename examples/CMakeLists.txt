cmake_minimum_required (VERSION 2.6)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -g3 -ggdb3 -gdwarf-2")


include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory (foo)
