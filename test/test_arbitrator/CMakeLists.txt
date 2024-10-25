#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

generate_erpc_test_variables()


set(TEST_ERPC_OUT_DIR ${ERPC_OUT_ROOT_DIR}/${ERPC_OUT_DIR})

# Arbitrator only support TCP transport
set(TRANSPORT "tcp")
set(CONFIG_DIR ${TEST_SOURCE_DIR}/config)

# Define required IDL files
set(TEST_ERPC_CLIENT_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_interface.cpp

    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_secondInterface_server.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_server.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_firstInterface_client.cpp
)

set(TEST_ERPC_SERVER_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_interface.cpp

    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_secondInterface_client.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_client.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_firstInterface_server.cpp
)

set(TEST_CLIENT_SOURCES
    ${TEST_ERPC_CLIENT_FILES}

    ${TEST_SOURCE_DIR}/${TEST_NAME}_client_impl.cpp
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_arbitrator_client.cpp
    
    ${ERPC_BASE}/erpcgen/src/format_string.cpp
    ${ERPC_BASE}/erpcgen/src/Logging.cpp
)

set(TEST_SERVER_SOURCES
    ${TEST_ERPC_SERVER_FILES}

    ${TEST_SOURCE_DIR}/${TEST_NAME}_server_impl.cpp
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_arbitrator_server.cpp
    ${ERPC_BASE}/erpcgen/src/format_string.cpp
    ${ERPC_BASE}/erpcgen/src/Logging.cpp
)

add_erpc_test(
    TEST_TYPE "client" 
    TRANSPORT ${TRANSPORT} 
    TEST_ERPC_FILES ${TEST_ERPC_CLIENT_FILES}
    TEST_SOURCES ${TEST_CLIENT_SOURCES}
    CUSTOM_CONFIG ${CONFIG_DIR}
)

add_erpc_test(
    TEST_TYPE "server" 
    TRANSPORT ${TRANSPORT} 
    TEST_ERPC_FILES ${TEST_ERPC_SERVER_FILES}
    TEST_SOURCES ${TEST_SERVER_SOURCES}
    CUSTOM_CONFIG ${CONFIG_DIR}
)
