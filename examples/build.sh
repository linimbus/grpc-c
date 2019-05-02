#
# Copyright (c) 2018, Juniper Networks, Inc.
# All rights reserved.
#

#!/bin/bash

cd lib

chmod -R +x *

rm -rf build
mkdir build
cd build
cmake ..
make -j 4
make install
cd ..

