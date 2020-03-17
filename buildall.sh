#
# Copyright (c) 2018, Juniper Networks, Inc.
# All rights reserved.
#

#!/bin/bash
work_path=$(dirname $(readlink -f $0))
PREFIX_INSTALL_PATH=/usr/local
if [ $# -gt 0 ]
then
    PREFIX_INSTALL_PATH=$1
fi


# build for grpc
function build_grpc()
{
    cd ${work_path}/third_party/grpc
    rm -rf cmake/build
    mkdir -p cmake/build
    cd cmake/build
    
    cmake -DgRPC_INSTALL=ON  -DCMAKE_BUILD_TYPE=Release  -DCMAKE_INSTALL_PREFIX=${PREFIX_INSTALL_PATH}  ../..
    make -j 4
    make prefix=${PREFIX_INSTALL_PATH} install
    
    # cp the libgrpc to ${PREFIX_INSTALL_PATH} and cp lib64 to lib
    if [ ${PREFIX_INSTALL_PATH} != "/usr/local" ]
    then
        cp lib*.a ${PREFIX_INSTALL_PATH}/lib
        cp ${work_path}/third_party/grpc/cmake/build/grpc_*_plugin ${PREFIX_INSTALL_PATH}/bin
        if [ -d ${PREFIX_INSTALL_PATH}/lib64 ]
        then
            cp ${PREFIX_INSTALL_PATH}/lib64/lib*.a ${PREFIX_INSTALL_PATH}/lib/
        fi
    fi
}

function build_protobuf_c()
{
    cd ${work_path}/third_party/protobuf-c
    rm -rf cmake/build
    mkdir -p cmake/build
    cd cmake/build
    cmake -DProtobuf_PROTOC_EXECUTABLE=${PREFIX_INSTALL_PATH}/bin/protoc \
    -DProtobuf_PROTOC_LIBRARY=${PREFIX_INSTALL_PATH}/lib/libprotoc.a \
    -DProtobuf_LIBRARY=${PREFIX_INSTALL_PATH}/lib/libprotobuf.a \
    -DProtobuf_USE_STATIC_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_INSTALL_PATH} ../../build-cmake
    make -j 4
    make install

}

function build_grpc_c_lib()
{
    cd ${work_path}/lib
    
    rm -rf build
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=${PREFIX_INSTALL_PATH} ..
    make -j 4
    make install

}

function build_grpc_compiler()
{
    cd ${work_path}/compiler
    rm -rf build
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=${PREFIX_INSTALL_PATH} ..
    make -j 4
    make install
}

build_grpc
build_protobuf_c
build_grpc_c_lib
build_grpc_compiler




