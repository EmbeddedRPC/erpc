import shlex
import sys
from time import sleep
from subprocess import Popen, PIPE, TimeoutExpired
from typing import Optional
import os
import argparse

tests_list = [
    # "test_arbitrator",
    "test_annotations",
    "test_arrays",
    "test_binary",
    "test_builtin",
    "test_const",
    # "test_callbacks",
    "test_enums",
    "test_lists",
    "test_struct",
    "test_typedef",
    # "test_unions"
]

PYTHON_TESTS_PATH = r"..\python_impl_tests"


def to_pascal_case(text: str, separator: str = "_") -> str:
    return "".join([i.capitalize() for i in text.split("_")])


def python_server_test_cmd(test_name: str) -> list[str]:
    return shlex.split(
        f"pytest {test_name}/{test_name}.py --server"
    )


class Runner:
    def __init__(self, server: str, client: str, port: int = 12345, mvn: str = "mvn", host="localhost"):
        self.port = port
        self.host = host
        self.mvn = mvn

        if server == "java":
            self.server = self.java_server
        elif server == "py":
            self.server = self.python_server
        else:
            raise AttributeError

        if client == "java":
            self.client = self.java_client
        elif client == "py":
            self.client = self.python_client
        else:
            raise AttributeError

    def execute_test(self, test_name: str):
        print("=================================")
        print(f"Running test case: {test_case}")
        print("=================================")

        server = self.server(test_name)
        sleep(1)
        client = self.client(test_name)

        try:
            print("============ SERVER =============")
            stdout, stderr = server.communicate(timeout=10)
        except TimeoutExpired:
            print("ERROR: TIMEOUT")
            server.kill()
            stdout, stderr = server.communicate()

        print(stdout.decode(errors="ignore"))
        print(stderr.decode())
        print("=================================")

        try:
            print("============ CLIENT =============")
            stdout, stderr = client.communicate(timeout=10)
        except TimeoutExpired:
            print("ERROR: TIMEOUT")
            client.kill()
            stdout, stderr = client.communicate()

        print(stdout.decode(errors="ignore"))
        print(stderr.decode())

    def java_client(self, test_name: str):
        return Popen(
            f"{self.mvn} test -Dtest={to_pascal_case(test_name)} -Dclient "
            f"-Dport={self.port} -Dhost={self.host}",
            stdout=PIPE, stderr=PIPE
        )

    def java_server(self, test_name: str):
        return Popen(
            f"{self.mvn} test -Dtest={to_pascal_case(test_name)}Server -Dserver "
            f"-Dport={self.port} -Dhost={self.host}",
            stdout=PIPE, stderr=PIPE
        )

    def python_client(self, test_name: str):
        return Popen(
            f"pytest {test_name}/{test_name}.py --client --port {self.port} --host {self.host}",
            cwd=PYTHON_TESTS_PATH,
            stdout=PIPE, stderr=PIPE
        )

    def python_server(self, test_name: str):
        return Popen(
            f"pytest {test_name}/{test_name}.py --server --port {self.port} --host {self.host}",
            cwd=PYTHON_TESTS_PATH,
            stdout=PIPE, stderr=PIPE
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="Run Java and Python tests.")

    # Options
    parser.add_argument("-s", "--server", default="java", choices=["java", "py"],
                        help="Select server side implementation: java or py")
    parser.add_argument("-c", "--client", default="java", choices=["java", "py"],
                        help="Select client side implementation: java or py")
    parser.add_argument("-m", "--maven", default="mvn", help="Path to Maven executable")
    parser.add_argument("-p", "--port", default=12345, type=int, help="Port")

    args = parser.parse_args()

    runner = Runner(args.server, args.client, args.port, args.maven)

    for test_case in tests_list:
        runner.execute_test(test_case)
