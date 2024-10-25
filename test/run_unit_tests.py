#! /usr/bin/python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import annotations

import argparse
import itertools
import os
import re
import shutil
import sys
from abc import ABC, abstractmethod
from datetime import datetime
from pathlib import Path
from subprocess import run, Popen, TimeoutExpired, PIPE
from time import sleep, time
from typing import Any, Type
from xml.etree import ElementTree


def to_pascal_case(text: str, separator: str = "_") -> str:
    return "".join([i.capitalize() for i in text.split("_")])


class TestResult:

    def __init__(self):
        self.skipped: set[str] = set()
        self.passed: set[str] = set()
        self.failed: set[str] = set()
        self.test_suit_name: str | None = None

    @property
    def all(self) -> set[str]:
        return self.skipped.union(self.passed, self.failed)

    @property
    def return_code(self) -> int:
        return len(self.failed)

    def set_test_suit_name(self, test_suit_name: str):
        self.test_suit_name = test_suit_name

    def add_failed(self, test_case: str):
        self.failed.add(test_case)

    def add_passed(self, test_case: str):
        self.passed.add(test_case)

    def add_skipped(self, test_case: str):
        self.skipped.add(test_case)

    def print_result(self):
        padding = 40
        print(f"{'':*^{padding}}")
        print(f"* {'TEST RESULTS':^{padding - 4}} *")

        if self.test_suit_name:
            print(f"* {self.test_suit_name:^{padding - 4}} *")
        print(f"{'':*^{padding}}")

        print(f"\n{len(self.passed)}/{len(self.all)} passed, "
              f"{len(self.failed)} failed, "
              f"{len(self.skipped)} skipped.")

        if len(self.failed) != 0:
            print(f"\n{bcolors.RED}Failed tests:")
            print("\n".join(f"- {t}" for t in self.failed))
            print(bcolors.ENDC)

        if len(self.skipped) != 0:
            print(f"\n{bcolors.ORANGE}Skipped tests:")
            print("\n".join(f"- {t}" for t in self.skipped))
            print(bcolors.ENDC)

        if len(self.failed) == 0:
            print(f"\n{bcolors.GREEN}All tests passed.{bcolors.ENDC}")


