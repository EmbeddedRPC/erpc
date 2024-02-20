#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

message(STATUS "Creating gtest library")
add_library(gtest STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common/gtest/gtest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/gtest/gtimer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common/gtest/gtest.h
)

set(ERPC_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/erpc_outputs)
set(TEST_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)

function(generate_shim erpc_file output_name output_dir)
    # message(STATUS "Generating shim: ${erpc_file} -> ${output_dir}")
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
        COMMENT "Generating ..."
    )
endfunction()

function(create_unit_test_target test_name)

    generate_shim(${CMAKE_CURRENT_SOURCE_DIR}/${test_name}/${test_name}.erpc test ${ERPC_OUT_DIR}/${test_name})
    generate_shim(${CMAKE_CURRENT_SOURCE_DIR}/${test_name}/${test_name}.erpc test_unit_test_common ${ERPC_OUT_DIR}/${test_name})

    set(test_output_folder ${ERPC_OUT_DIR}/${test_name}/erpc_outputs)

    set(unit_test_include_dirs
        ${test_output_folder}
        ${TEST_COMMON_DIR}
        ${TEST_COMMON_DIR}/config
        ${TEST_COMMON_DIR}/gtest
        ${ERPC_ROOT}/erpcgen/src
    )

    add_executable(${test_name}_client
        ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}/${test_name}_client_impl.cpp

        ${TEST_COMMON_DIR}/unit_test_tcp_client.cpp

        ${test_output_folder}/test_interface.cpp
        ${test_output_folder}/test_unit_test_common_interface.cpp

        ${test_output_folder}/test_client.cpp
        ${test_output_folder}/test_unit_test_common_client.cpp
        
        ${test_output_folder}/c_test_client.cpp
        ${test_output_folder}/c_test_unit_test_common_client.cpp

        ${ERPC_ROOT}/erpcgen/src/format_string.cpp
        ${ERPC_ROOT}/erpcgen/src/Logging.cpp
    )

    add_executable(${test_name}_server
        ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}/${test_name}_server_impl.cpp

        ${TEST_COMMON_DIR}/unit_test_tcp_server.cpp

        ${test_output_folder}/test_interface.cpp
        ${test_output_folder}/test_unit_test_common_interface.cpp

        ${test_output_folder}/test_server.cpp
        ${test_output_folder}/test_unit_test_common_server.cpp
        
        ${test_output_folder}/c_test_server.cpp
        ${test_output_folder}/c_test_unit_test_common_server.cpp

        ${ERPC_ROOT}/erpcgen/src/format_string.cpp
        ${ERPC_ROOT}/erpcgen/src/Logging.cpp
    )

    target_include_directories(${test_name}_client PRIVATE ${unit_test_include_dirs})
    target_include_directories(${test_name}_server PRIVATE ${unit_test_include_dirs})

    target_link_libraries(${test_name}_client PRIVATE gtest erpc)
    target_link_libraries(${test_name}_server PRIVATE gtest erpc)

endfunction()

# All test using CMake
# create_unit_test_target(test_annotations)
# create_unit_test_target(test_arbitrator)
# create_unit_test_target(test_arrays)
create_unit_test_target(test_binary)
create_unit_test_target(test_builtin)
# create_unit_test_target(test_callbacks)
create_unit_test_target(test_const)
create_unit_test_target(test_enums)
create_unit_test_target(test_lists)
create_unit_test_target(test_shared)
# create_unit_test_target(test_struct)
create_unit_test_target(test_typedef)
create_unit_test_target(test_unions)
