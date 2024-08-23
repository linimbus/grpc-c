cmake_minimum_required(VERSION 3.20)

project(grpc-c)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_C_FLAGS " -g -fPIC -fpermissive ")
endif()

set(CMAKE_THREAD_LIBS_INIT "-lpthread")

execute_process(
    COMMAND bash -c "git rev-parse HEAD"
    OUTPUT_VARIABLE COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND bash -c "git log -1 --format=%cd --date=format:'%Y-%m-%d %H:%M:%S'"
    OUTPUT_VARIABLE COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message("This is a Linux environment.")
    add_definitions(-DLINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message("This is a Windows environment.")
    add_definitions(-DWINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message("This is a macOS environment.")
    add_definitions(-DDARWIN)
else()
    message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}.")
endif()

set(CMAKE_CXX_FLAGS "${CXX_ONLY_FLAGS} ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

set(CMAKE_INSTALL_PREFIX "publish")

add_subdirectory(lib)
add_subdirectory(compiler)
add_subdirectory(example)
