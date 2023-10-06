This chapter is about creating simple eRPC client/server application. From the eRPC point of view, creating an application contains two steps:

 - Generating and importing eRPC shim code.
 - Import eRPC common files and call functions to init environment.

An application demo is shown on the matrix multiplication client/server baremetal application, which is using UART transport.

## Generating and importing eRPC shim code

This chapter contains three topics. First, _IDL_ file needs be created. Use this file for _generating shim code_. Finally, the client application needs to import client-relevant files, and server needs to import server-relevant files.

### Creating IDL file

IDL files contains all declarations used in eRPC calls. Simple example can looks like.

IDL.erpc:
```C
/*!
 * You can write copyrights rules here. These rules will be copied into the outputs.
 */
@outputDir("erpc_outputs") // output directory
program erpc_matrix_multiply; // specify name of output files

/*! This const defines the matrix size. The value has to be the same as the
    Matrix array dimension. Do not forget to re-generate the eRPC code once the
    matrix size is changed in the eRPC file */
const int32 matrix_size = 5;

/*! This is the matrix array type. The dimension has to be the same as the
    matrix size const. Do not forget to re-generate the eRPC code once the
    matrix size is changed in the eRPC file */
type Matrix = int32[matrix_size][matrix_size];

interface MatrixMultiplyService // cover functions for same topic
{
    erpcMatrixMultiply(in Matrix matrix1, in Matrix matrix2, out Matrix result_matrix) -> void
}
```

This function sends two matrices from client to server and one matrix with results are sent back.

### Generating shim code

For generating shim code is needed two things:

 - eRPC generator tool - See [erpcgen](erpcgen) documentation section.
 - IDL file - Created in previous topic.

The easiest way for generating code is have both the tool and the IDL file in same directory. Then, the following command needs be executed:
```
erpcgen.exe IDL.erpc
```
This action creates four files in _erpc_outputs_ folder:

 - erpc_matrix_multiply.h
 - erpc_matrix_multiply_client.cpp
 - erpc_matrix_multiply_server.h
 - erpc_matrix_multiply_server.cpp

### Importing client/server files

`Client project` needs to count with __erpc_matrix_multiply.h__ and __erpc_matrix_multiply_client.cpp__ files. In the source file where the eRPC functions are called, the __erpc_matrix_multiply.h__ header file needs to be included.

`Server project` needs to count with __erpc_matrix_multiply.h__, __erpc_matrix_multiply_server.h__, and __erpc_matrix_multiply_server.cpp__ files. In source file where eRPC functions calls are implemented, the __erpc_matrix_multiply_server.h__ header file needs to be included.

## Import eRPC common files and call functions to the init environment.

This chapter contains three topics:

 - _Common_ contains information which common files needs be imported for the client and server application.
 - _Client_ contains information which client files needs be imported for the client and server application.
 - _Server_ contains information which server files needs be imported for the client and server application.

### Common

The basic rule is that files do not contain "client" or "server" string in their names. These files need the server and client application:

- From _ERPCRootDir/erpc_c/config_ file __erpc_config.h__. This file should be copied to application specific directory and used it instead of using this global file.
- From _ERPCRootDir/erpc_c/infra_ files __erpc_codec.h__, __erpc_basic_codec.h__, __erpc_basic_codec.cpp__, __erpc_common.h__, __erpc_version.h__, __erpc_framed_transport.h__, __erpc_framed_transport.cpp__,  __erpc_manually_constructed.h__, __erpc_message_buffer.h erpc_transport.h__ and __erpc_message_buffer.cpp__
- From _ERPCRootDir/erpc_c/port_ files __erpc_config_internal.h__, __erpc_port.h__ and __erpc_port_stdlib.cpp__
- From _ERPCRootDir/erpc_c/setup_ files __erpc_mbf_setup.h__, __erpc_setup_mbf_dynamic.cpp__, __erpc_transport_setup.h__ and __erpc_setup_uart_cmsis.cpp__
- From _ERPCRootDir/erpc_c/transports_ files  __erpc_uart_cmsis_transport.h__ and __erpc_uart_cmsis_transport.cpp__

### Client

The basic rule is client files contains "client" string in their names.

- From _ERPCRootDir/erpc_c/infra_ files __erpc_client_manager.h__ and __erpc_client_manager.cpp__
- From _ERPCRootDir/erpc_c/setup_ files __erpc_client_setup.h__ and __erpc_client_setup.cpp__

In source file where eRPC client environment init functions are called needs be included __erpc_client_setup.h__ header file.

### Server

The basic rule is client files contains "server" string in their names.

- From _ERPCRootDir/erpc_c/infra_ files __erpc_server.h__, __erpc_server.cpp__, __erpc_simple_server.h__ and __erpc_simple_server.cpp__
- From _ERPCRootDir/erpc_c/setup files __erpc_server_setup.h__ and __erpc_server_setup.cpp__

In the source file where the eRPC server environment init functions are called, the __erpc_server_setup.h__ header file needs to be included.

## Example of client/server main files.

This chapter shows the easiest main source file for the client and server side.

### Client main file

```
#include "erpc_matrix_multiply.h"
#include "erpc_client_setup.h"
#include "fsl_uart_cmsis.h" /* Not an eRPC file. Needs to be replaced with the included transport type. */

int main()
{
    /* Matrices definitions */
    Matrix matrix1, matrix2, result_matrix = {{0}};

    /* Init eRPC client environment */
    /* UART transport layer initialization */
    erpc_transport_t transport = erpc_transport_cmsis_uart_init((void *)&DEMO_UART); /* DEMO_UART defined in fsl_uart_cmsis.h */

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory = erpc_mbf_dynamic_init();

    /* eRPC client side initialization */
    erpc_client_init(transport, message_buffer_factory);

    /* other code like init matrix1 and matrix2 values */
    ...

    /* call eRPC functions */
    erpcMatrixMultiply(matrix1, matrix2, result_matrix);

    /* other code like print result matrix */
    ...

    return 0;
}
```

### Server main file

```
#include "erpc_matrix_multiply_server.h"
#include "erpc_server_setup.h"
#include "fsl_uart_cmsis.h" /* Not eRPC file. Need be replaced with include of transport type. */

/* implementation of function call */
void erpcMatrixMultiply(Matrix matrix1, Matrix matrix2, Matrix result_matrix)
{
    /* code for multiplication of matrices */
    ...
}

int main()
{
    /* Init eRPC server environment */
    /* UART transport layer initialization */
    erpc_transport_t transport = erpc_transport_cmsis_uart_init((void *)&DEMO_UART); /* DEMO_UART defined in fsl_uart_cmsis.h */

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory = erpc_mbf_dynamic_init();

    /* eRPC server side initialization */
    erpc_server_init(transport, message_buffer_factory);

    /* connect generated service into server, look into erpc_matrix_multiply_server.h */
    erpc_service_t service = create_MatrixMultiplyService_service();
    erpc_add_service_to_server(service);

    /* run server */
    erpc_server_run(); /* or erpc_server_poll(); */

    return 0;
}
```
