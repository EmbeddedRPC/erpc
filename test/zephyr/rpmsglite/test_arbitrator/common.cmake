#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

if (APP_TYPE STREQUAL "client")
    set(APP_TYPE_SECOND "server")
elseif(APP_TYPE STREQUAL "server")
set(APP_TYPE_SECOND "client")
else()
    message(FATAL "Unmown APP_TYPE. Set it to the 'client' or 'server'.")
endif()


set(TEST_ERPC_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_firstInterface_${APP_TYPE}.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_firstInterface_${APP_TYPE}.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_secondInterface_${APP_TYPE_SECOND}.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_secondInterface_${APP_TYPE_SECOND}.cpp
)

set(TEST_SOURCES
    ${TEST_ERPC_FILES}

    ${TEST_SOURCE_DIR}/${TEST_NAME}_${APP_TYPE}_impl.cpp
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_arbitrator_${APP_TYPE}.cpp
)