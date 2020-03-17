#!/bin/bash

chmod -R +x *

function proto_to_c() 
{
	protoc -I $1 --grpc-c_out=$1 --plugin=protoc-gen-grpc-c=/usr/local/bin/protoc-gen-grpc-c $1/$2
	if [ $? == 0 ] ;then
		echo "protc to c " $1 $2 "success!"
	else
		echo "protc to c " $1 $2 "failed!"
	fi
}

proto_to_c ./foo foo.proto

GRPC_INSTALL_PATH=/usr/local
if [ $# -gt 0 ]
then
    GRPC_INSTALL_PATH=$1
fi

rm -rf build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${GRPC_INSTALL_PATH} ..
make -j 4
make install
cd ..

