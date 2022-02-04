eRPC Python Infrastructure
==========================

This folder contains the Python implementation of the eRPC infrastructure.

The eRPC project is stored on Github: https://github.com/EmbeddedRPC/erpc

The Python implementation of eRPC is fully compatible with the C/C++ implementation at the
protocol level. Also, the classes mirror those in the C++ infrastructure.

Installation:

   To install the eRPC Python infrastructure, run the setup.py script like this:

       pip install erpc


   Once installed, you can access the infrastructure via a standard import statement.

       import erpc
       xport = erpc.transport.SerialTransport("/dev/ttyS1", 115200)
       client = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)

