#! /usr/bin/python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

# To use this script, first run
# $make test
# to build all of the test files.
# then run
# $./run_unit_tests.py [tcp]
# to run this script with optional transport layer argument
from subprocess import call
import re
import os
import sys

#define output text colour class
class bcolors:
    GREEN = '\033[36m'
    BLUE = '\033[38;5;097m'
    ORANGE= '\033[38;5;172m'
    RED = '\033[31m'
    ENDC = '\033[0m'

def isTestDir(dir):
    regex = re.compile('test_*')
    if os.path.isdir(dir) and re.match(regex, dir):
        return True
    else:
        return False

testClientCommand = "run-tcp-client"
testServerCommand = "run-tcp-server"
transportLayer = "tcp"
target = "debug"

# Process command line options
# Check for 2 or more arguments because argv[0] is the script name
if len(sys.argv) > 2:
    print ("Too many arguments. Please specify only the transport layer to use. Options are: tcp")
    sys.exit(1)
if len(sys.argv) >= 2:
    for arg in sys.argv[1:]:
        if arg == "tcp":
            testClientCommand = "run-tcp-client"
            testServerCommand = "run-tcp-server"
            transportLayer = "tcp"
        elif arg == "-d":
            target = "debug"
        elif arg == "-r":
            target = "release"
        else:
            print("Invalid argument/s. Options are: tcp, -r, -d\n")
            sys.exit(1)

unitTestPath = "./test/"
# enter Unit Test Directory
os.chdir(unitTestPath)
# get names of unit test subdirs
dirs = os.listdir(os.curdir)

# Get all unit test directories
testDirs = filter(isTestDir, dirs)

build = "build=" + target
testsExitStatus = 0

for dir in testDirs:
    print(bcolors.BLUE + "\nRunning " + bcolors.ORANGE + dir + bcolors.BLUE +" unit tests with "
            + bcolors.ORANGE + transportLayer + bcolors.BLUE + " transport layer." + bcolors.ENDC)
    os.chdir(dir)
    call(["make", build, testServerCommand])
    testsExitStatus += call(["make", build, testClientCommand])
    os.chdir('..')

# For completeness, change back to erpc/ directory
os.chdir('..')

if testsExitStatus != 0:
    print("\n\nSome tests end with failures.\n\n")
    sys.exit(testsExitStatus)
