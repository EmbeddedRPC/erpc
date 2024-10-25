#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# To optimize build process, for each erpc_config.h file used, generate only one library. 
# This function compute MD5 of given config file and base on this hash check if target library exist,
# otherwise create new library with hash postfix 
function(erpc_link_c_library)
    cmake_parse_arguments(_ "" "TARGET;CONFIG_DIR;TRANSPORT" "" ${ARGN})
    

	include(${ERPC_C}/erpc_c_sources.cmake)
	
    if(NOT __TARGET)
        message(FATAL_ERROR "Not target specified.")
    endif()
    
    
    if(NOT __CONFIG_DIR)
        set(__CONFIG_DIR ${ERPC_C}/config)
    endif()
    
    file(MD5 ${__CONFIG_DIR}/erpc_config.h CONFIG_HASH)

    if(NOT TARGET erpc_c_lib_${CONFIG_HASH})
        add_library(erpc_c_lib_${CONFIG_HASH} ${ERPC_C_SOURCES})
        target_include_directories(erpc_c_lib_${CONFIG_HASH} PUBLIC ${ERPC_C_INCLUDES} ${__CONFIG_DIR})

		# Required for TCP transport
		if(WIN32)
			target_link_libraries(erpc_c_lib_${CONFIG_HASH} PUBLIC wsock32 ws2_32)
		endif()

        target_link_libraries(erpc_c_lib_${CONFIG_HASH} PUBLIC Threads::Threads)
    endif()

    find_package(Threads REQUIRED)
	target_link_libraries(${__TARGET} PRIVATE erpc_c_lib_${CONFIG_HASH})
endfunction()

function(erpc_generate_shim_code)
    cmake_parse_arguments(_ "" "IDL_FILE;OUTPUT_DIR;LANGUAGE;WORKING_DIRECTORY" "OUTPUT_FILES" ${ARGN})

    if(NOT ERPCGEN_EXECUTABLE)
        message(FATAL_ERROR "erpcgen executable not found. Enable CONFIG_ERPC_GENERATOR in Kconfig or provide erpcgen in PATH.")
    endif()

    add_custom_command(
        OUTPUT ${__OUTPUT_FILES}
        COMMAND ${ERPCGEN_EXECUTABLE} -g c -o ${__OUTPUT_DIR} ${__IDL_FILE}
        WORKING_DIRECTORY ${__WORKING_DIRECTORY}
        DEPENDS ${__IDL_FILE}
        COMMENT "erpcgen:  ${ERPCGEN_EXECUTABLE} -g c -o ${__OUTPUT_DIR} ${__IDL_FILE}"
    )
endfunction()