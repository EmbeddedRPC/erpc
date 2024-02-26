#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

include(${ERPC_TEST_ROOT}/variables.cmake)

set(TEST_EXT_SERVER_INCLUDES ${CURRENT_DIR})
set(TEST_EXT_CLIENT_INCLUDES ${CURRENT_DIR})
set(TEST_EXT_CLIENT_SOURCES ${TEST_COMMON_DIR}/addOne.cpp)

include(${ERPC_TEST_ROOT}/test.cmake)

