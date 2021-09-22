/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__RPMSG_LITE_BASE_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_LITE_BASE_TRANSPORT_H_

#include "erpc_transport.h"
#include "erpc_config_internal.h"

#include "rpmsg_lite.h"
#include "rpmsg_env_specific.h"

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
#ifndef RL_USE_STATIC_API
#warning "RPMSG is not set to use static allocation"
#endif
#endif


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
#if RL_USE_STATIC_API
    struct rpmsg_lite_instance m_static_context;
    struct rpmsg_lite_ept_static_context m_ept_context;
    rpmsg_static_queue_ctxt m_queue_context;
    uint8_t m_queue_stack[RL_ENV_QUEUE_STATIC_STORAGE_SIZE];
#endif
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__RPMSG_LITE_BASE_TRANSPORT_H_
