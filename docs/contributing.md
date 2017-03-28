Contributing
-------------
## Contributing to eRPC
The eRPC project is placed on github. Contributing can be managed there via pull-request. Before a pull-request is created, several tasks needs to be done: the code needs to be tested and properly formatted.

### How to test eRPC code
Two types of tests are used: pytest-based tests and acceptance tests.

#### erpcgen pytest based tests
These tests can be found in the ``erpc/erpcgen/test`` folder. Pytest can be executed via calling the pytest command from that directory. These tests have declared test cases in `.yml` files. Each test case has defined how an IDL file looks like and what is an expected output from that IDL file. Then pytest generatess the IDL file and calls the erpcgen application using that file. The output from the erpcgen application is controlled based on what was defined. More information can be found at [pytest](http://doc.pytest.org/en/latest/example/nonpython.html). Every time when new feature is added or bug is fixed, the test which covers that situation should be added.

#### eRPC acceptance tests
The acceptance tests are placed in the ``erpc/test`` folder. These tests can be executed on Linux/Mac OS-based systems. When the use of makefiles shim code for tests is generated (``make test`` or ``make all``), then tests can be executed with python script ``erpc/test/run_unit_tests.py``. These tests are testing the whole infrastructure of the eRPC application.

### How to format eRPC code
To format code, use the application developed by Google, named *clang-format*. This tool is part of the [llvm](http://llvm.org/) project. Currently, the eRPC is using the latest version of clang-format 3.9.
The set of style settings used for clang-format is defined in the `.clang-format` file, placed in a root of the eRPC directory where Python script ``run_clang_format.py`` can be executed. This script executes an application named *clang-format-3.9*. You need to have the path of this application in the OS's environment path, or you need to change the script.
