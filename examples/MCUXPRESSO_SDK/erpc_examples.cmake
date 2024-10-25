#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.examples.erpc_common_multicore)
    mcux_add_include(
        INCLUDES .
    )

    mcux_add_source(
        SOURCES erpc_error_handler.h
                erpc_error_handler.cpp
    )
endif()
if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.examples.erpc_matrix_multiply_client)
    mcux_add_include(
        INCLUDES erpc_matrix_multiply/service
    )

    mcux_add_source(
        SOURCES erpc_matrix_multiply/service/erpc_matrix_multiply_common.h
                erpc_matrix_multiply/service/erpc_matrix_multiply_common.hpp
                erpc_matrix_multiply/service/erpc_matrix_multiply_client.hpp
                erpc_matrix_multiply/service/erpc_matrix_multiply_client.cpp
                erpc_matrix_multiply/service/erpc_matrix_multiply_interface.hpp
                erpc_matrix_multiply/service/erpc_matrix_multiply_interface.cpp
                erpc_matrix_multiply/service/c_erpc_matrix_multiply_client.h
                erpc_matrix_multiply/service/c_erpc_matrix_multiply_client.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.examples.erpc_matrix_multiply_server)
    mcux_add_include(
        INCLUDES erpc_matrix_multiply/service
    )

    mcux_add_source(
        SOURCES erpc_matrix_multiply/service/erpc_matrix_multiply_common.h
                erpc_matrix_multiply/service/erpc_matrix_multiply_common.hpp
                erpc_matrix_multiply/service/erpc_matrix_multiply_server.hpp
                erpc_matrix_multiply/service/erpc_matrix_multiply_server.cpp
                erpc_matrix_multiply/service/erpc_matrix_multiply_interface.hpp
                erpc_matrix_multiply/service/erpc_matrix_multiply_interface.cpp
                erpc_matrix_multiply/service/c_erpc_matrix_multiply_server.h
                erpc_matrix_multiply/service/c_erpc_matrix_multiply_server.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.examples.erpc_two_way_rpc_core0)
    mcux_add_include(
        INCLUDES erpc_two_way_rpc/service
    )

    mcux_add_source(
        SOURCES erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_common.h
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_common.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_client.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_client.cpp
                erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core0Interface_client.h
                erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core0Interface_client.cpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_interface.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_interface.cpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_common.h
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_common.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_server.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_server.cpp
                erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core1Interface_server.h
                erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core1Interface_server.cpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_interface.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_interface.cpp
    )
endif()

if (CONFIG_MCUX_COMPONENT_middleware.multicore.erpc.examples.erpc_two_way_rpc_core1)
    mcux_add_include(
        INCLUDES erpc_two_way_rpc/service
    )

    mcux_add_source(
        SOURCES erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_common.h
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_common.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_client.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_client.cpp
                erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core1Interface_client.h
                erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core1Interface_client.cpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_interface.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_interface.cpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_common.h
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_common.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_server.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_server.cpp
                erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core0Interface_server.h
                erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core0Interface_server.cpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_interface.hpp
                erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_interface.cpp
    )
endif()

