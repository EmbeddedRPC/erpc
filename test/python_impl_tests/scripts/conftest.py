#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import pytest


def pytest_addoption(parser):
    parser.addoption("--client", action="store_true", default=False, help="run all combinations")
    parser.addoption("--server", action="store_true", default=False, help="run all combinations")
    parser.addoption("--host", action="store", default='localhost', help="run all combinations")
    parser.addoption("--port", action="store", default='12345', help="run all combinations")
    parser.addoption("--serial", action="store", default=None, help="run all combinations")
    parser.addoption("--baud", action="store", default='115200', help="run all combinations")

#def pytest_exception_interact(node, call, report):
#    excinfo = call.excinfo
#    if 'script' in node.funcargs:
#        excinfo.traceback = excinfo.traceback.cut(path=node.funcargs['script'])
#    report.longrepr = node.repr_failure(excinfo)