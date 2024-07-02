#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Zephyr
set(APP_TYPE "client")

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/unit_test.cmake)
project(${APP_TYPE})

set(ERPC_NAME_APP test_ArithmeticService)

# Include test
include(${ZEPHYR_TEST_CMAKE_DIR}/test.cmake)
