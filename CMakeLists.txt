cmake_minimum_required(VERSION 3.0.0)
project(erpc)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # Default to installing in eRPC source directory
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)
endif()

set(ERPC_INCLUDE_INSTALL_DIR include/erpc)
set(ERPC_LIB_INSTALL_DIR lib)
set(BUILD_TOOLS FALSE)

if(WIN32)
  set(OS "win32")
elseif(UNIX AND NOT APPLE)
  set(OS "linux")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(OS_LIBS pthread rt)
elseif(APPLE)
  # This must come *before* linux or MacOSX will identify as Unix.
  set(OS "macosx")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(OS_LIBS pthread pcap)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeRTOS")
  set(OS "FreeRTOS")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "ThreadX")
  set(OS "ThreadX")
endif()

message("We will build eRPC for the OS ${OS}")

add_library(erpc STATIC)

target_include_directories(erpc PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/erpc_c/config>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/erpc_c/infra>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/erpc_c/port>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/erpc_c/setup>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/erpc_c/transports>
)

if("${OS}" STREQUAL "ThreadX")
target_include_directories(erpc PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../AFE/Core/Inc>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../AFE/Middlewares/ST/threadx/common/inc>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../AFE/Middlewares/ST/threadx/ports/cortex_m7/gnu/inc>
)
endif()

target_sources(erpc PRIVATE
    # erpc_c/infra/erpc_arbitrated_client_manager.cpp
    erpc_c/infra/erpc_basic_codec.cpp
    erpc_c/infra/erpc_client_manager.cpp
    erpc_c/infra/erpc_crc16.cpp
    erpc_c/infra/erpc_framed_transport.cpp
    erpc_c/infra/erpc_message_buffer.cpp
    erpc_c/infra/erpc_message_loggers.cpp
    erpc_c/infra/erpc_server.cpp
    erpc_c/infra/erpc_simple_server.cpp
    # erpc_c/infra/erpc_transport_arbitrator.cpp
    erpc_c/infra/erpc_pre_post_action.cpp
    erpc_c/port/erpc_port_stdlib.cpp
    # erpc_c/port/erpc_threading_pthreads.cpp
    erpc_c/port/erpc_threading_threadx.cpp
    # erpc_c/port/erpc_serial.cpp
    # erpc_c/setup/erpc_arbitrated_client_setup.cpp
    erpc_c/setup/erpc_client_setup.cpp
    erpc_c/setup/erpc_setup_mbf_dynamic.cpp
    erpc_c/setup/erpc_setup_mbf_static.cpp
    erpc_c/setup/erpc_server_setup.cpp
    # erpc_c/setup/erpc_setup_serial.cpp
    # erpc_c/transports/erpc_inter_thread_buffer_transport.cpp
    # erpc_c/transports/erpc_serial_transport.cpp
    # erpc_c/transports/erpc_tcp_transport.cpp
)

if("${OS}" STREQUAL "linux")
  target_sources(erpc PRIVATE
    erpc_c/port/erpc_port_stdlib.cpp
    erpc_c/port/erpc_threading_pthreads.cpp
    erpc_c/port/erpc_serial.cpp
    # erpc_c/setup/erpc_setup_serial.cpp
    # erpc_c/transports/erpc_serial_transport.cpp
    # erpc_c/transports/erpc_tcp_transport.cpp
  )
endif()

target_link_libraries(erpc PUBLIC ${OS_LIBS})

# add_definitions( "-DERPC_THREADS=(ERPC_THREADS_THREADX)")

if(BUILD_TOOLS)
    message("We will build eRPC tools..")
    add_subdirectory(erpcgen)
    add_subdirectory(erpcsniffer)
  else()
    message("We will not build eRPC tools..")
endif()
