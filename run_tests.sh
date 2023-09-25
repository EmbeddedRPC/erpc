#!/bin/bash

# exit when any command fails
set -e

pytest erpcgen/test/
python3 test/run_unit_tests.py
