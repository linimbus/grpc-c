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
./configure --prefix=${pfx}
make -j 4
make prefix=${PERFIX_PATH} install
sudo ldconfig
cd ../../

echo "Building protobuf-c"
cd third_party/protobuf-c
./configure --prefix=${PERFIX_PATH}
make -j 4
make install
cd ../../

echo "Building gRPC"
cd third_party/grpc
make -j 4
make prefix=${PERFIX_PATH} install
cd ../../
