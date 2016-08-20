#!/usr/bin/env python

# Copyright (c) 2014-2016 Freescale Semiconductor, Inc.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# o Redistributions of source code must retain the above copyright notice, this list
#   of conditions and the following disclaimer.
#
# o Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# o Neither the name of Freescale Semiconductor, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
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

from __future__ import print_function
from io import open
import sys
import os
import argparse
import string

# Tool version number and copyright string.
kToolVersion = "0.1"
kToolCopyright = "Copyright (c) 2014 Freescale Semiconductor, Inc. All rights reserved."

##
# @brief Class to convert multiple input text files to C sources.
class Text2CConverter(object):
    def __init__(self, outputFile):
        self._outputFile = outputFile
        self._inputs = []
        self._isWide = False

    def addInputFile(self, inputFileName):
        self._inputs.append(inputFileName)

    def setWide(self, isWide):
        self._isWide = isWide

    def convert(self):
        self._writeHeader()

        # Iterate over inputs.
        for i in self._inputs:
            # Open input file as context.
            with open(i, 'r') as f:
                self._convertInput(i, f)

    def _writeHeader(self):
        h = """/*
 * AUTOMATICALLY GENERATED FILE - DO NOT EDIT
 */

"""

        if self._isWide:
            h += "#include <wchar.h>\n\n"

        self._outputFile.write(h)

    def _convertInput(self, name, inputFile):
        # Get the input file's name without path or extension.
        baseName = os.path.splitext(os.path.basename(name))[0]

        # Write the variable declaration.
        varName = self._formVariableName(baseName)
        var = "// Generated from " + name
        if self._isWide:
            var += "\nextern const wchar_t * const " + varName + ";\n"
            var += "\nconst wchar_t * const " + varName + " = \nL"
        else:
            var += "\nextern const char * const " + varName + ";\n"
            var += "\nconst char * const " + varName + " = \n"
        self._outputFile.write(var);

        # Convert each input line.
        for l in inputFile:
            self._outputFile.write(self._quoteLine(l))

        # Terminate the variable and file.
        self._outputFile.write(";\n\n");

    def _formVariableName(self, fileName):
        return 'k' + ''.join([i.capitalize() for i in fileName.replace('_', ' ').split()])

    def _quoteLine(self, line):
        return '"' + self._escapeline(line) + '"' + '\n';

    def _escapeline(self, line):
        # Remove CRLF line ending.
        if (len(line) >= 2 and line[-2] == '\r' and line[-1] == '\n'):
            line = line[:-2] + line[-1]
        escaped = ''
        for c in line:
            if c == '"':
                escaped += r'\"'
            elif c == '\\':
                escaped += '\\\\'
            elif c == '\n':
                escaped += r'\n'
            elif c not in string.printable:
                escaped += r'\x' + "%02x" % ord(c)
            else:
                escaped += c
        return escaped

##
# @brief Tool to generate register header files from XML.
class Text2CConverterTool(object):

    def _printVersion(self):
        print (os.path.basename(sys.argv[0]), kToolVersion)
        print (kToolCopyright)

    ##
    # @brief Read the command line and generate the output file.
    #
    # This method is the meat of the class; all other methods are here to support this one.
    #
    def run(self):
        # Process command line options.
        args = self._readOptions()
        self.args = args

        # Check for version option.
        if args.showVersion:
            self._printVersion()
            return 0
#         print args.input
        if args.output is None:
            args.output = sys.stdout

        converter = Text2CConverter(args.output)

        if args.wide:
            converter.setWide(True)

        for i in args.input:
            converter.addInputFile(i)

#         try:
        converter.convert()
#         except e:
#             print "Error:", e
#             return 1

        return 0

    ##
    # @brief Parse command line options into an options dictionary.
    #
    # @return An object is returned that contains the command line options as attributes.
    def _readOptions(self):
        # Build arg parser.
        parser = argparse.ArgumentParser(
                    formatter_class=argparse.RawDescriptionHelpFormatter,
                    description="Convert a text file to a C source file.")

        # Options
        parser.add_argument("-V", "--version", action="store_true", dest="showVersion", help="Show version information.")
        parser.add_argument("-o", "--output", metavar="PATH", type=argparse.FileType('w'), help="Specify the output file.")
        parser.add_argument("-n", "--name", metavar="IDENT", help="Set the string variable's name.")
        parser.add_argument("-w", "--wide", action="store_true", help="Make the output a wide string (wchar_t).")

        # Positional parameters
        parser.add_argument("input", nargs='*', help="Input text files.")

        return parser.parse_args()

# Are we being executed directly?
if __name__ == '__main__':
    ret = Text2CConverterTool().run()
