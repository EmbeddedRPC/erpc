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

set(ERPC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(ERPC_DIR ${ERPC_ROOT}/erpc_c)
set(SHIM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shim)
set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

find_library(ERPC_LIB erpc
  HINTS ${ERPC_ROOT}/Release/MINGW64/erpc/lib
)

message(STATUS ${ERPC_LIB})

if(${ERPC_LIB})
  message(FATAL_ERROR "ERPC lib not found")
else()
  message(STATUS "ERPC lib FOUND")
endif()

set(ERPC_INCLUDE_DIRS
  ${ERPC_DIR}/port
  ${ERPC_DIR}/infra
  ${ERPC_DIR}/service
  ${ERPC_DIR}/setup
  ${ERPC_DIR}/transports
) 

set(APP_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CONFIG_DIR}
  ${SHIM_DIR}
  ${ERPC_INCLUDE_DIRS}
)

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

target_link_libraries(client ${ERPC_LIB})
target_link_libraries(server ${ERPC_LIB})
target_link_libraries(client_cpp ${ERPC_LIB})
target_link_libraries(server_cpp ${ERPC_LIB})

target_include_directories(client PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(server PRIVATE ${APP_INCLUDE_DIRS})

target_include_directories(client_cpp PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(server_cpp PRIVATE ${APP_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries(client wsock32 ws2_32)
  target_link_libraries(server wsock32 ws2_32)

  target_link_libraries(client_cpp wsock32 ws2_32)
  target_link_libraries(server_cpp wsock32 ws2_32)
endif()