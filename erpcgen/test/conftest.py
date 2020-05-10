#! /usr/bin/python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import print_function
import sys
import pytest
import yaml
import subprocess
import re
import itertools
import string
import traceback
import imp
import textwrap
import errno
import shlex
from py import path
import os

# Handle maketrans difference between Python 2 and 3.
try:
    maketrans = string.maketrans
except AttributeError:
    maketrans = str.maketrans

# Import test config. First try to import a local config, otherwise the standard one.
try:
    import config_local as config
except ImportError:
    import config

# Names of files and directories.
RUNS_DIR_NAME = "runs"
LATEST_FILE_NAME = "latest"
OUTPUT_DIR_NAME = "output"
OBJECTS_DIR_NAME = "objects"
IDL_FILE_NAME = "test.erpc"
ERPCGEN_OUT_FILE_NAME = "erpcgen.out"

pytestConfig = None

# Get current, test and erpcgen dirs.
cur_dir = path.local(os.getcwd())
test_dir = path.local(__file__).dirpath()
erpc_dir = test_dir.dirpath().dirpath()

# Add erpc python dir to search path.
sys.path.append(str(erpc_dir.join("erpc_python")))

# Provide symlink support under Windows for Python 2.7.
if sys.version_info[:2] <= (2, 7) and os.name == "nt":
    import ctypes

    ## @brief Symlink function for python27 under windows.
    #
    # based on page: http://stackoverflow.com/questions/6260149/os-symlink-support-in-windows
    def symlink_ms(source, link_name):
        csl = ctypes.windll.kernel32.CreateSymbolicLinkW
        csl.argtypes = (ctypes.c_wchar_p, ctypes.c_wchar_p, ctypes.c_uint32)
        csl.restype = ctypes.c_ubyte
        err = csl(link_name, source.replace('/', '\\'), 1)
        if err == 0:
           raise IOError(err, "failed to create symbolic link", link_name)

    ## @brief islink function for python27 under windows.
    #
    # based on page: http://stackoverflow.com/questions/15258506/os-path-islink-on-windows-with-python
    def islink_ms(link):
        path = link.strpath
        if os.path.exists(path):
            if os.path.isdir(path):
                FILE_ATTRIBUTE_REPARSE_POINT = 0x0400
                attributes = ctypes.windll.kernel32.GetFileAttributesW(unicode(path))
                return (attributes & FILE_ATTRIBUTE_REPARSE_POINT) > 0
            else:
                command = ['dir', path]
                try:
                    with open(os.devnull, 'w') as NULL_FILE:
                        o0 = subprocess.check_output(command, stderr=NULL_FILE, shell=True)
                except subprocess.CalledProcessError as e:
                    print(e.output)
                    return False
                o1 = [s.strip() for s in o0.split('\n')]
                if len(o1) < 6:
                    return False
                else:
                    return 'SYMLINK' in o1[5]
        else:
            return False

    create_symlink = symlink_ms
    islink = islink_ms
else:
    create_symlink = os.symlink
    islink = lambda link: link.islink()

## @brief Create directory for this test run and update 'latest' link.
def create_test_run_dir():
    # Directory to hold the test runs.
    runs = test_dir.join(RUNS_DIR_NAME)
    runs.ensure_dir()

    # Unique directory for this run.
    dir = path.local.make_numbered_dir(prefix='', rootdir=runs, keep=config.RUN_KEEP_COUNT)

    # Update link to last run.
    link = runs.join(LATEST_FILE_NAME)
    if islink(link):
        link.remove()
    if not link.exists():
        reldir = runs.bestrelpath(dir)
        rellink =  cur_dir.bestrelpath(link)
        create_symlink(reldir, rellink)

    return dir

## Root test runs directory.
runs_dir = create_test_run_dir()

## @brief Add command line options.
def pytest_addoption(parser):
    group = parser.getgroup("erpcgen")
    group.addoption('--ev', '--erpcgen-verbosity', action="store", type=int, default=0, metavar="LEVEL", dest="erpcgen_verbosity",
        help="Set verbosity level (1-3, default 0) for erpcgen output and save to erpcgen.out file in test case directory.")
    group.addoption('--el', '--erpcgen-log-execs', action="store_true", dest="erpcgen_log_execs",
        help="Print command lines of executed tools.")

