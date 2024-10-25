#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

if (CONFIG_ERPC_MATRIX_MULTIPLY_TCP)
    add_subdirectory(${ERPC_EXAMPLES}/matrix_multiply_tcp_c)
endif()

if (CONFIG_ERPC_HELLO_WORLD)
    add_subdirectory(${ERPC_EXAMPLES}/hello_world)
endif()