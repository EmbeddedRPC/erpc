# eRPC Zephyr support

## Adding eRPC to Zephyr as module

To include eRPC Zephyr, update Zephyr's west.yml:

``` yml
  projects:
    - name: erpc
      revision: <revision with Zephyr support>
      url: <erpc repository>
      path: modules/lib/erpc
```
Then run:
``` sh
west update erpc
```

## eRPC module files

- *erpc/zephyr/* - This directory holds all files required for Zephyr module
- *erpc/zephyr/module.yml* - This file defines module name and Cmake and Kconfig location
- *erpc/zephyr/CMakeList.txt* - Defines module's includes and source files base on Kconfig
- *erpc/zephyr/Kconfig* - Defines eRPC module configuration

### Kconfig

Kconfig options:

- **CONFIG_ERPC** - enable eRPC support
- **CONFIG_ERPC_TRANSPORT_UART** - enable UART transport layer
- **CONFIG_ERPC_TRANSPORT_RPMSG_LITE** - enable RPMSG-Lite transport layer
  - To use *RPMSG-Lite* transport layer, RPMSG-Lite module is required.
  - RPMSG-Lite module has to be included in Zephyr
  - Enable RPMSG-Lite module by this options:
    - CONFIG_RPMSGLITE=y
    - CONFIG_RPMSGLITE_QUEUE=y
    - CONFIG_RPMSGLITE_NS=y
- **CONFIG_ERPC_ARBITRATED_CLIENT** - enable arbitrated client

### CMakeList

CMakeLists.txt defines eRPC as Zephyr library. Includes all required directories and source files for Zephyr environment. Optionaly import uart/rpmsg-lite/ns source files.

## Samples

Samples are located in *examples/zephyr/*. We can build the examples using west tool:

### Matrix multiply with RPMSG-Lite transport

```sh
cd path/to/zephyrproject
west build --sysbuild -b <supported board> modules/lib/erpc/examples/zephyr/matrix_multiply_rpmsglite/ -p
west flash
```

### Matrix multiply with UART transport

```sh
cd path/to/zephyrproject
west build -b <board> modules/lib/erpc/examples/zephyr/matrix_multiply_uart/ -p
west flash
```
