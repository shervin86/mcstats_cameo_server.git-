cmake_minimum_required(VERSION 3.10)

# set the project name
project(cameo-mcstas VERSION 0.10)

# add the executable
add_executable(fakeNomad src/fakeNomad.cpp)
#add_executable(mcstas_server.cpp)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_INSTALL_PREFIX ../)
target_link_libraries(fakeNomad PUBLIC m)
target_link_libraries(fakeNomad PUBLIC cameo)
target_link_libraries(fakeNomad PUBLIC zmq)
target_link_libraries(fakeNomad PUBLIC protobuf)

#target_include_directories(fakeNomad PUBLIC 


install(TARGETS fakeNomad DESTINATION bin)
