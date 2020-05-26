cmake_minimum_required(VERSION 3.10)

# Project name and version
project(cameo-mcstas VERSION 0.10.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Want to compile dynamic or static libraries
set(BUILD_SHARED_LIBS False)


add_compile_options(-Wall -Wextra -pedantic -Werror)
# where to find the header files
include_directories(include include/nlohmann)
#add_definitions(-DDEBUG)

# definitions for the client and server 
add_definitions(-DSERVERNAME="mcstas_server")
add_definitions(-DREQUESTER_RESPONDER_NAME="mcstas_responder")

#------------------------------------------------------------
# Libraries
#------------------------------------------------------------
add_library(sim_request         src/sim_request.cc)         # client
add_library(sim_request_server  src/sim_request_server.cc)  # server
target_link_libraries(sim_request_server sim_request)

add_library(sim_result src/sim_result.cc)               # client
add_library(sim_result_server src/sim_result_server.cc) # server
target_link_libraries(sim_result_server sim_result)

#------------------------------------------------------------
# Executables
#------------------------------------------------------------
# Client
add_executable(fakeNomad src/fakeNomad.cpp)
target_link_libraries(fakeNomad PRIVATE cameo)
target_link_libraries(fakeNomad PRIVATE stdc++fs)
target_link_libraries(fakeNomad PRIVATE m zmq protobuf)
target_link_libraries(fakeNomad PRIVATE sim_request sim_result)


# Server
add_executable(mcstas_server src/mcstas_server.cpp)
target_link_libraries(mcstas_server PRIVATE cameo)
target_link_libraries(mcstas_server PRIVATE m zmq protobuf)
target_link_libraries(mcstas_server PRIVATE stdc++fs)
target_link_libraries(mcstas_server PRIVATE sim_request_server sim_result_server)


#add_executable(mcstas_server.cpp)

############################# MCSTAS
link_directories(/usr/share/mcstas/2.6/libs/mcpl/)

add_custom_command(OUTPUT ILL_H512_D22-sSAMPLE.c
  COMMENT "creation of mcstas .c"
  COMMAND mcstas -t -o build/ILL_H512_D22-sSAMPLE.c mcstas/ILL_H512_D22-sSAMPLE.instr
  WORKING_DIRECTORY ../
  )

add_custom_command(OUTPUT ILL_H512_D22.c
  COMMENT "creation of mcstas .c"
  COMMAND mcstas -t -o build/ILL_H512_D22.c mcstas/ILL_H512_D22.instr
  WORKING_DIRECTORY ../
  )


add_executable(ILL_H512_D22-sSAMPLE.out ILL_H512_D22-sSAMPLE.c)
add_executable(ILL_H512_D22.out ILL_H512_D22.c)
target_compile_options(ILL_H512_D22-sSAMPLE.out PUBLIC -O3 -Wno-error -Wno-all -Wno-extra)
target_include_directories(ILL_H512_D22-sSAMPLE.out PUBLIC /usr/share/mcstas/2.6/libs/mcpl)
target_link_libraries(ILL_H512_D22-sSAMPLE.out PUBLIC m)
target_link_libraries(ILL_H512_D22-sSAMPLE.out PUBLIC mcpl)

target_compile_options(ILL_H512_D22.out PUBLIC -O3 -Wno-error -Wno-all -Wno-extra)
target_include_directories(ILL_H512_D22.out PUBLIC /usr/share/mcstas/2.6/libs/mcpl)
target_link_libraries(ILL_H512_D22.out PUBLIC m)
target_link_libraries(ILL_H512_D22.out PUBLIC mcpl)




################ Install
set(CMAKE_INSTALL_PREFIX /tmp/devel/)
install(TARGETS fakeNomad mcstas_server ILL_H512_D22-sSAMPLE.out ILL_H512_D22.out DESTINATION bin)
if (BUILD_SHARED_LIBS)
  install(TARGETS sim_request sim_request_server sim_result LIBRARY DESTINATION lib ARCHIVE DESTINATION obj)
endif(BUILD_SHARED_LIBS)
  
enable_testing()
add_test(NAME d22 COMMAND ILL_H512_D22.out -n 10000 lambda=4.5)

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
  add_custom_target( doc 
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)