class AbstractTests(ABC):
    """
    Base class for testing eRPC with different languages.

    Define this abstract functions:
        - client(): This function should start the eRPC client with test cases
        - test(): This function should start the eRPC server
        - test_case_server_exist(), test_case_client_exist(): Return if client/server implementation in given language exists
        - init_tests(): Init anything that concreate class need to run tests (build, flash, download, ...)

    How to use:
        - Init client and server with all arguments
        - Iter through client's test_cases
            - call init_test(test_case, test_suit_name, transport) on client and server
            - Check if test case exist for client/server
            - call server() on given implementation
            - call client() on given implementation
            - call wait_client() to get results and wait for client's termination
            - call wait_servet() to get results (if server exited correctly)
    """
    test_dir_regex = re.compile("test_*")

    def __init__(self, result_dir: str | None = None, build_dir: str | None = None, pristine: bool = False,
                 test_cases: set[str] | None = None, add_test_prefix: bool = False, erpcgen: str | None = None,
                 **kwargs: dict[str, Any]) -> None:
        self.test_dir = Path(os.path.dirname(os.path.abspath(__file__)))
        self.erpc_dir = self.test_dir.parent
        self.result_dir = Path(result_dir).absolute() if result_dir else self.test_dir.joinpath("results")
        self.build_dir = Path(build_dir).absolute() if build_dir else self.erpc_dir.joinpath("build")

        if erpcgen:
            self.erpcgen_executable = Path(erpcgen).absolute()
        elif erpcgen := shutil.which("erpcgen"):
            self.erpcgen_executable = Path(erpcgen)
        elif self.executable_exists(self.build_dir.joinpath("erpcgen/").joinpath("erpcgen")):
            self.erpcgen_executable = self.build_dir.joinpath("erpcgen/").joinpath("erpcgen")
        else:
            self.erpcgen_executable = None

        self.add_test_prefix = add_test_prefix

        if not self.result_dir.exists():
            self.result_dir.mkdir()

        self.pristine = pristine

        if test_cases:
            self.test_cases = set(test_cases)
        else:
            self.test_cases = set(self.test_dir.joinpath(p).name for p in os.listdir(self.test_dir) if
                                  self.is_test_dir(self.test_dir.joinpath(p)))

        self.server_process: Popen[bytes] | None = None
        self.client_process: Popen[bytes] | None = None
        self._test_case: str | None = None
        self._test_suit_name: str | None = None
        self._transport: str | None = None

        self.client_extra_parameters: list[str] = []
        self.server_extra_parameters: list[str] = []

    @property
    def test_case(self) -> str:
        if self._test_case is None:
            raise RuntimeError("Call init_test before client/server.")
        return self._test_case

    @test_case.setter
    def test_case(self, value: str) -> None:
        self._test_case = value

    @property
    def test_suit_name(self) -> str:
        if self._test_suit_name is None:
            raise RuntimeError("Call init_test before client/server.")
        return self._test_suit_name

    @test_suit_name.setter
    def test_suit_name(self, value: str) -> None:
        self._test_suit_name = value

    @property
    def transport(self) -> str:
        if self._transport is None:
            raise RuntimeError("Call init_test before client/server.")
        return self._transport

    @transport.setter
    def transport(self, value: str) -> None:
        self._transport = value

    def init_test(self, test_case: str, test_suit_name: str, transport: str) -> bool:
        """
        Initialise next test case values. This function have to be called before each test case.
        Can be overridden, bud the function must call super().init_test(test_case, test_suit_name, transport).

        @param test_case: Name of the test case
        @param test_suit_name: Name of the test suit (tcp_c_c, ...)
        @param transport: tcp/serial
        @return: Result
        """
        self.test_case = test_case
        self.test_suit_name = test_suit_name
        self.transport = transport

        return True

    def add_client_parameter(self, parameter: str):
        self.client_extra_parameters.append(parameter)

    def add_server_parameter(self, parameter: str):
        self.server_extra_parameters.append(parameter)

    def wait_server(self, timeout: int = 5) -> bool:
        """
        Wait for server to termine, otherwise kill the process and return False
        @param timeout: Server wait timeout
        @return: True if server terminate itself and correctly, False otherwise
        """
        if self.server_process is None:
            return True

        try:
            result = self.server_process.wait(timeout) == 0
        except TimeoutExpired:
            print(f"{bcolors.RED}SERVER TIMEOUT{bcolors.ENDC}")
            self.server_process.kill()
            result = False

        self.server_process = None
        return result

    def wait_client(self, timeout: int = 5) -> bool:
        """
        Wait for client to termine, otherwise kill the process and return False
        @param timeout: Client wait timeout
        @return: True if client terminate itself and correctly, False otherwise
        """
        if self.client_process is None:
            return True

        try:
            result = self.client_process.wait(timeout) == 0
        except TimeoutExpired:
            print(f"{bcolors.RED}CLIENT TIMEOUT{bcolors.ENDC}")
            self.client_process.kill()
            self.create_failed_test_suite_xml(
                self.test_case,
                self.get_result_file()
            )
            result = False

        result_file = self.get_result_file()

        if self.add_test_prefix:
            self.update_xml_testcases_name(
                result_file,
                self.test_suit_name
            )

        self.client_process = None
        return result

    def get_result_file(self) -> Path:
        """
        Get Path to result xml file base on result_dir, test_suit_name and test_case
        @return: Path
        """
        return self.result_dir.joinpath(f"{self.test_suit_name}_{self.test_case}.xml")

    @abstractmethod
    def init_tests(self) -> bool:
        pass

    @abstractmethod
    def generate_shim_code(self):
        pass

    @abstractmethod
    def test_case_client_exist(self, test_case: str) -> bool:
        """
        Return true if given test_case exists, false otherwise
        """
        pass

    @abstractmethod
    def test_case_server_exist(self, test_case: str) -> bool:
        """
        Return true if given test_case exists, false otherwise
        """
        pass

    @abstractmethod
    def client(self) -> None:
        """
        Client implementation of given language. It is called for each test case.
        Can set self.client_process and then client_wait() to wait for process to termine.

        self.client_extra_parameters can be used to modify client command from global scope.
        """
        pass

    @abstractmethod
    def server(self) -> None:
        """
        Client implementation of given language. It is called for each test case.
        Can set self.server_process and then server_wait() to wait for process to terminate.

        self.server_extra_parameters can be used to modify client command from global scope.
        """
        pass

    @staticmethod
    def is_test_dir(directory: Path):
        return directory.is_dir and AbstractTests.test_dir_regex.match(directory.name)

    @staticmethod
    def create_failed_test_suite_xml(test_case: str, result_file: Path):
        """
        Create empty failed xml result if test timeout.
        @param test_case: Name of the test_case
        @param result_file: Output file
        """
        xml = f"""<?xml version="1.0" encoding="UTF-8"?>
    <testsuites tests="0" failures="1" disabled="0" errors="0" timestamp="{datetime.now().strftime("%Y-%m-%dT%H:%M:%S")}" time="0.000" name="AllTests">
      <testsuite name="{test_case}" tests="0" failures="1" disabled="0" errors="0" time="0.000">
        <testcase name="{test_case}" status="run" time="0.015" classname="{test_case}" />
      </testsuite>
    </testsuites>
            """

        with open(result_file, mode="w", encoding="utf8") as fp:
            fp.write(xml)

    @staticmethod
    def update_xml_testcases_name(xml_file: Path, prefix: str = "", suffix: str = ""):
        """
        Update xml testcase name with prefix and suffix for better organization.
        (Simpler thant trying to change the name in pytest/gtest)
        @param xml_file: Path to the xml report
        @param prefix: Test case prefix
        @param suffix: Test case suffix
        """
        tree = ElementTree.parse(xml_file)
        root = tree.getroot()

        for testsuite in root.findall("testsuite") if root.tag != "testsuite" else [root]:
            for testcase in testsuite.findall("testcase"):
                testcase_name = testcase.get("name")
                testcase.set(
                    "name",
                    f"{prefix}{'_' if prefix else ''}{testcase_name}{'_' if suffix else ''}{suffix}"
                )

        tree.write(xml_file)

    @staticmethod
    def executable_exists(path: Path) -> bool:
        return path.exists() or Path(str(path) + ".exe").exists()

    def erpcgen(self, language: str, output_dir: str | Path, idl_file: str | Path, package: str | None = None,
                cwd: str | Path | None = None):
        erpcgen_command = [
            self.erpcgen_executable,
            f"-g{language}",
            f"-o{output_dir}",
            idl_file
        ]

        if package:
            erpcgen_command.insert(1, f"-p{package}")

        print(f"{bcolors.BLUE}Generating: '{' '.join(str(x) for x in erpcgen_command)}' in {cwd}{bcolors.ENDC}")

        erpcgen_proc = Popen(erpcgen_command, cwd=cwd)

        res = erpcgen_proc.wait(10)
        if res != 0:
            raise RuntimeError("Erpcgen execution failed.")


