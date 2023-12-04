# SPDX-License-Identifier: Apache-2.0
#
# Copyright 2022 NXP

# Add external project
ExternalZephyrProject_Add(
    APPLICATION erpc_rpmsg_remote
    SOURCE_DIR ${APP_DIR}/remote
    BOARD ${SB_CONFIG_RPMSG_LITE_REMOTE_BOARD}
  )

# Add dependencies so that the remote sample will be built first
# This is required because some primary cores need information from the
# remote core's build, such as the output image's LMA
add_dependencies(erpc_rpmsg erpc_rpmsg_remote)
sysbuild_add_dependencies(CONFIGURE erpc_rpmsg erpc_rpmsg_remote)

if(SB_CONFIG_BOOTLOADER_MCUBOOT)
  # Make sure MCUboot is flashed first
  sysbuild_add_dependencies(FLASH erpc_rpmsg mcuboot)
endif()
