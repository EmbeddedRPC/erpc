#!/usr/bin/env python

# Copyright 2016-2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

import os
from typing import Any
# Yocto and python27 combination
if "IS_YOCTO" in os.environ:
    from . import erpc_version
else:
    from . import arbitrator
    from . import basic_codec
    from . import codec
    from . import client
    from . import crc16
    from . import server
    from . import simple_server
    from . import transport

class Reference(object):
    """ Simple container class used for pass by reference.
    """

    def __init__(self, value: Any = None):
        # Read/write attribute holding the referent.
        self.value = value

    def __str__(self):
        return "<%s@%x value=%s>" % (self.__class__.__name__, id(self), repr(self.value))

    def __repr__(self):
        return self.__str__()
