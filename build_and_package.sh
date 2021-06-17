#!/bin/bash

BASE_BUILD_DIR=${1:-/dev/shm/mcstats_cameo_server/}
[ "$BASE_BUILD_DIR" != "${BASE_BUILD_DIR#/}" ] || BASE_BUILD_DIR=$PWD/$BASE_BUILD_DIR

packages_dir=${BASE_BUILD_DIR}/packages
mkdir ${packages_dir} -p

function mvPack(){
	mv ${build_dir}/packaging/*.deb ${packages_dir}/
}


#--------------- API
#---------- C++
build_dir=$BASE_BUILD_DIR/client_api/
cmake -S client_api -B $build_dir -DCMAKE_PREFIX_PATH=/usr/share/ -DSERVER=OFF -DCMAKE_INSTALL_PREFIX=/usr/ -DBUILD_SHARED_LIBS=ON # with this it adds x86_64-linux-gnu to the path when running cpack
cmake --build $build_dir 
cpack --config $build_dir/CPackConfig.cmake -B $build_dir/packaging  
mvPack

#--------------- Server
#---------- C++
build_dir=$BASE_BUILD_DIR/server/
cmake -S . -B $build_dir -DCMAKE_PREFIX_PATH=/usr/share/ -DSERVER=ON -DCMAKE_INSTALL_PREFIX=/usr/  # with this it adds x86_64-linux-gnu to the path when running cpack
cmake --build $build_dir 
cpack --config $build_dir/CPackConfig.cmake -B $build_dir/packaging 
mvPack