def pytest_configure(config):
    global pytestConfig
    pytestConfig = config

## @brief Generates an ErpcgenFile for valid YAML test spec files.
#
# Files must start with "test" and have an extension of ".yml" to be processed.
def pytest_collect_file(parent, path):
    if path.ext == ".yml" and path.basename.startswith("test"):
        return ErpcgenFile(path, parent)

## @brief Implements collection of erpcgen test cases from YAML files.
#
# An input YAML file may contain multiple test specifications as separate top level YAML
# documents. Each test spec may be parameterized, and results in one or more test cases.
# The ErpcgenTestSpec class is responsible for turning test specs into test cases.
class ErpcgenFile(pytest.File):
    def collect(self):
        verbosity = self.config.getvalue("erpcgen_verbosity")
        docs = yaml.safe_load_all(self.fspath.open())
        for n, d in enumerate(docs):
            name = d.get('name', self.fspath.purebasename + str(n)).replace(' ', '_')
            spec = ErpcgenTestSpec(name, self.fspath, d, verbosity)
            for case in spec:
                yield ErpcgenItem(case.desc, self, case)

## @brief Wraps an ErpcgenTestCase as a pytest test.
class ErpcgenItem(pytest.Item):
    def __init__(self, name, parent, case):
        super(ErpcgenItem, self).__init__(name, parent)
        self.case = case

    def runtest(self):
        self.case.run()

    def repr_failure(self, excinfo):
        """ called when self.runtest() raises an exception. """
        if isinstance(excinfo.value, ErpcgenTestException):
            return excinfo.value.args[0]
#         return str(excinfo.getrepr(showlocals=True, tbfilter=True))
        return "failure: " + str(excinfo)

    def reportinfo(self):
        return self.fspath, 0, "test case: " + self.parent.name + '::' + self.case.desc

## @brief Double standalone single open or close braces.
def filter_braces(text):
    result = ''
    state = 0
    brace = ''
    for c in text:
        if state == 0:
            if c == '{':
                brace = c
                state = 1
            elif c == '}':
                result += '}}'
            else:
                result += c
        elif state == 1:
            if c not in string.whitespace:
                result += '{' + c
                state = 2
            else:
                result += '{{' + c
                state = 0
        elif state == 2:
            if c == '}':
                result += '}'
                state = 0
            else:
                result += c
        else:
            raise Exception("unexpected state")
    if state == 1:
        result += '{{'
    return result

## @brief Wraps a call to the erpcgen tool.
#
# An instance of this class can be used to call erpcgen more than once.
#
# The ERPCGEN config variable is used for the path to the erpcgen tool.
class Erpcgen(object):
    def __init__(self, *args, **kwargs):
        self._path = config.ERPCGEN
        self._args = args
        self._input = kwargs.get('input', None)
        self._language = kwargs.get('language', None)
        self._output_dir = kwargs.get('output', None)
        self._include_dirs = kwargs.get('include', [])
        self._include_dirs.append(str(erpc_dir.join("erpcgen").join("test")))
        self._verbosity = 0

    def set_language(self, lang):
        self._language = lang

    def set_input(self, path):
        self._input = path

    def set_output(self, dir):
        self._output_dir = dir

    def add_include_dir(self, dir):
        self._include_dirs.append(dir)

    def set_verbosity(self, level=1):
        self._verbosity = level

    def run(self, captureOutput=False):
        args = [self._path]
        if self._verbosity > 0:
            args += ["-v"] * self._verbosity
        if self._language is not None:
            args += ["-g", self._language]
        if self._output_dir is not None:
            args += ["-o", self._output_dir]
        for dir in self._include_dirs:
            args += ["-I", dir]
        args += self._args
        if self._input is not None:
            args += [self._input]

        if pytestConfig and pytestConfig.getvalue("erpcgen_log_execs"):
            print("Calling erpcgen:", " ".join(args))

        # Only capture stdout.
        proc = subprocess.Popen(args, stdout=(subprocess.PIPE if captureOutput else None))
        output, _ = proc.communicate()
        if proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, " ".join(args), output)
        return output

