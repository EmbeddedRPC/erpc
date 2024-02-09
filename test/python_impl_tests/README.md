# Python implementation testing directory #

This directory contains acceptance tests for Python implementation.
To trigger the execution with using TCP communication run **pytest -vs --client --server** from this folder or 
enter the *scripts/test_case* subfolder and run individual test using **pytest -vs --client --server** 
To trigger the execution with using Serial communication run **pytest -vs --client --serial="COM_ID"** from the 
*scripts/test_case* subfolder and run individual test using **pytest -vs --client --serial="COM_ID"** 
