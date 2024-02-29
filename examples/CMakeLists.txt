#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

# All examples using CMake
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/hello_world/c)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/matrix_multiply_c)
