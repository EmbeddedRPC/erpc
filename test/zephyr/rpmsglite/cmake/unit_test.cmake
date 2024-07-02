#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(TRANSPORT "zephyr_rpmsg_lite")

if((NOT APP_TYPE STREQUAL "client") AND (NOT APP_TYPE STREQUAL "server"))
    message(FATAL_ERROR "Unmown APP_TYPE. Set it to the 'client' or 'server'.")
endif()

if (APP_TYPE STREQUAL "client")
    set(KCONFIG_ROOT  ${CMAKE_CURRENT_LIST_DIR}/../config/Kconfig)
endif()

if(NOT DEFINED APPLICATION_CONFIG_DIR)
    if (APP_TYPE STREQUAL "client")
        set(APPLICATION_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/../config/)
    elseif(APP_TYPE STREQUAL "server")
        set(APPLICATION_CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/../config/remote) 
    endif()
endif()

if(NOT DEFINED CONFIG_DIR)
    set(CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR}/../config/)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

zephyr_get(APP_DIR SYSBUILD GLOBAL)
get_filename_component(TEST_NAME ${APP_DIR} NAME)
set(ERPC_ROOT_DIR ${ZEPHYR_ERPC_MODULE_DIR})
set(ZEPHYR_TEST_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../cmake)

# Include test's variables
include(${ZEPHYR_TEST_CMAKE_DIR}/variables.cmake)

if(CONFIG_INCLUDE_REMOTE_DIR)
    target_include_directories(zephyr_interface
        INTERFACE 
        ${CMAKE_CURRENT_BINARY_DIR}/../${TEST_NAME}_remote/zephyr/include/public)
endif()