## @brief Wraps an invocation of the C/C++ compiler (i.e., gcc or clang)
class CCompiler(object):
    def __init__(self, cwd=None, *args):
        self._cwd = cwd
        self._args = args
        self._path = config.CC
        self._includes = []
        self._sources = []

    def add_include(self, path):
        self._includes.append(path)

    def add_source(self, path):
        self._sources.append(path)

    def run(self, captureOutput=False):
        # Enable all warnings except for unused functions.
        args = [self._path, "-c", "-Wall", "-Werror", "-Wno-unused-function"]
        args += self._args

        for i in self._includes:
            args += ["-I", str(i)]

        for s in self._sources:
            args.append(str(s))

        if pytestConfig and pytestConfig.getvalue("erpcgen_log_execs"):
            print("Calling C/C++ compiler:", " ".join(args))

        cwd = str(self._cwd) if self._cwd is not None else None
        if captureOutput:
            return subprocess.check_output(args, cwd=cwd)
        else:
            subprocess.check_call(args, cwd=cwd)

class ErpcgenTestException(Exception):
    pass

## @brief Handles all aspects of one test case for erpcgen.
#
# This class is responsible for parsing a test spec dict that was read in from a YAML file. It is
# an iterable object that will yield one or more ErpcgenTestCase instances.
class ErpcgenTestSpec(object):
    ## All non-filename keys in a test spec.
    FIXED_KEYS = ('args', 'name', 'idl', 'desc', 'params', 'lang', 'jira', 'skip', 'xfail')

    ## Characters not allowed in a filename.
    BAD_FN_CHARS = '/\\:\r\n\t "<>|?*.%'

    ## Translation table to replace illegal filename characters.
    BAD_FN_TABLE = maketrans(BAD_FN_CHARS, '_' * len(BAD_FN_CHARS))

    def __init__(self, name, path, spec, verbosity=0):
        self.name = name
        self.path = path
        self.spec = spec
        self.idl = spec['idl']
        self.lang = spec.get('lang', 'c')
        self.verbosity = verbosity

        args = spec.get('args', '')
        self.args = shlex.split(args)

        self.param_counts = {}
        self.case_names = []
        self.case_count = 0

    ## @brief Set up values used to generate all test cases for this spec.
    def _prepare(self):
        # Get the path for this spec in the run directory.
        self.test_dir = runs_dir.join(self.path.purebasename, self.name)

        # Extract a dict of output file names and test patterns.
        self.tests = {k:v for k,v in self.spec.items() if k not in self.FIXED_KEYS}

        # Generate parameter permutations.
        self.perms = None
        try:
            params = self.spec['params']
            if params is not None:
                assert isinstance(params, dict), "test spec params must be a dictionary (spec {})".format(self.path)

                # Ensure all param values are lists.
                for k,v in params.items():
                    assert v is not None, "param '{}' must have at least one value (spec {})".format(k, self.path)
                    if not isinstance(v, list):
                        params[k] = [v]

                # Generate all permutations of params.
                #
                # params is first converted into a list of lists of 2-tuples. The tuples have the
                # param name for the first element and param value for the second. This allows us to
                # build param dicts from the results, without losing the param name. It also avoids
                # issues caused by dicts not being ordered.
                #
                # Input:    params = [ {'foo' : [1, 2, 3]}, {'bar', ['a', 'b']} ]
                # Output:   prod =   [ [('foo',1), ('bar':'a')], [('foo',1),('bar','b')], ...]
                prod = itertools.product(*[[(k, v) for v in l] for k,l in params.items()])

                # Convert the permutations back into a list of dicts.
                #
                # Output:   perms =  [ {'foo':1, 'bar':'a'}, {'foo':1, 'bar':'b'}, ...]
                perms = [{v[0]:v[1] for v in p} for p in prod]
