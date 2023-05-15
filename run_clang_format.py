#! /usr/bin/python

# Copyright 2016-2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

# To use this script run
# $./run_clang_format.py
from __future__ import print_function
import subprocess
import sys
import os

# Folders to scan
folders = [
    "erpc_c",
    "erpcgen/src",
    "erpcsniffer/src",
    "test"]

# Files which will be not formatted
exceptions = [
    "test/common/gtest/gtest.h",
    "test/common/gtest/gtest.cpp",
    "erpc_c/port/erpc_serial.cpp",
    "erpcgen/src/cpptemplate/cpptempl.hpp",
    "erpcgen/src/cpptemplate/cpptempl.cpp",
    "erpcgen/src/cpptemplate/cpptempl_test.cpp"]

# For windows use "\\" instead of "/" path separators.
if os.environ.get('OS', '') == 'Windows_NT':
    folders = [os.path.normpath(folder) for folder in folders]
    exceptions = [os.path.normpath(e) for e in exceptions]

# Files with this extensions will be formatted/
extensions = [".h", ".hpp", ".c", ".cpp"]

# Check that the clang-format is installed and matches the required version. The clang-format binary
# chosen is specified by the environment variable $CLANG_FORMAT_PATH or "clang-format" using the
# regular $PATH resolution mechanism if $CLANG_FORMAT_PATH is undefined.
clang_format_path = os.environ.get("CLANG_FORMAT_PATH")
clang_format_bin = clang_format_path if clang_format_path is not None else "clang-format"
try:
    cf = subprocess.Popen(
        [clang_format_bin, "--version"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
except FileNotFoundError:
    print("clang-format is not in $PATH")
    exit(1)
clang_format_stdout, clang_format_stderr = cf.communicate()
if cf.returncode != 0:
    print("Failed to determine clang-format version. stderr=\"%s\"" % (
        clang_format_stderr.decode('utf-8')))
    exit(1)
clang_format_stdout = clang_format_stdout.decode("utf-8")
if "clang-format version 10.0.0" not in clang_format_stdout:
    print("clang-format is not the required version: 10.0.0")
    exit(1)

# processing formatting
for folder in folders:
    print('*****************************************************************************')
    print(folder)
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
