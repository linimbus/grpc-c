cmake_minimum_required (VERSION 2.6)
project(protoc-gen-grpc-c)

# Install dir set

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -g3 -ggdb3 -gdwarf-2 -std=c++11 -pthread")

# add the binary tree to the search path for include files
# so that we will find config.h
include_directories("${PROJECT_BINARY_DIR}")

# add the grpc library?
include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/../third_party/protobuf-c/")
include_directories("${PROJECT_SOURCE_DIR}/../third_party/protobuf-c/protobuf-c")
include_directories("${PROJECT_SOURCE_DIR}/../third_party/protobuf-c/protoc-c")
include_directories("${PROJECT_SOURCE_DIR}/../third_party/grpc/third_party/protobuf/src")

# add the executable
add_executable(protoc-gen-grpc-c
    ../third_party/protobuf-c/protoc-c/c_bytes_field.cc
    ../third_party/protobuf-c/protoc-c/c_enum.cc
    ../third_party/protobuf-c/protoc-c/c_enum_field.cc
    ../third_party/protobuf-c/protoc-c/c_extension.cc
    ../third_party/protobuf-c/protoc-c/c_field.cc
    ../third_party/protobuf-c/protoc-c/c_file.cc
    ../third_party/protobuf-c/protoc-c/c_generator.cc
    ../third_party/protobuf-c/protoc-c/c_helpers.cc
    ../third_party/protobuf-c/protoc-c/c_message.cc
    ../third_party/protobuf-c/protoc-c/c_message_field.cc
    ../third_party/protobuf-c/protoc-c/c_primitive_field.cc
    ../third_party/protobuf-c/protoc-c/c_service.cc
    ../third_party/protobuf-c/protoc-c/c_string_field.cc
    grpc_c_file.cc
    grpc_c_generator.cc
    grpc_c_helpers.cc
    grpc_c_message.cc
    grpc_c_service.cc
    main.cc )

set(PROTOC_LIB "${CMAKE_INSTALL_PREFIX}/lib/libprotoc.a")
set(PROTOBUF_LIB "${CMAKE_INSTALL_PREFIX}/lib/libprotobuf.a")

TARGET_LINK_LIBRARIES(protoc-gen-grpc-c ${PROTOC_LIB} ${PROTOBUF_LIB})

# add the install targets
install (TARGETS protoc-gen-grpc-c DESTINATION bin)
