/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_error_handler.h"
#include "fsl_debug_console.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

bool g_erpc_error_occurred = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void erpc_error_handler(erpc_status_t err, uint32_t functionID)
{
    if (err == (erpc_status_t)kErpcStatus_Success)
    {
        return;
    }

    switch (err)
    {
        case (erpc_status_t)kErpcStatus_Fail:
            (void)PRINTF("\r\nGeneric failure.");
            break;

        case (erpc_status_t)kErpcStatus_InvalidArgument:
            (void)PRINTF("\r\nArgument is an invalid value.");
            break;

        case (erpc_status_t)kErpcStatus_Timeout:
            (void)PRINTF("\r\nOperated timed out.");
            break;

        case (erpc_status_t)kErpcStatus_InvalidMessageVersion:
            (void)PRINTF("\r\nMessage header contains an unknown version.");
            break;

        case (erpc_status_t)kErpcStatus_ExpectedReply:
            (void)PRINTF("\r\nExpected a reply message but got another message type.");
            break;

        case (erpc_status_t)kErpcStatus_CrcCheckFailed:
            (void)PRINTF("\r\nMessage is corrupted.");
            break;

        case (erpc_status_t)kErpcStatus_BufferOverrun:
            (void)PRINTF("\r\nAttempt to read or write past the end of a buffer.");
            break;

        case (erpc_status_t)kErpcStatus_UnknownName:
            (void)PRINTF("\r\nCould not find host with given name.");
            break;

        case (erpc_status_t)kErpcStatus_ConnectionFailure:
            (void)PRINTF("\r\nFailed to connect to host.");
            break;

        case (erpc_status_t)kErpcStatus_ConnectionClosed:
            (void)PRINTF("\r\nConnected closed by peer.");
            break;

        case (erpc_status_t)kErpcStatus_MemoryError:
            (void)PRINTF("\r\nMemory allocation error.");
            break;

        case (erpc_status_t)kErpcStatus_ServerIsDown:
            (void)PRINTF("\r\nServer is stopped.");
            break;

        case (erpc_status_t)kErpcStatus_InitFailed:
            (void)PRINTF("\r\nTransport layer initialization failed.");
            break;

        case (erpc_status_t)kErpcStatus_ReceiveFailed:
            (void)PRINTF("\r\nFailed to receive data.");
            break;

        case (erpc_status_t)kErpcStatus_SendFailed:
            (void)PRINTF("\r\nFailed to send data.");
            break;

        /* unhandled error */
        default:
            (void)PRINTF("\r\nUnhandled error occurred.");
            break;
    }

    /* When error occurred on client side. */
    if (functionID != 0U)
    {
        (void)PRINTF("Function id '%u'.", (unsigned int)functionID);
    }
    (void)PRINTF("\r\n");

    /* error occurred */
    g_erpc_error_occurred = true;
}
