# Overview

This example demonstrates usage of eRPC between two PC's or boards (e.g. i.MX) with C/C++ using TCP transport layer. One
side acts like a server and the second as a client. When client starts, it generates two random matrixes and sends them
to the server. Server then performs matrix multiplication and sends the result matrix back to the client. Client then
prints the result matrix.

# Generating example

This example use CMake to create executables. In [erpc/](../../) run `cmake -B ./build` (or `cmake --preset mingw64` on win with local mingw) to generate Cmake project. Then
run `cmake --build ./build/ --target menuconfig` and enable _Build eRPC Examples → Matrix Multiply TCP example_
example (optionally disable _erpcgen_ and _eRPC C lib_). This can be also done by enabling in _erpc/prj.conf_.

Execute `cmake --build ./build/` to build the example.

Cmake generates four
executables (`matrix_multiply_client`, `matrix_multiply_client_cpp`, `matrix_multiply_server`, `matrix_multiply_server_cpp`)
in [build/examples/matrix_multiply_tcp_c](../../build/examples/matrix_multiply_tcp_c).

# Running

Execute server and then client. Can be combined with other implementations of this example (Python, Java, Zephyr).
