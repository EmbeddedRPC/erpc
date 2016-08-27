eRPC Python Infrastructure
==========================

This folder contains the Python implementation of the eRPC infrastructure.

The Python implementation of eRPC is fully compatible with the C/C++ implementation at the
protocol level. Also, the classes mirror those in the C++ infrastructure.


## Contents

`erpc` - Python package for eRPC infrastructure.

`setup.py` - setuptools setup script that installs the erpc package.

`test.py` - Example Python client and server in one file. The server is selected by passing `-s` on the command line, the client is selected with `-c`.

`run_test.sh` - Script to generate the Python shim code and run test.py as both server and client. It ensures that erpcgen is built.

## Installation

To install the eRPC Python infrastructure, run the setup.py script like this:

    python setup.py install

Alternatively, you may use pip to install the package, like this:

    pip install .


Once installed, you can access the infrastructure via a standard import statement.

    >>>import erpc
    >>>xport = erpc.transport.SerialTransport("/dev/ttyS1", 115200)
    >>>client = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)

