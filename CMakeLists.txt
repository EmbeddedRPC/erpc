#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required (VERSION 3.5)

project (erpc)

set(CMAKE_CXX_STANDARD 17)
set(ERPC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Build erpc_c lib
add_subdirectory(erpc_c)

# Include generate_erpc_files
include(${ERPC_ROOT}/generate_erpc_files.cmake)

# Option for building eRPC project
option(BUILD_TESTS "Build library tests" ON)
option(BUILD_EXAMPLES "Build example executables" ON)
option(BUILD_ERPCGEN "Build erpcgen" ON)
option(ERPCGEN "Path to erpcgen executable")

# BUILD_TESTS and BUILD_EXAMPLES required erpcgen
if(BUILD_TESTS OR BUILD_EXAMPLES)
    if (ERPCGEN)
        if (NOT IS_ABSOLUTE ${ERPCGEN})
            set(ERPCGEN ${CMAKE_CURRENT_SOURCE_DIR}/${ERPCGEN})
        endif()
    else()
        if(NOT BUILD_ERPCGEN)
            message(FATAL_ERROR "No ERPCGEN specified and BUILD_ERPCGEN is disabled. Not able to generate erpc file!")
        endif()

        set(ERPCGEN $<TARGET_FILE:erpcgen>)
    endif()

    message(STATUS "ERPCGEN path: ${ERPCGEN}")
endif()

# Build erpcgen
if (BUILD_ERPCGEN)
    message(STATUS "Building erpcgen...")
    add_subdirectory(erpcgen)
endif()

# Build all tests
if (BUILD_TESTS)
    message(STATUS "Building tests...")
    add_subdirectory(test)
endif()

# Build all examples
if (BUILD_EXAMPLES)
    message(STATUS "Building examples...")
    add_subdirectory(examples)
endif()
