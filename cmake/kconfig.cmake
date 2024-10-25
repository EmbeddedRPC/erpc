# Copyright 2024 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

include_guard(GLOBAL)
# (1) merge each proj.conf (2) kconfig entry file generation (3) call Kconfiglib
# to generate .config file (4) process .config file to generate header file or
# set compiler/linker flags


file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/kconfig)

set_ifndef(AUTOCONF_H
           ${CMAKE_CURRENT_BINARY_DIR}/kconfig/include/generated/autoconf.h)

set(SPLITCONFIG_DIR ${CMAKE_CURRENT_BINARY_DIR})
# Re-configure (Re-execute all CMakeLists.txt code) when autoconf.h changes
set_property(
  DIRECTORY
  APPEND
  PROPERTY CMAKE_CONFIGURE_DEPENDS ${AUTOCONF_H})

set_ifndef(KCONFIG_NAMESPACE "CONFIG")
set(DOTCONFIG ${CMAKE_CURRENT_BINARY_DIR}/.config)
set(PARSED_KCONFIG_SOURCES_TXT ${CMAKE_CURRENT_BINARY_DIR}/kconfig/sources.txt)
set(GENERATED_HEADERS_TXT ${CMAKE_CURRENT_BINARY_DIR}/kconfig/headers.txt)

# Only defined KCONFIG_ROOT if not set, because SYSBUILD has defined
# KCONFIG_ROOT before
if(NOT DEFINED KCONFIG_ROOT)
  if(EXISTS ${APPLICATION_SOURCE_DIR}/Kconfig)
    set(KCONFIG_ROOT ${APPLICATION_SOURCE_DIR}/Kconfig)
  else()
    set(KCONFIG_ROOT ${ERPC_BASE}/Kconfig)
  endif()
endif()

set(COMMON_KCONFIG_ENV_SETTINGS
    PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}
    CONFIG_=${KCONFIG_NAMESPACE}_
    KCONFIG_CONFIG=${DOTCONFIG}
    ERPC_BASE=${ERPC_BASE}
    KCONFIG_BINARY_DIR=${KCONFIG_BINARY_DIR}
    # The variables below is for sysbuild kconfig
    config=${CMAKE_BUILD_CONFIG})

set(EXTRA_KCONFIG_TARGET_COMMAND_FOR_menuconfig
    ${ERPC_BASE}/scripts/kconfig/menuconfig.py)

set(EXTRA_KCONFIG_TARGET_COMMAND_FOR_guiconfig
    ${ERPC_BASE}/scripts/kconfig/guiconfig.py)

set_ifndef(KCONFIG_TARGETS menuconfig guiconfig hardenconfig)

foreach(kconfig_target ${KCONFIG_TARGETS} ${EXTRA_KCONFIG_TARGETS})
  add_custom_target(
    ${kconfig_target}
    ${CMAKE_COMMAND}
    -E
    env
    ZEPHYR_BASE=${ERPC_BASE}
    ${COMMON_KCONFIG_ENV_SETTINGS}
    "SHIELD_AS_LIST=${SHIELD_AS_LIST_ESCAPED}"
    DTS_POST_CPP=${DTS_POST_CPP}
    DTS_ROOT_BINDINGS=${DTS_ROOT_BINDINGS}
    ${PYTHON_EXECUTABLE}
    ${EXTRA_KCONFIG_TARGET_COMMAND_FOR_${kconfig_target}}
    ${KCONFIG_ROOT}
    WORKING_DIRECTORY ${ERPC_BASE}
    USES_TERMINAL COMMAND_EXPAND_LISTS)
endforeach()
 
# if CUSTOM_PRJ_CONF_PATHS is not empty, append it to merge_config_files
if(CUSTOM_PRJ_CONF_PATHS)
  foreach(path ${CUSTOM_PRJ_CONF_PATHS})
    list(APPEND merge_config_files ${CUSTOM_PRJ_CONF_PATHS})
  endforeach()
endif()

if(CONF_FILE)
  string(CONFIGURE "${CONF_FILE}" CONF_FILE_EXPANDED)
  string(REPLACE " " ";" CONF_FILE_AS_LIST "${CONF_FILE_EXPANDED}")
  list(APPEND merge_config_files ${CONF_FILE_AS_LIST})
