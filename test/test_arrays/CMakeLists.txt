#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

set(TEST_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(TEST_COMMON_DIR ${TEST_ROOT_DIR}/common)

get_filename_component(TEST_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

message(STATUS "Test name: ${TEST_NAME}")

generate_shim(${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.erpc test ${ERPC_OUT_DIR}/${TEST_NAME})

add_executable(${TEST_NAME}_client
    ${TEST_COMMON_DIR}/unit_test_tcp_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_arrays_client_impl.cpp

    ${ERPC_OUT_DIR}/${TEST_NAME}/test_client.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/test_interface.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/c_test_client.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/test_unit_test_common_client.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/c_test_unit_test_common_client.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/test_unit_test_common_interface.cpp

    ${ERPC_ROOT}/erpcgen/src/format_string.cpp
    ${ERPC_ROOT}/erpcgen/src/Logging.cpp
)

add_executable(${TEST_NAME}_server
    ${TEST_COMMON_DIR}/unit_test_tcp_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_arrays_server_impl.cpp

    ${ERPC_OUT_DIR}/${TEST_NAME}/test_server.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/test_interface.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/c_test_server.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/test_unit_test_common_server.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/test_unit_test_common_interface.cpp
    ${ERPC_OUT_DIR}/${TEST_NAME}/c_test_unit_test_common_server.cpp

    ${ERPC_ROOT}/erpcgen/src/format_string.cpp
    ${ERPC_ROOT}/erpcgen/src/Logging.cpp
)

target_include_directories(${TEST_NAME}_client PRIVATE
    ${ERPC_OUT_DIR}/${TEST_NAME}
    ${TEST_COMMON_DIR}
    ${TEST_COMMON_DIR}/config
    ${TEST_COMMON_DIR}/gtest
    ${ERPC_ROOT}/erpcgen/src
)

target_include_directories(${TEST_NAME}_server PRIVATE
    ${ERPC_OUT_DIR}/${TEST_NAME}
    ${TEST_COMMON_DIR}
    ${TEST_COMMON_DIR}/config
    ${TEST_COMMON_DIR}/gtest
    ${ERPC_ROOT}/erpcgen/src
)

target_link_libraries(${TEST_NAME}_client PRIVATE gtest erpc)
target_link_libraries(${TEST_NAME}_server PRIVATE gtest erpc)