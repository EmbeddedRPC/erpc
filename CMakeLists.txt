
CMAKE_MINIMUM_REQUIRED (VERSION 3.1)

# Shared across all erpc instances
target_include_directories( pep PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/config
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/infra
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/port
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/setup
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/transports
)


target_sources(pep PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/infra/erpc_crc16.cpp
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/infra/erpc_client_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/infra/erpc_message_buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/infra/erpc_basic_codec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/setup/erpc_client_setup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/erpc_c/setup/erpc_setup_mbf_dynamic.cpp
)

if("${TARGET_ENVIRONMENT}" STREQUAL "linux_pepdev" OR "linux")
    target_sources(pep PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/port/erpc_port_stdlib.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/port/erpc_threading_pthreads.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/setup/erpc_setup_rpmsg_linux.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/transports/erpc_rpmsg_linux_transport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/transports/erpc_rpmsg_linux_endpoint.cpp
    )
elseif("${TARGET_ENVIRONMENT}" STREQUAL "zephyr")
    target_sources(pep PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/port/erpc_port_zephyr.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/port/erpc_threading_zephyr.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/setup/erpc_setup_rpmsg_lite_rtos_remote.cpp 
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/transports/erpc_rpmsg_lite_rtos_transport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/setup/erpc_setup_mbf_rpmsg.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/setup/erpc_server_setup.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/infra/erpc_client_manager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/infra/erpc_simple_server.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_c/infra/erpc_server.cpp
    )
else()
    message(FATAL_ERROR"Invalid TARGET_ENVIRONMENT specified!")
endif()



message("done erpc CMakeLists.txt")
