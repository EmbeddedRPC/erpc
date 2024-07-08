#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(REMOTE_ZEPHYR_DIR ${CMAKE_CURRENT_BINARY_DIR}/../remote_server/zephyr)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(matrix_multiply_rpmsglite)

if(CONFIG_INCLUDE_REMOTE_DIR)
  target_include_directories(zephyr_interface
    INTERFACE ${REMOTE_ZEPHYR_DIR}/include/public)
endif()

enable_language(C CXX)

target_include_directories(app PRIVATE
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/service
)

target_sources(app PRIVATE 
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/erpc_error_handler.cpp
    
    # Generated shim code
    ${PROJECT_SOURCE_DIR}/src/service/c_erpc_matrix_multiply_client.cpp
    ${PROJECT_SOURCE_DIR}/src/service/erpc_matrix_multiply_client.cpp
    ${PROJECT_SOURCE_DIR}/src/service/erpc_matrix_multiply_interface.cpp
)

# Also include this directory into rpmsg_lite library because of project specific include rpsmg_config.h
target_include_directories(rpmsg_lite  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Same for erpc library and erpc_config.h
target_include_directories(erpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
