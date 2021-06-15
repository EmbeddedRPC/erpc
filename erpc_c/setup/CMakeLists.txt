
target_sources(pep PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/erpc_client_setup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/erpc_setup_mbf_dynamic.cpp
)

if("${TARGET_ENVIRONMENT}" STREQUAL "linux_pepdev")
    target_sources(pep PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}/erpc_setup_rpmsg_linux.cpp 
        )
endif()
