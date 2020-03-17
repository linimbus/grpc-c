cmake_minimum_required (VERSION 2.6)

set(OBJECT "foo_bin")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -g3 -ggdb3 -gdwarf-2")

include_directories("${PROJECT_SOURCE_DIR}")

file(GLOB SRC_FILE "*.c")

add_executable(${OBJECT} ${SRC_FILE})

set(PROTOBUF_C_LIB "${CMAKE_INSTALL_PREFIX}/lib/libprotobuf-c.a")
set(GRPC_LIB "${CMAKE_INSTALL_PREFIX}/lib/libgrpc.a")
set(GRPC_C_LIB "${CMAKE_INSTALL_PREFIX}/lib/libgrpc-c.a")
set(GPR_LIB "${CMAKE_INSTALL_PREFIX}/liblibgpr.a")

file(GLOB LIBS "${CMAKE_INSTALL_PREFIX}/lib/lib*.a")
file(GLOB LIB64S "${CMAKE_INSTALL_PREFIX}/lib64/lib*.a")

TARGET_LINK_LIBRARIES(${OBJECT} -Wl,--start-group ${LIBS}  ${LIB64S} -lpthread -lstdc++ -lm -Wl,--end-group)

# add the install targets
install (TARGETS ${OBJECT} DESTINATION ${PROJECT_SOURCE_DIR}/bin)
