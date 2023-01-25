#! /usr/bin/python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

# Standard config file for erpcgen tests

import sys
import os
from py import path

# Get the OS name used in the erpcgen build path.
if sys.platform == 'darwin':
    os_name = "Darwin"
elif sys.platform == 'cygwin':
    os_name = "CYGWIN_NT-6.1"
elif sys.platform.startswith('linux'):
    os_name = "Linux"

# Get path to this file's directory.
this_dir = path.local(__file__).dirpath()

# Set erpcgen path. An environment variable takes precedence.
if 'ERPCGEN' in os.environ:
    ERPCGEN = os.environ['ERPCGEN']
elif sys.platform == 'win32':
    ERPCGEN = str(this_dir.join(r"..\VisualStudio_v14\Release\erpcgen.exe"))
else:
    ERPCGEN = str(this_dir.join("../../Release/{}/erpcgen/erpcgen".format(os_name)))

# Set path to C/C++ compiler.
if 'CC' in os.environ:
    CC = os.environ['CC']
else:
    CC = 'gcc'

# Number of test runs to keep.
RUN_KEEP_COUNT = 3
