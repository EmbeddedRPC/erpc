#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

# Basic directories
set(ERPC_TEST_ROOT ${ERPC_BASE}/test)
set(TEST_COMMON_DIR ${ERPC_TEST_ROOT}/common)

# Add GTest library
message(STATUS "Creating gtest library")
add_library(gtest STATIC
    ${TEST_COMMON_DIR}/gtest/gtest.cpp
    ${TEST_COMMON_DIR}/gtest/gtimer.c
)
target_include_directories(gtest PUBLIC ${TEST_COMMON_DIR}/gtest)

# Include test's functions 
include(${ERPC_BASE}/cmake/test.cmake)

# Create 'run_all' target
add_custom_target(test_all)

# Get all test's names. 
file(GLOB ALL_ENTRIES "${ERPC_TEST_ROOT}/test_*")

foreach(ENTRY ${ALL_ENTRIES})
    if(IS_DIRECTORY ${ENTRY})
        get_filename_component(TEST_NAME ${ENTRY} NAME)

        list(APPEND TEST_NAMES ${TEST_NAME})
    endif()    
endforeach()


# Add test if enabled by KConfig and add to 'run_all' target
foreach(case ${TEST_NAMES})
    if(CONFIG_ERPC_TESTS.testcase.${case})
        add_subdirectory(${ERPC_TEST_ROOT}/${case})

        # Bind to 'run_all' target
        add_dependencies(test_all ${case})
    endif()
endforeach()