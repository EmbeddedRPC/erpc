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
folders = []
folders.append("erpc_c");
folders.append("erpcgen/src");
folders.append("erpcsniffer/src");
folders.append("test");

#Files which will be not formatted
exceptions = []
exceptions.append("test/common/gtest/gtest.h");
exceptions.append("test/common/gtest/gtest.cpp");
exceptions.append("erpcgen/src/cpptemplate/cpptempl.h");
exceptions.append("erpcgen/src/cpptemplate/cpptempl.cpp");
exceptions.append("erpcgen/src/cpptemplate/cpptempl_test.cpp");

#For windows use "\\" instead of "/" path separators.
if os.environ.get('OS','') == 'Windows_NT':
    for i, folder in enumerate(folders):
        folders[i] = os.path.normpath(folder)

    for i, ext in enumerate(exceptions):
        exceptions[i] = os.path.normpath(ext)

#Files with this extensions will be formatted/
extensions = []
extensions.append(".h")
extensions.append(".hpp")
extensions.append(".c")
extensions.append(".cpp")

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
                    subprocess.call(["clang-format-5.0", "-i", file])
    print('*****************************************************************************\n')
