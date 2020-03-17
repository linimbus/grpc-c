# GRPC-C

C implementation of [gRPC](http://www.grpc.io/) layered of top of core libgrpc. 

## Prerequisites

Make sure you have the following install in order to install dependencies. Base on ubuntu 18.04 and x86_64.

```
apt install -y autoconf automake libtool curl make cmake g++ unzip zlib1g-dev \
               zlib1g openssl libssl-dev pkg-config libgflags-dev libgtest-dev golang
```

## Build

```sh
git clone https://github.com/h00448672/grpc-c.git
cd grpc-c
./buildall.sh
```

If you want to install dependencies in a different directory other than /usr/local/, use ./buildall.sh  <your-prefix>

## Examples

if you install dependencies  in /usr/local

```sh
cd examples
./build.sh
```

if you install dependencies  in  your-prefix

```cd  
cd examples
./build.sh  <your-prefix>
```

To run example code: 

```sh
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
cd examples
./bin/foo_bin server
./bin/foo_bin client
```

## Dependencies

- gRPC [v1.17.2](https://github.com/grpc/grpc/releases/tag/v1.17.2)
- protobuf 3.6.1.3
- protobuf-c 1.3.1
- c-ares 1.17.0
