#! /usr/bin/python

# Copyright 2016-2017 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

# To use this script run
# $./run_clang_format.py
from __future__ import print_function
import subprocess
import sys,os

#Folders to scan
folders = [
    "erpc_c",
    "erpcgen/src",
    "erpcsniffer/src",
    "test"]

#Files which will be not formatted
exceptions = [
    "test/common/gtest/gtest.h",
    "test/common/gtest/gtest.cpp",
    "erpcgen/src/cpptemplate/cpptempl.h",
    "erpcgen/src/cpptemplate/cpptempl.cpp",
    "erpcgen/src/cpptemplate/cpptempl_test.cpp"]

#For windows use "\\" instead of "/" path separators.
if os.environ.get('OS','') == 'Windows_NT':
    folders = [os.path.normpath(folder) for folder in folders]
    exceptions = [os.path.normpath(e) for e in exceptions]

#Files with this extensions will be formatted/
extensions = [".h", ".hpp", ".c", ".cpp"]

#processing formatting
for folder in folders:
    print('*****************************************************************************')
    print(folder);
    for path, subdirs, files in os.walk(folder):
        for name in files:
            if any(ext in name for ext in extensions):
                file = os.path.join(path, name)
                if file in exceptions:
                    print("Ignored: ", file)
                else:
                    print("Formatting: ", file)
                    subprocess.call(["clang-format", "-i", file])
    print('*****************************************************************************\n')
