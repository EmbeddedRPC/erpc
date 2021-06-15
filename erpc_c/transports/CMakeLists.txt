


if("${TARGET_ENVIRONMENT}" STREQUAL "linux_pepdev")
    target_sources(pep PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/erpc_rpmsg_linux_transport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_rpmsg_linux_endpoint.cpp
    )
endif()