class PythonTests(AbstractTests):

    def __init__(self, tcp_port: int, serial_port: str | None = None, **kwargs: dict[str, Any]) -> None:
        super().__init__(**kwargs)

        self.tcp_port = tcp_port
        self.serial_port = serial_port

    def init_tests(self) -> bool:
        return True

    def test_case_client_exist(self, test_case: str) -> bool:
        return self.pytest_test_path(test_case).exists()

    def test_case_server_exist(self, test_case: str) -> bool:
        return self.pytest_test_path(test_case).exists()

    def client(self) -> None:
        result_file = self.get_result_file()

        pytest_command = [
            "pytest", str(self.pytest_test_path(self.test_case)),
            f"--junitxml={result_file}",
            "--client",
            *self.client_extra_parameters
        ]

        if self.transport == "tcp":
            pytest_command.extend(["--port", str(self.tcp_port)])
        elif self.transport == "serial" and self.serial_port:
            pytest_command.extend(["--serial", self.serial_port])
        else:
            raise ValueError

        self.client_process = Popen(pytest_command)

    def server(self) -> None:
        pytest_command = [
            "pytest", str(self.pytest_test_path(self.test_case)),
            "--server",
            *self.server_extra_parameters
        ]

        if self.transport == "tcp":
            pytest_command.extend(["--port", str(self.tcp_port)])
        elif self.transport == "serial" and self.serial_port:
            pytest_command.extend(["--serial", self.serial_port])
        else:
            raise ValueError

        self.server_process = Popen(pytest_command)
        sleep(0.5)

    def pytest_test_path(self, test_case: str) -> Path:
        return self.test_dir.joinpath("python_impl_tests").joinpath(test_case)

    def generate_shim_code(self):
        for case in self.test_cases:
            if self.test_case_client_exist(case) or self.test_case_server_exist(case):
                self.erpcgen(
                    "py",
                    self.test_dir.joinpath(f"python_impl_tests/{case}/service/"),
                    self.test_dir.joinpath(f"{case}/{case}.erpc"),
                    cwd=self.test_dir.joinpath(f"python_impl_tests/")
                )


