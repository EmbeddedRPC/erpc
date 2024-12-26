#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Set test implementation directory 
set(TEST_SOURCE_DIR ${ERPC_ROOT_DIR}/test/${TEST_NAME})
set(TEST_COMMON_DIR ${ERPC_ROOT_DIR}/test/common)

# Set test name
set(ERPC_NAME "test")
set(ERPC_NAME_APP ${ERPC_NAME})

# Directory where test is generated 
set(TEST_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Directory where eRPC shim is generated
set(ERPC_OUT_ROOT_DIR ${TEST_OUT_DIR})
set(ERPC_OUT_DIR erpc_outputs)
set(TEST_ERPC_OUT_DIR ${ERPC_OUT_ROOT_DIR}/${ERPC_OUT_DIR})

# Disable/Enable ERPC Shim generation; Source code has to be added to TEST_CLIENT_SOURCES/TEST_SERVER_SOURCES
set(GENERATE_ERPC_IDL_FILES TRUE)

# IDL file
set(IDL_FILE ${TEST_SOURCE_DIR}/${TEST_NAME}.erpc)