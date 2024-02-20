#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project (${CMAKE_PROJECT_NAME})

set(SHIM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shim)
set(SHIM_NAME "hello_world")
set(SHIM_IDL ${CMAKE_CURRENT_SOURCE_DIR}/hello_world.erpc)

add_executable(client main.c)
# add_executable(server main.c)

generate_shim(${SHIM_IDL} ${SHIM_NAME} ${SHIM_DIR})

target_sources(client PRIVATE
    ${SHIM_DIR}/${SHIM_NAME}_interface.cpp
    ${SHIM_DIR}/c_${SHIM_NAME}_client.cpp
    ${SHIM_DIR}/${SHIM_NAME}_client.cpp
)

target_include_directories(erpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PRIVATE ${SHIM_DIR})

target_link_libraries(client PRIVATE erpc)