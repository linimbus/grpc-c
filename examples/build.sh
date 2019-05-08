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
proto_to_c ./server_streaming server_streaming.proto
proto_to_c ./client_streaming client_streaming.proto
proto_to_c ./bidi_streaming bidi_streaming.proto

rm -rf build
mkdir build
cd build
cmake ..
make -j 4
make install
cd ..