else()
  list(APPEND merge_config_files ${ERPC_BASE}/prj.conf)
endif()

# Calculate a checksum of merge_config_files to determine if we need to
# re-generate .config
set(merge_config_files_checksum "")
foreach(f ${merge_config_files})
  file(MD5 ${f} checksum)
  set(merge_config_files_checksum "${merge_config_files_checksum}${checksum}")
endforeach()

# Create a new .config if it does not exists, or if the checksum of the
# dependencies has changed
set(merge_config_files_checksum_file
    ${CMAKE_CURRENT_BINARY_DIR}/.cmake.dotconfig.checksum)
set(CREATE_NEW_DOTCONFIG 1)
# Check if the checksum file exists too before trying to open it, though it
# should under normal circumstances
if(EXISTS ${DOTCONFIG} AND EXISTS ${merge_config_files_checksum_file})
  # Read out what the checksum was previously
  file(READ ${merge_config_files_checksum_file}
       merge_config_files_checksum_prev)
  if(${merge_config_files_checksum} STREQUAL
     ${merge_config_files_checksum_prev})
    # Checksum is the same as before
    set(CREATE_NEW_DOTCONFIG 0)
  endif()
endif()

if(CREATE_NEW_DOTCONFIG)
  set(input_configs_flags --handwritten-input-configs)
  set(input_configs ${merge_config_files} ${FORCED_CONF_FILE})
else()
  set(input_configs ${DOTCONFIG} ${FORCED_CONF_FILE})
endif()
# set(FORCED_CONF_FILE)
if(DEFINED FORCED_CONF_FILE)
  list(APPEND input_configs_flags --forced-input-configs)
endif()

cmake_path(GET AUTOCONF_H PARENT_PATH autoconf_h_path)
if(NOT EXISTS ${autoconf_h_path})
  file(MAKE_DIRECTORY ${autoconf_h_path})
endif()

execute_process(
  COMMAND
    ${CMAKE_COMMAND} -E env 
    ${COMMON_KCONFIG_ENV_SETTINGS}
    ${PYTHON_EXECUTABLE}
    ${ERPC_BASE}/scripts/kconfig/kconfig.py 
    --zephyr-base=${ERPC_BASE}
    ${input_configs_flags} 
    ${KCONFIG_ROOT} 
    ${DOTCONFIG}
    ${AUTOCONF_H}
    ""
    # ${SPLITCONFIG_DIR} 
    ${PARSED_KCONFIG_SOURCES_TXT} 
    ${GENERATED_HEADERS_TXT}
    ${input_configs}
  WORKING_DIRECTORY ${ERPC_BASE}
  # The working directory is set to the app dir such that the user can use
  # relative paths in CONF_FILE, e.g. CONF_FILE=nrf5.conf
  RESULT_VARIABLE ret)
if(NOT "${ret}" STREQUAL "0")
  message(FATAL_ERROR "Kconfig process command run failed with return code: ${ret}")
endif()

if(CREATE_NEW_DOTCONFIG)
  # Write the new configuration fragment checksum. Only do this if kconfig.py
  # succeeds, to avoid marking zephyr/.config as up-to-date when it hasn't been
  # regenerated.
  file(WRITE ${merge_config_files_checksum_file} ${merge_config_files_checksum})
endif()

# Read out the list of 'Kconfig' sources that were used by the engine.
file(STRINGS ${PARSED_KCONFIG_SOURCES_TXT} PARSED_KCONFIG_SOURCES_LIST)
file(STRINGS ${GENERATED_HEADERS_TXT} GENERATED_HEADERS_LIST)

# Force CMAKE configure when the Kconfig sources or configuration files changes.
foreach(kconfig_input ${merge_config_files} ${DOTCONFIG}
                      ${PARSED_KCONFIG_SOURCES_LIST} ${GENERATED_HEADERS_LIST})
  set_property(
    DIRECTORY
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS ${kconfig_input})
endforeach()

# import config
import_kconfig(${KCONFIG_NAMESPACE} ${DOTCONFIG})