#                 print("perms=",perms)

                if len(perms):
                    self.perms = perms

        except KeyError:
            # No params key. Not an error.
            pass

    ## @brief Generator that yields ErpcgenTestCase objects.
    def __iter__(self):
        self._prepare()

        if self.perms is None:
            # No parameterization; yield a single test case.
            yield ErpcgenTestCase(self, self.name, self.idl, self.tests, self.test_dir, {})
        else:
            # Parameterization enabled; yield multiple parameterized test cases.
            for perm in self.perms:
                # Create name for this case.
                caseName = self._get_parametrized_name(perm)
                caseDir = self.test_dir.join(caseName)

                # Parameterize the IDL and test expressions.
                idl = filter_braces(self.idl).format(**perm)
                tests = self._get_parametrized_tests(perm)

                yield ErpcgenTestCase(self, caseName, idl, tests, caseDir, perm)

    ## @brief Generate a unique name for a parametrized test.
    def _get_parametrized_name(self, perm):
        name = ''
        for n,k in enumerate(perm.keys()):
            v = perm[k]

            # Convert parameter value to a short string. If the value is a list or dict, then
            # just use the parameter name following by an incrementing integer.
            if type(v) in (list, dict):
                count = self.param_counts.get(k, 0)
                self.param_counts[k] = count + 1
                valueString = k + str(count)
            else:
                valueString = str(v).translate(self.BAD_FN_TABLE)
                if len(valueString) > 10:
                    valueString = valueString[:10]

            if n != 0:
                name += '_'
            name += valueString

        # Ensure the case name is not empty and is unique. If not, append an int.
        if not name or name in self.case_names:
            name += '_' + str(self.case_count)
            self.case_count += 1
        self.case_names.append(name)

        return name

    ## @brief Substitute params into test patterns.
    def _get_parametrized_tests(self, perm):
        def do_pat(pattern):
            if isinstance(pattern, list):
                return [do_pat(p) for p in pattern if p is not None]
            elif isinstance(pattern, dict):
                # 're' and 'not_re' regex patterns don't have braces filtered.
                pattern = {k:(do_pat(v) if ('re' not in k) else v.format(**perm))
                                for k,v in pattern.items()}
                return pattern
            else:
                return filter_braces(pattern).format(**perm)

        return {filename:do_pat(patterns) for filename, patterns in self.tests.items() if patterns is not None}

    @property
    def desc(self):
        return self.name

## @brief Base class for compile tests.
class ErpcgenCompileTest(object):
    def __init__(self, spec, name, caseDir, outDir):
        self._spec = spec
        self._name = name
        self._case_dir = caseDir
        self._out_dir = outDir

    def run(self):
        pass

## @brief Tests that generated C code will compile successfully.
#
# An objects directory is created under the test case directory. It is used to hold the
# .o files written by the compiler. A .c file with the main() function is also written to
# the objects directory.
class ErpcgenCCompileTest(ErpcgenCompileTest):
    MAIN_CODE = textwrap.dedent("""
        int main(void) {
            return 0;
        }
        """)

    def __init__(self, spec, name, caseDir, outDir):
        super(ErpcgenCCompileTest, self).__init__(spec, name, caseDir, outDir)
        self._objs_dir = caseDir.mkdir(OBJECTS_DIR_NAME)
        self._compiler = CCompiler(self._objs_dir)

    def run(self):
        # TODO run compiler tests on Windows
        if sys.platform == 'win32':
            return

        # Add include directories.
        self._compiler.add_include(erpc_dir.join("erpc_c", "port"))
        self._compiler.add_include(erpc_dir.join("erpc_c", "config"))
        self._compiler.add_include(erpc_dir.join("erpc_c", "infra"))
        self._compiler.add_include(self._out_dir)
       
        # Add all server and client cpp files
        for file in os.listdir(str(self._out_dir)):
            if '.cpp' in file:
                self._compiler.add_source(self._out_dir.join(file))

        # Add all header includes into main code
        headers = ['#include "'+f+'"' for f in os.listdir(str(self._out_dir)) if '.h' in f]
        self.MAIN_CODE = '\n'.join(headers) + self.MAIN_CODE

        # Add both .c and .cpp copies of the main file.
        for main_filename in ("main_c.c", "main_cxx.cpp"):
            main = self._objs_dir.join(main_filename)
            main.write(self.MAIN_CODE)
            self._compiler.add_source(main)

        # Run the compiler.
        self._compiler.run()

