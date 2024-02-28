#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Function 'generate_erpc_files' generates erpc files based on the idl file to the output directory.
# It is required to specify the output file for CMake to correctly generate dependencies. 
#
#   generate_erpc_files(IDL_FILE idl_file
#                       OUTPUT_DIR output_dir
#                       OUTPUT_FILES [files...]
#                       [WORKING_DIRECTORY working_directory] # default: CMAKE_CURRENT_SOURCE_DIR
#                       [PROGRAMMING_LANGUAGE programming_language]) # default: c
#

function(generate_erpc_files)
    # Set function's parameter
    set(options )
    set(one_value_args IDL_FILE OUTPUT_DIR PROGRAMMING_LANGUAGE WORKING_DIRECTORY)
    set(multi_value_args OUTPUT_FILES)

    # Parse them
    cmake_parse_arguments(ARG "${options}" "${one_value_args}"
                            "${multi_value_args}" ${ARGN} )

    # Set defaults
    if(NOT ARG_WORKING_DIRECTORY)
        set(ARG_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    if(NOT ARG_PROGRAMMING_LANGUAGE)
        set(ARG_PROGRAMMING_LANGUAGE "c")
    endif()

    # Create custom target
    add_custom_command(
        OUTPUT ${ARG_OUTPUT_FILES}
        COMMAND ${ERPCGEN} -g ${ARG_PROGRAMMING_LANGUAGE} -o ${ARG_OUTPUT_DIR} ${ARG_IDL_FILE}
        WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
        # DEPENDS ${ERPCGEN} ensure that the erpcgen target is built before any use of this function, if necessary
        DEPENDS ${ARG_IDL_FILE} ${ERPCGEN}
        COMMENT "Generating eRPC files from: ${ARG_IDL_FILE}"
    )

endfunction()