CMAKE_MINIMUM_REQUIRED (VERSION 3.1)

target_sources(pep PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/erpc_crc16.cpp
    ${CMAKE_CURRENT_LIST_DIR}/erpc_client_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/erpc_message_buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/erpc_basic_codec.cpp
)