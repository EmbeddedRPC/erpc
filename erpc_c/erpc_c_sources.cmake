#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(ERPC_C_INCLUDES
    ${ERPC_C}/infra
    ${ERPC_C}/port
    ${ERPC_C}/service
    ${ERPC_C}/setup
    ${ERPC_C}/transports
) 

set(ERPC_C_HEADERS
    ${ERPC_C}/config/erpc_config.h
	  ${ERPC_C}/infra/erpc_arbitrated_client_manager.hpp
	  ${ERPC_C}/infra/erpc_basic_codec.hpp
	  ${ERPC_C}/infra/erpc_client_manager.h
	  ${ERPC_C}/infra/erpc_codec.hpp
	  ${ERPC_C}/infra/erpc_crc16.hpp
	  ${ERPC_C}/infra/erpc_common.h
	  ${ERPC_C}/infra/erpc_version.h
	  ${ERPC_C}/infra/erpc_framed_transport.hpp
	  ${ERPC_C}/infra/erpc_manually_constructed.hpp
	  ${ERPC_C}/infra/erpc_message_buffer.hpp
	  ${ERPC_C}/infra/erpc_message_loggers.hpp
	  ${ERPC_C}/infra/erpc_server.hpp
	  ${ERPC_C}/infra/erpc_static_queue.hpp
	  ${ERPC_C}/infra/erpc_transport_arbitrator.hpp
	  ${ERPC_C}/infra/erpc_transport.hpp
	  ${ERPC_C}/infra/erpc_utils.hpp
	  ${ERPC_C}/infra/erpc_client_server_common.hpp
	  ${ERPC_C}/infra/erpc_pre_post_action.h
	  ${ERPC_C}/port/erpc_setup_extensions.h
	  ${ERPC_C}/port/erpc_config_internal.h
	  ${ERPC_C}/port/erpc_port.h
	  ${ERPC_C}/port/erpc_threading.h
	  ${ERPC_C}/port/erpc_serial.h
	  ${ERPC_C}/setup/erpc_arbitrated_client_setup.h
	  ${ERPC_C}/setup/erpc_client_setup.h
	  ${ERPC_C}/setup/erpc_mbf_setup.h
	  ${ERPC_C}/setup/erpc_server_setup.h
	  ${ERPC_C}/setup/erpc_transport_setup.h
	  ${ERPC_C}/transports/erpc_inter_thread_buffer_transport.hpp
	  ${ERPC_C}/transports/erpc_serial_transport.hpp
	  ${ERPC_C}/transports/erpc_tcp_transport.hpp
)

set(ERPC_C_SOURCES
	${ERPC_C}/infra/erpc_arbitrated_client_manager.cpp
	${ERPC_C}/infra/erpc_basic_codec.cpp
	${ERPC_C}/infra/erpc_client_manager.cpp
	${ERPC_C}/infra/erpc_crc16.cpp
	${ERPC_C}/infra/erpc_framed_transport.cpp
	${ERPC_C}/infra/erpc_message_buffer.cpp
	${ERPC_C}/infra/erpc_message_loggers.cpp
	${ERPC_C}/infra/erpc_server.cpp
	${ERPC_C}/infra/erpc_simple_server.cpp
	${ERPC_C}/infra/erpc_transport_arbitrator.cpp
	${ERPC_C}/infra/erpc_utils.cpp
	${ERPC_C}/infra/erpc_pre_post_action.cpp

	${ERPC_C}/port/erpc_port_stdlib.cpp
	${ERPC_C}/port/erpc_threading_pthreads.cpp
	${ERPC_C}/port/erpc_serial.cpp

	${ERPC_C}/setup/erpc_arbitrated_client_setup.cpp
	${ERPC_C}/setup/erpc_client_setup.cpp
	${ERPC_C}/setup/erpc_setup_mbf_dynamic.cpp
	${ERPC_C}/setup/erpc_setup_mbf_static.cpp
	${ERPC_C}/setup/erpc_server_setup.cpp
	${ERPC_C}/setup/erpc_setup_serial.cpp
	${ERPC_C}/setup/erpc_setup_tcp.cpp

	${ERPC_C}/transports/erpc_inter_thread_buffer_transport.cpp
	${ERPC_C}/transports/erpc_tcp_transport.cpp
	${ERPC_C}/transports/erpc_serial_transport.cpp
)