# Hello world example

This example shows basic usage of eRPC framework.

## Prerequisites

Run these commands in this folder based on needs with correct path to erpcgen application

C/C++ shim code:

```bash
erpcgen -gc -o shim/py hello_world.erpc
```

Python shim code:

```bash
erpcgen -gpy -o shim/py hello_world.erpc
```

In case of C/C++ build application

## Run example

First run server, then client.
