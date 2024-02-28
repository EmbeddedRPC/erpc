#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
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
#   TEST_EXT_CLIENT_SOURCES
#   TEST_EXT_SERVER_SOURCES
#   TEST_EXT_CLIENT_IMPORT
#   TEST_EXT_SERVER_IMPORT

set(CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(CURRENT_DIR_NAME ${CURRENT_DIR} NAME)

# Test name
set(TEST_NAME ${CURRENT_DIR_NAME})

# Test transport
set(TRANSPORT tcp)

# Directory where test is generated 
set(TEST_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Directory where eRPC shim is generated
set(ERPC_OUT_ROOT_DIR ${TEST_OUT_DIR})
set(ERPC_OUT_DIR erpc_outputs)
set(TEST_ERPC_OUT_DIR ${ERPC_OUT_ROOT_DIR}/${ERPC_OUT_DIR})

# Name of eRPC shim
set(ERPC_NAME "test")
set(ERPC_NAME_APP ${ERPC_NAME})

# Disable ERPC Shim generation; Source code has to be added to TEST_CLIENT_SOURCES/TEST_SERVER_SOURCES
set(GENERATE_ERPC_IDL_FILES TRUE)

# Directory where are Unit Test common files 
set(UT_COMMON_SRC ${ERPC_TEST_ROOT}/common)

# eRPC IDL file
set(IDL_FILE ${CURRENT_DIR}/${CURRENT_DIR_NAME}.erpc)