cmake_minimum_required(VERSION 3.7.2)

# Project name and version
project(cameo-mcstas VERSION 0.10.0 LANGUAGES CXX)
enable_testing()

include(GNUInstallDirs)

# specify the C++ standard
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED False)

# Want to compile dynamic or static libraries
set(BUILD_SHARED_LIBS False)

#add_compile_options(-Wall -Wextra -pedantic -Werror)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Werror")

# where to find the header files
include_directories(include include/nlohmann)


# definitions for the client and server 
add_definitions(-DSERVERNAME="mcstas_server-dev")


add_subdirectory(client_api)
#------------------------------------------------------------
# Libraries
#------------------------------------------------------------
add_library(sim_server  src/sim_request_server.cc src/sim_result_server.cc)  # server
target_link_libraries(sim_server sim_client)

#------------------------------------------------------------
# Executables
#------------------------------------------------------------
if(SERVER)
find_package(libmongocxx 3.2.0 REQUIRED)
message(${LIBMONGOCXX_INCLUDE_DIRS})
include_directories(${LIBMONGOCXX_INCLUDE_DIRS})  # this should be avoided

  # Client
add_executable(fakeNomad src/fakeNomad.cpp)
target_link_libraries(fakeNomad PRIVATE cameo)
target_link_libraries(fakeNomad PRIVATE stdc++fs)
target_link_libraries(fakeNomad PRIVATE m zmq protobuf)
target_link_libraries(fakeNomad PRIVATE sim_client)

# Server
add_executable(mcstas_server src/mcstas_server.cpp)
target_include_directories(mcstas_server SYSTEM INTERFACE ${LIBMONGOCXX_INCLUDE_DIRS})
target_link_libraries(mcstas_server PRIVATE cameo)
target_link_libraries(mcstas_server PRIVATE m zmq protobuf)
target_link_libraries(mcstas_server PRIVATE stdc++fs)
target_link_libraries(mcstas_server PRIVATE sim_server)
target_compile_options(mcstas_server PRIVATE -std=c++17)
target_link_libraries(mcstas_server PRIVATE mongocxx bsoncxx)

if(DO_INSTRUMENTS)
  add_subdirectory(mcstas)
  add_test(NAME d22 COMMAND ILL_H512_D22.out -n 10000 lambda=4.5)
endif(DO_INSTRUMENTS)
endif(SERVER)

################ Install
#set(CMAKE_INSTALL_PREFIX /tmp/devel/)
#install(TARGETS fakeNomad mcstas_server ILL_H512_D22-sSAMPLE.out ILL_H512_D22.out DESTINATION bin)
if(SERVER)
  install(TARGETS fakeNomad mcstas_server RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}) 

  if (BUILD_SHARED_LIBS)
	install(TARGETS  sim_server  LIBRARY DESTINATION lib ARCHIVE DESTINATION obj)
  endif(BUILD_SHARED_LIBS)

endif(SERVER)
  

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


