# IDL Examples

## [`hello_world/`](hello_world/)

Simple hello_world examples for **C**, **C++**, **Java** and **Python**

## [`idl/ble/`](idl/ble/)

The `ble/` folder contains example IDL for the NXP Bluetooth Smart connectivity stack that is available for Kinetis KW40Z and KW30Z devices. The IDL demonstrates a very complex set of remote interfaces that wrap an existing library. The `ble/bluetooth.erpc` file is the main IDL file that imports the other files.


## [`idl/smac.erpc`](idl/smac.erpc)

IDL for the Simple MAC radio stack available for Kinetis KWxx series devices.

## [`idl/temp_alarm.erpc`](idl/temp_alarm.erpc)

This IDL file shows an example for a theoretical temperature alarm demo. It provides two interfaces, one in each direction.

## [`matrix_multiply_python/`](matrix_multiply_python/)

Matrix multiply example demonstrates usage of eRPC between two PC's or boards (e.g. i.MX) with Python using TCP or Serial transport layer. One side acts like a server and the other as a client. Client generates two random matrixes and sends them to the server, which performs matrix multiplication and sends the result matrix back.

## [`matrix_multiply_java/`](matrix_multiply_java/)

Java version of the Python Matrix multiply example

## [`matrix_multiply_tcp_c/`](matrix_multiply_tcp_c/)

C/C++ version of the Python Matrix multiply example using TCP transport layer

## [`zephyr/`](zephyr/)

Multicore and multiprocessor matrix multiply examples for Zephyr OS with different transport layers:
- **mbox** - native Zephyr multicore communication API
- **rpmsglite** - NXP multicore communication protocol
- **tcp** - Zephyr with posix and eRPC TCP transport layer
- **uart** - Serial communication 