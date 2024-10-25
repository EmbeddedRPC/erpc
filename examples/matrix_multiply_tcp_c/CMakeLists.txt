#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(matrix_multiply_c)

set(ERPC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shim)
set(APP_NAME "erpc_matrix_multiply")
set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/erpc_matrix_multiply.erpc)

set(OUTPUT_FILES
    ${ERPC_OUT_DIR}/${APP_NAME}_interface.cpp
    ${ERPC_OUT_DIR}/${APP_NAME}_client.cpp
    ${ERPC_OUT_DIR}/${APP_NAME}_server.cpp
    ${ERPC_OUT_DIR}/c_${APP_NAME}_client.cpp
    ${ERPC_OUT_DIR}/c_${APP_NAME}_server.cpp
)

erpc_generate_shim_code(
    IDL_FILE ${IDL_FILE}
    OUTPUT_DIR ${ERPC_OUT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_FILES ${OUTPUT_FILES}
)

set(INCLUDE_DIRS
    ${ERPC_C}/infra
    ${ERPC_C}/port
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ERPC_OUT_DIR}
)

set(MATRIX_MULTIPLY_SERVER
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp

    ${ERPC_OUT_DIR}/${APP_NAME}_interface.cpp    
    ${ERPC_OUT_DIR}/${APP_NAME}_server.cpp 
)
    
set(MATRIX_MULTIPLY_CLIENT 
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp
    
    ${ERPC_OUT_DIR}/${APP_NAME}_interface.cpp    
    ${ERPC_OUT_DIR}/${APP_NAME}_client.cpp  
)

find_package(Threads REQUIRED)

add_definitions(-DEXAMPLE_TCP_HOST="${CONFIG_ERPC_MATRIX_MULTIPLY_TCP.host}")
add_definitions(-DEXAMPLE_TCP_PORT=${CONFIG_ERPC_MATRIX_MULTIPLY_TCP.port})

if(CONFIG_ERPC_MATRIX_MULTIPLY_TCP.c)
    add_executable(matrix_multiply_client ${MATRIX_MULTIPLY_CLIENT} ${CMAKE_CURRENT_SOURCE_DIR}/main_client.c ${ERPC_OUT_DIR}/c_${APP_NAME}_client.cpp)
    add_executable(matrix_multiply_server ${MATRIX_MULTIPLY_SERVER} ${CMAKE_CURRENT_SOURCE_DIR}/main_server.c ${ERPC_OUT_DIR}/c_${APP_NAME}_server.cpp)

    target_include_directories(matrix_multiply_client PRIVATE ${INCLUDE_DIRS})
    target_include_directories(matrix_multiply_server PRIVATE ${INCLUDE_DIRS})

    erpc_link_c_library(TARGET matrix_multiply_client CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    erpc_link_c_library(TARGET matrix_multiply_server CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(CONFIG_ERPC_MATRIX_MULTIPLY_TCP.cpp)
    add_executable(matrix_multiply_client_cpp ${MATRIX_MULTIPLY_CLIENT} ${CMAKE_CURRENT_SOURCE_DIR}/main_client.cpp)
    add_executable(matrix_multiply_server_cpp ${MATRIX_MULTIPLY_SERVER} ${CMAKE_CURRENT_SOURCE_DIR}/main_server.cpp)

    target_include_directories(matrix_multiply_client_cpp PRIVATE ${INCLUDE_DIRS})
    target_include_directories(matrix_multiply_server_cpp PRIVATE ${INCLUDE_DIRS})

    erpc_link_c_library(TARGET matrix_multiply_client_cpp CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    erpc_link_c_library(TARGET matrix_multiply_server_cpp CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()