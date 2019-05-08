cmake_minimum_required (VERSION 2.6)

set(OBJECT "foo_bin")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -g3 -ggdb3 -gdwarf-2")

include_directories("${PROJECT_SOURCE_DIR}")

file(GLOB SRC_FILE "*.c")

add_executable(${OBJECT} ${SRC_FILE})

TARGET_LINK_LIBRARIES(${OBJECT} protobuf protobuf-c grpc grpc-c)

# add the install targets
install (TARGETS ${OBJECT} DESTINATION bin)