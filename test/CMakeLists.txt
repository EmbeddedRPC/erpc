#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

message(STATUS "Creating gtest library")
add_library(gtest STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common/gtest/gtest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/gtest/gtimer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common/gtest/gtest.h
)

set(ERPC_TEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_COMMON_DIR ${ERPC_TEST_ROOT}/common)

# All test using CMake
# create_unit_test_target(test_annotations)
# create_unit_test_target(test_arbitrator)
# create_unit_test_target(test_arrays)
# create_unit_test_target(test_binary)
# create_unit_test_target(test_builtin)
# create_unit_test_target(test_callbacks)
# create_unit_test_target(test_const)
# create_unit_test_target(test_enums)
# create_unit_test_target(test_lists)
# create_unit_test_target(test_shared)
# create_unit_test_target(test_struct)
# create_unit_test_target(test_typedef)
# create_unit_test_target(test_unions)

add_subdirectory(${ERPC_TEST_ROOT}/test_arrays)
add_subdirectory(${ERPC_TEST_ROOT}/test_binary)
add_subdirectory(${ERPC_TEST_ROOT}/test_builtin)
add_subdirectory(${ERPC_TEST_ROOT}/test_const)
add_subdirectory(${ERPC_TEST_ROOT}/test_enums)
add_subdirectory(${ERPC_TEST_ROOT}/test_lists)