# TI cgt-arm_5.2.9 cmake toolchain script
#
# This file is based on the cmake-toolchains(7) documentation.

# project(lwip C)
# Set the operating system and processor architecture
set(CMAKE_SYSTEM_NAME Generic)
# set(CMAKE_SYSTEM_PROCESSOR arm)
# set(CMAKE_SYSTEM_VERSION 1)

# Setup CMake's rules for using the CMAKE_FIND_ROOT_PATH for cross-compilation
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# TI cgtools command line programs
set(SDK_PATH "/Applications/ti_ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.1.LTS")
set(SDK_BIN "${SDK_PATH}/bin")

# This compiler doesn't work with CMAKE_SYSROOT, but we do need to set our
# CMAKE_FIND_ROOT_PATH. Note that setting a variable will override the value in
# the cache if the CACHE option was not used, so if we want to be able to use a
# CMAKE_FIND_ROOT_PATH passed to cmake via the command line, we must make sure
# not to overwrite any value that was already set.
if(NOT CMAKE_FIND_ROOT_PATH)
  set(CMAKE_FIND_ROOT_PATH ${SDK_PATH})
endif()

set(CMAKE_C_COMPILER "${SDK_BIN}/armcl")
set(CMAKE_CXX_COMPILER "${SDK_BIN}/armcl")
set(CMAKE_ASM_COMPILER "${SDK_BIN}/armasm")
set(CMAKE_AR "${SDK_BIN}/armar")
# SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -rq  <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_LINKER "${SDK_BIN}/armlnk")

# string(CONCAT ARCH_C_FLAGS
#   "-mv=7A8 --abi=eabi -me --float_support=VFPv3 --neon --printf_support=full")
# set(CMAKE_C_FLAGS_INIT "${ARCH_C_FLAGS}")
# set(CMAKE_CXX_FLAGS_INIT "${ARCH_C_FLAGS}")

# Normally, cmake checks if the compiler can compile and link programs. For
# TI's cgtools the link step doesn't work, and there seems to be no easy way
# to fix this. Instead, we simply disable the compiler checks for C and C++.
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_ASM_COMPILER_WORKS 1) 
set(CMAKE_DETERMINE_ASM_ABI_COMPILED 1) 
set(CMAKE_DETERMINE_C_ABI_COMPILED   1) 
set(CMAKE_DETERMINE_CXX_ABI_COMPILED 1) 


# Add the default include and lib directories for tool chain
include_directories(${SDK_PATH}/include)
link_directories(${SDK_PATH}/lib)
set(PLATFORM_CONFIG_C_FLAGS "--cmd_file=../CC1352P1_LAUNCHXL_TIRTOS.cmd -mv7M4  --abi=eabi -me -O2 -g --include_path=${SDK_PATH/}include --gcc" CACHE STRING "platform config c flags")

# set flags to C flags
SET(CMAKE_C_FLAGS "${PLATFORM_CONFIG_C_FLAGS} ${CMAKE_C_FLAGS}" CACHE STRING "platform config c flags")

# SET(CMAKE_EXE_LINKER_FLAGS "-mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=DeviceFamily_CC13X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --rom_model")


set(CMAKE_EXE_LINKER_FLAGS_INIT  "--diag_warning=225  --diag_wrap=off --display_error_number  --warn_sections--xml_link_info=gpiointerrupt_CC1352P1_LAUNCHXL_tirtos_ccs_linkInfo.xml --rom_model")
 



