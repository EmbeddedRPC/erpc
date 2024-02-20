# eRPC Python Infrastructure

This folder contains the Python implementation of the eRPC infrastructure.

The Python implementation of eRPC is fully compatible with the C/C++ implementation at the
protocol level. Also, the classes mirror those in the C++ infrastructure.

## Contents

[erpc](erpc) - Python package for eRPC infrastructure.

[setup.py](setup.py) - setuptools setup script that installs the erpc package.

[setup.cfg](setup.cfg), [README_Pypi.md](README_Pypi.md) - necessary files for the pypi support.

## Installation

Pip package installer needs to be installed in your Python version. Use following way for Python3 pip installation:

```sh
sudo apt-get install python3 python3-pip
```

To install the eRPC Python infrastructure from pypi:

```sh
pip install erpc
```

To install the eRPC Python infrastructure from local folder ([erpc/erpc_python](/erpc_python)):

```sh
pip install .
```

Once installed, you can access the infrastructure via a standard import statement.

```python
import erpc
xport = erpc.transport.SerialTransport("/dev/ttyS1", 115200)
client = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)
```
