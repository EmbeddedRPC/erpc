cmake_minimum_required(VERSION 3.10)
project(HelloWorldERPC)


# Generate eRPC code from IDL file
function(add_erpcgen_target target_name)
    # Construct file names based on the provided target name
    set(OUTPUT_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_interface.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_client.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_server.cpp"
    )

    # The custom command
    add_custom_command(
        OUTPUT ${OUTPUT_FILES}
        COMMAND erpcgen -g c ${CMAKE_CURRENT_SOURCE_DIR}/${target_name}.erpc
        DEPENDS src/${target_name}.erpc
    )

    add_library(${target_name}_client_lib 
        ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_client.cpp 
        ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_interface.cpp)

    add_library(${target_name}_server_lib 
        ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_server.cpp 
        ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_interface.cpp)
endfunction()


add_erpcgen_target("matrix_multiply")


add_compile_definitions(TCP_HOST="localhost" TCP_PORT=8085)

include_directories("/usr/local/include/erpc"
                    "${CMAKE_CURRENT_BINARY_DIR}")

# Server executable
add_executable(server src/server.cpp)
target_link_libraries(server erpc matrix_multiply_server_lib)

# Client executable
add_executable(client src/client.cpp)
target_link_libraries(client erpc matrix_multiply_client_lib)
