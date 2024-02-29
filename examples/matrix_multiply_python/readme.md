# Overview

This example demonstrates usage of eRPC between two PC's or boards (e.g. i.MX) with Python using TCP transport layer. One side acts like a server and the second as a client. When client starts, it generates two random matrixes and sends them to the server. Server then performs matrix multiplication and sends the result matrix back to the client. Client then prints the result matrix.

Example can be ran on Personal computer or on boards with Python installed, e.g. i.MX (i.MX7SD, i.MX6SX, etc.)

## eRPC documentation

- eRPC is open-source project stored on github: [https://github.com/EmbeddedRPC/erpc](https://github.com/EmbeddedRPC/erpc)
- eRPC documentation can be also found in: [https://github.com/EmbeddedRPC/erpc/wiki](https://github.com/EmbeddedRPC/erpc/wiki)

## Prerequisites

- Python 3.6+

## eRPC installation

1. run `python setup.py install` in folder [erpc/erpc_python/](/erpc_python)
2. `pip install erpc`: only major eRPC versions are available through pypi

## Example files

- [matrix_multiply.py](matrix_multiply.py): main example file
- [service/erpc_matrix_multiply.erpc](service/erpc_matrix_multiply.erpc): eRPC IDL file for example
- [service/erpc_matrix_multiply](service/erpc_matrix_multiply): eRPC output shim code generated from IDL file

## Running the example

- Run [matrix_multiply.py](matrix_multiply.py) with `-s` (or `--server`) parameter to run server
- Run [matrix_multiply.py](matrix_multiply.py) with `-c` (or `--client`) parameter to run client
- Both server and client has to have specified host and port with `-t` (`--host`) and `-p` (`--port`) parameters. By default is host set as localhost and port as 40.

### Example

```bash
  python matrix_multiply.py --server --host 192.168.1.10 --port 40
  python matrix_multiply.py --client --host 192.168.1.10 --port 40
```

The log below shows the output of the *eRPC Matrix Multiply* example in the terminal window:

```bash
$ python matrix_multiply.py --server
eRPC Matrix Multiply TCP example
Server created on localhost:40

Wait for client to send a eRPC request

Server received these matrices:

Matrix #1
=========
0022 0039 0049 0031 0043
0039 0006 0048 0029 0011
0048 0005 0011 0005 0002
0011 0038 0049 0043 0005
0013 0040 0004 0036 0004

Matrix #2
=========
0031 0020 0003 0003 0027
0003 0025 0022 0010 0013
0044 0004 0016 0027 0018
0012 0015 0048 0040 0039
0004 0043 0005 0015 0047

Result matrix
=========
3499 3925 3411 3664 5213
3731 2030 2464 2798 3643
2055 1290 0680 0721 1848
3147 2226 3742 3531 3585
1147 1988 2731 2047 2535
```

```bash
$ python matrix_multiply.py --client
eRPC Matrix Multiply TCP example
Client connecting to a host on localhost:40

Matrix #1
=========
0022 0039 0049 0031 0043
0039 0006 0048 0029 0011
0048 0005 0011 0005 0002
0011 0038 0049 0043 0005
0013 0040 0004 0036 0004

Matrix #2
=========
0031 0020 0003 0003 0027
0003 0025 0022 0010 0013
0044 0004 0016 0027 0018
0012 0015 0048 0040 0039
0004 0043 0005 0015 0047

eRPC request is sent to the server

Result matrix
=========
3499 3925 3411 3664 5213
3731 2030 2464 2798 3643
2055 1290 0680 0721 1848
3147 2226 3742 3531 3585
1147 1988 2731 2047 2535

Press Enter to initiate the next matrix multiplication
```
