cmake_minimum_required(VERSION 3.7.2)
project(instruments
  VERSION 0.10.0
  LANGUAGES C
  )

include(GNUInstallDirs)

#string(TOUPPER ${PROJECT_NAME}_RUNTIME component_runtime)
#string(TOUPPER ${PROJECT_NAME}_DEVELOPMENT component_development)
string(TOUPPER ${PROJECT_NAME} component_instruments)
set(component_instruments ${component_instruments} PARENT_SCOPE)

# mcstas code generates plenty of warnings!!! 
set(CMAKE_C_FLAGS "-O3 -Wno-error -Wno-all -Wno-extra -Wno-unused-result -Wno-format-truncation -Wno-format-overflow -Wno-format")
set(MCSTAS $ENV{MCSTAS} CACHE PATH "Path of the mcstas installation [MANDATORY]")
############################# MCSTAS
message(STATUS "MCSTAS: ${MCSTAS}")
#include_directories(/usr/share/mcstas/2.6/libs/mcpl)
#link_directories(/usr/share/mcstas/2.6/libs/mcpl/)
include_directories(${MCSTAS}/libs/mcpl)
link_directories(${MCSTAS}/libs/mcpl/)

#------------------------------------------------------------
# use mcstas to generate .c from the instrument file
#------------------------------------------------------------
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/mcstas)

function(run_mcstas outname )
  add_custom_command(OUTPUT ${outname}.c
	COMMENT "creation of ${outname}.c"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${outname}.instr ${CMAKE_CURRENT_SOURCE_DIR}/MCPL_input.comp
	COMMAND mcstas -t -o ${CMAKE_CURRENT_BINARY_DIR}/${outname}.c -I ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/${outname}.instr 
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
	)
  add_executable(${outname}.out ${outname}.c)
  target_link_libraries(${outname}.out PUBLIC m mcpl)
endfunction()

run_mcstas(ILL_D22_quick_merge)
run_mcstas(ILL_D22_quick)
run_mcstas(ILL_H512_D22-sSAMPLE)
run_mcstas(ILL_H512_D22)
run_mcstas(ILL_source_simple_test)

install(TARGETS  ILL_H512_D22-sSAMPLE.out ILL_H512_D22.out ILL_D22_quick.out ILL_D22_quick_merge.out
  EXPORT cameo_mcstas_instruments_targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT ${component_instruments} 
  )


#install(EXPORT cameo_mcstas_instruments_targets
#  FILE ${PROJECT_NAME}-targets.cmake
#  NAMESPACE ${NAMESPACE}::
#  DESTINATION ${ConfigPackageLocation}
#  COMPONENT ${component_instruments}
#  )
#export(EXPORT cameo_mcstas_instruments_targets
#  FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake
#  NAMESPACE ${NAMESPACE}::
#  )

if(ENABLE_TESTING)
  add_test(test_ILL_D22  ILL_H512_D22.out  lambda=4.51 D22_collimation=2.0 -s 554321 -n 200000 sample_size_r=0.010 sample_size_y=0.001)
  set_tests_properties(test_ILL_D22 PROPERTIES LABELS mcstas)
endif(ENABLE_TESTING)
