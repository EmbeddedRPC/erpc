#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
project (${CMAKE_PROJECT_NAME})

set(SHIM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shim)
set(target_name "hello_world")

add_executable(client main.c)
# add_executable(server main.c)

generate_shim(${target_name} ${SHIM_DIR})

target_sources(client PRIVATE
    ${SHIM_DIR}/${target_name}_interface.cpp
    ${SHIM_DIR}/c_${target_name}_client.cpp
    ${SHIM_DIR}/${target_name}_client.cpp
)

target_include_directories(erpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PRIVATE ${SHIM_DIR})

target_link_libraries(client PRIVATE erpc)