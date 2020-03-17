cmake_minimum_required (VERSION 2.6)
project(grpc-c)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -g3 -ggdb3 -gdwarf-2")
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/../third_party/grpc/include")
include_directories("${PROJECT_SOURCE_DIR}/../third_party/protobuf-c")

file(GLOB SRC_FILE "*.c")
file(GLOB INC_FILE "include/grpc-c/*.h")

# add the executable
add_library(grpc-c-shared SHARED ${SRC_FILE})
add_library(grpc-c-static STATIC ${SRC_FILE})

SET_TARGET_PROPERTIES (grpc-c-shared PROPERTIES OUTPUT_NAME "grpc-c")
SET_TARGET_PROPERTIES (grpc-c-static PROPERTIES OUTPUT_NAME "grpc-c")

set(PROTOBUF_C_LIB "${CMAKE_INSTALL_PREFIX}/lib/libprotobuf-c.a")
set(GRPC_LIB "${CMAKE_INSTALL_PREFIX}/lib/libgrpc.a")
set(PROTOBUF_LIB "${CMAKE_INSTALL_PREFIX}/lib/libprotobuf.a")

TARGET_LINK_LIBRARIES(grpc-c-shared ${PROTOBUF_LIB} ${PROTOBUF_C_LIB} ${GRPC_LIB})
TARGET_LINK_LIBRARIES(grpc-c-static ${PROTOBUF_LIB} ${PROTOBUF_C_LIB} ${GRPC_LIB})

# add the install targets
install (TARGETS grpc-c-shared DESTINATION lib)
install (TARGETS grpc-c-static DESTINATION lib)
install (FILES ${INC_FILE} DESTINATION include/grpc-c)
