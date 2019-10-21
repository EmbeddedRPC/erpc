#!/usr/bin/env python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from setuptools import setup
from erpc import erpc_version
from codecs import open
from os import path

here = path.abspath(path.dirname(__file__))

with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

#steps: https://packaging.python.org/distributing/
#source distribution: python setup.py sdist
#wheel distribution: python setup.py bdist_wheel
#web: https://pypi.python.org/pypi?%3Aaction=submit_form

setup(
    name="erpc",
    version=erpc_version.ERPC_VERSION,
    description="eRPC Python infrastructure",
    long_description=long_description,
    author="NXP",
    url='https://github.com/embeddedrpc/erpc',
    license="BSD 3-Clause",
    install_requires=["enum34","pyserial"],
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: Python",
        "Intended Audience :: Developers",
        "Topic :: Software Development :: Embedded Systems",
        "Natural Language :: English",
        "Operating System :: MacOS :: MacOS X",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX :: Linux",
    ],
    keywords='rpc rpc-framework embedded multicore multiprocessor amp',
    use_2to3=True,
    packages=['erpc'],
)
