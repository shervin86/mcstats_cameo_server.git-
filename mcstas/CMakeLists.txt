project(mcstas_instruments VERSION 0.10.0 LANGUAGES C)

# mcstas code generates plenty of warnings!!! 
set(CMAKE_C_FLAGS "-O3 -Wno-error -Wno-all -Wno-extra -Wno-unused-result -Wno-format-truncation -Wno-format-overflow -Wno-format")

############################# MCSTAS
include_directories(/usr/share/mcstas/2.6/libs/mcpl)
link_directories(/usr/share/mcstas/2.6/libs/mcpl/)

#------------------------------------------------------------
# use mcstas to generate .c from the instrument file
#------------------------------------------------------------
add_custom_command(OUTPUT ILL_H512_D22-sSAMPLE.c
  COMMENT "creation of mcstas .c"
  COMMAND mcstas -t -o build/mcstas/ILL_H512_D22-sSAMPLE.c mcstas/ILL_H512_D22-sSAMPLE.instr
  WORKING_DIRECTORY ../../
  )

add_custom_command(OUTPUT ILL_H512_D22.c
  COMMENT "creation of mcstas .c"
  COMMAND mcstas -t -o build/mcstas/ILL_H512_D22.c mcstas/ILL_H512_D22.instr
  WORKING_DIRECTORY ../../
  )


add_executable(ILL_H512_D22-sSAMPLE.out ILL_H512_D22-sSAMPLE.c)
add_executable(ILL_H512_D22.out ILL_H512_D22.c)
target_link_libraries(ILL_H512_D22-sSAMPLE.out PUBLIC m mcpl)

# target_compile_options(ILL_H512_D22.out PUBLIC -O3 -Wno-error -Wno-all -Wno-extra)
# target_include_directories(ILL_H512_D22.out PUBLIC /usr/share/mcstas/2.6/libs/mcpl)
target_link_libraries(ILL_H512_D22.out PUBLIC m mcpl)
