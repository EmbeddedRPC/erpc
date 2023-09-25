#!/bin/bash

# exit when any command fails
set -e

make clean
if [ "$1" = "clang" ]; then
    echo "Compiling by clang compiler."
    CC=clang CXX=clang++ make all
else
    echo "Compiling by default gnu compiler."
    CC=gcc CXX=g++ make all
fi
