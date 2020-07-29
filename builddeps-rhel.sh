#
# Copyright (c) 2018, Juniper Networks, Inc.
# All rights reserved.
#

#!/bin/sh
PERFIX_PATH=/usr/local
if [ $# -gt 0 ]
then
    PERFIX_PATH=$1
fi

function init(){
    # check for dependency version with .pc
    echo "PERFIX_PATH=${PERFIX_PATH}"
    export PKG_CONFIG_PATH=${PERFIX_PATH}/lib/pkgconfig:$PKG_CONFIG_PATH
    # load lib path
    cat /etc/ld.so.conf | grep ${PERFIX_PATH}/lib > /dev/null 2>&1
    if [ $? -gt 0 ]
    then
        ${PERFIX_PATH}/lib >> /etc/ld.so.conf
        sudo ldconfig
    fi
}

init
chmod -R +x *
echo "Building C-ares"
cd third_party/c-ares
./configure --prefix=${PERFIX_PATH}
make -j 4
make install
cd ../../

echo "Installing Protobuf"
cd third_party/protobuf
./autogen.sh
./configure --prefix=${PERFIX_PATH}
make -j 4
make prefix=${PERFIX_PATH} install
sudo ldconfig
cd ../../

echo "Building protobuf-c"
cd third_party/protobuf-c
./autogen.sh
./configure --prefix=${PERFIX_PATH}
make -j 4
make install
cd ../../

echo "Building gRPC"
cd third_party/grpc
make prefix=${PERFIX_PATH} install_c
cd ../../
