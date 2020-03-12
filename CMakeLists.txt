cmake_minimum_required(VERSION 3.10)

# set the project name
project(cameo-mcstas VERSION 0.10.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
include_directories(include include/nlohmann)


# add the executable
add_executable(fakeNomad src/fakeNomad.cpp)
target_link_libraries(fakeNomad PUBLIC m)
target_link_libraries(fakeNomad PUBLIC cameo)
target_link_libraries(fakeNomad PUBLIC zmq)
target_link_libraries(fakeNomad PUBLIC protobuf)

add_executable(mcstas_server src/mcstas_server.cpp)
target_link_libraries(mcstas_server PUBLIC m)
target_link_libraries(mcstas_server PUBLIC cameo)
target_link_libraries(mcstas_server PUBLIC zmq)
target_link_libraries(mcstas_server PUBLIC protobuf)
add_definitions(-DDEBUG)


#add_executable(mcstas_server.cpp)




################ Install
set(CMAKE_INSTALL_PREFIX ../)
install(TARGETS fakeNomad DESTINATION bin)



################ Doc
find_package(Doxygen)
if (DOXYGEN_FOUND)
  # set input and output files
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  
  # request to configure the file
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message("Doxygen build started")
  
  # note the option ALL which allows to build the docs together with the application
  add_custom_target( doc ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)


