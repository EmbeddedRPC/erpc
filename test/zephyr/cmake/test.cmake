#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Generate ERPC shim code and add sources and includes to app target. 
# If APP_TYPE is client, also add gtest. Link the config directory to erpc a rpmsg_lite.

set_ifndef(TEST_ERPC_FILES
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_interface.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_interface.cpp

    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME_APP}_${APP_TYPE}.cpp
    ${TEST_ERPC_OUT_DIR}/${ERPC_NAME}_unit_test_common_${APP_TYPE}.cpp

    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME_APP}_${APP_TYPE}.cpp
    ${TEST_ERPC_OUT_DIR}/c_${ERPC_NAME}_unit_test_common_${APP_TYPE}.cpp
)

if(GENERATE_ERPC_IDL_FILES)
    add_custom_command(
        OUTPUT ${TEST_ERPC_FILES}
        COMMAND erpcgen -g c -o ${ERPC_OUT_ROOT_DIR} ${IDL_FILE} # TODO: add builded erpcgen if not found in path 
        WORKING_DIRECTORY ${TEST_SOURCE_DIR}
        DEPENDS ${IDL_FILE}
        COMMENT "Generating eRPC files from: ${IDL_FILE} to ${ERPC_OUT_ROOT_DIR}"
    )
endif()

set_ifndef(TEST_SOURCES
    ${TEST_COMMON_DIR}/unit_test_${TRANSPORT}_${APP_TYPE}.cpp
    ${TEST_SOURCE_DIR}/${TEST_NAME}_${APP_TYPE}_impl.cpp

    ${TEST_ERPC_FILES}
    ${TEST_EXT_SOURCES}
)

set_ifndef(TEST_INCLUDES
    ${TEST_ERPC_OUT_DIR}
    ${TEST_COMMON_DIR}
    ${TEST_SOURCE_DIR}
    ${CONFIG_DIR}/
    ${TEST_EXT_INCLUDES}
)

target_include_directories(app PRIVATE ${TEST_INCLUDES})
target_sources(app PRIVATE ${TEST_SOURCES})

# Include gtest
if(APP_TYPE STREQUAL "client")
    target_sources(app
        PRIVATE ${TEST_COMMON_DIR}/gtest/gtest.cpp
        PRIVATE ${TEST_COMMON_DIR}/gtest/gtimer.c
    )

    target_include_directories(app PUBLIC ${TEST_COMMON_DIR}/gtest/)

    target_compile_definitions(app PUBLIC
        GTEST_OS_BARE_METAL=1
        GTEST_HAS_CLONE=0 
        GTEST_HAS_EXCEPTIONS=0 
        GTEST_HAS_GLOBAL_STRING=0 
        GTEST_HAS_GLOBAL_WSTRING=0 
        GTEST_HAS_POSIX_RE=0 
        GTEST_HAS_PTHREAD=0 
        GTEST_HAS_RTTI=0 
        GTEST_HAS_STD_WSTRING=1 
        GTEST_HAS_TR1_TUPLE=0 
        GTEST_HAS_SEH=0 
        GTEST_HAS_STREAM_REDIRECTION=0 
        GTEST_USE_OWN_TR1_TUPLE=1 
        GTEST_LANG_CXX11=1
        GTEST_LINKED_AS_SHARED_LIBRARY=0 
        GTEST_CREATE_SHARED_LIBRARY=0 
        UNITY_DUMP_RESULTS=1
    )
endif()

# Also include this directory into rpmsg_lite library because of project specific include rpsmg_config.h
if(CONFIG_RPMSGLITE)
    target_include_directories(rpmsg_lite  PRIVATE ${CONFIG_DIR})
endif()

# Same for erpc library and erpc_config.h
if(CONFIG_ERPC)
    target_include_directories(erpc PRIVATE ${CONFIG_DIR})
endif()