# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(erpc_rpmsg_remote)

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

# Also include this directory into rpmsg_lite library because of project specific include rpsmg_config.h
target_include_directories(rpmsg_lite PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
# Same for erpc library and erpc_config.h
target_include_directories(erpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)