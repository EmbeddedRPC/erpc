#! /usr/bin/python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# o Redistributions of source code must retain the above copyright notice, this list
#   of conditions and the following disclaimer.
#
# o Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# o Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    sys.exit()
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
            sys.exit()

unitTestPath = "./test/"
# enter Unit Test Directory
os.chdir(unitTestPath)
# get names of unit test subdirs
dirs = os.listdir(os.curdir)

# Get all unit test directories
testDirs = filter(isTestDir, dirs)

build = "build=" + target

for dir in testDirs:
    # skip doxy comment testing. There is nothing to run, only need to see output.
    if dir == "test_others":
        continue
    print(bcolors.BLUE + "\nRunning " + bcolors.ORANGE + dir + bcolors.BLUE +" unit tests with "
            + bcolors.ORANGE + transportLayer + bcolors.BLUE + " transport layer." + bcolors.ENDC)
    os.chdir(dir)
    call(["make", build, testServerCommand])
    call(["make", build, testClientCommand])
    os.chdir('..')

# For completeness, change back to erpc/ directory
os.chdir('..')
