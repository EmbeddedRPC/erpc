#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This file generate test client and server targets. CMake including this file
# can override the variables to custom output, test name, ...

# Add IDL files if not provided
if (GENERATE_ERPC_IDL_FILES)
    set(TEST_IDL_FILES
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp

        # Client
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_client.cpp
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_client.cpp

        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_client.cpp
        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_client.cpp

        # Server
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_server.cpp
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_server.cpp

        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_server.cpp
        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_server.cpp
    )

    # Generate IDL files
    add_custom_command(
        OUTPUT ${TEST_IDL_FILES}
        COMMAND ${ERPCGEN} -g c -o ${ERPC_OUT_ROOT_DIR} ${IDL_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${IDL_FILE}
        COMMENT "Generating IDL files"
    )
endif()

# Add sources and includes to client if not provided
if (NOT TEST_CLIENT_SOURCES)
    set(TEST_CLIENT_SOURCES
        ${CURRENT_DIR}/${CURRENT_DIR_NAME}_client_impl.cpp

        ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_client.cpp

        ${ERPC_ROOT}/erpcgen/src/format_string.cpp
        ${ERPC_ROOT}/erpcgen/src/Logging.cpp

        # IDL files
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp

        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_client.cpp
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_client.cpp

        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_client.cpp
        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_client.cpp

        # Ext files
        ${TEST_EXT_CLIENT_SOURCES}
    )
endif()

if (NOT TEST_CLIENT_INCLUDES)
    set(TEST_CLIENT_INCLUDES
        ${TEST_OUT_DIR}
        ${TEST_ERPC_OUT_DIR}
        ${TEST_COMMON_DIR}
        ${TEST_COMMON_DIR}/config
        ${TEST_COMMON_DIR}/gtest
        ${ERPC_ROOT}/erpcgen/src
        ${TEST_EXT_SERVER_INCLUDES}
    )
endif()

# Add sources to server if not provided
if (NOT TEST_SERVER_SOURCES)
    set(TEST_SERVER_SOURCES
        ${CURRENT_DIR}/${CURRENT_DIR_NAME}_server_impl.cpp

        ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_server.cpp

        ${ERPC_ROOT}/erpcgen/src/format_string.cpp
        ${ERPC_ROOT}/erpcgen/src/Logging.cpp

        # IDL files
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp

        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_server.cpp
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_server.cpp

        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_server.cpp
        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_server.cpp

        # Ext files
        ${TEST_EXT_SERVER_SOURCES}
    )
endif()

if (NOT TEST_SERVER_INCLUDES)
    set(TEST_SERVER_INCLUDES
        ${TEST_OUT_DIR}
        ${TEST_ERPC_OUT_DIR}
        ${TEST_COMMON_DIR}
        ${TEST_COMMON_DIR}/config
        ${ERPC_ROOT}/erpcgen/src
        ${TEST_EXT_SERVER_INCLUDES}
    )
endif()

find_package(Threads REQUIRED)

add_executable(${TEST_NAME}_client ${TEST_CLIENT_SOURCES})
add_executable(${TEST_NAME}_server ${TEST_SERVER_SOURCES})

target_include_directories(${TEST_NAME}_client PRIVATE ${TEST_CLIENT_INCLUDES})
target_include_directories(${TEST_NAME}_server PRIVATE ${TEST_SERVER_INCLUDES})

target_link_libraries(${TEST_NAME}_client PRIVATE gtest erpc Threads::Threads)
target_link_libraries(${TEST_NAME}_server PRIVATE erpc Threads::Threads)

add_custom_target(run_${TEST_NAME}
    # COMMAND $<TARGET_FILE:${TEST_NAME}_server> &
    COMMAND $<TARGET_FILE:${TEST_NAME}_client>
    VERBATIM
)