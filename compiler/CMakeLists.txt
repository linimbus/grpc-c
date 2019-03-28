cmake_minimum_required (VERSION 2.6)
project(protoc-gen-grpc-c)

# Install dir set
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install) 
set(BIN_DEST_DIR ${CMAKE_INSTALL_PREFIX}/bin)

link_directories(
    ${PROJECT_SOURCE_DIR}/../../opensource/install/lib
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -g3 -ggdb3 -gdwarf-2 -std=c++11 -pthread")

# add the binary tree to the search path for include files
# so that we will find config.h
include_directories("${PROJECT_BINARY_DIR}")

# add the grpc library?
include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/../../opensource/install/include")
include_directories("${PROJECT_SOURCE_DIR}/../../opensource/protobuf-c-1.3.1")
include_directories("${PROJECT_SOURCE_DIR}/../../opensource/protobuf-c-1.3.1/protobuf-c")
include_directories("${PROJECT_SOURCE_DIR}/../../opensource/protobuf-c-1.3.1/protoc-c")

# add the executable
add_executable(protoc-gen-grpc-c
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_bytes_field.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_enum.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_enum_field.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_extension.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_field.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_file.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_generator.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_helpers.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_message.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_message_field.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_primitive_field.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_service.cc
    ../../opensource/protobuf-c-1.3.1/protoc-c/c_string_field.cc
    grpc_c_file.cc
    grpc_c_generator.cc
    grpc_c_helpers.cc
    grpc_c_message.cc
    grpc_c_service.cc
    main.cc )

TARGET_LINK_LIBRARIES(protoc-gen-grpc-c libprotobuf.a libprotoc.a)

# add the install targets
install (TARGETS protoc-gen-grpc-c DESTINATION bin)