#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/unit_test.cmake)
project(${APP_TYPE})

set(ERPC_NAME test)

# Include test
include(${ZEPHYR_TEST_CMAKE_DIR}/test.cmake)
