#! /usr/bin/python

# The Clear BSD License
# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted (subject to the limitations in the disclaimer below) provided
# that the following conditions are met:
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
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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
    ERPCGEN = str(this_dir.join(r"..\VisualStudio_v14\Debug\erpcgen.exe"))
else:
    ERPCGEN = str(this_dir.join("../../Debug/{}/erpcgen/erpcgen".format(os_name)))

# Set path to C/C++ compiler.
if 'CC' in os.environ:
    CC = os.environ['CC']
else:
    CC = 'gcc'

# Number of test runs to keep.
RUN_KEEP_COUNT = 3

