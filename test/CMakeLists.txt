#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

message(STATUS "Adding gtest...")
add_library(gtest STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common/gtest/gtest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/gtest/gtimer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common/gtest/gtest.h
)

set(ERPC_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/erpc_output)

function(generate_shim erpc_file output_name output_dir)
    message(STATUS "Generating shim: ${erpc_file} -> ${output_dir}")
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
        COMMAND ${ERPCGEN} -g c -o ${output_dir} ${erpc_file}
        DEPENDS ${erpc_file}
    )
endfunction()

# All test using CMake
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test_arrays)
