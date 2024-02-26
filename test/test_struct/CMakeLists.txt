#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

include(${ERPC_TEST_ROOT}/variables.cmake)

set(ERPC_NAME_APP test_ArithmeticService)

include(${ERPC_TEST_ROOT}/test.cmake)

