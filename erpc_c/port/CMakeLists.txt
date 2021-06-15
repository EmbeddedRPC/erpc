
if("${TARGET_ENVIRONMENT}" STREQUAL "linux_pepdev")
    target_sources(pep PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/erpc_port_stdlib.cpp
        ${CMAKE_CURRENT_LIST_DIR}/erpc_threading_pthreads.cpp
    )
endif()