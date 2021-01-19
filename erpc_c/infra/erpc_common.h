/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ERPC_COMMON_H_
#define _EMBEDDED_RPC__ERPC_COMMON_H_

#include <stdint.h>

/*!
 * @addtogroup infra
 * @{
 */

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/*! @brief eRPC status return codes. */
enum _erpc_status
{
    //! No error occurred.
    kErpcStatus_Success = 0,

    //! Generic failure.
    kErpcStatus_Fail = 1,

    //! Argument is an invalid value.
    kErpcStatus_InvalidArgument = 4,

    //! Operated timed out.
    kErpcStatus_Timeout = 5,

    //! Message header contains an unknown version.
    kErpcStatus_InvalidMessageVersion = 6,

    //! Expected a reply message but got another message type.
    kErpcStatus_ExpectedReply,

    //! Message is corrupted.
    kErpcStatus_CrcCheckFailed,

    //! Attempt to read or write past the end of a buffer.
    kErpcStatus_BufferOverrun,

    //! Could not find host with given name.
    kErpcStatus_UnknownName,

    //! Failed to connect to host.
    kErpcStatus_ConnectionFailure,

    //! Connected closed by peer.
    kErpcStatus_ConnectionClosed,

    //! Memory allocation error.
    kErpcStatus_MemoryError,

    //! Server is stopped.
    kErpcStatus_ServerIsDown,

    //! Transport layer initialization failed.
    kErpcStatus_InitFailed,

    //! Failed to receive data.
    kErpcStatus_ReceiveFailed,

    //! Failed to send data.
    kErpcStatus_SendFailed,

    //! Sending/Receiving callback function which is not defined in IDL.
    kErpcStatus_UnknownCallback,

    //! Calling eRPC function from another eRPC function. For more information see erpc_common.h.
    kErpcStatus_NestedCallFailure,

    //! When sending address from bigger architecture to smaller.
    kErpcStatus_BadAddressScale
};

/*! @brief Type used for all status and error return values. */
typedef enum _erpc_status erpc_status_t;

/*! @} */

#endif /* _EMBEDDED_RPC__ERPC_COMMON_H_ */
