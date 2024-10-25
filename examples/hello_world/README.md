# Hello world example

This example shows basic usage of eRPC framework.

## Prerequisites

To build and run this example, you need to install all the necessary requirements for each programming language. For `C/C++`, you need to install Cmake and a compiler. For `Python`, you need to install Python and erpc from pip or source. For `Java`, you need to have `JDK21` and `Maven`. For more information, please check the source folder for each programming language (`erpc_c`, `erpc_python`, `erpc_java`).

### Shim code

Run these commands in this folder based on needs with **correct path** to erpcgen application

C/C++ shim code:

```bash
erpcgen -gc -o ./c/shim/ hello_world.erpc
```

Python shim code:

```bash
erpcgen -gpy -o ./py/shim/ hello_world.erpc
```

Java shim code:

```bash
erpcgen -gjava -p org.example.hello_world -o ./java/src/main/java/org/example/ ./hello_world.erpc
```

## Run example

First run server, then client.

### C/C++

This example use CMake to create executables. In [erpc/](../../) run `cmake -B ./build` (or `cmake --preset mingw64` on win with local mingw) to generate Cmake project. Then run `cmake --build ./build/ --target menuconfig` and enable _Build eRPC Examples -> hello_world_ example (optionally disable _erpcgen_ and _eRPC C lib_). This can be also done by enabling in [erpc/prj.conf](../../prj.conf).

Execute `cmake --build ./build/` to build the example.

Cmake generates four executables (`client`, `server`, `client_cpp`, `server_cpp`) in [build/examples/hello_world](../../build/examples/hello_world).

### Python

To run python example go to folder [py/](./py) and run `python main_server.py` or `python main_client.py` 

### Java

Go to folder [java/](./java). Then execute `mvn package` to install dependency and create `.jar` file. Then run `java -jar .\target\hello_world-1.0.jar client` or `java -jar .\target\hello_world-1.0.jar server`.

If you have any problem with Java or Maven, check [erpc_java/readme.md](../../erpc_java/readme.md).



