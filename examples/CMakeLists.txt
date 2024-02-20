#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

# Function that generates shim code for given target
# erpc_file - path to the erpc IDL file
# output_name - name of the output files ({output_name}_interface.cpp, ${output_name}_client.cpp, ...)
# output_dir - path where shim code will be generated
function(generate_shim erpc_file output_name output_dir)
    set(SHIM_FILES
        ${output_dir}/${output_name}_interface.cpp
        ${output_dir}/${output_name}_client.cpp
        ${output_dir}/${output_name}_server.cpp
        ${output_dir}/c_${output_name}_client.cpp
        ${output_dir}/c_${output_name}_server.cpp
    )

    # The custom command
    add_custom_command(
        OUTPUT ${SHIM_FILES}
        COMMAND ${ERPCGEN} -g c -o ${output_dir} ${CMAKE_CURRENT_SOURCE_DIR}/${target_name}.erpc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${target_name}.erpc
    )
endfunction()

# All examples using CMake
add_subdirectory(cmake_example)
