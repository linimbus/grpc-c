cmake_minimum_required (VERSION 2.6)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -g3 -ggdb3 -gdwarf-2")

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}")

include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory (bidi_streaming)
add_subdirectory (client_streaming)
add_subdirectory (server_streaming)
add_subdirectory (foo)