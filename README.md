# GRPC-C

C implementation of [gRPC](http://www.grpc.io/) layered of top of core libgrpc. 

## Prerequisites

Make sure you have the following install in order to install dependencies. Base on ubuntu 18.04 and x86_64.

```sh
#For Ubuntu
apt install -y autoconf automake libtool curl make cmake g++ unzip zlib1g-dev \
               zlib1g openssl libssl-dev pkg-config libgflags-dev libgtest-dev

#For CentOS/RHEL
yum install -y autoconf automake libtool curl make cmake  unzip  openssl gcc-c++ \
openssl-devel gflags-devel gtest gtest-devel zlib-devel zlib
```

## Build

```sh
git clone https://github.com/lixiangyun/grpc-c.git
cd grpc-c
#For Ubuntu dependency 
./builddeps.sh
#For CentOS/RHEL dependency
./builddeps-rhel.sh

./buildlib.sh
./buildcmp.sh
```

If you want to install dependencies in a different directory other than /usr/local/, use ./builddeps.sh and ./buildlib.sh <your-prefix>, such as `./builddeps.sh /usr`


## Examples

```sh
cd examples
./build.sh
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
- c-ares 1.15.0
