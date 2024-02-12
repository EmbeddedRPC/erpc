#
# Copyright 2023-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required (VERSION 3.5)

project (erpc C CXX)
set (ERPC_LIB erpc)

if (CONFIG_ERPC)
  set(ERPC_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/erpc_c)

  set(ERPC_INCLUDE_DIRS
    ${ERPC_DIR}/infra
    ${ERPC_DIR}/port
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
    ${ERPC_DIR}/port/erpc_port_zephyr.cpp
    ${ERPC_DIR}/port/erpc_threading_zephyr.cpp
    
    ${ERPC_DIR}/setup/erpc_client_setup.cpp
    ${ERPC_DIR}/setup/erpc_server_setup.cpp
    ${ERPC_DIR}/setup/erpc_setup_mbf_dynamic.cpp
    ${ERPC_DIR}/setup/erpc_setup_mbf_static.cpp
  )

  zephyr_library_named(${ERPC_LIB})
  zephyr_library_sources(${ERPC_SOURCES})
  zephyr_include_directories(${ERPC_INCLUDE_DIRS})
  zephyr_library_include_directories(${ERPC_INCLUDE_DIRS})

  # Optional UART transport
  zephyr_library_sources_ifdef(
    CONFIG_ERPC_TRANSPORT_UART
    ${ERPC_DIR}/transports/erpc_uart_zephyr_transport.cpp
    ${ERPC_DIR}/setup/erpc_setup_uart_zephyr.cpp
  )

  # Optional TCP transport
  zephyr_library_sources_ifdef(
    CONFIG_ERPC_TRANSPORT_TCP
    ${ERPC_DIR}/transports/erpc_tcp_transport.cpp
    ${ERPC_DIR}/setup/erpc_setup_tcp.cpp
    ${ERPC_DIR}/port/erpc_threading_pthreads.cpp # Required for POSIX threading
  )

  # Optional MBOX transport
  zephyr_library_sources_ifdef(
    CONFIG_ERPC_TRANSPORT_MBOX
    ${ERPC_DIR}/transports/erpc_mbox_zephyr_transport.cpp
    ${ERPC_DIR}/setup/erpc_setup_mbox_zephyr.cpp
  )

  # Optional RPMSG-LITE transport
  zephyr_library_sources_ifdef(
    CONFIG_ERPC_TRANSPORT_RPMSG_LITE
    ${ERPC_DIR}/setup/erpc_setup_mbf_rpmsg.cpp
    ${ERPC_DIR}/setup/erpc_setup_rpmsg_lite_rtos_master.cpp
    ${ERPC_DIR}/setup/erpc_setup_rpmsg_lite_rtos_remote.cpp
    ${ERPC_DIR}/transports/erpc_rpmsg_lite_rtos_transport.cpp
  )

  # Optional arbitrated client
  zephyr_library_sources_ifdef(
    CONFIG_ERPC_ARBITRATED_CLIENT
    ${ERPC_DIR}/infra/erpc_arbitrated_client_manager.cpp
    ${ERPC_DIR}/infra/erpc_transport_arbitrator.cpp
    ${ERPC_DIR}/setup/erpc_arbitrated_client_setup.cpp
  )

endif()