## @brief Tests that generated Python code can be successfully compiled.
#
# The generated Python package is loaded. Then the modules within the package are loaded
# successively. Loaded modules are not added into sys.modules.
class ErpcgenPythonCompileTest(ErpcgenCompileTest):
    def _load_module(self, name, dir):
        print("Loading {}".format(name))
        info = imp.find_module(name.split('.')[-1], [str(dir)])
        try:
            mod = imp.load_module(name, *info)
            return mod
        finally:
            # Make sure the file is closed if it's a module.
            if info[0] is not None:
                info[0].close()

    def run(self):
        # List all available packages.
        pkgNames = [f for f in os.listdir(str(self._out_dir)) if os.path.isdir(f)]

        for pkgName in pkgNames:
            # Load generated package.
            pkg = self._load_module(pkgName, self._out_dir)

            # Load modules in the package.
            packageDir = path.local(pkg.__path__[0])
            self._load_module("{}.interface".format(pkgName), packageDir)
            self._load_module("{}.common".format(pkgName), packageDir)
            self._load_module("{}.client".format(pkgName), packageDir)
            self._load_module("{}.server".format(pkgName), packageDir)

## @brief A fully parameterized test case.
#
# Handles actually executing the test. The values passed into the constructor are already
# parametrized. This includes the IDL and output file test patterns.
class ErpcgenTestCase(object):
    ## Map of language names to compilation test classes.
    COMPILE_TEST_CLASSES = {
            'c' :   ErpcgenCCompileTest,
            'py' :  ErpcgenPythonCompileTest,
        }

    def __init__(self, spec, name, idl, tests, caseDir, params):
        self._spec = spec
        self._name = name
        self._idl = idl
        self._tests = tests
        self._case_dir = caseDir
        self._params = params

        self._idl_path = caseDir.join(IDL_FILE_NAME)
        self._out_dir = caseDir.join(OUTPUT_DIR_NAME)

        # Create erpcgen instance.
        erpcgen = Erpcgen(*self._spec.args,  output=str(self._out_dir), input=str(self._idl_path))
        if self._spec.lang is not None:
            erpcgen.set_language(self._spec.lang)
        if self._spec.verbosity:
            erpcgen.set_verbosity(level=self._spec.verbosity)
        self._erpcgen = erpcgen

    def run(self):
        # Create the output dir for this case, and all parent dirs.
        self._out_dir.ensure_dir()

        # Write idl.
        self._idl_path.write(self._idl)

        # Run erpcgen.
        try:
            try:
                output = self._erpcgen.run(captureOutput=(self._spec.verbosity > 0))
            except subprocess.CalledProcessError as e:
                output = e.output
                raise
        finally:
            # We always want to write the output file, so errors can be diagnosed.
            if self._spec.verbosity:
                self._case_dir.join(ERPCGEN_OUT_FILE_NAME).write(output)

        # Examine output.
        self._is_first = True
        for filename, tests in self._tests.items():
            self._test_file(filename, tests)

        # Run through compiler.
        try:
            compileTestClass = self.COMPILE_TEST_CLASSES.get(self._spec.lang, None)
            if compileTestClass is not None:
                compileTestClass(self._spec, self._name, self._case_dir, self._out_dir).run()
        except:
            traceback.print_exc()
            raise

    def _get_line(self, pos):
        return self._contents.count(os.linesep, 0, pos) + 1

    def _get_column(self, pos):
        return pos - self._contents.rfind(os.linesep, 0, pos)

    def _test_file(self, filename, tests):
        # Skip files listed with no patterns.
        if tests is None:
            return

        self._filename = filename
        filepath = self._out_dir.join(filename)
        if not filepath.isfile():
            raise ErpcgenTestException("test specifies invalid file: " + filename)
        self._contents = filepath.read()

        self._pos = 0
        self._not_start_pos = None
        self._not_cases = []

        self._test_cases(tests)

        # If there are leftover not cases, handle them now.
        if self._not_cases:
            self._test_nots(len(self._contents))

    def _test_cases(self, tests):
        # Skip empty pattern lists.
        if tests is None:
            return

        for case in tests:
            # Skip empty cases.
            if case is None:
                continue

            if isinstance(case, dict) and 'if' in case:
                # If-then.
                self._test_if_cases(case)
            elif isinstance(case, dict) and ('not' in case or 'not_re' in case):
                # Record the not case and get back to it later.
                if self._not_start_pos is None:
                    self._not_start_pos = self._pos
                self._not_cases.append(case)
            else:
                # Single pattern.
                self._test_one_case(case)

    def _test_if_cases(self, case):
        ifPredicate = case['if']
        thenCases = case['then']

        if eval(ifPredicate, self._params):
            print("File '{}':{} matched if predicate '{}'".format(self._filename, self._get_line(self._pos), ifPredicate))
            self._test_cases(thenCases)
        elif 'else' in case:
            print("File '{}':{} taking else branch for if predicate '{}'".format(self._filename, self._get_line(self._pos), ifPredicate))
            self._test_cases(case['else'])

    def _test_one_case(self, case):
        # Get the pattern from the case and determine if it's a regular expression.
        isRegex = False
        if isinstance(case, dict):
            if len(case) > 1:
                raise ErpcgenTestException("regular expression pattern dict must have only one 're' key")
            pattern = case['re']
            isRegex = True
        else:
            pattern = case

        # Make sure we haven't hit the end of the file with more patterns to match.
        if self._pos >= len(self._contents):
            raise ErpcTestException("unmatched patterns at end of file {}".format(self._filename))

        # Print a newline to break after py.test prints the test case name.
        if self._is_first:
            print
            self._is_first = False

        # Escape non-regex cases.
        if not isRegex:
            pattern = re.escape(pattern).replace('\ ', '\s*')

        rx = re.compile(pattern, re.MULTILINE)
        match = rx.search(self._contents, self._pos)

        if not match:
            print("File '{}':{} FAILED to find pattern '{}'".format(self._filename, self._get_line(self._pos), pattern))
            raise ErpcgenTestException("file '{}' failed to match against pattern '{!s}' from {}. line".format(self._filename, pattern, self._get_line(self._pos)))
        else:
            self._pos = match.end()
            print("File '{}':{} found pattern '{}' at column {}".format(self._filename, self._get_line(self._pos), pattern, self._get_column(match.start())))

        # Match not cases now that we have an end range for them.
        if self._not_cases:
            self._test_nots(match.start())

    def _test_nots(self, endPos):
        pos = self._not_start_pos

        for case in self._not_cases:
            isRegex = False
            if 'not_re' in case:
                isRegex = True
                pattern = case['not_re']
            else:
                pattern = case['not']

            if not isRegex:
                pattern = re.escape(pattern).replace('\ ', '\s*')

            rx = re.compile(pattern, re.MULTILINE)
            match = rx.search(self._contents, pos, endPos)
            if match:
                print("File '{}':{} FAILED unexpectedly found pattern '{}'".format(self._filename, self._get_line(self._pos), pattern))
                raise ErpcgenTestException("file '{}' unexpected matched pattern '{!s}' from at {}. line".format(self._filename, pattern, self._get_line(pos)))
            else:
                print("File '{}':{}-{} passed negative search for '{}'".format(self._filename, self._get_line(pos), self._get_line(endPos), pattern))

        # Reset not cases.
        self._not_cases = []
        self._not_start_pos = None

    @property
    def desc(self):
        return self._spec.desc + '::' + self._name