class CTests(AbstractTests):

    def __init__(self, c_compiler: str | None = None, cxx_compiler: str | None = None, target: str | None = None,
                 build_all: bool = False, conf_file: str | None = None, menuconfig: bool = False,
                 **kwargs: dict[str, Any]) -> None:
        super().__init__(**kwargs)

        self.menuconfig = menuconfig
        self.c_compiler = c_compiler
        self.cxx_compiler = cxx_compiler
        self.target = target
        self.build_all = build_all
        self.conf_file = Path(conf_file).absolute() if conf_file else self.test_dir.joinpath("prj.conf")

    def init_tests(self) -> bool:
        """
        Generate cmake project and build test one by one or all together if --build/-b is selected.
        If --menuconfig is enabled, show menu config before build.
        @return: set of built tests
        """
        if self.pristine and self.build_dir.exists():
            print(f"{bcolors.ORANGE}Pristine enabled, removing {self.build_dir}.{bcolors.ENDC}")
            shutil.rmtree(self.build_dir)

        if not self.build_dir.exists():
            self.build_dir.mkdir()

        if self.cmake_generate() != 0:
            print(f"{bcolors.RED}CMake project generation FAILED.{bcolors.ENDC}")
            return False

        if self.menuconfig:
            if self.cmake_build_target("menuconfig") != 0:
                print(f"{bcolors.RED}Building menuconfig for Kconfig FAILED.{bcolors.ENDC}")
                return False

        if self.build_all:
            if self.cmake_build_target("test_all") != 0:
                print(f"{bcolors.RED}Tests build FAILED.{bcolors.ENDC}")
                return False
        else:
            for test_case in self.test_cases:
                print(f"{bcolors.BLUE}Building: {test_case}{bcolors.ENDC}")
                if self.cmake_build_target(test_case) != 0:
                    print(f"{bcolors.RED}Test'{test_case}' build FAILED.{bcolors.ENDC}")

        return True

    def test_case_client_exist(self, test_case: str) -> bool:
        return self.executable_exists(self.get_test_executable(test_case, "client", self.transport))

    def test_case_server_exist(self, test_case: str) -> bool:
        return self.executable_exists(self.get_test_executable(test_case, "server", self.transport))

    def client(self) -> None:
        result_file = self.get_result_file()
        client_executable = self.get_test_executable(self.test_case, "client", self.transport)

        self.client_process = Popen(
            [client_executable, f"--gtest_output=xml:{result_file}", *self.client_extra_parameters])

    def server(self) -> None:
        server_executable = self.get_test_executable(self.test_case, "server", self.transport)

        self.server_process = Popen([server_executable, *self.server_extra_parameters])
        sleep(0.5)  # Wait for server to start

    def get_test_executable(self, test: str, test_type: str, transport: str) -> Path:
        """
        Get path to executable of given test, type and transport (work for C++ tests)
        @param test: Name of the test
        @param test_type: client/server
        @param transport: tcp/serial
        @return: Path to the executable
        """
        executable = self.build_dir.joinpath("test").joinpath(test).joinpath(f"{test}_{test_type}_{transport}")

        return executable

    def cmake_generate(self) -> int:
        """
        Generate cmake project in build dir from erpc as source. Conf file, C and C++ compiler can be defined
        @return: Command return code
        """
        cmake_command = ["cmake", "-S", str(self.erpc_dir), "-B", str(self.build_dir),
                         f"-DCMAKE_BUILD_TYPE={self.target}"]

        if self.conf_file:
            cmake_command.append(f"-DCONF_FILE='{self.conf_file}'")

        if self.c_compiler:
            cmake_command.append(f"-DCMAKE_C_COMPILER={self.c_compiler}")

        if self.c_compiler:
            cmake_command.append(f"-DCMAKE_CXX_COMPILER={self.cxx_compiler}")

        return run(cmake_command).returncode

    def cmake_build_target(self, target: str | None = None) -> int:
        """
        Build given target using cmake in build dir
        @param target: Target to build or build all
        @return: Command return code
        """
        if target:
            ret = run(["cmake", "--build", str(self.build_dir), "--target", f"{target}"])
        else:
            ret = run(["cmake", "--build", str(self.build_dir)])

        return ret.returncode

    def generate_shim_code(self):
        pass


