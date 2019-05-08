cmake_minimum_required (VERSION 2.6)
project(grpc-c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -g3 -ggdb3 -gdwarf-2 -std=c++11 -pthread")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")

file(GLOB SRC_FILE "*.c")
file(GLOB INC_FILE "include/grpc-c/*.h")

# add the executable
add_library(grpc-c-shared SHARED ${SRC_FILE})
add_library(grpc-c-static STATIC ${SRC_FILE})

SET_TARGET_PROPERTIES (grpc-c-shared PROPERTIES OUTPUT_NAME "grpc-c")
SET_TARGET_PROPERTIES (grpc-c-static PROPERTIES OUTPUT_NAME "grpc-c")

TARGET_LINK_LIBRARIES(grpc-c-shared protobuf protobuf-c grpc)
TARGET_LINK_LIBRARIES(grpc-c-static protobuf protobuf-c grpc)

# add the install targets
install (TARGETS grpc-c-shared DESTINATION lib)
install (TARGETS grpc-c-static DESTINATION lib)
install (FILES ${INC_FILE} DESTINATION include/grpc-c)