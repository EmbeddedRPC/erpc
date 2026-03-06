#
# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(REMOTE_APP remote)

# Add external project
ExternalZephyrProject_Add(
    APPLICATION ${REMOTE_APP}
    SOURCE_DIR ${APP_DIR}/remote
    BOARD ${SB_CONFIG_RPMSG_LITE_REMOTE_BOARD}
  )

# Add dependencies so that the remote sample will be built first
# This is required because some primary cores need information from the
# remote core's build, such as the output image's LMA
add_dependencies(matrix_multiply_ipc_service ${REMOTE_APP})
sysbuild_add_dependencies(CONFIGURE matrix_multiply_ipc_service ${REMOTE_APP})

if(SB_CONFIG_BOOTLOADER_MCUBOOT)
  # Make sure MCUboot is flashed first
  sysbuild_add_dependencies(FLASH matrix_multiply_ipc_service mcuboot)
endif()
