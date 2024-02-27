#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(${CMAKE_PROJECT_NAME})

set(ERPC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(ERPC_INCLUDES
    ${ERPC_DIR}/infra
    ${ERPC_DIR}/port
    ${ERPC_DIR}/service
    ${ERPC_DIR}/setup
    ${ERPC_DIR}/transports
    ${ERPC_DIR}/config
) 

set(ERPC_SOURCES
    ${ERPC_DIR}/infra/erpc_arbitrated_client_manager.cpp       
    ${ERPC_DIR}/infra/erpc_basic_codec.cpp
    ${ERPC_DIR}/infra/erpc_client_manager.cpp
    ${ERPC_DIR}/infra/erpc_crc16.cpp
    ${ERPC_DIR}/infra/erpc_framed_transport.cpp
    ${ERPC_DIR}/infra/erpc_message_buffer.cpp
    ${ERPC_DIR}/infra/erpc_message_loggers.cpp
    ${ERPC_DIR}/infra/erpc_server.cpp
    ${ERPC_DIR}/infra/erpc_simple_server.cpp       
    ${ERPC_DIR}/infra/erpc_transport_arbitrator.cpp
    ${ERPC_DIR}/infra/erpc_utils.cpp
    ${ERPC_DIR}/infra/erpc_pre_post_action.cpp

    ${ERPC_DIR}/port/erpc_port_stdlib.cpp
    ${ERPC_DIR}/port/erpc_threading_pthreads.cpp

    ${ERPC_DIR}/setup/erpc_arbitrated_client_setup.cpp
    ${ERPC_DIR}/setup/erpc_client_setup.cpp
    ${ERPC_DIR}/setup/erpc_setup_mbf_dynamic.cpp
    ${ERPC_DIR}/setup/erpc_setup_mbf_static.cpp
    ${ERPC_DIR}/setup/erpc_server_setup.cpp
    ${ERPC_DIR}/setup/erpc_setup_serial.cpp
    ${ERPC_DIR}/setup/erpc_setup_tcp.cpp

    ${ERPC_DIR}/transports/erpc_inter_thread_buffer_transport.cpp
    ${ERPC_DIR}/transports/erpc_tcp_transport.cpp
    ${ERPC_DIR}/transports/erpc_serial_transport.cpp

    # ${ERPC_DIR}/port/erpc_serial.cpp
)

# Create erpc library
add_library(erpc STATIC ${ERPC_SOURCES})

target_include_directories(erpc PUBLIC ${ERPC_INCLUDES})

# Required for TCP transport
if(WIN32)
  target_link_libraries(erpc wsock32 ws2_32)
endif()
