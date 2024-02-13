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

set(ERPC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../erpc_c)

set(ERPC_INCLUDE_DIRS
  ${ERPC_DIR}/port
  ${ERPC_DIR}/infra
  ${ERPC_DIR}/service
  ${ERPC_DIR}/setup
  ${ERPC_DIR}/transports
) 

set(ERPC_SOURCES
  ${ERPC_DIR}/infra/erpc_basic_codec.cpp
  ${ERPC_DIR}/infra/erpc_client_manager.cpp
  ${ERPC_DIR}/infra/erpc_crc16.cpp
  ${ERPC_DIR}/infra/erpc_framed_transport.cpp
  ${ERPC_DIR}/infra/erpc_message_buffer.cpp
  ${ERPC_DIR}/infra/erpc_message_loggers.cpp
  ${ERPC_DIR}/infra/erpc_pre_post_action.cpp
  ${ERPC_DIR}/infra/erpc_server.cpp
  ${ERPC_DIR}/infra/erpc_simple_server.cpp       
  
  ${ERPC_DIR}/infra/erpc_utils.cpp
  ${ERPC_DIR}/port/erpc_port_stdlib.cpp
  
  ${ERPC_DIR}/setup/erpc_client_setup.cpp
  ${ERPC_DIR}/setup/erpc_server_setup.cpp
  ${ERPC_DIR}/setup/erpc_setup_mbf_dynamic.cpp
  ${ERPC_DIR}/setup/erpc_setup_mbf_static.cpp

  ${ERPC_DIR}/transports/erpc_tcp_transport.cpp
  ${ERPC_DIR}/port/erpc_threading_pthreads.cpp
  ${ERPC_DIR}/setup/erpc_setup_tcp.cpp
)

set(APP_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c
  ${CMAKE_CURRENT_SOURCE_DIR}/c
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${ERPC_INCLUDE_DIRS}
)

target_sources(client PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/c/main_client.c
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/c_hello_world_client.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/hello_world_client.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/hello_world_interface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/c/erpc_error_handler.cpp
  ${ERPC_SOURCES}
)

target_sources(server PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/c/main_server.c
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/c_hello_world_server.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/hello_world_server.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/hello_world_interface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/c/erpc_error_handler.cpp
  ${ERPC_SOURCES}
)

target_sources(client_cpp PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/c/main_client.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/hello_world_client.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/hello_world_interface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/c/erpc_error_handler.cpp
  ${ERPC_SOURCES}
)

target_sources(server_cpp PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/c/main_server.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/hello_world_server.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/shim/c/hello_world_interface.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/c/erpc_error_handler.cpp
  ${ERPC_SOURCES}
)

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