#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required (VERSION 3.5)

project (erpc)

set(CMAKE_CXX_STANDARD 17)
set(ERPC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(erpc_c)

# Option for building eRPC project
option(BUILD_TESTS "Build library tests" ON)
option(BUILD_EXAMPLES "Build example executables" ON)
option(ERPCGEN "Path to erpcgen executable" "erpcgen")

# Create absolute path for erpcgen
cmake_path(IS_RELATIVE ERPCGEN ERPCGEN_REL)

if(ERPCGEN_REL)
    set(ERPCGEN ${CMAKE_CURRENT_SOURCE_DIR}/${ERPCGEN})
endif()

message(STATUS "ERPCGEN path: ${ERPCGEN}")

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