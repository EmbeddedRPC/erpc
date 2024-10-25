#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20)
project(eRPC)

###############################################################################
# Define basic variables
###############################################################################
set(ERPC_BASE ${CMAKE_CURRENT_LIST_DIR})

set(ERPC_ERPCGEN ${ERPC_BASE}/erpcgen)
set(ERPC_C ${ERPC_BASE}/erpc_c)
set(ERPC_TEST ${ERPC_BASE}/test)
set(ERPC_EXAMPLES ${ERPC_BASE}/examples)

###############################################################################
# Includes required modules
###############################################################################
include(${ERPC_BASE}/cmake/extensions.cmake)
include(${ERPC_BASE}/cmake/python.cmake)
include(${ERPC_BASE}/cmake/kconfig.cmake)
include(${ERPC_BASE}/cmake/erpc_utils.cmake)

###############################################################################
# Add components base on Kconfig
###############################################################################

if(CONFIG_REQUIRE_ERPCGEN)
    if(CONFIG_ERPC_GENERATOR)
        set(ERPCGEN_EXECUTABLE $<TARGET_FILE:erpcgen>)
    else()
        find_program(ERPCGEN_EXECUTABLE NAMES erpcgen)

        if(NOT ERPCGEN_EXECUTABLE)
            message(FATAL_ERROR "It was not possible to find the erpcgen executable. Enable ERPC_GENERATOR in Kconfig to build erpcgen from source or add 'erpcgen' to PATH.")
        endif()
    endif()
endif()

if(CONFIG_ERPC_GENERATOR)
    add_subdirectory(${ERPC_ERPCGEN})
endif()

if(CONFIG_ERPC_LIB)
    add_subdirectory(${ERPC_C})
endif()

if(CONFIG_ERPC_TESTS)
    add_subdirectory(${ERPC_TEST})
endif()

if(CONFIG_ERPC_EXAMPLES)
    add_subdirectory(${ERPC_EXAMPLES})
endif()