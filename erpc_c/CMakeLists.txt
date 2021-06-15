CMAKE_MINIMUM_REQUIRED (VERSION 3.1)

# Shared across all erpc instances
# message("Adding eRPC include directories")
target_include_directories( pep PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/config
    ${CMAKE_CURRENT_LIST_DIR}/infra
    ${CMAKE_CURRENT_LIST_DIR}/port
    ${CMAKE_CURRENT_LIST_DIR}/setup
    ${CMAKE_CURRENT_LIST_DIR}/transports
)

add_subdirectory(config)
add_subdirectory(infra)
add_subdirectory(port)
add_subdirectory(setup)
add_subdirectory(transports)


message("done erpc_c CMakeLists.txt")