#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This file generate test client and server targets. CMake including this file
# can override the variables to custom output, test name, ...
#
# Variables
#   TEST_NAME
#   TEST_OUT_DIR
#   ERPC_OUT_DIR
#   ERPC_NAME
#   ERPC_NAME_APP
#   TEST_CLIENT_SOURCES
#   TEST_SERVER_SOURCES
#   TEST_CLIENT_INCLUDES
#   TEST_SERVER_INCLUDES

set(CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(CURRENT_DIR_NAME ${CURRENT_DIR} NAME)

# Test name
if (NOT DEFINED TEST_NAME)
    set(TEST_NAME ${CURRENT_DIR_NAME})
endif()

# Test transport
if (NOT DEFINED TRANSPORT)
    set(TRANSPORT tcp)
endif()

# Directory where test is generated 
if (NOT DEFINED TEST_OUT_DIR)
    set(TEST_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/output)
endif()

# Directory where eRPC shim is generated
if (NOT DEFINED ERPC_OUT_DIR)
    set(ERPC_OUT_DIR ${TEST_OUT_DIR}/erpc_outputs)
endif()

# Name of eRPC shim
if (NOT DEFINED ERPC_NAME)
    set(ERPC_NAME "test")
endif()

if (NOT DEFINED ERPC_NAME_APP)
    set(ERPC_NAME_APP ${ERPC_NAME})
endif()

# Directory where are Unit Test common files 
set(UT_COMMON_SRC ${ERPC_TEST_ROOT}/common)

# eRPC IDL file
set(IDL_FILE ${CURRENT_DIR_NAME}.erpc)

message(STATUS ${TEST_NAME})

#######################################################

message(STATUS "Building test: ${TEST_NAME}")

# Add sources and includes to client if not provided
if (NOT TEST_CLIENT_SOURCES)
    set(TEST_CLIENT_SOURCES
        ${CURRENT_DIR}/${CURRENT_DIR_NAME}_client_impl.cpp

        ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_client.cpp

        ${TEST_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
        ${TEST_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp

        ${TEST_OUT_DIR}/${ERPC_NAME_APP}_client.cpp
        ${TEST_OUT_DIR}/${ERPC_NAME}_unit_test_common_client.cpp

        ${TEST_OUT_DIR}/c_${ERPC_NAME_APP}_client.cpp
        ${TEST_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_client.cpp

        ${ERPC_ROOT}/erpcgen/src/format_string.cpp
        ${ERPC_ROOT}/erpcgen/src/Logging.cpp
    )
endif()

if (NOT TEST_CLIENT_INCLUDES)
    set(TEST_CLIENT_INCLUDES
        ${TEST_OUT_DIR}
        ${ERPC_OUT_DIR}
        ${TEST_COMMON_DIR}
        ${TEST_COMMON_DIR}/config
        ${TEST_COMMON_DIR}/gtest
        ${ERPC_ROOT}/erpcgen/src
    )
endif()

# Add sources to server if not provided
if (NOT TEST_SERVER_SOURCES)
    set(TEST_SERVER_SOURCES
        ${CURRENT_DIR}/${CURRENT_DIR_NAME}_server_impl.cpp

        ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_server.cpp

        ${TEST_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
        ${TEST_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp

        ${TEST_OUT_DIR}/${ERPC_NAME_APP}_server.cpp
        ${TEST_OUT_DIR}/${ERPC_NAME}_unit_test_common_server.cpp

        ${TEST_OUT_DIR}/c_${ERPC_NAME_APP}_server.cpp
        ${TEST_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_server.cpp

        ${ERPC_ROOT}/erpcgen/src/format_string.cpp
        ${ERPC_ROOT}/erpcgen/src/Logging.cpp
    )
endif()

if (NOT TEST_SERVER_INCLUDES)
    set(TEST_SERVER_INCLUDES
        ${TEST_OUT_DIR}
        ${ERPC_OUT_DIR}
        ${TEST_COMMON_DIR}
        ${TEST_COMMON_DIR}/config
        ${ERPC_ROOT}/erpcgen/src
    )
endif()

generate_shim(${ERPC_OUT_DIR} ${IDL_FILE} ${ERPC_NAME} ${ERPC_OUT_DIR})

message(STATUS "SOURCES: ${ERPC_OUT_DIR}")

add_executable(${TEST_NAME}_client ${TEST_CLIENT_SOURCES})
add_executable(${TEST_NAME}_server ${TEST_SERVER_SOURCES})

target_include_directories(${TEST_NAME}_client PRIVATE ${TEST_CLIENT_INCLUDES})
target_include_directories(${TEST_NAME}_server PRIVATE ${TEST_SERVER_INCLUDES})

target_link_libraries(${TEST_NAME}_client PRIVATE gtest erpc)
target_link_libraries(${TEST_NAME}_server PRIVATE erpc)