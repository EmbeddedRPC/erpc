#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Selects the secondary board based on the board set during build.

message(STATUS "BOARD: ${BOARD}")

if(BOARD STREQUAL "lpcxpresso54114_m4")
    set(RPMSG_LITE_REMOTE_BOARD "lpcxpresso54114_m0")
elseif(BOARD STREQUAL "lpcxpresso55s69_cpu0")
    set(RPMSG_LITE_REMOTE_BOARD "lpcxpresso55s69_cpu1")
elseif(BOARD STREQUAL "mps2_an521")
    set(RPMSG_LITE_REMOTE_BOARD "mps2_an521_remote")
elseif(BOARD STREQUAL "v2m_musca_b1")
    set(RPMSG_LITE_REMOTE_BOARD "v2m_musca_b1_ns")
elseif(BOARD STREQUAL "mimxrt1170_evk_cm7")
    set(RPMSG_LITE_REMOTE_BOARD "mimxrt1170_evk_cm4")
elseif(BOARD STREQUAL "mimxrt1160_evk_cm7")
    set(RPMSG_LITE_REMOTE_BOARD "mimxrt1160_evk_cm4")
elseif(BOARD STREQUAL "mimxrt1170_evkb_cm7")
    set(RPMSG_LITE_REMOTE_BOARD "mimxrt1170_evkb_cm4")
else()
    message(FATAL "RMPSG-Lite: Unsupported board.")
endif()


