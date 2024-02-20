#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

set(TEST_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(TEST_COMMON_DIR ${TEST_ROOT_DIR}/common)

get_filename_component(TEST_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

message(STATUS "Test name: ${TEST_NAME}")



create_unit_test_target(test_arrays)
