#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

generate_erpc_test_variables()

set(TEST_EXT_INCLUDES ${CURRENT_DIR})
set(TEST_EXT_SOURCES ${TEST_COMMON_DIR}/addOne.cpp)

generate_erpc_test()


