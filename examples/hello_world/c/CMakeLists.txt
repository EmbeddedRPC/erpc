#
# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

project(erpc_hello_world C CXX)

add_executable(client)
add_executable(server)

add_executable(client_cpp)
add_executable(server_cpp)

set(ERPC_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shim)
set(APP_NAME "hello_world")
set(IDL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../hello_world.erpc)
set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

set(APP_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CONFIG_DIR}
    ${ERPC_OUT_DIR}
)

set(OUTPUT_FILES
  ${ERPC_OUT_DIR}/${APP_NAME}_interface.cpp
  ${ERPC_OUT_DIR}/${APP_NAME}_client.cpp
  ${ERPC_OUT_DIR}/${APP_NAME}_server.cpp
  ${ERPC_OUT_DIR}/c_${APP_NAME}_client.cpp
  ${ERPC_OUT_DIR}/c_${APP_NAME}_server.cpp
)

generate_erpc_files(
    IDL_FILE ${IDL_FILES} 
    OUTPUT_DIR ${ERPC_OUT_DIR} 
    OUTPUT_FILES ${OUTPUT_FILES}
)

target_sources(client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/main_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp

    ${ERPC_OUT_DIR}/c_hello_world_client.cpp
    ${ERPC_OUT_DIR}/hello_world_client.cpp
    ${ERPC_OUT_DIR}/hello_world_interface.cpp
)

target_sources(server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/main_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp

    ${ERPC_OUT_DIR}/c_hello_world_server.cpp
    ${ERPC_OUT_DIR}/hello_world_server.cpp
    ${ERPC_OUT_DIR}/hello_world_interface.cpp
)

target_sources(client_cpp PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/main_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp

    ${ERPC_OUT_DIR}/hello_world_client.cpp
    ${ERPC_OUT_DIR}/hello_world_interface.cpp
)

target_sources(server_cpp PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/main_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_error_handler.cpp
    
    ${ERPC_OUT_DIR}/hello_world_server.cpp
    ${ERPC_OUT_DIR}/hello_world_interface.cpp
)

target_include_directories(client PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(server PRIVATE ${APP_INCLUDE_DIRS})

target_include_directories(client_cpp PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(server_cpp PRIVATE ${APP_INCLUDE_DIRS})

target_include_directories(erpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)

target_link_libraries(client PRIVATE erpc Threads::Threads)
target_link_libraries(server PRIVATE erpc Threads::Threads)

target_link_libraries(client_cpp PRIVATE erpc Threads::Threads)
target_link_libraries(server_cpp PRIVATE erpc Threads::Threads)