# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(erpc_matrix_multiply_example C CXX)

target_sources(app PRIVATE 
         ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/src/erpc_error_handler.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/src/service/c_erpc_matrix_multiply_server.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/src/service/erpc_matrix_multiply_interface.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/src/service/erpc_matrix_multiply_server.cpp
        )

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/service)

target_include_directories(erpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
