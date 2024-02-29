#!/usr/bin/python

# Copyright 2017 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import print_function

import sys, random, argparse

try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()

# eRPC service code
import service
from service import erpc_matrix_multiply

# make input function independent on Python version
input_fn = raw_input if sys.version_info[:2] <= (2, 7) else input

###############################################################################
# Variables
###############################################################################

# get matrix size defined in IDL
matrix_size = erpc_matrix_multiply.common.matrix_size

# define maximum random value matrix can have
MAX_VALUE = 50

###############################################################################
# Functions
###############################################################################

## Create matrix with specified size and random data
def fillMatrix(size, maxRandomValue):
    return [[random.randrange(maxRandomValue) for x in range(size)] for e in range(size)]

## Print matrix
def printMatrix(matrix):
    for i in range(len(matrix)):
        for j in range(len(matrix[0])):
            print('%04d' % matrix[i][j], end=" ")
        print()

###############################################################################
# Server
###############################################################################

## Matrix multiply service handler
class MatrixMultiplyServiceHandler(erpc_matrix_multiply.interface.IMatrixMultiplyService):
    ## eRPC matrix multiply method
    # @param matrix1 First matrix to be multiplied
    # @param matrix2 Second matrix to be multiplied
    # @param resultMatrix Reference object for storing result of matrix multiplication
    def erpcMatrixMultiply(self, matrix1, matrix2, resultMatrix):
        print('\r\nServer received these matrices:')
        print('\r\nMatrix #1\r\n=========')
        printMatrix(matrix1)

        print('\r\nMatrix #2\r\n=========')
        printMatrix(matrix2)

        # clear the result matrix
        resultMatrix.value = [[0 for x in range(matrix_size)] for y in range(matrix_size)]

        # multiply input matrices
        for i in range(matrix_size):
            for j in range(matrix_size):
                for k in range(matrix_size):
                    resultMatrix.value[i][j] += matrix1[i][k] * matrix2[k][j]

        print('\r\nResult matrix\r\n=============')
        printMatrix(resultMatrix.value)
        sys.stdout.flush()

## Run server on specified transport layer
def runServer(transport):
    # create matrix multiply eRPC service
    handler = MatrixMultiplyServiceHandler()
    service = erpc_matrix_multiply.server.MatrixMultiplyServiceService(handler)

    # run server
    server = erpc.simple_server.SimpleServer(transport, erpc.basic_codec.BasicCodec)
    server.add_service(service)
    print('\r\nWait for client to send a eRPC request')
    sys.stdout.flush()
    server.run()

###############################################################################
# Client
###############################################################################

## Run client on specified transport layer
def runClient(transport):
    # create matrix multiply eRPC service
    clientManager = erpc.client.ClientManager(transport, erpc.basic_codec.BasicCodec)
    client = erpc_matrix_multiply.client.MatrixMultiplyServiceClient(clientManager)

    while True:
        # create matrices with random values
        matrix1 = fillMatrix(matrix_size, MAX_VALUE)
        matrix2 = fillMatrix(matrix_size, MAX_VALUE)

        # print matrices to the console
        print('\r\nMatrix #1\r\n=========')
        printMatrix(matrix1)

        print('\r\nMatrix #2\r\n=========')
        printMatrix(matrix2)

        # create result matrix as eRPC reference object
        resultMatrix = erpc.Reference()

        # send request to the server
        print('\r\neRPC request is sent to the server')
        client.erpcMatrixMultiply(matrix1, matrix2, resultMatrix)

        # print result matrix's value
        print('\r\nResult matrix\r\n=============')
        printMatrix(resultMatrix.value)

        # wait for key press
        print('\r\nPress Enter to initiate the next matrix multiplication')
        sys.stdout.flush()
        input_fn()

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    # parse cmd parameters
    argParser = argparse.ArgumentParser(description='eRPC Matrix Multiply example')
    argParser.add_argument('-c', '--client', action='store_true', help='Run client')
    argParser.add_argument('-s', '--server', action='store_true', help='Run server')
    argParser.add_argument('-t', '--host', default='localhost', help='Host IP address (default value is localhost)')
    argParser.add_argument('-p', '--port', default='40', help='Port (default value is 40)')
    argParser.add_argument('-S', '--serial', default=None, help='Serial device (default value is None)')
    argParser.add_argument('-B', '--baud', default='115200', help='Baud (default value is 115200)')

    args = argParser.parse_args()

    # check if either server or client has been selected
    if not args.client and not args.server:
        argParser.print_help()
        sys.exit('\r\nEither server or client has to be selected to run')

    print('eRPC Matrix Multiply TCP example')

    if args.serial:
        # initialize Serial transport layer
        transport = erpc.transport.SerialTransport(args.serial, int(args.baud))
    else:
        # initialize TCP transport layer
        transport = erpc.transport.TCPTransport(args.host, int(args.port), args.server)

    if args.client:
        print('Client connecting to a host on %s:%s' % (args.host, args.port))
        runClient(transport)

    elif args.server:
        # Note: TCP server is ran in daemon thread, so flush is needed to see printouts
        print('Server created on \"%s:%s\"' % (args.host, args.port))
        runServer(transport)
