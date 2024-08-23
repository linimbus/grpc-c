cmake_minimum_required(VERSION 3.20)

project(example-foo)

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/../../lib/include")

set(PROTO_C_FILE "foo.proto")

find_package(protobuf CONFIG REQUIRED)
find_package(protobuf-c CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/foo.grpc-c.c
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} -I ${PROJECT_SOURCE_DIR} --grpc-c_out=${PROJECT_SOURCE_DIR} --plugin=protoc-gen-grpc-c=$<TARGET_FILE:protoc-gen-grpc-c> --proto_path=${PROTO_C_DIR}/protobuf-c/ ${PROTO_C_FILE}
)

add_executable(example-foo
    foo.grpc-c.c
    foo.grpc-c.service.c
    foo_client.c
    foo_server.c
    foo_main.c)

TARGET_LINK_LIBRARIES(example-foo PRIVATE protobuf-c::protobuf-c gRPC::gpr gRPC::grpc++ grpc-c-static)

install(TARGETS example-foo DESTINATION bin
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ
                WORLD_EXECUTE WORLD_READ)