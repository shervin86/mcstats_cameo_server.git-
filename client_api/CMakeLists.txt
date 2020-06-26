project(sim_client_api VERSION 0.10.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_library(sim_client src/sim_request.cc src/sim_result.cc)


# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(sim_client PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${PROJECT_NAME}/include>
  PRIVATE src)


# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS sim_client EXPORT ${PROJECT_NAME}
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT ${PROJECT_NAME} DESTINATION share/${PROJECT_NAME}/cmake)

# This makes the project importable from the build directory
export(TARGETS sim_client FILE ${PROJECT_NAME}.cmake)

# Every library has unit tests, of course
add_executable(testclient ./test/client.cpp)
target_include_directories(testclient PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../doctest/doctest/>)
target_link_libraries(testclient sim_client)

enable_testing()
add_test(testclient testclient )
add_test(test_json  diff testclient.json request.json)
set_tests_properties(testclient test_json PROPERTIES LABELS client)


add_custom_command(
  TARGET testclient POST_BUILD
  COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/request.json ${CMAKE_CURRENT_BINARY_DIR}/request.json
  COMMENT "Copying request.json in the current directory for running the test")
