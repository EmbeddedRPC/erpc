/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__RPMSG_LITE_BASE_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_LITE_BASE_TRANSPORT_H_

#include "erpc_transport.h"

#include "rpmsg_lite.h"

/*!
 * @addtogroup rpmsg_lite_transport
 * @addtogroup rpmsg_lite_rtos_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Transport that other RPMsg transports inherits.
 *
 * @ingroup rpmsg_lite_transport
 * @ingroup rpmsg_lite_rtos_transport
 */
class RPMsgBaseTransport : public Transport
{
public:
    RPMsgBaseTransport(void)
    : Transport(){};

    virtual ~RPMsgBaseTransport(void) {}

    /*!
     * @brief This function returns pointer to instance of RPMSG lite
     *
     * @retval pointer to instance of RPMSG lite
     */
    struct rpmsg_lite_instance *get_rpmsg_lite_instance(void) { return s_rpmsg; }

protected:
    static struct rpmsg_lite_instance *s_rpmsg; /*!< Pointer to instance of RPMSG lite. */
    static uint8_t s_initialized;               /*!< Represent information if the rpmsg-lite was initialized. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__RPMSG_LITE_BASE_TRANSPORT_H_
