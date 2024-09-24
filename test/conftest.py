#! /usr/bin/python

# Copyright (c) 2022 NXP
# Copyright (c) 2022 ACRIOS Systems s.r.o.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import pytest
import py
import subprocess
import os

PyLang = "py"
CLang = "c"

TcpTransport = "tcp"

Langs = [CLang, PyLang]
Transports = [TcpTransport]


class TestException(Exception):
    pass


class TestFile(pytest.File):
    def collect(self):
        for langClient in Langs:
            for langServer in Langs:
                for transport in Transports:
                    case = TestCase(self.fspath.purebasename,
                                    langClient, langServer, transport, self.fspath)
                    yield TestItem.from_parent(self, name=case.desc, case=case)


class TestCase(object):
    def __init__(self, name: str, langClient: str, langServer: str, transport: str, caseDir: str):
        self._name = name
        self._casename = f"{name}_{langClient}_{langServer}_{transport}"
        self._langClient = langClient
        self._langServer = langServer
        self._transport = transport
        self._caseDir = caseDir

    @property
    def desc(self):
        return f"{self._name}_{self._langClient}_{self._langServer}_{self._transport}"

    def run(self):
        timeToWaitForTest = 20
        serverProcess = subprocess.Popen(self._getServerCmd(), shell=True)
        clientProcess = subprocess.Popen(self._getClientCmd(), shell=True)
        serverProcess.wait(timeToWaitForTest)
        isServerKilled = False
        isClientKilled = False
        if serverProcess.poll() == None:
            serverProcess.kill()
            isServerKilled = True
        if clientProcess.poll() == None:
            clientProcess.kill()
            isClientKilled = True
        outsServer, errsServer = serverProcess.communicate()
        outsClient, errsClient = clientProcess.communicate()
        if isServerKilled:
            raise TestException(
                f"Server was killed as app didn't finished in {timeToWaitForTest}s: {outsServer} {errsServer}")
        if isClientKilled:
            raise TestException(
                f"Client was killed as app didn't finished in {timeToWaitForTest}s: {outsClient} {errsClient}")
        if serverProcess.poll() != 0:
            raise TestException(
                f"Server execution failed {serverProcess.poll()}: {outsServer} {errsServer}")
        if clientProcess.poll() != 0:
            raise TestException(
                f"Client execution failed {clientProcess.poll()}: {outsClient} {errsClient}")

    def _getClientCmd(self):
        return self._getAppCmd("client", self._langClient)

    def _getServerCmd(self):
        return self._getAppCmd("server", self._langServer)

    def _getAppCmd(self, clientServerString: str, lang: str):
        appCmd = ""
        if lang == PyLang:
            appCmd = f"{self._name}_{clientServerString}.py"
        elif lang == CLang:
            appCmd = f"Linux/{self._transport}/gcc/{self._name}_{clientServerString}/Debug/{self._name}_{clientServerString}_{self._transport}_test"
        else:
            raise TestException(f"Unknown lang: {lang}")

        appCmd = os.path.join(self._caseDir, appCmd)

        if lang == PyLang:
            appCmd = f"python3 {appCmd}"
        elif lang == CLang:
            pass
        else:
            raise TestException(f"Unknown lang: {lang}")

        return appCmd


class TestItem(pytest.Item):
    def __init__(self, name, parent, case):
        super(TestItem, self).__init__(name, parent)
        self.case = case

    def runtest(self):
        self.case.run()

    def repr_failure(self, excinfo):
        """ called when self.runtest() raises an exception. """
        if isinstance(excinfo.value, TestException):
            return excinfo.value.args[0]
        return "failure: " + str(excinfo)

    def reportinfo(self):
        return self.fspath, 0, "test case: " + self.parent.name.split("/")[0] + '::' + self.case.desc


def pytest_collect_file(path: py._path.local.LocalPath, parent: pytest.Collector):
    if path.basename.startswith("test") and path.basename.endswith("client.py"):
        return TestFile.from_parent(parent, fspath=py._path.local.LocalPath(path.dirname))