class ZephyrTests(AbstractTests):

    def __init__(self, zephyr_base: str, serial_port: int, board: str, **kwargs: dict[str, Any]):
        super().__init__(**kwargs)

        # Zephyr options
        self.zephyr_base = Path(zephyr_base)
        self.zephyr_erpc = self.zephyr_base.joinpath("modules/lib/erpc/")
        self.zephyr_erpc_test = self.zephyr_erpc.joinpath("test/zephyr/uart")
        self.serial_port = serial_port
        self.board = board

    def init_tests(self) -> bool:
        return True

    def init_test(self, test_case: str, test_suit_name: str, transport: str) -> bool:
        super().init_test(test_case, test_suit_name, transport)

        cwd = os.getcwd()
        os.chdir(self.zephyr_base)
        build_dir = Path(f"build/{test_case}").absolute()

        if self.pristine and build_dir.exists():
            print(f"{bcolors.ORANGE}Pristine enabled, removing Zephyr {str(build_dir)}.{bcolors.ENDC}")
            shutil.rmtree(build_dir)

        build_command = ["west", "build", "-b", self.board, f"./modules/lib/erpc/test/zephyr/uart/{test_case}", "-d",
                         f"build/{test_case}"]

        flash_command = ["west", "flash", "--build-dir", str(build_dir)]

        if run(build_command).returncode != 0:
            print(f"{bcolors.RED}Zephyr west build for {test_case} FAILED")
            os.chdir(cwd)
            return False

        if run(flash_command).returncode != 0:
            print(f"{bcolors.RED}Zephyr west flash for {test_case} FAILED")
            os.chdir(cwd)
            return False

        os.chdir(cwd)
        return True

    def test_case_client_exist(self, test_case: str) -> bool:
        raise NotImplemented("Zephyr does not support client tests.")

    def test_case_server_exist(self, test_case: str) -> bool:
        return self.zephyr_erpc_test.joinpath(test_case).exists()

    def client(self) -> None:
        raise NotImplemented("Zephyr does not support client tests.")

    def server(self) -> None:
        pass

    def generate_shim_code(self):
        pass


