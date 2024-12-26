#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

get_filename_component(CURRENT_DIR_NAME ${APP_DIR} NAME)
set(SERVER ${CURRENT_DIR_NAME}_remote)
set(CLIENT ${CURRENT_DIR_NAME})

# Include RPMSG_LITE_REMOTE_BOARD variable base on BOARD
include(${APP_DIR}/../cmake/rpmsg_lite.cmake)

# Add external project
ExternalZephyrProject_Add(
    APPLICATION ${SERVER}
    SOURCE_DIR ${APP_DIR}/remote
    BOARD ${RPMSG_LITE_REMOTE_BOARD}
  )

# Add dependencies so that the remote sample will be built first
# This is required because some primary cores need information from the
# remote core's build, such as the output image's LMA
add_dependencies(${CLIENT} ${SERVER})
sysbuild_add_dependencies(FLASH ${CLIENT} ${SERVER})
sysbuild_add_dependencies(CONFIGURE ${CLIENT} ${SERVER})

if(SB_CONFIG_BOOTLOADER_MCUBOOT)
  # Make sure MCUboot is flashed first
  sysbuild_add_dependencies(FLASH ${CLIENT} mcuboot)
endif()
