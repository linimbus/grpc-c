# GRPC-C

C implementation of [gRPC](http://www.grpc.io/) layered of top of core libgrpc. 

## Prerequisites

Make sure you have the following install in order to install dependencies

```
apt install -y autoconf automake libtool curl make g++ unzip zlib1g-dev zlib1g openssl libssl-dev pkg-config libgflags-dev libgtest-dev
```

## Build

```sh
./builddeps.sh
./buildlib.sh
./buildcmp.sh
```

If you want to install dependencies in a different directory other than /usr/local/, use ./buildgen.sh <your-prefix>


## Examples

```sh
cd examples
make gencode
make
```

This should build foo_client and foo_server. To run example code, 

```sh
./foo_server test
./foo_client test
```

## Status

Pre-alpha. Under active development. APIs may change.

## Dependencies

- gRPC [v1.17.2](https://github.com/grpc/grpc/releases/tag/v1.17.2)
- protobuf 3.6.1.3
- protobuf-c 1.3.1