class JavaTests(AbstractTests):

    def __init__(self, maven: str, tcp_port: int, serial_port: str | None = None, **kwargs: dict[str, Any]) -> None:
        super().__init__(**kwargs)

        self.maven = Path(shutil.which("mvn")) if maven == "mvn" else Path(maven)
        self.tcp_port = tcp_port
        self.serial_port = serial_port
        self.java_test_dir = self.test_dir.joinpath("java_impl_tests/src/test/java/io/github/embeddedrpc/erpc/tests/")
        self.java_result_dir = self.test_dir.joinpath("java_impl_tests/target/surefire-reports")

    def init_tests(self) -> bool:
        return run([self.maven, "generate-test-sources"], cwd=self.test_dir.joinpath("java_impl_tests")).returncode == 0

    def test_case_client_exist(self, test_case: str) -> bool:
        return self.java_test_dir.joinpath("client").joinpath(f"{to_pascal_case(test_case)}.java").exists()

    def test_case_server_exist(self, test_case: str) -> bool:
        return self.java_test_dir.joinpath("server").joinpath(f"{to_pascal_case(test_case)}Server.java").exists()

    def get_result_file(self) -> Path:
        original_file = self.java_result_dir.joinpath(
            f"TEST-io.github.embeddedrpc.erpc.tests.client.{to_pascal_case(self.test_case)}.xml")
        new_file = self.result_dir.joinpath(f"{self.test_suit_name}_{self.test_case}.xml")
        if original_file.exists():
            shutil.move(original_file, new_file)
        else:
            self.create_failed_test_suite_xml(self.test_case, new_file)

        return new_file

    def client(self) -> None:
        mvn_command = [
            self.maven, "test",
            f"-Dtest={to_pascal_case(self.test_case)}",
            "-Dclient",
            *self.client_extra_parameters
        ]

        if self.transport == "tcp":
            mvn_command.append(f"-Dport={str(self.tcp_port)}")
        elif self.transport == "serial" and self.serial_port:
            mvn_command.append(f"-Dserial={self.serial_port}")
        else:
            raise ValueError

        self.client_process = Popen(
            mvn_command,
            cwd=self.test_dir.joinpath('java_impl_tests')
        )

    def wait_client(self, timeout: int = 5) -> bool:
        return super().wait_client(10)

    def server(self) -> None:
        mvn_command = [
            self.maven, "test",
            f"-Dtest={to_pascal_case(self.test_case)}Server",
            "-Dserver",
            *self.server_extra_parameters
        ]

        if self.transport == "tcp":
            mvn_command.append(f"-Dport={str(self.tcp_port)}")
        elif self.transport == "serial" and self.serial_port:
            mvn_command.append(f"-Dserial={self.serial_port}")
        else:
            raise ValueError

        self.server_process = Popen(
            mvn_command,
            cwd=self.test_dir.joinpath('java_impl_tests'),
            stdout=PIPE
        )

        now = time()
        delay = 10

        while time() < now + delay:
            stdout = self.server_process.stdout.readline()

            if stdout.decode().strip() != "":
                print(stdout.decode().strip())
            if f"Running io.github.embeddedrpc.erpc.tests.server" in stdout.decode():
                print("XXX", stdout.decode())
                break

        sleep(1)  # Wait for server to start

    def generate_shim_code(self):
        for case in self.test_cases:
            case_dir = self.test_dir.joinpath(f"java_impl_tests/src/main/java/io/github/embeddedrpc/erpc/tests/{case}/")
            case_idl = self.test_dir.joinpath(f"{case}/{case}.erpc")
            case_package = f"io.github.embeddedrpc.erpc.tests.{case}.erpc_outputs"
            case_cwd = self.test_dir.joinpath(f"java_impl_tests/")

            if self.test_case_client_exist(case) or self.test_case_server_exist(case):
                if case == "test_struct":
                    case_idl = self.test_dir.joinpath(f"{case}/test_struct_java.erpc")

                if case == "test_arrays":
                    case_package = f"io.github.embeddedrpc.erpc.tests.{case}"

                if case == "test_annotations":
                    case_package = f"io.github.embeddedrpc.erpc.tests.{case}"

                self.erpcgen("java", case_dir, case_idl, case_package, case_cwd)


