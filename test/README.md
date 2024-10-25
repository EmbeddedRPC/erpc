# eRPC Tests

## Directory Structure

- [*common/*](common/) - Contains board specific code common to all tests to help enable testing
on target platforms and common files for tests.
- [*java_impl_tests/*](java_impl_tests/) - Java implementation of the tests (Contains README)
- [*mk/*](mk/) - Contains common makefiles for building tests
- [*python_impl_tests/*](python_impl_tests/) - Python implementation of the tests (Contains README)
- [*results/*](results/) - Contains the results for all unit tests in xml format.
- [*test_xxx/*](test_xxx/) - Subdirectories with acceptance tests
- [*zephyr/*](zephyr/) - Zephyr implementation of the tests (Contains README)


## Running test with script 
To make building and testing easier, we can use *run_unit_tests.py* (full usage is at the end of this readme).
The script generate and build cmake project, and then run the tests. It is possible to specify the client and server implementation.

Script also support testing against zephyr. Specify the board and serial port that would be used by Zephyr.
The script build and flash Zephyr app and then run client to test cases one by one. For testing Python-Zephyr Zephyr's testing framework Twister can be used too. See *erpc/test/zepyr/README.rst*.
```bash
python run_unit_tests.py --menuconfig # Edit C++ tests generation setting (or by editing test/prj.conf)
python run_unit_tests.py --client c --server python # Run c-python tests
python run_unit_tests.py -p --client c --server python # Remove old build directory, build, and run c-python tests
python run_unit_tests.py --client python --server python test_arrays # Run only 'test_arrays'
python run_unit_tests.py --client all --server all # Run all combinations C/Python/Java
python run_unit_tests.py --generate-shim # Only generate shim code for Python/Java
python run_unit_tests.py --zephyr '<path_to_zephyr_base>' --client python --server zephyr --serial-port COM4 --board mimxrt1060_evkb
```

## Building and running tests with KConfig and CMake

### Building
Building test with cmake from two steps - generating cmake project and building the test.
To generate cmake project in [erpc/](./..) execute:

```bash
cmake -B ./build -DCONF_FILE="$(pwd)/test/prj.conf" # Generate project with test/prj.conf. '-DCONF_FILE' must be absolute path
cmake --build ./build # Build the project/test
```

More options for building the tests:
```bash
cmake --build . --target menuconfig # Open menuconfig to select test case, transport parameters, server/client parst of the tests
cmake --build . --target test_all # Build all tests (enabled by kconfig)
cmake --build . --target test_arrays # Build only client/server for test_arrays (all enabled transport).
cmake --build . --target test_enums_server_tcp # Build only TCP server for test_enums
```

### Running

After building tests with cmake, all test's executables are in build dir:
***<build_dir>/test/test_xxx/test_xxx_{client|server}_{transport}***.

Run server and then client executable to get the test result.

### Cmake Notes

Main CMakeLists.txt in *test/*:
- set ERPC_TEST_ROOT, TEST_COMMON_DIR variables
- build gtest library
- include *erpc/cmake/test.cmake*
- create custom target 'test_all' that build all test cases
- load all test_xxx and create target 'test_xxx' for each test case
- for each test case enabled by kconfig, include test's subdirectory.
- generate python shim code if enabled and add dependency

*erpc/cmake/test.cmake*:
- function generate_erpc_test_variables() - generate variables for test it's called from
  - **TEST_NAME** - test name
  - **TEST_SOURCE_DIR** - test c sources dir
  - **TEST_COMMON_DIR** - common sources dir
  - **ERPC_NAME** - name of shim code test
  - **ERPC_NAME_APP** - erpc app name (default equal to ERPC_NAME, but some tests change it)
  - **TEST_OUT_DIR** - Directory where test is generated (<build_dir>/test/test_xxx/)
  - **ERPC_OUT_DIR** - Where shim code is generated
  - **IDL_FILE** - test's IDL file
  - **CONFIG_DIR** - From where 'erpc_config.h' should be taken
- function add_erpc_test() - Generate the test from sources base on test's variables
- function generate_erpc_test() - call add_erpc_test() for basic tests that do not need extra parameters. used in test's CMakeLists.txt

*test_xxx/CMakeLists.txt*:
- Each test calls generate_erpc_test_variables(), edit them if necessary
- Call generate_erpc_test() to generate the test
- If custom sources/setting is required add_erpc_test() is called instead


### run_unit_tests.py --help
```
usage: run_unit_tests.py [-h] [-B BUILD_DIR] [-b] [-p] [--c-compiler C_COMPILER] [--cxx-compiler CXX_COMPILER] [-T {Debug,Release}] [--transport {tcp,serial}] [--conf-file CONF_FILE] [--zephyr-base ZEPHYR_BASE]
                         [-C {python,c,java,all}] [-S {zephyr,python,c,java,all}] [--result-dir RESULT_DIR] [--tcp-port TCP_PORT] [--board BOARD] [--serial-port SERIAL_PORT] [--menuconfig] [--add-test-prefix] [--maven MAVEN]    
                         [--erpcgen ERPCGEN] [--generate-shim]
                         [test_cases ...]

Run eRPC unit tests.
Currently supported options (client - server):
    - C - C
    - C - Python
    - C - Java
    - C - Zephyr
    - Python - C
    - Python - Python
    - Python - Java
    - Python - Zephyr
    - Java - C
    - Java - Python
    - Java - Java
    - Java - Zephyr

positional arguments:
  test_cases            List of tests to execute.

options:
  -h, --help            show this help message and exit
  -B BUILD_DIR, --build-dir BUILD_DIR
                        Cmake build dir. If not specified, 'erpc/build' is used.
  -b, --build           Build all before running tests.
  -p, --pristine        Clear the CMake build dir.
  --c-compiler C_COMPILER
                        Set CMake default compiler for C (-DCMAKE_C_COMPILER=<compiler>).
  --cxx-compiler CXX_COMPILER
                        Set CMake default compiler for C++ (-DCMAKE_CXX_COMPILER=<compiler>).
  -T {Debug,Release}, --target {Debug,Release}
                        Set CMake build type (-DCMAKE_BUILD_TYPE=<target>).
  --transport {tcp,serial}
                        Set transport layer if there is more options, for example Python-Python only support TCP now.
  --conf-file CONF_FILE
                        Set Kconfig config file. Default test/prj.conf
  --zephyr-base ZEPHYR_BASE
                        Path to Zephyr base directory
  -C {python,c,java,all}, --client {python,c,java,all}
                        Set what client should be used. Default C.
  -S {zephyr,python,c,java,all}, --server {zephyr,python,c,java,all}
                        Set what server should be used. Default C.
  --result-dir RESULT_DIR
                        Path where xml results from gtest and pytest should be stored. Result files have format '[pytest|gtest]_[test_name]_[transport]_[client]_[server].xml'
  --tcp-port TCP_PORT   TCP port used for pytest. TCP port for C test should be specified in prj.conf
  --board BOARD         Board that would be used for zephyr build.
  --serial-port SERIAL_PORT
                        Serial port that would be used for zephyr build.
  --menuconfig          If selected, show menuconfig before building. This allows user to edit tests parameters and choose what to build.
  --add-test-prefix     Add prefix to testcase name corresponding to current test setting. TCP Pytohn - C test -> tcp_python_c_test_testName
  --maven MAVEN         Path to maven for Java tests. Default 'mvn'.
  --erpcgen ERPCGEN     Path to erpcgen. Default 'erpcgen'.
  --generate-shim       Generate shim code for Python and Java tests.
```