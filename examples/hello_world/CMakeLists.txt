#
# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

###############################################################################
# Define variables
###############################################################################

set(ERPC_C_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/c/shim)
set(ERPC_PY_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/py/shim)
set(ERPC_JAVA_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/java/src/main/java/org/example/)

set(APP_NAME "hello_world")
set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/hello_world.erpc)

set(INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/c
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ERPC_C_OUT_DIR}
)

set(OUTPUT_FILES
  ${ERPC_C_OUT_DIR}/${APP_NAME}_interface.cpp
  ${ERPC_C_OUT_DIR}/${APP_NAME}_client.cpp
  ${ERPC_C_OUT_DIR}/${APP_NAME}_server.cpp
  ${ERPC_C_OUT_DIR}/c_${APP_NAME}_client.cpp
  ${ERPC_C_OUT_DIR}/c_${APP_NAME}_server.cpp
)

###############################################################################
# Generate shim code
###############################################################################

if(CONFIG_ERPC_HELLO_WORLD.c OR CONFIG_ERPC_HELLO_WORLD.cpp)
    erpc_generate_shim_code(
        IDL_FILE ${IDL_FILE}
        OUTPUT_DIR ${ERPC_C_OUT_DIR}
        WORKING_DIRECTORY ${ERPC_EXAMPLES}
        OUTPUT_FILES ${OUTPUT_FILES}
    )
endif()

if(CONFIG_ERPC_HELLO_WORLD.python)
    add_custom_target(
        python_shim ALL
        COMMAND ${ERPCGEN_EXECUTABLE} -g py -o ${ERPC_PY_OUT_DIR} ${IDL_FILE}
        WORKING_DIRECTORY ${ERPC_EXAMPLES}
        DEPENDS ${IDL_FILE}
        COMMENT "Generating eRPC files from: ${IDL_FILE} to ${ERPC_PY_OUT_DIR}"
    )
endif()

if(CONFIG_ERPC_HELLO_WORLD.java)
    add_custom_target(
        java_shim ALL
        COMMAND ${ERPCGEN_EXECUTABLE} -g java -p org.example.hello_world -o ${ERPC_JAVA_OUT_DIR} ${IDL_FILE}
        WORKING_DIRECTORY ${ERPC_EXAMPLES}
        DEPENDS ${IDL_FILE}
        COMMENT "Generating eRPC files from: ${IDL_FILE} to ${ERPC_JAVA_OUT_DIR}"
    )
endif()

###############################################################################
# Create C/C++ targets
###############################################################################

find_package(Threads REQUIRED)

if(CONFIG_ERPC_HELLO_WORLD.c)
    add_executable(client)
    add_executable(server)

    target_sources(client PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/c/main_client.c
        ${CMAKE_CURRENT_SOURCE_DIR}/c/erpc_error_handler.cpp

        ${ERPC_C_OUT_DIR}/c_hello_world_client.cpp
        ${ERPC_C_OUT_DIR}/hello_world_client.cpp
        ${ERPC_C_OUT_DIR}/hello_world_interface.cpp
    )

    target_sources(server PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/c/main_server.c
        ${CMAKE_CURRENT_SOURCE_DIR}/c/erpc_error_handler.cpp

        ${ERPC_C_OUT_DIR}/c_hello_world_server.cpp
        ${ERPC_C_OUT_DIR}/hello_world_server.cpp
        ${ERPC_C_OUT_DIR}/hello_world_interface.cpp
    )

    target_include_directories(client PRIVATE ${INCLUDES})
    target_include_directories(server PRIVATE ${INCLUDES})

    target_link_libraries(client PRIVATE Threads::Threads)
    target_link_libraries(server PRIVATE Threads::Threads)

    erpc_link_c_library(TARGET client CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/c)
    erpc_link_c_library(TARGET server CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/c)
endif()

if(CONFIG_ERPC_HELLO_WORLD.cpp)
    add_executable(client_cpp)
    add_executable(server_cpp)

    target_sources(client_cpp PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/c/main_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c/erpc_error_handler.cpp

        ${ERPC_C_OUT_DIR}/hello_world_client.cpp
        ${ERPC_C_OUT_DIR}/hello_world_interface.cpp
    )

    target_sources(server_cpp PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/c/main_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c/erpc_error_handler.cpp
        
        ${ERPC_C_OUT_DIR}/hello_world_server.cpp
        ${ERPC_C_OUT_DIR}/hello_world_interface.cpp
    )

    target_include_directories(client_cpp PRIVATE ${INCLUDES})
    target_include_directories(server_cpp PRIVATE ${INCLUDES})

    target_link_libraries(client_cpp PRIVATE Threads::Threads)
    target_link_libraries(server_cpp PRIVATE Threads::Threads)

    erpc_link_c_library(TARGET client_cpp CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/c)
    erpc_link_c_library(TARGET server_cpp CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/c)
endif()