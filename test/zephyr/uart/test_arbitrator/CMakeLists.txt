#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(APPLICATION_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)
set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/unit_test.cmake)
project(${APP_TYPE})

set(APP_TYPE_SECOND "client")

set(TEST_ERPC_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_${APP_TYPE}.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_firstInterface_${APP_TYPE}.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_${APP_TYPE_SECOND}.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_secondInterface_${APP_TYPE_SECOND}.cpp
)

set(TEST_SOURCES
    ${TEST_ERPC_FILES}

    ${TEST_SOURCE_DIR}/${TEST_NAME}_${APP_TYPE}_impl.cpp
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_arbitrator_${APP_TYPE}.cpp
)

# Include test
include(${ZEPHYR_TEST_CMAKE_DIR}/test.cmake)
