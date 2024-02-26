#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

include(${ERPC_TEST_ROOT}/variables.cmake)

set(ERPC_NAME_APP test_core0)

# Disable shim generation
set(GENERATE_ERPC_IDL_FILES FALSE)

# Define required IDL files
set(TEST_IDL_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_client.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_server.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_core1_interface.cpp
    
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_server.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_client.cpp
)

# Generate 
add_custom_command(
    OUTPUT ${TEST_IDL_FILES}
    COMMAND ${ERPCGEN} -g c -o ${ERPC_OUT_ROOT_DIR} ${IDL_FILE}
    DEPENDS ${IDL_FILE}
    COMMENT "Generating IDL files"
)

set(TEST_CLIENT_SOURCES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_client.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_core1_interface.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_client.cpp

    ${CURRENT_DIR}/${CURRENT_DIR_NAME}_client_impl.cpp
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_client.cpp

    ${ERPC_ROOT}/erpcgen/src/format_string.cpp
    ${ERPC_ROOT}/erpcgen/src/Logging.cpp
)

set(TEST_CLIENT_INCLUDES
    ${TEST_OUT_DIR}
    ${TEST_ERPC_OUT_DIR}
    ${TEST_COMMON_DIR}
    ${TEST_COMMON_DIR}/config
    ${TEST_COMMON_DIR}/gtest
    ${ERPC_ROOT}/erpcgen/src
)

set(TEST_SERVER_SOURCES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_server.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_core1_interface.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_server.cpp

    ${CURRENT_DIR}/${CURRENT_DIR_NAME}_server_impl.cpp
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_server.cpp
    ${ERPC_ROOT}/erpcgen/src/format_string.cpp
    ${ERPC_ROOT}/erpcgen/src/Logging.cpp
)

set(TEST_SERVER_INCLUDES
    ${TEST_OUT_DIR}
    ${TEST_ERPC_OUT_DIR}
    ${TEST_COMMON_DIR}
    ${TEST_COMMON_DIR}/config
    ${TEST_COMMON_DIR}/gtest
    ${ERPC_ROOT}/erpcgen/src
)

include(${ERPC_TEST_ROOT}/test.cmake)

