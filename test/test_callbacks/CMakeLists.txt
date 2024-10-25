#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

generate_erpc_test_variables()

# Edit test variables
set(ERPC_NAME_APP test_core0)

# Shim code files for client and server
set(TEST_ERPC_CLIENT_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_client.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_core1_interface.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_client.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_client.cpp
)

set(TEST_ERPC_SERVER_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_server.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_core1_interface.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_server.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_server.cpp
)

# Test sources
set(TEST_CLIENT_SOURCES
    ${TEST_ERPC_CLIENT_FILES}

    ${TEST_SOURCE_DIR}/${TEST_NAME}_client_impl.cpp
    

    ${ERPC_BASE}/erpcgen/src/format_string.cpp
    ${ERPC_BASE}/erpcgen/src/Logging.cpp
)

set(TEST_SERVER_SOURCES
    ${TEST_ERPC_SERVER_FILES}

    ${TEST_SOURCE_DIR}/${TEST_NAME}_server_impl.cpp
    
    ${ERPC_BASE}/erpcgen/src/format_string.cpp
    ${ERPC_BASE}/erpcgen/src/Logging.cpp
)

if(CONFIG_ERPC_TESTS.transport.tcp)
    # Add test targets
    set(TRANSPORT "tcp")
    add_erpc_test(
        TEST_TYPE "client" 
        TRANSPORT ${TRANSPORT} 
        TEST_ERPC_FILES ${TEST_ERPC_CLIENT_FILES}
        TEST_SOURCES ${TEST_CLIENT_SOURCES} ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_client.cpp
    )

    add_erpc_test(
        TEST_TYPE "server" 
        TRANSPORT ${TRANSPORT} 
        TEST_ERPC_FILES ${TEST_ERPC_SERVER_FILES}
        TEST_SOURCES ${TEST_SERVER_SOURCES} ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_server.cpp
    )
endif()

if(CONFIG_ERPC_TESTS.transport.serial)
    # Add test targets
    set(TRANSPORT "serial")
    add_erpc_test(
        TEST_TYPE "client" 
        TRANSPORT ${TRANSPORT} 
        TEST_ERPC_FILES ${TEST_ERPC_CLIENT_FILES}
        TEST_SOURCES ${TEST_CLIENT_SOURCES} ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_client.cpp
    )

    add_erpc_test(
        TEST_TYPE "server" 
        TRANSPORT ${TRANSPORT} 
        TEST_ERPC_FILES ${TEST_ERPC_SERVER_FILES}
        TEST_SOURCES ${TEST_SERVER_SOURCES} ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_server.cpp
    )
endif()

