#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(matrix_multiply_c)

add_executable(matrix_multiply_client)
add_executable(matrix_multiply_server)

add_executable(matrix_multiply_client_cpp)
add_executable(matrix_multiply_server_cpp)

set(ERPC_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shim)
set(APP_NAME "erpc_matrix_multiply")
set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/erpc_matrix_multiply.erpc)
set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(OUTPUT_FILES
    ${ERPC_OUT_DIR}/${APP_NAME}_interface.cpp
    ${ERPC_OUT_DIR}/${APP_NAME}_client.cpp
    ${ERPC_OUT_DIR}/${APP_NAME}_server.cpp
    ${ERPC_OUT_DIR}/c_${APP_NAME}_client.cpp
    ${ERPC_OUT_DIR}/c_${APP_NAME}_server.cpp
)

# Generate IDL files
generate_erpc_files(
    IDL_FILE ${IDL_FILE} 
    OUTPUT_DIR ${ERPC_OUT_DIR}
    OUTPUT_FILES ${OUTPUT_FILES}
)

set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CONFIG_DIR}
    ${ERPC_OUT_DIR}
)

target_sources(matrix_multiply_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp

    ${ERPC_OUT_DIR}/${APP_NAME}_interface.cpp    
    ${ERPC_OUT_DIR}/c_${APP_NAME}_client.cpp
    ${ERPC_OUT_DIR}/${APP_NAME}_client.cpp
)

target_sources(matrix_multiply_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp

    ${ERPC_OUT_DIR}/${APP_NAME}_interface.cpp    
    ${ERPC_OUT_DIR}/c_${APP_NAME}_server.cpp
    ${ERPC_OUT_DIR}/${APP_NAME}_server.cpp
)

target_sources(matrix_multiply_client_cpp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp

    ${ERPC_OUT_DIR}/${APP_NAME}_interface.cpp    
    ${ERPC_OUT_DIR}/${APP_NAME}_client.cpp
)

target_sources(matrix_multiply_server_cpp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp

    ${ERPC_OUT_DIR}/${APP_NAME}_interface.cpp    
    ${ERPC_OUT_DIR}/${APP_NAME}_server.cpp
)

target_include_directories(matrix_multiply_client PRIVATE ${INCLUDE_DIRS})
target_include_directories(matrix_multiply_server PRIVATE ${INCLUDE_DIRS})

target_include_directories(matrix_multiply_client_cpp PRIVATE ${INCLUDE_DIRS})
target_include_directories(matrix_multiply_server_cpp PRIVATE ${INCLUDE_DIRS})

find_package(Threads REQUIRED)

target_link_libraries(matrix_multiply_client PRIVATE erpc Threads::Threads)
target_link_libraries(matrix_multiply_server PRIVATE erpc Threads::Threads)

target_link_libraries(matrix_multiply_client_cpp PRIVATE erpc Threads::Threads)
target_link_libraries(matrix_multiply_server_cpp PRIVATE erpc Threads::Threads)