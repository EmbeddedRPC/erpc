# Copyright 2023 NXP
#
# SPDX-License-Identifier: Apache-2.0

# Add external project
ExternalZephyrProject_Add(
    APPLICATION remote_server
    SOURCE_DIR ${APP_DIR}/remote
    BOARD ${SB_CONFIG_RPMSG_LITE_REMOTE_BOARD}
  )

# Add dependencies so that the remote sample will be built first
# This is required because some primary cores need information from the
# remote core's build, such as the output image's LMA
add_dependencies(matrix_multiply_rpmsglite remote_server)
sysbuild_add_dependencies(CONFIGURE matrix_multiply_rpmsglite remote_server)

if(SB_CONFIG_BOOTLOADER_MCUBOOT)
  # Make sure MCUboot is flashed first
  sysbuild_add_dependencies(FLASH matrix_multiply_rpmsglite mcuboot)
endif()
