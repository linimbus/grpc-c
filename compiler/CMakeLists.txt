cmake_minimum_required(VERSION 3.20)

project(protoc-gen-grpc-c)

set(PROTO_C_FILE "protobuf-c.proto")
set(PROTO_C_DIR "${PROJECT_SOURCE_DIR}/../protobuf-c/")

include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROTO_C_DIR}")

find_package(protobuf CONFIG REQUIRED)
find_package(protobuf-c CONFIG REQUIRED)

add_custom_command(
    OUTPUT ${PROTO_C_DIR}/protobuf-c/protobuf-c.pb.cc
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out=${PROTO_C_DIR}/protobuf-c/ --proto_path=${PROTO_C_DIR}/protobuf-c/ ${PROTO_C_FILE}
)

# add the executable
add_executable(protoc-gen-grpc-c
    ${PROTO_C_DIR}/protobuf-c/protobuf-c.pb.cc
    ${PROTO_C_DIR}/protoc-c/c_bytes_field.cc
    ${PROTO_C_DIR}/protoc-c/c_enum.cc
    ${PROTO_C_DIR}/protoc-c/c_enum_field.cc
    ${PROTO_C_DIR}/protoc-c/c_extension.cc
    ${PROTO_C_DIR}/protoc-c/c_field.cc
    ${PROTO_C_DIR}/protoc-c/c_file.cc
    ${PROTO_C_DIR}/protoc-c/c_generator.cc
    ${PROTO_C_DIR}/protoc-c/c_helpers.cc
    ${PROTO_C_DIR}/protoc-c/c_message.cc
    ${PROTO_C_DIR}/protoc-c/c_message_field.cc
    ${PROTO_C_DIR}/protoc-c/c_primitive_field.cc
    ${PROTO_C_DIR}/protoc-c/c_service.cc
    ${PROTO_C_DIR}/protoc-c/c_string_field.cc
    grpc_c_file.cc
    grpc_c_generator.cc
    grpc_c_helpers.cc
    grpc_c_message.cc
    grpc_c_service.cc
    main.cc )

TARGET_LINK_LIBRARIES(protoc-gen-grpc-c PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf-c::protobuf-c )

install(TARGETS protoc-gen-grpc-c DESTINATION bin
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ
                WORLD_EXECUTE WORLD_READ)