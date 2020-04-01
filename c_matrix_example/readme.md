# Overview
This example demonstrates usage of eRPC on a MacOS using TCP transport and an example written in C. One side acts like a server and the second as a client. When client starts, it generates two random matrixes and sends them to the server. Server then performs matrix multiplication and sends the result matrix back to the client. Client then prints the result matrix.

Example can be ran on Personal computer running MacOS. 

# eRPC documentation
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: https://github.com/EmbeddedRPC/erpc/wiki

# Prerequisites
- gcc and g++ compiler

# eRPC installation\
1. run `python setup.py install` in folder `erpc/erpc_python/`
2. `pip install erpc`: only major eRPC versions are available through pypi

# Example files
- `client_main.c`: main client example file
- `server_main.c`: main server example file
- `erpc_c/`: folder with the base code for C example - run make
- `service/erpc_matrix_multiply.erpc`: eRPC IDL file for example
- `service/erpc_matrix_multiply/`: eRPC output shim code generated from IDL file

# Running the example
- ./server to run the server
- ./client to run the client 
- By default is host set as localhost and port as 40.


The log below shows the output of the *eRPC Matrix Multiply* example in the terminal window:

```
$ ./server 
inside erpc_transport_tcp_init
transport successfully constructed
transport successfully init
eRPC intialized
MatrixMultiply service added
Before calling server run
Calculating the matrix multiplication...
Done!

$ ./client
inside erpc_transport_tcp_init
transport successfully constructed
transport successfully init
Initializing client side
Calling eRPC matrix multiply on server side
Here is the result of the matrix multiplication: 
result_matrix[0][0] = 12629	result_matrix[0][1] = 12750	result_matrix[0][2] = 12835	result_matrix[0][3] = 14070	result_matrix[0][4] = 8908	
result_matrix[1][0] = 11164	result_matrix[1][1] = 16342	result_matrix[1][2] = 12924	result_matrix[1][3] = 11936	result_matrix[1][4] = 7066	
result_matrix[2][0] = 17390	result_matrix[2][1] = 18378	result_matrix[2][2] = 20231	result_matrix[2][3] = 21953	result_matrix[2][4] = 12511	
result_matrix[3][0] = 6895	result_matrix[3][1] = 6506	result_matrix[3][2] = 6791	result_matrix[3][3] = 6716	result_matrix[3][4] = 5068	
result_matrix[4][0] = 13608	result_matrix[4][1] = 8586	result_matrix[4][2] = 12264	result_matrix[4][3] = 13074	result_matrix[4][4] = 9249

```
