cmake_minimum_required(VERSION 3.20.0)

# Project name and version
project(cameo-mcstas
  VERSION 0.14.2
  LANGUAGES CXX
  HOMEPAGE_URL "https://code.ill.fr/nourbakhsh/mcstats_cameo_server"
  )

#--------------- Helpers
include(GNUInstallDirs)
include(FetchContent)

set(NAMESPACE Panosc)
set(ConfigPackageLocation ${CMAKE_INSTALL_DATADIR}/cmake/${CMAKE_PROJECT_NAME})
string(TOUPPER server component_server)


# Want to compile dynamic or static libraries
option(BUILD_SHARED_LIBS "Compiling shared or static library version of the api" ON)
#set(BUILD_SHARED_LIBS True)

option(ENABLE_TESTING "Enable testing" OFF)
option(USE_MONGO "Use MongoDB to store requests and results" OFF)
option(DO_INSTRUMENTS "Compile the instruments with mcstas" OFF)
option(SERVER "Build the server and the instruments" OFF)
if(SERVER)
  set(BUILD_SHARED_LIBS False)
  set(DO_INSTRUMENTS OFF)
endif(SERVER)

if(NOT DEFINED ${CMAKE_BUILD_TYPE})
  set(CMAKE_BUILD_TYPE "Release") # set Release by default
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(ENABLE_TESTING)
  enable_testing()
endif()
# fix the RPATH for the linker
if(NOT APPLE)
  set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()

#add_compile_options(-Wall -Wextra -pedantic -Werror)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Werror")

#list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
# where to find the header files



if(ENABLE_TESTING)
  add_definitions(-DDOCTEST=ON)
	find_package(doctest 1.2.9 QUIET)
	if(NOT doctest_FOUND)
	  message(WARNING "Using Doctest from GIT repository")
	  FetchContent_Declare(doctest
		GIT_REPOSITORY https://github.com/onqtam/doctest.git
		GIT_TAG 1.2.9
		GIT_SHALLOW True
		CMAKE_ARGS -DDOCTEST_WITH_TESTS=OFF -DDOCTEST_NO_INSTALL=ON
		)
	  
	  #	  FetchContent_MakeAvailable(doctest)
	  	  FetchContent_Populate(doctest)
	  
	  
		  add_library(doctest INTERFACE ${doctest_SOURCE_DIR}/doctest/doctest.h)
		  target_include_directories(doctest INTERFACE ${doctest_SOURCE_DIR}/doctest/)
		  add_library(doctest::doctest ALIAS doctest)
	endif()
	message(STATUS "doctest found here: ${doctest_SOURCE_DIR}")
	
  endif(ENABLE_TESTING)
include_directories(include include/nlohmann)


# definition for the fakeNomad client
add_definitions(-DSERVERNAME="mcstas_server-dev")
#add_definitions(-DSERVERNAME="mcstas_server")

add_subdirectory(client_api)
#------------------------------------------------------------
# Libraries
#------------------------------------------------------------
if(SERVER)
add_library(cameo_mcstas_server  src/sim_request_server.cc src/sim_result_server.cc src/sim_request_answer_server.cc)  # server
target_link_libraries(cameo_mcstas_server Panosc::cameo_mcstas_client)
#------------------------------------------------------------
# Executables
#------------------------------------------------------------
if(SERVER)
  find_package(cameo-api-cpp 2.0.0 REQUIRED)
  if(USE_MONGO)
    find_package(mongocxx 3.2.0 REQUIRED)
  endif()
  #message(${LIBMONGOCXX_INCLUDE_DIRS})
  #include_directories(${LIBMONGOCXX_INCLUDE_DIRS})  # this should be avoided
  
  find_package (Threads REQUIRED)
  
  # Client
  add_executable(fakeNomad src/fakeNomad.cpp)
  target_link_libraries(fakeNomad PRIVATE Cameo::cameo-api-cpp)
  target_link_libraries(fakeNomad PRIVATE m zmq)
  target_link_libraries(fakeNomad PRIVATE Panosc::cameo_mcstas_client)
  target_link_libraries(fakeNomad PRIVATE stdc++fs Threads::Threads)
  target_compile_features(fakeNomad PUBLIC cxx_std_17)
  
  # Server
  add_executable(mcstas_server src/mcstas_server.cpp)
  target_link_libraries(mcstas_server PRIVATE Cameo::cameo-api-cpp)
  target_link_libraries(mcstas_server PRIVATE m zmq)
  target_link_libraries(mcstas_server PRIVATE stdc++fs Threads::Threads)
  target_compile_features(mcstas_server PUBLIC cxx_std_17)
  target_link_libraries(mcstas_server PRIVATE cameo_mcstas_server)
  if(USE_MONGO)
    target_include_directories(mcstas_server SYSTEM INTERFACE ${LIBMONGOCXX_INCLUDE_DIRS})
    target_link_libraries(mcstas_server PRIVATE mongo::mongocxx_shared mongo::bsoncxx_shared)
  endif()

  
  if(DO_INSTRUMENTS)
    add_subdirectory(mcstas)
  endif(DO_INSTRUMENTS)

endif(SERVER)



################ Install
if(SERVER)
  install(TARGETS  mcstas_server #fakeNomad
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	COMPONENT ${component_server}) 
  
  #if (BUILD_SHARED_LIBS)
  #  install(TARGETS  cameo_mcstas_server  LIBRARY DESTINATION lib ARCHIVE DESTINATION obj)
  #endif(BUILD_SHARED_LIBS)

  set(XML_DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
  configure_file(bin/launch_server.sh bin/launch_server.sh @ONLY)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/launch_server.sh
	TYPE BIN
	COMPONENT ${component_server}
	)

  configure_file(share/cameo_config.xml share/mcstas_server.xml)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/mcstas_server.xml
	DESTINATION	${XML_DESTINATION}
	COMPONENT ${component_server}
	)
	
if(ENABLE_TESTING)
    add_executable(testserver ./test/testserver.cpp)
    target_include_directories(testserver PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
    #  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/doctest/doctest/>)
    target_link_libraries(testserver PRIVATE cameo_mcstas_server)
    target_link_libraries(testserver PRIVATE stdc++fs)
    target_compile_features(testserver PUBLIC cxx_std_17)
	target_link_libraries(testserver PRIVATE doctest::doctest)
	target_link_libraries(testserver PRIVATE Cameo::cameo-api-cpp)
    
    add_test(testserver testserver -nc)
    set_tests_properties(testserver PROPERTIES LABELS server)
  
    configure_file(test/cameo_config.xml.in test/mcstas_server.xml)
	configure_file(test/D22_Detector_0000000000.x_y test/D22_Detector_0000000000.x_y)
  endif(ENABLE_TESTING)
endif(SERVER)

install(FILES ${CMAKE_SOURCE_DIR}/LICENSE
  DESTINATION ${CMAKE_INSTALL_DOCDIR}/#${PROJECT_NAME}
  RENAME copyright
  COMPONENT ${component_server}
  )

#------------------------------------------------------------
# CPack
#------------------------------------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/packaging/CPackConfig.cmake)
include(CPack)

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


