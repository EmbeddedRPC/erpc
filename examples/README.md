# IDL Examples

### [`ble/`](ble/)

The `ble/` folder contains example IDL for the NXP Bluetooth Smart connectivity stack that is available for Kinetis KW40Z and KW30Z devices. The IDL demonstrates a very complex set of remote interfaces that wrap an existing library. The `ble/bluetooth.erpc` file is the main IDL file that imports the other files.

### [`matrix_multiply_tcp_python/`](matrix_multiply_tcp_python/)

Matrix multiply example demonstrates usage of eRPC between two PC's or boards (e.g. i.MX) with Python using TCP transport layer. One side acts like a server and the other as a client. Client generates two random matrixes and sends them to the server, which performs matrix multiplication and sends the result matrix back.

### [`smac.erpc`](smac.erpc)

IDL for the Simple MAC radio stack available for Kinetis KWxx series devices.

### [`temp_alarm.erpc`](temp_alarm.erpc)

This IDL file shows an example for a theoretical temperature alarm demo. It provides two interfaces, one in each direction.

