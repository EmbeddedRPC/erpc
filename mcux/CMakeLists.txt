#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc)
    mcux_component_version(1.14.0)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/infra
                 erpc_c/setup
                 erpc_c/port
                 erpc_c/transports
                 erpc_c/config
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/infra/erpc_version.h
                erpc_c/infra/erpc_codec.hpp
                erpc_c/infra/erpc_basic_codec.hpp
                erpc_c/infra/erpc_transport.hpp
                erpc_c/infra/erpc_message_buffer.hpp
                erpc_c/infra/erpc_message_loggers.hpp
                erpc_c/infra/erpc_common.h
                erpc_c/infra/erpc_manually_constructed.hpp
                erpc_c/infra/erpc_crc16.hpp
                erpc_c/infra/erpc_basic_codec.cpp
                erpc_c/infra/erpc_message_buffer.cpp
                erpc_c/infra/erpc_message_loggers.cpp
                erpc_c/infra/erpc_crc16.cpp
                erpc_c/infra/erpc_client_server_common.hpp
                erpc_c/infra/erpc_pre_post_action.h
                erpc_c/infra/erpc_pre_post_action.cpp
                erpc_c/infra/erpc_utils.hpp
                erpc_c/infra/erpc_utils.cpp
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/setup/erpc_transport_setup.h
                erpc_c/setup/erpc_mbf_setup.h
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/port/erpc_setup_extensions.h
                erpc_c/port/erpc_endianness_agnostic_example.h
                erpc_c/port/erpc_endianness_undefined.h
                erpc_c/port/erpc_config_internal.h
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/config/erpc_config.h
    )

    mcux_add_iar_configuration(
            CC "--enable_restrict"
            CX "--enable_restrict"
    )

endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.client)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/infra
                 erpc_c/setup
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/infra/erpc_client_manager.h
                erpc_c/infra/erpc_client_manager.cpp
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/setup/erpc_client_setup.h
                erpc_c/setup/erpc_client_setup.cpp
    )

endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.server)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/infra
                 erpc_c/setup
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/infra/erpc_simple_server.hpp
                erpc_c/infra/erpc_simple_server.cpp
                erpc_c/infra/erpc_server.hpp
                erpc_c/infra/erpc_server.cpp
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/setup/erpc_server_setup.h
                erpc_c/setup/erpc_server_setup.cpp
    )

endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.arbitrator)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/infra
                 erpc_c/setup
                 erpc_c/port
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/infra/erpc_arbitrated_client_manager.hpp
                erpc_c/infra/erpc_arbitrated_client_manager.cpp
                erpc_c/infra/erpc_client_manager.h
                erpc_c/infra/erpc_client_manager.cpp
                erpc_c/infra/erpc_simple_server.hpp
                erpc_c/infra/erpc_simple_server.cpp
                erpc_c/infra/erpc_server.hpp
                erpc_c/infra/erpc_server.cpp
                erpc_c/infra/erpc_transport_arbitrator.hpp
                erpc_c/infra/erpc_transport_arbitrator.cpp
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/setup/erpc_arbitrated_client_setup.h
                erpc_c/setup/erpc_arbitrated_client_setup.cpp
                erpc_c/setup/erpc_server_setup.h
                erpc_c/setup/erpc_server_setup.cpp
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/port/erpc_threading.h
                erpc_c/port/erpc_threading_freertos.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.port_stdlib)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/port
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/port/erpc_port.h
                erpc_c/port/erpc_port_stdlib.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.port_freertos)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/port
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/port/erpc_port.h
                erpc_c/port/erpc_port_freertos.cpp
                erpc_c/port/erpc_threading.h
                erpc_c/port/erpc_threading_freertos.cpp
                erpc_c/port/erpc_setup_extensions.h
                erpc_c/port/erpc_setup_extensions_freertos.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_mu_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_mu_transport.hpp
                erpc_c/transports/erpc_mu_transport.cpp
                erpc_c/setup/erpc_setup_mbf_dynamic.cpp
                erpc_c/setup/erpc_setup_mu.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_rpmsg_lite_transport_master)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_rpmsg_lite_base.hpp
                erpc_c/transports/erpc_rpmsg_lite_transport.hpp
                erpc_c/transports/erpc_rpmsg_lite_transport.cpp
                erpc_c/infra/erpc_static_queue.hpp
                erpc_c/setup/erpc_setup_mbf_rpmsg.cpp
                erpc_c/setup/erpc_setup_rpmsg_lite_master.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_rpmsg_lite_transport_remote)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_rpmsg_lite_base.hpp
                erpc_c/transports/erpc_rpmsg_lite_transport.hpp
                erpc_c/transports/erpc_rpmsg_lite_transport.cpp
                erpc_c/infra/erpc_static_queue.hpp
                erpc_c/setup/erpc_setup_mbf_rpmsg.cpp
                erpc_c/setup/erpc_setup_rpmsg_lite_remote.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_rpmsg_lite_rtos_transport_master)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_rpmsg_lite_base.hpp
                erpc_c/transports/erpc_rpmsg_lite_rtos_transport.hpp
                erpc_c/transports/erpc_rpmsg_lite_rtos_transport.cpp
                erpc_c/setup/erpc_setup_mbf_rpmsg.cpp
                erpc_c/setup/erpc_setup_rpmsg_lite_rtos_master.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_rpmsg_lite_rtos_transport_remote)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_rpmsg_lite_base.hpp
                erpc_c/transports/erpc_rpmsg_lite_rtos_transport.hpp
                erpc_c/transports/erpc_rpmsg_lite_rtos_transport.cpp
                erpc_c/setup/erpc_setup_mbf_rpmsg.cpp
                erpc_c/setup/erpc_setup_rpmsg_lite_rtos_remote.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_rpmsg_tty_rtos_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_rpmsg_lite_base.hpp
                erpc_c/transports/erpc_rpmsg_tty_rtos_transport.hpp
                erpc_c/transports/erpc_rpmsg_tty_rtos_transport.cpp
                erpc_c/setup/erpc_setup_mbf_rpmsg.cpp
                erpc_c/infra/erpc_framed_transport.hpp
                erpc_c/infra/erpc_framed_transport.cpp
                erpc_c/setup/erpc_setup_rpmsg_tty_rtos_remote.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_uart_cmsis_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_uart_cmsis_transport.hpp
                erpc_c/transports/erpc_uart_cmsis_transport.cpp
                erpc_c/setup/erpc_setup_mbf_dynamic.cpp
                erpc_c/infra/erpc_framed_transport.hpp
                erpc_c/infra/erpc_framed_transport.cpp
                erpc_c/setup/erpc_setup_uart_cmsis.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_dspi_master_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_dspi_master_transport.hpp
                erpc_c/transports/erpc_dspi_master_transport.cpp
                erpc_c/setup/erpc_setup_mbf_dynamic.cpp
                erpc_c/infra/erpc_framed_transport.hpp
                erpc_c/infra/erpc_framed_transport.cpp
                erpc_c/setup/erpc_setup_dspi_master.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_dspi_slave_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_dspi_slave_transport.hpp
                erpc_c/transports/erpc_dspi_slave_transport.cpp
                erpc_c/setup/erpc_setup_mbf_dynamic.cpp
                erpc_c/infra/erpc_framed_transport.hpp
                erpc_c/infra/erpc_framed_transport.cpp
                erpc_c/setup/erpc_setup_dspi_slave.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_spi_master_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_spi_master_transport.hpp
                erpc_c/transports/erpc_spi_master_transport.cpp
                erpc_c/setup/erpc_setup_mbf_dynamic.cpp
                erpc_c/infra/erpc_framed_transport.hpp
                erpc_c/infra/erpc_framed_transport.cpp
                erpc_c/setup/erpc_setup_spi_master.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_spi_slave_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_spi_slave_transport.hpp
                erpc_c/transports/erpc_spi_slave_transport.cpp
                erpc_c/setup/erpc_setup_mbf_dynamic.cpp
                erpc_c/infra/erpc_framed_transport.hpp
                erpc_c/infra/erpc_framed_transport.cpp
                erpc_c/setup/erpc_setup_spi_slave.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_lpspi_slave_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_lpspi_slave_transport.hpp
                erpc_c/transports/erpc_lpspi_slave_transport.cpp
                erpc_c/setup/erpc_setup_mbf_dynamic.cpp
                erpc_c/infra/erpc_framed_transport.hpp
                erpc_c/infra/erpc_framed_transport.cpp
                erpc_c/setup/erpc_setup_lpspi_slave.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_i2c_slave_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_i2c_slave_transport.hpp
                erpc_c/transports/erpc_i2c_slave_transport.cpp
                erpc_c/setup/erpc_setup_mbf_dynamic.cpp
                erpc_c/infra/erpc_framed_transport.hpp
                erpc_c/infra/erpc_framed_transport.cpp
                erpc_c/setup/erpc_setup_i2c_slave.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.eRPC_lpi2c_slave_transport)
    mcux_add_include(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        INCLUDES erpc_c/transports
                 erpc_c/setup
                 erpc_c/infra
    )

    mcux_add_source(
        BASE_PATH ${CMAKE_CURRENT_LIST_DIR}/..
        SOURCES erpc_c/transports/erpc_lpi2c_slave_transport.hpp
                erpc_c/transports/erpc_lpi2c_slave_transport.cpp
                erpc_c/setup/erpc_setup_mbf_dynamic.cpp
                erpc_c/infra/erpc_framed_transport.hpp
                erpc_c/infra/erpc_framed_transport.cpp
                erpc_c/setup/erpc_setup_lpi2c_slave.cpp
    )
endif()

if (CONFIG_MCUX_PRJSEG_middleware.multicore.erpc.generated_config)
    # Left intentionaly empty - For ecosystem data generation
endif()


include(${CMAKE_CURRENT_LIST_DIR}/../examples/MCUXPRESSO_SDK/erpc_examples.cmake)
