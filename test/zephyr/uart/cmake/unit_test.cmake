#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# App type and transport
set(APP_TYPE "server")
set(TRANSPORT "zephyr_uart")

# Zephyr APPLICATION_CONFIG_DIR and CONFIG_DIR
if(NOT DEFINED APPLICATION_CONFIG_DIR)
    set(APPLICATION_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../config)
endif()

if(NOT DEFINED CONFIG_DIR)
    set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../config)
endif()

# Include zephyr
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Common variables
get_filename_component(TEST_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(ERPC_ROOT_DIR ${ZEPHYR_ERPC_MODULE_DIR})
set(ZEPHYR_TEST_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../cmake)

# Include test's variables
include(${ZEPHYR_TEST_CMAKE_DIR}/variables.cmake)