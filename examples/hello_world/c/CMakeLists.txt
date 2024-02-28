#
# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20.0)

project(erpc_hello_world C CXX)

add_executable(client)
add_executable(server)

add_executable(client_cpp)
add_executable(server_cpp)

set(SHIM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shim)
set(SHIM_NAME "hello_world")
set(SHIM_IDL ${CMAKE_CURRENT_SOURCE_DIR}/../hello_world.erpc)
set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

set(APP_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CONFIG_DIR}
  ${SHIM_DIR}
)

generate_shim(${SHIM_IDL} ${SHIM_NAME} ${SHIM_DIR})

target_sources(client PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/main_client.c
  ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp
  ${SHIM_DIR}/c_hello_world_client.cpp
  ${SHIM_DIR}/hello_world_client.cpp
  ${SHIM_DIR}/hello_world_interface.cpp
)

target_sources(server PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/main_server.c
  ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp
  ${SHIM_DIR}/c_hello_world_server.cpp
  ${SHIM_DIR}/hello_world_server.cpp
  ${SHIM_DIR}/hello_world_interface.cpp
)

target_sources(client_cpp PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/main_client.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp
  ${SHIM_DIR}/hello_world_client.cpp
  ${SHIM_DIR}/hello_world_interface.cpp
)

target_sources(server_cpp PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/main_server.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp
  ${SHIM_DIR}/hello_world_server.cpp
  ${SHIM_DIR}/hello_world_interface.cpp
)

target_include_directories(client PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(server PRIVATE ${APP_INCLUDE_DIRS})

target_include_directories(client_cpp PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(server_cpp PRIVATE ${APP_INCLUDE_DIRS})

target_include_directories(erpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)

target_link_libraries(client PRIVATE erpc Threads::Threads)
target_link_libraries(server PRIVATE erpc Threads::Threads)

target_link_libraries(client_cpp PRIVATE erpc Threads::Threads)
target_link_libraries(server_cpp PRIVATE erpc Threads::Threads)