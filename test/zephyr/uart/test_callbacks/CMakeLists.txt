#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/unit_test.cmake)
project(${APP_TYPE})

set(ERPC_NAME_APP test_core0)

# Define required IDL files
set(TEST_ERPC_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_${APP_TYPE}.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_${APP_TYPE}.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_core1_interface.cpp
    
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_${APP_TYPE}.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_${APP_TYPE}.cpp
)

set(TEST_SOURCES
    ${TEST_ERPC_FILES}

    ${TEST_SOURCE_DIR}/${TEST_NAME}_${APP_TYPE}_impl.cpp
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_${APP_TYPE}.cpp
)

# Include test
include(${ZEPHYR_TEST_CMAKE_DIR}/test.cmake)
