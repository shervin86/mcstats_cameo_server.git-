project(mcstas_instruments VERSION 0.10.0 LANGUAGES C)

# mcstas code generates plenty of warnings!!! 
set(CMAKE_C_FLAGS "-O3 -Wno-error -Wno-all -Wno-extra -Wno-unused-result -Wno-format-truncation -Wno-format-overflow -Wno-format")

############################# MCSTAS
include_directories(/usr/share/mcstas/2.6/libs/mcpl)
link_directories(/usr/share/mcstas/2.6/libs/mcpl/)

#------------------------------------------------------------
# use mcstas to generate .c from the instrument file
#------------------------------------------------------------
message("${CMAKE_BINARY_DIR}")
message("${CMAKE_BUILD_DIRECTORY}")
add_custom_command(OUTPUT ILL_H512_D22-sSAMPLE.c
  COMMENT "creation of ILL_H512_D22-sSAMPLE.c"
  COMMAND mcstas -t -o ${CMAKE_BINARY_DIR}/mcstas/ILL_H512_D22-sSAMPLE.c ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H512_D22-sSAMPLE.instr 
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
#
add_custom_command(OUTPUT ILL_H512_D22.c
  COMMENT "creation of mcstas ILL_H512_D22.c"
  COMMAND mcstas -t -o ${CMAKE_BINARY_DIR}/mcstas/ILL_H512_D22.c ${CMAKE_CURRENT_SOURCE_DIR}/ILL_H512_D22.instr 
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
#

add_executable(ILL_H512_D22-sSAMPLE.out ILL_H512_D22-sSAMPLE.c)
target_link_libraries(ILL_H512_D22-sSAMPLE.out PUBLIC m mcpl)

add_executable(ILL_H512_D22.out ILL_H512_D22.c)
target_link_libraries(ILL_H512_D22.out PUBLIC m mcpl)
# target_compile_options(ILL_H512_D22.out PUBLIC -O3 -Wno-error -Wno-all -Wno-extra)
# target_include_directories(ILL_H512_D22.out PUBLIC /usr/share/mcstas/2.6/libs/mcpl)


install(TARGETS  ILL_H512_D22-sSAMPLE.out ILL_H512_D22.out  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}) 
add_test(test_ILL_D22  ILL_H512_D22.out  lambda=4.5 -s 554321 -n 5e5 --no-output-file)
