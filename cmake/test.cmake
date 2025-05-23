#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Generate basic set of tests for given test case - client/server, tcp/serial
function(generate_erpc_test)

    # Add TCP test
    if(CONFIG_ERPC_TESTS.transport.tcp)

        if(CONFIG_ERPC_TESTS.client)
            add_erpc_test(TEST_TYPE "client" TRANSPORT "tcp")
        endif()
        if(CONFIG_ERPC_TESTS.server)
            add_erpc_test(TEST_TYPE "server" TRANSPORT "tcp")
        endif()
    endif()

    # Add SERIAL test
    if(CONFIG_ERPC_TESTS.transport.serial)
        if(CONFIG_ERPC_TESTS.client)
            add_erpc_test(TEST_TYPE "client" TRANSPORT "serial")
        endif()
        if(CONFIG_ERPC_TESTS.server)
            add_erpc_test(TEST_TYPE "server" TRANSPORT "serial")
        endif()
    endif()

endfunction(generate_erpc_test)

# This function inheres basic test's parameters from calling CMake generated by 
# 'generate_erpc_test_variables'. This allows to override some of them to fit each test's requirements.
# If test need to use different sources or incudes, it can use 'add_erpc_test' parameters:
#
#   add_erpc_test (
#       TEST_TYPE (client|server)
#       TRANSPORT (tcp|serial)
#       [CUSTOM_CONFIG path] 
#       [TEST_ERPC_FILES file [item1...]]
#       [TEST_SOURCES [item1...]]
#       [TEST_INCLUDES [item1...]]
#   )
#
# This allows to specify custom source for client/server in one test case (test_arbitrator, ...).
# If CUSTOM_CONFIG if specified, generator will try to find or creates erpc library with given config file.
function(add_erpc_test)
    set(oneValueArgs TEST_TYPE TRANSPORT CUSTOM_CONFIG)
    set(multiValueArgs TEST_ERPC_FILES TEST_SOURCES TEST_INCLUDES)
    cmake_parse_arguments(_ "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    set_ifndef(__TEST_ERPC_FILES
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp

        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_${__TEST_TYPE}.cpp
        ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_${__TEST_TYPE}.cpp

        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_${__TEST_TYPE}.cpp
        ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_${__TEST_TYPE}.cpp
    )

    if(GENERATE_ERPC_IDL_FILES)
        erpc_generate_shim_code(
            IDL_FILE ${IDL_FILE}
            OUTPUT_DIR ${ERPC_OUT_ROOT_DIR}
            WORKING_DIRECTORY ${TEST_SOURCE_DIR}
            OUTPUT_FILES ${__TEST_ERPC_FILES}
        )
    endif()

    set_ifndef(__TEST_SOURCES
        ${TEST_COMMON_DIR}/unit_test_${__TRANSPORT}_${__TEST_TYPE}.cpp
        ${TEST_SOURCE_DIR}/${TEST_NAME}_${__TEST_TYPE}_impl.cpp

        ${ERPC_ERPCGEN}/src/Logging.cpp
        ${__TEST_ERPC_FILES}

        ${TEST_EXT_SOURCES}
    )

    set_ifndef(__TEST_INCLUDES
        ${TEST_ERPC_OUT_DIR}
        ${TEST_SOURCE_DIR}
        ${CONFIG_DIR}
        ${TEST_COMMON_DIR}
        ${ERPC_ERPCGEN}/src
        ${TEST_EXT_INCLUDES}
    )

    # Custom target name base on test, test type and transport (test_arbitrator_client_tcp, ...)
    set(TARGET ${TEST_NAME}_${__TEST_TYPE}_${__TRANSPORT})

    add_executable(${TARGET} ${__TEST_SOURCES} )
    target_include_directories(${TARGET} PRIVATE ${__TEST_INCLUDES})
    
    target_link_libraries(${TARGET} PRIVATE gtest)
    erpc_link_c_library(TARGET ${TARGET} CONFIG_DIR ${CONFIG_DIR})

    # Pass transport parameters to test's source code
    if(CONFIG_ERPC_TESTS.transport.serial)
        add_definitions(-DUNIT_TEST_SERIAL_PORT="${CONFIG_ERPC_TESTS.transport.serial.port}")
        add_definitions(-DUNIT_TEST_SERIAL_BAUD=${CONFIG_ERPC_TESTS.transport.serial.baud})
    endif()

    if(CONFIG_ERPC_TESTS.transport.tcp)
        add_definitions(-DUNIT_TEST_TCP_HOST="${CONFIG_ERPC_TESTS.transport.tcp.host}")
        add_definitions(-DUNIT_TEST_TCP_PORT=${CONFIG_ERPC_TESTS.transport.tcp.port})
    endif()

    # Add test_name target to build current test
    if(NOT TARGET ${TEST_NAME})
        add_custom_target(${TEST_NAME})
    endif()

    add_dependencies(${TEST_NAME} ${TARGET})

endfunction(add_erpc_test)

macro(generate_erpc_test_variables)

    get_filename_component(TEST_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    # Set test implementation directory
    set(TEST_SOURCE_DIR ${ERPC_BASE}/test/${TEST_NAME})
    set(TEST_COMMON_DIR ${ERPC_BASE}/test/common)

    # Set test name
    set(ERPC_NAME "test")
    set(ERPC_NAME_APP ${ERPC_NAME})

    # Directory where test is generated 
    set(TEST_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

    # Directory where eRPC shim is generated
    set(ERPC_OUT_ROOT_DIR ${TEST_OUT_DIR})
    set(ERPC_OUT_DIR erpc_outputs)
    set(TEST_ERPC_OUT_DIR ${ERPC_OUT_ROOT_DIR}/${ERPC_OUT_DIR})

    # Disable/Enable ERPC Shim generation; Source code has to be added to TEST_CLIENT_SOURCES/TEST_SERVER_SOURCES
    set(GENERATE_ERPC_IDL_FILES TRUE)

    # IDL file
    set(IDL_FILE ${TEST_SOURCE_DIR}/${TEST_NAME}.erpc)

    # Config directory
    set(CONFIG_DIR ${TEST_COMMON_DIR}/config)

endmacro(generate_erpc_test_variables)