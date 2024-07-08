Zephyr eRPC tests
#################

This folder contains all the necessary files to test eRPC with Zephyr.
The folder contains two folders for dual core testing - `rpmsglite/` and PC-Board testing via UART - `uart/`.

To build images for the primary and secondary core, Zephyr requires the use of `sysbuild`. The primary `CMakeLists.txt` is 
located in the unit test folder, while the `CMakeLists.txt` for the secondary core is 
located in the nested `remote` folder. The `sysbuild.cmake` directs the Zephyr build system to 
first build the secondary core and then link it to the primary image.

Directory structure
*******************

* **cmake/** CMake files used for building tests
* **rpmsglite/cmake/** CMake files for sysbuild and tests 
* **uart/cmake/** CMake files for tests 
* **rpmsglite/config/** Configuration files for both cores
* **uart/config/** Configuration files for UART tests
* **rpmsglite/test\_xxx/** Directories with tests. Contains primary and remote CMakeLists.txt, sysbuild.cmake and testcase.yaml
* **uart/test\_xxx/** Directories with tests. Contains CMakeLists.txt and testcase.yaml

Running tests
*************

Note: To generate eRPC code, CMake requires `erpcgen` executable to be in Path.
If you have already builded `erpcgen`, add it to the path. Windows:

.. code-block:: console

    $env:PATH = $env:PATH + ';C:\path\to\erpc\erpcgen\' # Windows 

All tests (RPMSG-Lite and UART) can be executed with Zephyr's Twister:

.. code-block:: console

    west twister --device-testing --device-serial  COM8 -T .\erpc\test\zephyr -p mimxrt1160_evk_cm7 --west-flash --west-runner=jlink

For executing tests on one core boards (mimxrt1060_evk):

.. code-block:: console

    west twister --device-testing --device-serial  COM8 -T .\erpc\test\zephyr\uart -p mimxrt1060_evkb

* `--device-testing` Run tests on device 
* `--device-serial` Select port where is the device connected
* `-T` Path where Twister looks for test cases (testcase.yaml)
* `-p` Name of the device
* `--west-flash` is required to be able build sysbuild tests. 
* `--west-runner` Runner used to flash board

Test's structure notes
######################

Each test suite has it's own `cmake/unit_test.cmake`. Each test has its own folder with a CMakeLists files that include `cmake/unit_test.cmake`, which includes
Zephyr and all necessary variables. These variables can be overridden in the test's CMakeLists file. Finally, the `zephyr/cmake/test.cmake`` file is included. 
Ensure that all sources and includes are added to the test when included. If the client is involved, add the gtest as well.

`cmake/unit_test.cmake` has to define `ERPC_ROOT_DIR`, `ZEPHYR_TEST_CMAKE_DIR` and `ERPC_ROOT_DIR`. 
Additionally `TRANSPORT` and `APP_TYPE` has to be specified in test's `CMakeLists.txt` or in `unit_test.cmake` before including `zephyr/cmake/variables.cmake`.

Variable
********

* **RPMSG_LITE_REMOTE_BOARD** Remote board selected based on build board
* **APP_TYPE** CLIENT/SERVER, specified in projects's CMakeLists
* **TRANSPORT** In this case always `zephyr_uart` or `zephyr_rpmsg_lite`
* **TEST_NAME** Name of the test - test_binary, ...
* **TEST_SOURCE_DIR** Source dir of the test implementation 
* **TEST_COMMON_DIR** Common dir of the erpc test (`erpc/test/common``)
* **ERPC_NAME** Default `test`, can be overridden 
* **ERPC_NAME_APP** Default ERPC_NAME, overridden
* **TEST_OUT_DIR** The binary directory where the test is builded
* **ERPC_OUT_ROOT_DIR** The root directory for the erpc shim code is generation
* **ERPC_OUT_DIR** Name of the directory where shim code is generated (Default: erpc_outputs)
* **TEST_ERPC_OUT_DIR** The directory where the erpc shim code is generated
* **GENERATE_ERPC_IDL_FILES** If FALSE, test.cmake file will not generate the shim code
* **IDL_FILE** IDL file that is used for generating
* **TEST_EXT_SOURCES** External sources that are added to the app
* **TEST_EXT_INCLUDES** External includes that are added to the app

Files
*****

* **cmake/test.cmake** Generate shim code, add sources and includes to the target, link Configuration files to erpc and rpmsg_lite
* **cmake/variables.cmake** Set default variables 
* **{rpmsglite,uart}/cmake/unit_test.cmake** Main cmake for server and client, includes Zephyr package and variables.cmake
* **rpmsglite/cmake/sysbuild.cmake** Common sysbuild for all test. Has to be included from test's folder in file named **sysbuild.cmake**
* **rpmsglite/cmake/rpmsg_lite.cmake** Used in sysbuild.cmake to determinate remote board
* **{rpmsglite,uart}/test_xxx/testcase.yaml** Define test, so it can be discoverd by Twister
* **{rpmsglite,uart}/test_xxx/CMakeLists.cmake** CMake for the test
* **rpmsglite/test_xxx/sysbuild.cmake** Includes cmake/sysbuild.cmake. Used by Zephyr's sysbuild system
* **rpmsglite/test_xxx/remote/CMakeLists.cmake** CMake for the server 