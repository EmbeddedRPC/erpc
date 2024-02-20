#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
project (${CMAKE_PROJECT_NAME})

set(target_name hello_world)
SET(SHIM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shim)

add_executable(client main.c)

set(OUTPUT_FILES
    ${SHIM_DIR}/${target_name}_interface.cpp
    ${SHIM_DIR}/${target_name}_client.cpp
    ${SHIM_DIR}/${target_name}_server.cpp
    ${SHIM_DIR}/c_${target_name}_client.cpp
    ${SHIM_DIR}/c_${target_name}_server.cpp
)

# The custom command
add_custom_command(
    OUTPUT ${OUTPUT_FILES}
    COMMAND ${ERPCGEN} -g c -o ${SHIM_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${target_name}.erpc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${target_name}.erpc
)

target_sources(client PRIVATE
    ${SHIM_DIR}/${target_name}_interface.cpp
    ${SHIM_DIR}/c_${target_name}_client.cpp
    ${SHIM_DIR}/${target_name}_client.cpp
)

target_include_directories(erpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PRIVATE ${SHIM_DIR})

target_link_libraries(client PRIVATE erpc)