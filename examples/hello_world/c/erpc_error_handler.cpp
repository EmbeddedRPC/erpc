/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_error_handler.h"
#include "stdio.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

extern bool g_erpc_error_occurred;
bool g_erpc_error_occurred = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void erpc_error_handler(erpc_status_t err, uint32_t functionID)
{
    switch (err)
    {
        case kErpcStatus_Fail:
            printf("\r\nGeneric failure.");
            break;

        case kErpcStatus_InvalidArgument:
            printf("\r\nArgument is an invalid value.");
            break;

        case kErpcStatus_Timeout:
            printf("\r\nOperated timed out.");
            break;

        case kErpcStatus_InvalidMessageVersion:
            printf("\r\nMessage header contains an unknown version.");
            break;

        case kErpcStatus_ExpectedReply:
            printf("\r\nExpected a reply message but got another message type.");
            break;

        case kErpcStatus_CrcCheckFailed:
            printf("\r\nMessage is corrupted.");
            break;

        case kErpcStatus_BufferOverrun:
            printf("\r\nAttempt to read or write past the end of a buffer.");
            break;

        case kErpcStatus_UnknownName:
            printf("\r\nCould not find host with given name.");
            break;

        case kErpcStatus_ConnectionFailure:
            printf("\r\nFailed to connect to host.");
            break;

        case kErpcStatus_ConnectionClosed:
            printf("\r\nConnected closed by peer.");
            break;

        case kErpcStatus_MemoryError:
            printf("\r\nMemory allocation error.");
            break;

        case kErpcStatus_ServerIsDown:
            printf("\r\nServer is stopped.");
            break;

        case kErpcStatus_InitFailed:
            printf("\r\nTransport layer initialization failed.");
            break;

        case kErpcStatus_ReceiveFailed:
            printf("\r\nFailed to receive data.");
            break;

        case kErpcStatus_SendFailed:
            printf("\r\nFailed to send data.");
            break;

        /* no error occurred */
        case kErpcStatus_Success:
            return;

        /* unhandled error */
        default:
            printf("\r\nUnhandled error occurred.");
            break;
    }

    /* When error occurred on client side. */
    if (functionID != 0)
    {
        printf("Function id '%u'.", (unsigned int)functionID);
    }
    printf("\r\n");

    /* error occurred */
    g_erpc_error_occurred = true;
}
