This page shows a footprint of the eRPC code used in the eRPC Matrix multiply example. It's role is multiply two matrices and return a result. A transport is used for UART.

IDL:
```C
/*!
 * You can write copyrights rules here. Will be copied into outputs.
 */
@outputDir("erpc_outputs") // output directory
program erpc_matrix_multiply; // specify name of output files

/*! This const defines the matrix size. The value has to be the same as the
    Matrix array dimension. Do not forget to re-generate the erpc code once the
    matrix size is changed in the erpc file */
const int32 matrix_size = 5;

/*! This is the matrix array type. The dimension has to be the same as the
    matrix size const. Do not forget to re-generate the erpc code once the
    matrix size is changed in the erpc file */
type Matrix = int32[matrix_size][matrix_size];

interface MatrixMultiplyService // cover functions for same topic
{
    erpcMatrixMultiply(in Matrix matrix1, in Matrix matrix2, out Matrix result_matrix) -> void
}
```

_Footprint - release target, error checking code in shim code is enabled, compiled in IAR 8.20.1.14188:_
 - **client - shim code file: _erpc_matrix_multiply_client.o_**

file name | ro code | ro data |rw data
---|---|---|---
basic_codec.o                   |    296   |   192   |     0
client_manager.o                |    302   |    36   |     0
crc16.o                         |     64   |     0   |     0
erpc_client_setup.o             |    174   |    24   |    48
erpc_matrix_multiply_client.o   |    288   |     4   |     0
erpc_port_stdlib.o              |     32   |     0   |     0
erpc_setup_mbf_dynamic.o        |     78   |    32   |     8
erpc_setup_uart_cmsis.o         |     36   |     0   |    16
framed_transport.o              |    142   |     0   |     0
message_buffer.o                |     98   |     0   |     0
uart_cmsis_transport.o          |    192   |    44   |     2
---|---|---|---
**Total:**                      | **1702** | **332** | **74**

shim code | ro code | ro data |rw data
---|---|---|---
minimum shim size               |     96   |    0    |    0
interface                       |      0   |    0    |    0
oneway function                 |     80   |    0    |    0
void function                   |     80   |    0    |    0
in int                          |     16   |    0    |    0
out int                         |     16   |    0    |    0
in enum                         |     16   |    0    |    0
out enum                        |     20   |    0    |    0
in list @length                 |     48   |    0    |    0
out list @length                |     68   |    0    |    0
in array                        |     24   |    0    |    0
out array                       |     24   |    0    |    0
in struct{int}                  |     16   |    0    |    0
out struct{int}                 |     16   |    0    |    0
in struct{int} @shared          |     16   |    0    |    0
in union{int,int}               |     44   |    0    |    0
out union{int,int}              |     52   |    0    |    0
in string                       |     24   |    0    |    0
out string                      |     60   |    0    |    0
in string @nullable             |     48   |    0    |    0
out string @nullable            |     80   |    0    |    0


 - **server - shim code file: _erpc_matrix_multiply_server.o_**

file name | ro code | ro data |rw data
---|---|---|---
basic_codec.o                  |    296   |   192    |    0
crc16.o                        |     64   |     0    |    0
erpc_matrix_multiply_server.o  |    316   |    24    |    16
erpc_port_stdlib.o             |     32   |     0    |    0
erpc_server_setup.o            |    230   |    24    |   48
erpc_setup_mbf_dynamic.o       |     78   |    32    |    8
erpc_setup_uart_cmsis.o        |     36   |     0    |   16
framed_transport.o             |    142   |     0    |    0
message_buffer.o               |    108   |     0    |    0
server.o                       |    118   |     0    |    0
simple_server.o                |    360   |    40    |    0
uart_cmsis_transport.o         |    196   |    44    |    2
---|---|---|---
**Total:**                     | **1976** | **356**  | **90**

shim code | ro code | ro data |rw data
---|---|---|---
minimum shim size              |     48   |    20    |   16
interface                      |     36   |     0    |    0
oneway function                |     12   |     0    |    0
void function                  |     76   |     0    |    0
in int                         |     12   |     0    |    0
out int                        |     12   |     0    |    0
in enum                        |     12   |     0    |    0
out enum                       |     12   |     0    |    0
in list @length                |     88   |     0    |    0
out list @length               |     88   |     0    |    0
in array                       |     32   |     0    |    0
out array                      |     24   |     0    |    0
in struct{int}                 |     48   |     0    |    0
out struct{int}                |     48   |     0    |    0
in struct{int} @shared         |     16   |     0    |    0
in union{int,int}              |     68   |     0    |    0
out union{int,int}             |     76   |     0    |    0
in string                      |     72   |     0    |    0
out string                     |     76   |     0    |    0
in string @nullable            |     92   |     0    |    0
out string @nullable           |    108   |     0    |    0