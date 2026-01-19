#
# Copyright 2023-2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

if(("${BOARD}" STREQUAL "mimxrt1170_evkb") OR
  ("${BOARD}" STREQUAL "mimxrt1170_evk") OR
  ("${BOARD}" STREQUAL "mimxrt1160_evk"))
  message(STATUS "${BOARD} compile as remote in this sample")
else()
  message(FATAL_ERROR "${BOARD} is not supported for this sample")
endif()

project(remote)

message(STATUS "${BOARD} compiles as remote in this sample")

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

enable_language(C CXX)

target_include_directories(app PRIVATE
    ${PROJECT_SOURCE_DIR}/..
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/service
)

target_sources(app PRIVATE
    # Main files
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/erpc_error_handler.cpp

    # Generated shim code
    ${PROJECT_SOURCE_DIR}/src/service/c_erpc_matrix_multiply_server.cpp
    ${PROJECT_SOURCE_DIR}/src/service/erpc_matrix_multiply_server.cpp
    ${PROJECT_SOURCE_DIR}/src/service/erpc_matrix_multiply_interface.cpp
)

# Same for erpc library and erpc_config.h
target_include_directories(erpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
