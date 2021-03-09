cmake_minimum_required(VERSION 3.7.2)
project(mcstas_instruments VERSION 0.10.0 LANGUAGES C)
include(GNUInstallDirs)

# mcstas code generates plenty of warnings!!! 
set(CMAKE_C_FLAGS "-O3 -Wno-error -Wno-all -Wno-extra -Wno-unused-result -Wno-format-truncation -Wno-format-overflow -Wno-format")

############################# MCSTAS
message(STATUS "MCSTAS: ${MCSTAS}")
#include_directories(/usr/share/mcstas/2.6/libs/mcpl)
#link_directories(/usr/share/mcstas/2.6/libs/mcpl/)
include_directories(${MCSTAS}/libs/mcpl)
link_directories(${MCSTAS}/libs/mcpl/)
#------------------------------------------------------------
# use mcstas to generate .c from the instrument file
#------------------------------------------------------------
#message("${CMAKE_BINARY_DIR}")
#message("${CMAKE_BUILD_DIRECTORY}")
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/mcstas)

add_custom_command(OUTPUT ILL_D22_quick_merge.c
  COMMENT "creation of ILL_D22_quick_merge.c"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ILL_D22_quick_merge.instr ${CMAKE_CURRENT_SOURCE_DIR}/MCPL_input.comp
  COMMAND mcstas -t -o ${CMAKE_CURRENT_BINARY_DIR}/ILL_D22_quick_merge.c -I ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/ILL_D22_quick_merge.instr 
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
  )
add_executable(ILL_D22_quick_merge.out ILL_D22_quick_merge.c)
target_link_libraries(ILL_D22_quick_merge.out PUBLIC m mcpl)
  

add_custom_command(OUTPUT ILL_D22_quick.c
  COMMENT "creation of ILL_D22_quick.c"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ILL_D22_quick.instr
  COMMAND mcstas -t -o ${CMAKE_CURRENT_BINARY_DIR}/ILL_D22_quick.c -I ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/ILL_D22_quick.instr 
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
  )
#
add_custom_command(OUTPUT ILL_H512_D22-sSAMPLE.c
  COMMENT "creation of ILL_H512_D22-sSAMPLE.c"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H512_D22-sSAMPLE.instr
#  COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ILL_source.instr ${CMAKE_BINARY_DIR}/mcstas/ILL_source.instr
#  COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H51.instr ${CMAKE_BINARY_DIR}/mcstas/ILL_H51.instr
  COMMAND mcstas -t -o ${CMAKE_CURRENT_BINARY_DIR}/ILL_H512_D22-sSAMPLE.c -I ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H512_D22-sSAMPLE.instr 
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/mcstas
  )
#
add_custom_command(OUTPUT ILL_H512_D22.c
  COMMENT "creation of mcstas ILL_H512_D22.c"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H512_D22.instr ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H512.instr ${CMAKE_CURRENT_SOURCE_DIR}/ILL_source.instr ${CMAKE_CURRENT_SOURCE_DIR}/MCPL_output.comp
#  COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ILL_source.instr ${CMAKE_BINARY_DIR}/mcstas/ILL_source.instr
#  COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H51.instr ${CMAKE_BINARY_DIR}/mcstas/ILL_H51.instr
#  COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H512.instr ${CMAKE_BINARY_DIR}/mcstas/ILL_H512.instr
#  COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/MCPL_output.comp ${CMAKE_BINARY_DIR}/mcstas/MCPL_output.comp
  COMMAND mcstas -t -o ${CMAKE_CURRENT_BINARY_DIR}/ILL_H512_D22.c -I ${CMAKE_CURRENT_SOURCE_DIR}/ -I ${CMAKE_CURRENT_SOURCE_DIR}/mcstas/ ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H512_D22.instr 
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/mcstas
  )
#

add_custom_command(OUTPUT ILL_source_simple_test.c
  COMMENT "creation of ILL_source_simple_test.c"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ILL_source_simple_test.instr
  COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ILL_source_simple.instr ${CMAKE_BINARY_DIR}/mcstas/ILL_source_simple.instr
  COMMAND mcstas -t -o ${CMAKE_CURRENT_BINARY_DIR}/ILL_source_simple_test.c ${CMAKE_CURRENT_SOURCE_DIR}/ILL_source_simple_test.instr 
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/mcstas
  )

add_executable(ILL_D22_quick.out ILL_D22_quick.c)
target_link_libraries(ILL_D22_quick.out PUBLIC m mcpl)

add_executable(ILL_H512_D22-sSAMPLE.out ILL_H512_D22-sSAMPLE.c)
target_link_libraries(ILL_H512_D22-sSAMPLE.out PUBLIC m mcpl)

add_executable(ILL_H512_D22.out ILL_H512_D22.c)
target_link_libraries(ILL_H512_D22.out PUBLIC m mcpl)
# target_compile_options(ILL_H512_D22.out PUBLIC -O3 -Wno-error -Wno-all -Wno-extra)
# target_include_directories(ILL_H512_D22.out PUBLIC /usr/share/mcstas/2.6/libs/mcpl)

add_executable(ILL_source_simple_test.out ILL_source_simple_test.c)
target_link_libraries(ILL_source_simple_test.out PUBLIC m)

install(TARGETS  ILL_H512_D22-sSAMPLE.out ILL_H512_D22.out ILL_D22_quick.out ILL_D22_quick_merge.out
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

add_test(test_ILL_D22  ILL_H512_D22.out  lambda=4.51 D22_collimation=2.0 -s 554321 -n 200000 sample_size_r=0.010 sample_size_y=0.001)
set_tests_properties(test_ILL_D22 PROPERTIES LABELS mcstas)