# Verify that erpcgen and the compiler are available.
def verify_tools():
    def handle_err(e, toolName, expectedPath, envName):
        if isinstance(e, OSError):
            if e.errno == errno.ENOENT:
                print("Error: {} executable cannot be found.".format(toolName))
                print("Expected {} path: {}".format(toolName, expectedPath))
                print("To change the {} path, set the {} environment variable or create a config_local.py.".format(toolName, envName))
                print("See readme.txt for more information.")
            else:
                print("Fatal error: OS error when verifying {} is available. [errno {}]: {}".format(toolName,
                    e.errno, os.strerror(e.errno)))
        elif isinstance(e, subprocess.CalledProcessError):
            print("Fatal error: failure when verifying {} is available (error code {}).".format(toolName, e.returncode))
            print("Output:")
            print(e.output)
        sys.exit(1)

    try:
        Erpcgen("--version").run(captureOutput=True)
    except (OSError, subprocess.CalledProcessError) as e:
        handle_err(e, "erpcgen", config.ERPCGEN, "ERPCGEN")

    if sys.platform != 'win32':
        try:
            CCompiler(None, "--version").run(captureOutput=True)
        except (OSError, subprocess.CalledProcessError) as e:
            handle_err(e, "compiler", config.CC, "CC")

verify_tools()
