project(sim_client_api VERSION 0.10.0 LANGUAGES CXX)

add_library(sim_client src/sim_request.cc src/sim_result.cc)


# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(sim_client PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:sim_client/include>
  PRIVATE src)


# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS sim_client EXPORT ${PROJECT_NAME}
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT ${PROJECT_NAME} DESTINATION share/${PROJECT_NAME}/cmake)

# This makes the project importable from the build directory
export(TARGETS sim_client FILE ${PROJECT_NAME}.cmake)

# Every library has unit tests, of course
#add_executable(testlib
#    test/testlib.cpp)

#target_link_libraries(testlib
#    lib)

#add_test(testlib testlib)
