#!/bin/bash

TEST_FOLDER=$1

if [ $# -ne 1 ];
then
    echo "ERROR: Please only pass in the name of the directory you wish to create."

else
    #create test folder
    mkdir "${TEST_FOLDER}"

    #create skeleton files
    cat Makefile.unit_test > "${TEST_FOLDER}/Makefile"

    cat client_skeleton.cpp > "${TEST_FOLDER}/${TEST_FOLDER}_client_impl.cpp"

    cat server_skeleton.cpp > "${TEST_FOLDER}/${TEST_FOLDER}_server_impl.cpp"

    cat idl_skeleton.erpc >  "${TEST_FOLDER}/${TEST_FOLDER}.erpc"
fi
