/multicore/erpc/test/readme.txt

Directory Structure

common - Contains board specific code common to all tests to help enable testing
on target platforms and common files for tests.

mk - Contains common makefiles for building tests.

results - Contains the results for all unit tests in xml format.

skeleton - Holds skeleton files that can be used if a new unit test directory
needs to be added.

Each test_xxx/ subdirectory contains acceptance tests. These tests can be built
on Linux or OS X with gcc using makefiles. Run '$make' inside one of the
test_xxx/ folders to build the test client and test server. Then run
'$make run-ut-server' to start the server, and run '$make run-ut-client' to run
the unit tests.

