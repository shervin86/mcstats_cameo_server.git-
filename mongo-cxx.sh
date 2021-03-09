#!/bin/bash
INSTALL_PREFIX=/tmp/devel/

git clone -b r3.6.2 https://github.com/mongodb/mongo-cxx-driver.git || exit 1

cd mongo-cxx-driver/
mkdir build/
cd build/

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DCMAKE_INSTALL_RPATH=${INSTALL_PREFIX} -DCMAKE_CXX_STANDARD=17

cmake --build . -- -j 2
cmake --build . --target install
