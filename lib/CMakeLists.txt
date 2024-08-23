cmake_minimum_required(VERSION 3.20)
project(grpc-c)

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")

file(GLOB_RECURSE SRC_FILE "src/*.c")
file(GLOB_RECURSE INC_FILE "include/*.h")

# add the executable
add_library(grpc-c-shared SHARED ${SRC_FILE})
add_library(grpc-c-static STATIC ${SRC_FILE})

SET_TARGET_PROPERTIES (grpc-c-shared PROPERTIES OUTPUT_NAME "grpc-c")
SET_TARGET_PROPERTIES (grpc-c-static PROPERTIES OUTPUT_NAME "grpc-c")

find_package(protobuf CONFIG REQUIRED)
find_package(protobuf-c CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

TARGET_LINK_LIBRARIES(grpc-c-shared PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf-c::protobuf-c gRPC::gpr gRPC::grpc++)
TARGET_LINK_LIBRARIES(grpc-c-static PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf-c::protobuf-c gRPC::gpr gRPC::grpc++)

# add the install targets
install(TARGETS grpc-c-shared DESTINATION lib
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ
                WORLD_EXECUTE WORLD_READ)

install(TARGETS grpc-c-static DESTINATION lib
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
            GROUP_EXECUTE GROUP_READ
            WORLD_EXECUTE WORLD_READ)

install (FILES ${INC_FILE} DESTINATION include)
