#!/bin/bash

PERFIX_PATH=/usr/local
if [ $# -gt 0 ]
then
    PERFIX_PATH=$1
fi

cd lib
chmod -R +x *
rm -rf build

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${PERFIX_PATH} ..
make -j 4
make install
cd ..