class bcolors:
    # define output text colour class
    GREEN = "\033[36m"
    BLUE = "\033[38;5;097m"
    ORANGE = "\033[38;5;172m"
    RED = "\033[31m"
    ENDC = "\033[0m"


def run_test(client: AbstractTests, server: AbstractTests, test_suit_name: str, transport: str,
             client_extra_parameters: list[str] | None = None,
             server_extra_parameters: list[str] | None = None) -> TestResult:
    """
    Function to run test combination. Uses implementation of AbstractTests class to start test's client and server
    @param client: implementation of AbstractTests
    @param server: implementation of AbstractTests
    @param test_suit_name: Used to rename result file and test_cases
    @param transport: Transport used for testing
    @param client_extra_parameters: Extra parameter for executing client tests
    @param server_extra_parameters: Extra parameter for executing server tests
    @return: TestResult
    """
    if not client_extra_parameters:
        client_extra_parameters = []

    if not server_extra_parameters:
        server_extra_parameters = []

    client.init_tests()
    server.init_tests()
    result = TestResult()

    # Add extra parameters to client/server
    [client.add_client_parameter(x) for x in client_extra_parameters]
    [server.add_client_parameter(x) for x in server_extra_parameters]

    for test_case in client.test_cases:
        # Init each testcase
        client.init_test(test_case, test_suit_name, transport)
        server.init_test(test_case, test_suit_name, transport)

        # Check if test case exist on both sides
        if not client.test_case_client_exist(test_case) or not server.test_case_server_exist(test_case):
            result.add_skipped(test_case)
            continue

        # Start client/server
        server.server()
        client.client()

        # Wait for client to finish tests, and server to shut down
        client_result = client.wait_client(10)
        server_result = server.wait_server(10)

        if client_result and server_result:
            result.add_passed(test_case)
        else:
            result.add_failed(test_case)

    return result


