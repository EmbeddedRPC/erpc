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
add_subdirectory(${ERPC_TEST_ROOT}/test_annotations)
add_subdirectory(${ERPC_TEST_ROOT}/test_arbitrator)
add_subdirectory(${ERPC_TEST_ROOT}/test_arrays)
add_subdirectory(${ERPC_TEST_ROOT}/test_binary)
add_subdirectory(${ERPC_TEST_ROOT}/test_builtin)
add_subdirectory(${ERPC_TEST_ROOT}/test_callbacks)
add_subdirectory(${ERPC_TEST_ROOT}/test_const)
add_subdirectory(${ERPC_TEST_ROOT}/test_enums)
add_subdirectory(${ERPC_TEST_ROOT}/test_lists)
add_subdirectory(${ERPC_TEST_ROOT}/test_shared)
add_subdirectory(${ERPC_TEST_ROOT}/test_struct)
add_subdirectory(${ERPC_TEST_ROOT}/test_typedef)
add_subdirectory(${ERPC_TEST_ROOT}/test_unions)