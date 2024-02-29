#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

include(${ERPC_TEST_ROOT}/variables.cmake)

set(TEST_ERPC_OUT_DIR ${ERPC_OUT_ROOT_DIR})

include(${ERPC_TEST_ROOT}/test.cmake)

