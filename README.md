# GRPC-C

C implementation of [gRPC](http://www.grpc.io/) layered of top of core libgrpc. 

## Prerequisites

The current project is built based on [vcpkg + cmake] and supports multi-platform, such as windows, linux, macos.
You need to prepare the [vcpkg + cmake] tool local platform first. 
Refer to the official website: https://github.com/microsoft/vcpkg

## Build

```sh
git clone https://github.com/linimbus/grpc-c.git
cd grpc-c
#For linux/macos 
./build_release.sh
#For windows
./build_release.bat
```


If you see the following result, congratulations, you have compiled successfully.

```
-- Install configuration: "Release"
-- Installing: /Users/linimbus/workspace/grpc-c/publish/lib/libgrpc-c.dylib
-- Installing: /Users/linimbus/workspace/grpc-c/publish/lib/libgrpc-c.a
-- Up-to-date: /Users/linimbus/workspace/grpc-c/publish/include/grpc-c.h
-- Installing: /Users/linimbus/workspace/grpc-c/publish/bin/protoc-gen-grpc-c
-- Installing: /Users/linimbus/workspace/grpc-c/publish/bin/example-foo
```

To run example code: 

```sh
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
cd publish/bin
./bin/foo_bin server
./bin/foo_bin client
```

## Dependencies

Maintain in vcpkg.json file

```
{
    "dependencies": [
        "protobuf",
        {
            "name": "protobuf-c",
            "features": []
        },
        "grpc"
    ]
}
```
