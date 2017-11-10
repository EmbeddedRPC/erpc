#! /usr/bin/python

# Copyright 2016-2017 NXP
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
                    subprocess.call(["clang-format-3.9", "-i", file])
    print('*****************************************************************************\n')