def main(args: argparse.Namespace) -> int:
    """
    Main function that takes parsed arguments and pass them to AbstractTests implementations.
    Then call requested combination.
    @param args: Parsed CLI arguments
    @return: Number of failed tests, 0 if all passed
    """
    params = parser.parse_args().__dict__

    if type(params["generate_shim"]) is list:
        ctest = CTests(**params)
        ctest.cmake_generate()
        ctest.cmake_build_target("erpcgen")
        if "python" in params["generate_shim"] or len(params["generate_shim"]) == 0:
            PythonTests(**params).generate_shim_code()

        if "java" in params["generate_shim"] or len(params["generate_shim"]) == 0:
            JavaTests(**params).generate_shim_code()

    tests: list[tuple[str, str, Type[AbstractTests], Type[AbstractTests], str, list[str], list[str]]] = [
        ("c", "c", CTests, CTests, "tcp", [], []),
        ("c", "python", CTests, PythonTests, "tcp", ["--gtest_filter=-test_struct.TestSendingByrefMembers"], []),
        ("c", "java", CTests, JavaTests, "tcp", ["--gtest_filter=-test_struct.TestSendingByrefMembers"], []),
        ("python", "c", PythonTests, CTests, "tcp", [], []),
        ("python", "python", PythonTests, PythonTests, "tcp", [], []),
        ("python", "java", PythonTests, JavaTests, "tcp", [], []),
        ("java", "c", JavaTests, CTests, "tcp", [], []),
        ("java", "python", JavaTests, PythonTests, "tcp", [], []),
        ("java", "java", JavaTests, JavaTests, "tcp", [], []),
        ("c", "zephyr", CTests, ZephyrTests, "serial", ["--gtest_filter=-test_struct.TestSendingByrefMembers"], []),
        ("python", "zephyr", PythonTests, ZephyrTests, "serial", [], []),
        ("java", "zephyr", JavaTests, ZephyrTests, "serial", [], []),
    ]

    results: list[TestResult] = []
    return_code = 0

    clients = ["c", "python", "java"] if args.client == "all" else [args.client]
    servers = ["c", "python", "java"] if args.server == "all" else [args.server]

    tests = list(
        x[2] for x in  # Take given test suit
        itertools.product(clients, servers, tests)  # From all possible combination
        if x[0] == x[2][0] and x[1] == x[2][1]  # Filter only mathing
    )

    for (client, server, client_cls, server_cls, transport, client_args, server_args) in tests:
        print(f"{bcolors.RED}RUNNING {transport}_{client}_{server} {bcolors.ENDC}")
        result = run_test(
            client_cls(**params),
            server_cls(**params),
            f"{transport}_{client}_{server}",
            transport,
            client_args,
            server_args
        )
        result.set_test_suit_name(f"{transport}_{client}_{server}")

        results.append(result)
        return_code += result.return_code

    for result in results:
        result.print_result()

    return return_code


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter,
        prog=Path(__file__).name,
        description="""Run eRPC unit tests.
Currently supported options (client - server):
    - C - C
    - C - Python
    - C - Java
    - C - Zephyr
    - Python - C
    - Python - Python
    - Python - Java
    - Python - Zephyr
    - Java - C
    - Java - Python 
    - Java - Java 
    - Java - Zephyr""")

    parser.add_argument("test_cases", nargs="*",
                        help="List of tests to execute.")
    parser.add_argument("-B", "--build-dir",
                        help="Cmake build dir. If not specified, 'erpc/build' is used.")
    parser.add_argument("-b", "--build", action="store_true",
                        help="Build all before running tests.")
    parser.add_argument("-p", "--pristine", action="store_true",
                        help="Clear the CMake build dir.")
    parser.add_argument("--c-compiler",
                        help="Set CMake default compiler for C (-DCMAKE_C_COMPILER=<compiler>).")
    parser.add_argument("--cxx-compiler",
                        help="Set CMake default compiler for C++ (-DCMAKE_CXX_COMPILER=<compiler>).")
    parser.add_argument("-T", "--target", choices=["Debug", "Release"], default="Debug",
                        help="Set CMake build type (-DCMAKE_BUILD_TYPE=<target>).")
    parser.add_argument("--transport", choices=["tcp", "serial"], default="tcp",
                        help="Set transport layer if there is more options, "
                             "for example Python-Python only support TCP now.")
    parser.add_argument("--conf-file",
                        help="Set Kconfig config file. Default test/prj.conf")
    parser.add_argument("--zephyr-base",
                        help="Path to Zephyr base directory")
    parser.add_argument("-C", "--client", choices=["python", "c", "java", "all"],
                        help="Set what client should be used. Default C.")
    parser.add_argument("-S", "--server", choices=["zephyr", "python", "c", "java", "all"],
                        help="Set what server should be used. Default C.")
    parser.add_argument("--result-dir",
                        help="Path where xml results from gtest and pytest should be stored. "
                             "Result files have format '[pytest|gtest]_[test_name]_[transport]_[client]_[server].xml'")
    parser.add_argument("--tcp-port", default=12345,
                        help="TCP port used for pytest. TCP port for C test should be specified in prj.conf")
    parser.add_argument("--board", default="mimxrt1060_evkb",
                        help="Board that would be used for zephyr build.")
    parser.add_argument("--serial-port", default="COM4",
                        help="Serial port that would be used for zephyr build.")
    parser.add_argument("--menuconfig", action="store_true",
                        help="If selected, show menuconfig before building. "
                             "This allows user to edit tests parameters and choose what to build.")
    parser.add_argument("--add-test-prefix", action="store_true",
                        help="Add prefix to testcase name corresponding to current test setting. "
                             "TCP Pytohn - C test -> tcp_python_c_test_testName")
    parser.add_argument("--maven", default="mvn", help="Path to maven for Java tests. Default 'mvn'.")
    parser.add_argument("--erpcgen", help="Path to erpcgen. Default 'erpcgen'.")
    parser.add_argument("--generate-shim", nargs="*", help="Generate shim code for Python and Java tests.")

    sys.exit(main(parser.parse_args()))
