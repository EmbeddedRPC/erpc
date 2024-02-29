#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

include(${ERPC_TEST_ROOT}/variables.cmake)

set(TEST_ERPC_OUT_DIR ${ERPC_OUT_ROOT_DIR}/${ERPC_OUT_DIR})

# Disable shim generation
set(GENERATE_ERPC_IDL_FILES FALSE)

# Define required IDL files
set(TEST_IDL_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_client.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_secondInterface_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_secondInterface_server.cpp
    
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_client.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_firstInterface_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_firstInterface_server.cpp
)

# Generate 
add_custom_command(
    OUTPUT ${TEST_IDL_FILES}
    COMMAND ${ERPCGEN} -g c -o ${ERPC_OUT_ROOT_DIR} ${IDL_FILE}
    DEPENDS ${IDL_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating IDL files"
)

set(TEST_CLIENT_SOURCES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_interface.cpp

    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_secondInterface_server.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_server.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_firstInterface_client.cpp

    ${CURRENT_DIR}/${CURRENT_DIR_NAME}_client_impl.cpp
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_arbitrator_client.cpp
    
    ${ERPC_ROOT}/erpcgen/src/format_string.cpp
    ${ERPC_ROOT}/erpcgen/src/Logging.cpp
)

set(TEST_CLIENT_INCLUDES
    ${TEST_OUT_DIR}
    ${TEST_ERPC_OUT_DIR}
    ${CURRENT_DIR}/config
    ${TEST_COMMON_DIR}
    ${TEST_COMMON_DIR}/gtest
    ${ERPC_ROOT}/erpcgen/src
)

set(TEST_SERVER_SOURCES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_interface.cpp

    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_secondInterface_client.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_client.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_firstInterface_server.cpp

    ${CURRENT_DIR_NAME}_server_impl.cpp
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_arbitrator_server.cpp
    ${ERPC_ROOT}/erpcgen/src/format_string.cpp
    ${ERPC_ROOT}/erpcgen/src/Logging.cpp
)

set(TEST_SERVER_INCLUDES
    ${TEST_OUT_DIR}
    ${TEST_ERPC_OUT_DIR}
    ${CURRENT_DIR}/config
    ${TEST_COMMON_DIR}
    ${TEST_COMMON_DIR}/gtest
    ${ERPC_ROOT}/erpcgen/src
)

include(${ERPC_TEST_ROOT}/test.cmake)

